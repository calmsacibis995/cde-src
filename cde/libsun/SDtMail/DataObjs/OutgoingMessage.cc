/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Basic OutgoingMessage Class.

#pragma ident "@(#)OutgoingMessage.cc	1.62 97/03/27 SMI"

// includes
#include <iostream.h>
#include <assert.h>
#include <string.h>
#include <DataObjs/OutgoingMessage.hh>
#include <PortObjs/DataPort.hh>
#include <TranObjs/Request.hh>
#include <TranObjs/ArgumentList.hh>
#include <SDtMail/DeliveryResponse.hh>
#include <TranObjs/SyncRequestEntry.hh>
#include <SDtMail/Session.hh>
#include <Manager/OSess.hh>
#include <Utils/TnfProbe.hh>
#include <SDtMail/MailRc.hh>



SdmOutgoingMessage::SdmOutgoingMessage(SdmError& err, SdmOutgoingStore& dStore, 
        const SdmMessage &derivedFrom)
  :SdmMessage(err, dStore, derivedFrom)
{  
}

SdmOutgoingMessage::SdmOutgoingMessage(SdmError& err, SdmOutgoingStore& dStore)
  :SdmMessage(err, dStore)
{  
}

SdmOutgoingMessage::SdmOutgoingMessage(SdmError& err, SdmMessageStore& dStore, 
            const SdmMessageNumber msgnum)
  :SdmMessage(err, dStore, msgnum)
{
}

SdmOutgoingMessage& 
SdmOutgoingMessage::operator=(const SdmOutgoingMessage &rhs)
{
  cout << "*** Error: SdmOutgoingMessage assignment operator called\n";
  assert(Sdm_False);
  return *this;
}

SdmOutgoingMessage::~SdmOutgoingMessage()
{
}


SdmErrorCode
SdmOutgoingMessage::Submit(SdmError& err, SdmDeliveryResponse &r_response, 
  const SdmMsgFormat format, const SdmBoolean logMessage, const SdmStringL& customHeaders)
{
  TNF_PROBE_0 (SdmOutgoingMessage_Submit_start,
      "api SdmMessage",
      "sunw%start SdmOutgoingMessage_Submit_start");

  TNF_PROBE_0 (SdmOutgoingMessage_Submit_start,
      "async SdmOutgoingMessage",
      "sunw%debug SdmOutgoingMessage::Submit Called ");

  err = Sdm_EC_Success;
  
  if (IsNested()) {
      err = Sdm_EC_MessageIsNested;
  } else {
    SdmSyncRequestEntry entry(err, *this);  
    if (!err) {
      int st = _Submit(err, r_response, format, logMessage, customHeaders);
    }
  }

  TNF_PROBE_0 (SdmOutgoingMessage_Submit_end,
      "api SdmMessage",
      "sunw%end SdmOutgoingMessage_Submit_end");
  return err;  
}

SdmErrorCode 
SdmOutgoingMessage::Submit_Async(SdmError& err, const SdmServiceFunction& svf, 
    void* clientData, const SdmMsgFormat format, const SdmBoolean logMessage,
    const SdmStringL& customHeaders)
{
  TNF_PROBE_0 (SdmOutgoingMessage_Submit_Async_start,
      "api SdmOutgoingMessage",
      "sunw%start SdmOutgoingMessage_Submit_Async_start");

  TNF_PROBE_0 (SdmOutgoingMessage_Submit_Async_start,
      "async SdmOutgoingMessage",
      "sunw%debug SdmOutgoingMessage::Submit_Async Called ");

  err = Sdm_EC_Success;
  
  if (IsNested()) {
    err = Sdm_EC_MessageIsNested;
  } else {
    SdmOutgoingMessage *me = (SdmOutgoingMessage*) this;
    SdmArgumentList* args = new SdmArgumentList;
    args->PutArg((int)format);
    args->PutArg((int)logMessage);
    args->PutArg(customHeaders);
    SdmServiceFunction *svfClone = svf.SdmServiceFunctionFactory(clientData);

    SdmRequest* reqst = new SdmRequest;  
    reqst->SetArguments(args);
    reqst->SetServiceFunction(svfClone);
    reqst->SetOperator(SdmOutgoingMessage::AOP_Submit);
    reqst->SetClientObject(me);

    if (me->_CollectorSession->PostRequest(err, me, reqst)) {
      // need to clean up request and arguments;
      delete reqst;    
      delete args;
      delete svfClone;
    }
  }

  TNF_PROBE_0 (SdmOutgoingMessage_Submit_Async_end,
      "api SdmMessage",
      "sunw%end SdmOutgoingMessage_Submit_Async_end");
  return err;
}


