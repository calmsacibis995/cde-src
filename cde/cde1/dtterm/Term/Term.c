#ifndef lint
#ifdef  VERBOSE_REV_INFO
static char rcs_id[] = "$XConsortium: Term.c /main/cde1_maint/2 1995/09/29 11:13:20 lehors $";
#endif  /* VERBOSE_REV_INFO */
#endif  /* lint */

/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.                *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

extern char _DtTermPullInTermWhatString[];
static char *termWhatString = _DtTermPullInTermWhatString;

#include "TermHeader.h"
#include "TermPrimDebug.h"
#include "TermP.h"
#include "TermPrimAction.h"
#include "TermData.h"
#include "TermBufferP.h"
#include "TermAction.h"
#include "TermParseTable.h"
#include "TermPrimLineFontP.h"
#include "TermLineData.h"
#include "TermColor.h"
#include "TermFunctionKey.h"
#include "TermPrimRenderLineDraw.h"
#include "TermPrimMessageCatI.h"
#include "TermPrimSelectP.h"
#include <nl_types.h>
#include "TermPrimRenderP.h"
#include "TermGlyphFont.h"


static int TextInsertProc(Widget w, unsigned char *buffer, int length);
static void Resize(Widget w);
static void ClassInitialize(void);
static void ClassPartInitialize(WidgetClass wc);
static void Initialize(Widget rw, Widget nw, Arg *args, Cardinal *num_args);
static Boolean SetValues(Widget cw, Widget rw, Widget nw, ArgList args,
	Cardinal *num_args);
static void Realize(Widget w, XtValueMask *p_valueMask,
	XSetWindowAttributes *attributes);
static void Destroy(Widget w);
static void InitializeVerticalScrollBar(Widget w, Boolean initCallbacks);
static void VerticalScrollBarCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
static void StatusChangeNotify(Widget w, unsigned long mask);
static void PositionUpdate(Widget w, short row, short column);
static void GetAutoWrap( 
                        Widget wid,
                        int offset,
                        XtArgVal *value) ;
static XmImportOperator SetAutoWrap( 
                        Widget wid,
                        int offset,
                        XtArgVal *value) ;
static void GetReverseWrap( 
                        Widget wid,
                        int offset,
                        XtArgVal *value) ;
static XmImportOperator SetReverseWrap( 
                        Widget wid,
                        int offset,
                        XtArgVal *value) ;
static void GetWrapRightAfterInsert( 
                        Widget wid,
                        int offset,
                        XtArgVal *value) ;
static XmImportOperator SetWrapRightAfterInsert( 
                        Widget wid,
                        int offset,
                        XtArgVal *value) ;
static void GetAppCursorMode(
		        Widget wid,
		        int offset,
		        XtArgVal *value );
static XmImportOperator SetAppCursorMode(
		        Widget wid,
		        int offset,
		        XtArgVal *value );
static void GetAppKeypadMode(
		        Widget wid,
		        int offset,
		        XtArgVal *value );
static XmImportOperator SetAppKeypadMode(
		        Widget wid,
		        int offset,
		        XtArgVal *value );



/* action list for class: Term... */

static XtActionsRec actionsList[] = {
    {"beginning-of-buffer",	_DtTermActionBeginningOfBuffer},
    {"vt-break",		_DtTermActionBreak},
    {"end-of-buffer",		_DtTermActionEndOfBuffer},
    {"vt-function-key-execute",	_DtTermActionFunctionKeyExecute},
    {"hard-reset",		_DtTermActionHardReset},
    {"insert-line",		_DtTermActionInsertLine},
    {"keypad-key-execute",	_DtTermActionKeypadKeyExecute},
    {"move-cursor",		_DtTermActionMoveCursor},
    {"scroll",			_DtTermActionScroll},
    {"soft-reset",		_DtTermActionSoftReset},
    {"tab",			_DtTermActionTab},
    {"vt-edit-key",        	_DtTermActionEditKeyExecute},
    {"process-press",      	_DtTermPrimSelect2ButtonMouse},
    {"insert-meta-key",         _HandleMetaKeyPressed},
};

/* the resource list for Term... */
static XtResource resources[] =
{
    {
	DtNsunFunctionKeys, DtCSunFunctionKeys, XmRBoolean, sizeof(Boolean),
	XtOffsetOf(struct _DtTermRec, vt.sunFunctionKeys),
	XtRImmediate, (XtPointer) False
    },
    {
	DtNautoWrap, DtCAutoWrap, XmRBoolean, sizeof(Boolean),
	XtOffsetOf(struct _DtTermRec, vt.autoWrap),
	XtRImmediate, (XtPointer) True 
    },
    {
	DtNreverseWrap, DtCReverseWrap, XmRBoolean, sizeof(Boolean),
	XtOffsetOf(struct _DtTermRec, vt.reverseWrap),
	XtRImmediate, (XtPointer) False
    },
    {
 	SDtNwrapRightAfterInsert, SDtCWrapRightAfterInsert, XmRBoolean,
 	sizeof(Boolean),
 	XtOffsetOf(struct _DtTermRec, vt.wrapRightAfterInsert),
	XtRImmediate, (XtPointer) False
    },
    {
	DtNemulationId, DtCEmulationId, XmRString, sizeof(char *),
	XtOffsetOf( struct _DtTermPrimitiveRec, term.emulationId),
	XtRImmediate, (XtPointer) "DtTermWidget"
    },
    {
	DtNtermId, DtCTermId, XmRString, sizeof(char *),
	XtOffsetOf( struct _DtTermPrimitiveRec, term.termId),
	XtRImmediate, (XtPointer) "vt220"
    },
    {
	DtNtermName, DtCTermName, XmRString, sizeof(char *),
	XtOffsetOf( struct _DtTermPrimitiveRec, term.termName),
	XtRImmediate, (XtPointer) "dtterm"
    },
    {
	DtNc132, DtCC132, XmRBoolean, sizeof(Boolean),
	XtOffsetOf(struct _DtTermRec, vt.c132),
	XtRImmediate, (XtPointer) False
    },
    {
	DtNappCursorDefault, DtCAppCursorDefault, XmRBoolean, sizeof(Boolean),
	XtOffsetOf(struct _DtTermRec, vt.appCursorMode),
	XtRImmediate, (XtPointer) False
    },
    {
	DtNappKeypadDefault, DtCAppKeypadDefault, XmRBoolean, sizeof(Boolean),
	XtOffsetOf(struct _DtTermRec, vt.appKeypadMode),
	XtRImmediate, (XtPointer) False
    },
    {
	SDtNformFeedClearsScreen, SDtCFormFeedClearsScreen, XmRBoolean,
	sizeof(Boolean),
 	XtOffsetOf(struct _DtTermRec, vt.formFeedClearsScreen),
 	XtRImmediate, (XtPointer) False
    },
    {
	SDtNcustomColor0, SDtCCustomColor0, XtRPixel, sizeof(Pixel),
     	XtOffsetOf(struct _DtTermRec, vt.customColor[0]),
     	XtRString, "black"
    },
    {
	SDtNcustomColor1, SDtCCustomColor1, XtRPixel, sizeof(Pixel),
	XtOffsetOf(struct _DtTermRec, vt.customColor[1]),
     	XtRString, "black"
    },    
    {
	SDtNcustomColor2, SDtCCustomColor2, XtRPixel, sizeof(Pixel),
     	XtOffsetOf(struct _DtTermRec, vt.customColor[2]),
     	XtRString, "black"
    },
    {
	SDtNcustomColor3, SDtCCustomColor3, XtRPixel, sizeof(Pixel),
     	XtOffsetOf(struct _DtTermRec, vt.customColor[3]),
     	XtRString, "black"
    },
    {
	SDtNcustomColor4, SDtCCustomColor4, XtRPixel, sizeof(Pixel),
	XtOffsetOf(struct _DtTermRec, vt.customColor[4]),
     	XtRString, "black"
    },
    {
	SDtNcustomColor5, SDtCCustomColor5, XtRPixel, sizeof(Pixel),
     	XtOffsetOf(struct _DtTermRec, vt.customColor[5]),
     	XtRString, "black"
    },
    {
	SDtNcustomColor6, SDtCCustomColor6, XtRPixel, sizeof(Pixel),
     	XtOffsetOf(struct _DtTermRec, vt.customColor[6]),
     	XtRString, "black"
    },
    {
	SDtNcustomColor7, SDtCCustomColor7, XtRPixel, sizeof(Pixel),
     	XtOffsetOf(struct _DtTermRec, vt.customColor[7]),
     	XtRString, "black"
    },
    {
	SDtNglyphFont, SDtCGlyphFont, XtRPointer, sizeof(void *),
     	XtOffsetOf(struct _DtTermRec, vt.glyphFont),
     	XtRImmediate, 0
    },
    {
	SDtNglyphGutter, SDtCGlyphGutter, XtRInt, sizeof(int),
     	XtOffsetOf(struct _DtTermRec, vt.glyphGutter),
     	XtRImmediate, 0
    },
#ifdef	DKS
    {
	DtNstickyPrevCursor, DtCStickyCursor, XmRBoolean,
	sizeof(Boolean),
	XtOffsetOf(struct _DtTermRec, vt.stickyPrevCursor),
	XtRImmediate, (XtPointer) False
    },
    {
	DtNstickyNextCursor, DtCStickyCursor, XmRBoolean,
	sizeof(Boolean),
	XtOffsetOf(struct _DtTermRec, vt.stickyNextCursor),
	XtRImmediate, (XtPointer) False
    },
    {
	DtNstatusChangeCallback, DtCCallback, XmRCallback,
	sizeof(XtCallbackList),
	XtOffsetOf( struct _DtTermRec, vt.statusChangeCallback),
	XmRPointer, (XtPointer) NULL
    },
    {
	DtNverticalScrollBar, DtCVerticalScrollBar, XmRWidget, sizeof(Widget),
	XtOffsetOf( struct _DtTermRec, vt.verticalScrollBar),
	XmRImmediate, (XtPointer) NULL
    },
#endif	/* DKS */
};


