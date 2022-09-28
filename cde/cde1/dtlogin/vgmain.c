/* $XConsortium: vgmain.c /main/cde1_maint/11 1995/11/15 11:27:14 lehors $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

 /****************************************************************************
 **
 **   File:        vgmain.c
 **
 **   Project:     HP Visual User Environment (DT)
 **
 **   Description: Main line code for Dtgreet application
 **
 **                These routines initialize the toolkit, create the widgets,
 **		   set up callbacks, and wait for events.
 **
 **
 **   (c) Copyright 1987, 1988, 1989 by Hewlett-Packard Company
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/



/***************************************************************************
 *
 *  Includes
 *
 ***************************************************************************/

#include	<stdio.h>
#include        <setjmp.h>
#include	<stdlib.h>
#include	<unistd.h>
#include        <sys/signal.h>
#include        <sys/stat.h>
#include        <sys/param.h>
#include	<locale.h>

#include	<Xm/Xm.h>
#include	<X11/Xfuncs.h>
#include	<X11/Shell.h>
#include        <X11/cursorfont.h>
#include	<Xm/DragC.h>
#include	<Xm/DrawingA.h>
#include	<Xm/Frame.h>
#include	<Xm/Form.h>
#include	<Xm/Label.h>
#include	<Xm/LabelG.h>
#include	<Xm/Text.h>
#include	<Xm/TextF.h>
#include	<Xm/PushB.h>
#include	<Xm/PushBG.h>
#include	<Xm/MessageB.h>
#include	<Xm/RowColumn.h>
#include	<Xm/SeparatoG.h>
#include	<Xm/ToggleBG.h>
#include	<Xm/CascadeBG.h>
#include	<Dt/EnvControl.h>
#include	"vg.h"
#include	"vgmsg.h"
#include 	<Dt/MenuButton.h>


#if !defined(NL_CAT_LOCALE)
#define NL_CAT_LOCALE 0
#endif

/***************************************************************************
 *
 *  External declarations
 *
 ***************************************************************************/
extern char    password[];  /* pswd string value */
extern int     password_length;  /* pswd string length */


/***************************************************************************
 *
 *  Procedure declarations
 *
 ***************************************************************************/
#ifdef _NO_PROTO

static SIGVAL syncTimeout() ;	/* timer handler for initialize alarm	   */
static int  ErrorHandler();	/* X protocol error handler		   */
static void xtErrorHandler();	/* Xt protocol error handler		   */
static void xtWarningHandler();	/* Xt protocol warning handler		   */
static Widget InitToolKit();	/* initialize the toolkit		   */
static void MakeRootCursor();	/* make workspace cursor, if requested     */
static void MakeBackground();	/* make shell and table widgets		   */
static void MakeButtons();	/* make ok, clear, options, help buttons   */
static void MakeDtlabel();	/* make Desktop label 	                   */
static void MakeGreeting();	/* make welcome message			   */
static void MakeLogin();	/* make login/password areas		   */
static void MakeOptionsProc();	/* timeout proc to build options menu	   */
static void MyInsert();		/* custom Text SelfInsert function	   */
static void MyBackspace();	/* custom Text Backspace function	   */
static SIGVAL Terminate();	/* signal -TERM handler			   */
static char * GetLangName();    /* locale name conversion                  */
static void MakeAltDtButtons();	/* make alt desktop buttons in options menu */
static void DebugWidgetResources();
#else

static SIGVAL syncTimeout( int arg ) ;
static Widget InitToolKit( int argc, char **argv) ;
static void MakeRootCursor( void ) ;
static void MakeBackground( void ) ;
static void MakeButtons( void ) ;
static void MakeDtlabel( void );	
static void MakeGreeting( void ) ;
static void MakeLogin( void ) ;
static void MyInsert( Widget w, XEvent *event, char **params,
                        Cardinal *num_params) ;
static void MyBackspace( Widget w, XEvent *event, char **params,
                        Cardinal *num_params) ;
static int  ErrorHandler( Display *dpy, XErrorEvent *event) ;
static void xtErrorHandler( String msg ) ;
static void xtWarningHandler( String msg ) ;
static void MakeOptionsProc( XtPointer data, XtIntervalId *id) ;
static SIGVAL Terminate( int arg ) ;
static char * GetLangName( char * label );
static void MakeAltDtButtons( void );	
static void DebugWidgetResources(Widget w);

#endif /* _NO_PROTO */




/***************************************************************************
 *
 *  Global variables
 *
 ***************************************************************************/


AppInfo		appInfo;	/* application resources		   */
Arg		argt[100];	/* used for resources			   */
DisplayInfo	dpyinfo;	/* information about the display	   */
nl_catd		nl_fd = (nl_catd)-1;	/* message catalog file descriptor */
XmString	xmstr;		/* used for compound strings		   */
char		*langenv;	/* current value of LANG environment var.  */
char		*errorLogFile;	/* current value of environment var.	   */
int		showVerifyError;/* display a Verify() error dialog	   */

char 		altdtname[MAXPATHLEN];
char 		altdtclass[MAXPATHLEN];
char 		altdtkey[MAXPATHLEN];
char 		altdtkeyclass[MAXPATHLEN];
char 		altdtstart[MAXPATHLEN];
char 		altdtstartclass[MAXPATHLEN];
char 		altdtlogo[MAXPATHLEN];
char 		altlogoclass[MAXPATHLEN];
char            *logotype;        /* for XrmGetResource()                  */
XrmValue        logovalue;        /* for XrmGetResource()                    */
char            *rmtype;  
XrmValue        rmvalue;     
char            *keyrmtype;   
XrmValue        keyrmvalue;   

/******************************************************************************
**
**      WIDGET LAYOUT
**
** toplevel                 "main"                      (toplevel)
**  login_shell              "login_shell"              (overrideShell)
**   table                    "table"                   (DrawingAreaWidget)
**     copyright_msg            "copyright_msg"         (MessageBox)
**     error_message            "error_message"         (MessageBox)
**     help_message             "help_message"          (MessageBox)
**     passwd_message           "passwd_message"        (MessageBox)
**     hostname_message         "hostname_msg"          (MessageBox)
**     matte                   "matte"                  (FormWidget)
**      logo                   "logo"                   (FrameWidget)
**       logo_pixmap            "logo_pixmap"           (LabelGadget)
**      matteFrame         "matteFrame"         (FrameWidget)
**       matte1                "matte1"                 (FormWidget)
**        help_button             "help_button"         (PushButtonGadget)
**        greeting               "greeting"             (LabelGadget)
**        dt_label	         "dt_label"		(LabelGadget)	
**        login_form               "login_form"         (FormWidget)
**          login_label              "login_label"      (LabelGadget)
**          login_text               "login_text"       (TextField)
**          passwd_text              "passwd_text"      (TextField)
**        ok_button               "ok_button"           (PushButtonGadget)
**        clear_button            "clear_button"        (PushButtonGadget)
**        options_button                  "options_button"      (DtMenuButtonWidget)
**          options_menu                    "options_menu"      (PopupMenu)
**            options_item[0]         "options_languages"    (CascadeButtonGadget)
**            options_item[1]         "options_sep2"         (SeparatorGadget)
**            options_item[2]         "session_menus"        (CascadeButtonGadget)
**            options_item[3]         "options_sep1"         (SeparatorGadget)
**            options_item[4]         "options_noWindows"    (PushButtonGadget)
**            options_item[5]         "options_restartServer"(PushButtonGadget)
**            options_item[6]         "options_sep1"         (SeparatorGadget)
**            options_item[7]         "options_Copyright"    (PushButtonGadget)
**            session_menu            "session_menu"         (PulldownMenu)
**              options_dt            "options_dt"             (ToggleButtonGadget)
**              options_failsafe      "options_failsafe"       (ToggleButtonGadget)
**            lang_menu               "lang_menu"       (PulldownMenu)
**           (lang items)               (lang items)    (ToggleButtonGadget)
**      ...
**
*/


Widget toplevel;		/* top level shell widget		   */
Widget login_shell;		/* shell for the main login widgets.	   */
Widget table;			/* black background for everything	   */
Widget matte;			/* main level form widget		   */
Widget matteFrame;		/* main level form widget		   */
Widget matte1;		    /* second level form widget		   */

Widget greeting;		/* Welcome message			   */
Widget dt_label;		/* Desktop i.e. set in options menu        */

Widget logo1;			/* frame around the Corporate logo	   */
Widget logo_pixmap;		/* Corporate logo			   */
Widget logo_shadow;		/* drop shadow under the Corporate logo	   */

Widget login_matte;		/* bulletin board for login/password	   */
Widget login_form;		/* form containing the login widgets	   */
Widget login_label;		/* label to left of login text widget	   */
Widget login_text;		/* login text widget			   */

Widget ok_button;		/* accept name/password text button	   */
Widget clear_button;		/* clear name/password text button	   */
Widget options_button;		/* login options button			   */
Widget help_button;		/* help button				   */

Widget copyright_msg    = NULL;	/* copyright notice widget		   */
Widget help_message     = NULL;	/* the help message box			   */
Widget error_message    = NULL;	/* the error message box		   */
Widget hostname_message = NULL;	/* the invalid hostname message box	   */
Widget passwd_message   = NULL;	/* the expired password message box	   */
Widget bad_host         = NULL; /* bad remote hostname                     */
Widget remote_host_dialog = NULL;/* the expired password message box       */
Widget host_not_run_login = NULL;/* remote host not running message box    */
Widget connect_to_host  = NULL; /* connecting to remote host message box   */

Widget options_menu = NULL;	/* pop-up menu on options button	   */
Widget options_item[10];	/* items on options pop_up menu	  	   */
Widget options_nowindows;	/* nowindows pane on options pop_up menu   */
Widget options_failsafe;	/* failsafe pane on options pop_up menu	   */
Widget options_dtlite;		/* dtlite  pane on options pop_up menu	   */
Widget *alt_dts;		/* alt_dts  widgets on options pop_up menu */
Widget options_dt;		/* dt regular pane on options pop_up menu */
Widget options_last_dt;		/* user's last dt 			  */

Widget lang_menu = NULL;	/* cascading menu on "Language" option	   */
Widget session_menu = NULL;     /* cascading menu on "Session" option     */
Widget remote_host_menu = NULL;/* cascading menu on "Remote Login"option   */


