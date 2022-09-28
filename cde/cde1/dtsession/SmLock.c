/* $XConsortium: SmLock.c /main/cde1_maint/5 1995/11/21 16:38:53 lehors $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:        SmLock.c
 **
 **  Project:     HP DT Session Manager (dtsession)
 **
 **  Description:
 **  -----------
 **  In charge of locking and unlocking the display in response from
 **  the front panel to so.
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


#include <stdio.h>
#include <errno.h>
#include <pwd.h>
#include <string.h>
#include <fcntl.h>

#ifdef SIA
#include <sia.h>
#endif
 
#ifdef __hpux
#include <X11/XHPlib.h>
#endif /* __hpux */

#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Dt/UserMsg.h>
#include <Dt/Indicator.h>
#include <Dt/SessionM.h>
#include <Dt/SessionP.h>
#include <Tt/tttk.h>
#include <security/pam_appl.h>
#ifdef SVR4
# ifdef USL
#  include <iaf.h>
#  include <sys/types.h>
#  include <ia.h>
# else
#  include <shadow.h>
#endif
#endif
#include "Sm.h"
#include "SmUI.h"
#include "SmError.h"
#include "SmLock.h"
#include "SmScreen.h"

/*
 * Variables global to this module only
 */

/*
 * Global timer value
 */
static XtIntervalId		timerId, lockTimeId, lockDelayId, cycleId, flash_id;

/*
 * Global grab widget
 */
static Widget			grabWidget=NULL;

/*
 * Atom for XA_DT_SM_LOCK_SCREEN property
 */
Atom xa_dt_sm_lock_state;

/*
 * Lock dialog visibility
 */
static Boolean                  lockDlgVisible;
/*
 * PAM lockscreen structures
 */
static char *saved_user_passwd = NULL;
static int lockscrn_conv(int num_msg, struct pam_message **msg,
                      struct pam_response **response, void *appdata_ptr);


#ifdef LOCK_SERVER_ACCESS
/* 
 * Server Access Control Information
 */
static Boolean                  RestrictingAccess = False;
static XHostAddress             *hostList;
static Bool                     hostListActive;
static int                      hostListLen;
#endif

/*
 * Local Functions
 */
#ifdef _NO_PROTO

static void ProcessPopup();
static void FinishLocking() ;
static void RecolorCursor() ;
void EventDetected() ;
static void CheckString() ;
static Boolean CheckPassword() ;
#if defined (_AIX) && defined (_POWER)
static Boolean Authenticate() ;
#else
#define Authenticate(A,B,C) localAuthenticate(A,B,C)
#endif
static Boolean localAuthenticate() ;
static void UnlockDisplay() ;
static void TakeDownLogin() ;
static void PutUpLogin() ;
static void LockAttemptFailed() ;
static void RequirePassword() ;
static void BlinkCaret() ;
static void CycleSaver() ;

#else

static void ProcessPopup( Widget, XtPointer, XEvent *, Boolean *) ;
static void FinishLocking(Widget, XtPointer, XEvent *, Boolean *);
static void RecolorCursor( void ) ;
void EventDetected( Widget, XtPointer, XEvent *, Boolean *) ;
static void CheckString( char *, int ) ;
static Boolean CheckPassword( char * ) ;
#if defined (_AIX) && defined (_POWER)
static Boolean Authenticate( char *, uid_t, char * ) ;
#else
#define Authenticate(A,B,C) localAuthenticate(A,B,C)
#endif
static Boolean localAuthenticate( char *, uid_t, char * ) ;
static void UnlockDisplay( Boolean, Boolean ) ;
static void TakeDownLogin( XtPointer, XtIntervalId * ) ;
static void PutUpLogin( Boolean, Boolean ) ;
static void LockAttemptFailed( XtPointer, XtIntervalId *) ;
static void RequirePassword( XtPointer, XtIntervalId *) ;
static void CycleSaver( XtPointer, XtIntervalId *) ;
static void BlinkCaret( XtPointer, XtIntervalId *) ;

#endif



/*************************************<->*************************************
 *
 *  LockDisplay ()
 *
 *
 *  Description:
 *  -----------
 *  Calls the routines that are in charge of locking the display.
 *
 *
 *  Inputs:
 *  ------
 *  lockNow - request to lock the display immediately
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
/*
 * Place holders for the lock position
 */
static Position visibleY;
static Position hiddenY;

void 
#ifdef _NO_PROTO
LockDisplay(lockNow)
  Boolean lockNow;
#else
LockDisplay( 
  Boolean lockNow) 
