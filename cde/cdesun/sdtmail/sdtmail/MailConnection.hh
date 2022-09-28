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

#ifndef _MAILCONNECTION_HH
#define _MAILCONNECTION_HH

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)MailConnection.hh	1.4 05/07/96"
#endif

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Connection.hh>

class MailConnection {
  public:
    MailConnection(SdmError &);
    ~MailConnection(void);

    SdmConnection * connection(void) { return _app_connection; }

  private:
	static SdmConnection		*_app_connection;
};

#endif
