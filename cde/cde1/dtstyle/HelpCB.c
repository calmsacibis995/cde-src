/* $XConsortium: HelpCB.c /main/cde1_maint/2 1995/10/23 11:17:13 gtsang $ */
/*
 ****************************************************************************
 **
 **   File:        HelpCB.c
 **
 **   Project:     DT 3.0
 **
 **   Description: Contains the callbacks for Help
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990.  
 **
 **
 **
 ****************************************************************************/

#include <Dt/HelpDialog.h>

#include "Main.h"
#include "Help.h"

void
#ifdef _NO_PROTO
HelpRequestCB( w, client_data, call_data )
        Widget w ;
        caddr_t client_data ;
        caddr_t call_data ;
#else
HelpRequestCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
#endif /* _NO_PROTO */
{
    char *helpVolume, *locationId;
    int topic;
    
    topic = (int) client_data;
    helpVolume = HELP_VOLUME;

    switch (topic)
    {
	case HELP_MAIN_WINDOW:
	    locationId = HELP_MAIN_WINDOW_STR;
	    break;
	case HELP_MENUBAR:
	    locationId = HELP_MENUBAR_STR;
	    break;
	case HELP_FILE_BUTTON:
	    locationId = HELP_FILE_BUTTON_STR;
	    break;
	case HELP_HELP_BUTTON:
	    locationId = HELP_HELP_BUTTON_STR;
	    break;
	case HELP_FILE_MENU:
	    locationId = HELP_FILE_MENU_STR;
	    break;
	case HELP_HELP_MENU:
	    locationId = HELP_HELP_MENU_STR;
	    break;
	case HELP_EXIT:
	    locationId = HELP_EXIT_STR;
	    break;
	case HELP_INTRODUCTION:
	    locationId = HELP_INTRODUCTION_STR;
	    break;
	case HELP_TASKS:        
	    locationId = HELP_TASKS_STR;
	    break;
	case HELP_REFERENCE:
	    locationId = HELP_REFERENCE_STR;
	    break;
	case HELP_ITEM:
	    locationId = HELP_ITEM_STR;
	    break;
	case HELP_USING:
	    locationId = HELP_USING_STR;
            helpVolume = HELP_USING_HELP_VOLUME;
	    break;
	case HELP_VERSION:
	    locationId = HELP_VERSION_STR;
	    break;
	case HELP_COLOR_BUTTON:
	    locationId = HELP_COLOR_BUTTON_STR;
	    break;
	case HELP_FONT_BUTTON:
	    locationId = HELP_FONT_BUTTON_STR;
	    break;
	case HELP_BACKDROP_BUTTON:
	    locationId = HELP_BACKDROP_BUTTON_STR;
	    break;
	case HELP_KEYBOARD_BUTTON:
	    locationId = HELP_KEYBOARD_BUTTON_STR;
	    break;
	case HELP_MOUSE_BUTTON:
	    locationId = HELP_MOUSE_BUTTON_STR;
	    break;
	case HELP_AUDIO_BUTTON:
	    locationId = HELP_AUDIO_BUTTON_STR;
	    break;
	case HELP_SCREEN_BUTTON:
	    locationId = HELP_SCREEN_BUTTON_STR;
	    break;
	case HELP_DTWM_BUTTON:
	    locationId = HELP_DTWM_BUTTON_STR;
	    break;
	case HELP_STARTUP_BUTTON:
	    locationId = HELP_STARTUP_BUTTON_STR;
	    break;
	case HELP_COLOR_DIALOG:
	    locationId = HELP_COLOR_DIALOG_STR;
	    break;
	case HELP_ADD_PALETTE_DIALOG:
	    locationId = HELP_ADD_PALETTE_DIALOG_STR;
	    break;
	case HELP_ADD_PALETTE_WARNING_DIALOG:
	    locationId = HELP_ADD_PALETTE_WARNING_DIALOG_STR;
	    break;
	case HELP_DELETE_PALETTE_DIALOG:
	    locationId = HELP_DELETE_PALETTE_DIALOG_STR;
	    break;
	case HELP_DELETE_PALETTE_WARNING_DIALOG:
	    locationId = HELP_DELETE_PALETTE_WARNING_DIALOG_STR;
	    break;
	case HELP_MODIFY_PALETTE_DIALOG:
	    locationId = HELP_MODIFY_PALETTE_DIALOG_STR;
	    break;
	case HELP_COLOR_USE_DIALOG:
	    locationId = HELP_COLOR_USE_DIALOG_STR;
	    break;
	case HELP_FONT_DIALOG:
	    locationId = HELP_FONT_DIALOG_STR;
	    break;
	case HELP_BACKDROP_DIALOG:
	    locationId = HELP_BACKDROP_DIALOG_STR;
	    break;
	case HELP_KEYBOARD_DIALOG:
	    locationId = HELP_KEYBOARD_DIALOG_STR;
	    break;
	case HELP_MOUSE_DIALOG:
	    locationId = HELP_MOUSE_DIALOG_STR;
	    break;
	case HELP_AUDIO_DIALOG:
	    locationId = HELP_AUDIO_DIALOG_STR;
	    break;
	case HELP_SCREEN_DIALOG:
	    locationId = HELP_SCREEN_DIALOG_STR;
	    break;
	case HELP_DTWM_DIALOG:
	    locationId = HELP_DTWM_DIALOG_STR;
	    break;
	case HELP_STARTUP_DIALOG:
	    locationId = HELP_STARTUP_DIALOG_STR;
	    break;
	case HELP_FONT_ATTRIB_DIALOG:
	    locationId = HELP_FONT_ATTRIB_DIALOG_STR;
	    break;
	case HELP_FONT_ADD_GROUP_DLG:
	    locationId = HELP_FONT_ADD_GROUP_DLG_STR;
	    break;
	case HELP_FONT_DEL_GROUP_DLG:
	    locationId = HELP_FONT_DEL_GROUP_DLG_STR;
	    break;
	case HELP_FONT_GROUP_NAME_DLG:
	    locationId = HELP_FONT_GROUP_NAME_DLG_STR;
	    break;
    }

    Help(helpVolume, locationId);
}

