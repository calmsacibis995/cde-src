/* $XConsortium: SmUI.c /main/cde1_maint/4 1995/10/09 10:57:58 pascale $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:        SmUI.c
 **
 **  Project:     DT Session Manager (dtsession)
 **
 **  Description:
 **  -----------
 **  This file handles all UI components of the session manager.  This
 **  includes all dialog boxes.   The session manager does not handle its
 **  representations in the front panel and customizer.  That is handled
 **  by those tools.
 **
 **
 **
 *******************************************************************
 **  (c) Copyright Hewlett-Packard Company, 1990.  All rights are  
 **  reserved.  Copying or other reproduction of this program      
 **  except for archival purposes is prohibited without prior      
 **  written consent of Hewlett-Packard Company.		     
 ********************************************************************
 **
 **
 **
 *****************************************************************************
 *************************************<+>*************************************/

#include <signal.h>
#include <stdio.h>
#include <pwd.h>
#include <X11/Intrinsic.h>
#include <Xm/MwmUtil.h>
#include <Xm/Xm.h>
#include <Xm/PushB.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/DrawingA.h>
#include <Xm/BulletinB.h>
#include <Xm/Frame.h>
#include <Xm/Separator.h>
#include <Xm/MessageB.h>
#include <Xm/DialogS.h>
#include <Xm/Text.h>
#include <Dt/DtP.h>
#include <Dt/SessionM.h>
#include <Dt/StandardM.h>
#include <Dt/WsmM.h>
#include <Dt/FileM.h>
#include <Dt/Icon.h>
#ifdef __annotate
#include <Dt/AnnotateM.h>
#endif
#include <Dt/Indicator.h>
#include <Dt/DtpadM.h>

#include "Sm.h"
#include "SmUI.h"
#include "SmSave.h"
#include "SmHelp.h"
#include "SmGlobals.h"

#ifdef __apollo
#include <X11/apollosys.h>
#endif  /* __apollo */


/*
 * #define statements
 */
#define PASSWORD_INDICATOR " "

typedef struct _ExitRecord {
  Tt_message     *pmsg;
  union {
    Widget exitCancelledDialog;
    int queryExitConfirmedMode;
  } u;
} ExitRecord;

/*
 * Global variables
 */
DialogData   	smDD;
Arg      		uiArgs[20];
static XtIntervalId exitTimer=0;

/*
 * Local Function Declarations
 */
#ifdef _NO_PROTO

static int ConfirmExit() ;
static int QueryExit() ;
static void ExitConfirmed() ;
static void QueryExitConfirmed() ;
static void ExitCancelled() ;
static void ExitTimeConfirmed() ;
static void QueryExitTimeConfirmed() ;
static void LockDialogUp() ;
static void SimpleOK() ;

#else

static int ConfirmExit( Tt_message ) ;
static int QueryExit( Tt_message ) ;
static void ExitConfirmed( Widget, XtPointer, XtPointer ) ;
static void QueryExitConfirmed( Widget, XtPointer, XtPointer ) ;
static void ExitCancelled( Widget, XtPointer, XtPointer ) ;
static void ExitTimeConfirmed( XtPointer, XtIntervalId * ) ;
static void QueryExitTimeConfirmed( XtPointer, XtIntervalId * ) ;
static void LockDialogUp( Widget, XtPointer, XtPointer ) ;
static void SimpleOK( Widget, XtPointer, XtPointer ) ;

#endif




