/* $XConsortium: HelpCB.c /main/cde1_maint/2 1995/08/29 19:37:45 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           HelpCB.c
 *
 *   COMPONENT_NAME: Desktop File Manager (dtfile)
 *
 *   Description:    This file contains the help callbacks for the annotator.
 *
 *   FUNCTIONS: DTHelpRequestCB
 *              HelpRequestCB
 *              IsFilterIcon
 *              IsMainWinDialog
 *              IsMenuWidget
 *              LocateRecordStructure
 *              MapIconWidgetToFileType
 *              ObjectHelp
 *              ProcessItemHelp
 *              ReusePrimaryHelpWindow
 *              TrashHelpRequestCB
 *              closeCB_mainHelpDialog
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <malloc.h>
#include <nl_types.h>

#include <Xm/XmP.h>
#include <Xm/MessageB.h>
#include <Xm/MwmUtil.h>

#include <X11/Shell.h>
#include <X11/Xatom.h>

#include <Xm/Protocols.h>

#include <Dt/DtP.h>
#include <Dt/UserMsg.h>
#include <Dt/DtNlUtils.h>
#include <Dt/FileM.h>
#include <Dt/Action.h>
#include "Encaps.h"

#include <Dt/HelpDialog.h>

#include "Desktop.h"
#include "Main.h"
#include "Help.h"
#include "FileMgr.h"
#include "Prefs.h"
#include "Filter.h"
#include "SharedMsgs.h"


/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static Boolean IsMainWinDialog() ;
static XtPointer MapIconWidgetToFileType();
static Boolean IsMenuWidget();
static Boolean IsFilterIcon();
static Boolean ProcessItemHelp();
static void ReusePrimaryHelpWindow() ;

#else

static Boolean IsMainWinDialog( 
                        DialogData *dialogData) ;
static XtPointer MapIconWidgetToFileType(
                        Widget w,
                        FileMgrData * fileMgrData) ;
static Boolean IsMenuWidget(
                        Widget w,
                        FileMgrRec * fileMgrRec,
                        FileMgrData * fileMgrData,
                        String * aName,
                        String * ftName) ;
static Boolean IsFilterIcon(
                        Widget w,
                        FileMgrData * fileMgrData,
                        String * ftName) ;
static Boolean ProcessItemHelp(
                        Widget shell) ;
static void ReusePrimaryHelpWindow(
                        DialogData * primaryHelpDialog,
                        char * idString,
                        char * volume,
                        char * topicTitle,
                        char * fileType,
                        int helpType) ;

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/



/************************************************************************
 *
 *   The main help callback
 *
 ************************************************************************/

void 
#ifdef _NO_PROTO
HelpRequestCB( w, clientData, callData )
        Widget w ;
        XtPointer clientData ;
        XtPointer callData ;
#else
HelpRequestCB(
        Widget w,
        XtPointer clientData,
        XtPointer callData )
#endif /* _NO_PROTO */

