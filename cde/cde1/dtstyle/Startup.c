/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Startup.c
 **
 **   Project:     DT 3.0
 **
 **   Description: Controls the Dtstyle Startup dialog
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990.  
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
/*$XConsortium: Startup.c /main/cde1_maint/4 1995/10/23 11:29:25 gtsang $*/
/* +++++++++++++++++++++++++++++++++++++++*/
/*  include files                         */
/* +++++++++++++++++++++++++++++++++++++++*/

#include <X11/Xlib.h>
#include <Xm/MwmUtil.h>

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/MessageB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/ToggleBG.h>
#include <Xm/VendorSEP.h>

#include <Dt/DialogBox.h>
#include <Dt/Icon.h>
#include <Dt/TitleBox.h>

#include <Dt/Message.h>
#include <Dt/HourGlass.h>

#include "Help.h"
#include "Main.h"
#include "SaveRestore.h"
#include "Protocol.h"

/*+++++++++++++++++++++++++++++++++++++++*/
/* Internal Functions                    */
/*+++++++++++++++++++++++++++++++++++++++*/

#ifdef _NO_PROTO

static void GetStartupValues() ;
static Widget BuildStartup() ;
static void MapStartup() ;
static void LayoutCB() ;
static void SetCurrentCB() ;
static void SetHomeCB() ;
static void SetAskCB() ;
static void SetConfirmCB() ;
static void SetHomeSession() ;
static void SaveHomeOk() ;
static void ButtonCB() ;

#else

