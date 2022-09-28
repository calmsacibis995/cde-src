/* $XConsortium: vg.h /main/cde1_maint/6 1995/10/24 15:12:20 mgreess $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        vg.h
 **
 **   Project:     HP Visual User Environment (DT)
 **
 **   Description: Main header file for Dtgreet application.
 **
 **                Defines, structure definitions, and external declarations
 **		   are specified here.
 **
 **
 **   (c) Copyright 1987, 1988, 1989 by Hewlett-Packard Company
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _VG_H
#define _VG_H

#ifndef _NO_PROTO
#if    !defined(__STDC__) && !defined(__cplusplus) && !defined(c_plusplus)
#define _NO_PROTO
#endif /* __STDC__ */
#endif /* _NO_PROTO */

#include	<sys/errno.h>
#include	<stdlib.h>
#include	<Xm/Xm.h>
#include	"vgproto.h"

#ifdef BLS
#    include <sys/security.h>
#endif


#ifdef SIGNALRETURNSINT
#define SIGVAL int
#else
#define SIGVAL void
#endif


/****************************************************************************
 *
 *  Defines
 *
 ****************************************************************************/


/*
 * SAIC DT copyright is in a dt directory
 */
#if defined( _SUN_OS )
#  define COPYRIGHT		"/usr/lib/X11/dt/etc/copyright"
#else
#  if defined( __hp_osf )
#     define COPYRIGHT		"/usr/X11/copyright"
#  else
#     define COPYRIGHT		"/etc/copyright"
#  endif
#endif

#define NO_LOGIN_FILE		"/etc/nologin"

#define DTLITESESSION		"/usr/dt/bin/dtlitesession"

#define LOGIN_STR_LEN 1024

/*
 *  option menu items...
 */

#define	OB_RESTART_SERVER	1
#define	OB_NO_WINDOWS		2
#define	OB_COPYRIGHT		3
#define	OB_FAILSAFE		4
#define	OB_LANGUAGE		5
#define	OB_DTLITE		6
#define	OB_DT			7
#define	OB_ALT_DTS		8
#define	OB_LAST_DT		10
#define OB_REMOTE_HOST_LOGIN    11
#define OB_REMOTE_CHOOSE_LOGIN  12
#define OB_RETURN_TO_LOGIN      13


 
#ifndef LAST_SESSION_FILE
#define LAST_SESSION_FILE "/.dt/sessions/lastsession"
#endif

#ifndef DEF_SESSION
#define DEF_SESSION CDE_INSTALLATION_TOP "/bin/Xsession"
#endif

#ifndef REMOTE_GREET_HOST_FILE
#define REMOTE_GREET_HOST_FILE "/var/dt/remote_greet_host"
#endif

/*
 *  default resources...  (Dimensions are in units of 100TH_MILLIMETER)
 *
 *  - width/height of main matte
 *  - percentage offsets of login matte within main matte
 *  - shadow thickness
 *  - margin height for message box buttons
 *  - fonts
 *  - colors
 */

#ifndef XmNPIXELS
#define XmNPIXELS		"PIXELS"
#endif

#ifndef XmN100TH_MILLIMETERS
#define XmN100TH_MILLIMETERS	"100TH_MILLIMETERS"
#endif

#define VnNmatteUnitType	"Dtlogin*matte.unitType"
#define VnNmatteWidth		"Dtlogin*matte.width"
#define VnNmatteHeight		"Dtlogin*matte.height"
#define VnCMatteUnitType	"Dtlogin*Form.UnitType"
#define VnCMatteWidth		"Dtlogin*Form.Width"
#define VnCMatteHeight		"Dtlogin*Form.Height"

#define MATTE_WIDTH		    18500
#define MATTE_HEIGHT		6746

