/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Message Store Class.

#ifndef _Sdm_MESSGSTORE_H
#define _Sdm_MESSGSTORE_H

#pragma ident "@(#)MessageStore.hh	1.98 97/05/13 SMI"

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Server.hh>
#include <SDtMail/Message.hh>
#include <SDtMail/ServiceFunction.hh>
#include <SDtMail/Token.hh>
#include <SDtMail/Search.hh>
#include <SDtMail/PortObjects.hh>
#include <synch.h>

class SdmToken;
class SdmSearch;
class SdmDataPort;

// Message Store Status flags
// These flags are used in the GetStatus method of the SdmMessageStore object
//
typedef long SdmMessageStoreStatusFlag;		// a single message store status flag
typedef long SdmMessageStoreStatusFlags;	// a message store status flag mask

static const SdmMessageStoreStatusFlag Sdm_MSS_Messages = 1;    	// retrieve[d] number of messages
static const SdmMessageStoreStatusFlag Sdm_MSS_Recent = 2;      	// retrieve[d] number of recent messages
static const SdmMessageStoreStatusFlag Sdm_MSS_Unseen = 4;      	// retrieve[d] number of unseen messages
static const SdmMessageStoreStatusFlag Sdm_MSS_Uidnext = 8;     	// retrieve[d] next UID to be assigned
static const SdmMessageStoreStatusFlag Sdm_MSS_Uidvalidity = 16;	// retrieve[d] UID validity value
static const SdmMessageStoreStatusFlag Sdm_MSS_Checksum = 32;   	// retrieve[d] Checksum value
static const SdmMessageStoreStatusFlags Sdm_MSS_ALL = 
  (Sdm_MSS_Messages|Sdm_MSS_Recent|Sdm_MSS_Unseen|Sdm_MSS_Uidnext|Sdm_MSS_Uidvalidity|Sdm_MSS_Checksum);

// SdmMessageStoreStatus:
// status that may be retrieved on a per-message store basis
//

class SdmMessageStoreStatus {

public:

  SdmMessageStoreStatus() {};
  virtual ~SdmMessageStoreStatus() { };

  SdmMessageStoreStatusFlags flags;	// flags indicating values below contain valid information.
  long messages;		// number of messages 
  long recent;			// number of recent messages 
  long unseen;			// number of unseen messages 
  long uidnext;			// next UID to be assigned 
  long uidvalidity;		// UID validity value 
  unsigned short checksum;      // checksum value 
  unsigned long checksum_bytes; // n bytes checksummed 
};



class SdmMessageStore : public SdmServer {
public:
  virtual ~SdmMessageStore();
  
  virtual SdmErrorCode StartUp(SdmError& err);
  virtual SdmErrorCode ShutDown(SdmError& err);

          SdmErrorCode Parent(SdmError &err, SdmSession *&r_session);
    
  virtual SdmErrorCode Open(SdmError& err, SdmMessageNumber& nmsgs,
                            SdmBoolean& r_readOnly, const SdmToken &token);
  virtual SdmErrorCode Close(SdmError &err);
  virtual SdmErrorCode Create(SdmError &err, const SdmString &name);
  virtual SdmErrorCode Delete(SdmError &err, const SdmString &name);
  virtual SdmErrorCode Rename(SdmError &err, const SdmString &currentname, const SdmString &newname);

#ifdef INCLUDE_UNUSED_API
  virtual SdmErrorCode Open_Async(SdmError& err, const SdmServiceFunction& svf, void* clientData,
                          const SdmToken &token);                         
  virtual SdmErrorCode Close_Async(SdmError& err, const SdmServiceFunction& svf, void* clientData);
#endif
  
  // methods for disconnected mail support.
  virtual SdmErrorCode Attach(SdmError &err, const SdmToken &token);
  virtual SdmErrorCode Disconnect(SdmError &err);
  virtual SdmErrorCode Reconnect(SdmError &err);

  // returns existing message from message store.   
  virtual SdmErrorCode SdmMessageFactory(SdmError& err, SdmMessage *&r_message, 
                          const SdmMessageNumber msgnum);

  // creates new message.                          
  virtual SdmErrorCode SdmMessageFactory(SdmError& err, SdmMessage *&r_message);  
  virtual SdmErrorCode SdmMessageFactory(SdmError& err, SdmMessage *&r_message,
                          SdmMessage &deriveFrom);  
  