/***************************************************************************
 *
 *  Text widget actions and translations
 *
 ***************************************************************************/

static XtActionsRec textActions[] = {
        {"my-insert", (XtActionProc)MyInsert},
        {"my-bksp", (XtActionProc)MyBackspace},
    };

static char textEventBindings[] = {
"Shift <Key>Tab:			prev-tab-group() \n\
  Ctrl <Key>Tab:			next-tab-group() \n\
 <Key>osfEndLine:       		end-of-line() \n\
 <Key>osfBeginLine:     		beginning-of-line() \n\
 ~Shift <Key>osfRight:         		forward-character()\n\
 ~Shift <Key>osfLeft:          		backward-character()\n\
  Ctrl <Key>osfDelete:          	delete-to-end-of-line()\n\
 <Key>osfDelete:                	delete-next-character()\n\
  <Key>osfBackSpace:     		my-bksp()\n\
 <Key>osfActivate:      		activate()\n\
  Ctrl <Key>Return:             	activate()\n\
 <Key>Return:           		activate()\n\
 <Key>:                                 my-insert()\n\
 ~Ctrl ~Shift ~Meta ~Alt<Btn1Down>:     grab-focus() \n\
 <EnterWindow>:                         enter()\n\
 <LeaveWindow>:                         leave()\n\
 <FocusIn>:                             focusIn()\n\
 <FocusOut>:                            focusOut()\n\
 <Unmap>:                               unmap()"
};


static
    XtResource AppResources[] = {
    { "workspaceCursor", "WorkspaceCursor", 
	XtRBoolean, sizeof(Boolean), XtOffset(AppInfoPtr, workspaceCursor),
	XtRImmediate, (caddr_t)False					},

    { "labelFont", "LabelFont", 
	XmRFontList, sizeof(XmFontList), XtOffset(AppInfoPtr, labelFont),
	XmRString, "Fixed"						},

    { "textFont", "TextFont", 
	XmRFontList, sizeof(XmFontList), XtOffset(AppInfoPtr, textFont),
	XmRString, "Fixed"						},

    { "optionsDelay", "OptionsDelay", 
	XtRInt, sizeof(int), XtOffset(AppInfoPtr, optionsDelay),
	XtRImmediate, (XtPointer) 0					},

    { "altDts",        "AltDts",
        XtRInt, sizeof(int), XtOffset(AppInfoPtr, altDts),
        XtRImmediate, (XtPointer) 0
        },

    {"languageList", "LanguageList",
	XtRString, sizeof(char *), XtOffset(AppInfoPtr, languageList),
	XtRString, NULL							},

#if defined (ENABLE_DYNAMIC_LANGLIST)
    {"languageListCmd", "LanguageListCmd",
        XtRString, sizeof(char *), XtOffset(AppInfoPtr, languageListCmd),
        XtRString, NULL                                                 },
#endif /* ENABLE_DYNAMIC_LANGLIST */

    };

static SIGVAL
#ifdef _NO_PROTO
Refresh()
#else
Refresh( int arg )
#endif /* _NO_PROTO */

{
	RefreshEH(NULL, NULL, NULL, NULL);
#if defined(SYSV) || defined(SVR4)
        signal (SIGHUP, Refresh);
#endif
}


/***************************************************************************
 *
 *  Main
 *
 ***************************************************************************/


int 
#ifdef _NO_PROTO
main( argc, argv )
        int argc ;
        char **argv ;
#else
main( int argc, char **argv )
#endif /* _NO_PROTO */
{

    char 	*session;
    int		i;		/* index for argt			   */
    char	**p;		/* temp pointer to traverse argv	   */
    Boolean	nograb=FALSE;	/* debugging option to not grab server/key */
    int		debug=0;	/* print debugging output */

#ifdef VG_TRACE
    vg_TRACE_EXECUTION("--------------------- main ------------------------");
#endif /* VG_TRACE */

    setlocale(LC_ALL, "");
    XtSetLanguageProc( NULL, NULL, NULL );
    
    /*
     *  set signal handlers ...
     */
     
     (void) signal(SIGTERM, Terminate);
     (void) signal(SIGHUP, Refresh);

    
#ifdef BLS
    /*
     *  set up authorization parameters, see the identity(3) man page...
     */

    if (ISSECURE) {
	set_auth_parameters(1, argv);
	init_security();
    }
#endif /* BLS */

         
    /*
     *  check some environment variables...
     */

    errorLogFile = getenv(ERRORLOG);
	
    if ( getenv("NLSPATH") == NULL )
	putenv(NLSPATH_ENV);


#ifdef sun
    if ( getenv("OPENWINHOME") == NULL )
	putenv(OWPATH_ENV);
#endif

    _DtEnvControl( DT_ENV_SET );

    /*
     * open the message catalog. If the language_specific version cannot
     * be opened, try the default...
     */
		
    langenv = getenv("LANG");

    if ( (int)(nl_fd = catopen(NLS_CATALOG,NL_CAT_LOCALE)) < 0 ) {
	    LogError(
		ReadCatalog(MC_LOG_SET,MC_LOG_NO_MSGCAT,MC_DEF_LOG_NO_MSGCAT),
		langenv);
    }

    /*
     * set custom error handler for X protocol errors...
     */

    XSetErrorHandler(ErrorHandler);


    /*
     * scan argv looking for display name...
     */
     
    showVerifyError = 0;
    
    for ( i = argc, p = argv; i > 0; i--, p++ ) {
	if ( strcmp(*p, "-display") == 0) {
	    p++;
            i--;
	    dpyinfo.name = malloc(strlen(*p) + 1);
	    strcpy(dpyinfo.name, *p);
	    continue;
	}

	if ( strcmp(*p, "-debug") == 0) {
	    p++;
            i--;
	    debug = atoi(*p);
	    continue;
	}

	if ( strcmp(*p, "-nograb") == 0) {
	    nograb = TRUE;
	    continue;
	}

	if ( strcmp(*p, "-showerror") == 0) {
	    p++;
            i--;
	    showVerifyError = atoi(*p);
	    continue;
	}
    }
 
#ifdef VG_TRACE
    vg_TRACE_EXECUTION("main: after options.");
#endif /* VG_TRACE */
#ifdef VG_DEBUG
  #ifdef __hpux
    while (1) {}
  #else
    LogError((unsigned char *) "main:  sleeping %d seconds.\n", debug);
    if (debug) {
	sleep(debug);
    }
  #endif /* __hpux */
#endif /* VG_DEBUG */

    /*
     * initialize the Intrinsics...
     */
     
    toplevel = InitToolKit(argc, argv);
#ifdef VG_TRACE
    vg_TRACE_EXECUTION("main:  exited InitToolKit ...");
#endif /* VG_TRACE */

    if (debug) {
        XtSetErrorHandler(xtErrorHandler);
        XtSetWarningHandler(xtWarningHandler);
    }

#ifdef __hpux
    /*
     * prevent the toolkit from starting the NLIO server...
     */
    _XHPNlioctl(0,0,-1,0);
#ifdef VG_TRACE
    vg_TRACE_EXECUTION("main:  exited _XHPNlioctl ...");
#endif /* VG_TRACE */
#endif

    /*
     * get information about the display...
     */

    dpyinfo.dpy		= XtDisplay(toplevel);
/*    dpyinfo.name	= "";*/
    dpyinfo.screen	= DefaultScreen(dpyinfo.dpy);
    dpyinfo.root	= RootWindow   (dpyinfo.dpy, dpyinfo.screen);
    dpyinfo.depth	= DefaultDepth (dpyinfo.dpy, dpyinfo.screen);
    dpyinfo.width	= DisplayWidth (dpyinfo.dpy, dpyinfo.screen);
    dpyinfo.height	= DisplayHeight(dpyinfo.dpy, dpyinfo.screen);
    dpyinfo.black_pixel	= BlackPixel   (dpyinfo.dpy, dpyinfo.screen);
    dpyinfo.visual	= DefaultVisual(dpyinfo.dpy, dpyinfo.screen);

    /*
     *  check if any overrides were passed in the argv string...
     */

    for ( i = 1; i < argc; i++) {
	switch(i) {

	default:
	    break;
	}
    }

    /*
     *  add the unit convertor for resources...
     */
     
    XtAddConverter(XmRString, XmRUnitType, XmCvtStringToUnitType, NULL, 0);

    /*
     *  get user-specified resources...
     */

    SetResourceDatabase();
 
    XtGetApplicationResources(toplevel, &appInfo, AppResources,
    				XtNumber(AppResources), NULL, 0);

#ifdef VG_TRACE
    vg_TRACE_EXECUTION("main:  got application resources ...");
#endif /* VG_TRACE */
    
    /*
     *  build widgets...
     */

#ifdef VG_TRACE
    vg_TRACE_EXECUTION("main:  making UI ...");
#endif /* VG_TRACE */
    MakeBackground();	/* login_shell, table, matte		  */
    MakeLogo();			/* logo, logo_pixmap, logo_shadow	  */
    MakeGreeting();		/* greeting				  */
    MakeLogin();		/* login_matte ...            		  */
    MakeDtlabel();		/* Show Desktop selection in options  menu*/

    /*
     *  grab the display and keyboard...
     *	moved it from before to after creating text widgets in MakeLogin
     *	RK 01.11.94
     */
    if ( ! nograb )
	SecureDisplay();

    MakeButtons();		/* ok, clear, options, help buttons	  */
    MakeDialog(copyright);	/* copyright dialog	  		  */

    if (appInfo.optionsDelay == 0 )
	MakeOptionsMenu();	/* make option_button pop-up menu	  */
    else
	XtAddTimeOut((unsigned long) appInfo.optionsDelay * 1000, 
		      MakeOptionsProc, NULL);

    MakeAltDtButtons();        /* make alt desktop buttons, if any  	 */
#ifdef VG_TRACE
    vg_TRACE_EXECUTION("main:  made UI ...");
#endif /* VG_TRACE */

    /*
     * Add request callback.
     XtAddInput(0, (XtPointer)XtInputReadMask, RequestCB, NULL);
     */
   
    /*
     *  force the focus to the login_text widget...
     */
/*
     XtAddEventHandler(login_text, ExposureMask, False, 
     			FakeFocusIn, NULL);
*/

    /*
     *  create windows for the widgets...
     */
     
#ifdef VG_TRACE
    vg_TRACE_EXECUTION("main:  going to realize login_shell ...");
#endif /* VG_TRACE */
    XtRealizeWidget(login_shell);
#ifdef VG_TRACE
    vg_TRACE_EXECUTION("main:  realized login_shell ...");
#endif /* VG_TRACE */
    
    
    /*
     *  miscellaneous stuff...
     *
     *  - turn off keyboard bell
     *  - return root cursor to normal from hourglass
     *  - start pinging the server
     */
     
    ChangeBell("off");
    if (appInfo.workspaceCursor)
    {
	MakeRootCursor();
    }
    else
    {
	XUndefineCursor(dpyinfo.dpy, dpyinfo.root);
    }
    PingServerCB(NULL, NULL);

    
    /*
     *  bring up the windows and enter event loop...
     */

    XRaiseWindow(XtDisplay(greeting), XtWindow(greeting));
	/*
    XRaiseWindow(XtDisplay(logo_shadow), XtWindow(logo_shadow));
    XRaiseWindow(XtDisplay(logo), XtWindow(logo));
	*/
    /* XtPopup(login_shell, XtGrabNone); */
    _DtEnvControl( DT_ENV_RESTORE_PRE_DT );

    /*
     * Add request callback.
     */
     XtAddInput(0, (XtPointer)XtInputReadMask, RequestCB, NULL);
   
#ifdef VG_TRACE
    vg_TRACE_EXECUTION("main:  entering XtMainLoop ...");
#endif /* VG_TRACE */
    XtMainLoop();
    exit (0);
}