/*************************************<->*************************************
 *
 *  CreateLockDialog ()
 *
 *
 *  Description:
 *  -----------
 *  Create the lock dialog when it exists NOT as a part of a cover
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
Widget 
#ifdef _NO_PROTO
CreateLockDialog()
#else
CreateLockDialog( void )
#endif /* _NO_PROTO */
{
    int 	i;
    Widget	loginLabel, instructLabel, tmpLock, indFrame;
    Widget  	passwdLabel, passwdForm, picFrame, loginPic, loginFrame;
    Dimension	width;		/* width, height of login label    */
    XmString	lockString, passwordString;
    char	lockMessage[100+L_cuserid];
    Pixel   fg, bg, focus_color;         /* foreground, background colors   */
    char*	puser;
    struct passwd* pwd;
    
    i = 0;
    XtSetArg(uiArgs[i], XmNallowShellResize, True); i++;
    XtSetArg(uiArgs[i], XmNmarginWidth, 0); i++;
    XtSetArg(uiArgs[i], XmNmarginHeight, 0); i++;
    XtSetArg(uiArgs[i], XmNshadowType, XmSHADOW_OUT); i++;
    XtSetArg(uiArgs[i], XmNshadowThickness,5); i++;
    XtSetArg(uiArgs[i], XmNunitType, XmPIXELS); i++;
    XtSetArg(uiArgs[i], XmNresizePolicy, XmRESIZE_NONE);i++;
    XtSetArg(uiArgs[i], XmNdialogStyle, XmDIALOG_SYSTEM_MODAL); i++;
    XtSetArg(uiArgs[i], XmNmwmInputMode, MWM_INPUT_SYSTEM_MODAL);i++;
    XtSetArg(uiArgs[i], XmNmappedWhenManaged, True); i++;
    tmpLock = XmCreateFormDialog(smGD.topLevelWid, "lockDialog", uiArgs, i);
    XtAddCallback (XtParent(tmpLock), XmNpopupCallback,
		   LockDialogUp, NULL);
    smDD.matte[0] = tmpLock;

    i = 0;
    XtSetArg(uiArgs[i], XmNuseAsyncGeometry, True);i++;
    XtSetArg(uiArgs[i], XmNmwmDecorations, 0);i++;
    XtSetArg(uiArgs[i], XmNmwmInputMode, MWM_INPUT_SYSTEM_MODAL);i++;
    XtSetValues(XtParent(tmpLock), uiArgs, i);
		
    i = 0;
    XtSetArg(uiArgs[i], XmNshadowType, XmSHADOW_OUT); i++;
    XtSetArg(uiArgs[i], XmNshadowThickness, 2); i++; 
    XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_FORM); i++;
    XtSetArg(uiArgs[i], XmNtopOffset, 15); i++;
    XtSetArg(uiArgs[i], XmNbottomAttachment, XmATTACH_FORM); i++;
    XtSetArg(uiArgs[i], XmNbottomOffset, 15); i++;
    XtSetArg(uiArgs[i], XmNrightAttachment, XmATTACH_FORM); i++;
    XtSetArg(uiArgs[i], XmNrightOffset, 15); i++;
    picFrame = XmCreateFrame(tmpLock, "picFrame", uiArgs, i);

	i = 0;
	XtSetArg(uiArgs[i], XmNforeground, &fg); i++;
	XtSetArg(uiArgs[i], XmNbackground, &bg); i++;
	XtGetValues(tmpLock, uiArgs, i);

    i = 0;
	XtSetArg(uiArgs[i], XmNfillMode, XmFILL_SELF); i++;
	XtSetArg(uiArgs[i], XmNbehavior, XmICON_LABEL); i++;
	XtSetArg(uiArgs[i], XmNpixmapForeground, fg); i++;
	XtSetArg(uiArgs[i], XmNpixmapBackground, bg); i++;
	XtSetArg(uiArgs[i], XmNstring, NULL); i++;
	XtSetArg(uiArgs[i], XmNshadowThickness, 0); i++;
	XtSetArg(uiArgs[i], XmNtraversalOn, False); i++;
	loginPic = _DtCreateIcon(picFrame, "lockLabelPixmap", uiArgs, i);

    i = 0;
    XtSetArg(uiArgs[i], XmNshadowType, XmSHADOW_OUT); i++;
    XtSetArg(uiArgs[i], XmNshadowThickness, 2); i++; 
    XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_FORM); i++;
    XtSetArg(uiArgs[i], XmNtopOffset, 15); i++;
    XtSetArg(uiArgs[i], XmNbottomAttachment, XmATTACH_FORM); i++;
    XtSetArg(uiArgs[i], XmNbottomOffset, 15); i++;
    XtSetArg(uiArgs[i], XmNleftAttachment, XmATTACH_FORM); i++;
    XtSetArg(uiArgs[i], XmNleftOffset, 15); i++;
    XtSetArg(uiArgs[i], XmNrightAttachment, XmATTACH_WIDGET); i++;
    XtSetArg(uiArgs[i], XmNrightOffset, 0); i++;
    XtSetArg(uiArgs[i], XmNrightWidget, loginPic); i++;
    loginFrame = XmCreateFrame(tmpLock, "loginFrame", uiArgs, i);


    /* 
     * create the login matte...
     */
    i = 0;
    XtSetArg(uiArgs[i], XmNresizePolicy, XmRESIZE_NONE);i++;
    smDD.loginMatte[0] = XmCreateForm(loginFrame, "loginMatte", uiArgs, i);

    /*
     *  create the login/password forms
     */
    i = 0;
    XtSetArg(uiArgs[i], XmNshadowThickness, 0); i++; 
    XtSetArg(uiArgs[i], XmNrightAttachment, XmATTACH_FORM); i++;
    XtSetArg(uiArgs[i], XmNrightOffset, 15); i++;
    XtSetArg(uiArgs[i], XmNbottomAttachment, XmATTACH_POSITION); i++;
    XtSetArg(uiArgs[i], XmNbottomPosition, 50); i++;
    XtSetArg(uiArgs[i], XmNleftAttachment, XmATTACH_FORM); i++;
    XtSetArg(uiArgs[i], XmNleftOffset, 15); i++;
    smDD.loginForm[0] = XmCreateForm(smDD.loginMatte[0], "loginForm", uiArgs, i);

    i = 0;
    XtSetArg(uiArgs[i], XmNresizePolicy, XmRESIZE_NONE);i++;
    XtSetArg(uiArgs[i], XmNshadowThickness, 0); i++; 
    XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_POSITION); i++;
    XtSetArg(uiArgs[i], XmNtopPosition, 50); i++;
	XtSetArg(uiArgs[i], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); i++;
	XtSetArg(uiArgs[i], XmNleftWidget, smDD.loginForm[0]); i++;
	XtSetArg(uiArgs[i], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET); i++;
	XtSetArg(uiArgs[i], XmNrightWidget, smDD.loginForm[0]); i++;
    passwdForm = XmCreateForm(smDD.loginMatte[0], "passwdForm", uiArgs, i);


    /*
     *  create the login/password labels...
     */
    i = 0;
    puser = getenv("USER");
    if (!puser) {
	pwd = getpwuid(getuid());
	if (pwd) {
	   puser = pwd->pw_name;
	}
    }
    if (!puser) puser="";
    sprintf(lockMessage, ((char *)GETMESSAGE(18, 1, "Display locked by user %s.")), puser);
    lockString = XmStringCreateLtoR(lockMessage, XmFONTLIST_DEFAULT_TAG);
    XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_POSITION); i++;
    XtSetArg(uiArgs[i], XmNtopPosition, 20); i++;
    XtSetArg(uiArgs[i], XmNalignment, XmALIGNMENT_CENTER); i++;
    XtSetArg(uiArgs[i], XmNlabelString,	lockString); i++;
    loginLabel = XmCreateLabelGadget(smDD.loginForm[0],
				     "loginLabel", uiArgs, i);
    XtManageChild(loginLabel);
    XmStringFree(lockString);

    i = 0;
    lockString = XmStringCreateLtoR(((char *)GETMESSAGE(18, 2, "Enter password to unlock.")),
				    XmFONTLIST_DEFAULT_TAG);
    XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_WIDGET); i++;
    XtSetArg(uiArgs[i], XmNtopWidget, loginLabel); i++;
    XtSetArg(uiArgs[i], XmNalignment, XmALIGNMENT_CENTER); i++;
    XtSetArg(uiArgs[i], XmNlabelString,	lockString); i++;
    instructLabel = XmCreateLabelGadget(smDD.loginForm[0], "instructLabel",
					uiArgs, i);
    XtManageChild(instructLabel);
    XmStringFree(lockString);

    i = 0;
    passwordString = XmStringCreateLtoR(((char *)GETMESSAGE(18, 3, "Password: ")),
					XmFONTLIST_DEFAULT_TAG);
    XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_POSITION); i++;
    XtSetArg(uiArgs[i], XmNtopPosition, 20); i++;
    XtSetArg(uiArgs[i], XmNbottomAttachment, XmATTACH_POSITION); i++;
    XtSetArg(uiArgs[i], XmNbottomPosition, 80); i++;
    XtSetArg(uiArgs[i], XmNleftAttachment, XmATTACH_FORM); i++;
    XtSetArg(uiArgs[i], XmNalignment, XmALIGNMENT_END); i++;
    XtSetArg(uiArgs[i], XmNlabelString,	passwordString); i++;
    passwdLabel = XmCreateLabelGadget(passwdForm,
				      "passwdLabel", uiArgs, i);
    XtManageChild(passwdLabel);
    XmStringFree(passwordString);

    /*
     * Give the password label an offset
     */
    i = 0;
    XtSetArg(uiArgs[i], XmNwidth, &width);i++;
    XtGetValues(passwdLabel, uiArgs, i);

    i = 0;
    width += (width/6);
    XtSetArg(uiArgs[i], XmNwidth, width);i++;
    XtSetArg(uiArgs[i], XmNrecomputeSize, False);i++;
    XtSetValues(passwdLabel, uiArgs, i);


    i = 0;
    XtSetArg(uiArgs[i], XmNshadowType, XmSHADOW_IN); i++;
    XtSetArg(uiArgs[i], XmNshadowThickness, 1); i++;
    XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_POSITION); i++;
    XtSetArg(uiArgs[i], XmNtopPosition, 20); i++;
    XtSetArg(uiArgs[i], XmNbottomAttachment, XmATTACH_POSITION); i++;
    XtSetArg(uiArgs[i], XmNbottomPosition, 80); i++;
    XtSetArg(uiArgs[i], XmNleftAttachment, XmATTACH_WIDGET); i++;
    XtSetArg(uiArgs[i], XmNleftWidget, passwdLabel); i++;
    XtSetArg(uiArgs[i], XmNrightAttachment, XmATTACH_FORM); i++;
    XtSetArg(uiArgs[i], XmNrightOffset, 10); i++;
    XtSetArg(uiArgs[i], XmNalignment, XmALIGNMENT_END); i++;
    indFrame = XmCreateFrame(passwdForm,  "indFrame", uiArgs, i);

    i = 0;
    passwordString = XmStringCreateLtoR("|",
					XmFONTLIST_DEFAULT_TAG);
    XtSetArg(uiArgs[i], XmNlabelString,	passwordString); i++;
    XtSetArg(uiArgs[i], XmNalignment, XmALIGNMENT_BEGINNING); i++;
    smDD.indLabel[0] = XmCreateLabel(indFrame, "indLabel",
					uiArgs, i);
    XtManageChild(smDD.indLabel[0]);
    XmStringFree(passwordString);

    i = 0;
    XtSetArg(uiArgs[i], XmNhighlightColor,  &focus_color); i++;
    XtGetValues(indFrame, uiArgs, i);
    XtVaSetValues ( smDD.indLabel[0],
                    XmNborderWidth, 2,
                    XmNborderColor,  focus_color, 
                    NULL );
    
    /*
     * Manage forms AFTER all children have been managed
     */
    XtManageChild(indFrame);
    XtManageChild(passwdForm);
    XtManageChild(smDD.loginForm[0]);
    XtManageChild(smDD.loginMatte[0]);
    XtManageChild(loginPic);
    XtManageChild(picFrame);
    XtManageChild(loginFrame);

    return(tmpLock);
}



/*************************************<->*************************************
 *
 *  ExitSession ()
 *
 *
 *  Description:
 *  -----------
 *  Determines which exit routines get called when an exit request is made
 *  of the session manager.  
 *  If smGD.bmsDead == false, we just exit.
 *  If ASK_STATE is turned on, the query dialog is
 *  put up, if VERBOSE is on, confirm exit in current mode (restart or reset)
 *  if confirmation is turned off - exit immediately.
 *
 *
 *  Inputs:
 *  ------
 *  msg -- if non-zero, Session_Exit request to reply/fail && destroy
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void
#ifdef _NO_PROTO
ExitSession(msg)
	Tt_message msg;
#else
ExitSession(
	Tt_message msg)
#endif /* _NO_PROTO */
{
    if (smGD.bmsDead == True)
    {
	ImmediateExit(-1, msg);
    }
    else
    {
	if(smSettings.startState == DtSM_ASK_STATE)
	{
	    QueryExit( msg );
	}
	else
	{
	    if(smSettings.confirmMode == DtSM_VERBOSE_MODE)
	    {
		ConfirmExit( msg );
	    }
	    else
	    {
		ImmediateExit(smSettings.startState, msg);
	    }
	}
    }
}




