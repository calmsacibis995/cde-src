/*
 * $XConsortium: TermViewP.h /main/cde1_maint/1 1995/07/15 01:40:07 drk $";
 */

/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef	_Dt_TermViewP_h
#define	_Dt_TermViewP_h

#include <Xm/BulletinBP.h>
#include "TermView.h"

#ifdef	__cplusplus
extern "C" {
#endif	/* __cplusplus */

/* DtTermView class structure... */

typedef struct _DtTermViewClassPart
{
    int foo;
} DtTermViewClassPart;

/* full clas record declaration for DtTermView class... */
typedef struct _DtTermViewClassRec {
    CoreClassPart		core_class;
    CompositeClassPart		composite_class;
    ConstraintClassPart		constraint_class;
    XmManagerClassPart		manager_class;
    XmBulletinBoardClassPart	bulletin_board_class;
    DtTermViewClassPart		termview_class;
} DtTermViewClassRec;

externalref DtTermViewClassRec dtTermViewClassRec;

typedef struct {
    Widget frame;
    Widget widget;
    Boolean managed;
} widgetInfo;

/* dttermview instance record... */
typedef struct _DtTermViewPart
{
    Widget term;
    Widget scrolledWindowFrame;
    Widget scrolledWindow;
    widgetInfo scrollBar;
    widgetInfo menuBar;
    Widget helpWidget;
    Widget globalOptionsDialog;
    Widget terminalOptionsDialog;
    char *emulationId;			/* $TERMINAL_EMULATOR variable	*/
    unsigned char charCursorStyle;
    int blinkRate;
    Boolean visualBell;
    Boolean marginBell;
    int nMarginBell;
    Boolean jumpScroll;
    unsigned char shadowType;
    XmFontList fontList;
    XmFontList defaultFontList;
    char *userFontList;
    short rows;
    short columns;
    Dimension marginHeight;
    Dimension marginWidth;
    Dimension spacing;
    int baseWidth;
    int baseHeight;
    int widthInc;
    int heightInc;
    int pty;
    Boolean ptyAllocate;
    char *ptySlaveName;
    int subprocessPid;
    char *subprocessCmd;
    char **subprocessArgv;
    Boolean subprocessLoginShell;
    Boolean subprocessTerminationCatch;
    Boolean subprocessExec;
    Boolean scrollBarVisible;
    Boolean menuBarVisible;
    Boolean menuPopupVisible;
    Boolean capsLock;
    Boolean stop;
    DtTermInsertCharMode insertCharMode;
    XtCallbackList subprocessTerminationCallback;
    XtCallbackList newCallback;
    DtTermTerminalSize sizeDefault;
    DtTermTerminalSizeList sizeList;
    char    *userFontName;
    int currentFontToggleButtonIndex;
    Boolean sized;
    Boolean fontIndexSet;
    int     userFontIndex;
    int glyphGutter;	
    void *glyphFont;
} DtTermViewPart;

/* full instance record declaration... */

typedef struct _DtTermViewRec {
    CorePart		core;
    CompositePart	composite;
    ConstraintPart	constraint;
    XmManagerPart	manager;
    XmBulletinBoardPart bulletinBoard;
    DtTermViewPart	termview;
} DtTermViewRec;




/* end private function declarations... */

#ifdef	__cplusplus
} /* close scope of 'extern "C"'... */
#endif	/* __cplusplus */

#endif	/* _Dt_TermViewP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif... */
