/*
 * $XConsortium: TermPrimUtil.h /main/cde1_maint/2 1995/10/09 11:50:07 pascale $";
 */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
#ifndef	_Dt_TermPrimUtil_h
#define	_Dt_TermPrimUtil_h

#include <Xm/Xm.h>
#ifdef	__cplusplus
extern "C" {
#endif	/* __cplusplus */

extern void _DtTermPrimPointerOn(Widget w);
extern void _DtTermPrimPointerOff(Widget w, XtIntervalId *id) ;
extern void _DtTermPrimPointerFreeze(Widget w, Boolean frozen) ;
extern void _DtTermPrimRecolorPointer(Widget w) ;
extern void _DtTermWriteLog(DtTermPrimitiveWidget tw, char *buffer, int len) ;
extern void _DtTermStartLog(Widget w) ;
extern void _DtTermCloseLog(Widget w) ;
extern void _DtTermPrimFlushLog(Widget w);
extern void _DtTermPrimLogPipe(int);
extern void _DtTermPrimLogFileCleanup(void);

extern void _DtTermPrimRemoveSuidRoot();
extern void _DtTermPrimToggleSuidRoot(Boolean root);

#endif	/* _Dt_TermPrimUtil_h */

/* DON'T ADD ANYTHING AFTER THIS #endif... */
