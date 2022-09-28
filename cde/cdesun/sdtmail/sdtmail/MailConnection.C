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
#pragma ident "@(#)MailConnection.C	1.7 05/07/96"
#endif

#include <unistd.h>
#include <sys/stat.h>

#include <X11/Intrinsic.h>
#include <DtMail/DtMailXtProc.h>

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Error.hh>
#include "MailConnection.hh"
#include "MemUtils.hh"

SdmConnection	   * MailConnection::_app_connection = NULL;

MailConnection::MailConnection(SdmError & error)
{
	error.Reset();

	if ( _app_connection ) {
		error = Sdm_EC_Success;
		return;
	}

	_app_connection = new SdmConnection("dtmail");
	_app_connection->StartUp(error);
	if (error) {
		return;
    }
}

MailConnection::~MailConnection(void)
{
    delete _app_connection;
}