#endif /* _NO_PROTO */
{
    register int i;
    int	     screenNum;
    Widget   parent = NULL, lockDlg;
    XColor   xcolors[2];
    struct passwd *pw;
    Boolean  secure;
    int lockDelay;
    int rc;

    timerId = lockTimeId = lockDelayId = cycleId = flash_id = (XtIntervalId)0;

    #if defined (SMGETTIMEOUTS)
/*D*/ getTimeouts(&lockNow, NULL, NULL, NULL, NULL);

    {
      FILE *fp = fopen("/tmp/session", "a");
      if (fp) fprintf(fp, "lockNow=%d saverTimeout=%d "
                          "lockTimeout=%d cycleTimeout=%d"
                          "saverList='%s'\n",
                      lockNow, smSaverRes.saverTimeout,
                      smSaverRes.lockTimeout, smSaverRes.cycleTimeout,
                      smGD.saverList);
      if (fp) fclose(fp);
    }
    #endif

   /*
    * coverScreen 
    *  0 - screen will not be covered, nor will external screen saver run
    *  1 - screen will be covered, external screen saver may be run
    *
    * lockDelay
    * -1 = no password required to unlock display
    *  0 = password always required to unlock display
    *  N = password required to unlock display after N seconds
    */ 
    if (smSaverRes.saverTimeout == 0)
    {
      smGD.coverScreen = 0; 
      lockDelay = 0;        
    }
    else if (lockNow || smSaverRes.lockTimeout > 0)
    {
      smGD.coverScreen = 1; 
      if (lockNow)
        lockDelay = 0; 
      else if (smSaverRes.lockTimeout == 0)
        lockDelay = -1; 
      else if (smSaverRes.lockTimeout <= smSaverRes.saverTimeout)
        lockDelay = 0; 
      else
        lockDelay = smSaverRes.lockTimeout - smSaverRes.saverTimeout;
    }
    else
    {
      smGD.coverScreen = 1;
      lockDelay = -1;
    }

    /*
     * Before anything is done make sure we can unlock if we lock. 
     */
    if (localAuthenticate(NULL, getuid(), NULL) == False)
       {
	Tt_message msg;
	PrintError(DtError, smNLS.trustedSystemErrorString);

	XBell(smGD.display, 100);
	
	/*
	 * Tell the Workspace Manager to quit blinking
	 */
	msg = tttk_message_create( 0, TT_NOTICE, TT_SESSION, 0,
				   "DtActivity_Began", 0 );
	tt_message_send( msg );
	tt_message_destroy( msg );
	SetSystemReady();

	return;
    }
     
    if(((smDD.lockCoverDialog == NULL) && (smGD.coverScreen == True)) ||
       ((smDD.lockDialog == NULL) && (smGD.coverScreen == False)))
    {
	/*
	 * If the user has specified cover - create the cover dialog
	 */
	screenNum = DefaultScreen(smGD.display);
	if(smGD.coverScreen == True)
	{
	    for(i = (smGD.numSavedScreens - 1);i >= 0;i--)
	    {
		if(i == screenNum)
		{
		    smDD.coverDialog[i] = CreateCoverDialog(i, True);
		    parent = smDD.coverDrawing[i];
		}
		else
		{
		    smDD.coverDialog[i] = CreateCoverDialog(i, False);
		}
	    }
	    smDD.lockCoverDialog = CreateLockDialogWithCover(parent);
	}
	
	/*
	 * Create the lock dialog if the user has specified that
	 */
	if((smDD.lockDialog == NULL) && (smGD.coverScreen == False))
	{
	    smDD.lockDialog = CreateLockDialog();

	    /*
	     * Get colors for the password cursor
	     */
	    i = 0;
	    XtSetArg(uiArgs[i], XmNtopShadowColor, &(xcolors[0]));i++;
	    XtSetArg(uiArgs[i], XmNbottomShadowColor, &(xcolors[1]));i++;
	    XtGetValues(smDD.lockDialog, uiArgs, i);

	    smGD.backgroundPix = xcolors[0].pixel;
	    smGD.foregroundPix = xcolors[1].pixel;
	    if (smGD.backgroundPix == smGD.foregroundPix)
	    {
		smGD.backgroundPix = smGD.whitePixel;
		smGD.foregroundPix = smGD.blackPixel;
	    }		
	}
    }

   /*
    * Wait for a visibility event to occur on the window so that
    * we can grab it
    */
    if(smGD.coverScreen == True)
    {
       smGD.lockCursor = smGD.blankCursor;
       grabWidget = smDD.coverDialog[0];
       lockDlg = smDD.lockCoverDialog;
       lockDlgVisible = False; /* mappedWhenManaged False */
    }
    else
    {
       smGD.lockCursor = smGD.padlockCursor;
       grabWidget = smDD.lockDialog;
       lockDlg = smDD.lockDialog;
       visibleY = hiddenY = -1;
    }

   /*
    * Note: grabWidget must be mapped in order to grab it. This means
    * that if coverScreen is True, smDD.coverDialog[0] must be mapped
    * immediately and if coverScreen is False, smDD.lockDialog must be
    * mapped immediately. Also, if a grabWidget is unmapped, the grab
    * is lost. Ah X.
    */
    XtAddEventHandler(grabWidget, VisibilityChangeMask,
                      False, FinishLocking, NULL);

    XtManageChild(lockDlg);

    if(smGD.coverScreen == True) {
        flash_id = XtAppAddTimeOut(smGD.appCon,
                               1000, BlinkCaret,smDD.indLabel[1]);
    }
    else{
        flash_id = XtAppAddTimeOut(smGD.appCon,
                               1000, BlinkCaret,smDD.indLabel[0]);
    }

    if (lockDelay > 0)
    {
     /*
      * Wait for 'lockDelay' seconds before requiring a password. 
      */
      lockDelayId = XtAppAddTimeOut(smGD.appCon,
                               lockDelay*1000, RequirePassword, NULL);
  
    }
    else if (lockDelay == 0)
    { 
     /* 
      * Immediately require a password to unlock the display.
      */
      smGD.lockedState = LOCKED;
      PutUpLogin(True, False); /* map, but don't set timeout */
    }

    if (smGD.coverScreen == True && smSaverRes.cycleTimeout > 0)
    { 
     /*
      * Cycle to next saver in 'cycleTimeout' seconds.
      */
      cycleId = XtAppAddTimeOut(smGD.appCon,
                               smSaverRes.cycleTimeout*1000, CycleSaver, NULL);
    }
    
    if(smGD.coverScreen == True)
    {
       for(i = (smGD.numSavedScreens - 1);i >= 0;i--)
       {
          XtPopup(smDD.coverDialog[i], XtGrabNone);
       }
    }

   /*
    * Add an event handler for when the keyboard and pointer are grabbed
    */
    XtAddEventHandler(grabWidget,
                      (KeyPressMask | ButtonPressMask | PointerMotionMask),
                      False, EventDetected, NULL);
   /*
    * Add an event handler for client message to popup coverDialog
    */
    if ((smGD.numSavedScreens - 1) > 0) {
        PropDtSmLockState property;

        XtAddEventHandler(smGD.topLevelWid, 0, True, ProcessPopup, NULL);
        xa_dt_sm_lock_state = XInternAtom(smGD.display,
            _XA_DT_SM_LOCK_STATE, False);
        property.flags = 0;
        property.lockState = (unsigned long) True;
        XChangeProperty (smGD.display, smGD.topLevelWindow,
            xa_dt_sm_lock_state, xa_dt_sm_lock_state,
            32, PropModeReplace, (unsigned char *) &property,
            PROP_DT_SM_LOCK_STATE_ELEMENTS);
    }

    /** wait 90 seconds for lock dialog to come up **/
    lockTimeId = XtAppAddTimeOut(smGD.appCon,
                                 90000, LockAttemptFailed, lockDlg);
    return;
}



