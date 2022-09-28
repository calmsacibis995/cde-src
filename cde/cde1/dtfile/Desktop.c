/* $XConsortium: Desktop.c /main/cde1_maint/3 1995/09/29 18:34:02 lehors $ */
/************************************<+>*************************************
 ****************************************************************************
 *   FILE:           Desktop.c
 *
 *   COMPONENT_NAME: Desktop File Manager (dtfile)
 *
 *   DESCRIPTION:    Contains functions related to Desktop operations.
 *
 *   FUNCTIONS: BuildDesktopLinks
 *		BuildNewOrderlist
 *		CalculateRootCoordinates
 *		CheckDesktopMarquee
 *		CleanUpWSName
 *		CreatePopupMenu
 *		DTActionCallback
 *		DTFileIsSelected
 *		DeselectAllDTFiles
 *		DeselectDTFile
 *		DesktopObjectChanged
 *		DesktopObjectRemoved
 *		DrawAInput
 *		DropOnDesktopObject
 *		FreeCachedIcons
 *		FreeDesktopWindow
 *		FreeUpOldWorkspace
 *		GenerateShape
 *		GetWorkspaceNum
 *		InitializeDesktopGrid
 *		InitializeDesktopWindows
 *		InitializeNewWorkspaces
 *		IsAFileOnDesktop2
 *		LoadDesktopInfo
 *		MakeDesktopWindow
 *		OpenFolder
 *		ProcessBufferDropOnFolderDT
 *		ProcessDTSelection
 *		ProcessDropOnDesktopObject
 *		ProcessMoveCopyLinkDT
 *		ProcessNewViewDT
 *		PutOnDTCB
 *		RegisterIconDropsDT
 *		RegisterInGrid
 *		RemoveDT
 *		RemoveMovedObjectFromDT
 *		RunDTCommand
 *		SaveDesktopInfo
 *		SelectDTFile
 *		SetupDesktopWindow
 *		UnpostDTTextField
 *		WorkSpaceRemoved
 *		popupMenu
 *		renameDT
 *		PutDesktopHandler
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>

#include <Xm/XmP.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <X11/ShellP.h>
#include <X11/Shell.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#ifdef SHAPE
#include <X11/extensions/shape.h>
#endif

#include <Xm/DrawingA.h>
#include <Xm/Frame.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeBG.h>
#include <Xm/SeparatoG.h>
#include <Xm/VirtKeys.h>

#include <Dt/Icon.h>
#include <Dt/IconP.h>
#include <Dt/IconFile.h>
#include <Dt/Connect.h>
#include <Dt/Wsm.h>
#include <Dt/WsmM.h>
#include <Dt/UserMsg.h>
#include <Dt/DtNlUtils.h>

#include <Xm/DragIcon.h>
#include <Xm/DragC.h>
#include <Dt/Dnd.h>

#include <Dt/ActionP.h>
#include <Dt/Action.h>
#include <Dt/Session.h>

#include <Tt/tttk.h>

#include "Encaps.h"
#include "SharedProcs.h"
#include "Desktop.h"
#include "FileMgr.h"
#include "Main.h"
#include "Prefs.h"
#include "Help.h"
#include "SharedMsgs.h"

extern char *dt_path;

/* Don't use '#define'; leads to multiple copies of the string */
static char DESKTOP_SAVE_NAME[] =  ".!dtdesktop";

#ifdef _TT_MULTI_SCREEN
static char* 	GetDesktopFilename(int);
DesktopData** 	gDesktopData;
Widget* 	gTopWidget;
#else
DesktopData *desktop_data;
#endif
Widget widget_dragged;
DesktopRec *sacredDesktop;
Boolean *desktop_grid;
unsigned short int desktop_grid_size;

#ifdef _NO_PROTO

static void ProcessNewViewDT();
static void ProcessMoveCopyLinkDT();
static void ProcessBufferDropOnFolderDT();
static FileViewData * BuildNewOrderlist();
static void FreeDesktopWindow ();
static void FreeCachedIcons();
static void CreatePopupMenu ();
static void popupMenu ();
static void OpenFolder ();
static void renameDT ();
static void BuildDesktopLinks () ;
static int InitializeNewWorkspaces () ;
static void DropOnDesktopObject ();
static void DrawAInput ();
static void CalculateRootCoordinates ();
static int GetWorkspaceNum();
static void FreeUpOldWorkspace();

#else

static void ProcessNewViewDT ( DesktopRec *desktopWindow ) ;
static void ProcessMoveCopyLinkDT ( char *command,
                                    DesktopRec *desktopWindow,
                                    DtDndDropCallbackStruct *drop_parameters ) ;
static void ProcessBufferDropOnFolderDT ( char *command,
                                    DesktopRec *desktopWindow,
                                    DtDndDropCallbackStruct *drop_parameters ) ;
static FileViewData * BuildNewOrderlist(
                      FileViewData *order_list,
                      Widget icon,
                      char *hostName,
                      char *dirName,
                      char *file,
                      Boolean is_registered,
                      Boolean IsToolBox) ;
static void FreeDesktopWindow ( DesktopRec *desktopWindow ) ;
#ifdef _TT_MULTI_SCREEN
static void FreeCachedIcons ( int iconsToBeFreed , int scr) ;
#else
static void FreeCachedIcons ( int iconsToBeFreed ) ;
#endif
static void CreatePopupMenu ( Widget drawA ) ; 
static void popupMenu (
                      Widget w,
                      XtPointer client_data,
                      XtPointer call_data) ;
static void OpenFolder (
                      Widget w,
                      XtPointer client_data,
                      XtPointer call_data) ;
static void renameDT (
                      Widget w,
                      XtPointer client_data,
                      XtPointer call_data) ;
#ifdef _TT_MULTI_SCREEN
static void BuildDesktopLinks ( Display *display, int scr ) ;
#else
static void BuildDesktopLinks ( Display *display ) ;
#endif
static int InitializeNewWorkspaces (
                      Display *display,
                      char *workspaceName) ;
static void DropOnDesktopObject (
                      Widget w,
                      XtPointer client_data,
                      XtPointer call_data) ;
static void DrawAInput (
                      Widget w,
                      XtPointer client_data,
                      XEvent *event) ;
static void CalculateRootCoordinates (
                      int ws_num,
                      int width,
                      int height,
                      int *root_x,
                      int *root_y) ;
#ifdef _TT_MULTI_SCREEN 
static int GetWorkspaceNum(
                      char *workspace_name,
                      int NumWorkspaces,
                      int *workspace_index, 
                      int scr);
static void FreeUpOldWorkspace(
                      int oldNumWorkspaces,
                      int old_ws_num,
                      int ws_num,
                      int old_index,
                      int new_index, 
                      int scr);
#else
static int GetWorkspaceNum(
                      char *workspace_name,
                      int NumWorkspaces,
                      int *workspace_index);
static void FreeUpOldWorkspace(
                      int oldNumWorkspaces,
                      int old_ws_num,
                      int ws_num,
                      int old_index,
                      int new_index);
#endif /* _TT_MULTI_SCREEN */

#endif /* _NO_PROTO */

#define NULL_STRING "NULL"


/************************************************************************
 *
 *  InitializeDesktopWindows
 *
 ************************************************************************/

void
#ifdef _NO_PROTO
InitializeDesktopWindows(number_cache, display)
       int number_cache;
       Display *display;
#else
InitializeDesktopWindows(
       int number_cache,
       Display *display)
#endif /* _NO_PROTO */
{
   DesktopRec *desktop;

   int n, i, new_size;
   Arg args[20];
   char   *popup_name;
   Widget pu_shell, frame, drawA;
   XtTranslations trans_table;

#ifdef _TT_MULTI_SCREEN
   DesktopData*	desktop_data;
   int scr;
   Screen* screen;

   for (scr = 0; scr < gNScreens; scr++)
   {
      if(gDesktopData[scr] == NULL)
      {
         gDesktopData[scr] = (DesktopDataPtr)XtMalloc(sizeof(DesktopData));
         gDesktopData[scr]->desktopWindows =
           (DesktopRec ** )XtMalloc(sizeof(DesktopRec *) * number_cache);

         gDesktopData[scr]->popupMenu = (PopupRec *)XtMalloc(sizeof(PopupRec));
         gDesktopData[scr]->popupMenu->popup = NULL;
         gDesktopData[scr]->popupMenu->action_pane_file_type = NULL;
         gDesktopData[scr]->numCachedIcons = 0;
         gDesktopData[scr]->popup_name_count = 0;
         gDesktopData[scr]->numIconsUsed = 0;
         new_size = number_cache;

         /*
          *  Build desktop directories and get the number of workspaces for
          *  this screen.
          */
         BuildDesktopLinks(display, scr);
      }
      else
      {
         new_size = number_cache + gDesktopData[scr]->numCachedIcons +
           gDesktopData[scr]->numIconsUsed;
         gDesktopData[scr]->desktopWindows =
           (DesktopRec**)XtRealloc((char*) gDesktopData[scr]->desktopWindows,
                                   sizeof(DesktopRec *) * new_size);
      }
   }

#else
   if(desktop_data == NULL)
   {
      desktop_data = (DesktopDataPtr)XtMalloc(sizeof(DesktopData));
      desktop_data->desktopWindows =
               (DesktopRec ** )XtMalloc(sizeof(DesktopRec *) * number_cache);

      desktop_data->popupMenu = (PopupRec *)XtMalloc(sizeof(PopupRec));
      desktop_data->popupMenu->popup = NULL;
      desktop_data->popupMenu->action_pane_file_type = NULL;
      desktop_data->numCachedIcons = 0;
      desktop_data->popup_name_count = 0;
      desktop_data->numIconsUsed = 0;
      new_size = number_cache;

      /*
       *  Build desktop directories and get the number of workspaces for
       *  this screen.
       */
      BuildDesktopLinks(display);
   }
   else
   {
      new_size = number_cache + desktop_data->numCachedIcons +
                                             desktop_data->numIconsUsed;
      desktop_data->desktopWindows =
               (DesktopRec ** )XtRealloc((char *)desktop_data->desktopWindows,
                                            sizeof(DesktopRec *) * new_size);
   }

#endif /* _TT_MULTI_SCREEN */


#ifdef _TT_MULTI_SCREEN
   /* Do for each screen */
   for (scr = 0; 
        scr < gNScreens; 
        scr++ )
   {
      desktop_data = gDesktopData[scr];
#endif
   for(i = desktop_data->numCachedIcons + desktop_data->numIconsUsed;
                                                      i < new_size; i++)
   {
      desktop = (DesktopRec *)XtMalloc(sizeof(DesktopRec));
      popup_name = (char *)XtMalloc(strlen("popup_name") + 11);
      sprintf( popup_name, "popup_name%d", desktop_data->popup_name_count);

#ifdef _TT_MULTI_SCREEN
      /* Create desktop widget */
      n = 0;
      screen = ScreenOfDisplay(display, scr);
      XtSetArg (args[n], XmNscreen, screen); n++;
      gTopWidget[scr] = XtAppCreateShell ( "dtfile" , DTFILE_CLASS_NAME,
	  applicationShellWidgetClass, display, args, n );
#endif

      n = 0;
      XtSetArg (args[n], XmNmwmFunctions, NULL); n++;
      XtSetArg (args[n], XmNmwmDecorations, NULL); n++;
      XtSetArg (args[n], XmNallowShellResize, True); n++;
      XtSetArg (args[n], XmNvisual, CopyFromParent); n++;
#ifdef _TT_MULTI_SCREEN
      XtSetArg (args[n], XmNscreen, screen); n++;
      desktop->shell = pu_shell =
            XtCreatePopupShell(popup_name, topLevelShellWidgetClass,
		 gTopWidget[scr], args, n);
#else
      desktop->shell = pu_shell =
            XtCreatePopupShell(popup_name, topLevelShellWidgetClass,
                               toplevel, args, n);
#endif
      /* We need to save the background color for the popup shell.  When
       * we activate a text edit widget for the label of a workspace object,
       * the shell's background_pixel color will change.  If we don't save
       * the old background color and reuse the icon gadget, the background
       * will be different
       */
      XtSetArg(args[0], XmNbackground, &desktop->background);
      XtGetValues(desktop->shell, args, 1);
      XtFree(popup_name);
      popup_name = NULL;

      n = 0;
      XtSetArg (args[n], XmNmarginWidth, 1); n++;
      XtSetArg (args[n], XmNmarginHeight, 1); n++;
      XtSetArg (args[n], XmNshadowThickness, 2); n++;
      XtSetArg (args[n], XmNshadowType, XmSHADOW_OUT); n++;
      XtSetArg (args[n], XmNautoUnmanage, False); n++;
      XtSetArg (args[n], XmNbackgroundPixmap, XmUNSPECIFIED_PIXMAP); n++;
      desktop->frame = frame = XmCreateFrame(pu_shell, "frame", args, n);
      XtManageChild (frame);

      n = 0;
      XtSetArg (args[n], XmNmarginWidth, 1); n++;
      XtSetArg (args[n], XmNmarginHeight, 1); n++;
      XtSetArg (args[n], XmNshadowThickness, 0); n++;
      XtSetArg (args[n], XmNautoUnmanage, False); n++;
      XtSetArg (args[n], XmNbackgroundPixmap, XmUNSPECIFIED_PIXMAP); n++;
      desktop->drawA = drawA = XmCreateDrawingArea(frame, "drawA", args, n);
      XtManageChild (drawA);


      if(desktop_data->popupMenu->popup == NULL)
      {
         CreatePopupMenu(drawA);
         sacredDesktop = desktop;
      }
      else
         XmAddToPostFromList(desktop_data->popupMenu->popup, drawA);

      /* set up translations in main edit widget */
      trans_table = XtParseTranslationTable(translations_da);

      /* set up translations in main edit widget */
      XtOverrideTranslations(drawA, trans_table);

      /* Event handler for posting popup menu */
      XtAddEventHandler(drawA, ButtonPressMask, False,
                         (XtEventHandler)DrawAInput, (XtPointer)desktop);

      /* Callback for handling selection */
      XtAddCallback(drawA, XmNinputCallback,
                    FileWindowInputCallback, (XtPointer)desktop);

      /* Event handler for detecting drag threshold surpassed */
      XtAddEventHandler(drawA, Button1MotionMask|Button2MotionMask,
                        False, (XtEventHandler)FileIconMotion, NULL);

      n = 0;
      XtSetArg (args[n], XmNfillMode, XmFILL_TRANSPARENT);            n++;
      XtSetArg (args[n], XmNarmColor, white_pixel);                   n++;
#ifdef SHAPE
      XtSetArg (args[n], XmNshadowThickness, 2);                      n++;
#else
      XtSetArg (args[n], XmNshadowThickness, 0);                      n++;
#endif
      XtSetArg (args[n], XmNfontList, user_font);                     n++;
      XtSetArg (args[n], XmNspacing, 3);                              n++;
      if( keybdFocusPolicy == XmEXPLICIT)
      {
         XtSetArg (args[n], XmNtraversalOn, True);                    n++;

      }
      else
      {
         XtSetArg (args[n], XmNtraversalOn, False);                   n++;
         XtSetArg (args[n], XmNhighlightThickness, 0);                n++;
      }
      XtSetArg (args[n], XmNborderType, DtNON_RECTANGLE);            n++;
      XtSetArg (args[n], "drawShadow", True);                         n++;
      if(desktopIconType == LARGE)
         XtSetArg (args[n], XmNpixmapPosition, XmPIXMAP_TOP);
      else
         XtSetArg (args[n], XmNpixmapPosition, XmPIXMAP_LEFT);
      n++;
      desktop->iconGadget = _DtCreateIcon (drawA, "desktop_icon", args, n);
      XtManageChild (desktop->iconGadget);
      XtAddCallback (desktop->iconGadget, XmNhelpCallback,
                     (XtCallbackProc)DTHelpRequestCB, NULL);

      desktop->view = UNSET_VALUE;
      desktop->order = UNSET_VALUE;
      desktop->direction = UNSET_VALUE;
      desktop->positionEnabled = UNSET_VALUE;
      desktop->restricted_directory = NULL;
      desktop->root_type = NULL;
      desktop->title=  NULL;
      desktop->helpVol = NULL;
      desktop->toolbox = False;
      desktop->registered = False;
      desktop->text = NULL;
      desktop->root_x = -1;
      desktop->root_y = -1;
      desktop->workspace_name = NULL;
      desktop->workspace_num = 0;
      desktop->file_name = NULL;

      desktop_data->numCachedIcons++;
      desktop_data->popup_name_count++;

      desktop_data->desktopWindows[i] = desktop;
   }

#ifdef _TT_MULTI_SCREEN
 } /* for-loop */
#endif

}


void
#ifdef _NO_PROTO
RegisterIconDropsDT(desktopWindow)
      DesktopRec *desktopWindow;
#else
RegisterIconDropsDT(
      DesktopRec *desktopWindow)
#endif /* _NO_PROTO */
{
   XSync(XtDisplay(desktopWindow->shell), 0);

   SetHotRects(desktopWindow->file_view_data, desktopWindow->root_type,
               DropOnDesktopObject,
               (XtPointer) desktopWindow);

   XtRemoveAllCallbacks(desktopWindow->iconGadget, XmNcallback);
   XtAddCallback (desktopWindow->iconGadget,
                  XmNcallback,
                  (XtCallbackProc)IconCallback,
                  desktopWindow->file_view_data);
}


/**********************************************************************
 *
 *  BuildNewOrderlist - builds a new file_view for a Desktop object.
 *          If the order_list is NULL, then stat the file and build it
 *          from there. If order_list in not NULL then use it to build the
 *          new file_view.
 *
 **********************************************************************/
static FileViewData *
#ifdef _NO_PROTO
BuildNewOrderlist(order_list,icon,hostName,dirName,file,is_registered,IsToolBox)
     FileViewData *order_list;
     Widget icon;
     char *hostName;
     char *dirName;
     char *file;
     Boolean is_registered;
     Boolean IsToolBox;
#else
BuildNewOrderlist(
     FileViewData *order_list,
     Widget icon,
     char *hostName,
     char *dirName,
     char *file,
     Boolean is_registered,
     Boolean IsToolBox)
#endif /* _NO_PROTO */
{
   FileViewData *file_view_data;
   FileData *file_data;
   char *full_dir_name;

   file_view_data = (FileViewData *)XtMalloc(sizeof(FileViewData));
   file_view_data->directory_set =
                      (XtPointer)XtMalloc(sizeof(DirectorySet));

   ((DirectorySet *)file_view_data->directory_set)->file_count = 1;
   ((DirectorySet *)file_view_data->directory_set)->file_view_data = NULL;
   ((DirectorySet *)file_view_data->directory_set)->order_list = NULL;
   ((DirectorySet *)file_view_data->directory_set)->filtered_file_count = 0;
   ((DirectorySet *)file_view_data->directory_set)->invisible_file_count = 0;
   ((DirectorySet *)file_view_data->directory_set)->file_mgr_data = NULL;

   file_view_data->parent =
     file_view_data->next =
     file_view_data->desc = NULL;
   file_view_data->filtered = False;
   file_view_data->displayed = True;
   file_view_data->registered = is_registered;
   file_view_data->ts = tsNotRead;
   file_view_data->ndir =
     file_view_data->nfile =
     file_view_data->nnew = 0;
   file_view_data->widget = icon;
   file_view_data->treebtn = NULL;
   file_view_data->position_info = NULL;

   if(order_list != NULL)
   {
      file_view_data->file_data = file_data =
         (FileData *)XtMalloc(sizeof(FileData));

      file_data->link = NULL;
      file_data->next = NULL;
      file_data->is_subdir = False;
#ifdef  SUN_ACLFS
      file_data->has_acl  = False;
#endif  /* SUN_ACLFS */
      file_data->is_broken = False;

      file_data->file_name = XtNewString(order_list->file_data->file_name);
      file_data->action_name = XtNewString(order_list->file_data->action_name);
      file_data->physical_type = order_list->file_data->physical_type;
      file_data->logical_type = order_list->file_data->logical_type;
      file_data->errno = order_list->file_data->errno;
      file_data->stat = order_list->file_data->stat;

      ((DirectorySet *)file_view_data->directory_set)->name =
         XtNewString( ((DirectorySet *)order_list->directory_set)->name );
   }
   else
   {
      Tt_status tt_status;

      full_dir_name = ResolveLocalPathName(hostName, dirName, NULL,
                                           home_host_name, &tt_status);
      if( TT_OK == tt_status )
      {
        FileData2 file_data2;
        int n;

        DtEliminateDots (full_dir_name);

        if (strcmp(file, ".") == 0) {
           ReadFileData2(&file_data2, full_dir_name, NULL,IsToolBox);
        } else {
           ReadFileData2(&file_data2, full_dir_name, file,IsToolBox);
        }
        file_view_data->file_data = FileData2toFileData(&file_data2, &n);
        XtFree(full_dir_name);
        full_dir_name = NULL;
      }

      ((DirectorySet *)file_view_data->directory_set)->name =
                                                         XtNewString(dirName);
   }

   return(file_view_data);
}


/************************************************************************
 *
 *  DesktopObjectRemoved()
 *    Called when it is detected that an object that a desktop icon
 *    refers to no longer exists.  Removes the icon window from the
 *    screen and removes the object from the list of desktop objects.
 *
 ***********************************************************************/

