/* $XConsortium: vgcallback.c /main/cde1_maint/8 1995/10/24 15:12:38 mgreess $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        vgcallback.c
 **
 **   Project:     HP Visual User Environment (DT)
 **
 **   Description: Callback routines Dtgreet application.
 **
 **                These routines handle the callbacks from the widgets.
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
#include	<unistd.h>
#include	<setjmp.h>
#include	<signal.h>
#include	<sys/param.h>

#include	<Xm/Xm.h>
#include	<Xm/MessageB.h>
#include	<Xm/TextF.h>
#include	<Xm/TextFP.h>
#include	<Xm/PushBG.h>
#include	<Xm/ToggleBG.h>
#include	<Xm/SelectioB.h>
#include    <Dt/IconFile.h>
#include    <Dt/Icon.h>
#include	<pwd.h>

#include	<sys/select.h>
#include	"dm.h"
#include	<sys/stat.h>


#ifdef AUDIT
#    include <sys/audit.h>
#endif

/* necessary for bzero */
#ifdef SVR4
#include        <X11/Xfuncs.h>
#endif

#include	"vg.h"
#include	"vgmsg.h"

/* necessary for xdmcp */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
 
#include <X11/Xos.h>
#include <X11/X.h>
#include <X11/Xmd.h>
 
#include <X11/Xdmcp.h>
#include <X11/Xauth.h>
#include <time.h>
#include <sys/time.h>



/***************************************************************************
 *
 *  External declarations
 *
 ***************************************************************************/

extern  LogoInfo	logoInfo; /* information about the logo		   */



/***************************************************************************
 *
 *  Procedure declarations
 *
 ***************************************************************************/
#ifdef _NO_PROTO

static  void CenterForm();	/* center one form within another horiz.   */
static  void PingLost() ;	/* contact lost with remote server	   */
static  void PingBlocked() ;	/* contact to remote server blocked	   */
static	void ProcessTraversal();/* move the input focus			   */
static	void _DtShowDialog();	/* display a dialog box			   */
void 	SetDtLabelAndIcon();		
static	void TellRequester();
void    StoreHostname();
static  int connect_to_remote_host();/*check if remote host is running dtlogin*/
void 	ShowStatus();		


#  ifdef BLS
static	void PromptSensitivityLevel();	/* prompt for B1 Sensitivity Level */
	int  VerifySensitivityLevel();	/* verify B1 Sensitivity Level	   */
#  endif

#else

static void CenterForm( Widget w1, Widget w2);
static void PingLost( void ) ;
static SIGVAL PingBlocked( int arg ) ;
static void ProcessTraversal( Widget w, int direction) ;
static void _DtShowDialog(DialogType dtype, XmString msg);
void 	SetDtLabelAndIcon( void );		
static void TellRequester(char * buf, size_t nbytes);
void    StoreHostname(char *query_hostname);
static  int connect_to_remote_host(char *value);/*check if remote host 
							is running dtlogin*/


#  ifdef BLS
static	void PromptSensitivityLevel(void); /* prompt for B1 Sen. Level	   */
	int  VerifySensitivityLevel(void); /* verify B1 Sensitivity Level  */
#  endif

#endif /* _NO_PROTO */



/***************************************************************************
 *
 *  Global variables
 *
 ***************************************************************************/

       Widget	focusWidget = NULL;

#define LOGIN_STR_LEN 1024

char	*userName = "\0";
struct passwd *user_p;

static remote_host_cb_sensitive;


#ifdef BLS
static  int	normalPasswordWidget = True;
	char	*sensitivityLevel = NULL;
#endif

#ifndef SVR4
	long	groups[NGROUPS];
#endif

#ifdef SIA
  
#include <alloca.h>

SiaFormInfo siaFormInfo;


XmString multiline_xmstring(char *text)
{

    char *start, *end;
    Boolean done;
    XmString string = NULL;
    XmString tmp_string;
    XmString separator = NULL;
    char *buffer;

    if (!text) return (NULL);


    buffer = alloca(strlen((const char *)text) + 1);

    start = text;
    done = FALSE;


    while ( ! done)           /* loop thu local copy */
      {                               /* looking for \n */
      end = start;

      while ((*end != '\0') && (*end != '\n'))  end++;

      if (*end == '\0')
          done = TRUE;                /* we are at the end */

      /* Don't convert empty string unless it's an initial newline. */
      if ((start != end) || (start == text))
          {
          strncpy(buffer, start, end - start);
          buffer[end - start] = '\0';
          if (!string)
              string = XmStringCreate(buffer, XmFONTLIST_DEFAULT_TAG);
          else
              {
              tmp_string = XmStringCreate(buffer, XmFONTLIST_DEFAULT_TAG);
              string = XmStringConcat(string, tmp_string);
              XmStringFree(tmp_string);
              }
          }

      /* Make a separator if this isn't the last segment. */
      if (!done) 
          {
          if (!separator)
              separator = XmStringSeparatorCreate();
          string = XmStringConcat(string, separator);
          start = ++end;              /* start at next char */
          }
      }

    if (separator)
      XmStringFree(separator);

    return (string);
}

#endif /* SIA */


/***************************************************************************
 *
 *  CenterForm
 *
 *  Utility function to center one form horizontally within another.
 ***************************************************************************/

static void 
#ifdef _NO_PROTO
CenterForm( w1, w2)
        Widget w1 ;
        Widget w2 ;
#else
CenterForm( Widget w1, Widget w2 )
#endif /* _NO_PROTO */
{

    Dimension	width;
    int		i, width1, width2;
   
    XtSetArg(argt[0], XmNwidth,  &width);
    XtGetValues(w1, argt, 1);
    width1 = (int)width;    

    XtSetArg(argt[0], XmNwidth,  &width);
    XtGetValues(w2, argt, 1);
    width2 = (int)width;
    
    i = 0;
    XtSetArg(argt[i], XmNleftAttachment,	XmATTACH_FORM		); i++;
    XtSetArg(argt[i], XmNleftOffset,		(width1 - width2) / 2	); i++;
    XtSetValues(w2,  argt, i);
}



 
/***************************************************************************
 *
 *  CleanupAndExit
 *
 *  close things down gracefully and exit
 ***************************************************************************/

void 
#ifdef _NO_PROTO
CleanupAndExit( w, exit_code )
        Widget w;
        int exit_code ;
#else
CleanupAndExit( Widget w, int exit_code )
#endif /* _NO_PROTO */
{
    int i;
    Boolean  toggleon;		/* status of session toggle buttons     */
    
#ifdef VG_TRACE
    vg_TRACE_EXECUTION("main:  entered CleanupAndExit ...");
#endif /* VG_TRACE */

    if (w != NULL)
      XtDestroyWidget(w);
    
    /*
     *  if user is logging in, set type of session desired. No more than
     *  one session type can be selected at a time (if any) ...
     */

    if ( exit_code == NOTIFY_OK ) {
	XtSetArg(argt[0], XmNset,	&toggleon		);

	XtGetValues(options_failsafe, argt, 1);
	if ( toggleon ) exit_code = NOTIFY_FAILSAFE;

	XtGetValues(options_dtlite, argt, 1);
	if ( toggleon ) exit_code = NOTIFY_DTLITE;


	XtGetValues(options_dt, argt, 1);
	if ( toggleon ) exit_code = NOTIFY_DT;


	if(options_last_dt != NULL) {
	XtGetValues(options_last_dt, argt, 1);
	if ( toggleon ) exit_code = NOTIFY_LAST_DT;
	}

        for(i = 0; i < appInfo.altDts ; ++i) {
	  if(alt_dts[i] != NULL) {
           XtGetValues(alt_dts[i], argt, 1);
	      if ( toggleon ) exit_code = NOTIFY_ALT_DTS + i + 1; /* alt desktops start  at 1 */ 
	   }	

	}

    }
    catclose(nl_fd);

    ChangeBell("on");
    UnsecureDisplay();
    XSync (dpyinfo.dpy, 0);

    XtCloseDisplay(dpyinfo.dpy);
    exit (exit_code);
}


 
/***************************************************************************
 *
 *  RespondClearCB
 *
 *  clear name/password text fields
 ***************************************************************************/

void 
#ifdef _NO_PROTO
RespondClearCB( w, client, call )
        Widget w ;
        int client ;
        XtPointer call ;
#else
RespondClearCB( Widget w, XtPointer client, XtPointer call )
#endif /* _NO_PROTO */
{
  char buf[REQUEST_LIM_MAXLEN];
  ResponseClear *r = (ResponseClear *)buf;

#ifdef VG_TRACE
  vg_TRACE_EXECUTION("main:  entered RespondClearCB ...");
#endif /* VG_TRACE */

  r->hdr.opcode = REQUEST_OP_CLEAR;
  r->hdr.reserved = 0;
  r->hdr.length = sizeof(*r);

  TellRequester(buf, (size_t) r->hdr.length);
}


 
/***************************************************************************
 *
 *  CopyrightCB
 *
 *  move the highlight back to login or password fields AFTER the copyright
 *  dialog is unposted.
 *
 ***************************************************************************/

