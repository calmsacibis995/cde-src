/* $XConsortium: Main.h /main/cde1_maint/4 1995/10/09 14:51:32 pascale $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           Main.h
 *
 *   COMPONENT_NAME: Desktop File Manager
 *
 *   DESCRIPTION:    Structures and defines needed by all of the files
 *
 *   MACROS:    DPRINTF
 *              DPRINTF2
 *              FileManip
 *              FileMoveCopy
 *              FileMoveCopyDesktop
 *              GETMESSAGE
 *              MakeFile
 *              MakeFilesFromBuffers
 *              MakeFilesFromBuffersDT
 *              XtMalloc
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _Main_h
#define _Main_h

/* ANSI C definitions */
#ifdef __STDC__
#define Const const
#else
#define Const /**/
#endif

#ifndef _NO_PROTO
#if  !defined(__STDC__) && !defined(__cplusplus) && !defined(c_plusplus)
#define _NO_PROTO
#endif
#endif


/*  Replacing XtMalloc by XtCalloc */

#define XtMalloc(x)     XtCalloc(1,x)
/*  Unknown type of file.  */

#define DtUNKNOWN       255


/*  The defines for icon states  */

#define NORMAL          0
#define SELECTED        1


/*  Generally used defines  */

#define MAX_PATH        1024
#define BLOCK_SIZE      1024
#define BUF_SIZE        256


/*  Current Directory Area defines  */

#define CURRENT_DIRECTORY_PAD_X 10
#define CURRENT_DIRECTORY_PAD_Y 4

#define FAST_RENAME "fast_rename"

/*  Session restore defines  */

#define NORMAL_RESTORE  0
#define HOME_RESTORE    1
#define TOOL_RESTORE    2

#define TOOL_SETTINGS_FILENAME  "dtfile.appl"
#define HOME_SETTINGS_FILENAME  "dtfile.fmgr"

#define HOME_DIR_RESTORE     1
#define CURRENT_DIR_RESTORE  2

#define TOOL     0
#define NON_TOOL 1

#define PIXELS_PER_ROW_LARGE    82
#define PIXELS_PER_COLUMN_LARGE 128
#define PIXELS_PER_ROW_SMALL    35
#define PIXELS_PER_COLUMN_SMALL 128
#define FP_HEIGHT 115


/* Physical file type defines */

#define DtDATA          1
#define DtDIRECTORY     2
#define DtEXECUTABLE    3


/* Logical file type defines */

#define LT_BROKEN_LINK		"BROKEN_LINK"
#define LT_RECURSIVE_LINK	"RECURSIVE_LINK"
#define LT_DIRECTORY		"FOLDER"
#define LT_DATA			"DATA"
#define LT_AGROUP		"AGROUP"
#define LT_AGROUP_SUBDIR	"APPGROUP"
#define LT_TRASH		"TRASH_FOLDER"
#define LT_FOLDER_LOCK		"FOLDER_LOCK"
#define LT_DOT_FOLDER		"DOT_FOLDER"
#define LT_CURRENT_FOLDER	"CURRENT_FOLDER"
#define LT_NON_WRITABLE_FOLDER	"NON_WRITABLE_FOLDER"


/* _DtMessage Dialog build defines */

#define ERROR_DIALOG    1
#define WARNING_DIALOG  2
#define QUESTION_DIALOG 3


/* Trash movement defines -- see FileMgr.c and Desktop.c */

#define TO_TRASH     1
#define WITHIN_TRASH 2
#define FROM_TRASH   3


/* Trash Action */
#define TRASH_ACTION    "Trash"


/*  Internal bitmap file names  */
#define MULTIPLE_FILE_DRAG      "drag.bm"
#define SMALL_DIRECTORY_ICON    CDE_INSTALLATION_TOP "/appconfig/icons/C/dirblue.t.pm"

#define TOOL_ICON_NAME     CDE_INSTALLATION_TOP "/appconfig/icons/C/Fpapps.l.pm"
#define TRASH_ICON_NAME    CDE_INSTALLATION_TOP "/appconfig/icons/C/Fptrsh.l.pm"
#define HOME_ICON_NAME     CDE_INSTALLATION_TOP "/appconfig/icons/C/Fphome.l.pm"


/* File manipulation types */
#define MOVE_FILE       0
#define COPY_FILE       1
#define LINK_FILE       2
#define REMOVE_FILE     3
#define TRASH_FILE      4
#define RENAME_FILE     5
#define MERGE_DIR       6
#define MAKE_BUFFER     7

/* object placement values (for objectPlacement on the desktop) */
#define OBJ_PLACE_LEFT_PRIMARY         (1L << 0)
#define OBJ_PLACE_RIGHT_PRIMARY        (1L << 1)
#define OBJ_PLACE_TOP_PRIMARY          (1L << 2)
#define OBJ_PLACE_BOTTOM_PRIMARY       (1L << 3)
#define OBJ_PLACE_LEFT_SECONDARY       (1L << 4)
#define OBJ_PLACE_RIGHT_SECONDARY      (1L << 5)
#define OBJ_PLACE_TOP_SECONDARY        (1L << 6)
#define OBJ_PLACE_BOTTOM_SECONDARY     (1L << 7)
#define OBJ_PLACE_EDGE                 (1L << 8)
#define OBJ_PLACE_TIGHT                (1L << 9)
#define OBJ_PLACE_RESERVE              (1L << 10)

/* defines for the icon type on the Desktop */
#define DEFAULT 0
#define LARGE   1
#define SMALL   2

/* defines for whether to open New or use current */
#define CURRENT 0
#define NEW 1

/* defines for tree branch operations */
#define BRANCH_UPDATE   0
#define BRANCH_EXPAND   1
#define BRANCH_COLLAPSE 2
         
/* defines for default Buffer file names */
#define DEFAULT_BUFFER_FILENAME        "Untitled"
#define MAX_POSTFIX_LENGTH              11


/* Creation mask */
#define DtFILE_DATA_CREATION_MASK    S_IRUSR | S_IWUSR |  S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
#define DtFILE_DIR_CREATION_MASK     S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH
#define DtFILE_OTHER_CREATION_MASK   S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH


#include <Tt/tttk.h>

/*  The main directory structure and directory set list  */
typedef struct
{
   XtPointer dialog_data;
   char  * host_name;
   char  * directory_name;
   Tt_message msg;
   Tt_pattern * pats;
} View;

typedef struct
{
  int x, y;
} WindowPosition;

/* Used by File.c */
typedef int (*StrcollProc) (const char *,const char *);

/* Global strings */
extern char DESKTOP_DIR[];
extern char DTFILE_CLASS_NAME[];
extern char DTFILE_HELP_NAME[];


/* Trash globals */
extern Widget trashShell;
extern DialogData * primaryTrashHelpDialog;
extern DialogData ** secondaryTrashHelpDialogList;
extern int secondaryTrashHelpDialogCount;


/*  debugging output  */
#ifdef DEBUG
extern int debug;
#define DPRINTF(p)  if (debug) printf p
#define DPRINTF2(p) if (debug >= 2) printf p
#else
#define DPRINTF(p)  /* noop */
#define DPRINTF2(p) /* noop */
#endif

/*  Externally referenced globals  */