#define HRES_MATTE_WIDTH	"835"		/* pixels		   */
#define HRES_MATTE_HEIGHT	"425"		/* pixels		   */
#define MRES_MATTE_WIDTH	"775"		/* pixels		   */
#define MRES_MATTE_HEIGHT	"400"		/* pixels		   */
#define LRES_MATTE_WIDTH	"585"		/* pixels		   */
#define LRES_MATTE_HEIGHT	"300"		/* pixels		   */

#define LOGIN_TOP_POSITION	"31"		/* percent		   */
#define LOGIN_BOTTOM_POSITION	"79"		/* percent		   */
#define LOGIN_LEFT_POSITION	"12"		/* percent		   */
#define LOGIN_RIGHT_POSITION	"88"		/* percent		   */

#define	SHADOW_THICKNESS   	FromMM(100)	/* general border size	   */
#define	ETCH_THICKNESS   	FromMM(150)	/* login etch thickness	   */
#define	MBOX_MARGIN_WIDTH 	FromMM(300)
#define	MBOX_MARGIN_HEIGHT 	FromMM(300)
#define	DROP_SHADOW_THICKNESS  	FromMM(400)	/* logo drop shadow size   */
/*#define	MIN_BUTTON_SIZE  	FromMM(3600)	 minimum button size	   */
#define	MIN_BUTTON_SIZE  	FromMM(3100)	/* minimum button size	   */
#define	TEXT_MARGIN_WIDTH 	FromMM(200)	/* text wgt margin width   */
#define	TEXT_MARGIN_HEIGHT 	FromMM(100)	/* text wgt margin height  */


#  define LABEL_FONT		"-*-swiss 742-bold-r-normal-*-140-*-p-100-*"
#  define LABEL_FONT_HRES	"-*-swiss 742-medium-r-normal-*-140-*-p-110-*"
#  define TEXT_FONT		"-*-prestige-medium-r-normal-*-128-72-*"
#  define TEXT_FONT_LRES	"-*-helvetica-bold-r-normal-*-100-*"
#  define GREET_FONT		"-*-*schoolbook-medium-i-normal--18-*"

#if 0
#  define GREET_FONT		"-*-*schoolbook-medium-i-normal--18-*-*-*-*-*-ISO8859-1"
#endif

#define TABLE_BACKGROUND	"black"
#define TEXT_FOREGROUND		"white"

#define BW_BACKGROUND		"white"
#define BW_FOREGROUND		"black"
#define BW_TEXT_BACKGROUND	"black"

#define BW_LOGO_BACKGROUND	"#a8a8a8"
#define BW_LOGO_TOPSHPIXMAP	"25_foreground"
#define BW_DROP_BACKGROUND	"black"
#define BW_DROP_FOREGROUND	"white"
#define BW_DROP_BACKPIXMAP	"25_foreground"

/*
 *  define colors by rgb value rather than name. On other systems the name
 *  may map to a different shade of the desired color.
 *
 *  on hp-ux...
 *
 *  #a8a8a8  =  lightgray
 *  #7e7e7e  =  gray
 *  #ef506f  =  burgundy
 *
 */
#define CO_BACKGROUND		"#a8a8a8"
#define CO_FOREGROUND		"black"
#define CO_TEXT_BACKGROUND	"#7e7e7e"
#define CO_HIGHLIGHT		"#ef506f"
#define CO_SELECTCOLOR		"#ef506f"

#define CO_LOGO_BACKGROUND	"#ef506f"
#define CO_LOGO_TOPSHPIXMAP	"25_foreground"
#define CO_DROP_BACKGROUND	"#7e7e7e"
#define CO_DROP_FOREGROUND	"#7e7e7e"
#define CO_DROP_BACKPIXMAP	"background"


/*
 *  macros to use when differentiating on resolution...
 */

#define HIRES	(dpyinfo.width >  1200)		/* SUN MED_RES = 1182  */
#define MEDRES	(dpyinfo.width >   800 && dpyinfo.width <= 1200)
#define LOWRES	(dpyinfo.width <=  800)


