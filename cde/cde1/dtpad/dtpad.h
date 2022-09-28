/**********************************<+>*************************************
***************************************************************************
**
**  File:        dtpad.h
**
**  Project:     COSE dtpad, a memo maker type editor based on the motif
**               text widget.
**
**  Description:
**  -----------
**  This is the header file for the dtpad list of file which include:
**      dtpad.c
**
*******************************************************************
**  (c) Copyright Hewlett-Packard Company, 1990.  All rights are
**  reserved.  Copying or other reproduction of this program
**  except for archival purposes is prohibited without prior
**  written consent of Hewlett-Packard Company.
********************************************************************
**
********************************************************************
**  (c) Copyright 1993, 1994 Hewlett-Packard Company
**  (c) Copyright 1993, 1994 International Business Machines Corp.
**  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
**  (c) Copyright 1993, 1994 Novell, Inc.
********************************************************************
**
**
**************************************************************************
**********************************<+>*************************************/
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <Xm/Protocols.h>

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/CascadeB.h>
#include <Xm/CascadeBG.h>
#include <Xm/FileSB.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/MainW.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/SelectioB.h>
#include <Xm/SeparatoG.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>

#include <Dt/Session.h>
#include <Dt/Dt.h>
#include <Dt/Dts.h>
#include <Dt/Message.h>
#include <Dt/UserMsg.h>
#include <Dt/Editor.h>

#include <Tt/tttk.h>

#define DTPAD_CLASS_NAME	"Dtpad"
#define CHECKSPELLING		0
#define FINDCHANGE		1
#define OPTIONS			2

#define UNKNOWN_ACTION		-1
#define EXTEND_SELECTION	0

#define DEFAULT_GEOMETRY_STR	"80x24"
#define DEFAULT_COLS		80
#define DEFAULT_ROWS		24

typedef enum {
      SUCCESS,
      COPYOFTEMPFAILED,
      UNWRITABLEFILE,
      NOTEMPFILE
} WriteStatus;


/************************************************************************
 * Resources
 ************************************************************************/
typedef struct {
      Boolean	statusLine;
      Boolean	wordWrap;
      Boolean	overstrike;
      Boolean	saveOnClose;
      Boolean	missingFileWarning;
      Boolean	readOnlyWarning;
      Boolean	nameChange;
      Boolean	viewOnly;
      String	workspaceList;
      String	session;

      Boolean	standAlone;
      Boolean	blocking;
      Boolean	server;
      Boolean	exitOnLastClose;

      String	idString;
      String	directory;
      String	saveAsDir;
      String	mainTitle;
      Boolean	nameInTitle;
      String	exitLabel;
      String	exitMnemonic;
      String	helpVol;
      Boolean	menuBar;
      String	confirmationString;
      Boolean	allowExit;
      Boolean	closeImmediate;
} ApplicationData, *ApplicationDataPtr;


/************************************************************************
 * structures related to various dialogs
 ************************************************************************/
typedef struct {
	Widget	newl_radio,
		with_newl,
		without_newl;
} ToggleWidgets; 

/* -----> "Open" file selection box and "Save?" dialog widgets
 *        (indentation indicates widget hierarchy) */
typedef struct {
        Widget file_dialog,
               dir_list,
               filter_text,
               select_text,
               file_list,
               save_warning,
               work_area,
               row_col,
               wkArea_textLabel,
               separator,
               cancelBtn;
 ToggleWidgets toggleWidgets;
} Select;

/* -----> "Save As" file selection box and "Already Exists" dialog widgets
 *        (indentation indicates widget hierarchy) */
typedef struct {
        Widget saveAs_form,
	       alrdy_exist;
 ToggleWidgets toggleWidgets;
} SaveAs;

/* ----->  */
typedef struct _confirm {
    int confirmationStatus;	/* status from resource-spec'd comfirm dlg */
    Widget dialog,
	   wk_area,
	   wkArea_textLabel;
} ConfirmStuff;


/************************************************************************
 * structures related to menu bar functions
 ************************************************************************/
/* -----> structs related to "File" pulldown menu */
typedef struct {
    Select select;
    SaveAs saveAs;
    Widget gen_warning;
    Widget newBtn;
    Widget openBtn;
    Widget includeBtn;
    Widget saveBtn;
    Widget saveAsBtn;
    Widget printBtn;
    Widget exitBtn;
} FileWidgets;