/*************************************<->*************************************
 *
 *  ConfirmExit ()
 *
 *
 *  Description:
 *  -----------
 *  Create the exit confirmation dialog box (if it hasn't been) and confirm
 *  that the user wants to exit the session.  This routine only gets called
 *  when the user hasn't turned of exit confirmation and is not in ASK mode
 *
 *
 *  Inputs:
 *  ------
 *  msg -- if non-zero, Session_Exit request to reply/fail && destroy
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static int 
#ifdef _NO_PROTO
ConfirmExit(msg)
Tt_message msg;
#else
ConfirmExit( Tt_message msg )
#endif /* _NO_PROTO */
{
    int 	i;
    String	tmpString;
    ExitRecord  *exitRec;
    Tt_message  *pmsg;

    if(smDD.confExit == NULL)
    {
	/*
	 * Create all compound strings for confirmation dialogs
	 */
	smDD.saveString = XmStringLtoRCreate(((char *)GETMESSAGE(18, 4, 
		"Exiting the desktop session...\n\n\
Application updates you have not saved may be lost.\n\n\
Your current session will be saved and returned to \n\
upon your next login.  For more detail, select Help.\n\n\
Continue Logout?")),
		XmFONTLIST_DEFAULT_TAG);
	smDD.noSaveString = XmStringLtoRCreate(((char *)GETMESSAGE(18, 5, 
		"Exiting the desktop session...\n\n\
Application updates you have not saved may be lost.\n\n\
Your home session will be restored upon login.  Your current\n\
session will not be saved.  For more detail, select Help.\n\n\
Continue Logout?")),
		XmFONTLIST_DEFAULT_TAG);
	smDD.noRestoreString = XmStringLtoRCreate(((char *)GETMESSAGE(18, 34, 
		"Exiting the desktop session...\n\n\
Application updates you have not saved may be lost.\n\n\
Continue Logout?")),
		XmFONTLIST_DEFAULT_TAG);

	/*
	 * Build up the correct string for this invokation of the dialog
	 */
	i = 0;
	if (smGD.compatMode == True) 
	{
	    XtSetArg(uiArgs[i], XmNmessageString, smDD.noRestoreString);i++;
	} else if (smSettings.startState == DtSM_HOME_STATE) 
	{
	    XtSetArg(uiArgs[i], XmNmessageString, smDD.noSaveString);i++;
	} 
	else 
	{
	    XtSetArg(uiArgs[i], XmNmessageString, smDD.saveString);i++;
	}

	/*
	 * Now create the dialog box
	 */
	tmpString = GETMESSAGE(18, 6, "Logout Confirmation");
	XtSetArg (uiArgs[i], XmNallowShellResize, False);  i++;
	XtSetArg(uiArgs[i], XmNdialogStyle, XmDIALOG_SYSTEM_MODAL); i++;
	XtSetArg(uiArgs[i], XmNmessageAlignment, XmALIGNMENT_CENTER); i++;
	XtSetArg(uiArgs[i], XmNtitle, tmpString); i++;
	XtSetArg(uiArgs[i], XmNokLabelString, smDD.okLogoutString); i++;
	XtSetArg(uiArgs[i], XmNcancelLabelString, smDD.cancelLogoutString); i++;
	XtSetArg(uiArgs[i], XmNhelpLabelString, smDD.helpString); i++;
	XtSetArg(uiArgs[i], XmNautoUnmanage, False); i++;

        pmsg = (Tt_message *)XtMalloc(sizeof(Tt_message));
        XtSetArg(uiArgs[i], XmNuserData, pmsg); i++;

	smDD.confExit = XmCreateWarningDialog(smGD.topLevelWid, "exitDialog",
					      uiArgs, i);
	
	i = 0;
	XtSetArg(uiArgs[i], XmNuseAsyncGeometry, True);i++;
	XtSetArg(uiArgs[i], XmNmwmFunctions, 0);i++;
	XtSetArg(uiArgs[i], XmNmwmDecorations,
		 (MWM_DECOR_TITLE | MWM_DECOR_BORDER));i++;
    XtSetArg(uiArgs[i], XmNmwmInputMode, MWM_INPUT_SYSTEM_MODAL);i++;
	XtSetValues(XtParent(smDD.confExit), uiArgs, i);

        exitRec = (ExitRecord *)XtMalloc( sizeof(ExitRecord) );
        exitRec->pmsg = pmsg;
	XtAddCallback (smDD.confExit, XmNokCallback, ExitConfirmed, exitRec);

        /* dialog will self accept in 2 minutes */
        exitTimer = XtAppAddTimeOut(smGD.appCon, 120000, 
				    ExitTimeConfirmed, exitRec);

#ifndef NO_XVH
	XtAddCallback (smDD.confExit, XmNhelpCallback,
		       TopicHelpRequested, HELP_LOGOUT_STR);
#endif
	exitRec = (ExitRecord *)XtMalloc( sizeof(ExitRecord) );
	exitRec->pmsg = pmsg;
	exitRec->u.exitCancelledDialog = smDD.confExit;
	XtAddCallback (smDD.confExit, XmNcancelCallback,
		       ExitCancelled, exitRec);
    }
    else
    {
	i = 0;
	if (smGD.compatMode == True) 
	{
	    XtSetArg(uiArgs[i], XmNmessageString, smDD.noRestoreString);i++;
	} else if (smSettings.startState == DtSM_HOME_STATE) 
	{
	    XtSetArg(uiArgs[i], XmNmessageString, smDD.noSaveString);i++;
	} 
	else 
	{
	    XtSetArg(uiArgs[i], XmNmessageString, smDD.saveString);i++;
	}
	XtSetValues(smDD.confExit, uiArgs, i);

    }

   /*
    * Refresh buffer containing pointer to Tt_message.
    */
    i=0;
    XtSetArg(uiArgs[i], XmNuserData, &pmsg); i++;
    XtGetValues(smDD.confExit, uiArgs, i);
    *pmsg = msg;


    XtAddCallback (XtParent(smDD.confExit), XmNpopupCallback, DialogUp, NULL);
	
    XtManageChild(smDD.confExit);
    return(0);
}