/***************************************************************************
 *
 *  InitToolKit
 *
 *  initialize the toolkit
 ***************************************************************************/

#define MINTIMEOUT	20

static jmp_buf	syncJump;

static SIGVAL
#ifdef _NO_PROTO
syncTimeout()
#else
syncTimeout( int arg )
#endif /* _NO_PROTO */

{
    longjmp (syncJump, 1);
}


static Widget 
#ifdef _NO_PROTO
InitToolKit( argc, argv )
        int argc ;
        char **argv ;
#else
InitToolKit( int argc, char **argv )
#endif /* _NO_PROTO */
{
    
    int		timeout;		/* timeout to initialize toolkit   */
    char	*t;
    Widget	root;
    
#ifdef VG_TRACE
    vg_TRACE_EXECUTION("InitToolKit: enter ...");
#endif /* VG_TRACE */

    /*
     *  use server grabTimeout as initial value for timeout...
     */

    timeout = ((t = (char *)getenv(GRABTIMEOUT)) == NULL ? 0 : atoi(t));
    timeout += MINTIMEOUT;  /* minimum MINTIMEOUT seconds */


    /*
     *  initialize the toolkit. Wrap a timer around it in case the server
     *  is grabbed.
     */

    signal (SIGALRM, syncTimeout);
    if (setjmp (syncJump)) {
	LogError(
		ReadCatalog(MC_LOG_SET,MC_LOG_NO_DPYINIT,MC_DEF_LOG_NO_DPYINIT),
		dpyinfo.name);
	exit(NOTIFY_RESTART);
    }

    alarm ((unsigned) timeout);

    root = XtInitialize("dtlogin", "Dtlogin", NULL, 0, 
    			 &argc, argv);
    /* Disable Drag and Drop  RK 11.02.93 */
    XtVaSetValues(XmGetXmDisplay(XtDisplay(root)),
                XmNdragInitiatorProtocolStyle, XmDRAG_NONE,
                NULL);
 

    alarm (0);
    signal (SIGALRM, SIG_DFL);

    return(root);
}




/***************************************************************************
 *
 *  MakeRootCursor
 *
 *  Widgets: none
 ***************************************************************************/

static void 
#ifdef _NO_PROTO
MakeRootCursor()
#else
MakeRootCursor( void )
#endif /* _NO_PROTO */
{
	Cursor vg_cursor;

	vg_cursor = XCreateFontCursor (dpyinfo.dpy, XC_left_ptr);

	XDefineCursor (dpyinfo.dpy, dpyinfo.root, vg_cursor);

	return;
}

/***************************************************************************
 *
 *  MakeBackground
 *
 *  Widgets: login_shell, table, matte
 ***************************************************************************/

static void 
#ifdef _NO_PROTO
MakeBackground()
#else
MakeBackground( void )
#endif /* _NO_PROTO */
{
    register int i;
    
#ifdef VG_TRACE
    vg_TRACE_EXECUTION("MakeBackground:  entered ...");
#endif /* VG_TRACE */
    /* 
     * create the login shell widget...
     */

    i = 0;

    /*		CORE resource set					*/
    XtSetArg(argt[i], XmNancestorSensitive,	True			); i++;
    XtSetArg(argt[i], XmNbackgroundPixmap,	XmUNSPECIFIED_PIXMAP	); i++;
    XtSetArg(argt[i], XmNborderWidth,		0			); i++;
    XtSetArg(argt[i], XmNmappedWhenManaged,	False			); i++;
    XtSetArg(argt[i], XmNsensitive,		True			); i++;
    XtSetArg(argt[i], XmNtranslations,		NULL			); i++;

    /*		COMPOSITE resource set					*/
    XtSetArg(argt[i], XmNinsertPosition,	NULL			); i++;

    /*		SHELL resource set (set to avoid interference by user)	*/
    XtSetArg(argt[i], XmNallowShellResize,	False			); i++;
    XtSetArg(argt[i], XmNcreatePopupChildProc,	NULL			); i++;
    XtSetArg(argt[i], XmNgeometry,		NULL			); i++;
    XtSetArg(argt[i], XmNpopupCallback,		NULL			); i++;
    XtSetArg(argt[i], XmNpopdownCallback,	NULL			); i++;
    XtSetArg(argt[i], XmNoverrideRedirect,	False			); i++;
    XtSetArg(argt[i], XmNsaveUnder,		False			); i++;

    login_shell = XtCreatePopupShell("login_shell", transientShellWidgetClass,
				     toplevel, argt, i);
    XtAddCallback(login_shell, XmNpopupCallback, LayoutCB, NULL);

    /* Fix to display Input Method's status area. */
    XtSetArg(argt[0], XmNheight, dpyinfo.height);
    XtSetValues(login_shell, argt, 1);


    /* 
     * create the full-screen drawing area...
     */

    i = InitArg(DrawingA);
    XtSetArg(argt[i], XmNwidth,			dpyinfo.width		); i++;
    XtSetArg(argt[i], XmNheight,		dpyinfo.height		); i++;
    XtSetArg(argt[i], XmNunitType,		XmPIXELS		); i++;

    table = XtCreateManagedWidget("table", xmDrawingAreaWidgetClass,
				   login_shell, argt, i);

    XtAddEventHandler(table, ButtonPressMask, False, RefreshEH, NULL);
    XtAddCallback(table, XmNhelpCallback, ShowDialogCB, (XtPointer) help);


    /* 
     * create the main matte...
     */

    i = InitArg(Form);
    /*		      XmNwidth,			(set by user)		*/
    /*		      XmNheight,		(set by user)		*/
    XtSetArg(argt[i], XmNshadowThickness,       SHADOW_THICKNESS        ); i++;
/*
    XtSetArg(argt[i], XmNshadowType,	XmSHADOW_OUT	); i++;
    XtSetArg(argt[i], XmNshadowThickness,	5	); i++;
*/

    matte = XmCreateForm(table, "matte", argt, i);
    XtManageChild(matte);

    i = 0;
	XtSetArg(argt[i], XmNshadowType, XmSHADOW_OUT); i++;
	XtSetArg(argt[i], XmNshadowThickness, 2); i++;
	XtSetArg(argt[i], XmNtopAttachment, XmATTACH_FORM); i++;
	XtSetArg(argt[i], XmNbottomAttachment, XmATTACH_FORM); i++;
	XtSetArg(argt[i], XmNleftAttachment, XmATTACH_FORM); i++;
	/*
	XtSetArg(argt[i], XmNrightAttachment, XmATTACH_FORM); i++;
	*/
	XtSetArg(argt[i], XmNtopOffset, 15); i++;
	XtSetArg(argt[i], XmNbottomOffset, 15); i++;
	XtSetArg(argt[i], XmNleftOffset, 15); i++;
	/*
	XtSetArg(argt[i], XmNrightOffset, 15); i++;
	*/
	matteFrame = XmCreateFrame(matte, "matteFrame", argt, i);
    XtManageChild(matteFrame);

	i = 0;
	matte1 = XmCreateForm(matteFrame, "matte1", argt, i);
    XtManageChild(matte1);
	
}


