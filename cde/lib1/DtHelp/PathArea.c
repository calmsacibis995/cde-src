/* $XConsortium: PathArea.c /main/cde1_maint/2 1995/10/02 14:12:07 lehors $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        PathArea.c
 **
 **   Project:     Cache Creek (Rivers) Project
 **
 **   Description: Routines to create & process the Path area portion of our
 **                help dialog.
 ** 
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 ****************************************************************************
 ************************************<+>*************************************/
#include <sys/param.h>
#include <stdio.h>
#include <string.h>

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/ScrolledW.h>
#include <Xm/List.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/PushBG.h>

#include <X11/Intrinsic.h>
#include <X11/Shell.h>

#include <Dt/Help.h>

#include "Canvas.h"
#include "Access.h"
#include "bufioI.h"
#include "HelposI.h"
#include "HelpDialogI.h"
#include "HelpDialogP.h"
#include "HelpUtilI.h"
#include "PathAreaI.h"
#include "DestroyI.h"
#include "XUICreateI.h"


/*
 * Local Includes
 */

#ifdef	NOTDONE
#ifdef _NO_PROTO

#else

#endif /* _NO_PROTO */
#endif





/*****************************************************************************
 * Function:	    void _DtHelpBuildPathArea(Widget parent,
 *                                 DtHelpDialogWidget nw);  
 *
 *
 * Parameters:      parent      Specifies the widget ID of the help dialog you
 *                              want to set the topic in.
 *                  nw          Specifies the current help dialog widget.
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Creates/updates the Path Display area on top of the help 
 *                  dialog.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
void _DtHelpBuildPathArea(parent, nw)
    Widget parent;
    DtHelpDialogWidget nw;
#else
void _DtHelpBuildPathArea(
    Widget parent,
    DtHelpDialogWidget nw)
#endif 
{

  Arg		args[20];
  int      	n;	
  Widget pathForm, pathLabel, btnBox, tocFrame;
#ifdef AIX_ILS
  Widget _pathForm;
#endif /* AIX_ILS */
  XmString labelString;
  XmFontList  defaultList;
  DtHelpDialogWidget hw = (DtHelpDialogWidget) nw ;

  Dimension button1, button2, button3, button4;
  Dimension maxBtnWidth;

 
  /* Create a form to  hold the topic map stuff */
  n = 0;
  XtSetArg (args[n], XmNmarginWidth, 1);				n++;
  XtSetArg (args[n], XmNmarginHeight, 1);				n++;
  XtSetArg (args[n], XmNshadowType, XmSHADOW_OUT);			n++;
  XtSetArg (args[n], XmNautoUnmanage, False);				n++;
  pathForm = XmCreateForm (parent, "pathForm", args, n);

#ifdef AIX_ILS
  n = 0;
  XtSetArg (args[n], XmNmarginWidth, 1);				n++;
  XtSetArg (args[n], XmNmarginHeight, 1);				n++;
  XtSetArg (args[n], XmNshadowType, XmSHADOW_OUT);			n++;
  XtSetArg (args[n], XmNautoUnmanage, False);				n++;
  _pathForm = XmCreateForm (pathForm, "_pathForm", args, n);

#endif /* AIX_ILS */

  labelString = XmStringCreateLocalized((char *)_DTGETMESSAGE(9, 1,"Volume: "));
  n = 0;
  XtSetArg (args[n], XmNlabelString, labelString);			n++;
  XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); 		n++;
  XtSetArg (args[n], XmNleftOffset, 0);				        n++;
  XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);			n++;
  XtSetArg (args[n], XmNtopOffset, 0);                                  n++;
  XtSetArg (args[n], XmNtraversalOn, FALSE);                            n++;
#ifdef AIX_ILS
  pathLabel = 
         XmCreateLabelGadget (_pathForm, "pathLabel", args, n);
#else /* AIX_ILS */
  pathLabel = 
         XmCreateLabelGadget (pathForm, "pathLabel", args, n);
#endif /* AIX_ILS */
  XtManageChild (pathLabel);
  XmStringFree (labelString);


  /* Create a label for our volume title. We fill it out later 
   * bacause at this point we may not have yet opened the volume.
   */

  n = 0;
  XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); 		n++;
  XtSetArg (args[n], XmNleftWidget, pathLabel);		                n++;
  XtSetArg (args[n], XmNleftOffset, 1);				        n++;
  XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);			n++;
  XtSetArg (args[n], XmNtopOffset, 0);                                  n++;
  XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING);		n++;
#ifdef AIX_ILS
  hw->help_dialog.browser.volumeLabel = 
         XmCreateLabelGadget (_pathForm, "volumeLabel", args, n);
#else /* AIX_ILS */
  hw->help_dialog.browser.volumeLabel = 
         XmCreateLabelGadget (pathForm, "volumeLabel", args, n);
