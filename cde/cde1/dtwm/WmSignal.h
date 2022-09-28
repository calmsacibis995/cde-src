/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $XConsortium: WmSignal.h /main/cde1_maint/1 1995/07/15 02:00:25 drk $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void SetupWmSignalHandlers ();
extern void QuitWmSignalHandler ();
extern void RestoreDefaultSignalHandlers ();
#else /* _NO_PROTO */
extern void SetupWmSignalHandlers (int);
extern void QuitWmSignalHandler (int);
extern void RestoreDefaultSignalHandlers (void);
#endif /* _NO_PROTO */