static void
#ifdef _NO_PROTO
MakeAltDtButtons()
#else
MakeAltDtButtons( void )
#endif /* _NO_PROTO */
{
int i,j;
struct stat	statb;
char        *startup_name; 
XrmValue    startup_value; 
int	    custom_dt = True;
int	    last_dt = False;
char 	    temp[MAXPATHLEN] = "\0";
char 	    *session;
FILE	    *ls;
char	    lastsess[MAXPATHLEN];
int 	    adt = False;
char 	    *temp_p;



    if(getenv("SESSION_SET") == NULL) {
	last_dt = True;
	custom_dt = False;           /* disable the regular desktop */
    }


    if ((session = getenv ("SESSION")) == NULL)  {
		session = "  ";
	        last_dt = True;
    }


    if(appInfo.altDts > 0) { 
      if((alt_dts = (Widget *) calloc(appInfo.altDts, 
	      sizeof (Widget))) == NULL)
        LogError((unsigned char *)
	ReadCatalog(MC_ERROR_SET,MC_NO_MEMORY, MC_DEF_NO_MEMORY),dpyinfo.name);

     for(i = 0; i < appInfo.altDts; ++i) {
       sprintf(altdtname,"%s%d",
	"Dtlogin*altDtName",i+1);  /* alt desktops begin numbering with 1 */
       sprintf(altdtclass,"%s%d",
	"Dtlogin*AltDtName",i+1); 

       sprintf(altdtkey,"%s%d","Dtlogin*altDtKey",i+1); 
       sprintf(altdtkeyclass,"%s%d","Dtlogin*AltDtKey",i+1); 

       sprintf(altdtstart,"%s%d","Dtlogin*altDtStart",i+1); 
       sprintf(altdtstartclass,"%s%d","Dtlogin*AltDtStart",i+1); 

	if(XrmGetResource(XtDatabase(dpyinfo.dpy), 
	  altdtkey, altdtkeyclass, &keyrmtype, &keyrmvalue) == True) {

	  /* 
	   * remove trailing spaces 
	   */
	  if(strchr(keyrmvalue.addr,' '))
	      temp_p = strtok(keyrmvalue.addr," ");
	  else 
	      temp_p =  keyrmvalue.addr;


	  if(stat( temp_p, &statb) == NULL) { 
    	    j = InitArg(ToggleBG);
    	    if(XrmGetResource(XtDatabase(dpyinfo.dpy),
	       altdtstart,altdtstartclass,&startup_name,
	       &startup_value) == True) {

	      /* 
	       * remove trailing spaces 
	       */
	       if(strchr(startup_value.addr,' ')) 
		   strcpy(temp,strtok(startup_value.addr," "));
               else
	           strcpy(temp,startup_value.addr);

	       if(custom_dt)
		 if(strcmp(session,temp) == 0)    {
    		   XtSetArg(argt[j], XmNset,       True                );j++; 
		     custom_dt = False;  /* it is not a new desktop */
		     last_dt = False;	/* enable  the alt-desktop */
		 }
	    }
	    else
	      fprintf(stderr, "No startup script for altdt %d \n", i);

    	      if(XrmGetResource(XtDatabase(dpyinfo.dpy), 
		altdtname, altdtclass, &rmtype, &rmvalue) == True){
    		xmstr = XmStringCreateLtoR(rmvalue.addr, 
			XmFONTLIST_DEFAULT_TAG);
	      }
      	      else {
	       fprintf(stderr,"Couldn't find the altdtname resource in the database \n");
	       sprintf(altdtname,"%s%d", "Alternate Desktop-",i+1);
    	       xmstr = XmStringCreateLtoR(altdtname, 
		                XmFONTLIST_DEFAULT_TAG);
	      }

    	      sprintf(altdtlogo,"%s%d","Dtlogin*altDtLogo",i+1); 
    	      sprintf(altlogoclass,"%s%d","Dtlogin*AltDtLogo",i+1); 
    	      if(XrmGetResource(XtDatabase(dpyinfo.dpy), altdtlogo, 
		altlogoclass, &logotype, &logovalue) == True) {

    	        XtSetArg(argt[j], XmNuserData,              
			logovalue.addr); j++;
	      }
	      else{	
    	        XtSetArg(argt[j], XmNuserData,		   
		 	logoInfo.bitmapFile     ); j++;
	      }

    	      XtSetArg(argt[j], XmNlabelString,                   
				xmstr           ); j++;
              XtSetArg(argt[j], XmNrecomputeSize,                 
				True            ); j++;
 
    	      alt_dts[i] = XmCreateToggleButtonGadget(session_menu, 
				rmvalue.addr, argt, j);
    	      XmStringFree(xmstr);
    	      XtAddCallback(alt_dts[i], XmNvalueChangedCallback, 
		MenuItemCB, (XtPointer) OB_ALT_DTS);
    	      XtManageChild(alt_dts[i]);
	      adt = True;
	  }
	  else  
	    fprintf(stderr,"Couldn't find the keyfile \n");
	}
	else
	    fprintf(stderr,"Couldn't find the altkeyfile resource in the database\n");
     }  
    }   

    if ((appInfo.altDts == 0) || !adt)
    	XtManageChild(options_dt);  
	
	
    if(custom_dt && !last_dt) { 	
	/* 
	 * It is a regular desktop, if none of the known sessions matched ...
	 * 
	 */
	i = 0;
    	XtSetArg(argt[i], XmNset,       True                   );i++; 
    	XtSetValues(options_dt,  argt, i);

	if(adt)
       	  XtManageChild(options_dt);  
    }

    i = InitArg(ToggleBG);

    if(last_dt){
     /*	if session variable is not set, set user's last dt .. */
	XtSetArg(argt[i], XmNset,       True                   );i++; 
    }
    xmstr = ReadCatalog(MC_LABEL_SET, MC_USER_DT_LABEL, MC_DEF_USER_DT_LABEL);
    XtSetArg(argt[i], XmNuserData,                      logoInfo.bitmapFile            ); i++;
    XtSetArg(argt[i], XmNlabelString,                   xmstr           ); i++;
    XtSetArg(argt[i], XmNrecomputeSize,                 True            ); i++;
    options_last_dt = XmCreateToggleButtonGadget(session_menu, 
				"options_last_dt", argt, i);
    XmStringFree(xmstr);
    XtAddCallback(options_last_dt, XmNvalueChangedCallback, 
		MenuItemCB, (XtPointer) OB_LAST_DT);
    XtManageChild(options_last_dt);
 
    	/*
     	 *  [ Failsafe Session ] menu pane... 
     	 */
    i = InitArg(ToggleBG);
    xmstr = ReadCatalog(MC_LABEL_SET, MC_FS_LABEL, MC_DEF_FS_LABEL);
    XtSetArg(argt[i], XmNuserData,       logoInfo.bitmapFile            ); i++;
    XtSetArg(argt[i], XmNlabelString,                   xmstr           ); i++;
    XtSetArg(argt[i], XmNrecomputeSize,                 True            ); i++;
    options_failsafe = XmCreateToggleButtonGadget(session_menu, 
		"options_failsafe", argt, i);
    XmStringFree(xmstr);
    XtAddCallback(options_failsafe, XmNvalueChangedCallback, 
		MenuItemCB, (XtPointer) OB_FAILSAFE);

    XtManageChild(options_failsafe);

    /* 
     * which option to set..
     */

    SetDtLabelAndIcon();
}

/***************************************************************************
 *
 *  MakeButtons
 *
 *  Widgets:	ok_button, clear_button, options_button, help_button
 ***************************************************************************/


static void 
#ifdef _NO_PROTO
MakeButtons()
#else
MakeButtons( void )
#endif /* _NO_PROTO */
{
    register int i;

    Dimension	max_width;	/* maximum width  of a set of widgets	   */
    Dimension	max_height;	/* maximum height of a set of widgets	   */
    Dimension	thick1;		/* defaultButtonShadowThickness */
    Dimension	thick2;		/* shadowThickness */

    int		origin;		/* horizontal origin for button placement  */
    int		spacing;	/* spacing between buttons (width/32)      */

#ifdef VG_TRACE
    vg_TRACE_EXECUTION("MakeButtons:  entered ...");
#endif /* VG_TRACE */
     
    /* 
     * create the buttons...
     */

    /* ok button */
    
    i = InitArg(PushBG);
    XtSetArg(argt[i], XmNbottomAttachment, XmATTACH_POSITION); i++;
    XtSetArg(argt[i], XmNbottomPosition, 95); i++;
    XtSetArg(argt[i], XmNtraversalOn, True); i++;

    xmstr = ReadCatalog(MC_LABEL_SET, MC_OK_LABEL, MC_DEF_OK_LABEL );
    XtSetArg(argt[i], XmNlabelString,			xmstr		); i++;

    ok_button = XmCreatePushButtonGadget(matte1, "ok_button", argt, i);

    XmStringFree(xmstr);
    XtManageChild(ok_button);

    XtAddCallback(ok_button, XmNactivateCallback, RespondChallengeCB, NULL);    


    /* clear button */

    i -= 1;
    xmstr = ReadCatalog(MC_LABEL_SET, MC_CLEAR_LABEL, MC_DEF_CLEAR_LABEL);
    XtSetArg(argt[i], XmNlabelString,			xmstr		); i++;

    clear_button = XmCreatePushButtonGadget(matte1, "clear_button", argt, i);

    XmStringFree(xmstr);
    XtManageChild(clear_button);
    XtAddCallback(clear_button, XmNactivateCallback, RespondClearCB,
                  (XtPointer) 0);   



    /* help button */
    
    i -= 1;
    xmstr = ReadCatalog(MC_LABEL_SET, MC_HELP_LABEL, MC_DEF_HELP_LABEL);
    XtSetArg(argt[i], XmNlabelString,			xmstr		); i++;

    help_button = XmCreatePushButtonGadget(matte1, "help_button", argt, i);
    XtAddCallback(help_button, XmNactivateCallback, ShowDialogCB, 
    		  (XtPointer) help);
    XmStringFree(xmstr);
    XtManageChild(help_button);

    /* options button */

    i = InitArg(Label);
    XtSetArg(argt[i], XmNbottomAttachment, XmATTACH_POSITION); i++;
    XtSetArg(argt[i], XmNbottomPosition, 95); i++;
    xmstr = ReadCatalog(MC_LABEL_SET, MC_OPTIONS_LABEL, MC_DEF_OPTIONS_LABEL );
    XtSetArg(argt[i], XmNlabelString,			xmstr		); i++;

    options_button = DtCreateMenuButton(matte1, "options_button", argt, i);
    XtManageChild(options_button);
    XmStringFree(xmstr);

    /*
     *  tell form that ok_button is the default button...
     */

    i = 0;
    XtSetArg(argt[i], XmNdefaultButton,		ok_button		); i++;
    XtSetValues(matte1,  argt, i);
	
    /*
     * make all buttons *look* the same size...
     */
     

    max_width = max_height = 0;
    GetBiggest(ok_button, &max_width, &max_height);
    GetBiggest(clear_button,   &max_width, &max_height);
    GetBiggest(options_button, &max_width, &max_height);
    GetBiggest(help_button,    &max_width, &max_height);

    if ( max_width < MIN_BUTTON_SIZE) max_width = MIN_BUTTON_SIZE;
    
    i = 0;
    XtSetArg(argt[i], XmNdefaultButtonShadowThickness, &thick1); i++;
    XtSetArg(argt[i], XmNshadowThickness, &thick2); i++;
    XtGetValues(ok_button,      argt, i);
    thick1 *= 4;
    thick1 += thick2;

    i = 0;
    XtSetArg(argt[i], XmNwidth,			max_width		); i++;
    XtSetArg(argt[i], XmNheight,		max_height		); i++;
    XtSetArg(argt[i], XmNrecomputeSize,		False			); i++;
    XtSetArg(argt[i], XmNbottomOffset,		thick1			); i++;
    XtSetValues(options_button, argt, i);

    i = 0;
    XtSetArg(argt[i], XmNwidth,			max_width + 2*thick1	); i++;
    XtSetArg(argt[i], XmNheight,		max_height + 2*thick1	); i++;
    XtSetArg(argt[i], XmNrecomputeSize,		False			); i++;

    XtSetValues(ok_button,      argt, i);
    XtSetValues(clear_button,   argt, i);
    XtSetValues(help_button,    argt, i);
}