void
#ifdef _NO_PROTO
DesktopObjectRemoved(desktopWindow)
   DesktopRec *desktopWindow;
#else
DesktopObjectRemoved( DesktopRec *desktopWindow )
#endif /* _NO_PROTO */
{
  WorkspaceRec * workSpace;
  int i, j, k, l;
#ifdef _TT_MULTI_SCREEN
  DesktopData* desktop_data;
  int scr;
#endif

  /* Remove the object from the screen*/
  XtPopdown(desktopWindow->shell);
  XWithdrawWindow(XtDisplay(desktopWindow->shell),
                  XtWindow(desktopWindow->shell),
                  XDefaultScreen(XtDisplay(desktopWindow->shell)));

  /*
    File manager has 2 lists to deal with when a file is on the desktop
    1. desktop_data->desktopWindows and
    2. desktop_data->workspaceData[n]->selectedDTWindows

    list 1 contains all files that are on desktop.
    list 2 contains all files on desktop that are SELECTED.

    At this point, for some reason, the file is no longer present
      in the system. Remove it/them from both lists.

      NOTE: This is very inefficient code but have no other
      implementation choices.  Prefer trash to do it, but all trash
      handlers are designed to accept ICCCM message for deleting a file.
      Prefer the operating system to notify when a file is being deleted,
      but..
  */

#ifdef _TT_MULTI_SCREEN
 for(scr = 0; scr < gNScreens; scr++)
 {
  desktop_data = gDesktopData[scr];
#endif

  for( j = 0; j < desktop_data->numWorkspaces; ++j )
  {
    workSpace = desktop_data->workspaceData[j];
    for( k = 0; k < workSpace->files_selected; ++k )
    {
      if( workSpace->selectedDTWindows[k] == desktopWindow )
      {
        for( l = k; l < workSpace->files_selected; ++l )
        {
          workSpace->selectedDTWindows[l] =
            workSpace->selectedDTWindows[l+1];
        }
        --(workSpace->files_selected);
        break;
      }
    }
  }
#ifdef _TT_MULTI_SCREEN
 }
#endif

  FreeDesktopWindow(desktopWindow);
}


/************************************************************************
 *
 *  DesktopObjectChanged()
 *    Called when is detected that an object that a desktop icon
 *    refers has changed its data type.  Updates the icon pixmap to
 *    reflect the new type.
 *
 ***********************************************************************/

void
#ifdef _NO_PROTO
DesktopObjectChanged(desktopWindow)
   DesktopRec *desktopWindow;
#else
DesktopObjectChanged( DesktopRec *desktopWindow )
#endif /* _NO_PROTO */
{
  FileData *file_data;
  Arg args[1];
  PixmapData *pixmapData;

  /* remove text widget, if any */
  if(desktopWindow->text)
  {
    XtUnmanageChild(desktopWindow->text);
    XtDestroyWidget(desktopWindow->text);
    desktopWindow->text = NULL;
  }

  /* update icon pixmap */
  file_data = desktopWindow->file_view_data->file_data;
  pixmapData = _DtRetrievePixmapData(
                                     file_data->logical_type,
                                     file_data->file_name,
                                     desktopWindow->dir_linked_to,
                                     desktopWindow->shell,
                                     desktopIconType);

  if (pixmapData)
    XtSetArg (args[0], XmNimageName, pixmapData->iconFileName);
  else
    XtSetArg(args[0], XmNimageName, NULL);
  XtSetValues(desktopWindow->iconGadget, args, 1);

  _DtCheckAndFreePixmapData(
                            file_data->logical_type,
                            desktopWindow->shell,
                            (DtIconGadget) desktopWindow->iconGadget,
                            pixmapData);
#ifdef SHAPE
  GenerateShape(desktopWindow);
#endif
}


static void
#ifdef _NO_PROTO
MakeDesktopWindow(desktopWindow, rX, rY)
        DesktopRec *desktopWindow;
        int rX;
        int rY;
#else
MakeDesktopWindow(
        DesktopRec *desktopWindow,
        int rX,
        int rY)
#endif /* _NO_PROTO */
{
   Colormap colormap;
   Pixel background;
   Pixel foreground;
   Pixel top_shadow;
   Pixel bottom_shadow;
   Pixel select;
   Arg args[10];
   XmString icon_label;
   XSizeHints wmSizeHints;
   PixmapData *pixmapData;
   Dimension  width, height;
   Dimension  gadgetWidth, gadgetHeight;
   Position gadgetX, gadgetY, drawAX, drawAY;
   WMShellWidget wm;


   /* get the colors to use for the desktop window */
   background = desktopWindow->background;
   XtSetArg (args[0], XmNcolormap,  &colormap);
   XtGetValues (desktopWindow->shell, args, 1);

   XmGetColors (XtScreen(desktopWindow->shell), colormap, background,
                &foreground, &top_shadow, &bottom_shadow, &select);

   /* set the background color for both the frame and the DrawingA */
#ifdef SHAPE
   XtSetArg (args[0], XmNbackground, background);
#else
   XtSetArg (args[0], XmNbackground, select);
#endif
   XtSetValues (desktopWindow->frame, args, 1);
   XtSetValues (desktopWindow->drawA, args, 1);

   /* set up the icon label and pixmap */
   if(desktopWindow->file_view_data->file_data->action_name != NULL)
      icon_label = XmStringLtoRCreate (
                       desktopWindow->file_view_data->file_data->action_name,
                       XmFONTLIST_DEFAULT_TAG);
   else
      icon_label = XmStringLtoRCreate (desktopWindow->title,
                                       XmFONTLIST_DEFAULT_TAG);
   XtSetArg (args[0], XmNstring, icon_label);

   pixmapData = _DtRetrievePixmapData(
                   desktopWindow->file_view_data->file_data->logical_type,
                   desktopWindow->file_view_data->file_data->file_name,
                   desktopWindow->dir_linked_to,
                   desktopWindow->shell,
                   desktopIconType);
   if (pixmapData)
      XtSetArg (args[1], XmNimageName, pixmapData->iconFileName);
   else
      XtSetArg (args[1], XmNimageName, NULL);

   /* set up the icon colors */

   XtSetArg (args[2], XmNforeground, foreground);

   if (select == white_pixel)
   {
      XtSetArg (args[3], XmNbackground, white_pixel);
      XtSetArg (args[4], XmNpixmapBackground, white_pixel);
      XtSetArg (args[5], XmNpixmapForeground, black_pixel);
   }
   else if (select == black_pixel)
   {
      XtSetArg (args[3], XmNbackground, black_pixel);
      XtSetArg (args[4], XmNpixmapBackground, white_pixel);
      XtSetArg (args[5], XmNpixmapForeground, black_pixel);
   }
   else
   {
#ifdef SHAPE
      XtSetArg (args[3], XmNbackground, background);
#else
      XtSetArg (args[3], XmNbackground, select);
#endif
      XtSetArg (args[4], XmNpixmapBackground, top_shadow);
      XtSetArg (args[5], XmNpixmapForeground, bottom_shadow);
   }
   XtSetArg (args[6], XmNuserData,
             (DirectorySet *)desktopWindow->file_view_data->directory_set);

   XtSetValues (desktopWindow->iconGadget, args, 7);

   XmStringFree(icon_label);

   _DtCheckAndFreePixmapData(
                       desktopWindow->file_view_data->file_data->logical_type,
                       desktopWindow->shell,
                       (DtIconGadget) desktopWindow->iconGadget,
                       pixmapData);

   /* Set userData, so that help system can obtain the desktop info */
   XtSetArg(args[0], XmNuserData, (XtPointer)desktopWindow);
   XtSetValues(desktopWindow->frame, args, 1);

   XtSetMappedWhenManaged(desktopWindow->shell, False);
   XtRealizeWidget (desktopWindow->shell);

   /* Set the proper workspaces if needed */
   _DtEncapSetWorkSpaceHints(desktopWindow->shell,
                             desktopWindow->workspace_name);

   /* Map the window */
   XtSetMappedWhenManaged(desktopWindow->shell, True);

   /* get the width and the heighth of the icon gadget */
   XtSetArg (args[0], XmNwidth, &gadgetWidth);
   XtSetArg (args[1], XmNheight, &gadgetHeight);
   XtSetArg (args[2], XmNx, &gadgetX);
   XtSetArg (args[3], XmNy, &gadgetY);
   XtGetValues(desktopWindow->iconGadget, args, 4);

   XtSetArg (args[0], XmNx, &drawAX);
   XtSetArg (args[1], XmNy, &drawAY);
   XtGetValues(desktopWindow->drawA, args, 2);

   width = gadgetWidth + (Dimension)(2*gadgetX) + (Dimension)(2*drawAX);
   height = gadgetHeight + (Dimension)(2*gadgetY) + (Dimension)(2*drawAY);

   if (rX == -1)
   {
      CalculateRootCoordinates(desktopWindow->workspace_num, width, height,
                               &rX, &rY);
   }

   desktopWindow->root_x = rX;
   desktopWindow->root_y = rY;

   XtSetArg (args[0], XmNx, rX);
   XtSetArg (args[1], XmNy, rY);
   XtSetArg (args[2], XmNwidth, width);
   XtSetArg (args[3], XmNheight, height);
   XtSetValues (desktopWindow->shell, args, 4);

   /* force the Window Manager to map it where I want it */
   if (XtWindow(desktopWindow->shell) != NULL)
   {
       XGetNormalHints(XtDisplay(desktopWindow->shell),
                       XtWindow(desktopWindow->shell), &wmSizeHints);
       wmSizeHints.flags = USPosition | USSize;
       XSetNormalHints(XtDisplay(desktopWindow->shell),
                       XtWindow(desktopWindow->shell), &wmSizeHints);
       XSync(XtDisplay(desktopWindow->shell), 0);
   }

   wm = (WMShellWidget)desktopWindow->shell;
   wm->wm.size_hints.flags = USPosition | USSize;

#ifdef SHAPE
   GenerateShape(desktopWindow);
#endif

   /* map the Window on the root window */
   XtPopup(desktopWindow->shell, XtGrabNone);
   XSync(XtDisplay(desktopWindow->shell), False);

   RegisterIconDropsDT(desktopWindow);

   XtSetArg (args[0], XmNwidth, &width);
   XtSetArg (args[1], XmNheight, &height);
   XtGetValues(desktopWindow->shell, args, 2);

   RegisterInGrid((int)width, (int)height, rX, rY,
                                       desktopWindow->workspace_num, True);
}


void
#ifdef _NO_PROTO
SaveDesktopInfo(session)
     int session;
#else
SaveDesktopInfo(
     int session)
