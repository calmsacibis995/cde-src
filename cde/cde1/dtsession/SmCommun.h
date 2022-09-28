/* $XConsortium: SmCommun.h /main/cde1_maint/2 1995/08/30 16:25:09 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        SmCommun.h
 **
 **   Project:     HP DT
 **
 **   Description
 **   -----------
 **   Variables and declarations
 **   needed for Communication for the session manager
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990.  
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _smcommun_h
#define _smcommun_h
 
/* 
 *  #include statements 
 */


/* 
 *  #define statements 
 */

/* 
 * typedef statements 
 */
/*
 *  External variables  
 */


/*  
 *  External Interface  
 */
#ifdef _NO_PROTO

extern void StartMsgServer() ;
extern void ProcessClientMessage();
extern void ProcessScreenSaverMessage();
extern void RestoreDefaults() ;
extern void ProcessPropertyNotify() ;
extern void ProcessReloadActionsDatabase();
extern void ProcessEvent();

#else

extern void StartMsgServer(void) ;
extern void ProcessClientMessage(XEvent *);
extern void ProcessScreenSaverMessage(XEvent *);
extern void RestoreDefaults(Atom) ;
extern void ProcessPropertyNotify(XEvent *) ;
extern void ProcessReloadActionsDatabase(void);
extern void ProcessEvent(Widget, XtPointer, XEvent *, Boolean *);



#endif /* _NO_PROTO */

#endif /*_smcommun_h*/
/* DON'T ADD ANYTHING AFTER THIS #endif */