/*************************************<->*************************************
 *
 *  QueryExit ()
 *
 *
 *  Description:
 *  -----------
 *  If the user has specified via the customizer that he/she wants to be
 *  asked what to do at the end of every session, this routine is called.
 *  It asks the user whether to restart the current session, reset to the
 *  home session, or cancel the exit.
 *
 *
 *  Inputs:
 *  ------
 *  msg -- if non-zero, Session_Exit request to reply/fail && destroy
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static int 
#ifdef _NO_PROTO
QueryExit(msg)
Tt_message msg;
#else
QueryExit( Tt_message msg )
#endif /* _NO_PROTO */
{
    int 	i;
    XmString    restartString, resetString, queryString;
    Widget	queryMB;
    Widget	restartButton, resetButton, newCancel, newHelp;
    String	tmpString;
    ExitRecord* exitRec;
    Tt_message  *pmsg;

    if(smDD.qExit == NULL)
    {
	queryString = XmStringCreateLtoR(((char *)GETMESSAGE(18, 7, 
		"Exiting the desktop session...\n\n\
Application updates you have not saved may be lost.\n\n\
You can either save your current session and return to it upon login,\n\
or return to your home session upon login.  For more detail, select Help.\n\n\
Continue Logout?")),
		XmFONTLIST_DEFAULT_TAG);
	restartString = XmStringCreateLtoR(((char *)GETMESSAGE(18, 8, 
		"OK\nSave Current")),
		XmFONTLIST_DEFAULT_TAG);
	resetString = XmStringCreateLtoR(((char *)GETMESSAGE(18, 9, 
		"OK\nReturn Home")),
		XmFONTLIST_DEFAULT_TAG);

	i = 0;
	XtSetArg(uiArgs[i], XmNdialogStyle, XmDIALOG_SYSTEM_MODAL); i++;
	XtSetArg(uiArgs[i], XmNautoUnmanage, False); i++;

        pmsg = (Tt_message *)XtMalloc(sizeof(Tt_message));
        XtSetArg(uiArgs[i], XmNuserData, pmsg); i++;

	smDD.qExit = XmCreateFormDialog(smGD.topLevelWid, "exitDialog",
					uiArgs, i);

	i = 0;
	XtSetArg(uiArgs[i], XmNshadowThickness, 0); i++;
	XtSetValues(smDD.qExit, uiArgs, i);

	i = 0;
	tmpString = GETMESSAGE(18, 10, "Logout Confirmation");
	XtSetArg(uiArgs[i], XmNtitle, tmpString); i++;
	XtSetArg(uiArgs[i], XmNmwmFunctions, 0);i++;
	XtSetArg(uiArgs[i], XmNmwmDecorations,
		 (MWM_DECOR_TITLE | MWM_DECOR_BORDER));i++;
        XtSetArg(uiArgs[i], XmNmwmInputMode, MWM_INPUT_SYSTEM_MODAL);i++;
	XtSetArg(uiArgs[i], XmNuseAsyncGeometry, True);i++;
	XtSetValues(XtParent(smDD.qExit), uiArgs, i);
	
	XtAddCallback (XtParent(smDD.qExit), XmNpopupCallback, DialogUp, NULL);

	i = 0;
	XtSetArg(uiArgs[i], XmNrightAttachment, XmATTACH_FORM); i++;
	XtSetArg(uiArgs[i], XmNleftAttachment, XmATTACH_FORM); i++;
	XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_FORM); i++;
	XtSetArg(uiArgs[i], XmNmessageString, queryString); i++;
	XtSetArg(uiArgs[i], XmNmessageAlignment, XmALIGNMENT_CENTER); i++;
	XtSetArg(uiArgs[i], XmNdialogType, XmDIALOG_WARNING); i++;
	XtSetArg(uiArgs[i], XmNshadowThickness, 0); i++;
	queryMB = XmCreateMessageBox(smDD.qExit, "exitMB", uiArgs, i);
	XtUnmanageChild(XmMessageBoxGetChild(queryMB, XmDIALOG_OK_BUTTON));
	XtUnmanageChild(XmMessageBoxGetChild(queryMB, XmDIALOG_CANCEL_BUTTON));
	XtUnmanageChild(XmMessageBoxGetChild(queryMB, XmDIALOG_HELP_BUTTON));
	XtManageChild(queryMB);

	i = 0;
	XtSetArg(uiArgs[i], XmNlabelString, restartString); i++;
	XtSetArg(uiArgs[i], XmNleftAttachment, XmATTACH_POSITION); i++;
	XtSetArg(uiArgs[i], XmNleftPosition, 0); i++;
	XtSetArg(uiArgs[i], XmNleftOffset, 10); i++;
	XtSetArg(uiArgs[i], XmNrightAttachment, XmATTACH_POSITION); i++;
	XtSetArg(uiArgs[i], XmNrightPosition, 25); i++;
	XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_WIDGET); i++;
	XtSetArg(uiArgs[i], XmNtopWidget, queryMB); i++;
	XtSetArg(uiArgs[i], XmNbottomAttachment, XmATTACH_FORM); i++;
	XtSetArg(uiArgs[i], XmNtopOffset, 10); i++;
	XtSetArg(uiArgs[i], XmNbottomOffset, 10); i++;
	XtSetArg(uiArgs[i], XmNmarginTop, 5); i++;
	XtSetArg(uiArgs[i], XmNmarginBottom, 5); i++;
        restartButton = XmCreatePushButton(smDD.qExit, "restartButton",
					   uiArgs, i);
	XtManageChild(restartButton);
	
	i = 0;
	XtSetArg(uiArgs[i], XmNlabelString, resetString); i++;
	XtSetArg(uiArgs[i], XmNleftAttachment, XmATTACH_POSITION); i++;
	XtSetArg(uiArgs[i], XmNleftPosition, 25); i++;
	XtSetArg(uiArgs[i], XmNleftOffset, 5); i++;
	XtSetArg(uiArgs[i], XmNrightAttachment, XmATTACH_POSITION); i++;
	XtSetArg(uiArgs[i], XmNrightPosition, 50); i++;
	XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET); i++;
	XtSetArg(uiArgs[i], XmNtopWidget, restartButton); i++;
	XtSetArg(uiArgs[i], XmNbottomAttachment,
		 XmATTACH_OPPOSITE_WIDGET); i++;
	XtSetArg(uiArgs[i], XmNbottomWidget, restartButton); i++;
        resetButton = XmCreatePushButton(smDD.qExit, "resetButton",
					   uiArgs, i);
	XtManageChild(resetButton);

	i = 0;
	XtSetArg(uiArgs[i], XmNlabelString, smDD.cancelLogoutString); i++;
	XtSetArg(uiArgs[i], XmNleftAttachment, XmATTACH_POSITION); i++;
	XtSetArg(uiArgs[i], XmNleftPosition, 50); i++;
	XtSetArg(uiArgs[i], XmNleftOffset, 5); i++;
	XtSetArg(uiArgs[i], XmNrightAttachment, XmATTACH_POSITION); i++;
	XtSetArg(uiArgs[i], XmNrightPosition, 75); i++;
	XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET); i++;
	XtSetArg(uiArgs[i], XmNtopWidget, restartButton); i++;
	XtSetArg(uiArgs[i], XmNbottomAttachment,
		 XmATTACH_OPPOSITE_WIDGET); i++;
	XtSetArg(uiArgs[i], XmNbottomWidget, restartButton); i++;
        newCancel = XmCreatePushButton(smDD.qExit, "newCancel",
				       uiArgs, i);
	XtManageChild(newCancel);

	i = 0;
	XtSetArg(uiArgs[i], XmNlabelString, smDD.helpString); i++;
	XtSetArg(uiArgs[i], XmNleftAttachment, XmATTACH_POSITION); i++;
	XtSetArg(uiArgs[i], XmNleftPosition, 75); i++;
	XtSetArg(uiArgs[i], XmNleftOffset, 5); i++;
	XtSetArg(uiArgs[i], XmNrightAttachment, XmATTACH_POSITION); i++;
	XtSetArg(uiArgs[i], XmNrightPosition, 100); i++;
	XtSetArg(uiArgs[i], XmNrightOffset, 10); i++;
	XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET); i++;
	XtSetArg(uiArgs[i], XmNtopWidget, restartButton); i++;
	XtSetArg(uiArgs[i], XmNbottomAttachment,
		 XmATTACH_OPPOSITE_WIDGET); i++;
	XtSetArg(uiArgs[i], XmNbottomWidget, restartButton); i++;
        newHelp = XmCreatePushButton(smDD.qExit, "newHelp",
				     uiArgs, i);
	i = 0;
	XtSetArg(uiArgs[i], XmNdefaultButton, restartButton);i++;
	XtSetArg(uiArgs[i], XmNcancelButton, newCancel);i++;
	XtSetValues(smDD.qExit, uiArgs, i);
	
	XtManageChild(newHelp);

	exitRec = (ExitRecord *)XtMalloc( sizeof(ExitRecord) );
	exitRec->pmsg = pmsg;
	exitRec->u.queryExitConfirmedMode = DtSM_CURRENT_STATE;
	XtAddCallback (restartButton, XmNactivateCallback,
		       	QueryExitConfirmed, exitRec);

        /* Dialog will self accept in 2 minutes */
        exitTimer = XtAppAddTimeOut(smGD.appCon, 120000, 
				    QueryExitTimeConfirmed, exitRec);

	exitRec = (ExitRecord *)XtMalloc( sizeof(ExitRecord) );
        exitRec->pmsg = pmsg;
        exitRec->u.queryExitConfirmedMode = DtSM_HOME_STATE;
	XtAddCallback (resetButton, XmNactivateCallback,
		       	QueryExitConfirmed, exitRec);
	exitRec = (ExitRecord *)XtMalloc( sizeof(ExitRecord) );
        exitRec->pmsg = pmsg;
	exitRec->u.exitCancelledDialog = smDD.qExit;
	XtAddCallback (newCancel, XmNactivateCallback,
		       	ExitCancelled, exitRec);

#ifndef NO_XVH
	XtAddCallback (newHelp, XmNactivateCallback,
		       TopicHelpRequested, HELP_LOGOUT_QUERY_STR);
	XtAddCallback (smDD.qExit, XmNhelpCallback,
		       TopicHelpRequested, HELP_LOGOUT_QUERY_STR);
#endif
	XmStringFree(restartString);
	XmStringFree(resetString);
	XmStringFree(queryString);
    }

   /*
    * Refresh buffer containing pointer to Tt_message.
    */
    i=0;
    XtSetArg(uiArgs[i], XmNuserData, &pmsg); i++;
    XtGetValues(smDD.qExit, uiArgs, i);
    *pmsg = msg;

    /*
     * Pop up the dialog
     */
    XtManageChild(smDD.qExit);
    return(0);
}



/*************************************<->*************************************
 *
 *  WarnMsgFailue ()
 *
 *
 *  Description:
 *  -----------
 *  Let the user know that the bms has died and that the current session
 *  will not be saved.
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
int 
#ifdef _NO_PROTO
WarnMsgFailure()
#else
WarnMsgFailure( void )
#endif /* _NO_PROTO */
{
    int 	i;
    XmString 	bmsDeadString;
    String	tmpString;

    if(smDD.deadWid == NULL)
    {
	bmsDeadString = XmStringCreateLtoR(((char *)GETMESSAGE(18, 38, 
		"Messaging System Inoperative\n\n\
To restart:\n\n1) Save all open data files.\n\
2) Logout.\n\
3) Log in again.\n\nNote: The current session will not be saved.\n\n\
When you are ready to begin the restart process, click [OK] and\n\
proceed to save your files.")),
		XmFONTLIST_DEFAULT_TAG);


	/*
	 * Now create the dialog box
	 */
	i = 0;
	tmpString = GETMESSAGE(18, 12, "_DtMessage Failure");
	XtSetArg(uiArgs[i], XmNmessageString, bmsDeadString);i++;
	XtSetArg (uiArgs[i], XmNallowShellResize, True);  i++;
	XtSetArg(uiArgs[i], XmNdialogStyle, XmDIALOG_SYSTEM_MODAL); i++;
	XtSetArg(uiArgs[i], XmNmessageAlignment, XmALIGNMENT_BEGINNING); i++;
	XtSetArg(uiArgs[i], XmNtitle, tmpString); i++;
	XtSetArg(uiArgs[i], XmNokLabelString, smDD.okString); i++;
	XtSetArg(uiArgs[i], XmNhelpLabelString, smDD.helpString); i++;
	XtSetArg(uiArgs[i], XmNautoUnmanage, False); i++;
	smDD.deadWid = XmCreateWarningDialog(smGD.topLevelWid, "deadDialog",
					     uiArgs, i);

	i = 0;
	XtSetArg(uiArgs[i], XmNuseAsyncGeometry, True);i++;
	XtSetArg(uiArgs[i], XmNmwmFunctions, 0);i++;
	XtSetArg(uiArgs[i], XmNmwmDecorations,
		 (MWM_DECOR_TITLE | MWM_DECOR_BORDER));i++;
	XtSetArg(uiArgs[i], XmNmwmInputMode, MWM_INPUT_SYSTEM_MODAL);i++;
	XtSetValues(XtParent(smDD.deadWid), uiArgs, i);
	
	XtAddCallback (XtParent(smDD.deadWid),
		       XmNpopupCallback, DialogUp, NULL);
	XtUnmanageChild(XmMessageBoxGetChild(smDD.deadWid,
					     XmDIALOG_CANCEL_BUTTON));

	/*
	 * Now add in the callback and get out of here
	 */
	XtAddCallback (smDD.deadWid, XmNokCallback,
		       SimpleOK, (XtPointer) smDD.deadWid);
#ifndef NO_XVH
	XtAddCallback (smDD.deadWid, XmNhelpCallback,
		       TopicHelpRequested, HELP_BMS_DEAD_STR);
#endif
	XtAddCallback (smDD.deadWid, XmNcancelCallback,
		       	NULL, NULL);
	XmStringFree(bmsDeadString);
    }

    XtManageChild(smDD.deadWid);
    return(0);
}