typedef struct {
    void (*pendingFileFunc)();
    void (*pendingFileHelpFunc)();
    Boolean fileExists,
	    saveWithNewlines,
	    readOnly;
    char *fileName,
	 *netfile,		/* obpaque Took Talk string handle to file */
         *savingName,
	 pathDir[512];		/* dir in specified path */
    FileWidgets fileWidgets;
    XmString openTitle,
	     includeTitle,
	     saveBtnLabel,
	     saveNeededBtnLabel;
} FileStuff;

/* -----> structs related to "Edit" pulldown menu */
typedef struct {
    Widget undoBtn,
    	   cutBtn,
           copyBtn,
           pasteBtn,
           clearBtn,
           deleteBtn,
           selectAllBtn,
           findChangeBtn,
           checkSpellingBtn;
} EditWidgets;

typedef struct {
    EditWidgets widgets;
} EditStuff;

/* -----> structs related to "Format" pulldown menu */
typedef struct {
    Widget separator,
	   paragraph,
	   all,
	   close,
	   help,
	   formatBtn;
} FormatWidgets;

typedef struct {
    FormatWidgets widgets;
    int right_margin;		/* format size */
    int left_margin;
} FormatStuff;

/* -----> structs related to "Options" pulldown menu */
typedef struct {
    Widget overstrikeBtn,
	   wordWrapBtn,
	   statusLineBtn,
	   helpBtn,
	   optionsBtn;
} OptionsMenuWidgets;

typedef struct {
    OptionsMenuWidgets widgets;
} OptionsStuff;

/* -----> structs related to "Help" pulldown menu */
typedef struct _helpStruct {
    struct _helpStruct *pNext;
    struct _helpStruct *pPrev;
    Widget dialog;
    Boolean inUse;
} HelpStruct;


/************************************************************************
 * structs related to the status line
 ************************************************************************/
typedef struct _statusWidgets {
    Widget container,
	   lineLabel,
	   lineText,
	   colLabel,
	   colText;
} StatusWidgets;

typedef struct _status {
    StatusWidgets widgets;
    int currentLine,
    currentCol;
} StatusStuff;


/************************************************************************
 * structs related to a ToolTalk Edit/Display/Instantiate message and Quit
 ************************************************************************/
typedef struct _ttEditRequest {
    Tt_message contract;	/* media request */
    char *msg_id;		/* id of ToolTalk media request */
    Tttk_op op;			/* op code = Edit/Display/Instantiate */
    char *vtype;		/* media type */
    Boolean contents;		/* indicates file or buffer request */
    Boolean returnBufContents;	/* indicates if contents returned in reply */
    char *fileName;		/* file name supplied in media request */
    char *docName;		/* title of file/buffer */
} TTEditRequest;

typedef struct _ttQuitRequest {
    Tt_message contract;	/* original TTDT_QUIT request */	
    Boolean silent;		/* arg #1 */
    Boolean force;		/* arg #2 */
    char * operation2Quit;	/* arg #3 */
} TTQuitRequest;

/************************************************************************
 * Editor struct - the primary sturcture for each Editor instance
 ************************************************************************/
typedef struct _editor {
   char *progname; 		/* program name without the full pathname */
   char *dialogTitle;		/* string retrieved from message catalog */
   Display *display;		/* Display */
   XtAppContext app_context;	/* Application Context */
   ApplicationData  xrdb;
   TTEditRequest ttEditReq;	/* ToolTalk Edit/Display/Instantiate msg info */
   TTQuitRequest ttQuitReq;	/* ToolTalk Quit msg info */
   HelpStruct *pHelpCache;
   FileStuff fileStuff;		/* all data for File functions */
   EditStuff editStuff;		/* all data for Edit functions */
   FormatStuff formatStuff;	/* all data for Format functions */
   OptionsStuff optionsStuff;	/* all data for Options functions */
   ConfirmStuff confirmStuff;	/* data for resource spec'd confirm dialog */
   StatusStuff	statusStuff;
   Boolean nodo,
           saveRestore;
   Boolean iconic;
   Boolean iconicX, iconicY;    /* For old format - iconX,Y not specified */
   int iconX, iconY;            /* iconified position - X and Y */
   Position x,y;
   Dimension width, height;
   String geometry;
   char select_string[512];
   Widget app_shell,		/* ApplicationShell */
          editor,		/* Dt Editor widget */
          statusLineMsg,	/* Dt Editor widget status line text field */
          panedWindow,		/* Paned window */
          mainWindow,		/* MainWindow */
          menu_bar,		/* RowColumn */
          ExitWid,
	  MainHelp,
	  pseudo;
    unsigned short group_id,	/* users group id - for file type checking */
                   user_id;	/* the users id - for file type checking */
    struct _editor *pNextPad;
    struct _editor *pPrevPad;
    Boolean inUse;
    char *blockChannel;
} Editor;