/****************
 *
 * Synthetic resources for those ugly wart variables...
 *
 ****************/
static XmSyntheticResource GetResources[] =
{
   { DtNautoWrap,
     sizeof (Boolean),
     XtOffsetOf(struct _DtTermRec, vt.autoWrap),
     GetAutoWrap,
     SetAutoWrap },
   { DtNreverseWrap,
     sizeof (Boolean),
     XtOffsetOf(struct _DtTermRec, vt.reverseWrap),
     GetReverseWrap,
     SetReverseWrap },
   { SDtNwrapRightAfterInsert,
     sizeof (Boolean),
     XtOffsetOf(struct _DtTermRec, vt.wrapRightAfterInsert),
     GetWrapRightAfterInsert,
     SetWrapRightAfterInsert },
   { DtNappCursorDefault,
     sizeof (Boolean),
     XtOffsetOf(struct _DtTermRec, vt.appCursorMode),
     GetAppCursorMode,
     SetAppCursorMode },
   { DtNappKeypadDefault,
     sizeof (Boolean),
     XtOffsetOf(struct _DtTermRec, vt.appKeypadMode),
     GetAppKeypadMode,
     SetAppKeypadMode },
    
};

/*
 * Goofy debug'ed code by HP.
 * IBM's JP kbd accepts Shift+KP_Multiply as a valid key sequence.
 * I don't know why HP added non-exposed(internal) Xt's action ???
 * Anyway, remove KP_Multiply entry from defaultTranslations[]
 */