/*************************************<->*************************************
 *
 *  CreateLockDialogWithCover()
 *
 *
 *  Description:
 *  -----------
 *  Create the lock dialog when it exists as a part of a cover
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
Widget 
#ifdef _NO_PROTO
CreateLockDialogWithCover( parent )
        Widget parent ;
#else
CreateLockDialogWithCover(
        Widget parent )
#endif /* _NO_PROTO */
{
    int 	i;
    Widget	loginLabel, instructLabel, tmpLock;
    Widget	indFrame, loginPic, picFrame, loginFrame;
    Widget  	passwdLabel, passwdForm;
    Dimension	width;		/* width, height of drop shadow    */
    XmString	lockString, passwordString;
    char	lockMessage[100+L_cuserid];
    Pixel   fg, bg, focus_color;         /* foreground, background colors   */
    char*	puser;
    struct passwd* pwd;

    i = 0;
    XtSetArg(uiArgs[i], XmNmarginWidth, 0); i++;
    XtSetArg(uiArgs[i], XmNmarginHeight, 0); i++;
    XtSetArg(uiArgs[i], XmNshadowType, XmSHADOW_OUT); i++;
    XtSetArg(uiArgs[i], XmNshadowThickness,5); i++;
    XtSetArg(uiArgs[i], XmNunitType, XmPIXELS); i++;
    XtSetArg(uiArgs[i], XmNresizePolicy, XmRESIZE_NONE);i++;
    XtSetArg(uiArgs[i], XmNmappedWhenManaged, False); i++;
    tmpLock = XmCreateForm(parent, "lockDialog", uiArgs, i);
    smDD.matte[1] = tmpLock;

    i = 0;
    XtSetArg(uiArgs[i], XmNshadowType, XmSHADOW_OUT); i++;
    XtSetArg(uiArgs[i], XmNshadowThickness, 2); i++;
    XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_FORM); i++;
    XtSetArg(uiArgs[i], XmNtopOffset, 15); i++;
    XtSetArg(uiArgs[i], XmNbottomAttachment, XmATTACH_FORM); i++;
    XtSetArg(uiArgs[i], XmNbottomOffset, 15); i++;
    XtSetArg(uiArgs[i], XmNrightAttachment, XmATTACH_FORM); i++;
    XtSetArg(uiArgs[i], XmNrightOffset, 15); i++;
    picFrame = XmCreateFrame(tmpLock, "picFrame", uiArgs, i);

    i = 0;
    XtSetArg(uiArgs[i], XmNforeground, &fg); i++;
    XtSetArg(uiArgs[i], XmNbackground, &bg); i++;
    XtGetValues(tmpLock, uiArgs, i);
     
    i = 0;
    XtSetArg(uiArgs[i], XmNfillMode, XmFILL_SELF); i++;
    XtSetArg(uiArgs[i], XmNbehavior, XmICON_LABEL); i++;
    XtSetArg(uiArgs[i], XmNpixmapForeground, fg); i++;
    XtSetArg(uiArgs[i], XmNpixmapBackground, bg); i++;
    XtSetArg(uiArgs[i], XmNstring, NULL); i++;
    XtSetArg(uiArgs[i], XmNshadowThickness, 0); i++;
    XtSetArg(uiArgs[i], XmNtraversalOn, False); i++;
    loginPic = _DtCreateIcon(picFrame, "lockLabelPixmap", uiArgs, i);
     
    i = 0;
    XtSetArg(uiArgs[i], XmNshadowType, XmSHADOW_OUT); i++;
    XtSetArg(uiArgs[i], XmNshadowThickness, 2); i++; 
    XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_FORM); i++;
    XtSetArg(uiArgs[i], XmNtopOffset, 15); i++;
    XtSetArg(uiArgs[i], XmNbottomAttachment, XmATTACH_FORM); i++;
    XtSetArg(uiArgs[i], XmNbottomOffset, 15); i++;
    XtSetArg(uiArgs[i], XmNleftAttachment, XmATTACH_FORM); i++;
    XtSetArg(uiArgs[i], XmNleftOffset, 15); i++;
    XtSetArg(uiArgs[i], XmNrightAttachment, XmATTACH_WIDGET); i++;
    XtSetArg(uiArgs[i], XmNrightWidget, picFrame); i++;
    XtSetArg(uiArgs[i], XmNrightOffset, 0); i++;
    loginFrame = XmCreateFrame(tmpLock, "loginFrame", uiArgs, i);

    /* 
     * create the login matte...
     */
    i = 0;
    XtSetArg(uiArgs[i], XmNresizePolicy, XmRESIZE_NONE);i++;
    smDD.loginMatte[1] = XmCreateForm(loginFrame, "loginMatte", uiArgs, i);

    /*
     *  create the login/password forms
     */
    i = 0;
    XtSetArg(uiArgs[i], XmNshadowThickness, 0); i++; 
    XtSetArg(uiArgs[i], XmNrightAttachment, XmATTACH_FORM); i++;
    XtSetArg(uiArgs[i], XmNrightOffset, 15); i++;
    XtSetArg(uiArgs[i], XmNbottomAttachment, XmATTACH_POSITION); i++;
    XtSetArg(uiArgs[i], XmNbottomPosition, 50); i++;
    XtSetArg(uiArgs[i], XmNleftAttachment, XmATTACH_FORM); i++;
    XtSetArg(uiArgs[i], XmNleftOffset, 15); i++;
    smDD.loginForm[1] = XmCreateForm(smDD.loginMatte[1], "loginForm", uiArgs, i);

    i = 0;
    XtSetArg(uiArgs[i], XmNresizePolicy, XmRESIZE_NONE);i++;
    XtSetArg(uiArgs[i], XmNshadowThickness, 0); i++; 
    XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_POSITION); i++;
    XtSetArg(uiArgs[i], XmNtopPosition, 50); i++;
	XtSetArg(uiArgs[i], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); i++;
	XtSetArg(uiArgs[i], XmNleftWidget, smDD.loginForm[1]); i++;
	XtSetArg(uiArgs[i], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET); i++;
	XtSetArg(uiArgs[i], XmNrightWidget, smDD.loginForm[1]); i++;
    passwdForm = XmCreateForm(smDD.loginMatte[1], "passwdForm", uiArgs, i);


    /*
     *  create the login/password labels...
     */
    i = 0;
    puser = getenv("USER");
    if (!puser) {
	pwd = getpwuid(getuid());
	if (pwd) {
	   puser = pwd->pw_name;
	}
    }
    if (!puser) puser = "";
    sprintf(lockMessage, ((char *)GETMESSAGE(18, 1, "Display locked by user %s.")), puser);
    lockString = XmStringCreateLtoR(lockMessage, XmFONTLIST_DEFAULT_TAG);
    XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_POSITION); i++;
    XtSetArg(uiArgs[i], XmNtopPosition, 20); i++;
    XtSetArg(uiArgs[i], XmNalignment, XmALIGNMENT_CENTER); i++;
    XtSetArg(uiArgs[i], XmNlabelString,	lockString); i++;
    loginLabel = XmCreateLabelGadget(smDD.loginForm[1],
				     "loginLabel", uiArgs, i);
    XtManageChild(loginLabel);
    XmStringFree(lockString);

    i = 0;
    lockString = XmStringCreateLtoR(((char *)GETMESSAGE(18, 2, "Enter password to unlock.")),
				    XmFONTLIST_DEFAULT_TAG);
    XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_WIDGET); i++;
    XtSetArg(uiArgs[i], XmNtopWidget, loginLabel); i++;
    XtSetArg(uiArgs[i], XmNalignment, XmALIGNMENT_CENTER); i++;
    XtSetArg(uiArgs[i], XmNlabelString,	lockString); i++;
    instructLabel = XmCreateLabelGadget(smDD.loginForm[1], "instructLabel",
					uiArgs, i);
    XtManageChild(instructLabel);
    XmStringFree(lockString);

    i = 0;
    passwordString = XmStringCreateLtoR(((char *)GETMESSAGE(18, 3, "Password: ")),
					XmFONTLIST_DEFAULT_TAG);
    XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_POSITION); i++;
    XtSetArg(uiArgs[i], XmNtopPosition, 20); i++;
    XtSetArg(uiArgs[i], XmNbottomAttachment, XmATTACH_POSITION); i++;
    XtSetArg(uiArgs[i], XmNbottomPosition, 80); i++;
    XtSetArg(uiArgs[i], XmNleftOffset, 0); i++;
    XtSetArg(uiArgs[i], XmNalignment, XmALIGNMENT_END); i++;
    XtSetArg(uiArgs[i], XmNlabelString,	passwordString); i++;
    passwdLabel = XmCreateLabelGadget(passwdForm,
				      "passwdLabel", uiArgs, i);
    XtManageChild(passwdLabel);
    XmStringFree(passwordString);

    /*
     * Give the password label an offset
     */
    i = 0;
    XtSetArg(uiArgs[i], XmNwidth, &width);i++;
    XtGetValues(passwdLabel, uiArgs, i);

    i = 0;
    width += (width/6);
    XtSetArg(uiArgs[i], XmNwidth, width);i++;
    XtSetArg(uiArgs[i], XmNrecomputeSize, False);i++;
    XtSetValues(passwdLabel, uiArgs, i);


    i = 0;
    XtSetArg(uiArgs[i], XmNshadowType, XmSHADOW_IN); i++;
    XtSetArg(uiArgs[i], XmNshadowThickness, 1); i++;
    XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_POSITION); i++;
    XtSetArg(uiArgs[i], XmNtopPosition, 20); i++;
    XtSetArg(uiArgs[i], XmNbottomAttachment, XmATTACH_POSITION); i++;
    XtSetArg(uiArgs[i], XmNbottomPosition, 80); i++;
    XtSetArg(uiArgs[i], XmNleftAttachment, XmATTACH_WIDGET); i++;
    XtSetArg(uiArgs[i], XmNleftWidget, passwdLabel); i++;
    XtSetArg(uiArgs[i], XmNrightAttachment, XmATTACH_FORM); i++;
    XtSetArg(uiArgs[i], XmNrightOffset, 10); i++;
    XtSetArg(uiArgs[i], XmNalignment, XmALIGNMENT_END); i++;
    indFrame = XmCreateFrame(passwdForm,  "indFrame", uiArgs, i);

    i = 0;
    passwordString = XmStringCreateLtoR("|",
					XmFONTLIST_DEFAULT_TAG);
    XtSetArg(uiArgs[i], XmNlabelString,	passwordString); i++;
    XtSetArg(uiArgs[i], XmNalignment, XmALIGNMENT_BEGINNING); i++;
    smDD.indLabel[1] = XmCreateLabel(indFrame, "indLabel",
					uiArgs, i);
    XtManageChild(smDD.indLabel[1]);
    XmStringFree(passwordString);
    
    i = 0;
    XtSetArg(uiArgs[i], XmNhighlightColor,  &focus_color); i++;
    XtGetValues(indFrame, uiArgs, i);
    XtVaSetValues ( smDD.indLabel[1],
                    XmNborderWidth, 2,
                    XmNborderColor,  focus_color,
                    NULL );
    /*
     * Manage forms AFTER all children have been managed
     */
    XtManageChild(indFrame);
    XtManageChild(passwdForm);
    XtManageChild(smDD.loginForm[1]);
    XtManageChild(smDD.loginMatte[1]);
    XtManageChild(loginPic);
    XtManageChild(picFrame);
    XtManageChild(loginFrame);
	
    return(tmpLock);
}