#endif /* _NO_PROTO */
{
#ifdef _TT_MULTI_SCREEN
   char * desktopFileName;
#else
   static char * desktopFileName = NULL;
#endif /* _TT_MULTI_SCREEN */
   char * fileName;
   FILE * fptr;
   int i;
   DesktopRec *desktopWindow;
   char *full_path = NULL;
   char *rDir, *title, *helpVol;
   Tt_status tt_status;

#ifdef _TT_MULTI_SCREEN
   DesktopData*	  desktop_data;
   int scr;

  for (scr =0; scr < gNScreens; scr++ )
  {
   /* Save the #ifdef's */
   desktop_data = gDesktopData[scr];
#endif /* _TT_MULTI_SCREEN */

   /* Build the name which the desktop data will be saved */
#ifdef _TT_MULTI_SCREEN
   desktopFileName = GetDesktopFilename(scr);
#else
   if (desktopFileName == NULL)
   {
      desktopFileName = XtMalloc(strlen(DESKTOP_SAVE_NAME) + 1);
      sprintf(desktopFileName, "%s", DESKTOP_SAVE_NAME);
   }
#endif /* _TT_MULTI_SCREEN */

   switch( session )
   {
       case NORMAL_RESTORE:
           /* Construct full filename */
           fileName = ResolveLocalPathName(home_host_name,
                                           desktop_dir, desktopFileName,
                                           home_host_name, &tt_status);
           break;
       case HOME_DIR_RESTORE:
           full_path = (char *)XtMalloc(strlen(dt_path) + strlen("/home") + 1);
           sprintf( full_path, "%s/home", dt_path );
           fileName = ResolveLocalPathName(home_host_name,
                                           full_path, desktopFileName,
                                           home_host_name, &tt_status);
           XtFree(full_path);
           full_path = NULL;
           break;
       default:
           full_path = (char *)XtMalloc(strlen(dt_path) + strlen("/current")+1);
           sprintf( full_path, "%s/current", dt_path );
           fileName = ResolveLocalPathName(home_host_name,
                                           full_path, desktopFileName,
                                           home_host_name, &tt_status);
           XtFree(full_path);
           full_path = NULL;
           break;
   }

   if(desktop_data->numIconsUsed == 0)
   {
      unlink(fileName);
      XtFree(fileName);
      fileName = NULL;
#ifdef _TT_MULTI_SCREEN
      continue;
#else
      return;
#endif
   }

   /* Assume read-only directory, if we can't open the file */
   if ((fptr = fopen(fileName, "w+")) == NULL)
   {
      XtFree(fileName);
      fileName = NULL;
#ifdef _TT_MULTI_SCREEN
      continue;
#else
      return;
#endif
   }

   chmod(fileName, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
   XtFree((char *)fileName);  fileName = NULL;

   fprintf(fptr, "%d\n", desktop_data->numIconsUsed);

   for(i = 0;  i < desktop_data->numIconsUsed; i++)
   {
      desktopWindow = desktop_data->desktopWindows[i];

      if(desktopWindow->restricted_directory != NULL )
         rDir = XtNewString(desktopWindow->restricted_directory);
      else
         rDir = XtNewString(NULL_STRING);

      if(desktopWindow->title != NULL )
         title = XtNewString(desktopWindow->title);
      else
         title = XtNewString(NULL_STRING);

      if(desktopWindow->helpVol != NULL )
         helpVol = XtNewString(desktopWindow->helpVol);
      else
         helpVol = XtNewString(NULL_STRING);

      fprintf(fptr, "%s\n%s\n%s\n%s\n%s\n%s\n%d %d %d %d %d %d %d\n",
              desktopWindow->file_name,
              desktopWindow->workspace_name,
              desktopWindow->dir_linked_to,
              rDir,
              title,
              helpVol,
              desktopWindow->toolbox,
              desktopWindow->view,
              desktopWindow->order,
              desktopWindow->direction,
              desktopWindow->positionEnabled,
              desktopWindow->root_x,
              desktopWindow->root_y);

      /* if not normal restore (ie save current or home) remove the entry */
      if(session == CURRENT_DIR_RESTORE)
      {
         FileViewData *file_view_data;
         DirectorySet *directory_set;

         file_view_data = desktopWindow->file_view_data;
         directory_set = (DirectorySet *)file_view_data->directory_set;

         XtRemoveAllCallbacks(desktop_data->desktopWindows[i]->iconGadget,
                                                            XmNcallback);
         XtFree((char *)directory_set->file_mgr_data);
         directory_set->file_mgr_data = NULL;
         XtFree((char *)directory_set->name);
         directory_set->name = NULL;
         XtFree((char *)directory_set);
         directory_set = NULL;
         FreeFileData(file_view_data->file_data, True);
	 if(file_view_data->label)
	 {
           XtFree((char *)file_view_data->label);
           file_view_data->label = NULL;
	 }
         XtFree((char *)file_view_data);
         file_view_data = NULL;
         XtFree(desktop_data->desktopWindows[i]->workspace_name);
         desktop_data->desktopWindows[i]->workspace_name = NULL;
         XtFree(desktop_data->desktopWindows[i]->host);
         desktop_data->desktopWindows[i]->host = NULL;
         XtFree(desktop_data->desktopWindows[i]->dir_linked_to);
         desktop_data->desktopWindows[i]->dir_linked_to = NULL;
         XtFree(desktop_data->desktopWindows[i]->title);
         desktop_data->desktopWindows[i]->title = NULL;
         XtFree(desktop_data->desktopWindows[i]->helpVol);
         desktop_data->desktopWindows[i]->helpVol = NULL;
         XtFree(desktop_data->desktopWindows[i]->restricted_directory);
         desktop_data->desktopWindows[i]->restricted_directory = NULL;
         free(desktop_data->desktopWindows[i]->root_type);
         desktop_data->desktopWindows[i]->root_type = NULL;

         desktop_data->desktopWindows[i]->root_x = -1;
         desktop_data->desktopWindows[i]->root_y = -1;

         desktop_data->desktopWindows[i]->view = UNSET_VALUE;
         desktop_data->desktopWindows[i]->order = UNSET_VALUE;
         desktop_data->desktopWindows[i]->direction = UNSET_VALUE;
         desktop_data->desktopWindows[i]->positionEnabled = UNSET_VALUE;
         desktop_data->desktopWindows[i]->toolbox = False;
         desktop_data->desktopWindows[i]->title = NULL;
         desktop_data->desktopWindows[i]->helpVol = NULL;
         desktop_data->desktopWindows[i]->restricted_directory = NULL;
         desktop_data->desktopWindows[i]->root_type = NULL;

         desktop_data->desktopWindows[i]->file_view_data = NULL;
      }

      XtFree(rDir);
      rDir = NULL;
      XtFree(title);
      title = NULL;
      XtFree(helpVol);
      helpVol = NULL;
   }

   fclose(fptr);

   if(session == CURRENT_DIR_RESTORE)
   {
      desktop_data->numCachedIcons += desktop_data->numIconsUsed;
      desktop_data->numIconsUsed = 0;
      fileName = ResolveLocalPathName(home_host_name,
                                      desktop_dir, desktopFileName,
                                      home_host_name, &tt_status);
      if( TT_OK == tt_status )
      {
        unlink(fileName);
        XtFree(fileName);
        fileName = NULL;
      }
   }
#ifdef _TT_MULTI_SCREEN
  }
#endif
}


void
#ifdef _NO_PROTO
LoadDesktopInfo(session)
     char *session;
#else
LoadDesktopInfo(
     char *session)
#endif /* _NO_PROTO */
{
#ifdef _TT_MULTI_SCREEN
   char * desktopFileName = NULL;
#else
   static char * desktopFileName = NULL;
#endif
   char *fileName, *full_path, *ptr;
   char *workSpace;
   char *message;
   int i, j,  numWindows, count, index, bufferSize;
   int rX, rY;
   FILE * fptr;
   DesktopRec *desktopWindow;
   Boolean status = FALSE;
   Boolean haveOne;
   Tt_status tt_status;
   struct stat stat_buf;
#ifdef _TT_MULTI_SCREEN
   DesktopData*	  desktop_data;
   int scr;

  for ( scr = 0; scr < gNScreens; scr++ )
  {
   /* Less #ifdefs */
   desktop_data = gDesktopData[scr];

#endif

#ifdef _TT_MULTI_SCREEN
   desktopFileName = GetDesktopFilename(scr);
#else
   /* Build the name which the desktop data will be saved */
   if (desktopFileName == NULL)
   {
      desktopFileName = XtMalloc(strlen(DESKTOP_SAVE_NAME) + 1);
      sprintf(desktopFileName, "%s", DESKTOP_SAVE_NAME);
   }
#endif

   if(session == NULL)
   {
      /* Construct full filename */
      fileName = ResolveLocalPathName(home_host_name,
                                      desktop_dir, desktopFileName,
                                      home_host_name, &tt_status);
   }
   else
   {
      status = DtSessionRestorePath(toplevel, &full_path, session);

      if (!status)
         return;

      if (stat(full_path, &stat_buf) != 0)
      {
         XtFree(full_path);
         full_path = NULL;
         return;
      }

      ptr = strrchr(full_path, '/');
      *ptr = '\0';

      /* Construct full filename */
      fileName = ResolveLocalPathName(home_host_name,
                                      full_path, desktopFileName,
                                      home_host_name, &tt_status);

      XtFree(full_path);
      full_path = NULL;
   }

   /* Assume no file if the open fails */
   if ((fptr = fopen(fileName, "r")) == NULL)
   {
     /* do nothing */
      XtFree(fileName);
      fileName = NULL;
#ifdef _TT_MULTI_SCREEN
      if ( scr == ScreenCount(XtDisplay(toplevel)) - 1 )
         return;
      else
         continue;
#else
      return;
#endif
   }

   /* Load the number of entries */
   count = fscanf(fptr, "%d\n", &numWindows);

   if (count <= 0)
   {
      XtFree(fileName);
      fileName = NULL;
      fclose(fptr);
#ifdef _TT_MULTI_SCREEN
      if ( scr == ScreenCount(XtDisplay(toplevel)) - 1 )
         return;
      else
         continue;
#else
      return;
#endif
   }

   /* make sure there are enough predefined desktop windows to hold the
      amount restored */
   if(numWindows >= desktop_data->numCachedIcons)
      InitializeDesktopWindows(numWindows + 5 - desktop_data->numCachedIcons,
			       XtDisplay(toplevel));

   count = index = desktop_data->numIconsUsed;

   bufferSize = 2*MAX_PATH + 370;
   message = (char *)XtMalloc(bufferSize);

   /* read the data for each Window, then popup that window */
   for (i = count; i < numWindows + count; i++)
   {
      int len, toolbox, view, order, direction, positionEnabled;

      desktopWindow = desktop_data->desktopWindows[index];

      fgets(message, bufferSize, fptr);
      len = strlen(message); message[len-1] = 0x0;
      desktopWindow->file_name = XtNewString(message);

      fgets(message, bufferSize, fptr);
      len = strlen(message); message[len-1] = 0x0;
      workSpace = XtNewString(message);

      fgets(message, bufferSize, fptr);
      len = strlen(message); message[len-1] = 0x0;
      desktopWindow->dir_linked_to = XtNewString(message);

      { /* Construct the full path and check for the file or link
           existence.
           If it's not there on the file system, don't bother
           to load it.
        */
        char * path;
        struct stat stat_buf;

        if (strcmp(desktopWindow->dir_linked_to, "/") == 0)
        {
          path = (char *) XtMalloc(strlen(desktopWindow->dir_linked_to) +
                                   strlen(desktopWindow->file_name) + 1);
          sprintf(path, "%s%s", desktopWindow->dir_linked_to,
                  desktopWindow->file_name);
        }
        else
        {
          path = (char *) XtMalloc(strlen(desktopWindow->dir_linked_to) +
                                   strlen(desktopWindow->file_name) + 2);
          sprintf(path, "%s/%s", desktopWindow->dir_linked_to,
                  desktopWindow->file_name);
        }

        if( stat(path, &stat_buf) != 0
            && lstat(path, &stat_buf) != 0 )
        {
          XtFree(desktopWindow->file_name);
          desktopWindow->file_name = NULL;
          XtFree(workSpace);
          workSpace = NULL;
          XtFree(desktopWindow->dir_linked_to);
          desktopWindow->dir_linked_to = NULL;
          XtFree(path);
          path = NULL;
          continue;
        }

        XtFree(path);
        path = NULL;
      }

      fgets(message, bufferSize, fptr);
      len = strlen(message); message[len-1] = 0x0;
      if( strcmp(message, NULL_STRING) == 0 )
        desktopWindow->restricted_directory = NULL;
      else
        desktopWindow->restricted_directory = XtNewString(message);

      fgets(message, bufferSize, fptr);
      len = strlen(message); message[len-1] = 0x0;
      if( strcmp(message, NULL_STRING) == 0 )
        desktopWindow->title = NULL;
      else
        desktopWindow->title = XtNewString(message);

      fgets(message, bufferSize, fptr);
      len = strlen(message); message[len-1] = 0x0;
      if( strcmp(message, NULL_STRING) == 0 )
        desktopWindow->helpVol = NULL;
      else
        desktopWindow->helpVol = XtNewString(message);

      fgets(message, bufferSize, fptr);
      sscanf( message, "%d %d %d %d %d %d %d\n",
              &toolbox, &view, &order, &direction, &positionEnabled, &rX, &rY );

      desktopWindow->toolbox = (char)toolbox;
      desktopWindow->view = (char)view;
      desktopWindow->order = (char)order;
      desktopWindow->direction = (char)direction;
      desktopWindow->positionEnabled = (Boolean)positionEnabled;

      /* set up the workspace number */
      haveOne = False;
      for(j = 0; j < desktop_data->numWorkspaces; j++)
      {
         if(strcmp(workSpace, desktop_data->workspaceData[j]->name) == 0)
         {
             haveOne = True;
             desktopWindow->workspace_name = workSpace;
             desktopWindow->workspace_num =
                                       desktop_data->workspaceData[j]->number;
             break;
         }
      }

      if(!haveOne)
      {
         desktopWindow->workspace_num = 1;
         desktopWindow->workspace_name =
           (char *)XtMalloc(strlen(desktop_data->workspaceData[0]->name) + 1);
         strcpy(desktopWindow->workspace_name,
                desktop_data->workspaceData[0]->name);
         XtFree(workSpace);
         workSpace = NULL;
      }

      desktopWindow->host = XtNewString(home_host_name);
      desktopWindow->root_type = RootTypeOfDesktopObject( desktopWindow );

      desktopWindow->file_view_data =
             BuildNewOrderlist(NULL, desktopWindow->iconGadget,
                               desktopWindow->host,
                               desktopWindow->dir_linked_to,
                               desktopWindow->file_name,
                               desktopWindow->registered,
                               desktopWindow->toolbox);

      MakeDesktopWindow(desktopWindow, rX, rY);
      desktopWindow->registered = desktopWindow->file_view_data->registered;

      desktop_data->numIconsUsed++;
      desktop_data->numCachedIcons--;
      index++;
   }
   XtFree(message);
   message = NULL;

   fclose(fptr);

   XtFree(fileName);
   fileName = NULL;
#ifdef _TT_MULTI_SCREEN
  }  /* Screen loop */
#endif

   SaveDesktopInfo(NORMAL_RESTORE);
}


static void
#ifdef _NO_PROTO
FreeDesktopWindow(desktopWindow)
     DesktopRec *desktopWindow;
#else
FreeDesktopWindow(
     DesktopRec *desktopWindow)
#endif /* _NO_PROTO */
{
   int i;
   DesktopRec *tempDesktopWindow;
   FileViewData *file_view_data;
   DirectorySet *directory_set;
   Dimension width, height;
   Arg args[2];
#ifdef _TT_MULTI_SCREEN
   Screen* screen;
   int screen_num;
   DesktopData*  desktop_data;
#endif

   file_view_data = desktopWindow->file_view_data;
   directory_set = (DirectorySet *)file_view_data->directory_set;
   tempDesktopWindow = desktopWindow;

#ifdef _TT_MULTI_SCREEN
   screen_num = XScreenNumberOfScreen(XtScreen(desktopWindow->shell));
   desktop_data = gDesktopData[screen_num];
#endif

   XtSetArg (args[0], XmNwidth, &width);
   XtSetArg (args[1], XmNheight, &height);
   XtGetValues(desktopWindow->shell, args, 2);

   RegisterInGrid((int)width, (int)height, desktopWindow->root_x,
                   desktopWindow->root_y, desktopWindow->workspace_num, False);

   if(desktopWindow->text != NULL)
   {
      XtUnmanageChild(desktopWindow->text);
      XtDestroyWidget(desktopWindow->text);
      desktopWindow->text = NULL;
   }

   for(i = 0; i < desktop_data->numIconsUsed; i++)
        if(desktopWindow == desktop_data->desktopWindows[i])
           break;

   /* circulate this */
   for(;i < desktop_data->numIconsUsed - 1; i++)
   {
      desktop_data->desktopWindows[i] =
                       desktop_data->desktopWindows[i + 1];
   }

   /* clear all memory used by the desktop window */
   desktop_data->desktopWindows[i] = tempDesktopWindow;
   XtRemoveAllCallbacks(desktop_data->desktopWindows[i]->iconGadget,
                                                            XmNcallback);
   XtFree((char *)directory_set->file_mgr_data);
   directory_set->file_mgr_data = NULL;
   XtFree((char *)directory_set->name);
   directory_set->name = NULL;
   XtFree((char *)directory_set);
   directory_set = NULL;
   FreeFileData(file_view_data->file_data, True);
   if(file_view_data->label)
   {
      XtFree((char *)file_view_data->label);
      file_view_data->label = NULL;
   }
   XtFree((char *)file_view_data);
   file_view_data = NULL;
   XtFree(desktop_data->desktopWindows[i]->restricted_directory);
   desktop_data->desktopWindows[i]->restricted_directory = NULL;
   free(desktop_data->desktopWindows[i]->root_type);
   desktop_data->desktopWindows[i]->root_type = NULL;
   XtFree(desktop_data->desktopWindows[i]->title);
   desktop_data->desktopWindows[i]->title = NULL;
   XtFree(desktop_data->desktopWindows[i]->helpVol);
   desktop_data->desktopWindows[i]->helpVol = NULL;
   XtFree(desktop_data->desktopWindows[i]->workspace_name);
   desktop_data->desktopWindows[i]->workspace_name = NULL;
   XtFree(desktop_data->desktopWindows[i]->host);
   desktop_data->desktopWindows[i]->host = NULL;
   XtFree(desktop_data->desktopWindows[i]->dir_linked_to);
   desktop_data->desktopWindows[i]->dir_linked_to = NULL;
   XtFree(desktop_data->desktopWindows[i]->file_name);
   desktop_data->desktopWindows[i]->file_name = NULL;

   desktop_data->desktopWindows[i]->restricted_directory = NULL;
   desktop_data->desktopWindows[i]->root_type = NULL;
   desktop_data->desktopWindows[i]->title = NULL;
   desktop_data->desktopWindows[i]->helpVol = NULL;
   desktop_data->desktopWindows[i]->workspace_name = NULL;
   desktop_data->desktopWindows[i]->host = NULL;
   desktop_data->desktopWindows[i]->dir_linked_to = NULL;
   desktop_data->desktopWindows[i]->file_name = NULL;

   desktop_data->desktopWindows[i]->root_x = -1;
   desktop_data->desktopWindows[i]->root_y = -1;

   desktop_data->desktopWindows[i]->file_view_data = NULL;
   desktop_data->numIconsUsed--;
   desktop_data->numCachedIcons++;

   /* now lets check the number of cached Icons, if its too large free some
    * of the cache.
    */

   if(desktop_data->numCachedIcons > MAX_CACHED_ICONS)
#ifdef _TT_MULTI_SCREEN
      FreeCachedIcons(desktop_data->numCachedIcons - MAX_CACHED_ICONS, 
                      screen_num);
#else
      FreeCachedIcons(desktop_data->numCachedIcons - MAX_CACHED_ICONS);
#endif

   SaveDesktopInfo(NORMAL_RESTORE);

}


static void
#ifdef _NO_PROTO
DrawAInput (w, client_data, event)
      Widget w;
      XtPointer client_data;
      XEvent *event;
#else
DrawAInput (
      Widget w,
      XtPointer client_data,
      XEvent *event)
#endif /* _NO_PROTO */
{
   XmManagerWidget action_pane;
   DesktopRec *desktopWindow;
   XButtonEvent *bevent;
   Arg args[2];
   int i, j, num_children, num_of_files;
   static int beventx = -1;
   static int beventy = -1;

#ifdef _TT_MULTI_SCREEN
   Screen* screen;
   int screen_num;
   DesktopData*   desktop_data;  
#endif

   /* if a drag is active, don't do anything. */
   if(dragActive)
      return;


   bevent = (XButtonEvent *)event;
#ifndef _TT_MULTI_SCREEN
   desktop_data->event = *bevent;
#endif
   desktopWindow = (DesktopRec *) client_data;

#ifdef _TT_MULTI_SCREEN
   screen = XtScreen ( desktopWindow -> shell );
   screen_num = XScreenNumberOfScreen (screen);
   gDesktopData[screen_num]->event = *bevent;

   /* Use this local to save some #ifdef's */
   desktop_data = gDesktopData[screen_num];
#endif

   /* first decide whether more than one file is selected */
   for(i = 0; i < desktop_data->numWorkspaces; i++)
   {
      if(desktop_data->workspaceData[i]->number == desktopWindow->workspace_num)
         break;
   }

   num_of_files = 0;
   for(j = 0; j < desktop_data->workspaceData[i]->files_selected; j++)
   {
      /* we need to check to see if the dt object this popup is on is
         a selected file */
      if(desktop_data->workspaceData[i]->selectedDTWindows[j] == desktopWindow)
      {
         /* it is in the selected list */
         num_of_files = desktop_data->workspaceData[i]->files_selected;
         break;
      }
   }

   if(num_of_files > 1)
   {
      /* we have more than one files selected,
         Set userData, so that help system can obtain the desktop info */
      XtSetArg(args[0], XmNuserData,
          (XtPointer)desktop_data->workspaceData[i]->selectedDTWindows);
      XtSetValues(desktop_data->popupMenu->popup, args, 1);

      XtRemoveAllCallbacks(desktop_data->popupMenu->removeDT,
                                                 XmNactivateCallback);
      XtAddCallback(desktop_data->popupMenu->removeDT, XmNactivateCallback,
              RemoveDT,
              (XtPointer) desktop_data->workspaceData[i]->selectedDTWindows[0]);
      XtRemoveAllCallbacks(desktop_data->popupMenu->openFolder,
                                                      XmNactivateCallback);
      XtAddCallback(desktop_data->popupMenu->openFolder, XmNactivateCallback,
                                         OpenFolder, (XtPointer) NULL);

      XtRemoveAllCallbacks(desktop_data->popupMenu->rename,
                                                         XmNactivateCallback);
      XtAddCallback(desktop_data->popupMenu->rename, XmNactivateCallback,
                                         renameDT, (XtPointer) NULL);

      /* unsensitise the "Open Parent Folder" and "Rename" options */
      XtSetSensitive(desktop_data->popupMenu->openFolder, False);
      XtSetSensitive(desktop_data->popupMenu->rename, False);

      /* XXX Unmanage all action menu buttons. */
      action_pane = (XmManagerWidget) desktop_data->popupMenu->popup;

      num_children = action_pane->composite.num_children;
      for (i = 7; i < num_children; i++)
      {
        XtUnmanageChild (action_pane->composite.children[i]);
      }
   }
   else
   {
      char *fileName;
       /* No point in continuing any further if any of these pointers
          is NULL */
      if(!desktop_data->popupMenu || !desktopWindow->file_view_data)
         return;
      /* we have zero or one files selected, so just put up the popup for
         the one */

      /* Set userData, so that help system can obtain the desktop info */
      XtSetArg(args[0], XmNuserData, client_data);
      XtSetValues(desktop_data->popupMenu->popup, args, 1);

      XtRemoveAllCallbacks(desktop_data->popupMenu->removeDT,
                           XmNactivateCallback);
      XtAddCallback(desktop_data->popupMenu->removeDT, XmNactivateCallback,
                    RemoveDT, (XtPointer) desktopWindow);

      XtRemoveAllCallbacks(desktop_data->popupMenu->openFolder,
                           XmNactivateCallback);
      XtAddCallback(desktop_data->popupMenu->openFolder, XmNactivateCallback,
                    OpenFolder, (XtPointer) desktopWindow);

      XtRemoveAllCallbacks(desktop_data->popupMenu->rename,
                           XmNactivateCallback);

      if( desktopWindow->toolbox == False )
      {  /* Sensitize rename
          */
         XtSetSensitive(desktop_data->popupMenu->rename, True);
         XtAddCallback(desktop_data->popupMenu->rename, XmNactivateCallback,
                       renameDT, (XtPointer) desktopWindow);
      }
      else
      {  /* Don't allow user to rename the objects in the tool box.
            These things are action labels.
         */
         XtSetSensitive(desktop_data->popupMenu->rename, False);

      }

      XtSetSensitive(desktop_data->popupMenu->openFolder, True);

      XtFree(desktop_data->popupMenu->action_pane_file_type);
      desktop_data->popupMenu->action_pane_file_type = NULL;
      desktop_data->popupMenu->action_pane_file_type = XtNewString(
               desktopWindow->file_view_data->file_data->logical_type);

      fileName = _DtGetSelectedFilePath( desktopWindow->file_view_data );
      UpdateActionMenuPane ((XtPointer)desktopWindow, NULL,
                       desktop_data->popupMenu->action_pane_file_type,
                       DESKTOP, 7, desktop_data->popupMenu->popup,
                       desktopWindow->file_view_data->file_data->physical_type,
                       fileName);
      XtFree( fileName );

      if(desktopWindow->file_view_data->file_data->physical_type == DtDIRECTORY
         && desktopWindow->file_name )
      {
         char *ptr;

         ptr = DtStrrchr(desktopWindow->title, ':');
         if(ptr != NULL)
         {
            ptr++;
            if(strcmp(root_title, ptr) == 0)
            { /* The desktop object is a ROOT directory */
               XtSetSensitive(desktop_data->popupMenu->openFolder, False);
               XtSetSensitive(desktop_data->popupMenu->rename, False);
               XtRemoveAllCallbacks(desktop_data->popupMenu->rename,
                                    XmNactivateCallback);
            }
         }
      }
   }

   if(event == NULL)
   {
      Position x, y;
      Dimension width, height;
      int displayWidth, displayHeight;
      DtIconGadget g;

      XtSetArg (args[0], XmNwidth, &width);
      XtSetArg (args[1], XmNheight, &height);
      XtGetValues(XtParent(desktop_data->popupMenu->popup), args, 2);

#ifdef _TT_MULTI_SCREEN
      displayWidth = DisplayWidth(XtDisplay(w), screen_num);
      displayHeight = DisplayHeight(XtDisplay(w), screen_num);
#else
      displayWidth = DisplayWidth(XtDisplay(w), DefaultScreen(XtDisplay(w)));
      displayHeight = DisplayHeight(XtDisplay(w), DefaultScreen(XtDisplay(w)));
#endif

      g = (DtIconGadget)desktopWindow->iconGadget;

      if(beventx == -1)
         beventx = desktopWindow->root_x + g->icon.pixmap_width/2;

      if((Dimension)(beventx + width) > (Dimension)displayWidth)
         beventx = displayWidth - width - 4;

      if(beventy == -1)
         beventy = desktopWindow->root_y +
                 desktopWindow->iconGadget->core.height/2;

      if((Dimension)(beventy + height) > (Dimension)displayHeight)
         beventy = beventy - height - 1;

      XtSetArg (args[0], XmNx, beventx);
      XtSetArg (args[1], XmNy, beventy);
      XtSetValues(XtParent(desktop_data->popupMenu->popup), args, 2);
      beventx = -1;
      beventy = -1;
   }
   else
   {
      beventx = bevent->x_root;
      beventy = bevent->y_root;
      XmMenuPosition(desktop_data->popupMenu->popup,
                                  (XButtonPressedEvent *)bevent);
   }

   XtManageChild(desktop_data->popupMenu->popup);

}

/**********************************************************************
 *
 *  RemoveDT - Callback from the Desktop Popup menu to 'Remove From
 *      Desktop'.  Popdown's and Free the desktop window passed in
 *      in the client_data pointer.  Then checks to see if any other
 *      desktop window is pointing to the file in the File Manager the
 *      removed desktop object was pointing to.  If there's not, it
 *      removes the file from the desktop list.
 *
 ************************************************************************/

void
#ifdef _NO_PROTO
RemoveDT (w, client_data, call_data)
      Widget w;
      XtPointer client_data;
      XtPointer call_data;
#else
RemoveDT (w, client_data, call_data)
      Widget w;
      XtPointer client_data;
      XtPointer call_data;
#endif /* _NO_PROTO */
{
   DesktopRec *desktopWindow = (DesktopRec *) client_data;
   int i, j, num_of_files;
   Boolean selected = False;

#ifdef _TT_MULTI_SCREEN
   Screen* screen = XtScreen ( desktopWindow -> shell );
   int screen_num = XScreenNumberOfScreen (screen);
   DesktopData*  desktop_data = gDesktopData[screen_num];
#endif


   /* first decide whether more than one file is selected */
   for(i = 0; i < desktop_data->numWorkspaces; i++)
   {
      if(desktop_data->workspaceData[i]->number == desktopWindow->workspace_num)
         break;
   }

   /* next determine if the file is really in the selected list */
   num_of_files = 0;
   for(j = 0; j < desktop_data->workspaceData[i]->files_selected; j++)
   {
      /* we need to check to see if the dt object this popup is on is
         a selected file */
      if(desktop_data->workspaceData[i]->selectedDTWindows[j] == desktopWindow)
      {
         /* it is in the selected list */
         num_of_files = desktop_data->workspaceData[i]->files_selected;
         break;
      }
   }

   if(num_of_files == 0)
     num_of_files = 1;
   else
     selected = True;

   for(j = 0; j < num_of_files; j++)
   {
      if(selected)
         desktopWindow = desktop_data->workspaceData[i]->selectedDTWindows[j];

      XtPopdown(desktopWindow->shell);
      XWithdrawWindow(XtDisplay(desktopWindow->shell),
                      XtWindow(desktopWindow->shell),
#ifdef _TT_MULTI_SCREEN
                      XScreenNumberOfScreen(XtScreen(desktopWindow->shell)));
#else
                      XDefaultScreen(XtDisplay(desktopWindow->shell)));
#endif

      FreeDesktopWindow(desktopWindow);
      if(selected)
         desktop_data->workspaceData[i]->files_selected--;
   }
}

#ifdef SUN_PERF
void
#ifdef _NO_PROTO
RemoveMovedObjectFromDT (w, client_data, file_cnt, file_list)
      Widget w;
      XtPointer client_data;
      int file_cnt ;
      char **file_list ;
#else
RemoveMovedObjectFromDT (w, client_data, file_cnt, file_list)
      Widget w;
      XtPointer client_data;
      int file_cnt ;
      char **file_list ;