#ifdef AIX_ILS
static char defaultTranslations[] = "\
        Meta <Key>x:            insert-meta-key(8)\n\
        Alt <Key>x:             insert-meta-key(8)\n\
        <Key>osfCancel:         process-cancel()\n\
	<Key>osfCopy:		copy-clipboard()\n\
	<Key>osfCut:		copy-clipboard()\n\
	<Key>osfPaste:		paste-clipboard()\n\
        <Key>osfBeginLine:      beginning-of-buffer()\n\
        <Key>Home:              beginning-of-buffer()\n\
        <Key>osfEndLine:        end-of-buffer()\n\
        <Key>End:               end-of-buffer()\n\
	Shift<Key>osfUp:	scroll(1,line)\n\
	Shift<Key>Up:	        scroll(1,line)\n\
	Shift<Key>osfDown:	scroll(-1,line)\n\
	Shift<Key>Down:	        scroll(-1,line)\n\
	<Key>osfUp:		move-cursor(up)\n\
	<Key>Up:		move-cursor(up)\n\
	<Key>osfDown:		move-cursor(down)\n\
	<Key>Down:		move-cursor(down)\n\
	<Key>osfLeft:		move-cursor(backward)\n\
	<Key>Left:		move-cursor(backward)\n\
	<Key>osfRight:		move-cursor(forward)\n\
	<Key>Right:		move-cursor(forward)\n\
	<Key>Find:	        vt-edit-key(find)\n\
	<Key>osfInsert:         vt-edit-key(insert)\n\
	<Key>Insert:            vt-edit-key(insert)\n\
	:<Key>osfDelete:         vt-edit-key(delete)\n\
	:<Key>Delete:            vt-edit-key(delete)\n\
	<Key>osfSelect:	        vt-edit-key(select)\n\
	<Key>Select:	        vt-edit-key(select)\n\
	~Shift<Key>osfPageUp:	vt-edit-key(prior)\n\
	~Shift<Key>Prior:	vt-edit-key(prior)\n\
	~Shift<Key>osfPageDown:	vt-edit-key(next)\n\
	~Shift<Key>Next:	vt-edit-key(next)\n\
	<Key>osfPageUp:		scroll(-1,page)\n\
	<Key>Prior:	        scroll(-1,page)\n\
	<Key>osfPageDown:	scroll(1,page)\n\
	<Key>Next:	        scroll(1,page)\n\
	Mod1<Key>Break:		soft-reset()\n\
	Shift<Key>Break:	hard-reset()\n\
	~Shift ~Mod1<Key>Break:	vt-break(long)\n\
	Ctrl<Key>Cancel:	stop(long)\n\
	~Ctrl<Key>Cancel:	stop()\n\
	~Shift<Key>Tab:		tab()\n\
        <Key>osfActivate:	keypad-key-execute(enter)\n\
        <Key>KP_Space:		keypad-key-execute(space)\n\
	<Key>KP_Tab:		keypad-key-execute(tab)\n\
        <Key>KP_Enter:		keypad-key-execute(enter)\n\
        <Key>KP_F1:		keypad-key-execute(f1)\n\
        <Key>KP_F2:		keypad-key-execute(f2)\n\
        <Key>KP_F3:		keypad-key-execute(f3)\n\
        <Key>KP_F4:		keypad-key-execute(f4)\n\
        <Key>KP_Equal:		keypad-key-execute(equal)\n\
        <Key>KP_Multiply:	keypad-key-execute(multiply)\n\
        <Key>KP_Add:		keypad-key-execute(add)\n\
        <Key>KP_Separator:	keypad-key-execute(separator)\n\
        <Key>KP_Subtract:	keypad-key-execute(subtract)\n\
        <Key>KP_Decimal:	keypad-key-execute(decimal)\n\
        <Key>KP_Divide:		keypad-key-execute(divide)\n\
	!Mod1<Key>KP_0:		insert()\n\
	!Mod1<Key>KP_1:		insert()\n\
	!Mod1<Key>KP_2:		insert()\n\
	!Mod1<Key>KP_3:		insert()\n\
	!Mod1<Key>KP_4:		insert()\n\
	!Mod1<Key>KP_5:		insert()\n\
	!Mod1<Key>KP_6:		insert()\n\
	!Mod1<Key>KP_7:		insert()\n\
	!Mod1<Key>KP_8:		insert()\n\
	!Mod1<Key>KP_9:		insert()\n\
        <Key>KP_0:		keypad-key-execute(0)\n\
        <Key>KP_1:		keypad-key-execute(1)\n\
        <Key>KP_2:		keypad-key-execute(2)\n\
        <Key>KP_3:		keypad-key-execute(3)\n\
        <Key>KP_4:		keypad-key-execute(4)\n\
        <Key>KP_5:		keypad-key-execute(5)\n\
        <Key>KP_6:		keypad-key-execute(6)\n\
        <Key>KP_7:		keypad-key-execute(7)\n\
        <Key>KP_8:		keypad-key-execute(8)\n\
        <Key>KP_9:		keypad-key-execute(9)\n\
	Shift<Key>F1:		vt-function-key-execute(1, UDK)\n\
	Shift<Key>F2:		vt-function-key-execute(2, UDK)\n\
	Shift<Key>F3:		vt-function-key-execute(3, UDK)\n\
	Shift<Key>F4:		vt-function-key-execute(4, UDK)\n\
	Shift<Key>F5:		vt-function-key-execute(5, UDK)\n\
	Shift<Key>F6:		vt-function-key-execute(6, UDK)\n\
	Shift<Key>F7:		vt-function-key-execute(7, UDK)\n\
	Shift<Key>F8:		vt-function-key-execute(8, UDK)\n\
	Shift<Key>F9:		vt-function-key-execute(9, UDK)\n\
	Shift<Key>F10:		vt-function-key-execute(10, UDK)\n\
	Shift<Key>F11:		vt-function-key-execute(11, UDK)\n\
	Shift<Key>F12:		vt-function-key-execute(12, UDK)\n\
	Shift<Key>F13:		vt-function-key-execute(13, UDK)\n\
	Shift<Key>F14:		vt-function-key-execute(14, UDK)\n\
	Shift<Key>F15:		vt-function-key-execute(15, UDK)\n\
	Shift<Key>F16:		vt-function-key-execute(16, UDK)\n\
	Shift<Key>F17:		vt-function-key-execute(17, UDK)\n\
	Shift<Key>F18:		vt-function-key-execute(18, UDK)\n\
	Shift<Key>F19:		vt-function-key-execute(19, UDK)\n\
	Shift<Key>F20:		vt-function-key-execute(20, UDK)\n\
	Shift<Key>F21:		vt-function-key-execute(21, UDK)\n\
	Shift<Key>F22:		vt-function-key-execute(22, UDK)\n\
	Shift<Key>F23:		vt-function-key-execute(23, UDK)\n\
	Shift<Key>F24:		vt-function-key-execute(24, UDK)\n\
	Shift<Key>F25:		vt-function-key-execute(25, UDK)\n\
	Shift<Key>F26:		vt-function-key-execute(26, UDK)\n\
	Shift<Key>F27:		vt-function-key-execute(27, UDK)\n\
	Shift<Key>F28:		vt-function-key-execute(28, UDK)\n\
	Shift<Key>F29:		vt-function-key-execute(29, UDK)\n\
	Shift<Key>F30:		vt-function-key-execute(30, UDK)\n\
	Shift<Key>F31:		vt-function-key-execute(31, UDK)\n\
	Shift<Key>F32:		vt-function-key-execute(32, UDK)\n\
	Shift<Key>F33:		vt-function-key-execute(33, UDK)\n\
	Shift<Key>F34:		vt-function-key-execute(34, UDK)\n\
	Shift<Key>F35:		vt-function-key-execute(35, UDK)\n\
	~Shift<Key>F1:		vt-function-key-execute(1, function)\n\
	~Shift<Key>F2:		vt-function-key-execute(2, function)\n\
	~Shift<Key>F3:		vt-function-key-execute(3, function)\n\
	~Shift<Key>F4:		vt-function-key-execute(4, function)\n\
	~Shift<Key>F5:		vt-function-key-execute(5, function)\n\
	~Shift<Key>F6:		vt-function-key-execute(6, function)\n\
	~Shift<Key>F7:		vt-function-key-execute(7, function)\n\
	~Shift<Key>F8:		vt-function-key-execute(8, function)\n\
	~Shift<Key>F9:		vt-function-key-execute(9, function)\n\
	~Shift<Key>F10:		vt-function-key-execute(10, function)\n\
	~Shift<Key>F11:		vt-function-key-execute(11, function)\n\
	~Shift<Key>F12:		vt-function-key-execute(12, function)\n\
	~Shift<Key>F13:		vt-function-key-execute(13, function)\n\
	~Shift<Key>F14:		vt-function-key-execute(14, function)\n\
	~Shift<Key>F15:		vt-function-key-execute(15, function)\n\
	~Shift<Key>F16:		vt-function-key-execute(16, function)\n\
	~Shift<Key>F17:		vt-function-key-execute(17, function)\n\
	~Shift<Key>F18:		vt-function-key-execute(18, function)\n\
	~Shift<Key>F19:		vt-function-key-execute(19, function)\n\
	~Shift<Key>F20:		vt-function-key-execute(20, function)\n\
	~Shift<Key>F21:		vt-function-key-execute(21, function)\n\
	~Shift<Key>F22:		vt-function-key-execute(22, function)\n\
	~Shift<Key>F23:		vt-function-key-execute(23, function)\n\
	~Shift<Key>F24:		vt-function-key-execute(24, function)\n\
	~Shift<Key>F25:		vt-function-key-execute(25, function)\n\
	~Shift<Key>F26:		vt-function-key-execute(26, function)\n\
	~Shift<Key>F27:		vt-function-key-execute(27, function)\n\
	~Shift<Key>F28:		vt-function-key-execute(28, function)\n\
	~Shift<Key>F29:		vt-function-key-execute(29, function)\n\
	~Shift<Key>F30:		vt-function-key-execute(30, function)\n\
	~Shift<Key>F31:		vt-function-key-execute(31, function)\n\
	~Shift<Key>F32:		vt-function-key-execute(32, function)\n\
	~Shift<Key>F33:		vt-function-key-execute(33, function)\n\
	~Shift<Key>F34:		vt-function-key-execute(34, function)\n\
	~Shift<Key>F35:		vt-function-key-execute(35, function)\n\
	<KeyRelease>:		key-release()\n\
	<KeyPress>:		insert()\n\
        ~Shift~Ctrl<Btn1Down>:	grab-focus()\n\
        Shift~Ctrl<Btn1Down>:   extend-start()\n\
	~Ctrl<Btn1Motion>:      select-adjust()\n\
	~Ctrl<Btn1Up>:		extend-end()\n\
        ~Shift<Btn2Down>:	process-bdrag()\n\
        ~Shift<Btn2Up>:		copy-to()\n\
	<EnterWindow>:		enter()\n\
	<LeaveWindow>:		leave()\n\
	<FocusIn>:		focus-in()\n\
	<FocusOut>:		focus-out()\n\
	";
