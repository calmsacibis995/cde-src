/*
 *+SNOTICE
 *
 *
 *	$Revision:$
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
#pragma ident "@(#)DtMailMailBox.C	1.16 06/02/94"
#endif

#include <string.h>
#include <stdlib.h>
#include <DtMail/DtMail.hh>
#include <DtMail/Threads.hh>
#include <DtMail/IO.hh>

DtMail::MailBox::MailBox(DtMailEnv & error,
			 Session * session,
			 DtMailObjectSpace space,
			 void * arg,
			 DtMailCallback cb,
			 void * clientData) 
{
    _key = session->newObjectKey();
    _session = session;
    error.clear();
    _space = space;
    _arg = arg;
    _callback = cb;
    _cb_data = clientData;
    _obj_mutex = MutexInit();
}

DtMail::MailBox::~MailBox(void)
{
    if (_obj_mutex) {
	MutexDestroy(_obj_mutex);
	_obj_mutex = NULL;

	_session->removeObjectKey(_key);
    }
}

DtMail::Session *
DtMail::MailBox::session(void)
{
    return(_session);
}

DtMailBoolean
DtMail::MailBox::mailBoxWritable(DtMailEnv & error)
{
    error.clear();

    return(_mail_box_writable);
}
