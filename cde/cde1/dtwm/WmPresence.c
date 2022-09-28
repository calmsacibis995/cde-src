#ifdef WSM
/* 
 * (c) Copyright 1987, 1988, 1989, 1990, 1992 HEWLETT-PACKARD COMPANY 
 * ALL RIGHTS RESERVED 
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: WmPresence.c /main/cde1_maint/2 1995/10/11 18:10:52 montyb $"
#endif
#endif

/*
 * Included Files:
 */

#include "WmGlobal.h"
#include "WmResNames.h"
#include "WmHelp.h"

#include <X11/Core.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include <X11/Shell.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/PushBG.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/SeparatoG.h>
#include <Xm/ToggleB.h>
#ifndef  PANELIST
#include <Dt/Help.h>
#endif /* PANELIST */
#include <errno.h>
#include <signal.h>
#include <stdio.h>

/*
 * Function Declarations:
 */

#include "WmPresence.h"

/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static void wspSetWindowName() ;
static Boolean wspCreateWidgets() ;
static Widget wspCreateShell() ;
static Widget wspCreateManager() ;
static Widget wspCreateLabel() ;
static Widget wspCreateSeparator() ;
static Widget wspCreateWorkspaceList() ;
static void wspUpdateWorkspaceList() ;
static Widget wspCreateToggleButton() ;
static Widget wspCreatePushButton() ;
static void wspSetPosition() ;
static void wspLayout() ;
static void wspOkCB() ;
static void wspHelpCB() ;
static void wspAllWsCB() ;
static void wspCancelCB() ;
static void wspExtendedSelectCB() ;
static Dimension wspCharWidth();

#else

static void wspSetWindowName( 
                        PtrWsPresenceData pPres) ;
static Boolean wspCreateWidgets( 
                        WmScreenData *pSD) ;
static Widget wspCreateShell( 
                        WmScreenData *pSD) ;
static Widget wspCreateManager( 
                        Widget shellW) ;
static Widget wspCreateLabel( 
                        Widget mgrW,
                        unsigned char *pchName,
                        unsigned char *pchString) ;
static Widget wspCreateSeparator( 
                        Widget mgrW) ;
static Widget wspCreateWorkspaceList( 
                        Widget mgrW,
                        PtrWsPresenceData pPres,
                        WmScreenData *pSD) ;
static void wspUpdateWorkspaceList( 
                        PtrWsPresenceData pPres) ;
static Widget wspCreateToggleButton( 
                        Widget mgrW,
                        unsigned char *pch) ;
static Widget wspCreatePushButton( 
                        Widget mgrW,
                        char *name,
                        XmString label) ;
static void wspSetPosition( 
                        PtrWsPresenceData pPres) ;
static void wspLayout( 
                        PtrWsPresenceData pPres) ;
static void wspOkCB( 
                        Widget buttonW,
                        WmScreenData *pSD,
                        XtPointer call_data) ;