#else /* AIX_ILS */
static char defaultTranslations[] = "\
        Meta <Key>x:        insert-meta-key(8)\n\
        Alt <Key>x:         insert-meta-key(8)\n\
	Shift ~Ctrl<Key>KP_Multiply:	XtDisplayInstalledAccelerators()\n\
	~Shift Ctrl<Key>KP_Multiply:	XtDisplayAccelerators()\n\
	Shift Ctrl<Key>KP_Multiply:	XtDisplayTranslations()\n\
        <Key>osfCancel:         process-cancel()\n\
	<Key>osfCopy:		copy-clipboard()\n\
	<Key>osfCut:		copy-clipboard()\n\
	<Key>osfPaste:		paste-clipboard()\n\
        <Key>osfBeginLine:      beginning-of-buffer()\n\
        <Key>Home:              beginning-of-buffer()\n\
        <Key>osfEndLine:        end-of-buffer()\n\
        <Key>End:               end-of-buffer()\n\
	Shift<Key>osfUp:	scroll(1,line)\n\
	Shift<Key>Up:	        scroll(1,line)\n\
	Shift<Key>osfDown:	scroll(-1,line)\n\
	Shift<Key>Down:	        scroll(-1,line)\n\
	<Key>osfUp:		move-cursor(up)\n\
	<Key>Up:		move-cursor(up)\n\
	<Key>osfDown:		move-cursor(down)\n\
	<Key>Down:		move-cursor(down)\n\
	<Key>osfLeft:		move-cursor(backward)\n\
	<Key>Left:		move-cursor(backward)\n\
	<Key>osfRight:		move-cursor(forward)\n\
	<Key>Right:		move-cursor(forward)\n\
	<Key>Find:	        vt-edit-key(find)\n\
	<Key>osfInsert:         vt-edit-key(insert)\n\
	<Key>Insert:            vt-edit-key(insert)\n\
	<Key>osfSelect:	        vt-edit-key(select)\n\
	<Key>Select:	        vt-edit-key(select)\n\
	~Shift<Key>osfPageUp:	scroll(-1,page)\n\
	~Shift<Key>Prior:	scroll(-1,page)\n\
	~Shift<Key>osfPageDown:	scroll(1,page)\n\
	~Shift<Key>Next:	scroll(1,page)\n\
	<Key>osfPageUp:		scroll(-1,page)\n\
	<Key>Prior:	        scroll(-1,page)\n\
	<Key>osfPageDown:	scroll(1,page)\n\
	<Key>Next:	        scroll(1,page)\n\
	Mod1<Key>Break:		soft-reset()\n\
	Shift<Key>Break:	hard-reset()\n\
	~Shift ~Mod1<Key>Break:	vt-break(long)\n\
	Ctrl<Key>Cancel:	stop(long)\n\
	~Ctrl<Key>Cancel:	stop()\n\
	~Shift<Key>Tab:		tab()\n\
        <Key>osfActivate:	keypad-key-execute(enter)\n\
        <Key>KP_Space:		keypad-key-execute(space)\n\
	<Key>KP_Tab:		keypad-key-execute(tab)\n\
        <Key>KP_Enter:		keypad-key-execute(enter)\n\
        <Key>KP_F1:		keypad-key-execute(f1)\n\
        <Key>F1:		keypad-key-execute(f1)\n\
        <Key>Help:		keypad-key-execute(help)\n\
        <Key>osfHelp:		keypad-key-execute(help)\n\
        <Key>KP_F2:		keypad-key-execute(f2)\n\
        <Key>KP_F3:		keypad-key-execute(f3)\n\
        <Key>KP_F4:		keypad-key-execute(f4)\n\
        <Key>KP_Equal:		keypad-key-execute(equal)\n\
        <Key>KP_Multiply:	keypad-key-execute(multiply)\n\
        <Key>KP_Add:		keypad-key-execute(add)\n\
        <Key>KP_Separator:	keypad-key-execute(separator)\n\
        <Key>KP_Subtract:	keypad-key-execute(subtract)\n\
        <Key>KP_Decimal:	keypad-key-execute(decimal)\n\
        <Key>KP_Divide:		keypad-key-execute(divide)\n\
	!Mod1<Key>KP_0:		insert()\n\
	!Mod1<Key>KP_1:		insert()\n\
	!Mod1<Key>KP_2:		insert()\n\
	!Mod1<Key>KP_3:		insert()\n\
	!Mod1<Key>KP_4:		insert()\n\
	!Mod1<Key>KP_5:		insert()\n\
	!Mod1<Key>KP_6:		insert()\n\
	!Mod1<Key>KP_7:		insert()\n\
	!Mod1<Key>KP_8:		insert()\n\
	!Mod1<Key>KP_9:		insert()\n\
        <Key>KP_0:		keypad-key-execute(0)\n\
        <Key>KP_1:		keypad-key-execute(1)\n\
        <Key>KP_2:		keypad-key-execute(2)\n\
        <Key>KP_3:		keypad-key-execute(3)\n\
        <Key>KP_4:		keypad-key-execute(4)\n\
        <Key>KP_5:		keypad-key-execute(5)\n\
        <Key>KP_6:		keypad-key-execute(6)\n\
        <Key>KP_7:		keypad-key-execute(7)\n\
        <Key>KP_8:		keypad-key-execute(8)\n\
        <Key>KP_9:		keypad-key-execute(9)\n\
	Shift<Key>F1:		vt-function-key-execute(1, UDK)\n\
	Shift<Key>F2:		vt-function-key-execute(2, UDK)\n\
	Shift<Key>F3:		vt-function-key-execute(3, UDK)\n\
	Shift<Key>F4:		vt-function-key-execute(4, UDK)\n\
	Shift<Key>F5:		vt-function-key-execute(5, UDK)\n\
	Shift<Key>F6:		vt-function-key-execute(6, UDK)\n\
	Shift<Key>F7:		vt-function-key-execute(7, UDK)\n\
	Shift<Key>F8:		vt-function-key-execute(8, UDK)\n\
	Shift<Key>F9:		vt-function-key-execute(9, UDK)\n\
	Shift<Key>F10:		vt-function-key-execute(10, UDK)\n\
	Shift<Key>F11:		vt-function-key-execute(11, UDK)\n\
	Shift<Key>SunF36:	vt-function-key-execute(11, UDK)\n\
	Shift<Key>F12:		vt-function-key-execute(12, UDK)\n\
	Shift<Key>SunF37:	vt-function-key-execute(12, UDK)\n\
	Shift<Key>F13:		vt-function-key-execute(13, UDK)\n\
	Shift<Key>F14:		vt-function-key-execute(14, UDK)\n\
	Shift<Key>F15:		vt-function-key-execute(15, UDK)\n\
	Shift<Key>F16:		vt-function-key-execute(16, UDK)\n\
	Shift<Key>F17:		vt-function-key-execute(17, UDK)\n\
	Shift<Key>F18:		vt-function-key-execute(18, UDK)\n\
	Shift<Key>F19:		vt-function-key-execute(19, UDK)\n\
	Shift<Key>F20:		vt-function-key-execute(20, UDK)\n\
	Shift<Key>F21:		vt-function-key-execute(21, UDK)\n\
	Shift<Key>F22:		vt-function-key-execute(22, UDK)\n\
	Shift<Key>F23:		vt-function-key-execute(23, UDK)\n\
	Shift<Key>F24:		vt-function-key-execute(24, UDK)\n\
	Shift<Key>F25:		vt-function-key-execute(25, UDK)\n\
	Shift<Key>F26:		vt-function-key-execute(26, UDK)\n\
	Shift<Key>F27:		vt-function-key-execute(27, UDK)\n\
	Shift<Key>F28:		vt-function-key-execute(28, UDK)\n\
	Shift<Key>F29:		vt-function-key-execute(29, UDK)\n\
	Shift<Key>F30:		vt-function-key-execute(30, UDK)\n\
	Shift<Key>F31:		vt-function-key-execute(31, UDK)\n\
	Shift<Key>F32:		vt-function-key-execute(32, UDK)\n\
	Shift<Key>F33:		vt-function-key-execute(33, UDK)\n\
	Shift<Key>F34:		vt-function-key-execute(34, UDK)\n\
	Shift<Key>F35:		vt-function-key-execute(35, UDK)\n\
	~Shift<Key>F1:		vt-function-key-execute(1, function)\n\
	~Shift<Key>F2:		vt-function-key-execute(2, function)\n\
	~Shift<Key>F3:		vt-function-key-execute(3, function)\n\
	~Shift<Key>F4:		vt-function-key-execute(4, function)\n\
	~Shift<Key>F5:		vt-function-key-execute(5, function)\n\
	~Shift<Key>F6:		vt-function-key-execute(6, function)\n\
	~Shift<Key>F7:		vt-function-key-execute(7, function)\n\
	~Shift<Key>F8:		vt-function-key-execute(8, function)\n\
	~Shift<Key>F9:		vt-function-key-execute(9, function)\n\
	~Shift<Key>F10:		vt-function-key-execute(10, function)\n\
	~Shift<Key>F11:		vt-function-key-execute(11, function)\n\
	~Shift<Key>SunF36:	vt-function-key-execute(11, UDK)\n\
	~Shift<Key>F12:		vt-function-key-execute(12, function)\n\
	~Shift<Key>SunF37:	vt-function-key-execute(12, UDK)\n\
	~Shift<Key>F13:		vt-function-key-execute(13, function)\n\
	~Shift<Key>F14:		vt-function-key-execute(14, function)\n\
	~Shift<Key>F15:		vt-function-key-execute(15, function)\n\
	~Shift<Key>F16:		vt-function-key-execute(16, function)\n\
	~Shift<Key>F17:		vt-function-key-execute(17, function)\n\
	~Shift<Key>F18:		vt-function-key-execute(18, function)\n\
	~Shift<Key>F19:		vt-function-key-execute(19, function)\n\
	~Shift<Key>F20:		vt-function-key-execute(20, function)\n\
	~Shift<Key>F21:		vt-function-key-execute(21, function)\n\
	~Shift<Key>F22:		vt-function-key-execute(22, function)\n\
	~Shift<Key>F23:		vt-function-key-execute(23, function)\n\
	~Shift<Key>F24:		vt-function-key-execute(24, function)\n\
	~Shift<Key>F25:		vt-function-key-execute(25, function)\n\
	~Shift<Key>F26:		vt-function-key-execute(26, function)\n\
	~Shift<Key>F27:		vt-function-key-execute(27, function)\n\
	~Shift<Key>F28:		vt-function-key-execute(28, function)\n\
	~Shift<Key>F29:		vt-function-key-execute(29, function)\n\
	~Shift<Key>F30:		vt-function-key-execute(30, function)\n\
	~Shift<Key>F31:		vt-function-key-execute(31, function)\n\
	~Shift<Key>F32:		vt-function-key-execute(32, function)\n\
	~Shift<Key>F33:		vt-function-key-execute(33, function)\n\
	~Shift<Key>F34:		vt-function-key-execute(34, function)\n\
	~Shift<Key>F35:		vt-function-key-execute(35, function)\n\
	<KeyRelease>:		key-release()\n\
	<KeyPress>:		insert()\n\
        ~Shift~Ctrl<Btn1Down>:	grab-focus()\n\
        Shift~Ctrl<Btn1Down>:   extend-start()\n\
	~Ctrl<Btn1Motion>:      select-adjust()\n\
	~Ctrl<Btn1Up>:		extend-end()\n\
        ~Shift<Btn2Down>:	process-bdrag()\n\
        ~Shift<Btn2Up>:		copy-to()\n\
	<EnterWindow>:		enter()\n\
	<LeaveWindow>:		leave()\n\
	<FocusIn>:		focus-in()\n\
	<FocusOut>:		focus-out()\n\
	";
