/**********************************<+>*************************************
***************************************************************************
**
**  File:        EditMenu.c
**
**  Project:     Text Editor widget
**
**  Description:
**  -----------
**  This contains the support for the Text Editor widget popup menu.
**
*******************************************************************
**  (c) Copyright 1993, 1994 Hewlett-Packard Company
**  (c) Copyright 1993, 1994 International Business Machines Corp.
**  (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
**  (c) Copyright 1993, 1994 Novell, Inc.
********************************************************************
**
**
**************************************************************************
**********************************<+>*************************************/

/*-------------------------------------------------------------
**      Include Files
*/

#include <ctype.h>
#include <unistd.h>
#include <pwd.h>
#include <locale.h>

#ifdef __hpux
#include <wchar.h> 
#endif /* __hpux */

#ifdef sun
#if (_XOPEN_VERSION==3)
#include <wctype.h>
#else
#include <wchar.h>
#endif
#include <libintl.h>
#endif /* sun */

#include <string.h>

#include <limits.h>	/* For LINE_MAX definition */

#ifdef USE_SDTSPELL
#include <dlfcn.h>
#endif /* USE_SDTSPELL */

#include "EditorP.h"
#include "DtMsgsP.h"

#include <Xm/RowColumn.h>
#include <Xm/CascadeBG.h>
#include <Xm/PushBG.h>
#include <Xm/SeparatoG.h>
#include <Xm/ToggleBG.h>
#include <Xm/FileSB.h>
#include <Xm/DialogS.h>

/*-------------------------------------------------------------
**      Public Interface
**-------------------------------------------------------------
*/

/********    Public Function Declarations    ********/

/********    End Public Function Declarations    ********/

/*-------------------------------------------------------------
**      Forward Declarations
**-------------------------------------------------------------
*/

/********    Static Function Declarations    ********/

static void	EditCascadingCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);

static void	OptionsCascadingCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);

static void	ExtrasCascadingCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);

static void	FileIncludeCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );

static void	IncludeFile(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );

static void	CreateFileSelectionDialog(
	DtEditorWidget editor );

static void	EditUndoCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );

static void	EditCutCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );

static void	EditCopyCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );

static void	EditPasteCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );

static void	EditClearCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );

static void	EditDeleteCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );

static void	SelectAllCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );

static void	FindChangeCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );

static void	CheckSpellingCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data );

static void	FormatCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);

static void	FormatParaCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);

static void	FormatAllCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);

static void	OverstrikeCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);

static void	WordWrapCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);

static void	StatusLineCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data);

static void	ExtrasItemCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data);

/********    End Static Function Declarations    ********/


/****************************************************************
 *
 *	Defines
 *
 ****************************************************************/

#ifdef nodef
#define NL_SETN 1

#ifdef _AIX
#define	iswctype(a,b)	is_wctype(a,b)
#define wctype(a)	get_wctype(a)
#endif /* _AIX */

#if !(defined(sun) && (_XOPEN_VERSION==3))
#define iswblank(wc)	iswctype((wc),blnkclass)
#undef getwc				/* Use the libc function */
const char *blankString = "space";
static wctype_t _DtEditor_blankClass;
#else
#define wctype_t 	int
#define iswblank(a)	iswspace(a)
#define	iswctype(a,b)	_iswctype(a,b)
#define wcwidth(a)	sun_wcwidth(a)
#endif /* not sun */
#endif

/*
 * Structure of the [Edit] menu.
 */
#define	EDIT_UNDO		0
#define	EDIT_SEP1		1
#define	EDIT_CUT		2
#define	EDIT_COPY		3
#define	EDIT_PASTE		4
#define	EDIT_CLEAR		5
#define	EDIT_DELETE		6
#define	EDIT_SELECT_ALL		7
#define	EDIT_SEP2		8
#define	EDIT_FIND		9
#define	EDIT_SPELL		10
#define	EDIT_NUMITEMS		11

/*
 * Structure of the [Options] menu.
 */
#define	OPTIONS_OVERSTRIKE	0
#define	OPTIONS_WRAP		1
#define	OPTIONS_STATUS		2


/****************************************************************
 *
 *  Private Procs
 *
 ****************************************************************/

void
_DtEditorCreateMenu(
	DtEditorWidget	editor)
{
	M_menu(editor) =
	    XmCreatePopupMenu(M_text(editor), "_dteditor_menu", NULL, 0);
	DtEditorCreateFileMenu((Widget)editor, M_menu(editor));
	DtEditorCreateEditMenu((Widget)editor, M_menu(editor));
	DtEditorCreateFormatMenu((Widget)editor, M_menu(editor));
	DtEditorCreateOptionsMenu((Widget)editor, M_menu(editor));
	DtEditorCreateExtrasMenu((Widget)editor, M_menu(editor));
}

#define	GETMESSAGE(a, b, c)	(c)	/* XXX - kludge no message catalog */
#define	charset			XmFONTLIST_DEFAULT_TAG


/************************************************************************
 * DtEditorCreateFileMenu - Create the "File" pulldown menu
 *
 *	Include...
 *		
 ************************************************************************/