typedef struct {
	Editor *pPad;
	int argc;
	char **argv;
} StartupStruct;



/************************************************************************
 *	macro to get message catalog strings
 ************************************************************************/
#ifdef MESSAGE_CAT
#define GETMESSAGE(set, number, string)\
    ((char *) _DtpadGetMessage(set, number, string))
#else /* MESSAGE_CAT */
#define GETMESSAGE(set, number, string)\
    string
#endif /* MESSAGE_CAT */

#define UNNAMED_TITLE ((char *)GETMESSAGE(7, 6, "(UNTITLED)"))
/*[SuG 5/10/95] Added for fixing print bug. */
#define UNNAMED_TITLE_P ((char *)GETMESSAGE(5, 21, "UNTITLED"))


/************************************************************************
 *			functions in fileCB.c
 ************************************************************************/
extern void FileCascadingCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void FileNewCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void FileOpenCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void FileIncludeCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void FileSaveCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void FileSaveAsCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void FilePrintCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void FileExitCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void NoSaveCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void CancelFileSelectCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void SaveNewLinesCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data);
extern void SaveAsOkCB(
	Widget w,
	caddr_t client_data,
	caddr_t call_data);
extern void AlrdyExistsOkCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void SaveAsCancelCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void AlrdyExistsCancelCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void AskIfSaveCancelCB(
	Widget w,
	caddr_t client_data,
	caddr_t call_data);
extern void ConfirmYesCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void ConfirmNoCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);


/************************************************************************
 *			functions in fileDlg.c
 ************************************************************************/
extern void PostAlreadyExistsDlg( 
        Editor *pPad);
extern void SetSaveAsDefaultString(Editor *pPad);
extern void CreateSaveAsDialog(
        Editor *pPad);
extern void ExtractAndStoreDir(
	Editor *pPad,
	char *fileName);
extern void GetFileName( 
	Editor *pPad,
        XmString title);
extern void Warning( 
        Editor *pPad,
        char *mess,
	unsigned char dialogType);
extern void AskIfSave(
        Editor *pPad);
extern void PostSaveError(
      Editor *pPad,
      char *saveName,
      DtEditorErrorCode errorCode);
extern void AskForConfirmation(
	Editor *pPad);


/************************************************************************
 *			functions in editCB.c
 ************************************************************************/
extern void EditUndoCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void EditCutCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void EditCopyCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void EditPasteCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void EditClearCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void EditDeleteCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void SelectAllCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void FindChangeCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void CheckSpellingCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void SetSelectionMenuItems(
        Editor *pPad,
	Boolean sensitivity);
extern void TextSelectedCB(
	Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void TextDeselectedCB(
	Widget w,
        caddr_t client_data,
        caddr_t call_data);


/************************************************************************
 *			functions in formatCB.c
 ************************************************************************/
extern void FormatCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void FormatParaCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);
extern void FormatAllCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);


/************************************************************************
 *			functions in optionsCB.c
 ************************************************************************/
extern void OverstrikeCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);

extern void WordWrapCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);

extern void StatusLineCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);


/************************************************************************
 *			functions in main.c
 ************************************************************************/
extern void CreatePad(
	Editor **ppPad);
extern void StartDbUpdate(
        XtPointer clientData);
extern void exitCB(
        DtSvcHandle service,
        DtSvcMsgContext replyContext,
        XtPointer clientData,
        String *fields,
        int numFields);
extern void setPanicConditions(void);
extern void SetWindowSize(
	Editor *pPad);
extern void SetAppShellResizeHints(
        Editor *pPad);
extern void RealizeNewPad(
        Editor *pPad);
extern void ManageOldPad(
        Editor *pPad,
        Boolean isCurrentlyVisible);
extern void InitDtEditSession(
	Editor *pPad,
	int argc,
	char **argv);
