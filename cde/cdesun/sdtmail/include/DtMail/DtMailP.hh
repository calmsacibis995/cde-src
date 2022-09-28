/*
 *+SNOTICE
 *
 *
 *	$Revision: $
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
#pragma ident "@(#)DtMailP.hh	1.3 11/14/95"
#endif

#ifndef _DTMAILP_HH
#define _DTMAILP_HH

// This method will write a single mail message to a file, in
// the correct SVID format.
//
void RFCWriteMessage(DtMailEnv & error,
		     DtMail::Session * session,
		     const char * path,
		     DtMail::Message * msg,
		     Boolean append);

void DtMailProcessClientEvents(void);

#endif