{
   XtPointer recordStruct;
   DialogData * dialogData;
   Arg args[8];
   int n;
   String topicTitle;
   int helpType;
   Boolean freeClientData;
   String fileType;
   String strVal;
   char *vol;
   char *locId;
#ifdef _TT_MULTI_SCREEN 
   int scr = XScreenNumberOfScreen(XtScreen(w));
#endif

   /* Refresh the display */
   /* printf ("in HelpRequestCB: clientdata=\"%s\"\n",(char *)clientData); */
   XmUpdateDisplay(w);

   if (recordStruct = LocateRecordStructure(w))
   {
      if (dialogData = _DtGetInstanceData(recordStruct))
      {
         if (IsMainWinDialog(dialogData))
         {
            FileMgrData * fileMgrData = (FileMgrData *)dialogData->data;
            FileMgrRec * fileMgrRec = (FileMgrRec *)recordStruct;

            /* Check for item help */
            if (clientData && strcmp(clientData, HELP_HELP_MODE_STR) == 0)
            {
              if( !ProcessItemHelp( fileMgrRec->shell ) )
              {
                char *tmpStr, *title, *msg;

                tmpStr = GetSharedMessage(ITEM_HELP_ERROR_TITLE);
                title = XtNewString(tmpStr);

                if( fileMgrData->toolbox )
                {
                  tmpStr = GetSharedMessage( AMITEM_HELP_ERROR );
                  msg = XtNewString(tmpStr);
                }
                else
                {
                  tmpStr = GetSharedMessage( ITEM_HELP_ERROR ),
                  msg = XtNewString(tmpStr);
                }

                _DtMessage(fileMgrRec->shell, title, msg, NULL,
                           HelpRequestCB);
                XtFree(title);
                XtFree(msg);
              }
              return;
            }
            /* Item help comes in with a NULL client data */
            if (clientData)
            {
               fileType = NULL;
               topicTitle = NULL;
               helpType = DtHELP_TYPE_TOPIC;
               freeClientData = False;
            }
            else
            {
               if(fileMgrData->selected_file_count > 1)
               {
                 Widget warn;
                 String s;
                 XmString xmstr;

                 n=0;
                 XtSetArg(args[n], XmNokLabelString, okXmString);                        n++;
                 XtSetArg(args[n], XmNcancelLabelString, cancelXmString);                n++;
                 s = GETMESSAGE(29,10, "Help is not available for multiple selected objects.");
                 xmstr = XmStringCreateLtoR(s, XmFONTLIST_DEFAULT_TAG);

                 XtSetArg(args[n], XmNmessageString, xmstr);                             n++;
                 XtSetArg(args[n], XmNtitle, (GETMESSAGE(29,1, "File Manager Help")));   n++;

                 warn = XmCreateWarningDialog(fileMgrRec->shell, "warn_msg", args, n);

                 XtUnmanageChild(XmMessageBoxGetChild(warn,XmDIALOG_HELP_BUTTON));
                 XtManageChild(warn);
                 XmStringFree(xmstr);

                 return;
               }

               /* Action/Filetype Help */
               topicTitle = GetSharedMessage(ACTION_FT_HELP_TITLE);
               helpType = DtHELP_TYPE_DYNAMIC_STRING;
               fileType = NULL;

               /* Determine string, and set clientData accordingly */
               if ((clientData =
                      MapIconWidgetToFileType(w, fileMgrData)) == NULL)
               {
                  /* Not a file icon; is it an action menu item? */
                  if (!IsMenuWidget(w, fileMgrRec, fileMgrData, &strVal, 
                                    &fileType))
                  {
                     if (! IsFilterIcon(w, fileMgrData, &strVal))
                        return;
                  }
                  clientData = (XtPointer)strVal;
               }
               freeClientData = True;
            }

            /* printf ("topic = %s\n", clientData); */

            /* Special check for 'Using Help'; required different volume */
            locId = (char *)clientData;
#ifdef _TT_MULTI_SCREEN
            if ((w == gMenuWidgets[scr].usingHelp) && (strcmp(locId, HELP_HOME_TOPIC) == 0))
#else
            if ((w == *usingHelp) && (strcmp(locId, HELP_HOME_TOPIC) == 0))
#endif
               vol = "Help4Help";
            else
               vol = fileMgrData->helpVol;

            if (fileMgrData->primaryHelpDialog)
            {
               ReusePrimaryHelpWindow( fileMgrData->primaryHelpDialog, 
                                       clientData, vol, topicTitle,
                                       fileType, helpType);
            }
            else
            {
               /* Create the main help window for this view */
               ShowHelpDialog(fileMgrRec->shell, fileMgrRec, MAIN_HELP_DIALOG, 
                              NULL, clientData, vol,
                              topicTitle, fileType, helpType);
            }

            if (freeClientData)
            {
               XtFree(fileType);
               XtFree(clientData);
            }
         }
      }
   }
      else if (clientData)
      {
         Widget mainHelpDialog=NULL;
         Arg args[10];
         int i=0;

         XtSetArg(args[0],XmNuserData,&mainHelpDialog);
         XtGetValues(w,args,1);

         XtSetArg(args[i], DtNhelpType,(unsigned char) DtHELP_TYPE_TOPIC); i++;
         XtSetArg(args[i], DtNhelpVolume, DTFILE_HELP_NAME); i++;
         XtSetArg(args[i], DtNlocationId, clientData); i++;
         XtSetArg(args[i], XmNtitle, (GETMESSAGE(29,1, "File Manager Help")));i++;
         if(!mainHelpDialog || ( mainHelpDialog && !XtIsManaged(mainHelpDialog)) )
	 {

             mainHelpDialog = DtCreateHelpDialog(w, "mainHelpDialog", args, i);
             XtAddCallback(mainHelpDialog, DtNcloseCallback,
                       closeCB_mainHelpDialog,
                       (XtPointer)NULL);
             XtManageChild(mainHelpDialog);
	     XtSetArg(args[0],XmNuserData,mainHelpDialog);
	     XtSetValues(w,args,1);
	 }
	 else
           XtSetValues(mainHelpDialog, args, i);
      }
}