static void wspHelpCB( 
                        Widget buttonW,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void wspAllWsCB( 
                        Widget buttonW,
                        WmScreenData *pSD,
                        XmToggleButtonCallbackStruct *xmTbcs) ;
static void wspCancelCB( 
                        Widget buttonW,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void wspExtendedSelectCB(
			Widget w,
			XtPointer client_data,
			XmListCallbackStruct *cb );
static Dimension wspCharWidth (
			XmFontList xmfl);

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/

/*
 * External references
 */
#include "WmCDecor.h"
#include "WmCDInfo.h"
#include "WmIconBox.h"
#ifdef PANELIST
#include "WmPanelP.h"  /* for typedef in WmManage.h */
#endif /* PANELIST */
#include "WmManage.h"
#include "WmResParse.h"
#include "WmResource.h"
#include "WmWinInfo.h"
#include "WmWrkspace.h"


/*
 * Global Variables:
 */
/* 
 * These two XmStrings are used in the workspace
 * presence box to indicate the name of the current
 * window for whice the presence box is active.  If the
 * window is iconified, the title will be "Icon:  <iconName>",
 * otherwise the title wil be "Window: <windowName>
 */
XmString windowLabelString;
XmString iconLabelString;

/*************************************<->*************************************
 *
 *  Boolean
 *  MakePresenceBox (pSD)
 *
 *
 *  Description:
 *  -----------
 *
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 *
 * 
 *  Outputs:
 *  -------
 *  Return = (Boolean) True if successful
 *
 *
 *  Comments:
 *  --------
 * 
 ******************************<->***********************************/

Boolean 
#ifdef _NO_PROTO
MakePresenceBox( pSD )
        WmScreenData *pSD ;
#else
MakePresenceBox(
        WmScreenData *pSD )
#endif /* _NO_PROTO */

{
    PtrWsPresenceData pPres = &pSD->presence;
    Boolean	  rval;

    /*
     * Create the widgets for the workspace presence dialog
     */   

    pPres->onScreen = False;

    if (wspCreateWidgets (pSD))
    {
	/*
	 * lay out the form
	 */
	wspLayout (pPres);

	/*
	 * Set the ClientData fields.
	 */   
	XtRealizeWidget (pPres->shellW);
#ifdef PANELIST
	DtWsmRemoveWorkspaceFunctions (DISPLAY1, XtWindow(pPres->shellW));
#endif /* PANELIST */

	ProcessPresenceResources (pSD);

	rval = True;
    }
    else
    {
	Warning(((char *)GETMESSAGE(52, 1, "Unable to create Occupy Workspace dialog.")));
	rval = False;
    }

    return (rval);

} /* END OF FUNCTION MakePresenceBox */


/*************************************<->*************************************
 *
 *  void
 *  ShowPresenceBox (pClient, wsContext)
 *
 *
 *  Description:
 *  -----------
 *  Pops up (shows) the workspace presence dialog
 *
 *  Inputs:
 *  ------
 *  pClient = pointer to client data which needs a presence dialog
 *            up. (This is not the presence dialog's client data!!!)
 *
 *  wsContext = context to post dialog for 
 *
 * 
 *  Outputs:
 *  -------
 *  Return = none
 *
 *
 *  Comments:
 *  --------
 * 
 ******************************<->***********************************/

void 
#ifdef _NO_PROTO
ShowPresenceBox( pClient, wsContext )
        ClientData *pClient ;
        Context wsContext ;
#else
ShowPresenceBox(
        ClientData *pClient,
        Context wsContext )
#endif /* _NO_PROTO */
{
    PtrWsPresenceData pPres;
    WmScreenData *pSD;

    pPres = &pClient->pSD->presence;
    pSD = pClient->pSD;
    pPres->pCDforClient = pClient;
    pPres->contextForClient = wsContext;

    /*
     * Create the presence dialog if not done yet.
     */
    if (!pSD->presence.shellW)
    {
	pPres->ItemSelected = NULL;
	pPres->ItemStrings = NULL;
	pPres->currentWsItem = 0;

	MakePresenceBox (pSD);
    }

    if (pPres->onScreen)
    {
	HidePresenceBox (pSD, True);
    }

    /* update workspace list  */
    wspUpdateWorkspaceList (pPres);

    /*  set position of dialog relative to client window  */
    wspSetPosition (pPres);

    /* 
     * pop it up 
     */
    XtPopup (pPres->shellW, XtGrabNone);
    pPres->onScreen = True;
} /* END OF FUNCTION  ShowPresenceBox */


/*************************************<->*************************************
 *
 *  static void
 *  wspSetWindowName (pPres)
 *
 *
 *  Description:
 *  -----------
 *  Sets the name of the current window in the presence dialog
 *
 *  Inputs:
 *  ------
 *  pPres = ptr to workspace presence dialog data
 * 
 *  Outputs:
 *  -------
 *  Return = none
 *
 *
 *  Comments:
 *  --------
 * 
 ******************************<->***********************************/

static void 
#ifdef _NO_PROTO
wspSetWindowName( pPres )
        PtrWsPresenceData pPres ;
#else
wspSetWindowName(
        PtrWsPresenceData pPres )
#endif /* _NO_PROTO */

{
    int nameN, labelN;
    Arg nameArgs[1];
    Arg labelArgs[1];

    /*
     *  Set the name of the current window
     */

    nameN = 0;
    labelN = 0;
    if (pPres->contextForClient == F_CONTEXT_ICON)
    {
	XtSetArg (nameArgs[nameN], XmNlabelString, 
		  pPres->pCDforClient->iconTitle);	nameN++;
	XtSetArg (labelArgs[labelN], XmNlabelString, 
		  iconLabelString);	                labelN++;
    }
    else
    {
	XtSetArg (nameArgs[nameN], XmNlabelString, 
		  pPres->pCDforClient->clientTitle);	nameN++;
	XtSetArg (labelArgs[labelN], XmNlabelString, 
		  windowLabelString);	                labelN++;
    }

    XtSetValues (pPres->windowNameW, nameArgs, nameN);
    XtSetValues (pPres->windowLabelW, labelArgs, labelN);

} /* END OF FUNCTION  wspSetWindowName */


/*************************************<->*************************************
 *
 *  void
 *  HidePresenceBox (pSD, userDismissed)
 *
 *
 *  Description:
 *  -----------
 *  Pops down (hides) the workspace presence dialog
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 *  userDismissed = did the user dismiss or did workspace switching 
 *                  unpost the workspace presence box ?
 * 
 *  Outputs:
 *  -------
 *  Return = none
 *
 *
 *  Comments:
 *  --------
 * 
 ******************************<->***********************************/

void 
#ifdef _NO_PROTO
HidePresenceBox( pSD, userDismissed )
        WmScreenData *pSD ;
        Boolean  userDismissed;
#else
HidePresenceBox(
        WmScreenData *pSD,
        Boolean userDismissed )
#endif /* _NO_PROTO */

{
    if (pSD->presence.onScreen)
    {
	/* Pop down the shell */
	XtPopdown (pSD->presence.shellW);

	/* 
	 * Do a withdraw to make sure window gets unmanaged
	 * (popdown does nothing if its unmapped)
	 */
	XWithdrawWindow (DISPLAY, XtWindow (pSD->presence.shellW),
			 pSD->screen);
	/* must sync to insure event order */
	XSync (DISPLAY, False);


	pSD->presence.onScreen = False;
	pSD->presence.userDismissed = userDismissed;
    }
} /* END OF FUNCTION   */


/*************************************<->*************************************
 *
 *  wspCreateWidgets (pSD)
 *
 *
 *  Description:
 *  -----------
 *  Creates all the widgets for the workspace presence dialog box
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 * 
 *  Outputs:
 *  -------
 *  Return = false on any failure
 *
 *  Comments:
 *  ---------
 *  Only creates widgets
 ******************************<->***********************************/
static Boolean 
#ifdef _NO_PROTO
wspCreateWidgets( pSD )
        WmScreenData *pSD ;
#else
wspCreateWidgets(
        WmScreenData *pSD )
#endif /* _NO_PROTO */
{
    PtrWsPresenceData pPres = &pSD->presence;
    Arg args [5];
    int n;
    Boolean rval /* = True */;


    rval = ((pPres->shellW = wspCreateShell (pSD)) != NULL);

    if (rval)
    {
	rval = ((pPres->formW = wspCreateManager (pPres->shellW)) != NULL);
    }

    if (rval)
    {
        rval = ((pPres->windowLabelW = wspCreateLabel (pPres->formW,
			(unsigned char *)"window", 
			 NULL))  != NULL);
    }

    if (rval)
    {
        rval = ((pPres->windowNameW = wspCreateLabel (pPres->formW, 
			(unsigned char *)"windowName", 
			(unsigned char *)" ")) != NULL);
    }

    if (rval)
    {
        rval = ((pPres->workspaceLabelW = 
		wspCreateLabel (pPres->formW, (unsigned char *)"workspaces", 
	       ((unsigned char *)GETMESSAGE(52, 3, "Workspaces: ")))) != NULL);
    }

    if (rval)
    {
        rval = ((pPres->workspaceListW =
		wspCreateWorkspaceList (pPres->formW, pPres, pSD)) != NULL);
        pPres->workspaceScrolledListW = XtParent (pPres->workspaceListW);
    }

    if (rval)
    {
        rval = ((pPres->allWsW = wspCreateToggleButton (pPres->formW, 
		  ((unsigned char *)GETMESSAGE(52, 4, "All Workspaces")))) 
		!= NULL);
	XtAddCallback (pPres->allWsW, XmNvalueChangedCallback, 
			(XtCallbackProc)wspAllWsCB, (XtPointer)pSD); 
    }

    if (rval)
    {
        rval = ((pPres->sepW = wspCreateSeparator (pPres->formW)) != NULL);
    }

    if (rval)
    {
#ifdef MESSAGE_CAT
        rval = ((pPres->OkW = 
		 wspCreatePushButton (pPres->formW, "ok", wmGD.okLabel))
			!= NULL);
#else /* MESSAGE_CAT */
        rval = ((pPres->OkW = 
		 wspCreatePushButton (pPres->formW, "ok",
		     XmStringCreateLtoR ("OK", XmFONTLIST_DEFAULT_TAG)))
			!= NULL);
#endif  /* MESSAGE_CAT */

	/* set the default action */
	n = 0;
	XtSetArg (args[n], XmNdefaultButton, pPres->OkW);    n++;
	XtSetValues (pPres->formW, args, n);

	XtAddCallback (pPres->OkW, XmNactivateCallback, 
		(XtCallbackProc) wspOkCB, (XtPointer)pSD); 
    }

    if (rval)
    {
#ifdef MESSAGE_CAT
        rval = ((pPres->CancelW = wspCreatePushButton (pPres->formW, 
			"cancel", wmGD.cancelLabel)) != NULL);
#else /* MESSAGE_CAT */
        rval = ((pPres->CancelW = 
		wspCreatePushButton (pPres->formW, "cancel",
		     XmStringCreateLtoR ("Cancel", XmFONTLIST_DEFAULT_TAG)))
			!= NULL);
#endif /* MESSAGE_CAT */
	XtAddCallback (pPres->CancelW, XmNactivateCallback, 
		(XtCallbackProc) wspCancelCB, (XtPointer)pSD); 

	/* set the cancel button (for KCancel) */
	n = 0;
	XtSetArg (args[n], XmNcancelButton, pPres->CancelW);    n++;
	XtSetValues (pPres->formW, args, n);
    }

    if (rval)
    {
#ifdef MESSAGE_CAT
        rval = ((pPres->HelpW = 
		wspCreatePushButton (pPres->formW, "help", wmGD.helpLabel))
			!= NULL);
#else /* MESSAGE_CAT */
        rval = ((pPres->HelpW = 
		wspCreatePushButton (pPres->formW,  "help",
		     XmStringCreateLtoR ("Help", XmFONTLIST_DEFAULT_TAG)))
			!= NULL);
#endif /* MESSAGE_CAT */

#ifdef  PANELIST
	XtAddCallback (pPres->HelpW, XmNactivateCallback, 
		       WmDtWmTopicHelpCB, 
		       WM_DT_WSPRESENCE_TOPIC);
#else /* PANELIST */
	XtAddCallback (pPres->HelpW, XmNactivateCallback, 
		(XtCallbackProc) wspHelpCB, (XtPointer)pSD); 
#endif /* PANELIST */
    }

    return(rval);
} /* END OF FUNCTION   */



/*************************************<->*************************************
 *
 *  Widget
 *  wspCreateShell (pSD)
 *
 *
 *  Description:
 *  -----------
 *  Creates the shell widget for the workspace presence dialog
 *
 *
 *  Inputs:
 *  ------
 *  psD  =  pointer to screen data
 * 
 *  Outputs:
 *  -------
 *  Return = shell widget created
 *
 ******************************<->***********************************/
static Widget 
#ifdef _NO_PROTO
wspCreateShell( pSD )
        WmScreenData *pSD ;
#else
wspCreateShell(
        WmScreenData *pSD )
#endif /* _NO_PROTO */
{
    Arg setArgs[20];
    int i;
    Widget shellW;

    /*
     * Create top level shell for workspace presence dialog
     */

    i=0;

    XtSetArg (setArgs[i], XmNallowShellResize, (XtArgVal)True);	i++; 
    
    XtSetArg (setArgs[i], XmNborderWidth, (XtArgVal)0); 	i++; 

    XtSetArg (setArgs[i], XmNkeyboardFocusPolicy, 
	(XtArgVal)XmEXPLICIT); 					i++;

    XtSetArg (setArgs[i], XmNmappedWhenManaged, 
	(XtArgVal)False); 					i++;

    XtSetArg (setArgs[i], XmNmwmFunctions, 
	PRESENCE_BOX_FUNCTIONS);				i++;

    XtSetArg (setArgs[i], XmNmwmDecorations, 
	(MWM_DECOR_TITLE|MWM_DECOR_BORDER));	i++;

    XtSetArg (setArgs[i], XmNdepth, 
	(XtArgVal) DefaultDepth (DISPLAY1, pSD->screen)); 	i++;

    XtSetArg (setArgs[i], XmNscreen, (XtArgVal) 
	ScreenOfDisplay (DISPLAY1, pSD->screen)); 		i++;

    XtSetArg (setArgs[i], XtNcolormap, 
	(XtArgVal )DefaultColormap(DISPLAY1, pSD->screen));	i++;

    shellW = (Widget) XtCreatePopupShell (WmNfeedback,
					transientShellWidgetClass,
                                        pSD->screenTopLevelW1,
				        (ArgList)setArgs, i);

    return (shellW);

} /* END OF FUNCTION wspCreateShell */


/*************************************<->*************************************
 *
 *  wspCreateManager (shellW)
 *
 *
 *  Description:
 *  -----------
 *  Creates the manager widget for the workspace presence dialog
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  ---------
 *  Creates a form widget
 ******************************<->***********************************/
static Widget 
#ifdef _NO_PROTO
wspCreateManager( shellW )
        Widget shellW ;
#else
wspCreateManager(
        Widget shellW )
#endif /* _NO_PROTO */
{
    Arg setArgs[20];
    Widget formW;


    /* !!! set colors? !!! */

    formW = (Widget) XmCreateForm (shellW, "form", (ArgList) setArgs, 0);

    XtAddCallback (formW, XmNhelpCallback,
                   WmDtWmTopicHelpCB, WM_DT_WSPRESENCE_TOPIC);

    XtManageChild (formW);

    return (formW);
} /* END OF FUNCTION   */



/*************************************<->*************************************
 *
 *  wspCreateLabel (mgrW, pchName, pchString)
 *
 *
 *  Description:
 *  -----------
 *  Creates a label widget as a child of the passed in manager
 *
 *  Inputs:
 *  ------
 *  mgrW = manager widget (parent of this label)
 *  pchName = name of the widget
 *  pchString = string that is to be in label
 * 
 *  Outputs:
 *  -------
 *  Return = Widget created.
 *
 *  Comments:
 *  ---------
 ******************************<->***********************************/
static Widget 
#ifdef _NO_PROTO
wspCreateLabel( mgrW, pchName, pchString )
        Widget mgrW ;
        unsigned char *pchName ;
        unsigned char *pchString ;
#else
wspCreateLabel(
        Widget mgrW,
        unsigned char *pchName,
        unsigned char *pchString )
#endif /* _NO_PROTO */
{
    Arg setArgs[20];
    int i;
    Widget labelW;

    i = 0;

    
    if (!strcmp((char *)pchName, "window"))
    {
	windowLabelString = 
	    XmStringCreateLtoR ((char *)GETMESSAGE(52, 2, "Window: "), 
				XmFONTLIST_DEFAULT_TAG);
	/* 
	 * If we do this, USE the message catalog for iconLabelString 
	 * just as we do for windowLabelString !!! 
	 */
	iconLabelString = 
	    XmStringCreateLtoR ((char *)GETMESSAGE(52, 6, "Icon: "), 
				XmFONTLIST_DEFAULT_TAG);


	XtSetArg (setArgs[i], XmNlabelString, windowLabelString );   i++;
    }
    else
    {
	XtSetArg (setArgs[i], XmNlabelString, 
		  XmStringCreateLtoR ((char *)pchString, 
				      XmFONTLIST_DEFAULT_TAG));   i++;
    }
    
    labelW = XmCreateLabelGadget (mgrW, (char *)pchName, setArgs, i);
    XtManageChild (labelW);

    return (labelW);
} /* END OF FUNCTION   */


/*************************************<->*************************************
 *
 *  wspCreateSeparator (mgrW)
 *
 *
 *  Description:
 *  -----------
 *  Creates a separator widget as a child of the passed in manager
 *
 *  Inputs:
 *  ------
 *  mgrW = manager widget (parent of this label)
 * 
 *  Outputs:
 *  -------
 *  Return = Widget created.
 *
 *  Comments:
 *  ---------
 ******************************<->***********************************/
static Widget 
#ifdef _NO_PROTO
wspCreateSeparator( mgrW )
        Widget mgrW ;
#else
wspCreateSeparator(
        Widget mgrW )
#endif /* _NO_PROTO */
{
    Arg setArgs[10];
    int i;
    Widget sepW;

    i = 0;

    sepW = XmCreateSeparatorGadget (mgrW, "separator", setArgs, i);
    XtManageChild (sepW);

    return (sepW);
} /* END OF FUNCTION   */



/*************************************<->*************************************
 *
 *  wspCreateWorkspaceList (mgrW, pPres, pSD)
 *
 *
 *  Description:
 *  -----------
 *  Creates a list widget containing all the workspaces defined for a 
 *  screen.
 *
 *  Inputs:
 *  ------
 *  mgrW = manager widget (parent of this child)
 *  pPres = pointer to presence data
 *  pSD = ptr to screen data
 * 
 *  Outputs:
 *  -------
 *  Return = widget created.
 *
 *  Comments:
 *  ---------
 ******************************<->***********************************/
static Widget 
#ifdef _NO_PROTO
wspCreateWorkspaceList( mgrW, pPres, pSD )
        Widget mgrW ;
        PtrWsPresenceData pPres ;
        WmScreenData *pSD ;
#else
wspCreateWorkspaceList(
        Widget mgrW,
        PtrWsPresenceData pPres,
        WmScreenData *pSD )
#endif /* _NO_PROTO */
{
    Arg setArgs[20];
    int i;
    Widget listW;
    int numVisible;

#define MIN_VISIBLE	6

    /*
     * Create the array of strings that will go into the list.
     */
    if (((pPres->ItemStrings = (XmStringTable) XtMalloc 
		(pSD->numWorkspaces * sizeof(XmString *))) == NULL) ||
	((pPres->ItemSelected = (Boolean *) XtMalloc 
		(pSD->numWorkspaces * sizeof(Boolean))) == NULL))
    {
	Warning (((char *)GETMESSAGE(52, 5, "Insufficient memory to create Occupy Workspace dialog.")));
	return (NULL);
    }

    pPres->numWorkspaces = pSD->numWorkspaces;

    for (i = 0; i < pSD->numWorkspaces; i++)
    {
	pPres->ItemStrings[i] = XmStringCopy (pSD->pWS[i].title);

	if (pSD->pWS[i].id == pSD->pActiveWS->id)
	{
	    pPres->currentWsItem = 1+i;
	}
    }


    /* 
     * Create the widget
     */
    i = 0;

    XtSetArg (setArgs[i], XmNitemCount, pSD->numWorkspaces);	i++;

    XtSetArg (setArgs[i], XmNitems, pPres->ItemStrings);	i++;

    XtSetArg (setArgs[i], XmNselectionPolicy, XmEXTENDED_SELECT); i++;

    XtSetArg (setArgs[i], XmNlistSizePolicy, XmRESIZE_IF_POSSIBLE); i++;

    listW = XmCreateScrolledList (mgrW, "list", setArgs, i);
    XtManageChild (listW);

    if (pPres->pCDforClient)
    {
	/*
	 * Highlight the workspaces this client resides in
	 */
	for (i = 0; i < pPres->pCDforClient->pSD->numWorkspaces; i++)
	{
	    if (ClientInWorkspace (&pPres->pCDforClient->pSD->pWS[i], 
					pPres->pCDforClient))
	    {
		XmListSelectPos (listW, i+1, TRUE);
		pPres->ItemSelected[i] = True;
	    }
	    else
	    {
		pPres->ItemSelected[i] = False;
	    }
	}
    }

    /*
     * Insure a minimum number are visible.
     */
    i = 0;
    XtSetArg (setArgs[i], XmNvisibleItemCount, &numVisible);	i++;
    XtGetValues (listW, setArgs, i);

    if (numVisible < MIN_VISIBLE)
    {
	i = 0;
	XtSetArg (setArgs[i], XmNvisibleItemCount, MIN_VISIBLE);i++;
	XtSetValues (listW, setArgs, i);
    }

    XtAddCallback (listW, XmNextendedSelectionCallback,
		(XtCallbackProc) wspExtendedSelectCB, (XtPointer)pSD); 


    return (listW);
} /* END OF FUNCTION   */


/*************************************<->*************************************
 *
 *  wspUpdateWorkspaceList (pPres)
 *
 *
 *  Description:
 *  -----------
 *  Updates the list widget containing all the workspaces.
 *  Sets the ones for this client.
 *
 *  Inputs:
 *  ------
 *  pPres = ptr to presence dialog data
 * 
 *  Outputs:
 *  -------
 *  none
 *
 *  Comments:
 *  ---------
 ******************************<->***********************************/
static void 
#ifdef _NO_PROTO
wspUpdateWorkspaceList( pPres )
        PtrWsPresenceData pPres ;
#else
wspUpdateWorkspaceList(
        PtrWsPresenceData pPres )
#endif /* _NO_PROTO */
{
    int n;
    XmString xmsTmp, *pxmsSelected;
    WmScreenData *pSD;
    Arg args[5];
    int wsnum, numSelected;

    /* 
     * Update the list of workspaces to -- one may have been
     * renamed since the last time we were up.
     */
    pSD = pPres->pCDforClient->pSD;
    for (wsnum = 0; wsnum < pSD->numWorkspaces; wsnum++)
    {
	xmsTmp = XmStringCopy (pSD->pWS[wsnum].title);

	if (!XmStringCompare (xmsTmp, pPres->ItemStrings[wsnum]))
	{
	    /* 
	     * Replace the string in our local list
	     */
	    XmStringFree (pPres->ItemStrings[wsnum]);
	    pPres->ItemStrings[wsnum] = xmsTmp;

	    /* 
	     * Replace the item in the scrolled list.
	     */
	    XmListDeletePos (pPres->workspaceListW, 1+wsnum);
	    XmListAddItem (pPres->workspaceListW, xmsTmp, 1+wsnum);
	}
	else
	{
	    XmStringFree (xmsTmp);
	}
    }

    /*
     * Highlight the workspaces this client resides in
     */
    XmListDeselectAllItems (pPres->workspaceListW);
    pxmsSelected = (XmString *) 
      XtMalloc (pPres->pCDforClient->pSD->numWorkspaces * sizeof (XmString));
    numSelected = 0;
    for (wsnum = 0; wsnum < pPres->pCDforClient->pSD->numWorkspaces; wsnum++)
    {
	if (ClientInWorkspace (&pPres->pCDforClient->pSD->pWS[wsnum], 
	    pPres->pCDforClient))
	{
	    pxmsSelected[numSelected++] = pPres->ItemStrings[wsnum];
	    pPres->ItemSelected[wsnum] = True;
	}
	else
	{
	    pPres->ItemSelected[wsnum] = False;
	    pPres->pCDforClient->putInAll = False;
	}

	if (pPres->pCDforClient->pSD->pActiveWS->id ==
			pPres->pCDforClient->pSD->pWS[wsnum].id)
	{
	    /* save item number of current workspace */
	    pPres->currentWsItem = 1+wsnum;
	}
    }

    /* set the selected items */
    n = 0;
    XtSetArg (args[n], XmNselectedItems, pxmsSelected);		n++;
    XtSetArg (args[n], XmNselectedItemCount, numSelected);	n++;
    XtSetValues (pPres->workspaceListW, args, n);

    /* set state of all workspaces button */
    n = 0;
    XtSetArg (args[n], XmNset, pPres->pCDforClient->putInAll);	n++;
    XtSetValues (pPres->allWsW, args, n);

    /* set name of window we're popped up for */
    wspSetWindowName (pPres);

    XtFree ((char *) pxmsSelected);
    
} /* END OF FUNCTION   */


/*************************************<->*************************************
 *
 *  wspCreateToggleButton (mgrW, pch)
 *
 *
 *  Description:
 *  -----------
 *  Creates a toggle button as a child of mgrW with the string pch.
 *
 *  Inputs:
 *  ------
 *  mgrW = parent widget
 *  pch  = string to use for toggle button
 * 
 *  Outputs:
 *  -------
 *  Return = widget created
 *
 *  Comments:
 *  ---------
 ******************************<->***********************************/
static Widget 
#ifdef _NO_PROTO
wspCreateToggleButton( mgrW, pch )
        Widget mgrW ;
        unsigned char *pch ;
#else
wspCreateToggleButton(
        Widget mgrW,
        unsigned char *pch )
#endif /* _NO_PROTO */
{
    Arg setArgs[20];
    int i;
    Widget toggleW;

    i = 0;

    XtSetArg (setArgs[i], XmNlabelString, 
		XmStringCreateLtoR ((char *)pch, XmFONTLIST_DEFAULT_TAG));   i++;

    toggleW = XmCreateToggleButton (mgrW, (char *)pch, setArgs, i);

    XtManageChild (toggleW);

    return (toggleW);

} /* END OF FUNCTION wspCreateToggleButton  */


/*************************************<->*************************************
 *
 *  wspCreatePushButton (mgrW, label)
 *
 *
 *  Description:
 *  -----------
 *  Creates a push button as a child of mgrW with the string pch.
 *
 *  Inputs:
 *  ------
 *  mgrW = parent widget
 *  label  = XmString to use for button label
 * 
 *  Outputs:
 *  -------
 *  Return = widget created
 *
 *  Comments:
 *  ---------
 ******************************<->***********************************/
static Widget 
#ifdef _NO_PROTO
wspCreatePushButton( mgrW, name, label )
        Widget mgrW ;
        char *name ;
        XmString label ;
#else
wspCreatePushButton(
        Widget mgrW,
        char *name,
        XmString label )
#endif /* _NO_PROTO */

{
    Arg setArgs[20];
    int i;
    Widget pushW;

    i = 0;

    XtSetArg (setArgs[i], XmNlabelString, label);		i++;

    pushW = (Widget) XmCreatePushButton (mgrW, name, setArgs, i);

    XtManageChild (pushW);

    return (pushW);

} /* END OF FUNCTION wspCreatePushButton */


/*************************************<->*************************************
 *
 *  wspSetPosition (pPres)
 *
 *
 *  Description:
 *  -----------
 *  Sets the position of the workspace presence dialog
 *
 *  Inputs:
 *  ------
 *  pPres = pointer to workspace presence data
 *
 *  Outputs:
 *  --------
 * 
 ******************************<->***********************************/
static void 
#ifdef _NO_PROTO
wspSetPosition( pPres )
        PtrWsPresenceData pPres ;
#else
wspSetPosition(
        PtrWsPresenceData pPres )
#endif /* _NO_PROTO */
{
    WmScreenData *pSD = pPres->pCDforClient->pSD;
    Arg args[10];
    int n;
    XFontStruct *font;
    Dimension height;
    int x, y;

    /*
     * Get size of this dialog
     */
    n = 0;
    XtSetArg (args[n], XmNheight, &height);		n++;
    XtGetValues (pPres->shellW, args, n);

    if (wmGD.positionIsFrame)
    {
	if (pSD->decoupleTitleAppearance)
	{
	    font = pSD->clientTitleAppearance.font;
	}
	else
	{
	    font = pSD->clientAppearance.font;
	}

	height += TEXT_HEIGHT(font) + (2 * pSD->frameBorderWidth);
    }


    /* 
     * set position of dialog relative to client window 
     * (use system menu position)
     * set this dialog to be transient for the client
     * for which it is posted.
     */
    GetSystemMenuPosition (pPres->pCDforClient, 
    		 	    &x, &y, height,
			    pPres->contextForClient);

    n = 0;
    XtSetArg (args[n], XmNx, x);				n++;
    XtSetArg (args[n], XmNy, y);				n++;
    XtSetArg (args[n], XmNtransientFor, NULL);			n++;
    if (pPres->contextForClient != F_CONTEXT_ICON)
    {
	XtSetArg (args[n], XmNwindowGroup, pPres->pCDforClient->client); n++;
    }
    else if (pSD->useIconBox && P_ICON_BOX(pPres->pCDforClient))
    {
	XtSetArg (args[n], XmNwindowGroup, 
		  P_ICON_BOX(pPres->pCDforClient)->pCD_iconBox->client); n++;
    }
    else
    {
	XtSetArg (args[n], XmNwindowGroup, 0); n++;
    }
    XtSetArg (args[n], XmNwaitForWm, False);			n++;

    XtSetValues (pPres->shellW, args, n);

} /* END OF FUNCTION wspSetPosition */


/*************************************<->*************************************
 *
 *  wspCharWidth (xmfl)
 *
 *
 *  Description:
 *  -----------
 *  Returns the max logical character width for this fontList
 *
 *
 *  Inputs:
 *  ------
 *  xmfl  -  XmFontList
 * 
 *  Returns:
 *  -------
 *  max logical character width
 *
 *  Comments:
 *  ---------
 ******************************<->***********************************/
static Dimension
#ifdef _NO_PROTO
wspCharWidth( xmfl )
        XmFontList xmfl ;
#else
wspCharWidth(
        XmFontList xmfl )
#endif /* _NO_PROTO */
{
    XmFontContext 	fc;
    XmFontListEntry 	entry;
    Dimension 		dWidth, dTmpWidth;
    XtPointer 		pFont;
    XmFontType 		type;
    XFontSetExtents	*pExtents;

    XmFontListInitFontContext ( &fc, xmfl);

    dWidth = 0;

    entry = XmFontListNextEntry (fc);
    while (entry)
    {
	pFont = XmFontListEntryGetFont (entry, &type);

	switch (type)
	{
	    case XmFONT_IS_FONT:
		dTmpWidth = ((XFontStruct *)pFont)->max_bounds.rbearing -
			    ((XFontStruct *)pFont)->min_bounds.lbearing;
		break;

            case XmFONT_IS_FONTSET:
		pExtents = XExtentsOfFontSet ((XFontSet) pFont);
		dTmpWidth = pExtents->max_logical_extent.width;
		break;

	    default:
		dTmpWidth = 0;
		break;
	}

	if (dTmpWidth > dWidth)
	    dWidth = dTmpWidth;

	entry = XmFontListNextEntry (fc);
    }

    XmFontListFreeFontContext (fc);

    return (dWidth);
}


/*************************************<->*************************************
 *
 *  wspLayout (pPres)
 *
 *
 *  Description:
 *  -----------
 *  Lays out the workspace presence dialog
 *
 *
 *  Inputs:
 *  ------
 *  pPres = pointer to workspace presence data
 * 
 *  Outputs:
 *  -------
 *  none
 *
 *  Comments:
 *  ---------
 ******************************<->***********************************/
static void 
#ifdef _NO_PROTO
wspLayout( pPres )
        PtrWsPresenceData pPres ;
#else
wspLayout(
        PtrWsPresenceData pPres )
#endif /* _NO_PROTO */
{
    Arg args[20];
    int n;

#define SEP_OFFSET	10
#define IW_OFFSET_1	8
#define IW_OFFSET_0	4


    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_POSITION);	n++;
    XtSetArg (args[n], XmNtopPosition, 5);			n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);	n++;
    XtSetArg (args[n], XmNleftOffset, 5);			n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_NONE);	n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_NONE);	n++;
    XtSetValues (pPres->windowLabelW, args, n);

    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);	n++;
    XtSetArg (args[n], XmNtopWidget, pPres->windowLabelW);	n++;
    XtSetArg (args[n], XmNtopOffset, IW_OFFSET_0);		n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);	n++;
    XtSetArg (args[n], XmNleftOffset, 5);			n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_NONE);	n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_NONE);	n++;
    XtSetArg (args[n], XmNbottomOffset, 0);			n++;
    XtSetValues (pPres->workspaceLabelW, args, n);

    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_POSITION);	n++;
    XtSetArg (args[n], XmNtopPosition, 5); 			n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET);	n++;
    XtSetArg (args[n], XmNleftWidget, pPres->workspaceLabelW);	n++;
    XtSetArg (args[n], XmNleftOffset, 5);			n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);	n++;
    XtSetArg (args[n], XmNrightOffset, 5);			n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_NONE);	n++;
    XtSetArg (args[n], XmNbottomOffset, 0);			n++;
    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING);	n++;
    XtSetValues (pPres->windowNameW, args, n);

    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);	n++;
    XtSetArg (args[n], XmNtopWidget, pPres->windowNameW);	n++;
    XtSetArg (args[n], XmNtopOffset, IW_OFFSET_0);		n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET);	n++;
    XtSetArg (args[n], XmNleftWidget, pPres->workspaceLabelW);	n++;
    XtSetArg (args[n], XmNleftOffset, 5);			n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);	n++;
    XtSetArg (args[n], XmNrightOffset, 5);			n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_NONE);	n++;
    XtSetArg (args[n], XmNbottomOffset, 0);			n++;
    XtSetValues (pPres->workspaceScrolledListW, args, n);

    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);	n++;
    XtSetArg (args[n], XmNtopWidget, pPres->workspaceScrolledListW);	n++;
    XtSetArg (args[n], XmNtopOffset, IW_OFFSET_1);		n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET);	n++;
    XtSetArg (args[n], XmNleftWidget, pPres->workspaceLabelW);	n++;
    XtSetArg (args[n], XmNleftOffset, 5);			n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);	n++;
    XtSetArg (args[n], XmNrightOffset, 5);			n++;
    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING);	n++;
    XtSetArg (args[n], XmNmarginRight, 5);			n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_NONE);	n++;
    XtSetArg (args[n], XmNbottomOffset, 0);			n++;
    XtSetValues (pPres->allWsW, args, n);

    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);	n++;
    XtSetArg (args[n], XmNtopWidget, pPres->allWsW);		n++;
    XtSetArg (args[n], XmNtopOffset, SEP_OFFSET);		n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);	n++;
    XtSetArg (args[n], XmNleftOffset, 0);			n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);	n++;
    XtSetArg (args[n], XmNrightOffset, 0);			n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_NONE);	n++;
    XtSetArg (args[n], XmNbottomOffset, 0);			n++;
    XtSetValues (pPres->sepW, args, n);

    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);	n++;
    XtSetArg (args[n], XmNtopWidget, pPres->sepW);		n++;
    XtSetArg (args[n], XmNtopOffset, SEP_OFFSET);		n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION);	n++;
    XtSetArg (args[n], XmNleftPosition, 5);			n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION);	n++;
    XtSetArg (args[n], XmNrightPosition, 30);			n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_POSITION);	n++;
    XtSetArg (args[n], XmNbottomPosition, 95);			n++;
    XtSetValues (pPres->OkW, args, n);

    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);	n++;
    XtSetArg (args[n], XmNtopWidget, pPres->sepW);		n++;
    XtSetArg (args[n], XmNtopOffset, SEP_OFFSET);		n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION);	n++;
    XtSetArg (args[n], XmNleftPosition, 36);			n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION);	n++;
    XtSetArg (args[n], XmNrightPosition, 66);			n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_POSITION);	n++;
    XtSetArg (args[n], XmNbottomPosition, 95);			n++;
    XtSetValues (pPres->CancelW, args, n);

    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);	n++;
    XtSetArg (args[n], XmNtopWidget, pPres->sepW);		n++;
    XtSetArg (args[n], XmNtopOffset, SEP_OFFSET);		n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION);	n++;
    XtSetArg (args[n], XmNleftPosition, 71);			n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION);	n++;
    XtSetArg (args[n], XmNrightPosition, 95);			n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_POSITION);	n++;
    XtSetArg (args[n], XmNbottomPosition, 95);			n++;
    XtSetValues (pPres->HelpW, args, n);
} /* END OF FUNCTION   */



