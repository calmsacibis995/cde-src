/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm Template Reply Queue Class.

#ifndef _SDM_REPLYQUEUE_H
#define _SDM_REPLYQUEUE_H

#pragma ident "@(#)ReplyQueue.hh	1.31 97/03/21 SMI"

// Include Files
#include <SDtMail/Sdtmail.hh>
#include <TranObjs/Reply.hh>
#include <CtrlObjs/ThreadDefs.hh>
#include <SDtMail/DoubleLinkedList.hh>

class SdmReplyQueue : public SdmDoubleLinkedList {

public:
  SdmReplyQueue();
  virtual ~SdmReplyQueue();
  SdmBoolean operator== (const SdmReplyQueue& queue)
  { return this == &queue ? Sdm_True : Sdm_False; }

public:
  sema_t* GetPrimarySemaVar();
  void PostReply(const SdmReply* reply);
  SdmReply* GetNextLogicalReply();
  void ClearAllReplies(SdmError& error);


private:
  sema_t _ReplyDeposited;
  mutex_t _QueueAccessLock; 
};

#endif