/************************************************************************
 *
 *   The main help callback for desktop items
 *
 ************************************************************************/

void 
#ifdef _NO_PROTO
DTHelpRequestCB( w, clientData, callData )
        Widget w ;
        XtPointer clientData ;
        XtPointer callData ;
#else
DTHelpRequestCB(
        Widget w,
        XtPointer clientData,
        XtPointer callData )
#endif /* _NO_PROTO */

{
   DesktopRec * dtInfo;
   Arg args[8];
   int n,i;
   String topicTitle;
   int helpType;
   String filetypeOrActionName;
   String fileType = NULL;
#ifdef _TT_MULTI_SCREEN
   DesktopData* desktop_data;
   int scr;

   if ( XmIsGadget(w) )
       scr = XScreenNumberOfScreen(XtScreen(XtParent(w)));
   else
       scr = XScreenNumberOfScreen(XtScreen(w));
   
   desktop_data = gDesktopData[scr];
#endif

   /* Refresh the display */
   XmUpdateDisplay(w);

   /* 
    * For all of the following cases, the desktop information is
    * attached as userData to the direct child of the closest shell.
    */
   if ((dtInfo = (DesktopRec *)LocateRecordStructure(w)) == NULL)
      return;

   /*
    * If the clientData is not NULL, then it represents the topic string.
    */
   if (clientData)
   {
      helpType = DtHELP_TYPE_TOPIC;
      filetypeOrActionName = clientData;
   }
   else
   {
      /* Context sensitive help (menu items or the file icon) */
      if (XtParent(w) == desktop_data->popupMenu->popup)
      {
         /* 
          * One of the action related menu items. The command
          * string for the action is attached as userData.
          */
         topicTitle = GetSharedMessage(ACTION_FT_HELP_TITLE);
         helpType = DtHELP_TYPE_DYNAMIC_STRING;
         XtSetArg(args[0], XmNuserData, &filetypeOrActionName);
         XtGetValues(w, args, 1);

         /* Get the filetype from the active item */
         fileType = dtInfo->file_view_data->file_data->logical_type;
      }
      else
      {
         /* The desktop icon itself */
         topicTitle = GetSharedMessage(ACTION_FT_HELP_TITLE);
         helpType = DtHELP_TYPE_DYNAMIC_STRING;

         /* Determine string, and set clientData accordingly */
         filetypeOrActionName = dtInfo->file_view_data->file_data->logical_type;
      }
   }

   /* printf ("topic = %s\n", filetypeOrActionName); */
   for(i=0;i<desktop_data->numWorkspaces;i++)
     if(desktop_data->workspaceData[i]->number == dtInfo->workspace_num)
       break;
   if(desktop_data->workspaceData[i]->primaryHelpDialog)
   {
      ReusePrimaryHelpWindow(desktop_data->workspaceData[i]->primaryHelpDialog,
                             filetypeOrActionName, DTFILE_HELP_NAME, 
                             topicTitle, fileType, helpType);
   }
   else
   {
      /* Create the main help window for this workspace */
      ShowDTHelpDialog(NULL, i, MAIN_HELP_DIALOG, 
                       NULL, filetypeOrActionName, DTFILE_HELP_NAME,
                       topicTitle, fileType, helpType);
   }
}



