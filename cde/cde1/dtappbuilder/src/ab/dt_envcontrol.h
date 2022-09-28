
/*
 *	$XConsortium: dt_envcontrol.h /main/cde1_maint/2 1995/10/16 11:03:40 rcs $
 *
 * @(#)dt_envcontrol.h	1.2 11 Aug 1994
 *
 * 	RESTRICTED CONFIDENTIAL INFORMATION:
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
 */

/* 
 * File dt_envcontrol.h
 *
 * <Dt/EnvControl.h> is not C++-safe. We cannot put extern "C" statements
 * in the .c file, because stubsmerge cannot handle it.
 */

#ifndef _AB_DT_ENVCONTROL_H_
#define _AB_DT_ENVCONTROL_H_

#ifdef __cplusplus
extern "C" {                    /* header file is hosed */
#endif

#include <Dt/EnvControl.h>

#ifdef __cplusplus
} // extern "C"
#endif


#endif /* _AB_DT_ENVCONTROL_H_ */

