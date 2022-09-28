/*
 * $XConsortium: TermPrimSetUtmp.h /main/cde1_maint/1 1995/07/15 01:36:07 drk $";
 */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
#ifndef	_Dt_TermPrimSetUtmp_h
#define	_Dt_TermPrimSetUtmp_h

#define	TermUtmpIdString	"__TermUtmpId"

void _DtTermPrimUtmpInit(Widget w);
void _DtTermPrimUtmpAddEntry
(
    char		 *utmpLine
);
char *_DtTermPrimUtmpGetUtLine(int pty, char *ptyName);
char *_DtTermPrimUtmpEntryCreate(Widget w, int pid, char *utmpLine);
void _DtTermPrimUtmpEntryDestroy(Widget w, char *utId);
void _DtTermPrimUtmpCleanup();

#endif	/* _Dt_TermPrimSetUtmp_h */
/* DON'T ADD ANYTHING AFTER THIS #endif... */
