/* $XConsortium: Menu.c /main/cde1_maint/2 1995/08/29 19:39:19 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           Menu.c
 *
 *   COMPONENT_NAME: Desktop File Manager (dtfile)
 *
 *   Description:    Contains the menu creation code for the file manager
 *                   main window, and the simpler callback functions.
 *
 *   FUNCTIONS: ActivateMultipleSelect
 *		ActivateNoSelect
 *		ActivateSingleSelect
 *		ChangeToHome
 *		ChangeToParent
 *		CreateMenu
 *		GetHomeDir
 *		GetMainMenuData
 *		NewView
 *		RereadDirectoryMenu
 *		RestoreMenuSensitivity
 *		SelectAll
 *		SetMenuSensitivity
 *		ShowTrash
 *		TerminalCWD
 *		TrashFiles
 *		UnmanageAllActionItems
 *		UnselectAll
 * 
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <sys/stat.h>

#include <Xm/XmP.h>
#include <Xm/CascadeBG.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>
#include <Xm/ToggleBG.h>
#include <Xm/MessageB.h>
#include <Xm/MenuShellP.h>

#include <X11/ShellP.h>

#include <Dt/DtP.h>
#include <Dt/DtNlUtils.h>
#include <Dt/Action.h>
#include <Dt/ActionP.h>
#include <Dt/Connect.h>
#include <Dt/Indicator.h>
#include <Dt/FileM.h>

#include "Encaps.h"
#include "SharedProcs.h"
#include "MultiView.h"

#include "FileMgr.h"
#include "Desktop.h"
#include "Main.h"
#include "Help.h"
#include "SharedMsgs.h"

typedef enum { ArgNone, ArgCWD, ArgRoot, ArgSelected } MenuActionArg;

#define INDICATOR_TIME 12

static int menuItemCount = 48;
const int MaxAnyFolderActions = 10;
const int MaxAnyFolderViewActions = 10;
const int MaxAnyObjectActions = 10;

#ifdef _TT_MULTI_SCREEN

  /* Declared in Main.h, Allocated in Main.c */
  MenuDesc** gMainMenu;

  /* These pups are now local */
  static Widget * moveBtn		= 0;
  static Widget * linkBtn		= 0;

  static Widget * create_dataBtn	= 0;
  static Widget * renameBtn		= 0;
  static Widget * duplicateBtn		= 0;
  static Widget * findBtn		= 0;
  static Widget * create_directoryBtn	= 0;
  static Widget * change_directoryBtn	= 0;
  static Widget * preferencesBtn	= 0;
  static Widget * filterBtn		= 0;
  static Widget * defaultEnvBtn		= 0;
  static Widget * homeBarBtn		= 0;
  static Widget * upBarBtn		= 0;
  static Widget * newViewBtn		= 0;
  static Widget * cleanUpBtn		= 0;
  static Widget * actionsId		= 0;
  static Widget * directoryBarBtn	= 0;
  static Widget * showHiddenMenu	= 0;
  static Widget * usingHelp		= 0;
  static Widget * fileManagerHelp	= 0;
  static Widget * applicationManagerHelp= 0;
  static Widget * rootFolderActionBtns[ MaxRootFolderActions ];
  static Widget * rootFolderActionSeparator	= 0;

#else 

static MenuDesc * mainMenu = NULL;

#endif /* _TT_MULTI_SCREEN */

static Widget * actionsPaneId = NULL;
static char * homeDir = NULL;

/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static void GetMainMenuData() ;
static void RereadDirectoryMenu() ;
static void ShowTrash() ;
static void NewView() ;
static void SetMenuSensitivity() ;
static void RestoreMenuSensitivity() ;
static void ActionInvokeOnCWD () ;
static void ActionInvokeOnRoot () ;
static void ActionInvokeOnSelected () ;
static void ActionInvoke () ;

#else
static void GetMainMenuData (
                        Widget mbar,
                        XtPointer * mainRecordPtr,
                        MenuDesc ** menuDescPtr,
                        int * menuItemCountPtr,
                        unsigned int ** globalMenuStatesPtr,
                        unsigned int ** viewMenuStatesPtr) ;