void
DtEditorCreateFileMenu(
	Widget		editor,
	Widget		parent)
{
    Widget file_pane, cascade, WidgList[12];
    Arg al[10];
    int ac, count = 0;
    char      *mnemonic;
    XmString tmpStr;

    /* -----> create "File" PulldownMenu */
    ac = 0;
    file_pane = XmCreatePulldownMenu(parent, "fileMenu", al, ac);
#ifdef help_available
    XtAddCallback(file_pane, XmNhelpCallback, 
		  (XtCallbackProc)HelpFileCB, editor);
#endif
    mnemonic = strdup(FILE_MNEMONIC);
    tmpStr = XmStringCreateLtoR(FILE_MENU, charset);
    ac = 0;
    XtSetArg(al[ac], XmNsubMenuId, file_pane);  ac++;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    cascade = XmCreateCascadeButtonGadget(parent, "File", al, ac);
    XtManageChild(cascade);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Include..." push button */
    mnemonic = strdup(INCLUDE_MNEMONIC);
    tmpStr = XmStringCreateLtoR(INCLUDE_MENU, charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget(file_pane, "include", al, ac);
    XtAddCallback(WidgList[count-1], XmNactivateCallback, 
		   (XtCallbackProc) FileIncludeCB, (XtPointer) editor);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> manage all the "File" menu buttons */
    XtManageChildren(WidgList, count);
}


/************************************************************************
 * DtEditorCreateEditMenu - Create the "Edit" pulldown menu
 *		
 *	Undo
 *	----------
 *	Cut
 *	Copy
 *	Paste
 *	Clear
 *	Delete
 *	Select All
 *	----------
 *	Find/Change...
 *	Check Spellilng...
 *		
 ************************************************************************/
void
DtEditorCreateEditMenu(
	Widget		editor,
	Widget		parent)
{
    Widget edit_pane, cascade, WidgList[12];
    Arg al[10];
    int ac, count = 0;
    char *mnemonic;
    XmString tmpStr;
    XmTextPosition left, right;

    /* -----> create "Edit" PulldownMenu */
    ac = 0;
    edit_pane = XmCreatePulldownMenu(parent, "editMenu", al, ac);
#ifdef help_available
    XtAddCallback(edit_pane, XmNhelpCallback, 
		  (XtCallbackProc)HelpEditCB, editor);
#endif
    mnemonic = strdup(EDIT_MNEMONIC);
    tmpStr = XmStringCreateLtoR(EDIT_MENU, charset);
    ac = 0;
    XtSetArg(al[ac], XmNsubMenuId, edit_pane); ac++;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    cascade = XmCreateCascadeButtonGadget(parent, "Edit", al, ac);
    XtAddCallback(cascade, XmNcascadingCallback, 
		  (XtCallbackProc)EditCascadingCB, editor);
    XtManageChild(cascade);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Undo" push button */
    mnemonic = strdup(UNDO_MNEMONIC);
    tmpStr = XmStringCreateLtoR(UNDO_MENU, charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget(edit_pane, "undo", al, ac);
    XtAddCallback(WidgList[count-1], XmNactivateCallback, 
		  (XtCallbackProc) EditUndoCB, (XtPointer) editor);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create undo separator */
    ac = 0;
    WidgList[count++] = XmCreateSeparatorGadget(edit_pane, "CtSp", al, ac);

    /* -----> create "Cut" push button */
    mnemonic = strdup(CUT_MNEMONIC);
    tmpStr = XmStringCreateLtoR(CUT_MENU, charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget(edit_pane, "cut", al, ac);
    XtAddCallback(WidgList[count-1], XmNactivateCallback, 
		  (XtCallbackProc) EditCutCB, (XtPointer) editor);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Copy" push button */
    mnemonic = strdup(COPY_MNEMONIC);
    tmpStr = XmStringCreateLtoR(COPY_MENU, charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget(edit_pane, "copy", al, ac);
    XtAddCallback(WidgList[count-1], XmNactivateCallback, 
		  (XtCallbackProc) EditCopyCB, (XtPointer) editor);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Paste" push button */
    mnemonic = strdup(PASTE_MNEMONIC);
    tmpStr = XmStringCreateLtoR(PASTE_MENU, charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget(edit_pane, "paste", al, ac);
    XtAddCallback(WidgList[count-1], XmNactivateCallback, 
		  (XtCallbackProc) EditPasteCB, (XtPointer) editor);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Clear" push button */
    mnemonic = strdup(CLEAR_MNEMONIC);
    tmpStr = XmStringCreateLtoR(CLEAR_MENU, charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget(edit_pane, "clear", al, ac);
    XtAddCallback(WidgList[count-1], XmNactivateCallback, 
		  (XtCallbackProc) EditClearCB, (XtPointer) editor);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Delete" push button */
    mnemonic = strdup(DELETE_MNEMONIC);
    tmpStr = XmStringCreateLtoR(DELETE_MENU, charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget(edit_pane, "delete", al, ac);
    XtAddCallback(WidgList[count-1], XmNactivateCallback, 
		  (XtCallbackProc) EditDeleteCB, (XtPointer) editor);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> XXX - create "Select All" push button */
    mnemonic = strdup(SALL_MNEMONIC);
    tmpStr = XmStringCreateLtoR(SALL_MENU, charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget(edit_pane, "selectAll", al, ac);
    XtAddCallback(WidgList[count-1], XmNactivateCallback, 
		  (XtCallbackProc) SelectAllCB, (XtPointer) editor);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create find/change separator */
    ac = 0;
    WidgList[count++] = XmCreateSeparatorGadget(edit_pane, "FSSp", al, ac);

    /* -----> create "Find/Change" push button */
    mnemonic = strdup(FC_MNEMONIC);
    tmpStr =XmStringCreateLtoR(FC_MENU, charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = 
		XmCreatePushButtonGadget(edit_pane, "findChange", al, ac);
    XtAddCallback(WidgList[count-1], XmNactivateCallback, 
		  (XtCallbackProc) FindChangeCB, (XtPointer) editor);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Check Spelling" push button (don't do if 16-bit data) */
    if (MB_CUR_MAX <= 1) {
        mnemonic = strdup(SPELL_MNEMONIC);
        tmpStr = XmStringCreateLtoR(SPELL_MENU, charset);
        ac = 0;
        XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
        XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
        WidgList[count++] = XmCreatePushButtonGadget(edit_pane, "checkSpelling", 
						     al, ac);
        XtAddCallback(WidgList[count-1], XmNactivateCallback, 
		      (XtCallbackProc) CheckSpellingCB, (XtPointer) editor);
        XtFree(mnemonic);
        XmStringFree(tmpStr);
    }

    /* indicate that the sensitivity and editable flags are not initialized */
    XtSetArg(al[0], XmNuserData, -1);
    XtSetValues(cascade, al, 1);

    /* -----> manage all the "Edit" menu buttons */
    XtManageChildren(WidgList, count);
}


/************************************************************************
 * DtEditorCreateFormatMenu - Create the "Format" pulldown menu
 *
 *	Settings...
 *	Paragraph
 *	All
 *		
 ************************************************************************/
void
DtEditorCreateFormatMenu(
	Widget		editor,
	Widget		parent)
{
    Widget format_pane, cascade, WidgList[12];
    Arg al[10];
    int ac, count = 0;
    char *mnemonic;
    XmString tmpStr;

    /* -----> create "Format" PulldownMenu */
    ac = 0;
    format_pane = XmCreatePulldownMenu(parent, "formatMenu", al, ac);
#ifdef help_available
    XtAddCallback(format_pane, XmNhelpCallback, 
		  (XtCallbackProc)HelpFormatCB, editor);
#endif
    mnemonic = strdup(FORMAT_MNEMONIC);
    tmpStr = XmStringCreateLtoR(FORMAT_MENU, charset);
    ac = 0;
    XtSetArg(al[ac], XmNsubMenuId, format_pane);  ac++;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    cascade= XmCreateCascadeButtonGadget(parent, "format", al, ac);
    XtManageChild(cascade);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Settings..." button */
    mnemonic = strdup(SETTINGS_MNEMONIC);
    tmpStr = XmStringCreateLtoR(SETTINGS_MENU, charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] =
                   XmCreatePushButtonGadget(format_pane, "settings", al, ac);
    XtAddCallback(WidgList[count-1], XmNactivateCallback, 
		   (XtCallbackProc) FormatCB, (XtPointer) editor);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Paragraph" button */
    mnemonic = strdup(PARAG_MNEMONIC);
    tmpStr = XmStringCreateLtoR(PARAG_MENU, charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] =
                  XmCreatePushButtonGadget(format_pane, "paragraph", al, ac);
    XtAddCallback(WidgList[count-1], XmNactivateCallback,
		   (XtCallbackProc) FormatParaCB, (XtPointer) editor);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "All" button */
    mnemonic = strdup(ALL_MNEMONIC);
    tmpStr = XmStringCreateLtoR(ALL_MENU, charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget(format_pane, "all", al, ac);
    XtAddCallback(WidgList[count-1], XmNactivateCallback, 
		   (XtCallbackProc) FormatAllCB, (XtPointer) editor);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> manage the all "Format" menu buttons */
    XtManageChildren(WidgList, count);
}


/************************************************************************
 * DtEditorCreateOptionsMenu - Create the "Options" pulldown menu
 *
 *	Overstrike
 *	Wrap To Fit
 *	Status Line
 *		
 ************************************************************************/
void
DtEditorCreateOptionsMenu(
	Widget		widget,
	Widget		parent)
{
    Widget options_pane, cascade, WidgList[12];
    DtEditorWidget editor = (DtEditorWidget)widget;
    Arg al[10];
    int ac, count = 0;
    char *mnemonic;
    XmString tmpStr;

    /* -----> create "Options" PulldownMenu */
    ac = 0;
    options_pane = XmCreatePulldownMenu(parent, "optionsMenu", al, ac);
#ifdef help_available
    XtAddCallback(options_pane, XmNhelpCallback, 
		  (XtCallbackProc)HelpOptionsCB, editor);
#endif
    mnemonic = strdup(OPTIONS_MNEMONIC);
    tmpStr = XmStringCreateLtoR(OPTIONS_MENU, charset);
    ac = 0;
    XtSetArg(al[ac], XmNsubMenuId, options_pane);  ac++;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    cascade= XmCreateCascadeButtonGadget(parent, "options", al, ac);
    XtAddCallback(cascade, XmNcascadingCallback, 
		  (XtCallbackProc)OptionsCascadingCB, editor);
    XtManageChild(cascade);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Overstrike" toggle button */
    mnemonic = strdup(OSTRIKE_MNEMONIC);
    tmpStr = XmStringCreateLtoR(OSTRIKE_MENU, charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    XtSetArg(al[ac], XmNvisibleWhenOff, True); ac++;
    XtSetArg(al[ac], XmNset, M_overstrikeMode(editor)); ac++;
    WidgList[count++] =
      XmCreateToggleButtonGadget(options_pane, "overstrike", al, ac);
    XtAddCallback(WidgList[count-1], XmNvalueChangedCallback, 
		   (XtCallbackProc) OverstrikeCB, (XtPointer) editor);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Wrap To Fit" toggle button */
    mnemonic = strdup(WRAP_MNEMONIC);
    tmpStr = XmStringCreateLtoR(WRAP_MENU, charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    XtSetArg(al[ac], XmNvisibleWhenOff, True); ac++;
    XtSetArg(al[ac], XmNset, M_wordWrap(editor)); ac++;
    WidgList[count++] =
      XmCreateToggleButtonGadget(options_pane, "wordWrap", al, ac);
    XtAddCallback(WidgList[count-1], XmNvalueChangedCallback, 
		   (XtCallbackProc) WordWrapCB, (XtPointer) editor);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Status Line" toggle button */
    mnemonic = strdup(STATUS_MNEMONIC);
    tmpStr = XmStringCreateLtoR(STATUS_MENU, charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    XtSetArg(al[ac], XmNvisibleWhenOff, True); ac++;
    XtSetArg(al[ac], XmNset, M_status_showStatusLine(editor)); ac++;
    WidgList[count++] =
      XmCreateToggleButtonGadget(options_pane, "statusLine", al, ac);
    XtAddCallback(WidgList[count-1], XmNvalueChangedCallback, 
		   (XtCallbackProc) StatusLineCB, (XtPointer) editor);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    XtManageChildren(WidgList, count);
}


/************************************************************************
 * DtEditorCreateExtrasMenu - Create the "Extras" pulldown menu
 *
 *		
 ************************************************************************/
void
DtEditorCreateExtrasMenu(
	Widget		editor,
	Widget		parent)
{
    Widget extras_pane, cascade, WidgList[12];
    Arg al[10];
    int ac, count = 0;
    char *mnemonic;
    XmString tmpStr;

    /* -----> create "Extras" PulldownMenu */
    ac = 0;
    extras_pane = XmCreatePulldownMenu(parent, "extrasMenu", al, ac);
#ifdef help_available
    XtAddCallback(extras_pane, XmNhelpCallback, 
		  (XtCallbackProc)HelpExtrasCB, editor);
#endif
    mnemonic = strdup(EXTRAS_MNEMONIC);
    tmpStr = XmStringCreateLtoR(EXTRAS_MENU, charset);
    ac = 0;
    XtSetArg(al[ac], XmNsubMenuId, extras_pane);  ac++;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    cascade = XmCreateCascadeButtonGadget(parent, "extras", al, ac);
    XtAddCallback(cascade, XmNcascadingCallback, 
		  (XtCallbackProc)ExtrasCascadingCB, editor);
    XtManageChild(cascade);
    XtFree(mnemonic);
    XmStringFree(tmpStr);
}



/************************************************************************
 * FileIncludeCB - callback assigned to "File" menu "Include..." button
 ************************************************************************/
/* ARGSUSED */
static void
FileIncludeCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    DtEditorWidget editor = (DtEditorWidget)client_data;
    Widget dialog;
    Arg al[10];
    int ac;

    _DtTurnOnHourGlass(M_topLevelShell(editor));

    /* -----> create the FSB if it doesn't exits */
    if (M_menu_includeDialog(editor) == (Widget)NULL) {
	CreateFileSelectionDialog(editor);
    }

    dialog = M_menu_includeDialog(editor);
    /* -----> set the FSB "ok" and "help" button callbacks */
    XtRemoveAllCallbacks(dialog, XmNokCallback);
    XtAddCallback(dialog, XmNokCallback,
			(XtCallbackProc)IncludeFile,
			(XtPointer)editor);
#ifdef help_available
    XtRemoveAllCallbacks(dialog, XmNhelpCallback);
    XtAddCallback(dialog, XmNhelpCallback,
			(XtCallbackProc) pStuff->pendingFileHelpFunc, 
			(XtPointer)editor);
#endif

    /* -----> find the seeded directory */
    XmFileSelectionDoSearch(dialog, NULL);

    /* -----> manage the FSB */
    XtManageChild(dialog);
    XmProcessTraversal(XmFileSelectionBoxGetChild(dialog,
					XmDIALOG_TEXT), XmTRAVERSE_CURRENT);
    XSync(M_display(editor), 0);

    _DtTurnOffHourGlass(M_topLevelShell(editor));
}


/************************************************************************
 * CancelFileSelectCB - 
 ************************************************************************/
/* ARGSUSED */
static void
CancelFileSelectCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    DtEditorWidget 	editor = (DtEditorWidget)client_data;

    /* popdown the file selection box */
    XtUnmanageChild(w);
    _DtTurnOffHourGlass(w);
    _DtTurnOffHourGlass(M_topLevelShell(editor));
}


/************************************************************************
 * IncludeFile - obtains the name of a file to include (via a Xm FSB),
 *	and inserts the file contents into the Dt Editor Widget.
 *
 *	This callback is assigned to the "Ok" button of the
 *	File Selection Box displayed by the callback,
 *	FileIncludeCB() assigned to the "File" menu "Include" button.
 ************************************************************************/
static void
IncludeFile(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    DtEditorWidget 	editor = (DtEditorWidget)client_data;
    XmFileSelectionBoxCallbackStruct *cb = (XmFileSelectionBoxCallbackStruct *)
					   call_data;
    DtEditorErrorCode	error;
    char *name;

    name = (char *)XtMalloc(sizeof(char) * cb->length + 1);
    name[0] ='\0';

    _DtTurnOnHourGlass(M_topLevelShell(editor));
    _DtTurnOnHourGlass(w);

    /*
     *  Get the name of the file
     */
    XmStringGetLtoR(cb->value, XmFONTLIST_DEFAULT_TAG, &name);

    error = DtEditorInsertFromFile((Widget)editor, name);
    if (error != DtEDITOR_NO_ERRORS && error != DtEDITOR_READ_ONLY_FILE)
        XtCallActionProc(M_text(editor), "beep", cb->event, NULL, 0);
    CancelFileSelectCB(w, client_data, call_data);
    if (name != (char *)NULL)
        XtFree(name);
}


/************************************************************************
 * CreateFileSelectionDialog - 
 ************************************************************************/
static void 
CreateFileSelectionDialog(
        DtEditorWidget editor )
{
    Arg al[10];             /* arg list */
    register int ac;        /* arg count */
    XmString tmpStr, tmpStr2;

    ac = 0;
    tmpStr = XmStringCreateLtoR((char *)GETMESSAGE(2, 1, "Help"),
					XmFONTLIST_DEFAULT_TAG);
    XtSetArg(al[ac], XmNhelpLabelString, tmpStr);	ac++;
    tmpStr2 = XmStringCreateLtoR((char *)GETMESSAGE(4, 3, "Include a File"),
					XmFONTLIST_DEFAULT_TAG);
    XtSetArg(al[ac], XmNdialogTitle, tmpStr2); ac++;

    M_menu_includeDialog(editor) = XmCreateFileSelectionDialog((Widget)editor,
					"file_sel_dlg", al, ac);
    XmStringFree(tmpStr);
    XmStringFree(tmpStr2);

    XtAddCallback(M_menu_includeDialog(editor), XmNcancelCallback,
			(XtCallbackProc) CancelFileSelectCB, (XtPointer)editor);

    XtRealizeWidget(M_menu_includeDialog(editor));
    XSync(M_display(editor), 0);
}

/************************************************************************
 * EditCascadingCB - Called when bringing up [Edit] menu.
 *
 *	Set the sensitivity of the edit menu widgets based
 *	on whether there's a current selection and whether
 *	the Editor widget is editable.
 *
 *	"w" is the Edit XmCascadeButtonGadget
 ************************************************************************/
static void
EditCascadingCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data)
{
    DtEditorWidget editor = (DtEditorWidget)client_data;
    Widget menu = NULL;
    Widget *WidgList = NULL;
    Arg al[2];
    int oldflags = -1, newflags = 0;

    if (M_editable(editor))
        newflags |= 1;
    if (M_hasSelection(editor))
        newflags |= 2;
    XtSetArg(al[0], XmNuserData, &oldflags);
    XtGetValues(w, al, 1);
    if (oldflags == -1)
	oldflags = ~newflags;
    if (oldflags == newflags)
        return;
    XtSetArg(al[0], XmNuserData, newflags);
    XtSetValues(w, al, 1);

    XtSetArg(al[0], XmNsubMenuId, &menu);
    XtGetValues(w, al, 1);
    XtSetArg(al[0], XmNchildren, &WidgList);
    XtGetValues(menu, al, 1);

    if ((oldflags & 1) != (newflags & 1)) {
        XtSetSensitive(WidgList[EDIT_UNDO],
          M_editable(editor));
        XtSetSensitive(WidgList[EDIT_PASTE],
          M_editable(editor));
    }
    if ((oldflags & 2) != (newflags & 2)) {
        XtSetSensitive(WidgList[EDIT_COPY],
          M_hasSelection(editor));	/* Copy can be done in viewOnly mode */
    }
    XtSetSensitive(WidgList[EDIT_CUT],
      M_hasSelection(editor) && M_editable(editor));
    XtSetSensitive(WidgList[EDIT_CLEAR],
      M_hasSelection(editor) && M_editable(editor));
    XtSetSensitive(WidgList[EDIT_DELETE],
      M_hasSelection(editor) && M_editable(editor));
}

/************************************************************************
 * EditUndoCB - [Edit] menu, [Undo] button.
 *	Undoes the last edit.
 ************************************************************************/
/* ARGSUSED */
static void
EditUndoCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    DtEditorUndoEdit((Widget)client_data);
}


/************************************************************************
 * EditCutCB - [Edit] menu, [Cut] button.
 *	Cuts the current selection to the Motif clipboard.
 ************************************************************************/
/* ARGSUSED */
static void
EditCutCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    DtEditorCutToClipboard((Widget)client_data);
}


/************************************************************************
 * EditCopyCB - [Edit] menu, [Copy] button.
 *	Copies the current selection to the Motif clipboard.
 ************************************************************************/
/* ARGSUSED */
static void
EditCopyCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    DtEditorCopyToClipboard((Widget)client_data);
}


/************************************************************************
 * EditPasteCB - [Edit] menu, [Paste] button.
 *	Pastes from the Motif clipboard.
 ************************************************************************/
/* ARGSUSED */
static void
EditPasteCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    DtEditorPasteFromClipboard((Widget)client_data);
}


/************************************************************************
 * EditClearCB - [Edit] menu, [Replace] button
 *	Replaces the current selection with blanks.
 ************************************************************************/
/* ARGSUSED */
static void
EditClearCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    DtEditorClearSelection((Widget)client_data);
}


/************************************************************************
 * EditDeleteCB - [Edit] menu, [Delete] button.
 *	Deletes the current selection.
 ************************************************************************/
/* ARGSUSED */
static void
EditDeleteCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    DtEditorDeleteSelection((Widget)client_data);
}


/************************************************************************
 * SelectAllCB - [Edit] menu, [Select All] button.
 *	Selects all text.
 ************************************************************************/
/* ARGSUSED */
static void
SelectAllCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    DtEditorSelectAll((Widget)client_data);
}


/************************************************************************
 * FindChangeCB - [Edit] menu, [Find/Change...] button.
 *	Invokes the Dt Editor widget search dialog.
 ************************************************************************/
/* ARGSUSED */
static void
FindChangeCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    DtEditorInvokeFindChangeDialog((Widget)client_data);
}


/************************************************************************
 * CheckSpellingCB - [Edit] menu, [Check Spelling...] button.
 *	Invokes the Dt Editor widget spell dialog.
 ************************************************************************/
/* ARGSUSED */
static void
CheckSpellingCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
#ifdef USE_SDTSPELL
    /* Function pointer to SDtInvokeSpell(), as returned by dlsym. */
    Boolean (*SDtInvokeSpell_FP)(Widget);
    void *SDtSpellLibHandle;
    
    /* Can we access the Solaris CDE spelling checker? */
    SDtSpellLibHandle = dlopen("libSDtSpell.so", RTLD_LAZY);
    if (SDtSpellLibHandle != (void *) NULL) {
	/* We have access to the Sun Spell Checking library.
	 * Get the handle to the spell checking function.
	 */
	SDtInvokeSpell_FP = (Boolean (*)(Widget)) dlsym(SDtSpellLibHandle, 
							"SDtInvokeSpell");

	if (SDtInvokeSpell_FP != NULL)
	    /* Run the Sun Spell checker. */
	    if ((*SDtInvokeSpell_FP)((Widget) client_data))
		return;
    }
#endif

    /* If not Sun, or if Sun but could not open the Solaris CDE
     * spelling checker, then default to the standard CDE spelling
     * checker.
     */

    DtEditorInvokeSpellDialog((Widget)client_data);
}


/************************************************************************
 * FormatCB - [Format] menu, [Settings...] button
 ************************************************************************/
/* ARGSUSED */
static void
FormatCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data)
{
    DtEditorInvokeFormatDialog((Widget)client_data);
}


