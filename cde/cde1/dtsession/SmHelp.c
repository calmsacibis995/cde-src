/* $XConsortium: SmHelp.c /main/cde1_maint/2 1995/08/30 16:27:22 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:        SmHelp.c
 **
 **  Project:     HP DT Session Manager (dtsession)
 **
 **  Description:
 **  -----------
 **  This module is in charge of all interaction between the session
 **  manager and the help subsystem.  Anytime the user requests help,
 **  it goes through this routine.
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
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Dt/HelpQuickD.h>

#include "Sm.h"
#include "SmUI.h"
#include "SmHelp.h"
#include "SmGlobals.h"

#ifdef __apollo
#include <X11/apollosys.h>
#endif  /* __apollo */


/*
 * #define statements
 */

/*
 * Global variables
 */


/*
 * Local Function Declarations
 */
#ifdef _NO_PROTO

static void CloseHelpDialog ();

#else

static void CloseHelpDialog (Widget, XtPointer, XtPointer);

#endif


/*************************************<->*************************************
 *
 *  TopicHelpRequested ()
 *
 *
 *  Description:
 *  -----------
 *  When a user requests help on a given topic - display that topic in a
 *  new or cached help dialog
 *
 *
 *  Inputs:
 *  ------
 *  client_data - the id that is sent to Cache Creek which tells it which
 *		topic to display
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
TopicHelpRequested( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
TopicHelpRequested(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
    int           i;

    char	  *locationId = (char *) client_data;
    Widget	  transientShell;
    Arg         setArgs2[5];
    int         n2;
    Widget printButton = NULL;
    Widget helpButton = NULL;
    Widget backButton = NULL;
    Widget okButton = NULL;

    /*
     * Find the shell to make this a transient for for
     */
    transientShell = w;
    while(!XtIsSubclass(transientShell, shellWidgetClass))
    {
	transientShell = XtParent(transientShell);
    }

    if((smDD.smHelpDialog != NULL) && 
       (XtIsManaged(smDD.smHelpDialog)))
    {
	return ;
    }


    /*
     * Use the help dialog already created if it has been - otherwise
     * create it
     */
    if (smDD.smHelpDialog != NULL)
    {
	i = 0;
	XtSetArg(uiArgs[i], DtNlocationId,locationId);        	i++;
	XtSetValues(smDD.smHelpDialog, uiArgs, i);

	i = 0;
	XtSetArg(uiArgs[i], XmNtransientFor, transientShell);  i++;
	XtSetValues(XtParent(smDD.smHelpDialog), uiArgs, i);
  
	XtManageChild(smDD.smHelpDialog);
    }
    else
    {
        i = 0;
	XtSetArg(uiArgs[i], XmNtitle, GETMESSAGE(29, 1,
          "Session Manager Help"));  i++;
	XtSetArg (uiArgs[i],DtNhelpVolume, SM_HELP_VOLUME);     i++; 
	XtSetArg (uiArgs[i],DtNhelpType, DtHELP_TYPE_TOPIC);     i++; 
        XtSetArg (uiArgs[i], DtNlocationId,locationId);        i++;
        smDD.smHelpDialog =  DtCreateHelpQuickDialog(smGD.topLevelWid,
						      "helpWidget", uiArgs, i);

	i = 0;
	XtSetArg(uiArgs[i], XmNtransientFor, transientShell);  i++;
	XtSetValues(XtParent(smDD.smHelpDialog), uiArgs, i);
	
	
        XtAddCallback(smDD.smHelpDialog, DtNcloseCallback,
                      CloseHelpDialog, NULL);

	okButton = DtHelpQuickDialogGetChild(smDD.smHelpDialog,
					  DtHELP_QUICK_CLOSE_BUTTON );
        n2=0;
        XtSetArg (setArgs2[n2], XmNlabelString, smDD.okString);    n2++;
        XtSetValues(okButton, setArgs2, n2);

	printButton = DtHelpQuickDialogGetChild(smDD.smHelpDialog,
                                             DtHELP_QUICK_PRINT_BUTTON );
	helpButton = DtHelpQuickDialogGetChild(smDD.smHelpDialog,
					    DtHELP_QUICK_HELP_BUTTON );
	backButton = DtHelpQuickDialogGetChild(smDD.smHelpDialog,
					    DtHELP_QUICK_BACK_BUTTON );
	XtUnmanageChild(printButton);
        XtUnmanageChild(helpButton);
        XtUnmanageChild(backButton);
	
	XtAddCallback (XtParent(smDD.smHelpDialog), XmNpopupCallback,
		       DialogUp, NULL);

        XtManageChild(smDD.smHelpDialog);
    }
}



/*************************************<->*************************************
 *
 *  CloseHelpDialog ()
 *
 *
 *  Description:
 *  -----------
 *  This callback is called when the user wishes to dismiss the help callback
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
CloseHelpDialog(w, clientData, callData) 
Widget		w;		/*  widget id		*/
XtPointer	clientData;	/*  data from applicaiton   */
XtPointer     	callData;	/*  data from widget class  */
#else
CloseHelpDialog (Widget w,
		 XtPointer clientData,
		 XtPointer callData)
#endif /* _NO_PROTO */
{
    XtUnmanageChild(smDD.smHelpDialog);
}