static void RereadDirectoryMenu(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void ShowTrash(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void NewView(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void SetMenuSensitivity(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void RestoreMenuSensitivity(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void ActionInvokeOnCWD (
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void ActionInvokeOnRoot (
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void ActionInvokeOnSelected (
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void ActionInvoke (
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/


/************************************************************************
 *
 *  CreateMenu
 *	Create the pull down menuing system for the program.
 *
 ************************************************************************/ 

Widget
#ifdef _NO_PROTO
CreateMenu( main, file_mgr_rec )
        Widget main ;
        FileMgrRec *file_mgr_rec ;
#else
CreateMenu(
        Widget main,
        FileMgrRec *file_mgr_rec )
#endif /* _NO_PROTO */
{
   static Boolean createSharedComponents = True;
#ifndef _TT_MULTI_SCREEN
   static Widget * actionsId;
   static Widget * directoryBarBtn;
#endif
   int i, j;
   Widget menu_bar;
   char **actions;
   int actionNum;
   Arg args[5];

#ifdef _TT_MULTI_SCREEN
   int  scr = XScreenNumberOfScreen (XtScreen(main));

   /* We are using one shared menu per screen */
   createSharedComponents = False;

   /* See if shared menu for current screen has been created */
   if (!gMainMenu[scr])
#else
   if (createSharedComponents)
#endif /* _TT_MULTI_SCREEN */
   {

#ifdef _TT_MULTI_SCREEN
     /* Create the menu for the current screen */
      createSharedComponents = True;  /* Force new menu creation */

     /* NOTE: In order to avoid ifdef'ng this whole function, which 
      * is bad for later merging, I'll just use the dreaded #define trick
      * to 'replace' mainMenu with gMainMenu[scr] 
      */
# define   mainMenu 	gMainMenu[scr]

#endif

      mainMenu = (MenuDesc *)
	 XtCalloc(
	      sizeof(MenuDesc)
	    * (  menuItemCount + MaxAnyFolderActions + MaxAnyFolderViewActions
	       + MaxRootFolderActions + MaxAnyObjectActions
	       + 3 /* for separators */ ),
	    1);
      j = 0;

      for (i = 0; i < menuItemCount; i++)
      {
         mainMenu[i].helpCallback = HelpRequestCB;
         mainMenu[i].helpData = NULL;
         mainMenu[i].activateCallback = NULL;
         mainMenu[i].activateData = NULL;
         mainMenu[i].maskBit = NULL;
         mainMenu[i].isHelpBtn = False;
         mainMenu[i].label = NULL;
         mainMenu[i].mnemonic = NULL;
      }


      /*************************************/
      /* Create the 'File' menu components */
      /*************************************/

      mainMenu[j].type = MENU_PULLDOWN_BUTTON;
      mainMenu[j].label = GETMESSAGE(20, 1, "File");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 2, "F");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].name = "file";
      mainMenu[j++].helpData = HELP_FILE_MENU_STR;

      mainMenu[j].type = SHARED_MENU_PANE;
      mainMenu[j].name = "fileMenu";
      mainMenu[j++].helpData = HELP_FILE_MENU_STR;

      create_directoryBtn = &(mainMenu[j].widget);
      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].maskBit = CREATE_DIR;
      mainMenu[j].label = GETMESSAGE(20, 130, "New Folder...");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 131, "N");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_FILE_MENU_STR;
      mainMenu[j].name = "newFolder";
      mainMenu[j++].activateCallback = ShowMakeFileDialog;

      create_dataBtn = &(mainMenu[j].widget);
      mainMenu[j].maskBit = CREATE_FILE;
      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].label = GETMESSAGE(20, 132, "New File...");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 133, "w");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_FILE_MENU_STR;
      mainMenu[j].name = "newFile";
      mainMenu[j++].activateCallback = ShowMakeFileDialog;

      mainMenu[j].type = MENU_SEPARATOR;
      mainMenu[j].name = "separator";
      mainMenu[j++].helpCallback = NULL;

      homeBarBtn = &(mainMenu[j].widget);
      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].label = GETMESSAGE(20, 38, "Go Home");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 41, "H");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_FILE_MENU_STR;
      mainMenu[j].name = "goHome";
      mainMenu[j++].activateCallback = ChangeToHome;

      upBarBtn = &(mainMenu[j].widget);
      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].maskBit = MOVE_UP;
      mainMenu[j].label = GETMESSAGE(20, 155, "Go Up");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20,94, "U");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_FILE_MENU_STR;
      mainMenu[j].name = "goUp";
      mainMenu[j++].activateCallback = ChangeToParent;

      change_directoryBtn = &(mainMenu[j].widget);
      mainMenu[j].maskBit = CHANGEDIR;
      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].label = GETMESSAGE(20, 134, "Go To...");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 135, "G");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_FILE_MENU_STR;
      mainMenu[j].name = "changeTo";
      mainMenu[j++].activateCallback = ShowChangeDirDialog;

      findBtn = &(mainMenu[j].widget);
      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].maskBit = FIND;
      mainMenu[j].label = GETMESSAGE(20, 18, "Find...");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 19, "F");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_FILE_MENU_STR;
      mainMenu[j].name = "find";
      mainMenu[j++].activateCallback = ShowFindDialog;

      mainMenu[j].type = MENU_SEPARATOR;
      mainMenu[j].name = "separator";
      mainMenu[j++].helpCallback = NULL;

      actions = _DtCompileActionVector("ANY_FOLDER");
      actionNum = 0;
      while (   (actions != 0) && (actions[actionNum] != 0)
	     && (actionNum < MaxAnyFolderActions))
      {
	 mainMenu[j].type = MENU_BUTTON;
	 mainMenu[j].label = DtActionLabel( actions[actionNum] );
	 mainMenu[j].mnemonic = 0;
	 mainMenu[j].helpData = HELP_FILE_MENU_STR;
	 mainMenu[j].name = "anyFolder";
	 mainMenu[j].activateData = actions[actionNum];
	 mainMenu[j++].activateCallback = ActionInvokeOnCWD;
	 menuItemCount++;
         actionNum++;
      }
      if ((actions != 0) && (*actions != 0)) {
	 mainMenu[j].type = MENU_SEPARATOR;
	 mainMenu[j].name = "separator";
	 mainMenu[j++].helpCallback = NULL;
	 menuItemCount++;
      }

      actions = _DtCompileActionVector("ANY_FOLDER_VIEW");
      actionNum = 0;
      while (   (actions != 0) && (actions[actionNum] != 0)
	     && (actionNum < MaxAnyFolderViewActions))
      {
	 mainMenu[j].type = MENU_BUTTON;
	 mainMenu[j].label = DtActionLabel( actions[actionNum] );
	 mainMenu[j].mnemonic = 0;
	 mainMenu[j].helpData = HELP_FILE_MENU_STR;
	 mainMenu[j].name = "anyFolderView";
	 mainMenu[j].activateData = actions[actionNum];
	 mainMenu[j++].activateCallback = ActionInvoke;
	 menuItemCount++;
         actionNum++;
      }
      if ((actions != 0) && (*actions != 0)) {
	 mainMenu[j].type = MENU_SEPARATOR;
	 mainMenu[j].name = "separator";
	 mainMenu[j++].helpCallback = NULL;
	 menuItemCount++;
      }

      for (actionNum = 0; actionNum < MaxRootFolderActions; actionNum++) {
	 rootFolderActionBtns[ actionNum ] = &(mainMenu[j].widget);
	 mainMenu[j].type = MENU_BUTTON;
	 mainMenu[j].mnemonic = 0;
	 mainMenu[j].helpData = HELP_FILE_MENU_STR;
	 mainMenu[j].name = "rootFolder";
	 mainMenu[j].label = mainMenu[j].name ;
	 mainMenu[j].activateData = 0;
	 mainMenu[j++].activateCallback = ActionInvokeOnRoot;
	 menuItemCount++;
      }
      rootFolderActionSeparator = &(mainMenu[j].widget);
      mainMenu[j].type = MENU_SEPARATOR;
      mainMenu[j].name = "separator";
      mainMenu[j++].helpCallback = NULL;
      menuItemCount++;

      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].label = GETMESSAGE(20, 117, "Close");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 118, "C");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_FILE_MENU_STR;
      mainMenu[j].name = "close";
      mainMenu[j++].activateCallback = Close;

      /***************************************/
      /* Create the 'Action' menu components */
      /***************************************/

      selectedMenuMinSize = 0;

      actionsId = &(mainMenu[j].widget);
      mainMenu[j].type = MENU_PULLDOWN_BUTTON;
      mainMenu[j].label = GETMESSAGE(20, 7, "Selected");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 8, "S");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].name = "actions";
      mainMenu[j++].helpData = HELP_SELECTED_MENU_STR;

      actionsPaneId = &(mainMenu[j].widget);
      mainMenu[j].type = SHARED_MENU_PANE;
      mainMenu[j].name = "actionMenu";
      mainMenu[j++].helpData = HELP_SELECTED_MENU_STR;

      moveBtn = &(mainMenu[j].widget);
      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].maskBit = MOVE;
      mainMenu[j].label = GETMESSAGE(20, 152, "Move to...");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 125, "M");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_MOVE_AS_COMMAND_STR;
      mainMenu[j].name = "move";
      mainMenu[j++].activateCallback = ShowMoveFileDialog;
      selectedMenuMinSize++;

      duplicateBtn = &(mainMenu[j].widget);
      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].maskBit = DUPLICATE;
      mainMenu[j].label = GETMESSAGE(20, 153, "Copy to...");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 145, "C");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_COPY_AS_COMMAND_STR;
      mainMenu[j].name = "copy";
      mainMenu[j++].activateCallback = ShowCopyFileDialog;
      selectedMenuMinSize++;

      linkBtn = &(mainMenu[j].widget);
      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].maskBit = LINK;
      mainMenu[j].label = GETMESSAGE(20, 154, "Copy As Link...");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 127, "L");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_LINK_AS_COMMAND_STR;
      mainMenu[j].name = "link";
      mainMenu[j++].activateCallback = ShowLinkFileDialog;
      selectedMenuMinSize++;

      mainMenu[j].type = MENU_SEPARATOR;
      mainMenu[j].name = "separator";
      mainMenu[j++].helpCallback = NULL;
      selectedMenuMinSize++;

      renameBtn = &(mainMenu[j].widget);
      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].maskBit = RENAME;
      mainMenu[j].label = GETMESSAGE(20, 137, "Rename...");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 110, "R");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_RENAME_COMMAND_STR;
      mainMenu[j].name = "rename";
      mainMenu[j++].activateCallback = ShowRenameFileDialog;
      selectedMenuMinSize++;

      mainMenu[j].maskBit = PUT_ON_DESKTOP;
      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].label = GETMESSAGE(20,84, "Put in Workspace");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20,85, "W");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_PUT_ON_DT_COMMAND_STR;
      mainMenu[j].name = "putInWorkspace";
      mainMenu[j++].activateCallback = PutOnDTCB;
      selectedMenuMinSize++;

      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].maskBit = TRASH;
      mainMenu[j].label = GETMESSAGE(20,151, "Put in Trash");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 91, "T");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_TRASH_FILES_COMMAND_STR;
      mainMenu[j].name = "putInTrash";
      mainMenu[j++].activateCallback = TrashFiles;
      selectedMenuMinSize++;

      actions = _DtCompileActionVector("ANY_OBJECT");
      actionNum = 0;
      while (   (actions != 0) && (actions[actionNum] != 0)
	     && (actionNum < MaxAnyObjectActions))
      {
	 mainMenu[j].type = MENU_BUTTON;
	 mainMenu[j].label = DtActionLabel( actions[actionNum] );
	 mainMenu[j].mnemonic = 0;
	 mainMenu[j].helpData = HELP_FILE_MENU_STR;
	 mainMenu[j].name = "anyObject";
	 mainMenu[j].activateData = actions[actionNum];
	 mainMenu[j++].activateCallback = ActionInvokeOnSelected;
	 selectedMenuMinSize++;
	 menuItemCount++;
         actionNum++;
      }

      mainMenu[j].type = MENU_SEPARATOR;
      mainMenu[j].name = "separator";
      mainMenu[j++].helpCallback = NULL;
      selectedMenuMinSize++;

      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].label = GETMESSAGE(20, 48, "Select All");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 51, "S");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_SELECT_ALL_COMMAND_STR;
      mainMenu[j].name = "selectAll";
      mainMenu[j++].activateCallback = SelectAll;
      selectedMenuMinSize++;

      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].label = GETMESSAGE(20, 52, "Deselect All");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 55, "D");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_UNSELECT_ALL_COMMAND_STR;
      mainMenu[j].name = "deselectAll";
      mainMenu[j++].activateCallback = UnselectAll;
      selectedMenuMinSize++;

      mainMenu[j].type = MENU_SEPARATOR;
      mainMenu[j].name = "separator";
      mainMenu[j++].helpCallback = NULL;
      selectedMenuMinSize++;

      /*************************************/
      /* Create the 'View' menu components */
      /*************************************/

      directoryBarBtn = &(mainMenu[j].widget);
      mainMenu[j].type = MENU_PULLDOWN_BUTTON;
      mainMenu[j].label = GETMESSAGE(20, 5, "View");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 6, "V");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].name = "view";
      mainMenu[j++].helpData = HELP_VIEW_MENU_STR;

      mainMenu[j].type = SHARED_MENU_PANE;
      mainMenu[j].name = "viewMenu";
      mainMenu[j++].helpData = HELP_VIEW_MENU_STR;

      if(openDirType == CURRENT)
      {
         newViewBtn = &(mainMenu[j].widget);
         mainMenu[j].type = MENU_BUTTON;
         mainMenu[j].label = GETMESSAGE(20, 138, "Open New View");
         mainMenu[j].label = XtNewString(mainMenu[j].label);
         mainMenu[j].mnemonic = GETMESSAGE(20, 139, "O");
         mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
         mainMenu[j].helpData = HELP_NEW_VIEW_COMMAND_STR;
         mainMenu[j].name = "new";
         mainMenu[j++].activateCallback = NewView;
      }
      else 
         menuItemCount--;

      mainMenu[j].type = MENU_SEPARATOR;
      mainMenu[j].name = "separator";
      mainMenu[j++].helpCallback = NULL;

      preferencesBtn = &(mainMenu[j].widget);
      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].maskBit = PREFERENCES;
      mainMenu[j].label = GETMESSAGE(20, 141, "Set View Options...");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 158, "V");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_PREFERENCES_COMMAND_STR;
      mainMenu[j].name = "setViewOptions";
      mainMenu[j++].activateCallback = ShowPreferencesDialog;

      defaultEnvBtn = &(mainMenu[j].widget);
      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].maskBit = SETTINGS;
      mainMenu[j].label = GETMESSAGE(20, 142, "Save As Default Options...");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 143, "S");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_SAVE_SETTINGS_COMMAND_STR;
      mainMenu[j].name = "saveViewOptions";
      mainMenu[j++].activateCallback = SaveSettingsCB;

      showHiddenMenu = &(mainMenu[j].widget);
      mainMenu[j].type = MENU_TOGGLE_BUTTON;
      mainMenu[j].label = GETMESSAGE(20,156, "Show Hidden Objects");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20,102, "H");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_SHOW_HIDDEN_COMMAND_STR;
      mainMenu[j].name = "showHiddenObjects";
      mainMenu[j++].activateCallback = ShowHiddenFiles;

      filterBtn = &(mainMenu[j].widget);
      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].maskBit = FILTER;
      mainMenu[j].label = GETMESSAGE(20,144, "Set Filter Options...");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 104, "F");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_FILTER_COMMAND_STR;
      mainMenu[j].name = "setFilterOptions";
      mainMenu[j++].activateCallback = ShowFilterDialog;

      mainMenu[j].type = MENU_SEPARATOR;
      mainMenu[j].name = "separator";
      mainMenu[j++].helpCallback = NULL;

      cleanUpBtn = &(mainMenu[j].widget);
      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].maskBit = CLEAN_UP;
      mainMenu[j].label = GETMESSAGE(20,99, "Clean Up");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20,100, "C");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_CLEAN_UP_COMMAND_STR;
      mainMenu[j].name = "cleanUp";
      mainMenu[j++].activateCallback = CleanUp;

      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].label = GETMESSAGE(20, 30, "Update");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 33, "U");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_REREAD_COMMAND_STR;
      mainMenu[j].name = "refresh";
      mainMenu[j++].activateCallback = RereadDirectoryMenu;

      /*************************************/
      /* Create the 'Help' menu components */
      /*************************************/

      mainMenu[j].type = MENU_PULLDOWN_BUTTON;
      mainMenu[j].isHelpBtn = True;
      mainMenu[j].label = GETMESSAGE(20, 123, "Help");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 9, "H");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].name = "help";
      mainMenu[j++].helpData = HELP_HELP_MENU_STR;

      mainMenu[j].type = SHARED_MENU_PANE;
      mainMenu[j].name = "helpMenu";
      mainMenu[j++].helpData = HELP_HELP_MENU_STR;

      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].label = GETMESSAGE(20, 105, "Overview");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 106, "v");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_HELP_MENU_STR;
      mainMenu[j].activateCallback = HelpRequestCB;
      mainMenu[j].name = "introduction";
      mainMenu[j++].activateData = HELP_HOME_TOPIC;

      mainMenu[j].type = MENU_SEPARATOR;
      mainMenu[j].name = "separator";
      mainMenu[j++].helpCallback = NULL;

      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].label = GETMESSAGE(20, 148, "Table Of Contents");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 149, "C");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_HELP_MENU_STR;
      mainMenu[j].activateCallback = HelpRequestCB;
      mainMenu[j].name = "toc";
      mainMenu[j++].activateData = HELP_TOC_STR;

      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].label = GETMESSAGE(20, 107, "Tasks");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20, 108, "T");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_HELP_MENU_STR;
      mainMenu[j].activateCallback = HelpRequestCB;
      mainMenu[j].name = "tasks";
      mainMenu[j++].activateData = HELP_TASKS_STR;

      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].label = GETMESSAGE(20,109, "Reference");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20,110, "R");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_HELP_MENU_STR;
      mainMenu[j].activateCallback = HelpRequestCB;
      mainMenu[j].name = "reference";
      mainMenu[j++].activateData = HELP_REFERENCE_STR;

      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].label = GETMESSAGE(20,111, "On Item");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20,112, "O");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_HELP_MENU_STR;
      mainMenu[j].activateCallback = HelpRequestCB;
      mainMenu[j].name = "onItem";
      mainMenu[j++].activateData = HELP_HELP_MODE_STR;

      mainMenu[j].type = MENU_SEPARATOR;
      mainMenu[j].name = "separator";
      mainMenu[j++].helpCallback = NULL;

      usingHelp = &(mainMenu[j].widget);
      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].label = GETMESSAGE(20,113, "Using Help");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20,114, "U");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_HELP_MENU_STR;
      mainMenu[j].activateCallback = HelpRequestCB;
      mainMenu[j].name = "usingHelp";
      mainMenu[j++].activateData = HELP_HOME_TOPIC;
   
      mainMenu[j].type = MENU_SEPARATOR;
      mainMenu[j].name = "separator";
      mainMenu[j++].helpCallback = NULL;

      fileManagerHelp = &(mainMenu[j].widget);
      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].label = GETMESSAGE(20,115, "About File Manager");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20,116, "A");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_HELP_MENU_STR;
      mainMenu[j].activateCallback = HelpRequestCB;
      mainMenu[j].name = "version";
      mainMenu[j++].activateData = HELP_ABOUT_STR;

      applicationManagerHelp = &(mainMenu[j].widget);
      mainMenu[j].type = MENU_BUTTON;
      mainMenu[j].label = GETMESSAGE(20,157, "About Application Manager");
      mainMenu[j].label = XtNewString(mainMenu[j].label);
      mainMenu[j].mnemonic = GETMESSAGE(20,116, "A");
      mainMenu[j].mnemonic = XtNewString(mainMenu[j].mnemonic);
      mainMenu[j].helpData = HELP_HELP_MENU_STR;
      mainMenu[j].activateCallback = HelpRequestCB;
      mainMenu[j].name = "version";
      mainMenu[j++].activateData = HELP_ABOUT_STR;
   }

   menu_bar = _DtCreateMenuSystem(main, "mainMenu", HelpRequestCB, 
                               HELP_HELP_MENU_STR, createSharedComponents, 
                               mainMenu, menuItemCount, GetMainMenuData,
                               SetMenuSensitivity, RestoreMenuSensitivity);