          SdmErrorCode CheckForNewMessages(SdmError& err, SdmMessageNumber &r_messages);

          SdmErrorCode GetStatus(SdmError& err, SdmMessageStoreStatus &r_status, 
                          const SdmMessageStoreStatusFlags storeflags);

          SdmErrorCode SaveMessageStoreState(SdmError& err);
          
  virtual SdmErrorCode CancelPendingOperations(SdmError& err);

          SdmErrorCode ExpungeDeletedMessages(SdmError &err, SdmMessageNumberL &r_msgnums);

#ifdef INCLUDE_UNUSED_API
          SdmErrorCode ExpungeDeletedMessages_Async(SdmError &err, const SdmServiceFunction& svf, 
                        void* clientData);
#endif

  // Service Function Registration
  SdmErrorCode RegisterServiceFunction(SdmError& err, const SdmServiceFunction &svf);
  SdmErrorCode UnregisterServiceFunction(SdmError& err, SdmServiceFunctionType sft);
  SdmErrorCode GetServiceFunction(SdmError& err, SdmServiceFunction *&r_serviceFunc, 
                 SdmServiceFunctionType sft);

#ifdef INCLUDE_UNUSED_API
  SdmErrorCode UnregisterServiceFunction(SdmError& err, const SdmServiceFunction &svf);
#endif

  //
  // Message related methods.
  //     
          SdmErrorCode GetFlags(SdmError &err, SdmMessageFlagAbstractFlags &r_flags, 
                                  const SdmMessageNumber msgnum);
          SdmErrorCode GetFlags(SdmError &err, SdmMessageFlagAbstractFlagsL &r_flags,  
                                  const SdmMessageNumber startmsgnum, 
                                  const SdmMessageNumber endmsgnum);
          SdmErrorCode GetFlags(SdmError &err, SdmMessageFlagAbstractFlagsL &r_flags, 
                                  const SdmMessageNumberL &msgnum);
                          
          SdmErrorCode ClearFlags(SdmError &err, const SdmMessageFlagAbstractFlags flag, 
                                  const SdmMessageNumber msgnum);

          SdmErrorCode ClearFlags(SdmError &err, const SdmMessageFlagAbstractFlags flag, 
                                  const SdmMessageNumberL &msgnums);

#ifdef INCLUDE_UNUSED_API
          SdmErrorCode ClearFlags(SdmError &err, const SdmMessageFlagAbstractFlags flag, 
                                  const SdmMessageNumber startmsgnum, 
                                  const SdmMessageNumber endmsgnum);
          SdmErrorCode SetFlags(SdmError &err, const SdmMessageFlagAbstractFlags aflags, 
                                  const SdmMessageNumber startmsgnum, 
                                  const SdmMessageNumber endmsgnum);
#endif

          SdmErrorCode SetFlags(SdmError &err, const SdmMessageFlagAbstractFlags aflags, 
                                  const SdmMessageNumber msgnum);
          SdmErrorCode SetFlags(SdmError &err, const SdmMessageFlagAbstractFlags aflags, 
                                  SdmMessageNumberL &msgnums);
                                  
          SdmErrorCode GetHeader(SdmError &err, SdmStrStrL &r_hdr,
                                  const SdmString &hdr,		
                                  const SdmMessageNumber msgnum);

          SdmErrorCode GetHeaders(SdmError &err, SdmIntStrL &r_hdr, 
                                  const SdmMessageHeaderAbstractFlags header, 
                                  const SdmMessageNumber msgnum);

          SdmErrorCode GetHeaders(SdmError &err, SdmIntStrLL &r_copiesof_hdr, 
                                  const SdmMessageHeaderAbstractFlags header, 
                                  const SdmMessageNumber startmsgnum, 
                                  const SdmMessageNumber endmsgnum);


          SdmErrorCode GetHeaders(SdmError &err, SdmIntStrLL &r_copiesof_hdr,  
                                  const SdmMessageHeaderAbstractFlags header, 
                                  const SdmMessageNumberL &msgnums);


#ifdef INCLUDE_UNUSED_API
          SdmErrorCode GetHeader(SdmError &err,SdmStrStrLL &r_copiesof_hdr,			
                                  const SdmString &hdr,			
                                  const SdmMessageNumber startmsgnum,	
                                  const SdmMessageNumber endmsgnum);