/*************************************<->*************************************
 *
 *  static void
 *  wspOkCB (w, client_data, call_data)
 *
 *
 *  Description:
 *  -----------
 *  OK callback.
 *
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static void 
#ifdef _NO_PROTO
wspOkCB( buttonW, pSD, call_data )
        Widget buttonW ;
        WmScreenData *pSD ;
        XtPointer call_data ;
#else
wspOkCB(
        Widget buttonW,
        WmScreenData *pSD,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
    Arg args[20];
    int n, j;
    int selectedItemCount;
    XmStringTable selectedItems;
    PtrWsPresenceData pPres = &pSD->presence;
    Boolean bAllSelected;


    /* find the selected workspaces */
    n = 0;
    XtSetArg (args[n], XmNselectedItemCount, &selectedItemCount); n++;
    XtSetArg (args[n], XmNselectedItems, &selectedItems); n++;
    XtGetValues (pPres->workspaceListW, args, n);

    /* find the state of all workspaces button */
    n = 0;
    XtSetArg (args[n], XmNset, &bAllSelected);	n++;
    XtGetValues (pPres->allWsW, args, n);

    if (bAllSelected)
    {
	F_AddToAllWorkspaces(NULL, pPres->pCDforClient, NULL);
    }
    else if (selectedItemCount)
    {
	for (n = 0; n < pSD->numWorkspaces; n++)
	{
	    pPres->ItemSelected[n] = False;
	    for (j = 0; j < selectedItemCount; j++)
	    {
		if (XmStringCompare (selectedItems[j], 
			pPres->ItemStrings[n]))
		{
		    pPres->ItemSelected[n] = True;
		}
	    }

	    if (!pPres->ItemSelected[n]  &&
		ClientInWorkspace (&pSD->pWS[n], pPres->pCDforClient))
	    {
		RemoveClientFromWorkspaces (pPres->pCDforClient, 
		    &pSD->pWS[n].id, 1);
	    }

	    if (pPres->ItemSelected[n] &&
		!ClientInWorkspace (&pSD->pWS[n], pPres->pCDforClient))
	    {
		AddClientToWorkspaces (pPres->pCDforClient, 
			&pSD->pWS[n].id, 1);
	    }
	}
    }

    /* withdraw the dialog */
    wspCancelCB (buttonW, (XtPointer)pSD, call_data);
} /* END OF FUNCTION   */