void 
#ifdef _NO_PROTO
CopyrightCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
CopyrightCB( Widget w, XtPointer client_data, XtPointer call_data )
#endif /* _NO_PROTO */
{

#ifdef VG_TRACE
    vg_TRACE_EXECUTION("main:  entered CopyrightCB ...");
#endif /* VG_TRACE */

    if ( focusWidget != NULL)
	ProcessTraversal(focusWidget, XmTRAVERSE_CURRENT);

}



/***************************************************************************
 *
 *  EditPasswdCB
 *
 *  implement no-echo and no-cursor motion of the password
 ***************************************************************************/


void 
#ifdef _NO_PROTO
EditPasswdCB( w, client, call_data )
        Widget w ;
        XtPointer client ;
        XmTextVerifyPtr call_data ;
#else
EditPasswdCB( Widget w, XtPointer client, XtPointer call_data )
#endif /* _NO_PROTO */
{
  LoginTextPtr textdata;
  XmTextVerifyPtr	call;

#ifdef VG_TRACE
  vg_TRACE_EXECUTION("main:  entered EditPasswdCB ...");
#endif /* VG_TRACE */

  textdata = GetLoginTextPtr(w);

  if (textdata && !textdata->bEcho)
  {
    call = (XmTextVerifyPtr) call_data;
    if(call->reason == XmCR_MOVING_INSERT_CURSOR)
    {
      call->doit=False;
      return;
    }

    if (call->text->ptr)
    {
      if (strlen(textdata->noechobuf) + call->text->length <=
          LOGIN_STR_LEN - 5 )
      {
        strcat(textdata->noechobuf, call->text->ptr);
      }

     /*
      * Set insert string to zero-len rather than simply vetoing
      * to work aroung XmTextField bug that causes cursor to
      * disappear (widget 'cursor_on' field goes way negative).
      */
      call->text->ptr[0] = '\0';
    }
  }
}


/***************************************************************************
 *
 *  FakeFocusIn
 *
 *  simulate a FocusIn event to the login_shell in order to turn on
 *  traversal. There is a bug in the Toolkit that is normally masked by 
 *  the window manager. Since we have no window manager, we need to simulate
 *  the FocusIn event it sends to the application.
 *
 *  Also force the initial focus to the login_text widget.
 ***************************************************************************/

void
#ifdef _NO_PROTO
FakeFocusIn( focus_widget, client_data, eventprm, continue_to_dispatch )
        Widget focus_widget ;
        XtPointer client_data ;
        XEvent *eventprm ;
	Boolean *continue_to_dispatch ;
#else
FakeFocusIn( Widget focus_widget, XtPointer client_data, XEvent *eventprm,
	     Boolean *continue_to_dispatch )
#endif /* _NO_PROTO */
{
   XEvent event;
   XEvent * eventPtr = &event;

#ifdef VG_TRACE
   vg_TRACE_EXECUTION("main:  entered FakeFocusIn ...");
#endif /* VG_TRACE */

   /*
    * set the input focus to the login text widget...
    */
    
   XSetInputFocus( XtDisplay(focus_widget),
   		   XtWindow(focus_widget),
		   RevertToNone,
		   CurrentTime);

   /*
    *  create a synthetic focus-in event. 
    *
    *  Note: The above call to XSetInputFocus() was not originally included
    *        in this routine. A bug fix to Motif made it necessary to add
    *        the call. The synthetic focus-in event is probably now
    *        unnecessary but is left in for caution's sake. (12/08/92)
    */
    
   /* focus_widget = login_shell; */
   
   eventPtr->type = FocusIn;
   eventPtr->xfocus.serial = LastKnownRequestProcessed(XtDisplay(focus_widget));
   eventPtr->xfocus.send_event = True;
   eventPtr->xfocus.display = XtDisplay(focus_widget);
   eventPtr->xfocus.window = XtWindow(focus_widget);
   eventPtr->xfocus.mode = NotifyNormal;
   eventPtr->xfocus.detail = NotifyAncestor;

   XtDispatchEvent (eventPtr);

   ProcessTraversal(focus_widget, XmTRAVERSE_CURRENT);
   
   XtRemoveEventHandler(focus_widget, ExposureMask, FALSE,
   		   FakeFocusIn, NULL);
}

/***************************************************************************
 *
 *  LayoutCB
 *
 *  do final layout adjustments right before windows are mapped. This is
 *  necessary because the size of managers (Forms, etc) is not known until
 *  their window has been created (XtRealize). We want to make adjustments
 *  before the windows become visible
 *
 *  1. squeeze dialog width to fit on screen.
 *  2. increase dialog height if widgets overlap.
 *  3. center the main matte horizontally and vertically
 *  4. position the pushbuttons
 *  5. position the copyright
 *
 ***************************************************************************/

void
#ifdef _NO_PROTO
LayoutCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
LayoutCB( Widget w, XtPointer client_data, XtPointer call_data )
#endif /* _NO_PROTO */
{
    register int	i, j;
    Dimension	width, height;	/* size values returned by XtGetValues	   */
    Dimension	shadowThickness;/* size values returned by XtGetValues	   */
    Position	x, y;		/* position values returned by XtGetValues */
    
    struct {			/* position, size of widgets (pixels)	   */
    int x, y;
    int	width;
    int height;
    }		mw, pw; /* matte, logo, drop shadow, login matte
    				       and greeting widgets		   */

    int		width1, width2; /* general width variable		   */
    int		height1;	/* general height variable		   */
    Position	x1, y1;		/* general position variables		   */
    int		offsety;	/* general offset variable		   */
    int		shadow_offsetx; /* offset for drop shadow (pixels)	   */
    int		shadow_offsety; /* offset for drop shadow (pixels)	   */
    int		spacing;	/* spacing between login & matte bottoms   */
    
    Widget	buttons[4];	/* pushbutton widgets			   */

    XtWidgetGeometry  geometry;	/* geometry of a widget			   */
    
    int		max_width;	/* maximum width  of a set of widgets	   */
    int		origin;		/* horizontal origin for button placement  */
    int		space;		/* total available space left between buttons */
    int         overlap;        /* possible widget overlap                 */

#ifdef VG_TRACE
    vg_TRACE_EXECUTION("main:  entered LayoutCB ...");
#endif /* VG_TRACE */

    /*
     * - squeeze dialog to fit onto screen (if necessary)
     */
    i = 0;
    XtSetArg(argt[i], XmNwidth,                 &width                  ); i++;
    XtGetValues(matte, argt, i);
    mw.width  = ToPixel(matte, XmHORIZONTAL, (int)width  );
#define HMARGIN 4 /* min sum horizontal margin of matte */
    if (mw.width+HMARGIN > dpyinfo.width)
    {
      int delta = mw.width + HMARGIN - dpyinfo.width;
     /*
      * Matte width greater than screen so shrink matteFrame
      * and matte1 width to compensate.
      */
      i=0;
      XtSetArg(argt[i], XmNwidth,       &width          ); i++;
      XtGetValues(matteFrame, argt, i);

      width1 = ToPixel(matteFrame, XmHORIZONTAL, (int)width  );
      width1 -= delta;
      width1 = FromPixel(matteFrame, XmHORIZONTAL, width1 );

      i=0;
      XtSetArg(argt[i], XmNwidth,       width1          ); i++;
      XtSetValues(matteFrame, argt, i);

      width1 = dpyinfo.width - HMARGIN;
      mw.width = FromPixel(matte, XmHORIZONTAL, width1 );

      i=0;
      XtSetArg(argt[i], XmNwidth,       mw.width        ); i++;
      XtSetValues(matte, argt, i);
    }

    /*
     * - Make sure the login widgets don't overlap.
     */
    if (login_form) {
	i = 0;
	XtSetArg(argt[i], XmNy,			&y			); i++;
	XtSetArg(argt[i], XmNheight, 		&height			); i++;
	XtGetValues(greeting, argt, i);

	i = 0;
	XtSetArg(argt[i], XmNy,			&y1			); i++;
	XtGetValues(login_form, argt, i);

	overlap = y + height - y1;

	if (overlap > -10) {
            i = 0;
            XtSetArg(argt[i], XmNbottomAttachment, XmATTACH_WIDGET); 	i++;
            XtSetArg(argt[i], XmNbottomWidget,	   login_form); 	i++;
            XtSetArg(argt[i], XmNbottomOffset,	   10);			i++;
            XtSetValues(greeting, argt, i);
	}
    }
 

    /*
     *  - center the main matte horizontally and vertically...
     */
     
    i = 0;
    XtSetArg(argt[i], XmNx,			&x			); i++;
    XtSetArg(argt[i], XmNy,			&y			); i++;
    XtSetArg(argt[i], XmNwidth,			&width			); i++;
    XtSetArg(argt[i], XmNheight,		&height			); i++;
    XtSetArg(argt[i], XmNshadowThickness,	&shadowThickness	); i++;
    XtGetValues(matte, argt, i);

    mw.width  = ToPixel(matte, XmHORIZONTAL, (int)width  );
    mw.height = ToPixel(matte, XmVERTICAL,   (int)height );

    mw.x = ( x > 0 ? ToPixel(matte, XmHORIZONTAL, (int) x)
	    : (dpyinfo.width - mw.width)/2 );
    
    mw.y = ( y > 0 ? ToPixel(matte, XmVERTICAL, (int) y)
	    : (dpyinfo.height - mw.height)/2 );
 
    if ( mw.x < 0 ) mw.x = 0;
    if ( mw.y < 0 ) mw.y = 0;

    x1 = FromPixel(matte, XmHORIZONTAL, mw.x );
    y1 = FromPixel(matte, XmVERTICAL,   mw.y );

    i = 0;
    XtSetArg(argt[i], XmNx,			x1			); i++;
    XtSetArg(argt[i], XmNy,			y1			); i++;
    XtSetValues(matte, argt, i);


    /*
     * space the buttons horizontally. Start at the center of the matte
     * and allow them to grow towards the edges...
     */

    i = 0;
    XtSetArg(argt[i], XmNwidth,		&width				); i++;
    XtGetValues(matte1, argt, i);

    max_width = width;

    i = 0;
    XtSetArg(argt[i], XmNwidth,		&width				); i++;
    XtGetValues(clear_button, argt, i);
    
    space = max_width - 4*width;
    spacing = space/4;
    
    if (spacing < 12) spacing = 12;

    i = 0;
    XtSetArg(argt[i], XmNleftAttachment, XmATTACH_FORM); i++;
    XtSetArg(argt[i], XmNleftOffset, spacing/2); i++;
    XtSetValues(ok_button,  argt, i);

    i = 0;
    XtSetArg(argt[i], XmNleftAttachment,	XmATTACH_WIDGET		); i++;
    XtSetArg(argt[i], XmNleftWidget, ok_button); i++;
    XtSetArg(argt[i], XmNleftOffset, spacing); i++;
    XtSetValues(clear_button,  argt, i);

    i = 0;
    XtSetArg(argt[i], XmNleftAttachment,	XmATTACH_WIDGET		); i++;
    XtSetArg(argt[i], XmNleftWidget, clear_button); i++;
    XtSetArg(argt[i], XmNleftOffset, spacing); i++;
    XtSetValues(options_button,  argt, i);
    
    i = 0;
    XtSetArg(argt[i], XmNleftAttachment,	XmATTACH_WIDGET		); i++;
    XtSetArg(argt[i], XmNleftWidget, options_button); i++;
    XtSetArg(argt[i], XmNleftOffset, spacing); i++;
    XtSetValues(help_button,  argt, i);
    
    /*
     *  - adjust the copyright vertically to align top with login_matte...
     */

    if (copyright_msg) {
	XtQueryGeometry(copyright_msg, NULL, &geometry);
    
	i = 0;
	XtSetArg(argt[i], XmNshadowThickness,	&width			); i++;
	XtGetValues(copyright_msg, argt, i);

	width1 = ToPixel(copyright_msg, XmHORIZONTAL, width);
	width1 = (dpyinfo.width - (int) geometry.width - 2 * width1)/2;

	x1 = FromPixel(copyright_msg, XmHORIZONTAL, width1);
	y1 = FromPixel(copyright_msg, XmVERTICAL, mw.y);

	i = 0;
	XtSetArg(argt[i], XmNdefaultPosition,	False			); i++;
	XtSetArg(argt[i], XmNx,			x1			); i++;
	XtSetArg(argt[i], XmNy,			y1			); i++;
	XtSetValues(copyright_msg, argt, i);
    }
}




