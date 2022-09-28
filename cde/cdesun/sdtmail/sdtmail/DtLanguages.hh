/*
 *+SNOTICE
 *
 *
 *	$Revision: 1.3 $
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
#pragma ident "@(#)DtLanguages.hh	1.3 02/17/94"
#endif

#ifndef _DTLANGUAGES_HH
#define _DTLANGUAGES_HH

#include <stddef.h>

// The DtCPlusPlusAllocator class is provided to allow C applications
// to access the C++ implementation. C applications will not typically
// have access to new and delete so they are over ridden here.
//
class DtCPlusPlusAllocator {
  public:
    static void * operator new(size_t size);
    static void operator delete(void * ptr);
};

#endif