SdmErrorCode
SdmOutgoingMessage::_Submit(SdmError& err, SdmDeliveryResponse &r_response, 
  const SdmMsgFormat format, const SdmBoolean logMessage, const SdmStringL& customHeaders)
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //
  TNF_PROBE_0 (SdmOutgoingMessage__Submit_start,
      "api SdmOutgoingMessage",
      "sunw%start SdmOutgoingMessage__Submit_start");

  TNF_PROBE_0 (SdmOutgoingMessage__Submit_start,
      "async SdmOutgoingMessage",
      "sunw%debug SdmOutgoingMessage::_Submit Called ");

  static const SdmString Sdm_XlibSDtMail_Value("submit=pending");
  
  err = Sdm_EC_Success; 	
  SdmMessageFlagAbstractFlags r_flags;
  assert (_ParentStore != NULL);
    
  if (IsExpunged()) {
    err = Sdm_EC_MessageExpunged;
        
  } else if (!_ParentStore->GetDataPort()->GetMessageFlags(err, r_flags, _MessgNumber)) {

    // return error if message is deleted.
    if (r_flags & Sdm_MFA_Deleted) {
      err = Sdm_EC_SubmitDeletedMessage;

    } else {
      // we use a copy of the message instead of the actual message so that 
      // any changes to the headers (for aliases, etc) will not change affect
      // the actual message.  the assumption is that the call to 
      // ResolveAndExtractAddresses could change the values of the headers.
      SdmMessage *copy = new SdmOutgoingMessage(err, 
              *((SdmOutgoingStore*)_ParentStore), *this);
    
      if (!err) {
        SdmStringL fileList;
        SdmVector<SdmTransportData*> transportDataList;

        // note that we want to resolve the addresses before we save the message
        // in the message store so that when the message is recreated during 
        // reconnect, the headers will not get changed.
        if (!GetAddressesForTransports(err, transportDataList, fileList, copy,
                                       format, customHeaders)) {

          // outgoing store is not connected.  we add the X-libSDtMail header to the message indicating
          // that this message is pending submit.  we then commit the changes made to the message.
          if (!((SdmOutgoingStore*)_ParentStore)->IsConnected()) {
          
            SdmString headerValue = Sdm_XlibSDtMail_Value + " format=" + 
                (format == Sdm_MSFMT_SunV3 ? "sunV3" : "mime");

            int nCustom;
            if (nCustom = customHeaders.ElementCount()) {
              headerValue += " custom=";
              for (int i = 0; i < nCustom; i++) {
                headerValue += customHeaders[i];
                if (i+1 < nCustom)
                  headerValue += ":";
              }
            }

            if (!_ParentStore->GetDataPort()->AddMessageHeader(err, SdmOutgoingStore::Sdm_XlibSDtMail_Header, 
              headerValue, copy->_MessgNumber))
            {
              SdmBoolean r_disposition;
              if (!_ParentStore->GetDataPort()->CommitPendingMessageChanges(err, r_disposition, copy->_MessgNumber, Sdm_False)) {
                err = Sdm_EC_CommitDisconnectedMessage;
              }
            }
            
          } else {  // outgoing store is connected.  we submit the message.

            //
            // check for the X-libSDtMail header.  If it exists, we don't want
            // to include this header in the message we are about to send.
            // if we do include it, then the message will get resent when the
            // receiver's outgoing store is opened.  therefore, we need to 
            // remove this header before we send the message.
            //
            SdmStrStrL r_hdr; 
            _ParentStore->GetDataPort()->GetMessageHeader(err, r_hdr,
                 SdmOutgoingStore::Sdm_XlibSDtMail_Header, copy->_MessgNumber);
 
            if (err == Sdm_EC_RequestedDataNotFound) {
               err = Sdm_EC_Success;    // reset error.    
            } else if (err == Sdm_EC_Success) {
               // remove the X-libSDtMail header.
               _ParentStore->GetDataPort()->RemoveMessageHeader(err,
                 SdmOutgoingStore::Sdm_XlibSDtMail_Header, copy->_MessgNumber);
            }
             
            // add name of log file to file list if we are to log the message.
            if (!err && logMessage && transportDataList.ElementCount() > 0) {
              AddLogFile(err, fileList);
            }

            if (!err) {  
              if (fileList.ElementCount() == 0 && transportDataList.ElementCount() == 0) {
                err = Sdm_EC_NoAddressFoundInMessage;

              } else {
                // second, append message in files if there are any filenames specified.          
                if (fileList.ElementCount() > 0) {
                  AppendMessageToFiles(err, fileList, format);
                }

                // next, send messages to transport.
                if (!err && transportDataList.ElementCount() > 0) { 
                  SendMessageThroughTransports(err, transportDataList, format);
                  if (!err) {
                     GatherSendResults(err, r_response, transportDataList);
                  }

                  // if the message was successfully sent, we set the delete 
                  // flag on the message.
                  if (!err) {
                    _ParentStore->GetDataPort()->SetMessageFlagValues(err, Sdm_True, 
                        Sdm_MFA_Deleted, _MessgNumber);            
                  }

                }  /* end if transport list not empty */
              } /* end if file or transport list not empty */
            }  /* end if no error in adding to log file */       
          }  /* end if connected */
          
          SdmError localError;
          // call CleanUpTransportData with a local error so that we don't lose
          // any error that was set earlier.  only save the error from the
          // cleanup call if the error was not set earlier.
          if (CleanUpTransportData(localError, transportDataList) && !err) {
            err = localError;
          }

        } /* end if !GetAddressesForTransports */
      } /* end if no error in creating message copy */
      
      // delete the copy of the message. need to set _HasSyncLock so that 
      // the destruction of the message does not try to obtain the lock again.
      if (copy) {
        copy->_HasSyncLock = Sdm_True;
        delete copy;
      }
      
    } /* end if message not deleted */
  } /* end if !GetMessageFlags */

  TNF_PROBE_0 (SdmOutgoingMessage__Submit_end,
      "api SdmMessage",
      "sunw%end SdmOutgoingMessage__Submit_end");
  return err;
}