extern Widget toplevel;
extern char * application_name;
extern long root_user;
extern char home_host_name[];
extern char users_home_dir[];
extern Pixel black_pixel;
extern Pixel white_pixel;
extern XtPointer initiating_view;
extern Boolean special_view;
extern Boolean special_reuse;
extern Boolean TrashView;
extern XmFontList user_font;
extern Boolean showFilesystem;
extern Boolean showDropZone;
extern Boolean showEmptySet;
extern Boolean showEmptyMsg;
extern Boolean restrictMode;
extern int openDirType;
extern char *root_title;
extern char *fileMgrTitle;
extern char *fileMgrHelpVol;
extern int desktopIconType;
extern Boolean freezeOnConfig;
#if defined(__hpux) || defined(sun)
extern Boolean follow_links;
#endif
extern int maxDirectoryProcesses;
extern int maxRereadProcesses;
extern int maxRereadProcsPerTick;
extern int rereadTime;
extern int checkBrokenLink;
extern int trashWait;
extern int desktopPlacement;
extern int treeType;
extern int treeFiles;
extern int viewType;
extern int orderType;
extern int directionType;
extern int randomType;
extern int special_treeType;
extern int special_treeFiles;
extern int special_viewType;
extern int special_orderType;
extern int special_directionType;
extern int special_randomType;
extern int instanceWidth;
extern int instanceHeight;
extern char *special_restricted;
extern char *special_title;
extern char *special_helpVol;
extern char *fileMgrIcon;
extern char *appMgrIcon;
extern char *trashIcon;
extern int retryLoadDesktopInfo;
extern int smallIconWidth;
extern int smallIconHeight;
extern int largeIconWidth;
extern int largeIconHeight;
extern Boolean emptyTrashOnExit;
extern String openInPlace;
extern String openNewView;
extern char * desktop_dir;
extern char * trash_dir;
extern char * remote_sys_dir;
extern XmString okXmString;
extern XmString cancelXmString;
extern XmString helpXmString;
extern XmString applyXmString;
extern XmString closeXmString;
extern unsigned int currentMenuStates;
extern Boolean b1Drag;
extern Boolean dragActive;
extern int restoreType;
extern int file_mgr_dialog;
extern int change_dir_dialog;
extern int preferences_dialog;
extern int filter_dialog;
extern int find_dialog;
extern int mod_attr_dialog;
extern int help_dialog;
extern Boolean TurnOnTextName;
extern int numColumns;
extern int numRows;


#ifdef _TT_MULTI_SCREEN
#include "MultiView.h"

/* gMainMenu used to be called mainMenu, and was static to Menu.c */
extern int		gTTScreenNum;  /* ToolTalk Requested screen */
extern int 		gNScreens;  /* Number of screens */
extern MenuDesc** 	gMainMenu;   
extern FileMgrPopup* 	gFileMgrPopup;
extern char**		gDisplayStrings; /* Display strings */

#else

extern FileMgrPopup fileMgrPopup;

#endif  /* _TT_MULTI_SCREEN */

extern unsigned char keybdFocusPolicy;
extern FileMgrData *trashFileMgrData;
extern DialogData *trashDialogData;
#ifdef SHAPE
extern Boolean shapeExtension;
#endif
extern int selectedMenuMinSize;

#define MaxRootFolderActions 5
#ifdef _TT_MULTI_SCREEN

/* We want a global array of the widgets  */
typedef struct 
{
   Widget actionsId;
   Widget actionsPaneId;
   Widget directoryBarBtn;
   Widget create_dataBtn;
   Widget renameBtn;
   Widget duplicateBtn;
   Widget findBtn;
   Widget moveBtn;
   Widget linkBtn;
   Widget create_directoryBtn;
   Widget create_directoryBtn_child;
   Widget change_directoryBtn;
   Widget create_dataBtn_child;
   Widget preferencesBtn;
   Widget filterBtn;
   Widget defaultEnvBtn;
   Widget homeBarBtn;
   Widget upBarBtn;
   Widget newViewBtn;
   Widget cleanUpBtn;
   Widget terminalBtn;
   Widget showHiddenMenu;
   Widget usingHelp;
   Widget fileManagerHelp;
   Widget applicationManagerHelp;
   Widget rootFolderActionBtns[ MaxRootFolderActions ];
   Widget rootFolderActionSeparator;
} MenuWidgets;

extern MenuWidgets* gMenuWidgets;

#else /* _TT_MULTI_SCREEN */

extern Widget * create_dataBtn;
extern Widget * renameBtn;
extern Widget * moveBtn;
extern Widget * duplicateBtn;
extern Widget * linkBtn;
extern Widget * findBtn;
extern Widget * create_directoryBtn;
extern Widget * change_directoryBtn;
extern Widget * preferencesBtn;
extern Widget * filterBtn;
extern Widget * defaultEnvBtn;
extern Widget * homeBarBtn;
extern Widget * upBarBtn;
extern Widget * newViewBtn;
extern Widget * cleanUpBtn;
extern Widget * separator1;
extern Widget * separator2;
extern Widget * terminalBtn;
extern Widget * showHiddenMenu;
extern Widget * usingHelp;
extern Widget * fileManagerHelp;
extern Widget * applicationManagerHelp;

#endif /* _TT_MULTI_SCREEN */

extern Widget * usingHelpTrash;

extern Widget widget_dragged;

/* Drag state variables */
extern Boolean B1DragPossible;
extern Boolean B2DragPossible;
extern Boolean ProcessBtnUp;
extern Boolean ProcessBtnUpCD;
extern int initialDragX;
extern int initialDragY;
extern int dragThreshold;
extern int xErrorDetected;

/* BMenu button binding */
extern int bMenuButton;

extern char translations_sp_esc[];
extern char translations_space[];
extern char translations_escape[];
extern char translations_da[];

extern View ** view_set;
extern int view_count;
extern int view_set_size;

extern StrcollProc FMStrcoll;
extern XtIntervalId checkBrokenLinkTimerId;

/********    Public Function Declarations    ********/

#ifdef _NO_PROTO
#include <Xm/GadgetP.h>
#include <Xm/ManagerP.h>

#include <Dt/Dnd.h>

#include "sharedFuncs.h"
#include "FileMgr.h"
#include "Common.h"


/* old-style prototypes from Main.c */

extern void main() ;
extern void SaveSettingsCB() ;
extern Boolean _FileMoveCopy();
extern void SaveSessionCallback() ;
extern DialogData *ViewDirectoryProc() ;
extern DialogData *GetNewView() ;
extern void CloseView() ;
extern void DirectoryContentsModified() ;
extern void DirectoryChanged() ;
extern PixmapData * CheckForOpenDirectory();
extern FileMgrData * ReturnDesktopPtr ();
extern void CleanUp () ;
extern void DragEndedHandler() ;
extern char *GetMessage();
extern FileMgrData * CheckOpenDir() ;
extern char * GetRestrictedDirectory() ;
extern char * BuildTrueDTFilePath() ;
extern void ForceMyIconClosed () ;
extern void ForceMyIconOpen ();
extern Tt_callback_action HandleTtRequest() ;
extern int getScreenFromTTMsg();
extern char *getScreenDisplayString();

/* old-style prototypes from ChangeDir.c */

extern void ChangeToNewDir () ;
extern void TextChange( ) ;

/* old-style prototypes from ChangeDirP.c */

