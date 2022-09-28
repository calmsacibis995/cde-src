/*
 * $XConsortium: TermPrimWMProtocols.h /main/cde1_maint/1 1995/07/15 01:37:52 drk $";
 */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
#ifndef	_Dt_TermPrimWMProtocols_h
#define	_Dt_TermPrimWMProtocols_h

extern void _DtTermPrimAddDeleteWindowCallback(Widget topLevel,
	XtCallbackProc callback, XtPointer client_data);
extern void _DtTermPrimAddSaveYourselfCallback(Widget topLevel,
	XtCallbackProc callback, XtPointer client_data);

#endif	/* _Dt_TermPrimWMProtocols_h */
/* DON'T ADD ANYTHING AFTER THIS #endif... */