SdmErrorCode
SdmOutgoingMessage::GetAddressesForTransports(SdmError &err, 
  SdmVector<SdmTransportData*> &r_transportDataList, SdmStringL &r_fileList,
  SdmMessage *message, const SdmMsgFormat format, const SdmStringL &customHeaders)
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //
  static SdmString kServiceType("servicetype");
  static SdmString kServiceClass("serviceclass");
  static SdmString kDataPortObject("dataportobject");
  
  err = Sdm_EC_Success;
 
  // remove later once we can query the data port for all the possible transport available.
  SdmStringL transportNames;
  transportNames(-1) = "SMTP";
  transportNames(-1) = "NNTP";

  // set service class of token to message transport.
  SdmToken token;
  SdmMessageNumber nmsgs;
  SdmBoolean readOnlyFlag;

  token.SetValue(kServiceClass,SdmString("messagetransport"));

  // Loop through transports.    
  for (int i=0; i<transportNames.ElementCount(); i++) {
    token.SetValue(kServiceType,transportNames[i]);

    // create, startup, and open transport. if there is an error in opening 
    // or starting up the transport, just continue to  the next transport.
    SdmDataPort *transport  = new SdmDataPort(Sdm_CL_DataPort);
    assert (transport != NULL);
    if (transport->StartUp(err)) {
      delete transport;
      err = Sdm_EC_Success;  // reset error.
      continue;
    }
    
    char buf[10];
    sprintf(buf, "%x", _ParentStore->_DataPort);
    token.SetValue(kDataPortObject, SdmString(buf));

    if (transport->Open(err, nmsgs, readOnlyFlag, token)) {    
      SdmError localError;
      transport->ShutDown(localError);
      delete transport;
      if (err == Sdm_EC_Fail) {
	// reset error and continue only if error code is Sdm_EC_Fail.
	// for other error codes, return them back to caller.
        err = Sdm_EC_Success;  
        continue;
      } else {
        break;   // we will return to caller at end of method.
      }
    }

    SdmStringL r_addressList;
    int numOfFiles = r_fileList.ElementCount();

    // get the addresses that apply for this transport.  if some error occurred continue 
    // from start of loop only if the error is because no addresses are found for this
    // transport.  In this case, we close and shutdown this transport and continue at
    // the beginning of the loop.  
    if (transport->ResolveAndExtractAddresses(err, r_addressList, r_fileList, 
                      *(message->_ParentStore->GetDataPort()), message->_MessgNumber)) 
    {
      SdmError localError;
      transport->Close(localError);
      transport->ShutDown(localError);
      delete transport;
      
      // if no address found for this transport, reset error and continue.
      if (err == Sdm_EC_NoAddressFoundInMessage) {
        err = Sdm_EC_Success;
        continue;
      } else {
        break;
      }

    // we have an address list and/or a file list for this transport.
    // create SdmSubmitMessageData structure with transport.
    // add structure to transportDataList.
    } else {
      // We can skip adding this transport IF no e-mail addresses were added, but at least
      // one file address was added
      if (r_addressList.ElementCount() > 0 || r_fileList.ElementCount() == numOfFiles) {
	SdmTransportData *data = new SdmTransportData;
	data->transport = transport;
	data->error = new SdmError;
	data->deliveryResponse = new SdmDeliveryResponse;
	data->dataport = message->_ParentStore->GetDataPort();
	data->messageNumber = message->_MessgNumber;
	data->format = format;
	data->customHeaders = &customHeaders;
	r_transportDataList.AddElementToList(data);
      }
      // we don't use the address list so clean it up.
      r_addressList.ClearAllElements();
    }
  }
  
  // if an error occurred, clean up transport list.  use a local error object
  // instead of err so we don't lose the value of the original error.
  if (err) {
    SdmError localError;
    CleanUpTransportData(localError, r_transportDataList);
  }
  
  return err;
}