#endif /* AIX_ILS */

/* global class record for instances of class: Vt
 */

externaldef(vtclassrec) DtTermClassRec dtTermClassRec =
{
    /* core class record */
    {
	/* superclass		*/	(WidgetClass) &dtTermPrimitiveClassRec,
	/* class_name		*/	"DtTerm",
	/* widget_size		*/	sizeof(DtTermRec),
	/* class_initialize	*/	ClassInitialize,
	/* class_part_init	*/	ClassPartInitialize,
	/* class_inited		*/	FALSE,
	/* initialize		*/	Initialize,
	/* initialize_hook	*/	(XtArgsProc) NULL,
	/* realize		*/	XtInheritRealize,
	/* actions		*/	actionsList,
	/* num_actions		*/	XtNumber(actionsList),
	/* resources		*/	resources,
	/* num_resources	*/	XtNumber(resources),
	/* xrm_class		*/	NULLQUARK,
	/* compress_motion	*/	TRUE,
	/* compress_exposure	*/	FALSE,
	/* compress_enterlv	*/	TRUE,
	/* visible_interest	*/	TRUE,
	/* destroy		*/	Destroy,
	/* resize		*/	XtInheritResize, /* Resize */
	/* expose		*/	XtInheritExpose,
	/* set_values		*/	SetValues,
	/* set_values_hook	*/	(XtArgsFunc) NULL,
	/* set_values_almost	*/	XtInheritSetValuesAlmost,
	/* get_values_hook	*/	(XtArgsProc) NULL,
	/* accept_focus		*/	(XtAcceptFocusProc) NULL,
	/* version		*/	XtVersion,
	/* callback_private	*/	(XtPointer) NULL,
	/* tm_table		*/	defaultTranslations,
	/* query_geometry	*/	(XtGeometryHandler) NULL,
	/* display_accelerator	*/	(XtStringProc) NULL,
	/* extension		*/	(XtPointer) NULL,
    },

    /* primitive class rec */
    {
	/* Primitive border_highlight	*/  XmInheritWidgetProc,
	/* Primitive border_unhighlight	*/  XmInheritWidgetProc,
	/* translations			*/  "" /*NULL*/ /*XtInheritTranslations*/,
	/* arm_and_activate		*/  NULL,
	/* get resources		*/  (XmSyntheticResource *) GetResources,
	/* num get_resources		*/  XtNumber(GetResources),
	/* extension			*/  (XtPointer) NULL,
    },

    /* term class record */
    {
        /* parser_start_state           */  &_DtTermStateStart,
	/* use_history_buffer           */  True,
	/* allow_scroll_below_buffer    */  False,
	/* wrap_right_after_insert	*/  False,
        /* buffer_create_proc           */  _DtTermBufferCreateBuffer,
	/* buffer_free_proc		*/  _DtTermBufferFreeBuffer,
	/* term_insert_proc		*/  TextInsertProc,
        /* sizeOfBuffer                 */  (short) sizeof(DtTermBufferRec),
        /* sizeOfLine                   */  (short) sizeof(DtTermLineRec),
        /* sizeOfEnh                    */  (short) sizeof(DtTermEnhPart),
                        
    },

    /* vt class record */
    {
	0
    }
};

externaldef(vtwidgetclass) WidgetClass dtTermWidgetClass =
	(WidgetClass)&dtTermClassRec;

/* Class Initialize...
 */
static void
ClassInitialize(void)
{
    return;
}

static void
ClassPartInitialize(WidgetClass w_class)
{
}


/* 
** Initialize the modes to their default values...
*/
static void
initializeModes
(
    DtTermData td
)
{
    td->applicationMode  = False; /* Normal mode */
    td->applicationKPMode= False; /* Normal mode */
    td->tpd->transmitFunctions = True;  /* ALWAYS true for ANSI */
    td->originMode   = False;        /* true when origin mode is active  */
    td->S8C1TMode    = False;        /* use 7 bit c1 codes               */
    td->enhFieldState = FIELD_UNPROTECT; /* Vt selective erase mode          */
    td->fixCursesMode = False;         /* xterm fix curses */
    td->userKeysLocked = False;        /* are User Defined Keys locked? */
    td->needToLockUserKeys = False;        /* Parser state */
    td->saveCursor.cursorRow = 0;    /* init for Save Cursor (Esc 7) */
    td->saveCursor.cursorColumn = 0;    /* init for Save Cursor (Esc 7) */
    td->saveCursor.originMode = False;  /* init for Save Cursor (Esc 7) */
    td->saveCursor.wrapMode  = True;    /* init for Save Cursor (Esc 7) */
    td->saveCursor.enhVideoState = 0;   /* init for Save Cursor (Esc 7) */
    td->saveCursor.enhFieldState = FIELD_UNPROTECT;
    td->saveCursor.enhFgColorState = 0;
    td->saveCursor.enhBgColorState = 0; /* init for Save Cursor (Esc 7) */
    td->saveCursor.GL = td->GL;		/* init for Save Cursor (Esc 7) */
    td->saveCursor.GR = td->GR;		/* init for Save Cursor (Esc 7) */
    td->saveCursor.G0 = td->G0;		/* init for Save Cursor (Esc 7) */
    td->saveCursor.G1 = td->G1;		/* init for Save Cursor (Esc 7) */
    td->saveCursor.G2 = td->G2;		/* init for Save Cursor (Esc 7) */
    td->saveCursor.G3 = td->G3;		/* init for Save Cursor (Esc 7) */
    td->saveCursor.singleShiftFont = td->singleShiftFont;
    td->saveCursor.singleShiftPending = td->singleShiftPending;
    td->saveDECMode.allow80_132ColMode = False;  
    td->saveDECMode.col132Mode         = False;  
    td->saveDECMode.applicationMode    = False;  
    td->saveDECMode.applicationKPMode  = False;  
    td->saveDECMode.originMode         = False;  
    td->saveDECMode.wrapMode           = False;  
    td->saveDECMode.jumpScrollMode     = True;  
    td->saveDECMode.inverseVideoMode   = False; 
    td->saveDECMode.fixCursesMode     = False;
    td->saveDECMode.reverseWrapMode   = False;
    td->saveDECMode.marginBellMode    = False;
    td->saveDECMode.cursorVisible     = True;
    td->saveDECMode.logging           = False;

}

