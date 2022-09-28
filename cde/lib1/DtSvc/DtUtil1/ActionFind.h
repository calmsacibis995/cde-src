/* $XConsortium: ActionFind.h /main/cde1_maint/3 1995/10/08 22:16:30 pascale $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        ActionFind.h
 **
 **   Project:     DT
 **
 **   Description: Public include file for the ActionFind functions.
 **
 ** (c) Copyright 1993, 1994 Hewlett-Packard Company
 ** (c) Copyright 1993, 1994 International Business Machines Corp.
 ** (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 ** (c) Copyright 1993, 1994 Novell, Inc.
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _Dt_ActionFind_h
#define _Dt_ActionFind_h

#include <X11/Xlib.h>
#include <X11/Xresource.h>

#include <Dt/ActionP.h>
#include <Dt/DtsDb.h>
#include <Dt/DtsMM.h>

#ifdef _NO_PROTO

extern void _DtSortActionDb();
extern ActionPtr _DtActionFindDBEntry();	

#else

#  ifdef __cplusplus
extern "C" {
#  endif

extern void _DtSortActionDb(void);
extern ActionPtr
_DtActionFindDBEntry(	ActionRequest *reqp,
			DtShmBoson actQuark );

#  ifdef __cplusplus
}
#  endif

#endif /* _NO_PROTO */

#endif /* _Dt_ActionFind_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