/************************************************************************
 *
 *   The trash window's help callback
 *
 ************************************************************************/

void 
#ifdef _NO_PROTO
TrashHelpRequestCB( w, clientData, callData )
        Widget w ;
        XtPointer clientData ;
        XtPointer callData ;
#else
TrashHelpRequestCB(
        Widget w,
        XtPointer clientData,
        XtPointer callData )
#endif /* _NO_PROTO */

{
   Arg args[5];
   char *vol;
   char *locId;

   /* Refresh the display */
   XmUpdateDisplay(w);

   /* printf ("topic = %s\n", clientData); */

   /* Check for item help */
   if (strcmp(clientData, HELP_HELP_MODE_STR) == 0)
   {
      if( ! ProcessItemHelp(trashShell) )
      {
        char *tmpStr, *title, *msg;

        tmpStr = GetSharedMessage(ITEM_HELP_ERROR_TITLE);
        title = XtNewString(tmpStr);

        tmpStr = GetSharedMessage( ITEM_HELP_ERROR ),
        msg = XtNewString(tmpStr);

        _DtMessage(trashShell, title, msg, NULL,
                   HelpRequestCB);
        XtFree(title);
        XtFree(msg);
      }
      return;
   }

   /* Special check for 'Using Help'; required different volume */
   locId = (char *)clientData;
   if ((w == *usingHelpTrash) && (strcmp(locId, HELP_HOME_TOPIC) == 0))
      vol = "Help4Help";
   else
      vol = DTFILE_HELP_NAME;

   if (primaryTrashHelpDialog)
   {
      ReusePrimaryHelpWindow(primaryTrashHelpDialog,
                             clientData, vol, NULL, NULL, DtHELP_TYPE_TOPIC);
   }
   else
   {
      /* Create the main help window for this view */
      ShowTrashHelpDialog(trashShell, MAIN_HELP_DIALOG, NULL, clientData, 
                          vol);
   }
}


/*
 * Given a widget, trace up through its ancestors, until you come to the
 * shell.  Get the userData from the shell's child; this gets the 'record' 
 * structure, which can then be used to determine which view's help window 
 * to use.
 */

XtPointer 
#ifdef _NO_PROTO
LocateRecordStructure( w )
        Widget w ;
#else
LocateRecordStructure(
        Widget w )
#endif /* _NO_PROTO */

{
   Arg args[2];
   XtPointer recordPtr;

   while (XtParent(w) && !XtIsShell(XtParent(w)))
      w = XtParent(w);

   if (XtParent(w))
   {
      XtSetArg(args[0], XmNuserData, &recordPtr);
      XtGetValues(w, args, 1);
      return(recordPtr);
   }

   return(NULL);
}


/*
 *  Check to see if this is the dialog data for one of the top level
 * application windows.
 */

static Boolean 
#ifdef _NO_PROTO
IsMainWinDialog( dialogData )
        DialogData * dialogData ;
#else
IsMainWinDialog(
        DialogData * dialogData )
#endif /* _NO_PROTO */

{
   int i;

   for (i = 0; i < view_count; i++)
   {
      if (view_set[i]->dialog_data == (XtPointer)dialogData)
         return(True);
   }

   if( (FileMgrData *)dialogData->data == trashFileMgrData)
	return(True);
   return(False);
}


/*
 * The application must free up the returned string.
 */

static XtPointer
#ifdef _NO_PROTO
MapIconWidgetToFileType(w, fileMgrData)
   Widget w;
   FileMgrData * fileMgrData;
#else
MapIconWidgetToFileType(
   Widget w,
   FileMgrData * fileMgrData )
#endif /* _NO_PROTO */

{
   int i;
   int j;
   int directoryCount;
   char * ftName;

   /* Based on whether in directory graph mode */
   if (fileMgrData->show_type == SINGLE_DIRECTORY)
      directoryCount = 1;
   else
      directoryCount = fileMgrData->directory_count;

   for (i = directoryCount - 1; i >= 0; i--)
   {
      for (j = 0; j < fileMgrData->directory_set[i]->file_count; j++)
      {
         if (w == fileMgrData->directory_set[i]->file_view_data[j]->widget)
         {
            ftName = 
     fileMgrData->directory_set[i]->file_view_data[j]->file_data->logical_type;
            return (XtNewString(ftName));
         }
      }
   }

   /* The icon was unknown; should never happen */
   return(NULL);
}



