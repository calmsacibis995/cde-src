/*
 *+SNOTICE
 *
 *
 *	$Revision: 1.1 $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement bertween
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel wihtout
 *	Sun's specific written approval.  This documment and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)FileShare.C	1.20 11/30/95"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <DtMail/FileShare.hh>
#include <DtMail/DtMailXtProc.h>

XtAppContext	DtMailDamageContext = NULL;

static const int FileShareTimeout = 900000;
static int tlock_flag = 1;


static Tt_message msg_create(char *op, char *file, Tt_class tt_class, Tt_message_callback f)
{
Tt_status ttErrorStatus;
DtMailEnv error;

  error.clear();

  // Create the tooltalk message
  Tt_message msg = tt_message_create();
  if (tt_ptr_error(msg) != TT_OK) {
    error.logError(DTM_FALSE, "tt_message_create: Tt_status (%u) %s\n", 
		       (unsigned long)tt_pointer_error(msg),
		       tt_status_message(tt_pointer_error(msg)) );
    return ((Tt_message) NULL);
  }
  
  /* Set the message class type */
  if ((ttErrorStatus = tt_message_class_set (msg, tt_class)) != TT_OK) {
    error.logError(DTM_FALSE, "tt_message_class_set: Tt_status (%u) %s\n", 
		   ttErrorStatus, tt_status_message(ttErrorStatus));
    tt_message_destroy(msg);
    return ((Tt_message) NULL);
  }
  
  /* Set the message address */
  if ((ttErrorStatus = tt_message_address_set (msg, TT_PROCEDURE)) != TT_OK) {
    error.logError(DTM_FALSE, "tt_message_address_set: Tt_status (%u) %s\n", 
		   ttErrorStatus, tt_status_message(ttErrorStatus));
    tt_message_destroy(msg);
    return ((Tt_message) NULL);
  }
  
  /* Set the disposition of the message */
  if ((ttErrorStatus = tt_message_disposition_set (msg, TT_DISCARD)) != TT_OK) {
    error.logError(DTM_FALSE, "tt_message_disposition_set: Tt_status (%u) %s\n", 
		   ttErrorStatus, tt_status_message(ttErrorStatus));
    tt_message_destroy(msg);
    return ((Tt_message) NULL);
  }
  
  /* Set the message operation. */
  if ((ttErrorStatus = tt_message_op_set (msg, op)) != TT_OK) {
    error.logError(DTM_FALSE, "tt_message_op_set: Tt_status (%u) %s\n", 
		   ttErrorStatus, tt_status_message(ttErrorStatus));
    tt_message_destroy(msg);
    return ((Tt_message) NULL);
  }
  
  /* Set the message scope */
  if ((ttErrorStatus = tt_message_scope_set (msg, TT_FILE)) != TT_OK) {
    error.logError(DTM_FALSE, "tt_message_scope_set: Tt_status (%u) %s\n", 
		   ttErrorStatus, tt_status_message(ttErrorStatus));
    tt_message_destroy(msg);
    return ((Tt_message) NULL);
  }
  
  if ((ttErrorStatus = tt_message_file_set (msg, file)) != TT_OK) {  
    error.logError(DTM_FALSE, "tt_message_file_set: Tt_status (%u) %s\n", 
		   ttErrorStatus, tt_status_message(ttErrorStatus));
    tt_message_destroy(msg);
    return ((Tt_message) NULL);
  }

  if ((ttErrorStatus = tt_message_arg_add(msg, TT_IN, "DtMail", "lock")) != TT_OK) {
    error.logError(DTM_FALSE, "tt_message_arg_add: Tt_status (%u) %s\n", 
		   ttErrorStatus, tt_status_message(ttErrorStatus));
    tt_message_destroy(msg);
    return ((Tt_message) NULL);
  }

  if (f) {
    if ((ttErrorStatus = tt_message_callback_add(msg, f)) != TT_OK) {
    error.logError(DTM_FALSE, "tt_message_callback_add: Tt_status (%u) %s\n", 
		   ttErrorStatus, tt_status_message(ttErrorStatus));
      tt_message_destroy(msg);
      return ((Tt_message) NULL);
    }
  }

  return (msg);
}

