/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm Template Reply Queue Class.

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)ReplyQueue.cc	1.31 97/03/21 SMI"
#endif

// Include Files
#include <CtrlObjs/ReplyQueue.hh>
#include <DataStructs/MutexEntry.hh>

SdmReplyQueue::SdmReplyQueue()
{
   // Fix?? Should we change the following to return an error instead
   // or make an assertion?
   int rc;
   rc = mutex_init(&_QueueAccessLock, USYNC_THREAD, NULL);
   assert (rc == 0);
#if defined(USE_SOLARIS_THREADS)
   rc = sema_init(&_ReplyDeposited, 0, USYNC_THREAD, 0);
   assert (rc == 0);
#endif
}
 
SdmReplyQueue::~SdmReplyQueue()
{
  mutex_destroy(&_QueueAccessLock);
#if defined(USE_SOLARIS_THREADS)
  sema_destroy(&_ReplyDeposited);
#endif
}

sema_t*
SdmReplyQueue::GetPrimarySemaVar()
{
  return &_ReplyDeposited;
}

void
SdmReplyQueue::PostReply(const SdmReply* rep) 
{ 
  SdmReply *reply = (SdmReply*) rep;

  // we need to get lock before we update the queue.
  {
	  SdmMutexEntry entry(_QueueAccessLock);
      AddElementToList(reply);  // add to end of queue.
  }
  sema_post(&_ReplyDeposited);
} 

SdmReply*
SdmReplyQueue::GetNextLogicalReply()
{ 
  SdmReply* reply = NULL;

  // we need to get lock before we read from the queue. 
  // removes reply from front of queue.
  {
    SdmMutexEntry entry(_QueueAccessLock);
    if (ElementCount() > 0) {
        reply = (SdmReply *) operator[](0);        // remove from front of queue.
        RemoveElementFromList(0);
    }
  }
    
  return reply;
}

void 
SdmReplyQueue::ClearAllReplies(SdmError& error)
{
    SdmMutexEntry entry(_QueueAccessLock);
    ClearAndDestroyAllElements();
}
