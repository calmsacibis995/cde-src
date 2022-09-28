/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Outgoing Store Class.

#pragma ident "@(#)OutgoingStore.cc	1.75 97/04/15 SMI"

// Include Files.
#include <iostream.h>
#include <assert.h>
#include <DataStore/OutgoingStore.hh>
#include <SDtMail/Token.hh>
#include <SDtMail/MailRc.hh>
#include <Manager/OSess.hh>
#include <CtrlObjs/RepSWER.hh>
#include <DataObjs/OutgoingMessage.hh>
#include <PortObjs/DataPort.hh>
#include <DataStructs/MutexEntry.hh>
#include <TranObjs/SyncRequestEntry.hh>
#include <DataStructs/RWLockEntry.hh>
#include <SDtMail/DeliveryResponse.hh>
#include <TranObjs/Reply.hh>

// we need to forward declare strtok_r because it is
// not declared when the _REENTRANT identifier is not defined
// on the intel and sparc 2.4 platforms.
extern "C" char *strtok_r(char *, const char *, char **);



SdmString SdmOutgoingStore::Sdm_XlibSDtMail_Header("X-libSDtMail");
  


SdmOutgoingStore::SdmOutgoingStore(SdmError& err, SdmOutgoingSession& outSession)
  :SdmMessageStore(err, outSession), _IsConnected(Sdm_False)
{
}

SdmOutgoingStore::~SdmOutgoingStore()
{
}

SdmOutgoingStore& 
SdmOutgoingStore::operator=(const SdmOutgoingStore &rhs)
{
  cout << "*** Error: SdmOutgoingStore assignment operator called\n";
  assert(Sdm_False);
  return *this;
}



SdmErrorCode
SdmOutgoingStore::Open(SdmError& err, SdmMessageNumber &r_nmsgs,
                       SdmBoolean& r_readOnly, const SdmToken &token)
{
  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmToken revisedToken;
  if (!CheckOpenToken(err, revisedToken, token)) {
    // use revised token in Open call.
    SdmMessageStore::Open(err, r_nmsgs, r_readOnly, revisedToken);
  }
  return err;
}

#ifdef INCLUDE_UNUSED_API

SdmErrorCode
SdmOutgoingStore::Open_Async(SdmError& err, const SdmServiceFunction& svf, void* clientData, 
                   const SdmToken &token)
{
  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmToken revisedToken;
  if (!CheckOpenToken(err, revisedToken, token)) {
    // use revised token in Open call.
    SdmMessageStore::Open_Async(err, svf, clientData, revisedToken);
  }
  return err;
}

#endif


SdmErrorCode 
SdmOutgoingStore::Disconnect(SdmError &err)
{
#ifdef INCLUDE_DISCONNECT
  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  // Get SyncRequestEntry lock to make sure that any pending requests
  // are finished before we disconnect.  This means that any ourstanding
  // messages being submitted will be complete and it is safe to
  // disconnect.  The _IsConnectedFlag should be set before we
  // release this lock to obtain concurrent access to it.
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    if (!GetDataPort()->CheckOpen(err)) {
      if (_IsConnected) {
        _IsConnected = Sdm_False;
      } else {
        err = Sdm_EC_AlreadyDisconnected;
      }
    }
  }  
#else
  err = Sdm_EC_Operation_Unavailable;
#endif
  return err;
}
  
SdmErrorCode 
SdmOutgoingStore::Reconnect(SdmError &err)
{
#ifdef INCLUDE_DISCONNECT
  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)

  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    if (!GetDataPort()->CheckOpen(err)) {
      if (_IsConnected) {
        err = Sdm_EC_AlreadyConnected;
      } else {
        _IsConnected = Sdm_True;
        // submit all messages in the outgoing store.
        SubmitOutstandingMessages(err);
      }
    }
  }  
#else 
  err = Sdm_EC_Operation_Unavailable;
#endif
  return err;
}
  