/*************************************<->*************************************
 *
 *  CreateCoverDialog ()
 *
 *
 *  Description:
 *  -----------
 *  Create the cover dialogs for all the screens
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
Widget 
#ifdef _NO_PROTO
CreateCoverDialog( screenNum, withLock )
        int screenNum ;
        Boolean withLock ;
#else
CreateCoverDialog(
        int screenNum,
        Boolean withLock )
#endif /* _NO_PROTO */
{
    int i;
    Widget	tmpCover, table;
    char	geomString[50];

    sprintf(geomString, "%dx%d+0+0",
	    DisplayWidth(smGD.display, screenNum),
	    DisplayHeight(smGD.display, screenNum));
	    

    i = 0;
    XtSetArg(uiArgs[i], XmNmwmDecorations, 0);i++;
    XtSetArg(uiArgs[i], XmNgeometry, (String) geomString);i++;
    XtSetArg(uiArgs[i], XmNuseAsyncGeometry, True);i++;
    XtSetArg(uiArgs[i], XmNallowShellResize, True); i++;
    XtSetArg(uiArgs[i], XmNresizePolicy, XmRESIZE_NONE);i++;

    /* set this resource only in a single-head situation */
    if ((smGD.numSavedScreens - 1) == 0)
        XtSetArg(uiArgs[i], XmNmwmInputMode, MWM_INPUT_SYSTEM_MODAL);i++;

    XtSetArg(uiArgs[i], XmNdepth, DefaultDepth(smGD.display, screenNum));i++;
    XtSetArg(uiArgs[i], XmNscreen,
	     ScreenOfDisplay(smGD.display, screenNum));i++;
    XtSetArg(uiArgs[i], XmNcolormap,
	     DefaultColormap(smGD.display, screenNum));i++;
    tmpCover = XtCreatePopupShell("coverDialog", topLevelShellWidgetClass,
				  smGD.topLevelWid,  uiArgs, i);

    i = 0;
    XtSetArg(uiArgs[i], XmNmarginWidth, 0); i++;
    XtSetArg(uiArgs[i], XmNmarginHeight, 0); i++;
    XtSetArg(uiArgs[i], XmNshadowThickness, 0); i++;
    XtSetArg(uiArgs[i], XmNheight,
             (Dimension) DisplayHeight(smGD.display, smGD.screen)); i++;
    XtSetArg(uiArgs[i], XmNwidth,
             (Dimension) DisplayWidth(smGD.display, smGD.screen)); i++;
    XtSetArg(uiArgs[i], XmNresizePolicy, XmRESIZE_NONE);i++;
    table = XmCreateDrawingArea(tmpCover, "drawArea", uiArgs, i);
    XtManageChild(table);
    smDD.coverDrawing[screenNum] = table;

    if(withLock == True)
    {
	XtAddCallback (tmpCover, XmNpopupCallback,
		       LockDialogUp, NULL);
    }

    XtRealizeWidget(tmpCover);
    
    return(tmpCover);
}


/*************************************<->*************************************
 *
 *  ExitConfirmed ()
 *
 *
 *  Description:
 *  -----------
 *  Callback that is called when user confirms the exit of a session by
 *  pressing the OK button on the confirmation dialog.  This routine just
 *  facilitates the exit process.
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
ExitConfirmed( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
ExitConfirmed(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
    ExitRecord *exitRec = (ExitRecord *)client_data;

    XtUnmanageChild(smDD.confExit);
    ImmediateExit(smSettings.startState, *exitRec->pmsg);
}



/*************************************<->*************************************
 *
 *  QueryExitConfirmed ()
 *
 *
 *  Description:
 *  -----------
 *  Callback when the user confirms exit in Query mode.  This routine
 *  determines the type of exit, and then calls the routine which actually
 *  does the exit work.
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
QueryExitConfirmed( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
QueryExitConfirmed(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
    ExitRecord *exitRec = (ExitRecord *)client_data;
    
    XtUnmanageChild(smDD.qExit);

    ImmediateExit(exitRec->u.queryExitConfirmedMode, *exitRec->pmsg);
}



/*************************************<->*************************************
 *
 *  ImmediateExit (mode)
 *
 *
 *  Description:
 *  -----------
 *  This process puts in motion the exit procedure, and then exits.
 *
 *
 *  Inputs:
 *  ------
 *  mode = Whether this session should be reset or restarted
 *  msg -- if non-zero, Session_Exit request to reply/fail && destroy
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
ImmediateExit( mode, msg )
        int mode ;
	Tt_message msg ;
#else
ImmediateExit(
        int mode,
	Tt_message msg )
#endif /* _NO_PROTO */
{

    long old;
    Tt_message notice;

    /* 
    ** Run the user's exit script if there is one
    */

    if (smGD.compatMode == False) 
    {
        StartEtc(True);    /* run sessionexit */
    }

    /*
     * Turn off SIGTERM so we don't catch one in the middle of shutting
     * down
     */
#if !defined(SVR4) && !defined(sco)
    old = sigblock(sigmask(SIGTERM));
#else
    old = sighold(SIGTERM);
#endif
    /*
     *
     */
    if(smGD.bmsDead == False)
    {
	notice = (Tt_message) tttk_message_create( 0, TT_NOTICE, TT_SESSION, 0,
						  "XSession_Ending", 0);
	tt_message_send( notice );
	tt_message_destroy( notice );
    }
    if (msg != 0) {
	tt_message_reply( msg );
	tt_message_destroy( msg );
    }

    /*
     * Do not save state when simply aborting. 
     */

    if (mode >= 0)
    {
	Tt_message msg;
    	msg = (Tt_message) tttk_message_create( 0, TT_NOTICE, TT_SESSION, 0,
					       "DtActivity_Beginning", 0 );
	tt_message_send( msg );
	tt_message_destroy( msg );

	SaveState (False, mode);

	XSync(smGD.display, 0);
    }

    _DtReleaseLock(smGD.display, SM_RUNNING_LOCK);
    SM_EXIT(0);
}