/***************************************************************************
 *
 *  MenuItemCB
 *
 *  callback for options menu items
 ***************************************************************************/

void 
#ifdef _NO_PROTO
MenuItemCB( w, client_data, call_data )
        Widget w ;
        int client_data ;
        XtPointer call_data ;
#else
MenuItemCB( Widget w, XtPointer client_data, XtPointer call_data )
#endif /* _NO_PROTO */
{
	int i;
    	char     *logoFile;
    	char     *logoName;
	char 	*temp_p;
	char 	temp[MAXPATHLEN];

#ifdef VG_TRACE
    vg_TRACE_EXECUTION("main:  entered MenuItemCB ...");
#endif /* VG_TRACE */

    switch ( (int) client_data) {

    case OB_RESTART_SERVER:
	CleanupAndExit(NULL, NOTIFY_RESTART);
	break;

    case OB_NO_WINDOWS:
	CleanupAndExit(NULL, NOTIFY_NO_WINDOWS);
	break;

     case OB_REMOTE_CHOOSE_LOGIN:
         CleanupAndExit(NULL, NOTIFY_REMOTE_CHOOSE_LOGIN);
         break;

     case OB_REMOTE_HOST_LOGIN:
         MakeHostDialog();
         break;

    case OB_COPYRIGHT:
	_DtShowDialog(copyright, NULL);
	break;

    case OB_ALT_DTS:
    case OB_FAILSAFE:
    case OB_DTLITE:
    case OB_DT: 
    case OB_LAST_DT:
	/*
	 * set the label on the dt_label widget..
	 */ 

	if(w != options_last_dt) {
          XtSetArg(argt[0], XmNlabelString,       &xmstr                    );
          XtGetValues(w, argt, 1);

          XtSetArg(argt[0], XmNlabelString,       xmstr                     );
          XtSetValues(dt_label, argt, 1);
	}
	else {
          xmstr = XmStringCreateLtoR("       ", XmFONTLIST_DEFAULT_TAG);
          i = 0;
          XtSetArg(argt[i], XmNlabelString,      xmstr            ); i++;
          XtSetValues(dt_label, argt, i);
	}
        i = 0;
        XtSetArg(argt[i], XmNuserData,          &logoFile          ); i++;
        XtGetValues(w, argt, i);

        /* 
	 * remove trailing spaces 
	 */
	if(strchr(logoFile,' '))
            temp_p = strtok(logoFile," ");	
	else
	    temp_p = logoFile;
 
        logoName = _DtGetIconFileName(DefaultScreenOfDisplay(dpyinfo.dpy),
                          temp_p, NULL, NULL, NULL);
 
        if (logoName == NULL) {
           LogError( 
	     ReadCatalog(MC_LOG_SET,MC_LOG_NO_LOGOBIT,MC_DEF_LOG_NO_LOGOBIT),
                             logoFile);
           logoFile = NULL;
        }
        i = 0;
        XtSetArg(argt[i], XmNimageName, logoName); i++;
        XtSetValues(logo_pixmap, argt, i);



	/* 
	 * set the selected toggle button...
	 */
        XtSetArg(argt[0], XmNset,       True                              );
        XtSetValues(w, argt, 1);

	/*
	 *  clear other toggles...
	 */
	XtSetArg(argt[0], XmNset,	False			          );

   	for(i = 0; i < appInfo.altDts ; ++i)
	  if(alt_dts[i] != NULL) 
	   if (w != alt_dts[i]) XtSetValues(alt_dts[i], argt, 1);

	if ( w != options_failsafe ) XtSetValues(options_failsafe, argt, 1);
	if ( w != options_dtlite  ) XtSetValues(options_dtlite , argt, 1);
	if ( w != options_dt      ) XtSetValues(options_dt     ,  argt, 1); 
        if ((w != options_last_dt) && (options_last_dt != NULL)) 
	   XtSetValues(options_last_dt,  argt, 1);


	/*
	 *  return focus to name/password widgets...
	 */
	 
	if ( focusWidget != NULL)
	    ProcessTraversal(focusWidget, XmTRAVERSE_CURRENT);

	break;
    }

}




/***************************************************************************
 *
 *  OptionsUnmapCB
 *
 *  callback when options menu unmaps
 ***************************************************************************/

void 
#ifdef _NO_PROTO
OptionsUnmapCB( wd, client_data, call_data )
        Widget wd ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
OptionsUnmapCB( Widget wd, XtPointer client_data, XtPointer call_data )
#endif /* _NO_PROTO */
{

    int		i;
    Dimension	width, height;

    Widget	w;
    XEvent	event;
    
#ifdef VG_TRACE
    vg_TRACE_EXECUTION("main:  entered OptionsUnmapCB ...");
#endif /* VG_TRACE */

    /*
     *  simulate an exposure event over the Options pushbutton to make sure
     *  the pushbutton elevates. (there is a bug in the toolkit where this
     *  doesn't always happen on some servers)...
     */

    w = options_button;
    
    i = 0;
    XtSetArg(argt[i], XmNwidth,			&width			); i++;
    XtSetArg(argt[i], XmNheight,		&height			); i++;
    XtGetValues(w, argt, i);

    event.type                = Expose;
    event.xexpose.serial      = LastKnownRequestProcessed(XtDisplay(w));
    event.xexpose.send_event  = True;
    event.xexpose.display     = XtDisplay(w);
    event.xexpose.window      = XtWindow(w);
    event.xexpose.x           = 0;
    event.xexpose.y           = 0;
    event.xexpose.width       = 1;   /* one pixel seems to be good enough, */
    event.xexpose.height      = 1;   /* but time will tell...		   */
#if 0
    event.xexpose.width       = ToPixel(matte, XmHORIZONTAL, (int)width  );
    event.xexpose.height      = ToPixel(matte, XmVERTICAL,   (int)height );
#endif
    event.xexpose.count       = 0;
   
    XtDispatchEvent (&event);
}


 
/***************************************************************************
 *
 *  PingServerCB
 *
 *  Ping the server occasionally with an Xsync to see if it is still there.
 *  We do this here rather than in dtlogin since dtgreet has the server
 *  grabbed.
 *
 ***************************************************************************/