#endif /* _NO_PROTO */
{
   DesktopRec *desktopWindow = (DesktopRec *) client_data;
   int i, j, k, m, num_of_files;
   Boolean selected = False;
   char filename[MAX_PATH] ;

#ifdef _TT_MULTI_SCREEN
   Screen* screen = XtScreen ( desktopWindow -> shell );
   int screen_num = XScreenNumberOfScreen (screen);
   DesktopData*  desktop_data = gDesktopData[screen_num];
#endif

   /* first decide whether more than one file is selected */
   for(i = 0; i < desktop_data->numWorkspaces; i++)
   {
      if(desktop_data->workspaceData[i]->number == desktopWindow->workspace_num)
         break;
   }

   /* next determine if the file is really in the selected list */
   num_of_files = 0;
   for(j = 0; j < desktop_data->workspaceData[i]->files_selected; j++)
   {
      /* we need to check to see if the dt object this popup is on is
         a selected file */
      if(desktop_data->workspaceData[i]->selectedDTWindows[j] == desktopWindow)
      {
         /* it is in the selected list */
         num_of_files = desktop_data->workspaceData[i]->files_selected;
         break;
      }
   }
   
   if(num_of_files == 0)
     selected = 1;
   else
     selected = True;

   for(k = 0; k < num_of_files; k++)
   {
      if(selected)
         desktopWindow = desktop_data->workspaceData[i]->selectedDTWindows[k];
      for(j = 0; j < file_cnt; j++) {
         strcpy(filename, desktopWindow->dir_linked_to) ;
         strcat(filename, "/") ;
         strcat(filename, desktopWindow->file_name) ;
         if (!strcmp(filename, file_list[j])) {
             XtPopdown(desktopWindow->shell);
             XWithdrawWindow(XtDisplay(desktopWindow->shell),
                      XtWindow(desktopWindow->shell),
#ifdef _TT_MULTI_SCREEN
                      XScreenNumberOfScreen(XtScreen(desktopWindow->shell)));
#else
                      XDefaultScreen(XtDisplay(desktopWindow->shell)));
#endif

             FreeDesktopWindow(desktopWindow);
             if(selected) {
         	desktop_data->workspaceData[i]->files_selected--;
         	for (m = k ; m < num_of_files - 1 ; m++) {
	 	  desktop_data->workspaceData[i]->selectedDTWindows[m] = 
		    desktop_data->workspaceData[i]->selectedDTWindows[m + 1];
         	}
             }
         } 
      }
   }
}
#endif /* SUN_PERF */

static void
#ifdef _NO_PROTO
OpenFolder (w, client_data, call_data)
      Widget w;
      XtPointer client_data;
      XtPointer call_data;
#else
OpenFolder (
      Widget w,
      XtPointer client_data,
      XtPointer call_data)
#endif /* _NO_PROTO */
{
   DesktopRec   *desktopWindow;
   DirectorySet *directory_data;
   FileViewData *file_view_data;
   FileMgrData  *file_mgr_data;
   FileMgrRec   *file_mgr_rec;
   DialogData   *dialog_data;
   int j;

   desktopWindow = (DesktopRec *) client_data;
   file_view_data = desktopWindow->file_view_data;

   if (desktopWindow->toolbox)
   {
      DtActionArg  *action_args;
      char *pwd_dir;

      action_args = (DtActionArg *) XtCalloc(1, sizeof(DtActionArg));
      if (action_args)
      {
         action_args[0].argClass = DtACTION_FILE;
         action_args[0].u.file.name = desktopWindow->dir_linked_to;
      }

#ifdef _TT_MULTI_SCREEN 
      /* Set the proper display for the action */
      _XmOSPutenv(gDisplayStrings[XScreenNumberOfScreen(XtScreen(w))]);
#endif

      pwd_dir = XtNewString(desktopWindow->dir_linked_to);
      DtActionInvoke(desktopWindow->shell, "OpenParentAppGroup", action_args, 1,
                     NULL, NULL, pwd_dir, True, NULL, NULL);
      XtFree(pwd_dir);
      pwd_dir = NULL;
   }
   else
   {
      initiating_view = (XtPointer) NULL;

#ifdef _TT_MULTI_SCREEN 
      gTTScreenNum = XScreenNumberOfScreen(XtScreen(w));
#endif

      if(desktopWindow->restricted_directory != NULL)
      {
         special_view = True;
         special_treeType = SINGLE_DIRECTORY;
         special_viewType = desktopWindow->view;
         special_orderType = desktopWindow->order;
         special_directionType = desktopWindow->direction;
         special_randomType = desktopWindow->positionEnabled;
         special_restricted = XtNewString(desktopWindow->restricted_directory);
         if(desktopWindow->title == NULL)
            special_title = NULL;
         else
            special_title = XtNewString(desktopWindow->title);
         special_helpVol = XtNewString(desktopWindow->helpVol);
         if(desktopWindow->toolbox)
            dialog_data = GetNewView(desktopWindow->host,
                                     desktopWindow->dir_linked_to,
                                     desktopWindow->restricted_directory,
				     NULL, 0);
         else
            dialog_data = GetNewView(desktopWindow->host,
                                     desktopWindow->dir_linked_to,
				     NULL, NULL, 0);
      }
      else
      {
         dialog_data = GetNewView(desktopWindow->host,
                                  desktopWindow->dir_linked_to,
                                  NULL, NULL, 0);
      }

      if (openDirType == NEW && dialog_data == NULL)
         return;

      if( dialog_data == NULL )
         return ;

      file_mgr_data = (FileMgrData *) dialog_data->data;
      directory_data = file_mgr_data->directory_set[0];

      for (j = 0; j < directory_data->file_count; j++)
      {
         file_view_data = directory_data->file_view_data[j];

         if (file_view_data->filtered != True &&
             strcmp(desktopWindow->file_name,
             file_view_data->file_data->file_name) == 0)
         {
            SelectFile (file_mgr_data, file_view_data);
            break;
         }
      }
      if(!directory_data->file_count)
        file_mgr_data->desktop_file = XtNewString(desktopWindow->file_name);
      else
        file_mgr_data->desktop_file = NULL;

      file_mgr_rec = (FileMgrRec *)file_mgr_data->file_mgr_rec;

      /* Initially, all menubuttons are sensitive */
      file_mgr_rec->menuStates = (RENAME | MOVE | DUPLICATE | LINK | TRASH |
                                  MODIFY | CHANGEDIR | PREFERENCES | FILTER |
                                  FIND | CREATE_DIR | CREATE_FILE | SETTINGS |
                                  CLEAN_UP | MOVE_UP |
                                  HOME | CHANGE_DIR);

      if (file_mgr_data->selected_file_count == 0)
         ActivateNoSelect(file_mgr_rec);
      else if (file_mgr_data->selected_file_count == 1)
         ActivateSingleSelect(file_mgr_rec,
                    file_mgr_data->selection_list[0]->file_data->logical_type);
      else
         ActivateMultipleSelect(file_mgr_rec);

      PositionFileView(file_view_data, file_mgr_data);
   }
}

static void
#ifdef _NO_PROTO
renameDT (w, client_data, call_data)
      Widget w;
      XtPointer client_data;
      XtPointer call_data;
#else
renameDT (
      Widget w,
      XtPointer client_data,
      XtPointer call_data)
#endif /* _NO_PROTO */
{
   DesktopRec   *desktopWindow;
   FileViewData *file_view_data;

   desktopWindow = (DesktopRec *) client_data;
   file_view_data = desktopWindow->file_view_data;

   CreateNameChangeDialog(desktopWindow->iconGadget, file_view_data,
                          (XtPointer)desktopWindow, DESKTOP);
}

void
#ifdef _NO_PROTO
SelectDTFile (desktopWindow)
      DesktopRec *desktopWindow;
#else
SelectDTFile (DesktopRec *desktopWindow)
#endif /* _NO_PROTO */
{
   int selection_count;
   int i,j;

   Display *display;
   Window   rootWindow;
   Atom     pCurrent;
   Screen   *currentScreen;
   int      screen;
   char     *workspace_name;
#ifdef _TT_MULTI_SCREEN
   DesktopData* desktop_data;
#endif


   /* Get the current workspace */
#ifdef _TT_MULTI_SCREEN
   display = XtDisplay(desktopWindow->shell);
   currentScreen = XtScreen(display);
   screen = XScreenNumberOfScreen(currentScreen);
   rootWindow = RootWindowOfScreen(currentScreen);

   desktop_data = gDesktopData[screen];

#else
   display = XtDisplay(desktop_data->desktopWindows[0]->shell);
   screen = XDefaultScreen(display);
   currentScreen = XScreenOfDisplay(display, screen);
   rootWindow = RootWindowOfScreen(currentScreen);
#endif

   if(DtWsmGetCurrentWorkspace(display, rootWindow, &pCurrent) == Success)
   {
      workspace_name = XGetAtomName (display, pCurrent);
      CleanUpWSName(workspace_name);
   }
   else
      workspace_name = XtNewString(desktop_data->workspaceData[0]->name);

   for(i = 0; i < desktop_data->numWorkspaces; i++)
   {
      if(strcmp(workspace_name, desktop_data->workspaceData[i]->name) == 0)
      {
              desktop_data->workspaceData[i]->files_selected++;

              desktop_data->workspaceData[i]->selectedDTWindows =
                  (DesktopRec **) 
                  XtRealloc ((char *) desktop_data->workspaceData[i]->selectedDTWindows,
                     sizeof(DesktopRec *) *(desktop_data->workspaceData[i]->files_selected + 2));

              /* Add to the front of the list */
              for (j = desktop_data->workspaceData[i]->files_selected; j > 0; j--)
              {
                  desktop_data->workspaceData[i]->selectedDTWindows[j] = desktop_data->workspaceData[i]->selectedDTWindows[j-1];
										              }
              desktop_data->workspaceData[i]->selectedDTWindows[0] = desktopWindow;
              break;
      }
   }
   SetToSelectColors (desktopWindow->iconGadget, desktopWindow->frame,
		      LINK_FILE);
   XSync(display, False);

}

void
#ifdef _NO_PROTO
DeselectAllDTFiles (workspaceData)
      WorkspaceRec *workspaceData;
#else
DeselectAllDTFiles (
      WorkspaceRec *workspaceData)
#endif /* _NO_PROTO */
{
   int selection_count;
   register int i;

   selection_count = workspaceData->files_selected;

   for (i = 0; i < selection_count; i++)
   {
         SetToNormalColors (workspaceData->selectedDTWindows[i]->iconGadget,
                            workspaceData->selectedDTWindows[i]->frame,
                            workspaceData->selectedDTWindows[i]->frame,
                            LINK_FILE);
   }

   if (workspaceData->selectedDTWindows != NULL)
   {
      XtFree ((char *) workspaceData->selectedDTWindows);
      workspaceData->selectedDTWindows = NULL;
   }

   workspaceData->selectedDTWindows =
      (DesktopRec **) XtMalloc (sizeof (DesktopRec *));
   workspaceData->selectedDTWindows[0] = NULL;
   workspaceData->files_selected = 0;

}

void
#ifdef _NO_PROTO
DeselectDTFile (workspaceData, desktopWindow)
      WorkspaceRec *workspaceData;
      DesktopRec *desktopWindow;
#else
DeselectDTFile (
      WorkspaceRec *workspaceData,
      DesktopRec *desktopWindow)
#endif /* _NO_PROTO */
{
   int selection_count;
   register int i;
   register int j;


   selection_count = workspaceData->files_selected;
   workspaceData->files_selected--;

   for (i = 0; i < selection_count; i++)
      if (workspaceData->selectedDTWindows[i] == desktopWindow)
         break;

   for (j = i; j < selection_count - 1; j++)
      workspaceData->selectedDTWindows[j] =
                                      workspaceData->selectedDTWindows[j + 1];

   workspaceData->selectedDTWindows = (DesktopRec **)
                   XtRealloc ((char *) workspaceData->selectedDTWindows,
                               sizeof(DesktopRec *) * selection_count);
   workspaceData->selectedDTWindows[selection_count - 1] = NULL;

   SetToNormalColors (desktopWindow->iconGadget, desktopWindow->frame,
                                        desktopWindow->frame, LINK_FILE);

}

void
#ifdef _NO_PROTO
DTActionCallback (w, client_data, call_data)
     Widget w;
     XtPointer client_data;
     XtPointer call_data;
#else
DTActionCallback (
     Widget w,
     XtPointer client_data,
     XtPointer call_data)
#endif /* _NO_PROTO */
{
   DesktopRec *desktopWindow;
   char *command;
   Arg args[1];

   desktopWindow = (DesktopRec *) client_data;

   XtSetArg (args[0], XmNuserData, (XtPointer) &command);
   XtGetValues (w, args, 1);

   RunDTCommand(command, desktopWindow, NULL);
}


void
#ifdef _NO_PROTO
RunDTCommand(command, desktopWindow, drop_parameters)
      char *command;
      DesktopRec *desktopWindow;
      DtDndDropCallbackStruct *drop_parameters;
#else
RunDTCommand(
      char *command,
      DesktopRec *desktopWindow,
      DtDndDropCallbackStruct *drop_parameters)
#endif /* _NO_PROTO */

{
   if ((strcmp(openNewView, command) == 0) ||
       (strcmp(openInPlace, command) == 0))
   {
      char *ltype = desktopWindow->file_view_data->file_data->logical_type;

      /* If the folder is locked, don't allow user to go into it */
      if( strcmp( ltype, LT_FOLDER_LOCK ) == 0 )
      {
        char *tmpStr, *title, *msg;

        tmpStr = GETMESSAGE(9, 6, "Action Error");
        title = XtNewString(tmpStr);
        msg = (char *)XtMalloc(
                   strlen( GETMESSAGE(30, 1, "Cannot read from %s") )
                 + strlen( desktopWindow->file_view_data->file_data->file_name )
                 + 1 );
        sprintf( msg, GETMESSAGE(30, 1, "Cannot read from %s"),
                 desktopWindow->file_view_data->file_data->file_name );
        _DtMessage(desktopWindow->shell,
                   title, msg, NULL, HelpRequestCB );
        XtFree(title);
        title = NULL;
        XtFree(msg);
        msg = NULL;
        return;
      }

      /* this statement applies to the case where a user traverses down the *
       * part of the directory tree containing the application manager      *
       * directories or the trash directory                                 */
      if( (strcmp(ltype, LT_AGROUP) == 0) ||
          (strstr(ltype, LT_AGROUP_SUBDIR)) ||
          (strcmp(ltype, LT_TRASH) == 0) )
      {
         ProcessAction(command, 
                       desktopWindow->file_view_data,
                       drop_parameters,
                       desktopWindow->host,
                       desktopWindow->dir_linked_to,
                       desktopWindow->restricted_directory,
                       desktopWindow->shell);
      }
      else
      {
         ProcessNewViewDT(desktopWindow);
      }
   }

   else if ((strcmp (command, "FILESYSTEM_MOVE") == 0) ||
            (strcmp (command, "FILESYSTEM_COPY") == 0) ||
            (strcmp (command, "FILESYSTEM_LINK") == 0))
   {
      if (drop_parameters->dropData->protocol == DtDND_FILENAME_TRANSFER)
         ProcessMoveCopyLinkDT(command, desktopWindow, drop_parameters);
      else
         ProcessBufferDropOnFolderDT(command, desktopWindow, drop_parameters);
   }

   else
   {
      ProcessAction(command, 
                    desktopWindow->file_view_data,
                    drop_parameters,
                    desktopWindow->host,
                    desktopWindow->dir_linked_to,
                    desktopWindow->restricted_directory,
                    desktopWindow->shell);
   }
}


/************************************************************************
 *
 *  ProcessNewViewDT
 *
 ************************************************************************/

static void
#ifdef _NO_PROTO
ProcessNewViewDT (desktopWindow)
     DesktopRec *desktopWindow;
#else
ProcessNewViewDT (
     DesktopRec *desktopWindow)
#endif /* _NO_PROTO */
{
   char directory_name[MAX_PATH];

   sprintf(directory_name, "%s/%s", desktopWindow->dir_linked_to,
                                    desktopWindow->file_name);
   DtEliminateDots (directory_name);

#ifdef _TT_MULTI_SCREEN
   gTTScreenNum = XScreenNumberOfScreen(XtScreen(desktopWindow->shell));
#endif


   initiating_view = (XtPointer) NULL;
   if(desktopWindow->restricted_directory == NULL)
   {
      GetNewView (desktopWindow->host, directory_name, NULL, NULL, 0);
   }
   else
   {
      special_view = True;
      special_treeType = SINGLE_DIRECTORY;
      special_viewType = desktopWindow->view;
      special_orderType = desktopWindow->order;
      special_directionType = desktopWindow->direction;
      special_randomType = desktopWindow->positionEnabled;
      special_restricted = XtNewString(desktopWindow->restricted_directory);
      if(desktopWindow->title == NULL)
         special_title = NULL;
      else
         special_title = XtNewString(desktopWindow->title);
      special_helpVol = XtNewString(desktopWindow->helpVol);
      if(desktopWindow->toolbox)
         GetNewView (desktopWindow->host, directory_name,
                     desktopWindow->restricted_directory, NULL, 0);
      else
         GetNewView (desktopWindow->host, directory_name, NULL, NULL, 0);
   }
}


/************************************************************************
 *
 *  ProcessMoveCopyLinkDT
 *
 ************************************************************************/

static void
#ifdef _NO_PROTO
ProcessMoveCopyLinkDT (command, desktopWindow, drop_parameters)
     char *command;
     DesktopRec *desktopWindow;
     DtDndDropCallbackStruct *drop_parameters;
#else
ProcessMoveCopyLinkDT (
     char *command,
     DesktopRec *desktopWindow,
     DtDndDropCallbackStruct *drop_parameters)
#endif /* _NO_PROTO */
{
   unsigned int modifiers = NULL;
   int numFiles, i;
   char ** file_set;
   char ** host_set;
   FileViewData * file_view_data;
   Boolean error = FALSE;

     /***************************************************/
     /* if no drop_parameters, there is nothing to move */
     /***************************************************/
   if (!drop_parameters)
     return;

     /***************************************************/
     /* extract file and host sets from drop parameters */
     /***************************************************/
   numFiles = drop_parameters->dropData->numItems;
   _DtSetDroppedFileInfo(drop_parameters, &file_set, &host_set);


      /******************************/
      /* set movement modifier mask */
      /******************************/
   if( (initiating_view != NULL) &&
       (((FileMgrData *)initiating_view)->toolbox) )
   {
      /* if initiating_view is a toolbox, the transfer must be */
      /* a copy                                                */
      modifiers = ControlMask;
   }
   else
   {
      if (strcmp(command, "FILESYSTEM_COPY") == 0)
         modifiers = ControlMask;
      else if (strcmp(command, "FILESYSTEM_LINK") == 0)
         modifiers = ShiftMask;
      else
         modifiers = NULL;
   }

      /*************************************************************/
      /* is the desktop object linked to any of the dropped files? */
      /*************************************************************/
   file_view_data = desktopWindow->file_view_data;
   if (file_view_data->file_data->physical_type == DtDIRECTORY)
   {
      char dir_name[MAX_PATH];

      sprintf(dir_name, "%s/%s", desktopWindow->dir_linked_to,
              desktopWindow->file_name);
      DtEliminateDots(dir_name);

      for (i=0; i < numFiles; i++)
      {
         if (strcmp(dir_name, file_set[i]) == 0 &&
             strcmp(desktopWindow->host, host_set[i]) == 0)
         {
            char *tmpStr, *msg;

            error = True;
            tmpStr =
               GETMESSAGE(11,16, "A folder cannot be moved, copied or linked into itself.\n%s");
            msg = XtNewString(tmpStr);
            FileOperationError(file_view_data->widget, msg, dir_name);
            XtFree(msg);
         }
      }
   }

      /**************/
      /* Move files */
      /**************/
   if (!error)
      CheckMoveType((FileMgrData *)NULL, file_view_data,
                    (DirectorySet *)(file_view_data->directory_set),
                    desktopWindow, file_set, host_set, modifiers,
                    numFiles, drop_parameters->x, drop_parameters->y, DESKTOP);

      /***************************/
      /* free file and host sets */
      /***************************/
   _DtFreeDroppedFileInfo(numFiles, file_set, host_set);
}


/************************************************************************
 *
 * ProcessBufferDropOnFolderDT
 *
 ************************************************************************/

static void
#ifdef _NO_PROTO
ProcessBufferDropOnFolderDT (command, desktopWindow, drop_parameters)
     char *command;
     DesktopRec *desktopWindow;
     DtDndDropCallbackStruct *drop_parameters;
#else
ProcessBufferDropOnFolderDT (
     char *command,
     DesktopRec *desktopWindow,
     DtDndDropCallbackStruct *drop_parameters)
#endif /* _NO_PROTO */
{
   int num_of_buffers;
   char ** file_set = NULL;
   char ** host_set = NULL;
   BufferInfo *buffer_set = NULL;
   char  directory[MAX_PATH];
   FileViewData *file_view_data = desktopWindow->file_view_data;


   /********************************************/
   /* if no drop_parameters, or invalid params */
   /* then disallow the drop                   */
   /********************************************/
   if (!drop_parameters)
     return;

   /* if dropped on a folder icon and file_view_data */
   /* is NULL, disallow the drop                     */
   if (file_view_data == NULL)
     return;


  /***************************************************/
  /* extract file and host sets from drop parameters */
  /***************************************************/
  num_of_buffers = drop_parameters->dropData->numItems;

  /* Allocate memory for file and buffer structures */
  file_set = (char **)XtMalloc(sizeof(char **) * num_of_buffers );
  host_set = (char **)XtMalloc(sizeof(char **) * num_of_buffers);
  buffer_set = (BufferInfo * )XtMalloc (sizeof (BufferInfo) * num_of_buffers);

  _DtSetDroppedBufferInfo(file_set, buffer_set, host_set, drop_parameters);


  /****************/
  /* create files */
  /****************/
  DPRINTF(("ProcessBufferDropOnFolderDT...Buffers dropped on Folder icon %s\n",
          file_view_data->file_data->file_name));

   sprintf(directory,"%s/%s",
           ((DirectorySet *)file_view_data->directory_set)->name,
           file_view_data->file_data->file_name);
   DtEliminateDots(directory);

   DPRINTF (("Copying buffer to %s\n", directory));
   MakeFilesFromBuffersDT(file_view_data, directory, file_set, host_set,
                          buffer_set, num_of_buffers,
                          desktopWindow, NULL, NULL);
 

  /******************************/
  /* free file_set + buffer_set */
  /******************************/
  _DtFreeDroppedBufferInfo (file_set, buffer_set, host_set, num_of_buffers);
}