/*************************************<->*************************************
 *
 *  FinishLocking ()
 *
 *
 *  Description:
 *  -----------
 *  After the lock dialog is up - do the grab and lock the display
 *
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
FinishLocking(wid, ptr, ev, bl)
Widget		wid;
XtPointer	ptr;
XEvent		*ev;
Boolean		*bl;
#else
FinishLocking(Widget		wid,
	      XtPointer		ptr,
	      XEvent		*ev,
	      Boolean		*bl)
#endif /* _NO_PROTO */
{
    int i,j;
    Boolean kbdGrabbed, pointerGrabbed;
    int rc;

        if (lockTimeId == (XtIntervalId)0)
          return;

        XtRemoveTimeOut(lockTimeId); 
        lockTimeId = (XtIntervalId)0;
        XtRemoveEventHandler(wid, VisibilityChangeMask,
                             False, FinishLocking, NULL);
        XSync(smGD.display, 0);

        i = 0;
        XtSetArg(uiArgs[i], XmNy, &visibleY);i++;
        XtGetValues(wid, uiArgs, i);

        hiddenY = (Position) DisplayHeight(smGD.display, smGD.screen) + 15;

	/*
	 * Color the cursor for this color scheme
	 */
	RecolorCursor();

        XSync(smGD.display, 0);

	/*
	 * grab control of the keyboard for the entire display
	 */
        rc = XtGrabKeyboard(grabWidget, False,
				     GrabModeAsync, GrabModeAsync,
				     CurrentTime);
        kbdGrabbed = (rc == GrabSuccess);

#ifdef __hpux
       XHPDisableReset(smGD.display);
#endif /* __hpux */

#if defined (AIXV3) && !defined(_POWER)
       if(smGD.secureSystem)
       {
         SM_SETEUID(smGD.unLockUID);
         AixEnableHftRing(0);
         SM_SETEUID(smGD.runningUID);
       }
#endif

	pointerGrabbed = (XtGrabPointer(grabWidget, False,
					ButtonPressMask|PointerMotionMask,
					GrabModeAsync, GrabModeAsync,
					None, smGD.lockCursor, CurrentTime)
			  == GrabSuccess);

        {
          pointerGrabbed = (XtGrabPointer(grabWidget, False,
                                        ButtonPressMask|PointerMotionMask,
                                        GrabModeAsync, GrabModeAsync,
                                        None, smGD.lockCursor, CurrentTime)
                            == GrabSuccess);
        }


	/*
	 * If the grab failed - try 3 more times and give up
	 */
	if((kbdGrabbed == False) || (pointerGrabbed == False))
	{
	    for(j = 0;(j < 3) && ((pointerGrabbed == False) ||
				  (kbdGrabbed == False));j++)
	    {
		/*
		 * If a grab fails try one more time and then give up
		 */
		if(kbdGrabbed == False)
		{
		    sleep(1);
		    kbdGrabbed = (XtGrabKeyboard(grabWidget, False,
						 GrabModeAsync, GrabModeAsync,
						 CurrentTime) == GrabSuccess);
		}

		if(pointerGrabbed == False)
		{
		    sleep(1);
		    pointerGrabbed = (XtGrabPointer(grabWidget, False,
						    ButtonPressMask |
						    PointerMotionMask,
						    GrabModeAsync,
						    GrabModeAsync,
						    None, smGD.lockCursor,
						    CurrentTime)
				      == GrabSuccess);
		}
	    }
	}


	/*
	 * Set status variable to lock if the lock has succeeded
	 */
	if((pointerGrabbed != True) || (kbdGrabbed != True))
	{
	    PrintError(DtError, smNLS.cantLockErrorString);
	    smGD.lockedState = UNLOCKED;
	    UnlockDisplay(pointerGrabbed, kbdGrabbed);
	}
	else
	{

#ifdef LOCK_SERVER_ACCESS
            /*
             *  Wipe & enable X server access control list
             */
            hostList = XListHosts(smGD.display,
                &hostListLen, (Bool *) &hostListActive);
            XRemoveHosts(smGD.display, hostList, hostListLen);
            XEnableAccessControl(smGD.display);
            RestrictingAccess = True;
#endif
 
            PutUpLogin(False, True); /* already mapped, but set timeout */

           /*
            * Start external screen saver.
            */
            if (smGD.coverScreen)
            {
              StartScreenSaver();
            }
	}

    return;
}