/*************************************<->*************************************
 *
 *  static void
 *  wspHelpCB (w, client_data, call_data)
 *
 *
 *  Description:
 *  -----------
 *  Help callback.
 *
 *
 *  Inputs:
 *  ------
 *  None.
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifndef	PANELIST
static void 
#ifdef _NO_PROTO
wspHelpCB( buttonW, client_data, call_data )
        Widget buttonW ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
wspHelpCB(
        Widget buttonW,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{

    DtHelpOnTopic (XtParent(buttonW), "vw_pres");
} /* END OF FUNCTION   */
#endif	/* PANELIST */

/*************************************<->*************************************
 *
 *  static void
 *  wspAllWsCB (w, client_data, call_data)
 *
 *
 *  Description:
 *  -----------
 *  All Workspace toggle button callback.
 *
 *
 *  Inputs:
 *  ------
 *  None.
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static void 
#ifdef _NO_PROTO
wspAllWsCB( buttonW, pSD, xmTbcs )
        Widget buttonW ;
        WmScreenData *pSD ;
        XmToggleButtonCallbackStruct *xmTbcs ;
#else
wspAllWsCB(
        Widget buttonW,
        WmScreenData *pSD,
        XmToggleButtonCallbackStruct *xmTbcs )
#endif /* _NO_PROTO */
{
    PtrWsPresenceData pPres = &pSD->presence;
    int wsnum;
    XmString *pxmsSelected;
    Arg args[5];
    int n;

    if (xmTbcs->reason == XmCR_VALUE_CHANGED)
    {
	if (xmTbcs->set)
	{
	    pxmsSelected = (XmString *)
		XtMalloc (pSD->numWorkspaces * sizeof (XmString));

	    for (wsnum = 0; wsnum < pSD->numWorkspaces; wsnum++)
	    {
		pxmsSelected[wsnum] = pPres->ItemStrings[wsnum];
	    }

	    /* set the selected items */
	    n = 0;
	    XtSetArg (args[n], XmNselectedItems, pxmsSelected);		n++;
	    XtSetArg (args[n], XmNselectedItemCount, pSD->numWorkspaces); n++;
	    XtSetValues (pPres->workspaceListW, args, n);

	    XtFree ((char *) pxmsSelected);
	}
	else
	{
	    /* select current workspace */
	    XmListDeselectAllItems (pPres->workspaceListW);
	    XmListSelectPos (pPres->workspaceListW, pPres->currentWsItem, TRUE);
	}
    }
} /* END OF FUNCTION   */