/*
 *  macro to determine authentication mechanism name...
 */
 
#define vg_IsVerifyName(n) \
    ((strcmp(getenv(VERIFYNAME),(n)) == 0 ) ? TRUE : FALSE)

/****************************************************************************
 *
 *  Structure definitions
 *
 ****************************************************************************/

typedef enum widgetType { CascadeBG, DrawingA, Frame, Form, MessageBox,
			  Label, LabelG, PushB, PushBG, SeparatorG,
			  ToggleB, ToggleBG, Text }
	     WidgetType;


typedef enum dialogType { error, help, copyright, hostname, expassword,
			  help_chooser, bad_hostname, conn_to_host,
			  host_not_runlogin }
	     DialogType;


typedef struct {
    Boolean    	workspaceCursor;/* whether to define a root window cursor  */
    XmFontList	labelFont;	/* font for labels and buttons		   */
    XmFontList	textFont;	/* font for dialog text			   */
    XmFontList	chlistFont;	/* font for chooser list and heading       */
    int		optionsDelay;	/* seconds to delay before building menu   */
    int         altDts;         /* number of alternate desktops            */
    char 	*languageList;	/* list of languages to display in menu	   */
    int 	unitType;	/* widgets' unit type			   */
    char        *languageListCmd; /* command to produce language list      */
} AppInfo, *AppInfoPtr;



typedef struct displayInfo {
    Display	*dpy;		/* initialized with XtDisplay()		   */
    char	*name;		/* ASCII name of the display being used	   */
    int		screen;		/* initialized with DefaultScreen()	   */
    Window	root;		/* initialized with RootWindow()	   */
    int		depth;		/* initialized with DefaultDepth()	   */
    int		width;		/* initialized with DisplayWidth()	   */
    int		height;		/* initialized with DisplayHeight()	   */
    Pixel	black_pixel;	/* initialized with BlackPixel()	   */
    Visual	*visual;	/* initialized with DefaultVisual()	   */
} DisplayInfo;



typedef  struct {
    char	*bitmapFile;	/* bitmap file (if any)			   */
} LogoInfo, *LogoInfoPtr;

typedef struct {
  int bEcho;                    /* True=echoOn, False=echoOff              */
  Widget text[2];               /* [0] is echoOff, [1] is echoOn           */ 
  String onGreeting;            /* echoOn greeting                         */
  char *offGreetingFormat;      /* printf format for echoOff greeting      */
  char *offGreetingUname;       /* insert for echoOff greeting             */
  char noechobuf[LOGIN_STR_LEN+1]; /* alternate buffer for echoOff         */
} LoginText, *LoginTextPtr;


/****************************************************************************
 *
 *  External variable declarations
 *
 ****************************************************************************/

extern Arg	argt[];		/* used for resources			   */
extern DisplayInfo dpyinfo;	/* information about the display	   */
extern int	errno;		/* system call error value		   */
extern char	*errorLogFile;  /* name of error log file (if any)	   */
extern char	*langenv;       /* current value of LANG environment var.  */
extern XmString	xmstr;          /* used for compound strings               */
extern int	showVerifyError;/* display a Verify() error dialog	   */
extern int	amChooser;      /* 1 if chooser process, 0 if not 	   */

extern AppInfo	appInfo;	/* application resources		   */

extern Widget toplevel;		/* top level shell widget		   */
extern Widget login_shell;	/* shell for the main login widgets.	   */
extern Widget table;		/* black background for everything	   */
extern Widget matte;		/* main level form widget		   */
extern Widget matteFrame;	/* main level form widget		   */
extern Widget matte1;		/* main level form widget		   */

extern Widget greeting;		/* Welcome message			   */
extern Widget dt_label;		/* Desktop label set in options menu       */

extern Widget logo1;		/* frame around the Corporate logo	   */
extern Widget logo_pixmap;	/* Corporate logo			   */
extern Widget logo_shadow;	/* drop shadow under the Corporate logo	   */