/*************************************<->*************************************
 *
 *  CreateLockCursor ()
 *
 *
 *  Description:
 *  -----------
 *  Creates a padlock cursor if the user has specified lock.  Creates a
 *  blank cursor if the user has specified cover.  Both are specified in the
 *  users resource file.
 * 
 *
 *
 *  Inputs:
 *  ------
 *  buttonForm = widget from which cursor gets its color
 *  smGD.coverScreen = (global) cover screen or put up a padlock
 
 *
 * 
 *  Outputs:
 *  -------
 *  smGD.lockCursor = (global) cursor when lock is active (blank or padlock)
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#define lock_m_hot_x	16
#define lock_m_hot_y	16
#define lock_m_bm_width 32
#define lock_m_bm_height 32
static unsigned char lock_m_bm_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x0f, 0x00,
   0x00, 0xfc, 0x9f, 0x00, 0x00, 0x0e, 0x90, 0x01, 0x00, 0x06, 0x80, 0x01,
   0x00, 0x06, 0x80, 0x01, 0x00, 0x06, 0x80, 0x01, 0x00, 0x06, 0x80, 0x01,
   0x00, 0x06, 0x80, 0x01, 0x00, 0x06, 0x80, 0x01, 0x00, 0x06, 0x80, 0x01,
   0x00, 0x06, 0x80, 0x01, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x0c,
   0x80, 0xaa, 0xaa, 0x0e, 0x00, 0x55, 0x55, 0x0f, 0x80, 0xaa, 0xaa, 0x0e,
   0x00, 0x55, 0x55, 0x0f, 0x80, 0xaa, 0xaa, 0x0e, 0x00, 0x55, 0x55, 0x0f,
   0x80, 0xaa, 0xaa, 0x0e, 0x00, 0x55, 0x55, 0x0f, 0x80, 0xaa, 0xaa, 0x0e,
   0x00, 0x55, 0x55, 0x0f, 0x80, 0xaa, 0xaa, 0x0e, 0x00, 0x55, 0x55, 0x0f,
   0x80, 0xaa, 0xaa, 0x0e, 0x00, 0x55, 0x55, 0x0f, 0x80, 0xaa, 0xaa, 0x0e,
   0xc0, 0xff, 0xff, 0x0f, 0xe0, 0xff, 0xff, 0x0f};

#define lock_m_m_bm_width 32
#define lock_m_m_bm_height 32
static unsigned char lock_m_m_bm_bits[] = {
   0x00, 0xf8, 0x1f, 0x00, 0x00, 0xfe, 0x7f, 0x00, 0x00, 0xff, 0xff, 0x00,
   0x00, 0xff, 0xff, 0x00, 0x80, 0x0f, 0xf0, 0x01, 0x80, 0x07, 0xe0, 0x01,
   0x80, 0x07, 0xe0, 0x01, 0x80, 0x07, 0xe0, 0x01, 0x80, 0x07, 0xe0, 0x01,
   0x80, 0x07, 0xe0, 0x01, 0x80, 0x07, 0xe0, 0x01, 0x80, 0x07, 0xe0, 0x01,
   0x80, 0x07, 0xe0, 0x01, 0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f,
   0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f,
   0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f,
   0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f,
   0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f,
   0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f,
   0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f};

#define lock_s_hot_x	7
#define lock_s_hot_y	8
#define lock_s_bm_width 13
#define lock_s_bm_height 16
static unsigned char lock_s_bm_bits[] = {
   0x00, 0x02, 0x00, 0x04, 0xf0, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
   0x00, 0x00, 0xa8, 0x1a, 0x54, 0x1d, 0xa8, 0x1a, 0x54, 0x1d, 0xa8, 0x1a,
   0x54, 0x1d, 0xa8, 0x1a, 0x54, 0x1d, 0xfe, 0x1f};

#define lock_s_m_bm_width 13
#define lock_s_m_bm_height 16
static unsigned char lock_s_m_bm_bits[] = {
   0xf8, 0x03, 0xfc, 0x07, 0xfe, 0x0f, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e,
   0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f,
   0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f};

void 
#ifdef _NO_PROTO
CreateLockCursor()
#else
CreateLockCursor( void )
#endif /* _NO_PROTO */
{
    Pixmap source, mask;
    Colormap cmap = XDefaultColormap(smGD.display, smGD.screen);
    XColor xcolors[2];
    char noCursorBits = 0x1;

    if(InitCursorInfo() == False)
    {
	/*
	 * Create the SMALL padlock cursor
	 */
	source = XCreateBitmapFromData(smGD.display,
				       XRootWindow(smGD.display,
						   smGD.screen),
				       (char *) lock_s_bm_bits,
				       lock_s_bm_width,
				       lock_s_bm_height);
	mask = XCreateBitmapFromData(smGD.display,
				     XRootWindow(smGD.display,
						 smGD.screen),
				     (char *) lock_s_m_bm_bits,
				     lock_s_m_bm_width,
				     lock_s_m_bm_height);

	/* translate the Pixels into XColors */
	xcolors[0].pixel = smGD.blackPixel;
	xcolors[1].pixel = smGD.whitePixel;
	XQueryColors(smGD.display, cmap, xcolors, 2);

	/* create the padlock cursor */
	smGD.padlockCursor = XCreatePixmapCursor(smGD.display, source, mask,
					      &(xcolors[0]), &(xcolors[1]),
					      lock_s_hot_x,
					      lock_s_hot_y);
	XFreePixmap(smGD.display, source);
	XFreePixmap(smGD.display, mask);
    }
    else
    {
	/*
	 * Create the LARGE padlock cursor
	 */
	source = XCreateBitmapFromData(smGD.display,
				       XRootWindow(smGD.display,
						   smGD.screen),
				       (char *) lock_m_bm_bits,
				       lock_m_bm_width,
				       lock_m_bm_height);
	mask = XCreateBitmapFromData(smGD.display,
				     XRootWindow(smGD.display,
						 smGD.screen),
				     (char *) lock_m_m_bm_bits,
				     lock_m_m_bm_width,
				     lock_m_m_bm_height);

	/* translate the Pixels into XColors */
	xcolors[0].pixel = smGD.blackPixel;
	xcolors[1].pixel = smGD.whitePixel;
	XQueryColors(smGD.display, cmap, xcolors, 2);

	/* create the padlock cursor */
	smGD.padlockCursor = XCreatePixmapCursor(smGD.display, source, mask,
						 &(xcolors[0]), &(xcolors[1]),
						 lock_m_hot_x,
						 lock_m_hot_y);
	XFreePixmap(smGD.display, source);
	XFreePixmap(smGD.display, mask);
    }

    /*
     *
     * create the blank cursor
     */
    source = XCreateBitmapFromData(smGD.display,
				   XRootWindow(smGD.display, smGD.screen),
				   &noCursorBits, 1, 1);

    xcolors[0].pixel = smGD.blackPixel;
    XQueryColor(smGD.display, cmap, &(xcolors[0]));
    smGD.blankCursor = XCreatePixmapCursor(smGD.display, source, source,
					  &(xcolors[0]), &(xcolors[0]),
					  0, 0);
    XFreePixmap(smGD.display, source);
}



/*************************************<->*************************************
 *
 *  RecolorCursor ()
 *
 *
 *  Description:
 *  -----------
 *  Recolors the padlock cursor to be the current color scheme.  This has
 *  to be done because XCreatePixmapCursor allocates colors instead of jusst
 *  using a pixel value.
 * 
 *
 *
 *  Inputs:
 *  ------
 *  smGD.backgroundPix = Pixel value to use for background color
 *  smGD.foregroundPix = Pixel value to use for foreground color
 *
 * 
 *  Outputs:
 *  -------
 *  smGD.lockCursor = (global) cursor when lock is active (padlock)
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
RecolorCursor()
#else
RecolorCursor( void )
#endif /* _NO_PROTO */
{
    Colormap cmap = XDefaultColormap(smGD.display, smGD.screen);
    XColor xcolors[2];

    /*
     * translate the Pixels into XColors
     */
    xcolors[0].pixel = smGD.foregroundPix;
    xcolors[1].pixel = smGD.backgroundPix;
    XQueryColors(smGD.display, cmap, xcolors, 2);

    /*
     * recolor the padlock cursor
     */
    XRecolorCursor(smGD.display, smGD.lockCursor, &(xcolors[0]),
		   &(xcolors[1]));
}



/*************************************<->*************************************
 *
 *  ProcessPopup (w, client_data, event)
 *
 *
 *  Description:
 *  -----------
 *  Callback routine that detects ClientMessage from dtwm when window
 *  gets managed. We then popup coverDialog so that no window appears over
 *  locked screen.
 *
 *  Inputs:
 *  ------
 *  w = widget where event occured
 *  client_data = client specific data sent to callback
 *  event = event that triggered callback
 *
 *
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/
static void
#ifdef _NO_PROTO
ProcessPopup( w, client_data, ev, bl)
Widget w ;
XtPointer client_data ;
XEvent *ev;
Boolean   *bl;
#else
ProcessPopup(
              Widget w,
              XtPointer client_data,
              XEvent *ev,
              Boolean *bl)
#endif /* _NO_PROTO */
{
    int i;

    if ( (smGD.numSavedScreens - 1) > 0 && 
	 ev->type == ClientMessage &&
         smGD.lockedState == LOCKED ) {
        for (i = 0; i <= (smGD.numSavedScreens - 1);i++) {
	    if (smDD.coverDialog[i] != NULL) {
                XtPopup(smDD.coverDialog[i], XtGrabNone);
	    }
	}
    }
}