static jmp_buf	pingTime;
static int	serverDead = FALSE;
static int	pingInterval = 0;	/* ping interval (sec.)		   */
static int	pingTimeout;		/* ping timeout (sec.)		   */

static void 
#ifdef _NO_PROTO
PingLost()
#else
PingLost( void )
#endif /* _NO_PROTO */
{
    serverDead = TRUE;
    longjmp (pingTime, 1);
}


static SIGVAL
#ifdef _NO_PROTO
PingBlocked()
#else
PingBlocked( int arg )
#endif /* _NO_PROTO */

{
    serverDead = TRUE;
    longjmp (pingTime, 1);
}


int 
#ifdef _NO_PROTO
VGPingServer()
#else
VGPingServer( void )
#endif /* _NO_PROTO */
{
    int	    (*oldError)();
    SIGVAL  (*oldSig)();
    int	    oldAlarm;

    oldError = XSetIOErrorHandler ((XIOErrorHandler)PingLost);
    oldAlarm = alarm (0);
    oldSig = signal (SIGALRM, (void(*)()) PingBlocked);
    alarm (pingTimeout * 60);
    if (!setjmp (pingTime))
    {
	XSync (dpyinfo.dpy, 0);
    }
    else
    {
	if ( serverDead ) {
	    LogError(ReadCatalog(
                                 MC_LOG_SET,MC_LOG_DEADSRV,MC_DEF_LOG_DEADSRV),
		dpyinfo.name);
	    alarm (0);
	    signal (SIGALRM, SIG_DFL);
	    XSetIOErrorHandler (oldError);
	    return 0;
	}
    }
    alarm (0);
    signal (SIGALRM, oldSig);
    alarm (oldAlarm);
    XSetIOErrorHandler (oldError);
    return 1;
}


void 
#ifdef _NO_PROTO
PingServerCB( call_data, id )
        XtPointer call_data ;
        XtIntervalId *id ;
#else
PingServerCB( XtPointer call_data, XtIntervalId *id )
#endif /* _NO_PROTO */
{

    char *t;
    

    /*
     *  get ping values from the environment...
     */

    if ( pingInterval == 0 ) {
	pingInterval  = ((t = (char *)getenv(PINGINTERVAL)) == NULL ? 0 : atoi(t));
	pingTimeout   = ((t = (char *)getenv(PINGTIMEOUT))  == NULL ? 0 : atoi(t));
    }

    
    /*
     *  ping the server. If successful, set a timer for the next ping,
     *  otherwise cleanup and exit...
     */

    if ( pingInterval != 0 ) {
	if (VGPingServer())
	    XtAddTimeOut((unsigned long) pingInterval * 60 * 1000, 
	    		  PingServerCB, NULL);
	else
	    exit(NOTIFY_RESTART);
    }
}


 
/***************************************************************************
 *
 *  PostMenuCB
 *
 *  post the option_button pop-up menu
 ***************************************************************************/

void 
#ifdef _NO_PROTO
PostMenuCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XmAnyCallbackStruct *call_data ;
#else
PostMenuCB( Widget w, XtPointer client_data, XtPointer call_data )
#endif /* _NO_PROTO */
{

    XmAnyCallbackStruct *p;

    p = (XmAnyCallbackStruct *) call_data;

#ifdef VG_TRACE
    vg_TRACE_EXECUTION("main:  entered PostMenuCB ...");
#endif /* VG_TRACE */

    /*
     *  make options menus if they don't yet exist...
     */

    if (options_menu == NULL)
	MakeOptionsMenu();


    /*
     *  post menu...
     */

    if (p->reason == XmCR_ARM       	&&
	p->event->type == ButtonPress) {

	XmMenuPosition(options_menu, p->event);
	XtManageChild(options_menu);
    }
}




/***************************************************************************
 *
 *  ProcessTraversal
 *
 *  move the input focus
 ***************************************************************************/

static void 
#ifdef _NO_PROTO
ProcessTraversal( w, direction )
        Widget w ;
        int direction ;
#else
ProcessTraversal( Widget w, int direction )
#endif /* _NO_PROTO */
{
    int i;

#ifdef VG_TRACE
    vg_TRACE_EXECUTION("main:  entered ProcessTraversal ...");
#endif /* VG_TRACE */

    i = XmProcessTraversal(w, direction);

#ifndef __hpux
    /*
     * Versions of Motif other than HP do not support the XmfocusCallback
     * on the TextField widget. We simulate it here by manually invoking the
     * callback routine...
     */

    TextFocusCB(w, NULL, NULL);
#endif    
}



 
/***************************************************************************
 *
 *  RefreshEH
 *
 *  cause the entire screen to refresh via exposure events
 ***************************************************************************/

void 
#ifdef _NO_PROTO
RefreshEH( w, client_data, event, continue_to_dispatch )
        Widget w ;
        XtPointer client_data ;
        XEvent *event ;
	Boolean *continue_to_dispatch)
#else
RefreshEH( Widget w, XtPointer client_data, XEvent *event,
	Boolean *continue_to_dispatch)
#endif /* _NO_PROTO */
{

    Window	cover;
    
#ifdef VG_TRACE
    vg_TRACE_EXECUTION("main:  entered RefreshEH ...");
#endif /* VG_TRACE */

    /*
     *  map/unmap a window that covers the entire screen. The resultant
     *  exposure events will refresh the screen. Note, the default
     *  background pixmap is NONE.
     */
     
    cover = XCreateWindow ( dpyinfo.dpy,		/* display	   */
    			    dpyinfo.root,		/* root window ID  */
    			    0,				/* x origin	   */
    			    0,				/* y origin	   */
    			    dpyinfo.width,		/* width	   */
    			    dpyinfo.height,		/* height	   */
			    0,				/* border width	   */
			    0,				/* depth	   */
			    InputOutput,		/* class	   */
			    CopyFromParent,		/* visual	   */
			    0,				/* value mask	   */
			    (XSetWindowAttributes *)NULL); /* attributes   */
    
    XMapWindow(dpyinfo.dpy, cover);
    XDestroyWindow(dpyinfo.dpy, cover);
    XFlush(dpyinfo.dpy);
}


/***************************************************************************
 *
 *  RequestCB
 *
 *  Accept a request from client
 ***************************************************************************/
static char requestBuf[REQUEST_LIM_MAXLEN];
static char *requestPtr = requestBuf;
static int headerSoFar = 0;
static int requestSoFar = 0;

void 
#ifdef _NO_PROTO
RequestCB(client_data, source, id)
        XtPointer client_data;
        int *source;
        XtInputId *id;  
#else
RequestCB(
        XtPointer client_data,
        int *source,
        XtInputId *id)