extern void SigcldHndlr(
	int dummy);
extern void UnmanageAllDialogs(
	Editor *pPad);
extern void RestoreInitialServerResources(
        Editor *pPad);

#ifdef MESSAGE_CAT
extern char * _DtpadGetMessage(
	int set,
	int number,
	char *string);
#endif /* MESSAGE_CAT */


/************************************************************************
 *			functions in dtpad.c
 ************************************************************************/
extern void CreateDefaultImage(
        unsigned char *bits,
        int width,
        int height,
        char *name);
extern void ChangeMainWindowTitle(
        Editor *pPad);
extern void SetWindowTitles(
        Editor *pPad);
extern void SetSaveAsLabelAndDialog(
        Editor *pPad);
extern void CreateMenuBar(
        Editor *pPad);
extern void CreateEditorWidget(
        Editor *pPad);
extern Boolean FindOrCreatePad(
	Editor **ppPad);
extern char *MbStrchr(
	char *str,
	int ch);
extern char *MbStrchri(
	char *str,
	int ch,
	int* in);
extern char *MbStrrchr(
	char *str,
	int ch);
extern char *MbBasename(
	char *str);
#ifdef MESSAGE_CAT
extern char *GetMessage(
        int set,
        int n,
        char *s);
#endif  /* MESSAGE_CAT */


/************************************************************************
 *			functions in session.c
 ************************************************************************/
extern void SaveSessionCB(
	Widget w,
	caddr_t client_data,
	caddr_t call_data );
extern void closeCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );
extern void restoreSession(
        Editor *pPad);
extern void RestorePad(
        Editor *pPad,
	int padNum,
        XrmDatabase db);


/************************************************************************
 *			functions in fileIo.c
 ************************************************************************/
extern void LoadFile(
        Editor *pPad,
        char *include);
extern char * GetTempFile(
	void);
extern void PanicSave(
	int);
extern void SetStatusLineMsg(
	Editor *pPad);


/************************************************************************
 *			functions in helpDlg.c
 ************************************************************************/
extern void SetHelpVolAndDisplayHelp(
	Editor *pPad,
	char *locationId,
	char *helpVolume);
extern void DisplayHelp(
        Editor *pPad,
        char *helpVolume,
        char *locationId);
extern void DisplayNewHelpWindow(
        Editor *pPad,
	char *helpVolume,
	char *locationId);
extern Widget CreateHelpDialog(
        Editor *pPad);
extern void DisplayHelpDialog(
        Editor *pPad,
	Widget helpDialog,
	char *helpVolume,
	char *locationId);


/************************************************************************
 *			functions in helpCB.c
 ************************************************************************/
extern void HelpMenubarCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );

extern void HelpFileCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );
extern void HelpEditCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );
extern void HelpFormatCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );
extern void HelpOptionsCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );
extern void HelpHelpCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );

extern void HelpOpenDialogCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );
extern void HelpIncludeDialogCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );
extern void HelpSaveAsDialogCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );
extern void HelpFileSaveNewLinesCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );
extern void HelpAskIfSaveDialogCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );
extern void HelpFileAlreadyExistsCB(
	Widget w,
	caddr_t client_data,
	caddr_t call_data );
extern void HelpConfirmDialogCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );

extern void HelpEditorWidgetCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );

extern void HelpOverviewCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );
extern void HelpTasksCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );
extern void HelpTOCCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );
extern void HelpReferenceCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );
extern void HelpOnItemCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );
extern void HelpUsingHelpCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );
extern void HelpAboutCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );

extern void HelpHyperlinkCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );
extern void HelpCloseCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );


/************************************************************************
 *			functions in messageCB.c
 ************************************************************************/
extern Tt_callback_action HandleTtRequest(
	Tt_message m,
	Tt_pattern p);
extern Tt_callback_action RequestCB(
	Tt_message m,
	Tt_pattern p);


/************************************************************************
 *			functions in ttMsgSupport.c
 ************************************************************************/
extern Boolean TTstartDesktopMediaExchange(
	Widget topLevelWithWmCommand,
	Editor *pPad);
extern Boolean TTmediaDepositContents(
	Editor *pPad);
extern Boolean TTmediaReply(
	Editor *pPad);
extern void TTresetQuitArgs(
	Editor *pPad);
extern void TTfailPendingQuit(
	Editor *pPad);