extern void ShowNewDirectory();
extern void ShowChangeDirDialog() ;
extern void CurrentDirSelected() ;
extern void CurrentDirIconCallback() ;
extern void CurrentDirExposed() ;
extern Boolean GetStatusMsg() ;
extern void DrawCurrentDirectory() ;
extern void CurrentDirectoryIconMotion() ;
extern void ShowFastChangeDir() ;
extern void IconicPathExposeCallback() ;
extern void IconicPathResizeCallback() ;

/* old-style prototypes from Command.c */

extern void UpdateActionMenuPane() ;
extern void RunCommand() ;
extern Boolean InvalidTrashDragDrop() ;
extern void ProcessNewView() ;
extern void ProcessMoveCopyLink() ;
extern void ProcessAction() ;

/* old-style prototypes from Common.c */
extern void VFTextChangeSpace() ;
extern void CancelOut() ;
extern void AddString() ;

/* old-style prototypes from MakeDir.c */

extern int RunFileCommand();

/* old-style prototypes from Directory.c */

extern void InitializeDirectoryRead() ;
extern int ReadDir() ;
extern void FileWindowMapUnmap();
extern void RereadDirectory() ;
extern void UpdateDirectory() ;
extern void DirectoryBeginModify() ;
extern void DirectoryFileModified() ;
extern void DirectoryEndModify() ;
extern void UpdateDirectorySet() ;
extern void UpdateCachedDirectories() ;
extern char * GetLongName() ;
extern Boolean DirectoryBusy() ;
extern char * GetDirectoryLogicalType() ;
extern char * RootTypeOfView() ;
extern char * RootTypeOfDesktopObject() ;
extern int ReadFileData2() ;
extern FileData * FileData2toFileData() ;
extern void FreeFileData() ;
extern FileData * LookupFileData() ;
extern int GetDirectoryPositionInfo() ;
extern int SetDirectoryPositionInfo() ;
extern void TimerEventBrokenLinks();
extern char * GetTTPath() ;

/* old-style prototype from File.c */

extern void SetHotRects() ;
extern unsigned char TypeToDropOperations () ;
extern char * TypeToAction() ;
extern void OrderFiles() ;
extern void FilterFiles() ;
extern void UnmanageFileIcons() ;
extern void UpdateFileIcons() ;
extern void AddFileIcons() ;
extern void EraseTreeLines() ;
extern void RedrawTreeLines() ;
extern void LayoutFileIcons() ;
extern Pixmap GetTreebtnPixmap();
extern void IconCallback() ;
extern void FileIconMotion() ;
extern Boolean FileIsSelected() ;
extern void SelectFile() ;
extern void DeselectFile() ;
extern void DeselectAllFiles() ;
extern void SelectAllFiles() ;
extern void SetToSelectColors() ;
extern void SetToNormalColors() ;
extern Boolean FileMoveCopy() ;
extern void GetTarget() ;
extern void RepositionIcons() ;
extern void RegisterDesktopHotspots() ;
extern void DestroyIconName() ;
extern void CreateNameChangeDialog() ;
extern void SavePositionalData() ;
extern void RestorePositionalData() ;
extern void UnpostTextField() ;
extern void UnpostTextPath() ;
extern void RedrawOneGadget() ;
extern void RepaintDesktop() ;
extern void DrawingAreaRedisplay() ;
extern ObjectPtr GetBottomOfStack() ;
extern ObjectPtr GetTopOfStack() ;
extern void RepositionUpInStack() ;
extern void RepairStackingPointers() ;
extern void OrderChildrenList() ;
extern void PositionFileView() ;
extern void DrawHighlight ();
extern void DrawUnhighlight ();
extern void DrawShadowTh ();
extern void FmPopup ();
extern Boolean VerticalScrollbarIsVisible() ;
extern Boolean HorizontalScrollbarIsVisible();
extern StrcollProc GetStrcollProc ();

/* old-style prototypes from FileDialog.h */

extern void ShowRenameFileDialog() ;
extern void ShowCopyFileDialog() ;
extern void ShowMoveFileDialog() ;
extern void ShowLinkFileDialog() ;
extern void ShowMakeFileDialog() ;
extern void RenameDone() ;
extern int RecheckFlag() ;
extern void ResetFlag() ;

/* old-style prototypes from FileManip.h */

extern void FileOperationError() ;
extern Boolean FileManip() ;
extern char * DName() ;

/* old-style prototypes from FileMgr.h */

extern PixmapData * GetPixmapData() ;
extern void SetSpecialMsg() ;
extern void UpdateHeaders() ;
extern void FileMgrRedisplayFiles() ;
extern void ShowNewDirectory() ;
extern void FileMgrReread() ;
extern void FileMgrBuildDirectories() ;
extern void FileMgrProcessDrop() ;
extern void FileMgrPropagateSettings() ;
extern void Close() ;
extern void FreePositionInfo() ;
extern Boolean PositioningEnabledInView() ;
extern Boolean PositionFlagSet() ;
extern void InheritPositionInfo() ;
extern void SavePositionInfo() ;
extern void LoadPositionInfo() ;
extern void SetPWD() ;
extern void DropOnFileWindow () ;
extern void DropOnObject () ;
extern void CheckMoveType() ;
extern void UpdateBranchList() ;
extern Boolean QueryBranchList();
extern void UpdateBranchState();
extern void DirTreeExpand();

/* old-style prototype from Filter.c */
extern void UpdataFilterAfterDBReread() ;

/* old-style prototype from FilterP.c */

extern void ShowFilterDialog() ;
extern void ShowHiddenFiles () ;
extern void NewFileTypeSelected () ;
extern void SelectAllFileTypes () ;
extern void UnselectAllFileTypes () ;

/* old-style prototype from FindP.c */

extern void ShowFindDialog() ;

/* old-style prototype from IconWindow.c */

extern void FileWindowExposeCallback() ;
extern void FileWindowResizeCallback() ;
extern void FileWindowMapCallback() ;
extern void FileWindowInputCallback() ;
extern void WidgetRectToRegion () ;

/* old-style prototype from Menu.c */

extern Widget CreateMenu() ;
extern void ActivateSingleSelect() ;
extern void ActivateMultipleSelect() ;
extern void ActivateNoSelect() ;
extern void UnselectAll() ;
extern void SelectAll() ;

extern void TrashFiles() ;
extern void ChangeToHome() ;
extern void ChangeToParent() ;

/* old-style prototype from ModAttr.c */

extern Boolean LoadFileAttributes();
extern void ModAttrChange() ;
extern void ModAttrClose() ;
extern void ModAttrFreeValues() ;

/* old-style prototype from ModAttrP.c */

extern void ShowModAttrDialog() ;

/* old-style prototypes from Prefs.c */

extern void ShowTypeToString() ;
extern void StringToShowType() ;
extern void TreeFilesToString() ;
extern void StringToTreeFiles() ;
extern void ViewToString() ;
extern void StringToView() ;
extern void OrderToString() ;
extern void StringToOrder() ;
extern void DirectionToString() ;
extern void RandomToString() ;
extern void StringToDirection() ;
extern void StringToRandom() ;
extern void UpdatePreferencesDialog () ;

/* old-style prototypes from PrefsP.c */

extern void ShowPreferencesDialog() ;

/* old-style prototypes from Trash.c */