extern Widget login_matte;	/* bulletin board for login/password	   */
extern Widget login_form;	/* form containing the login extern Widgets	   */
extern Widget login_label;	/* label to left of login text widget	   */
extern Widget login_text;	/* login text widget			   */
extern Widget passwd_form;	/* form containing the password widgets	   */
extern Widget passwd_label;	/* label to left of password text widget   */
extern Widget passwd_text;	/* password text widget			   */

extern Widget ok_button;	/* accept name/password text button	   */
extern Widget clear_button;	/* clear name/password text button	   */
extern Widget options_button;	/* login options button			   */
extern Widget help_button;	/* help button				   */

extern Widget copyright_msg;	/* copyright notice widget		   */
extern Widget help_message;	/* the help message box			   */
extern Widget error_message;	/* the error message box		   */
extern Widget hostname_message;	/* the invalid hostname message box	   */
extern Widget bad_host;         /* bad remote hostname                     */
extern Widget host_not_run_login;/* remote host not running message box    */
extern Widget connect_to_host; /* connecting to remote host message box   */

extern Widget remote_host_dialog;
extern Widget passwd_message;	/* the password expired message box	   */


extern Widget options_menu;	/* pop-up menu on options button	   */
extern Widget options_item[];	/* items on options pop_up menu	  	   */
extern Widget options_failsafe; /* failsafe pane on options pop_up menu    */
extern Widget options_dtlite;   /* dtlite  pane on options pop_up menu    */
extern Widget options_dt;	/* dt pane on options pop_up menu	   */
extern Widget *alt_dts;         /* alt_dts  widgets on options pop_up menu */
extern Widget options_last_dt;	/* toggle button for user's last dt 	   */
extern Widget options_sp_dt;	/* toggle for user's special dt            */


extern Widget lang_menu;	/* cascading menu on "Language" option	   */
extern LogoInfo logoInfo;       /* logo resources                          */





/****************************************************************************
 *
 *  Macro definitions
 *
 ****************************************************************************/
#define vg_TRACE_EXECUTION(str)	\
LogError((unsigned char *)"VG_TRACE:  %s\n", str)


/****************************************************************************
 *
 *  External procedure declarations
 *
 ****************************************************************************/
#ifdef _NO_PROTO

extern void ChangeBell();	/* change state of system bell		   */
extern void CleanupAndExit();	/* shutdown windows and exit		   */
extern void CopyrightCB();	/* Copyright dialog call back		   */
extern void EditPasswdCB();	/* edit the password to turn off echo	   */
extern void FakeFocusIn();	/* fake a focus in event	   	   */
extern int  FromMM();		/* convert from 100thMM to pixels	   */
extern int  FromPixel();	/* convert from pixels to widget units     */
extern void GetBiggest();	/* utility function to determine size	   */
extern LoginTextPtr GetLoginTextPtr();	/* get login text instance data    */
extern int  InitArg();		/* initialize the widget arg list	   */
extern void LayoutCB();		/* final layout adjustments	 	   */
extern void LogError();		/* write messages to the error log	   */
extern void MakeDialog();	/* make various dialogs			   */
extern void MakeHostDialog();   /* make hostname dialog                    */
extern void MakeLangMenu();	/* make language menu 			   */
extern void MakeLogo();		/* make Corporate logo			   */
extern void MakeOptionsMenu();	/* make "Options" pop-up menu		   */
extern void MenuItemCB();	/* callback for menu items		   */
extern void OptionsUnmapCB();	/* callback for Options menu unmap	   */
extern void PingServerCB();	/* ping server callback			   */
extern void PostMenuCB();	/* post options menu callback		   */
extern XmString ReadCatalog();	/* get a string from the message catalog   */
extern void RefreshEH();	/* refresh the entire display		   */
extern void RequestCB();	/* request callback  		           */
extern void RespondChallengeCB();/* respond callback  		           */
extern void RespondClearCB();	/* callback for clear button		   */
extern void RespondDialogCB();  /* respond callback                        */
extern void RespondExitCB();	/* respond callback  		           */
extern void RespondLangCB();	/* respond callback  		           */
extern int  SecureDisplay();	/* grab the server and keyboard	   	   */
extern void SetResourceDatabase();
				/* set up resource database		   */