#endif /* AIX_ILS */
  XtManageChild (hw->help_dialog.browser.volumeLabel);


  /* Build a container for our toc to sit in.  We do this because we cannot
  * set form constraints on our toc, but can on the container.
  */
  
  n = 0;
  XtSetArg(args[n],  XmNshadowThickness, 0);		                n++;
  XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);         	n++;
  XtSetArg (args[n], XmNleftOffset, 0);			                n++;
  XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); 		n++;
  XtSetArg (args[n], XmNbottomOffset, 0);		                n++;
  XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);		n++;
  XtSetArg (args[n], XmNtopOffset, 0);                                  n++;
#ifdef AIX_ILS
  XtSetArg (args[n], XmNtopWidget, _pathForm);n++;
#else /* AIX_ILS */
  XtSetArg (args[n], XmNtopWidget, hw->help_dialog.browser.volumeLabel);n++;
#endif /* AIX_ILS */
  tocFrame =  XmCreateFrame (pathForm, "tocFrame", args, n);
  XtManageChild (tocFrame);


  /* Build our button box rowcol widget container */
  n = 0;
  XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET);        	n++;
  XtSetArg (args[n], XmNleftOffset,3);		                        n++;
  XtSetArg (args[n], XmNleftWidget, tocFrame);                          n++;
  XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); 		n++;
  XtSetArg (args[n], XmNbottomOffset, 2);		                n++;
  XtSetArg (args[n], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET);	n++;
  XtSetArg (args[n], XmNtopOffset, 2);                                  n++;
  XtSetArg (args[n], XmNtopWidget, tocFrame);                           n++;
  XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); 		n++;
  XtSetArg (args[n], XmNrightOffset, 1);		                n++;
  XtSetArg (args[n], XmNspacing, 5);			                n++;
  XtSetArg (args[n], XmNentryVerticalAlignment, XmALIGNMENT_CENTER);    n++;
  XtSetArg (args[n], XmNpacking, XmPACK_COLUMN);	                n++;
  XtSetArg (args[n], XmNorientation, XmVERTICAL);	                n++;
  btnBox = XmCreateRowColumn (pathForm, "BtnBox", args, n);
  XtManageChild (btnBox);

  
  /* Backtrack button */
  labelString = XmStringCreateLocalized((char *)_DTGETMESSAGE(9, 2,"Backtrack"));
  n = 0; 
  XtSetArg(args[n], XmNlabelString, labelString); n++;
  hw->help_dialog.browser.btnBoxBackBtn  = XmCreatePushButtonGadget(btnBox,
                                            "btnBoxBackBtn", args, n);
  XtManageChild (hw->help_dialog.browser.btnBoxBackBtn);
  XmStringFree(labelString); 
  XtAddCallback(hw->help_dialog.browser.btnBoxBackBtn,XmNactivateCallback, 
                 _DtHelpDisplayBackCB, (XtPointer) hw); 
  XtSetSensitive(hw->help_dialog.browser.btnBoxBackBtn, FALSE);


  /* History Button */
  labelString = XmStringCreateLocalized((char *)_DTGETMESSAGE(9, 3,"History..."));
  n = 0;
  XtSetArg(args[n], XmNlabelString, labelString); n++;
  hw->help_dialog.browser.btnBoxHistoryBtn = XmCreatePushButtonGadget(btnBox,
				          "btnBoxHistoryBtn",args, n);
  XtManageChild (hw->help_dialog.browser.btnBoxHistoryBtn);
  XmStringFree(labelString); 
  XtAddCallback(hw->help_dialog.browser.btnBoxHistoryBtn,XmNactivateCallback, 
                 _DtHelpDisplayHistoryCB, (XtPointer) hw); 
   

  /* Index  button */
  labelString = 
  XmStringCreateLocalized((char *)_DTGETMESSAGE(9, 4,"Index..."));
  n = 0;
  XtSetArg(args[n], XmNlabelString, labelString); n++;
  hw->help_dialog.browser.btnBoxIndexBtn = XmCreatePushButtonGadget(btnBox,
	    		          "btnBoxIndexBtn",args, n);
  XtManageChild (hw->help_dialog.browser.btnBoxIndexBtn);
  XmStringFree(labelString); 
  XtAddCallback(hw->help_dialog.browser.btnBoxIndexBtn,XmNactivateCallback, 
                _DtHelpDisplayIndexCB, (XtPointer) hw); 


  /* Top Level Browser Button */
 
  if (hw->help_dialog.browser.showTopLevelBtn == TRUE)
    {
      labelString = 
      XmStringCreateLocalized((char *)_DTGETMESSAGE(9, 5,"Top Level"));
      n = 0;
      XtSetArg(args[n], XmNlabelString, labelString); n++;
      hw->help_dialog.browser.btnBoxTopLevelBtn = XmCreatePushButtonGadget(btnBox,
	    		          "btnBoxTopLevelBtn",args, n);
      XtManageChild (hw->help_dialog.browser.btnBoxTopLevelBtn);
      XmStringFree(labelString); 
      XtAddCallback(hw->help_dialog.browser.btnBoxTopLevelBtn,XmNactivateCallback, 
                    _DtHelpDisplayBrowserHomeCB, (XtPointer) hw); 

    }

  /* Get our current fontlist value */
  n = 0;
  XtSetArg (args[n], XmNfontList, &(defaultList));  ++n;
  XtGetValues (hw->help_dialog.menu.topBtn, args, n);


  /* Build the Display Area */

  hw->help_dialog.browser.pTocArea = _DtHelpCreateDisplayArea(tocFrame,
                                   "TocArea",
                                    ((short) hw->help_dialog.display.scrollBarPolicy),
                                    ((short) hw->help_dialog.display.scrollBarPolicy),
				    True,
                                    ((int) hw->help_dialog.browser.visiblePathCount),
                                    1,
                                    _DtHelpDialogHypertextCB,
                                    NULL /*ResizeHelpDialogCB*/,
                                    _DtHelpFilterExecCmdCB,
                                    (XtPointer) hw,
                                    defaultList);
 

  /* We build a blank toc at first, it gets filled in later when 
   * updatePathArea gets called.
   */
   