extern Boolean TrashIsInitialized() ;
extern Boolean InitializeTrash() ;
extern void TrashCreateDialog() ;
extern void TrashDisplayHandler() ;
extern void TrashEmptyHandler() ;
extern void TrashEmpty() ;
extern void TrashRemoveHandler() ;
extern void TrashRemoveNoConfirmHandler() ;
extern void TrashRestoreHandler() ;
extern Widget CreateTrashMenu();
extern void SensitizeTrashBtns();
extern void DropOnTrashCan() ;
extern void MoveOutOfTrashCan() ;
extern void Restore() ;
extern void ConfirmRemove() ;
extern void Remove() ;
extern Boolean FileFromTrash() ;
extern int CheckDeletePermission();

/* old-style prototypes from Desktop.c */

extern void InitializeDesktopWindows() ;
extern void CheckDesktop() ;
extern void DesktopObjectRemoved() ;
extern void DesktopObjectChanged() ;
extern FileViewData * DropOnDesktopHotspot() ;
extern Boolean FileMoveCopyDesktop() ;
extern void SaveDesktopInfo() ;
extern void LoadDesktopInfo() ;
extern void RemoveDT() ;
#ifdef SUN_PERF
extern void RemoveMovedObjectFromDT ();
#endif /* SUN_PERF */
extern void DeselectAllDTFiles() ;
extern void DeselectDTFile() ;
extern void DTActionCallback() ;
extern void RunDTCommand() ;
extern char * IsAFileOnDesktop() ;
extern char * IsAFileOnDesktop2() ;
extern void ProcessDTSelection() ;
extern void UnpostDTTextField() ;
extern Boolean DTFileIsSelected() ;
extern void InitializeDesktopGrid() ;
extern void RegisterInGrid() ;
extern void PutOnDTCB() ;
extern void SetupDesktopWindow() ;
extern void CleanUpWSName() ;
extern void CheckDesktopMarquee() ;
#ifdef SHAPE
extern void GenerateShape( ) ;
#endif
extern void RegisterIconDropsDT();
extern void WorkSpaceRemoved();

/* old-style prototype for FileOp.c */
extern int PipeRead() ;
extern int PipeWriteString() ;
extern char * PipeReadString() ;
extern void FileOpError() ;
extern void ChangeIconName() ;
extern void ChangeIconNameDT() ;
extern void MakeFile() ;
extern Boolean MakeFilesFromBuffers();
extern Boolean MakeFilesFromBuffersDT();

/* old-style prototypes from fsDialog.c */
extern Boolean fsDialogAvailable();

/* old-style prototypes from Utils.c */
extern char * ResolveLocalPathName();
extern char * ResolveTranslationString();

/* old-style prototypes from OverWrite.c */
extern void create_replace_rename_dialog();
extern void create_replace_merge_dialog();
extern void create_multicollide_dialog();
extern void replace_rename_cancel_callback();
extern void replace_rename_ok_callback();
extern void buffer_replace_rename_ok_callback();
extern void multicollide_ok_callback();
extern void replace_merge_cancel_callback();
extern void replace_merge_ok_callback();
extern void multicollide_ok_callback();
extern void multicollide_cancel_callback();
extern void buffer_multicollide_ok_callback();

/* old-style prototypes from HelpCB.c */
XtPointer LocateRecordStructure() ;

#else /* _NO_PROTO */

#include <Xm/GadgetP.h>
#include <Xm/ManagerP.h>

#include <Dt/DtP.h>
#include <Dt/Action.h>
#include <Dt/Dnd.h>

#include "sharedFuncs.h"
#include "FileMgr.h"
#include "ChangeDir.h"
#include "ModAttr.h"

/* prototypes from Main.c */