/************************************************************************
 * FormatParaCB - [Format] menu, [Paragraph] button
 ************************************************************************/
/* ARGSUSED */
static void
FormatParaCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data)
{
    DtEditorFormat((Widget)client_data, (DtEditorFormatSettings *) NULL,
	DtEDITOR_FORMAT_PARAGRAPH);
}


/************************************************************************
 * FormatAllCB - [Format] menu, [All] button
 ************************************************************************/
/* ARGSUSED */
static void
FormatAllCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data)
{
    DtEditorFormat((Widget)client_data, (DtEditorFormatSettings *) NULL,
	DtEDITOR_FORMAT_ALL);
}

/************************************************************************
 * OptionsCascadingCB - Called when bringing up [Options] menu.
 *
 *	Set the state of the option menu widgets based
 *	on current Editor widget state.
 *
 *	"w" is the Options XmCascadeButtonGadget
 ************************************************************************/
static void
OptionsCascadingCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data)
{
    DtEditorWidget editor = (DtEditorWidget)client_data;
    Widget menu = NULL;
    Widget *WidgList = NULL;
    Arg al[2];

    XtSetArg(al[0], XmNsubMenuId, &menu);
    XtGetValues(w, al, 1);
    XtSetArg(al[0], XmNchildren, &WidgList);
    XtGetValues(menu, al, 1);

    XmToggleButtonGadgetSetState(WidgList[OPTIONS_OVERSTRIKE],
      M_overstrikeMode(editor), False);
    XmToggleButtonGadgetSetState(WidgList[OPTIONS_WRAP],
      M_wordWrap(editor), False);
    XmToggleButtonGadgetSetState(WidgList[OPTIONS_STATUS],
      M_status_showStatusLine(editor), False);
}