/*
 * This function takes a widget, and determines if it is one of the
 * menu buttons in the action menupane, or the action popup.  If so, 
 * then it returns the action name and the associated filetype string.
 *
 * The caller must free up the returned string values.
 */

static Boolean
#ifdef _NO_PROTO
IsMenuWidget(w, fileMgrRec, fileMgrData, aName, ftName)
   Widget w;
   FileMgrRec * fileMgrRec;
   FileMgrData * fileMgrData;
   String * aName;
   String * ftName;
#else
IsMenuWidget(
   Widget w,
   FileMgrRec * fileMgrRec,
   FileMgrData * fileMgrData,
   String * aName,
   String * ftName )
#endif /* _NO_PROTO */

{
   int i;
   CompositeWidget action_pane = (CompositeWidget)fileMgrRec->action_pane;
   String actionName;
   Arg args[2];
#ifdef _TT_MULTI_SCREEN
   int scr;

   if ( XmIsGadget(w) )
       scr = XScreenNumberOfScreen(XtScreen(XtParent(w)));
   else
       scr = XScreenNumberOfScreen(XtScreen(w));
#endif

   if (action_pane == NULL)
      return(False);

   /* First, check if this is a menubar item */
   for (i = 0; i < action_pane->composite.num_children; i++)
   {
      if (w == action_pane->composite.children[i])
      {
         XtSetArg(args[0], XmNuserData, &actionName);
         XtGetValues(w, args, 1);
         *aName = XtNewString(actionName);

         /* Get the filetype from the active item */
         *ftName = XtNewString(
                     fileMgrData->selection_list[0]->file_data->logical_type);
         return (True);
      }
   }

   /* Secondly, check if this is a fileMgr popup item */
#ifdef _TT_MULTI_SCREEN
   if (XtParent(w) == gFileMgrPopup[scr].menu)
#else
   if (XtParent(w) == fileMgrPopup.menu)
#endif
   {
      XtSetArg(args[0], XmNuserData, &actionName);
      XtGetValues(w, args, 1);
      *aName = XtNewString(actionName);

      /* Get the filetype from the active item */
      *ftName = XtNewString(
                  fileMgrData->popup_menu_icon->file_data->logical_type);
      fileMgrData->popup_menu_icon = NULL;
      return (True);
   }

   /* This was not a menu item */
   return(False);
}



/*
 * This function takes a widget, and determines if it is one of the
 * filetype icons in the filter dialog.  If so, then it returns the 
 * filetype string associated with the icon.
 *
 * The caller must free up the returned string values.
 */

static Boolean
#ifdef _NO_PROTO
IsFilterIcon(w, fileMgrData, ftName)
   Widget w;
   FileMgrData * fileMgrData;
   String * ftName;
#else
IsFilterIcon(
   Widget w,
   FileMgrData * fileMgrData,
   String * ftName )
#endif /* _NO_PROTO */

{
   Arg args[2];
   DialogData * filterEditDialogData;
   FilterData * filterData;
   FilterRec * filterRec;
   FTData * filterEntryData;

   filterEditDialogData = fileMgrData->filter_edit;
   if (!filterEditDialogData)
      return(False);

   filterData = (FilterData *)filterEditDialogData->data;

   if (!filterData->displayed)
      return(False);

   filterRec = (FilterRec *) _DtGetDialogInstance(filterEditDialogData);

   /* Check if the parent of the widget is the filter's drawingArea */
   if (XtParent(w) != filterRec->file_window)
      return(False);

   /*
    * Each icon in the filter dialog has attached as its userData a
    * pointer to a structure of information about the associated
    * filetype; get this structure, to get the filetype index.
    */
   XtSetArg(args[0], XmNuserData, &filterEntryData);
   XtGetValues(w, args, 1);
   *ftName = XtNewString(filterEntryData->filetype);
   return (True);
}


