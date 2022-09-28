/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $XConsortium: WmError.h /main/cde1_maint/1 1995/07/15 01:47:30 drk $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
#ifdef WSM
#ifdef DEBUGGER
extern void PrintFormatted ();
#endif /* DEBUGGER */
#endif /* WSM */
extern void WmInitErrorHandler ();
extern int WmXErrorHandler ();
extern int WmXIOErrorHandler ();
extern void WmXtErrorHandler ();
extern void WmXtWarningHandler ();
extern void Warning ();
#ifdef MESSAGE_CAT
extern char * GetMessage();
#endif /* MESSAGE_CAT */
#else /* _NO_PROTO */
#ifdef WSM
#ifdef DEBUGGER
extern void PrintFormatted(char *f, char *s0, char *s1, char *s2, char *s3, char *s4, char *s5, char *s6, char *s7, char *s8, char *s9);
#endif /* DEBUGGER */
#endif /* WSM */
extern void WmInitErrorHandler (Display *display);
extern int WmXErrorHandler (Display *display, XErrorEvent *errorEvent);
extern int WmXIOErrorHandler (Display *display);
extern void WmXtErrorHandler (char *message);
extern void WmXtWarningHandler (char *message);
extern void Warning (char *message);
#ifdef MESSAGE_CAT
extern char * GetMessage(int set, int n, char * s);
#endif /* MESSAGE_CAT */
#endif /* _NO_PROTO */
#ifdef WSM
/****************************   eof    ***************************/
#endif /* WSM */