SdmErrorCode 
SdmOutgoingStore::SdmMessageFactory(SdmError& err, SdmMessage *&r_message, 
                          const SdmMessageNumber msgnum)
{
  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)

  SdmMessage *message = NULL;
  {
    SdmSyncRequestEntry entry(err, *this);
    if (!err) {
      // scope the following code so we don't hold on to the
      // _MessageListLock at a minimum.
      {
        SdmRWLockEntry listEntry(_MessageListLock, Sdm_False);
        if (!GetMessageAt(err, r_message, msgnum)) {
          // need to create outgoing message from store.
          message = new SdmOutgoingMessage(err, *this, msgnum);
          if (!err && !AddToMessageList(err, message)) {
              r_message = message;
          }
        }
      }

      // clean up message if error occurred.  we need to set the _HasSyncLock
      // flag to make sure the message destructor doesn't try to get the sync
      // lock again.
      if (err && message) {
        message->_HasSyncLock = Sdm_True;
        delete message;
      }
    }
  }
  
  return err;
}

SdmErrorCode
SdmOutgoingStore::SdmMessageFactory(SdmError& err, SdmMessage *&r_message)
{
  err = Sdm_EC_Success;	
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmMessage *message = NULL;
  {
    SdmSyncRequestEntry entry(err, *this);
    if (!err) {
      message = new SdmOutgoingMessage(err, *this);
      if (!err) {
        SdmRWLockEntry rwlock_entry(*(GetMessageListLock()), Sdm_False);  // get write lock!
        if (!AddToMessageList(err, message)) {
          r_message = message;
        }
      }
      
      // clean up message if error occurred.  we need to set the _HasSyncLock
      // flag to make sure the message destructor doesn't try to get the sync
      // lock again.
      if (err && message) {
        message->_HasSyncLock = Sdm_True;
        delete message;
      }
    }
  }
  
  return err;
}


SdmErrorCode 
SdmOutgoingStore::SdmMessageFactory(SdmError& err, SdmMessage *&r_message,
  SdmMessage &deriveFrom)
{
  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmMessage *message = NULL;
  {
    SdmSyncRequestEntry entry(err, *this);
    if (!err) {
      message = new SdmOutgoingMessage(err, *this, deriveFrom);
      if (!err) {
        SdmRWLockEntry rwlock_entry(*(GetMessageListLock()), Sdm_False);  // get write lock!
        if (!AddToMessageList(err, message))
          r_message = message;
      }
      
      // clean up message if error occurred.  we need to set the _HasSyncLock
      // flag to make sure the message destructor doesn't try to get the sync
      // lock again.
      if (err && message) {
        message->_HasSyncLock = Sdm_True;
        delete message;
      }
    }
  }

  return err;
}


SdmErrorCode 
SdmOutgoingStore::_Open(SdmError& err, SdmMessageNumber &r_nmsgs,
                        SdmBoolean& r_readOnly, const SdmToken &token)
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //
  
  err = Sdm_EC_Success;
  SdmToken revisedToken(token);
  SdmBoolean openDisconnected = Sdm_False;
  SdmString tokenParm;

  // check for serviceoption = disconnected.
  // if true, remove serviceoption from token.  the dataport
  // returns error if trying to open a local message 
  // store disconnected.
  if (revisedToken.CheckValue("serviceoption", "disconnected")) {
    openDisconnected = Sdm_True;
    revisedToken.ClearValue("serviceoption", "disconnected");
  }

  // call inherited _Open and set connected flag to true only 
  // if open is successful.
  if (!SdmMessageStore::_Open(err, r_nmsgs, r_readOnly, revisedToken)) {
    if (openDisconnected) {
      _IsConnected = Sdm_False;
    } else {
      _IsConnected = Sdm_True;
#ifdef INCLUDE_DISCONNECT
      SubmitOutstandingMessages(err);
#endif
    }
  }
  return err;
}

SdmErrorCode
SdmOutgoingStore::CheckOpenToken(SdmError& err, SdmToken& r_revisedToken, 
  const SdmToken& token)
{
  err = Sdm_EC_Success;
  SdmString tokenParm;
  
  r_revisedToken = token;

  // need to check that service type = LOCAL.  if service type not
  // set, then set it to LOCAL.
  if (token.FetchValue(tokenParm, "servicetype")) {
    if (tokenParm != "local") { 
      err = Sdm_EC_InvalidOutgoingStoreToken;
      return err;
    }
  } else {
    r_revisedToken.SetValue("servicetype", "local");
  }

  // need to check that service class = messagestore.  if not
  // set, then set it to messagestore.
  if (token.FetchValue(tokenParm, "serviceclass")) {
    if (tokenParm != "messagestore") {
      err = Sdm_EC_InvalidOutgoingStoreToken;
      return err;
    }
  } else {
    r_revisedToken.SetValue("serviceclass", "messagestore");
  }

  // need to check that message name is specified.  if not set,
  // return error.
  if (!token.FetchValue(tokenParm, "serviceoption", "messagestorename")) {
    err = Sdm_EC_InvalidOutgoingStoreToken;
    return err;
  }

  return err;
}

