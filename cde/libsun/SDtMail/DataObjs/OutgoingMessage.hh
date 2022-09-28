/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Basic OutgoingMessage Class.

#ifndef _SDM_OUTGOINGMESSAGE_H
#define _SDM_OUTGOINGMESSAGE_H

#pragma ident "@(#)OutgoingMessage.hh	1.48 96/08/21 SMI"

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Message.hh>
#include <DataStore/OutgoingStore.hh>

class SdmOutgoingMessage : public SdmMessage {

public:
  SdmOutgoingMessage(SdmError& err, SdmOutgoingStore& dStore);
  SdmOutgoingMessage(SdmError& err, SdmOutgoingStore& dStore, 
            const SdmMessage &derivedFrom);
  SdmOutgoingMessage(SdmError& err, SdmMessageStore& dStore, 
            const SdmMessageNumber msgnum);
  virtual ~SdmOutgoingMessage();

  // Submit sends message.
  virtual SdmErrorCode Submit(SdmError& err, SdmDeliveryResponse& r_response, 
                          const SdmMsgFormat format, const SdmBoolean logMessage,
                          const SdmStringL& customHeaders);
  virtual SdmErrorCode Submit_Async(SdmError& err, const SdmServiceFunction& svf, 
                          void* clientData, const SdmMsgFormat format,
                          const SdmBoolean logMessage, const SdmStringL& customHeaders);

                                                
protected:
  virtual SdmErrorCode _Submit(SdmError& err, SdmDeliveryResponse& r_response, 
                          const SdmMsgFormat format, const SdmBoolean logMessage,
                          const SdmStringL& customHeaders);

  virtual void GetEventForRequest(const SdmRequest* reqst, SdmEvent *&r_event,
                          const SdmBoolean isCancelled);
  
  

private:
  // Disallow creating copies of SdmOutgoingMessage.
  // SdmOutgoingMessage(const SdmOutgoingMessage& copy);
  SdmOutgoingMessage& operator=(const SdmOutgoingMessage &rhs);

  typedef struct {
    SdmDataPort  *transport;
    SdmDeliveryResponse *deliveryResponse;
    SdmMessageNumber messageNumber;
    SdmDataPort  *dataport;
    SdmError     *error;
    SdmMsgFormat  format;
    const SdmStringL *customHeaders;
  } SdmTransportData;

  // methods used for submitting message.
  SdmErrorCode SendMessageThroughTransports(SdmError &err, 
                  SdmVector<SdmTransportData*> &transportDataList,
                  const SdmMsgFormat format);
  SdmErrorCode AppendMessageToFiles(SdmError &err, const SdmStringL &fileList,
                  const SdmMsgFormat format);
  SdmErrorCode GetAddressesForTransports(SdmError &err, 
                  SdmVector<SdmTransportData*> &r_transportDataList, 
                  SdmStringL &r_fileList, SdmMessage *message,
                  const SdmMsgFormat format, const SdmStringL &customHeaders);
  SdmErrorCode GatherSendResults(SdmError &err, SdmDeliveryResponse &r_deliveryResponse, 
                  SdmVector<SdmTransportData*> &transportDataList);
  SdmErrorCode CleanUpTransportData(SdmError &err, 
                  SdmVector<SdmTransportData*> &transportDataList);
  SdmErrorCode AddLogFile(SdmError& err, SdmStringL &fileList);

                  
  // static void SR_SubmitMessage(SdmPrim* objThis, void* callData);
  void ProcessSubmitMessage(void* callData);
  
  friend class SdmOutgoingStore;
    
};

#endif