#endif /* _NO_PROTO */
{
  char *buf = requestBuf;
  int count;
  int remainder;
  RequestHeader *phdr = (RequestHeader *)requestBuf;

#ifdef VG_TRACE
  vg_TRACE_EXECUTION("main:  entered RequestCB ...");
#endif /* VG_TRACE */

 /*
  * Work on forming a complete RequestHeader...
  */
  if (headerSoFar != sizeof(RequestHeader)) {
    count = read(0, requestPtr, sizeof(RequestHeader)-headerSoFar);
    if (count < 0) {
      count = 0;
    }
    requestPtr += count;
    headerSoFar += count;
    if (headerSoFar != sizeof(RequestHeader)) {
      return;
    }
  }

 /*
  * Work on forming a complete request...
  */ 
  remainder =  phdr->length - sizeof(RequestHeader);
  if (requestSoFar != remainder) {
    count = read(0, requestPtr, remainder-requestSoFar);
    if (count < 0) {
      count = 0;
    }
    requestPtr += count;
    requestSoFar += count;
    if (requestSoFar != remainder) {
      return;
    }
  } 
  requestPtr = requestBuf;
  headerSoFar = 0;
  requestSoFar = 0;

 /* 
  * Initiate response to request.
  */
  switch(phdr->opcode)
  {
    case REQUEST_OP_EXIT:
#ifdef VG_TRACE
      vg_TRACE_EXECUTION("main:  got REQUEST_OP_EXIT ...");
#endif /* VG_TRACE */
      RespondExitCB(NULL, NULL, NULL);
      break;

    case REQUEST_OP_MESSAGE:
      {
        RequestMessage *r = (RequestMessage *)phdr;
        XmString string;

#ifdef VG_TRACE
        vg_TRACE_EXECUTION("main:  got REQUEST_OP_MESSAGE ...");
#endif /* VG_TRACE */
        if (r->idMC)
        {
          FILE *fp;

         /*
          * Caller passed in MC_* message id.
          */
          if (r->idMC == MC_NO_LOGIN &&
              (fp = fopen(NO_LOGIN_FILE,"r")) != NULL)
          {
           /*
            * For MC_NO_LOGIN read message from file.
            */
            char buffer[256];
            int j;

            string = ReadCatalog(MC_ERROR_SET, r->idMC, buf+r->offMessage);
            string = XmStringConcat(string, XmStringSeparatorCreate());

            while (fgets(buffer, 256, fp) != NULL)
            {
              j = strlen(buffer);
              if ( buffer[j-1] == '\n' ) buffer[j-1] = '\0';

              string = XmStringConcat(string,
                                      XmStringCreate(buffer,
                                      XmFONTLIST_DEFAULT_TAG));
              string = XmStringConcat(string, XmStringSeparatorCreate());
            }
          }
          else
          {
           /*
            * Read message from message catalog.
            */
            string = ReadCatalog(MC_ERROR_SET, r->idMC, buf+r->offMessage);
          }
        }
        else
        {
         /*
          * Generate message from provided string. 
          */
#ifdef SIA
          string = multiline_xmstring(buf+r->offMessage);
#else
          string = XmStringCreate(buf+r->offMessage,XmFONTLIST_DEFAULT_TAG);
#endif
        }
      
        _DtShowDialog(error, string);
 
        XmStringFree(string);
      }
      break;

    case REQUEST_OP_HOSTNAME:
#ifdef VG_TRACE
      vg_TRACE_EXECUTION("main:  got REQUEST_OP_HOSTNAME ...");
#endif /* VG_TRACE */
      _DtShowDialog(hostname, NULL);
      break;

    case REQUEST_OP_EXPASSWORD:
#ifdef VG_TRACE
      vg_TRACE_EXECUTION("main:  got REQUEST_OP_EXPASSWORD ...");
#endif /* VG_TRACE */
      _DtShowDialog(expassword, NULL);
      break;

    case REQUEST_OP_CHPASS:
#ifdef VG_TRACE
      vg_TRACE_EXECUTION("main:  got REQUEST_OP_CHPASS ...");
#endif /* VG_TRACE */
      break;

    case REQUEST_OP_CHALLENGE:
      {
        RequestChallenge *r = (RequestChallenge *)phdr;
        XmString string;
        int i;
        LoginTextPtr textdata;
        Boolean change;

#ifdef VG_TRACE
        vg_TRACE_EXECUTION("main:  got REQUEST_OP_CHALLENGE ...");
#endif /* VG_TRACE */
        textdata = GetLoginTextPtr(login_text);

        change = (textdata->bEcho != r->bEcho);

        XtUnmapWidget(textdata->text[textdata->bEcho]);

        textdata->bEcho = r->bEcho;
        textdata->noechobuf[0] = '\0';

        XtAddEventHandler(textdata->text[textdata->bEcho], ExposureMask, False,
                        FakeFocusIn, NULL);

        XtMapWidget(textdata->text[textdata->bEcho]);

        XtPopup(login_shell, XtGrabNone); 

	XGrabKeyboard (dpyinfo.dpy, XtWindow (textdata->text[textdata->bEcho]),
        False, GrabModeAsync, GrabModeAsync, CurrentTime); 

        XmTextFieldSetString(
          textdata->text[textdata->bEcho],
          r->offUserNameSeed ? buf+r->offUserNameSeed : "");

        XmTextFieldSetSelection (
          textdata->text[1],
          0, XmTextFieldGetLastPosition(textdata->text[1]),
          CurrentTime );

        if (r->idMC)
        {
         /*
          * Read message from message catalog.
          */
          string = ReadCatalog(MC_LABEL_SET, r->idMC, buf+r->offChallenge);
        }
        else
        {
         /*
          * Generate message from provided string.
          */
          string = XmStringCreate(buf+r->offChallenge,XmFONTLIST_DEFAULT_TAG);
        }

        i = 0;
        XtSetArg(argt[i], XmNlabelString,       string                   ); i++;
        XtSetValues(login_label, argt, i);

        XmStringFree(string); 

        if (change)
        {
           char buf[256];

           i = 0;
           if (textdata->bEcho)
           {
             XtSetArg(argt[i], XmNlabelString, textdata->onGreeting     ); i++;
           }
           else
           {
             sprintf(buf, textdata->offGreetingFormat,
                     textdata->offGreetingUname);
             string = XmStringCreate(buf, XmFONTLIST_DEFAULT_TAG);
             XtSetArg(argt[i], XmNlabelString, string    ); i++;
             free(textdata->offGreetingUname);
           }
           XtSetValues(greeting, argt, i);
	   /* set the dt_label with the session that is enabled */
  	   SetDtLabelAndIcon();

        }

        XtSetSensitive(ok_button, True);
        XtSetSensitive(options_button, True);
        XtSetSensitive(help_button, True);

        if(r->enableClearBtn) {
            XtSetSensitive(clear_button, True);
	} else {
            XtSetSensitive(clear_button, False);
	}

	XUngrabKeyboard(dpyinfo.dpy,CurrentTime);
      }
      break;
#ifdef SIA
    case REQUEST_OP_FORM:
      {
      RequestForm *r = (RequestForm *)buf;
      int i;
      char *prompt_ptr;

#ifdef VG_TRACE
      vg_TRACE_EXECUTION("main:  got REQUEST_OP_FORM ...");
#endif /* VG_TRACE */
      siaFormInfo.num_prompts = r->num_prompts;
      siaFormInfo.rendition = r->rendition;
      siaFormInfo.title = XtMalloc(strlen(buf + r->offTitle));
      strcpy(siaFormInfo.title, buf + r->offTitle);

      prompt_ptr = buf + r->offPrompts;

      for (i=0; i < siaFormInfo.num_prompts; i++)
          {
          siaFormInfo.visible[i] = r->visible[i];
          siaFormInfo.prompts[i] = XtMalloc(strlen(prompt_ptr));
          siaFormInfo.answers[i] = NULL;
          strcpy(siaFormInfo.prompts[i], prompt_ptr);
          prompt_ptr += strlen(prompt_ptr) + 1;
          }

      /*
       * Create Widgets:
       *  Form
       *   Title
       *   prompt labels and answer text fields for each prompt
       *   OK button
       * Add callbacks as needed.  If not visible don't echo.
       * On OK callback, collect info and send it.  Destroy widgets.
       */
       SiaForm(&siaFormInfo);

      }
      break;
#endif /* SIA */
  }


#if 0
  {
   /*
    * Send immediate response to debug.
    */
    char outbuf[REQUEST_LIM_MAXLEN];
    char *p;
    ResponseDebug *rdebug = (ResponseDebug *)outbuf;

    rdebug->hdr.opcode = REQUEST_OP_DEBUG;
    rdebug->hdr.reserved = 0;
    rdebug->offString = sizeof(*rdebug);
    p = ((char *)(rdebug)) + rdebug->offString;
    strcpy(p, "This is my debug string");
    rdebug->hdr.length = sizeof(*rdebug) + strlen(p) + 1;
    
    TellRequester(outbuf, (size_t) rdebug->hdr.length);
  } 
#endif
}

/***************************************************************************
 *
 *  RespondExitCB
 *
 *  Respond to an exit request from client
 ***************************************************************************/

void 
#ifdef _NO_PROTO
RespondExitCB( w, client, call)
        Widget w ;
        XtPointer client;
        XtPointer call;
#else
RespondExitCB(
        Widget w,
        XtPointer client,
        XtPointer call)
#endif /* _NO_PROTO */
{
  char buf[REQUEST_LIM_MAXLEN];
  ResponseExit *r = (ResponseExit *)buf;

#ifdef VG_TRACE
  vg_TRACE_EXECUTION("main:  entered RespondExitCB ...");
#endif /* VG_TRACE */

  r->hdr.opcode = REQUEST_OP_EXIT;
  r->hdr.reserved = 0;
  r->hdr.length = sizeof(*r);

  TellRequester(buf, (size_t) r->hdr.length);

  CleanupAndExit(NULL, NOTIFY_OK);
}
/***************************************************************************
 *
 *  RespondLangCB
 *
 *  Respond to a lang item selection from user
 ***************************************************************************/
int amChooser = 0;
int orig_argc;
char **orig_argv;

#define NUM_LC_TYPES 7

static char* lc_types[] = { 
	"LC_COLLATE",
	"LC_MESSAGES",
	"LC_NUMERIC",
	"LC_CTYPE",
	"LC_MONETARY",
	"LC_TIME",
	"LANG",
};

void
#ifdef _NO_PROTO
RespondLangCB( w, language, call)
  Widget w ;
  XtPointer language;
  XtPointer call;