unsigned char
_DtTermGetRenderFontIndex(Widget w, int id)
{
    DtTermWidget tw = (DtTermWidget) w;
    DtTermData td = tw->vt.td;
    int i;

    /* search for the index that matches this id... */
    for (i = 0; i < (sizeof(td->renderFonts) / sizeof(td->renderFonts[0]));
	    i++) {
	if (td->renderFonts[i].id == id) {
	    return(i);
	}
    }

    /* default to base if not found... */
    return(0);
}
    
/*ARGSUSED*/
static void
Initialize(Widget ref_w, Widget w, Arg *args, Cardinal *num_args)
{
    DtTermPrimitiveWidget tpw = (DtTermPrimitiveWidget) w;
    DtTermWidget tw = (DtTermWidget) w;
    DtTermData td;
    int i;

    Debug('T', timeStamp("Term Initialize() starting"));

    /* initialize...
     */

    /* malloc Vt dataspace... */
    tw->vt.td =
	    (DtTermData) XtMalloc(sizeof(DtTermDataRec));
    td = tw->vt.td;

    /* clear malloc'ed memory... */
    memset(td, '\0', sizeof(DtTermDataRec));

    /* stick a pointer to the DtTermPrimData struct in our DtTermData... */
    td->tpd = tw->term.tpd;

    /* set up our fonts...
     */
    for (i = 0; i < (sizeof(td->renderFonts) / sizeof(td->renderFonts[0]));
	    i++) {
	td->renderFonts[i].termFont = (TermFont) 0;
	td->renderFonts[i].id = -1;
    }
    td->renderFonts[0].termFont = td->tpd->defaultTermFont;
    td->renderFonts[0].id = 'B';
    td->baseFontIndex = 0;

    /* allocate a line drawing font... */
    td->renderFonts[1].termFont =
	    _DtTermPrimRenderLineDrawCreate(w,	/* Widget		*/
	    _DtTermGlyphs,			/* glyphInfo		*/
	    _DtTermNumGlyphs,			/* numGlyphs		*/
	    tw->term.widthInc,			/* width		*/
	    tw->term.ascent,			/* ascent		*/
	    tw->term.heightInc - tw->term.ascent);
						/* descent		*/

    td->renderFonts[1].id = '0';
    td->altFontIndex = 1;


    td->renderFonts[2].termFont = td->tpd->defaultBoldTermFont;

    /* make a glyph font */
    td->renderFonts[3].termFont =
	    _DtTermPrimRenderGlyphFontCreate(w, (GlyphFont **)&tw->vt.glyphFont);
    td->renderFonts[3].id = 'G';

    td->G0 = RENDER_FONT_NORMAL;
    td->G1 = RENDER_FONT_LINEDRAW;		/* DKS!!! change this	*/
    td->G2 = RENDER_FONT_NORMAL;
    td->G3 = RENDER_FONT_NORMAL;

    td->GL = &td->G0;
    td->GR = &td->G0;

    td->singleShiftFont = RENDER_FONT_NORMAL;
    td->singleShiftPending = False;

    _DtTermFunctionKeyInit(w);

    /* 
    ** Initialize modes...
    */
    initializeModes(td);

    tpw->term.tpd->autoWrapRight = tw->vt.autoWrap ;
    tw->vt.td->reverseWrapMode = tw->vt.reverseWrap ;
    tpw->term.tpd->wrapRightAfterInsert = tw->vt.wrapRightAfterInsert ;

    /* Initialize color...
     */
    _DtTermColorInit(w);

    if ( !strcmp(tpw->term.termId,"vt100")) td->terminalId  = 100 ;
    else if ( !strcmp(tpw->term.termId,"vt101")) td->terminalId  = 101 ;
    else if ( !strcmp(tpw->term.termId,"vt102")) td->terminalId  = 102 ;
    else td->terminalId  = 220 ;               /* vt200  */
    if ( td->terminalId < 200 ) td->compatLevel = 1;
    else td->compatLevel = 2;

    _DtTermPrimSelectInitBtnEvents(w) ;  /* set button events */

    Debug('T', timeStamp("Term Initialize() finished"));
    return;
}

#ifdef	DKS
static void
InitializeVerticalScrollBar(Widget w, Boolean initCallbacks)
{
    DtTermWidget tw = (DtTermWidget) w;
    Arg arglist[20];
    int i;

    if (initCallbacks) {
	/* set up the scrollbar callbacks... */
	XtAddCallback(tw->vt.verticalScrollBar, XmNdragCallback,
		VerticalScrollBarCallback, (XtPointer) w);
	XtAddCallback(tw->vt.verticalScrollBar,
		XmNvalueChangedCallback, VerticalScrollBarCallback,
		(XtPointer) w);
    }

    if (!tw->vt.td) {
	/* no termBuffer yet, we set anything yet... */
	return;
    }

    /* update the scrollbar... */
    if (tw->vt.verticalScrollBar) {
	tw->vt.verticalScrollBarMaximum =
		(tw->term.tpd->lastUsedRow - tw->term.tpd->memoryLockRow) +
		(tw->term.tpd->term.rows - tw->term.tpd->memoryLockRow);
	tw->vt.verticalScrollBarSliderSize =
		tw->term.rows - tw->term.tpd->memoryLockRow;
	tw->vt.verticalScrollBarPageIncrement =
		tw->term.rows - tw->term.tpd->memoryLockRow;
	tw->vt.verticalScrollBarValue = tw->term.tpd->topRow;

	i = 0;
	XtSetArg(arglist[i], XmNincrement, 1); i++;
	XtSetArg(arglist[i], XmNminimum, 0); i++;
	XtSetArg(arglist[i], XmNmaximum,
		tw->term.verticalScrollBarMaximum); i++;
	XtSetArg(arglist[i], XmNpageIncrement,
		tw->term.verticalScrollBarPageIncrement); i++;
	XtSetArg(arglist[i], XmNsliderSize,
		tw->term.verticalScrollBarSliderSize); i++;
	XtSetArg(arglist[i], XmNvalue, tw->term.verticalScrollBarValue);
		i++;
	XtSetValues(tw->term.verticalScrollBar, arglist, i);
    }
}
#endif	/* DKS */

static void
Resize(Widget w)
{
    DtTermWidget tw = (DtTermWidget) w;
    DtTermData td = tw->vt.td;

    /* let our superclass (the Term Widget) perform the resize... */
    tw->core.widget_class->core_class.superclass->core_class.resize(w);

#ifdef	DKS
    /* and update the scrollbar... */
    InitializeVerticalScrollBar(w, False);
#endif	/* DKS */
}
/***************************************************************************
 *									   *
 * Import and Export functions for those resources that can't really be	   *
 * accessed through the widget DS.					   *
 *									   *
 ***************************************************************************/
 /* ARGSUSED */
static void 
#ifdef _NO_PROTO
GetAutoWrap( wid, offset, value )
        Widget wid ;
        int offset ;
        XtArgVal *value ;
#else
GetAutoWrap(
        Widget wid,
        int offset,
        XtArgVal *value )
#endif /* _NO_PROTO */
{
     DtTermWidget tw = (DtTermWidget) wid ;
    (*value) = (XtArgVal) tw->term.tpd->autoWrapRight;
}

/* ARGSUSED */
static XmImportOperator 
#ifdef _NO_PROTO
SetAutoWrap( wid, offset, value )
        Widget wid ;
        int offset ;
        XtArgVal *value ;
#else
SetAutoWrap(
        Widget wid,
        int offset,
        XtArgVal *value )