static Boolean
#ifdef _NO_PROTO
ProcessItemHelp( shell )
   Widget shell;
#else
ProcessItemHelp(
   Widget shell )
#endif /* _NO_PROTO */

{
   Widget selectedWidget;
   int returnVal;

   returnVal = DtHelpReturnSelectedWidgetId(shell, NULL, &selectedWidget);
   switch(returnVal)
   {  
      case DtHELP_SELECT_VALID:
      {
         while (!XtIsShell(selectedWidget))
         {
            if (XtHasCallbacks(selectedWidget, XmNhelpCallback)
                  == XtCallbackHasSome)
            {
               XtCallCallbacks(selectedWidget, 
                                  XmNhelpCallback, NULL);
               return True;
            }

            selectedWidget = XtParent(selectedWidget);
         }
         break;
      }

      case DtHELP_SELECT_INVALID:
         return False;

      case DtHELP_SELECT_ABORT:
      case DtHELP_SELECT_ERROR:
      default:
         break;
   }
   return True;
}


static void
#ifdef _NO_PROTO
ReusePrimaryHelpWindow( primaryHelpDialog, idString, volume, topicTitle,
                        fileType, helpType  )
   DialogData * primaryHelpDialog;
   char * idString;
   char * volume;
   char * topicTitle;
   char * fileType;
   int helpType;
#else
ReusePrimaryHelpWindow(
   DialogData * primaryHelpDialog,
   char * idString,
   char * volume,
   char * topicTitle,
   char * fileType,
   int helpType )
#endif /* _NO_PROTO */

{
   HelpData * helpData;
   HelpRec * helpRec;
   String helpString = NULL;
   int n;
   Arg args[10];

   /* Reuse the existing main help window for this view */
   helpData = (HelpData *)primaryHelpDialog->data;
   XtFree(helpData->idString);
   XtFree(helpData->volString);
   XtFree(helpData->topicTitle);
   XtFree(helpData->fileType);
   helpData->idString = XtNewString(idString);
   helpData->volString = XtNewString(volume);
   helpData->topicTitle = XtNewString(topicTitle);
   helpData->fileType = XtNewString(fileType);
   helpData->helpType = helpType;
   helpRec = (HelpRec *) _DtGetDialogInstance(primaryHelpDialog);
   n = 0;
   XtSetArg(args[n], DtNhelpVolume, volume); n++;
   XtSetArg(args[n], DtNhelpType, (unsigned char )helpType); n++;
   if (helpType == DtHELP_TYPE_TOPIC)
   {
      XtSetArg(args[n], DtNlocationId, helpData->idString); n++;
   }
   else
   {
      helpString = MapFileTypeToHelpString(helpData->idString,
                                           helpData->fileType);
      XtSetArg(args[n], DtNstringData, helpString); n++;
      XtSetArg(args[n], DtNtopicTitle, helpData->topicTitle); n++;
   }
   XtSetValues(helpRec->helpDialog, args, n);
   XtFree(helpString);
}

void 
#ifdef _NO_PROTO
ObjectHelp( w, clientData, callData )
        Widget w ;
        XtPointer clientData ;
        XtPointer callData ;
#else
ObjectHelp(
        Widget w,
        XtPointer clientData,
        XtPointer callData )
#endif /* _NO_PROTO */

{
  XtCallCallbacks((Widget)clientData, XmNhelpCallback, NULL);
}


/******************************************************************************/
/*                                                                            */
/* closeCB_mainHelpDialog                                                     */
/*                                                                            */
/* INPUT:  Widget wid - widget id                                             */
/*         XtPointer cd - client data                                         */
/*         XtPointer cbs - callback data                                      */
/* OUTPUT: none                                                               */
/*                                                                            */
/******************************************************************************/

void
#ifdef _NO_PROTO
closeCB_mainHelpDialog(wid, client_data, cbs)
     Widget wid;
     XtPointer client_data;
     XtPointer cbs;
#else /* _NO_PROTO */
closeCB_mainHelpDialog(Widget wid, XtPointer client_data,
                           XtPointer cbs)
#endif /* _NO_PROTO */
{
  XtDestroyWidget(wid);
}