          SdmErrorCode GetHeader(SdmError &err, SdmStrStrLL &r_copiesof_hdr,	
                                  const SdmString &hdr,		
                                  const SdmMessageNumberL &msgnums);

          SdmErrorCode GetHeaders(SdmError &err, SdmStrStrLL &r_copiesof_hdr,				
                                  const SdmMessageHeaderAbstractFlags hdr,
                                  const SdmMessageNumberL &msgnums);

          SdmErrorCode GetHeaders(SdmError &err, SdmStrStrLL &r_copiesof_hdr,
                                  const SdmMessageHeaderAbstractFlags hdr,	
                                  const SdmMessageNumber startmsgnum,		
                                  const SdmMessageNumber endmsgnum);

          SdmErrorCode GetHeaders(SdmError &err, SdmStrStrL &r_hdr,
                                  const SdmMessageHeaderAbstractFlags hdr,	
                                  const SdmMessageNumber msgnum);

          SdmErrorCode GetHeaders(SdmError &err,SdmStrStrL &r_hdr,			
                                  const SdmMessageNumber msgnum);

          SdmErrorCode GetHeaders(SdmError &err, SdmStrStrLL &r_hdr,		
                                  const SdmMessageNumber startmsgnum,	
                                  const SdmMessageNumber endmsgnum);

          SdmErrorCode GetHeaders(SdmError &err,SdmStrStrLL &r_copiesof_hdr,			
                                  const SdmMessageNumberL &msgnums);

          SdmErrorCode GetHeaders(SdmError &err, SdmStrStrL &r_hdrs,			
                                  const SdmStringL &hdrs,		
                                  const SdmMessageNumber msgnum);

          SdmErrorCode GetHeaders(SdmError &err, SdmStrStrLL &r_copiesof_hdrs,		
                                  const SdmStringL &hdrs,		
                                  const SdmMessageNumber startmsgnum,	
                                  const SdmMessageNumber endmsgnum);

          SdmErrorCode GetHeaders(SdmError &err, SdmStrStrLL &r_copiesof_hdrs,			
                                  const SdmStringL &hdrs,		
                                  const SdmMessageNumberL &msgnums);

#endif

          SdmErrorCode CopyMessages(SdmError& err, const SdmToken& token,	
                                  const SdmMessageNumberL& msgnums);

#ifdef INCLUDE_UNUSED_API
          SdmErrorCode CopyMessage(SdmError& err, const SdmToken& token,
                                  const SdmMessageNumber msgnum);
 
          SdmErrorCode CopyMessages(SdmError& err, const SdmToken& token,	
                                  const SdmMessageNumber startmsgnum,
                                  const SdmMessageNumber endmsgnum);
          
          SdmErrorCode MoveMessage(SdmError& err, const SdmToken& token,
                                  const SdmMessageNumber msgnum);
 
          SdmErrorCode MoveMessages(SdmError& err, const SdmToken& token,	
                                  const SdmMessageNumber startmsgnum,
                                  const SdmMessageNumber endmsgnum);
          
          SdmErrorCode MoveMessages(SdmError& err, const SdmToken& token,	
                                  const SdmMessageNumberL& msgnums);                                  
#endif

          SdmErrorCode AppendMessage(SdmError& err, const SdmMessage& message);

#ifdef INCLUDE_UNUSED_API
          SdmErrorCode AppendMessages(SdmError& err, const SdmMessageL& messages);
#endif

  // Name space related methods
          SdmErrorCode ScanNamespace(SdmError& err, SdmIntStrL& r_nameL, const SdmString& ref,
                         const SdmString& pat);

#ifdef INCLUDE_UNUSED_API
          SdmErrorCode ScanSubscribedNamespace(SdmError& err, SdmIntStrL& r_nameL, 
                         const SdmString& ref, const SdmString& pat);
          SdmErrorCode AddToSubscribedNamespace(SdmError& err, const SdmString& name);
          SdmErrorCode RemoveFromSubscribedNamespace(SdmError& err, const SdmString& name);
#endif

