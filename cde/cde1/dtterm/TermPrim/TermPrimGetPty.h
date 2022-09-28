/*
 * $XConsortium: TermPrimGetPty.h /main/cde1_maint/1 1995/07/15 01:26:36 drk $";
 */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
#ifndef	_Dt_TermPrimGetPty_h
#define	_Dt_TermPrimGetPty_h

int _DtTermPrimGetPty(char **ptySlave, char **ptyMaster);
int _DtTermPrimSetupPty(char *ptySlave, int ptyFd);
void _DtTermPrimReleasePty(char *ptySlave);
void _DtTermPrimPtyCleanup();

#endif	/* _Dt_TermPrimGetPty_h */
/* DON'T ADD ANYTHING AFTER THIS #endif... */