/************************************************************************
 * OverstrikeCB - set Editor widget overstike state based on value passed in
 *	call_data->set.
 ************************************************************************/
/* ARGSUSED */
static void
OverstrikeCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data)
{
    Arg al[1];
    XmToggleButtonCallbackStruct *cb = (XmToggleButtonCallbackStruct *) 
				       call_data;

    XtSetArg(al[0], DtNoverstrike, (Boolean) cb->set);
    XtSetValues((Widget)client_data, al, 1);
}


/************************************************************************
 * WordWrapCB - set Editor widget word wrap state based on value passed in
 *	call_data->set.
 ************************************************************************/
/* ARGSUSED */
static void
WordWrapCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data)
{
    Arg al[1];
    XmToggleButtonCallbackStruct *cb = (XmToggleButtonCallbackStruct *) 
				       call_data;

    XtSetArg(al[0], DtNwordWrap, (Boolean) cb->set);
    XtSetValues((Widget)client_data, al, 1);
}


/************************************************************************
 * StatusLineCB - set Editor widget statusLine based on value passed in
 *	call_data->set.
 ************************************************************************/
/* ARGSUSED */
static void
StatusLineCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data)
{
    Arg al[1];
    XmToggleButtonCallbackStruct *cb = (XmToggleButtonCallbackStruct *) 
				       call_data;
				       
    XtSetArg(al[0], DtNshowStatusLine, (Boolean) cb->set);
    XtSetValues((Widget)client_data, al, 1);
}

