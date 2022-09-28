/* $XConsortium: Main.h /main/cde1_maint/2 1995/10/23 11:19:55 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Main.h
 **
 **   Project:     SUI
 **
 **   Description: defines and typedefs for DtStyle 
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990.  
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _main_h
#define _main_h
 
/* 
 *  #include statements 
 */

#include <Xm/Xm.h>
#include "Font.h"
#include <Dt/DtP.h>
#include <Dt/SessionM.h>
#include <Dt/SessionP.h>

/* 
 *  #define statements 
 */

#define MAX_ARGS         20
#define CMPSTR(str)      XmStringCreateLtoR (str, XmFONTLIST_DEFAULT_TAG)
#define XMCLASS          "Dtstyle"
#define DIALOG_MWM_FUNC  MWM_FUNC_MOVE | MWM_FUNC_CLOSE 

#define COLOR   "Color"
#define FONT    "Font"
#define BACKDROP "Backdrop"
#define KEYBOARD "Keyboard"
#define MOUSE   "Mouse"
#define BEEP   "Beep"
#define SCREEN  "Screen"
#define DTWM   "Dtwm"
#define STARTUP "Startup"

#define COLOR_ICON   "DtColor"
#define FONT_ICON    "DtFont"
#define BACKDROP_ICON "DtBkdrp"
#define KEYBOARD_ICON "DtKeybd"
#define MOUSE_ICON   "DtMouse"
#define BEEP_ICON   "DtBeep"
#define SCREEN_ICON  "DtScrn"
#define DTWM_ICON   "DtDtwm"
#define STARTUP_ICON "DtStart"


/* help files */
#define KB_TRAV_HELP           "vs_keys"
#define BACKDROP_HELP          "vs_back"
#define AUDIO_HELP             "vs_beep"
#define PALETTE_HELP           "vs_colo"
#define COLOR_EDIT_HELP        "vs_dtd"
#define ADD_PALETTE_HELP       "vs_coad"
#define DELETE_PALETTE_HELP    "vs_code"
#define FONT_HELP              "vs_font"
#define HOST_HELP              "vs_host"
#define KEYBOARD_HELP          "vs_keyb"
#define MOUSE_HELP             "vs_mous"
#define PASSWORD_HELP          "vs_pass"
#define SCREEN_HELP            "vs_scre"
#define STARTUP_HELP           "vs_star"
#define STYLE_HELP             "vs_wind"

/* DialogBoxDialog label #defines */
#define OK_BUTTON             1
#define CANCEL_BUTTON         2
#define HELP_BUTTON           3

#define NUM_LABELS            3

/* geometry */
#define LB_MARGIN_HEIGHT      2
#define LB_MARGIN_WIDTH       12
#define SCALE_HIGHLIGHT_THICKNESS  (Dimension) 2

/* 
 * typedef statements 
 */

typedef struct {
    int        numFonts;
    XmFontList userFont,
               systemFont;
    String     userFontStr,
               systemFontStr;
    Fontset    fontChoice[10];
    String     session,
               backdropDir,
               paletteDir,
               timeoutScale, lockoutScale;
    Boolean    writeXrdbImmediate;
    Boolean    writeXrdbColors;
    String     fontGroup;
    String     componentList;
    
} ApplicationData, *ApplicationDataPtr;

typedef struct {
    Boolean     restoreFlag;
    int         poscnt;
    Arg         posArgs[MAX_ARGS];
} saveRestore;

typedef struct {
    Display         *display;
    Screen          *screen;
    int              screenNum;
    Window           root;
    Colormap         colormap;
    Boolean          useMultiColorIcons;
    Pixel            tgSelectColor,
                     secSelectColor,              /** for editable text bg **/
		     primTSCol, primBSCol,        /** TS and BS colors are **/
                     secTSCol, secBSCol,          /** for icon fg and bg   **/
                     secBgCol;
    ApplicationData  xrdb;
    char            *home;
    char            *lang;
    char            *execName;
    SmStateInfo      smState;
    SmSaverInfo      smSaver; 
    Boolean          longfilename;
    Widget           shell,
                     mainWindow,
                     colorDialog,
                     backdropDialog,
                     fontDialog,
                     kbdDialog,
                     mouseDialog,
                     audioDialog,
                     screenDialog,
                     startupDialog,
                     dtwmDialog,
                     buttonsForm;
    XmString         tmpXmStr;
    Widget           errDialog, errParent;
    char             tmpBigStr[1024];
    int              fontSize, timeoutscale;
    short            count;
    Boolean          colorSrv;
    Boolean          dynamicColor;
    Boolean          workProcs;
    int              horizontalSpacing, verticalSpacing;
} Style;

/*
 *  External variables  
 */

extern Style style;
extern char  *progName;
extern XrmDatabase sys_pl_DB;
extern XrmDatabase adm_pl_DB;
extern XrmDatabase hm_pl_DB;
extern XrmDatabase pl_DB;

/*  
 *  External Interface  
 */

#ifdef _NO_PROTO
#else
extern int local_xerror( Display *dpy, XErrorEvent *rep) ; /* defined in hostBB.c for 
								historical reasons*/
#endif  /*  _NO_PROTO */

#ifdef _NO_PROTO

extern char *_DtGetMessage();
extern void raiseWindow() ;
extern void CenterMsgCB() ;
extern void ErrDialog() ;
extern void InfoDialog() ;
extern void putDialog() ;
extern void smStateCB() ;
extern void HandleStopMsg() ;
extern void LoadDatabase();

#else

extern char *_DtGetMessage( 
		        char *filename,
                        int set, 
                        int n, 
                        char *s );
extern void raiseWindow(
                        Window dialogWin) ;
extern void CenterMsgCB( 
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern void ErrDialog( 
                        char *errString,
                        Widget visualParent) ;
extern void InfoDialog( 
                        char *infoString,
                        Widget parent,
                        Boolean unmapParent) ;
extern void putDialog( 
                        Widget parent,
                        Widget dialog) ;
extern void smStateCB( 
                        DtString fields[],
                        XtPointer client_data,
                        int num_fields) ;
extern void HandleStopMsg( 
                        DtString fields[],
                        XtPointer client_data,
                        int num_words) ;
extern void LoadDatabase(void);
#endif /* _NO_PROTO */

/*
 * macro to get message catalog strings
 */
#ifdef MESSAGE_CAT
#ifdef DT_LITE
#define GETMESSAGE(set, number, string)\
    _DtGetMessage(set, number, "")
#else /*  DT_LITE*/
#define CLIENT_MESSAGE_CAT_NAME       "dtstyle"
#define GETMESSAGE(set, number, string)\
    _DtGetMessage(CLIENT_MESSAGE_CAT_NAME, set, number, string)
#endif /* DT_LITE */
#else /* MESSAGE_CAT */
#define GETMESSAGE(set, number, string)\
    string
#endif /* MESSAGE_CAT */

#endif /* _main_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
  