/***************************************************************************
 *
 *  MakeDialog
 *
 *  Widgets: error_message, help_message, copyright_msg, hostname_message,
 *	     passwd_message
 ***************************************************************************/

void 
#ifdef _NO_PROTO
MakeDialog( dtype )
        DialogType dtype ;
#else
MakeDialog( DialogType dtype )
#endif /* _NO_PROTO */
{
    register int i, j;

    int		width;
    
    FILE	*fp, *fopen();
    char	buffer[128];

    Widget	w, text;
    Dimension txt_width, txt_height;
    XmString	ok, cancel, nw, sv;
    
    
#ifdef VG_TRACE
    vg_TRACE_EXECUTION("MakeDialog:  entered ...");
#endif /* VG_TRACE */
    /*
     *  do things common to all dialogs...
     */

    ok     = ReadCatalog(MC_LABEL_SET, MC_OK_LABEL, MC_DEF_OK_LABEL);
    cancel = ReadCatalog(MC_LABEL_SET, MC_CANCEL_LABEL, MC_DEF_CANCEL_LABEL);

    i = InitArg(MessageBox);
    XtSetArg(argt[i], XmNmarginHeight,		MBOX_MARGIN_HEIGHT	); i++;
    XtSetArg(argt[i], XmNmarginWidth,		MBOX_MARGIN_WIDTH	); i++;
    XtSetArg(argt[i], XmNshadowThickness,	SHADOW_THICKNESS	); i++;
    XtSetArg(argt[i], XmNokLabelString,		ok			); i++;
    XtSetArg(argt[i], XmNcancelLabelString,	cancel			); i++;
    XtSetArg(argt[i], XmNnoResize,		False			); i++;
    XtSetArg(argt[i], XmNresizePolicy,		XmRESIZE_ANY		); i++;

    /*
     *  create the various dialogs...
     */

    switch (dtype) {

    case error:
	xmstr = ReadCatalog(MC_ERROR_SET, MC_LOGIN, "");
	XtSetArg(argt[i], XmNmessageString,		xmstr		); i++;

	w = XmCreateErrorDialog(table, "error_message", argt, i);
	XtUnmanageChild(XmMessageBoxGetChild(w,XmDIALOG_CANCEL_BUTTON));
	XtUnmanageChild(XmMessageBoxGetChild(w,XmDIALOG_HELP_BUTTON));

	error_message = w;
	break;


    case help:
	xmstr = ReadCatalog(MC_HELP_SET, MC_HELP, MC_DEF_HELP);

        w = XmCreateInformationDialog(table, "help_message", argt, i);
        XtUnmanageChild(XmMessageBoxGetChild(w,XmDIALOG_CANCEL_BUTTON));
        XtUnmanageChild(XmMessageBoxGetChild(w,XmDIALOG_HELP_BUTTON));

        txt_width = DisplayWidth (XtDisplay(w), DefaultScreen(XtDisplay(w)));
        txt_height = DisplayHeight (XtDisplay(w), DefaultScreen(XtDisplay(w)));
        txt_width = (txt_width > 850) ? 800 : txt_width - 50;
        txt_height = (txt_height > 900) ? 750 : txt_height - 150;
        i = InitArg(Text);
        XtSetArg(argt[i], XmNheight, txt_height); i++;
        XtSetArg(argt[i], XmNwidth, txt_width); i++;
        XtSetArg(argt[i], XmNeditMode, XmMULTI_LINE_EDIT); i++;
        XtSetArg(argt[i], XmNscrollBarDisplayPolicy, XmAS_NEEDED); i++;
        XtSetArg(argt[i], XmNscrollingPolicy, XmAUTOMATIC); i++;
        XtSetArg(argt[i], XmNeditable, False); i++;
        XtSetArg(argt[i], XmNvalue,
		 strdup(catgets(nl_fd, MC_HELP_SET, MC_HELP, MC_DEF_HELP)));i++;
        text = XmCreateScrolledText(w, "help_message_text", argt, i);

        XtManageChild(text);
        XtManageChild(w);
        help_message = w;
	break;


    case copyright:
	if ((fp = fopen(COPYRIGHT,"r")) == NULL)
#if defined( __hp_osf )
	    xmstr = XmStringCreate("Cannot open copyright file '/usr/X11/copyright'.",
				XmFONTLIST_DEFAULT_TAG);
#else
	    xmstr = XmStringCreate("Cannot open copyright file '/etc/copyright'.",
				XmFONTLIST_DEFAULT_TAG);
#endif
	else {
	    xmstr = (XmString) NULL;
	
	    while (fgets(buffer, 128, fp) != NULL) {
		j = strlen(buffer);
		if ( buffer[j-1] == '\n' ) buffer[j-1] = '\0';
	    
		if ( xmstr != NULL )
		    xmstr = XmStringConcat(xmstr, XmStringSeparatorCreate());

		xmstr = XmStringConcat(xmstr,
	    			       XmStringCreate(buffer,
				       XmFONTLIST_DEFAULT_TAG));
	    }	    
	}

	    fclose(fp);
	XtSetArg(argt[i], XmNmessageString,		xmstr		); i++;

	w = XmCreateInformationDialog(table, "copyright_msg", argt, i);
	XtUnmanageChild(XmMessageBoxGetChild(w,XmDIALOG_CANCEL_BUTTON));
	XtUnmanageChild(XmMessageBoxGetChild(w,XmDIALOG_HELP_BUTTON));
	
	XtAddCallback(w, XmNokCallback, CopyrightCB, (XtPointer) 0);    

	copyright_msg = w;
	break;
    

    case hostname:
	    
	nw = ReadCatalog(MC_LABEL_SET, MC_NW_LABEL,  MC_DEF_NW_LABEL);
	sv = ReadCatalog(MC_LABEL_SET, MC_START_LABEL, MC_DEF_START_LABEL);

	xmstr = ReadCatalog(MC_HELP_SET, MC_SYSTEM, MC_DEF_SYSTEM);
	XtSetArg(argt[i], XmNmessageString,		xmstr		); i++;
	XtSetArg(argt[i], XmNokLabelString,		nw		); i++;
	XtSetArg(argt[i], XmNcancelLabelString,		sv		); i++;

	w = XmCreateWarningDialog(table, "hostname_msg", argt, i);

	XtUnmanageChild(XmMessageBoxGetChild(w,XmDIALOG_HELP_BUTTON));

	XmStringFree(nw);
	XmStringFree(sv);

	hostname_message = w;
	break;

    case bad_hostname:
        xmstr = ReadCatalog(MC_LOG_SET, MC_BAD_HOSTNAME, MC_DEF_BAD_HOSTNAME);
        XtSetArg(argt[i], XmNmessageString,            xmstr           ); i++;
        XtSetArg(argt[i], XmNlabelFontList,     appInfo.labelFont       ); i++;
        XtSetArg(argt[i], XmNtextFontList,      appInfo.labelFont       ); i++;
 
        w = XmCreateWarningDialog(table, "hostname_msg", argt, i);
 
        XtUnmanageChild(XmMessageBoxGetChild(w,XmDIALOG_HELP_BUTTON));
        XtUnmanageChild(XmMessageBoxGetChild(w,XmDIALOG_CANCEL_BUTTON));
 
        bad_host = w;
 
        break;

    case host_not_runlogin:
        xmstr = ReadCatalog(MC_LOG_SET, MC_HOST_NOT_RUN_LOGIN, 
                                              MC_DEF_HOST_NOT_RUN_LOGIN);
        XtSetArg(argt[i], XmNmessageString,            xmstr           ); i++;
        XtSetArg(argt[i], XmNlabelFontList,     appInfo.labelFont       ); i++;
        XtSetArg(argt[i], XmNtextFontList,      appInfo.labelFont       ); i++;
 
        w = XmCreateWarningDialog(table, "host_not_running", argt, i);
 
        XtUnmanageChild(XmMessageBoxGetChild(w,XmDIALOG_HELP_BUTTON));
        XtUnmanageChild(XmMessageBoxGetChild(w,XmDIALOG_CANCEL_BUTTON));
 
        host_not_run_login = w;
        break;
 
    case conn_to_host:
        xmstr = ReadCatalog(MC_LOG_SET, MC_CONNECT_TO_HOST,
                                                 MC_DEF_CONNECT_TO_HOST);
        XtSetArg(argt[i], XmNmessageString,            xmstr           ); i++;
        XtSetArg(argt[i], XmNlabelFontList,     appInfo.labelFont       ); i++;
        XtSetArg(argt[i], XmNtextFontList,      appInfo.labelFont       ); i++;
 
        w = XmCreateWarningDialog(table, "conn_to_host", argt, i);
 
        XtUnmanageChild(XmMessageBoxGetChild(w,XmDIALOG_OK_BUTTON));
        XtUnmanageChild(XmMessageBoxGetChild(w,XmDIALOG_HELP_BUTTON));
        XtUnmanageChild(XmMessageBoxGetChild(w,XmDIALOG_CANCEL_BUTTON));
 
        connect_to_host = w;
        break;
 
    case expassword:

	xmstr = ReadCatalog(MC_ERROR_SET, MC_PASSWD_EXPIRED, 
			    MC_DEF_PASSWD_EXPIRED);
	XtSetArg(argt[i], XmNmessageString,		xmstr		); i++;

	w = XmCreateQuestionDialog(table, "password_msg", argt, i);

	XtUnmanageChild(XmMessageBoxGetChild(w,XmDIALOG_HELP_BUTTON));

	passwd_message = w;
	break;
    }

    /*
     *  finish up...
     */

    switch (dtype) {
      case error:
      case hostname:
      case expassword:
        XtAddCallback(w, XmNokCallback,     RespondDialogCB, NULL);
        XtAddCallback(w, XmNcancelCallback, RespondDialogCB, NULL);
        break;
    }


    XtSetArg(argt[0], XmNdialogStyle,	XmDIALOG_APPLICATION_MODAL	); i++;
    XtSetValues(w, argt, 1);

    XmStringFree(xmstr);
    XmStringFree(ok);
    XmStringFree(cancel);


    /*
     *  adjust the width of the "ok" button on the dialogs...
     */

    width = (dtype == hostname ? FromMM(4000) : MIN_BUTTON_SIZE);
    
    i = 0;
    XtSetArg(argt[i], XmNrecomputeSize,			False		); i++;
    XtSetArg(argt[i], XmNwidth,				width		); i++;

    XtSetValues(XmMessageBoxGetChild(w, XmDIALOG_OK_BUTTON), argt, i);

}