/*************************************<->*************************************
 *
 *  ExitCancelled ()
 *
 *
 *  Description:
 *  -----------
 *  Called when the user bails out from a logout at the confirmation dialog
 *
 *
 *  Inputs:
 *  ------
 *  client_data - tells which dialog to unmange (the query or confirm exit)
 *  msg -- if non-zero, Session_Exit request to reply/fail && destroy
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
ExitCancelled( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
ExitCancelled(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
    ExitRecord *exitRec = (ExitRecord *)client_data;

    if (exitTimer != (XtIntervalId) 0) 
    {
	XtRemoveTimeOut(exitTimer);
	exitTimer=0;
    }

    if(XtIsManaged(exitRec->u.exitCancelledDialog))
    {
	XtUnmanageChild(exitRec->u.exitCancelledDialog);
    }

    if(smDD.smHelpDialog && XtIsManaged(smDD.smHelpDialog))
    {
	XtUnmanageChild(smDD.smHelpDialog);
    }

    if (*exitRec->pmsg != 0) {
	    tttk_message_fail(*exitRec->pmsg, TT_DESKTOP_ECANCELED, 0, 1 );
    }
    SetSystemReady();
}


/*************************************<->*************************************
 *
 *  ExitTimeConfirmed()
 *
 *
 *  Description:
 *  -----------
 *  Called when the timer expires from a logout at the confirmation dialog.
 *  Dialog will self accept after timer expires.
 *
 *
 *  Inputs:
 *  ------
 *  client_data - tells which dialog to unmange (the query or confirm exit)
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
ExitTimeConfirmed( client_data, id)
        XtPointer client_data;
        XtIntervalId *id;
#else
ExitTimeConfirmed(
        XtPointer client_data,
        XtIntervalId *id)
#endif /* _NO_PROTO */
{
    ExitRecord *exitRec = (ExitRecord *)client_data;

    ExitConfirmed(NULL, client_data, NULL);
}



/*************************************<->*************************************
 *
 *  QueryExitTimeConfirmed()
 *
 *
 *  Description:
 *  -----------
 *  Called when the timer expires from a logout at the confirmation dialog.
 *  Dialog will self accept after timer expires.
 *
 *
 *  Inputs:
 *  ------
 *  client_data - tells which dialog to unmange (the query or confirm exit)
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
QueryExitTimeConfirmed( client_data, id)
        XtPointer client_data;
        XtIntervalId *id;
#else
QueryExitTimeConfirmed(
        XtPointer client_data,
        XtIntervalId *id)
#endif /* _NO_PROTO */
{
    ExitRecord *exitRec = (ExitRecord *)client_data;

    QueryExitConfirmed(NULL, client_data, NULL);
}



/*************************************<->*************************************
 *
 *  DialogUp ()
 *
 *
 *  Description:
 *  -----------
 *  Once the dialog is managed, but not popped up - reposition it so that
 *  it appears in the middle of the screen then remove the popup callback
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *  None
 *
 *
 *  Comments:
 *  --------
 *  This routine can be used for any generic SYSTEM_MODAL dialog
 * 
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
DialogUp( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
DialogUp(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
    int 	i;
    Dimension	width, height;
    Position	x, y;

    /*
     * Get the size of the dialog box - then compute its position
     */
    i = 0;
    XtSetArg(uiArgs[i], XmNwidth, &width);i++;
    XtSetArg(uiArgs[i], XmNheight, &height);i++;
    XtGetValues(w, uiArgs, i);
    
    x = (DisplayWidth(smGD.display, smGD.screen) / 2) - (width / 2);
    y = (DisplayHeight(smGD.display, smGD.screen) / 2) - (height / 2);
    
    i = 0;
    XtSetArg(uiArgs[i], XmNx, x);i++;
    XtSetArg(uiArgs[i], XmNy, y);i++;
    XtSetValues(w, uiArgs, i);

    XtRemoveCallback(w, XmNpopupCallback, DialogUp, NULL);
}




/*************************************<->*************************************
 *
 *  ShowWaitState (flag)
 *
 *
 *  Description:
 *  -----------
 *  Enter/Leave the wait state.
 *
 *
 *  Inputs:
 *  ------
 *  flag = TRUE for Enter, FALSE for Leave.
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  Stolen from the window manager code.
 * 
 *************************************<->***********************************/

void 
#ifdef _NO_PROTO
ShowWaitState( flag )
        Boolean flag ;
#else
ShowWaitState(
        Boolean flag )
#endif /* _NO_PROTO */
{
    if (flag)
    {
	XGrabPointer (smGD.display, DefaultRootWindow(smGD.display), FALSE, 
		      0, GrabModeAsync, GrabModeAsync, None, 
		      smGD.waitCursor, CurrentTime);
	XGrabKeyboard (smGD.display, DefaultRootWindow(smGD.display), FALSE, 
		       GrabModeAsync, GrabModeAsync, CurrentTime);
    }
    else
    {
	XUngrabPointer (smGD.display, CurrentTime);
	XUngrabKeyboard (smGD.display, CurrentTime);
    }
    XSync(smGD.display, 0);
}


/*************************************<->*************************************
 *
 *  InitCursorInfo ()
 *
 *
 *  Description:
 *  -----------
 *  This function determines whether a server supports large cursors.  It it
 *  does large feedback cursors are used in some cases (wait state and
 *  system modal state); otherwise smaller (16x16) standard cursors are used.
 *
 *  Outputs:
 *  -------
 *  Returns true if large cursors are supported, false otherwise
 *
 *  Comments:
 *  ---------
 *  This code was stolen from the window manager
 * 
 *************************************<->***********************************/
Boolean 
#ifdef _NO_PROTO
InitCursorInfo()
#else
InitCursorInfo( void )
#endif /* _NO_PROTO */
{
    unsigned int cWidth;
    unsigned int cHeight;

    if (XQueryBestCursor (smGD.display, DefaultRootWindow(smGD.display),
			  32, 32, &cWidth, &cHeight))
    {
	if ((cWidth >= 32) && (cHeight >= 32))
	{
	   return(True);
	}
    }

    return(False);
}



/*************************************<->*************************************
 *
 *  LockDialogUp ()
 *
 *
 *  Description:
 *  -----------
 *  Once the lock dialog is managed, but not popped up - reposition it so that
 *  it appears in the middle of the screen then remove the popup callback
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
LockDialogUp( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
LockDialogUp(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
    
    register int	i;
    Dimension	width, height;	/* size values returned by XtGetValues	   */
    Dimension	shadowThickness;/* size values returned by XtGetValues	   */
    
    struct
    {			/* position, size of widgets (pixels)	   */
	int x, y;
	int	width;
	int height;
	int shadow;
    } mw;	/* matte, logo, drop shadow & login matte  */

    int		width1, width2; /* general width variable		   */
    int		x1, y1;		/* general position variables		   */
    int		index;
    
    /*
     * The partial cover has widgets of index 0 - the cover has
     * index 1
     */
    if(smGD.coverScreen == True)
    {
	index = 1;
    }
    else
    {
	index = 0;
    }
    
    /*
     *  - center the main matte horizontally and vertically...
     */
    i = 0;
    XtSetArg(uiArgs[i], XmNwidth, &width); i++;
    XtSetArg(uiArgs[i], XmNheight, &height); i++;
    XtSetArg(uiArgs[i], XmNshadowThickness, &shadowThickness); i++;
    XtGetValues(smDD.matte[index], uiArgs, i);

    mw.shadow = shadowThickness;
    mw.width  = width;
    mw.height = height;
    mw.x      = (DisplayWidth(smGD.display, smGD.screen)  - mw.width)/2;
    mw.y      = (DisplayHeight(smGD.display, smGD.screen) - mw.height)/2;

    if ( mw.x < 0 ) mw.x = 0;
    if ( mw.y < 0 ) mw.y = 0;
    
    x1 = mw.x;
    y1 = mw.y;

    i = 0;

    XtSetArg(uiArgs[i], XmNx, x1); i++;
    XtSetArg(uiArgs[i], XmNy, y1); i++;

    XtSetValues(smDD.matte[index], uiArgs, i);

    /*
     *  - center the login/password frames horizontally in the login_matte...
     */
    XtSetArg(uiArgs[0], XmNwidth,  &width);
    XtGetValues(smDD.loginMatte[index], uiArgs, 1);
    width1 = (int)width;    

    XtSetArg(uiArgs[0], XmNwidth,  &width);
    XtGetValues(smDD.loginForm[index], uiArgs, 1);
    width2 = (int)width;
    
    i = 0;
    XtSetArg(uiArgs[i], XmNleftAttachment, XmATTACH_FORM); i++;
    XtSetArg(uiArgs[i], XmNleftOffset, (width1 - width2) / 2); i++;
    XtSetValues(smDD.loginForm[index],  uiArgs, i);
}



