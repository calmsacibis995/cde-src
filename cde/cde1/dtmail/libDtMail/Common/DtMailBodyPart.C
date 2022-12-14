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
#pragma ident "@(#)DtMailBodyPart.C	1.8 03/15/94"
#endif

#include <DtMail/DtMail.hh>
#include <DtMail/Threads.hh>

DtMail::BodyPart::BodyPart(DtMailEnv &, DtMail::Message * parent)
{
    _parent = parent;
    _obj_mutex = MutexInit();
}

DtMail::BodyPart::~BodyPart(void)
{
    MutexDestroy(_obj_mutex);
}