#ifdef _TT_MULTI_SCREEN
   /* If we created new widgets for a screen, then save them in our
    * widget array
    */
   if (createSharedComponents)
   {
      /* Save the global widgets defined in the help creation */
      gMenuWidgets[scr].actionsId = *actionsId;
      gMenuWidgets[scr].actionsPaneId = *actionsPaneId;
      gMenuWidgets[scr].directoryBarBtn = *directoryBarBtn;
      gMenuWidgets[scr].create_directoryBtn_child= NULL;
      gMenuWidgets[scr].create_dataBtn_child= NULL;
      gMenuWidgets[scr].create_dataBtn = *create_dataBtn;
      gMenuWidgets[scr].renameBtn = *renameBtn;
      gMenuWidgets[scr].moveBtn = *moveBtn;
      gMenuWidgets[scr].linkBtn = *linkBtn;
      gMenuWidgets[scr].duplicateBtn = *duplicateBtn;
      gMenuWidgets[scr].findBtn = *findBtn;
      gMenuWidgets[scr].create_directoryBtn = *create_directoryBtn;
      gMenuWidgets[scr].change_directoryBtn = *change_directoryBtn;
      gMenuWidgets[scr].homeBarBtn = *homeBarBtn;
      gMenuWidgets[scr].upBarBtn = *upBarBtn;
      gMenuWidgets[scr].newViewBtn = newViewBtn ? *newViewBtn : 0;
      gMenuWidgets[scr].cleanUpBtn = *cleanUpBtn;
      gMenuWidgets[scr].showHiddenMenu = *showHiddenMenu;
      gMenuWidgets[scr].filterBtn = *filterBtn;
      gMenuWidgets[scr].preferencesBtn = *preferencesBtn;
      gMenuWidgets[scr].defaultEnvBtn = *defaultEnvBtn;
      gMenuWidgets[scr].usingHelp = *usingHelp;
      gMenuWidgets[scr].fileManagerHelp = *fileManagerHelp;
      gMenuWidgets[scr].applicationManagerHelp = *applicationManagerHelp;
      for (actionNum = 0; actionNum < MaxRootFolderActions; actionNum++) {
	  gMenuWidgets[scr].rootFolderActionBtns[actionNum] =
	      *rootFolderActionBtns[actionNum];
      }
      gMenuWidgets[scr].rootFolderActionSeparator = *rootFolderActionSeparator;
   }