/***************************************************************************
 *
 *  MakeDtlabel
 *
 *  Widgets:	dt_label
 ***************************************************************************/

static void
#ifdef _NO_PROTO
MakeDtlabel()
#else
MakeDtlabel( void )
#endif /* _NO_PROTO */
{
    int i;

#ifdef VG_TRACE
    vg_TRACE_EXECUTION("MakeDtlabel:  entered ...");
#endif /* VG_TRACE */

    i = InitArg(LabelG);
    XtSetArg(argt[i], XmNtraversalOn,           False                   ); i++;
    XtSetArg(argt[i], XmNtopAttachment,         XmATTACH_WIDGET         ); i++;
    XtSetArg(argt[i], XmNleftAttachment, 	XmATTACH_FORM           ); i++;
    XtSetArg(argt[i], XmNrightAttachment,       XmATTACH_FORM           ); i++;
    XtSetArg(argt[i], XmNalignment,             XmALIGNMENT_CENTER      ); i++;
    XtSetArg(argt[i], XmNtopWidget,             greeting                ); i++;
 
    dt_label = XmCreateLabel(matte1, "dt_label", argt, i);
    XtManageChild(dt_label);
}



/***************************************************************************
 *
 *  MakeGreeting
 *
 *  Widgets:	greeting
 ***************************************************************************/
    
#define LOCALHOST "%LocalHost%"
#define DISPLAYNAME "%DisplayName%"

typedef  struct {
    char	*labelString;	/* string for label			   */
    char        *persLabelString; /* alternate string for label */
    XmFontList	fontList;
} GreetInfo, *GreetInfoPtr;

static GreetInfo greetInfo;

static	XtResource greetResources[] = {
    {XmNlabelString, XmCLabelString, 
        XmRString, sizeof(char *),
	XtOffset(GreetInfoPtr, labelString), XtRString, "default"	},

    {"persLabelString", "PersLabelString",
        XmRString, sizeof(char *),
        XtOffset(GreetInfoPtr, persLabelString), XtRString, "default"       },

    {XmNfontList, XmCFontList, 
        XmRFontList, sizeof(XmFontList),
	XtOffset(GreetInfoPtr, fontList), XtRString, NULL		}
};     

static void 
#ifdef _NO_PROTO
MakeGreeting()
#else
MakeGreeting( void )
#endif /* _NO_PROTO */
{
    register int i;

    char    *greetmsg;
    char    host[128];
    char    disp[128];
    char    *p, *q, *s, *t;
    int	    newLine = False;
    int     skip;

#ifdef VG_TRACE
    vg_TRACE_EXECUTION("MakeGreeting:  entered ...");
#endif /* VG_TRACE */

    /*
     *  get the user's greeting preferences...
     */
     
    XtGetSubresources(table, &greetInfo, "greeting", "Greeting",
	greetResources, XtNumber(greetResources), NULL, 0);


    /*
     *  get the local hostname...
     */
     
    gethostname(host, sizeof (host));
    if ( (p = strchr(host,'.')) != NULL )
	*p = '\0';

    /* 
    **  Get display name (for %DisplayName% substitutions),
    **  reducing "a.b.c.d:0" constructs to shorter "a:0" form.
    */

    strncpy(disp,
        dpyinfo.name ? dpyinfo.name : (DisplayString(dpyinfo.dpy)), 127);
    disp[127] = '\0';
    p = strchr(disp, '.');
    t = strchr(disp, ':');
    if (p && t) strcpy(p,t);
    

    /*
     *  use the default string if the user has not specified one...
     */

    if ( greetInfo.persLabelString &&
         strcmp(greetInfo.persLabelString, "default") == 0 )
    {
      greetInfo.persLabelString = strdup(catgets(nl_fd, MC_LABEL_SET,
        MC_PERS_GREET_LABEL, MC_DEF_PERS_GREET_LABEL));
    }
 
    if ( greetInfo.labelString && 
         strcmp(greetInfo.labelString, "default") == 0 ) {
	
	xmstr = ReadCatalog(MC_LABEL_SET, MC_GREET_LABEL, MC_DEF_GREET_LABEL);
	xmstr = XmStringConcat(xmstr,
    			       XmStringCreate(" ",  XmFONTLIST_DEFAULT_TAG));
    if ( getenv(PINGINTERVAL) != NULL )
        xmstr = XmStringConcat(xmstr,
                       XmStringCreate("remote host ", XmFONTLIST_DEFAULT_TAG));
	xmstr = XmStringConcat(xmstr,
    			       XmStringCreate(host, XmFONTLIST_DEFAULT_TAG));
    }
    else {
	/*
	 *  scan user's message for %LocalHost% token. Replace with hostname
	 *  if found...
	 */
	 

	if ( strlen(greetInfo.labelString) == 0 	||
	     strcmp(greetInfo.labelString, "None") == 0	||
	     strcmp(greetInfo.labelString, "none") == 0	)
	     
	    greetmsg = strdup(" ");
	else {
	    greetmsg = strdup(greetInfo.labelString);
	}

	s = greetmsg;
	xmstr = (XmString) NULL;

	do {
	    q = s;
	
	    /*
	     *  scan for a new line character in remaining label string. 
	     *  If found, work with that substring first...
	     */
	     
	    if ( (p = strchr(q, '\n')) != NULL ) {
		*p = '\0';
		newLine = True;
		s = ++p;

		if ( *q == '\0' )	/* handle consecutive newlines */
		    q = " ";
		    
	    }
	    else {
		newLine = False;
	    }
	    
	    /*
	     *  replace all occurrances of %LocalHost% and %DisplayName%
             *  in the current substring...
	     */
	     
            while (1) {
                p = strstr(q, LOCALHOST);
                t = strstr(q, DISPLAYNAME);
                
                if (p && t) { /* both present? do whichever comes first */
                    if (p > t) p = NULL;
                    else t = NULL;
                }
                if (p) { /* replace a %LocalHost% string */
                    t = host;
                    skip = sizeof(LOCALHOST);
                } else if (t) { /* replace a %DisplayName% string */
                    p = t;
                    t = disp;
                    skip = sizeof(DISPLAYNAME);
                } else /* nothing left to replace */
                    break;
		*p = '\0';
		xmstr = XmStringConcat(xmstr,
				   XmStringCreate(q,
				   XmFONTLIST_DEFAULT_TAG));
		xmstr = XmStringConcat(xmstr,
				   XmStringCreate(t,
				   XmFONTLIST_DEFAULT_TAG));
                q = p + skip - 1;
	    }

	    if ( strlen(q) != 0 )
		xmstr = XmStringConcat(xmstr,
				   XmStringCreate(q,
				   XmFONTLIST_DEFAULT_TAG));

	    /*
	     *  add a line seperator if this is a multi-line greeting...
	     */
	     
	    if ( newLine == True ) {
		xmstr = XmStringConcat(xmstr, XmStringSeparatorCreate());
	    }
	    	    	     
	} while ( newLine == True ) ;
	
	free(greetmsg);
    }


    /* 
     * create the Welcome message...
     */
    
    i = InitArg(LabelG);
    XtSetArg(argt[i], XmNtraversalOn,		False			); i++;
    XtSetArg(argt[i], XmNlabelString,		xmstr			); i++;
    XtSetArg(argt[i], XmNleftAttachment, XmATTACH_FORM); i++;
    XtSetArg(argt[i], XmNtopAttachment, XmATTACH_POSITION); i++; 
   /* XtSetArg(argt[i], XmNtopPosition, 15); i++; */
   /* Changed this to accomodate desktop label */
    XtSetArg(argt[i], XmNtopPosition, 9); i++; 
    XtSetArg(argt[i], XmNrightAttachment, XmATTACH_FORM); i++;


    /*
     *  use the user's font if one has been specified, otherwise use
     *  the application's default...
     */

    if ( greetInfo.fontList != NULL ) {
	XtSetArg(argt[i], XmNfontList,		greetInfo.fontList	); i++;
    }

    greeting = XmCreateLabel(matte1, "greeting", argt, i);
    XtManageChild(greeting);

    XmStringFree(xmstr);
}

/***************************************************************************
 *
 *  MakeLogin
 *
 *  Widgets: login_matte, 
 *	     login_form, login_label, login_text
 ***************************************************************************/