char *
#ifdef _NO_PROTO
IsAFileOnDesktop2(file_set, file_count, number,IsToolBox)
        char **file_set;
        int  file_count;
        int  *number;
        Boolean  *IsToolBox;
#else
IsAFileOnDesktop2(
        char **file_set,
        int  file_count,
        int  *number,
        Boolean  *IsToolBox)
#endif /* _NO_PROTO */
{
   int i,j;
   DesktopRec *desktopWindow;
   char tmp_filename[MAX_PATH];
   Boolean onDesktop = False;
   Boolean Link = False;
   char *filesOnDesktop;
   char *filename1,*filename2;
   int newLen, stat_result;
   Tt_status tt_status;
   static Tt_message dummy_msg = NULL;
   struct stat stat_buf;

#ifdef _TT_MULTI_SCREEN
   int scr;
   int breakout;
   DesktopData* desktop_data;
#endif

   *IsToolBox = False;

   if(dummy_msg == NULL)
      dummy_msg = tt_message_create();

   *number = 0;
   for(i = 0; i < file_count; i++)
   {
      /* Let's check to see if file coming in is a link, if it is let's
       * test it instead of where it is linked to.
       */
      stat_result = lstat (file_set[i], &stat_buf);
      if (stat_result == 0 && (stat_buf.st_mode & S_IFMT) == S_IFLNK)
      {
         filename1 = XtNewString(file_set[i]);
         Link = True;
      }
      else
      {
         tt_status = tt_message_file_set(dummy_msg, file_set[i]);
         filename1 = tt_message_file(dummy_msg);
         Link = False;
      }
#ifdef _TT_MULTI_SCREEN
      for (breakout = False, scr = 0; 
           !breakout && (scr < gNScreens); 
           scr++ )
      {
         desktop_data = gDesktopData[scr];
#endif

      for(j = 0; j < desktop_data->numIconsUsed; j++)
      {
        desktopWindow = desktop_data->desktopWindows[j];

/*
        if( strcmp( desktopWindow->dir_linked_to, "/" ) != 0 )
          sprintf(tmp_filename, "%s/%s", desktopWindow->dir_linked_to,
                    desktopWindow->file_view_data->file_data->file_name);
        else
          sprintf(tmp_filename, "%s%s", desktopWindow->dir_linked_to,
                    desktopWindow->file_view_data->file_data->file_name);
*/
        strcpy( tmp_filename, desktopWindow->dir_linked_to );
        if(Link)
           filename2 = XtNewString(tmp_filename);
        else
        {
           tt_status = tt_message_file_set (dummy_msg, tmp_filename);
           filename2 = tt_message_file(dummy_msg);
        }
        filename2 = (char *)XtRealloc(filename2, strlen(filename2)
                    + strlen(desktopWindow->file_view_data->file_data->file_name)
                    + 2 );
        strcat(filename2, "/");
        strcat(filename2, desktopWindow->file_view_data->file_data->file_name);



        if( strcmp( filename1, filename2 ) == 0 )
        {
          *IsToolBox = desktopWindow->toolbox;
          if(onDesktop == False)
          {
            filesOnDesktop = (char *)XtMalloc(strlen(filename1) + 10);
            sprintf( filesOnDesktop, "\n   %s\n", filename1);
            onDesktop = True;
          }
          else
          {
            newLen = strlen(filesOnDesktop) + strlen(filename1) + 6;
            filesOnDesktop = (char *)XtRealloc(filesOnDesktop, newLen);
            sprintf(filesOnDesktop, "%s   %s\n", filesOnDesktop, filename1);
          }
          *number += 1;
#ifdef _TT_MULTI_SCREEN
          breakout = True;
#endif
          break;
        }
        if(Link)
           XtFree(filename2);
        else
           tt_free(filename2);
      }
#ifdef _TT_MULTI_SCREEN
      } /* Each screen */
#endif
      if(Link)
         XtFree(filename1);
      else
         tt_free(filename1);
   }

   if(onDesktop)
     return(filesOnDesktop);

   return(NULL);
}


/*
 * This function handles button 1 selection in a desktop icon.
 */

void
#ifdef _NO_PROTO
ProcessDTSelection (desktopRec, event)
   DesktopRec * desktopRec;
   XButtonEvent * event;
#else
ProcessDTSelection (
   DesktopRec * desktopRec,
   XButtonEvent * event)
#endif /* _NO_PROTO */
{
    Window   rootWindow;
    Atom     pCurrent;
    Screen   *currentScreen;
    int      screen;
    char     *workspace_name;
    Boolean  toggle_select = False;
    int i, j;
#ifdef _TT_MULTI_SCREEN
    DesktopData*  desktop_data;
#endif

    if (event->state & ControlMask)
       toggle_select = True;

#ifdef _TT_MULTI_SCREEN
    screen = XScreenNumberOfScreen(XtScreen(desktopRec->shell));
    currentScreen =  XtScreen(desktopRec->shell);
    desktop_data = gDesktopData[screen];
#else
    screen = XDefaultScreen(XtDisplay(desktopRec->shell));
    currentScreen = XScreenOfDisplay(XtDisplay(desktopRec->shell), screen);
#endif
    rootWindow = RootWindowOfScreen(currentScreen);

    if(DtWsmGetCurrentWorkspace(XtDisplay(desktopRec->shell), rootWindow,
                                                      &pCurrent) == Success)
    {
       workspace_name = XGetAtomName (XtDisplay(desktopRec->shell), pCurrent);
       CleanUpWSName(workspace_name);
    }
    else
       workspace_name = XtNewString(desktop_data->workspaceData[0]->name);

    for(i = 0; i < desktop_data->numWorkspaces; i++)
    {
       if(strcmp(workspace_name, desktop_data->workspaceData[i]->name) == 0)
       {
          if(toggle_select == False)
             DeselectAllDTFiles(desktop_data->workspaceData[i]);
          else
          {
             for(j = 0;j < desktop_data->workspaceData[i]->files_selected; j++)
             {
                if(desktop_data->workspaceData[i]->selectedDTWindows[j] ==
                                                                 desktopRec)
                {
                   DeselectDTFile(desktop_data->workspaceData[i], desktopRec);
                   return;
                }
             }
          }

          desktop_data->workspaceData[i]->files_selected++;

          desktop_data->workspaceData[i]->selectedDTWindows =
               (DesktopRec **) XtRealloc ((char *)
                         desktop_data->workspaceData[i]->selectedDTWindows,
                         sizeof(DesktopRec *) *
                         (desktop_data->workspaceData[i]->files_selected + 2));

          /* Add to the front of the list */
          for (j = desktop_data->workspaceData[i]->files_selected; j > 0; j--)
          {
             desktop_data->workspaceData[i]->selectedDTWindows[j] =
                     desktop_data->workspaceData[i]->selectedDTWindows[j-1];
          }

          desktop_data->workspaceData[i]->selectedDTWindows[0] = desktopRec;
          break;
       }
    }
    XtFree(workspace_name);

    SetToSelectColors (desktopRec->iconGadget, desktopRec->frame, LINK_FILE);
}


void
#ifdef _NO_PROTO
UnpostDTTextField ()
#else
UnpostDTTextField ()
#endif /* _NO_PROTO */

{
   DesktopRec *desktopWindow;
   int i;

#ifdef _TT_MULTI_SCREEN
   int scr;
   DesktopData*  desktop_data;

   for (scr = 0; scr < gNScreens; scr++ )
   {
      desktop_data = gDesktopData[scr];
#endif

      for(i = 0; i < desktop_data->numIconsUsed; i++)
      {
         desktopWindow = desktop_data->desktopWindows[i];
         if(desktopWindow->text)
         {
            XtUnmanageChild(desktopWindow->text);
#ifdef SHAPE
            GenerateShape(desktopWindow);
#endif
            XtDestroyWidget(desktopWindow->text);
            desktopWindow->text = NULL;
            return;
         }
      }
#ifdef _TT_MULTI_SCREEN
   }
#endif
}


Boolean
#ifdef _NO_PROTO
DTFileIsSelected (desktopRec, fileViewData)
   DesktopRec * desktopRec;
   FileViewData * fileViewData;
#else
DTFileIsSelected (
   DesktopRec * desktopRec,
   FileViewData * fileViewData)
#endif /* _NO_PROTO */
{
   Window   rootWindow;
   Atom     pCurrent;
   Screen   *currentScreen;
   int      screen;
   char     *workspace_name;
   int i, j;

#ifdef _TT_MULTI_SCREEN
   DesktopData*  desktop_data;

   currentScreen = XtScreen(desktopRec->shell);
   screen = XScreenNumberOfScreen(currentScreen);
   desktop_data = gDesktopData[screen];
#else
   screen = XDefaultScreen(XtDisplay(desktopRec->shell));
   currentScreen = XScreenOfDisplay(XtDisplay(desktopRec->shell), screen);
#endif
   rootWindow = RootWindowOfScreen(currentScreen);

   if(DtWsmGetCurrentWorkspace(XtDisplay(desktopRec->shell), rootWindow,
                                                      &pCurrent) == Success)
   {
       workspace_name = XGetAtomName (XtDisplay(desktopRec->shell), pCurrent);
       CleanUpWSName(workspace_name);
   }
   else
       workspace_name = XtNewString(desktop_data->workspaceData[0]->name);


   for(i = 0; i < desktop_data->numWorkspaces; i++)
   {
      if(strcmp(workspace_name, desktop_data->workspaceData[i]->name) == 0)
      {
         for(j = 0;j < desktop_data->workspaceData[i]->files_selected; j++)
         {
            if(desktop_data->workspaceData[i]->selectedDTWindows[j] ==
                                 desktopRec)
            {
	       XtFree(workspace_name);
               return(True);
            }
         }
	 XtFree(workspace_name);
         return(False);
      }
   }
   XtFree(workspace_name);
   return(False);
}

/***********************************************************************
 *
 *  InitializeDesktopGrid
 *
 ************************************************************************/

void
#ifdef _NO_PROTO
InitializeDesktopGrid()
#else
InitializeDesktopGrid( void )
#endif /* _NO_PROTO */
{
   int i,j,k;

#ifdef _TT_MULTI_SCREEN
   DesktopData* desktop_data = gDesktopData[0];
#endif

   desktop_grid_size = desktop_data->numWorkspaces * numColumns * numRows;
   desktop_grid = (Boolean *)XtCalloc(1, desktop_grid_size);

   /* want to take out space where the FP lays ... Note this only for the
      default size of the FP.  Right now there is no dynamic way of registering
      the FP no matter what size it is */

   for(i = 1; i <= desktop_data->numWorkspaces; i++)
   {
      RegisterInGrid((int)1132, (int)115, 78, 910, i, True);
   }
}

void
#ifdef _NO_PROTO
RegisterInGrid(width, height, rX, rY, workspace, type)
       int width;
       int height;
       int rX;
       int rY;
       int workspace;
       Boolean type;
#else
RegisterInGrid(
       int width,
       int height,
       int rX,
       int rY,
       int workspace,
       Boolean type)
#endif /* _NO_PROTO */
{
   int row, column;
   int rowHeight, columnWidth;
   int desktop_grid_index;

   if(desktopIconType == LARGE)
   {
      row = rY / PIXELS_PER_ROW_LARGE;
      column = rX / PIXELS_PER_COLUMN_LARGE;
      rowHeight = (rY + height) / PIXELS_PER_ROW_LARGE;
      columnWidth = (rX + width) / PIXELS_PER_COLUMN_LARGE;
   }
   else
   {
      row = rY / PIXELS_PER_ROW_SMALL;
      column = rX / PIXELS_PER_COLUMN_SMALL;
      rowHeight = (rY + height) / PIXELS_PER_ROW_SMALL;
      columnWidth = (rX + width) / PIXELS_PER_COLUMN_SMALL;
   }

   desktop_grid_index = (workspace - 1) * numColumns * numRows;
   if(column < numColumns && row < numRows)
     desktop_grid[ desktop_grid_index + (column * numRows) + row] = type;
   if(rowHeight < numRows)
      desktop_grid[desktop_grid_index + (column * numRows) + rowHeight] = type;
   if(columnWidth < numColumns)
      desktop_grid[desktop_grid_index + (columnWidth * numRows) + row] = type;
   if(rowHeight < numRows && columnWidth < numColumns)
      desktop_grid[desktop_grid_index +
                                   (columnWidth * numRows) + rowHeight] = type;
}



void
#ifdef _NO_PROTO
PutOnDTCB (w, client_data, call_data)
     Widget w;
     XtPointer client_data;
     XtPointer call_data;
#else
PutOnDTCB (
     Widget w,
     XtPointer client_data,
     XtPointer call_data)
#endif /* _NO_PROTO */
{
   Arg args[1];
   FileMgrRec *file_mgr_rec;
   DialogData  * dialog_data;
   FileMgrData * file_mgr_data;
   FileViewData * file_view_data;
   Widget mbar;
   char *directory;
   char *file_name;
   int i,EndIndex;

#ifdef _TT_MULTI_SCREEN
   int screen_num = XScreenNumberOfScreen(XtScreen(w)); 

   DesktopData* desktop_data = gDesktopData[screen_num];
#endif


   XmUpdateDisplay (w);

   if((int)client_data != NULL)
      mbar = XtParent(w);
   else
      mbar = XmGetPostedFromWidget(XtParent(w));

   XtSetArg(args[0], XmNuserData, &file_mgr_rec);
   XtGetValues(mbar, args, 1);

   /* Ignore accelerators when we're insensitive */
   if(client_data == NULL)
      if ((file_mgr_rec->menuStates & PUT_ON_DESKTOP) == 0)
         return;

   /* Ignore accelerators received after we're unposted */
   if ((dialog_data = _DtGetInstanceData (file_mgr_rec)) == NULL)
      return;
   file_mgr_data = (FileMgrData *) dialog_data->data;

   /* get the file_view_data object from which the menu was invoked */
   if (client_data == NULL)
     file_view_data = NULL;
   else
   {
     file_view_data = file_mgr_data->popup_menu_icon;
     if (!file_mgr_data->selected_file_count && file_view_data == NULL)
       /* the object has gone away (probably deleted) */
       return;

     file_mgr_data->popup_menu_icon = NULL; /* Reset it, we don't need it */

       /* No need to see the selection_list if file_view_data is NULL */

 
     if(file_view_data)
     {
       for(i = 0; i < file_mgr_data->selected_file_count; i++)
       {
          if(file_mgr_data->selection_list[i] == file_view_data)
          {
             file_view_data = NULL;
             break;
          }
       }
     }
   }

   EndIndex = desktop_data->numIconsUsed;
   if(file_view_data == NULL)
   {
      for (i = 0; i < file_mgr_data->selected_file_count; i++)
      {
         if (file_mgr_data->toolbox)
         {
            char *path, *ptr;
            
            path = _DtGetSelectedFilePath(file_mgr_data->selection_list[i]);
            path = _DtResolveAppManPath(path, 
                                        file_mgr_data->restricted_directory);
            ptr = strrchr(path, '/');
            *ptr = '\0';
            directory = path;
            file_name = ptr + 1;
            
#ifdef _TT_MULTI_SCREEN
            SetupDesktopWindow (XtDisplay(w), file_mgr_data, file_mgr_rec,
                                file_name, file_mgr_data->host,
                                directory,
                                -1, -1, NULL, EndIndex, screen_num);
#else
            SetupDesktopWindow (XtDisplay(w), file_mgr_data, file_mgr_rec,
                                file_name, file_mgr_data->host,
                                directory,
                                -1, -1, NULL, EndIndex);
#endif
    
            *ptr = '/';
            XtFree(path);
            path = NULL;
         }
         else
         {
            directory = ((DirectorySet *)
                   (file_mgr_data->selection_list[i]->directory_set))->name;
            file_name = file_mgr_data->selection_list[i]->file_data->file_name;

#ifdef _TT_MULTI_SCREEN
            SetupDesktopWindow (XtDisplay(w), file_mgr_data, file_mgr_rec,
                                file_name, file_mgr_data->host,
                                directory,
                                -1, -1, NULL, EndIndex, screen_num);
#else
            SetupDesktopWindow (XtDisplay(w), file_mgr_data, file_mgr_rec,
                                file_name, file_mgr_data->host,
                                directory,
                                -1, -1, NULL, EndIndex);
#endif
         }
      }
   }
   else
   {
      if (file_mgr_data->toolbox)
      {
         char *path, *ptr;

         path = _DtGetSelectedFilePath(file_view_data);
         path = _DtResolveAppManPath(path, file_mgr_data->restricted_directory);
         ptr = strrchr(path, '/');
         *ptr = '\0';
         directory = path;
         file_name = ptr + 1;

#ifdef _TT_MULTI_SCREEN
         SetupDesktopWindow (XtDisplay(w), file_mgr_data, file_mgr_rec,
                             file_name, file_mgr_data->host,
                             directory,
                             -1, -1, NULL, EndIndex, screen_num);
#else
         SetupDesktopWindow (XtDisplay(w), file_mgr_data, file_mgr_rec,
                             file_name, file_mgr_data->host,
                             directory,
                             -1, -1, NULL, EndIndex);
#endif

         *ptr = '/';
         XtFree(path);
         path = NULL;
      }
      else
      {
         directory = ((DirectorySet *)(file_view_data->directory_set))->name;
         file_name = file_view_data->file_data->file_name;
#ifdef _TT_MULTI_SCREEN
         SetupDesktopWindow (XtDisplay(w), file_mgr_data, file_mgr_rec,
                             file_name, file_mgr_data->host,
                             file_mgr_data->current_directory, -1, -1, 
                             NULL, EndIndex, screen_num);
#else
         SetupDesktopWindow (XtDisplay(w), file_mgr_data, file_mgr_rec,
                             file_name, file_mgr_data->host,
                             file_mgr_data->current_directory, -1, -1, NULL,
                             EndIndex);
#endif
      }
   }
}


static void
#ifdef _NO_PROTO
CalculateRootCoordinates ( ws_num, width, height, root_x, root_y)
     int ws_num;
     int width;
     int height;
     int *root_x;
     int *root_y;
#else
CalculateRootCoordinates (
     int ws_num,
     int width,
     int height,
     int *root_x,
     int *root_y)
