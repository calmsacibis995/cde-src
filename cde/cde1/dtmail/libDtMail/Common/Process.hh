/*
 *+SNOTICE
 *
 *
 *	$Revision: 1.2 $
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
#pragma ident "@(#)Process.hh	1.2 03/02/94"
#endif

#ifndef _PROCESS_HH
#define _PROCESS_HH

int RunProg(const char * program,
	    char *const * argv,
	    const char * stdin_data,
	    const unsigned long stdin_size,
	    char ** stdout_data,
	    unsigned long & stdout_size,
	    char ** stderr_data,
	    unsigned long & stderr_size);

#endif