#endif /* _NO_PROTO */
{
     DtTermWidget tw = (DtTermWidget) wid ;
    tw->term.tpd->autoWrapRight =  (* (int *) value);
/*    return (XmSYNTHETIC_LOAD);*/
    return (XmSYNTHETIC_NONE);
}
 /* ARGSUSED */
static void 
#ifdef _NO_PROTO
GetReverseWrap( wid, offset, value )
        Widget wid ;
        int offset ;
        XtArgVal *value ;
#else
GetReverseWrap(
        Widget wid,
        int offset,
        XtArgVal *value )
#endif /* _NO_PROTO */
{
     DtTermWidget tw = (DtTermWidget) wid ;
    (*value) = (XtArgVal) tw->vt.td->reverseWrapMode;;
}

/* ARGSUSED */
static XmImportOperator 
#ifdef _NO_PROTO
SetReverseWrap( wid, offset, value )
        Widget wid ;
        int offset ;
        XtArgVal *value ;
#else
SetReverseWrap(
        Widget wid,
        int offset,
        XtArgVal *value )
#endif /* _NO_PROTO */
{
     DtTermWidget tw = (DtTermWidget) wid ;
    tw->vt.td->reverseWrapMode =  (* (int *) value);
/*    return (XmSYNTHETIC_LOAD);*/
    return (XmSYNTHETIC_NONE);
}

/* ARGSUSED */
static void 
#ifdef _NO_PROTO
GetWrapRightAfterInsert( wid, offset, value )
        Widget wid ;
        int offset ;
        XtArgVal *value ;
#else
GetWrapRightAfterInsert(
        Widget wid,
        int offset,
        XtArgVal *value )
#endif /* _NO_PROTO */
{
     DtTermWidget tw = (DtTermWidget) wid ;
    (*value) = (XtArgVal) tw->term.tpd->wrapRightAfterInsert;
}

/* ARGSUSED */
static XmImportOperator 
#ifdef _NO_PROTO
SetWrapRightAfterInsert( wid, offset, value )
        Widget wid ;
        int offset ;
        XtArgVal *value ;
#else
SetWrapRightAfterInsert(
        Widget wid,
        int offset,
        XtArgVal *value )
#endif /* _NO_PROTO */
{
     DtTermWidget tw = (DtTermWidget) wid ;
    tw->term.tpd->wrapRightAfterInsert =  (* (int *) value);
/*    return (XmSYNTHETIC_LOAD);*/
    return (XmSYNTHETIC_NONE);
}

 /* ARGSUSED */
static void 
#ifdef _NO_PROTO
GetAppCursorMode( wid, offset, value )
        Widget wid ;
        int offset ;
        XtArgVal *value ;
#else
GetAppCursorMode(
        Widget wid,
        int offset,
        XtArgVal *value )
#endif /* _NO_PROTO */
{
     DtTermWidget tw = (DtTermWidget) wid ;
    (*value) = (XtArgVal) tw->vt.td->applicationMode;;
}

/* ARGSUSED */
static XmImportOperator 
#ifdef _NO_PROTO
SetAppCursorMode( wid, offset, value )
        Widget wid ;
        int offset ;
        XtArgVal *value ;
#else
SetAppCursorMode(
        Widget wid,
        int offset,
        XtArgVal *value )
#endif /* _NO_PROTO */
{
     DtTermWidget tw = (DtTermWidget) wid ;
    tw->vt.td->applicationMode =  (* (int *) value);
    return (XmSYNTHETIC_NONE);
}

 /* ARGSUSED */
static void 
#ifdef _NO_PROTO
GetAppKeypadMode( wid, offset, value )
        Widget wid ;
        int offset ;
        XtArgVal *value ;
#else
GetAppKeypadMode(
        Widget wid,
        int offset,
        XtArgVal *value )
#endif /* _NO_PROTO */
{
     DtTermWidget tw = (DtTermWidget) wid ;
    (*value) = (XtArgVal) tw->vt.td->applicationKPMode;;
}

/* ARGSUSED */
static XmImportOperator 
#ifdef _NO_PROTO
SetAppKeypadMode( wid, offset, value )
        Widget wid ;
        int offset ;
        XtArgVal *value ;
#else
SetAppKeypadMode(
        Widget wid,
        int offset,
        XtArgVal *value )
#endif /* _NO_PROTO */
{
     DtTermWidget tw = (DtTermWidget) wid ;
    tw->vt.td->applicationKPMode =  (* (int *) value);
    return (XmSYNTHETIC_NONE);
}
    
/* SetValues...
 */
/*ARGSUSED*/
static Boolean
SetValues(Widget cur_w, Widget ref_w, Widget w, ArgList args,
	Cardinal *num_args)
{
    DtTermWidget cur_tw = (DtTermWidget) cur_w;
    DtTermWidget tw = (DtTermWidget) w;
    Boolean flag = False;	/* return value... */
    int i;


#ifdef	DKS
    /* DKS: this needs to be done later...
     */

    if (cur_tw->vt.verticalScrollBar != tw->vt.verticalScrollBar) {
	if (tw->vt.verticalScrollBar) {
	    /* set up the scrollbar values and callbacks... */
	    InitializeVerticalScrollBar(w, True);
	}
    }
#endif	/* DKS */

    if (tw->term.fontList != cur_tw->term.fontList) {
	/* the font has been changed... */
	XmFontList fontList;
	XFontSet fontSet;
	XFontStruct *font;

	/* replace the base font...
	 */
	tw->vt.td->renderFonts[0].termFont = tw->term.tpd->termFont;

	/* replace the linedraw font...
	 */
	/* release the old font... */
	_DtTermPrimDestroyFont(w, tw->vt.td->renderFonts[1].termFont);

	/* replace the font... */
	tw->vt.td->renderFonts[1].termFont =
		_DtTermPrimRenderLineDrawCreate(w,
		_DtTermGlyphs,
		_DtTermNumGlyphs,
		tw->term.widthInc,
		tw->term.ascent,
		tw->term.heightInc - tw->term.ascent);

	/* replace the bold font.  If the base font is the default, then
	 * use the default bold font that we generated.  Else, null it out
	 * and we will fake by overstriking...
	 */
	if (tw->vt.td->renderFonts[0].termFont ==
		tw->vt.td->tpd->defaultTermFont) {
	    tw->vt.td->renderFonts[2].termFont =
		    tw->vt.td->tpd->defaultBoldTermFont;
	} else {
	    tw->vt.td->renderFonts[2].termFont = (TermFont) 0;
	}

	flag = True;
    }

    for ( i = 0 ; i < 8 ; i++ ) {
	if (tw->vt.customColor[i] != cur_tw->vt.customColor[i]) {
	    VtColorPair cp = &tw->vt.td->colorPairs[9+i];
	    cp->fg.pixel = tw->vt.customColor[i];
	    XQueryColor(XtDisplay(w), w->core.colormap, &cp->fg);
	    cp->bg = cp->fg;
	    cp->bgCommon = True;
	    flag = True;
	}
    }

    if (tw->vt.glyphFont != cur_tw->vt.glyphFont) {
	if ( tw->vt.glyphFont )
	    tw->vt.td->G3 = RENDER_FONT_GLYPH;
	else
	    tw->vt.td->G3 = RENDER_FONT_NORMAL;
    }

    return(flag);
}

/* Destroy...
 */
static void
Destroy(Widget w)
{
    DtTermWidget tw = (DtTermWidget) w;
    DtTermData td = tw->vt.td;

    /* remove the DtTermData structure contents, followed by the structure...
     */
    if (tw->vt.td) {
        /*  free up any fonts we allocated for this widget... */
        /*  don't free the others because they're done in TermPrim Destroy()*/
	_DtTermPrimDestroyFont(w, tw->vt.td->renderFonts[1].termFont);

        /* free up any color cells that we allocated for this widget... */
        _DtTermColorDestroy(w);

	/* free up our function key related resources... */
	_DtTermFunctionKeyClear(w);

        /* 
        ** free up the block mode stuff...
        _vtBlockModeDestroy(td->blockModeInfo);
        td->blockModeInfo = (BlockModeInfo) 0;
        */

        /* free up the DtTermData structure... */
        XtFree((char *) tw->vt.td);
        tw->vt.td = (DtTermData) 0;
    }
}