extern void SaveSettingsCB(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern void SaveSessionCallback(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern DialogData *ViewDirectoryProc(
                        char *root_dir,
                        int restricted,
                        Tt_message msg) ;
extern DialogData *GetNewView(
                        char *host_name,
                        char *directory_name,
                        char *type,
                        WindowPosition *position,
                        Tt_message msg) ;
extern void CloseView(
                        DialogData *dialog_data) ;
extern void DirectoryContentsModified(
                        char *host_name,
                        char *directory_name,
                        FileMgrRec *file_mgr_rec) ;
extern void DirectoryChanged(
                        XtPointer file_mgr_data,
                        char *old_host_name,
                        char *new_host_name,
                        char *old_directory_name,
                        char *new_directory_name) ;
extern PixmapData * CheckForOpenDirectory(
                        FileViewData *order_list,
                        DirectorySet *directory_set,
                        FileMgrData *file_mgr_data,
                        char * logical_type) ;
extern FileMgrData * ReturnDesktopPtr (
                        Widget w);
extern void CleanUp (
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern void DragEndedHandler(
                        Display *drop_display,
                        Window drop_window,
                        Position drop_x,
                        Position drop_y,
                        Window root_window,
                        Position root_x,
                        Position root_y,
                        unsigned int modifiers,
                        char *types,
                        char *objects,
                        XtPointer client_data) ;
extern char * GetMessage(
                        int set,
                        int n,
                        char *s) ;
extern FileMgrData * CheckOpenDir (
                        char * directory,
                        char * host) ;
extern char * GetRestrictedDirectory ( Widget widget) ;
extern char * BuildTrueDTFilePath(
                        DesktopRec *desktopWindow,
                        FileViewData *file_view_data) ;
extern void ForceMyIconClosed (
                        char * host_name,
                        char * directory_name) ;
extern void ForceMyIconOpen (
                        char * host_name,
                        char * directory_name) ;
extern Tt_callback_action HandleTtRequest(
			Tt_message msg,
			Tt_pattern pat) ;

extern int getScreenFromTTMsg(Tt_message msg, int def);
extern char *getScreenDisplayString(Display* display, int screen);

/* prototype from ChangeDir.c */

extern void ChangeToNewDir (
                        Widget text_field,
                        XtPointer client_data,
                        XmAnyCallbackStruct * callback_data );
extern void TextChange(
                        Widget text,
                        XtPointer client_data,
                        XmTextVerifyCallbackStruct * callback_data );

/* prototypes from ChangeDirP.c */

extern void ShowNewDirectory(
                        FileMgrData *,
                        char *,
                        char * );
extern void ShowChangeDirDialog(
                        Widget w,
                        XtPointer client_data,
                        XtPointer callback) ;
extern void CurrentDirSelected(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern void CurrentDirIconCallback(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern void CurrentDirExposed(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern Boolean GetStatusMsg(
                        FileMgrData *file_mgr_data,
                        char *buf) ;
extern void DrawCurrentDirectory(
                        Widget w,
                        FileMgrRec *file_mgr_rec,
                        FileMgrData *file_mgr_data) ;
extern void CurrentDirectoryIconMotion(
                        Widget w,
                        XtPointer client_data,
                        XEvent *event) ;
extern void ShowFastChangeDir(
                        FileMgrRec *file_mgr_rec,
                        FileMgrData *file_mgr_data ) ;
extern void IconicPathExposeCallback(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern void IconicPathResizeCallback(
                        Widget w,
                        XtPointer client_data,
                        XEvent *event) ;

/* prototype from Command.c */

extern void UpdateActionMenuPane(
                        XtPointer *client_data,
                        FileMgrRec *file_mgr_rec,
                        char * file_type,
                        int type,
                        int number,
                        Widget widget,
                        unsigned char physical_type,
                        char * file_name) ;
extern void RunCommand( char *command,
                        FileMgrData *file_mgr_data,
                        FileViewData *file_view_data,
                        WindowPosition *position,
                        DtDndDropCallbackStruct *drop_parameters,
                        Widget drop_window) ;
extern Boolean InvalidTrashDragDrop ( int drop_op,
                                      int trash_context,
                                      Widget w) ;
extern void ProcessNewView ( char *command,
                             FileMgrData *file_mgr_data,
                             FileViewData *file_view_data,
                             WindowPosition *position) ;
extern void ProcessMoveCopyLink ( char *command,
                                  FileMgrData *file_mgr_data,
                                  FileViewData *file_view_data,
                                  DtDndDropCallbackStruct *drop_parameters,
                                  Widget drop_window) ;
extern void ProcessAction ( char *action,
                            FileViewData *file_view_data,
                            DtDndDropCallbackStruct *drop_parameters,
                            char *cur_host,
                            char *cur_dir,
                            char *restricted_dir,
                            Widget w) ;

/* prototype from Common.c */

extern void VFTextChangeSpace(
                        Widget text,
                        XEvent *event,
                        XtPointer params,
                        XtPointer num_params ) ;
extern void CancelOut(
                        Widget widget,
                        XEvent *event,
                        XtPointer params,
                        XtPointer num_params ) ;
extern void AddString(
                        String *buf,
                        int *size,
                        String string,
                        String header ) ;

/* prototype from MakeDir.c */

extern int RunFileCommand(
                        register char *command_path,
                        register char *argument1,
                        register char *argument2,
                        register char *argument3) ;

/* prototypes from Directory.h */

extern void InitializeDirectoryRead(
                        Widget widget) ;
extern void FileWindowMapUnmap(
                        FileMgrData *file_mgr_data) ;
extern int ReadDir(
                        Widget w,
                        FileMgrData *file_mgr_data,
                        char *host_name,
                        char *directory_name,
                        FileViewData *dp,
                        int level,
                        int read_level,
                        char **branch_list) ;
extern void RereadDirectory(
                        Widget w,
                        char *host_name,
                        char *directory_name) ;
extern void UpdateDirectory(
                        Widget w,
                        char *host_name,
                        char *directory_name) ;
extern void DirectoryBeginModify(
                        char *host_name,
                        char *directory_name) ;
extern void DirectoryModifyTime(
                        char *host_name,
                        char *directory_name,
                        long modify_time) ;
extern void DirectoryFileModified(
                        char *host_name,
                        char *directory_name,
                        char *file_name) ;
extern void DirectoryEndModify(
                        char *host_name,
                        char *directory_name) ;
extern void UpdateDirectorySet( void ) ;
extern void UpdateCachedDirectories(
                        View **view_set,
                        int view_count) ;
extern char * GetLongName(
                        FileData *file_data) ;
extern Boolean DirectoryBusy(
                        char *path) ;
extern char * GetDirectoryLogicalType(
                        FileMgrData *file_mgr_data,
                        char *path) ;
extern char * RootTypeOfView(
                        FileMgrData *file_mgr_data) ;
extern char * RootTypeOfDesktopObject(
                        DesktopRec *object) ;
extern FileData * FileData2toFileData(
                        FileData2 *file_data2,
                        int *l);
extern int ReadFileData2(
			FileData2 *fdata,
                        char *full_directory_name,
                        char *file_name,
                        Boolean IsToolBox) ;
extern void FreeFileData(
                        FileData *file_data,
                        Boolean free_all) ;
extern FileData * LookupFileData(
                        char *host_name,
                        char *directory_name,
                        char *file_name) ;
extern int GetDirectoryPositionInfo(
                        char *host_name,
                        char *directory_name,
                        PositionInfo **position_info) ;
extern int SetDirectoryPositionInfo(
                        char *host_name,
                        char *directory_name,
                        int position_count,
                        PositionInfo *position_info) ;
extern void TimerEventBrokenLinks(
                        XtPointer client_data,
                        XtIntervalId *id);
extern char * GetTTPath(
                        char *name) ;

/* prototypes for File.c */

extern void SetHotRects (
                        FileViewData * file_view_data,
                        const char * view_type,
                        XtCallbackProc callback,
                        XtPointer callback_data) ;
extern unsigned char TypeToDropOperations (
                        const char * file_type) ;
extern char * TypeToAction (
                        unsigned char operation,
                        const char * logical_type) ;
extern void OrderFiles(
                        FileMgrData *file_mgr_data,
                        DirectorySet *directory_set) ;
extern void FilterFiles(
                        FileMgrData *file_mgr_data,
                        DirectorySet *directory_set) ;
extern void UnmanageFileIcons(
                        FileMgrRec *file_mgr_rec,
                        FileMgrData *file_mgr_data,
                        FileViewData *file_view_data);
extern void UpdateFileIcons(
                        FileMgrRec *file_mgr_rec,
                        FileMgrData *file_mgr_data,
                        Boolean new_directory) ;
extern void AddFileIcons(
                        FileMgrRec *file_mgr_rec,
                        FileMgrData *file_mgr_data,
                        DirectorySet * add_dir_set) ;
extern void EraseTreeLines(
                        FileMgrRec *file_mgr_rec,
                        FileMgrData *file_mgr_data,
                        FileViewData *file_view_data) ;
extern void RedrawTreeLines(
                        Widget w,
                        int ex, int ey, int ewidth, int eheight, int ecount,
                        FileMgrRec *file_mgr_rec,
                        FileMgrData *file_mgr_data) ;
extern void LayoutFileIcons(
                        FileMgrRec *file_mgr_rec,
                        FileMgrData *file_mgr_data,
                        Boolean update_scrolling_position,
                        Boolean turn_off_hourglass) ;
extern Pixmap GetTreebtnPixmap(
                        FileMgrData *file_mgr_data,
                        FileViewData *file_view_data);
extern void IconCallback(
                        Widget w,
                        XtPointer clientData,
                        XtPointer callData) ;
extern void FileIconMotion(
                        Widget w,
                        XtPointer clientData,
                        XEvent *event) ;
extern Boolean FileIsSelected(
                        FileMgrData *file_mgr_data,
                        FileViewData *file_view_data) ;
extern void SelectFile(
                        FileMgrData *file_mgr_data,
                        FileViewData *file_view_data) ;
extern void DeselectFile(
                        FileMgrData *file_mgr_data,
                        FileViewData *file_view_data,
                        Boolean valid ) ;
extern void DeselectAllFiles(
                        FileMgrData *file_mgr_data) ;
extern void SelectAllFiles(
                        FileMgrData *file_mgr_data) ;
extern void SetToSelectColors(
                        Widget widget,
                        Widget file_window,
                        int type) ;
extern void SetToNormalColors(
                        Widget widget,
                        Widget bg_fg_colors,
                        Widget top_shadow_colors,
                        int type) ;
extern void GetTarget(
                        char *from_host,
                        char *from_dir,
                        char *new_name,
                        char *to_host,
                        char *to_dir,
                        char *to_file);
extern Boolean FileMoveCopy(
                        FileMgrData *file_mgr_data,
                        char *to_file,
                        char *directory,
                        char *host,
                        char **host_set,
                        char **file_set,
                        int file_count,
                        unsigned int modifiers,
                        void (*finish_callback)(),
                        XtPointer callback_data);
extern void RepositionIcons(
                        FileMgrData *file_mgr_data,
                        char **file_list,
                        int file_count,
                        Position drop_x,
                        Position drop_y,
                        Boolean late_binding_needed) ;
extern void RegisterDesktopHotspots(
                        FileMgrData *file_mgr_data,
                        FileMgrRec *file_mgr_rec) ;
extern void DestroyIconName(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern void CreateNameChangeDialog(
                        Widget w,
                        FileViewData *file_view_data,
                        XtPointer client_data,
                        int type) ;
extern void SavePositionalData(
                        int fd,
                        FileMgrData *file_mgr_data,
                        char **name_list,
                        char *name) ;
extern void RestorePositionalData(
                        XrmDatabase db,
                        char **name_list,
                        FileMgrData *file_mgr_data,
                        char *name) ;
extern void UnpostTextField(
                        FileMgrData *file_mgr_data) ;
extern void UnpostTextPath(
                        FileMgrData *file_mgr_data) ;
extern void RedrawOneGadget(
                        Widget child,
                        XEvent *event,
                        Region region) ;
extern void RepaintDesktop(
                        FileMgrData *file_mgr_data,
                        XEvent *event,
                        Region region) ;
extern void DrawingAreaRedisplay(
                        Widget wid,
                        XEvent *event,
                        Region region) ;
extern ObjectPtr GetBottomOfStack(
                        FileMgrData *file_mgr_data) ;
extern ObjectPtr GetTopOfStack(
                        FileMgrData *file_mgr_data) ;
extern void RepositionUpInStack(
                        FileMgrData *file_mgr_data,
                        int oldPos,
                        int newPos) ;
extern void RepairStackingPointers(
                        FileMgrData *file_mgr_data) ;
extern void OrderChildrenList(
                        FileMgrData *file_mgr_data) ;
extern void PositionFileView(
                        FileViewData *file_view_data,
                        FileMgrData *file_mgr_data) ;
extern void DrawHighlight (
                        Widget w,
                        FileViewData *file_view_data,
                        FileMgrData *file_mgr_data,
                        int type );
extern void DrawUnhighlight (
                        Widget w,
                        int type);
extern void DrawShadowTh (
                        Widget w,
                        GC gc,
                        int type);
extern void FmPopup (
                        Widget w,
                        XtPointer client_data,
                        XEvent *event,
                        FileMgrData *file_mgr_data);
extern Boolean VerticalScrollbarIsVisible(
                        Widget vertSB,
                        Widget scrolledWin);
extern Boolean HorizontalScrollbarIsVisible(
                        Widget hortSB,
                        Widget scrolledWin);
StrcollProc GetStrcollProc(void);




/* prototypes from FileDialog.h */

extern void ShowRenameFileDialog(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern void ShowCopyFileDialog(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern void ShowMoveFileDialog(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern void ShowLinkFileDialog(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern void ShowMakeFileDialog(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern void RenameDone(
                        XtPointer client_data,
                        int rc) ;
extern int RecheckFlag(
                        Widget parent,
                        Widget w);
extern void ResetFlag(
                        Widget parent,
                        Widget w);

/* prototypes from FileManip.h */

extern void FileOperationError(
                        Widget w,
                        char *message1,
                        char *message2) ;
extern Boolean FileManip(
                        Widget w,
                        int mode,
                        register char *from,
                        register char *to,
                        Boolean isContainer,
                        void (*errorHandler)(),
                        Boolean checkForBusyDir,
                        int type);
extern char * DName(
                        register char *name) ;
int CheckAccess(
			char *fname,
			int what);

/* prototypes from FileMgr.h */

extern PixmapData * GetPixmapData(
                        FileMgrRec *file_mgr_rec,
                        FileMgrData *file_mgr_data,
                        char *path,
                        Boolean large) ;
extern void SetSpecialMsg(
                        FileMgrRec *file_mgr_rec,
                        FileMgrData *file_mgr_data,
                        char *msg) ;
extern void UpdateHeaders(
                        FileMgrRec *file_mgr_rec,
                        FileMgrData *file_mgr_data,
                        Boolean icons_changed) ;
extern void FileMgrRedisplayFiles(
                        FileMgrRec *file_mgr_rec,
                        FileMgrData *file_mgr_data,
                        Boolean new_directory) ;
extern void ShowNewDirectory(
                        FileMgrData *file_mgr_data,
                        char *host_name,
                        char *directory_name) ;
extern void FileMgrReread(
                        FileMgrRec *file_mgr_rec) ;
extern void FileMgrBuildDirectories(
                        FileMgrData *file_mgr_data,
                        char *host_name,
                        char *directory_name) ;
extern void FileMgrProcessDrop(
                        FileMgrData *file_mgr_data,
                        FileMgrRec *file_mgr_rec,
                        Window w,
                        Position drop_x,
                        Position drop_y,
                        unsigned int modifiers,
                        char *types,
                        char *files) ;
extern void FileMgrPropagateSettings(
                        FileMgrData *src_data,
                        FileMgrData *dst_data) ;
extern void Close(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern void FreePositionInfo(
                        FileMgrData *file_mgr_data) ;
extern Boolean PositioningEnabledInView(
                        FileMgrData *file_mgr_data) ;
extern Boolean PositionFlagSet(
                        FileMgrData *file_mgr_data) ;
extern void InheritPositionInfo(
                        FileMgrData *src_file_mgr_data,
                        FileMgrData *dest_file_mgr_data) ;
extern void SavePositionInfo(
                        FileMgrData *file_mgr_data) ;
extern void LoadPositionInfo(
                        FileMgrData *file_mgr_data) ;
extern void SetPWD(
                        char *viewHost,
                        char *viewDir,
                        char **pwdHostRet,
                        char **pwdDirRet,
                        char *type) ;
extern void DropOnFileWindow (
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data);
extern void DropOnObject (
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data);
extern void CheckMoveType(
                        FileMgrData *file_mgr_data,
                        FileViewData *file_view_data,
                        DirectorySet * directory_data,
                        DesktopRec * desktopWindow,
                        char **file_set,
                        char **host_set,
                        unsigned int modifiers,
                        int file_count,
                        Position drop_x,
                        Position drop_y,
                        int view ) ;
extern void UpdateBranchList(
                        FileMgrData *file_mgr_data,
                        DirectorySet *directory_set) ;
extern Boolean QueryBranchList(
                        FileMgrData *file_mgr_data,
                        char **branch_list,
                        char *directory_name,
                        TreeShow *tsp) ;
extern void UpdateBranchState(
                        FileMgrData *file_mgr_data,
                        FileViewData *ip,
                        int op,
                        Boolean busy) ;
extern void DirTreeExpand(
                        FileMgrData *file_mgr_data,
                        FileViewData *ip,
                        Boolean expand) ;

/* prototype from Filter.c */
extern void UpdataFilterAfterDBReread (
                        DialogData * dialog_data) ;

/* prototype from FilterP.c */

extern void ShowFilterDialog(
                        Widget w,
                        XtPointer client_data,
                        XtPointer callback) ;
extern void ShowHiddenFiles (
                        Widget  w,
                        XtPointer client_data,
                        XtPointer callback) ;
extern void NewFileTypeSelected (
                        Widget  w,
                        XtPointer client_data,
                        XtPointer callback) ;
extern void SelectAllFileTypes (
                        Widget  w,
                        XtPointer client_data,
                        XtPointer callback) ;
extern void UnselectAllFileTypes (
                        Widget  w,
                        XtPointer client_data,
                        XtPointer callback) ;

/* prototype from FindP.c */

extern void ShowFindDialog(
                        Widget w,
                        XtPointer client_data,
                        XtPointer callback) ;

/* prototype from IconWindow.c */

extern void FileWindowExposeCallback(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern void FileWindowResizeCallback(
                        Widget w,
                        XtPointer client_data,
                        XEvent *event) ;
extern void FileWindowMapCallback(
                        Widget w,
                        XtPointer client_data,
                        XEvent *event ) ;
extern void FileWindowInputCallback(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern void WidgetRectToRegion (
                        FileMgrData * file_mgr_data,
                        Widget w,
                        Region region) ;

/* prototype from Menu.c */

extern Widget CreateMenu(
                        Widget main,
                        FileMgrRec *file_mgr_rec) ;
extern void ActivateSingleSelect(
                        FileMgrRec *file_mgr_rec,
                        char * file_type) ;
extern void ActivateMultipleSelect(
                        FileMgrRec *file_mgr_rec) ;
extern void ActivateNoSelect(
                        FileMgrRec *file_mgr_rec) ;
extern void UnselectAll(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern void SelectAll(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;

extern void TrashFiles(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern void ChangeToHome(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data ) ;
extern void ChangeToParent(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data ) ;

/* prototype from ModAttr.c */

extern Boolean LoadFileAttributes(
                        String host,
                        String dir,
                        String file,
                        ModAttrData *modAttr_data) ;
extern void ModAttrChange(
                        XtPointer client_data,
                        DialogData *old_dialog_data,
                        DialogData *new_dialog_data,
                        XtPointer call_data) ;
extern void ModAttrClose(
                        XtPointer client_data,
                        DialogData *old_dialog_data,
                        DialogData *new_dialog_data) ;
extern void ModAttrFreeValues(
                        ModAttrData *modAttr_data );


/* prototype from ModAttrP.c */

extern void ShowModAttrDialog(
                        Widget w,
                        XtPointer client_data,
                        XtPointer callback) ;

/* prototypes from Prefs.c */

extern void ShowTypeToString(
                        int fd,
                        unsigned char *value,
                        char *out_buf) ;
extern void StringToShowType(
                        XrmValuePtr args,
                        Cardinal *num_args,
                        XrmValue *from_val,
                        XrmValue *to_val) ;
extern void TreeFilesToString(
                        int fd,
                        unsigned char *value,
                        char *out_buf) ;
extern void StringToTreeFiles(
                        XrmValuePtr args,
                        Cardinal *num_args,
                        XrmValue *from_val,
                        XrmValue *to_val) ;
extern void ViewToString(
                        int fd,
                        unsigned char *value,
                        char *out_buf) ;
extern void StringToView(
                        XrmValuePtr args,
                        Cardinal *num_args,
                        XrmValue *from_val,
                        XrmValue *to_val) ;
extern void OrderToString(
                        int fd,
                        unsigned char *value,
                        char *out_buf) ;
extern void StringToOrder(
                        XrmValuePtr args,
                        Cardinal *num_args,
                        XrmValue *from_val,
                        XrmValue *to_val) ;
extern void DirectionToString(
                        int fd,
                        unsigned char *value,
                        char *out_buf) ;
extern void RandomToString(
                        int fd,
                        unsigned char *value,
                        char *out_buf) ;
extern void StringToDirection(
                        XrmValuePtr args,
                        Cardinal *num_args,
                        XrmValue *from_val,
                        XrmValue *to_val) ;
extern void StringToRandom(
                        XrmValuePtr args,
                        Cardinal *num_args,
                        XrmValue *from_val,
                        XrmValue *to_val) ;
extern void UpdatePreferencesDialog (
                        FileMgrData * file_mgr_data) ;

/* prototype for PrefsP.c */

extern void ShowPreferencesDialog(
                        Widget w,
                        XtPointer client_data,
                        XtPointer callback) ;

/* prototype for Trash.c */

extern Boolean TrashIsInitialized(
                        void ) ;
extern Boolean InitializeTrash(
                        Boolean enableVerifyPrompt);
extern void TrashCreateDialog(
                        Display *display) ;
extern void TrashDisplayHandler(
                        Tt_message msg,
                        char *workspaces,
                        Boolean iconify_state,
                        Boolean UseIconX, int IconX,
                        Boolean UseIconY, int IconY);
extern void TrashEmptyHandler(
                        Tt_message msg);
extern void TrashRemoveHandler(
                        Tt_message msg);
extern void TrashEmpty( void );
extern void TrashRestoreHandler(
                        Tt_message msg);
extern void TrashRemoveNoConfirmHandler(
                        Tt_message msg);
extern Widget CreateTrashMenu(
                        Widget mainw,
                        FileMgrRec *file_mgr_rec) ;
extern void SensitizeTrashBtns( void ) ;
extern void DropOnTrashCan(
                        int file_count,
                        char **host_set,
                        char **file_set,
                        DtDndDropCallbackStruct *parameters) ;
extern void MoveOutOfTrashCan(
                        FileMgrData *file_mgr_data,
                        FileMgrRec *file_mgr_rec,
                        Window w,
                        int file_count,
                        char **host_set,
                        char **file_set,
                        Position drop_x,
                        Position drop_y);
extern void Restore(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data ) ;
extern void ConfirmRemove(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data ) ;
extern void Remove(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data ) ;
extern Boolean FileFromTrash(
                        char *filename) ;
extern int CheckDeletePermission(
                        char *dir,
                        char *file);

/* prototype for Desktop.c */

extern void InitializeDesktopWindows(
                        int number_cache,
                        Display *display) ;
extern void CheckDesktop( void ) ;
extern void DesktopObjectRemoved(
                        DesktopRec *desktopWindow ) ;
extern void DesktopObjectChanged(
                        DesktopRec *desktopWindow ) ;
extern FileViewData * DropOnDesktopHotspot(
                        Window win,
                        Position drop_x,
                        Position drop_y,
                        DirectorySet **directory_data,
                        DesktopRec *desktopWindow) ;
extern Boolean FileMoveCopyDesktop(
                        FileViewData *file_view_data,
                        char *directory,
                        char **host_set,
                        char **file_set,
                        int file_count,
                        unsigned int modifiers,
                        DesktopRec *desktopWindow,
                        void (*finish_callback)(),
                        XtPointer callback_data);
extern void SaveDesktopInfo(
                        int session) ;
extern void LoadDesktopInfo(
                        char *session) ;
extern void RemoveDT(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
#ifdef SUN_PERF
extern void RemoveMovedObjectFromDT(
                        Widget w,
                        XtPointer client_data,
                        int  file_cnt,
			char **file_list) ;
#endif /* SUN_PERF */

extern void DeselectAllDTFiles(
                        WorkspaceRec *workspaceData) ;
extern void DeselectDTFile(
                        WorkspaceRec *workspaceData,
                        DesktopRec *desktopWindow) ;
extern void DTActionCallback(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern void RunDTCommand(
                        char *command,
                        DesktopRec *desktopWindow,
                        DtDndDropCallbackStruct *parameters) ;
extern char * IsAFileOnDesktop(
                        DtString message,
                        int *number) ;
extern char * IsAFileOnDesktop2(
                        char **file_set,
                        int file_count,
                        int *number,
                        Boolean *IsToolBox) ;
extern void ProcessDTSelection(
                        DesktopRec *desktopRec,
                        XButtonEvent *event) ;
extern void UnpostDTTextField(void) ;
extern Boolean DTFileIsSelected(
                        DesktopRec *desktopRec,
                        FileViewData *fileViewData) ;
extern void InitializeDesktopGrid( void ) ;
extern void RegisterInGrid(
                        int width,
                        int height,
                        int rX,
                        int rY,
                        int workspace,
                        Boolean type) ;
extern void PutOnDTCB(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
#ifdef _TT_MULTI_SCREEN
extern void SetupDesktopWindow(
                        Display *display,
                        FileMgrData *file_mgr_data,
                        FileMgrRec *file_mgr_rec,
                        char *file_name,
                        char *host_name,
                        char *directory_name,
                        int root_x,
                        int root_y,
                        char *type,
                        int EndIndex, 
                        int screen_num);
#else
extern void SetupDesktopWindow(
                        Display *display,
                        FileMgrData *file_mgr_data,
                        FileMgrRec *file_mgr_rec,
                        char *file_name,
                        char *host_name,
                        char *directory_name,
                        int root_x,
                        int root_y,
                        char *type,
                        int EndIndex) ;
#endif
extern void CleanUpWSName(
                        char *workspace_name) ;
#ifdef _TT_MULTI_SCREEN
extern void CheckDesktopMarquee(
                        int x,
                        int y,
                        int width,
                        int height, 
			          int scr) ;
#else
extern void CheckDesktopMarquee(
                        int x,
                        int y,
                        int width,
                        int height) ;
#endif
#ifdef SHAPE
extern void GenerateShape( DesktopRec *desktopWindow );
#endif
extern void RegisterIconDropsDT(DesktopRec *desktopWindow) ;
extern void WorkSpaceRemoved(
                         Widget w,
                         Atom atom_name,
                         int type,
                         XtPointer client_data );

/* prototype for HelpCB.c */
XtPointer LocateRecordStructure(
                        Widget w) ;

/* prototype for FileOp.c */
extern int PipeRead(
                        int fd,
                        void *buf,
                        int len) ;
extern int PipeWriteString(
                        int fd,
                        char *s) ;
extern char * PipeReadString(
                        int fd) ;
extern void FileOpError(
                        Widget w,
                        char *message1,
                        char *message2) ;
extern void ChangeIconName(
                        Widget w,
                        XtPointer client_data,
                        XmTextVerifyCallbackStruct *call_data) ;
extern void ChangeIconNameDT(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern void MakeFile(
                        Widget w,
                        char *host_name,
                        char *directory_name,
                        char *new_name,
                        unsigned char type,
                        void (*finish_callback)(),
                        XtPointer callback_data) ;
extern Boolean MakeFilesFromBuffers(
                             FileMgrData *file_mgr_data,
                             char *directory,
                             char *host,
                             char **file_set,
                             char **host_set,
                             BufferInfo *buffer_set,
                             int num_of_buffers,
                             void (*finish_callback)(),
                             XtPointer callback_data); 
extern Boolean MakeFilesFromBuffersDT(
                             FileViewData *file_view_data,
                             char *directory,
                             char **file_set,
                             char **host_set,
                             BufferInfo *buffer_set,
                             int num_of_buffers,
                             DesktopRec *desktopWindow,
                             void (*finish_callback)(),
                             XtPointer callback_data); 

/* prototype for fsDialog.c */
extern Boolean fsDialogAvailable(const String      path,
                                       dtFSData  * fsDialogData);


/* prototype for Utils.c */
extern char *
ResolveLocalPathName(
     char *hostname,
     char *directory_name,
     char *file_name,
     char *local_hostname,
     Tt_status *tt_status);
extern char * ResolveTranslationString(
     char * originalString,
     char * addressString );

/* prototype for OverWrite.c */
extern void
create_replace_rename_dialog(
     Widget          parent_widget,
     int             mode,
     String          directory,
     String          file,
     int             filop_confirm_fd,
     ActionAreaDefn  actions,
     Boolean         Same);

extern void
create_replace_merge_dialog(
     Widget          parent_widget,
     int             mode,
     String          directory,
     String          file,
     int             filop_confirm_fd,
     ActionAreaDefn  actions);

extern void
create_multicollide_dialog(
     Widget          parent_widget,
     int             mode,
     int             nSelected,
     int             nCollisions,
     String          destination,
     String        * fileList,
     int             filop_confirm_fd,
     ActionAreaDefn  actions,
     Boolean         Same);

extern void
replace_rename_cancel_callback(
     Widget    w,
     XtPointer client_data,
     XtPointer call_data);

extern void
replace_rename_ok_callback(
     Widget    w,
     XtPointer client_data,
     XtPointer call_data);

extern void
buffer_replace_rename_ok_callback(
     Widget    w,
     XtPointer client_data,
     XtPointer call_data);

extern void
replace_merge_cancel_callback(
     Widget    w,
     XtPointer client_data,
     XtPointer call_data);

extern void
replace_merge_ok_callback(
     Widget    w,
     XtPointer client_data,
     XtPointer call_data);

extern void
multicollide_cancel_callback(
     Widget w,
     XtPointer client_data,
     XtPointer call_data);

extern void
multicollide_ok_callback(
     Widget w,
     XtPointer client_data,
     XtPointer call_data);

extern void
buffer_multicollide_ok_callback(
     Widget w,
     XtPointer client_data,
     XtPointer call_data);

#endif  /* _NO_PROTO */

#ifdef _TT_MULTI_SCREEN
#define DESKTOP_DIR_ROOT (-1)
char* GetDesktopDir(int screen);
#endif

#ifdef  SUN_ACLFS
/* File ACL determination */

#include <sys/acl.h>
#include <dlfcn.h>

extern int (*acl_fptr)( char *, int, int, aclent_t* );
extern void *libc_handle;
#endif  /* SUN_ACLFS */

 /*
 * macro to get message catalog strings
 */

#ifdef MESSAGE_CAT
extern const char* _CLIENT_CAT_NAME;
/*
 * Without this proto, standard C says that _DtGetMessage() returns
 * an int, even though it really returns a pointer.  The compiler is
 * then free to use the high 32-bits of the return register for
 * something else (like scratch), and that can garble the pointer.
 */
#ifdef _NO_PROTO
extern char *_DtGetMessage();
#else  /* _NO_PROTO */
extern char *_DtGetMessage(
                          const char *filename,
                          int set,
                          int n,
                          char *s );
#endif /* _NO_PROTO */
#define GETMESSAGE(set, number, string)\
    (_DtGetMessage(_CLIENT_CAT_NAME, set, number, string))
#else /* MESSAGE_CAT */
#define GETMESSAGE(set, number, string)\
    string
#endif /* MESSAGE_CAT */

#endif /* _Main_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