SdmErrorCode
SdmOutgoingMessage::AddLogFile(SdmError &err, SdmStringL &fileList)
{
  err = Sdm_EC_Success;
  
  SdmString logFile;
  char *log;

  // note: GetValue returns copy of string which we are responsible for deleting.
  SdmMailRc::GetMailRc()->GetValue(err, "record", &log);
  if (err == Sdm_EC_RequestedDataNotFound) {
    err = Sdm_EC_Success;    // reset error.
    char* folder;

    // use folder variable to determine location of default log file.
    SdmMailRc::GetMailRc()->GetValue(err, "folder", &folder);
    if (err) {
      err.Reset();  // just reset the error.  we won't specify a path before the file name.
    } else {
      if (*folder != '/' && *folder != '~' && *folder != '+' && *folder != '$') {
        logFile = "$HOME/";
      }
      logFile += folder;
      logFile += "/";
      free (folder);
    }
    logFile += "sent.mail";
  } else {
    logFile = log;
    free (log);
  }

  if (!err) {
    const char* logFileName = (const char*)logFile;
    if (*logFileName != '/' && *logFileName != '~' && *logFileName != '+' && *logFileName != '$') {
      char *buf = (char *)malloc(strlen(logFileName) + 3);
      if (buf != NULL) {
          // "folder" is set. Relative to folder directory
          strcpy(buf, "+");
          strcat(buf, logFileName);
          logFile = buf;
      }
    }
  }

  // add log file to file list.
  fileList(-1) = logFile;

  return err;
}

SdmErrorCode
SdmOutgoingMessage::AppendMessageToFiles(SdmError &err, const SdmStringL &fileList, 
  const SdmMsgFormat format)
{
  err = Sdm_EC_Success;
  SdmString r_messageHeaders;
  SdmString r_messageBody;
  
  if (!SdmMessageUtility::ConstructWholeMessage(err, r_messageHeaders, r_messageBody, 
						*(_ParentStore->GetDataPort()), _MessgNumber, Sdm_False, Sdm_False, Sdm_True))
  {
    for (int i=0; i<fileList.ElementCount(); i++) {
      if (SdmMessageUtility::AppendMessageToFile(err, fileList[i], r_messageHeaders, r_messageBody)) {
        break;
      }
    }
  }
  return err;
}


SdmErrorCode
SdmOutgoingMessage::SendMessageThroughTransports(SdmError &err, 
  SdmVector<SdmTransportData*> &transportDataList, const SdmMsgFormat format)
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //
  TNF_PROBE_0 (SdmOutgoingMessage_SendMessageThroughTransports_start,
      "async SdmOutgoingMessage",
      "sunw%debug SdmOutgoingMessage::SendMessageThroughTransports Called ");

  err = Sdm_EC_Success;
  
  // Loop through transport data list and add a task to the
  // dispatcher for each transport.
  for (int i=0; i<transportDataList.ElementCount(); i++) {
    ProcessSubmitMessage(transportDataList[i]);
  }
  
  return err;
}