  // Search related methods
          SdmErrorCode PerformSearch(SdmError &err, SdmMessageNumberL &r_msgnums, 
                                  const SdmSearch &srch);	// Search all 
          SdmErrorCode PerformSearch(SdmError &err, SdmMessageNumberL &r_msgnums, 
                                  const SdmSearch &srch, 
                                  const SdmMessageNumber startmsgnum, 
                                  const SdmMessageNumber endmsgnum);	// Search range
          SdmErrorCode PerformSearch(SdmError &err, SdmMessageNumberL &r_msgnums, 
                                  const SdmSearch &srch, 
                                  const SdmMessageNumberL &msgnums);	// Search list

          SdmErrorCode PerformSearch(SdmError &err, SdmBoolean &r_bool, 
                                  const SdmSearch &srch, 
                                  const SdmMessageNumber msgnum);	// Serach one 
                                  
          SdmBoolean   IsReadOnly() const { return _ReadOnly; }

protected:
  // Disallow construction of message store.  Only session classes may construct
  // message stores.
  SdmMessageStore(SdmError& err, SdmSession& inSession);

  // Internal use by back end.  
          SdmDataPort* GetDataPort();
          rwlock_t*    GetMessageListLock() { return &_MessageListLock; }

  virtual SdmErrorCode _Open(SdmError& err, SdmMessageNumber& r_nmsgs,
                             SdmBoolean& r_readOnly, const SdmToken &token);
          SdmErrorCode _Close(SdmError& err);
          SdmErrorCode _ExpungeDeletedMessages(SdmError &err, SdmMessageNumberL &r_msgnums);  

          SdmErrorCode _AppendMessage(SdmError& err, SdmString& contents, SdmString& date,
                          SdmString& from, SdmMessageFlagAbstractFlags flags);

          SdmErrorCode AddToMessageList(SdmError &err, SdmMessage* message);
          SdmErrorCode RemoveFromMessageList(SdmError &err, SdmMessage* message);
          SdmBoolean   GetMessageAt(SdmError &err, SdmMessage *&r_message, 
                          const SdmMessageNumber msgnum);

  virtual void         GetEventForRequest(const SdmRequest* reqst, 
                        SdmEvent *&r_event, const SdmBoolean isCancelled);   

public:
  enum OperationList {AOP_Open=300, AOP_Close, AOP_AppendMessage, AOP_AppendMessages, 
                      AOP_ExpungeDeletedMessages, AOP_CheckNewMail, AOP_AutoSave,
                      AOP_DataPortEvent };

private:

  // Disallow creating copies of SdmMessageStore.
  SdmMessageStore(const SdmMessageStore& copy);
  SdmMessageStore& operator=(const SdmMessageStore &rhs);
  
  SdmErrorCode ShutDownLocalServer(SdmError &err);
  
  static  void SR_ProcessDataPortEvent(void* objThis, SdmDpCallbackType type,
                        SdmDpEvent* response);
          void ProcessDataPortEvent(SdmDpCallbackType type, SdmDpEvent* event);

  SdmErrorCode HandleUnsolicitMailCheck(SdmError &err);
  SdmErrorCode HandleUnsolicitAutoSave(SdmError &err);
  SdmErrorCode HandleUnsolicitPing(SdmError &err);
         
  SdmErrorCode FindServiceFunctionForType(SdmError& err, SdmServiceFunction *&r_serviceFunc, 
                SdmServiceFunctionType sft);
  SdmErrorCode FindServiceFunction(SdmError& err, SdmServiceFunction *&r_serviceFunc, 
                SdmServiceFunctionType &r_serviceFound,
                SdmServiceFunctionType serviceToFind);

  SdmSession*                         _ParentSession;
  SdmVector<SdmMessage *>             _MessgList;
  SdmVector<SdmMessage *>             _NewMessgList;
  SdmDataPort*                        _DataPort;	
  SdmPtrVector<SdmServiceFunction*>   _ServiceFunctionList;
  rwlock_t                            _MessageListLock;
  mutex_t                             _ServiceListLock;   
  mutex_t                             _LocalServerDataLock;   
  mutex_t                             _MessageLogLock;
  SdmBoolean                          _ReadOnly;

  // the following classes are friends because they need access to GetDataPort.
  friend class SdmMessage;
  friend class SdmOutgoingMessage;
  friend class SdmMessageBody;
  friend class SdmMessageEnvelope;
  friend class SdmSession;
  friend class SdmIncomingSession;
  friend class SdmOutgoingStore;  

  friend class SdmMessageStoreTest;     // for testing.
};

#endif