/*************************************<->*************************************
 *
 *  static void
 *  wspExtendedSelectCB (w, client_data, cb)
 *
 *
 *  Description:
 *  -----------
 *  Cancel callback.
 *
 *
 *  Inputs:
 *  ------
 *  None.
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static void 
#ifdef _NO_PROTO
wspExtendedSelectCB( w, client_data, cb )
        Widget w ;
        XtPointer client_data ;
        XmListCallbackStruct *cb ;
#else
wspExtendedSelectCB(
        Widget w,
        XtPointer client_data,
        XmListCallbackStruct *cb )
#endif /* _NO_PROTO */
{
    WmScreenData *pSD = (WmScreenData *) client_data;
    PtrWsPresenceData pPres = &pSD->presence;
    int n;
    Arg args[5];

    if (cb->reason == XmCR_EXTENDED_SELECT)
    {
	if ((cb->selected_item_count < pSD->numWorkspaces) &&
	    (cb->event != None))
	{
	    n = 0;
	    XtSetArg (args[n], XmNset, False);	n++;
	    XtSetValues (pPres->allWsW, args, n);
	}
    }

} /* END OF FUNCTION wspExtendedSelectCB */



/*************************************<->*************************************
 *
 *  static void
 *  wspCancelCB (w, client_data, call_data)
 *
 *
 *  Description:
 *  -----------
 *  Cancel callback.
 *
 *
 *  Inputs:
 *  ------
 *  None.
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static void 
#ifdef _NO_PROTO
wspCancelCB( buttonW, client_data, call_data )
        Widget buttonW ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
wspCancelCB(
        Widget buttonW,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
    WmScreenData *pSD = (WmScreenData *) client_data;

    XtPopdown (pSD->presence.shellW);
    pSD->presence.onScreen = False;

} /* END OF FUNCTION wspCancelCB */


