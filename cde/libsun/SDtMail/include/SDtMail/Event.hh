/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */
 
// --> This header defines the events returned by the Session to the callbacks.
 
#ifndef _SDM_EVENT_H
#define _SDM_EVENT_H

#pragma ident "@(#)Event.hh	1.37 97/06/09 SMI"

#include <SDtMail/Sdtmail.hh>
#include <synch.h>


class SdmError;
class SdmString;
class SdmMessageStore;
class SdmToken;
class SdmDeliveryResponse;
class SdmMessage;
class SdmMessageEnvelope;
class SdmMessageBody;
class SdmContentBuffer;


/* Event names.  Used in "type" field in SdmEvent structures. 
   They start from 2 because 0 and 1 are reserved in the protocol for 
   errors and replies.
*/
typedef enum SdmEventType_T {
  Sdm_Event_RequestError,
  Sdm_Event_AutoSave,
  Sdm_Event_CheckNewMail,
  Sdm_Event_DataPortBusy,
  Sdm_Event_DataPortLockActivity,
  Sdm_Event_DebugLog,
  Sdm_Event_ErrorLog,
  Sdm_Event_NotifyLog,
  Sdm_Event_ExpungeDeletedMessages,
  Sdm_Event_GetLastInteractiveEventTime,
  Sdm_Event_GetMessageBodyContents,
  Sdm_Event_GetMessageContents,
  Sdm_Event_GetMessageEnvelopeContents,
  Sdm_Event_OpenMessageStore,
  Sdm_Event_CloseMessageStore,
  Sdm_Event_SetMessageBodyContents,
  Sdm_Event_SubmitMessage,
  Sdm_Event_SessionShutdown,                // internal use only.
  Sdm_Event_GetPartialContents              // remove later. used for testing.
} SdmEventType;

typedef struct {
  SdmEventType         type;
  SdmError            *error;              // callback should NOT delete error in event.
} SdmBaseEvent;

typedef struct {
  SdmEventType         type;
  SdmError            *error;              // callback should NOT delete error in event.
} SdmRequestErrorEvent;

typedef struct {
  SdmEventType         type;
  SdmError            *error;              // callback should NOT delete error in event.
  SdmContentBuffer    *buffer;   
  SdmMessageNumber     messageNumber;
  int                  bodyPartNumber;
  unsigned long        startOffset;       // remove later.  used for testing
  unsigned long        length;            // remove later.  used for testing
} SdmGetContentsEvent;

typedef struct {
  SdmEventType         type;
  SdmError            *error;              // callback should NOT delete error in event.
  SdmMessageNumber     messageNumber;
  int                  bodyPartNumber;
} SdmSetContentsEvent;

typedef struct {
  SdmEventType         type;
  SdmError            *error;              // callback should NOT delete error in event.
  SdmDeliveryResponse *response;   
  SdmMessageNumber     messageNumber;
} SdmSubmitMessageEvent;

typedef struct {
  SdmEventType         type;
  SdmError            *error;              // callback should NOT delete error in event.
  SdmMessageNumber     nmsgs;
  SdmBoolean           readOnly;	         // indicates if open succeeded in read only mode
} SdmOpenMessageStoreEvent;

typedef struct {
  SdmEventType         type;
  SdmError            *error;              // callback should NOT delete error in event.
} SdmCloseMessageStoreEvent;

typedef struct {
  SdmEventType 	       type;
  SdmError            *error;              // callback should NOT delete error in event.
  SdmMessageNumberL   *deletedMessages;   
} SdmExpungeMessageEvent;

typedef struct {
  SdmEventType 	       type;
  SdmError            *error;              // callback should NOT delete error in event.
  sema_t              *callerLock;   
} SdmSessionShutdown;

typedef struct {
  SdmEventType 	       type;
  SdmError            *error;              // callback should NOT delete error in event.
  SdmBoolean           isStart;
  SdmMessageNumber     numMessages;   
} SdmCheckNewMail;

typedef struct {
  SdmEventType 	       type;
  SdmError            *error;              // callback should NOT delete error in event.
  SdmBoolean           isStart;
} SdmAutoSave;

typedef struct {
  SdmEventType 	       type;
  SdmError            *error;              // callback should NOT delete error in event.
  SdmString*           errorMessage;
} SdmErrorLog;

typedef struct {
  SdmEventType 	       type;
  SdmError            *error;              // callback should NOT delete error in event.
  SdmString*           debugMessage;
} SdmDebugLog;

typedef struct {
  SdmEventType 	       type;
  SdmError            *error;              // callback should NOT delete error in event.
  SdmString*           notifyMessage;
} SdmNotifyLog;

typedef struct {
  SdmEventType 	       type;
  SdmError            *error;              // callback should NOT delete error in event.
} SdmDataPortBusy;

typedef struct {
  SdmEventType 	       type;
  SdmError            *error;              // callback should NOT delete error in event.
  enum lockActivityEvent_T {
    Sdm_DPLA_SessionLockGoingAway,
    Sdm_DPLA_SessionLockTakenAway,
    Sdm_DPLA_WaitingForSessionLock,
    Sdm_DPLA_NotWaitingForSessionLock,
    Sdm_DPLA_WaitingForUpdateLock,
    Sdm_DPLA_NotWaitingForUpdateLock,
    Sdm_DPLA_ServerConnectionBroken,
    Sdm_DPLA_MailboxConnectionReadOnly,
    Sdm_DPLA_MailboxChangedByOtherUser
  }	lockActivityEvent;
} SdmDataPortLockActivity;

typedef struct {                                
  SdmEventType 	       type;
  SdmError            *error;              // callback should NOT delete error in event.
  time_t              *lastInteractiveTime;// fill in by callback.  callback should NOT delete!!
} SdmGetLastInteractiveEventTime;

/*
 * this union is defined so libSdm can always use the same sized
 * event structure internally, to avoid memory fragmentation.
 */
typedef union _SessionEvent {
  SdmAutoSave                                 sdmAutoSave;
  SdmBaseEvent                                sdmBase;
  SdmCheckNewMail                             sdmCheckNewMail;
  SdmDataPortBusy                             sdmDataPortBusy;
  SdmDataPortLockActivity		      sdmDataPortLockActivity;
  SdmDebugLog                                 sdmDebugLog;
  SdmErrorLog                                 sdmErrorLog;
  SdmNotifyLog                                sdmNotifyLog;
  SdmExpungeMessageEvent                      sdmExpungeDeletedMessages;
  SdmGetContentsEvent                         sdmGetContents;
  SdmGetLastInteractiveEventTime              sdmGetLastInteractiveEventTime;
  SdmOpenMessageStoreEvent                    sdmOpenMessageStore;
  SdmCloseMessageStoreEvent                   sdmCloseMessageStore;
  SdmRequestErrorEvent                        sdmRequestError;
  SdmSessionShutdown                          sdmSessionShutdown;
  SdmSetContentsEvent                         sdmSetContents;
  SdmSubmitMessageEvent                       sdmSubmitMessage;
} SdmEvent;

#endif