/*************************************<->*************************************
 *
 *  EventDetected (w, client_data, event)
 *
 *
 *  Description:
 *  -----------
 *  Callback routine that detects an event when the display is locked.
 *  If it's a correct password, it unlocks the display.  Otherwise, it
 *  just displays status.  The event is detected by the popup login dialog.
 *
 *
 *  Inputs:
 *  ------
 *  w = widget where event occured
 *  client_data = client specific data sent to callback
 *  event = event that triggered callback
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
EventDetected( w, client_data, ev, bl)
Widget w ;
XtPointer client_data ;
XEvent *ev;
Boolean   *bl;
#else
EventDetected(
	      Widget w,
	      XtPointer client_data,
	      XEvent *ev,
	      Boolean *bl)
#endif /* _NO_PROTO */
{
    char str[256];
    int len;
    XKeyEvent	*event = (XKeyEvent *) ev;
   
    if (smGD.lockedState == LOCKED)
    {
      if (lockDlgVisible == False)
      {
       /*
        * Anytime input is received - show the passwd dialog and
        * discard event. This is so a keypress event that causes
        * the passwd dialog to appear will not be used in the password.
        * CMVC 612.
        */
	PutUpLogin(True, True); /* map, and reset timeout */
        return;
      }
      PutUpLogin(False, True); /* already mapped, but reset timeout */
    }
    else 
    {
      UnlockDisplay(True, True);
      return;
    }

    /*
     * If the detected event is anything but a keypress, processing is
     * complete after refreshing the status string.  Note: event will be
     * NULL if this was triggered by a screen saver off extension call.
     */
    if (!event || event->type != KeyPress)
	return;

    /*
     * If XLookupString() returns anything (which it won't in the case of, say,
     * pressing the shift key or an arrow key), process it.
     */
#ifdef __hpux
    if (len = XHPConvertLookup(event, str, sizeof(str), NULL, NULL,
			       XHPGetEurasianCvt(smGD.display)))
#else /* __hpux */
    if (len = XLookupString (event, str, sizeof(str), NULL, NULL))
#endif /* __hpux */
    {
	if (smGD.lockedState == LOCKED)
	    CheckString(str, len);
    }
}



/*************************************<->*************************************
 *
 *  CheckString (s, i)
 *
 *
 *  Description:
 *  -----------
 *  Check string entered by user.  If it is a valid password, call routine to
 *  unlock the display.  Otherwise, just keep track of what we have until
 *  the password is valid
 *
 *
 *  Inputs:
 *  ------
 *  s = string passed in for checking
 *  i = length of string
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
CheckString( s, i )
        register char *s ;
        register int i ;
#else
CheckString(
        register char *s,
        register int i )
#endif /* _NO_PROTO */
{
    /*
     * password rules:
     *	- Only the first eight characters are used.
     *	- If pw_length > 8, we've gone over eight characters and won't
     *	  accept any more.
     *	- An ESC kills the line.
     */
#ifdef SIA
    static char passwd[82];           /* password space */
#else
    static char passwd[10];		/* password space */
#endif
    static int pw_length = 0;		/* password length */

    if (s == NULL)
    {
     /*
      * Clear password.
      */
      pw_length = 0;
      return;
    }
    
    for (; i>0; s++,i--)
    {
	switch(*s)
	{
	    case '\010':
		if (pw_length)
		    pw_length--;		/* back up one character */
		break;
		
	    case '':
		pw_length = 0;			/* kill the character string */
		break;

	    case '\n':
	    case '\r':
#ifdef SIA
		if (pw_length > 80)
                {
                    pw_length = 80;
                }
#else
		if (pw_length > 8)
		{
		    pw_length = 8;
		}
#endif
		passwd[pw_length] = '\0';	/* terminate string */
		pw_length = 0;			/* reset length */
		if (CheckPassword(passwd))
		{
		    UpdatePasswdField(0);
		    UnlockDisplay(True, True);
		    return;
		}
		XBell(smGD.display, 100);	/* wrong, laserbreath */
		break;

	    default:
#ifdef SIA
                if (pw_length < 80)
#else
		if (pw_length < 8)
#endif
		    passwd[pw_length] = *s;	/* store character */
                /*
		 * The length is incremented no matter what, so the user can
		 * think the program handles multi-thousand-character
		 * passwords.  If the user types twenty characters and eighteen
		 * erases (#), the result will be the first two characters
		 * entered, as expected.  Up to a point -- 65536 is long
		 * enough!
		 */
		if (pw_length < 65535)
		    pw_length++;
		break;
	}
    }

    if(pw_length > 8)
    {
	UpdatePasswdField(8);
    }
    else
    {
	UpdatePasswdField(pw_length);
    }
}



/*************************************<->*************************************
 *
 *  CheckPassword (passwd)
 *
 *
 *  Description:
 *  -----------
 *  Check the password to see if it is the user's, roots, or one of the
 *  users specified in the host string
 *
 *
 *  Inputs:
 *  ------
 *  passwd = password passed in
 *
 * 
 *  Outputs:
 *  -------
 *  True if it is a valid password, false otherwise.
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static Boolean 
#ifdef _NO_PROTO
CheckPassword( passwd )
        char *passwd ;
#else
CheckPassword(
        char *passwd )
#endif /* _NO_PROTO */
{
    char *p, *q;
    char *keyholderbuf;

    if (Authenticate(NULL, getuid(), passwd) == True)
    {	
      return(True);  /* user password ok */
    }

    if (Authenticate(NULL, 0, passwd) == True)
    {
      return(True);  /* root password ok */
    }

    /* check passwords of users specified as keyholders */
    if (smGD.keyholders == NULL)
    {
	return(False);			/* no keyholders */
    }

    /* since strtok() is destructive, copy the keyholders string */
    keyholderbuf = (char *) SM_MALLOC(strlen(smGD.keyholders)+1);
    if(keyholderbuf == NULL)
    {
	PrintErrnoError(DtError, smNLS.cantMallocErrorString);
	return(False); /* no memory */
    }
    
    strcpy(keyholderbuf, smGD.keyholders);
    for (p = keyholderbuf; (q = strtok(p, ", \t")) != NULL; p = NULL)
    {
        if (Authenticate(q, -1, passwd) == True)
	{
          SM_FREE(keyholderbuf);
          return(True); /* keyholder password ok */
        }
    }
    SM_FREE(keyholderbuf);

    return(False); /* no matches */
}



