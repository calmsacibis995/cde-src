/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $XConsortium: WmProtocol.h /main/cde1_maint/1 1995/07/15 01:57:52 drk $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void SetupWmICCC ();
extern void SendConfigureNotify ();
extern void SendClientOffsetMessage ();
extern void SendClientMsg ();
extern Boolean AddWmTimer ();
extern void DeleteClientWmTimers ();
extern void TimeoutProc ();
#else /* _NO_PROTO */
extern void SetupWmICCC (void);
extern void SendConfigureNotify (ClientData *pCD);
extern void SendClientOffsetMessage (ClientData *pCD);
extern void SendClientMsg (Window window, long type, long data0, Time time, long *pData, int dataLen);
extern Boolean AddWmTimer (unsigned int timerType, unsigned long timerInterval, ClientData *pCD);
extern void DeleteClientWmTimers (ClientData *pCD);
extern void TimeoutProc (caddr_t client_data, XtIntervalId *id);
#endif /* _NO_PROTO */
