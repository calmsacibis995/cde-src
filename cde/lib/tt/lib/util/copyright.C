//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $XConsortium: copyright.C /main/cde1_maint/1 1995/07/14 18:34:48 drk $ 			 				
/*
 * copyright file needed in each executable or library
 */

#define DONT_EXTERN_TT_LIB_VERSION
#include "util/copyright.h"
#include <tt_options.h>

const char _tt_lib_copyright[] = _TT_COPYRIGHT;
const char _tt_lib_copyright2[] = _TT_COPYRIGHT2;
const char _tt_lib_copyright3[] = _TT_COPYRIGHT3;

const char *_tt_lib_version = TT_VERSION_STRING;


#ifdef OPT_PATCH
const char PatchID[] = "\nPatch Id: none.";
int Patch_ID_none;
#endif