#endif  /* _TT_MULTI_SCREEN */

   /* Fine tune the menubar */
   XtSetArg(args[0], XmNmarginWidth, 2);
   XtSetArg(args[1], XmNmarginHeight, 2);
   XtSetValues(menu_bar, args, 2);

#ifdef _TT_MULTI_SCREEN
   /* Use the widget array */
   file_mgr_rec->actions = gMenuWidgets[scr].actionsId;
   file_mgr_rec->action_pane = gMenuWidgets[scr].actionsPaneId;
   file_mgr_rec->directoryBarBtn = gMenuWidgets[scr].directoryBarBtn;
#else /* _TT_MULTI_SCREEN */
   file_mgr_rec->actions = *actionsId;
   file_mgr_rec->action_pane = *actionsPaneId;
   file_mgr_rec->directoryBarBtn = *directoryBarBtn;
#endif /* _TT_MULTI_SCREEN */

   file_mgr_rec->create_directoryBtn_child= NULL;
   file_mgr_rec->create_dataBtn_child= NULL;

   ActivateNoSelect (file_mgr_rec);

   createSharedComponents = False;

   return (menu_bar);
}


static void 
#ifdef _NO_PROTO
GetMainMenuData (mbar, mainRecordPtr, menuDescPtr, menuItemCountPtr,
                  globalMenuStatesPtr, viewMenuStatesPtr)
   Widget mbar;
   XtPointer * mainRecordPtr;
   MenuDesc ** menuDescPtr;
   int * menuItemCountPtr;
   unsigned int ** globalMenuStatesPtr;
   unsigned int ** viewMenuStatesPtr;
#else
GetMainMenuData (
   Widget mbar,
   XtPointer * mainRecordPtr,
   MenuDesc ** menuDescPtr,
   int * menuItemCountPtr,
   unsigned int ** globalMenuStatesPtr,
   unsigned int ** viewMenuStatesPtr )
#endif /* _NO_PROTO */

{
   Arg args[1];
   FileMgrRec * fileMgrRec;

   XtSetArg(args[0], XmNuserData, &fileMgrRec);
   XtGetValues(mbar, args, 1);
   *mainRecordPtr = (XtPointer) fileMgrRec;
#ifdef _TT_MULTI_SCREEN
   *menuDescPtr = gMainMenu[XScreenNumberOfScreen(XtScreen(mbar))];
#else
   *menuDescPtr = mainMenu;
#endif
   *menuItemCountPtr = menuItemCount;
   *globalMenuStatesPtr = &currentMenuStates;
   *viewMenuStatesPtr = &fileMgrRec->menuStates;
   *actionsPaneId = fileMgrRec->action_pane;
}


void
#ifdef _NO_PROTO
GetHomeDir( )
#else
GetHomeDir( )
#endif
{
  struct passwd * pwInfo;

  if( homeDir == NULL )
  {
    if ((homeDir = getenv("HOME_CLIENT")) == NULL)
    {
      if ((homeDir = getenv("HOME")) == NULL)
      {
        pwInfo = getpwuid (getuid());
        homeDir = pwInfo->pw_dir;
      }
    }
  }
}


/************************************************************************
 *
 *  ActivateSingleSelect
 *	Activate the menu items that are to be active upon a
 *	single selection.
 *
 ************************************************************************/

void
#ifdef _NO_PROTO
ActivateSingleSelect( file_mgr_rec, file_type )
        FileMgrRec *file_mgr_rec ;
        char *file_type ;
#else
ActivateSingleSelect(
        FileMgrRec *file_mgr_rec,
        char *file_type )
#endif /* _NO_PROTO */
{
   DialogData * dialog_data;
   FileMgrData * file_mgr_data;
   char * fileName;

   if ((dialog_data = _DtGetInstanceData (file_mgr_rec)) == NULL)
      return;
   file_mgr_data = (FileMgrData *) dialog_data->data;

   if ((strcmp(file_mgr_data->selection_list[0]->file_data->file_name, ".." ) == 0 )
       || (strcmp(file_mgr_data->selection_list[0]->file_data->file_name, "." ) == 0 ) )
   {
     file_mgr_rec->menuStates |= TRASH | MODIFY | PUT_ON_DESKTOP;
     file_mgr_rec->menuStates &= (~RENAME);
   }
   else
     file_mgr_rec->menuStates |= RENAME | TRASH | MODIFY | PUT_ON_DESKTOP;

   if ((strcmp(file_type, LT_BROKEN_LINK) != 0)
       && (strcmp(file_type, LT_RECURSIVE_LINK) != 0)
/*
       && (file_mgr_data->selection_list[0]->file_data->physical_type !=
        DtDIRECTORY)
*/
       )
   {
      file_mgr_rec->menuStates |= MOVE | DUPLICATE | LINK;
   }
   else
      file_mgr_rec->menuStates &= ~(MOVE | DUPLICATE | LINK);

   /*  Update the action menu pane defined for the file type  */
   /*  of the selected file.                                  */

   fileName = _DtGetSelectedFilePath( file_mgr_data->selection_list[0] );
   UpdateActionMenuPane ((XtPointer)file_mgr_data, file_mgr_rec, file_type, 
                   NOT_DESKTOP, 0, (Widget)NULL, 
                   file_mgr_data->selection_list[0]->file_data->physical_type,
                   fileName);
   XtFree( fileName );
}



static void
#ifdef _NO_PROTO
UnmanageAllActionItems( file_mgr_rec )
        FileMgrRec *file_mgr_rec ;
#else
UnmanageAllActionItems(
        FileMgrRec *file_mgr_rec )
#endif /* _NO_PROTO */
{
  register int i, children;
  XmManagerWidget action_pane;

  action_pane = (XmManagerWidget) file_mgr_rec->action_pane;

  for (i = selectedMenuMinSize; i < action_pane->composite.num_children; ++i)
  {
    XtUnmanageChild (action_pane->composite.children[i]);
  }
}

/************************************************************************
 *
 *  ActivateMultipleSelect
 *	Activate (deactivate) the appropriate menu items when multiple
 *	files are selected.
 *
 *
 ************************************************************************/

void
#ifdef _NO_PROTO
ActivateMultipleSelect( file_mgr_rec )
        FileMgrRec *file_mgr_rec ;
#else
ActivateMultipleSelect(
        FileMgrRec *file_mgr_rec )