#else
RespondLangCB( Widget w, XtPointer language, XtPointer call)
#endif /* _NO_PROTO */
{
  XtSetArg(argt[0], XmNset, True);
  XtSetValues(w, argt, 1);

#ifdef VG_TRACE
  vg_TRACE_EXECUTION("main:  entered RespondLangCB ...");
#endif /* VG_TRACE */

  if (amChooser) {

	if (XmToggleButtonGadgetGetState(w)) {
	    char buff[MAXPATHLEN+1];
	    FILE *fp;
	    char *lastlangfile;
	    int i;

	    /*  Record language setting */

	    if ( (lastlangfile = getenv("SDT_LASTLANG_FILE")) != NULL  &&
		  (fp = fopen(lastlangfile,"w")) != NULL ) {
		fputs(language, fp);	
		fclose(fp);
	    }
	

	    /* Re-initialize the Chooser locale environment */

	    for (i=0; i < NUM_LC_TYPES; i++) {
		if (getenv(lc_types[i])) {
	            strcpy(buff, lc_types[i]);
	            strcat(buff, "=");
	            strcat(buff, language);
		}
	        putenv(strdup(buff));
	    }

            sprintf(buff,"/usr/lib/locale/%s/locale_map", language);

            if ((fp = fopen(buff, "r")) != NULL) {

	        /* process the corresponding language locale_map */

		while (fgets(buff, MAXPATHLEN, fp)) {
		    buff[strlen(buff)-1] = '\0';
	    	    putenv(strdup(buff));
                }
	    } 

	    execv(orig_argv[0], orig_argv);
	}
  } else {
    char buf[REQUEST_LIM_MAXLEN];
    ResponseLang *r = (ResponseLang *)buf;
    char *p;
  
    r->hdr.opcode = REQUEST_OP_LANG;
    r->hdr.reserved = 0;
    r->offLang = sizeof(*r);
    p = ((char *)(r)) + r->offLang;
    strcpy(p, XmToggleButtonGadgetGetState(w) ? language: "default");
    r->hdr.length = sizeof(*r) + strlen(p) + 1;

    TellRequester(buf, (size_t) r->hdr.length);
 
    CleanupAndExit(NULL, NOTIFY_LANG_CHANGE);
  }
}

/***************************************************************************
 *
 *  RespondChallengeCB
 *
 *  Respond to a challenge request from client
 ***************************************************************************/

void 
#ifdef _NO_PROTO
RespondChallengeCB( w, client, call )
        Widget w;
        XtPointer client;
        XtPointer call;
#else
RespondChallengeCB(
        Widget w,
        XtPointer client,
        XtPointer call)
#endif /* _NO_PROTO */
{
  char buf[REQUEST_LIM_MAXLEN];
  ResponseChallenge *r = (ResponseChallenge *)buf;
  char *value;
  char *p;
  LoginTextPtr textdata;

#ifdef VG_TRACE
  vg_TRACE_EXECUTION("main:  entered RespondChallengeCB ...");
#endif /* VG_TRACE */

  XtSetSensitive(ok_button, False);
  XtSetSensitive(clear_button, False);
  XtSetSensitive(options_button, False);
  XtSetSensitive(help_button, False);

  textdata = GetLoginTextPtr(login_text);

 /*
  * Get username and password. Username is obtained from widget
  * while password is stored in global buffer.
  */
  if (textdata->bEcho)
  {
    value = XmTextFieldGetString(login_text);
    textdata->offGreetingUname = strdup(value);
    userName = strdup(value);
    if (strlen(textdata->offGreetingUname) > 16)
    {
      textdata->offGreetingUname[16] = '\0';
      userName[16] = '\0';
    }
  }
  else
  {
    value = textdata->noechobuf;
  }

  r->hdr.opcode = REQUEST_OP_CHALLENGE;
  r->hdr.reserved = 0;
  r->offResponse = sizeof(*r);
  p = buf + r->offResponse;
  strcpy(p, value);
  r->hdr.length = r->offResponse + strlen(p) + 1; 

  if (textdata->bEcho)
  {
    XtFree(value);
    XmTextFieldSetString(login_text, "");
  }
  else
  {
   /*
    * Clean password memory to foil snoopers.
    */
    bzero(textdata->noechobuf, strlen(textdata->noechobuf));
  }

  TellRequester(buf, (size_t) r->hdr.length);
}

/***************************************************************************
 *
 *  RespondDialogCB
 *
 *  Respond to a request that displayed a dialog
 ***************************************************************************/

void
#ifdef _NO_PROTO
RespondDialogCB( w, client, reason)
        Widget w ;
        XtPointer client;
        XmSelectionBoxCallbackStruct *reason;
#else
RespondDialogCB(
        Widget w,
        XtPointer client,
        XtPointer reason )
#endif /* _NO_PROTO */
{
  char buf[REQUEST_LIM_MAXLEN];
  char *value;
  struct hostent      *hostent;
  XmString string;

#ifdef VG_TRACE
  vg_TRACE_EXECUTION("main:  entered RespondDialogCB ...");
#endif /* VG_TRACE */

  if (w == error_message)
  {
  ResponseMessage *r= (ResponseMessage *)buf;

  r->hdr.opcode = REQUEST_OP_MESSAGE;
  r->hdr.reserved = 0;
  r->hdr.length = sizeof(*r);

  TellRequester(buf, (size_t) r->hdr.length);
  } 
  else if (w == passwd_message)
  {
    if (((XmSelectionBoxCallbackStruct*)reason)->reason == XmCR_OK)
    {
      CleanupAndExit(w, NOTIFY_PASSWD_EXPIRED);
    } 
    else
    {
  ResponseExpassword *r= (ResponseExpassword *)buf;

  r->hdr.opcode = REQUEST_OP_EXPASSWORD;
  r->hdr.reserved = 0;
  r->hdr.length = sizeof(*r);

  TellRequester(buf, (size_t) r->hdr.length);
    }
  }
  else if (w == hostname_message)
  {
    if (((XmSelectionBoxCallbackStruct*)reason)->reason == XmCR_OK)
    {
      CleanupAndExit(w, NOTIFY_OK);
    } 
    else
    {
      CleanupAndExit(w, NOTIFY_NO_WINDOWS);
    }
  }
  else if (w == remote_host_dialog)
  {
    if (((XmSelectionBoxCallbackStruct*)reason)->reason == XmCR_CANCEL) {
          XtUnmanageChild(remote_host_dialog);
          XmUpdateDisplay(remote_host_dialog); 

          XtSetSensitive(ok_button, True);
          XtSetSensitive(options_button, True);
          XtSetSensitive(help_button, True);

    	  if (remote_host_cb_sensitive) XtSetSensitive(clear_button, True);

	  return;
    }

   XmStringGetLtoR(((XmSelectionBoxCallbackStruct*)reason)->value,
			 XmSTRING_DEFAULT_CHARSET, &value);
   hostent = gethostbyname (value);
   if (!hostent)
      _DtShowDialog(bad_hostname,NULL);
   else {
 
          XtUnmanageChild(remote_host_dialog);
          XmUpdateDisplay(remote_host_dialog); 

          _DtTurnOnHourGlass(login_shell);
          string = ReadCatalog(MC_LOG_SET, MC_CONNECT_TO_HOST,
                                                 MC_DEF_CONNECT_TO_HOST);
          string = XmStringConcat(string,
                       XmStringCreate(value, XmFONTLIST_DEFAULT_TAG));

          _DtShowDialog(conn_to_host,string);
          _DtTurnOnHourGlass(connect_to_host);
          XmUpdateDisplay(connect_to_host);

          if (connect_to_remote_host(value)) {
             StoreHostname(value);
             CleanupAndExit(NULL, NOTIFY_REMOTE_HOST_LOGIN);
          } 
          else  {
             _DtTurnOffHourGlass(login_shell);
             _DtTurnOffHourGlass(connect_to_host);
             XtUnmanageChild(connect_to_host);
             _DtShowDialog(host_not_runlogin,NULL);
          }
          XtSetSensitive(ok_button, True);
          XtSetSensitive(clear_button, True);
          XtSetSensitive(options_button, True);
          XtSetSensitive(help_button, True);
   }
  }
}

/***************************************************************************
 *
 *  SetDtLabelAndIcon
 *
 *  label to display in the dt_label widget and 
 *  the logo to display in logo_pixmap 
 **************************************************************************/