#endif /* _NO_PROTO */
{
   int      row, column;
   Boolean  rDirection, cDirection, whichFirst;
   Boolean  error = False;
   int      numGridsR, numGridsC, i, j;

   if(desktopIconType == LARGE)
   {
      numGridsC = (width / PIXELS_PER_COLUMN_LARGE) + 1;
      numGridsR = (height / PIXELS_PER_ROW_LARGE) + 1;
   }
   else /* small */
   {
      numGridsC = (width / PIXELS_PER_COLUMN_SMALL) + 1;
      numGridsR = (height / PIXELS_PER_ROW_SMALL) + 1;
   }

   /*
    * set up where to get the first grid spot and which directions to
    * search for one until one is found.  For directions True means in the
    * positive direction.  If whichFirst is True then row is moved first.
    */
   switch( desktopPlacement )
   {
       case (OBJ_PLACE_LEFT_PRIMARY | OBJ_PLACE_TOP_SECONDARY):
       case (OBJ_PLACE_TOP_PRIMARY | OBJ_PLACE_LEFT_SECONDARY):
           row = 0;
           column = 0;
           rDirection = True;
           cDirection = True;
           if(desktopPlacement & OBJ_PLACE_LEFT_PRIMARY)
             whichFirst = False;
           else
             whichFirst = True;
           break;

       case (OBJ_PLACE_LEFT_PRIMARY | OBJ_PLACE_BOTTOM_SECONDARY):
       case (OBJ_PLACE_BOTTOM_PRIMARY | OBJ_PLACE_LEFT_SECONDARY):
           row = numRows - numGridsR;
           column = 0;
           rDirection = False;
           cDirection = True;
           if(desktopPlacement & OBJ_PLACE_LEFT_PRIMARY)
             whichFirst = False;
           else
             whichFirst = True;
           break;

       case (OBJ_PLACE_RIGHT_PRIMARY | OBJ_PLACE_TOP_SECONDARY):
       case (OBJ_PLACE_TOP_PRIMARY | OBJ_PLACE_RIGHT_SECONDARY):
           row = 0;
           column = numColumns - numGridsC;
           rDirection = True;
           cDirection = False;
           if(desktopPlacement & OBJ_PLACE_RIGHT_PRIMARY)
             whichFirst = False;
           else
             whichFirst = True;
           break;

       case (OBJ_PLACE_RIGHT_PRIMARY | OBJ_PLACE_BOTTOM_SECONDARY):
       case (OBJ_PLACE_BOTTOM_PRIMARY | OBJ_PLACE_RIGHT_SECONDARY):
           row = numRows - numGridsR;
           column = numColumns - numGridsC;
           rDirection = False;
           cDirection = False;
           if(desktopPlacement & OBJ_PLACE_RIGHT_PRIMARY)
             whichFirst = False;
           else
             whichFirst = True;
           break;
   }


   /*
    * Given the information which is set up above, find the first availible
    * spot in the grid where the Desktop Icon can be placed.  NOTE: if the
    * Icon turns out to be bigger then the size of the grid, this may have
    * to be recalculated.  But we won't know the size until we do a realize
    */
   ws_num = ((ws_num - 1) * numColumns * numRows);
   while(1)
   {
      if(desktop_grid[ws_num + (column * numRows) + row] == False)
      {
         if(numGridsR == 1 && numGridsC == 1)
         {
            if(desktopIconType == LARGE)
            {
               *root_x = column * PIXELS_PER_COLUMN_LARGE;
               *root_y = row * PIXELS_PER_ROW_LARGE;
            }
            else
            {
               *root_x = column * PIXELS_PER_COLUMN_SMALL;
               *root_y = row * PIXELS_PER_ROW_SMALL;
            }
            return;
         }

         for(i = 0; i < numGridsR; i++)
         {
            for(j = 0; j < numGridsC; j++)
            {
               if(desktop_grid[ws_num + ((column + j) * numRows) + (row + i)]
                                                                       == True)
              {
                 error = True;
                 break;
              }
            }
            if(error)
               break;
         }
         if(!error)
         {
            if(desktopIconType == LARGE)
            {
               *root_x = column * PIXELS_PER_COLUMN_LARGE;
               *root_y = row * PIXELS_PER_ROW_LARGE;
            }
            else
            {
               *root_x = column * PIXELS_PER_COLUMN_SMALL;
               *root_y = row * PIXELS_PER_ROW_SMALL;
            }
            return;
         }
         else
            error = False;
      }

      /*
       * area taken so let get the next grid depending on rDirection,
       * cDirection and whchis is first
       */
      if(whichFirst)
      {
         if(rDirection)
         {
            row += 1;
            if(row >= numRows)
            {
               row = 0;
               if(cDirection)
               {
                  column += 1;
                  if(column + numGridsC - 1 >= numColumns)
                  {
                     error = True;
                     break;
                  }
               }
               else
               {
                  column -= 1;
                  if(column < 0)
                  {
                     error = True;
                     break;
                  }
               }
            }
         }
         else
         {
            row -= 1;
            if(row <  0)
            {
               row = numRows - 1;
               if(cDirection)
               {
                  column += 1;
                  if(column + numGridsC - 1 >= numColumns)
                  {
                     error = True;
                     break;
                  }
               }
               else
               {
                  column -= 1;
                  if(column < 0)
                  {
                     error = True;
                     break;
                  }
               }
            }
         }
      }
      else
      {
         if(cDirection)
         {
            column += 1;
            if(column >= numColumns)
            {
               column = 0;
               if(rDirection)
               {
                  row += 1;
                  if( row + numGridsR - 1 >= numRows)
                  {
                     error = True;
                     break;
                  }
               }
               else
               {
                  row -= 1;
                  if( row < 0)
                  {
                     error = True;
                     break;
                  }
               }
            }
         }
         else
         {
            column -= 1;
            if(column < 0)
            {
               column = numColumns - 1;
               if(rDirection)
               {
                  row += 1;
                  if( row + numGridsR - 1>= numRows)
                  {
                     error = True;
                     break;
                  }
               }
               else
               {
                  row -= 1;
                  if( row < 0)
                  {
                     error = True;
                     break;
                  }
               }
            }
         }
      }
   }
}


#ifdef _TT_MULTI_SCREEN

void
#ifdef _NO_PROTO
SetupDesktopWindow (display, file_mgr_data, file_mgr_rec, file_name, hst_name, dir_name, root_x, root_y, type, EndIndex, screen_num)
     Display *display;
     FileMgrData *file_mgr_data;
     FileMgrRec *file_mgr_rec;
     char *file_name;
     char *hst_name;
     char *dir_name;
     int root_x;
     int root_y;
     char *type;
     int EndIndex;
     int screen_num;
#else
SetupDesktopWindow (
     Display *display,
     FileMgrData *file_mgr_data,
     FileMgrRec *file_mgr_rec,
     char *file_name,
     char *hst_name,
     char *dir_name,
     int root_x,
     int root_y,
     char *type,
     int EndIndex, 
     int screen_num)
#endif /* _NO_PROTO */

#else  /* _TT_MULTI_SCREEN */

void
#ifdef _NO_PROTO
SetupDesktopWindow (display, file_mgr_data, file_mgr_rec, file_name, hst_name, dir_name, root_x, root_y, type, EndIndex)
     Display *display;
     FileMgrData *file_mgr_data;
     FileMgrRec *file_mgr_rec;
     char *file_name;
     char *hst_name;
     char *dir_name;
     int root_x;
     int root_y;
     char *type;
     int EndIndex;
#else
SetupDesktopWindow (
     Display *display,
     FileMgrData *file_mgr_data,
     FileMgrRec *file_mgr_rec,
     char *file_name,
     char *hst_name,
     char *dir_name,
     int root_x,
     int root_y,
     char *type, 
     int EndIndex)

#endif /* _NO_PROTO */

#endif /* _TT_MULTI_SCREEN */
{
   char *title;
   char title_buf[256];
   Boolean restricted_top;
   DesktopRec *desktopWindow;
   int i, j;
   int ws_num;
   Window   rootWindow;
   Atom     pCurrent;
   Screen   *currentScreen;
   int      screen;
   char   * workspace_name;
   char *ptr, * directory_name;
   Tt_status tt_status;

#ifdef _TT_MULTI_SCREEN
   DesktopData* desktop_data = gDesktopData[screen_num];
#endif

   DPRINTF(("SetupDesktopWindow: %s\n", file_name));

   /*
    * Get the host, directory, and file name for the real file
    */

   /* get the full path and eliminate dots */
   directory_name = ResolveLocalPathName(hst_name,
                                         dir_name,
                                         file_name,
                                         home_host_name,
                                         &tt_status);
   if( TT_OK != tt_status )
   { /* Should pop up an error dialog.
     */
     return;
   }

   DtEliminateDots(directory_name);

   /* check if this is a restricted directory */
   restricted_top = file_mgr_data &&
                    file_mgr_data->restricted_directory &&
                    strcmp(directory_name,
                           file_mgr_data->restricted_directory) == 0;

   /* separate the rest of the path into directory and file name */
   ptr = strrchr(directory_name, '/');
   if (*(ptr + 1) != '\0')
   {
      *ptr++ = '\0';
      file_name = ptr;
   }
   else
      file_name = ".";

   /*
    * Get the window title
    */
   if (restricted_top && file_mgr_data->title)
      title = file_mgr_data->title;
   else if (strcmp(file_name, ".") != 0)
      title = file_name;
   else
   {
      sprintf(title_buf, "%s:%s", home_host_name, root_title);
      title = title_buf;
   }

#ifdef _TT_MULTI_SCREEN
   screen = screen_num;
#else
   screen = XDefaultScreen(display);
#endif
   currentScreen = XScreenOfDisplay(display, screen);
   rootWindow = RootWindowOfScreen(currentScreen);

   /* want to get the current workspace so that the window maps
      to the right workspace */
   if (DtWsmGetCurrentWorkspace(display, rootWindow, &pCurrent) == Success)
   {
      workspace_name = XGetAtomName (display, pCurrent);
      CleanUpWSName(workspace_name);
   }
   else
      workspace_name = XtNewString(desktop_data->workspaceData[0]->name);

   desktop_data->event.type = INVALID_TYPE;

   ws_num = -1;
   /* determine the workspace number */
   for(j = 0; j < desktop_data->numWorkspaces; j++)
   {
      if(strcmp(workspace_name, desktop_data->workspaceData[j]->name) == 0)
      {
          ws_num = desktop_data->workspaceData[j]->number;
          break;
      }
   }

   /* if the workspace number is still -1, it means there are new
      workspaces, so lets go initialize them */
   if(ws_num == -1)
     ws_num = InitializeNewWorkspaces(display, workspace_name);


   /* loop through the current objects on the desktop and see if one
      with the same name is already in this workspace.  If it is,
      don't map the window and put out a warning message telling
      the user this */
   for(i = 0; i < EndIndex; i++)
   {
      if (strcmp(desktop_data->desktopWindows[i]->title, title) == 0 &&
          strcmp(desktop_data->desktopWindows[i]->workspace_name,
                 workspace_name) == 0)
      {
         char * msg;
         char * tmpStr;

         tmpStr = (GETMESSAGE(28,11, "An object named:\n\n   %s\n\nalready exists on the backdrop in this Workspace.\nYou cannot put another object with the same name on the Workspace.\nTo have both objects on the Workspace, rename one of them."));
         msg = XtNewString(tmpStr);
         FileOperationError (file_mgr_rec->main, msg, title);
         XtFree(msg);
         msg = NULL;
         XtFree(workspace_name);
         workspace_name = NULL;
         XtFree(directory_name);
         directory_name = NULL;
         return;
      }
   }

   /* get the next available cached iconWindow and use it to map
      this topLevelWindow */
   desktopWindow = desktop_data->desktopWindows[desktop_data->numIconsUsed];
   desktop_data->numIconsUsed++;
   desktop_data->numCachedIcons--;

   /* store some of the data needed for the Desktop object */
   desktopWindow->host = XtNewString(home_host_name);
   if( *directory_name == 0x0 )
     desktopWindow->dir_linked_to = XtNewString("/");
   else
     desktopWindow->dir_linked_to = XtNewString(directory_name);
   desktopWindow->file_name = XtNewString(file_name);
   desktopWindow->title = XtNewString(title);

   desktopWindow->workspace_name = workspace_name;
   desktopWindow->workspace_num = ws_num;

   if (file_mgr_data != NULL)
   {
      if (file_mgr_data->restricted_directory == NULL)
         desktopWindow->restricted_directory = NULL;
      else
         desktopWindow->restricted_directory =
                XtNewString(file_mgr_data->restricted_directory);
      desktopWindow->root_type = RootTypeOfDesktopObject( desktopWindow );

      if (file_mgr_data->helpVol == NULL)
         desktopWindow->helpVol = NULL;
      else
         desktopWindow->helpVol = XtNewString(file_mgr_data->helpVol);

      desktopWindow->view = file_mgr_data->view;
      desktopWindow->order = file_mgr_data->order;
      desktopWindow->direction = file_mgr_data->direction;
      desktopWindow->positionEnabled = file_mgr_data->positionEnabled;
      desktopWindow->toolbox = file_mgr_data->toolbox;
   }
   else  /* there is no file_mgr_data associated with this file, so it
            must be "Put On Desktop" but not from a File Manager View
            (i.e. Find File) */
   {
       /* store the directory the file is linked to */
       /* need to determine the toolbox dir !!!*/
       if (type == NULL)
         desktopWindow->restricted_directory = NULL;
       else
         desktopWindow->restricted_directory = XtNewString(type);
       desktopWindow->root_type = RootTypeOfDesktopObject( desktopWindow );
       desktopWindow->helpVol = NULL;
       desktopWindow->toolbox = False;
       desktopWindow->view = UNSET_VALUE;
       desktopWindow->order = UNSET_VALUE;
       desktopWindow->direction = UNSET_VALUE;
       desktopWindow->positionEnabled = UNSET_VALUE;
   }

   desktopWindow->file_view_data = BuildNewOrderlist(
                          NULL /* @@@ use cached info if possible! */,
                          desktopWindow->iconGadget,
                          home_host_name,
                          desktopWindow->dir_linked_to,
                          file_name,
                          desktopWindow->registered,
                          desktopWindow->toolbox);

   MakeDesktopWindow(desktopWindow, root_x, root_y);
   desktopWindow->registered = desktopWindow->file_view_data->registered;

   /* go cache some more windows if there isn't enough cached */
   if (desktop_data->numCachedIcons < 5)
      InitializeDesktopWindows(5, display);

   /* go save the current state of the desktop */
   SaveDesktopInfo(NORMAL_RESTORE);

   XtFree(directory_name);
   directory_name = NULL;

   if(openDirType == NEW)
      ForceMyIconOpen(desktopWindow->host, NULL);

   if( checkBrokenLink != 0 && checkBrokenLinkTimerId == NULL )
   {
     checkBrokenLinkTimerId = XtAppAddTimeOut(
                   XtWidgetToApplicationContext( toplevel ),
                   checkBrokenLink * 1000,
                   TimerEventBrokenLinks,
                   NULL );
   }

}


#ifdef _TT_MULTI_SCREEN

static void
#ifdef _NO_PROTO
FreeCachedIcons(iconsToBeFreed, screen_num)
     int iconsToBeFreed;
     int screen_num;
#else
FreeCachedIcons (
     int iconsToBeFreed, 
     int screen_num)
#endif /* _NO_PROTO */

#else  /* _TT_MULTI_SCREEN */

static void
#ifdef _NO_PROTO
FreeCachedIcons(iconsToBeFreed)
     int iconsToBeFreed;
#else
FreeCachedIcons (
     int iconsToBeFreed)
#endif /* _NO_PROTO */

#endif /* _TT_MULTI_SCREEN */
{
   int i, new_size;
   DesktopRec *desktop;
   DesktopRec *tmpDesktop;

#ifdef _TT_MULTI_SCREEN
   DesktopData*  desktop_data = gDesktopData[screen_num];
#endif

   new_size = (desktop_data->numCachedIcons + desktop_data->numIconsUsed) -
                                                              iconsToBeFreed;
   for(i = desktop_data->numCachedIcons + desktop_data->numIconsUsed;
                                                         i > new_size; i--)
   {
      desktop = desktop_data->desktopWindows[i - 1];
      if(desktop == sacredDesktop)
      {
         desktop = desktop_data->desktopWindows[desktop_data->numIconsUsed];
         desktop_data->desktopWindows[desktop_data->numIconsUsed] =
                                                                 sacredDesktop;
      }
      XtDestroyWidget(desktop->shell);
      XtFree((char *)desktop);
      desktop = NULL;

      desktop_data->numCachedIcons--;
   }
   desktop_data->desktopWindows =
               (DesktopRec ** )XtRealloc((char *)desktop_data->desktopWindows,
                                            sizeof(DesktopRec *) * new_size);
}

static void
#ifdef _NO_PROTO
CreatePopupMenu (drawA)
     Widget drawA;
#else
CreatePopupMenu (
     Widget drawA )
#endif /* _NO_PROTO */
{
   Widget popupBtns[15];
   XmString label_string;
   char * mnemonic;
   Arg args[6];
   int n;

#ifdef _TT_MULTI_SCREEN
   int scr = XScreenNumberOfScreen(XtScreen(drawA));
   DesktopData* desktop_data = gDesktopData[scr];
#endif

   /*
    * WARNING!!!
    *
    * IF YOU ADD ANY NEW MENU ITEMS TO THE DESKTOP POPUP MENU, THEN YOU
    * MUST ALSO ADD A CHECK FOR IT IN DTHelpRequestCB() [in HelpCB.c],
    * SINCE THAT FUNCTION HAS A SPECIAL CHECK FOR EACH MENU BUTTON!
    */

   n = 0;
   XtSetArg(args[n], XmNwhichButton, bMenuButton); n++;
   desktop_data->popupMenu->popup =
                           XmCreatePopupMenu(drawA, "DTPopup", args, n);

   XtAddCallback (desktop_data->popupMenu->popup, XmNmapCallback,
                                        popupMenu, (XtPointer) NULL);
   XtAddCallback (desktop_data->popupMenu->popup, XmNhelpCallback,
                  (XtCallbackProc)DTHelpRequestCB,
                  HELP_DESKTOP_POPUP_MENU_STR);

   label_string = XmStringCreateLtoR((GETMESSAGE(28,4, "Workspace Object")),
                                                  XmFONTLIST_DEFAULT_TAG);
   XtSetArg (args[0], XmNlabelString, label_string);
   popupBtns[0] = (Widget) XmCreateLabelGadget(desktop_data->popupMenu->popup,
                                                          "title", args, 1);
   popupBtns[1] = XmCreateSeparatorGadget(desktop_data->popupMenu->popup,
                                                           "sep1", NULL, 0);
   popupBtns[2] = XmCreateSeparatorGadget(desktop_data->popupMenu->popup,
                                                           "sep2", NULL, 0);
   XmStringFree (label_string);

   label_string = XmStringCreateLtoR((GETMESSAGE(28,5, "Remove From Workspace")),
                                                  XmFONTLIST_DEFAULT_TAG);
   XtSetArg (args[0], XmNlabelString, label_string);
   mnemonic = ((char *)GETMESSAGE(28, 6, "R"));
   XtSetArg (args[1], XmNmnemonic, mnemonic[0]);
   desktop_data->popupMenu->removeDT = popupBtns[3] =
           XmCreatePushButtonGadget (desktop_data->popupMenu->popup,
                                                             "rmDT", args, 2);
   XmStringFree (label_string);
   XtAddCallback (desktop_data->popupMenu->removeDT, XmNhelpCallback,
                  (XtCallbackProc)DTHelpRequestCB,
                  HELP_DESKTOP_POPUP_MENU_STR);


   label_string = XmStringCreateLtoR ((GETMESSAGE(28,7, "Open Parent Folder")),
                                                  XmFONTLIST_DEFAULT_TAG);
   XtSetArg (args[0], XmNlabelString, label_string);
   mnemonic = ((char *)GETMESSAGE(28, 8, "O"));
   XtSetArg (args[1], XmNmnemonic, mnemonic[0]);
   desktop_data->popupMenu->openFolder = popupBtns[4] =
            XmCreatePushButtonGadget (desktop_data->popupMenu->popup,
                                                             "info", args, 2);
   XmStringFree (label_string);
   XtAddCallback (desktop_data->popupMenu->openFolder, XmNhelpCallback,
                  (XtCallbackProc)DTHelpRequestCB,
                  HELP_DESKTOP_POPUP_MENU_STR);

   label_string = XmStringCreateLtoR ((GETMESSAGE(20,128, "Rename")),
                                      XmFONTLIST_DEFAULT_TAG);
   XtSetArg (args[0], XmNlabelString, label_string);
   mnemonic = ((char *)GETMESSAGE(20, 129, "n"));
   XtSetArg (args[1], XmNmnemonic, mnemonic[0]);
   desktop_data->popupMenu->rename = popupBtns[5] =
            XmCreatePushButtonGadget (desktop_data->popupMenu->popup,
                                                            "rename", args, 2);
   XmStringFree (label_string);
   XtAddCallback (desktop_data->popupMenu->rename, XmNhelpCallback,
                  (XtCallbackProc)DTHelpRequestCB,
                  HELP_DESKTOP_POPUP_MENU_STR);

   popupBtns[6] = XmCreateSeparatorGadget(desktop_data->popupMenu->popup,
                                                          "sep1", NULL, 0);

   XtManageChildren(popupBtns, 7);
}

static void
#ifdef _NO_PROTO
popupMenu (w, client_data, call_data)
     Widget w;
     XtPointer client_data;
     XtPointer call_data;
#else
popupMenu (
     Widget w,
     XtPointer client_data,
     XtPointer call_data)
#endif /* _NO_PROTO */
{
   Widget shell;
   Boolean found = False;
   int i;
   XmAnyCallbackStruct * callback;
   XEvent * event;
#ifdef _TT_MULTI_SCREEN
   int scr;
   DesktopData*  desktop_data;
#endif

   callback = (XmAnyCallbackStruct *) call_data;
   event = (XEvent *) callback->event;

   shell = XmGetPostedFromWidget(w);

#ifdef _TT_MULTI_SCREEN
   scr = XScreenNumberOfScreen ( XtScreen ( shell ) );
   desktop_data = gDesktopData[scr];
#endif

   for(i = 0; i < desktop_data->numIconsUsed; i++)
   {
      if(desktop_data->desktopWindows[i]->shell == shell ||
                      desktop_data->desktopWindows[i]->drawA == shell)
      {
         found = True;
         break;
      }
   }
   if(found)
   {
      DrawAInput(desktop_data->popupMenu->popup,
                 desktop_data->desktopWindows[i],
                 event);
   }
}


#ifdef SHAPE
void
#ifdef _NO_PROTO
GenerateShape( desktopWindow )
      DesktopRec *desktopWindow;
#else
GenerateShape(
      DesktopRec *desktopWindow)