static void 
#ifdef _NO_PROTO
MakeLogin()
#else
MakeLogin( void )
#endif /* _NO_PROTO */
{
    register int i;
    int	j;
    LoginTextPtr textdata; 
    XtTranslations      textTable;
    Widget passwd_text;
    String greetstr;

#ifdef VG_TRACE
    vg_TRACE_EXECUTION("MakeLogin:  entered ...");
#endif /* VG_TRACE */

    /*
     *  create the login form
     */

    i = 0;
    XtSetArg(argt[i], XmNshadowThickness, 0); i++; 
    XtSetArg(argt[i], XmNresizable,		False			); i++;
    XtSetArg(argt[i], XmNleftAttachment, XmATTACH_FORM); i++;
    XtSetArg(argt[i], XmNleftOffset, 80); i++;
    XtSetArg(argt[i], XmNrightAttachment, XmATTACH_FORM); i++;
    XtSetArg(argt[i], XmNrightOffset, 80); i++;
    XtSetArg(argt[i], XmNbottomAttachment, XmATTACH_POSITION); i++;
    XtSetArg(argt[i], XmNbottomPosition, 65); i++;
/*
    XtSetArg(argt[i], XmNresizePolicy, XmRESIZE_ANY); i++;
*/
    XtSetArg(argt[i], XmNallowShellResize, True); i++;

    login_form = XmCreateForm(matte1, "login_form", argt, i);
    XtManageChild(login_form);

    /*
     *  create the login text field...
     */

    i = InitArg(Text);
    XtSetArg(argt[i], XmNbottomAttachment,	XmATTACH_POSITION       ); i++;
    XtSetArg(argt[i], XmNleftAttachment,	XmATTACH_POSITION       ); i++;
    XtSetArg(argt[i], XmNrightAttachment,	XmATTACH_POSITION       ); i++;
    XtSetArg(argt[i], XmNbottomPosition,	95			); i++;
    XtSetArg(argt[i], XmNrightPosition, 	80			); i++;
    XtSetArg(argt[i], XmNleftPosition,          20                      ); i++;
    XtSetArg(argt[i], XmNselectionArrayCount,	1			); i++;
    XtSetArg(argt[i], XmNmaxLength,		80			); i++;
    XtSetArg(argt[i], XmNmappedWhenManaged,     False                   ); i++;

    textdata = malloc(sizeof(LoginText));
    XtSetArg(argt[i], XmNuserData,              textdata                ); i++;

    login_text = XmCreateTextField(login_form, "login_text", argt, i);

    /*
     *  From Human Interface model, Tab key operation should work same on
     *  user field as it does on password field.  Password field is setup
     *  to take Tab key as password data.  HIE model is for user field to
     *  do same.
     */	 
    XtOverrideTranslations( login_text, XtParseTranslationTable(
				" Shift <Key>Tab: prev-tab-group() \n\
				   Ctrl <Key>Tab: next-tab-group() \n\
				        <Key>Tab: my-insert() "));

    XtManageChild(login_text);
#ifdef __hpux
    XtAddCallback(login_text, XmNfocusCallback, TextFocusCB, NULL);
#endif


    XtAddActions(textActions, 2);
    textTable = XtParseTranslationTable(textEventBindings);

    XtSetArg(argt[i], XmNtranslations,          textTable               ); i++;
    XtSetArg(argt[i], XmNverifyBell,            False                   ); i++;

    passwd_text = XmCreateTextField(login_form, "passwd_text", argt, i);

    textdata->bEcho = True;
    textdata->noechobuf[0] = '\0';
    textdata->text[0] = passwd_text;
    textdata->text[1] = login_text;

    XtManageChild(passwd_text);
#ifdef __hpux
    XtAddCallback(passwd_text, XmNfocusCallback, TextFocusCB, NULL);
#endif
    XtAddCallback(passwd_text, XmNmodifyVerifyCallback, EditPasswdCB, NULL);
    XtAddCallback(passwd_text, XmNmotionVerifyCallback, EditPasswdCB, NULL);

   /*
    * Get default greeting string
    */
    i = 0;
    XtSetArg(argt[i], XmNlabelString, &textdata->onGreeting); i++;
    XtGetValues(greeting, argt, i);
    textdata->offGreetingFormat = greetInfo.persLabelString;
    textdata->offGreetingUname = NULL;

    /*
     *  create the login labels...
     */

    i = InitArg(LabelG);

   /* modified recomputeSize initial value from False to True, fails 
    * when setting longer strings. Manifested as bug ID:1200690.
    */ 
    XtSetArg(argt[i], XmNrecomputeSize,         True                    ); i++;

    XtSetArg(argt[i], XmNtraversalOn,		False			); i++;
    XtSetArg(argt[i], XmNbottomAttachment,	XmATTACH_WIDGET		); i++;
    XtSetArg(argt[i], XmNleftAttachment,        XmATTACH_OPPOSITE_WIDGET); i++;

/* XtSetArg(argt[i], XmNleftAttachment,	XmATTACH_FORM           ); i++; 
   Commented this statement to  align login_label and login_text    			
    XtSetArg(argt[i], XmNrightAttachment,	XmATTACH_FORM           ); i++;
    XtSetArg(argt[i], XmNalignment,		XmALIGNMENT_CENTER      ); i++;
    XtSetArg(argt[i], XmNbottomOffset,          10                      ); i++;
*/

    XtSetArg(argt[i], XmNleftWidget,            login_text              ); i++;
    XtSetArg(argt[i], XmNbottomWidget,  	login_text              ); i++;



    xmstr = ReadCatalog(MC_LABEL_SET, MC_LOGIN_LABEL, MC_DEF_LOGIN_LABEL);
    XtSetArg(argt[i], XmNlabelString,		xmstr			); i++;
    
    login_label = XmCreateLabel(login_form, "login_label", argt, i);
    XtManageChild(login_label);

    XmStringFree(xmstr);

}




/***************************************************************************
 *
 *  MakeOptionsMenu
 *
 *  Widgets: options_menu, options_item[]
 ***************************************************************************/

void 
#ifdef _NO_PROTO
MakeOptionsMenu()
#else
MakeOptionsMenu( void )
#endif /* _NO_PROTO */
{
    int 	i, j, k, ri = 0;

    struct stat	statb;
    Widget remote_item[10];

#ifdef VG_TRACE
    vg_TRACE_EXECUTION("MakeOptionsMenu:  entered ...");
#endif /* VG_TRACE */

    /*
     * get the built-in pop_up menu from the DtMenuButton...
     */
     
    XtVaGetValues(options_button, DtNsubMenuId, &options_menu, NULL);

    /*
     *  create language cascade menus...
     */

    if ( lang_menu == NULL )
	MakeLangMenu();


    /*
     *  create first level menu items...
     */
    j = 0;

    /*
     *  build [ Language ] menu pane if there are languages to choose from...
     */
    if ( lang_menu != NULL ) {
        /*
         *  [ Language ] menu pane...
         *  attach language cascade menu to this pane
         */
        i = InitArg(CascadeBG);
        xmstr = ReadCatalog(MC_LABEL_SET, MC_LANG_LABEL, MC_DEF_LANG_LABEL);
        XtSetArg(argt[i], XmNlabelString,               xmstr           ); i++;
        XtSetArg(argt[i], XmNsubMenuId,                 lang_menu       ); i++;
        XtSetArg(argt[i], XmNrecomputeSize,             True            ); i++;
        options_item[j] = XmCreateCascadeButtonGadget(options_menu,
                                "options_languages", argt, i);
        XmStringFree(xmstr);
        j++;

        /*
         *  separator...
         */
        i = InitArg(SeparatorG);
        options_item[j] = XmCreateSeparatorGadget(options_menu,
                                                    "options_sep2",
                                                     argt, i);
        j++;
    }

    if( session_menu == NULL)  {
        session_menu = XmCreatePulldownMenu(options_menu, "session_menu", NULL, 0);


    /*
     *  [ Dt "Reg" ] menu pane...
     */
    i = k = InitArg(ToggleBG);
    xmstr = ReadCatalog(MC_LABEL_SET, MC_DT_LABEL, MC_DEF_DT_LABEL);
    /* XtSetArg(argt[i], XmNset,       True                   );i++;  */
    XtSetArg(argt[i], XmNlabelString,                   xmstr           ); i++;
    XtSetArg(argt[i], XmNrecomputeSize,                 True            ); i++;
    XtSetArg(argt[i], XmNuserData,              logoInfo.bitmapFile     ); i++;

/*
    if ( getenv(DTLITE) != NULL ) {
        XtSetArg(argt[i], XmNsensitive,                 False           ); i++;
    }
*/


    options_dt = XmCreateToggleButtonGadget(session_menu,
                                                 "options_dt",
                                                 argt, i);
    XmStringFree(xmstr);
    XtAddCallback(options_dt,
                  XmNvalueChangedCallback,
                  MenuItemCB,
                  (XtPointer) OB_DT);
    /*XtManageChild(options_dt);   */
    /*
     *  [ Dt Lite ] menu pane...
     */
    i = k;
    xmstr = ReadCatalog(MC_LABEL_SET, MC_DTLITE_LABEL, MC_DEF_DTLITE_LABEL);
    XtSetArg(argt[i], XmNlabelString,                   xmstr           ); i++;
    XtSetArg(argt[i], XmNrecomputeSize,                 True            ); i++;

    options_dtlite = XmCreateToggleButtonGadget(session_menu,
                                                 "options_dtlite",
                                                 argt, i);
    XmStringFree(xmstr);
    XtAddCallback(options_dtlite,
                  XmNvalueChangedCallback,
                  MenuItemCB,
                  (XtPointer) OB_DTLITE);


    }

if ( session_menu != NULL ) {
        /*
         *  [ Language ] menu pane...
         *  attach language cascade menu to this pane
         */
        i = InitArg(CascadeBG);
        xmstr = ReadCatalog(MC_LABEL_SET, MC_SES_LABEL, MC_DEF_SES_LABEL);
        XtSetArg(argt[i], XmNlabelString,               xmstr           ); i++;
        XtSetArg(argt[i], XmNsubMenuId,                 session_menu       ); i++;
        XtSetArg(argt[i], XmNrecomputeSize,             True            ); i++;
        options_item[j] = XmCreateCascadeButtonGadget(options_menu,
                                "session_menus", argt, i);
        XmStringFree(xmstr);
        j++;

    /*
     *  separator...
     */
    i = InitArg(SeparatorG);
    options_item[j] = XmCreateSeparatorGadget(options_menu, "options_sep1",
                                                argt, i);
    j++;

    }

    if ( getenv(PINGINTERVAL) == NULL ) {
      if(remote_host_menu == NULL)
        remote_host_menu = XmCreatePulldownMenu(options_menu,
                                "remote_host_menu", NULL, 0);
    }
 
      if ( remote_host_menu != NULL ) {
        i = InitArg(CascadeBG);
        xmstr = ReadCatalog(MC_LABEL_SET,MC_REMOTE_LABEL, MC_DEF_REMOTE_LABEL);
        XtSetArg(argt[i], XmNlabelString,              xmstr           );i++;
        XtSetArg(argt[i], XmNsubMenuId,                remote_host_menu);i++;
 
        XtSetArg(argt[i], XmNrecomputeSize,             True            ); i++;
        options_item[j] = XmCreateCascadeButtonGadget(options_menu,
                                "remote_host_menu", argt, i);
        XmStringFree(xmstr);
        j++;

        /*
         *  separator...
         */
        i = InitArg(SeparatorG);
        options_item[j] = XmCreateSeparatorGadget(options_menu, "options_sep2",
                                                argt, i);
        j++;
      }
 

#ifndef __apollo
    /*
     *  [ No Windows ] menu pane...
     */
    i = k = InitArg(PushBG);
    xmstr = ReadCatalog(MC_LABEL_SET, MC_NW_LABEL, MC_DEF_NW_LABEL);
    XtSetArg(argt[i], XmNlabelString,                   xmstr           ); i++;
    options_item[j] = options_nowindows
                      = XmCreatePushButtonGadget(options_menu,
                                                 "options_noWindows",
                                                 argt, i);
    XmStringFree(xmstr);
    XtAddCallback(options_item[j], XmNactivateCallback,
                  MenuItemCB, (XtPointer) OB_NO_WINDOWS);

    if (getenv(LOCATION) == NULL || strcmp(getenv(LOCATION), "local") != 0 )
        XtSetSensitive(options_item[j], False);
    j++;
#endif

  if ( remote_host_menu != NULL ) {
    i = InitArg(PushBG);
    xmstr = ReadCatalog(MC_LABEL_SET, 
		MC_REMOTE_HOST_LABEL, MC_DEF_REMOTE_HOST_LABEL);
    XtSetArg(argt[i], XmNlabelString,                  xmstr ); i++;
 
    remote_item[ri] = XtCreateWidget("remote_login_host",
                        xmPushButtonGadgetClass, remote_host_menu, argt, i);
 
    XmStringFree(xmstr);
    XtAddCallback(remote_item[ri], XmNactivateCallback,
                  MenuItemCB, (XtPointer) OB_REMOTE_HOST_LOGIN);
    ri++;
    i = InitArg(PushBG);
    xmstr = ReadCatalog(MC_LABEL_SET, 
		MC_CHOOSE_HOST_LABEL, MC_DEF_CHOOSE_HOST_LABEL);
    XtSetArg(argt[i], XmNlabelString,                  xmstr ); i++;
 
    remote_item[ri] = XtCreateWidget("choose_login_host",
                        xmPushButtonGadgetClass, remote_host_menu, argt, i);
    XmStringFree(xmstr);
    XtAddCallback(remote_item[ri], XmNactivateCallback,
                  MenuItemCB, (XtPointer) OB_REMOTE_CHOOSE_LOGIN);
 
      ri++;
    /*
     *  separator...
     */

     i = InitArg(SeparatorG);
     options_item[j] = XmCreateSeparatorGadget(options_menu, "options_sep2",
                                                argt, i);
     j++;
  }  

	
    /* 
     *  [ Restart Server ] menu pane...
     */
         i = k = InitArg(PushBG);

    if ( getenv(PINGINTERVAL) == NULL )
         xmstr = ReadCatalog(MC_LABEL_SET, MC_RS_LABEL, MC_DEF_RS_LABEL);
    else
         xmstr = ReadCatalog(MC_LABEL_SET, MC_RET_LOGIN_LABEL, 
	                                         MC_DEF_RET_LOGIN_LABEL);

         XtSetArg(argt[i], XmNlabelString,		xmstr		); i++;
         options_item[j] = XmCreatePushButtonGadget(options_menu,
    				 "options_restartServer", argt, i); 
         XmStringFree(xmstr);
         XtAddCallback(options_item[j], XmNactivateCallback, 
    		  MenuItemCB, (XtPointer) OB_RESTART_SERVER);
         j++;

#ifdef copyright_option
    /*
     *  separator...
     */
    i = InitArg(SeparatorG);
    options_item[j] = XmCreateSeparatorGadget(options_menu, "options_sep1",
                                                argt, i);
    j++;


    /* 
     *  [ Copyright ] menu pane...
     */
    i = k = InitArg(PushBG);
    xmstr = ReadCatalog(MC_LABEL_SET, MC_COPY_LABEL, MC_DEF_COPY_LABEL);
    XtSetArg(argt[i], XmNlabelString,			xmstr		); i++;
    options_item[j] = XmCreatePushButtonGadget(options_menu, 
    						 "options_copyright",
    						 argt, i);
    XmStringFree(xmstr);
    XtAddCallback(options_item[j], XmNactivateCallback, 
    		  MenuItemCB, (XtPointer) OB_COPYRIGHT);
    j++;
#endif

    /*
     *  manage the [Options] menu...
     */
    XtManageChildren(options_item, j);
    if(ri > 0)
    XtManageChildren(remote_item, ri);



    /*
     *  If the DT Lite Session Manager is not available, remove the DT Lite
     *  and DT menu panes. The actual widgets must still be created since
     *  other code (ex. MenuItemCB()) tries to obtain some of their resources.
     */
     
    if  ( stat(DTLITESESSION, &statb) != 0 ||
	  ((statb.st_mode & S_IXOTH) != S_IXOTH) ) {

	XtUnmanageChild(options_dtlite);
/*
	XtUnmanageChild(options_dt);
*/
    }

    if ( getenv(PINGINTERVAL) != NULL )
	XtUnmanageChild(options_nowindows);
	

}