#ifdef	DKS
static void
StatusChangeNotify(Widget w, unsigned long mask)
{
    DtTermWidget tw = (DtTermWidget) w;
    struct termData *tpd = tw->term.tpd;
    struct vtData *td = tw->vt.td;
    TermVtCallbackStruct cb;

    if (!tw->vt.statusChangeCallback) {
	/* no callback, nothing to do... */
	return;
    }

    /*****************************************************************
     *  Caps Lock
     */
    if (mask & TermSTATUS_CHANGE_NOTIFY_CAPS_LOCK) {
	memset(&cb, '\0', sizeof(cb));
	cb.reason = DtTermCR_CAPS_LOCK_CHANGE;
	cb.capsLock = tpd->capsLock;
	XtCallCallbackList(w,
		tw->vt.statusChangeCallback, (XtPointer) &cb);
    }

    /*****************************************************************
     *  Insert Char Mode
     */
    if (mask & TermSTATUS_CHANGE_NOTIFY_INSERT_MODE) {
	memset(&cb, '\0', sizeof(cb));
	cb.reason = DtTermCR_INSERT_MODE_CHANGE;
	cb.insertCharMode = tpd->insertCharMode;
	XtCallCallbackList(w,
		tw->vt.statusChangeCallback, (XtPointer) &cb);
    }

    /*****************************************************************
     *  Stop
     */
    if (mask & TermSTATUS_CHANGE_NOTIFY_STOP) {
	memset(&cb, '\0', sizeof(cb));
	cb.reason = DtTermCR_STOP_CHANGE;
	cb.stop = tpd->stop;
	XtCallCallbackList(w,
		tw->vt.statusChangeCallback, (XtPointer) &cb);
    }
}

static void PositionUpdate(Widget w, short row, short column)
{
    DtTermWidget tw = (DtTermWidget) w;
    struct termData *tpd = tw->term.tpd;
    struct vtData *td = tw->vt.td;
    TermVtCallbackStruct cb;

    if (!tw->vt.statusChangeCallback) {
	/* no callback, nothing to do... */
	return;
    }

    cb.reason = DtTermCR_CURSOR_MOTION;
    cb.cursorColumn = tpd->cursorColumn;
    cb.cursorRow = tpd->cursorRow = tpd->topRow;
    XtCallCallbackList(w,
	    tw->vt.statusChangeCallback, (XtPointer) &cb);
}
#endif	/* DKS */

Widget
DtCreateTerm(Widget parent, char *name, ArgList arglist, Cardinal argcount)
{
    Widget w;

    Debug('T', timeStamp("DtCreateTerm() starting"));
    w = XtCreateWidget(name, dtTermWidgetClass, parent, arglist, argcount);
    Debug('T', timeStamp("DtCreateTerm() finished"));
    return(w);
}

#ifdef	DKS
static void VerticalScrollBarCallback(Widget wid, XtPointer client_data,
	XtPointer call_data)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) client_data;
    struct termData *tpd = tw->term.tpd;
    XmScrollBarCallbackStruct *cb = (XmScrollBarCallbackStruct *) call_data;

    _DtTermPrimScrollTextTo((Widget) tw, cb->value);
    /* we need to complete the scroll or it won't happen... */
    _DtTermPrimScrollComplete((Widget) tw, True);
    if (cb->reason != XmCR_DRAG) {
	_DtTermPrimCursorOn((Widget) tw);
    }
}
#endif	/* DKS */

static int
TextInsertProc
(
    Widget		  w,
    unsigned char	 *buffer,
    int			  length
)
{
    DtTermWidget	  tw = (DtTermWidget) w;
    DtTermPrimData	  tpd = tw->term.tpd;
    DtTermData		  td = tw->vt.td;
    int			  partialLen = 0;
    int			  holdFont;

    if (td->singleShiftPending) {
	unsigned char localbuf[16];
	int n;

	/* save away the active GL font... */
	holdFont = *td->GL;
	/* make it the single shift font... */
	*td->GL = td->singleShiftFont;
	_DtTermPrimBufferSetEnhancement(tpd->termBuffer,
		tpd->topRow + tpd->cursorRow, tpd->cursorColumn,
		enhFont, *td->GL);

	localbuf[0] = buffer[0];
	n = 1;

	if ( *td->GL == FONT_GLYPH ) {
		/* add padding, if necessary */
		GlyphFont *gf = *(GlyphFont **)
			(td->renderFonts[FONT_GLYPH].termFont->fontInfo);
		if ( gf && gf->maxWidth > tpd->cellWidth) {
			int pad = (gf->maxWidth / tpd->cellWidth);
			for ( ; pad ; pad--, n++ )
				localbuf[n] = ' ';
		}
	}


	/* insert the first character and possible pad... */
	partialLen = _DtTermPrimInsertText(w, localbuf, n);

	/* restore the active GL font... */
	*td->GL = holdFont;
	_DtTermPrimBufferSetEnhancement(tpd->termBuffer,
		tpd->topRow + tpd->cursorRow, tpd->cursorColumn,
		enhFont, *td->GL);

	if (partialLen == n) {
	    partialLen = 1;
	    /* successful insert...
	     */
	    /* clear the pending flag... */
	    td->singleShiftPending = False;
	    /* skip over the character... */
	    buffer++;
	    length--;
	} else {
	    return(partialLen);
	}
    } else if ( *td->GL == FONT_GLYPH ) {
	    /* must insert singly, with padding */
	    int i;
	    partialLen = 0;
	    td->singleShiftFont = FONT_GLYPH;
	    for ( i = 0; i < length; i++ ) {
		    td->singleShiftPending = True; /* reset by TextInserProc */
		    partialLen += TextInsertProc(w, buffer + i, 1);
	    }
	    td->singleShiftPending = False;
	    return(partialLen);
    }

    if (length > 0) {
	partialLen += _DtTermPrimInsertText(w, buffer, length);
    }

    return(partialLen);
}

void
_DtTermSetGlyph(Widget w, int glyph, int hilight)
{
    DtTermWidget tw = (DtTermWidget) w;
    DtTermData td = tw->vt.td;
    DtTermPrimData tpd = td->tpd;
    TermBuffer tb = tpd->termBuffer;
    int lineno = tpd->cursorRow;
    DtLine line = DT_LINE_OF_TBUF(tb, lineno);
    DtEnh  enh;
    unsigned char buf[2];
    int i;

    buf[0] = (unsigned char)glyph;

    if ( lineno > 1024 || lineno < 0 )
	    abort();

    _DtTermPrimBufferSetLineGlyph(tb, lineno, glyph);

#if 0	    
    if ( glyph != 0 ) {
	/* insert glyph (still displays in the gutter!) */
	_DtTermPrimBufferSetEnhancement(tb,
		   tpd->topRow + tpd->cursorRow, tpd->cursorColumn,
		   enhFont, td->G3);

	_DtTermPrimInsertText(w, buf, 1);

	_DtTermPrimBufferSetEnhancement(tb,
		    tpd->topRow + tpd->cursorRow, tpd->cursorColumn,
		    enhFont, *td->GL);
    }
#endif

    /* set background color for entire line */
    enh  = DT_ENH(line);
    if ( !enh ) {
	    if ( INSERT_ENH(tb) ) {
		    DT_ENH_DIRTY(tb) = 1;
		    (*INSERT_ENH(tb))(tb, lineno, 0, WIDTH(line), False);
		    enh  = DT_ENH(line);
	    }
    }
    switch ( hilight ) {
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
	    hilight -= 40;
	    break;
    case 58:
    case 59:
    case 60:
    case 61:
    case 62:
    case 63:
    case 64:
    case 65:
	   hilight -= 50;
	   break;
    default:
	    hilight = 0;
	    break;
    }
    for ( i = 0; enh && i < WIDTH(line); i++ ) {
	    enh[i].bgColor = hilight;
    }

    /* and now refresh it to make background color appear */
    _DtTermPrimRefreshText(w, 0, lineno,
		tw->term.columns, lineno, False);
    
}