#endif /* _NO_PROTO */
{
   Dimension shadowThickness, highlightThickness;
   Dimension pixmapWidth, pixmapHeight;
   Dimension stringWidth, stringHeight;
   XRectangle rect[2];
   unsigned char flags;
   unsigned char pixmapPosition;
   unsigned char alignment;
   Arg args[4];
   DtIconGadget g;

   if(!shapeExtension)
      return;

   XtSetArg (args[0], XmNshadowThickness, &shadowThickness);
   XtSetArg (args[1], XmNhighlightThickness, &highlightThickness);
   XtSetArg (args[2], XmNpixmapPosition, &pixmapPosition);
   XtSetArg (args[3], XmNalignment, &alignment);
   XtGetValues (desktopWindow->iconGadget, args, 4);

   g = (DtIconGadget)desktopWindow->iconGadget;
   pixmapWidth = g->icon.pixmap_width;
   pixmapHeight = g->icon.pixmap_height;
   stringWidth = g->icon.string_width;
   stringHeight = g->icon.cache->string_height;

   _DtIconGetIconRects((DtIconGadget)desktopWindow->iconGadget,
                                                &flags, &rect[0], &rect[1]);

   switch ((int) pixmapPosition)
   {
      case XmPIXMAP_TOP:
        if(alignment != XmALIGNMENT_CENTER)
        {
           /* 1 */
            rect[0].x += 1;
            rect[0].y += 1;
            rect[0].width += 2*shadowThickness;
            rect[0].height += 2*shadowThickness;

           /* 2 */
            rect[1].x += 1;
            rect[1].y += 1;
            rect[1].width += 2*shadowThickness;
            rect[1].height += 2*shadowThickness;

            if(rect[0].width > rect[1].width)
               rect[1].width = rect[0].width;
         }
         else /* is XmALIGNMENT_CENTER */
         {
            if(stringWidth > pixmapWidth)
            {
              /* 1 */
               rect[0].x += 1;
               rect[0].y += 1;
               rect[0].width += 2*shadowThickness;
               rect[0].height += 2*shadowThickness;

              /* 2 */
               rect[1].x += 1;
               rect[1].y += 1;
               rect[1].width += 2*shadowThickness;
               rect[1].height += 2*shadowThickness;

               if(rect[0].width > rect[1].width)
                  rect[1].width = rect[0].width;
            }
            else
            {
             /* 1 */
               rect[0].x += 1;
               rect[0].y += 1;
               rect[0].width += 2*shadowThickness;
               rect[0].height += 2*shadowThickness;

              /* 2 */
               rect[1].x = rect[0].x;
               rect[1].y += 1;
               rect[1].width = rect[0].width;
               rect[1].height += 2*shadowThickness;
            }
         }

         break;
      case XmPIXMAP_LEFT:
         if((Dimension)(stringHeight+5) >= pixmapHeight)
         {
            if(pixmapHeight == 0)
            {
              /* 1 */
               rect[0].x = 0;
               rect[0].y = 0;
               rect[0].width = 0;
               rect[0].height = 0;

               rect[1].x += 1;
               rect[1].y += 1;
               rect[1].width += 2*shadowThickness;
               rect[1].height += 2*shadowThickness;
            }
            else
            {
              /* 2 */
               rect[1].y = rect[0].y + 1;
               rect[1].x += 1;
               rect[1].width += 2*shadowThickness;
               if(pixmapHeight > stringHeight)
                  rect[1].height += 2*shadowThickness +
                                       (pixmapHeight - stringHeight);
               else
                  rect[1].height += 2*shadowThickness;

              /* 1 */
               rect[0].x += 1;
               rect[0].y = rect[1].y;
               rect[0].width += shadowThickness;
               rect[0].height = rect[1].height;
            }
         }
         else
         {
           /* 1 */
            rect[0].x += 1;
            rect[0].y += 1;
            rect[0].width += 2*shadowThickness;
            rect[0].height += 2*shadowThickness;

           /* 2 */
            rect[1].x += 1;
            rect[1].y += 1;
            rect[1].width += 2*shadowThickness;
            rect[1].height += 2*shadowThickness;
         }
         break;
   }

   XShapeCombineRectangles(XtDisplay(desktopWindow->shell),
                           XtWindow(desktopWindow->shell),
                           ShapeBounding, 0, 0, &rect[0], 2,
                           ShapeSet, Unsorted);
}
#endif

#ifdef _TT_MULTI_SCREEN

static void
#ifdef _NO_PROTO
BuildDesktopLinks (display, scr)
       Display *display;
       int scr;
#else
BuildDesktopLinks (
       Display *display, int scr)
#endif /* _NO_PROTO */

#else  /* _TT_MULTI_SCREEN */

static void
#ifdef _NO_PROTO
BuildDesktopLinks (display)
       Display *display;
#else
BuildDesktopLinks (
       Display *display)
#endif /* _NO_PROTO */

#endif  /* _TT_MULTI_SCREEN */
{
   struct stat statBuf;
   Atom     *paWS;
   int       numInfo;
   Window   rootWindow;
   Screen   *currentScreen;
   int      i, screen, result;
   int      retry;

#ifdef _TT_MULTI_SCREEN
   DesktopData*  desktop_data;
#endif

   /* Build the 'Desktop' directory */
#ifdef _TT_MULTI_SCREEN
   desktop_dir = XtMalloc(strlen(users_home_dir) + strlen(DESKTOP_DIR) + 10);
#else
   desktop_dir = XtMalloc(strlen(users_home_dir) + strlen(DESKTOP_DIR) + 2);
#endif

   sprintf(desktop_dir, "%s%s", users_home_dir, DESKTOP_DIR);
   if (stat(desktop_dir, &statBuf) < 0)
      mkdir(desktop_dir, S_IRUSR|S_IWUSR|S_IXUSR);

   /* Now build the list of workspaces */
#ifdef _TT_MULTI_SCREEN
   currentScreen = XScreenOfDisplay(display, scr);
   desktop_data = gDesktopData[scr];
#else
   screen = XDefaultScreen(display);
   currentScreen = XScreenOfDisplay(display, screen);
#endif
   rootWindow = RootWindowOfScreen(currentScreen);

   retry = retryLoadDesktopInfo;
   if (retry <= 0)
     retry = 1;

   while (retry > 0)
   {
      result = DtWsmGetWorkspaceList (display, rootWindow, &paWS, &numInfo);
      if (result == Success)
      {
         desktop_data->numWorkspaces = (int)numInfo;
         break;
      }
      else
      {
         desktop_data->numWorkspaces = 1;
         sleep(2);
         retry--;
      }
   }

   desktop_data->workspaceData = (WorkspaceRec **)
               XtMalloc(sizeof(WorkspaceRec *) * desktop_data->numWorkspaces);
   for (i=0; i < desktop_data->numWorkspaces; i++)
   {
      WorkspaceRec *workspaceData;

      workspaceData = (WorkspaceRec *)XtMalloc(sizeof(WorkspaceRec));

      if(result == Success)
      {
         workspaceData->name = XGetAtomName (display, paWS[i]);
         CleanUpWSName(workspaceData->name);
      }
      else
         workspaceData->name = XtNewString("One");
      workspaceData->number = i + 1;
      workspaceData->files_selected = 0;
      workspaceData->selectedDTWindows =
                         (DesktopRec **) XtMalloc (sizeof (DesktopRec *));
      workspaceData->selectedDTWindows[0] = NULL;
      workspaceData->primaryHelpDialog = NULL;
      workspaceData->secondaryHelpDialogCount = 0;
      workspaceData->secondaryHelpDialogList = NULL;

      desktop_data->workspaceData[i] = workspaceData;
   }

   if (result == Success)
   {
      XFree ((char *) paWS);
      paWS = NULL;
   }
}

static int
#ifdef _NO_PROTO
InitializeNewWorkspaces (display, workspaceName)
       Display *display;
       char *workspaceName;
#else
InitializeNewWorkspaces (
       Display *display,
       char *workspaceName)
#endif /* _NO_PROTO */
{
   struct stat statBuf;
   Atom     *paWS;
   int       numInfo;
   Window   rootWindow;
   Screen   *currentScreen;
   int      i, j, k, screen;
   Boolean  haveIt;

   /* Now build the directories for each desktop (i.e. workspace) */
#ifdef _TT_MULTI_SCREEN
   int          scr;
   int	       retScreen = -1;
   DesktopData* desktop_data;

   for ( scr = 0; scr < gNScreens; scr++ )
   {
      currentScreen = XScreenOfDisplay(display, scr);
      desktop_data = gDesktopData[scr];
#else
   screen = XDefaultScreen(display);
   currentScreen = XScreenOfDisplay(display, screen);
#endif
   rootWindow = RootWindowOfScreen(currentScreen);

   if (DtWsmGetWorkspaceList (display, rootWindow, &paWS, &numInfo) == Success)
   {
      if(numInfo <= desktop_data->numWorkspaces)
      {
         /* should never get here, just put the new object in ws 0 */
         return(0);
      }

      /* go realloc room for the workspace Data */
      desktop_data->workspaceData = (WorkspaceRec **) XtRealloc(
                               (char *)desktop_data->workspaceData,
                               sizeof(WorkspaceRec *) * numInfo);

      /* now initialize the new workspaces */
      for(i=0; i < numInfo - desktop_data->numWorkspaces; i++)
      {
         WorkspaceRec *workspaceData;

         workspaceData = (WorkspaceRec *)XtMalloc(sizeof(WorkspaceRec));

         for(j=0; j < numInfo; j++)
         {
            workspaceData->name = XGetAtomName (display, paWS[j]);
            CleanUpWSName(workspaceData->name);
            haveIt = False;
            for(k=0; k<desktop_data->numWorkspaces; k++)
            {
               if(strcmp(workspaceData->name,
                            desktop_data->workspaceData[k]->name) == 0)
               {
                  haveIt = True;
                  break;
               }
            }
            if(!haveIt)
               break;
         }

         workspaceData->number = desktop_data->numWorkspaces + i + 1;
         workspaceData->files_selected = 0;
         workspaceData->selectedDTWindows =
                            (DesktopRec **) XtMalloc (sizeof (DesktopRec *));
         workspaceData->selectedDTWindows[0] = NULL;
         workspaceData->primaryHelpDialog = NULL;
         workspaceData->secondaryHelpDialogCount = 0;
         workspaceData->secondaryHelpDialogList = NULL;

         desktop_data->workspaceData[desktop_data->numWorkspaces + i] =
                                                               workspaceData;
      }
      XFree ((char *) paWS);
      paWS = NULL;
   }

   /* now lets add more desktop grids for the new workspaces */

   desktop_grid_size = numInfo * numColumns * numRows;
   desktop_grid = (Boolean *)XtRealloc((char *)desktop_grid,desktop_grid_size);
   for(i = desktop_data->numWorkspaces; i < numInfo; i++)
   {
      for(j = 0; j < numColumns; j++)
         for(k = 0; k < numRows; k++)
            desktop_grid[(i * numRows * numColumns) +
                                           (j * numRows) + k] = False;
   }
   desktop_data->numWorkspaces = numInfo;

   /* now determine which workspace the new object is in */
#ifdef _TT_MULTI_SCREEN
   /* Only want to get into this code once */
   if (retScreen < 0)
   {
      /* now determine which workspace the new object is in */
      for(j = 0; j < desktop_data->numWorkspaces; j++)
      {
         if(strcmp(workspaceName, desktop_data->workspaceData[j]->name) == 0)
            retScreen = (desktop_data->workspaceData[j]->number);
      }
   }

   } /* End screen for */

   return retScreen;

#else
   for(j = 0; j < desktop_data->numWorkspaces; j++)
   {
      if(strcmp(workspaceName, desktop_data->workspaceData[j]->name) == 0)
         return(desktop_data->workspaceData[j]->number);
   }

   /* should never get here */
   return(0);
#endif
}

/**********************************************************************
 *
 * CleanUpWSName - routine which makes sure there are no spaces, "/",
 *     "'", and ":" in a workspace name. If there are it changes them to
 *     "_".
 *
 *********************************************************************/
void
#ifdef _NO_PROTO
CleanUpWSName(workspace_name)
       char *workspace_name;
#else
CleanUpWSName(
       char *workspace_name)
#endif
{
   char *ptr;

   /* the workspace name could be something other than "One" "Two" etc.
       if the user has change his/her xrdb.  Need to check for characters
       in the name which will break us.
      First check " "
   */
   ptr = DtStrchr(workspace_name, ' ');
   while(ptr != NULL)
   {
      *ptr = '_';
      ptr = DtStrchr(ptr, ' ');
   }

   /*
     Next check "/"
   */
   ptr = DtStrchr(workspace_name, '/');
   while(ptr != NULL)
   {
      *ptr = '_';
      ptr = DtStrchr(ptr, '/');
   }

   /*
     Next check ":"
   */
   ptr = DtStrchr(workspace_name, ':');
   while(ptr != NULL)
   {
      *ptr = '_';
      ptr = DtStrchr(ptr, ':');
   }

   /*
     Next check "'"
   */
   ptr = DtStrchr(workspace_name, '\'');
   while(ptr != NULL)
   {
      *ptr = '_';
      ptr = DtStrchr(ptr, '\'');
   }
}


/**********************************************************************
 *
 *  CheckDesktoMarquee - given a x, y, width, and height determine
 *        whether a desktop objects pixmap falls within the bounded
 *        region.  If it does select it. If it is selected and no
 *        longer is in the bounded region, deslect it.
 *
 ***********************************************************************/
#ifdef _TT_MULTI_SCREEN

void
#ifdef _NO_PROTO
CheckDesktopMarquee(x, y, width, height, scr)
   int x;
   int y;
   int width;
   int height;
   int scr;
#else
CheckDesktopMarquee(
   int x,
   int y,
   int width,
   int height, 
   int scr)
#endif /* _NO_PROTO */

#else  /* _TT_MULTI_SCREEN */

void
#ifdef _NO_PROTO
CheckDesktopMarquee(x, y, width, height)
   int x;
   int y;
   int width;
   int height;
#else
CheckDesktopMarquee(
   int x,
   int y,
   int width,
   int height)
#endif /* _NO_PROTO */

#endif  /* _TT_MULTI_SCREEN */
{
   int i, j;
   Window   rootWindow;
   Atom     pCurrent;
   Screen   *currentScreen;
   int      screen;
   char     *workspace_name;
   Dimension shadowThickness;
   XRectangle pixmapRect;
   XRectangle labelRect;
   XRectangle incoming_rect;
   unsigned char pixmapPosition;
   DtIconGadget g;
   WorkspaceRec *workspaceData;
   unsigned char flags;
   Region region;
   Display *display;
   Boolean match;
   Boolean pointInRegion;

#ifdef _TT_MULTI_SCREEN
   DesktopData*		desktop_data = gDesktopData[scr];
#endif

   if(desktop_data->numIconsUsed <= 0)
      return;

   /* Create a region with the incoming x, y, width, and height */
   incoming_rect.x = (short)x;
   incoming_rect.y = (short)y;
   incoming_rect.width = (unsigned short)width;
   incoming_rect.height = (unsigned short)height;
   region = XCreateRegion();
   XUnionRectWithRegion (&incoming_rect, region, region);

   /* Get the current workspace */
#ifdef _TT_MULTI_SCREEN
   display = XtDisplay(desktop_data->desktopWindows[0]->shell);
   currentScreen = XScreenOfDisplay(display, scr);
   rootWindow = RootWindowOfScreen(currentScreen);
#else
   display = XtDisplay(desktop_data->desktopWindows[0]->shell);
   screen = XDefaultScreen(display);
   currentScreen = XScreenOfDisplay(display, screen);
   rootWindow = RootWindowOfScreen(currentScreen);
#endif

   if(DtWsmGetCurrentWorkspace(display, rootWindow, &pCurrent) == Success)
   {
      workspace_name = XGetAtomName (display, pCurrent);
      CleanUpWSName(workspace_name);
   }
   else
      workspace_name = XtNewString(desktop_data->workspaceData[0]->name);

   for(i = 0; i < desktop_data->numWorkspaces; i++)
   {
      if(strcmp(workspace_name, desktop_data->workspaceData[i]->name) == 0)
      {
         workspaceData = desktop_data->workspaceData[i];
         break;
      }
   }

   /* now lets loop through the icons used, first check to see if they are
    * in the same workspace. If not continue else we need to check to see
    * if they are in the region passed in.
    */
   for(i = 0; i < desktop_data->numIconsUsed; i++)
   {
      if(strcmp(desktop_data->desktopWindows[i]->workspace_name,
                                                  workspace_name) != 0)
         continue;

      /* first we need to get the pixmap region for the desktop object */
      g = (DtIconGadget)desktop_data->desktopWindows[i]->iconGadget;
      shadowThickness = g->gadget.shadow_thickness;
      pixmapPosition = g->icon.cache->pixmap_position;

      _DtIconGetIconRects((DtIconGadget)g, &flags, &pixmapRect, &labelRect);

      switch ((int) pixmapPosition)
      {
         case XmPIXMAP_TOP:  /* 0 */
         case XmPIXMAP_LEFT: /* 2 */
            if( flags & XmPIXMAP_RECT )
            {
              pixmapRect.x = pixmapRect.x + desktop_data->desktopWindows[i]->root_x + 1;
              pixmapRect.y = pixmapRect.y + desktop_data->desktopWindows[i]->root_y + 1;
              pixmapRect.width += 2*shadowThickness;
              pixmapRect.height += 2*shadowThickness;

              /* now lets check to see if the upper left and lower right corners
               * of the pixmap falls within the passed in region
               */
              pointInRegion = (XPointInRegion(region, pixmapRect.x, pixmapRect.y) &&
                               XPointInRegion(region, pixmapRect.x + pixmapRect.width,
                                              pixmapRect.y + pixmapRect.height));
            }
            else if( flags & XmLABEL_RECT )
            { /* For the ones that don't have pixmap */
              labelRect.x = labelRect.x + desktop_data->desktopWindows[i]->root_x + 1;
              labelRect.y = labelRect.y + desktop_data->desktopWindows[i]->root_y + 1;
              labelRect.width += 2*shadowThickness;
              labelRect.height += 2*shadowThickness;

              /* now lets check to see if the upper left and lower right corners
               * of the pixmap falls within the passed in region
               */
              pointInRegion = (XPointInRegion(region, labelRect.x, labelRect.y) &&
                               XPointInRegion(region, labelRect.x + labelRect.width,
                                              labelRect.y + labelRect.height));
            }
            else
              pointInRegion = False;

            if( pointInRegion )
            {
               /* it falls within the region, lets see if its already
                * selected.
                */
               match = False;
               for(j = 0; j < workspaceData->files_selected; j++)
               {
                  if(workspaceData->selectedDTWindows[j] ==
                                            desktop_data->desktopWindows[i])
                  {
                     match = True;
                     break;
                  }
               }

               if(match)
                  continue;

               /* its not already selected, so lets select it ... */
               workspaceData->files_selected++;

               workspaceData->selectedDTWindows =
                     (DesktopRec **) XtRealloc ((char *)
                                  workspaceData->selectedDTWindows,
                                  sizeof(DesktopRec *) *
                                  (workspaceData->files_selected + 2));

               /* Add to the front of the list */
               for (j = workspaceData->files_selected; j > 0; j--)
               {
                  workspaceData->selectedDTWindows[j] =
                                   workspaceData->selectedDTWindows[j-1];
               }

               workspaceData->selectedDTWindows[0] =
                                       desktop_data->desktopWindows[i];
               SetToSelectColors ((Widget)g,
                                 desktop_data->desktopWindows[i]->frame,
                                 LINK_FILE);
               XSync(display, False);
            }
            else
            {
               /* its pixmap is not it the region so lets see if it is
                * selected.  If it is lets deslect it.
                */
               for(j = 0; j < workspaceData->files_selected; j++)
               {
                  if(workspaceData->selectedDTWindows[j] ==
                                              desktop_data->desktopWindows[i])
                  {
                     DeselectDTFile (workspaceData,
                                          desktop_data->desktopWindows[i]);
                     XSync(display, False);
                     break;
                  }
               }
            }
            break;
      }
   }
   XDestroyRegion (region);
   XtFree(workspace_name);
}


static void
#ifdef _NO_PROTO
ProcessDropOnDesktopObject (w, dropInfo, desktopWindow)
     Widget w;
     DtDndDropCallbackStruct *dropInfo;
     DesktopRec *desktopWindow;
#else
ProcessDropOnDesktopObject (
     Widget w,
     DtDndDropCallbackStruct *dropInfo,
     DesktopRec *desktopWindow)
#endif /* _NO_PROTO */
{
   char *command = NULL;

   /******************/
   /* transfer phase */
   /******************/
   if (dropInfo->reason != DtCR_DND_DROP_ANIMATE)
   {
      DPRINTF(("DropOnObject: Transfer Callback\n"));

      /* In case when the drag is from non File Manager client */
      if(!dragActive)
	initiating_view = NULL;

      /* check for invalid trash drop */
      if (FileFromTrash(dropInfo->dropData->data.files[0]))
      {
         if (InvalidTrashDragDrop(dropInfo->operation,
               FROM_TRASH,
               desktopWindow->shell))
         {
            dropInfo->status = DtDND_FAILURE;
            return;
         }
      }

      command = TypeToAction(dropInfo->operation,
                       desktopWindow->file_view_data->file_data->logical_type);
      if(command &&
         (strncmp("FILESYSTEM_", command, strlen("FILESYSTEM_")) != 0) &&
         dropInfo->dropData->protocol == DtDND_BUFFER_TRANSFER)
      {
         dropInfo->completeMove = True;
         DtDtsFreeAttributeValue(command);
      }
      else
         /* set the complete move flag to False since we will be handling */
         /* the deletion of the original file                             */
         dropInfo->completeMove = False;
   }

   /******************************************/
   /* animate phase, retrieve action and run */
   /******************************************/
   else
   {
      DPRINTF(("DropOnObject: Animate Callback\n"));

      command =
        TypeToAction(dropInfo->operation,
                     desktopWindow->file_view_data->file_data->logical_type);
      if (command)
      {
         RunDTCommand (command,
                       desktopWindow,
                       dropInfo);
         DtDtsFreeAttributeValue(command);
      }
   }
}