/************************************************************************
 * _DtEditorLoadExtras - load the textsw extras file into a common
 *	data structure shared by all DtEditor instances in this
 *	process.
 ************************************************************************/

#define	MAXARGS		100	/* maximum number of arguments per command */
typedef struct _extrasCmds {
	char	*args[MAXARGS+1];	/* args[0] is the label */
} ExtrasCmds;

static	ExtrasCmds *extrasCmds;
static	int numCmds;

static char *
FindMenuFile(char *name, char *locale)
{
	static char buf[PATH_MAX + 1];
	char *p;
	static char *expand(char *);

	/* check for overflow of buffer */
	if (strlen(name) + strlen(locale) + 2 > sizeof (buf))
		return (NULL);

	strcpy(buf, name);
	strcat(buf, ".");
	strcat(buf, locale);
	p = expand(buf);
	if (access(p, R_OK) >= 0)
		return (p);
	p = expand(name);
	if (access(p, R_OK) >= 0)
		return (p);
	return (NULL);
}

/*
 * Load the extras menu file.  We locate the file using the
 * following algorithm:
 *
 * 0. DtNextrasMenuFilename  (XXX - should I try with {.<locale>} as well?)
 * 1. text.extrasMenuFilename{.<locale>} (Xdefaults)
 * 2. $(EXTRASMENU){.<locale>} (environment variable),
 * 3. $(HOME)/.text_extras_menu{.<locale>} (home dir),
 * 4. locale sensitive system default
 *       ("/usr/dt/appconfig/dteditor/<locale>/.text_extras_menu")
 */