/*************************************<->*************************************
 *
 *  SimpleOK()
 *
 *
 *  Description:
 *  -----------
 *  Simply dismiss a dialog.  Does special process for a compatibility mode
 *  logout dialog and when the bms won't start.
 *
 *
 *  Inputs:
 *  ------
 *  client_data - sends in the dialog to be dismissed.
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
SimpleOK( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
SimpleOK(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
    Widget	dismissDialog = (Widget) client_data;

    if(XtIsManaged(dismissDialog))
    {
	XtUnmanageChild(dismissDialog);
    }

    if(dismissDialog == smDD.noStart)
    {
	SM_EXIT(-1);
    }
}



/*************************************<->*************************************
 *
 *  UpdatePasswdField ()
 *
 *
 *  Description:
 *  -----------
 *  Give the visual feedback neccessary when the user is entering a password
 *
 *
 *  Inputs:
 *  ------
 *  numChars = number of characters entered into the field
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
UpdatePasswdField( numChars )
        int numChars ;
#else
UpdatePasswdField(
        int numChars )
#endif /* _NO_PROTO */
{
    int i, index;
    char passwdMessage[25];
    XmString tmpString;


    if(numChars > 0)
    {
	strcpy(passwdMessage, "|");
	for(i = 1;i < numChars;i++)
	{
	    if(i==1)
	    	strcpy(passwdMessage, "|");
	    else
	    	strcat(passwdMessage, " ");
	}
	strcat(passwdMessage, PASSWORD_INDICATOR);
    }
    else
    {
	strcpy(passwdMessage, "|");
    }

    tmpString = XmStringCreateLtoR (passwdMessage, XmFONTLIST_DEFAULT_TAG);

    /*
     * Set the index for the indLabel widget
     */
    if(smGD.coverScreen == True)
    {
	index = 1;
    }
    else
    {
	index = 0;
    }
    
    i = 0;
    XtSetArg(uiArgs[i], XmNlabelString,	tmpString); i++;
    XtSetValues(smDD.indLabel[index], uiArgs, i);

    XmStringFree(tmpString);
}



/*************************************<->*************************************
 *
 *  WarnNoStartup ()
 *
 *
 *  Description:
 *  -----------
 *  When the BMS refuses to be started, warn the user about why dt is
 *  crashing and then exit.
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
int 
#ifdef _NO_PROTO
WarnNoStartup()
#else
WarnNoStartup( void )
#endif /* _NO_PROTO */
{
    int 	i;
    XmString 	bmsNoStartString;
    String	tmpString;

#ifdef SUN_OS
    bmsNoStartString = XmStringCreateLtoR(((char *)GETMESSAGE(18, 36,
        "The DT messaging system could not be started.\n\n\
To correct the problem:\n\n\
1.  Choose [OK] to return to the login screen.\n\n\
2.  Select Failsafe Session from the login screen's option\n\
	 menu and log in.\n\n\
3.  Check to see that the hostname is correct in these locations:\n\
	 /etc/src.sh\n\
     /etc/hosts\n\
     /usr/adm/inetd.sec\n\n\
For additional information, see the DT User's Guide.")),
                                           XmFONTLIST_DEFAULT_TAG);
#endif
#ifdef __hpux
    bmsNoStartString = XmStringCreateLtoR(((char *)GETMESSAGE(18, 36, 
        "The DT messaging system could not be started.\n\n\
To correct the problem:\n\n\
1.  Choose [OK] to return to the login screen.\n\n\
2.  Select Failsafe Session from the login screen's option\n\
	 menu and log in.\n\n\
3.  Check to see that the hostname is correct in these locations:\n\
	 /etc/src.sh\n\
     /etc/hosts\n\
     /usr/adm/inetd.sec\n\n\
For additional information, see the DT User's Guide.")),
                                           XmFONTLIST_DEFAULT_TAG);
#endif
#ifdef _AIX
    bmsNoStartString = XmStringCreateLtoR(((char *)GETMESSAGE(18, 36, 
        "The DT messaging system could not be started.\n\n\
To correct the problem:\n\n\
1.  Choose [OK] to return to the login screen.\n\n\
2.  Select Failsafe Session from the login screen's option\n\
	 menu and log in.\n\n\
3.  Check to see that the hostname is correct in these locations:\n\
	 /etc/src.sh\n\
     /etc/hosts\n\
     /usr/adm/inetd.sec\n\n\
For additional information, see the DT User's Guide.")),
                                           XmFONTLIST_DEFAULT_TAG);
#endif
#ifdef __osf__
    bmsNoStartString = XmStringCreateLtoR(((char *)GETMESSAGE(18, 36,
        "The DT messaging system could not be started.\n\n\
To correct the problem:\n\n\
1.  Choose [OK] to return to the login screen.\n\n\
2.  Select Failsafe Session from the login screen's option\n\
         menu and log in.\n\n\
3.  Check to see that your hostname exists correctly in /etc/hosts if your\n\
     network has already been configured.\n\
4.  If your network has not yet been configured, make sure that /etc/hosts\n\
     has the following entry in it:\n\
     127.0.0.1 localhost \n\
For additional information, see the CDE User's Guide.")),
                                           XmFONTLIST_DEFAULT_TAG);
#endif

    /*
     * Now create the dialog box
     */
    i = 0;
    tmpString = GETMESSAGE(18, 37, "Action Required");
    XtSetArg(uiArgs[i], XmNmessageString, bmsNoStartString);i++;
    XtSetArg (uiArgs[i], XmNallowShellResize, True);  i++;
    XtSetArg(uiArgs[i], XmNdialogStyle, XmDIALOG_SYSTEM_MODAL); i++;
    XtSetArg(uiArgs[i], XmNmessageAlignment, XmALIGNMENT_BEGINNING); i++;
    XtSetArg(uiArgs[i], XmNtitle, tmpString); i++;
    XtSetArg(uiArgs[i], XmNokLabelString, smDD.okString); i++;
	XtSetArg(uiArgs[i], XmNautoUnmanage, False); i++;
    smDD.noStart = XmCreateWarningDialog(smGD.topLevelWid, "noStartDialog",
					     uiArgs, i);

    XtSetArg(uiArgs[i], XmNuseAsyncGeometry, True);i++;
    XtSetValues(XtParent(smDD.noStart), uiArgs, i);

    XtAddCallback (XtParent(smDD.noStart),
		   XmNpopupCallback, DialogUp, NULL);
    XtUnmanageChild(XmMessageBoxGetChild(smDD.noStart,
					 XmDIALOG_HELP_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(smDD.noStart,
					 XmDIALOG_CANCEL_BUTTON));

    /*
     * Now add in the callback and get out of here
     */
    XtAddCallback (smDD.noStart, XmNokCallback,
		   SimpleOK, smDD.noStart);
    XmStringFree(bmsNoStartString);

    XtManageChild(smDD.noStart);

    return(0);
} /* END OF FUNCTION WarnNoStartup */


#ifdef __osf__

/*************************************<->*************************************
 *
 *  WarnNewProfile()
 *
 *
 *  Description:
 *  -----------
 *  Warn the user that a new .dtprofile has just been added to their $HOME
 *  directory, indicating a need to edit it and their .login/.profile files.
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
int 
#ifdef _NO_PROTO
WarnNewProfile()
#else
WarnNewProfile( void )
#endif /* _NO_PROTO */
{
    int       i;
    XmString  newProfileString;
    String    tmpString;

    newProfileString = XmStringCreateLtoR(((char *)GETMESSAGE(18, 99,
   "The new file '.dtprofile' has been added to your home directory.\n\
\n\
   Follow the instructions in this file to ensure that when you log in\n\
   again your '.login' or '.profile' file will be activated and \n\
   that it will interact correctly with CDE. \n\
\n\
   For additional information, see the CDE Advanced User's and System\n\
   Administrator's Guide.")), XmFONTLIST_DEFAULT_TAG);


    /*
     * Now create the dialog box
     */
    i = 0;
    tmpString = GETMESSAGE(18, 37, "Action Required");
    XtSetArg(uiArgs[i], XmNmessageString, newProfileString);i++;
    XtSetArg (uiArgs[i], XmNallowShellResize, True);  i++;
    XtSetArg(uiArgs[i], XmNdialogStyle, XmDIALOG_SYSTEM_MODAL); i++;
    XtSetArg(uiArgs[i], XmNmessageAlignment, XmALIGNMENT_BEGINNING); i++;
    XtSetArg(uiArgs[i], XmNtitle, tmpString); i++;
    XtSetArg(uiArgs[i], XmNokLabelString, smDD.okString); i++;
    XtSetArg(uiArgs[i], XmNautoUnmanage, False); i++;
    smDD.newProfile = XmCreateWarningDialog(smGD.topLevelWid, 
					    "newProfileDialog",
					    uiArgs, i);

    XtSetArg(uiArgs[i], XmNuseAsyncGeometry, True);i++;
    XtSetValues(XtParent(smDD.newProfile), uiArgs, i);

    XtAddCallback (XtParent(smDD.newProfile),
		   XmNpopupCallback, DialogUp, NULL);
    XtUnmanageChild(XmMessageBoxGetChild(smDD.newProfile,
					 XmDIALOG_HELP_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(smDD.newProfile,
					 XmDIALOG_CANCEL_BUTTON));

    /*
     * Now add in the callback and get out of here
     */
    XtAddCallback (smDD.newProfile, XmNokCallback,
		   SimpleOK, smDD.newProfile);
    XmStringFree(newProfileString);

    XtManageChild(smDD.newProfile);

    return(0);
} /* END OF FUNCTION WarnNewProfile */


#endif
