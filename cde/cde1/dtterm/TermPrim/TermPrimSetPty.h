/*
 * $XConsortium: TermPrimSetPty.h /main/cde1_maint/1 1995/07/15 01:35:39 drk $";
 */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
#ifndef	_Dt_TermPrimSetPty_h
#define	_Dt_TermPrimSetPty_h

void _DtTermPrimPtySendBreak(int pty, int msec);
void _DtTermPrimPtyGetDefaultModes();
void _DtTermPrimPtyInit(int pty, char *modeString, char *csWidthString);
void _DtTermPrimPtySetWindowSize(int pty, short pixelWidth, short pixelHeight,
	short characterRows, short characterColumns);
void _DtTermPrimPtyConsoleModeEnable(int pty);

#endif	/* _Dt_TermPrimSetPty_h */
/* DON'T ADD ANYTHING AFTER THIS #endif... */
