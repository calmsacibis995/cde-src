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
#pragma ident "@(#)DlDynamicLib.hh	1.2 02/17/94"
#endif

// This implementation is for platforms that use dlopen/dlsym/dlclose.
//

#if !defined(_DLDYNAMICLIB_HH) && defined(DL_DYNAMIC_LIBS)
#define _DLDYNAMICLIB_HH

#include "DynamicLib.hh"

#include <dlfcn.h>

class DlDynamicLib : public DynamicLib {
  public:
    DlDynamicLib(const char * path);
    virtual ~DlDynamicLib(void);
    void * getSym(const char * symbol);

  private:
    void	*_lib_handle;
};

#endif