static void
#ifdef _NO_PROTO
DropOnDesktopObject (w, client_data, call_data)
     Widget w;
     XtPointer client_data;
     XtPointer call_data;
#else
DropOnDesktopObject (
     Widget w,
     XtPointer client_data,
     XtPointer call_data)
#endif /* _NO_PROTO */
{
   DtDndDropCallbackStruct *dropInfo = (DtDndDropCallbackStruct *)call_data;

   switch (dropInfo -> dropData->protocol)
     {
       case DtDND_FILENAME_TRANSFER:
       case DtDND_BUFFER_TRANSFER:
         ProcessDropOnDesktopObject (w, dropInfo, (DesktopRec *) client_data);
         break;
       default :
         dropInfo->status = DtDND_FAILURE;
     } /* endswitch */
}

void
#ifdef _NO_PROTO
WorkSpaceRemoved (w, deleted_ws_atom,type, client_data)
     Widget w;
     Atom deleted_ws_atom;
     int type;
     XtPointer client_data;
#else
WorkSpaceRemoved (
     Widget w,
     Atom deleted_ws_atom,
     int type,
     XtPointer client_data)
#endif /* _NO_PROTO */
{
  Display  *display;
  Window   rootWindow;
  Atom     pCurrent;
  Screen   *currentScreen;
  int      screen;
  char     *workspace_name,*old_workspace_name;
  Atom     *paWS;
  int      numInfo,oldNumWorkspaces,old_ws_num,ws_num;
  int      i,j,k,retry,result,old_index,new_index;
#ifdef _TT_MULTI_SCREEN
  DesktopData* desktop_data;
#endif

  display = XtDisplay(w);
#ifdef _TT_MULTI_SCREEN
  currentScreen = XtScreen(w);
  screen = XScreenNumberOfScreen(currentScreen);
  desktop_data = gDesktopData[screen];
#else
  screen = XDefaultScreen(display);
  currentScreen = XScreenOfDisplay(display, screen);
#endif
  rootWindow = RootWindowOfScreen(currentScreen);
  oldNumWorkspaces = desktop_data->numWorkspaces;

  old_workspace_name = XGetAtomName (display, deleted_ws_atom);

  if( type == DtWSM_MODIFY_WORKSPACE_TYPE_DELETE )
  {
    retry = retryLoadDesktopInfo;
    if (retry <= 0)
      retry = 1;
    /* Get total new number of workspaces */
    while (retry > 0)
    {
       result = DtWsmGetWorkspaceList (display, rootWindow, &paWS, &numInfo);
       if (result == Success)
       {
          desktop_data->numWorkspaces = (int)numInfo;
          break;
       }
       else
       {
          desktop_data->numWorkspaces = 1;
          sleep(2);
          retry--;
       }
    }
    /* Get the current workspace */
    if(DtWsmGetCurrentWorkspace(display,rootWindow,&pCurrent) == Success)
    {
       workspace_name = XGetAtomName (display, pCurrent);
       CleanUpWSName(workspace_name);
    }
    else
       workspace_name = XtNewString(desktop_data->workspaceData[0]->name);

    /* determine the workspaces number of deleted and current*/
#ifdef _TT_MULTI_SCREEN
    old_ws_num = GetWorkspaceNum(old_workspace_name,oldNumWorkspaces,
                                 &old_index, screen);
    ws_num = GetWorkspaceNum(workspace_name,oldNumWorkspaces,
                             &new_index, screen);

    /* Free up or reassign any workspace data in the deleted workspace */
    FreeUpOldWorkspace(oldNumWorkspaces,old_ws_num,ws_num,old_index,
                       new_index, screen);
#else
    old_ws_num = GetWorkspaceNum(old_workspace_name,oldNumWorkspaces,&old_index);
    ws_num = GetWorkspaceNum(workspace_name,oldNumWorkspaces,&new_index);
   
    /* Free up or reassign any workspace data in the deleted workspace */
    FreeUpOldWorkspace(oldNumWorkspaces,old_ws_num,ws_num,old_index,new_index);

#endif
   
    /* Look up for the duplication of workspace objects in the new workspace */
    for(i=0;i<desktop_data->numIconsUsed;i++)
    {    /* Those objects whose workspace number matches the deleted workspace number */
       if (desktop_data->desktopWindows[i]->workspace_num == old_ws_num)
       {  /* Look up in all of the workspace objects */
         for(j=0;j<desktop_data->numIconsUsed;j++)
	 {  /* Those objects in the current workspace */
           if (j!= i && desktop_data->desktopWindows[j]->workspace_num == ws_num)
           {
	     /* If there is an object matching the old workspace object delete the old */
	     if(strcmp(desktop_data->desktopWindows[j]->file_name,desktop_data
	       ->desktopWindows[i]->file_name) == 0)
	     {
#ifdef _TT_MULTI_SCREEN
               ws_num = GetWorkspaceNum(workspace_name,desktop_data->
                                        numWorkspaces, &new_index, screen);
#else
               ws_num = GetWorkspaceNum(workspace_name,desktop_data->
                                        numWorkspaces, &new_index);
#endif
	       for(k=0;
		 k<desktop_data->workspaceData[new_index]->files_selected;k++)
               {
                /* Adjust the selectedDTWindows of current to accommadate the old list */
		  if(desktop_data->workspaceData[new_index]->
		       selectedDTWindows[k] == desktop_data->desktopWindows[i])
		  {
		    desktop_data->workspaceData[new_index]->
					selectedDTWindows[k] = NULL;
		    for(;k<desktop_data->workspaceData[new_index]->files_selected;k++)
		      desktop_data->workspaceData[new_index]->
	 	       selectedDTWindows[k] = desktop_data->
			  workspaceData[new_index]-> selectedDTWindows[k+1];
		      desktop_data->workspaceData[new_index]->files_selected--;
		    break;
		  }
               }
               XtPopdown(desktop_data->desktopWindows[i]->shell);
               XWithdrawWindow(XtDisplay(desktop_data->desktopWindows[i]->shell),
                      XtWindow(desktop_data->desktopWindows[i]->shell),
                      XDefaultScreen(XtDisplay(desktop_data->desktopWindows[i]->shell)));
	       FreeDesktopWindow(desktop_data->desktopWindows[i]);
	       i--;  /* To make sure that other icons are also covered */
	       break;
	     }
	   }
	 }
	 if(j == desktop_data->numIconsUsed)
	 {
           desktop_data->desktopWindows[i]->workspace_num  = ws_num;
           XtFree(desktop_data->desktopWindows[i]->workspace_name);
           desktop_data->desktopWindows[i]->workspace_name = NULL;
           desktop_data->desktopWindows[i]->workspace_name = 
                                      XtNewString(workspace_name);
	 }
       }
    }
  }
  else if( type == DtWSM_MODIFY_WORKSPACE_TYPE_ADD )
  {
     InitializeNewWorkspaces(display, old_workspace_name);
  }
  XFree(old_workspace_name);
}

static int
#ifdef _TT_MULTI_SCREEN
# ifdef _NO_PROTO
GetWorkspaceNum(workspace_name,NumWorkspaces,workspace_index, scr)
char *workspace_name;
int  NumWorkspaces;
int  *workspace_index;
int  scr;
# else
GetWorkspaceNum(char *workspace_name,
                int NumWorkspaces,
                int  *workspace_index, 
                int scr)
# endif /* _NO_PROTO */

#else /* _TT_MULTI_SCREEN */
#ifdef _NO_PROTO
GetWorkspaceNum(workspace_name,NumWorkspaces,workspace_index)
char *workspace_name;
int  NumWorkspaces;
int  *workspace_index;
#else
GetWorkspaceNum(char *workspace_name,
                int NumWorkspaces,
                int  *workspace_index)
#endif
#endif /* _TT_MULTI_SCREEN */
{
  int j,ws_num;
#ifdef _TT_MULTI_SCREEN
  DesktopData* desktop_data = gDesktopData[scr];
#endif

  for(j = 0; j < NumWorkspaces; j++)
  {
    if(strcmp(workspace_name, desktop_data->workspaceData[j]->name) == 0)
    {
        ws_num = desktop_data->workspaceData[j]->number;
        break;
    }
  }
  *workspace_index = (j == NumWorkspaces)?-1:j;
  return ws_num;
}


static void
#ifdef _TT_MULTI_SCREEN

#ifdef _NO_PROTO
FreeUpOldWorkspace(oldNumWorkspaces,old_ws_num,ws_num,old_index,new_index,scr)
   int oldNumWorkspaces,old_ws_num,ws_num;
   int  old_index;
   int  new_index;
   int  scr;
#else
FreeUpOldWorkspace(
   int oldNumWorkspaces,
   int old_ws_num,
   int ws_num,
   int  old_index,
   int  new_index, 
   int  scr)
#endif

#else /* _TT_MULTI_SCREEN */

#ifdef _NO_PROTO
FreeUpOldWorkspace(oldNumWorkspaces,old_ws_num,ws_num,old_index,new_index)
   int oldNumWorkspaces,old_ws_num,ws_num;
   int  old_index;
   int  new_index;
#else
FreeUpOldWorkspace(
   int oldNumWorkspaces,
   int old_ws_num,
   int ws_num,
   int  old_index,
   int  new_index)
#endif

#endif /* _TT_MULTI_SCREEN */
{
   int j,numnewfiles,numoldfiles;

#ifdef _TT_MULTI_SCREEN
  DesktopData* desktop_data = gDesktopData[scr];
#endif

   if(desktop_data->workspaceData[old_index] == NULL) /* Something  is wrong */
     return;
   numnewfiles = desktop_data->workspaceData[new_index]->files_selected;
   numoldfiles = desktop_data->workspaceData[old_index]->files_selected;
   if(numoldfiles)
   {
     desktop_data->workspaceData[new_index]->selectedDTWindows=(DesktopRec **) 
      XtRealloc((XtPointer) desktop_data->workspaceData[new_index]->selectedDTWindows,
          sizeof(DesktopRec *) * ( numnewfiles+numoldfiles));
     for(j=numnewfiles;j< numnewfiles+numoldfiles;j++)
     {
       desktop_data->workspaceData[new_index]->selectedDTWindows[j] = 
      desktop_data->workspaceData[old_index]->selectedDTWindows[j-numnewfiles];
     }
     XtFree((XtPointer) desktop_data->workspaceData[old_index]->selectedDTWindows);
      desktop_data->workspaceData[old_index]->selectedDTWindows = NULL;
     desktop_data->workspaceData[new_index]->files_selected = numnewfiles+numoldfiles;
   }

   numnewfiles=desktop_data->workspaceData[new_index]->secondaryHelpDialogCount;
   numoldfiles=desktop_data->workspaceData[old_index]->secondaryHelpDialogCount;
   if(numoldfiles)
   {
     desktop_data->workspaceData[new_index]->secondaryHelpDialogList=
      (DialogData **) XtRealloc((XtPointer) desktop_data->workspaceData[old_index]->
    secondaryHelpDialogList,sizeof(DialogData *) * ( numnewfiles+numoldfiles));
     for(j=numnewfiles;j< numnewfiles+numoldfiles;j++)
     {
      desktop_data->workspaceData[new_index]->secondaryHelpDialogList[j] =
      desktop_data->workspaceData[old_index]->secondaryHelpDialogList[j-ws_num];
     }
     desktop_data->workspaceData[new_index]->secondaryHelpDialogCount = numnewfiles+numoldfiles;
   }
   /* Move up the workspaceData */
   XtFree((XtPointer) desktop_data->workspaceData[old_index]);
    desktop_data->workspaceData[old_index] = NULL;
   for(j=old_index; j<desktop_data->numWorkspaces;j++)
      desktop_data->workspaceData[j] = desktop_data->workspaceData[j+1];
   desktop_data->workspaceData[j] = NULL;
}
/************************************************************************
 *
 ************************************************************************/

void
#ifdef _NO_PROTO
PutOnWorkspaceHandler( ttMsg )
   Tt_message ttMsg;
#else
PutOnWorkspaceHandler(
   Tt_message ttMsg)
#endif /* _NO_PROTO */
{
  DesktopRec * desktopWindow;
  struct stat fileInfo;
  char title[256], invalidWorkspace = 0x0;
  int numArgs, i, screen, ws;
  char * fullName, * fileName = NULL, * dirName = NULL, * workspace = NULL;
  char * requestWorkspace = NULL;

#ifdef _TT_MULTI_SCREEN
  /* SCW: @@@: This is just a temp fix. We need to find a decent way to 
   * determine 'what screen to put on'.  Probably, add a "-screen" argument
   * to the TT message, but this has potentially far reaching implications.
   */
  DesktopData* desktop_data = gDesktopData[0];
#endif

  fullName = tt_message_file( ttMsg );

  if( tt_is_err( tt_ptr_error( fullName ) ) )
  { /* No file name */
    tt_message_reply( ttMsg );
    tttk_message_destroy( ttMsg );
    return;
  }

  DtEliminateDots( fullName );

  if( stat( fullName, &fileInfo ) != 0
      && lstat( fullName, &fileInfo ) != 0 )
  { /* File does not exist */
    char * template, * errorMsg, * dialogTitle;

    tt_message_reply( ttMsg );
    tttk_message_destroy( ttMsg );

    dialogTitle = XtNewString(GETMESSAGE(20,84,"Put in Workspace"));

    template = XtNewString( GETMESSAGE(28,13, "Object:\n\n   %s\n\ndoes not exist in the file system."));
    errorMsg = (char *) XtMalloc( strlen( template )
                                  + strlen( fullName ) + 1 );
    sprintf( errorMsg, template, fullName );
    _DtMessage( toplevel, dialogTitle, errorMsg, NULL, HelpRequestCB );

    XtFree( template );
    XtFree( errorMsg );
    tt_free( fullName );
    return;
  }

  numArgs = tt_message_args_count( ttMsg );
  if( tt_is_err( tt_int_error( numArgs ) ) )
    numArgs = 0;

  for( i = 0; i < numArgs; ++i )
  {
    char * vtype;

    vtype = tt_message_arg_type( ttMsg, i );

    if( (vtype == 0)
        || (tt_is_err( tt_ptr_error( vtype ))) )
      continue;

    if( strcmp( vtype, "-workspace" ) == 0 )
    {
      requestWorkspace = tt_message_arg_val( ttMsg, i );
    }
    tt_free( vtype );
  }

  /* separate the rest of the path into directory and file name */
  if( strcmp( fullName, "/" ) == 0 )
  {
    sprintf( title, "%s:%s", home_host_name, root_title );
    fileName = XtNewString( "." );
    dirName = XtNewString( "/" );
  }
  else
  {
    char * ptr;
    ptr = strrchr( fullName, '/' );
    if( ptr == fullName )
    {
      strcpy( title, ptr + 1 );
      dirName = XtNewString( "/" );
      fileName = XtNewString( ptr + 1 );
    }
    else if( *(ptr + 1) != '\0' )
    {
      char savedChar = *ptr;
      *ptr = '\0';
      dirName = XtNewString( fullName );
      *ptr = savedChar;
      strcpy( title, ptr + 1 );
      fileName = XtNewString( ptr + 1 );
    }
    else
    {
      tt_message_reply( ttMsg );
      tttk_message_destroy( ttMsg );

      tt_free( fullName );
      return;
    }
  }

  tt_free( fullName );
  workspace = XtNewString( requestWorkspace );
  tt_free( requestWorkspace );

  if( strcmp( workspace, "current" ) == 0 )
  {
    Display  * display;
    Window   rootWindow;
    Atom     pCurrent;
    Screen   * currentScreen;
    int      screen;

    XtFree( workspace );

    /* Get the current workspace */
    display = XtDisplay( toplevel );
    screen = XDefaultScreen( display );
    currentScreen = XScreenOfDisplay( display, screen );
    rootWindow = RootWindowOfScreen( currentScreen );

    if( DtWsmGetCurrentWorkspace( display, rootWindow, &pCurrent ) == Success )
      workspace = XGetAtomName( display, pCurrent );
    else
      workspace = XtNewString( desktop_data->workspaceData[0]->name );
  }

  ws = -1;
  /* determine the workspace number
  */
  for( i = 0; i < desktop_data->numWorkspaces; ++i )
  {
    if( strcmp( workspace, desktop_data->workspaceData[i]->name ) == 0 )
    {
      ws = desktop_data->workspaceData[i]->number - 1;
      break;
    }
  }

  if( ws == -1 )
  { /* Invalid workspace
       choose workspace one
    */
    ws = 0;
    XtFree( workspace );
    workspace = XtNewString( "ws0" );
    invalidWorkspace = 0x1;
  }

  for( i = 0; i < desktop_data->numIconsUsed; ++i )
  {
    if( strcmp( desktop_data->desktopWindows[i]->title, title ) == 0
        && strcmp( desktop_data->desktopWindows[i]->workspace_name,
                   workspace ) == 0 )
    {
      char * template, * errorMsg, * dialogTitle;

      dialogTitle = XtNewString(GETMESSAGE(20,84,"Put in Workspace"));
      if( invalidWorkspace )
        template = XtNewString( GETMESSAGE(28,12, "An invalid workspace was passed. The default workspace will be\nworkspace one and an object named:\n\n   %s\n\nalready exists on the backdrop in this Workspace.\nYou cannot put another object with the same name on the Workspace.\nTo have both objects on the Workspace, rename one of them."));
      else
        template = XtNewString( GETMESSAGE(28,11, "An object named:\n\n   %s\n\nalready exists on the backdrop in this Workspace.\nYou cannot put another object with the same name on the Workspace.\nTo have both objects on the Workspace, rename one of them."));
      errorMsg = (char *) XtMalloc( strlen( template )
                                    + strlen( title ) + 1 );
      sprintf( errorMsg, template, title );
      _DtMessage( toplevel, dialogTitle, errorMsg, NULL, HelpRequestCB );

      XtFree( errorMsg );
      XtFree( workspace );
      XtFree( dirName );
      XtFree( fileName );
      XtFree( dialogTitle );
      XtFree( template );

      tt_message_reply( ttMsg );
      tttk_message_destroy( ttMsg );

      return;
    }
  }

  /* get the next available cached iconWindow and use it to map
     this topLevelWindow */
  desktopWindow = desktop_data->desktopWindows[desktop_data->numIconsUsed];
  desktop_data->numIconsUsed++;
  desktop_data->numCachedIcons--;

  /* store some of the data needed for the Desktop object */
  desktopWindow->host = XtNewString( home_host_name );
  desktopWindow->dir_linked_to = dirName;
  desktopWindow->file_name = fileName;
  desktopWindow->title = XtNewString( title );

  desktopWindow->workspace_name = XtNewString( desktop_data->workspaceData[ws]->name );
  desktopWindow->workspace_num = desktop_data->workspaceData[ws]->number;
  desktopWindow->restricted_directory = NULL;
  desktopWindow->root_type = NULL;
  desktopWindow->helpVol = NULL;
  desktopWindow->toolbox = False;
  desktopWindow->view = UNSET_VALUE;
  desktopWindow->order = UNSET_VALUE;
  desktopWindow->direction = UNSET_VALUE;
  desktopWindow->positionEnabled = UNSET_VALUE;

  desktopWindow->file_view_data = BuildNewOrderlist(
                              NULL /* @@@ use cached info if possible! */,
                              desktopWindow->iconGadget,
                              home_host_name,
                              desktopWindow->dir_linked_to,
                              fileName,
                              desktopWindow->registered,
                              desktopWindow->toolbox );

  MakeDesktopWindow( desktopWindow, -1, -1 );
  desktopWindow->registered = desktopWindow->file_view_data->registered;

  /* go cache some more windows if there isn't enough cached */
  if( desktop_data->numCachedIcons < 5 )
    InitializeDesktopWindows( 5, XtDisplay( toplevel ) );

  /* go save the current state of the desktop */
  SaveDesktopInfo( NORMAL_RESTORE );

  tt_message_reply( ttMsg );
  tttk_message_destroy( ttMsg );

  XtFree( workspace );

  if( checkBrokenLink != 0 && checkBrokenLinkTimerId == NULL )
  {
    checkBrokenLinkTimerId = XtAppAddTimeOut(
                                 XtWidgetToApplicationContext( toplevel ),
                                 checkBrokenLink * 1000,
                                 TimerEventBrokenLinks,
                                 NULL );
  }

}
#ifdef _TT_MULTI_SCREEN
static char*
GetDesktopFilename(int scr)
{
   static char* name = NULL;
   static int   lastScr = -1;

   if (scr != lastScr)	/* Is Previously built filename is still correct? */
   {
      /* Free Previous, allocate new */
      if (name)   XtFree(name);	/* Free Previous */
      name = XtMalloc(strlen(DESKTOP_SAVE_NAME) + 10);

      if (scr == 0)
	 /* For screen 0, don't append screen num.  This will allow some 
	  * backward compat. */
	 sprintf(name, "%s", DESKTOP_SAVE_NAME);
      else
	 sprintf(name, "%s_%d", DESKTOP_SAVE_NAME, scr);
      
      /* Don't rebuild this if not needed */
      lastScr = scr;
   }

   return name;
}
#endif