/*************************************<->*************************************
 *
 * GetPresenceBoxMenuItems (pSD)
 *
 *
 *  Description:
 *  -----------
 *  XXDescription ...
 * 
 *************************************<->***********************************/

MenuItem * 
#ifdef _NO_PROTO
GetPresenceBoxMenuItems( pSD )
        WmScreenData *pSD ;
#else
GetPresenceBoxMenuItems(
        WmScreenData *pSD )
#endif /* _NO_PROTO */
{

    return(NULL);

} /* END OF FUNCTION GetPresenceBoxMenuItems */


/*************************************<->*************************************
 *
 * UpdatePresenceWorkspaces (pSD)
 *
 *
 *  Description:
 *  -----------
 *  Update the presence dialog when the number of workspaces changes.
 * 
 *************************************<->***********************************/

void
#ifdef _NO_PROTO
UpdatePresenceWorkspaces( pSD )
        WmScreenData *pSD ;
#else
UpdatePresenceWorkspaces(
        WmScreenData *pSD )
#endif /* _NO_PROTO */
{
    PtrWsPresenceData pPres = &pSD->presence;
    int wsnum;
    XmString xmsTmp;

    if (pPres->shellW)
    {
	if (pPres->numWorkspaces < pSD->numWorkspaces)
	{
	    if (((pPres->ItemStrings = (XmStringTable) XtRealloc 
		    ((char *)pPres->ItemStrings,
		    (pSD->numWorkspaces * sizeof(XmString *)))) == NULL) ||
		((pPres->ItemSelected = (Boolean *) XtRealloc 
		    ((char *)pPres->ItemSelected,
		    (pSD->numWorkspaces * sizeof(Boolean)))) == NULL))
	    {
		Warning (((char *)GETMESSAGE(52, 5, "Insufficient memory to create Occupy Workspace dialog.")));
		pPres->shellW = NULL;
		return;
	    }
	}

	/*
	 * Replace the names in the dialog's list
	 */
	for (wsnum = 0; wsnum < pPres->numWorkspaces; wsnum++)
	{
	    if (wsnum < pSD->numWorkspaces)
	    {
		xmsTmp = XmStringCopy (pSD->pWS[wsnum].title);

		if (!XmStringCompare (xmsTmp, pPres->ItemStrings[wsnum]))
		{
		    /* 
		     * Replace the string in our local list
		     */
		    XmStringFree (pPres->ItemStrings[wsnum]);
		    pPres->ItemStrings[wsnum] = xmsTmp;

		    /* 
		     * Replace the item in the scrolled list.
		     */
		    XmListDeletePos (pPres->workspaceListW, 1+wsnum);
		    XmListAddItem (pPres->workspaceListW, xmsTmp, 1+wsnum);
		}
		else
		{
		    XmStringFree (xmsTmp);
		}
	    }
	    else
	    {
		/*
		 * Delete this workspace from the list
		 */
		XmStringFree (pPres->ItemStrings[wsnum]);
		pPres->ItemStrings[wsnum] = NULL;
		XmListDeletePos (pPres->workspaceListW, 1+wsnum);
	    }
	}
	for (; wsnum < pSD->numWorkspaces; wsnum++)
	{
	    /*
	     * Add these workspaces to the list.
	     */
	    xmsTmp = XmStringCopy (pSD->pWS[wsnum].title);
	    pPres->ItemStrings[wsnum] = xmsTmp;
	    XmListAddItem (pPres->workspaceListW, xmsTmp, 1+wsnum);

	    if (pPres->pCDforClient && 
	        (ClientInWorkspace (&pPres->pCDforClient->pSD->pWS[wsnum], 
					pPres->pCDforClient)))
	    {
		XmListSelectPos (pPres->workspaceListW, 1+wsnum, TRUE);
		pPres->ItemSelected[wsnum] = True;
	    }
	    else
	    {
		pPres->ItemSelected[wsnum] = False;
	    }
	}

	pPres->numWorkspaces = pSD->numWorkspaces;
    }

} /* END OF FUNCTION UpdatePresenceWorkspaces */

/* DO NOT ADD ANYTHING AFTER THIS ENDIF */
#endif /* WSM */
/****************************   eof    ***************************/
