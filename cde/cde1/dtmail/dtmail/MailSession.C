/*
 *+SNOTICE
 *
 *	$Revision: 1.1 $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement between
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel without
 *	Sun's specific written approval.  This document and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)MailSession.C	1.32 01/25/96"
#endif

#include <unistd.h>
#include <sys/stat.h>

#include <X11/Intrinsic.h>
#include <DtMail/DtMailXtProc.h>

#include "MailSession.hh"
#include <DtMail/DtMailError.hh>
#include "MemUtils.hh"
#include "RoamMenuWindow.h"

DtMail::Session * MailSession::_app_session = NULL;

MailSession::MailSession(DtMailEnv & error, XtAppContext context)
: _open_mailboxes(128)
{
    error.clear();

    if (_app_session) {
	error.setError(DTME_ObjectInUse);
	return;
    }

    _app_session = new DtMail::Session(error, "dtmail");
    if (error.isSet()) {
	if ((DTMailError_t)error == DTME_NoMsgCat) {
	    // Benign error
	  error.clear();
	}
	// Else need to translate the error into an DTME_* error
	// defined in DtMailError.hh
    }

    XtAppAddInput(context,
		  _app_session->eventFileDesc(error),
		  (XtPointer)(XtInputReadMask),
		  DtMailXtInputProc,
		  _app_session);

    DtMailDamageContext = context;
}

MailSession::~MailSession(void)
{
    for (int mb = 0; mb < _open_mailboxes.length(); mb++) {
	delete _open_mailboxes[mb]->handle;
	free (_open_mailboxes[mb]->path);
    }

    delete _app_session;
}

Boolean
MailSession::isMboxOpen( const char *path )
{
    int pos = locate(path);

    if (pos >= 0)
	return (TRUE);
    else
	return (FALSE);
}

RoamMenuWindow *
MailSession::getRMW( DtMail::MailBox *mbox )
{
    int slot = locate(mbox);

    return(_open_mailboxes[slot]->rmw);
}

DtMail::MailBox *
MailSession::open(
    DtMailEnv & error, 
    const char * path,
    DtMailCallback cb_func, 
    void * client_data,
    DtMailBoolean auto_create, 
    DtMailBoolean request_lock,
    DtMailBoolean auto_parse
)
{
    // First, see if we have this open in this process.
    int slot = locate(path);
    if (slot >= 0) {
	_open_mailboxes[slot]->open_ref_count += 1;
	return(_open_mailboxes[slot]->handle);
    }

    // Create a handle for determining what to do next. This will
    // add us to the file session so we will start getting call
    // back requests on this file.
    DtMail::MailBox * mailbox = 
	_app_session->mailBoxConstruct(error, DtMailFileObject,
				       (void *)path, cb_func, client_data);
    if (error.isSet()) {
	error.setError(DTME_ObjectCreationFailed);
	delete mailbox;
	return(NULL);
    }
    
    // Does this file exist? If it doesn't and create is
    // on then make one. If create is off, then raise an
    // error.
    
    mailbox->open(error, 
	      auto_create, 
	      DTMAIL_DEFAULT_OPEN_MODE, 
	      DTMAIL_DEFAULT_CREATE_MODE,
	      request_lock,
	      auto_parse);
 
    // Need to translate from BE error to a FE error that the
    // user can understand.  Opening a mail container can result
    // in a variety of errors.  Translate those that are considered
    // to be relevant (for now) - the set can always be extended.
    // Toss others into an UnknownFormat error.
    
    if (error.isSet()) {
	delete mailbox;
	return(NULL);
    }

    addToList(mailbox, path, (RoamMenuWindow *) client_data);

    return(mailbox);
}

void
MailSession::close(DtMailEnv & error, const DtMail::MailBox * mb)
{
    // Find the mail box slot.
    int slot = locate(mb);
    if (slot < 0) {
	error.setError(DTME_ObjectInvalid);
	return;
    }

    MailBoxItem * mbi = _open_mailboxes[slot];
    mbi->open_ref_count -= 1;
    if (mbi->open_ref_count <= 0) {
	delete mbi->handle;
	free (mbi->path);

	_open_mailboxes.remove(slot);
	delete mbi;
    }
}

void 
MailSession::convert(
    DtMailEnv & error, 
    const char *,		// old_path
    const char *,		// new_path
    DtMailStatusCallback,	// cb_func
    void *			// client_data
)
{
  error.clear();

  return;
}

void
MailSession::copy(
    DtMailEnv & error, 
    DtMail::Message &,		// msg
    const char *		// path
)
{
  error.clear();
}

int
MailSession::locate(const char * path)
{
    for (int slot = 0; slot < _open_mailboxes.length(); slot++) {
	if (strcmp(_open_mailboxes[slot]->path, path) == 0) {
	    return(slot);
	}
    }

    return(-1);
}

int
MailSession::locate(const DtMail::MailBox * mb)
{
    for (int slot = 0; slot < _open_mailboxes.length(); slot++) {
	if (mb == _open_mailboxes[slot]->handle) {
	    return(slot);
	}
    }

    return(-1);
}

void
MailSession::addToList(
    DtMail::MailBox * mb, 
    const char * path,
    RoamMenuWindow *obj
)
{
    MailBoxItem * mbi = new MailBoxItem;

    mbi->handle = mb;
    mbi->path = strdup(path);
    mbi->open_ref_count = 1;
    mbi->rmw = obj;
    
    _open_mailboxes.append(mbi);
}

Tt_status 
MailSession::lockCB(
    Tt_message, 
    Tttk_op, 
    const char *,
    uid_t, 
    int, 
    void *
)
{
    // We never give up the lock during a copy!
    return(TT_ERR_INVALID);
}
