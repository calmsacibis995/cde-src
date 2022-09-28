/*
 * $XConsortium: TermPrimCursor.h /main/cde1_maint/1 1995/07/15 01:24:27 drk $";
 */

/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef	_Dt_TermPrimCursor_h
#define	_Dt_TermPrimCursor_h

extern void _DtTermPrimCursorOff(Widget w);
extern void _DtTermPrimCursorOn(Widget w);
extern void _DtTermPrimCursorChangeFocus(Widget w);
extern void _DtTermPrimCursorMove(Widget w, int row, int col);
extern void _DtTermPrimCursorUpdate(Widget w);
extern void _DtTermPrimSetCursorVisible(Widget w, Boolean visible);
extern Boolean _DtTermPrimGetCursorVisible(Widget w);
#endif	/* _Dt_TermPrimCursor_h */
/* DON'T ADD ANYTHING AFTER THIS #endif... */