void
#ifdef _NO_PROTO
SetDtLabelAndIcon( )
#else
SetDtLabelAndIcon()
#endif /* _NO_PROTO */
{
  int i;
  Boolean        toggleon;          /* status of session toggle buttons     */
  Widget         w;  		/* widget that is enabled in the menu   */
  char           *logoFile;
  char           *logoName;
  char           lastsession[MAXPATHLEN];
  FILE           *lasts;
  char           altdtstart[MAXPATHLEN];
  char           altdtstartclass[MAXPATHLEN];
  char           *startup_name;
  XrmValue       startup_value;
  char	         temp[MAXPATHLEN];
  int		 rand_dt = 0;/* if the user is not valid, give a random one */
  int		 dts = 0; 
  int		 k = 0;
  static Widget	defWidget = NULL;
	char 	*temp_p;
  int		 first_login=False;



   
  /*
  *  if user is logging in, set dt_label desired. No more than
  *  one desktop  can be selected at a time ...
  */

 
  XtSetArg(argt[0], XmNset,       &toggleon               );
 
  XtGetValues(options_failsafe, argt, 1);
  if ( toggleon ) w = options_failsafe; 

  XtGetValues(options_dtlite, argt, 1);
  if ( toggleon ) w = options_dtlite;
 
  XtGetValues(options_dt, argt, 1);
  if ( toggleon ) w = options_dt;
 
 
  if(options_last_dt != NULL) {
   XtGetValues(options_last_dt, argt, 1);
   if ( toggleon ) w = options_last_dt;
  }
 
  for(i = 0; i < appInfo.altDts ; ++i) {
    if(alt_dts[i] != NULL) {
      XtGetValues(alt_dts[i], argt, 1);
      if ( toggleon )  w = alt_dts[i]; 
      ++dts;                 /* total valid dts out of the appInfo.altDts */
    }
  }


  i = 0;
  XtSetArg(argt[i], XmNset,      False		); i++;
  XtSetValues(w, argt, i);

  if(defWidget == NULL)
    defWidget = w;

  if(w == options_last_dt) {
    if(strcmp(userName,"\0") != 0)  {

      if((user_p = getpwnam(userName)) != NULL) {
	strcpy(lastsession,user_p->pw_dir);
	strcat(lastsession,LAST_SESSION_FILE);

	seteuid(user_p->pw_uid); 

	if(access(lastsession,R_OK) == 0) {

	   if((lasts = fopen(lastsession,"r")) == NULL)
              fprintf(stderr,"Unable to open file for reading: %s\n",
					lastsession);
           else{
	      strcpy(lastsession,"\0");
              fgets(lastsession,MAXPATHLEN,lasts);
              fclose(lasts);
           }

	   seteuid(0);

	 if((strlen(lastsession) != 0) && (strcmp(lastsession,"\0") != 0)){

	   if(appInfo.altDts == 0) { 		
	   /* 
	    * show regular desktop string and logo if the altdts is 0
	    */
	       w = options_dt;
		
	   }
	   else {
             for(i = 0; i < appInfo.altDts ; ++i) {
                sprintf(altdtstart,"%s%d","Dtlogin*altDtStart",i+1);
                sprintf(altdtstartclass,"%s%d","Dtlogin*AltDtStart",i+1);

                if(XrmGetResource(XtDatabase(dpyinfo.dpy),altdtstart,
		    altdtstartclass,&startup_name,&startup_value) == True) {

          	if(strchr(startup_value.addr,' ')) 
                    strcpy(temp, strtok(startup_value.addr," "));
		else
                    strcpy(temp, startup_value.addr);

          	   if(alt_dts[i] != NULL) {
              	     if (strcmp(temp, lastsession) == 0){
		       w = alt_dts[i]; 
		       break; 		/* found the last dt, quit now*/ 
  		     }
	   	   }
	 	}
	     }
	   }
	 }
        } else {
	 seteuid(0);
	 first_login = True;
        }
      }
      else {
	/*
	 * if user is not valid, pick a random desktop
	 * same one for the invalid user
	 */ 
	for(i=0; i<= strlen(userName) ; i++)
	  rand_dt = rand_dt + userName[i];

          if(XtIsManaged(options_dt)) 
		++dts;
	  rand_dt = rand_dt % dts;
	  if(rand_dt == 0 && XtIsManaged(options_dt))
		w = options_dt;
	  else {
	        dts = 0;
	        for(i = 0; i < appInfo.altDts; ++i)
                 if(alt_dts[i] != NULL){ 
		     if(dts == rand_dt)  {
		        w = alt_dts[i];
		        break; /* quit  now */
		     }
		++dts;
		 }
		}
		
	  }

	if(w == options_last_dt){        
	    /* 
	     * none of the alt desktops or regular dt matched  
	     * set it to the regular desktop or the logically first one
	     */
             if(XtIsManaged(options_dt)){   
               w = options_dt;     /* if managed it is the first one... */
             }
             else {
	       struct stat statb; 

	       if ( first_login &
		    dts > 1 &&
		    stat(FIRST_LOGIN, &statb) != -1 &&
		    (statb.st_mode & S_IEXEC) == S_IEXEC ) {
		  /* 
		   * First login, more than one desktop present, and 
		   * sdt_firstlogin is on system.  Let user pick correct dt
		   * later in login cycle via sdt_firstlogin. 
		   * 
		   * Leave options_last_dt selected & first_login True. 
		   */
	       } else {
		  first_login=False;
		  for(i = 0; i < appInfo.altDts ; ++i) {
                     if(alt_dts[i] != NULL) {
                        w = alt_dts[i];
                        break;          /* found the first altdt, quit now */
                     }
                  }
	       }
            }
        }
    }
  }

  i = 0;
  XtSetArg(argt[i], XmNset,      True		); i++;
  XtSetValues(w, argt, i);

  i = 0;
  XtSetArg(argt[i], XmNlabelString,      &xmstr		); i++;
  XtGetValues(w, argt, i);  

  if (first_login) { 
        xmstr = XmStringCreateLtoR("       ", XmFONTLIST_DEFAULT_TAG);
  }

  if(strcmp(userName,"\0") == 0) {

     if(defWidget == options_last_dt)
        xmstr = XmStringCreateLtoR("       ", XmFONTLIST_DEFAULT_TAG);

     i = 0;
     XtSetArg(argt[i], XmNset,      False		); i++;
     XtSetValues(w, argt, i);

     i = 0;
     XtSetArg(argt[i], XmNset,      True		); i++;
     XtSetValues(defWidget, argt, i);
     
     w = defWidget;
  }

 /* to set the defaults  after the user is done			 
  * entering the login .. 
  */
  strcpy(userName,"\0");  


  i = 0;
  XtSetArg(argt[i], XmNlabelString,      xmstr		); i++;
  XtSetValues(dt_label, argt, i); 

  i = 0;
  XtSetArg(argt[i], XmNuserData,          &logoFile          ); i++;
  XtGetValues(w, argt, i);

  /* 
   * remove trailing spaces 
   */
  if(strchr(logoFile,' '))
     temp_p = strtok(logoFile," ");
  else
     temp_p = logoFile;
 
  logoName = _DtGetIconFileName(DefaultScreenOfDisplay(dpyinfo.dpy),
                              temp_p, NULL, NULL, NULL);
 
  if (logoName == NULL) {
        LogError(
	ReadCatalog(MC_LOG_SET,MC_LOG_NO_LOGOBIT,MC_DEF_LOG_NO_LOGOBIT),
                             logoFile);
        logoFile = NULL;
  }

  i = 0;
  XtSetArg(argt[i], XmNimageName, logoName); i++;
  XtSetValues(logo_pixmap, argt, i);
}

/***************************************************************************
 *
 *  _DtShowDialog
 *
 *  display a dialog message box
 ***************************************************************************/

static void 
#ifdef _NO_PROTO
_DtShowDialog( dtype, msg)
        DialogType dtype ;
        XmString msg ;
#else
_DtShowDialog( DialogType dtype, XmString msg)
#endif /* _NO_PROTO */
{
    Widget *w;
    
#ifdef VG_TRACE
    vg_TRACE_EXECUTION("main:  entered _DtShowDialog ...");
#endif /* VG_TRACE */

    switch (dtype) {
      case copyright:  w = &copyright_msg; break;
      case error:      w = &error_message; break;
      case expassword: w = &passwd_message; break;
      case help:       w = &help_message; break;
      case hostname:   w = &hostname_message; break;
      case help_chooser: w = &help_message; break;
      case bad_hostname: w = &bad_host; break;
      case host_not_runlogin: w = &host_not_run_login; break;
      case conn_to_host: w = &connect_to_host; break;
    }

    if (*w == NULL)
    {
      MakeDialog(dtype);
    }
    
    if ( msg != NULL ) {
	XtSetArg(argt[0], XmNmessageString, msg	);
	XtSetValues(*w, argt, 1);
    }

    XtManageChild(*w);

    XSetInputFocus(XtDisplay(*w), XtWindow(*w), 
    		   RevertToPointerRoot, CurrentTime);
}    

void
#ifdef _NO_PROTO
MakeHostDialog()
#else
MakeHostDialog()
#endif /* _NO_PROTO */
{
    XmString ok,cancel,string;
    register int i;
    int len;
    char buf[100];
    FILE        *f;

    remote_host_cb_sensitive = XtIsSensitive(clear_button); 

    XtSetSensitive(ok_button, False);
    XtSetSensitive(clear_button, False);
    XtSetSensitive(options_button, False);
    XtSetSensitive(help_button, False);
 
 
    ok     = ReadCatalog(MC_LABEL_SET, MC_OK_LABEL, MC_DEF_OK_LABEL);
    cancel = ReadCatalog(MC_LABEL_SET, MC_CANCEL_LABEL, MC_DEF_CANCEL_LABEL);
    i = InitArg(MessageBox);

    if ((f = fopen(REMOTE_GREET_HOST_FILE,"r")) != (FILE *) NULL) {
            strcpy(buf, "");
            fgets(buf,100,f);
            len = strlen (buf);
            if (buf[len-1] == '\n')
                buf[len-1] = '\0';
            fclose(f);

    string = XmStringCreate(buf, XmFONTLIST_DEFAULT_TAG);
    XtSetArg(argt[i], XmNtextString,            string                  ); i++;
    }

    XtSetArg(argt[i], XmNmarginHeight,          MBOX_MARGIN_HEIGHT      ); i++;
    XtSetArg(argt[i], XmNmarginWidth,           MBOX_MARGIN_WIDTH       ); i++;
    XtSetArg(argt[i], XmNshadowThickness,       SHADOW_THICKNESS        ); i++;
    XtSetArg(argt[i], XmNokLabelString,         ok                      ); i++;
    XtSetArg(argt[i], XmNcancelLabelString,     cancel                  ); i++;
    XtSetArg(argt[i], XmNnoResize,              False                   ); i++;
    XtSetArg(argt[i], XmNresizePolicy,          XmRESIZE_ANY            ); i++;
    XtSetArg(argt[i], XmNlabelFontList,         appInfo.labelFont       ); i++;
    XtSetArg(argt[i], XmNtextFontList,          appInfo.labelFont       ); i++;
 
 
    xmstr = ReadCatalog(MC_LABEL_SET, MC_REMOTE_HOST, MC_DEF_REMOTE_HOST);
    XtSetArg(argt[i], XmNmessageString,             xmstr           ); i++;
 
 
 
    XtSetArg(argt[i], XmNselectionLabelString, xmstr);i++;
    XtSetArg(argt[i], XmNautoUnmanage, False);i++;
    remote_host_dialog = XmCreatePromptDialog(table, "prompt", argt, i);
    XtUnmanageChild(XmSelectionBoxGetChild(remote_host_dialog,XmDIALOG_HELP_BUTTON));
    XmStringFree(xmstr);
 
    XtAddCallback(remote_host_dialog, 
		  XmNokCallback, (XtCallbackProc) RespondDialogCB, NULL);

    XtAddCallback(remote_host_dialog, 
		  XmNcancelCallback, (XtCallbackProc) RespondDialogCB, NULL);
 

    XtManageChild(remote_host_dialog);
 
    /* ;XtPopup(XtParent(remote_host_dialog), XtGrabNone); */
 
    XSetInputFocus(XtDisplay(remote_host_dialog), XtWindow(remote_host_dialog),
                   RevertToPointerRoot, CurrentTime);
 
 
}