#ifdef AIX_ILS
  XtManageChild(_pathForm);
#endif /* AIX_ILS */
  XtManageChild(pathForm);


  /* Perform some sizing stuff here to ensure that we layout our UI 
   * per the current fonts.
   */

   /* Get the size of each button */
   n = 0;
   XtSetArg(args[n], XmNwidth, &button1);   ++n;
   XtGetValues(hw->help_dialog.browser.btnBoxBackBtn, args, n);
   n = 0;
   XtSetArg(args[n], XmNwidth, &button2);   ++n;
   XtGetValues(hw->help_dialog.browser.btnBoxHistoryBtn, args, n);
 
   n = 0;
   XtSetArg(args[n], XmNwidth, &button3);   ++n;
   XtGetValues(hw->help_dialog.browser.btnBoxIndexBtn, args, n);

   if (hw->help_dialog.browser.showTopLevelBtn == True)
    {
      n = 0;
      XtSetArg(args[n], XmNwidth, &button4);   ++n;
      XtGetValues(hw->help_dialog.browser.btnBoxTopLevelBtn, args, n);
    }
   else
     button4 = 0;

   if (button1 >= button2)
     maxBtnWidth = button1;
   else
     maxBtnWidth = button2;

   if (button3 >= maxBtnWidth)
     maxBtnWidth = button3;
   
   if (hw->help_dialog.browser.showTopLevelBtn == True)
     if (button4 >= maxBtnWidth)
       maxBtnWidth = button4;

   /* Set our toc right hand attachments */
   n = 0;
   XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); 		n++;
   XtSetArg (args[n], XmNrightOffset, maxBtnWidth +10);	                n++;
   XtSetValues (tocFrame, args, n);


}



/*****************************************************************************
 * Function:	    void _DtHelpUpdatePathArea(char *locationId,
 *                                 DtHelpDialogWidget nw);  
 *
 *
 * Parameters:      locationId Specifies the ID string for the new topic we
 *                              are going to display in the HelpDialog widget.
 *
 *                  helpDialogWidget  Specifies the current help dialog widget.
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Updates the Path Display area on top of the help
 *                  dialog.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
void _DtHelpUpdatePathArea(locationId, nw)
    char *locationId;
    DtHelpDialogWidget nw;
#else
void _DtHelpUpdatePathArea(
    char *locationId,
    DtHelpDialogWidget nw)
#endif 
{
  DtHelpDialogWidget hw = (DtHelpDialogWidget) nw ;
  int retState=-1;


  if (locationId != NULL)
    {
      retState = _DtHelpUpdatePath (hw->help_dialog.browser.pTocArea,
					hw->help_dialog.display.volumeHandle,
					locationId);

      if (retState == -1) 
        {

          /* Some error occured while processing the new toc, 
           * error out or something ??? 
           */
        }
    }
  else if (locationId == NULL)
    { 
      /* We have most likely entered an error state and must set our path 
       * area to NULL values while we display and error message in the 
       * display area 
       */   
      _DtHelpDisplayAreaClean(hw->help_dialog.browser.pTocArea);
 
      /* We set our volumeFlag back to false so we are sure to create a new
       * toc next time we enter this function (i.e. recovering from our
       * error state.
       */
      hw->help_dialog.ghelp.volumeFlag  = FALSE; 
    }
}