SdmBoolean
SdmOutgoingStore::IsConnected() const
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //
  return _IsConnected;
}

#ifdef INCLUDE_DISCONNECT

SdmErrorCode 
SdmOutgoingStore::SubmitOutstandingMessages(SdmError &err) 
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //

  err = Sdm_EC_Success;

  // go through all messages in the store and submit the ones are not
  // already deleted (eg. check deleted flag in message).
  // need to be careful about what methods to call because we
  // don't want to call a method that tries to get the 
  // SyncRequestEntry lock; this will cause us to hang.

  SdmMessageStoreStatus r_status;
  SdmMessageNumber r_nummsgs;
  
  // first, call CheckForNewMessages so the dataport can update the message count.
  if (_DataPort->CheckForNewMessages(err, r_nummsgs)) {
    return err;

  // next, get the total number of messages in the store.
  } else if (_DataPort->GetMessageStoreStatus(err, r_status, Sdm_MSS_Messages)) {
    return err;
    
  } else if (r_status.messages > 0) {
    SdmVector<SdmOutgoingMessage *> msgsToSend;
    SdmStringL xlibsdtmail_hdrs;
    SdmBoolean msgSubmit;
  
    if (!GetMessagesToSend(err, msgsToSend, xlibsdtmail_hdrs, r_status.messages) &&
       msgsToSend.ElementCount() > 0) 
    {
      // the following two vectors better have the same number of elements.
      assert (xlibsdtmail_hdrs.ElementCount() == msgsToSend.ElementCount());
      SendMessages(err, msgSubmit, msgsToSend, xlibsdtmail_hdrs);

      // only save the message store state if we've successfully sent at lease one message.
      if (msgSubmit) {
        SdmError localError;   
        if (_DataPort->SaveMessageStoreState(localError) && !err) {
          err = localError;
        }
      }

      // clean up local variables.
      msgsToSend.ClearAllElements();
      xlibsdtmail_hdrs.ClearAllElements();
    }
 }

  return err;
}


SdmErrorCode
SdmOutgoingStore::GetMessagesToSend(SdmError& err,
    SdmVector<SdmOutgoingMessage *> &r_msgsToSend, 
    SdmStringL &r_xlibsdtmail_hdrs, const long nummsgs)

{
  // this method gets the list of the messages that need to be sent.  we
  // put the messages in a separate list and send them later because we
  // don't want to hold on to the _MessageListLock while we are doing 
  // the sending.  we scope the code below so that we can release the
  // lock once we are done checking the message list for messages to send.
  
  err = Sdm_EC_Success;

  SdmMessageFlagAbstractFlags r_flags;
  SdmMessage *message;
  SdmOutgoingMessage *outmsg;
  SdmStrStrL r_hdr;

  // get lock on message list.  we could be adding new messages to the 
  // message list as we get them so we need to get the lock.
  SdmRWLockEntry listEntry(_MessageListLock, Sdm_False);

  // loop through messags; only submit messages that are not deleted.
  for (long i=1; i<=nummsgs; i++) {

    // get flag for message.  
    if (_DataPort->GetMessageFlags(err, r_flags, i)) {
      return err;

    } else if (!(r_flags & Sdm_MFA_Deleted)) {
      message = outmsg = NULL;

      // get the message with the given msgno.  create it if we
      // don't have it in our message list cache.  return if error occurs.
      if (GetMessageAt(err, message, i) == Sdm_False) {
        outmsg = new SdmOutgoingMessage(err, *this, i); 
        if (err || AddToMessageList(err, outmsg)) {
          delete outmsg;
          return err;
        }
      } else {
        outmsg = (SdmOutgoingMessage*)message;
      }

      // get the XlibSDtMail header for this message.
      // add the message to the list only if it's got the XlibSDtMail header.          
      r_hdr.ClearAllElements();
      _DataPort->GetMessageHeader(err, r_hdr, SdmOutgoingStore::Sdm_XlibSDtMail_Header, i);
      if (err == Sdm_EC_RequestedDataNotFound) {
        err = Sdm_EC_Success;
      } else if (err == Sdm_EC_Success) {
        r_msgsToSend.AddElementToList(outmsg);
        r_xlibsdtmail_hdrs.AddElementToList(r_hdr[0].GetSecondString());
      } else {
        return err;
      }
    }
  } /* end for */

  return err;
}