SdmErrorCode
SdmOutgoingMessage::GatherSendResults(SdmError &err, SdmDeliveryResponse &r_response, 
  SdmVector<SdmTransportData*> &transportDataList)
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //
  err = Sdm_EC_Success;
  SdmTransportData *data;
  for (int i=0; i< transportDataList.ElementCount(); i++) {
    // Fix!  need to handle multiple responses.
    assert (transportDataList[i]->deliveryResponse != NULL);
    r_response = *(transportDataList[i]->deliveryResponse);
    
    // save error if the send was not successful.
    assert (transportDataList[i]->error != NULL);
    if (*(transportDataList[i]->error)) {
      err = *(transportDataList[i]->error);
    }
  }
  return err;
}

SdmErrorCode
SdmOutgoingMessage::CleanUpTransportData(SdmError &err, SdmVector<SdmTransportData*> &transportDataList)
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //
  err = Sdm_EC_Success;
  SdmError localError;
  
  /// clean up items in trasport list.
  SdmTransportData *data;
  for (int i=0; i<transportDataList.ElementCount(); i++) {
    data = transportDataList[i];
    delete (data->error);
    delete (data->deliveryResponse);
    data->transport->Close(localError);
    if (localError) {
      err = localError;
      localError = Sdm_EC_Success;
    }    
    data->transport->ShutDown(localError);
    if (localError) {
      err = localError;
      localError = Sdm_EC_Success;
    }
    delete (data->transport);
    delete data;
  }
  transportDataList.ClearAllElements();
  return err;
}


void
SdmOutgoingMessage::ProcessSubmitMessage(void* callData)
{
  TNF_PROBE_0 (SdmOutgoingMessage_ProcessSubmitMessage_start,
      "async SdmOutgoingMessage",
      "sunw%debug SdmOutgoingMessage::ProcessSubmitMessage Called ");

  TNF_PROBE_1 (SdmOutgoingMessage_ProcessSubmitMessage_thread_info,
      "async SdmSession",
      "sunw%debug SdmOutgoingMessage::ProcessSubmitMessage: request processed on thread: ",
      tnf_long, thread, thr_self());

  SdmTransportData *data = (SdmTransportData *)callData;

  data->transport->Submit(*(data->error), *(data->deliveryResponse), data->format, 
      *(data->dataport), data->messageNumber, *data->customHeaders);

}

void 
SdmOutgoingMessage::GetEventForRequest(const SdmRequest* reqst, SdmEvent *&r_event,
    const SdmBoolean isCancelled)
{
  TNF_PROBE_0 (SdmOutgoingMessage_GetEventForRequest_start,
      "api SdmMessage",
      "sunw%start SdmOutgoingMessage_GetEventForRequest_start");

  switch(reqst->GetOperator()) {

    case AOP_Submit:
    // case AOP_PostSubmitRequest:
      {
        SdmArgumentList* args = reqst->GetArguments();
        int i = *((int *) args->GetArg(SdmArgumentList::ArgT_Int));
        SdmMsgFormat format = (SdmMsgFormat)i;
        i = *((int *) args->GetArg(SdmArgumentList::ArgT_Int));
        SdmBoolean logMessage = (SdmBoolean)i;

        SdmStringL *customHeaders =
          (SdmStringL *) args->GetArg(SdmArgumentList::ArgT_StringL);

        // create and setup event that is returned in callback.	
        r_event = new SdmEvent;
        r_event->sdmSubmitMessage.error = new SdmError;
        r_event->sdmSubmitMessage.type = Sdm_Event_SubmitMessage;
        r_event->sdmSubmitMessage.response = new SdmDeliveryResponse;
        r_event->sdmSubmitMessage.messageNumber = _MessgNumber;

        if (!isCancelled) {
          // major error if this is being called for AOP_PostSubmitRequest
          // and this is not a canceled request!!
          assert (reqst->GetOperator() == AOP_Submit);
          
          // do the actual submit of the message.
          int st = _Submit(*(r_event->sdmSubmitMessage.error), 
                           *(r_event->sdmSubmitMessage.response), format,
                           logMessage, *customHeaders);
        } else {
          r_event->sdmSubmitMessage.response = NULL;
          *(r_event->sdmSubmitMessage.error) = Sdm_EC_Cancel;
        }

        delete args;
        
        break;
      }
   default:
      {
        SdmMessage::GetEventForRequest(reqst, r_event, isCancelled);
        break;
      }
 }  

  TNF_PROBE_0 (SdmOutgoingMessage_GetEventForRequest_end,
      "api SdmMessage",
      "sunw%end SdmOutgoingMessage_GetEventForRequest_end");
  return;
}

