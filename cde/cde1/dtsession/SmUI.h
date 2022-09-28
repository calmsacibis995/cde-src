/* $XConsortium: SmUI.h /main/cde1_maint/3 1995/10/09 10:58:08 pascale $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        SmUI.h
 **
 **   Project:     HP DT
 **
 **   Description
 **   -----------
 **   Variables and declarations needed for
 **   User Interface for the session manager
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990.  
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _smui_h
#define _smui_h
 
/* 
 *  #include statements 
 */
#include <Xm/Xm.h>


/* 
 *  #define statements 
 */

/* 
 * typedef statements 
 */

typedef struct
{
    Widget              confExit;               /* Exit confirmation dialogs*/
    Widget              qExit;
    Widget              compatExit;
    Widget              lockDialog;             /* lock-login shell */
    Widget              lockCoverDialog;        /* lock-login shell */
    Widget              coverDialog[10];        /* Cover shell*/
    Widget              coverDrawing[10];       /* Cover drawing area */
    Widget              matte[2];               /* lock dialog pieces */
    Widget              loginMatte[2];
    Widget              loginForm[2];
    Widget              indLabel[2];
    Widget              deadWid;                /* Bms-dead dialog */
    XmString            saveString, noSaveString, noRestoreString;
    XmString            okString, cancelString, helpString;
    XmString            okLogoutString, cancelLogoutString;
    Widget              noStart;                /* Cant start dt dialog */
#ifdef __osf__
    Widget              newProfile;             /* new dtprofile dialog */
#endif
    Widget              smHelpDialog;   /* Help dialog for all topics */
} DialogData;

/*
 *  External variables  
 */
extern DialogData       smDD;
extern Arg              uiArgs[20];


/*  
 *  External Interface  
 */

#ifdef _NO_PROTO

extern Widget CreateLockDialog() ;
extern void ExitSession() ;
extern int WarnMsgFailure() ;
extern Widget CreateLockDialogWithCover() ;
extern Widget CreateCoverDialog() ;
extern void ImmediateExit() ;
extern void ShowWaitState() ;
extern Boolean InitCursorInfo() ;
extern void UpdatePasswdField();
extern int WarnNoStartup();
extern void DialogUp() ;
#ifdef __osf__
extern int WarnNewProfile();
#endif

#else

extern Widget CreateLockDialog( void ) ;
extern void ExitSession( Tt_message ) ;
extern int WarnMsgFailure( void ) ;
extern Widget CreateLockDialogWithCover( Widget ) ;
extern Widget CreateCoverDialog( int, Boolean ) ;
extern void ImmediateExit( int, Tt_message ) ;
extern void ShowWaitState( Boolean ) ; 
extern Boolean InitCursorInfo( void ) ; 
extern void UpdatePasswdField( int );
extern int WarnNoStartup( void );
extern void DialogUp( Widget, XtPointer, XtPointer ) ;
#ifdef __osf__
extern int WarnNewProfile( void );
#endif

#endif /* _NO_PROTO */


#endif /*_smui_h*/
/* DON'T ADD ANYTHING AFTER THIS #endif */