/***************************************************************************
 *
 *  ShowDialogCB
 *
 *  callback to display a dialog message box
 ***************************************************************************/

void 
#ifdef _NO_PROTO
ShowDialogCB( w, dialog, call_data )
        Widget w ;
        DialogType dialog ;
        XtPointer call_data ;
#else
ShowDialogCB( Widget w, XtPointer dialog, XtPointer call_data )
#endif /* _NO_PROTO */
{

    _DtShowDialog( (DialogType) dialog, NULL);
}




/***************************************************************************
 *
 *  TextFocusCB
 *
 *  set focusWidget global variable when focus changes
 ***************************************************************************/


void 
#ifdef _NO_PROTO
TextFocusCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
TextFocusCB( Widget w, XtPointer client_data, XtPointer call_data )
#endif /* _NO_PROTO */
{
    if ( w == login_text )
	focusWidget = w;
}

/***************************************************************************
 *
 *  GetLoginTextPtr
 *
 *  Return login text widget instance data
 ***************************************************************************/

LoginTextPtr
#ifdef _NO_PROTO
GetLoginTextPtr( w )
        Widget w ;
#else
GetLoginTextPtr( Widget w )
#endif /* _NO_PROTO */
{
  LoginTextPtr textdata;
  int i;

  i = 0;
  XtSetArg(argt[i], XmNuserData,     &textdata              ); i++;
  XtGetValues(w, argt, i);

  return(textdata);
}


#ifdef BLS
/***************************************************************************
 *
 *  PromptSensitivityLevel
 *
 *  Prompt for B1 Sensitivity Level. The password widget set is reused for
 *  this purpose rather than creating another complete widget set. It already
 *  has most of the proper size and alignment specifications needed. Using
 *  the password set also allows the B1 code changes to be more localized.
 *
 ***************************************************************************/

static void 
#ifdef _NO_PROTO
PromptSensitivityLevel( )
#else
PromptSensitivityLevel( void)
#endif /* _NO_PROTO */
{

    Dimension	width;
    int		i, width1, width2;

    /*
     *  Use the password widget set to prompt for the B1 Sensitivity Level.
     *  Remember to put it back to normal if the user presses [Clear].
     */
     
    normalPasswordWidget = False;
    
    XtRemoveAllCallbacks(_text, XmNmodifyVerifyCallback);
    XmTextFieldSetString(_text,"");


    /*
     *  Change the label and resize the password form...
     */
     
    i = 0;
    XtSetArg(argt[i], XmNresizable,		True			); i++;
    XtSetArg(argt[i], XmNresizePolicy,		XmRESIZE_ANY		); i++;
    XtSetValues(_form, argt, i);

    i = 0;
    xmstr = ReadCatalog(MC_LABEL_SET, -1, "Sensitivity Level:");
    XtSetArg(argt[i], XmNrecomputeSize,		True			); i++;
    XtSetArg(argt[i], XmNlabelString,		xmstr			); i++;
    XtSetValues(_label, argt, i);

    XmStringFree(xmstr);
    
    /*
     *  Center the form horizontally in the login_matte...
     *
     */

    CenterForm(matte1, _form);
    
    ProcessTraversal(_text, XmTRAVERSE_CURRENT);

}

#endif /* BLS */


static void
#ifdef _NO_PROTO
TellRequester(buf, nbytes)
  char * buf;
  size_t nbytes;
#else
TellRequester(char * buf, size_t nbytes)
#endif
{
#ifdef VG_TRACE
  vg_TRACE_EXECUTION("main:  entered TellRequester ...");
#endif /* VG_TRACE */
  write(1, buf, nbytes);
}
/***************************************************************************
 *
 *
 *  Store the hostname to query.
 **************************************************************************/
void
#ifdef _NO_PROTO
StoreHostname(query_hostname)
char *query_hostname;
#else
StoreHostname(char *query_hostname)
#endif /* _NO_PROTO */
{
FILE *query_host_ptr = NULL;
char host_file[100];
 
   
    sprintf(host_file,"%s",REMOTE_GREET_HOST_FILE);
       
    query_host_ptr = fopen(host_file, "w");
 
    if (!query_host_ptr) {
        LogError(ReadCatalog(MC_LOG_SET,MC_LOG_SRV_OPEN,MC_DEF_LOG_SRV_OPEN),
                host_file);
        query_host_ptr = NULL;
    }
    else{
        fputs(query_hostname, query_host_ptr);
        (void) fflush(query_host_ptr);
        fclose(query_host_ptr);
    }
       
}

 /****************************************************************************
 *                These routines check if the remote host is running dtlogin
 *                or any XDM based login manager.
 *
 ************************************<+>*************************************/
static int
#ifdef _NO_PROTO
connect_to_remote_host(value)
char *value;
#else
connect_to_remote_host(char *value)
#endif /* _NO_PROTO */
{
static XdmcpBuffer          buffer;
int    socketFD;
struct sockaddr_in to, clnt_addr;
static ARRAYofARRAY8    AuthenticationNames;
struct hostent *hp;
int i = -1; 
int clnt_len = 0; 
int xdm_serv_len = 0;
int n = 0;
XdmcpHeader header;
int waitTime = 10; /* time to get the response from remote dtlogin */
struct FD_TYPE reads;
struct FD_TYPE SocketsMask = {0};
int     SocketsMax  = -1;
struct timeval      timeout, *ptimeout;
int nready;
struct sockaddr addr;
int             addrlen;

    if ((hp = gethostbyname(value)) == NULL)  {
                fprintf(stderr,"gethostbyname failed \n");
                return 0;
     }
 
     xdm_serv_len = sizeof(to);
     memset((char *)&to, '\0', xdm_serv_len);
     to.sin_family = AF_INET;
     to.sin_port   = htons (XDM_UDP_PORT);
     memcpy((char *)&to.sin_addr, hp->h_addr, hp->h_length);

     clnt_len = sizeof(clnt_addr);
     memset((char *)&clnt_addr, '\0', clnt_len);
     clnt_addr.sin_family = AF_INET;
     clnt_addr.sin_addr.s_addr = INADDR_ANY;
     clnt_addr.sin_port = 0; 

    if((socketFD = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	 perror("socket");
         return 0; 
    }

    SocketsMax = socketFD;

    FD_SET (socketFD, &SocketsMask);
    reads = SocketsMask;

    if (waitTime >= 0 ) {
            timeout.tv_sec  = waitTime;
            timeout.tv_usec = 10;
            ptimeout = &timeout;
    }
    else
            ptimeout = NULL;

    if((bind (socketFD,(struct sockaddr *)&clnt_addr,clnt_len)) < 0) {
	 perror("bind");
	 return 0;
    }

 
    header.version = XDM_PROTOCOL_VERSION;
    header.opcode = (CARD16) QUERY;
    header.length = 1;
    for (i = 0; i < (int)AuthenticationNames.length; i++)
        header.length += 2 + AuthenticationNames.data[i].length;
    XdmcpWriteHeader (&buffer, &header);
    XdmcpWriteARRAYofARRAY8 (&buffer, &AuthenticationNames);


    XdmcpFlush (socketFD, &buffer, (struct sockaddr *)&to, sizeof(to));

    nready = select (SocketsMax + 1, &reads, 0, 0, ptimeout);

    if (nready > 0) {
       addrlen = sizeof (addr);

       if (XdmcpFill (socketFD, &buffer, &addr, &addrlen)) {
	   /*
	    * xdm based display manager is running on remote host..
	    */
	   close(socketFD);
	   return 1;
       }
    }
    else {
	   /*
	    * There is no xdm based display manager running on remote host..
	    */
	   close(socketFD);
	   return 0;
    }
}