void
_DtEditorLoadExtras(DtEditorWidget editor)
{
	char *extrasFile = M_menu_extrasFileContents(editor);
	struct stat sb;
	char *p, *np, **ap, quote;
	int fd;
	ssize_t len;
	ExtrasCmds *ep;

	if (M_menu_realExtrasFilename(editor) == NULL) {
		static char buf[PATH_MAX + 1];
		XrmDatabase db;
		XrmValue value;
		char *name, *locale;

		locale = setlocale(LC_MESSAGES, NULL);
		if (locale == NULL)
			locale = "C";

		if ((p = M_menu_extrasMenuFilename(editor)) != NULL &&
		    (name = FindMenuFile(p, locale)) != NULL)
			goto found;

		/*
		 * Get the resource database for the display associated
		 * with the widget and see if the filename is specified
		 * in there.
		 */
		db = XrmGetStringDatabase(
		    XResourceManagerString(XtDisplay(editor)));
		if (XrmGetResource(db, "text.extrasMenuFilename",
		    "text.extrasMenuFilename", &name, &value) &&
		    (name = FindMenuFile(value.addr, locale)) != NULL)
			goto found;

		if ((p = getenv("EXTRASMENU")) != NULL &&
		    (name = FindMenuFile(p, locale)) != NULL)
			goto found;

		if ((p = getenv("HOME")) != NULL) {
			/* XXX - check for overflow */
			strcpy(buf, p);
			strcat(buf, "/.text_extras_menu");
			if ((name = FindMenuFile(buf, locale)) != NULL)
				goto found;
		}
		strcpy(buf, "/usr/dt/appconfig/dteditor/");
		strcat(buf, locale);
		strcat(buf, "/.text_extras_menu");
		if (access(buf, R_OK) >= 0) {
			name = buf;
			goto found;
		}
		if ((p = getenv("OPENWINHOME")) == NULL)
			p = "/usr/openwin";
		sprintf(buf, "%s/lib/locale/%s/xview/.text_extras_menu",
		    p, locale);
		if (access(buf, R_OK) >= 0) {
			name = buf;
			goto found;
		}
		/* no file anywhere, give up */
		return;
	found:
		M_menu_realExtrasFilename(editor) = strdup(name);
	}
	sb.st_mtime = 0;	/* in case stat fails */
	if (stat(M_menu_realExtrasFilename(editor), &sb) >= 0 &&
	    sb.st_mtime == M_menu_extrasTime(editor))
		return;		/* no change, just return */

	M_menu_extrasTime(editor) = sb.st_mtime;
	numCmds = 0;

	/*
	 * Have to read in the file.
	 * First, get rid of any data from last time we read it.
	 */

	if (extrasFile != NULL) {
		free(extrasFile);
		M_menu_extrasFileContents(editor) = NULL;
	}
	if (extrasCmds != NULL)
		free(extrasCmds);

	/*
	 * Allocate space for the file contents and read it all in.
	 */
	extrasCmds = NULL;
	extrasFile = malloc(sb.st_size + 1);
	if (extrasFile == NULL)
		return;
	if ((fd = open(M_menu_realExtrasFilename(editor), O_RDONLY)) < 0) {
		free(extrasFile);
		return;
	}
	len = read(fd, extrasFile, sb.st_size);
	close(fd);
	if (len <= 0) {
		free(extrasFile);
		return;
	}
	extrasFile[len] = '\0';
	M_menu_extrasFileContents(editor) = extrasFile;

	/*
	 * Step through the file data a line at a time
	 * breaking up each line into arguments.
	 */
	for (np = p = extrasFile; *np != '\0'; p = np) {
		if ((np = strchr(p, '\n')) != NULL)
			*np++ = '\0';
		else
			np = strchr(p, '\0');
		if (*p == '#')
			continue;
		while (*p != '\0' && isspace(*p))
			p++;
		if (*p == '\0')
			continue;
		numCmds++;
		ep = realloc(extrasCmds, numCmds * sizeof (*extrasCmds));
		if (ep == NULL) {
			numCmds--;
			break;
		}
		extrasCmds = ep;
		ap = &extrasCmds[numCmds-1].args[0];
		while (*p != '\0') {
			if (*p == '"' || *p == '\'') {
				quote = *p;
				*ap++ = ++p;
				while (*p != '\0' && *p != quote) {
					if (*p =='\\')
						p++;
					p++;
				}
			} else {
				*ap++ = p;
				while (*p != '\0' && !isspace(*p)) {
					if (*p =='\\')
						p++;
					p++;
				}
			}
			if (*p != '\0')
				*p++ = '\0';
			while (*p != '\0' && isspace(*p))
				p++;
			if (ap >= &extrasCmds[numCmds-1].args[MAXARGS])
				break;
		}

		/*
		 * If we got enough fields, NULL terminate the list.
		 * Otherwise, forget about this command.
		 */
		if (ap >= &extrasCmds[numCmds-1].args[2])
			*ap = NULL;
		else
			numCmds--;
	}
}