void
#ifdef _NO_PROTO
QuickHelpRequestCB( w, client_data, call_data )
        Widget w ;
        caddr_t client_data ;
        caddr_t call_data ;
#else
QuickHelpRequestCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
#endif /* _NO_PROTO */
{

}

void
#ifdef _NO_PROTO
HelpModeCB( w, client_data, call_data )
        Widget w ;
        caddr_t client_data ;
        caddr_t call_data ;
#else
HelpModeCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
#endif /* _NO_PROTO */
{
   Widget widget;

   switch(DtHelpReturnSelectedWidgetId(style.shell, (Cursor)NULL, &widget))
   {
      /*
       * There are additional cases (e.g. user aborts request) but I
       * don't feel they warrant an error dialog.
       */
      case DtHELP_SELECT_VALID:

          while (!XtIsShell(widget))
          {
             if (XtHasCallbacks(widget, XmNhelpCallback) == XtCallbackHasSome)
             {
                XtCallCallbacks(widget, XmNhelpCallback, (XtPointer)NULL);
                return;
             }
             widget = XtParent(widget);
          }
          break;

      case DtHELP_SELECT_INVALID:

          ErrDialog((char *)GETMESSAGE(2, 8, "You must select an item\nwithin the Style Manager."), style.shell);
          break;

   }
}

void
#ifdef _NO_PROTO
HelpHyperlinkCB( w, client_data, call_data )
        Widget w ;
        caddr_t client_data ;
        caddr_t call_data ;
#else
HelpHyperlinkCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
#endif /* _NO_PROTO */
{
    DtHelpDialogCallbackStruct *pHyper = 
	    (DtHelpDialogCallbackStruct *) call_data;

    switch(pHyper->hyperType)
    {
	case DtHELP_LINK_JUMP_NEW:
	    DisplayHelp(pHyper->helpVolume, pHyper->locationId);
	    break;
	default:
            ;
	    /* application defined link code goes here */
    }
}

void
#ifdef _NO_PROTO
HelpCloseCB( w, client_data, call_data )
        Widget w ;
        caddr_t client_data ;
        caddr_t call_data ;
#else
HelpCloseCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
#endif /* _NO_PROTO */
{

    HelpStruct *pHelpCache = (HelpStruct *)client_data;
    HelpStruct *pHelp;

    /*
     * empty for loop just marches pHelp to the right place
     */
    for(pHelp = pHelpCache; 
	pHelp != (HelpStruct *)NULL && pHelp->dialog != w;
	pHelp = pHelp->pNext);

    if(pHelp != (HelpStruct *)NULL)
    {
	/* this should always happen */
        pHelp->inUse = False;
    }

    XtUnmapWidget(XtParent(w));

}