static void GetStartupValues( void ) ;
static Widget BuildStartup( Widget shell) ;
static void MapStartup( 
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void LayoutCB( 
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void SetCurrentCB( 
                        Widget w,
                        XtPointer client_data,
                        XmToggleButtonCallbackStruct *call_data) ;
static void SetHomeCB( 
                        Widget w,
                        XtPointer client_data,
                        XmToggleButtonCallbackStruct *call_data) ;
static void SetAskCB( 
                        Widget w,
                        XtPointer client_data,
                        XmToggleButtonCallbackStruct *call_data) ;
static void SetConfirmCB( 
                        Widget w,
                        XtPointer client_data,
                        XmToggleButtonCallbackStruct *call_data) ;
static void SetHomeSession( 
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void SaveHomeOk( 
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void ButtonCB( 
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;

#endif /* _NO_PROTO */

/*+++++++++++++++++++++++++++++++++++++++*/
/* Internal Variables                    */
/*+++++++++++++++++++++++++++++++++++++++*/
typedef struct {
  Widget      startupForm;
  Widget      resumeTgl;
  Widget      homeTgl;
  Widget      askTgl;
  Widget      modeYesTgl;
  Widget      modeNoTgl;
  Widget      setHome;
  Widget      dlg;
  int         origStartState;
  int         origConfirmMode;
} Startup;
static Startup startup;


static saveRestore save = {FALSE, 0, };


/*+++++++++++++++++++++++++++++++++++++++*/
/* popup_startupBB                       */
/*+++++++++++++++++++++++++++++++++++++++*/
void 
#ifdef _NO_PROTO
popup_startupBB( shell )
        Widget shell ;
#else
popup_startupBB(
        Widget shell )
#endif /* _NO_PROTO */
{

    if (style.startupDialog == NULL) 
    {
      if (smWindow != NULL)
      {
          if (style.smState.smCompatMode) 
          {
               InfoDialog(((char *)GETMESSAGE(7, 13, 
               "This session was started from an X Window System\n\
startup script.  Startup settings are not valid,\n\
since the session cannot be restored.")), 
                style.startupDialog ? style.startupDialog : style.shell, False);

               return;
          }

          _DtTurnOnHourGlass(shell);  
          GetStartupValues();
          BuildStartup(shell);
          XtManageChild(style.startupDialog);
          _DtTurnOffHourGlass(shell);  
      }

      else /* SM not running - post error dialog */
      {
       InfoDialog(((char *)GETMESSAGE(7, 15, 
       "The Session Manager is not running.\n\
Startup settings are not valid.")),
        style.startupDialog ? style.startupDialog : style.shell, False);

       return;
      }
    }
    else 
    {
      XtManageChild(style.startupDialog);
      raiseWindow(XtWindow(XtParent(style.startupDialog)));
    }
}

/*++++++++++++++++++++++++++++++++++++++++*/
/* Get startup values ... used for cancel */
/*++++++++++++++++++++++++++++++++++++++++*/
static void
#ifdef _NO_PROTO
GetStartupValues()
#else
GetStartupValues( void )
#endif /* _NO_PROTO */
{
    startup.origStartState = style.smState.smStartState;
    startup.origConfirmMode = style.smState.smConfirmMode;
}


/*++++++++++++++++++++++++++++++++++++++++*/
/* build__startupBB                       */
/*++++++++++++++++++++++++++++++++++++++++*/
static Widget 
#ifdef _NO_PROTO
BuildStartup( shell )
        Widget shell ;
#else
BuildStartup(
        Widget shell )
#endif /* _NO_PROTO */
{
  register int     n;
  Arg              args[MAX_ARGS];
  XmString         button_string[NUM_LABELS]; 
  XmString         string;
  int              count = 0;
  int              count2 = 0;
  Widget           widget_list[4], 
                   widget_list2[3], 
                   modeForm,
                   shutdownRC;

  startup.dlg = NULL;

  /* Set up DialogBoxDialog button labels */
  button_string[0] = CMPSTR(_DtOkString);
  button_string[1] = CMPSTR(_DtCancelString);
  button_string[2] = CMPSTR(_DtHelpString);
  
  /* Create toplevel DialogBox */
  /* saveRestore
   * Note that save.poscnt has been initialized elsewhere.  
   * save.posArgs may contain information from restoreStartup().*/
  
  XtSetArg(save.posArgs[save.poscnt], XmNbuttonCount, NUM_LABELS);  
  save.poscnt++;
  XtSetArg(save.posArgs[save.poscnt], XmNbuttonLabelStrings, button_string); 
  save.poscnt++;
  XtSetArg(save.posArgs[save.poscnt], XmNdefaultPosition, False); 
  save.poscnt++;
  style.startupDialog = 
    __DtCreateDialogBoxDialog(shell, "startupDlg", save.posArgs, save.poscnt);
  XtAddCallback(style.startupDialog, XmNcallback, ButtonCB, NULL);
  XtAddCallback(style.startupDialog, XmNhelpCallback,
		(XtCallbackProc)HelpRequestCB, (XtPointer)HELP_STARTUP_DIALOG);
  
  XmStringFree(button_string[0]);
  XmStringFree(button_string[1]);
  XmStringFree(button_string[2]);
  
  widget_list[0] = _DtDialogBoxGetButton(style.startupDialog,2);
  n=0;
  XtSetArg(args[n], XmNautoUnmanage, False); n++;
  XtSetArg(args[n], XmNcancelButton, widget_list[0]); n++;
  XtSetValues (style.startupDialog, args, n);
  
  n=0;
  XtSetArg(args[n], XmNtitle, ((char *)GETMESSAGE(7, 14, "Style Manager - Startup"))); n++;
  XtSetArg(args[n], XmNuseAsyncGeometry, True); n++;
  XtSetArg(args[n], XmNmwmFunctions, DIALOG_MWM_FUNC  ); n++;
  XtSetValues (XtParent(style.startupDialog), args, n);
  
  n = 0;
  XtSetArg(args[n], XmNhorizontalSpacing, style.horizontalSpacing); n++;
  XtSetArg(args[n], XmNverticalSpacing, style.verticalSpacing); n++;
  XtSetArg(args[n], XmNallowOverlap, False); n++;
  XtSetArg(args[n], XmNchildType, XmWORK_AREA);  n++;
  startup.startupForm = XmCreateForm(style.startupDialog, "startupForm", args, n);
  
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNfillMode, XmFILL_SELF); n++;
  XtSetArg(args[n], XmNbehavior, XmICON_LABEL); n++;
  XtSetArg(args[n], XmNpixmapForeground, style.secBSCol); n++;
  XtSetArg(args[n], XmNpixmapBackground, style.secTSCol); n++;
  XtSetArg(args[n], XmNstring, NULL); n++;  
  XtSetArg(args[n], XmNshadowThickness, 0); n++;  
  XtSetArg(args[n], XmNimageName, STARTUP_ICON); n++;  
  XtSetArg(args[n], XmNtraversalOn, False); n++;  
  widget_list[count++] = 
    _DtCreateIcon(startup.startupForm, "startupPict", args, n);
  
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNtopWidget, widget_list[count-1]);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNmarginHeight, style.verticalSpacing);  n++; 
  XtSetArg(args[n], XmNmarginWidth, style.horizontalSpacing);  n++; 
  string = CMPSTR(((char *)GETMESSAGE(7, 3, "At Login:")));
  XtSetArg(args[n], XmNtitleString, string);  n++;
  widget_list[count++] = 
    _DtCreateTitleBox(startup.startupForm,"shutdownBox", args, n);
  XmStringFree(string);
  
  n = 0;
  XtSetArg(args[n], XmNmarginHeight, 0);  n++; 
  XtSetArg(args[n], XmNmarginWidth, 0);  n++; 
  shutdownRC= XmCreateRadioBox(widget_list[count-1],"shutdownRC", args, n);
  
  n = 0;
  XtSetArg(args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
  string = CMPSTR(((char *)GETMESSAGE(7, 4, "Resume current session")));
  XtSetArg(args[n], XmNlabelString, string); n++;
  if (style.smState.smStartState == DtSM_CURRENT_STATE)
    {
      XtSetArg(args[n], XmNset, True); n++;
    }
  widget_list2[count2++] = startup.resumeTgl
    = XmCreateToggleButtonGadget(shutdownRC,"resumeTgl", args, n);
  XmStringFree(string);
  
  n = 0;
  XtSetArg(args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
  string = CMPSTR(((char *)GETMESSAGE(7, 5, "Return to Home session")));
  XtSetArg(args[n], XmNlabelString, string); n++;
  if (style.smState.smStartState == DtSM_HOME_STATE)
    {
      XtSetArg(args[n], XmNset, True); n++;
    }
  widget_list2[count2++] = startup.homeTgl
    = XmCreateToggleButtonGadget(shutdownRC,"homeTgl", args, n);
  XmStringFree(string);
  
  n = 0;
  XtSetArg(args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
  string = CMPSTR(((char *)GETMESSAGE(7, 6, "Ask me at Logout")));
  XtSetArg(args[n], XmNlabelString, string); n++;
  if (style.smState.smStartState == DtSM_ASK_STATE)
    {
      XtSetArg(args[n], XmNset, True); n++;
    }
  widget_list2[count2++] = startup.askTgl 
    = XmCreateToggleButtonGadget(shutdownRC,"askTgl", args, n);
  XmStringFree(string);
  
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNtopWidget,  widget_list[count-1]);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNmarginHeight, style.verticalSpacing);  n++; 
  XtSetArg(args[n], XmNmarginWidth, style.horizontalSpacing);  n++; 
  string = CMPSTR(((char *)GETMESSAGE(7, 7, "Logout Confirmation Dialog:")));
  XtSetArg(args[n], XmNtitleString, string);  n++;
  widget_list[count++] = _DtCreateTitleBox(startup.startupForm,"modeBox", args, n);
    XmStringFree(string);
  
  n = 0;
  XtSetArg(args[n], XmNmarginHeight, 0);  n++; 
  XtSetArg(args[n], XmNmarginWidth, 0);  n++; 
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL);  n++; 
  modeForm = XmCreateRadioBox(widget_list[count-1], "modeForm", args, n);
  
  n = 0;
  XtSetArg(args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
  string = CMPSTR(((char *)GETMESSAGE(7, 8, "On"))); 
  XtSetArg(args[n], XmNlabelString, string ); n++;
  if (style.smState.smConfirmMode == DtSM_VERBOSE_MODE)
    {
      XtSetArg(args[n], XmNset, True); n++;
    }
  else {
      XtSetArg(args[n], XmNset, False); n++;
    }
  startup.modeYesTgl= XmCreateToggleButtonGadget(modeForm,"modeYesTgl", args, n);
  XmStringFree(string);
  
  n = 0;
  XtSetArg(args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
  string = CMPSTR(((char *)GETMESSAGE(7, 9, "Off"))); 
  XtSetArg(args[n], XmNlabelString, string ); n++;
  if (style.smState.smConfirmMode != DtSM_VERBOSE_MODE)
    {
      XtSetArg(args[n], XmNset, True); n++;
    }
  else {
    XtSetArg(args[n], XmNset, False); n++;
  }
  startup.modeNoTgl= XmCreateToggleButtonGadget(modeForm,"modeNoTgl", args, n);
  XmStringFree(string);
  
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNtopWidget,  widget_list[count-1]);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNmarginHeight, LB_MARGIN_HEIGHT);  n++;
  XtSetArg(args[n], XmNmarginWidth, LB_MARGIN_WIDTH);  n++;
  XtSetArg(args[n], XmNnavigationType, XmTAB_GROUP); n++;  
  string = CMPSTR(((char *)GETMESSAGE(7, 10, "Set Home Session ...")));
  XtSetArg(args[n], XmNlabelString, string); n++;
  widget_list[count++] = startup.setHome = 
    XmCreatePushButtonGadget(startup.startupForm,"setPB", args, n);
  XmStringFree(string);
  
  XtAddCallback(style.startupDialog, XmNmapCallback, MapStartup, shell);
  XtAddCallback(style.startupDialog, XmNmapCallback, LayoutCB, shell);
  XtAddCallback(startup.resumeTgl, XmNvalueChangedCallback, 
		(XtCallbackProc)SetCurrentCB, NULL);
  XtAddCallback(startup.homeTgl, XmNvalueChangedCallback, 
		(XtCallbackProc)SetHomeCB, NULL);
  XtAddCallback(startup.askTgl, XmNvalueChangedCallback, 
		(XtCallbackProc)SetAskCB, NULL);
  XtAddCallback(startup.modeYesTgl, XmNvalueChangedCallback, 
		(XtCallbackProc)SetConfirmCB, NULL);
  XtAddCallback( widget_list[count-1], XmNactivateCallback, 
		SetHomeSession, style.startupDialog);
  
  XtManageChild(modeForm);
  XtManageChild(startup.startupForm);
  XtManageChild(shutdownRC);
  XtManageChildren(widget_list2,count2); 
  XtManageChild(startup.modeYesTgl);
  XtManageChild(startup.modeNoTgl);
  XtManageChildren(widget_list,count); 
  
  return(style.startupDialog);
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* MapStartup                            */
/*+++++++++++++++++++++++++++++++++++++++*/
static void 
#ifdef _NO_PROTO
MapStartup( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
MapStartup(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{

    DtWsmRemoveWorkspaceFunctions (style.display, XtWindow(XtParent(w)));

    if (!save.restoreFlag)
        putDialog ((Widget)client_data, w);

    XtRemoveCallback(style.startupDialog, XmNmapCallback, MapStartup, NULL);
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* LayoutCB                              */
/*+++++++++++++++++++++++++++++++++++++++*/
static void 
#ifdef _NO_PROTO
LayoutCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
LayoutCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
    int n=0;
    Arg args[4];

    if (XtWidth(startup.startupForm) < (Dimension) (XtWidth(startup.setHome) +
					       (2*style.horizontalSpacing)))
    {
        n=0;
        XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
        XtSetValues(startup.setHome, args, n);
    }
    XtRemoveCallback(style.startupDialog, XmNmapCallback, LayoutCB, NULL);
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* SetCurrentCB                          */
/* tell SM to resume current session     */
/*+++++++++++++++++++++++++++++++++++++++*/
static void 
#ifdef _NO_PROTO
SetCurrentCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XmToggleButtonCallbackStruct *call_data ;
#else
SetCurrentCB(
        Widget w,
        XtPointer client_data,
        XmToggleButtonCallbackStruct *call_data )
#endif /* _NO_PROTO */
{
    if (call_data->set)
        style.smState.smStartState = DtSM_CURRENT_STATE;
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* SetHomeCB                             */
/* tell SM to use home session           */
/*+++++++++++++++++++++++++++++++++++++++*/
static void 
#ifdef _NO_PROTO
SetHomeCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XmToggleButtonCallbackStruct *call_data ;
#else
SetHomeCB(
        Widget w,
        XtPointer client_data,
        XmToggleButtonCallbackStruct *call_data )
#endif /* _NO_PROTO */
{
    if (call_data->set)
        style.smState.smStartState = DtSM_HOME_STATE;
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* SetAskCB                              */
/* tell SM to ask which session          */
/*+++++++++++++++++++++++++++++++++++++++*/
static void 
#ifdef _NO_PROTO
SetAskCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XmToggleButtonCallbackStruct *call_data ;
#else
SetAskCB(
        Widget w,
        XtPointer client_data,
        XmToggleButtonCallbackStruct *call_data )
#endif /* _NO_PROTO */
{
   int n;
   Arg   args[2];

   if (call_data->set)
   {
       style.smState.smStartState = DtSM_ASK_STATE;
       XmToggleButtonSetState(startup.modeYesTgl, True, True);       
   }

}


/*+++++++++++++++++++++++++++++++++++++++*/
/* SetConfirmCB                          */
/* tell SM to be verbose or not          */
/*+++++++++++++++++++++++++++++++++++++++*/
static void 
#ifdef _NO_PROTO
SetConfirmCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XmToggleButtonCallbackStruct *call_data ;
#else
SetConfirmCB(
        Widget w,
        XtPointer client_data,
        XmToggleButtonCallbackStruct *call_data )
#endif /* _NO_PROTO */
{
   int n;
   Arg   args[2];
   Boolean mode;

    if (call_data->set)
        style.smState.smConfirmMode = DtSM_VERBOSE_MODE;
    else
    { 
        style.smState.smConfirmMode = DtSM_QUIET_MODE;
        if (XmToggleButtonGadgetGetState(startup.askTgl))
            XmToggleButtonGadgetSetState(startup.resumeTgl, True, True);
    }
} 


/*+++++++++++++++++++++++++++++++++++++++*/
/* SaveHomeOk                            */
/*+++++++++++++++++++++++++++++++++++++++*/
static void 
#ifdef _NO_PROTO
SaveHomeOk( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
SaveHomeOk(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
    SmSaveHomeSession(startup.origStartState, startup.origConfirmMode);    
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* SetHomeSession                        */
/* allow user to cancel out              */
/*+++++++++++++++++++++++++++++++++++++++*/
static void 
#ifdef _NO_PROTO
SetHomeSession( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
SetHomeSession(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
  int         n=0;
  Arg         args[6];
  Widget      shell = (Widget) client_data;
  XmString    string;
  XmString    okstr, cancelstr;

  if (startup.dlg == NULL)
    {
      string = CMPSTR(((char *)GETMESSAGE(7, 11, "This will replace your home session with\nthe current session.\n\n                   Continue?")));
      XtSetArg(args[n], XmNmessageString, string); n++;
      okstr = CMPSTR(_DtOkString);
      cancelstr = CMPSTR(_DtCancelString);
      XtSetArg(args[n], XmNokLabelString, okstr); n++;
      XtSetArg(args[n], XmNcancelLabelString, cancelstr); n++;
      XtSetArg(args[n], XmNmwmFunctions, DIALOG_MWM_FUNC ); n++;
      startup.dlg = XmCreateWarningDialog(shell, "QNotice", args, n);
      XtUnmanageChild (XmMessageBoxGetChild(startup.dlg, XmDIALOG_HELP_BUTTON));
      XtAddCallback(startup.dlg, XmNokCallback, SaveHomeOk, NULL);
      XmStringFree(string);
      XmStringFree(okstr);
      XmStringFree(cancelstr);
      
      n=0;
      XtSetArg (args[n], XmNmwmInputMode,
		MWM_INPUT_PRIMARY_APPLICATION_MODAL); n++;
      XtSetArg(args[n], XmNtitle, ((char *)GETMESSAGE(7, 12, "Warning"))); n++;
      XtSetValues (XtParent(startup.dlg), args, n);
    }
  XtManageChild(startup.dlg);
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* ButtonCB                              */
/* CB for OK, Cancel, Help in DialogBox  */
/*+++++++++++++++++++++++++++++++++++++++*/
static void 
#ifdef _NO_PROTO
ButtonCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
ButtonCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
    Arg         set[1], unset[1];
    DtDialogBoxCallbackStruct *cb = (DtDialogBoxCallbackStruct *) call_data;

    switch (cb->button_position)
    {
      case OK_BUTTON:

          XtUnmanageChild (w);
	  if (XmToggleButtonGadgetGetState(startup.modeYesTgl))
	    startup.origConfirmMode = DtSM_VERBOSE_MODE;
	  else
	    startup.origConfirmMode = DtSM_QUIET_MODE;

          SmNewStartupSettings();
          break;

      case CANCEL_BUTTON:

         XmToggleButtonGadgetSetState(startup.resumeTgl, True, True);
         XmToggleButtonGadgetSetState(startup.resumeTgl, False, True);

         XtSetArg(set[0], XmNset, True); 
         XtSetArg(unset[0], XmNset, False); 

         if (startup.origStartState == DtSM_CURRENT_STATE)
            XmToggleButtonGadgetSetState(startup.resumeTgl, True, True);

         else if (startup.origStartState == DtSM_HOME_STATE)
            XmToggleButtonGadgetSetState(startup.homeTgl, True, True);

         else if (startup.origStartState == DtSM_ASK_STATE)
            XmToggleButtonGadgetSetState(startup.askTgl, True, True);

         if (startup.origConfirmMode == DtSM_VERBOSE_MODE)
           XmToggleButtonGadgetSetState(startup.modeYesTgl, True, True);
         else 
           XmToggleButtonGadgetSetState(startup.modeNoTgl, True, True);

         style.smState.smStartState = startup.origStartState;
         style.smState.smConfirmMode = startup.origConfirmMode;

         XtUnmanageChild (w);
         break;

      case HELP_BUTTON:
          XtCallCallbacks(style.startupDialog, XmNhelpCallback, (XtPointer)NULL);
	  break;

      default:
	  break;
    }
}


/************************************************************************
 * restoreStartup()
 *
 * restore any state information saved with saveStartup.
 * This is called from restoreSession with the application
 * shell and the special xrm database retrieved for restore.
 ************************************************************************/
void 
#ifdef _NO_PROTO
restoreStartup( shell, db )
        Widget shell ;
        XrmDatabase db ;
#else
restoreStartup(
        Widget shell,
        XrmDatabase db )
#endif /* _NO_PROTO */
{
  XrmName xrm_name[5];
  XrmRepresentation rep_type;
  XrmValue value;

    xrm_name [0] = XrmStringToQuark ("startupDlg");
    xrm_name [2] = NULL;

    /* get x position */
    xrm_name [1] = XrmStringToQuark ("x");
    if (XrmQGetResource (db, xrm_name, xrm_name, &rep_type, &value)){
      XtSetArg (save.posArgs[save.poscnt], XmNx, atoi((char *)value.addr)); 
      save.poscnt++;
      save.restoreFlag = True;
    }

    /* get y position */
    xrm_name [1] = XrmStringToQuark ("y");
    if (XrmQGetResource (db, xrm_name, xrm_name, &rep_type, &value)){
      XtSetArg (save.posArgs[save.poscnt], XmNy, atoi((char *)value.addr)); 
      save.poscnt++;
    }

    xrm_name [1] = XrmStringToQuark ("ismapped");
    XrmQGetResource (db, xrm_name, xrm_name, &rep_type, &value);
    /* Are we supposed to be mapped? */
    if (strcmp(value.addr, "True") == 0)
      popup_startupBB(shell);
}

/************************************************************************
 * saveStartup()
 *
 * This routine will write out to the passed file descriptor any state
 * information this dialog needs.  It is called from saveSessionCB with the
 * file already opened.
 * All information is saved in xrm format.  There is no restriction
 * on what can be saved.  It doesn't have to be defined or be part of any
 * widget or Xt definition.  Just name and save it here and recover it in
 * restoreStartup.  The suggested minimum is whether you are mapped, and 
 * your location.
 ************************************************************************/
void 
#ifdef _NO_PROTO
saveStartup( fd )
        int fd ;
#else
saveStartup(
        int fd )
#endif /* _NO_PROTO */
{
    Position   x,y;
    char *bufr = style.tmpBigStr;     /* size=[1024], make bigger if needed */
    XmVendorShellExtObject  vendorExt;
    XmWidgetExtData         extData;

    if (style.startupDialog != NULL) {
        if (XtIsManaged(style.startupDialog))
          sprintf(bufr, "*startupDlg.ismapped: True\n");
        else
          sprintf(bufr, "*startupDlg.ismapped: False\n");

        /* Get and write out the geometry info for our Window */
	x = XtX(XtParent(style.startupDialog));
	y = XtY(XtParent(style.startupDialog));

        /* Modify x & y to take into account window mgr frames
         * This is pretty bogus, but I don't know a better way to do it.
         */
        extData = _XmGetWidgetExtData(style.shell, XmSHELL_EXTENSION);
        vendorExt = (XmVendorShellExtObject)extData->widget;
        x -= vendorExt->vendor.xOffset;
        y -= vendorExt->vendor.yOffset;

        sprintf(bufr, "%s*startupDlg.x: %d\n", bufr, x);
        sprintf(bufr, "%s*startupDlg.y: %d\n", bufr, y);

        write (fd, bufr, strlen(bufr));
    }
}