Tt_callback_action
FileShare::mt_lock_cb(Tt_message m, Tt_pattern p)
{
  Tt_state state = tt_message_state(m);
  char *op;
  char *flag = NULL;

  op = tt_message_op(m);
  if (!op) {
    return TT_CALLBACK_CONTINUE;
  }

  flag = tt_message_arg_val(m, 0);

  if (!strcmp(op, "tlock")) {
    // handle tlock request
    if (state == TT_HANDLED) {
      tlock_flag = -1;
      tt_message_destroy(m);
    } else if (state == TT_FAILED) {
      tlock_flag = 0;
      tt_message_destroy(m);
    } else if (state == TT_SENT) {
      if (tt_ptr_error(flag) == TT_OK) {
	// this message is from another dtmail, ignore it
	return TT_CALLBACK_CONTINUE;
      }

      tt_message_reply(m);
    }
  } else if (!strcmp(op, "rulock")) {
    // handle rulock notice
    if (state == TT_SENT) {
      DtMailBoolean answer = DTM_FALSE;		// default is to not give up the lock
      FileShare *f = (FileShare *)tt_pattern_user(p, 1);
      if (f->_cb_func) {
	answer = f->_cb_func(DTMC_UNLOCK,
			     f->_path,
			     "Another user would like your lock.",
			     f->_cb_data);
      }
      tt_message_destroy(m);
    }
  }

  tt_free(op);
  return TT_CALLBACK_PROCESSED;
}

FileShare::FileShare(DtMailEnv & error,
		     DtMail::Session * session,
		     const char * path,
		     DtMailCallback cb,
		     void * clientData)
{
  DtMail::MailRc *mailrc = session->mailRc(error);

    _key = session->newObjectKey();
    _session = session;
    _path = strdup(path);
    _cb_func = cb;
    _cb_data = clientData;

    // For now, assume we can't write to the file.
    //
    _have_write_access = DTM_FALSE;
    _other_modified = DTM_TRUE;
    _mt_pattern = NULL;
    error.clear();

    // Register the file pattern.
    //
    _tt_handle = new TTHandle;
    _tt_handle->session = _session;
    _tt_handle->key = _key;
    _tt_handle->self = this;

    _file_pats = ttdt_file_join(_path, TT_FILE, 0, fileCB, _tt_handle);
    if (tt_pointer_error(_file_pats) != TT_OK) {
        error.logError(DTM_FALSE, "ttdt_file_join error: path '%s' Tt_status (%u) %s\n", 
		       _path, 
		       (unsigned long)tt_pointer_error(_file_pats),
		       tt_status_message(tt_pointer_error(_file_pats)) );
	error.setError(DTME_TTFailure);
	_file_pats = NULL;
	return;
    }

  // isModified(error);
}

FileShare::~FileShare(void)
{

    if (_have_write_access == DTM_TRUE && _file_pats) {
	_pending = PENDING_DESTROY;
	
	// ttdt_file_event(NULL, TTDT_SAVED, _file_pats, 1);

	_session->removeObjectKey(_key);
    }

    if (_file_pats) {
      ttdt_file_quit(_file_pats, 1);
    }

    if (_mt_pattern) {
      tt_pattern_destroy(_mt_pattern);
      _mt_pattern = NULL;
    }

    if (_path) {
	free(_path);
    }

    _have_write_access = DTM_FALSE;
    _file_pats = NULL;

    // we must delete _tt_handle after we call
    // ttdt_file_quit with _file_pats (above). 
    if (_tt_handle) {
	delete _tt_handle;
    }
}

DtMailBoolean
FileShare::isModified(DtMailEnv & error)
{
Tt_status ttErrorStatus;

    error.clear();

    if (!_path) {
	return(DTM_FALSE);
    }

    DtMailBoolean	answer = DTM_FALSE;

    Tt_message mt_msg;

    mt_msg = msg_create("tlock", _path, TT_REQUEST, mt_lock_cb);
    if (mt_msg == NULL) {
      error.setError(DTME_TTFailure);
      return DTM_TRUE;
    }

    if ((ttErrorStatus = tt_message_send(mt_msg)) != TT_OK) {
      error.logError(DTM_FALSE, "tt_message_send: Tt_status (%u) %s\n", 
		   ttErrorStatus, tt_status_message(ttErrorStatus));
      error.setError(DTME_TTFailure);
      return DTM_TRUE;
    }

    tttk_block_while((XtAppContext)0, &tlock_flag, FileShareTimeout);

    // mt_lock_cb sets tlock_flag to -1 if mbox is locked
    if (tlock_flag == -1) {
      tlock_flag = 1;	// reset the tlock_flag
      _other_modified = DTM_TRUE;
      _mt_lock = DTM_TRUE;

      return DTM_TRUE;
    } else {
      // else tlock_flag == 0, means no lock on this mbox
      // or tlock_flag == 1, means time out
      tlock_flag = 1;	// reset the tlock_flag
      _mt_lock = DTM_FALSE;

      // now let's try the dtmail protocol
      if (ttdt_Get_Modified(NULL, _path, TT_FILE, NULL, FileShareTimeout)) {
	answer = DTM_TRUE;
	_other_modified = DTM_TRUE;
      } else {
	answer = DTM_FALSE;
	_other_modified = DTM_FALSE;
      }

      return(answer);
    }
}

