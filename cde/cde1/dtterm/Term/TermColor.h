/*
 * $XConsortium: TermColor.h /main/cde1_maint/1 1995/07/15 01:17:27 drk $";
 */

/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef	_Dt_TermColor_h
#define	_Dt_TermColor_h

void _DtTermColorInit(Widget w);
void _DtTermColorDestroy(Widget w);
void _DtTermColorInitializeColorPair(Widget w, VtColorPair colorPair);

#endif	/* _Dt_TermColor_h */
/* DON'T ADD ANYTHING AFTER THIS #endif... */