static int
BuildMenu(DtEditorWidget editor, Widget menu, int i, Widget *WidgList)
{
	Widget btn, submenu;
	XmString tmpStr;
	Arg al[2];
	int wi = 0;

	for (; i < numCmds; i++) {
		if (strcmp(extrasCmds[i].args[1], "END") == 0)
			break;

		tmpStr = XmStringCreateLtoR(extrasCmds[i].args[0],
						XmFONTLIST_DEFAULT_TAG);
		XtSetArg(al[0], XmNlabelString, tmpStr);
		if (strcmp(extrasCmds[i].args[1], "MENU") == 0) {
			submenu = XmCreatePulldownMenu(menu,
			    "_extras_pulldown", NULL, 0);
			XtSetArg(al[1], XmNsubMenuId, submenu);
			btn = XmCreateCascadeButtonGadget(menu, "extras",
			    al, 2);
			i = BuildMenu(editor, submenu, ++i, &WidgList[wi]);
			XtManageChild(btn);
		} else {
			XtSetArg(al[1], XmNuserData, editor);
			btn = XmCreatePushButtonGadget(menu,
			    extrasCmds[i].args[0], al, 2);
			XmStringFree(tmpStr);
			XtAddCallback(btn, XmNactivateCallback, ExtrasItemCB,
			    (XtPointer)i);
			WidgList[wi++] = btn;
		}
	}
	XtManageChildren(WidgList, wi);
	return (i);
}