void
FileShare::lockFile(DtMailEnv & error)
{
    Tt_status ttErrorStatus;
    int always;

    error.clear();

    // If we have the access, then we locked it before. Simply return.
    //
    if (_have_write_access == DTM_TRUE) {
	return;
    }

    // First step in locking is determining if anyone else has the lock.
    // If they do, then we need to ask them to save their changes and
    // exit.
    //
    if (isModified(error) == DTM_TRUE) {
      DtMailBoolean take_lock = DTM_FALSE;	// default is to not request access

      // calls syncViewAndStoreCallback which then calls syncViewAndStore
      if (_cb_func) {
	take_lock = _cb_func(DTMC_QUERYLOCK,
			     _path,
			     "Another session has this mailbox locked. Request access?",
			     _cb_data);
      }

      if (take_lock == DTM_FALSE) {
	error.setError(DTME_OtherOwnsWrite);
	return;
      }

      // isModified sets _mt_lock to DTM_TRUE is the mailbox is locked
      // by mailtool
      if (_mt_lock == DTM_TRUE) {
	// mailtool style locking
	Tt_message mt_msg;

	mt_msg = msg_create("rulock", _path, TT_NOTICE, NULL);
	if (mt_msg == NULL) {
	  error.setError(DTME_TTFailure);
	  return;
	}

	if ((ttErrorStatus = tt_message_send(mt_msg)) != TT_OK) {
	  error.logError(DTM_FALSE, "tt_message_send: Tt_status (%u) %s\n", 
			 ttErrorStatus, tt_status_message(ttErrorStatus));
	  error.setError(DTME_TTFailure);
	  return;
	}
	tt_message_destroy(mt_msg);
      } else {
	// ttdt style locking
	ttdt_Save(NULL, _path, TT_FILE, DtMailDamageContext, FileShareTimeout);
      }

      // Give the other mailer FileShareTimeout seconds to give up the lock
      time_t t_start;

      time(&t_start);
      while (1) {
	sleep(5);
	if (isModified(error) == DTM_FALSE) {
	  break;
	} else {
	  if (time((time_t)NULL) - t_start > FileShareTimeout) {
	    // time out!
	    error.setError(DTME_OtherOwnsWrite);
	    return;
	  }
	}
      }
    }

    // Set this so we don't call our client during this handshake.
    //
    _pending = PENDING_LOCK;
    _outstanding = DTM_TRUE;

    // Now we are ready to lock the mailbox

    // register this pattern so we can handle messages from mailtool
    _mt_pattern = tt_pattern_create();
    tt_pattern_category_set(_mt_pattern, TT_HANDLE);
    tt_pattern_scope_add(_mt_pattern, TT_FILE);
    tt_pattern_file_add(_mt_pattern, _path);
    tt_pattern_op_add(_mt_pattern, "tlock");
    tt_pattern_op_add(_mt_pattern, "rulock");
    tt_pattern_callback_add(_mt_pattern, mt_lock_cb);
    tt_pattern_user_set(_mt_pattern, 1, (void *)this);

    if ((ttErrorStatus = tt_pattern_register(_mt_pattern)) != TT_OK) {
      error.logError(DTM_FALSE, "tt_pattern_register: Tt_status (%u) %s\n", 
		     ttErrorStatus, tt_status_message(ttErrorStatus));
      error.setError(DTME_TTFailure);
      return;
    }

    // Send the message saying we want to be the owner.
    if ((ttErrorStatus = ttdt_file_event(NULL, TTDT_MODIFIED, _file_pats, 1)) != TT_OK) {
      error.logError(DTM_FALSE, "ttdt_file_event: Tt_status (%u) %s\n", 
		     ttErrorStatus, tt_status_message(ttErrorStatus));
      error.setError(DTME_TTFailure);
      return;
    }

    // We need to process any messages that have arrived. We will get our own
    // modified message, which is not terribly interesting. What is interesting
    // is a modified message from someone else. That means that we have a race
    // condition where two processes both asked if the file was being modified,
    // and it wasn't. Then both said they were the owner, which is obviously
    // wrong so we need to blow both off and make them try again. Hopefully
    // there is enough randomness in our clients that the race condition will
    // clear itself up and we won't get here very often.
    //
    always = 1;
    while(_outstanding == DTM_TRUE) {
      tttk_block_while((XtAppContext)0, &always, 0);
    }

    if (_other_modified == DTM_TRUE) {
      // Well, we have a race. Fail this lock as will the other process,
      // we hope.
      error.setError(DTME_OtherOwnsWrite);
      return;
    }

    // Okay, we now have the lock.
    _have_write_access = DTM_TRUE;
}