/***************************************************************************
 *
 *  MyInsert
 *
 *  Local self-insert action for the text widget. The default action
 *  discards control characters, which are allowed in password.
 ***************************************************************************/

static void 
#ifdef _NO_PROTO
MyInsert( w, event, params, num_params )
        Widget w ;
        XEvent *event ;
        char **params ;
        Cardinal *num_params ;
#else
MyInsert( Widget w, XEvent *event, char **params, Cardinal *num_params )
#endif /* _NO_PROTO */
{
    char	   str[32];
    int 	   n;

    n = XLookupString((XKeyEvent *)event, str, sizeof(str),
                      (KeySym *)NULL, NULL);

    if (n > 0) {
       str[n] = '\0';
       XmTextFieldInsert(w, XmTextFieldGetCursorPosition(w), str);
    }
}

/***************************************************************************
 *
 *  MyBackspace
 *
 *  Local backspace action for the text widget. 
 *  Deletes the last character of the password string in the 
 *  widget for each backspace key press, and also does not move the cursor
 *  position in the widget.
 ***************************************************************************/

static void
#ifdef _NO_PROTO
MyBackspace( w, event, params, num_params )
        Widget w ;
        XEvent *event ;
        char **params ;
        Cardinal *num_params ;
#else
MyBackspace( Widget w, XEvent *event, char **params, Cardinal *num_params )
#endif /* _NO_PROTO */
{
  LoginTextPtr textdata;

  textdata = GetLoginTextPtr(w);

  if (textdata && !textdata->bEcho && strlen(textdata->noechobuf) > 0)
  { 
    textdata->noechobuf[strlen(textdata->noechobuf) - 1] = '\0';
  }
}


/***************************************************************************
 *
 *  ErrorHandler
 *
 *  X protocol error handler to override the default
 ***************************************************************************/

static int
#ifdef _NO_PROTO
ErrorHandler( dpy, event )
        Display *dpy ;
        XErrorEvent *event ;
#else
ErrorHandler( Display *dpy, XErrorEvent *event )
#endif /* _NO_PROTO */
{
    return 0;   /* non-fatal */
}




/***************************************************************************
 *
 *  xtErrorHandler
 *
 *  Xt protocol error handler to override the default
 ***************************************************************************/

static void 
#ifdef _NO_PROTO
xtErrorHandler( msg )
        String msg ;
#else
xtErrorHandler( String msg )
#endif /* _NO_PROTO */
{
    LogError((unsigned char *) "%s\n", msg ) ;
    exit(NOTIFY_RESTART);
}




/***************************************************************************
 *
 *  xtWarningHandler
 *
 *  Xt protocol error handler to override the default
 ***************************************************************************/

static void 
#ifdef _NO_PROTO
xtWarningHandler( msg )
        String msg ;
#else
xtWarningHandler( String msg )
#endif /* _NO_PROTO */
{
    LogError( (unsigned char *) "%s\n", msg ) ;
    return ;
}




/***************************************************************************
 *
 *  MakeOptionsProc
 *
 *  Timeout routine to build options menu
 ***************************************************************************/

static void 
#ifdef _NO_PROTO
MakeOptionsProc( data, id )
        XtPointer data ;
        XtIntervalId *id ;
#else
MakeOptionsProc( XtPointer data, XtIntervalId *id )
#endif /* _NO_PROTO */
{

    if (options_menu == NULL)
	MakeOptionsMenu();
    
    return;
}




/***************************************************************************
 *
 *  Terminate
 *
 *  Catch a SIGTERM and unmanage display
 ***************************************************************************/

static SIGVAL
#ifdef _NO_PROTO
Terminate()
#else
Terminate( int arg )
#endif /* _NO_PROTO */

{
    write(1, "terminate", 9);
    CleanupAndExit(NULL, NOTIFY_ABORT);
}




/***************************************************************************
 *
 *  DebugWidgetResources
 *
 *  Get widget resources
 ***************************************************************************/

typedef struct resource_values {
    int	height;
    int	width;
    int	x;
    int	y;
    int rightAttachment;
    int leftAttachment;
    int topAttachment;
    int bottomAttachment;
} ResourceValues;

static void
#ifdef _NO_PROTO
DebugWidgetResources(w)
    Widget	w;
#else
DebugWidgetResources(Widget w)
#endif /* _NO_PROTO */

{
    struct resource_values	values;
    int i;

    i = 0;
    bzero((char *) &values, sizeof(values));
    XtSetArg(argt[i], XmNheight,	&values.height); i++;
    XtSetArg(argt[i], XmNwidth,		&values.width); i++;
    XtSetArg(argt[i], XmNx,		&values.x); i++;
    XtSetArg(argt[i], XmNy,		&values.y); i++;
    XtSetArg(argt[i], XmNrightAttachment,	&values.rightAttachment); i++;
    XtSetArg(argt[i], XmNleftAttachment,	&values.leftAttachment); i++;
    XtSetArg(argt[i], XmNtopAttachment,		&values.topAttachment); i++;
    XtSetArg(argt[i], XmNbottomAttachment,	&values.bottomAttachment); i++;

    XtGetValues(w, argt, i);
}