extern void ShowDialogCB();	/* callback for displaying dialogs	   */
extern void TextFocusCB();	/* text widget has accepted input focus	   */
extern int  ToPixel();		/* convert from widget units to pixels	   */
extern void UnsecureDisplay();	/* release the server and keyboard	   */
extern void SetDtLabelAndIcon();	/* display the dt selected in options menu */
#ifndef _DM_H
extern int  Verify();		/* entry point to validation policies	   */
#endif

#else

extern void ChangeBell( char *string) ;
extern void CleanupAndExit( Widget w, int exit_code) ;
extern void ClearCB( Widget w, XtPointer client_data, XtPointer call_data) ;
extern void CopyrightCB( Widget w, XtPointer client_data, XtPointer call_data) ;
extern void EditPasswdCB( Widget w, XtPointer client, XtPointer call_data) ;
extern void FakeFocusIn( Widget focus_widget, XtPointer client_data,
				   XEvent *eventprm, 
				   Boolean *continue_to_dispatch) ;
extern int  FromMM( int mm ) ;
extern int  FromPixel( Widget w, int orientation, int pixel) ;
extern void GetBiggest( Widget widget, Dimension *width, Dimension *height) ;
extern LoginTextPtr GetLoginTextPtr( Widget w );
extern int  InitArg( WidgetType wtype) ;
extern void LangItemCB(	Widget w, XtPointer client_data, XtPointer call_data) ;
extern void LayoutCB( Widget w, XtPointer client_data, XtPointer call_data) ;
extern void LogError( unsigned char *fmt, ... );
extern void MakeDialog( DialogType dtype) ;
extern void MakeHostDialog( void );
extern void MakeLangMenu( void ) ;
extern void MakeLogo( void ) ;
extern void MakeOptionsMenu( void ) ;
extern void MenuItemCB( Widget w, XtPointer client_data, XtPointer call_data) ;
extern void OptionsUnmapCB( Widget wd, XtPointer client_data, XtPointer call_data) ;
extern void PingServerCB( XtPointer call_data, XtIntervalId *id) ;
extern void PostMenuCB( Widget w, XtPointer client_data, XtPointer call_data);
extern XmString ReadCatalog( int set_num, int msg_num, char *def_str) ;
extern void RefreshEH( Widget w, XtPointer client_data, XEvent *event,
				   Boolean *continue_to_dispatch) ;
extern void RequestCB( XtPointer client_data, int *source, XtInputId *id);
extern void RespondExitCB( Widget w, XtPointer client, XtPointer call);
extern void RespondLangCB( Widget w, XtPointer client, XtPointer call);
extern void RespondChallengeCB(Widget w, XtPointer client, XtPointer call);
extern void RespondClearCB(Widget w, XtPointer client, XtPointer call);
extern void RespondDialogCB(Widget w, XtPointer client, XtPointer reason);
extern int  SecureDisplay( void ) ;
extern void SetResourceDatabase( void ) ;
extern void ShowDialogCB( Widget w, XtPointer dialog, XtPointer call_data) ;
extern void TextFocusCB( Widget w, XtPointer client_data, XtPointer call_data) ;
extern int  ToPixel( Widget w, int orientation, int pixel) ;
extern void UnsecureDisplay( void ) ;
extern void SetDtLabelAndIcon( void );	
#ifndef _DM_H
extern int  Verify( char *name, char *passwd) ;
#endif

#endif /* _NO_PROTO */


#endif /* _VG_H */