/*************************************<->*************************************
 *
 *  UnlockDisplay ()
 *
 *
 *  Description:
 *  -----------
 *  If the user has entered a correct password, unlock the display and
 *  uncover the root window.
 *
 *
 *  Inputs:
 *  ------
 *  pointerGrabbed - Boolean tells if pointer is currently grabbed
 *  kbdGrabbed - Boolean tells if keyboard is currently grabbed
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
UnlockDisplay( pointerGrabbed, kbdGrabbed )
        Boolean pointerGrabbed ;
        Boolean kbdGrabbed ;
#else
UnlockDisplay(
        Boolean pointerGrabbed,
        Boolean kbdGrabbed)
#endif /* _NO_PROTO */
{
    int i;
    Tt_message msg;

    if (grabWidget == NULL) return;

#ifdef LOCK_SERVER_ACCESS
    /*
     * Restore X server access state
     */
    if (RestrictingAccess) {
        XAddHosts(smGD.display, hostList, hostListLen);
        if (!hostListActive) XDisableAccessControl(smGD.display);
        RestrictingAccess = False;
        XFree((void *) hostList);
    }
#endif

    /*
     * Stop external screen saver.
     */
    StopScreenSaver();

    /*
     * Remove the event handler to grab the events
     */
    XtRemoveEventHandler(grabWidget,
			 (KeyPressMask | ButtonPressMask | PointerMotionMask),
			 False, EventDetected, NULL);
    /*
     * Remove event handler to popup coverDialog
     */
    if ((smGD.numSavedScreens - 1) > 0) {
        PropDtSmLockState property;

        XtRemoveEventHandler(smGD.topLevelWid, 0, True, ProcessPopup, NULL);
        property.flags = 0;
        property.lockState = (unsigned long) False;
        XChangeProperty (smGD.display, smGD.topLevelWindow,
            xa_dt_sm_lock_state, xa_dt_sm_lock_state,
            32, PropModeReplace, (unsigned char *) &property,
            PROP_DT_SM_LOCK_STATE_ELEMENTS);
    }


    /*
     * Turn off alarms
     */
    if(lockDelayId != (XtIntervalId)0)
    {
	XtRemoveTimeOut(lockDelayId);
    }

    if(cycleId != (XtIntervalId)0)
    {
        XtRemoveTimeOut(cycleId);
    }

    if(timerId != (XtIntervalId)0)
    {
        XtRemoveTimeOut(timerId);
    }

    if(flash_id != (XtIntervalId)0)
    {
        XtRemoveTimeOut(flash_id);
    }

    if(pointerGrabbed == True)
    {
	XtUngrabPointer(grabWidget, CurrentTime);
    }

    if(kbdGrabbed == True)
    {
	XtUngrabKeyboard(grabWidget, CurrentTime);
    }
    
#ifdef __hpux
    XHPEnableReset(smGD.display);
#endif /* __hpux */

#if defined (AIXV3) && !defined(_POWER)
    if(smGD.secureSystem)
    {
      SM_SETEUID(smGD.unLockUID);
      AixEnableHftRing(1);
      SM_SETEUID(smGD.runningUID);
    }
#endif

    XSync(smGD.display, 0);

   /*
    * Unmanage session lock dialogs. If LOCKDLG_PERSIST is undefined,
    * destroy them. This is so the passwd dialog icon colors get freed
    * since currently it uses a lot of colors. 
    */
    if(smGD.coverScreen == False) 
    {
#if defined (LOCKDLG_PERSIST)
        if (XtIsManaged(smDD.lockDialog))
        {
	  XtUnmanageChild(smDD.lockDialog);
        }
#else
        XtDestroyWidget(smDD.lockDialog);
        smDD.lockDialog = grabWidget = NULL;
#endif
    }
    else
    {
#if defined (LOCKDLG_PERSIST)
	if(!XtIsManaged(smDD.lockCoverDialog))
	{
	    XtManageChild(smDD.lockCoverDialog);
	}
	
	for(i = (smGD.numSavedScreens - 1);i >= 0;i--)
	{
	    XtPopdown(smDD.coverDialog[i]);
	}
#else
        for(i = (smGD.numSavedScreens - 1);i >= 0;i--)
        {
            XtDestroyWidget(smDD.coverDialog[i]);
            smDD.coverDialog[i] = grabWidget = NULL;
        }
	smDD.lockCoverDialog = NULL;
#endif
    }

    smGD.lockedState = UNLOCKED;

    XSync(smGD.display, 0);

    /*
     * Tell the Workspace Manager to unlock the display (depress the lock
     * button)
    if(smGD.bmsDead == False)
    {
	msg = tttk_message_create(0, TT_REQUEST, TT_SESSION, 0,
				  "Display_Unlock", 0);
	tt_message_send(msg);
	tt_message_destroy(msg);
    }
     */

    SetSystemReady();
    
}

static Position visibleY = -1;
static Position hiddenY = -1;


/*************************************<->*************************************
 *
 *  TakeDownLogin ()
 *
 *
 *  Description:
 *  -----------
 *  When a timeout occurs - take down the login screen by unmanaging it.
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
TakeDownLogin( client_data, id )
        XtPointer client_data ;
        XtIntervalId *id ;
#else
TakeDownLogin(
        XtPointer client_data,
        XtIntervalId *id )
#endif /* _NO_PROTO */
{
  if (lockDlgVisible == True)
  {
    if (smGD.coverScreen == True)
    {
      XtUnmapWidget(smDD.lockCoverDialog);
    }
    else
    {
      if (hiddenY > -1)
      {
        int i = 0;
        XtSetArg(uiArgs[i], XmNy, hiddenY);i++;
        XtSetValues(smDD.lockDialog, uiArgs, i);
      }
    }

    if(flash_id != (XtIntervalId)0)
    {
        XtRemoveTimeOut(flash_id);
    }
    
    timerId = (XtIntervalId)0; 

   /*
    * Clear partially entered password if any.
    */
    CheckString(NULL, 0);
    lockDlgVisible = False;

    XSync(smGD.display, 0);
  }
}


/*************************************<->*************************************
 *
 *  PutUpLogin ()
 *
 *
 *  Description:
 *  -----------
 *  Redisplays the cover and the login when neccessary.
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
PutUpLogin(map, timeout)
  Boolean map;
  Boolean timeout;
#else
PutUpLogin(
  Boolean map,
  Boolean timeout )
#endif /* _NO_PROTO */
{
  if (map == True && lockDlgVisible == False)
  {
    if (smGD.coverScreen == True)
    {
      XtMapWidget(smDD.lockCoverDialog);
    }
    else
    {
      if (visibleY > -1)
      {
        int i = 0;
        XtSetArg(uiArgs[i], XmNy, visibleY);i++;
        XtSetValues(smDD.lockDialog, uiArgs, i);
      }
    }
    lockDlgVisible = True;
  }

  if (timeout == True)
  {
    if(timerId != (XtIntervalId)0)
    {
	XtRemoveTimeOut(timerId);
    }
 
    if(smRes.alarmTime > 0)
    {
	timerId = XtAppAddTimeOut(smGD.appCon,
				  (smRes.alarmTime * 1000),
				  TakeDownLogin,NULL);
    }
  }
  XSync(smGD.display, 0);
}


