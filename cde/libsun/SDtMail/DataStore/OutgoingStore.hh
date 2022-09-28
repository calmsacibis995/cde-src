/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Outgoing Store Class.

#ifndef _SDM_OUTGOINGSTORE_H
#define _SDM_OUTGOINGSTORE_H

#pragma ident "@(#)OutgoingStore.hh	1.53 97/04/15 SMI"

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/MessageStore.hh>
#include <SDtMail/Message.hh>

class SdmOutgoingSession;
class SdmToken;

class SdmOutgoingStore : public SdmMessageStore {
public:
  SdmOutgoingStore(SdmError& err, SdmOutgoingSession& outSession);
  virtual ~SdmOutgoingStore();

  virtual SdmErrorCode Open(SdmError& err, SdmMessageNumber &nmsgs,
                            SdmBoolean& r_readOnly, const SdmToken &token);

#ifdef INCLUDE_UNUSED_API
  virtual SdmErrorCode Open_Async(SdmError& err, 
                          const SdmServiceFunction& svf, void* clientData,
                          const SdmToken &token);                          
#endif
  
  // methods for disconnected mail.
  virtual SdmErrorCode Reconnect(SdmError &err);
  virtual SdmErrorCode Disconnect(SdmError &err);
  
  // get existing message from outgoing store.
  virtual SdmErrorCode SdmMessageFactory(SdmError& err, SdmMessage *&r_message, 
                          const SdmMessageNumber msgnum);
    
  // creates a outgoing.                          
  virtual SdmErrorCode SdmMessageFactory(SdmError& err, SdmMessage *&r_message);  
  virtual SdmErrorCode SdmMessageFactory(SdmError& err, SdmMessage *&r_message,
                          SdmMessage &deriveFrom);  

protected:
  virtual SdmErrorCode _Open(SdmError& err, SdmMessageNumber& r_nmsgs,
                             SdmBoolean& r_readOnly, const SdmToken &token);
          SdmErrorCode CheckOpenToken(SdmError& err, SdmToken &r_token, 
                          const SdmToken& tokenToCheck);
#ifdef INCLUDE_DISCONNECT
          SdmErrorCode SubmitOutstandingMessages(SdmError &err);       
#endif
          SdmBoolean   IsConnected() const;

  static  SdmString Sdm_XlibSDtMail_Header; 
  
  
private:
  // Disallow creating copies of SdmOutgoingStore.
  // SdmOutgoingStore(const SdmOutgoingStore& copy);
  SdmOutgoingStore& operator=(const SdmOutgoingStore &rhs);  
  SdmBoolean _IsConnected;
  
#ifdef INCLUDE_DISCONNECT
  SdmErrorCode GetMessagesToSend(SdmError& err, SdmVector<SdmOutgoingMessage *> &r_msgsToSend, 
                          SdmStringL &r_xlibsdtmail_hdrs, const long nummessages);

  SdmErrorCode SendMessages(SdmError& err, SdmBoolean &r_msgSubmitted, 
                          const SdmVector<SdmOutgoingMessage *> &msgsToSend, 
                          const SdmStringL &xlibsdtmail_hdr);
#endif

  friend class SdmOutgoingMessage;
  friend class SdmDisconnectTest;
};

#endif