DtMailBoolean
FileShare::readOnly(DtMailEnv & error)
{
  DtMailBoolean answer = DTM_TRUE;	// default is to accept read-only access

  if (_cb_func) {
    answer = _cb_func(DTMC_READONLY,
		      _path,
		      "Unable to obtain lock, open this mailbox as read only?",
		      _cb_data);
  }

  if (answer)
    error.clear();

  return(answer);
}

DtMailBoolean
FileShare::readWriteOverride(DtMailEnv & error)
{
  DtMailBoolean answer = DTM_FALSE;	// default is to open for read-only access

  if (_cb_func) {
    answer = _cb_func(DTMC_READWRITEOVERRIDE,
		      _path,
		      "Unable to obtain lock because system not responding, open this mailbox as read only, read write, or cancel?",
		      _cb_data);
  }

  if (answer == ((DtMailBoolean)((DTM_FALSE+DTM_TRUE)*2))) {
    error.setError(DTME_UserInterrupted);
    answer = DTM_FALSE;
  }
  else {
    error.clear();
  }

  return(answer);
}

#ifdef DEAD_WOOD
DtMailBoolean
FileShare::locked(void)
{
    return(_have_write_access);
}
#endif /* DEAD_WOOD */

Tt_message 
FileShare::fileCB(Tt_message msg,
		  Tttk_op op,
		  char * path,
		  void *clientData,
		  int,
		  int same_proc)
{
    TTHandle	*tt_handle = (TTHandle *)clientData;
    DtMailBoolean answer;

    if (tt_handle->session->validObjectKey(tt_handle->key) == DTM_FALSE) {
	// This object has been destroyed. We got here most likely because
	// ToolTalk is responding to one of our clean up messages. In any
	// case, fail the message and return.
	//
	tttk_message_fail(msg, TT_DESKTOP_ECANCELED, "Object destroyed", 1);
	return(0);
    }

    FileShare * self = tt_handle->self;

    switch(op) {
      case TTDT_MODIFIED:
	if (self->_outstanding == DTM_FALSE && !same_proc) {
	  if (self->_cb_func) {
	    self->_cb_func(DTMC_LOSTLOCK,
			   path,
			   "Another user has taken your lock.",
			   self->_cb_data);
	  }
	  self->_other_modified = DTM_TRUE;
	  self->_have_write_access = DTM_FALSE;
	  break;
	}
	
	if (self->_outstanding == DTM_TRUE && self->_pending == PENDING_LOCK) {
	    // This could be one of 2 conditions. If the message is
	    // from us, then we have the lock, and we are done.
	    // If not, then someone else is asking for the lock. We
	    // reflect this by giving them the lock.
	    //
	    if (same_proc) {
		self->_other_modified = DTM_FALSE;
		self->_have_write_access = DTM_TRUE;
		self->_outstanding = DTM_FALSE;
	    }
	    else {
		self->_other_modified = DTM_TRUE;
		self->_have_write_access = DTM_FALSE;
		// We haven't seen our own request yet. Leave outstanding
		// so we can process it before leaving.
	    }
	}
	break;

      case TTDT_GET_MODIFIED:
	tt_message_arg_ival_set(msg, 1, 1);
	tt_message_reply(msg);
	break;

      case TTDT_SAVED:
      case TTDT_REVERTED:
	// The other process has saved their changes (or tossed them).
	// At this point we should be able to start modifying the file.
	//
	self->_other_modified = DTM_FALSE;
	if (self->_outstanding == DTM_TRUE && self->_pending == PENDING_SAVE) {
	    self->_outstanding = DTM_FALSE;
	}
	break;

      case TTDT_REVERT:
      case TTDT_SAVE:
	// Someone is asking us to save our changes and close the file.
	//
	answer = DTM_FALSE;	// default is to not give up the lock

	if (self->_cb_func) {
	  answer = self->_cb_func(DTMC_UNLOCK,
				  path,
				  "Another user would like your lock.",
				  self->_cb_data);
	}

	if (answer == DTM_TRUE) {
	  tt_message_reply(msg);
	} else {
	  tttk_message_fail(msg, TT_DESKTOP_EACCES, 0, 0);
	}
	break;

      default:
	// Other messages, we simply smile and say thank you.:-)
	//
	tt_message_reply(msg);
	break;

    }

    tt_message_destroy(msg);
    return(0);
}