/*************************************<->*************************************
 *
 *  LockAttemptFailed ()
 *
 *
 *  Description:
 *  -----------
 *  Timed out trying to get a visibilitynotify on the lock
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
LockAttemptFailed(ptr, invId)
XtPointer	ptr;
XtIntervalId	*invId;
#else
LockAttemptFailed(XtPointer	ptr,
		  XtIntervalId	*invId)
#endif /* _NO_PROTO */
{
    Widget lockWid = (Widget) ptr;
    
    PrintError(DtError, smNLS.cantLockErrorString);
    smGD.lockedState = UNLOCKED;
    XtRemoveEventHandler(lockWid, VisibilityChangeMask,
			 False, FinishLocking, NULL);
    UnlockDisplay(False, False);
    XSync(smGD.display, 0);
}




/*************************************<->*************************************
 *
 *  RequirePassword ()
 *
 *
 *  Description:
 *  -----------
 *  Callback indicating a password is now required to unlock display.
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
RequirePassword(ptr, invId)
XtPointer	ptr;
XtIntervalId	*invId;
#else
RequirePassword(XtPointer	ptr,
		  XtIntervalId	*invId)
#endif /* _NO_PROTO */
{
  smGD.lockedState = LOCKED;
}


/*************************************<->*************************************
 *
 *  BlinkCaret ()
 *
 *
 *  Description:
 *  -----------
 *  blinks the caret in the password field
 *
 *
 *  Inputs:
 *  ------
 *
 *
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/
static void
#ifdef _NO_PROTO
BlinkCaret(ptr, invId)
XtPointer       ptr;
XtIntervalId    *invId;
#else
BlinkCaret(XtPointer    ptr,
                  XtIntervalId  *invId)
#endif /* _NO_PROTO */
{
static int flag = 1;
XmString tmpString;
int i;
 
 /*
  * Blink cursor to show the focus ..
  */
  if(flag){
  tmpString = XmStringCreateLtoR (" ", XmFONTLIST_DEFAULT_TAG);
  i = 0;
  XtSetArg(uiArgs[i], XmNlabelString,     tmpString); i++;
  XtSetValues(ptr, uiArgs, i);
  flag = 0;
  }
  else{
  tmpString = XmStringCreateLtoR ("|", XmFONTLIST_DEFAULT_TAG);
  i = 0;
  XtSetArg(uiArgs[i], XmNlabelString,     tmpString); i++;
  XtSetValues(ptr, uiArgs, i);
  flag = 1;
  }
 
  XmStringFree(tmpString);
  flash_id = XtAppAddTimeOut(smGD.appCon, 1000,
                            BlinkCaret, ptr);
}



/*************************************<->*************************************
 *
 *  CycleSaver ()
 *
 *
 *  Description:
 *  -----------
 *  Callback indicating we should cycle to next screen saver
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
CycleSaver(ptr, invId)
XtPointer	ptr;
XtIntervalId	*invId;
#else
CycleSaver(XtPointer	ptr,
		  XtIntervalId	*invId)
#endif /* _NO_PROTO */
{
 /*
  * Stop running screen saver, start a new one and reset timer.
  */
  StopScreenSaver();
  StartScreenSaver();  
  cycleId = XtAppAddTimeOut(smGD.appCon, smSaverRes.cycleTimeout*1000,
                            CycleSaver, NULL);
}



/*************************************<->*************************************
 *
 *  localAuthenticate (name, uid, passwd)
 *
 *
 *  Description:
 *  -----------
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static Boolean 
#ifdef _NO_PROTO
localAuthenticate( name, uid, passwd )
        char *name ;
        uid_t uid;
        char *passwd ;
#else
localAuthenticate(
        char *name,
        uid_t uid,
        char *passwd )
#endif /* _NO_PROTO */
#ifdef SIA



  {
    char *Argv[2] = { "dtsession", NULL };

    SIAENTITY *se = NULL;
    int code;
    char *  pw_name;
    uid_t real_uid;


    real_uid = getuid();

    if (-1 == seteuid(0))
      return FALSE;


    if (passwd == NULL)
    {
      /*
      * Caller just checking if it is possible to access 
      * password file (ie is dtsession suid bit set properly).
      */
      seteuid(real_uid);
      return TRUE;
    }
    if (name && name[0])
      pw_name = name;
    else if (uid == 0)
      pw_name = "root";
    else
      pw_name = getlogin();


    if ( sia_ses_init(&se, 1, Argv, (char *)NULL,
                      pw_name, NULL, 0 /* don't collect info */, 
                      NULL) != SIASUCCESS)
      {
      seteuid(real_uid);
      return FALSE;
      }
    
    se->password = (char *)malloc(strlen(passwd) + 1);
    if ( se->password == (char *)NULL ) 
    {
        sia_ses_release(&se);
      seteuid(real_uid);
      return FALSE;
    }

    strcpy(se->password, passwd);

    code = sia_ses_reauthent (NULL, se);
    sia_ses_release(&se);

    seteuid(real_uid);

    if ( code == SIASUCCESS )
      return TRUE;
    else
      return FALSE;
}
    

#else