#endif /* _NO_PROTO */
{
   file_mgr_rec->menuStates |= TRASH | PUT_ON_DESKTOP;
   file_mgr_rec->menuStates &= ~(RENAME | MOVE | DUPLICATE | LINK |
                                 MODIFY);

   UnmanageAllActionItems (file_mgr_rec);
}




/************************************************************************
 *
 *  ActivateNoSelect
 *	Activate (deactivate) the appropriate menu items when no
 *	files are selected.
 *
 ************************************************************************/

void
#ifdef _NO_PROTO
ActivateNoSelect( file_mgr_rec )
        FileMgrRec *file_mgr_rec ;
#else
ActivateNoSelect(
        FileMgrRec *file_mgr_rec )
#endif /* _NO_PROTO */
{
   file_mgr_rec->menuStates &= ~(RENAME | MOVE | DUPLICATE | LINK | TRASH |
                                 MODIFY | PUT_ON_DESKTOP);

   UnmanageAllActionItems (file_mgr_rec);
}



/************************************************************************
 *
 *  UnselectAll
 *	Unselect all of the selected files.
 *
 ************************************************************************/

void
#ifdef _NO_PROTO
UnselectAll( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
UnselectAll(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
   FileMgrRec * file_mgr_rec;
   DialogData * dialog_data;
   FileMgrData * file_mgr_data;
   Arg args[1];
   Widget mbar;

   if ((int)client_data == FM_POPUP)
     mbar = XtParent(w);
   else
     mbar = XmGetPostedFromWidget(XtParent(w));

   XmUpdateDisplay (w);
   XtSetArg(args[0], XmNuserData, &file_mgr_rec);
   XtGetValues(mbar, args, 1);

   /* Ignore accelerators received after we're unposted */
   if ((dialog_data = _DtGetInstanceData (file_mgr_rec)) == NULL)
      return;
   file_mgr_data = (FileMgrData *) dialog_data->data;

   DeselectAllFiles (file_mgr_data);
   ActivateNoSelect (file_mgr_rec);

   if( file_mgr_data == trashFileMgrData )
     SensitizeTrashBtns();
}




/************************************************************************
 *
 *  SelectAll
 *	Select all of file displayed.
 *
 ************************************************************************/

void
#ifdef _NO_PROTO
SelectAll( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
SelectAll(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
   FileMgrRec * file_mgr_rec;
   DialogData * dialog_data;
   FileMgrData * file_mgr_data;
   Arg args[1];
   Widget mbar;

   if ((int)client_data == FM_POPUP)
     mbar = XtParent(w);
   else
     mbar = XmGetPostedFromWidget(XtParent(w));

   XmUpdateDisplay (w);
   XtSetArg(args[0], XmNuserData, &file_mgr_rec);
   XtGetValues(mbar, args, 1);

   /* Ignore accelerators received after we're unposted */
   if ((dialog_data = _DtGetInstanceData (file_mgr_rec)) == NULL)
      return;
   file_mgr_data = (FileMgrData *) dialog_data->data;

   SelectAllFiles (file_mgr_data);

   if( file_mgr_data == trashFileMgrData )
     SensitizeTrashBtns();
}




/************************************************************************
 *
 *  RereadDirectoryMenu
 *	Callback function invoked from the Reread Directory
 *	menu item.  This function calls the file manager functions
 *	which rereads and processes the directory.
 *
 ************************************************************************/

static void
#ifdef _NO_PROTO
RereadDirectoryMenu( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
RereadDirectoryMenu(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
   Arg args[1];
   Widget mbar = XmGetPostedFromWidget(XtParent(w));
   FileMgrRec * file_mgr_rec;
   DialogData * dialog_data;

   XmUpdateDisplay (w);

   XtSetArg(args[0], XmNuserData, &file_mgr_rec);
   XtGetValues(mbar, args, 1);

   /* Ignore accelerators received after we're unposted */
   if ((dialog_data = _DtGetInstanceData (file_mgr_rec)) == NULL)
      return;

   UnpostTextPath( (FileMgrData *) dialog_data->data );

   /* force this directory to be updated */
   FileMgrReread (file_mgr_rec);
}



/************************************************************************
 *
 *  TrashFiles
 *	Send the selected set of files to the trash.
 *
 ************************************************************************/ 

void
#ifdef _NO_PROTO
TrashFiles( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
TrashFiles(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
   FileMgrRec * file_mgr_rec;
   DialogData * dialog_data;
   FileMgrData * file_mgr_data;
   FileViewData * file_view_data;
   Arg args[1];
   Widget mbar;
   DtActionArg * action_args;
   int arg_count;
   register int i;

   if (!TrashIsInitialized())
   {
     char *title, *msg;

     title = XtNewString(GetSharedMessage(TRASH_ERROR_TITLE));

     msg = XtMalloc( strlen( GETMESSAGE(27,95, "Unable to access the Trash directory:\n   %s\n   All trash operations will not be performed.\n   The most common causes are:\n     - Network authentication.\n     - Insufficient disk space.\n     - Wrong permissions.") )
                     + strlen( trash_dir )
                     + 1 );

     sprintf( msg, GETMESSAGE(27, 95, "Unable to access the Trash directory:\n   %s\n   All trash operations will not be performed.\n   The most common causes are:\n     - Network authentication.\n     - Insufficient disk space.\n     - Wrong permissions."), trash_dir );

     _DtMessage(toplevel, title, msg, NULL, HelpRequestCB);
     XtFree(title);
     XtFree(msg);
     return;
   }

   XmUpdateDisplay (w);

   if((int)client_data == NULL)
      mbar = XmGetPostedFromWidget(XtParent(w));
   else
      mbar =  XtParent(w);

   XtSetArg(args[0], XmNuserData, &file_mgr_rec);
   XtGetValues(mbar, args, 1);

   /* Ignore accelerators when we're insensitive */
   if(client_data == NULL)
      if ((file_mgr_rec->menuStates & TRASH) == 0)
         return;

   /* Ignore accelerators received after we're unposted */
   if ((dialog_data = _DtGetInstanceData (file_mgr_rec)) == NULL)
      return;
   file_mgr_data = (FileMgrData *) dialog_data->data;

   /* 'widget_dragged' is set in InitiateIconDrag() and
      is never set to NULL.  Because of this, after doing a desktop drag
      if you do a 'PutInTrash' on the File Manager window object, in
      TrashRemoveHandler() in Trash.c, it displays an incorrect message,
      So set it to NULL here. */

   widget_dragged = NULL;

   /* get the file_view_data object from which the menu was invoked */
   if (client_data == NULL)
     file_view_data = NULL;
   else
   {
     file_view_data = file_mgr_data->popup_menu_icon;
     /* The following test makes sure that the it is not the case of 
        multiple selected files */
     if (!file_mgr_data->selected_file_count && file_view_data == NULL)
       /* the object has gone away (probably deleted) */
       return;

     file_mgr_data->popup_menu_icon = NULL;

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

   if(file_view_data)
      _DtBuildActionArgsWithSelectedFiles(&file_view_data, 1,
                                          &action_args, &arg_count);
   else
      _DtBuildActionArgsWithSelectedFiles(file_mgr_data->selection_list,
                                          file_mgr_data->selected_file_count,
                                          &action_args, &arg_count);
#ifdef _TT_MULTI_SCREEN
   /* Set the proper display for the action */
   _XmOSPutenv( gDisplayStrings[ XScreenNumberOfScreen(
        XtScreen(file_mgr_rec->shell)) ] );
#endif
   DtActionInvoke(file_mgr_rec->shell, TRASH_ACTION,
                  action_args, arg_count, NULL, NULL,
                  trash_dir, True, NULL, NULL);

   _DtFreeActionArgs(action_args, arg_count);
}




/************************************************************************
 *
 *  NewView
 *	Create a new view of the currently display directory
 *
 ************************************************************************/ 

static void
#ifdef _NO_PROTO
NewView( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
NewView(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
   FileMgrRec * file_mgr_rec;
   DialogData * dialog_data;
   FileMgrData * file_mgr_data;
   Arg args[1];
   Widget mbar = XmGetPostedFromWidget(XtParent(w));


   XmUpdateDisplay (w);
   XtSetArg(args[0], XmNuserData, &file_mgr_rec);
   XtGetValues(mbar, args, 1);

   /* Ignore accelerators received after we're unposted */
   if ((dialog_data = _DtGetInstanceData (file_mgr_rec)) == NULL)
      return;
   file_mgr_data = (FileMgrData *) dialog_data->data;

   initiating_view = (XtPointer) file_mgr_data;
   if(file_mgr_data->restricted_directory != NULL)
   {
      special_view = True;
      special_treeType = file_mgr_data->show_type;
      special_viewType = file_mgr_data->view;
      special_orderType = file_mgr_data->order;
      special_directionType = file_mgr_data->direction;
      special_randomType = file_mgr_data->positionEnabled;
      special_restricted = XtNewString(file_mgr_data->restricted_directory);
      if(file_mgr_data->title == NULL)
         special_title = NULL;
      else
         special_title = XtNewString(file_mgr_data->title);
      special_helpVol = XtNewString(file_mgr_data->helpVol);

      if(file_mgr_data->toolbox)
         GetNewView (file_mgr_data->host, file_mgr_data->current_directory,
                     file_mgr_data->restricted_directory, NULL, 0);
      else
         GetNewView (file_mgr_data->host, file_mgr_data->current_directory,
                     NULL, NULL, 0);
   }
   else
      GetNewView (file_mgr_data->host, file_mgr_data->current_directory, NULL,
                  NULL, 0);
   initiating_view = (XtPointer) NULL;
}




/************************************************************************
 *
 *  ChangeToHome
 *	Update the current view to show the home directory.
 *
 ************************************************************************/ 

void
#ifdef _NO_PROTO
ChangeToHome( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
ChangeToHome(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
   FileMgrRec * file_mgr_rec;
   DialogData * dialog_data;
   FileMgrData * file_mgr_data;
   char *host;
   char *ptr, *ptr1;
   Arg args[1];
   Widget mbar = XmGetPostedFromWidget(XtParent(w));
   Boolean found;

   XmUpdateDisplay (w);
   if((int)client_data == FM_POPUP)
      mbar = XtParent(w);
   else
      mbar = XmGetPostedFromWidget(XtParent(w));

   XtSetArg(args[0], XmNuserData, &file_mgr_rec);
   XtGetValues(mbar, args, 1);

  /* Ignore accelerators when we're insensitive */
   if ((file_mgr_rec->menuStates & HOME) == 0)
      return;

   /* Ignore accelerators received after we're unposted */
   if ((dialog_data = _DtGetInstanceData (file_mgr_rec)) == NULL)
      return;
   file_mgr_data = (FileMgrData *) dialog_data->data;

   found = False;

   if( homeDir == NULL )
     GetHomeDir( );

   if((ptr = DtStrchr(homeDir, ':')) != NULL)
   {
      ptr1 = DtStrchr(homeDir, '/');
      if(strlen(ptr) > strlen(ptr1))
      {
         *ptr = '\0';
         host = XtNewString(homeDir);
         ptr1 = ptr;
         ptr++;
         homeDir = ptr;
         *ptr1 = ':';
         found = True;
      }
      else
         host = home_host_name;
   }
   else
      host = home_host_name;

   if (file_mgr_data->cd_select != NULL)
   {
      XtFree (file_mgr_data->cd_select);
      file_mgr_data->cd_select = NULL;
   }

   ShowNewDirectory( file_mgr_data, host, homeDir );

   if( found )
      XtFree(host);
}




/************************************************************************
 *
 *  ChangeToParent
 *	Update the current view to show the parent directory.
 *
 ************************************************************************/ 

void
#ifdef _NO_PROTO
ChangeToParent( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
ChangeToParent(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
   FileMgrRec * file_mgr_rec;
   DialogData * dialog_data;
   FileMgrData * file_mgr_data;
   Arg args[1];
   Widget mbar = XmGetPostedFromWidget(XtParent(w));
   char host_name[MAX_PATH];
   Widget focus_widget;

   XmUpdateDisplay (w);
   if((int)client_data == FM_POPUP)
      mbar = XtParent(w);
   else
      mbar = XmGetPostedFromWidget(XtParent(w));

   XtSetArg(args[0], XmNuserData, &file_mgr_rec);
   XtGetValues(mbar, args, 1);

  /* Ignore accelerators when we're insensitive */
   if ((file_mgr_rec->menuStates & MOVE_UP) == 0)
      return;

   /* Ignore accelerators received after we're unposted */
   if ((dialog_data = _DtGetInstanceData (file_mgr_rec)) == NULL)
      return;

   focus_widget = XmGetFocusWidget(file_mgr_rec->file_window);
   if (focus_widget)
   {
      if (focus_widget == file_mgr_rec->current_directory_text)
      {
         XmTextFieldReplace(focus_widget,
                            (XmTextPosition) 0,
                            XmTextFieldGetInsertionPosition(focus_widget),
                            "");
         return;
      }
      else if (strcmp(focus_widget->core.name, "nameChangeT") == 0)
      {
         XmTextReplace(focus_widget,
                       (XmTextPosition) 0,
                       XmTextFieldGetInsertionPosition(focus_widget),
                       "");
         return;
      }
   }

   file_mgr_data = (FileMgrData *) dialog_data->data;

   if (strcmp (file_mgr_data->current_directory, "/") == 0)
      return;

   strcpy(host_name, file_mgr_data->host);

   if (file_mgr_data->cd_select != NULL)
   {
      XtFree (file_mgr_data->cd_select);
      file_mgr_data->cd_select = NULL;
   }

   ShowNewDirectory (file_mgr_data, host_name,
                     _DtPName (file_mgr_data->current_directory));

   if(strcmp(file_mgr_data->current_directory,
                   file_mgr_data->restricted_directory) == 0)
   {
#ifdef _TT_MULTI_SCREEN
      _DtGlobalUpdateMenuItemState(
          gMenuWidgets[XScreenNumberOfScreen(XtScreen(file_mgr_rec->shell))].upBarBtn,  
          MOVE_UP, False, &currentMenuStates);
#else
      _DtGlobalUpdateMenuItemState(*upBarBtn, MOVE_UP, False, &currentMenuStates);
#endif
      file_mgr_rec->menuStates &= ~(MOVE_UP);
   }
   else
   {
      file_mgr_rec->menuStates |= MOVE_UP;
#ifdef _TT_MULTI_SCREEN
      _DtGlobalUpdateMenuItemState(
	gMenuWidgets[XScreenNumberOfScreen(XtScreen(file_mgr_rec->shell))].upBarBtn, 
        MOVE_UP, True, &currentMenuStates);
#else
      _DtGlobalUpdateMenuItemState(*upBarBtn, MOVE_UP, True, &currentMenuStates);
#endif
   }
}




/************************************************************************
 *
 *  SetMenuSensitivity
 *	Called before each menupane is posted; it will correctly set
 *      the menu button sensitivity for the view from which the menu
 *      is being posted; this is necessary because the menupanes are
 *      shared.  Only those buttons whose state is different than that
 *      of the current menu will be changed.
 *
 ************************************************************************/ 

static void
#ifdef _NO_PROTO
SetMenuSensitivity( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
SetMenuSensitivity(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
   Arg args[1];
   Widget mbar;
   FileMgrRec * file_mgr_rec;
   XmMenuShellWidget mshell = (XmMenuShellWidget) w;
   DialogData * dialog_data;
   FileMgrData * file_mgr_data;
   char ** actions;
   int actionNum, actionsEnmenued;
#ifdef _TT_MULTI_SCREEN
   int scr = XScreenNumberOfScreen(XtScreen(w));
#endif

   _XmGetActiveTopLevelMenu(mshell->composite.children[0], &mbar);
   XtSetArg(args[0], XmNuserData, &file_mgr_rec);
   XtGetValues(mbar, args, 1);

#ifdef _TT_MULTI_SCREEN
   /* To make the code less cluttered, I'll just set all the local 
    * vars to the file_mgr_rec vars.
    */
   homeBarBtn 		= &(gMenuWidgets[scr].homeBarBtn);
   create_dataBtn 	= &(gMenuWidgets[scr].create_dataBtn);
   create_directoryBtn 	= &(gMenuWidgets[scr].create_directoryBtn);
   upBarBtn 		= &(gMenuWidgets[scr].upBarBtn);
   defaultEnvBtn 	= &(gMenuWidgets[scr].defaultEnvBtn);
   change_directoryBtn 	= &(gMenuWidgets[scr].change_directoryBtn);
   filterBtn 		= &(gMenuWidgets[scr].filterBtn);
   if (gMenuWidgets[scr].newViewBtn) {
	   newViewBtn	= &(gMenuWidgets[scr].newViewBtn);
   }
   showHiddenMenu 	= &(gMenuWidgets[scr].showHiddenMenu);
   findBtn 		= &(gMenuWidgets[scr].findBtn);
   fileManagerHelp	= &(gMenuWidgets[scr].fileManagerHelp);
   applicationManagerHelp = &(gMenuWidgets[scr].applicationManagerHelp);
   for (actionNum = 0; actionNum < MaxRootFolderActions; actionNum++) {
       rootFolderActionBtns[actionNum] =
	   &(gMenuWidgets[scr].rootFolderActionBtns[actionNum]);
   }
   rootFolderActionSeparator = &(gMenuWidgets[scr].rootFolderActionSeparator);
#endif  /* _TT_MULTI_SCREEN */

   /* Enable the appropriate menu widgets based on the view type */
   if ((dialog_data = _DtGetInstanceData (file_mgr_rec)) != NULL)
   {
      struct stat file_info;

      file_mgr_data = (FileMgrData *) dialog_data->data;

      if( stat( file_mgr_data->current_directory, &file_info ) == 0 )
      {
        if( ! (file_info.st_mode & S_IXUSR)
            || ! (file_info.st_mode & S_IWUSR) )
        {
          file_mgr_rec->menuStates &= ~(CREATE_DIR) & ~(CREATE_FILE);
          _DtGlobalUpdateMenuItemState(*create_dataBtn, CREATE_FILE, False, &currentMenuStates);
          _DtGlobalUpdateMenuItemState(*create_directoryBtn, CREATE_DIR, False, &currentMenuStates);
        }
        else if( ! ( file_mgr_data->toolbox ) ||
                   ( file_mgr_data->toolbox && geteuid() == root_user ) )
        {
          file_mgr_rec->menuStates |= CREATE_DIR | CREATE_FILE;
          _DtGlobalUpdateMenuItemState(*create_dataBtn, CREATE_FILE, True, &currentMenuStates);
          _DtGlobalUpdateMenuItemState(*create_directoryBtn, CREATE_DIR, True, &currentMenuStates);
        }
      }

      if (!PositioningEnabledInView(file_mgr_data))
         file_mgr_rec->menuStates &= ~(CLEAN_UP);
      else
         file_mgr_rec->menuStates |= CLEAN_UP;

      XtManageChild(*defaultEnvBtn);
      if(file_mgr_data->restricted_directory == NULL)
      {
        if(strcmp(file_mgr_data->current_directory, "/") == 0)
        {
          file_mgr_rec->menuStates &= ~(MOVE_UP);
          _DtGlobalUpdateMenuItemState( *upBarBtn, MOVE_UP,
                                        False,&currentMenuStates);
        }
        else
        {
          file_mgr_rec->menuStates |= MOVE_UP;
          _DtGlobalUpdateMenuItemState(*upBarBtn, MOVE_UP, True,&currentMenuStates);
        }

        if( homeDir == NULL )
          GetHomeDir( );

        if( homeDir != NULL
            && strcmp( homeDir, file_mgr_data->current_directory ) != 0 )
        {
          file_mgr_rec->menuStates |= HOME;
          _DtGlobalUpdateMenuItemState(*homeBarBtn, HOME, True,
                                       &currentMenuStates);
        }
        else
        {
          file_mgr_rec->menuStates &= ~(HOME);
          _DtGlobalUpdateMenuItemState(*homeBarBtn, HOME, False,
                                       &currentMenuStates);
        }

        file_mgr_rec->menuStates |= CHANGE_DIR;
        _DtGlobalUpdateMenuItemState(*change_directoryBtn, CHANGE_DIR, True, 
                                     &currentMenuStates);
        XtManageChild(*change_directoryBtn);

      }
      else
      {
         if(strcmp(file_mgr_data->current_directory, 
                         file_mgr_data->restricted_directory) == 0)
         {
            _DtGlobalUpdateMenuItemState(*upBarBtn, MOVE_UP, False, 
                                      &currentMenuStates);
            file_mgr_rec->menuStates &= ~(MOVE_UP);
         }
         else
         {
            file_mgr_rec->menuStates |= MOVE_UP;
            _DtGlobalUpdateMenuItemState(*upBarBtn, MOVE_UP, True,
                                      &currentMenuStates);
         }
         _DtGlobalUpdateMenuItemState(*homeBarBtn, HOME, False,&currentMenuStates);
         file_mgr_rec->menuStates &= ~(HOME);
         _DtGlobalUpdateMenuItemState(*change_directoryBtn, CHANGE_DIR, False, 
                                   &currentMenuStates);
         file_mgr_rec->menuStates &= ~(CHANGE_DIR);
      }

      actions = _DtCompileActionVector(file_mgr_data->root_type);
      for (actionNum = actionsEnmenued = 0;
	   (   (actions != 0) && (actions[actionNum] != 0)
	    && (actionsEnmenued < MaxRootFolderActions));
	   actionNum++)
      {
	 XmString xmStr;
	 char *actionLabel;
	 char *action;
	 if (strcmp(actions[actionNum], "Open") == 0) continue;
	 if (strcmp(actions[actionNum], "OpenInPlace") == 0) continue;
	 if (strcmp(actions[actionNum], "OpenNewView") == 0) continue;
	 if (strcmp(actions[actionNum], "Print") == 0) continue;
	 actionLabel = DtActionLabel( actions[actionNum] );
	 xmStr = XmStringCreateLocalized( actionLabel );
	 XtFree( actionLabel );
	 XtVaSetValues( *rootFolderActionBtns[ actionsEnmenued ],
			XmNlabelString, xmStr, 0 );
	 XmStringFree( xmStr );
	 XtVaGetValues( *rootFolderActionBtns[ actionsEnmenued ],
			XmNuserData, &action, 0 );
	 if (action != 0) XtFree( action );
	 XtVaSetValues( *rootFolderActionBtns[ actionsEnmenued ],
			XmNuserData, XtNewString( actions[actionNum]), 0 );
	 XtManageChild( *rootFolderActionBtns[ actionsEnmenued ] );
	 actionsEnmenued++;
      }
      _DtFreeStringVector( actions );
      if (actionsEnmenued > 0) {
	 XtManageChild( *rootFolderActionSeparator );
      } else {
	 XtUnmanageChild( *rootFolderActionSeparator );
      }
      /* unmanage unused rootFolder action buttons */
      for (; actionsEnmenued < MaxRootFolderActions; actionsEnmenued++) {
	 XtUnmanageChild( *rootFolderActionBtns[ actionsEnmenued ] );
      }

      XtManageChild(*preferencesBtn);
      XtManageChild(*filterBtn);
      if ((openDirType == CURRENT) && newViewBtn)
         XtManageChild(*newViewBtn);
      XtManageChild(*showHiddenMenu);

      XtManageChild(*findBtn);

      /* The problem is when users have 2 views
         and selected one file on one view and popup the
         'Selected' menu on the other view.
         The 'Selected' menu still show the other view action items.
      */
      if( file_mgr_data->selected_file_count == 0 )
        ActivateNoSelect( file_mgr_rec );
      else if( file_mgr_data->selected_file_count == 1 )
        ActivateSingleSelect( file_mgr_rec,
          file_mgr_data->selection_list[0]->file_data->logical_type );
      else
        ActivateMultipleSelect( file_mgr_rec );

      if( file_mgr_data->toolbox )
      {
        if( geteuid() == root_user || access(file_mgr_data->current_directory,
                                        W_OK|X_OK) == 0)
        {
          file_mgr_rec->menuStates |= CREATE_DIR
                                      | CREATE_FILE;
          _DtGlobalUpdateMenuItemState(*create_dataBtn, CREATE_FILE, True,
                                       &currentMenuStates);
          _DtGlobalUpdateMenuItemState(*create_directoryBtn, CREATE_DIR, True,
                                       &currentMenuStates);
        }
        else
        {
          file_mgr_rec->menuStates &= ~(CREATE_DIR
                                        | CREATE_FILE
                                        | MOVE
                                        | LINK
                                        | RENAME
                                        | TRASH);
          _DtGlobalUpdateMenuItemState(*create_dataBtn, CREATE_FILE, False,
                                       &currentMenuStates);
          _DtGlobalUpdateMenuItemState(*create_directoryBtn, CREATE_DIR, False,
                                       &currentMenuStates);
          _DtGlobalUpdateMenuItemState(*moveBtn, MOVE, False,
                                       &currentMenuStates);
          _DtGlobalUpdateMenuItemState(*linkBtn, LINK, False,
                                       &currentMenuStates);
          _DtGlobalUpdateMenuItemState(*renameBtn, RENAME, False,
                                       &currentMenuStates);
        }
      }
   }

   if(showFilesystem )
   {
      if (file_mgr_data->show_hid_enabled)
         XmToggleButtonGadgetSetState(*showHiddenMenu, True, False);
      else
         XmToggleButtonGadgetSetState(*showHiddenMenu, False, False);
   }

   if( file_mgr_data->toolbox )
   {
     if( ! XtIsManaged( *applicationManagerHelp ) )
       XtManageChild( *applicationManagerHelp );
     if( XtIsManaged( *fileManagerHelp ) )
       XtUnmanageChild( *fileManagerHelp );
   }
   else
   {
     if( ! XtIsManaged( *fileManagerHelp ) )
       XtManageChild( *fileManagerHelp );
     if( XtIsManaged( *applicationManagerHelp ) )
       XtUnmanageChild( *applicationManagerHelp );
   }

   _DtGlobalSetMenuSensitivity(w, client_data, call_data);
}




/************************************************************************
 *
 *  RestoreMenuSensitivity
 *	Called after each menupane is unposted; it will restore the
 *      sensitivity state of each menubutton to 'TRUE'; this is so that
 *      accelerators will work correctly.
 *
 ************************************************************************/ 

static void
#ifdef _NO_PROTO
RestoreMenuSensitivity( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
RestoreMenuSensitivity(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
   Arg args[1];
   Widget mbar;
   FileMgrRec * file_mgr_rec;
   XmMenuShellWidget mshell = (XmMenuShellWidget) w;
   DialogData * dialog_data;
   FileMgrData * file_mgr_data;
#ifdef _TT_MULTI_SCREEN
   int scr = XScreenNumberOfScreen(XtScreen(w));
#endif

   _XmGetActiveTopLevelMenu(mshell->composite.children[0], &mbar);
   XtSetArg(args[0], XmNuserData, &file_mgr_rec);
   XtGetValues(mbar, args, 1);

#ifdef _TT_MULTI_SCREEN
   /* To make the code less cluttered, I'll just set all the local 
    * vars to the file_mgr_rec vars.
    */
   upBarBtn   = &(gMenuWidgets[scr].upBarBtn);
   homeBarBtn = &(gMenuWidgets[scr].homeBarBtn);
   cleanUpBtn = &(gMenuWidgets[scr].cleanUpBtn);
#endif  /* _TT_MULTI_SCREEN */

   /* Enable the appropriate menu widgets based on the view type */
   dialog_data = _DtGetInstanceData (file_mgr_rec);
   file_mgr_data = (FileMgrData *) dialog_data->data;

   _DtGlobalRestoreMenuSensitivity(w, client_data, call_data);

   if(restrictMode) 
   {
      char *ptr;

      ptr = strrchr(users_home_dir, '/');
      *ptr = '\0';
      if(strcmp(file_mgr_data->current_directory, users_home_dir) == 0)
      {
         _DtGlobalUpdateMenuItemState(*upBarBtn, MOVE_UP, False,
                                      &currentMenuStates);
      }
      else
      {
         _DtGlobalUpdateMenuItemState(*upBarBtn, MOVE_UP, True,
                                      &currentMenuStates);
      }
      *ptr = '/';
   }
   else
     if ((currentMenuStates & MOVE_UP) == 0)
         _DtGlobalUpdateMenuItemState(*upBarBtn, MOVE_UP, True,&currentMenuStates);

   if ((currentMenuStates & CLEAN_UP) == 0)
      _DtGlobalUpdateMenuItemState(*cleanUpBtn, CLEAN_UP, True,&currentMenuStates);

}



static void 
#ifdef _NO_PROTO
InvokeAction (action, argType, w, client_data, call_data)
     const char *action;
     MenuActionArg argType;
     Widget w;
     XtPointer client_data;
     XtPointer call_data;
#else
InvokeAction (
     const char *action,
     MenuActionArg argType,
     Widget w,
     XtPointer client_data,
     XtPointer call_data)
#endif /* _NO_PROTO */
{
   FileMgrRec * file_mgr_rec;
   DialogData * dialog_data;
   FileMgrData * file_mgr_data;
   DtActionArg action_arg[1];
   DtActionArg * action_args;
   int arg_count;
   Widget mbar = XmGetPostedFromWidget(XtParent(w));

   XmUpdateDisplay (w);
   XtVaGetValues(mbar, XmNuserData, &file_mgr_rec, 0 );

   /* Ignore accelerators received after we're unposted */
   if ((dialog_data = _DtGetInstanceData (file_mgr_rec)) == NULL)
      return;
   file_mgr_data = (FileMgrData *) dialog_data->data;

   if (action == 0) {
      XtVaGetValues( w, XmNuserData, &action, 0 );
   }

   action_arg[0].argClass = DtACTION_FILE;
   switch (argType) {
      case ArgCWD:
         action_arg[0].u.file.name = file_mgr_data->current_directory;
	 action_args = action_arg;
	 arg_count = 1;
	 break;
      case ArgRoot:
         action_arg[0].u.file.name = file_mgr_data->restricted_directory ?
	     file_mgr_data->restricted_directory : "/";
	 action_args = action_arg;
	 arg_count = 1;
	 break;
      case ArgSelected:
	 _DtBuildActionArgsWithSelectedFiles(
		file_mgr_data->selection_list,
		file_mgr_data->selected_file_count,
		&action_args, &arg_count);
	 break;
      case ArgNone:
      default:
	 action_args = 0;
	 arg_count = 0;
	 break;
   }

#ifdef _TT_MULTI_SCREEN
   /* Set the proper display for the action */
   _XmOSPutenv( gDisplayStrings[ XScreenNumberOfScreen(
        XtScreen(file_mgr_rec->shell)) ] );
#endif
   DtActionInvoke(file_mgr_rec->shell, (char *)action,
		  action_args, arg_count, NULL, NULL,
		  file_mgr_data->current_directory,
		  True, NULL, NULL);
   if (argType == ArgSelected) {
      _DtFreeActionArgs( action_args, arg_count );
   }

}

/************************************************************************
 *
 *  ActionInvokeOnCWD
 *	Invoke an action on the current working directory of the dtfile view.
 *
 ************************************************************************/ 

static void 
#ifdef _NO_PROTO
ActionInvokeOnCWD (w, client_data, call_data)
     Widget w;
     XtPointer client_data;
     XtPointer call_data;
#else
ActionInvokeOnCWD (
     Widget w,
     XtPointer client_data,
     XtPointer call_data)
#endif /* _NO_PROTO */
{
   InvokeAction( (char *)client_data, ArgCWD, w, client_data, call_data );
}

/************************************************************************
 *
 *  ActionInvokeOnRoot
 *	Invoke an action on the root folder of the dtfile view.
 *
 ************************************************************************/ 

static void 
#ifdef _NO_PROTO
ActionInvokeOnRoot (w, client_data, call_data)
     Widget w;
     XtPointer client_data;
     XtPointer call_data;
#else
ActionInvokeOnRoot (
     Widget w,
     XtPointer client_data,
     XtPointer call_data)
#endif /* _NO_PROTO */
{
   InvokeAction( (char *)client_data, ArgRoot, w, client_data, call_data );
}

/************************************************************************
 *
 *  ActionInvokeOnSelected
 *	Invoke an action on the current working directory of the dtfile view.
 *
 ************************************************************************/ 

static void 
#ifdef _NO_PROTO
ActionInvokeOnSelected (w, client_data, call_data)
     Widget w;
     XtPointer client_data;
     XtPointer call_data;
#else
ActionInvokeOnSelected (
     Widget w,
     XtPointer client_data,
     XtPointer call_data)
#endif /* _NO_PROTO */
{
   InvokeAction( (char *)client_data, ArgSelected, w, client_data, call_data );
}

/************************************************************************
 *
 *  ActionInvoke
 *	Invoke an action with no arguments
 *
 ************************************************************************/ 

static void 
#ifdef _NO_PROTO
ActionInvoke (w, client_data, call_data)
     Widget w;
     XtPointer client_data;
     XtPointer call_data;
#else
ActionInvoke (
     Widget w,
     XtPointer client_data,
     XtPointer call_data)
#endif /* _NO_PROTO */
{
   InvokeAction( (char *)client_data, ArgNone, w, client_data, call_data );
}
