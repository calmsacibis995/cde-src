/**********************************<+>*************************************
***************************************************************************
**
**  File:        Editor.c
**
**  Project:     Text Editor widget
**
**  Description:
**  -----------
**  This is the main source file for the Text Editor widget.
**
*******************************************************************
* (c) Copyright 1993, 1994 Hewlett-Packard Company
* (c) Copyright 1993, 1994 International Business Machines Corp.
* (c) Copyright 1993, 1994 Sun Microsystems, Inc.
* (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of Novell, Inc.
********************************************************************
**
**
**************************************************************************
**********************************<+>*************************************/

/*-------------------------------------------------------------
**      Include Files
*/

#include <ctype.h>

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

#include <sys/wait.h>
#include "signal.h"

#include <string.h>

#include <limits.h>	/* For LINE_MAX definition */
#include <sys/errno.h>	/* For Error handling */

#include "EditorP.h"
#include <Dt/Dnd.h>
#include "X11/Xutil.h"
#include <X11/StringDefs.h>
#include <X11/keysymdef.h>
#include <Xm/Form.h>
#include <Xm/MessageB.h>
#include <Xm/MwmUtil.h>
#include <Xm/TextF.h>

/* Need the following for _DtOkString */
#include <Dt/DtP.h>

#include <Dt/DtMsgsP.h>
#include "DtStrDefs.h"

#include <Dt/Action.h>

/* Sun highlight callbacks */
/* NOTE: This should be defined in Xm, but is defined here to allow
   building against an older Xm library while working with new highlight
   action enabled Xm library.  Be sure to match changes here with Xm */
#if (!defined(XmCHighlightColorname1) && !defined(XmCHighlightColorname2))
#define XmCHighlightColorname1 "HighlightColorname1"
#define XmNhighlightColorname1 "highlightColorname1"
#define XmCHighlightColorname2 "HighlightColorname2"
#define XmNhighlightColorname2 "highlightColorname2"
#endif
#ifndef XmHIGHLIGHT_COLOR_1
#define XmHIGHLIGHT_COLOR_1 (XmHIGHLIGHT_SECONDARY_SELECTED + 1)
#endif
#ifndef XmHIGHLIGHT_COLOR_2
#define XmHIGHLIGHT_COLOR_2 (XmHIGHLIGHT_SECONDARY_SELECTED + 2)
#endif
/* end Sun highlight callbacks */

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

static void	Initialize(
                        Widget rw,
                        Widget nw,
                        ArgList al,
                        Cardinal *num_args) ;

static void	VariableInitialize( 
			DtEditorWidget new);

static void	ValidateResources(
        		DtEditorWidget new,
        		DtEditorWidget request);

static Widget 	CreateText(
        		DtEditorWidget parent);

static void	extractFontMetrics( 
			XmFontList fontlist,
			int	*height,
			int	*width);

static void	getFontMetrics( 
			DtEditorWidget w);

static void	Destroy(
                        Widget widget);

static void	SetStatusLine(
			DtEditorWidget	ew,
			Boolean	statusLineOn);

static Widget 	CreateStatusLine(
        		DtEditorWidget parent);

static Boolean	SetValues(
                        Widget cw,
                        Widget rw,
                        Widget nw);

static void	_DtEditorGetCenterToggleLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	_DtEditorGetChangeAllButtonLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	_DtEditorGetChangeButtonLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	_DtEditorGetChangeFieldLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	_DtEditorGetColumns(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	_DtEditorGetCurrentLineLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	_DtEditorGetCursorPosition(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	_DtEditorGetFindButtonLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	_DtEditorGetFindFieldLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	_DtEditorGetFormatAllButtonLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	_DtEditorGetFormatParagraphButtonLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	_DtEditorGetJustifyToggleLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	_DtEditorGetLeftAlignToggleLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	_DtEditorGetLeftMarginFieldLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	_DtEditorGetMaxLength(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	_DtEditorGetMisspelledListLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	_DtEditorGetRightAlignToggleLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	_DtEditorGetRightMarginFieldLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	_DtEditorGetRows(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	_DtEditorGetScrollLeftSide(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	_DtEditorGetScrollTopSide(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	 _DtEditorGetTextBackground(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	 _DtEditorGetTextForeground(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	 _DtEditorGetTopCharacter(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	 _DtEditorGetLineCountLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value );

static void	FixWordWrap(		   /* XXX Word Wrap workaround */
	Widget		w,		   /* XXX Word Wrap workaround */
	Boolean		wrapOn);	   /* XXX Word Wrap workaround */

static void	BackwardChar(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	BackwardField(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	BackwardPara(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	BackwardWord(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	BeginningOfFile(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	BeginningOfLine(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void 	CallWebBrowser(
        Widget 		w,
        XtPointer 	udata,
        XtPointer 	cbdata);

static void	ClearSelection(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	CopyClipboard(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	CutClipboard(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	DeleteNextChar(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	DeleteNextWord(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	DeletePrevChar(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	DeletePrevWord(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	DeleteToEOL(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	DeleteToSOL(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	DeselectAll(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	EndOfFile(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	EndOfLine(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	Filter(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	FindSelection(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	FindString(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	ForwardChar(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	ForwardField(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	ForwardPara(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	ForwardWord(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	GoToLine(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	GoToLine_I(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	Help(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	InsertFile(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	InsertString(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	KeySelect(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	NewlineAndBackup(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	NewlineAndIndent(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	NextPage(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	PageLeft(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	PageRight(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void 	ParseUrlsCB(
        Widget 		w,
        caddr_t 	client_data,
        caddr_t 	call_data);

static void	PasteClipboard(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	PopupMenu(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	PreviousPage(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	ProcessCancel(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	ProcessDown(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	ProcessShiftDown(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	ProcessShiftUp(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	ProcessUp(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	QuoteNextChar(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	QuoteNextChar_I(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	SelectAll(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	ToggleInsertMode(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	ToggleInsertMode_I(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	UndoEdit(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	UndoEdit_I(
	Widget		w,
	XEvent		*event,
	char		**params,
	Cardinal	*num_params);

static void	Call_TextSelectCallback(
	DtEditorWidget	editor);

static void	Call_TextDeselectCallback(
	DtEditorWidget	editor);

static void	Editor_SetSelectionProc(
        XmTextSource	source,
        XmTextPosition	left,
        XmTextPosition	right,
        Time		set_time );

static void	CallHelpCallback(
	DtEditorWidget  editor,
	int             reason);

static void	HelpEditWindowCB(
        Widget		w,
        caddr_t		client_data,
        caddr_t		call_data );

static void	HelpStatusCurrentLineCB(
        Widget		w,
        caddr_t		client_data,
        caddr_t		call_data );

static void	HelpStatusTotalLinesCB(
        Widget		w,
        caddr_t		client_data,
        caddr_t		call_data );

static void	HelpStatusMessageCB(
        Widget		w,
        caddr_t		client_data,
        caddr_t		call_data );

static void	HelpStatusOverstrikeCB(
        Widget		w,
        caddr_t		client_data,
        caddr_t		call_data );

static void	RegisterDropZone( 
			DtEditorWidget w );

static void	UnregisterDropZone( 
			DtEditorWidget w );

static void	SetInfoDialogTitle(
        		DtEditorWidget editor );

static int	FormatText ( 
			AdjRecPtr pAdjRec );

static void	AdjustParaCB(
			Widget w,
			caddr_t client_data,
			caddr_t call_data );

static void	AdjustAllCB(
        		Widget w,
        		caddr_t client_data,
        		caddr_t call_data );

static DtEditorErrorCode DoAdjust(
        		DtEditorWidget	editor,
			int		leftMargin,
			int		rightMargin,
			unsigned int	alignment,
        		XmTextPosition	start,
        		XmTextPosition	end);

static void	SetFormatDialogTitle(
        		DtEditorWidget editor);

static void	ResetFormatDialog(
			DtEditorWidget editor);

static void	CreateFormatDialog(
			DtEditorWidget editor);

static void	GetAdjustSettings(
			DtEditorWidget pPad,
			DtEditorFormatSettings	*formatSettings);

static void	UpdateOverstrikeIndicator( 
			DtEditorWidget	widget,
			Boolean		overstrikeOn );

static void	SelectNextField(
			Widget		w,
			XEvent		*event,
			XmTextDirection	direction);

/********    End Static Function Declarations    ********/


/****************************************************************
 *
 *      Translations and Actions
 *
 ****************************************************************/

/* 
 * The following are the translations which DtEditor places (overrides) 
 * on the scrolled text widget.  If the DtNtextTranslations resource is
 * set, it will also be added (in override mode) to the text widget.
 */
static char	EditorTranslationTable[] = "\
	~s ~c ~m ~a <Key>Return: newline-and-indent()\n\
	~s m<Key>osfBackSpace:	I-undo-edit()\n\
	~s c<Key>osfBackSpace:	delete-previous-word()\n\
	s<Key>osfBackSpace:	delete-to-start-of-line()\n\
	~s ~c <Key>osfInsert:	I-toggle-insert-mode()\n\
	<Key>osfUndo:		I-undo-edit()\n\
	c<Key>g:		I-go-to-line()\n\
	c<Key>q:		I-quote-next-character()\n\
	c<Key>z:		I-undo-edit()\n\
	<Btn3Down>:		popup-menu()\n\
	<Key>osfUndo:		I-undo-edit()\n\
	<Key>osfCancel:		insert-file()\n\
	m c ~s<Key>Tab:		forward-field()\n\
	m c s<Key>Tab:		backward-field()\n\
	c <Key>SunFind:		find-string()\n\
	~c ~s <Key>SunFind:	find-selection(forward)\n\
	~c s <Key>SunFind:	find-selection(backward)";


/*
 * The following are DtEditor's actions.  A few are internal only (_I
 * suffix) and will be called by the default translations DtEditor places
 * on the text widget (see previous comment).  The rest will only be called
 * from an application with XtCallActionProc().  The main difference is
 * the internal ones will be passed a text widget ID, while the public
 * ones will be passed a DtEditor ID.
 */
static XtActionsRec EditorActionTable[] = {
	{"I-go-to-line",		(XtActionProc)GoToLine_I},
	{"I-toggle-insert-mode",	(XtActionProc)ToggleInsertMode_I},
	{"I-undo-edit",			(XtActionProc)UndoEdit_I},
	{"I-quote-next-character",	(XtActionProc)QuoteNextChar_I},
	{"backward-character",		(XtActionProc)BackwardChar},
	{"backward-field",		(XtActionProc)BackwardField},
	{"backward-paragraph",		(XtActionProc)BackwardPara},
	{"backward-word",		(XtActionProc)BackwardWord},
	{"beginning-of-file",		(XtActionProc)BeginningOfFile},
	{"beginning-of-line",		(XtActionProc)BeginningOfLine},
	{"clear-selection",		(XtActionProc)ClearSelection},
	{"copy-clipboard",		(XtActionProc)CopyClipboard},
	{"cut-clipboard",		(XtActionProc)CutClipboard},
	{"delete-next-character",	(XtActionProc)DeleteNextChar},
	{"delete-next-word",		(XtActionProc)DeleteNextWord},
	{"delete-previous-character",	(XtActionProc)DeletePrevChar},
	{"delete-previous-word",	(XtActionProc)DeletePrevWord},
	{"delete-to-end-of-line",	(XtActionProc)DeleteToEOL},
	{"delete-to-start-of-line",	(XtActionProc)DeleteToSOL},
	{"deselect-all",		(XtActionProc)DeselectAll},
	{"end-of-file",			(XtActionProc)EndOfFile},
	{"end-of-line",			(XtActionProc)EndOfLine},
	{"filter",			(XtActionProc)Filter},
	{"find-selection",		(XtActionProc)FindSelection},
	{"find-string",			(XtActionProc)FindString},
	{"forward-character",		(XtActionProc)ForwardChar},
	{"forward-field",		(XtActionProc)ForwardField},
	{"forward-paragraph",		(XtActionProc)ForwardPara}, 
	{"forward-word",		(XtActionProc)ForwardWord},
	{"go-to-line",			(XtActionProc)GoToLine},
	{"Help",			(XtActionProc)Help},
	{"insert-file",			(XtActionProc)InsertFile},
	{"insert-string",		(XtActionProc)InsertString},
	{"key-select",			(XtActionProc)KeySelect},
	{"newline-and-backup",		(XtActionProc)NewlineAndBackup},
	{"newline-and-indent",		(XtActionProc)NewlineAndIndent},
	{"next-page",			(XtActionProc)NextPage},
	{"page-left",			(XtActionProc)PageLeft},
	{"page-right",			(XtActionProc)PageRight},
	{"paste-clipboard",		(XtActionProc)PasteClipboard},
	{"popup-menu",			(XtActionProc)PopupMenu},
	{"previous-page",		(XtActionProc)PreviousPage},
	{"process-cancel",		(XtActionProc)ProcessCancel},
	{"process-down",		(XtActionProc)ProcessDown},
	{"process-shift-down",		(XtActionProc)ProcessShiftDown},
	{"process-shift-up",		(XtActionProc)ProcessShiftUp},
	{"process-up",			(XtActionProc)ProcessUp},
	{"quote-next-character",	(XtActionProc)QuoteNextChar},
	{"select-all",			(XtActionProc)SelectAll},
	{"toggle-insert-mode",		(XtActionProc)ToggleInsertMode},
	{"undo-edit",			(XtActionProc)UndoEdit},
};


/****************************************************************
 *
 *	Defines
 *
 ****************************************************************/

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

#define	MAXTABSIZE 100			/* max legal tabsize	*/


/****************************************************************
 *
 *  Define misc data structures
 *
 ****************************************************************/


/****************************************************************
 *
 *  Resource List
 *
 ****************************************************************/

static XmSyntheticResource syn_resources[] =
{

        {
                DtNcolumns, sizeof(short),
                XtOffset (DtEditorWidget, editor.editStuff.columns),
                _DtEditorGetColumns, NULL
        },
        {
                DtNcenterToggleLabel, sizeof (XmString),
                XtOffset (DtEditorWidget, editor.formatStuff.centerToggleLabel),
                _DtEditorGetCenterToggleLabel, NULL
        },
        {
                DtNchangeAllButtonLabel, sizeof (XmString),
                XtOffset (DtEditorWidget,
			  editor.searchStuff.changeAllButtonLabel),
                _DtEditorGetChangeAllButtonLabel, NULL
        },
        {
                DtNchangeButtonLabel, sizeof (XmString),
                XtOffset(DtEditorWidget, editor.searchStuff.changeButtonLabel),
		_DtEditorGetChangeButtonLabel, NULL
        },
        {
                DtNchangeFieldLabel, sizeof (XmString),
                XtOffset(DtEditorWidget, editor.searchStuff.changeFieldLabel),
		_DtEditorGetChangeFieldLabel, NULL
        },
        {
                DtNcurrentLineLabel, sizeof (XmString),
                XtOffset(DtEditorWidget, editor.statusStuff.currentLineLabel),
		_DtEditorGetCurrentLineLabel, NULL
        },
        {
		DtNcursorPosition, sizeof (XmTextPosition),
		XtOffset(DtEditorWidget, editor.editStuff.cursorPos),
                _DtEditorGetCursorPosition, NULL
        },
        {
                DtNfindButtonLabel, sizeof (XmString),
                XtOffset(DtEditorWidget, editor.searchStuff.findButtonLabel),
		_DtEditorGetFindButtonLabel, NULL
        },
        {
                DtNfindFieldLabel, sizeof (XmString),
                XtOffset(DtEditorWidget, editor.searchStuff.findFieldLabel),
		_DtEditorGetFindFieldLabel, NULL
        },
        {
                DtNformatAllButtonLabel, sizeof (XmString),
                XtOffset(DtEditorWidget, 
			 editor.formatStuff.formatAllButtonLabel),
		_DtEditorGetFormatAllButtonLabel, NULL
        },
        {
                DtNformatParagraphButtonLabel, sizeof (XmString),
                XtOffset(DtEditorWidget,
			 editor.formatStuff.formatParaButtonLabel),
		_DtEditorGetFormatParagraphButtonLabel, NULL
        },
        {
                DtNjustifyToggleLabel, sizeof (XmString),
                XtOffset (DtEditorWidget, 
			  editor.formatStuff.justifyToggleLabel),
                _DtEditorGetJustifyToggleLabel, NULL
        },
        {
                DtNleftAlignToggleLabel, sizeof (XmString),
                XtOffset (DtEditorWidget,  
			  editor.formatStuff.leftAlignToggleLabel),
                _DtEditorGetLeftAlignToggleLabel, NULL
        },
        {
                DtNleftMarginFieldLabel, sizeof (XmString),
                XtOffset (DtEditorWidget,  
			  editor.formatStuff.leftMarginFieldLabel),
                _DtEditorGetLeftMarginFieldLabel, NULL
        },
        {
		DtNmaxLength, sizeof(int),
		XtOffset(DtEditorWidget, editor.editStuff.maxLength),
                _DtEditorGetMaxLength, NULL
        },
        {
                DtNmisspelledListLabel, sizeof (XmString),
                XtOffset (DtEditorWidget,  
			  editor.searchStuff.misspelledListLabel),
                _DtEditorGetMisspelledListLabel, NULL
        },
        {
                DtNrightAlignToggleLabel, sizeof (XmString),
                XtOffset (DtEditorWidget,  
			  editor.formatStuff.rightAlignToggleLabel),
                _DtEditorGetRightAlignToggleLabel, NULL
        },
        {
                DtNrightMarginFieldLabel, sizeof (XmString),
                XtOffset (DtEditorWidget,  
			  editor.formatStuff.rightMarginFieldLabel),
                _DtEditorGetRightMarginFieldLabel, NULL
        },
        {
                DtNrows, sizeof(short),
                XtOffset (DtEditorWidget, editor.editStuff.rows),
                _DtEditorGetRows, NULL
        },
        {
                DtNscrollLeftSide, sizeof (Boolean),
                XtOffset (DtEditorWidget, editor.editStuff.scrollLeft),
                _DtEditorGetScrollLeftSide, NULL
        },
        {
                DtNscrollTopSide, sizeof (Boolean),
                XtOffset (DtEditorWidget, editor.editStuff.scrollTop),
                _DtEditorGetScrollTopSide, NULL
        },
        {
                DtNtextBackground, sizeof(Pixel),
                XtOffset (DtEditorWidget, editor.editStuff.background),
                _DtEditorGetTextBackground, NULL
        },
        {
                DtNtextForeground, sizeof(Pixel),
                XtOffset (DtEditorWidget, editor.editStuff.foreground),
                _DtEditorGetTextForeground, NULL
        },
        {
                DtNtopCharacter, sizeof(XmTextPosition),
                XtOffset (DtEditorWidget, editor.editStuff.topCharacter),
                _DtEditorGetTopCharacter, NULL
        },
        {
                DtNtotalLineCountLabel, sizeof (XmString),
                XtOffset (DtEditorWidget, editor.statusStuff.totalLineLabel),
                _DtEditorGetLineCountLabel, NULL
        },
};

static XtResource resources[] =
{
        {
                DtNautoShowCursorPosition,
                DtCAutoShowCursorPosition, XmRBoolean, sizeof (Boolean),
                XtOffset(DtEditorWidget, 
			 editor.editStuff.autoShowCursorPos),
                XmRImmediate, (XtPointer) True
        },
        {
                DtNbuttonFontList, DtCFontList, XmRFontList, sizeof(XmFontList),
                XtOffsetOf(struct _XmBulletinBoardRec, 
			   bulletin_board.button_font_list),
                XmRFontList, (XtPointer) NULL
        },
        {
		DtNblinkRate, DtCBlinkRate, XmRInt, sizeof(int),
		XtOffset(DtEditorWidget, editor.editStuff.blinkRate),
                XmRImmediate, (XtPointer) 500
        },
        {
                DtNcenterToggleLabel, DtCCenterToggleLabel, 
		XmRXmString, sizeof (XmString),
                XtOffset (DtEditorWidget, editor.formatStuff.centerToggleLabel),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNchangeAllButtonLabel, DtCChangeAllButtonLabel, 
		XmRXmString, sizeof (XmString),
                XtOffset (DtEditorWidget,
			  editor.searchStuff.changeAllButtonLabel),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNchangeButtonLabel, DtCChangeButtonLabel, 
		XmRXmString, sizeof (XmString),
                XtOffset(DtEditorWidget, editor.searchStuff.changeButtonLabel),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNchangeFieldLabel, DtCChangeFieldLabel, 
		XmRXmString, sizeof (XmString),
                XtOffset(DtEditorWidget, editor.searchStuff.changeFieldLabel),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNcolumns, DtCColumns, XmRShort, sizeof(short),
                XtOffset (DtEditorWidget, editor.editStuff.columns),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNcurrentLineLabel, DtCCurrentLineLabel, 
		XmRXmString, sizeof (XmString),
                XtOffset(DtEditorWidget, editor.statusStuff.currentLineLabel),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
		DtNcursorPosition, DtCCursorPosition, XmRTextPosition,
		sizeof (XmTextPosition),
		XtOffset(DtEditorWidget, editor.editStuff.cursorPos),
		XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNcursorPositionVisible, DtCCursorPositionVisible, 
		XmRBoolean, sizeof (Boolean), 
		XtOffset(DtEditorWidget, 
			 editor.editStuff.cursorPosVisible),
                XmRImmediate, (XtPointer) True
        },
        {
                DtNdialogTitle, DtCDialogTitle, XmRXmString, sizeof (XmString),
                XtOffsetOf(struct _XmBulletinBoardRec, 
			   bulletin_board.dialog_title),
                XmRString, (XtPointer) NULL
        },
        {
                DtNeditable, DtCEditable, XmRBoolean, sizeof (Boolean),
                XtOffset (DtEditorWidget, editor.editStuff.editable),
                XmRImmediate, (XtPointer) True
        },
        {
                DtNfindButtonLabel, DtCFindButtonLabel, 
		XmRXmString, sizeof (XmString),
                XtOffset(DtEditorWidget, editor.searchStuff.findButtonLabel),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNfindChangeDialogTitle, DtCFindChangeDialogTitle, 
		XmRXmString, sizeof (XmString),
                XtOffset(DtEditorWidget, editor.searchStuff.fndChngTitle),
                XmRString, (XtPointer) NULL
        },
        {
                DtNfindFieldLabel, DtCFindFieldLabel, 
		XmRXmString, sizeof (XmString),
                XtOffset(DtEditorWidget, editor.searchStuff.findFieldLabel),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNformatAllButtonLabel, DtCFormatAllButtonLabel, 
		XmRXmString, sizeof (XmString),
                XtOffset(DtEditorWidget, 
			 editor.formatStuff.formatAllButtonLabel),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNformatParagraphButtonLabel, DtCFormatParagraphButtonLabel, 
		XmRXmString, sizeof (XmString),
                XtOffset(DtEditorWidget, 
			 editor.formatStuff.formatParaButtonLabel),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNformatSettingsDialogTitle, DtCFormatSettingsDialogTitle, 
		XmRXmString, sizeof (XmString),
                XtOffset(DtEditorWidget, editor.formatStuff.formatDialogTitle),
                XmRString, (XtPointer) NULL
        },
        {
                DtNinformationDialogTitle, DtCInformationDialogTitle, 
		XmRXmString, sizeof (XmString),
                XtOffset(DtEditorWidget, editor.warningStuff.infoDialogTitle),
                XmRString, (XtPointer) NULL
        },
        {
                DtNinsertLabel, DtCInsertLabel, XmRXmString, sizeof (XmString),
                XtOffset (DtEditorWidget, editor.statusStuff.ins),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNjustifyToggleLabel, DtCJustifyToggleLabel, 
		XmRXmString, sizeof (XmString),
                XtOffset(DtEditorWidget, editor.formatStuff.justifyToggleLabel),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNlabelFontList, DtCFontList, XmRFontList, sizeof(XmFontList),
                XtOffsetOf(struct _XmBulletinBoardRec, 
			   bulletin_board.label_font_list),
                XmRFontList, (XtPointer) NULL
        },
        {
                DtNleftAlignToggleLabel, DtCLeftAlignToggleLabel, 
		XmRXmString, sizeof (XmString),
                XtOffset(DtEditorWidget, 
			 editor.formatStuff.leftAlignToggleLabel),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNleftMarginFieldLabel, DtCLeftMarginFieldLabel, 
		XmRXmString, sizeof (XmString),
                XtOffset(DtEditorWidget, 
			 editor.formatStuff.leftMarginFieldLabel),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
		DtNmaxLength, DtCMaxLength, XmRInt, sizeof(int),
		XtOffset(DtEditorWidget, editor.editStuff.maxLength),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNmisspelledListLabel, DtCMisspelledListLabel, 
		XmRXmString, sizeof (XmString),
                XtOffset (DtEditorWidget,  
			  editor.searchStuff.misspelledListLabel),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
  	{
                DtNoverstrike, DtCOverstrike, XmRBoolean, sizeof (Boolean),
                XtOffset(DtEditorWidget, editor.editStuff.overstrikeMode),
                XmRImmediate, (XtPointer) False
        },
        {
                DtNoverstrikeLabel, DtCOverstrikeLabel, 
		XmRXmString, sizeof (XmString),
                XtOffset (DtEditorWidget, editor.statusStuff.ovr),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNrightAlignToggleLabel, DtCRightAlignToggleLabel, 
		XmRXmString, sizeof (XmString),
                XtOffset(DtEditorWidget, 
			 editor.formatStuff.rightAlignToggleLabel),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNrightMarginFieldLabel, DtCRightMarginFieldLabel, 
		XmRXmString, sizeof (XmString),
                XtOffset(DtEditorWidget, 
			 editor.formatStuff.rightMarginFieldLabel),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNrows, DtCRows, XmRShort, sizeof(short),
                XtOffset (DtEditorWidget, editor.editStuff.rows),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNscrollHorizontal, DtCScroll, XmRBoolean, sizeof (Boolean),
                XtOffset (DtEditorWidget, editor.editStuff.scrollHorizontal),
                XmRImmediate, (XtPointer) True
        },
        {
                DtNscrollLeftSide, DtCScrollSide, XmRBoolean, sizeof (Boolean),
                XtOffset (DtEditorWidget, editor.editStuff.scrollLeft),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNscrollTopSide, DtCScrollSide, XmRBoolean, sizeof (Boolean),
                XtOffset (DtEditorWidget, editor.editStuff.scrollTop),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNscrollVertical, DtCScroll, XmRBoolean, sizeof (Boolean),
                XtOffset (DtEditorWidget, editor.editStuff.scrollVertical),
                XmRImmediate, (XtPointer) True
        },
  	{
                DtNshowStatusLine,
                DtCShowStatusLine, XmRBoolean, sizeof (Boolean),
                XtOffset (DtEditorWidget, editor.statusStuff.showStatusLine),
                XmRImmediate, (XtPointer) False
        },
        {
                DtNspellDialogTitle, DtCSpellDialogTitle, 
		XmRXmString, sizeof (XmString),
                XtOffset(DtEditorWidget, editor.searchStuff.spellTitle),
                XmRString, (XtPointer) NULL
        },
  	{
                DtNspellFilter, DtCSpellFilter, XmRString, sizeof(XmRString),
                XtOffset (DtEditorWidget, editor.searchStuff.spellFilter),
                XmRString, "spell"
        },
        {
                DtNtextBackground, DtCBackground, 
		XmRPixel, sizeof(Pixel),
                XtOffset (DtEditorWidget, editor.editStuff.background),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNtextDeselectCallback,
                DtCCallback, XtRCallback, sizeof (XtCallbackList),
                XtOffset (DtEditorWidget, editor.textDeselect),
                XtRCallback, (XtPointer) NULL
        },
        {
                DtNtextForeground, DtCForeground, 
		XmRPixel, sizeof(Pixel),
                XtOffset (DtEditorWidget, editor.editStuff.foreground),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNtextFontList, DtCFontList, XmRFontList, sizeof(XmFontList),
                XtOffsetOf(struct _XmBulletinBoardRec, 
			   bulletin_board.text_font_list),
                XmRFontList, (XtPointer) NULL
        },
        {
                DtNtextSelectCallback,
                DtCCallback, XtRCallback, sizeof (XtCallbackList),
                XtOffset (DtEditorWidget, editor.textSelect),
                XtRCallback, (XtPointer) NULL
        },
        {
		DtNtextTranslations, DtCTranslations,
		XmRTranslationTable, sizeof (XtTranslations),
		XtOffsetOf(struct _XmBulletinBoardRec, 
			   bulletin_board.text_translations),
		XmRImmediate, (XtPointer) NULL
        },
        {
                DtNtopCharacter, DtCTopCharacter, 
		XmRTextPosition, sizeof(XmTextPosition),
                XtOffset (DtEditorWidget, editor.editStuff.topCharacter),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNtotalLineCountLabel, DtCTotalLineCountLabel, 
		XmRXmString, sizeof (XmString),
                XtOffset (DtEditorWidget, 
			  editor.statusStuff.totalLineLabel),
                XmRImmediate, (XtPointer) DtUNSPECIFIED
        },
        {
                DtNwordWrap,
                DtCWordWrap, XmRBoolean, sizeof (Boolean),
                XtOffset (DtEditorWidget, editor.editStuff.wordWrap),
                XmRImmediate, (XtPointer) False
        },
        {
                SDtNenableUrlAwareness, SDtCEnableUrlAwareness, 
		XmRBoolean, sizeof (Boolean),
                XtOffset(DtEditorWidget, editor.editStuff.enableUrlAwareness),
                XmRImmediate, (XtPointer) False
        },
        {
                SDtNextrasMenuFilename, SDtCExtrasMenuFilename, 
		XmRXmString, sizeof (XmString),
                XtOffset(DtEditorWidget, editor.menuStuff.extrasMenuFilename),
                XmRImmediate, (XtPointer) NULL
        },
};

/****************************************************************
 *
 *  Public Function Declarations
 *
 ****************************************************************/

/****************************************************************
 *
 *  Class Record
 *
 ****************************************************************/

externaldef( dteditorclassrec ) DtEditorClassRec 
dtEditorClassRec =
{
/*	Core Part
*/
	{	
		(WidgetClass) &xmFormClassRec,	/* superclass	*/
		"DtEditor",			/* class_name		*/
 	  	sizeof (DtEditorRec),		/* widget_size		*/
	  	NULL,				/* class_initialize	*/
	 	NULL,				/* class_part_initialize*/
		False,				/* class_inited		*/
		(XtInitProc) Initialize,	/* initialize		*/
		NULL,				/* initialize_hook	*/
		XtInheritRealize,		/* realize		*/
		(XtActionList)EditorActionTable,/* actions		*/
		(Cardinal)XtNumber(EditorActionTable), /* num_actions	*/
		resources,			/* resources		*/
		XtNumber (resources),		/* num_resources	*/
		NULLQUARK,			/* xrm_class		*/
		True,				/* compress_motion	*/
		XtExposeCompressMaximal,	/* compress_exposure	*/
		False,				/* compress_enterleave	*/
		False,				/* visible_interest	*/	
		(XtWidgetProc) Destroy,		/* destroy		*/	
		XtInheritResize,		/* resize		*/
		XtInheritExpose,		/* expose		*/	
		(XtSetValuesFunc) SetValues,	/* set_values		*/	
		NULL,				/* set_values_hook	*/
		XtInheritSetValuesAlmost,	/* set_values_almost	*/
		NULL,		 		/* get_values_hook	*/
		XtInheritAcceptFocus,		/* accept_focus		*/	
		XtVersion,			/* version		*/
		NULL,				/* callback private	*/
		XtInheritTranslations,		/* tm_table		*/
		XtInheritQueryGeometry,		/* query_geometry	*/
		(XtStringProc)NULL,		/* display_accelerator	*/
		NULL,				/* extension		*/
	},

/*	Composite Part
*/
	{
		XtInheritGeometryManager,	/* geometry_manager	*/
		XtInheritChangeManaged,		/* change_managed	*/
		XtInheritInsertChild,		/* insert_child		*/
		XtInheritDeleteChild,		/* delete_child		*/
		NULL,				/* extension		*/
	},


/*	Constraint Part
*/
	{
		NULL,				/* constraint_resources	   */
		0,				/* num_constraint_resource */
      		sizeof(XmFormConstraintRec),	/* size of constraint      */
      		NULL,				/* initialization          */
		NULL,				/* constraint_destroy	   */
		NULL,				/* constraint_set_values   */
		NULL,				/* extension		   */
	},

/*	XmManager Part
*/
	{
		XmInheritTranslations,		/* default_translations	*/
		syn_resources,			/* syn_resources	*/
		XtNumber(syn_resources),	/* num_syn_resources	*/
		NULL,				/* syn_cont_resources	*/
		0,				/* num_syn_cont_resources */
		XmInheritParentProcess,		/* parent_process	*/
		NULL,				/* extension		*/
	},

/* XmbulletinBoard Part
*/
   {  
      FALSE,				/* always_install_accelerators */
      (XmGeoCreateProc)NULL,			/* geo_matrix_create  */
      XmInheritFocusMovedProc,			/* focus_moved_proc   */
      NULL,					/* extension          */
   },

/*	XmForm Part 
*/
	{
		(int) NULL,			/* extension		*/
	},

/*	DtEditor Part
*/
	{
		(int) NULL,			/* extension		*/
	}
};

WidgetClass dtEditorWidgetClass = (WidgetClass) &dtEditorClassRec;

/*-------------------------------------------------------------
**      Editor Procs
**-------------------------------------------------------------
*/

/****************************************************************
 *
 *  Private Procs
 *
 ****************************************************************/


/*-------------------------------------------------------------
**      Function: static void Initialize (
**                              Widget rw,
**                              Widget nw,
**			        ArgList arg_list,
**			        Cardinal *num_args)
**
**      Parameters:
**
**      Purpose:    This is the Editor widget initialize routine.
**                  It is responsible for the following:
**                      1) Validate all resources the user passed in,
**                      2) Override any invalid resources,
**                      3) Initialize the internal data structures,
**                      4) Create the edit area widget
**                      5) Create the status area widget, if requested
**                      6) Add any callbacks and actions
*/
static void
Initialize(
        Widget rw,
        Widget nw,
        ArgList arg_list,
        Cardinal *num_args)
{

  DtEditorWidget  request = (DtEditorWidget) rw,
  	  	  new     = (DtEditorWidget) nw;

  /* Initialize non-resource portion of the instance structure */
  VariableInitialize (new);

  /* Validate the incoming arguments to make sure they are OK */
  ValidateResources (new, request);

  /* 
   * Create & customize the scrolled text widget 
   */
  M_text(new) = CreateText (new);
  XtManageChild( M_text(new)) ;

  /*
   * If the widget is not "read only" then register it as a drop zone
   */
  if ( M_editable(new) == True )
     RegisterDropZone( new );

  /* 
   * Compute the width & height of the scrolled text's font.  These values 
   * will be used when formatting and in calculating the window manager 
   * hints.  
   */
  getFontMetrics(new);

  /*
   *  Create the status line
   */ 
  SetStatusLine( new, M_status_showStatusLine(request) );


} /* end Initialize */


/*-------------------------------------------------------------
**      Function: static void VariableInitialize (
**                              DtEditorWidget new )
**
**      Parameters: The Editor widget being created
**
**      Purpose:    This routine:
**                      1) Initializes the widget's instance structure
*/
static void
VariableInitialize(
        DtEditorWidget new)
{
  Widget		thisParent;

  /* 
   * Initialize the non-resource instance fields
   */

  M_display(new) = XtDisplayOfObject( (Widget)new->core.parent );
  M_app_context(new) = XtDisplayToApplicationContext(M_display(new));

  /*
   * Empty 'for' walks up the widget tree to find a shell.
   */
  for (thisParent = new->core.parent; 
       thisParent != (Widget)NULL && XtIsShell(thisParent) == False;
       thisParent = XtParent(thisParent));
  M_topLevelShell(new) = thisParent;

  /* Initialize edit area fields                                  */
  M_loadingAllNewData(new) = False;
  M_unreadChanges(new) = False;     /* There have not been any changes since */
                                    /* the app requested the data            */
  M_deletionStart(new) = NO_DELETION_IN_PROGRESS;
  M_deletedText(new) = (char *)NULL;
  M_insertStart(new) = 0;
  M_insertionLength(new) = 0;
  M_textSelectCbCalled(new) = False;
  M_fontWidth(new) = -1;
  

  /* Initialize status line fields                                 */
  M_status_statusArea(new) = (Widget)NULL;
  M_status_messageText(new) = (Widget)NULL;
  M_status_currentLine(new) = -1;
  M_status_lastLine(new) = -1;

  /* Initialize search function data                               */
  M_search_dialog(new) = (Widget) NULL;
  M_search_dialogMode(new) = SPELL;
  M_search_string(new) = (char *)NULL;
  M_replace_string(new) = (char *)NULL;
  M_misspelled_string(new) = (char *)NULL;
  M_misspelled_found(new) = False;

  /* Initialize format function data                               */
  M_format_dialog(new) = (Widget)NULL;

  /* Initialize menu data					   */
  M_menu(new) = NULL;
  M_menu_includeDialog(new) = NULL;
  M_menu_extrasTime(new) = 0;
  M_menu_extrasMenuFilename(new) = NULL;
  M_menu_realExtrasFilename(new) = NULL;
  M_menu_extrasFileContents(new) = NULL;

#if !(defined(sun) && (_XOPEN_VERSION==3))
  _DtEditor_blankClass = wctype(blankString);
#endif /* not sun */

  /* Initialize warning dialogs data                               */
  M_gen_warning(new) = (Widget)NULL;

} /* end VariableInitialize */


/*-------------------------------------------------------------
**      Function: static void ValidateResources (
**                              DtEditorWidget new,
**                              DtEditorWidget request )
**
**      Parameters: The Editor widget being created & its requested
**		    resource values
**
**      Purpose:    This routine:
**                      1) Validates the widget's requested resources
*/
static void
ValidateResources(
        DtEditorWidget new,
        DtEditorWidget request)
{
  /* 
   * Validate the requested values for the editor's resources
   */

  /* 
   * Make local copies of all resource strings assigned by the application. 
   */

  if (M_spellFilter(request) != (char *) NULL)
    M_spellFilter(new) = XtNewString( M_spellFilter(request) );

  /* 
   * Copy the dialog titles if the application set them, otherwise, 
   * get their values from the message catalog.
   */

  if (M_spellTitle(request) != (XmString) NULL)
    M_spellTitle(new) = XmStringCopy( M_spellTitle(request) );
  else
    M_spellTitle(new) = 
		XmStringCreateLtoR(SPELL_TITLE, XmFONTLIST_DEFAULT_TAG);

  if (M_fndChngTitle(request) != (XmString) NULL)
    M_fndChngTitle(new) = XmStringCopy( M_fndChngTitle(request) );
  else
    M_fndChngTitle(new) = 
		XmStringCreateLtoR(FIND_TITLE, XmFONTLIST_DEFAULT_TAG);

  if (E_format_dialogTitle(request) != (XmString) NULL)
    E_format_dialogTitle(new) = XmStringCopy( E_format_dialogTitle(request) );
  else
    E_format_dialogTitle(new) = 
		XmStringCreateLtoR(FORMAT_SETTINGS, XmFONTLIST_DEFAULT_TAG);

  if (E_infoDialogTitle(request) != (XmString) NULL)
    E_infoDialogTitle(new) = XmStringCopy( E_infoDialogTitle(request) );
  else
    E_infoDialogTitle(new) = 
		XmStringCreateLtoR(INFO_TITLE, XmFONTLIST_DEFAULT_TAG);

  /* 
   * Copy the insert & overstrike label indicators if the appli-
   * cation set them, otherwise, get their value from the message 
   * catalog.
   * Check for DtUNSPECIFIED because NULL is a valid value.
   */

  if (M_status_insertLabel(request) != (XmString) DtUNSPECIFIED)
    M_status_insertLabel(new) = 
			XmStringCopy( M_status_insertLabel(request) );
  else
    M_status_insertLabel(new) = 
		XmStringCreateLtoR(INS, XmFONTLIST_DEFAULT_TAG);

  if (M_status_overstrikeLabel(request) != (XmString) DtUNSPECIFIED)
    M_status_overstrikeLabel(new) = 
			XmStringCopy( M_status_overstrikeLabel(request) );
  else
    M_status_overstrikeLabel(new) = 
		XmStringCreateLtoR(OVR, XmFONTLIST_DEFAULT_TAG);

} /* end ValidateResources */


/*****************************************************************************
 * 
 * CallWebBrowser - Sun highlight action to launch a Web browser.
 * 
 * Returns - nothing.
 * 
 *****************************************************************************/
static void CallWebBrowser(
        Widget w,
        XtPointer udata,
        XtPointer cbdata1)
{
    XmTextWidget tw = (XmTextWidget) w;
    XmTextVerifyCallbackStruct cbdata = *((XmTextVerifyCallbackStruct*)cbdata1);
    char url[LINE_MAX];
    DtActionArg *actionArg;
    static int initdone = 0;

    if ((cbdata.reason == XmCR_HIGHLIGHT_1_SELECTED) ||
	(cbdata.reason == XmCR_HIGHLIGHT_2_SELECTED)) {

	/* Get the string which is in the highlighted region */
	XmTextGetSubstring((Widget)tw, cbdata.startPos,
                           (cbdata.endPos - cbdata.startPos), LINE_MAX, url);

	/* Flip the highlight color to mark it as an visited URL */
	XmTextSetHighlight((Widget) w, cbdata.startPos, cbdata.endPos, 
			   XmHIGHLIGHT_COLOR_2);
	XFlush(XtDisplay(w));

	if (!initdone) {
	    if (!DtInitialize(XtDisplay(w), w, "Unknown", "Unknown"))
		return;
	    DtDbLoad();
	    initdone = True;
	}

	/* Set the Dt action to be opened as a URL with a Web browser */
        actionArg = (DtActionArg *) malloc(sizeof(DtActionArg) * 1);
        memset(actionArg, 0, sizeof(DtActionArg));
	actionArg->argClass = DtACTION_BUFFER;
	actionArg->u.buffer.bp = url;
	actionArg->u.buffer.size = strlen(url);
	actionArg->u.buffer.type = NULL;
	actionArg->u.buffer.writable = False;

	DtActionInvoke(w, "Open", actionArg, 1, NULL, NULL, NULL, 1,
		       NULL, NULL);
    }

}

/*****************************************************************************
 * 
 * ParseUrlsCB - Sun URL aware text parser callback.
 * 
 * Returns - nothing.
 * 
 *****************************************************************************/
static void ParseUrlsCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data)
{
   /* Sun URL awareness */
   XmTextPosition start, pos, end, delim_pos;
   Boolean endOfLineFound;
   Boolean urlFound;
   char	buffer[LINE_MAX];
   static const char	*urlPrefix[] = {
       "http://", "ftp://", "gopher://", 
       "wais://", "file:/", "mailto:", 
       "news:"};
   int index;


   /* Loop through array of URL prefixes */
   for (index=0; index<XtNumber(urlPrefix); index++) {

       /* Find the first occurence of this URL prefix in the Text widget */
       start = 0;
       while (XmTextFindString((Widget)w, start, 
			       (char *)urlPrefix[index],
			       XmTEXT_FORWARD, &pos)) {

	   /* Find the end of this line */
	   endOfLineFound = XmTextFindString((Widget)w, pos, " ",
				       XmTEXT_FORWARD, &end);
	   if (!endOfLineFound)
	       endOfLineFound = XmTextFindString((Widget)w, 
						 pos, "\n", XmTEXT_FORWARD, 
						 &end);
	   if (!endOfLineFound)
	       end = ((XmTextWidget)w)->text.source->data->length - 1;

	   /* Get one line from Text widget to extract the URL */
	   if (XmTextGetSubstring((Widget)w, pos, (end-pos+1),
				  LINE_MAX, buffer) == XmCOPY_SUCCEEDED) {

	       /* Loop til the end of the URL 
		  (i.e. no more valid characters) */
	       delim_pos = 0;
	       while ((delim_pos < LINE_MAX) && 
		      ((buffer[delim_pos] >= 'A'&&buffer[delim_pos] <= 'Z') ||
		       (buffer[delim_pos] >= 'a'&&buffer[delim_pos] <= 'z') ||
		       (buffer[delim_pos] >= '0'&&buffer[delim_pos] <= '9') ||
		       (buffer[delim_pos] == ':') ||
		       (buffer[delim_pos] == '/') ||
		       (buffer[delim_pos] == '.') ||
		       (buffer[delim_pos] == '_') ||
		       (buffer[delim_pos] == '~') ||
		       (buffer[delim_pos] == '-') ||
		       (buffer[delim_pos] == '+') ||
		       (buffer[delim_pos] == '=') ||
		       (buffer[delim_pos] == '%') ||
		       (buffer[delim_pos] == '&') ||
		       (buffer[delim_pos] == '#') ||
		       (buffer[delim_pos] == '@') ||
		       (buffer[delim_pos] == ',') ||
		       (buffer[delim_pos] == '*') ||
		       (buffer[delim_pos] == ';') ||
		       (buffer[delim_pos] == '$') ||
		       (buffer[delim_pos] == '!') ||
		       (buffer[delim_pos] == '(') ||
		       (buffer[delim_pos] == ')') ||
		       (buffer[delim_pos] == '\'') ||
		       (buffer[delim_pos] == '?'))) {
		   delim_pos++;
	       }

	       /* Don't highlight empty URLs */
	       if (delim_pos <= strlen(urlPrefix[index])) {
		  start = pos + delim_pos + 1;
		  continue;
	       }

	       /* If URL ends with one of ".,!", cut it off. The URL
		  has been found at the end of a sentence in the Text
		  widget */
	       if ((delim_pos > 0) && ((buffer[delim_pos-1] == '.') ||
			               (buffer[delim_pos-1] == ',') ||
			               (buffer[delim_pos-1] == '!'))) {
		   delim_pos--;
	       }

	       /* Calculate the end position and highlight it to arm it
		  for highlight action 1 call from Motif */
	       end = pos + delim_pos;
 	       XmTextSetHighlight((Widget) w, pos, end, 
				  XmHIGHLIGHT_COLOR_1);
	   }

	   /* Continue while loop with rest of Text widget buffer */
	   start = end + 1;
       }	       	  
   }
}


/*-------------------------------------------------------------
**      Function: static Widget CreateText (
**                              DtEditorWidget parent)
**
**      Parameters: The parent of the text widget
**
**      Purpose:    This routine creates the scrolled text widget which
**                  lives inside the editor widget.
**                  It is responsible for the following:
**                      1) Creating the scrolled text widget,
**                      2) Adding specific translations,
**                      3) Adding our own callbacks,
**                      4) Substituting our own set selection routine.
*/
static Widget
CreateText(
        DtEditorWidget parent)
{
    register            int ac;         /* arg count */
    Arg 		al[21];		/* arg list */
    Widget		text;

   /* 
    * First, set up the hardwired scrolled text widget resource values.
    * (Change these and you die!  Aha! Ha! Ha! Ha! [evil laugh])
    */

   ac = 0;
   XtSetArg (al[ac], XmNeditMode, XmMULTI_LINE_EDIT);  ac++;
   XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++; 
   XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++; 
   XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++; 
   XtSetArg(al[ac], XmNpendingDelete, True); ac++; 

   /* 
    * Now, set up the resource values which can vary (passed in from 
    * application or default values).
    *
    * If a synthetic resource is DtUNSPECIFIED don't set it, but let
    * it default to the scrolled text default value.  
    * If it is specified, clear the data field after setting the 
    * resource because the value in the field will be out of sync 
    * with the real value.
    */
   XtSetArg(al[ac], XmNautoShowCursorPosition, M_autoShowCursorPos(parent));
   ac++;

   XtSetArg(al[ac], XmNblinkRate, M_blinkRate(parent)); ac++;

   if ( M_columns(parent) != DtUNSPECIFIED) {
     XtSetArg (al[ac], XmNcolumns, M_columns(parent) );  ac++;
     M_columns(parent) = (short) DtUNSPECIFIED;
   }

   if ( M_cursorPos(parent) != DtUNSPECIFIED) {
     XtSetArg (al[ac], XmNcursorPosition, M_cursorPos(parent));  ac++;
     M_cursorPos(parent) = (XmTextPosition) DtUNSPECIFIED;
   }

   XtSetArg(al[ac], XmNcursorPositionVisible, M_cursorPosVisible(parent));
   ac++;

   XtSetArg(al[ac], XmNeditable, M_editable(parent)); ac++;
  
   if ( E_textFontList(parent) != (XmFontList) NULL) {
     XtSetArg (al[ac], XmNfontList, E_textFontList(parent));  ac++;
   }

   if ( M_maxLength(parent) != DtUNSPECIFIED) {
     XtSetArg (al[ac], XmNmaxLength, M_maxLength(parent) );  ac++;
     M_maxLength(parent) = (int) DtUNSPECIFIED;
   }

   if ( M_rows(parent) != DtUNSPECIFIED) {
     XtSetArg (al[ac], XmNrows, M_rows(parent) );  ac++;
     M_rows(parent) = (short) DtUNSPECIFIED;
   }

   XtSetArg (al[ac], XmNscrollHorizontal, M_scrollHorizontal(parent) ); ac++;

   if ( M_scrollLeftSide(parent) != (Boolean)DtUNSPECIFIED) {
     XtSetArg (al[ac], XmNscrollLeftSide, M_scrollLeftSide(parent) ); ac++;
     M_scrollLeftSide(parent) = (Boolean)DtUNSPECIFIED;
   }

   if ( M_scrollTopSide(parent) != (Boolean)DtUNSPECIFIED) {
     XtSetArg (al[ac], XmNscrollTopSide, M_scrollTopSide(parent) ); ac++;
     M_scrollTopSide(parent) = (Boolean)DtUNSPECIFIED;
   }

   XtSetArg (al[ac], XmNscrollVertical, M_scrollVertical(parent) );  ac++;

   if ( M_topCharacter(parent) != DtUNSPECIFIED) {
     XtSetArg (al[ac], XmNtopCharacter, M_topCharacter(parent) );  ac++;
     M_topCharacter(parent) = (XmTextPosition) DtUNSPECIFIED;
   }

   XtSetArg (al[ac], XmNwordWrap, M_wordWrap(parent) );  ac++;
 
   /* Create text widget */
   text = XmCreateScrolledText ( (Widget) parent, "text", al, ac );


   /* 
    * Now, set the foreground & background of the text widget.  Could not 
    * set it at create time because the scrolled window would have
    * picked it up, too.
    */
   ac=0;

   if ( M_textBackground(parent) != DtUNSPECIFIED) {
     XtSetArg (al[ac], XmNbackground, M_textBackground(parent) );  ac++;
     M_textBackground(parent) = (Pixel) DtUNSPECIFIED;
   }

   if ( M_textForeground(parent) != DtUNSPECIFIED) {
     XtSetArg (al[ac], XmNforeground, M_textForeground(parent) );  ac++;
     M_textForeground(parent) = (Pixel) DtUNSPECIFIED;
   }

   if (ac)
       XtSetValues( text, al, ac);


   /* XXX Word Wrap workaround.  See comments for FixWordWrap() */
   if ( M_scrollHorizontal(parent) )         /* XXX Word Wrap workaround */
     FixWordWrap(text, M_wordWrap(parent));  /* XXX Word Wrap workaround */
   

   /* Put the editor-specific translation routines in place. */
   XtOverrideTranslations(text, 
			XtParseTranslationTable(EditorTranslationTable));

   /* Add any translations set by the application. */
   if ( E_textTranslations(parent) != (XtTranslations)NULL ) {
     XtOverrideTranslations(text, E_textTranslations(parent));
   }


   /*
    * Add modify verify callback
    */
   XtAddCallback(text, XmNmodifyVerifyCallback,
                  (XtCallbackProc) _DtEditorModifyVerifyCB,
                  (XtPointer) parent);

   /*
    * Add value changed callback for highlighting URLs, if read-only and
    * URL Awareness is enabled
    */
    if ( !M_editable(parent) && (M_enableUrlAwareness(parent))) {
       XtAddCallback(text, XmNvalueChangedCallback,
		     (XtCallbackProc) ParseUrlsCB,
		     NULL);

       /* Add callback which calls the Web browser when the URL is selected */
       XtAddCallback(text, XmNtextHighlightCallback,
                     (XtCallbackProc) CallWebBrowser,
                     NULL);
       M_urlCallbackAdded(parent) = True;
   }
   else
       M_urlCallbackAdded(parent) = False;

   /* 
    * Add the Help callback
    */
   XtAddCallback( text, XmNhelpCallback, (XtCallbackProc) HelpEditWindowCB,
                  (XtPointer) parent );


   /*
    * Substitute our own Set Selection routine which can
    * call the textSelect and textDeselect callback procs
    */
   if(((XmTextWidget)(text))->text.source->SetSelection !=
      Editor_SetSelectionProc) {
       M_setSelection(parent) =
               ((XmTextWidget)(text))->text.source->SetSelection;
       ((XmTextWidget)(text))->text.source->SetSelection =
                                                Editor_SetSelectionProc;
   }


   XtSetSensitive (text, True);
   return(text);

} /* end CreateText */


/*-------------------------------------------------------------
**      Function: static int extractFontMetrics (
**                              XmFontList fontlist )
**
**      Purpose: Given a font list, determine the width & height
**		 of the characters.
**
**	This routine is lifted almost straight out of 
**	lib/Xm/TextOut.c (see LoadFontMetrics() ) so the editor will
**	select the same font the text widget is working with.
*/
static void 
extractFontMetrics(
	XmFontList fontlist,
	int     *height,
	int     *width)
{
    XmFontContext context;
    XmFontListEntry next_entry;
    XmFontType type_return = XmFONT_IS_FONT;
    XtPointer tmp_font;
    XFontStruct *font;
    Boolean have_font_struct = False;
    Boolean have_font_set = False;
    Boolean use_font_set = False;
    XFontSetExtents *fs_extents;
    unsigned long tmp_width = 0;
    int	font_descent, font_ascent;
    char* font_tag = NULL;
    

    *width = *height = 0;
    if (XmFontListInitFontContext(&context, fontlist)) {

       /*
	* Look through the font list for a fontset with the default tag.
	* If we do not find it, use the first font set we came to, otherwise,
	* use the first font struct we came to.
	*/
       do {
          next_entry = XmFontListNextEntry(context);
          if (next_entry) {

             tmp_font = XmFontListEntryGetFont(next_entry, &type_return);
             if (type_return == XmFONT_IS_FONTSET) {

		/* 
		 * Have a font set
		 */
                font_tag = XmFontListEntryGetTag(next_entry);

                if (!have_font_set){ 

                   /* 
		    * Save the first fontset found in case we don't find 
		    * a default tag font set.
                    */
                   have_font_set = True;    /* we have a font set. */
                   use_font_set = True;
                   font = (XFontStruct *)tmp_font;

                   /* 
		    * We have a font set, so no need to consider future font 
		    * structs
		    */
                   have_font_struct = True; 

		   /*
		    * If this is the default font set break out, we've
		    * found the one we want.
		    */
                   if (!strcmp(XmFONTLIST_DEFAULT_TAG, font_tag))
                      break; 

                } 
		else if (!strcmp(XmFONTLIST_DEFAULT_TAG, font_tag)) {
		   /*
		    * If this is the default font set save it & break out,
		    * we've found the one we want.
		    */
                   font = (XFontStruct *)tmp_font;
                   have_font_set = True; 
                   break;
                }

             } 
	     else if (!have_font_struct){

		/*
		 * Have a font
		 */
                use_font_set = False;

                /* 
                 * Save the first one in case no font set is found 
		 */
                font = (XFontStruct *)tmp_font;
                use_font_set = False;
                have_font_struct = True;

             }
          }

       } while (next_entry != NULL);

       XmFontListFreeFontContext(context);

       /*
	* Now, extract the font metrics from the font set or font that
	* we chose.
	*/

       if (use_font_set) {
          /*
	   * Chose a font set
           */
          fs_extents = XExtentsOfFontSet((XFontSet)font);

          /*
           * XXX FONT DEFECT
           *
           * Although we should use the max_logical_extent field as the
           * width of a character, we must use max_ink_extent instead
           * because XmText uses it.  This is a bug in XmText (see
	   * LoadFontMetrics() in TextOut.c).  If this bug is ever
           * fixed, DtEditor should do the same.
           */

#ifdef NON_OSF_FIX
          tmp_width = (unsigned long)fs_extents->max_logical_extent.width;
#else /* NON_OSF_FIX */
          tmp_width = (unsigned long)fs_extents->max_ink_extent.width;
#endif /* NON_OSF_FIX */

          /* 
	   * max_ink_extent.y is number of pixels from origin to top of
           * rectangle (i.e. y is negative) 
	   */

          /*
           * XXX FONT DEFECT
           *
           * Although we should use the max_logical_extent field as the
           * height of a character, we must use max_ink_extent instead
           * because XmText uses it.  This is NOT a bug in XmText, but
           * rather a defect in the font metrics for the R4 fonts.  If
           * the metrics are ever corrected, DtEditor should switch.
           */

#ifdef NON_OSF_FIX
          font_ascent = -fs_extents->max_logical_extent.y;
          font_descent = fs_extents->max_logical_extent.height +
                         fs_extents->max_logical_extent.y;
#else /* NON_OSF_FIX */
          font_ascent = -fs_extents->max_ink_extent.y;
          font_descent = fs_extents->max_ink_extent.height +
                         fs_extents->max_ink_extent.y;
#endif /* NON_OSF_FIX */

       } 
       else {

          /*
	   * Chose a font struct
           */
          font_ascent = font->max_bounds.ascent;
          font_descent = font->max_bounds.descent;
          if ( (!XGetFontProperty(font, XA_QUAD_WIDTH, &tmp_width)) || 
	       tmp_width == 0 ) 
	  { 
             if ( font->per_char && font->min_char_or_byte2 <= '0' &&
                  font->max_char_or_byte2 >= '0' )
               tmp_width = font->per_char['0' - font->min_char_or_byte2].width;
             else
               tmp_width = font->max_bounds.width;
          }

       }

       if (tmp_width <= 0) tmp_width = 1;
       *width = (int)tmp_width; /* This assumes there will be no truncation */
       *height = font_descent + font_ascent;

    }

} /* end extractFontMetrics */



/*-------------------------------------------------------------
**      Function: static void getFontMetrics (
**                              DtEditorWidget w )
**
**      Parameters: An Editor widget
**
**      Purpose:    Determine the height & width of the scrolled text 
**		    widget's font
**
*/
static void
getFontMetrics( 
        DtEditorWidget w)
{
    Arg al[10];                 /* arg list */
    XmFontList fontlist;
    int	width, height;

    /*
     * Get the text widget's font list
     *
     * Note, have to retrieve the fontList from the text widget rather
     * than use the DtNtextFontList resource because the text widget may
     * have rejected the font specified with DtNtextFontList.  This way
     * we will get the fontList actually used by the text widget.
     */

    XtSetArg(al[0], XmNfontList, &fontlist); 
    XtGetValues(M_text(w), al, 1);

    /*
     * Now, extract the height and width
     */
    extractFontMetrics( fontlist, &height, &width );
    M_fontWidth(w) = width; 
    M_fontHeight(w) = height;

} /* end getFontMetrics */


/*-------------------------------------------------------------
**      Destroy
**              Release resources allocated for a widget instance.
*/
static void
Destroy(
        Widget widget )
{
   DtEditorWidget editor = (DtEditorWidget) widget;

   /*
    * Cleanup the edit window 
    */
   
   /* 
    * (Nothing to be done. Unregistering it as a drop site is handled
    *  automatically by the DnD library.)
    */

   /*
    * Cleanup the status line
    */
   
   if (M_status_insertLabel(editor) != (XmString)DtUNSPECIFIED)
      XmStringFree(M_status_insertLabel(editor));

   if (M_status_overstrikeLabel(editor) != (XmString)DtUNSPECIFIED)
      XmStringFree(M_status_overstrikeLabel(editor));

   if (E_status_currentLineLabel(editor) != (XmString)DtUNSPECIFIED)
      XmStringFree(E_status_currentLineLabel(editor));

   if (E_status_totalLineCountLabel(editor) != (XmString)DtUNSPECIFIED)
      XmStringFree(E_status_totalLineCountLabel(editor));

   /*
    * Cleanup the Find/Change & Spell dialogs
    */

   if (M_search_string(editor))
      XtFree(M_search_string(editor));

   if (M_replace_string(editor))
      XtFree(M_replace_string(editor));

   if (M_misspelled_string(editor))
      XtFree(M_misspelled_string(editor));

   if (M_spellFilter(editor))
      XtFree(M_spellFilter(editor));

   if (M_spellTitle(editor) != (XmString)NULL)
      XmStringFree(M_spellTitle(editor));

   if (E_misspelledListLabel(editor) != (XmString)DtUNSPECIFIED)
      XmStringFree(E_misspelledListLabel(editor));

   if (M_fndChngTitle(editor) != (XmString)NULL)
      XmStringFree(M_fndChngTitle(editor));

   if (E_findFieldLabel(editor) != (XmString)DtUNSPECIFIED)
      XmStringFree(E_findFieldLabel(editor));

   if (E_changeFieldLabel(editor) != (XmString)DtUNSPECIFIED)
      XmStringFree(E_changeFieldLabel(editor));

   if (E_findButtonLabel(editor) != (XmString)DtUNSPECIFIED)
      XmStringFree(E_findButtonLabel(editor));

   if (E_changeButtonLabel(editor) != (XmString)DtUNSPECIFIED)
      XmStringFree(E_changeButtonLabel(editor));

   if (E_changeAllButtonLabel(editor) != (XmString)DtUNSPECIFIED)
      XmStringFree(E_changeAllButtonLabel(editor));

   /*
    * Cleanup the Format Settings dialog
    */
   if (E_format_dialogTitle(editor) != (XmString)NULL)
      XmStringFree(E_format_dialogTitle(editor));

   if (E_format_leftMarginFieldLabel(editor) != (XmString)DtUNSPECIFIED)
      XmStringFree(E_format_leftMarginFieldLabel(editor));

   if (E_format_rightMarginFieldLabel(editor) != (XmString)DtUNSPECIFIED)
      XmStringFree(E_format_rightMarginFieldLabel(editor));

   if (E_format_leftAlignToggleLabel(editor) != (XmString)DtUNSPECIFIED)
      XmStringFree(E_format_leftAlignToggleLabel(editor));

   if (E_format_rightAlignToggleLabel(editor) != (XmString)DtUNSPECIFIED)
      XmStringFree(E_format_rightAlignToggleLabel(editor));

   if (E_format_justifyToggleLabel(editor) != (XmString)DtUNSPECIFIED)
      XmStringFree(E_format_justifyToggleLabel(editor));

   if (E_format_centerToggleLabel(editor) != (XmString)DtUNSPECIFIED)
      XmStringFree(E_format_centerToggleLabel(editor));

   if (E_format_formatAllButtonLabel(editor) != (XmString)DtUNSPECIFIED)
      XmStringFree(E_format_formatAllButtonLabel(editor));

   if (E_format_formatParagraphButtonLabel(editor) != (XmString)DtUNSPECIFIED)
      XmStringFree(E_format_formatParagraphButtonLabel(editor));

   /*
    * Cleanup the Information dialog
    */
   if (E_infoDialogTitle(editor) != (XmString)NULL)
      XmStringFree(E_infoDialogTitle(editor));

   /*
    * Cleanup the Undo deletion context
    */

   _DtEditorResetUndo(editor);

   /*
    * Cleanup the popup menu
    */
   if (M_menu(editor) != NULL)
      XtDestroyWidget(M_menu(editor));
   if (M_menu_includeDialog(editor) != NULL)
      XtDestroyWidget(M_menu_includeDialog(editor));

} /* end Destroy */


/*-------------------------------------------------------------
**      SetValues
**              Handle changes in resource data.
*/


static Boolean
SetValues(
        Widget cw,
        Widget rw,
        Widget nw )
{
  Boolean         redisplay_flag = False;
  Arg		al[10];		/*  arg list            */
  register int    ac;		/*  arg count           */

  Boolean	resetSpellTitle = False,
  		resetFormatTitle = False,
  		resetInfoTitle = False,
  		newButtonFontList = False,
  		newLabelFontList = False,
  		newTextFontList = False,
		NeedToFixWordWrap = False;	/* XXX Word Wrap workaround*/

  DtEditorWidget  current = (DtEditorWidget) cw;
  DtEditorWidget  request = (DtEditorWidget) rw;
  DtEditorWidget  new     = (DtEditorWidget) nw;

  /* 
   * XXX Need to propagate changes to Core resources, like XmNbackground,
   * to the children.
   */

  /*
   * If a synthetic resource is specified, clear the data field 
   * after setting the resource because the value in the field 
   * will be out of sync with the real value (contained in the 
   * child widget).
   */

  /* 
   * General changes, not directly affecting a child
   */

  /* Check for a new spell filter */

  if ( strcmp(M_spellFilter(request), M_spellFilter(current)) != 0 )
  {
    XtFree( M_spellFilter(current) );
    M_spellFilter(new) = XtNewString( M_spellFilter(request) );
  }

  /* Check for new font lists */

  if ( E_buttonFontList(request) != E_buttonFontList(current) ) 
     newButtonFontList = True;

  if ( E_labelFontList(request) != E_labelFontList(current) ) 
     newLabelFontList = True;

  if ( E_textFontList(request) != E_textFontList(current) ) 
     newTextFontList = True;

  /* 
   * Check for any changes which affect the dialog titles
   */

  if (XmStringCompare(E_dialogTitle(request),E_dialogTitle(current)) == False) {
    resetSpellTitle = resetFormatTitle = resetInfoTitle = True;
  }

  if (XmStringCompare(M_spellTitle(request),M_spellTitle(current)) == False) {
    XmStringFree( M_spellTitle(current) );
    M_spellTitle(new) = XmStringCopy( M_spellTitle(request) );
    resetSpellTitle = True;
  }

  if (XmStringCompare(M_fndChngTitle(request),M_fndChngTitle(current)) == False)
  {
    XmStringFree( M_fndChngTitle(current) );
    M_fndChngTitle(new) = XmStringCopy( M_fndChngTitle(request) );
    resetSpellTitle = True;
  }

  if (XmStringCompare(E_format_dialogTitle(request),
		      E_format_dialogTitle(current) ) == False)
  {
    XmStringFree( E_format_dialogTitle(current) );
    E_format_dialogTitle(new) = XmStringCopy( E_format_dialogTitle(request) );
    resetFormatTitle = True;
  }

  if (XmStringCompare(E_infoDialogTitle(request),
		      E_infoDialogTitle(current) ) == False)
  {
    XmStringFree( E_infoDialogTitle(current) );
    E_infoDialogTitle(new) = XmStringCopy( E_infoDialogTitle(request) );
    resetInfoTitle = True;
  }

 /*
  * Calculate new titles for the dialogs depending upon which resources
  * changed.
  */
  if ( resetSpellTitle == True ) {
    /*
     * No need to do anything because the Spell and Find/Change dialog
     * titles are recomputed every time.
     */
    redisplay_flag = True;
  }

  if ( resetFormatTitle == True ) {
    SetFormatDialogTitle( new );
    redisplay_flag = True;
  }

  if ( resetInfoTitle == True ) {
    SetInfoDialogTitle( new );
    redisplay_flag = True;
  }


  /* 
   * Check for any changes which affect the status line
   */
  if (M_status_showStatusLine(request) != M_status_showStatusLine(current)) {
    SetStatusLine( new, M_status_showStatusLine(request) );
    redisplay_flag = True;
  }

  if ( M_overstrikeMode(request) != M_overstrikeMode(current) ) {

    UpdateOverstrikeIndicator( new, M_overstrikeMode(request) );
    XtCallActionProc( (Widget) M_text(current), "toggle-overstrike", 
		      (XEvent *)NULL, (String *)NULL, 0 );
    redisplay_flag = True;
  }

  if ( M_status_statusArea(current) != (Widget)NULL ) {

    if (newLabelFontList == True) {
      XtSetArg (al[0], XmNfontList, E_labelFontList(request) );  
      XtSetValues( (Widget) M_status_lineLabel(current), al, 1);
      XtSetValues( (Widget) M_status_totalLabel(current), al, 1);
      XtSetValues( (Widget) M_status_totalText(current), al, 1);
      XtSetValues( (Widget) M_status_overstrikeWidget(current), al, 1);
    }

    ac=0;
    if (newTextFontList == True) {
       XtSetArg( al[ac], XmNfontList, E_textFontList(request) ); ac++;
    }

    if ( M_textBackground(request) != DtUNSPECIFIED) {
       XtSetArg( al[ac], XmNbackground, M_textBackground(request) ); ac++;
    }

    if ( M_textForeground(request) != DtUNSPECIFIED) {
       XtSetArg( al[ac], XmNforeground, M_textForeground(request) ); ac++;
    }

    if ( ac ) {
       XtSetValues( (Widget) M_status_lineText(current), al, ac);
       XtSetValues( (Widget) M_status_messageText(current), al, ac);
    }

  }


  /* 
   * Check for any changes which affect the dialog boxes
   */

  /* Information dialog */

  if ( M_gen_warning(current) != (Widget)NULL ) {

    if ( newButtonFontList == True ) { 
      XtSetArg (al[0], XmNfontList, E_buttonFontList(request) );
      XtSetValues( 
	 XmMessageBoxGetChild(M_gen_warning(current), XmDIALOG_OK_BUTTON),
	 al, 1 );
    }

    if ( newLabelFontList == True ) { 
      XtSetArg (al[0], XmNfontList, E_labelFontList(request) ); 
      XtSetValues( 
	 XmMessageBoxGetChild(M_gen_warning(current), XmDIALOG_MESSAGE_LABEL),
	 al, 1 );
    }

  }

  /* Find/Change dialog */
  if ( M_search_dialog(current) != (Widget)NULL ) 
  {
    if ( newButtonFontList == True ) { 
      XtSetArg (al[0], XmNfontList, E_buttonFontList(request)); 
      XtSetValues( (Widget)M_search_findBtn(current), al, 1 );
      XtSetValues( (Widget)M_search_replaceBtn(current), al, 1 );
      XtSetValues( (Widget)M_search_replaceAllBtn(current), al, 1 );
      XtSetValues( (Widget)M_search_closeBtn(current), al, 1 );
      XtSetValues( (Widget)M_search_helpBtn(current), al, 1 );
    }

    if ( newLabelFontList == True ) { 
      XtSetArg (al[0], XmNfontList, E_labelFontList(request)  );
      XtSetValues( (Widget)M_search_listLbl(current), al, 1   );
      XtSetValues( (Widget)M_search_findLbl(current), al, 1   );
      XtSetValues( (Widget)M_search_replaceLbl(current), al, 1);
    }

    if ( newTextFontList == True ) {
      XtSetArg (al[0], XmNfontList, E_textFontList(request)  ); 
      XtSetValues( (Widget)M_search_spellList(current), al, 1);
      XtSetValues( (Widget)M_findText(current), al, 1 );
      XtSetValues( (Widget)M_replaceText(current), al, 1 );
    }

    ac=0;
    if ( M_textBackground(request) != DtUNSPECIFIED) {
       XtSetArg( al[ac], XmNbackground, M_textBackground(request) ); ac++;
    }

    if ( M_textForeground(request) != DtUNSPECIFIED) {
       XtSetArg( al[ac], XmNforeground, M_textForeground(request) ); ac++;
    }

    if ( ac ) {
       XtSetValues( (Widget) M_findText(current), al, ac);
       XtSetValues( (Widget) M_replaceText(current), al, ac);
    }

  }

  /* Format Settings dialog */
  if ( M_format_dialog(current) != (Widget)NULL ) 
  {
    if ( newButtonFontList == True ) { 
      XtSetArg (al[0], XmNfontList, E_buttonFontList(request)); 
      XtSetValues( (Widget)M_format_leftJust(current), al, 1 );
      XtSetValues( (Widget)M_format_rightJust(current), al, 1);
      XtSetValues( (Widget)M_format_bothJust(current), al, 1 );
      XtSetValues( (Widget)M_format_center(current), al, 1   );
      XtSetValues( (Widget)M_format_paragraph(current), al, 1);
      XtSetValues( (Widget)M_format_all(current), al, 1      );
      XtSetValues( (Widget)M_format_close(current), al, 1    );
      XtSetValues( (Widget)M_format_help(current), al, 1     );
    }

    if ( newLabelFontList == True ) { 
      XtSetArg (al[0], XmNfontList, E_labelFontList(request)  );
      XtSetValues( (Widget)M_format_leftLabel(current), al, 1 );
      XtSetValues( (Widget)M_format_rightLabel(current), al, 1);
    }

    if ( newTextFontList == True ) {
      XtSetArg (al[0], XmNfontList, E_textFontList(request) ); 
      XtSetValues( (Widget)M_format_leftMarginField(current), al, 1 );
      XtSetValues( (Widget)M_format_rightMarginField(current), al, 1);
    }

    ac=0;
    if ( M_textBackground(request) != DtUNSPECIFIED) {
       XtSetArg( al[ac], XmNbackground, M_textBackground(request) ); ac++;
    }

    if ( M_textForeground(request) != DtUNSPECIFIED) {
       XtSetArg( al[ac], XmNforeground, M_textForeground(request) ); ac++;
    }

    if ( ac ) {
       XtSetValues( (Widget) M_format_leftMarginField(current), al, ac);
       XtSetValues( (Widget) M_format_rightMarginField(current), al, ac);
    }

  }


  /* 
   * Check for any changes which affect the edit area (scrolled text widget)
   */

  ac=0;
  if ( M_autoShowCursorPos(request) != M_autoShowCursorPos(current) ) {
     XtSetArg( al[ac], XmNautoShowCursorPosition, M_autoShowCursorPos(request));
     ac++;
  }

  if ( M_blinkRate(request) != M_blinkRate(current) ) {
     XtSetArg( al[ac], XmNblinkRate, M_blinkRate(request)); ac++;
  }

  if ( M_columns(request) != DtUNSPECIFIED) {
     XtSetArg (al[ac], XmNcolumns, M_columns(request) );  ac++;
     M_columns(new) = (short) DtUNSPECIFIED;
     redisplay_flag = True;
  }

  /* PERF - disable redisplay for all SetValues */
  if ( M_cursorPos(request) != DtUNSPECIFIED) {
     XmTextSetCursorPosition( M_text(new), M_topCharacter(request) );
     M_cursorPos(new) = (XmTextPosition) DtUNSPECIFIED;
  }

  if ( M_cursorPosVisible(request) != M_cursorPosVisible(current) ) {
     XtSetArg( al[ac], XmNcursorPositionVisible, M_cursorPosVisible(request));
     ac++;
  }

  if ( M_editable(request) != M_editable(current) ) {
     XtSetArg( al[ac], XmNeditable, M_editable(request)); ac++;

     if( M_editable(request) ) {

	/*
	 * If the widget is becoming editable, register it as a 
	 * drop site and...
	 */
        RegisterDropZone( new );

	/*
	 * ...turn back on the Change To field in the Find/Change dialog
	 */
	if ( M_search_dialog(current) != (Widget)NULL ) {
	  XtSetSensitive(M_search_replaceLbl(current), True);
          XtSetSensitive(M_replaceText(current), True);
        }
        if (M_enableUrlAwareness(request)) {
          XtRemoveAllCallbacks(M_text(request), XmNtextHighlightCallback);
          XtRemoveCallback(M_text(request), XmNvalueChangedCallback,
                       (XtCallbackProc) ParseUrlsCB, NULL);
        }

     } 
     else {
	/* 
	 * It's no longer available as a drop site.
	 */
        UnregisterDropZone( current );

	/*
	 * Add value changed callback for highlighting URLs, if URL Awareness 
	 * is enabled
	 */
	if (M_enableUrlAwareness(request) && !M_urlCallbackAdded(request)) {
	    XtAddCallback(M_text(request), XmNvalueChangedCallback,
			  (XtCallbackProc) ParseUrlsCB,
			  NULL);

       /* Add callback which calls the Web browser when the URL is selected */
            XtAddCallback(M_text(request), XmNtextHighlightCallback,
                     (XtCallbackProc) CallWebBrowser,
                     NULL);
	    M_urlCallbackAdded(request) = True;
	}

     }

  }

  if ( M_maxLength(request) != DtUNSPECIFIED) {
     XtSetArg (al[ac], XmNmaxLength, M_maxLength(request) );  ac++;
     M_maxLength(new) = (int) DtUNSPECIFIED;
  }

  if ( M_rows(request) != DtUNSPECIFIED) {
     XtSetArg (al[ac], XmNrows, M_rows(request) );  ac++;
     M_rows(new) = (short) DtUNSPECIFIED;
     redisplay_flag = True;
  }

  if ( M_textBackground(request) != DtUNSPECIFIED) {
     XtSetArg (al[ac], XmNbackground, M_textBackground(request) );  ac++;
     M_textBackground(new) = (XmTextPosition) DtUNSPECIFIED;
     redisplay_flag = True;
  }

  if ( newTextFontList == True ) {
     XtSetArg (al[ac], XmNfontList, E_textFontList(request) );  ac++;
     redisplay_flag = True;
  }

  if ( M_textForeground(request) != DtUNSPECIFIED) {
     XtSetArg (al[ac], XmNforeground, M_textForeground(request) );  ac++;
     M_textForeground(new) = (XmTextPosition) DtUNSPECIFIED;
     redisplay_flag = True;
  }

  /* PERF - disable redisplay for all SetValues */
  if ( M_topCharacter(request) != DtUNSPECIFIED) {
     XmTextSetTopCharacter( M_text(new), M_topCharacter(request) );
     M_topCharacter(new) = (XmTextPosition) DtUNSPECIFIED;
  }

  if ( M_wordWrap(request) != M_wordWrap(current) ) { 
    XtSetArg( al[ac], XmNwordWrap, M_wordWrap(request)); ac++; 
    NeedToFixWordWrap=M_scrollHorizontal(current); /* XXX Word Wrap workaround*/
  }

  if ( ac )
     XtSetValues( (Widget) M_text(new), al, ac);

  if ( NeedToFixWordWrap )			 /* XXX Word Wrap workaround */
    FixWordWrap(M_text(new), M_wordWrap(request)); /*XXX Word Wrap workaround*/

  if ( E_textTranslations(request) != E_textTranslations(current) ) {
   XtOverrideTranslations(M_text(new), E_textTranslations(request));
  }

  if ( newTextFontList == True ) {
    getFontMetrics(new);
  }

  return (redisplay_flag);

} /* end Set values */


/**************************************************************
**
**      Get values routines for synthetic resources
**
**/

static void
_DtEditorGetCenterToggleLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget editor = (DtEditorWidget) wid;
    XmString	data;
    Arg	al[1];

    /*
     * Get the value directly from the field in the instance structure.
     * If it is DtUNSPECIFIED then either:
     *   1) the value has been assigned to the widget (so get it from
     *      there), or
     *   2) the value has never been set (so return the default value).
     */

    if (E_format_centerToggleLabel(editor) != (XmString) DtUNSPECIFIED)
      data = XmStringCopy( E_format_centerToggleLabel(editor) );
    else if (M_format_dialog(editor) != (Widget)NULL)
    { 
       XtSetArg( al[0], XmNlabelString, &data) ;
       XtGetValues( (Widget) M_format_center(editor), al, 1) ;
    } 
    else
      data = XmStringCreateLtoR(CENTER, XmFONTLIST_DEFAULT_TAG);  
    *value = (XtArgVal) data ;

    return;
} /* end _DtEditorGetCenterToggleLabel */

static void
_DtEditorGetChangeAllButtonLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget editor = (DtEditorWidget) wid;
    XmString	data;
    Arg	al[1];

    /*
     * Get the value directly from the field in the instance structure.
     * If it is DtUNSPECIFIED then either:
     *   1) the value has been assigned to the widget (so get it from
     *      there), or
     *   2) the value has never been set (so return the default value).
     */

    if (E_changeAllButtonLabel(editor) != (XmString) DtUNSPECIFIED)
      data = XmStringCopy( E_changeAllButtonLabel(editor) );
    else if (M_search_dialog(editor) != (Widget) NULL)
    { 
       XtSetArg( al[0], XmNlabelString, &data) ;
       XtGetValues( (Widget) M_search_replaceAllBtn(editor), al, 1) ;
    } 
    else
      data = XmStringCreateLtoR(CHNG_ALL_BUTTON, XmFONTLIST_DEFAULT_TAG);  
    *value = (XtArgVal) data ;
    return;

} /* end _DtEditorGetChangeAllButtonLabel */

static void
_DtEditorGetChangeButtonLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget editor = (DtEditorWidget) wid;
    XmString	data;
    Arg	al[1];

    /*
     * Get the value directly from the field in the instance structure.
     * If it is DtUNSPECIFIED then either:
     *   1) the value has been assigned to the widget (so get it from
     *      there), or
     *   2) the value has never been set (so return the default value).
     */

    if (E_changeButtonLabel(editor) != (XmString) DtUNSPECIFIED)
      data = XmStringCopy( E_changeButtonLabel(editor) );
    else if (M_search_dialog(editor) != (Widget) NULL)
    { 
       XtSetArg( al[0], XmNlabelString, &data) ;
       XtGetValues( (Widget) M_search_replaceBtn(editor), al, 1) ;
    } 
    else
      data = XmStringCreateLtoR(CHANGE_BUTTON, XmFONTLIST_DEFAULT_TAG);  
    *value = (XtArgVal) data ;
    return;

} /* end _DtEditorGetChangeButtonLabel */

static void
_DtEditorGetChangeFieldLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget editor = (DtEditorWidget) wid;
    XmString	data;
    Arg	al[1];

    /*
     * Get the value directly from the field in the instance structure.
     * If it is DtUNSPECIFIED then either:
     *   1) the value has been assigned to the widget (so get it from
     *      there), or
     *   2) the value has never been set (so return the default value).
     */

    if (E_changeFieldLabel(editor) != (XmString) DtUNSPECIFIED)
      data = XmStringCopy( E_changeFieldLabel(editor) );
    else if (M_search_dialog(editor) != (Widget) NULL)
    { 
       XtSetArg( al[0], XmNlabelString, &data) ;
       XtGetValues( (Widget) M_search_replaceLbl(editor), al, 1) ;
    } 
    else
      data = XmStringCreateLtoR(CHANGE_LABEL, XmFONTLIST_DEFAULT_TAG);  
    *value = (XtArgVal) data ;
    return;

} /* end _DtEditorGetChangeFieldLabel */

static void
_DtEditorGetColumns(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget editor = (DtEditorWidget) wid;
    short	data;
    Arg	al[1];

    XtSetArg( al[0], XmNcolumns, &data) ;
    XtGetValues( (Widget) M_text(editor), al, 1) ;
    *value = (XtArgVal) data ;

    return;
} /* end _DtEditorGetColumns */

static void
_DtEditorGetCurrentLineLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget editor = (DtEditorWidget) wid;
    XmString	data;
    Arg	al[1];

    /*
     * Get the value directly from the field in the instance structure.
     * If it is DtUNSPECIFIED then either:
     *   1) the value has been assigned to the widget (so get it from
     *      there), or
     *   2) the value has never been set (so return the default value).
     */

    if (E_status_currentLineLabel(editor) != (XmString) DtUNSPECIFIED)
      data = XmStringCopy( E_status_currentLineLabel(editor) );
    else if (M_status_statusArea(editor) != (Widget) NULL)
    { 
       XtSetArg( al[0], XmNlabelString, &data) ;
       XtGetValues( (Widget) M_status_lineLabel(editor), al, 1) ;
    } 
    else
      data = XmStringCreateLtoR(LINE, XmFONTLIST_DEFAULT_TAG);  
    *value = (XtArgVal) data ;

    return;
} /* end _DtEditorGetCurrentLineLabel */

static void
_DtEditorGetCursorPosition(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget editor = (DtEditorWidget) wid;
    XmTextPosition	data;
    Arg	al[1];

    XtSetArg( al[0], XmNcursorPosition, &data) ;
    XtGetValues( (Widget) M_text(editor), al, 1) ;
    *value = (XtArgVal) data ;

    return;
} /* end _DtEditorGetCursorPosition */

static void
_DtEditorGetFindButtonLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget editor = (DtEditorWidget) wid;
    XmString	data;
    Arg	al[1];

    /*
     * Get the value directly from the field in the instance structure.
     * If it is DtUNSPECIFIED then either:
     *   1) the value has been assigned to the widget (so get it from
     *      there), or
     *   2) the value has never been set (so return the default value).
     */

    if (E_findButtonLabel(editor) != (XmString) DtUNSPECIFIED)
      data = XmStringCopy( E_findButtonLabel(editor) );
    else if (M_search_dialog(editor) != (Widget) NULL)
    { 
       XtSetArg( al[0], XmNlabelString, &data) ;
       XtGetValues( (Widget) M_search_findBtn(editor), al, 1) ;
    } 
    else
      data = XmStringCreateLtoR(FIND_BUTTON, XmFONTLIST_DEFAULT_TAG);  
    *value = (XtArgVal) data ;
    return;

} /* end _DtEditorGetFindButtonLabel */

static void
_DtEditorGetFindFieldLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget editor = (DtEditorWidget) wid;
    XmString	data;
    Arg	al[1];

    /*
     * Get the value directly from the field in the instance structure.
     * If it is DtUNSPECIFIED then either:
     *   1) the value has been assigned to the widget (so get it from
     *      there), or
     *   2) the value has never been set (so return the default value).
     */

    if (E_findFieldLabel(editor) != (XmString) DtUNSPECIFIED)
      data = XmStringCopy( E_findFieldLabel(editor) );
    else if (M_search_dialog(editor) != (Widget) NULL)
    { 
       XtSetArg( al[0], XmNlabelString, &data) ;
       XtGetValues( (Widget) M_search_findLbl(editor), al, 1) ;
    } 
    else
      data = XmStringCreateLtoR(FIND_LABEL, XmFONTLIST_DEFAULT_TAG);  
    *value = (XtArgVal) data ;
    return;

} /* end _DtEditorGetFindFieldLabel */

static void
_DtEditorGetFormatAllButtonLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget editor = (DtEditorWidget) wid;
    XmString	data;
    Arg	al[1];

    /*
     * Get the value directly from the field in the instance structure.
     * If it is DtUNSPECIFIED then either:
     *   1) the value has been assigned to the widget (so get it from
     *      there), or
     *   2) the value has never been set (so return the default value).
     */

    if (E_format_formatAllButtonLabel(editor) != (XmString) DtUNSPECIFIED)
      data = XmStringCopy( E_format_formatAllButtonLabel(editor) );
    else if (M_format_dialog(editor) != (Widget)NULL)
    { 
       XtSetArg( al[0], XmNlabelString, &data) ;
       XtGetValues( (Widget) M_format_all(editor), al, 1) ;
    } 
    else
      data = XmStringCreateLtoR(ALL, XmFONTLIST_DEFAULT_TAG);  
    *value = (XtArgVal) data ;

    return;
} /* end _DtEditorGetFormatAllButtonLabel */

static void
_DtEditorGetFormatParagraphButtonLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget editor = (DtEditorWidget) wid;
    XmString	data;
    Arg	al[1];

    /*
     * Get the value directly from the field in the instance structure.
     * If it is DtUNSPECIFIED then either:
     *   1) the value has been assigned to the widget (so get it from
     *      there), or
     *   2) the value has never been set (so return the default value).
     */

    if(E_format_formatParagraphButtonLabel(editor) != (XmString) DtUNSPECIFIED)
      data = XmStringCopy( E_format_formatParagraphButtonLabel(editor) );
    else if (M_format_dialog(editor) != (Widget)NULL)
    { 
       XtSetArg( al[0], XmNlabelString, &data) ;
       XtGetValues( (Widget) M_format_paragraph(editor), al, 1) ;
    } 
    else
      data = XmStringCreateLtoR(PARAGRAPH, XmFONTLIST_DEFAULT_TAG);  
    *value = (XtArgVal) data ;

    return;
} /* end _DtEditorGetFormatParagraphButtonLabel */

static void
_DtEditorGetJustifyToggleLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget editor = (DtEditorWidget) wid;
    XmString	data;
    Arg	al[1];

    /*
     * Get the value directly from the field in the instance structure.
     * If it is DtUNSPECIFIED then either:
     *   1) the value has been assigned to the widget (so get it from
     *      there), or
     *   2) the value has never been set (so return the default value).
     */

    if (E_format_justifyToggleLabel(editor) != (XmString) DtUNSPECIFIED)
      data = XmStringCopy( E_format_justifyToggleLabel(editor) );
    else if (M_format_dialog(editor) != (Widget)NULL)
    { 
       XtSetArg( al[0], XmNlabelString, &data) ;
       XtGetValues( (Widget) M_format_bothJust(editor), al, 1) ;
    } 
    else
      data = XmStringCreateLtoR(JUSTIFY, XmFONTLIST_DEFAULT_TAG);  
    *value = (XtArgVal) data ;

    return;
} /* end _DtEditorGetJustifyToggleLabel */

static void
_DtEditorGetLeftAlignToggleLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget editor = (DtEditorWidget) wid;
    XmString	data;
    Arg	al[1];

    /*
     * Get the value directly from the field in the instance structure.
     * If it is DtUNSPECIFIED then either:
     *   1) the value has been assigned to the widget (so get it from
     *      there), or
     *   2) the value has never been set (so return the default value).
     */

    if (E_format_leftAlignToggleLabel(editor) != (XmString) DtUNSPECIFIED)
      data = XmStringCopy( E_format_leftAlignToggleLabel(editor) );
    else if (M_format_dialog(editor) != (Widget)NULL)
    { 
       XtSetArg( al[0], XmNlabelString, &data) ;
       XtGetValues( (Widget) M_format_leftJust(editor), al, 1) ;
    } 
    else
      data = XmStringCreateLtoR(LEFT_ALIGN, XmFONTLIST_DEFAULT_TAG);  
    *value = (XtArgVal) data ;

    return;
} /* end _DtEditorGetLeftAlignToggleLabel */

static void
_DtEditorGetLeftMarginFieldLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget editor = (DtEditorWidget) wid;
    XmString	data;
    Arg	al[1];

    /*
     * Get the value directly from the field in the instance structure.
     * If it is DtUNSPECIFIED then either:
     *   1) the value has been assigned to the widget (so get it from
     *      there), or
     *   2) the value has never been set (so return the default value).
     */

    if (E_format_leftMarginFieldLabel(editor) != (XmString) DtUNSPECIFIED)
      data = XmStringCopy( E_format_leftMarginFieldLabel(editor) );
    else if (M_format_dialog(editor) != (Widget)NULL)
    { 
       XtSetArg( al[0], XmNlabelString, &data) ;
       XtGetValues( (Widget) M_format_leftLabel(editor), al, 1) ;
    } 
    else
      data = XmStringCreateLtoR(LEFT_MARGIN, XmFONTLIST_DEFAULT_TAG);  
    *value = (XtArgVal) data ;

    return;
} /* end _DtEditorGetLeftMarginFieldLabel */

static void
_DtEditorGetMaxLength(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget editor = (DtEditorWidget) wid;
    int	data;
    Arg	al[1];

    XtSetArg( al[0], XmNmaxLength, &data) ;
    XtGetValues( (Widget) M_text(editor), al, 1) ;
    *value = (XtArgVal) data ;

    return;
} /* end _DtEditorGetMaxLength */

static void
_DtEditorGetMisspelledListLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget editor = (DtEditorWidget) wid;
    XmString	data;
    Arg	al[1];

    /*
     * Get the value directly from the field in the instance structure.
     * If it is DtUNSPECIFIED then either:
     *   1) the value has been assigned to the widget (so get it from
     *      there), or
     *   2) the value has never been set (so return the default value).
     */

    if (E_misspelledListLabel(editor) != (XmString) DtUNSPECIFIED)
      data = XmStringCopy( E_misspelledListLabel(editor) );
    else if (M_search_dialog(editor) != (Widget) NULL)
    { 
       XtSetArg( al[0], XmNlabelString, &data) ;
       XtGetValues( (Widget) M_search_listLbl(editor), al, 1) ;
    } 
    else
      data = XmStringCreateLtoR(MISSPELLED, XmFONTLIST_DEFAULT_TAG);  
    *value = (XtArgVal) data ;
    return;

} /* end _DtEditorGetMisspelledListLabel */

static void
_DtEditorGetRightAlignToggleLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget editor = (DtEditorWidget) wid;
    XmString	data;
    Arg	al[1];

    /*
     * Get the value directly from the field in the instance structure.
     * If it is DtUNSPECIFIED then either:
     *   1) the value has been assigned to the widget (so get it from
     *      there), or
     *   2) the value has never been set (so return the default value).
     */

    if (E_format_rightAlignToggleLabel(editor) != (XmString) DtUNSPECIFIED)
      data = XmStringCopy( E_format_rightAlignToggleLabel(editor) );
    else if (M_format_dialog(editor) != (Widget)NULL)
    { 
       XtSetArg( al[0], XmNlabelString, &data) ;
       XtGetValues( (Widget) M_format_rightJust(editor), al, 1) ;
    } 
    else
      data = XmStringCreateLtoR(RIGHT_ALIGN, XmFONTLIST_DEFAULT_TAG);  
    *value = (XtArgVal) data ;

    return;
} /* end _DtEditorGetRightAlignToggleLabel */

static void
_DtEditorGetRightMarginFieldLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget editor = (DtEditorWidget) wid;
    XmString	data;
    Arg	al[1];

    /*
     * Get the value directly from the field in the instance structure.
     * If it is DtUNSPECIFIED then either:
     *   1) the value has been assigned to the widget (so get it from
     *      there), or
     *   2) the value has never been set (so return the default value).
     */

    if (E_format_rightMarginFieldLabel(editor) != (XmString) DtUNSPECIFIED)
      data = XmStringCopy( E_format_rightMarginFieldLabel(editor) );
    else if (M_format_dialog(editor) != (Widget)NULL)
    { 
       XtSetArg( al[0], XmNlabelString, &data) ;
       XtGetValues( (Widget) M_format_rightLabel(editor), al, 1) ;
    } 
    else
      data = XmStringCreateLtoR(RIGHT_MARGIN, XmFONTLIST_DEFAULT_TAG);  
    *value = (XtArgVal) data ;

    return;
} /* end _DtEditorGetRightMarginFieldLabel */

static void
_DtEditorGetRows(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget editor = (DtEditorWidget) wid;
    short	data;
    Arg	al[1];

    XtSetArg( al[0], XmNrows, &data) ;
    XtGetValues( (Widget) M_text(editor), al, 1) ;
    *value = (XtArgVal) data ;

    return;
} /* end _DtEditorGetRows */

static void
_DtEditorGetScrollLeftSide(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget editor = (DtEditorWidget) wid;
    Boolean	data;
    Arg	al[1];

    XtSetArg( al[0], XmNscrollLeftSide, &data) ;
    XtGetValues( (Widget) M_text(editor), al, 1) ;
    *value = (XtArgVal) data ;

    return;
} /* end _DtEditorGetScrollLeftSide */

static void
_DtEditorGetScrollTopSide(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget editor = (DtEditorWidget) wid;
    Boolean	data;
    Arg	al[1];

    XtSetArg( al[0], XmNscrollTopSide, &data) ;
    XtGetValues( (Widget) M_text(editor), al, 1) ;
    *value = (XtArgVal) data ;

    return;
} /* end _DtEditorGetScrollTopSide */

static void
_DtEditorGetTextBackground(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget	editor = (DtEditorWidget) wid;
    Pixel		data;
    Arg	al[1];

    XtSetArg( al[0], XmNbackground, &data) ;
    XtGetValues( (Widget) M_text(editor), al, 1) ;
    *value = (XtArgVal) data ;

    return;
} /* end _DtEditorGetTextBackground */

static void
_DtEditorGetTextForeground(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget	editor = (DtEditorWidget) wid;
    Pixel		data;
    Arg	al[1];

    XtSetArg( al[0], XmNforeground, &data) ;
    XtGetValues( (Widget) M_text(editor), al, 1) ;
    *value = (XtArgVal) data ;

    return;
} /* end _DtEditorGetTextForeground */

static void
_DtEditorGetTopCharacter(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget	editor = (DtEditorWidget) wid;
    XmTextPosition	data;
    Arg	al[1];

    XtSetArg( al[0], XmNtopCharacter, &data) ;
    XtGetValues( (Widget) M_text(editor), al, 1) ;
    *value = (XtArgVal) data ;

    return;
} /* end _DtEditorGetTopCharacter */

static void
_DtEditorGetLineCountLabel(
	Widget		wid,
	int		resource_offset,
	XtArgVal	*value )
{
    DtEditorWidget editor = (DtEditorWidget) wid;
    XmString	data;
    Arg	al[1];

    /*
     * Get the value directly from the field in the instance structure.
     * If it is DtUNSPECIFIED then either:
     *   1) the value has been assigned to the widget (so get it from
     *      there), or
     *   2) the value has never been set (so return the default value).
     */

    if (E_status_totalLineCountLabel(editor) != (XmString) DtUNSPECIFIED)
      data = XmStringCopy( E_status_totalLineCountLabel(editor) );
    else if (M_status_statusArea(editor) != (Widget) NULL)
    { 
       XtSetArg( al[0], XmNlabelString, &data) ;
       XtGetValues( (Widget) M_status_totalLabel(editor), al, 1) ;
    } 
    else
      data = XmStringCreateLtoR(TOTAL, XmFONTLIST_DEFAULT_TAG);  
    *value = (XtArgVal) data ;

    return;
} /* end _DtEditorGetLineCountLabel */


/**************************************************************
**
**      Action Procedures
**
**/

/*
 * The following are DtEditor's actions.  A few are internal only (_I
 * suffix) and will be called by the default translations DtEditor places
 * on the text widget.  The rest will be called from an application with 
 * XtCallActionProc().  The main difference is the internal ones will be 
 * passed a text widget ID, while the public ones will be passed a DtEditor ID.
 */

static void
BackwardChar(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "backward-character", event, 
		      params, *num_params );
} 

static void
BackwardField(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{
    if (!XtIsRealized(w))
	return;

    /*
     * If we weren't passed an XmText widget it must be the
     * DtEditor widget.  Convert it to the XmText widget.
     */
    if (!XtIsSubclass(w, xmTextWidgetClass)) {
        w = M_text((DtEditorWidget)w);
    }

    SelectNextField(w, event, XmTEXT_BACKWARD);
} 

static void
BackwardPara(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "backward-paragraph", event, 
		      params, *num_params );
} 

static void
BackwardWord(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "backward-word", event, 
		      params, *num_params );
} 

static void
BeginningOfFile(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "beginning-of-file", event, 
		      params, *num_params );
} 

static void
BeginningOfLine(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "beginning-of-line", event, 
		      params, *num_params );
} 

static void
ClearSelection(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "clear-selection", event, 
		      params, *num_params );

    /* 
     * Clearing selected text also deselects the selection, but for some
     * reason the selection proc (Editor_SetSelectionProc) does not get
     * called, therefore, we need to call the DtNtextDeselectCallback
     * from here.
     */
    Call_TextDeselectCallback(editor);

} 

static void
CopyClipboard(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "copy-clipboard", event, 
		      params, *num_params );
} 

static void
CutClipboard(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "cut-clipboard", event, 
		      params, *num_params );
} 

static void
DeleteNextChar(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "delete-next-character", event, 
		      params, *num_params );
} 

static void
DeleteNextWord(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "delete-next-word", event, 
		      params, *num_params );
} 

static void
DeletePrevChar(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "delete-previous-character", event, 
		      params, *num_params );
} 

static void
DeletePrevWord(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "delete-previous-word", event, 
		      params, *num_params );
} 

static void
DeleteToEOL(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "delete-to-end-of-line", event, 
		      params, *num_params );
} 

static void
DeleteToSOL(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "delete-to-start-of-line", event, 
		      params, *num_params );
} 

static void
DeselectAll(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "deselect-all", event, 
		      params, *num_params );
} 

static void
EndOfFile(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "end-of-file", event, 
		      params, *num_params );
} 

static void
EndOfLine(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "end-of-line", event, 
		      params, *num_params );
} 

/*
 * Action routine to filter the current selection through a
 * command specified as the argument to the action.
 */
/* ARGSUSED */
static void
Filter(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{
    DtEditorWidget editor;
    char **args;
    DtEditorErrorCode error;

    if (!XtIsRealized(w))
        return;

    /*
     * If we weren't passed an XmText widget it must be the
     * DtEditor widget.  Convert it to the XmText widget.
     * Or vice versa.
     */
    if (!XtIsSubclass(w, xmTextWidgetClass))
        editor = (DtEditorWidget)w;
    else
        editor = M_editor((XmTextWidget)w);

    if (!M_editable(editor))
        return;

    /*
     * Copy the arguments to a NULL-terminated array.
     * (Xt doesn't guarantee that params is NULL-terminated.)
     */
    args = (char **)malloc((*num_params + 1) * sizeof (char *));
    if (args == NULL)
        return;
    memcpy(args, params, *num_params * sizeof (char *));
    args[*num_params] = NULL;

    error = _DtEditorFilterSelection(editor, args);
    if (error != DtEDITOR_NO_ERRORS)
        XtCallActionProc(M_text(editor), "beep", event, NULL, 0);

    free(args);
}

/*
 * Find a string specified by the current selection.
 * If we don't own the current selection, have to use
 * a callback routine to get the selection from the
 * owner.  The following structure passes information
 * into the callback routine.
 */
struct FindSelectionData {
	XKeyEvent	event;
	XmTextDirection	direction;
};

static void
DoGetSelectionCB(
		Widget text,
		XtPointer client_data,
		Atom *selection,
		Atom *type,
		XtPointer value,
		unsigned long *length,
		int *format)
{
    XmTextPosition start, pos1;
    char *string;
    struct FindSelectionData *data = (struct FindSelectionData *)client_data;

    if (value == NULL && *length == 0)
	goto beep;
    if (*type != XA_STRING)
	goto beep;

    start = XmTextGetInsertionPosition(text);
    string = (char *)value;
    if (data->direction == XmTEXT_FORWARD) {
        if (XmTextFindString(text, start, string, XmTEXT_FORWARD, &pos1) ||
            XmTextFindString(text, 0, string, XmTEXT_FORWARD, &pos1)) {

            XmTextSetSelection(text, pos1,
		pos1 + _DtEditor_CountCharacters(string, strlen(string)),
                data->event.time);
            goto out;
        }
    }
    else {

        if (XmTextFindString(text, start, string, XmTEXT_BACKWARD, &pos1) ||
            XmTextFindString(text, XmTextGetLastPosition(text), string,
		XmTEXT_BACKWARD, &pos1)) {

            XmTextSetSelection(text, pos1,
		pos1 + _DtEditor_CountCharacters(string, strlen(string)),
                data->event.time);
            goto out;
        }
    }

beep:
    /* didn't find the string, beep */
    XtCallActionProc(text, "beep", (XEvent*)(&data->event), NULL, 0);
out:
    if (value != NULL)
	XtFree(value);
    if (data != NULL)
	free(data);
}

static void
FindSelection(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{
    Widget text;
    char *string;
    XmTextPosition left, right, pos1;
    XmTextDirection direction;

    if (!XtIsRealized(w))
        return;

    /*
     * If we weren't passed an XmText widget it must be the
     * DtEditor widget.  Convert it to the XmText widget.
     */
    if (!XtIsSubclass(w, xmTextWidgetClass)) {
        text = M_text((DtEditorWidget)w);
    } else {
        text = w;
    }

    if (*num_params == 1 && strcmp(params[0], "backward") == 0)
        direction = XmTEXT_BACKWARD;
    else
        direction = XmTEXT_FORWARD;

    /*
     * Handle the easy case here.
     * If we've got the selection, find the next instance of it.
     */

    if (!XmTextGetSelectionPosition(text, &left, &right)) {
        struct FindSelectionData *data;

	/*
         * We don't own the selection, have to do it the hard way...
	 * Pass the event and the search direction into the callback.
	 * The callback routine will free it.
	 */
        data = (struct FindSelectionData *)malloc(sizeof (*data));
	if (data == NULL)
	    return;
	data->event = *(XKeyEvent *)event;
	data->direction = direction;
        XtGetSelectionValue(text, XA_PRIMARY, XA_STRING, DoGetSelectionCB,
	    (XtPointer)data, ((XKeyEvent *)event)->time);
        return;
    }

    string = XmTextGetSelection(text);
    if (string == NULL)
	goto beep;

    if (direction == XmTEXT_FORWARD) {
        if (XmTextFindString(text, right, string, XmTEXT_FORWARD, &pos1) ||
            XmTextFindString(text, 0, string, XmTEXT_FORWARD, &pos1)) {

            if (pos1 != left) {
                XmTextSetSelection(text, pos1, pos1 + right - left,
                    ((XKeyEvent *)event)->time);
                XtFree(string);
                return;
            }
        }
    }
    else {
	XmTextPosition start;

	start = left - (right - left);
        if ((start >= 0 && XmTextFindString(text, start, string,
                XmTEXT_BACKWARD, &pos1)) ||
            XmTextFindString(text, XmTextGetLastPosition(text), string,
		XmTEXT_BACKWARD, &pos1)) {

            if (pos1 != left) {
                XmTextSetSelection(text, pos1, pos1 + right - left,
                    ((XKeyEvent *)event)->time);
                XtFree(string);
                return;
            }
        }
    }
    XtFree(string);

beep:
    /* didn't find the string, beep */
    XtCallActionProc(text, "beep", event, NULL, 0);
}

static void
FindString(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{
    DtEditorWidget editor;
    Widget text;
    char *string;
    XmTextPosition left, right, pos1;
    XmTextDirection direction;

    if (!XtIsRealized(w))
        return;

    /*
     * If we weren't passed an XmText widget it must be the
     * DtEditor widget.  Convert it to the XmText widget.
     * Or vice versa.
     */
    if (!XtIsSubclass(w, xmTextWidgetClass)) {
        editor = (DtEditorWidget)w;
        text = M_text(editor);
    } else {
        text = w;
        editor = M_editor((XmTextWidget)text);
    }

    if (*num_params == 0) {
	DtEditorInvokeFindChangeDialog((Widget)editor);
	return;
    }

    if (*num_params == 1 ||
	(*num_params == 2 && strcmp(params[0], "backward") != 0))
        direction = XmTEXT_FORWARD;
    else
        direction = XmTEXT_BACKWARD;

    string = params[0];
    if (!XmTextGetSelectionPosition(text, &left, &right))
        left = right = XmTextGetInsertionPosition(text);

    if (direction == XmTEXT_FORWARD) {
        if (XmTextFindString(text, right, string, XmTEXT_FORWARD, &pos1) ||
            XmTextFindString(text, 0, string, XmTEXT_FORWARD, &pos1)) {

            if (pos1 != left) {
                XmTextSetSelection(text, pos1,
		    pos1 + _DtEditor_CountCharacters(string, strlen(string)),
                    ((XKeyEvent *)event)->time);
                return;
            }
        }
    }
    else {
	XmTextPosition start;

	start = left - (right - left);
        if ((start >= 0 && XmTextFindString(text, start, string,
                XmTEXT_BACKWARD, &pos1)) ||
            XmTextFindString(text, XmTextGetLastPosition(text), string,
		XmTEXT_BACKWARD, &pos1)) {

            if (pos1 != left) {
                XmTextSetSelection(text, pos1,
		    pos1 + _DtEditor_CountCharacters(string, strlen(string)),
                    ((XKeyEvent *)event)->time);
                return;
            }
        }
    }

beep:
    /* didn't find the string, beep */
    XtCallActionProc(text, "beep", event, NULL, 0);
}

static void
ForwardChar(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "forward-character", event, 
		      params, *num_params );
} 

static void
ForwardField(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{
    if (!XtIsRealized(w))
	return;

    /*
     * If we weren't passed an XmText widget it must be the
     * DtEditor widget.  Convert it to the XmText widget.
     */
    if (!XtIsSubclass(w, xmTextWidgetClass)) {
        w = M_text((DtEditorWidget)w);
    }

    SelectNextField(w, event, XmTEXT_FORWARD);
} 

static void
ForwardPara(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "forward-paragraph", event, 
		      params, *num_params );
} 

static void
ForwardWord(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "forward-word", event, 
		      params, *num_params );
} 

static void
GoToLine(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{
    DtEditorWidget editor = (DtEditorWidget)w;

    if( M_status_statusArea(editor) != (Widget)NULL && 
        XtIsManaged(M_status_statusArea(editor)) == True ) 
    {

       /*
        * Set the input focus to the "Line" text field.
        */

       XmProcessTraversal(M_status_lineText(editor), XmTRAVERSE_CURRENT);

       /*
        * Select the current contents to allow easy modification.
        */

       XtCallActionProc( (Widget)M_status_lineText(editor), "select-all",
			 event, (String *)NULL, 0 );
    }

} /* end GoToLine */

/* 
 * Internal action called only from the scrolled text widget
 */
static void
GoToLine_I(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{
    XmTextWidget tw = (XmTextWidget)w;
    DtEditorWidget editor = (DtEditorWidget)M_editor(tw);

    GoToLine( (Widget)editor, event, params, num_params );

} /* end GoToLine_I */

static void
Help(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    CallHelpCallback((DtEditorWidget)w, DtEDITOR_HELP_EDIT_WINDOW);

} 

/*
 * Action to insert a file named by the action argument, or if no
 * argument by the preceeding contents of the current line.
 */
static void
InsertFile(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{
    DtEditorWidget editor;
    Widget text;
    char *string;
    XmTextPosition start, pos;
    DtEditorErrorCode error;

    if (!XtIsRealized(w))
        return;

    /*
     * If we weren't passed an XmText widget it must be the
     * DtEditor widget.  Convert it to the XmText widget.
     * Or vice versa.
     */
    if (!XtIsSubclass(w, xmTextWidgetClass)) {
        editor = (DtEditorWidget)w;
        text = M_text(editor);
    } else {
        text = w;
        editor = M_editor((XmTextWidget)text);
    }

    if (!M_editable(editor))
        return;

    _DtTurnOnHourGlass(M_topLevelShell(editor));

    pos = XmTextGetInsertionPosition(text);
    if (*num_params > 0) {
	/*
	 * Action argument names the file.
	 */
	string = params[0];
	start = pos;
    } else {
	/*
	 * Find the beginning of the current line.
	 */
	if (XmTextFindString(text, pos, "\n", XmTEXT_BACKWARD, &start))
	    start++;
	else
	    start = 0;

	/*
	 * Make the current line the selection,
	 * and then get the contents of the selection.
	 * XXX - better to just use XmTextGetSubString?
	 */
	XmTextSetSelection(text, start, pos, ((XKeyEvent *)event)->time);
	string = XmTextGetSelection(text);
	/* XXX - strip leading and trailing whitespace? */
    }

    /* XXX - if it's a directory, do "cd" instead */
    error = DtEditorReplaceFromFile((Widget)editor, start, pos, string);
    if (error != DtEDITOR_NO_ERRORS && error != DtEDITOR_READ_ONLY_FILE)
        XtCallActionProc(text, "beep", event, NULL, 0);

    if (*num_params == 0)
	XtFree(string);

    _DtTurnOffHourGlass(M_topLevelShell(editor));
} 

static void
InsertString(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "insert-string", event, 
		      params, *num_params );
} 

static void
KeySelect(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "key-select", event, 
		      params, *num_params );
} 

static void
NewlineAndBackup(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "newline-and-backup", event, 
		      params, *num_params );
} 

static void
NewlineAndIndent(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "newline-and-indent", event, 
		      params, *num_params );
} 

static void
NextPage(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "next-page", event, 
		      params, *num_params );
} 

static void
PageLeft(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "page-left", event, 
		      params, *num_params );
} 

static void
PageRight(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "page-right", event, 
		      params, *num_params );
} 

static void
PasteClipboard(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "paste-clipboard", event, 
		      params, *num_params );
} 

static void
PopupMenu(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{
    DtEditorWidget editor;

    /*
     * If we weren't passed an XmText widget it must be the
     * DtEditor widget.  Convert it to the XmText widget.
     * Or vice versa.
     */
    if (!XtIsSubclass(w, xmTextWidgetClass))
        editor = (DtEditorWidget)w;
    else
        editor = M_editor((XmTextWidget)w);

    if (M_menu(editor) == NULL)
	_DtEditorCreateMenu(editor);
    XmMenuPosition(M_menu(editor), (XButtonPressedEvent *)event);
    XtManageChild(M_menu(editor));
}

static void
PreviousPage(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "previous-page", event, 
		      params, *num_params );
} 

static void
ProcessCancel(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "process-cancel", event, 
		      params, *num_params );
} 

static void
ProcessDown(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "process-down", event, 
		      params, *num_params );
} 

static void
ProcessShiftDown(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "process-shift-down", event, 
		      params, *num_params );
} 

static void
ProcessShiftUp(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "process-shift-up", event, 
		      params, *num_params );
} 

static void
ProcessUp(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "process-up", event, 
		      params, *num_params );
} 

/************************************************************************
 *  GetModeSwitchModifier
 *      Find if any Mod<n> modifier is acting as the Group modifier - you
 *      can't assume Mod1Mask is always it.
 *      Returns the mask of the modifier key to which the Mode_switch
 *      keysym is attached.
 *
 ************************************************************************/
/* ARGSUSED */
static unsigned int
GetModeSwitchModifier(
                Display *dpy)
{
    register XModifierKeymap *pMap;
    register int mapIndex, keyCol, mapSize;
    KeySym keySym;
    unsigned int modeSwitchModMask = 0;
    pMap = XGetModifierMapping(dpy);
    mapSize = 8*pMap->max_keypermod;

    for (mapIndex = 3*pMap->max_keypermod; mapIndex < mapSize; mapIndex++) {
        /* look only at the first 4 columns of key map */
        for (keyCol = 0; keyCol < 4; keyCol++) {
          keySym = XKeycodeToKeysym(dpy, pMap->modifiermap[mapIndex], keyCol);
          if (keySym == XK_Mode_switch)
             modeSwitchModMask |= 1 << (mapIndex / pMap->max_keypermod);
        }
    }

    XFreeModifiermap(pMap);
    return modeSwitchModMask;
}

/************************************************************************
 *
 *  DtEditorQuoteNextChar
 *      This action routine circumvents the normal XtDispatchEvent
 *      mechanism, inserting the next control or extended character
 *      directly into text without processed by event handlers or the
 *      translation manager.  This means, for
 *      example, that the next control or extended character will be
 *      inserted into text without being intrepreted as a Motif menu
 *      or text widget accelerator.
 *
 ************************************************************************/
/* ARGSUSED */
static void
QuoteNextChar(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{
    DtEditorWidget editor = (DtEditorWidget)w;
    static unsigned int ExtendcharMask = 0;

    /*  NOTE:  ExtendcharMask could be a global set via a MappingNotify
     *         event handler.
     */

    if (! ExtendcharMask) 
        ExtendcharMask = GetModeSwitchModifier( M_display(editor) );

    for (;;) 
    {
      XEvent nextEvent;

      XtAppNextEvent(M_app_context(editor), &nextEvent);
      if ((nextEvent.type == KeyPress) &&
          (! IsModifierKey(XLookupKeysym((XKeyEvent *) &nextEvent,0))) ) 
      {
         if (nextEvent.xkey.state & (ControlMask|ExtendcharMask)) 
	 {
            XtCallActionProc(M_text(editor), "self-insert", 
			     &nextEvent, NULL, 0);
         } 
	 else {
            XtDispatchEvent(&nextEvent);
         }
         break;
      } 
      else {
          XtDispatchEvent(&nextEvent);
      }

    }

} /* end QuoteNextChar */

/* 
 * Internal action called only from the scrolled text widget
 */
static void
QuoteNextChar_I(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{
    XmTextWidget tw = (XmTextWidget)w;
    DtEditorWidget editor = (DtEditorWidget) M_editor(tw);

    QuoteNextChar( (Widget)editor, event, params, num_params );

} /* end QuoteNextChar_I */

static void
SelectAll(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorWidget editor = (DtEditorWidget)w;

    XtCallActionProc( M_text(editor), "select-all", event, 
		      params, *num_params );
} 

static void
ToggleInsertMode(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{
    DtEditorWidget editor = (DtEditorWidget)w;

    /*
     * Toggle the state of the DtNoverstrike resource
     */
    XtVaSetValues(w, DtNoverstrike, !M_overstrikeMode(editor), NULL);

} /* end ToggleInsertMode */

static void
ToggleInsertMode_I(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{
    XmTextWidget tw = (XmTextWidget)w;
    DtEditorWidget editor = (DtEditorWidget)M_editor(tw);

    ToggleInsertMode( (Widget)editor, event, params, num_params );

} /* end ToggleInsertMode_I */


static void
UndoEdit(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{

    DtEditorUndoEdit(w);

} /* end UndoEdit */

static void
UndoEdit_I(
                Widget w,
                XEvent *event,
                char **params,
                Cardinal *num_params)
{
    XmTextWidget tw = (XmTextWidget)w;

    UndoEdit( (Widget)M_editor(tw), event, params, num_params );

} /* end UndoEdit_I */


/*
 * XXX All lines in this file marked "XXX Word Wrap workaround"
 * XXX are related to a Motif 1.2 Text widget design defect regarding 
 * XXX word wrap.  For Motif 1.2, the Text widget was designed so word
 * XXX wrap will not function if the scrolled text widget has a 
 * XXX horizontal scroll bar ("a hscrollbar means the paper is infinite
 * XXX so there is no edge to wrap at").  This change was made in 
 * XXX Xm/TextOut.c RCS version 1.71 checked in July, 1991.  A
 * XXX CMVC report was filed 9/8/94 & assigned number 4772.
 */
#include <Xm/TextOutP.h>		   /* XXX Word Wrap workaround */
static void		 		   /* XXX Word Wrap workaround */
FixWordWrap(				   /* XXX Word Wrap workaround */
	Widget	w,			   /* XXX Word Wrap workaround */
	Boolean wrapOn)			   /* XXX Word Wrap workaround */
{                                          /* XXX Word Wrap workaround */
  XmTextWidget tw = (XmTextWidget)w;	   /* XXX Word Wrap workaround */
  OutputData data = tw->text.output->data; /* XXX Word Wrap workaround */
  
  data->scrollhorizontal = !wrapOn;	   /* XXX Word Wrap workaround */
}                                          /* XXX Word Wrap workaround */

static void
Call_TextSelectCallback(
	DtEditorWidget	editor)
{

   DtEditorSelectCallbackStruct	select_cb;

   /*
    * Call the Editor widget's DtNtextSelectCallback proc 
    * if it hasn't been called since the last select.  
    */

   if ( !M_textSelectCbCalled(editor) ) {

	M_textSelectCbCalled(editor) = True;
	select_cb.reason = DtEDITOR_TEXT_SELECT;
	select_cb.event = (XEvent *)NULL;
	XtCallCallbackList ((Widget)editor, M_textSelect(editor),
	  	            (XtPointer) &select_cb);
   }

} /* end Call_TextSelectCallback */

static void
Call_TextDeselectCallback(
	DtEditorWidget	editor)
{

   DtEditorSelectCallbackStruct	select_cb;

   /*
    * Call the Editor widget's DtNtextDeselectCallback proc
    * if it hasn't been called since the last deselect.
    */

   if ( M_textSelectCbCalled(editor) ) {

	M_textSelectCbCalled(editor) = False;
	select_cb.reason = DtEDITOR_TEXT_DESELECT;
        select_cb.event = (XEvent *)NULL;
        XtCallCallbackList ((Widget)editor, M_textDeselect(editor),
			    (XtPointer) &select_cb);
   }

} /* end Call_TextDeselectCallback */

/*
 * Editor_SetSelectionProc 
 *		   is put into the widget->text.source->SetSelection.
 *		   Used to call the DtEditor's select/unselect callbacks
 */
static void
Editor_SetSelectionProc(
        XmTextSource source,
        XmTextPosition left,
        XmTextPosition right,
        Time set_time )  
{
    XmSourceData data = source->data;
    int 				i;
    Widget 				widget;
    XmTextWidget 			tw;
    DtEditorWidget 			editor;
    DtEditorSelectCallbackStruct	select_cb;


    /*
     * Sanity check
     */
    if (!XtIsRealized((Widget)data->widgets[0]) ||
	data->numwidgets <= 0)
	return;

    if (left < 0) left = right = 0;

    widget = (Widget) data->widgets[0];
    tw = (XmTextWidget)widget;
    editor = M_editor(tw);

    if (!editor->core.being_destroyed) {

       if (left < right) {
          /*
  	   * There is a selected area if left < right so call the
	   * DtNtextSelectCallback.
	   */

          Call_TextSelectCallback( editor );
	  M_hasSelection(editor) = True;

       } 
       else {
          /*
	   * Left = Right so nothing is selected; call the
	   * DtNtextDeselectCallback.
	   */

	  Call_TextDeselectCallback( editor );
	  M_hasSelection(editor) = False;

       }
    }

    /* 
     * Now, call the text widget's real Set Selection proc
     */
    (*M_setSelection(editor))(source, left, right, set_time);

} /* end Editor_SetSelectionProc */

/*
 * Locate the next textsw-like field (delimited by |> <|)
 * and make it the primary selection.  Ring the bell if
 * there is no other field.
 */
static void
SelectNextField(
	Widget w,
	XEvent *event,
	XmTextDirection direction)
{
    XmTextPosition  left, right, pos1, pos2;
    /* XXX - how do I declare these constant strings in an I18N-safe way? */
    static char leftdelim[] = "|>", rightdelim[] = "<|";

    if (!XmTextGetSelectionPosition(w, &left, &right) || left == right) {

        /* no selection, start at the insertion point */
        left = right = XmTextGetInsertionPosition(w);
    }

    if (direction == XmTEXT_FORWARD) {
        if (XmTextFindString(w, right, leftdelim, XmTEXT_FORWARD, &pos1) ||
            XmTextFindString(w, 0, leftdelim, XmTEXT_FORWARD, &pos1)) {

            /* found first delimiter, now try for the second one */
            if (XmTextFindString(w, pos1, rightdelim, XmTEXT_FORWARD, &pos2)) {

                /* found them both */
                XmTextSetSelection(w, pos1, pos2 + strlen(rightdelim),
                    ((XKeyEvent *)event)->time);
                return;
            }
        }
    }
    else {

        if (XmTextFindString(w, left, rightdelim, XmTEXT_BACKWARD, &pos2) ||
            XmTextFindString(w, XmTextGetLastPosition(w), rightdelim,
		XmTEXT_BACKWARD, &pos2)) {

            /* found first delimiter, now try for the second one */
            if (XmTextFindString(w, pos2, leftdelim, XmTEXT_BACKWARD, &pos1)) {

                /* found them both */
                XmTextSetSelection(w, pos1, pos2 + strlen(rightdelim),
                    ((XKeyEvent *)event)->time);
                return;
            }
        }
    }

    /* didn't find a field, beep */
    XtCallActionProc(w, "beep", event, NULL, 0);
}

/*
 * Filter the current selection through the specified command.
 */
DtEditorErrorCode
_DtEditorFilterSelection(
		DtEditorWidget editor,
                char **args)
{
    Widget text;
    char *string;
    FILE *fp = NULL;
    char *buf = NULL;
    size_t len, bufsize;
    ssize_t got;
    int p[2] = { -1, -1 };
    int child, execvp_stat = 0;
    XmTextPosition start, end;
    DtEditorContentRec cr;  /* Structure for passing data to widget */
    DtEditorErrorCode ret_value = DtEDITOR_NO_ERRORS;
#define	MAX_BUFSIZ	(8 * BUFSIZ)	/* stop doubling buffer at this size */

    text = M_text(editor);

    _DtTurnOnHourGlass(M_topLevelShell(editor));

    /* 
     * Create a tmp file.
     * XXX - Really should use two pipes and an I/O poll loop.
     */
    if ((fp = tmpfile()) == NULL) {
        ret_value = DtEDITOR_FILTER_FAILED;
        goto out;
    }

    /* 
     * Temporary file created sucessfully so write out contents of
     * selection in preparation for feeding it to the filter.
     */
    string = (char *)XmTextGetSelection(text);
    if (string != NULL) {
        len = strlen(string);
        fwrite(string, sizeof(char), len, fp);
        XtFree(string);
    } else
        len = 0;

    rewind(fp);

    /* start command */
    if (pipe(p) < 0) {
        ret_value = DtEDITOR_FILTER_FAILED;
        goto out;
    }
    child = fork1();
    if (child < 0) {
        close(p[0]);
        close(p[1]);
        ret_value = DtEDITOR_FILTER_FAILED;
        goto out;
    } else if (child == 0) {
        /*
         * The child process.
         * Setup the tmp file as stdin and the pipe as stdout.
         */
        dup2(fileno(fp), 0);
        dup2(p[1], 1);
        close(fileno(fp));
        close(p[1]);
        close(p[0]);
        execvp(args[0], args);

        /* Notifies parent execvp() failed. Assumes we used fork1(). */
        execvp_stat = -1;
        _exit(-1);
    }

    /*
     * The parent process.
     * Close the tmp file and read from the pipe.
     */
    fclose(fp);
    fp = NULL;
    close(p[1]);
    p[1] = -1;

    /*
     * Allocate a buffer to receive the data into.
     * We don't use XtMalloc because we don't want to
     * go down in flames if we're out of memory.
     * Start with a buffer a bit bigger than the string
     * we're replacing.
     */
    len += BUFSIZ;
    buf = (char *)malloc(len);
    if (buf == NULL) {
        ret_value = DtEDITOR_FILTER_FAILED;
        goto out;
    }

    /*
     * Read the data coming back from the pipe until EOF.
     */
    bufsize = 0;
    while ((got = read(p[0], &buf[bufsize], len - bufsize)) > 0) {
        bufsize += got; 
        /*
         * If we've filled up the buffer, expand it.
         */
        if (bufsize == len) {
            char *nbuf;
            /* double the size until we get to MAX_BUFSIZ */
            if (len < MAX_BUFSIZ)
                len *= 2;
            else
                len += MAX_BUFSIZ;
            nbuf = (char *)realloc(buf, len);
            if (nbuf == NULL) {
                ret_value = DtEDITOR_FILTER_FAILED;
                goto out;
            }
            buf = nbuf;
        }
    }
    buf[bufsize] = '\0';

    /* find the position of the selection so we can replace it all */
    if (!XmTextGetSelectionPosition(text, &start, &end) || start == end) {
        /* no selection, put it at the insertion point */
        start = end = XmTextGetInsertionPosition(text);
    }

    cr.type = DtEDITOR_TEXT;
    cr.value.string = buf;

    /* Only replace the selected text if the filter successfully executed. */
    if (execvp_stat == -1)
        ret_value = DtEDITOR_FILTER_FAILED;
    else
        (void) DtEditorReplace((Widget)editor, start, end, &cr);

out:
    if (p[0] > 0)
        close(p[0]);
    if (p[1] > 0)
        close(p[1]);
    if (fp != NULL)
        fclose(fp);
    if (buf != NULL)
        free(buf);

    _DtTurnOffHourGlass(M_topLevelShell(editor));
    return (ret_value);
}


/**************************************************************
**
**      Drag and drop routines
**
**/


/*
 * Handles drops of a file into the text editor, after validation in the 
 * transfer callback.  Files are handled by reading their contents. 
 * Text is handled by Motif.
 *
 * Changing the contents of the text widget occurs here, rather than in
 * the transfer callback, because of the visual changes which occur when
 * changing the text widget.
 */

static void
AnimateCallback(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
{ 
    int numItems, ii; 
    DtEditorErrorCode	error;

    DtDndDropAnimateCallbackStruct *animateInfo = 
				(DtDndDropAnimateCallbackStruct *) call_data;
    DtEditorWidget editor = (DtEditorWidget) client_data;

/* XXX Only really needed if # of items is > 1
    _DtTurnOnHourGlass( M_topLevelShell(editor) ); 
*/

    /*
     * How many items are being dropped on us?
     */
    numItems = animateInfo->dropData->numItems;

    switch (animateInfo->dropData->protocol) 
    {
       case DtDND_FILENAME_TRANSFER:
       {
	 /*
	  * OK, insert each file we are given
	  */
	 for (ii = 0; ii < numItems; ii++) 
	 {
           error = DtEditorInsertFromFile( (Widget)editor, 
				animateInfo->dropData->data.files[ii] );

	   /*
	    * If the file was not inserted successfully, then quit
	    */
           if( error != DtEDITOR_NO_ERRORS )
	   {
	     ii = numItems; /* break out of loop */
	   }

	 } /* end for */

	 break;

       } /* end file transfer case */

       case DtDND_BUFFER_TRANSFER:
       {
	 /*
	  * OK, insert each buffer we are given
	  */

	 DtDndBuffer *buffers = animateInfo->dropData->data.buffers;
	 DtEditorContentRec cr;
	 cr.type = DtEDITOR_DATA;

	 for (ii = 0; ii < numItems; ii++) 
	 {
	   cr.value.data.buf = buffers[ii].bp;
	   cr.value.data.length = buffers[ii].size;
	   
           error = DtEditorInsert( (Widget)editor, &cr );

	   /*
	    * If the buffer was not inserted successfully, then quit
	    */
           if( error != DtEDITOR_NO_ERRORS )
	   {
	     ii = numItems; /* break out of loop */
	   }

	 } /* end for */

	 break;

       } /* end buffer transfer case */

       case DtDND_TEXT_TRANSFER:
       {
	 /*
	  * Receiving a text drop.
	  *
	  * Text drag and drop is handled by Motif so this switch 
	  * will never be called.
	  *
	  */
	 break;

       } /* end text transfer case */

       default:
       {
       } 

    } /* end switch */

/* XXX  _DtTurnOffHourGlass( M_topLevelShell(editor) ); */

} /* end AnimateCallback */


/*
 * Validates a drop of a file into the text widget.  
 *
 * Changing the contents of the text widget occurs in the animate
 * callback, rather than here, because of the visual changes which 
 * occur when changing the text widget.
 */

/* ARGSUSED */
static void
TransferCallback(
        Widget w,
        XtPointer client_data,
        XtPointer call_data)
{
    int numItems, ii; 
    DtEditorErrorCode 	error;

    DtDndTransferCallbackStruct *transferInfo = 
				    (DtDndTransferCallbackStruct *) call_data;
    DtEditorWidget editor = (DtEditorWidget) client_data;

    transferInfo->status = DtDND_SUCCESS;

    /*
     * How many items are being dropped on us?
     */
    numItems = transferInfo->dropData->numItems;

    switch (transferInfo->dropData->protocol) 
    {
       case DtDND_FILENAME_TRANSFER:
       {

         /*
	  * Check to see if we can read each file we are given
	  */
	 for (ii = 0; ii < numItems; ii++) 
	 {
	   error = _DtEditorValidateFileAccess(
	   			transferInfo->dropData->data.files[ii],
	   			READ_ACCESS);

	   /*
	    * Can't access it for reading so reject drop
	    */
           if ( error != DtEDITOR_NO_ERRORS ) {
              transferInfo->status = DtDND_FAILURE;
	      ii = numItems; /* break out of loop */
	   }

         } /* end for each file */

	 break;

       } /* end file transfer case */

       case DtDND_BUFFER_TRANSFER:
       {
	 /*
	  * Receiving a buffer drop.
	  *  -- do nothing in transfer
	  */
	 break;

       } /* end buffer transfer case */

       case DtDND_TEXT_TRANSFER:
       {
	 /*
	  * Receiving a text drop.
	  *
	  * Text drag and drop is handled by Motif so this switch 
	  * will never be called.
	  *
	  */
	 break;

       } /* end text transfer case */

       default:
       {
         transferInfo->status = DtDND_FAILURE;
	 break;
       }

    } /* end switch */

} /* end TransferCallback */ 


/*-------------------------------------------------------------
**	Function: static void RegisterDropZone ( 
**      			DtEditorWidget w)
**
**	Parameters: A DtEditor widget 
**
**	Purpose:    This routine registers the edit window's text widget 
**		    as a drop site for file & buffer drops.  Because it 
**		    is a text widget it will automatically accept text 
**		    drops.
*/
static void
RegisterDropZone(
        DtEditorWidget w)
{
    int             ac;
    Arg		    al[2];
    
    static XtCallbackRec transferCB[] = { {TransferCallback, NULL},
                                          {NULL, NULL} };
    static XtCallbackRec animateCB[] = { {AnimateCallback, NULL},
					 {NULL, NULL} };
    transferCB[0].closure = (XtPointer) w;
    animateCB[0].closure = (XtPointer) w;

    /*
     * Register file & buffer transfers... let Motif handle text DnD
     */
    ac=0;
    XtSetArg( al[ac], DtNdropAnimateCallback, animateCB ); ac++;

    DtDndDropRegister( M_text(w), 
		       DtDND_FILENAME_TRANSFER|DtDND_BUFFER_TRANSFER,
		       XmDROP_COPY, transferCB, al, ac );

} /* end RegisterDropZone */

/*-------------------------------------------------------------
**	Function: static void UnregisterDropZone ( 
**      			DtEditorWidget w)
**
**	Parameters: A DtEditor widget 
**
**	Purpose:    This routine unregisters the edit window
*/
static void
UnregisterDropZone(
        DtEditorWidget w)
{

    DtDndDropUnregister( M_text(w) );

} /* end UnregisterDropZone */

/************************************************************************
 *
 *  SetInfoDialogTitle - Change the title for the Information dialog
 *
 ************************************************************************/

static void
SetInfoDialogTitle(
        DtEditorWidget editor)
{

  Arg al[2];

  /*
   * If the Information dialog has been created, change its title
   */
  if( M_gen_warning(editor) != (Widget)NULL )
  {

       /*
	* Prepend the DialogTitle resource, if it has been set
	*/
       if( E_dialogTitle(editor) != (XmString)NULL ) {
         XmString titleStr;

	 /*
	  * Add the "functional title" to the DialogTitle
	  */
         titleStr = XmStringConcat( E_dialogTitle(editor), 
				    E_infoDialogTitle(editor) );

         XtSetArg( al[0], XmNdialogTitle, titleStr );
         XtSetValues(M_gen_warning(editor), al, 1);

         XmStringFree( titleStr );

     }
     else {
         XtSetArg( al[0], XmNdialogTitle, E_infoDialogTitle(editor) );
         XtSetValues(M_gen_warning(editor), al, 1);
     }
  }

} /* end SetInfoDialogTitle */

/************************************************************************
 *
 *  _DtEditorWarning - get a message to the user
 *
 ************************************************************************/

void 
_DtEditorWarning(
	DtEditorWidget editor,
        char *mess,
	unsigned char dialogType)
{
    Arg al[10];
    int ac;
    char *tmpMess;
 
    tmpMess = strdup(mess);

    /* create the dialog if it is the first time */
    if(M_gen_warning(editor) == (Widget) NULL) 
    {
	XmString titleStr = (XmString) NULL;

        ac = 0;

       /*
        * First, create the dialog's title, prepending the
        * DtNdialogTitle resource, if it is set
        */

	if( E_dialogTitle(editor) != (XmString)NULL ) {
	  
	  /*
	   * Add the "functional title" to the DialogTitle
	   */
          titleStr = XmStringConcat( E_dialogTitle(editor), 
				    E_infoDialogTitle(editor) );

          XtSetArg (al[ac], XmNdialogTitle, titleStr ); ac++;

	}
	else {
          XtSetArg (al[ac], XmNdialogTitle, E_infoDialogTitle(editor)); ac++;
	}

        XtSetArg (al[ac], XmNokLabelString, XmStringCreateLtoR(_DtOkString,
						XmFONTLIST_DEFAULT_TAG)); ac++;
        M_gen_warning(editor) = (Widget) XmCreateMessageDialog(
				               M_topLevelShell(editor), "Warn",
					       al, ac);

	if (titleStr != (XmString) NULL)
	  XmStringFree( titleStr );

        /* Set the correct font lists for the message & OK button. */
        XtSetArg (al[0], XmNfontList, E_buttonFontList(editor));
	XtSetValues( 
	   XmMessageBoxGetChild(M_gen_warning(editor), XmDIALOG_OK_BUTTON),
	   al, 1) ;

        XtSetArg (al[0], XmNfontList, E_labelFontList(editor));
	XtSetValues( 
	   XmMessageBoxGetChild(M_gen_warning(editor), XmDIALOG_MESSAGE_LABEL),
	   al, 1) ;

        /* Unmanage unneeded children. */
        XtUnmanageChild ( XmMessageBoxGetChild(M_gen_warning(editor), 
                                                 XmDIALOG_CANCEL_BUTTON) );
        XtUnmanageChild ( XmMessageBoxGetChild(M_gen_warning(editor),
                                                 XmDIALOG_HELP_BUTTON) );

        XtRealizeWidget (M_gen_warning(editor));
        ac=0;
        XtSetArg(al[ac], XmNmwmInputMode, 
		 MWM_INPUT_PRIMARY_APPLICATION_MODAL);ac++;
        XtSetValues(XtParent(M_gen_warning(editor)), al, ac);
    }

    ac = 0;
    XtSetArg(al[ac], XmNdialogType, dialogType); ac++; 
    XtSetArg(al[ac], XmNmessageString,
                 XmStringCreateLtoR(tmpMess, XmFONTLIST_DEFAULT_TAG)); ac++;
    XtSetValues(M_gen_warning(editor), al, ac);

    XtFree( (char *) tmpMess );

    XtManageChild (M_gen_warning(editor));

} /* end _DtEditorWarning */ 


/*********************************************************************
 *
 * The following section contains the procedures related to the staus
 * line
 *
 *********************************************************************/

#define	FORCE		True
#define	DONT_FORCE	False

/*
 * PositionActivateCB is invoked when the user presses [Return] after
 * (presumably) modifying the current line text field.  It retrieves
 * the user specified line number and calls DtEditorGoToLine().
 */
 
/* ARGSUSED */
static void
PositionActivateCB(
        Widget w,
	caddr_t client_data,
	caddr_t call_data )
{
    DtEditorWidget editor = (DtEditorWidget)client_data;
    char *lineStr = XmTextFieldGetString(M_status_lineText(editor));
    int newLineNum = atoi(lineStr);
    XtFree(lineStr);

    DtEditorGoToLine ((Widget)editor, newLineNum);
    XmProcessTraversal(M_text(editor), XmTRAVERSE_CURRENT);

} /* PositionActivateCB */

/* 
 * _DtEditorUpdateLineDisplay updates the current & total line displays
 * in the status line, if needed.  Normally, the displays are not changed 
 * if the correct numbers are (supposedly) displayed.  Setting forceUpdate 
 * will cause the numbers to be updated anyways.  This is primarily used
 * when the user enters a number into the current line display to force
 * display of the correct number.
 */
void
_DtEditorUpdateLineDisplay(
        DtEditorWidget	editor,
	int		currentLine,
	Boolean		forceUpdate )
{
    XmTextWidget tw = (XmTextWidget) M_text(editor);
    Arg		al[10];
    char 	tmpChars[8];
    int		lastLine;
    XmString	tmpXmStr;

    /*
     * Only change the current & total lines displays if the status 
     * line is visible
     */
    if ( M_status_showStatusLine(editor) == True )
    {
      /* 
       * Current line
       */
      if (M_status_currentLine(editor) != currentLine || forceUpdate) 
      {
        sprintf(tmpChars, "%d", currentLine);
        XmTextFieldSetString(M_status_lineText(editor), tmpChars);
	M_status_currentLine(editor) = currentLine;
      }

      /*
       * Total lines
       */
      lastLine = tw->text.total_lines;
      if(M_status_lastLine(editor) != lastLine )
      {

	sprintf(tmpChars, "%d", lastLine);
	tmpXmStr = XmStringCreateLtoR(tmpChars, XmFONTLIST_DEFAULT_TAG); 
	XtSetArg(al[0], XmNlabelString, tmpXmStr); 
        XtSetValues( M_status_totalText(editor), al, 1 );

	XmStringFree(tmpXmStr);
	M_status_lastLine(editor) = lastLine;

      }
    }
} /* end _DtEditorUpdateLineDisplay */


static void
UpdateOverstrikeIndicator( 
	DtEditorWidget	widget,
	Boolean		overstrikeOn )
{
    Arg 	   al[10];
    register	   int ac;
    DtEditorWidget ew = (DtEditorWidget) widget;
 
    /*
     * Only change the overstrike indicator if the status line is visible
     */
    if ( M_status_showStatusLine(ew) == True &&
	 M_status_overstrikeWidget(ew) != (Widget) NULL )
    {
       ac=0;

       if ( overstrikeOn == True ) {
	 XtSetArg(al[ac], XmNlabelString, M_status_overstrikeLabel(ew)); ac++;
       }
       else {
         XtSetArg(al[ac], XmNlabelString, M_status_insertLabel(ew)); ac++;
       }

       XtSetValues( M_status_overstrikeWidget(ew), al, ac );
    }

} /* end UpdateOverstrikeIndicator */


int
_DtEditorGetLineIndex(
	XmTextWidget tw,
	XmTextPosition pos)
{
    int startLine, lastLine, middleLine;
    XmTextLineTable lineTab = tw->text.line_table;

    startLine = 0;
    lastLine = tw->text.total_lines - 1;

    while(startLine != lastLine)
    {
	middleLine = (startLine + lastLine)/2;
	if(middleLine == startLine || middleLine == lastLine)
	{
	    /*
	     * We're down to 2 lines.  It's gotta be on one of these
	     * two lines.
	     */
	    if(pos < (XmTextPosition) lineTab[lastLine].start_pos)
		lastLine = startLine;
	    else
		startLine = lastLine;
	}
	else
	{
	    if (pos < (XmTextPosition) lineTab[middleLine].start_pos)
	        lastLine = middleLine;
	    else
	        startLine = middleLine;
	}
    }
    return startLine;
} /* end _DtEditorGetLineIndex */


/*
 * SetCursorPosStatus is called as an XmNmotionVerifyCallback on the
 * text widget when the statusLine is turned on.  It computes and
 * displays the new line of the insert cursor.
 */
static void
SetCursorPosStatus(
        Widget w,
	caddr_t client_data,
	caddr_t call_data )
{
    DtEditorWidget editor = (DtEditorWidget)client_data;
    XmTextWidget tw = (XmTextWidget)w;
    XmTextVerifyCallbackStruct * cb = (XmTextVerifyCallbackStruct *) call_data;
    int startLine;

    startLine = _DtEditorGetLineIndex(tw, cb->newInsert) + 1;

    _DtEditorUpdateLineDisplay( editor, startLine, FORCE );

} /* end SetCursorPosStatus */


static void
SetStatusLine(
	DtEditorWidget	ew,
	Boolean	statusLineOn)
{
   Arg 		al[10];
   register int	ac;
   int currentLine;
   XmTextPosition cursorPos;

   M_status_showStatusLine(ew) = statusLineOn;

   if( statusLineOn == True )
   {
     if( M_status_statusArea(ew) == (Widget)NULL )
	M_status_statusArea(ew) = CreateStatusLine( ew ); 

     /*
      * Update the line counts
      */

     cursorPos = (XmTextPosition)DtEditorGetInsertionPosition( (Widget)ew );
     currentLine = _DtEditorGetLineIndex( (XmTextWidget) M_text(ew), 
					  cursorPos ) + 1;
     _DtEditorUpdateLineDisplay( ew, currentLine, DONT_FORCE );

     /*
      * Update the overstrike indicator
      */
     UpdateOverstrikeIndicator( ew, M_overstrikeMode(ew) );

     /* 
      * Show the status line
      */
     
     XtManageChild( M_status_statusArea(ew) );

     /*
      * Hook the scrolled text widget to the status line
      */
     ac = 0;
     XtSetArg( al[ac], XmNbottomAttachment, XmATTACH_WIDGET ); ac++;
     XtSetArg( al[ac], XmNbottomWidget, M_status_statusArea(ew) ); ac++;
     XtSetValues( XtParent(M_text(ew)), al, ac );
     
     XtAddCallback( M_text(ew), XmNmotionVerifyCallback,
		    (XtCallbackProc) SetCursorPosStatus, (XtPointer)ew);

   }
   else 
   {
     /*
      * Hook the scrolled text widget to the bottom of the form
      */
     ac = 0;
     XtSetArg( al[ac], XmNbottomAttachment, XmATTACH_FORM ); ac++;
     XtSetValues( XtParent(M_text(ew)), al, ac );

     XtRemoveCallback( M_text(ew), XmNmotionVerifyCallback,
		    (XtCallbackProc) SetCursorPosStatus, (XtPointer)ew);

     /* 
      * Hide the status line
      */
     if( M_status_statusArea(ew) != (Widget)NULL )
       XtUnmanageChild( M_status_statusArea(ew) );

   }

} /* SetStatusLine */


/************************************************************************
 *
 * CreateStatusLine - Creates the status line
 *
 ************************************************************************/

static Widget 	
CreateStatusLine(
	DtEditorWidget parent)
{
    Arg 	al[20];
    int 	ac;
    Widget	container;
    XmString 	tmpStr;
    Pixel	background, foreground;

    /* 
     * Match the background & foreground colors of the edit window
     * Don't use DtNtextBackground/Foreground directly because they 
     * will be DtUNSPECIFIED.
     */
    ac = 0;
    XtSetArg(al[ac], XmNforeground, &foreground); ac++;
    XtSetArg(al[ac], XmNbackground, &background); ac++;
    XtGetValues(M_text(parent), al, ac);

    /*
     * Create the status line container
     */
    ac = 0;
    XtSetArg( al[ac], XmNbottomAttachment, XmATTACH_FORM ); ac++;
    XtSetArg( al[ac], XmNleftAttachment, XmATTACH_FORM ); ac++;
    XtSetArg( al[ac], XmNrightAttachment, XmATTACH_FORM ); ac++;
    XtSetArg( al[ac], XmNverticalSpacing, 3 ); ac++;
    XtSetArg( al[ac], XmNhorizontalSpacing, 3 ); ac++;
    container = (Widget) XmCreateForm( (Widget)parent, "statusLine", 
					    al, ac );

    /*
     * Create the current line label and text field
     */
    ac = 0;
    if (E_status_currentLineLabel(parent) != (XmString)DtUNSPECIFIED)
    {
      /*
       * Use the resource value & clear it (to save space).
       */
      tmpStr = XmStringCopy(E_status_currentLineLabel(parent));
      E_status_currentLineLabel(parent) = (XmString)DtUNSPECIFIED;
    }
    else {
      /*
       * The resource has not been set so use its default value
       */
      tmpStr = XmStringCreateLtoR(LINE, XmFONTLIST_DEFAULT_TAG);
    }

    XtSetArg( al[ac], XmNbottomAttachment, XmATTACH_FORM ); ac++;
    XtSetArg( al[ac], XmNtopAttachment, XmATTACH_FORM ); ac++;
    XtSetArg( al[ac], XmNleftAttachment, XmATTACH_FORM ); ac++;

    XtSetArg(al[ac], XmNlabelString, tmpStr);	ac++;
    XtSetArg(al[ac], XmNfontList, E_labelFontList(parent) ); ac++;
    M_status_lineLabel(parent) = (Widget) XmCreateLabelGadget( container, 
						"lineLabel", al, ac);
    XtManageChild(M_status_lineLabel(parent));
    XtAddCallback(M_status_lineLabel(parent), XmNhelpCallback,
		  (XtCallbackProc)HelpStatusCurrentLineCB, parent);
    XmStringFree(tmpStr);

    ac = 0;
    XtSetArg(al[ac], XmNcolumns, 6);	ac++;
    XtSetArg(al[ac], XmNforeground, foreground); ac++;
    XtSetArg(al[ac], XmNbackground, background); ac++;
    XtSetArg( al[ac], XmNfontList, E_textFontList(parent)); ac++;
    XtSetArg( al[ac], XmNbottomAttachment, XmATTACH_FORM ); ac++;
    XtSetArg( al[ac], XmNtopAttachment, XmATTACH_FORM ); ac++;
    XtSetArg( al[ac], XmNleftAttachment, XmATTACH_WIDGET ); ac++;
    XtSetArg( al[ac], XmNleftWidget, M_status_lineLabel(parent) ); ac++;
    M_status_lineText(parent) = XmCreateTextField( container, "lineText", 
						   al, ac );
    XtManageChild(M_status_lineText(parent));
    XtAddCallback(M_status_lineText(parent), XmNactivateCallback,
		  (XtCallbackProc)PositionActivateCB, parent);
    XtAddCallback(M_status_lineText(parent), XmNhelpCallback,
		  (XtCallbackProc)HelpStatusCurrentLineCB, parent);

    /*
     * Create the total lines labels
     */
    ac = 0;
    if (E_status_totalLineCountLabel(parent) != (XmString)DtUNSPECIFIED)
    {
      /*
       * Use the resource value & clear it (to save space).
       */
      tmpStr = XmStringCopy(E_status_totalLineCountLabel(parent));
      E_status_totalLineCountLabel(parent) = (XmString)DtUNSPECIFIED;
    }
    else {
      /*
       * The resource has not been set so use its default value
       */
      tmpStr = XmStringCreateLtoR(TOTAL, XmFONTLIST_DEFAULT_TAG);
    }

    XtSetArg(al[ac], XmNlabelString, tmpStr);	ac++;
    XtSetArg(al[ac], XmNfontList, E_labelFontList(parent) ); ac++;
    XtSetArg( al[ac], XmNbottomAttachment, XmATTACH_FORM ); ac++;
    XtSetArg( al[ac], XmNtopAttachment, XmATTACH_FORM ); ac++;
    XtSetArg( al[ac], XmNleftAttachment, XmATTACH_WIDGET ); ac++;
    XtSetArg( al[ac], XmNleftWidget, M_status_lineText(parent) ); ac++;
    M_status_totalLabel(parent) = (Widget) XmCreateLabelGadget( container, 
						"totalLabel", al, ac);
    XtManageChild(M_status_totalLabel(parent));
    XtAddCallback(M_status_totalLabel(parent), XmNhelpCallback,
		  (XtCallbackProc)HelpStatusTotalLinesCB, parent);
    XmStringFree(tmpStr);

    ac = 0;
    XtSetArg(al[ac], XmNfontList, E_labelFontList(parent) ); ac++;
    XtSetArg( al[ac], XmNbottomAttachment, XmATTACH_FORM ); ac++;
    XtSetArg( al[ac], XmNtopAttachment, XmATTACH_FORM ); ac++;
    XtSetArg( al[ac], XmNleftAttachment, XmATTACH_WIDGET ); ac++;
    XtSetArg( al[ac], XmNleftWidget, M_status_totalLabel(parent) ); ac++;
    M_status_totalText(parent) = (Widget) XmCreateLabelGadget( container, 
						"totalText", al, ac);
    XtManageChild(M_status_totalText(parent));
    XtAddCallback(M_status_totalText(parent), XmNhelpCallback,
		  (XtCallbackProc)HelpStatusTotalLinesCB, parent);

    /*
     * Create the overstrike indicator
     */
    ac = 0;
    /* 
    XtSetArg(al[ac], XmNrecomputeSize, False); ac++; 
    */
    XtSetArg(al[ac], XmNmarginLeft, 0); ac++;
    XtSetArg(al[ac], XmNmarginRight, 0); ac++;
    XtSetArg(al[ac], XmNmarginWidth, 0); ac++;
    XtSetArg( al[ac], XmNrightOffset, 3 ); ac++;
    XtSetArg(al[ac], XmNfontList, E_labelFontList(parent) ); ac++;
    XtSetArg( al[ac], XmNbottomAttachment, XmATTACH_FORM ); ac++;
    XtSetArg( al[ac], XmNtopAttachment, XmATTACH_FORM ); ac++;
    XtSetArg( al[ac], XmNrightAttachment, XmATTACH_FORM ); ac++;
    M_status_overstrikeWidget(parent) = 
	(Widget) XmCreateLabelGadget( container, "overstrikeLabel", al, ac);
    XtManageChild(M_status_overstrikeWidget(parent));
    XtAddCallback(M_status_overstrikeWidget(parent), XmNhelpCallback,
		  (XtCallbackProc)HelpStatusOverstrikeCB, parent);

    /*
     * Create the application message area
     */
    ac = 0;
    XtSetArg(al[ac], XmNbackground, parent->core.background_pixel); ac++;
    XtSetArg(al[ac], XmNforeground, foreground); ac++;
    XtSetArg( al[ac], XmNfontList, E_textFontList(parent)); ac++;
    XtSetArg(al[ac], XmNeditable, False); ac++;
    XtSetArg(al[ac], XmNcursorPositionVisible, False); ac++;
    XtSetArg(al[ac], XmNtraversalOn, False); ac++;
    XtSetArg( al[ac], XmNbottomAttachment, XmATTACH_FORM ); ac++;
    XtSetArg( al[ac], XmNtopAttachment, XmATTACH_FORM ); ac++;
    XtSetArg( al[ac], XmNleftAttachment, XmATTACH_WIDGET ); ac++;
    XtSetArg( al[ac], XmNleftWidget, M_status_totalText(parent) ); ac++;
    XtSetArg( al[ac], XmNleftOffset, 17 ); ac++;
    XtSetArg( al[ac], XmNrightAttachment, XmATTACH_WIDGET ); ac++;
    XtSetArg(al[ac], XmNrightWidget, M_status_overstrikeWidget(parent)); ac++;
    M_status_messageText(parent) = 
                      XmCreateTextField( container, "messageText", al, ac );

    XtManageChild(M_status_messageText(parent));
    XtAddCallback(M_status_messageText(parent), XmNhelpCallback,
		  (XtCallbackProc)HelpStatusMessageCB, parent);

    return( container );

} /* end CreateStatusLine */


/*********************************************************************
 *
 * The following section contains the procedures related to help
 *
 *********************************************************************/


static void
CallHelpCallback(
	DtEditorWidget	editor,
	int		reason)
{
    DtEditorHelpCallbackStruct      help_cb;

    help_cb.reason = reason;
    help_cb.event = (XEvent *) NULL;
    XtCallCallbackList( (Widget)editor, M_HelpCB(editor), 
			(XtPointer) &help_cb );

} /* end CallHelpCallback */

/****
 * Edit window help callbacks
 */

/* ARGSUSED */
static void
HelpEditWindowCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    CallHelpCallback((DtEditorWidget)client_data, DtEDITOR_HELP_EDIT_WINDOW);
}

/****
 * Status Line help callbacks
 */

/* XXX
 * Who uses this??
 */

/* ARGSUSED */
static void
HelpStatusLineCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    CallHelpCallback((DtEditorWidget)client_data, DtEDITOR_HELP_STATUS_LINE);
}

/* ARGSUSED */
static void
HelpStatusCurrentLineCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    CallHelpCallback( (DtEditorWidget)client_data, 
		      DtEDITOR_HELP_STATUS_CURRENT_LINE );
}

/* ARGSUSED */
static void
HelpStatusTotalLinesCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    CallHelpCallback( (DtEditorWidget)client_data, 
		      DtEDITOR_HELP_STATUS_TOTAL_LINES );
}

/* ARGSUSED */
static void
HelpStatusMessageCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    CallHelpCallback( (DtEditorWidget)client_data, 
		      DtEDITOR_HELP_STATUS_MESSAGE );
}

/* ARGSUSED */
static void
HelpStatusOverstrikeCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    CallHelpCallback( (DtEditorWidget)client_data, 
		      DtEDITOR_HELP_STATUS_OVERSTRIKE );
}

/****
 * Format Settings dialog help callbacks
 */

/* ARGSUSED */
static void
HelpFormatDialogCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    CallHelpCallback((DtEditorWidget)client_data, DtEDITOR_HELP_FORMAT_DIALOG);
}

/* ARGSUSED */
static void
HelpFormatRightMarginCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
  CallHelpCallback( (DtEditorWidget)client_data, 
		    DtEDITOR_HELP_FORMAT_RIGHT_MARGIN);
}

/* ARGSUSED */
static void
HelpFormatLeftMarginCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    CallHelpCallback( (DtEditorWidget)client_data, 
		      DtEDITOR_HELP_FORMAT_LEFT_MARGIN);
}

/* ARGSUSED */
static void
HelpFormatJustifyButtonsCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    CallHelpCallback( (DtEditorWidget)client_data, 
		      DtEDITOR_HELP_FORMAT_ALIGNMENT);
}

/****
 * Find/Change & Spell dialogs help callbacks
 */

/* ARGSUSED */
void
_DtEditorHelpSearchCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    DtEditorWidget editor = (DtEditorWidget)client_data;

    switch (M_search_dialogMode(editor))
    {
       case SPELL:
          CallHelpCallback( (DtEditorWidget)client_data, 
			    DtEDITOR_HELP_SPELL_DIALOG);
          break;
       case REPLACE:
       default:
          CallHelpCallback( (DtEditorWidget)client_data, 
			     DtEDITOR_HELP_CHANGE_DIALOG);
          break;
    }
} /* _DtEditorHelpSearchCB */

/* ARGSUSED */
void
_DtEditorHelpSearchFindCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    CallHelpCallback( (DtEditorWidget)client_data, 
		      DtEDITOR_HELP_CHANGE_FIND);
}
/* ARGSUSED */
void
_DtEditorHelpSearchChangeCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    DtEditorWidget editor = (DtEditorWidget)client_data;

    switch (M_search_dialogMode(editor))
    {
       case SPELL:
          CallHelpCallback( (DtEditorWidget)client_data, 
			    DtEDITOR_HELP_SPELL_CHANGE);
          break;
       case REPLACE:
       default:
          CallHelpCallback( (DtEditorWidget)client_data, 
			     DtEDITOR_HELP_CHANGE_CHANGE);
          break;
    }
} /* _DtEditorHelpSearchChangeCB */

/* ARGSUSED */
void
_DtEditorHelpSearchSpellCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    CallHelpCallback( (DtEditorWidget)client_data, 
		      DtEDITOR_HELP_SPELL_MISSPELLED_WORDS);
}

/*********************************************************************
 *
 * The following section contains the procedures related to formating
 * text and the Format Settings dialog.
 *
 *********************************************************************/

/*
 * Do simple formatting of paragraphs of text.
 * Designed for speed, at some cost in complexity and redundancy.
 *
 * There is a minor undocumented bug in Dump().  It calls Justify() in the
 * case where the last word of the last line of a paragraph is an end-of-
 * sentence word (typical) and ends just one blank before the margin
 * (rare).  This results in one blank being inserted in the line when it's
 * not necessary.  It happens because the two trailing blanks after the
 * word cause an "ordinary" line dump before Fill() sees the next line and
 * knows it has an end of paragraph.  WARNING:  The situation would be
 * aggravated if FillWord() ever set blanks to an even larger number.
 */

/*
** This section contains changes for EUC_4B compatibility.
** WPI interface is used.
 */

/* Global Variables and extern declarations */

static wchar_t Blank;
static int mbcodeset;		/* Variable set to true if MB_CUR_MAX > 1 */

static wctype_t	ekinclass;	/* Prop value for "ekinsoku" class */
static wctype_t	bekinclass;	/* Prop value for "bekinsoku" class */
static wctype_t	blnkclass;	/* Prop value for "Blank" Class */
static AdjRecPtr pAdj;

#define isekinsoku(wc)	 iswctype((wc),ekinclass)
#define isbekinsoku(wc)	 iswctype((wc),bekinclass)

/************************************************************************
 * MISC GLOBALS:
 */

#define	proc				/* null; easy to grep for procs */
#define	false	   0
#define	true	   1


/************************************************************************
 * FORMATTING CONTROL:
 */

static int m0flag;
static long fillmargin;
static long centermiddle;
static int tabsize;      		/* tab size in use  */
static	 int	centerstartpara;	/* expect para. start?	in "Center" */

/*
 * Global/static vars used in "Fill".
 */
#define	LINESIZE BUFSIZ		/* how big a line can be */
#define	WORDMAX LINESIZE	/* see above for discussion */
static wint_t	*wordbuf;	/* see comments for "Fill" for discussion */
static wint_t	*wordcp;
static wint_t	*wordcpwrap;

typedef struct _WORD{		/* describe one word		*/
	wint_t	*cp;		/* word location in buffer	*/
	int	inlinenum;	/* word start line in input	*/
	int	incolumn;	/* word start column in input	*/
	int	length;		/* size of word			*/
	int	score;		/* word + next for jflag	*/
	int	blanks;		/* output trailing blanks need	*/
	int	wclen;		/* Length of wide character word */
} WORD;

static WORD	*word;
static WORD	*wordbase;
static WORD	*wordlimit;
#define WORDNEXT(at) { if (++at == wordlimit) at = wordbase; }	/* statement */
#define WORDPREV(at) (((at == wordbase) ? wordlimit : at) - 1)	/* function  */
static WORD	*wordfirst;
static WORD	*wordat;
static WORD	*wordnext;

static int	inlinenum;
static int	incolumn;
static int	outlinenum;

static int	pendchars;
static int	indent1, indent2;

#include <locale.h>
extern int _nl_space_alt;


/*
 * Forward declarations
 */
proc static int DoLine ();
proc static int Center ();
proc static int Fill ();
proc static int FillWord ();
proc static void Dump ();
proc static int PrintIndent ();
proc static int PrintTag ();
proc static int PrintWords ();
proc static void Justify ();
proc static int CompareWords ();
static int	EkinBekinProc(
			WORD	**wordpast);
static int	postEkinBekinProc();

#if defined(sun)
/*
 *  A small workaround for systems that don't have wcwidth...
 */
static int
sun_wcwidth(
	const wchar_t wc)
{
    int status;
    char mbstr[MB_LEN_MAX + 1];

    status = wctomb(mbstr, wc);

    if (status > 0)
       return(euccol(mbstr));
    else
       return(status);
}
#endif /*  sun  */

/*
 * The following utilities: lineBlank, findNonBlank, and countBlanks all
 * require the lineLen parameter to specify the number of characters, not
 * bytes, in the line.
 */

static Boolean
lineBlank(
	char *pLine, 
	int lineLen)
{
    int byteNum, charNum, mcnt;
    wint_t wc;
    size_t mbCurMax = MB_CUR_MAX;

    for(byteNum = charNum = 0; charNum < lineLen; charNum++)
    {
	(void) mbtowc(&wc, &pLine[byteNum], mbCurMax);
#if !(defined(sun) && (_XOPEN_VERSION==3))
	if( !iswctype(wc, _DtEditor_blankClass) )
	    return False;
#else
	if( !iswblank(wc) )
	    return False;
#endif /* not sun */

	/* calculete byteNum outside of loop for mblen check */
	if ((mcnt= mblen(&pLine[byteNum], mbCurMax)) < 0) mcnt = 1;
	byteNum += mcnt;
    }
    return True;
}

static char *
findNonBlank(
	char *pLine, 
	int lineLen)
{
    int byteNum, charNum, mcnt;
    wint_t wc;
    size_t mbCurMax = MB_CUR_MAX;

    for(byteNum = charNum = 0; charNum < lineLen; charNum++)
    {
	(void) mbtowc(&wc, &pLine[byteNum], mbCurMax);
#if !(defined(sun) && (_XOPEN_VERSION==3))
	if( !iswctype(wc, _DtEditor_blankClass) )
            return &pLine[byteNum];
#else
	if(! iswblank(wc) )
            return &pLine[byteNum];
#endif /* not sun */
	/* calculete byteNum outside of loop for mblen check */
	if ((mcnt= mblen(&pLine[byteNum], mbCurMax)) < 0) mcnt = 1;
	byteNum += mcnt;
    }
    return &pLine[byteNum];
}

static int
countBlanks(
	char *pLine, 
	int lineLen)
{
    int byteNum, charNum, count, mcnt;
    wint_t wc;
    size_t mbCurMax = MB_CUR_MAX;

    for(byteNum = charNum = 0, count = 0; charNum < lineLen; charNum++)
    {
	(void) mbtowc(&wc, &pLine[byteNum], mbCurMax);
	if(iswcntrl(wc))
	{
	    if(mblen(&pLine[byteNum], mbCurMax) == 1)
	    {
               switch((int) pLine[byteNum]) 
	       {
                   /*
                    * I don't really know what to do with weird embedded chars.
                    * This is just a guess.  In non-ascii locales this could
		    * screw up, but I don't know how else to deal with
		    * weird embedded control characters.
                    */
                   case '\n': /* newline */
                   case '\f': /* form feed */
                       count++;
                       continue;
                   case '\t': /* horizontal tab */
                       count += 8;
                       continue;
                   case '\b': /* backspace */
                       count--;
                       continue;
                   case '\r': /* carriage return */
                       count = 0;
                       continue;
                   default:
                       return count;
               }
	    }
	    else
	        count++; /* multibyte control chars??? */
	    continue;
	}
#if !(defined(sun) && (_XOPEN_VERSION==3))
	if(!iswctype(wc, _DtEditor_blankClass))
            return count;
#else
	if(! iswblank(wc) )
            return count;
#endif /* not sun */

	/* calculete byteNum outside of loop for mblen check */
	if ((mcnt= mblen(&pLine[byteNum], mbCurMax)) < 0) mcnt = 1;
	byteNum += mcnt;

	count++;
    }
    return count;
}

/*
 * fixLeftMarginAndNewlines - writes out a substring from the text widget,
 * inserting leading blanks as needed to set the left margin, and adding
 * newlines at all "virtual" (i.e. word-wrapped) lines.  It assumes that the
 * specified substring is an integer number of lines - i.e. the substring
 * cannot begin or end in the middle of a line.  Non-complying substrings
 * are expanded to encompass whole lines.
 */
static void
fixLeftMarginAndNewlines(
        DtEditorWidget editor,
	FILE *fp,
	int leftMargin,
	int rightMargin,
	int startPos,
	int endPos)
{
    Widget widget = M_text(editor);
    register XmTextLineTable lineTable;
    register int i;
    int lineIndex, lineLen, nextLen, lineIndent, lastIndex,
        lineByteLen, total_lines, mcnt;
    Boolean newPara = True, formatLast = False;
    char *pLine, *pNext, *pFirstNonBlank, *pLastChar;
    size_t mbCurMax = MB_CUR_MAX;

    lineTable = _XmTextGetLineTable(widget, &total_lines);

    if (leftMargin > rightMargin || leftMargin < 0) 
      leftMargin = 0;

    lineIndent = leftMargin;

    /*
     * calculate the starting line number and ending line number
     * for the [sub]string we're operating upon.
     */
    lineIndex = _DtEditorGetLineIndex((XmTextWidget)M_text(editor), startPos);

    lastIndex = _DtEditorGetLineIndex((XmTextWidget)M_text(editor), endPos);

    /* sun fix 4010948: we're checking to make sure that we only format
       the last line if we're formatting all of the text or only
       formatting the last paragraph */
    if (lineIndex == lastIndex)
	formatLast = True;
    else if ((lastIndex+1) == total_lines)
    {
	int _len = lineTable[lastIndex].start_pos - 
	    lineTable[lastIndex-1].start_pos;
        char *_line = _XmStringSourceGetString((XmTextWidget)M_text(editor), 
			       lineTable[lastIndex-1].start_pos, 
			       lineTable[lastIndex].start_pos, False);
	int last = XmTextGetLastPosition(widget);

	/* Check if lastIndex begins paragraph */
	if (!lineBlank(_line, _len))
	    formatLast = True;

	if ((startPos == 0) && (endPos == last))
	    formatLast = True;
    }

    
    /*
     * This loop terminates with one line unwritten.  This is correct
     * if we're not formatting the last line of the file, as that means
     * that we must be formatting a paragraph, and the paragraph adjust
     * code has trouble locating the end of a paragraph (as opposed to the
     * beginning of the next paragraph.  If the last line of the text is
     * to be formatted, it is handled after this loop terminates.
     */
    for(; lineIndex < lastIndex; lineIndex++)
    {
	/*
	 * if the current line was word-wrapped, put out a nl.
	 */
        if(lineTable[lineIndex].virt_line)
            fwrite("\n", sizeof(char), 1, fp);

	lineLen = lineTable[lineIndex + 1].start_pos - 
		     lineTable[lineIndex].start_pos;
        pLine = _XmStringSourceGetString((XmTextWidget)M_text(editor), 
		    lineTable[lineIndex].start_pos, 
		    lineTable[lineIndex + 1].start_pos, False);
	pLastChar = _DtEditorGetPointer(pLine, lineLen - 1);
	if ((mcnt = mblen(pLastChar, mbCurMax)) < 0) mcnt = 0;
	lineByteLen = pLastChar - pLine + mcnt;

        if(lineBlank(pLine, lineLen))
	{
	    /*
	     * Blank lines indicate the start of a new paragraph.
	     * They also don't require any indent adjustment so are
	     * quick/easy to write out.
	     */
	    fwrite( pLine, sizeof(char), 
		    _DtEditorGetPointer(pLine, lineLen) - pLine, fp );
	    newPara = True;
	    XtFree(pLine);
	    continue;
	}
	if(newPara == True)
	{
	    int firstIndent;
	    /*
	     * Calc the proper indent for the first line, and
	     * the indent for the rest of the lines
	     * in the paragraph.  If there's only one line in the
	     * paragraph, then the proper indent is adjustStuff.left_margin.
	     */

	    /*
	     * Get the next line.
	     */
	    if((total_lines - lineIndex) > 2) 
	        nextLen = lineTable[lineIndex + 2].start_pos -
                          lineTable[lineIndex + 1].start_pos;
            else
                nextLen = XmTextGetLastPosition(widget) - 
                                        lineTable[lineIndex + 1].start_pos;
	    pNext = _XmStringSourceGetString((XmTextWidget)M_text(editor), 
		    lineTable[lineIndex + 1].start_pos, 
		    lineTable[lineIndex + 1].start_pos + nextLen, False);

	    if(lineBlank(pNext, nextLen)) /* single line paragraph */
		firstIndent = lineIndent = leftMargin;
	    else 
	    {
		int curFirstIndent, curSecondIndent, offset;

		curFirstIndent = countBlanks(pLine, lineLen);
		curSecondIndent = countBlanks(pNext, nextLen);

		offset = curFirstIndent - curSecondIndent;
		if(offset >= 0)
		{
		    /* second line is closer to the left margin */

		    /*
		     * Note, the first line will still be indented even
		     * if the remaining words in the paragraph can
		     * all fit on the first line.  The end result is a 
		     * one line indented paragraph.  I am mentioning
		     * this because some may think this is a defect.
		     */

		    firstIndent = leftMargin + offset;
		    lineIndent = leftMargin;
		}
		else
		{
		    firstIndent = leftMargin;
		    lineIndent = leftMargin - offset;
		}
	    }
	    for(i = firstIndent; i-- > 0;)
                fwrite(" ", sizeof(char), 1, fp); /* that's a _space_ */
	    pFirstNonBlank = findNonBlank(pLine, lineLen);
            fwrite(pFirstNonBlank, sizeof(char), lineByteLen - 
		   (pFirstNonBlank - pLine), fp);

	    newPara = False;
	    XtFree(pLine);
	    continue;
	}
	for(i = lineIndent; i-- > 0;)
            fwrite(" ", sizeof(char), 1, fp); /* that's a _space_ */
	pFirstNonBlank = findNonBlank(pLine, lineLen);
        fwrite(pFirstNonBlank, sizeof(char), lineByteLen - 
	       (pFirstNonBlank - pLine), fp);
	XtFree(pLine);
    }

    if(((total_lines - lineIndex) == 1) && formatLast)
    {
       /*
        * Now we have to handle the last line.
        */
       if(lineTable[lineIndex].virt_line)
           fwrite("\n", sizeof(char), 1, fp);
    
       if((total_lines - lineIndex) > 2) 
           lineLen = lineTable[lineIndex + 1].start_pos -
                     lineTable[lineIndex].start_pos;
       else
           lineLen = XmTextGetLastPosition(widget) - 
                  lineTable[lineIndex].start_pos;

       if(lineLen > 0)
       {
	   for(i = lineIndent; i-- > 0;)
               fwrite(" ", sizeof(char), 1, fp); /* that's a _space_ */

           pLine = _XmStringSourceGetString((XmTextWidget)M_text(editor), 
		    lineTable[lineIndex].start_pos, 
		    lineTable[lineIndex].start_pos + lineLen, False);
	   pLastChar = _DtEditorGetPointer(pLine, lineLen - 1);
	   if ((mcnt = mblen(pLastChar, mbCurMax)) < 0) mcnt = 0;
	   lineByteLen = pLastChar - pLine + mcnt;
           if(lineBlank(pLine, lineLen))
	   {
               fwrite(pLine, sizeof(char), lineByteLen, fp);
	   }
	   else
	   {
	       pFirstNonBlank = findNonBlank(pLine, lineLen);
               fwrite(pFirstNonBlank, sizeof(char), lineByteLen - 
	              (pFirstNonBlank - pLine), fp);
	   }
	   XtFree(pLine);
       }
    }
    XtFree((XtPointer)lineTable);
}


/************************************************************************
 * FormatText - 
 *
 * Initialize, check arguments embedded in AdjRec, open and read files,
 * and pass a series of lines to lower-level routines.
 */

static int 
FormatText ( 
	AdjRecPtr 
	pAdjRec) 

{ 
  long		maxmargin;	/* max allowed */ 
  register FILE	*filep;		/* file to read */ 
  int		retVal;

/*
 * INITIALIZE FOR NLS
 */
	mbcodeset = (MB_CUR_MAX > 1);
	mbtowc(&Blank," ",1); /* Mainly to put Blank into the wide char buffer*/

/*
 * Initialize global/statics.
 */
	if((wordbuf = malloc(sizeof(wint_t) * LINESIZE * 3)) == (wint_t *)NULL)
			return 0;
	if((word  = malloc(sizeof(WORD) * WORDMAX)) == (WORD *)NULL)
	{
		retVal = 0;
		goto CleanUp;
	}
	m0flag = 0;
	fillmargin = 72L;
	centermiddle = 40L;
	tabsize = 8;      			/* default tab size */
	centerstartpara = true;

	fillmargin = centermiddle = pAdjRec->margin;
	m0flag = (fillmargin == 0L);

	/*
	 * Initialize global/statics for "Fill".
	 */
	wordcp = wordbuf;
	wordcpwrap = wordbuf + (LINESIZE * 2);
	wordbase  = & word [0];
	wordlimit = & word [WORDMAX];
	wordfirst = & word [0];
	wordnext  = & word [0];
	inlinenum  = 0;
	outlinenum = 1;
	pendchars = 0;

/*
 * CHECK ARGUMENTS:
 */

	if (pAdjRec->cflag + pAdjRec->jflag + pAdjRec->rflag > 1)
	{
			retVal = 0;
			goto CleanUp;
	}

	maxmargin = pAdjRec->cflag? (LINESIZE / 2) : LINESIZE;
	if ((fillmargin < 0L) || (fillmargin > maxmargin))
	{
			retVal = 0;
			goto CleanUp;
	}

	if ((pAdjRec->tabsize > 0) && (pAdjRec->tabsize <= MAXTABSIZE))
		tabsize = pAdjRec->tabsize;

	/*
	** These calls determine if the particular value is true for
	** the current locale. A value of -1 is returned if the locale
	** does not support the charclass. If the locale supports the
	** charclass, the return value is passed as a parameter to the
	** iswctype call.
	*/
#if !(defined(sun) && (_XOPEN_VERSION==3))
	ekinclass = wctype("ekinsoku");
	bekinclass = wctype("bkinsoku");
	blnkclass = wctype("blank");
#endif /* end not Sun */


/*
 * OPEN AND READ INPUT FILE:
 */

	filep = pAdjRec->infp;
	pAdj = pAdjRec; /* set (file) global var to passed in rec */

	while (DoLine (filep))
		;

/*
 * DUMP LAST LINE AND EXIT:
 */

	if (! pAdj->cflag)			/* not centering	*/
	{
		Dump (true);			/* force end paragraph	*/
	}

	retVal = 1;

CleanUp:
	if(wordbuf != (wint_t *)NULL)
	{
		free(wordbuf);
		wordbuf = (wint_t *)NULL;
	}
	if(word != (WORD *)NULL)
	{
		free(word);
		word = (WORD *)NULL;
	}

	return retVal;
}


/************************************************************************
 * D O   L I N E
 *
 * Read one input line and do generic processing (read chars from input
 * to inline, crunching backspaces).  Return true if successful, else
 * false at end of input file.  This must be done before expanding tabs;
 * pass half-processed line to Center() or Fill() for more work.
 *
 * Assumes getc() is efficient.
 * inline[] is static so it is only allocated once.
 */

proc static int 
DoLine (filep)
register FILE	*filep;				/* open file to read	*/
{
	wint_t	inline [LINESIZE];	        /* after reading in	*/
	register wint_t	*incp	 = inline;	/* place in inline	*/
	register wint_t	*incplim = inline + LINESIZE;	/* limit of inline */
	register wint_t   c;

/*
 * READ LINE WITH BACKSPACE CRUNCHING:
 */

	while (incp < incplim)		/* buffer not full */
	{
		c = getwc (filep);	/* get and save wide char */
		switch(c)
		{

			case WEOF:
				if (incp == inline)	/* nothing read yet */
					return (false);

			case L'\0':			/* fall through	 */
			case L'\n':			/* end of line	 */
				incplim = incp;		/* set limit (quit loop) */
				break;

			case L'\b':			/* backspace	*/
				if (incp > inline)	/* not at start	*/
					incp--;		/* just back up	*/
				break;

			default:			/* any other char */
				*incp++ = c;
				break;
		}
	}
	/* now incplim is correctly set to last char + 1 */

/*
 * PASS LINE ON FOR MORE WORK:
 */

	if (pAdj->cflag)	Center (inline, incplim);
	else	Fill   (inline, incplim);

	return (true);				/* maybe more to read */

} /* DoLine */


/************************************************************************
 * C E N T E R
 *
 * Center text (starting at inline, ending before inlinelim).  First copy
 * chars from inline to outline skipping nonprintables and expanding tabs.
 * For efficiency (at the cost of simplicity), note and skip indentation
 * at the same time.  Then, print outline with indentation to center it.
 *
 * outline[] is static so it is only allocated once; startpara so it is
 * used for successive lines.
 */

proc static
Center (inline, inlinelim)
wint_t	*inline;			/* start of input line	*/
register wint_t	*inlinelim;		/* end of line + 1	*/
{
	register wint_t	*incp;		/* pointer in inline	*/
	wint_t	outline [LINESIZE + MAXTABSIZE];  /* after generic work	*/
	register wint_t	*outcp      = outline;	  /* place in outline	*/
	register wint_t	*outlinelim = outline + LINESIZE; /* limit of outline*/

	int	haveword  = false;	/* hit word in inline?	*/
	register wint_t	ch;		/* current working char	*/
	register int	needsp;		/* spaces need for tab	*/
	register int	indent = 0;	/* size of indentation	*/
	int	textwidth;		/* size of text part	*/

/*
 * SCAN INPUT LINE:
 */

	for (incp = inline; (incp < inlinelim) && (outcp < outlinelim); incp++)
	{

	  ch = *incp;

/*
 * SKIP WHITE SPACE:
 */

	  if (iswspace(ch))
	  {
	    needsp = ((ch != L'\t') && (iswblank(ch))) ? 
		     1 : tabsize - ((indent + outcp - outline) % tabsize);

	    if (! haveword)		/* indentation */
		indent += needsp;
	    else			/* past text */
		for ( ; needsp > 0; needsp--)
		  *outcp++ = Blank;
	  }

/*
 * CARRY OVER PRINTABLE CHARS AND NOTE INDENTATION:
 */

	  else if (iswprint (ch))	/* note: ch != ' ' */
	  {
	     *outcp++ = ch;		/* just copy it over */
	     haveword = true;
	  }

/* else do nothing, which tosses other chars */

	} /* end for */

	/* Now outcp is the new outlinelim */

/*
 * HANDLE BLANK LINE (no text):
 */

	if (! haveword)
	{
	  putc ('\n', pAdj->outfp);	/* "eat" any whitespace */
	  centerstartpara = true;	/* expect new paragraph */
	}

/*
 * EAT TRAILING BLANKS, SET TEXTWIDTH:
 */

	else
	{
	  /* note that outcp > outline */
	  while ((outcp[-1] != L'\t') && iswblank(outcp [-1]))
	    outcp--;

	  textwidth = (outcp - outline);	/* thus textwidth > 0 */

/*
 * SET AUTOMARGIN AND PRINT CENTERED LINE:
 *
 * The equations used depend on truncating division:
 *
 *		eqn. 1		eqn. 2
 *	Margin	Middle	Width	Results
 *	odd	exact	odd	exact centering
 *	odd	exact	even	extra char to right
 *	even	left	odd	extra char to left
 *	even	left	even	exact centering (due to "extra" char to right)
 *
 * When centermiddle is default or given by user, it is the same as the
 * first two lines above (middle exactly specified).
 */

	  if (centerstartpara)			/* start of paragraph */
	  {
	    centerstartpara = false;

	    if (m0flag)				/* set automargin */
	     /* 1 */	    centermiddle = (indent + textwidth + 1) / 2;
	  }

	  /* 2 */    PrintIndent (centermiddle - ((textwidth + 1) / 2));

	  {
	    int i;
 	    for(i = 0; i < textwidth; i++)
	    {
		putwc(outline[i], pAdj->outfp);
	    }
	    putc('\n', pAdj->outfp);
	  } 

	} /* else */

} /* Center */


/************************************************************************
 * WORD DATA STRUCTURES USED TO TRACK WORDS WHILE FILLING:
 *
 * This complicated scheme is used to minimize the actual data moving
 * and manipulation needed to do the job.
 *
 * Words are kept in wordbuf[] without trailing nulls.  It is large enough
 * to accomodate two lines' worth of words plus wrap around to start a new
 * word (which might be as big as a line) without overwriting old words
 * not yet dumped.  wordcp -> next free location in wordbuf[];
 * wordcpwrap -> last place a new word is allowed to start.
 *
 * Words are pointed to and described by word[] structures.  The array is
 * big enough to accomodate two lines' worth of words, assuming each word
 * takes at least two characters (including separator).  wordbase and
 * wordlimit are the bounds of the array.  wordfirst remembers the first
 * word in the array not yet printed.  wordat is the word being worked on
 * after wordnext is advanced (and maybe wrapped around).  New elements
 * are "allocated" using wordnext, a pointer that is wrapped around as
 * needed.
 *
 * inlinenum and incolumn track the current input line and column per
 * paragraph.  outlinenum tracks the next line to print.  All are base 1,
 * but inlinenum is preset to zero at the start of each paragraph.
 *
 * pendchars tracks the number of dumpable characters accrued so far, to
 * trigger dumping.  indent1 and indent2 remember the indentations seen
 * for the first two lines of each paragraph.
 */

/************************************************************************
 * F I L L
 *
 * Parse an input line (inline to inlinelim) into words and trigger
 * FillWord() as needed to finish each word, which in turn can call
 * Dump() to dump output lines.  This routine sacrifices simplicity and
 * clarity for efficiency.  It uses shared global word data except
 * outlinenum and pendchars.
 */

proc static
Fill (inline, inlinelim)
wint_t	*inline;		/* start of input line	*/
register wint_t	*inlinelim;	/* end of line + 1	*/
{
	register wint_t	*incp;		/* place in inline	*/
	register wint_t	ch;		/* current working char	*/
	int	haveword = false;	/* hit word in inline?	*/
	register int	inword	 = false;	/* currently in a word?	*/
/*
 * SCAN INPUT LINE:
 */

	inlinenum++;
	incolumn = 1;				/* starting a line */

	for (incp = inline; incp < inlinelim; incp++)
	{
	  ch = *incp;

/*
 * AT WHITE SPACE; FINISH PREVIOUS WORD if any, then skip white space:
 */

	  if (iswspace(ch))
	  {
	    if (inword)		/* was skipping a word */
	    {
		inword = false;
		FillWord();
	    }

	    incolumn += ((ch != L'\t') && (iswblank(ch))) ?
				1 : tabsize - ((incolumn - 1) % tabsize);
	  }

/*
 * AT PART OF WORD; START NEW ONE IF NEEDED:
 */

	  else if (iswprint (ch))
	  {
  	    if (! inword)		/* start new word */
	    {
		inword = true;

		if (wordcp > wordcpwrap)/* past wrap point   */
		  wordcp = wordbuf;/* wraparound buffer */

		wordat = wordnext;
		WORDNEXT (wordnext);

		wordat->cp = wordcp;	/* note start of word */
		wordat->inlinenum = inlinenum;	/* note input line    */
		wordat->incolumn  = incolumn;	/* note input column  */
		wordat->wclen = 0;

		if (! haveword)
		{
		  haveword = true;
	
		  switch (inlinenum)/* note indentations */
		  {
		    case 1:	
			indent1 = incolumn - 1; 
			break;
		    case 2:	
			indent2 = incolumn - 1; 
			break;
		  }
		}
	    }

/*
 * SAVE ONE CHAR OF WORD:
 */
	    *wordcp++ = ch;
	    {
		int n = wcwidth(ch);

		if (n > 0) 
		{ /* == 0 for a null char */
		  incolumn += n; 
		  wordat->wclen++;
		}
	    }

	  } /* else */

/* else skip other chars by doing nothing */

	} /* for */

/*
 * END OF LINE; HANDLE BLANK LINE, OR FINISH WORD AND AUTOMARGIN:
 */

	if (! haveword)			/* no text on line */
	{
	  inlinenum--;			/* don't count empty lines */
	  Dump (true);			/* force end paragraph */
	  fputc ('\n', pAdj->outfp);	/* put this empty line */
	  inlinenum = 0;		/* start new paragraph */
	}
	else				/* have text on line */
	{
	  if (inword)			/* line ends in word */
		FillWord();
	  if (pendchars > fillmargin)
	  {
	      Dump (false);		/* not end of paragraph */
	  }
	  
	  if (m0flag && (inlinenum == 1)) /* need to note right margin */
		fillmargin = wordat->incolumn + wordat->length - 1;
	}
} /* Fill */


/************************************************************************
 * F I L L   W O R D
 *
 * Save values for the word just finished and dump the output line if
 * needed.  Uses shared global word values, but does not touch outlinenum,
 * and only increments pendchars.
 *
 * Trailing blanks (1 or 2) needed after a word are figured here.
 * wordlen is the total length (nonzero) and wordcp points to the char
 * past the end of the word.  Two blanks are needed after words ending in:
 *
 *	<terminal>[<quote>][<close>]
 *
 * where <terminal> is any of	. : ? !
 *	 <quote>    is any of	' "
 *	 <close>    is any of	) ] }
 *
 * For efficiency, this routine avoids calling others to do character
 * matching; it does them in line.
 */

proc static
FillWord ()
{
  int	wordlen;		/* length of word to fill */
  int	blanks = 1;		/* trailing blanks needed */
  register wint_t	ch1, ch2, ch3;		/* last chars of word	  */

  wordat->length = (incolumn - wordat->incolumn);
  wordlen = wordat->wclen;

  /*
   * CHECK FOR SPECIAL END OF WORD:
   */

  ch3 = wordcp [-1];

  if ((ch3 == L'.') || (ch3 == L':') || (ch3 == L'?') || (ch3 ==L'!'))
  {
	blanks = 2;			/* <terminal> */
  }

  else if (wordlen >= 2)
  {
    ch2 = wordcp [-2];

    if( ((ch2 == L'.')  || (ch2 == L':') || (ch2 ==L'?') || (ch2 == L'!')) &&
        ((ch3 == L'\'') || (ch3 == L'"') || (ch3 ==L')')  || (ch3 == L']') 
	 || (ch3 == L'}'))
      )
    {
      blanks = 2;		/* <terminal><quote or close> */
    }
    else if (wordlen >= 3)
    {
      ch1 = wordcp [-3];

      if( ((ch1 == L'.')  || (ch1 == L':') || (ch1 == L'?') || (ch1 == L'!'))
	  && ((ch2 == L'\'') || (ch2 == L'"'))
	  && ((ch3 == L')')  || (ch3 == L']') || (ch3 == L'}')) )
      {
	blanks = 2;		/* <terminal><quote><close> */
      }
    }

  } /* else */

/*
 * SAVE VALUES
 */

    pendchars += wordlen + (wordat->blanks = blanks);

} /* FillWord */


/************************************************************************
 * D U M P
 *
 * Print output line(s), with all necessary indentation and formatting,
 * if required (at end of paragraph) or if permissible.  If required,
 * indent1 is used if indent2 is not set yet (not on second line of
 * input).  Otherwise, if not at end of paragraph, dumping is only
 * permissible after beginning the second input line, so fillmargin and
 * indent2 are known, so tagged paragraphs are done right.
 *
 * Whenever dumping, all "full" lines are dumped, which means more than
 * just one may be printed per call.  jflag or rflag formatting is
 * applied to all lines, except that jflag is ignored for the last line
 * of each paragraph.
 *
 * Uses shared global word data, but does not touch inlinenum, incolumn,
 * indent1, or indent2.  
 */

proc static
void Dump (endpara)
int	endpara;			/* end of paragraph? */
{
  int	haveindent2 = (inlinenum >= 2);	/* indent2 known?	*/
  int	startpara;			/* start of paragraph?	*/
  int	normal;				/* non-tagged line?	*/
  WORD	*wordpast = wordfirst;		/* past last to dump	*/
  register int	wordlen;		/* length of one word	*/
  int	indent;				/* local value		*/
  register int	outneed;		/* chars need to dump	*/
  int	outchars;			/* chars found to dump	*/

/*
 * IF DUMPING NEEDED, DUMP LINES TILL DONE:
 */

   if (! (endpara || haveindent2))			/* not time to dump */
      return;

   while (    (pendchars > fillmargin) 			/* line is full */
	   || (endpara && (wordfirst != wordnext)) )	/* more to dump */
   {
      startpara = (outlinenum < 2);
      indent    = (startpara || (! haveindent2)) ? indent1 : indent2;

/*
 * CHECK FOR TAGGED PARAGRAPH if needed:
 */

      normal = true;				/* default == no tag */

      if (startpara && haveindent2 && (indent1 < indent2))
      {
	int	incol2 = indent2 + 1;	/* column needed */

	while (   (wordpast != wordnext)	/* more words */
	       && (wordpast->inlinenum == 1))	/* from line 1 */
	{
	   if (wordpast->incolumn == incol2)	/* bingo */
	   {
	      normal = false;
	      break;
	   }
	   WORDNEXT (wordpast);
	}

	if (normal)
	  wordpast = wordfirst;		/* reset value */

/*
 * PRINT TAG PART OF TAGGED PARAGRAPH:
 */

	else
	{
	  WORD *wordat = wordfirst;	/* local value */

	  while (wordat != wordpast)	/* decrement pendchars */
	  {
	     pendchars -= wordat->length + wordat->blanks;
	     WORDNEXT (wordat);
	  }
				
	  PrintIndent (indent);
	  PrintTag  (wordpast);	/* preceding words   */
	  wordfirst = wordpast;	/* do rest of line   */
	  indent    = indent2;	/* as if second line */
	}
      } /* if */

 /*
  * FIND WORDS WHICH FIT ON [REST OF] LINE:
  */

      if (indent >= fillmargin)		/* don't over indent */
	indent  = fillmargin - 1;

      outneed   = fillmargin - indent;	/* always greater than zero */
      outchars  = 0;
      wordlen   = wordpast->length;

      do {				/* always consume one */
	   outchars += wordlen + wordpast->blanks;
	   WORDNEXT (wordpast);
      }	while (    (wordpast != wordnext)	/* not last word */
		&& (outchars + (wordlen = wordpast->length) <= outneed)
		);
      /* next will fit */


      /*
      ** BEKINSOKU/EKINSOKU PROCESSING
      */
      if( EkinBekinProc(&wordpast) )
        postEkinBekinProc(&outchars,wordpast);
      pendchars -= outchars;		/* pendchars to consume */

      /* from now on, don't include trailing blanks on last word */
      outchars -= (WORDPREV (wordpast) -> blanks);

      /* now wordfirst and wordpast specify the words to dump */
/*
 * PRINT INDENTATION AND PREPARE JUSTIFICATION:
 */

      if (pAdj->rflag)			/* right-justify only */
      {
	if (normal)		/* nothing printed yet */
	  PrintIndent (fillmargin - outchars);
	else	/* indent + tag printed */
	{
	int blanks = fillmargin - outchars - indent;

	while (blanks-- > 0)		/* can't use PrintIndent()*/
	  putwc(Blank, pAdj->outfp);
	}
      }
      else
      {
	if (normal)			/* not already done */
	  PrintIndent (indent);

	if (pAdj->jflag && ! (endpara && (wordpast == wordnext)))
	  Justify (outneed - outchars, wordpast);
      }

/*
 * PRINT REST OF LINE:
 */

      PrintWords (wordpast);
      putwc('\n', pAdj->outfp);
      wordfirst = wordpast;
      outlinenum++;				/* affects startpara */

   } /* while */

   if (endpara)
     outlinenum = 1;

} /* Dump */


/************************************************************************
 * P R I N T   I N D E N T
 *
 * Print line indentation (if > 0), optionally using tabs where possible.
 * Does not print a newline.
 */

proc static
PrintIndent (indent)
int	indent;			/* leading indentation */
{
   if (indent > 0)		/* indentation needed */
   {
      if (! pAdj->bflag)	/* unexpand leading blanks */
      {
	 while (indent >= tabsize)
         {
	   putc ('\t', pAdj->outfp);
	   indent -= tabsize;
         }
      }
   fprintf (pAdj->outfp, "%*s", indent, "");/*[remaining] blanks */
   }
} /* PrintIndent */


/************************************************************************
 * P R I N T   T A G
 *
 * Print paragraph tag words from word[] array beginning with (global)
 * wordfirst and ending before (parameter) wordpast, using input column
 * positions in each word's data.  Assumes indentation of indent1 was
 * already printed on the line.  Assumes *wordpast is the next word on
 * the line and its column position is valid, and appends spaces up to
 * the start of that word.   Doesn't print a newline.
 *
 * Line indentation must already be done, as this routine doesn't know
 * how to print leading tabs, only blanks.
 */

proc static
PrintTag (wordpast)
WORD	*wordpast;		/* past last to print */
{
   register WORD	*wordat = wordfirst;	/* local value	  */
   register int	outcol = indent1 + 1;	/* next column	  */
   int	wordcol;		/* desired column */
   register wint_t	*wordcp;		/* place in word  */
   wint_t	*wordcplim;		/* limit of word  */

   while (true)				/* till break */
   {
     wordcol = wordat->incolumn;

        while (outcol < wordcol)/* space over to word */
        {
           putwc(Blank, pAdj->outfp);
           outcol++;
        }

        if (wordat == wordpast)		/* past last word */
	   break;			/* quit the loop  */

        wordcp = wordat->cp;

        wordcplim = wordcp + wordat->wclen;

        while (wordcp < wordcplim)		/* print word */
	   putwc(*wordcp++, pAdj->outfp);

	outcol += wordat->length;
  	WORDNEXT (wordat);
   }
} /* PrintTag */


/************************************************************************
 * P R I N T   W O R D S
 *
 * Print words from word[] array beginning with (global) wordfirst and
 * ending before (parameter) wordpast, using word sizes and trailing
 * blanks (except for last word on line).  Doesn't print a newline.
 */

proc static
PrintWords (wordpast)
WORD	*wordpast;		/* past last to print */
{
   register WORD	*wordat = wordfirst;	/* local value   */
   register wint_t	*wordcp;		/* place in word */
   wint_t	*wordcplim;		/* limit of word */
   register int	blanks;			/* after a word	 */

   while (true)				/* till break */
   {
     wordcp    = wordat->cp;
     wordcplim = wordcp + wordat->wclen;
     blanks    = wordat->blanks;	/* set before do WORDNEXT() */

     while (wordcp < wordcplim)		/* print word */
     {
	putwc(*wordcp++, pAdj->outfp);
     }

     wordat->wclen = 0;
     WORDNEXT (wordat);

     if (wordat == wordpast)		/* just did last word */
	break;

     while (blanks-- > 0)		/* print trailing blanks */
	putwc(Blank, pAdj->outfp);
   }

} /* PrintWords */


/************************************************************************
 * J U S T I F Y
 *
 * Do left/right justification of [part of] a line in the word[] array
 * beginning with (global) wordfirst and ending before (parameter)
 * wordpast, by figuring where to insert blanks.
 *
 * Gives each word (except the last on the line) a score based on its
 * size plus the size of the next word.  Quicksorts word indices into
 * order of current trailing blanks (least first), then score (most
 * first).  Cycles through this list adding trailing blanks to word[]
 * entries such that words will space out nicely when printed.
 *
 * sort[] and words are global so they are accessible to a debug routine;
 * also, sort[] is only allocated once.
 */

WORD	*sort [WORDMAX];		/* sorted pointers */
int	words;				/* words in sort[] */

proc static
void Justify (blanks, wordpast)
register int	blanks;			/* blanks to insert   */
WORD	*wordpast;		/* past last to print */
{
   register WORD	*wordat;		/* local value		*/
   register int	sortat;			/* place in sort[]	*/
   register int	wordlen;		/* size of this word	*/
   register int	nextlen;		/* size of next word	*/
   int	level;			/* current blanks level	*/
   int	CompareWords();		/* for qsort()		*/

   wordat = WORDPREV (wordpast);		/* last word on line */

   if ((blanks < 1) || (wordat == wordfirst))
      return;				/* can't do anything */

   /*
 * COMPUTE SCORES FOR WORDS AND SORT INDICES:
 *
 * March backwards through the words on line, starting with next to last.
 */

   words	= 0;
   nextlen	= wordat->length;		/* length of last word */

   do {					/* always at least one */
      wordat  = WORDPREV (wordat);
      wordlen = wordat->length;
      wordat->score = wordlen + nextlen;	/* this plus next */
      nextlen = wordlen;
      sort [words++] = wordat;		/* prepare for sorting */
   }	while (wordat != wordfirst);

   qsort ((wint_t *) sort, words, sizeof (WORD *), CompareWords);

/*
 * ADD TRAILING BLANKS TO PAD OUT WORDS:
 *
 * Each pass through the sorted list adds one trailing blank to each word
 * not already past the current level.  Thus all one-blank words are brought
 * up to two; then all twos up to three; etc.
 */

   level = 0;

   while (true)					/* till return */
   {
      level++;

      for (sortat = 0; sortat < words; sortat++)
      {
	wordat = sort [sortat];
	if (wordat->blanks > level)		/* end of this level */
	  break;				/* start next level  */

	wordat->blanks++;
	if (--blanks <= 0)			/* no more needed */
	  return;
      }
   }

} /* Justify */


/************************************************************************
 * C O M P A R E   W O R D S
 *
 * Compare two word[] entries based on pointers to (WORD *), as called
 * from qsort(3).  Tell which entry has priority for receiving inserted
 * blanks (least trailing blanks first, then highest scores), by returning
 * -1 for the first entry, +1 for second entry, or 0 if no difference.
 * (-1 literally means first entry < second entry, so it sorts first.)
 */

proc static int
CompareWords (wordp1, wordp2)
WORD	**wordp1, **wordp2;	/* pointers to (WORD *) */
{
   WORD	*word1	= *wordp1;		/* (WORD *) pointers */
   WORD	*word2	= *wordp2;
   int	blanks1	= word1->blanks;	/* trailing blanks */
   int	blanks2	= word2->blanks;

   if (blanks1 == blanks2)		/* commonest case */
   {
      int	score1 = word1->score;	/* word scores */
      int	score2 = word2->score;

      if (score1 > score2) return (-1);	/* word1 has priority */
	else if (score1 < score2)	return ( 1);
	else	return (0);
   }
   else if (blanks1 < blanks2)	return (-1);	/* word1 has priority */
     else return ( 1);

} /* CompareWords */


/*
** BKINSOKU and EKINSOKU processing for Japanese text
**
** The function scans the wordlist that are ready for printing
** and updates the "wordpast" pointer based on the rules of
** BKINSOKU and EKINSOKU characters.
**
** The code does not split a line just after a "gyomatu kinsoku" 
** character and just before a "gyoto kinsoku" character.
** "Gyomatsu kinsoku" (e-kinsoku) are characters which cannot be 
** placed at the end of a line.  "Gyoto kinsoku" (b-kinsoku) are
** characters which can not be placed at the top of a line.
**
** Global variables used:
**	wordfirst	- beginning the word list (buffer of pending lines)
**	wordnext	- next available spot in the pending buffer (i.e. 
**			  end of the buffer).
**
** Global variables modified:
**	None.
**	
** Return Values:
**	0 - No Post processing is required.
**	1 - Post processing is required
*/

#if !(defined(sun) && (_XOPEN_VERSION==3))
static int
EkinBekinProc(
	WORD	**wordpast)
{
   WORD	*curword,*compword,*prevword;
   int	ekinflag = false, margin;

   /*
    * curword points to the last word of the first line.
    * compword points to the first word of the next line or to the next
    * available spot if there is no next line.
    */
   compword = *wordpast;
   curword = WORDPREV(compword);

   /*
    * If the length of the current word is > fillmargin
    * or it is the only word then return.
    */
   if (curword->length > fillmargin || (curword == wordfirst))
	return false;

   /*
    * EKINSOKU processing
    * If this character set supports ekinclass (can't end a line) char,
    * start at the end of the line and search back until 
    * 1) we find a non-ekinsoku character, or 
    * 2) come to the beginning of the pending buffer.
    */
   if (ekinclass) 
   {
      while (isekinsoku(*(curword->cp+curword->wclen-1))) 
      {
	ekinflag = true;
	if (curword == wordfirst)	/* Boundary Condition */
	  break;
	curword = WORDPREV(curword);
      }

      /*
       * Post EKIN processing (i.e. we found an ekinsoku character in 
       * the line before finding an non-ekinsoku).
       */
      if (ekinflag) {

	 if (curword != wordfirst) {
	    /* 
	     * We found a non-ekinsoku after we found the ekinsoku.
	     * Move the end of line to just after the non-ekinsoku.
	     */
	    WORDNEXT(curword);
	    *wordpast = curword;
	    return true;
	 }
	 else { /* Boundary condition */
	    /*
	     * Reached the beginning of the buffer without finding a
	     * non-ekinsoku.  If the last word in the line can begin
	     * a line (i.e. non-bkinsoku) make it the first word of 
	     * the next line.
	     */
	    curword = WORDPREV(compword);
	    if (!isbekinsoku(*curword->cp)) {
	       *wordpast = curword;
	       return true;
	    }
	    return false;
	 }
      } 
   } 

   /* 
    * If we reached this point then:
    * 1) the character set does not support ekinclass characters, or
    * 2) the last character in the line is non-ekinsoku.
    * Now, need to see if we can begin the next line with the first 
    * character on that line.
    */

   /*
    * BEKINSOKU processing
    * If this character set supports bekinclass (can't begin a line) char,
    * search forward from the begining of the next line (curword) to the
    * end of the buffer (wordnext) until:
    * 1) A non-bekinsoku word is found,
    * 2) if the line limit exceeds the set value, or
    * 3) we reach the end of the buffer.
    */

   if (bekinclass) 
   {
      /*
       * compword points to the first word of the next line.
       */
      curword = compword;
      margin = indent2;

      while (curword != wordnext) 
      {
         if (!isbekinsoku(*curword->cp))
           break;
         margin += (curword->length + curword->blanks);
         if (margin >= fillmargin)
            return false;
         WORDNEXT(curword);
      }

      if (curword != wordnext && curword != compword) 
      {
	/* 
	 * The first word of the second line is bekinsoku so search
	 * backwards from end of first line until we are:
	 * 1) not at the first word of the buffer, and
	 * 2) not in between either an ekinsoku or bekinsoku character.
	 */ 

        /*
         * compword points to the first word of the next line.
         * curword points to the last word of the first line.
         */
        curword = WORDPREV(compword);
        prevword = WORDPREV(curword);
        while ( (curword != wordfirst) && 
		(isbekinsoku(*curword->cp) || 
		 isekinsoku(*(prevword->cp+prevword->wclen-1)))
	      ) 
        {
   	   margin += curword->length + curword->blanks;
 	   if (margin >= fillmargin)
	     return false;
	   curword = prevword;
	   prevword = WORDPREV(curword);
        }

        if (curword != wordfirst) {
	  /* 
	   * Did not reach the beginning of the buffer so we are between
	   * two non-ekinsoku & non-bekinsoku characters.  Move the end
	   * of the first line here.
	   */

          *wordpast = curword;
          return true;
        }

      }
      return false;
   }

   return false;

} /* end EkinBekinProc */

#else

static int
EkinBekinProc(
	WORD	**wordpast)
{
   WORD	*curword,*compword,*prevword,*nextword;
   int	margin = indent2;

   /*
    * prevword points to the last word of the first line.
    * compword & curword point to the first word of the next line or
    * to the next available spot in the buffer if there is no next line.
    */
   compword = *wordpast;
   curword = compword;
   prevword = WORDPREV(curword);

   /*
    * If the length of the previous word is > fillmargin
    * or it is the only word in the line then return.
    * wordfirst points to the beginning of the buffer of pending lines.
    */
   if (prevword->length > fillmargin || (prevword == wordfirst))
	return false;

   /*
    * Starting at the beginning of the next line, search backwards
    * until:
    * 1) we find two words we can split between lines, or
    * 2) we reach the beginning of the buffer.
    * 
    * If there is no next line start with the last two words of this line
    * (wordnext points to the next available space in the buffer (i.e
    * the end of the buffer).
    */

    if (curword == wordnext) {
      curword = prevword;
      prevword = WORDPREV(curword);
    }

    while ( (curword != wordfirst) && 
	    (wdbindf(*(prevword->cp+prevword->wclen-1), *curword->cp, 1) >4)
	  ) 
    {
      curword = prevword;
      prevword = WORDPREV(curword);
    }

    if (curword != wordfirst) {

      /* 
       * Did not reach the beginning of the buffer so we are between
       * two non-ekinsoku & non-bekinsoku characters.  Move the end
       * of the first line here.
       */

      *wordpast = curword;
      return true;

    } 
    else {

      /* 
       * Reached the beginning of the buffer so search forward
       * from the beginning of the second line until:
       * 1) we find two words we can split between lines, 
       * 2) we reach the end of the buffer, or
       * 3) the line becomes too long.
       * 
       * If there is no next line then just exit.
       */

      /*
       * compword points to the first word of the next line or to the
       *   next available spot in the buffer if there is no next line.
       * nextword points to the second word of the next line.
       * wordnext points to the next available space in the buffer (i.e
       * the end of the buffer)
       */
      curword = compword;
      nextword = compword;
      WORDNEXT(nextword);

      if (curword == wordnext)
	return false;

      while (nextword != wordnext) 
      {

         if (wdbindf(*(curword->cp+curword->wclen-1), *nextword->cp, 1) < 5)
           break;
         margin += (curword->length + curword->blanks);
         if (margin >= fillmargin)
            return false;
         curword = nextword;
         WORDNEXT(nextword);
      }

      if (nextword != wordnext) {

        /* 
         * Did not reach the end of the buffer so we are between
         * two non-ekinsoku & non-bekinsoku characters.  Move the end
         * of the first line here.
         */
        *wordpast = nextword;
        return true;
      }

      return false;

    } 

} /* end EkinBekinProc */

#endif /* not sun */

static
postEkinBekinProc(poutchars,wordpast)
int	*poutchars;
WORD	*wordpast;
{
   WORD	*curword;
   int	colvalue,curlineno;

/*
 * Recompute the value of *poutchars
 */

   (*poutchars) = 0;

   curword = wordfirst;
   while (curword != wordpast) 
   {
	(*poutchars) +=  curword->length + curword->blanks;
	WORDNEXT(curword);
   }

   /*
   ** Adjust the word parameters -
   ** inlinenum,incolumn of all the words
   ** from 'curword' till 'wordnext'
   */

   curword = wordpast;
   curlineno = curword->inlinenum+1;
   colvalue = indent2;
   while (curword != wordnext) 
   {
     curword->inlinenum = curlineno;
     curword->incolumn = colvalue;
     colvalue += curword->length + curword->blanks;
     if (colvalue > fillmargin) 
     {
       colvalue = indent2;
       curlineno++;
     }
     WORDNEXT(curword);
   }
   incolumn = colvalue;
   inlinenum = curlineno;
}

#ifdef DEBUG

static
prnword(pword)
WORD	*pword;
{
   int i = 0;

   if (pword == NULL)
     return;
   for (i=0; i < pword->wclen; i++)
     fprintf(stderr,(const char *)"%c",(char)*(pword->cp+i));
   fprintf(stderr,"Word Length :%d\n",pword->wclen);
}

#endif /* DEBUG */

/* ARGSUSED */
static void
AdjustParaCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{

    DtEditorFormat( (Widget)client_data, (DtEditorFormatSettings *)NULL,
    		    DtEDITOR_FORMAT_PARAGRAPH );

} /* AdjustParaCB */

/* ARGSUSED */
static void
AdjustAllCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{

    DtEditorFormat( (Widget)client_data, (DtEditorFormatSettings *)NULL,
    		    DtEDITOR_FORMAT_ALL );

} /* AdjustAllCB */


/*********
 *
 * DoAdjust - given left & right margin values & an alignment style, formats
 * 	      the text from one specified text position to another.
 *
 *	      Returns: 
 *		DtEDITOR_NO_ERRORS    if the text was formatted successfully.
 * 		DtEDITOR_ILLEGAL_SIZE if the left & right margins don't make 
 *				      sense.
 *		DtEDITOR_NO_TMP_FILE  if the 2 temporary files cannot be 
 *				      created.
 *		DtEDITOR_INVALID_TYPE if specified alignment is
 *				      unrecognized
 *
 */
static DtEditorErrorCode
DoAdjust(
        DtEditorWidget	editor,
	int		leftMargin,
	int		rightMargin,
	unsigned int	alignment,
        XmTextPosition	start,
        XmTextPosition	end)
{
    char tempName1[L_tmpnam], tempName2[L_tmpnam];
    DtEditorErrorCode returnVal;
    AdjRec adjRec;

    /* 
     * Check that valid margin values were passed in
     */
    if( leftMargin >= 0 && 
	rightMargin > 0 &&
	rightMargin < 1025 &&
	leftMargin <= rightMargin ) 
    { 
       /* 
        * Set up 
        */
       _DtTurnOnHourGlass(M_topLevelShell(editor));
       if (M_format_dialog(editor) != (Widget)NULL)
         _DtTurnOnHourGlass(M_format_dialog(editor));

       memset(&adjRec, 0, sizeof(AdjRec));

       switch (alignment)
       {
         case DtEDITOR_ALIGN_CENTER:
	 {
	   adjRec.cflag = 1;
	   adjRec.margin = (rightMargin + leftMargin) / 2;
           break;
         }

         case DtEDITOR_ALIGN_JUSTIFY:
         {
	   adjRec.jflag = 1;
	   adjRec.margin = rightMargin;
           break;
         }

         case DtEDITOR_ALIGN_RIGHT:
         {
	   adjRec.rflag = 1;
	   adjRec.margin = rightMargin;
           break;
         }

         case DtEDITOR_ALIGN_LEFT:
         {
	   adjRec.margin = rightMargin;
           break;
         }

         default:
         {
	   returnVal = DtEDITOR_INVALID_TYPE;
         }
  
       } /* end switch */

       /*
        * Turn off converting leading spaces into tabs if we are 
	* working in a multi-byte locale.  This is necessary
	* because a tab may not equal (be as wide as) 8 spaces in 
	* a multi-byte locale.
        */
       adjRec.tabsize = 8;

       if (MB_CUR_MAX > 1)
         adjRec.bflag = True;
       else
         adjRec.bflag = False;


       /*
        * Create the two temp files
        */
       (void)tmpnam(tempName1);
       (void)tmpnam(tempName2);
       if ((adjRec.infp = fopen(tempName1, "w+")) != (FILE *)NULL) {

         /* 
          * Successfully opened the first temporary file 
          */

         if((adjRec.outfp = fopen(tempName2, "w")) != (FILE *)NULL) {

            /* 
	     * Successfully opened the second temporary file, so do the
	     * formatting.
	     */ 
       	    returnVal = DtEDITOR_NO_ERRORS;

            fixLeftMarginAndNewlines( editor, adjRec.infp, leftMargin, 
				      rightMargin, (int)start, (int)end );
            fflush(adjRec.infp);
            rewind(adjRec.infp);

            FormatText(&adjRec);

            fclose(adjRec.infp);
            unlink(tempName1);

            fclose(adjRec.outfp);
            DtEditorReplaceFromFile( (Widget)editor, start, end, tempName2 );
            unlink(tempName2);

         } 
	 else {
	   /*
	    * Could not open second temporary file, so clean up first one
	    */
           fclose(adjRec.infp);
           unlink(tempName1);
           returnVal = DtEDITOR_NO_TMP_FILE;
         }

       } /* end creating temporary files */ 
       else
         returnVal = DtEDITOR_NO_TMP_FILE;

       /*
        * Clean up
        */
       _DtTurnOffHourGlass(M_topLevelShell(editor));
       if (M_format_dialog(editor) != (Widget)NULL)
         _DtTurnOffHourGlass(M_format_dialog(editor));

    } /* end check for valid margins */ 
    else
      returnVal = DtEDITOR_ILLEGAL_SIZE;

    return (returnVal);
}

/* ARGSUSED */
static void
AdjustCloseCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    DtEditorWidget editor = (DtEditorWidget )client_data;
    XtUnmanageChild (M_format_dialog(editor));
}

/* ARGSUSED */
static int 
ComputeRightMargin(
        DtEditorWidget editor)
{
    Dimension text_width, highlight_thickness, shadow_thickness,
   	       margin_width;
    int	rightMargin;
    Arg al[5];             /* arg list */
    register	int ac;    /* arg count */

    ac=0;
    XtSetArg(al[ac], XmNwidth, &text_width); ac++;
    XtSetArg(al[ac], XmNhighlightThickness, &highlight_thickness); ac++;
    XtSetArg(al[ac], XmNshadowThickness, &shadow_thickness); ac++;
    XtSetArg(al[ac], XmNmarginWidth, &margin_width); ac++;
    XtGetValues(M_text(editor), al, ac);

    if (M_fontWidth(editor) != 0)
      rightMargin = ( (int)text_width - 
		      (2 * ((int)highlight_thickness + 
			    (int)shadow_thickness + (int)margin_width)) 
		    ) / M_fontWidth(editor);
    else
      rightMargin = (int)text_width - 
		    (2 * ((int)highlight_thickness + 
			  (int)shadow_thickness + (int)margin_width)); 

    return( rightMargin );

} /* end ComputeRightMargin */

/* ARGSUSED */
static void 
UpdateAdjust(
        Widget widget,
        XtPointer *client_data,
        XConfigureEvent *event )
{
    DtEditorWidget editor = (DtEditorWidget) client_data;
    char orgnum[20];        /* original margin size string */

    sprintf( orgnum, "%d", ComputeRightMargin(editor) );
    XmTextFieldSetString(M_format_rightMarginField(editor), orgnum);
}

/************************************************************************
 *
 *  SetFormatDialogTitle - Change the title for the Format Settings dialog
 *
 ************************************************************************/

static void
SetFormatDialogTitle(
        DtEditorWidget editor)
{
  Arg al[2];

  /*
   * If the Format Settings dialog has been created, change its title
   */
  if( M_format_dialog(editor) != (Widget)NULL )
  {

       /*
	* Prepend the DialogTitle resource, if it has been set
	*/
       if( E_dialogTitle(editor) != (XmString)NULL ) {
         XmString titleStr;

	 /*
	  * Add the "functional title" to the DialogTitle
	  */
         titleStr = XmStringConcat( E_dialogTitle(editor), 
				    E_format_dialogTitle(editor) );

         XtSetArg( al[0], XmNdialogTitle, titleStr );
         XtSetValues(M_format_dialog(editor), al, 1);

         XmStringFree( titleStr );

     }
     else {
         XtSetArg( al[0], XmNdialogTitle, E_format_dialogTitle(editor) );
         XtSetValues(M_format_dialog(editor), al, 1);
     }

  }

} /* end SetFormatDialogTitle */

/************************************************************************
 *
 *  ResetFormatDialog - Reset margins & alignment of the Format Settings 
 *  			dialog
 *
 ************************************************************************/

static void
ResetFormatDialog(
	DtEditorWidget editor)
{
    char orgnum[20];        /* original margin size string */

    /*
     * Reset the margins to default values
     */

    /* Right margin default value */
    sprintf( orgnum, "%d", ComputeRightMargin(editor) );
    XmTextFieldSetString(M_format_rightMarginField(editor), orgnum);

    /* Left margin default value */
    sprintf(orgnum, "%d", 0);
    XmTextFieldSetString(M_format_leftMarginField(editor), orgnum);

    /*
     * Reset the alignment style to default value
     */
    XmToggleButtonGadgetSetState(M_format_leftJust(editor), True, True);

} /* end ResetFormatDialog */


/************************************************************************
 *
 *  CreateFormatDialog - Create & initialize the Format Settings dialog
 *
 ************************************************************************/

static void
CreateFormatDialog(
	DtEditorWidget editor)
{
    Arg		al[15];        /* arg list */
    register	int ac;        /* arg count */
    Pixel	textBackground, textForeground;
    XmString	tempString = (XmString)NULL;

       /* 
        * Match the background & foreground colors of the edit window
        * Don't use DtNtextBackground/Foreground directly because they 
        * will be DtUNSPECIFIED.
        */
       ac = 0;
       XtSetArg(al[ac], XmNforeground, &textForeground); ac++;
       XtSetArg(al[ac], XmNbackground, &textBackground); ac++;
       XtGetValues(M_text(editor), al, ac);

       ac = 0;

       /* 
	* First, create the dialog's title, prepending the 
	* DtNdialogTitle resource, if it is set
	*/
       if( E_dialogTitle(editor) != (XmString)NULL ) {

         /*
	  * Add the "functional title" to the DialogTitle
	  */
         tempString = XmStringConcat( E_dialogTitle(editor), 
		 		      E_format_dialogTitle(editor) );
         XtSetArg (al[ac], XmNdialogTitle, tempString ); ac++;
 
       }
       else {
         XtSetArg(al[ac], XmNdialogTitle, E_format_dialogTitle(editor)); ac++;
       }

       XtSetArg (al[ac], XmNautoUnmanage, False); 	ac++;
       XtSetArg (al[ac], XmNmarginWidth, 5);            ac++;
       XtSetArg (al[ac], XmNmarginHeight, 5);           ac++;
       XtSetArg (al[ac], XmNshadowThickness, 1);        ac++;
       XtSetArg (al[ac], XmNshadowType, XmSHADOW_OUT);  ac++;
       M_format_dialog(editor) = XmCreateFormDialog(
       				M_topLevelShell(editor), "ad_dial", al, ac);
       if (tempString != (XmString)NULL)
         XmStringFree( tempString );

       XtAddCallback(M_format_dialog(editor), XmNhelpCallback,
		     (XtCallbackProc)HelpFormatDialogCB, (XtPointer)editor);


       /*
	* When creating the fields & buttons use the appropriate label
	* resource (e.g. DtNcenterToggleLabel), if it has been set, then
	* clear the resource to save space.  The field or button widget 
	* will contain the actual value & it can be gotten from there, 
	* if it is needed.
	*
	* If the appropriate resource has not been set, use its default 
	* value from the message catalog.
	*/

       /*
        * create the left margin label and text field
        */

       ac = 0;
       if (E_format_leftMarginFieldLabel(editor) != (XmString) DtUNSPECIFIED) { 
         /*
	  * Use the resource value & clear it (to save space).
	  */
         tempString = XmStringCopy(E_format_leftMarginFieldLabel(editor));
	 E_format_leftMarginFieldLabel(editor) = (XmString) DtUNSPECIFIED;
       }
       else { 
         /*
	  * The resource has not been set so use its default value
	  */
         tempString = XmStringCreateLtoR(LEFT_MARGIN, XmFONTLIST_DEFAULT_TAG);
       }

       XtSetArg(al[ac], XmNlabelString, tempString); ac++;
       XtSetArg (al[ac], XmNleftAttachment, XmATTACH_POSITION);  ac++;
       XtSetArg (al[ac], XmNleftPosition, 2);                    ac++;
       XtSetArg (al[ac], XmNtopAttachment, XmATTACH_FORM);       ac++;
       XtSetArg (al[ac], XmNtopOffset, 10);                      ac++;
       XtSetArg (al[ac], XmNfontList, E_labelFontList(editor));  ac++;
       M_format_leftLabel(editor) = (Widget) XmCreateLabelGadget (
			     M_format_dialog(editor), "left_label", al, ac);
       XtManageChild (M_format_leftLabel(editor));
       XmStringFree (tempString);

       XtAddCallback(M_format_leftLabel(editor), XmNhelpCallback,
		     (XtCallbackProc)HelpFormatLeftMarginCB, (XtPointer)editor);

       ac = 0;
       XtSetArg (al[ac], XmNbackground, textBackground);   ac++;
       XtSetArg (al[ac], XmNforeground, textForeground);   ac++;
       XtSetArg (al[ac], XmNleftAttachment, XmATTACH_WIDGET);   ac++;
       XtSetArg (al[ac], XmNleftWidget, M_format_leftLabel(editor));  ac++;
       XtSetArg (al[ac], XmNleftOffset, 3);                     ac++;
       XtSetArg (al[ac], XmNrightAttachment, XmATTACH_POSITION);   ac++;
       XtSetArg (al[ac], XmNrightPosition, 45);   ac++;
       XtSetArg (al[ac], XmNtopAttachment, XmATTACH_FORM);     ac++;
       XtSetArg (al[ac], XmNtopOffset, 5);                     ac++;
       XtSetArg (al[ac], XmNtraversalOn, True);                ac++;
       XtSetArg (al[ac], XmNcolumns, 3);                       ac++;
       XtSetArg (al[ac], XmNfontList, E_textFontList(editor)); ac++;
       M_format_leftMarginField(editor) = (Widget) XmCreateTextField (
				M_format_dialog(editor), "left_text", al, ac);
       XtManageChild (M_format_leftMarginField(editor));
       XtAddCallback(M_format_leftMarginField(editor), XmNhelpCallback,
		     (XtCallbackProc)HelpFormatLeftMarginCB, (XtPointer)editor);
       /*
        * create the right margin label and text field
        */
       ac = 0;
       if (E_format_rightMarginFieldLabel(editor) != (XmString) DtUNSPECIFIED)
       { 
         /*
	  * Use the resource value & clear it (to save space).
	  */
         tempString = XmStringCopy(E_format_rightMarginFieldLabel(editor));
	 E_format_rightMarginFieldLabel(editor) = (XmString) DtUNSPECIFIED;
       }
       else
       { 
         /*
	  * The resource has not been set so use its default value
	  */
         tempString = XmStringCreateLtoR(RIGHT_MARGIN, XmFONTLIST_DEFAULT_TAG);
       }
       XtSetArg(al[ac], XmNlabelString, tempString); ac++;
       XtSetArg (al[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
       XtSetArg (al[ac], XmNleftPosition, 55);                  ac++;
       XtSetArg (al[ac], XmNtopAttachment, XmATTACH_FORM);      ac++;
       XtSetArg (al[ac], XmNtopOffset, 10);                     ac++;
       XtSetArg (al[ac], XmNfontList, E_labelFontList(editor)); ac++;
       M_format_rightLabel(editor)= (Widget) XmCreateLabelGadget (
			M_format_dialog(editor), "right_label", al, ac);
       XtManageChild (M_format_rightLabel(editor));
       XmStringFree (tempString);
       XtAddCallback(M_format_rightLabel(editor), XmNhelpCallback,
		     (XtCallbackProc)HelpFormatRightMarginCB, 
		     (XtPointer)editor);
       ac = 0;
       XtSetArg (al[ac], XmNbackground, textBackground);   ac++;
       XtSetArg (al[ac], XmNforeground, textForeground);   ac++;
       XtSetArg (al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
       XtSetArg (al[ac], XmNrightPosition, 98);                 ac++;
       XtSetArg (al[ac], XmNleftAttachment, XmATTACH_WIDGET);   ac++;
       XtSetArg (al[ac], XmNleftWidget, M_format_rightLabel(editor));   ac++;
       XtSetArg (al[ac], XmNleftOffset, 3);                     ac++;
       XtSetArg (al[ac], XmNtopAttachment, XmATTACH_FORM);      ac++;
       XtSetArg (al[ac], XmNtopOffset, 5);                      ac++;
       XtSetArg (al[ac], XmNtraversalOn, True);                 ac++;
       XtSetArg (al[ac], XmNcolumns, 3);                        ac++;
       XtSetArg (al[ac], XmNfontList, E_textFontList(editor)); ac++;
       M_format_rightMarginField(editor) = (Widget) XmCreateTextField (
						M_format_dialog(editor),
                                                "right_text", al, ac);
       XtManageChild (M_format_rightMarginField(editor));
       XtAddCallback(M_format_rightMarginField(editor), XmNhelpCallback,
		     (XtCallbackProc)HelpFormatRightMarginCB, 
		     (XtPointer)editor);

       /*
        * create the radio box for justification choices
        */
       ac = 0;
       XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);     ac++;
       XtSetArg (al[ac], XmNleftOffset, 5);                     ac++;
       XtSetArg (al[ac], XmNtopAttachment, XmATTACH_WIDGET);    ac++;
       XtSetArg (al[ac], XmNtopWidget, M_format_leftMarginField(editor));  ac++;
       XtSetArg (al[ac], XmNtopOffset, 5);                     ac++;
       XtSetArg (al[ac], XmNrightAttachment, XmATTACH_FORM);    ac++;
       XtSetArg (al[ac], XmNrightOffset, 5);                    ac++;
       XtSetArg (al[ac], XmNtraversalOn, True);                 ac++;
       M_format_radioBox(editor) = (Widget) XmCreateRadioBox(
				M_format_dialog(editor), "radioBox", al, ac);
       XtAddCallback(M_format_radioBox(editor), XmNhelpCallback,
		     (XtCallbackProc)HelpFormatJustifyButtonsCB,
		     (XtPointer)editor);
       XtManageChild(M_format_radioBox(editor));

       /* Create Left Align toggle */
       ac = 0;
       if (E_format_leftAlignToggleLabel(editor) != (XmString) DtUNSPECIFIED) { 
         /*
	  * Use the resource value & clear it (to save space).
	  */
         tempString = XmStringCopy(E_format_leftAlignToggleLabel(editor));
	 E_format_leftAlignToggleLabel(editor) = (XmString) DtUNSPECIFIED;
       }
       else { 
         /*
	  * The resource has not been set so use its default value
	  */
         tempString = XmStringCreateLtoR(LEFT_ALIGN, XmFONTLIST_DEFAULT_TAG);
       }
       XtSetArg(al[ac], XmNlabelString, tempString); ac++;
       XtSetArg (al[ac], XmNfontList, E_buttonFontList(editor)); ac++;
       M_format_leftJust(editor) = (Widget) XmCreateToggleButtonGadget(
			M_format_radioBox(editor), "left_just", al, ac);
       XmStringFree (tempString);
       XtManageChild(M_format_leftJust(editor));

       /* Create Right Align toggle */
       ac = 0;
       if(E_format_rightAlignToggleLabel(editor) != (XmString)DtUNSPECIFIED) { 
         /*
	  * Use the resource value & clear it (to save space).
	  */
         tempString = XmStringCopy(E_format_rightAlignToggleLabel(editor));
	 E_format_rightAlignToggleLabel(editor) = (XmString) DtUNSPECIFIED;
       }
       else { 
         /*
	  * The resource has not been set so use its default value
	  */
         tempString = XmStringCreateLtoR(RIGHT_ALIGN, XmFONTLIST_DEFAULT_TAG);
       }
       XtSetArg(al[ac], XmNlabelString, tempString); ac++;
       XtSetArg (al[ac], XmNfontList, E_buttonFontList(editor)); ac++;
       M_format_rightJust(editor) = (Widget) XmCreateToggleButtonGadget(
       			M_format_radioBox(editor), "right_just", al, ac);
       XmStringFree (tempString);
       XtManageChild(M_format_rightJust(editor));

       /* Create Justify toggle */
       ac = 0;
       if (E_format_justifyToggleLabel(editor) != (XmString) DtUNSPECIFIED) { 
         /*
	  * Use the resource value & clear it (to save space).
	  */
         tempString = XmStringCopy(E_format_justifyToggleLabel(editor));
	 E_format_justifyToggleLabel(editor) = (XmString) DtUNSPECIFIED;
       }
       else { 
         /*
	  * The resource has not been set so use its default value
	  */
         tempString = XmStringCreateLtoR( JUSTIFY, XmFONTLIST_DEFAULT_TAG );
       }
       XtSetArg(al[ac], XmNlabelString, tempString); ac++;
       XtSetArg (al[ac], XmNfontList, E_buttonFontList(editor)); ac++;
       M_format_bothJust(editor) = (Widget) XmCreateToggleButtonGadget(
       				M_format_radioBox(editor), "both_just", al, ac);
       XmStringFree (tempString);
       XtManageChild(M_format_bothJust(editor));

       /* Create Center align toggle */
       ac = 0;
       if (E_format_centerToggleLabel(editor) != (XmString) DtUNSPECIFIED) { 
         /*
	  * Use the resource value & clear it (to save space).
	  */
         tempString = XmStringCopy(E_format_centerToggleLabel(editor));
	 E_format_centerToggleLabel(editor) = (XmString) DtUNSPECIFIED;
       }
       else { 
         /*
	  * The resource has not been set so use its default value
	  */
         tempString = XmStringCreateLtoR(CENTER, XmFONTLIST_DEFAULT_TAG); 
       }
       XtSetArg(al[ac], XmNlabelString, tempString); ac++;
       XtSetArg (al[ac], XmNfontList, E_buttonFontList(editor)); ac++;
       M_format_center(editor) = (Widget) XmCreateToggleButtonGadget(
				M_format_radioBox(editor), "center", al, ac);
       XmStringFree(tempString);
       XtManageChild(M_format_center(editor));

       /*  Create a separator between the radio box and buttons  */

       ac = 0;
       XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);     ac++;
       XtSetArg (al[ac], XmNrightAttachment, XmATTACH_FORM);    ac++;
       XtSetArg (al[ac], XmNtopAttachment, XmATTACH_WIDGET);    ac++;
       XtSetArg (al[ac], XmNtopWidget, M_format_radioBox(editor));    ac++;
       XtSetArg (al[ac], XmNtopOffset, 15);                     ac++;
       M_format_separator(editor) =  (Widget) XmCreateSeparatorGadget (
				M_format_dialog(editor), "separator", al, ac);
       XtManageChild (M_format_separator(editor));

       /* Create Format Paragraph button */
       ac = 0;
       if ( E_format_formatParagraphButtonLabel(editor) != 
	    (XmString) DtUNSPECIFIED ) { 
         /*
	  * Use the resource value & clear it (to save space).
	  */
         tempString = XmStringCopy(E_format_formatParagraphButtonLabel(editor));
	 E_format_formatParagraphButtonLabel(editor) = (XmString) DtUNSPECIFIED;
       }
       else { 
         /*
	  * The resource has not been set so use its default value
	  */
         tempString =  XmStringCreateLtoR(PARAGRAPH, XmFONTLIST_DEFAULT_TAG);
       }
       XtSetArg(al[ac], XmNlabelString, tempString); ac++;
       XtSetArg (al[ac], XmNleftAttachment, XmATTACH_POSITION);    ac++;
       XtSetArg (al[ac], XmNleftPosition, 2);                      ac++;
       XtSetArg (al[ac], XmNrightAttachment, XmATTACH_POSITION);   ac++;
       XtSetArg (al[ac], XmNrightPosition, 25);                    ac++;
       XtSetArg (al[ac], XmNtopAttachment, XmATTACH_WIDGET);       ac++;
       XtSetArg (al[ac], XmNtopWidget, M_format_separator(editor));  ac++;
       XtSetArg (al[ac], XmNtopOffset, 5);                         ac++;
       XtSetArg (al[ac], XmNbottomAttachment, XmATTACH_FORM);      ac++;
       XtSetArg (al[ac], XmNbottomOffset, 5);                      ac++;
       XtSetArg (al[ac], XmNmarginWidth, 4);                       ac++;
       XtSetArg (al[ac], XmNmarginHeight, 4);                      ac++;
       XtSetArg (al[ac], XmNfontList, E_buttonFontList(editor)); ac++;
       M_format_paragraph(editor) = (Widget) XmCreatePushButtonGadget (
				M_format_dialog(editor), "para", al, ac);
       XtManageChild (M_format_paragraph(editor));
       XmStringFree (tempString);
       XtAddCallback (M_format_paragraph(editor), XmNactivateCallback,
                       (XtCallbackProc) AdjustParaCB, (XtPointer) editor);

       /* Create Format All button */
       ac = 0;
       if (E_format_formatAllButtonLabel(editor) != (XmString)DtUNSPECIFIED) { 
         /*
	  * Use the resource value & clear it (to save space).
	  */
         tempString = XmStringCopy(E_format_formatAllButtonLabel(editor));
	 E_format_formatAllButtonLabel(editor) = (XmString) DtUNSPECIFIED;
       }
       else { 
         /*
	  * The resource has not been set so use its default value
	  */
         tempString = XmStringCreateLtoR(ALL, XmFONTLIST_DEFAULT_TAG);
       }
       XtSetArg(al[ac], XmNlabelString, tempString); ac++;
       XtSetArg (al[ac], XmNleftAttachment, XmATTACH_POSITION);    ac++;
       XtSetArg (al[ac], XmNleftPosition, 27);                     ac++;
       XtSetArg (al[ac], XmNrightAttachment, XmATTACH_POSITION);   ac++;
       XtSetArg (al[ac], XmNrightPosition, 49);                    ac++;
       XtSetArg (al[ac], XmNtopAttachment, XmATTACH_WIDGET);       ac++;
       XtSetArg (al[ac], XmNtopWidget, M_format_separator(editor));  ac++;
       XtSetArg (al[ac], XmNtopOffset, 5);                         ac++;
       XtSetArg (al[ac], XmNbottomAttachment, XmATTACH_FORM);      ac++;
       XtSetArg (al[ac], XmNbottomOffset, 5);                      ac++;
       XtSetArg (al[ac], XmNmarginWidth, 4);                       ac++;
       XtSetArg (al[ac], XmNmarginHeight, 4);                      ac++;
       XtSetArg (al[ac], XmNfontList, E_buttonFontList(editor)); ac++;
       M_format_all(editor) = (Widget) XmCreatePushButtonGadget(
       				     M_format_dialog(editor), "all", al, ac);
       XtManageChild (M_format_all(editor));
       XmStringFree (tempString);
       XtAddCallback (M_format_all(editor), XmNactivateCallback,
                       (XtCallbackProc) AdjustAllCB, (XtPointer) editor);

       /* Create Close button */
       tempString = XmStringCreateLtoR(CLOSE_BUTTON, XmFONTLIST_DEFAULT_TAG);
       ac = 0;
       XtSetArg (al[ac], XmNlabelString, tempString);  ac++;
       XtSetArg (al[ac], XmNleftAttachment, XmATTACH_POSITION);    ac++;
       XtSetArg (al[ac], XmNleftPosition, 51);                     ac++;
       XtSetArg (al[ac], XmNrightAttachment, XmATTACH_POSITION);   ac++;
       XtSetArg (al[ac], XmNrightPosition, 73);                    ac++;
       XtSetArg (al[ac], XmNtopAttachment, XmATTACH_WIDGET);       ac++;
       XtSetArg (al[ac], XmNtopWidget, M_format_separator(editor)); ac++;
       XtSetArg (al[ac], XmNtopOffset, 5);                         ac++;
       XtSetArg (al[ac], XmNbottomAttachment, XmATTACH_FORM);      ac++;
       XtSetArg (al[ac], XmNbottomOffset, 5);                      ac++;
       XtSetArg (al[ac], XmNmarginWidth, 4);                       ac++;
       XtSetArg (al[ac], XmNmarginHeight, 4);                      ac++;
       XtSetArg (al[ac], XmNfontList, E_buttonFontList(editor));   ac++;
       M_format_close(editor) = (Widget) XmCreatePushButtonGadget (
			M_format_dialog(editor), "close", al, ac);
       XtManageChild (M_format_close(editor));
       XmStringFree (tempString);
       XtAddCallback (M_format_close(editor), XmNactivateCallback,
                       (XtCallbackProc) AdjustCloseCB, (XtPointer) editor);

       tempString = XmStringCreateLtoR(HELP_BUTTON, XmFONTLIST_DEFAULT_TAG);
       ac = 0;
       XtSetArg (al[ac], XmNlabelString, tempString);              ac++;
       XtSetArg (al[ac], XmNleftAttachment, XmATTACH_POSITION);    ac++;
       XtSetArg (al[ac], XmNleftPosition, 75);                     ac++;
       XtSetArg (al[ac], XmNrightAttachment, XmATTACH_POSITION);   ac++;
       XtSetArg (al[ac], XmNrightPosition, 98);                    ac++;
       XtSetArg (al[ac], XmNtopAttachment, XmATTACH_WIDGET);       ac++;
       XtSetArg (al[ac], XmNtopWidget, M_format_separator(editor));  ac++;
       XtSetArg (al[ac], XmNtopOffset, 5);                         ac++;
       XtSetArg (al[ac], XmNbottomAttachment, XmATTACH_FORM);      ac++;
       XtSetArg (al[ac], XmNbottomOffset, 5);                      ac++;
       XtSetArg (al[ac], XmNmarginWidth, 4);                       ac++;
       XtSetArg (al[ac], XmNmarginHeight, 4);                      ac++;
       XtSetArg (al[ac], XmNfontList, E_buttonFontList(editor));   ac++;
       M_format_help(editor) = (Widget) XmCreatePushButtonGadget(
				M_format_dialog(editor), "help", al, ac);
       XtManageChild (M_format_help(editor));
       XmStringFree (tempString);
       XtAddCallback( M_format_help(editor), XmNactivateCallback,
                      (XtCallbackProc) HelpFormatDialogCB, (XtPointer) editor);

       XtRealizeWidget (M_format_dialog(editor));

       /*
	* Initialize the margins & alignment toggles
	*/
       ResetFormatDialog(editor);

       XtAddEventHandler(M_text(editor), StructureNotifyMask, False,
                            (XtEventHandler) UpdateAdjust, (XtPointer) editor);

} /* end CreateFormatDialog */

/************************************************************************
 *
 *  GetAdjustSettings - Read the alignment settings from the Format
 *			Settings dialog.
 *
 ************************************************************************/

static void
GetAdjustSettings(
        DtEditorWidget editor,
	DtEditorFormatSettings	*formatSettings)
{

    if( M_format_dialog(editor)!= (Widget)NULL ) {

      /*
       * Read the current alignment settings from the Format Settings
       * dialog.
       */
      char *num;

      /* 
       * Get current type of alignment 
       */
      if (XmToggleButtonGadgetGetState(M_format_leftJust(editor)))
          formatSettings->alignment = DtEDITOR_ALIGN_LEFT;
      else if (XmToggleButtonGadgetGetState(M_format_rightJust(editor)))
          formatSettings->alignment = DtEDITOR_ALIGN_RIGHT;
      else if (XmToggleButtonGadgetGetState(M_format_bothJust(editor)))
          formatSettings->alignment = DtEDITOR_ALIGN_JUSTIFY;
      else
          formatSettings->alignment = DtEDITOR_ALIGN_CENTER;

      /* 
       * Get current margin settings 
       */
      num = (char *) XmTextFieldGetString( 
				(Widget)M_format_rightMarginField(editor) );
      formatSettings->rightMargin = atoi(num);
      XtFree(num);
      num = (char *) XmTextFieldGetString( 
				(Widget)M_format_leftMarginField(editor) );
      formatSettings->leftMargin = atoi(num);
      XtFree(num);

    } 
    else {

      /* 
       * The Format Settings dialog box has not been created so use the
       * default values.
       */
      formatSettings->leftMargin = 0;
      formatSettings->rightMargin = ComputeRightMargin(editor);
      formatSettings->alignment = DtEDITOR_ALIGN_LEFT;

    }
}

/****************************************************************
 *
 *  Public Procs 
 *
 ****************************************************************/



/*********************************************************
 *
 *  DtCreateEditor
 *      Create an instance of an editor and return the widget id.
 */

Widget
DtCreateEditor(
        Widget parent,
        char *name,
        ArgList arglist,
        Cardinal argcount )
{
  return( XtCreateWidget(name,dtEditorWidgetClass,parent,arglist,argcount) );
}

Boolean
DtEditorCheckForUnsavedChanges(
	Widget widget)
{

  DtEditorWidget editor = (DtEditorWidget) widget;
  return M_unreadChanges( editor );

} /* end DtEditorCheckForUnsavedChanges */


Boolean
DtEditorDeleteSelection(
        Widget widget)
{
    XmTextPosition first, last;
    DtEditorWidget editor = (DtEditorWidget) widget;

    /* 
     * Check for a non-null selection
     */
    if ( XmTextGetSelectionPosition(M_text(editor), &first, &last) &&
	 first != last )
    {
      XmTextRemove(M_text(editor));
      return( True );
    }
    else
      return (False);

} /* end DtEditorDeleteSelection */


/*
 * DtEditorClearSelection replaces the currently selected text with
 * spaces.  It requires an event because the underlying text widget
 * routines require one to look at the time stamp within.
 */
Boolean
DtEditorClearSelection(
        Widget widget)
{
    DtEditorWidget editor = (DtEditorWidget) widget;
    char *params=(char *)NULL;
    Cardinal num_params = 0;

    /*
     * Create an event with a correct timestamp
     */
    XEvent *event = (XEvent *) XtMalloc( sizeof(XEvent) );
    event->xkey.time = XtLastTimestampProcessed( M_display(editor) );

    /*
     * Call routine to clear primary selection
     */

    ClearSelection( widget, event, &params, &num_params );

    XtFree( (char *) event );

} /* end DtEditorClearSelection */


/*
 *
 * DtEditorDeselect - Unselects any selected text of an Editor widget
 *
 */
Boolean 
DtEditorDeselect( 
	Widget widget)
{
  DtEditorWidget editor = (DtEditorWidget) widget;
  char *params=(char *)NULL;
  Cardinal num_params = 0;

  /*
   * Create an event with a correct timestamp
   */
  XEvent *event = (XEvent *) XtMalloc( sizeof(XEvent) );
  event->xkey.time = XtLastTimestampProcessed( M_display(editor) );

  DeselectAll( widget, event, &params, &num_params );

  XtFree( (char *)event );

} /* end DtEditorDeselect */


/*
 *
 * DtEditorDisableRedisplay - Temporarily prevent visual update of a
 *			      Editor widget
 *
 */
void 
DtEditorDisableRedisplay( 
	Widget widget)
{
  DtEditorWidget editor = (DtEditorWidget) widget;

  XmTextDisableRedisplay( M_text(editor) );

} /* end DtEditorDisableRedisplay */

/*
 *
 * DtEditorEnableRedisplay - Force visual update of an Editor widget 
 *
 */
void 
DtEditorEnableRedisplay( 
	Widget widget)
{
  DtEditorWidget editor = (DtEditorWidget) widget;

  XmTextEnableRedisplay( M_text(editor) );

} /* end DtEditorEnableRedisplay */


/*********
 *
 * DtEditorFormat 
 *		Given left & right margin values & an alignment style, 
 *		formats either the current paragraph or the entire document,
 *		as specified.  If margin values & alignment style are not
 *		provided, DtEditorFormat will use the current values from
 *		the Format Settings dialog.
 *
 *	      Returns: 
 *		DtEDITOR_NO_ERRORS    if the text was formatted successfully.
 *		DtEDITOR_NO_TMP_FILE  if the 2 temporary files cannot be 
 *				      created.
 * 		DtEDITOR_ILLEGAL_SIZE if the left & right margins don't make 
 *				      sense.
 *		DtEDITOR_INVALID_RANGE if specified amount to format is
 *				      unrecognized
 *		DtEDITOR_INVALID_TYPE if specified alignment is
 *				      unrecognized
 *
 */
DtEditorErrorCode
DtEditorFormat(
	Widget 			widget,
	DtEditorFormatSettings	*formatSettings,
	unsigned int		amountToFormat )
{

  DtEditorWidget editor = (DtEditorWidget) widget;
  DtEditorErrorCode error;
  int	leftMargin, rightMargin, alignment;

  /* 
   * Check to see if we should use the format settings from the 
   * Format Settings dialog.
   */
  if( formatSettings == (DtEditorFormatSettings *) NULL )
  {

    DtEditorFormatSettings tmpFormatSettings;

    /* 
     * Get the format settings from the Format Settings dialog
     */
    GetAdjustSettings(editor, &tmpFormatSettings);

    /*
     * Set up the correct format settings
     */
    leftMargin = tmpFormatSettings.leftMargin;
    rightMargin = tmpFormatSettings.rightMargin;
    alignment = tmpFormatSettings.alignment;

  }
  else {
    leftMargin = formatSettings->leftMargin;
    rightMargin = formatSettings->rightMargin;
    alignment = formatSettings->alignment;
  }

  /*
   * Now, do the formatting
   */

  switch (amountToFormat)
  {
    case DtEDITOR_FORMAT_ALL: 
    {
       error = DoAdjust( editor, leftMargin, rightMargin, alignment,  
		     	 0, XmTextGetLastPosition(M_text(editor)) );
       break;
    }

    case DtEDITOR_FORMAT_PARAGRAPH: 
    {
       XmTextPosition start, end;
       char *params=(char *)NULL;
       Cardinal num_params = 0;

       /*
	* Create an event with a correct timestamp
	*/
       XEvent *event = (XEvent *) XtMalloc( sizeof(XEvent) );
       event->xkey.time = XtLastTimestampProcessed( M_display(editor) );

       /* 
	* Get the beginning & ending positions of the current paragraph
	*/
       ForwardPara( widget, event, &params, &num_params );
       end = XmTextGetCursorPosition(M_text(editor));
       if( end != XmTextGetLastPosition(M_text(editor)) ) {
          /*
	   * If we're not at the end, then we need to back up to the
	   * start of this line, otherwise we'll disturb the first line
	   * of the following paragraph.  If we are at the end, then "end"
	   * must point to the end of the line, or we'll leave the last
	   * line along with the formatted text.
	   */
          BeginningOfLine( widget, event, &params, &num_params );
    	  end = XmTextGetCursorPosition(M_text(editor));
       }

       BackwardPara( widget, event, &params, &num_params );
       BeginningOfLine( widget, event, &params, &num_params );
       start = XmTextGetCursorPosition(M_text(editor));

       /*
	* Pass in beginning & ending positions to adjust the current 
	* paragraph
	*/
       error = DoAdjust( editor, leftMargin, rightMargin, alignment, 
			 start, end );
       break;
    }

    default: {
      error = DtEDITOR_INVALID_RANGE;
    }

  } /* end switch */

  return( error );

} /* DtEditorFormat */


/*
 *
 * DtEditorGetMessageTextFieldID - Returns the widget ID of the Text
 * 	Field used to display application status messages.
 *
 */
Widget
DtEditorGetMessageTextFieldID(
	Widget  	widget)
{
   DtEditorWidget editor = (DtEditorWidget) widget;
   /*
    * Create the status line if is does not exist
    */
   if( M_status_statusArea(editor) == (Widget) NULL )
      M_status_statusArea(editor) = CreateStatusLine( editor ); 

   return( M_status_messageText(editor) );

} /* end DtEditorGetMessageTextFieldID */

/*
 * DtEditorGoToLine moves the insert cursor to the beginning of the
 * line specified by lineNumber.  It figures out the text character 
 * position corresponding to the specified line and sets the text 
 * widget's insertionPosition to that location.  If this new 
 * insertionPosition is not currently on-screen, then the
 * text widget's contents are scrolled to display the new position.
 *
 * The cursor can be moved to last line by specifying DtEDITOR_LAST_LINE 
 * as the line number.
 * If lineNumber is less than one, the insert cursor will be placed
 * at the beginning of the first line.  If it is greater than the total
 * number of lines, the cursor will be placed at the last line of text.
 */
 
/* ARGSUSED */
void DtEditorGoToLine(
	Widget	widget,
	int	lineNumber)
{
    DtEditorWidget editor = (DtEditorWidget)widget;
    XmTextWidget tw = (XmTextWidget)M_text(editor);
    XmTextLineTable lineTab = tw->text.line_table;

    XmTextPosition newPos;

    /*
     * Validate the specified line number, check it is in range.
     * If we adjust the line number, then update the current line display
     * in the status line so it reflects where we are really going.  This
     * is only a problem if lineNumber is less/greater than the first/last
     * line becausee we won't be moving the cursor so the status line will 
     * not be updated.
     */
    if (lineNumber > tw->text.total_lines || lineNumber == DtEDITOR_LAST_LINE)
    {
       lineNumber = tw->text.total_lines;
       _DtEditorUpdateLineDisplay( editor, lineNumber, FORCE ); 
    }
    else if(lineNumber < 1)
    {
       lineNumber = 1;
       _DtEditorUpdateLineDisplay( editor, lineNumber, FORCE ); 
    }


    /*
     * Move the insertion cursor
     */
    newPos = lineTab[lineNumber - 1].start_pos;
    XmTextSetInsertionPosition(M_text(editor), newPos);

    /* 
     * Scroll the widget, if necessary
     */
    if (newPos < tw->text.top_character || newPos >= tw->text.bottom_position)
    {
	Arg al[5];
	int ac;
	Dimension height;
	short rows;
	Position x, y;

        ac = 0;
        XtSetArg(al[ac], XmNheight, &height);  ac++;
        XtSetArg(al[ac], XmNrows, &rows);  ac++;
        XtGetValues(M_text(editor), al, ac);

        if(XmTextPosToXY(M_text(editor), newPos, &x, &y) == True)
        {
            int offset = (y - height/2) * rows;
            XmTextScroll(M_text(editor), offset/(int) height);
        }
    }

} /* end DtEditorGoToLine */

/*
 * DtEditorGetEditorSizeHints - Set the resize increment, minimum window 
 * 	size, and base dimension properties in the supplied XSizeHints 
 * 	struct for the editor as a whole.  
 * 
 * NOTE: This routine returns data in the struct pointed to by pHints.
 * 
 */
void
DtEditorGetSizeHints(
    Widget widget,
    XSizeHints *pHints) /* Return */
{
    Arg al[10];                 /* arg list */
    register int ac;            /* arg count */
    Dimension FormWidth, FormHeight, 
	      Twidth, Theight, statusHeight,
	      highlightThickness, shadowThickness,
	      marginWidth, marginHeight,
              textRegionWidth, textRegionHeight;

    DtEditorWidget editor = (DtEditorWidget) widget;

    /*
     * Window manager should resize in increments of a character or line.
     */
    pHints->width_inc   = M_fontWidth(editor);
    pHints->height_inc  = M_fontHeight(editor);

    /*
     * Size of the Editor (Form)
     */

    FormWidth = editor->core.width + (2 * editor->core.border_width);
    FormHeight = editor->core.height + (2 * editor->core.border_width);

    /* 
     * Size of the Edit window (text widget) 
     */
    ac=0;
    XtSetArg(al[ac], XmNwidth, &Twidth); ac++;
    XtSetArg(al[ac], XmNheight, &Theight); ac++;
    XtSetArg(al[ac], XmNhighlightThickness, &highlightThickness); ac++;
    XtSetArg(al[ac], XmNshadowThickness, &shadowThickness); ac++;
    XtSetArg(al[ac], XmNmarginWidth, &marginWidth); ac++;
    XtSetArg(al[ac], XmNmarginHeight, &marginHeight); ac++;
    XtGetValues(M_text(editor), al, ac);

    /*
     * Calculate the width & height of the area within the text widget 
     * which can actually display characters.
     */
    textRegionWidth  = (int)Twidth - 
		       (2 * ((int)highlightThickness + 
			     (int)shadowThickness + (int)marginWidth));
    textRegionHeight = (int)Theight - 
		       (2 * ((int)highlightThickness + 
			     (int)shadowThickness + (int)marginHeight));

    /*
     * Set the base width/height to the size of the area which will not
     * display characters.
     * 
     * The base width/height is used by the window manager in concert 
     * with the height & width increments to display the current size 
     * (rows & columns) of edit window (text widget) while resizing the
     * editor.
     */
    pHints->base_width  = FormWidth - textRegionWidth;
    pHints->base_height = FormHeight - textRegionHeight;
    
    /* 
     * Get the height of the status line, if it is turned on. 
     * Remember, the border width is not included in the height of the
     * status line so add it in to account for the border between the
     * edit window (text widget) and the status line.
     */

    /* XXX Actually, the statusHeight should already be included in the 
     * FormHeight, so it should not be necessary to add it in again.  
     * However, the numbers don't come out right otherwise. Hmmm? */

    if( M_status_showStatusLine(editor) == True )
    {
        XtSetArg( al[0], XmNheight, &statusHeight );
        XtGetValues( M_status_statusArea(editor), al, 1 );
        statusHeight = statusHeight + editor->core.border_width;
    }
    else
        statusHeight = 0;

    /*
     * What is being returned here is the minimum width & height of the 
     * editor. Leave room for the scrollbars, shadows, etc., plus one 
     * character.
     */
    pHints->min_width  = pHints->base_width + pHints->width_inc;
    pHints->min_height = pHints->base_width + pHints->height_inc +
                         statusHeight;

    pHints->flags = PMinSize | PResizeInc | PBaseSize;

}  /* end DtEditorGetSizeHints */


void
DtEditorInvokeFormatDialog(
	Widget widget)
{
  DtEditorWidget editor = (DtEditorWidget) widget;

  /* 
   * Create the dialog if it is the first time 
   */
  if (M_format_dialog(editor) == (Widget)NULL) 
      CreateFormatDialog( editor );

  /*
   * Post the dialog
   */
  XtUnmanageChild (M_format_dialog(editor));
  XtManageChild (M_format_dialog(editor));
  XmProcessTraversal(M_format_paragraph(editor), XmTRAVERSE_CURRENT);

} /* DtEditorInvokeFormatDialog */


void
DtEditorReset(
	Widget widget)
{
  DtEditorContentRec cr;
  DtEditorWidget editor = (DtEditorWidget) widget;

  /*
   * Reset the edit window (includes the Undo context)
   */
  cr.type = DtEDITOR_TEXT;
  cr.value.string = "";
  DtEditorSetContents( widget, &cr );

  /*
   * Reset the status line
   * Note, the current & total line displays are reset by DtEditorSetContents()
   */
  if( M_status_statusArea(editor) != (Widget) NULL ) {

    XmTextFieldSetString( M_status_messageText(editor), "" );

  }

  /*
   * Reset the Find/Change & Spell dialogs
   */
  if ( M_search_string(editor) ) 
    XtFree(M_search_string(editor));
  M_search_string(editor) = (char *)NULL;

  if ( M_replace_string(editor) ) 
    XtFree(M_replace_string(editor));
  M_replace_string(editor) = (char *)NULL;

  if (M_misspelled_string(editor))
    XtFree(M_misspelled_string(editor));
  M_misspelled_string(editor) = (char *)NULL;

  /*
   * Reset the Format Settings dialog
   */
  if (M_format_dialog(editor) != (Widget)NULL)
    ResetFormatDialog(editor);  
 
} /* end DtEditorReset */


/*
 *
 * DtEditorSelectAll - Selects the contents of an Editor widget
 *
 */
Boolean 
DtEditorSelectAll( 
	Widget widget)
{
    DtEditorWidget editor = (DtEditorWidget) widget;
    char *params=(char *)NULL;
    Cardinal num_params = 0;

    /*
     * Create an event with a correct timestamp
     */
    XEvent *event = (XEvent *) XtMalloc( sizeof(XEvent) );
    event->xkey.time = XtLastTimestampProcessed( M_display(editor) );

    SelectAll( widget, event, &params, &num_params );

    XtFree( (char *)event );

} /* end DtEditorSelectAll */


void
DtEditorTraverseToEditor(
    Widget widget)
{
  DtEditorWidget editor = (DtEditorWidget) widget;
  XmProcessTraversal(M_text(editor), XmTRAVERSE_CURRENT);
} /* end DtEditorTraverseToEditor */