/************************************************************************
 * ExtrasCascadingCB - Called when bringing up [Extras] menu.
 *
 *	Reread the menu file and reconstruct the menu, if necessary.
 *
 *	"w" is the Extras XmCascadeButtonGadget
 ************************************************************************/
static void
ExtrasCascadingCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data)
{
	DtEditorWidget editor = (DtEditorWidget)client_data;
	Widget menu = NULL, btn;
	Widget *WidgList;
	time_t menuTime = 0;
	Arg al[2];
	int i;

	XtSetArg(al[0], XmNuserData, &menuTime);
	XtGetValues(w, al, 1);
	_DtEditorLoadExtras(editor);
	if (M_menu_extrasTime(editor) == menuTime)
		return;
	menuTime = M_menu_extrasTime(editor);
	XtSetArg(al[0], XmNuserData, menuTime);
	XtSetValues(w, al, 1);

	XtSetArg(al[0], XmNsubMenuId, &menu);
	XtGetValues(w, al, 1);
	if (menu != NULL)
		XtDestroyWidget(menu);
	menu = XmCreatePulldownMenu(XtParent(w),
	    "_extras_pulldown", NULL, 0);

	WidgList = malloc(numCmds * sizeof (Widget));
	if (WidgList == NULL)
		return;		/* XXX */
	BuildMenu(editor, menu, 0, WidgList);
	free(WidgList);

	XtSetArg(al[0], XmNsubMenuId, menu);
	XtSetValues(w, al, 1);
}

/*
 * Callback routine when a filter command button is selected.
 * The commands are stored in the global extrasCmds.  client_data
 * tells which command to execute.  XmNuserData on the button
 * refers to the DtEditor widget.
 */
static void
ExtrasItemCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data)
{
	Arg al[1];
	DtEditorWidget editor = NULL;
	DtEditorErrorCode error;
	XmAnyCallbackStruct *cb = (XmAnyCallbackStruct *) call_data;

	XtSetArg(al[0], XmNuserData, &editor);
	XtGetValues(w, al, 1);
	error = _DtEditorFilterSelection(editor,
		&extrasCmds[(int)client_data].args[1]);
	if (error != DtEDITOR_NO_ERRORS)
		XtCallActionProc(M_text(editor), "beep", cb->event, NULL, 0);
}


/*
 * Expand any shell metacharacters in name.
 * XXX - should use some XPG4 function, like wordexp.
 */
static char *
expand(char *name)
{
	static char buf[PATH_MAX + 1];
	char *p;
	FILE *fp;

	if (strpbrk(name, "~{[*?$`'\"\\") == NULL)
		return (name);
	strcpy(buf, "echo ");
	if (name[0] == '~') {
		struct passwd *pw;
		char user[PATH_MAX + 1];
		char *cp;

		p = &name[1];
		if (*p == '/') {
			pw = getpwuid(getuid());
		} else {
			cp = user;
			while (*p != '\0' && *p != '/')
				*cp++ = *p++;
			*cp = '\0';
			pw = getpwnam(user);
		}
		if (pw == NULL)
			return (name);
		strcat(buf, pw->pw_dir);
		strcat(buf, p);
	} else
		strcat(buf, name);
	fp = popen(buf, "r");
	if (fp == NULL)
		return (name);
	if (fgets(buf, sizeof (buf), fp) == NULL) {
		pclose(fp);
		return (name);
	}
	pclose(fp);
	if ((p = strchr(buf, '\n')) != NULL)
		*p = '\0';
	return (buf);
}