SdmErrorCode
SdmOutgoingStore::SendMessages(SdmError &err, SdmBoolean &r_msgSubmit,
    const SdmVector<SdmOutgoingMessage *> &msgToSend, 
    const SdmStringL &xlibsdtmail_hdrs)
{
  err = Sdm_EC_Success;
  
  SdmBoolean logMessage = SdmMailRc::GetMailRc()->IsValueDefined("record");
  SdmOutgoingMessage *msg;
  SdmBoolean r_disposition;

  // submit the messages. use mime format.  call _Submit directly 
  // because we don't want to try to get the sync request lock again.
  // note that we continue to try to submit all the messages even if there
  // is an error in submitting one of them.
  // Revisit:  Is mime format ok?  Is it ok to log based on "record"
  // variable in mailrc? 
  SdmError localError, localError2;  
  SdmServiceFunction *r_serviceFunc = NULL;
  SdmServiceFunctionType r_serviceFound;
  SdmDeliveryResponse *deliveryResponse = NULL;
  SdmMsgFormat format;

  FindServiceFunction(localError, r_serviceFunc, r_serviceFound,
                  Sdm_SVF_SubmitOutgoingCachedMessage);

  for (int i=0; i< msgToSend.ElementCount(); i++) {
    msg = msgToSend[i];
    
    localError = localError2 = Sdm_EC_Success;
    if (deliveryResponse == NULL) {
      deliveryResponse = new SdmDeliveryResponse;
    }

    // determine the message format from the value in the X-libSDtMail header.
    // Fix??  Who deletes this header??
    // if (xlibsdtmail_hdrs[i].CaseFind("format=sunV3", 1)) {
    if (strstr((const char*)xlibsdtmail_hdrs[i], "format=sunV3") != NULL) {
      format = Sdm_MSFMT_SunV3;
    } else {
      format = Sdm_MSFMT_Mime;    // default to mime.
    }

    // Determine the custom headers from the corresponding value in the X-libSDtMail header
    SdmStringL customHeaders;
    char *customStart = strstr((const char*)xlibsdtmail_hdrs[i], "custom=");

    if (customStart) {
      // strtok writes into the string, so need to make a copy
      char *p = customStart = strdup(customStart+strlen("custom="));
      char *lasts = NULL;
      while (p = strtok_r(p, ":", &lasts)) {
        customHeaders(-1) = p;
        printf("%s\n", p);
        p = NULL; // Subsequent calls to strtok require a NULL first argument
      }
      free(customStart);
    }

    // submit the message.  if successful, save the changes to the message.  in our case,
    // we want to save the setting of the delete flag on the message which happened
    // in the SdmOutgoingMessage::_Sumit call.
    if (!msg->_Submit(localError, *deliveryResponse, format, logMessage, customHeaders)) {
      r_msgSubmit = Sdm_True;
      _DataPort->CommitPendingMessageChanges(localError2, r_disposition, msg->_MessgNumber, Sdm_False);
    } 

    // if there is a service function registered, then we post a reply with the 
    // results of the submit.
    if (r_serviceFunc) {
      SdmEvent *event = new SdmEvent;
      event->sdmSubmitMessage.error = new SdmError(localError);
      event->sdmSubmitMessage.type = Sdm_Event_SubmitMessage;
      event->sdmSubmitMessage.messageNumber = msg->_MessgNumber;
      event->sdmSubmitMessage.response = deliveryResponse;
      deliveryResponse = NULL;    // must do this otherwise it'll get deleted below.
      SdmReply* reply = new SdmReply;  
      reply->SetOperator(SdmOutgoingMessage::AOP_Submit);
      reply->SetClientObject(this);
      reply->SetEvent(event);
      reply->SetServiceFunction(new SdmServiceFunction(*r_serviceFunc));
      reply->SetServiceFunctionType(r_serviceFound);
      OutCallDispatchPoint(reply);
    }
  }

  if (deliveryResponse) {
    delete deliveryResponse;
  }
  return err;
}

#endif