{
    register struct passwd *pwent;
    char *p, *q;
    char *crypt();
    Boolean rc = True;
    Boolean done = False;


#ifdef SVR4
# ifdef USL
    uinfo_t uinfo;
    char *upasswd, *newname = NULL;
# else
    struct spwd *sp=NULL;
# endif
#endif

    if(smGD.secureSystem)
    {	
	SM_SETEUID(smGD.unLockUID);
    }

#ifdef SVR4
   /*
    * Get shadow password entry for 'name' or 'uid'.
    */
    if (name == NULL)
    {
      pwent = getpwuid(uid);
      if (pwent != NULL) 
      {
# ifdef USL
	name = newname = strdup(pwent->pw_name);
# else
        name = pwent->pw_name;
#endif
      }
    }

    if (name == NULL ||
# ifdef USL
	ia_openinfo(name, &uinfo)
# else
	(sp = getspnam(name)) == NULL
# endif
	)
    {
     /*
      * Can't get entry.
      */
      rc = False;
      done = True;
    }
#else  /* SVR4 */
   /*
    * Get password entry for 'name' or 'uid'.
    */
    if ((pwent = (name == NULL ? getpwuid(uid) : getpwnam(name))) == NULL)
    {
     /*
      * Can't get entry.
      */
      rc = False;
      done = True;
    }
#endif /* SVR4 */

    if(smGD.secureSystem)
    {
        SM_SETEUID(smGD.runningUID);
    }

    if (done == False)
    {
#ifdef USL
        ia_get_logpwd(uinfo, &upasswd);
#endif
#ifdef sun 
	if (sp == NULL)
#else
	if (
	    pwent->pw_passwd == NULL 
	    || pwent->pw_passwd[0] == '*' 
#ifdef USL
	    || upasswd == NULL
#endif
	    )
#endif
      {
       /*
        * Could not read password.
        */
        rc = False;
        done = True;
      }
    }

#ifdef sun
    if ( done == False && 
         (strcmp("*NP*",sp->sp_pwdp) == 0) ) {
	   /* 
	    * Try reading shadow password table one more time
	    * (with user runningUID).  Previously (with root unLockUID), 
	    * could not get access to the user's NIS+ shadow password 
	    * with root's credendtials. 
	    */
    	  if ( (sp = getspnam(name)) == NULL ||
               strcmp("*NP*",sp->sp_pwdp) == 0) {
     	     /*
      	      * Can't get NIS+ shadow password entry.
      	      */
      	      rc = False;
      	      done = True;
          }
    }
#endif

    if (done == False)
    {
      if (passwd == NULL)
      {
       /*
        * Caller just checking if it is possible to access 
        * password file (ie is dtsession suid bit set properly).
        */
        rc = True; 
        done = True;
      }
    }

    if (done == False)
    {
     /*
      * Check password.
      */
#ifdef SVR4
# ifdef USL
      if (strcmp(crypt(passwd, upasswd), upasswd) != 0)
# else
      if (strcmp(crypt(passwd,sp->sp_pwdp),sp->sp_pwdp) != 0)
# endif
#else
      if (strcmp(pwent->pw_passwd, crypt(passwd, pwent->pw_passwd)) != 0)
#endif
      {
	struct pam_conv pam_conv = {lockscrn_conv, NULL};
	pam_handle_t *pamh = NULL;
	int status=PAM_SUCCESS;

       /*
	* Password does not match Unix password.  Try PAM authentication
        * for any alternate authentication mechanisms that may be present.
        */

	status = pam_start(DtProgName, pwent->pw_name, &pam_conv, &pamh);

	if (status != PAM_SUCCESS) {
	    char errbuf[PAM_MAX_MSG_SIZE];

	    snprintf(errbuf, PAM_MAX_MSG_SIZE, 
		     "pam_start status = %d\n", status);
	    PrintError(DtError, errbuf);
	}

	saved_user_passwd = passwd;

	if (status == PAM_SUCCESS) {
            SM_SETEUID(smGD.unLockUID);
	    status = pam_authenticate(pamh, 0);
            SM_SETEUID(smGD.runningUID);

	    if (status != PAM_SUCCESS) {
	        /* Try one more time with generic user's id */
	        status = pam_authenticate(pamh, 0);
	    }
        }

        if (pamh) {
	    pam_end(pamh, PAM_ABORT);
	}

	if (status != PAM_SUCCESS) {
            /*
             * Password incorrect.
             */
            rc = False;
            done = True;
	}
      }
    }

    endpwent();
#ifdef SVR4
# ifdef USL
    ia_closeinfo(uinfo);
    if (newname) free(newname);
# else
    endspent();
# endif
#endif

    return(rc);
}
#endif /* SIA */


/*************************************<->*************************************
 *
 *  Authenticate (name, uid, passwd)
 *
 *
 *  Description:
 *  -----------
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

#if defined (_AIX) && defined(_POWER)

static Boolean 
#ifdef _NO_PROTO
Authenticate( name, uid, passwd )
        char *name ;
        uid_t uid;
        char *passwd ;
#else
Authenticate(
        char *name,
        uid_t uid,
        char *passwd )
#endif /* _NO_PROTO */
{
    register struct passwd *pwent;
    char *p, *q;
    char *crypt();
    Boolean rc = True;
    Boolean done = False;

    int arc;
    int reenter;
    char *newname = NULL;
    char *msg;

    if(smGD.secureSystem)
    {
        SM_SETEUID(smGD.unLockUID);
    }

   /*
    * Map uid to name.
    */
    if (name == NULL)
    {
      pwent = getpwuid(uid);
      if (pwent != NULL)
      {
        name = newname = strdup(pwent->pw_name);
      }
      endpwent();
    }
 
   /* 
    * Authenticate user. Note: normally, we should check 'reenter' to
    * see if the user has another challenge. Since the dtsession screen
    * lock i/f does not yet have the support, our policy is to let the
    * user back in if they pass the first (password) challenge.
    */ 
    arc = authenticate(name, passwd, &reenter, &msg);

    if(smGD.secureSystem)
    {
        SM_SETEUID(smGD.runningUID);
    }

    if (msg) free(msg);
    if (newname) free(newname);

    return(arc == 0 ? True : False);
}
#endif /* _AIX && _POWER */

/*****************************************************************************
 * lockscrn_conv():
 *
 * This is a conv (conversation) function called from the PAM 
 * authentication scheme.  It returns the user's password when requested by
 * internal PAM authentication modules and also logs any internal PAM error
 * messages.
 *****************************************************************************/

static int
lockscrn_conv(int num_msg, struct pam_message **msg,
           struct pam_response **response, void *appdata_ptr)
{
	struct pam_message	*m;
	struct pam_response	*r;
	char 			*temp;
	int			k;
	char 			errbuf[PAM_MAX_MSG_SIZE];

	if (num_msg <= 0)
		return (PAM_CONV_ERR);

	*response = (struct pam_response*) 
				calloc(num_msg, sizeof (struct pam_response));
	if (*response == NULL)
		return (PAM_CONV_ERR);

	(void) memset(*response, 0, sizeof (struct pam_response));

	k = num_msg;
	m = *msg;
	r = *response;
	while (k--) {

		switch (m->msg_style) {

		case PAM_PROMPT_ECHO_OFF:
                    if (saved_user_passwd != NULL) {
                        r->resp = (char *) malloc(strlen(saved_user_passwd)+1);
                        if (r->resp == NULL) {
                     	    __pam_free_resp(num_msg, *response);
                            *response = NULL;
                            return (PAM_CONV_ERR);
                        }
                        (void) strcpy(r->resp, saved_user_passwd);
                        r->resp_retcode=0;
                    }

                    m++;
                    r++;
                    break;


		case PAM_ERROR_MSG:
			if (m->msg != NULL) { 
			    _DtSimpleError(DtProgName, DtError, NULL,
	    	 	    		  "lock screen: %s\n", m->msg); 
			}
			m++;
			r++;
			break;

		case PAM_TEXT_INFO:
			if (m->msg != NULL) { 
			    _DtSimpleError(DtProgName, DtInformation, NULL,
	    	 	    		  "lock screen: %s\n", m->msg); 
			}
			m++;
			r++;
			break;

		default:
			snprintf(errbuf, PAM_MAX_MSG_SIZE, 
				"lockscrn unexpected case: %d\n", m->msg_style);
	        	PrintError(DtError, errbuf);
			break;
		}
	}
	return (PAM_SUCCESS);
}
