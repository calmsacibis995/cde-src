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
#pragma ident "@(#)DynamicLib.hh	1.5 03/01/95"
#endif

#ifndef _DYNAMICLIB_HH
#define _DYNAMICLIB_HH

#include <DtMail/DtLanguages.hh>

class DynamicLib : public DtCPlusPlusAllocator {
  public:
    virtual ~DynamicLib(void);
    virtual void * getSym(const char * symbol) = 0;

  protected:
    DynamicLib(const char *) { }
};

DynamicLib * CreatePlatformDl(const char * lib);

#endif
