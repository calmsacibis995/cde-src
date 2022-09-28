/* $XConsortium: motif.c /main/cde1_maint/6 1995/11/27 18:12:25 gtsang $ */
/*                                                                      *
 *  motif.c                                                             *
 *   Contains the user interface portion of the Desktop                 *
 *   Calculator.                                                        *
 *                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc. 				*
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/stat.h>
#include "calctool.h"
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>

#ifdef hpux
#include <X11/XHPlib.h>
#include <X11/Xutil.h>
#endif

#include <Xm/XmP.h>
#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include <Xm/DrawingAP.h>
#include <Xm/MessageB.h>
#include <Xm/RowColumn.h>
#include <Xm/MwmUtil.h>
#include <Xm/ManagerP.h>
#include <Xm/Text.h>
#include <Xm/ColorObj.h>
#include <Xm/MainW.h>
#include <Xm/MenuShell.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/PushBP.h>
#include <Xm/PushBG.h>
#include <Xm/CascadeB.h>
#include <Xm/CascadeBG.h>
#include <Xm/MwmUtil.h>
#include <Xm/Protocols.h>
#include <Xm/VendorSEP.h>
#include <Xm/AtomMgr.h>


#include <X11/CoreP.h>
#include <X11/Core.h>
#include <X11/ShellP.h>
#include <X11/Shell.h>
#include <X11/IntrinsicP.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xatom.h>
#include <Xm/Protocols.h>
#include <X11/keysymdef.h>

#include <Dt/Session.h>
#include <Dt/Dt.h>
#include <Dt/Connect.h>
#include <Dt/FileM.h>
#include <Dt/Indicator.h>
#include <Dt/Lock.h>
#include <Dt/Message.h>
#include <Dt/StandardM.h>
#include <Dt/Wsm.h>
#include <Dt/CommandM.h>
#include <Dt/EnvControl.h>

#include "motif.h"

#include "ds_widget.h"
#include "ds_common.h"
#include "ds_popup.h"
#include "ds_xlib.h"

#include<Xm/DragDrop.h>
Boolean colorSrv;

static XtResource resources[] =
{
   {
     "postMenuBar", "PostMenuBar", XmRBoolean, sizeof (Boolean),
     XtOffset (ApplicationArgsPtr, menuBar), XmRImmediate, (XtPointer) True,
   },

   {
     "accuracy", "Accuracy", XmRInt, sizeof (int),
     XtOffset (ApplicationArgsPtr, accuracy), XmRImmediate, (XtPointer) 2,
   },

   {
     "base", "Base", XmRString, sizeof (char *),
     XtOffset (ApplicationArgsPtr, base), XmRImmediate, (XtPointer) "decimal",
   },

   {
     "displayNotation", "DisplayNotation", XmRString, sizeof (char *),
     XtOffset (ApplicationArgsPtr, display), XmRImmediate, (XtPointer) "fixed",
   },

   {
     "mode", "Mode", XmRString, sizeof (char *),
     XtOffset (ApplicationArgsPtr, mode), XmRImmediate, (XtPointer)"scientific",
   },

   {
     "trigType", "TrigType", XmRString, sizeof (char *),
     XtOffset (ApplicationArgsPtr, trigType), XmRImmediate, 
     (XtPointer)"degrees",
   },
};

char DTCALC_CLASS_NAME[] = "Dtcalc";

extern char *opts[] ;           /* Command line option strings. */

Widget modeArry[3];
PixelSet    pixels[MAX_NUM_COLORS];
Pixel white_pixel;
Pixel black_pixel;
Boolean BlackWhite = False;

char * dt_path = NULL;

static Widget funBtn = NULL;
static Widget constBtn = NULL;
static Widget accBtn = NULL;
static Widget stoBtn = NULL;
static Widget rclBtn = NULL;
static Widget exchBtn = NULL;

static Atom saveatom ;
static Atom command_atom ;
static Atom wm_state_atom;

Boolean ignore_event = False;
XtIntervalId timerId = NULL;

static int lastArmed[10];
static int countArmed = 0;

/*  Structure used on a save session to see if a dt is iconic  */
typedef struct
{
   int state;
   Window icon;
} WM_STATE;

void menu_proc        P((Widget, XtPointer, XtPointer)) ;
void show_ascii       P((Widget, XtPointer, XtPointer)) ;
void write_cf_value   P((Widget, XtPointer, XtPointer)) ;
void close_cf         P((Widget, XtPointer, XtPointer)) ;
void close_ascii      P((Widget, XtPointer, XtPointer)) ;
void move_cf          P((Widget, XtPointer, XtPointer)) ;
void FocusInCB        P((Widget, XtPointer, XtPointer)) ;
void map_popup        P((Widget, XtPointer, XtPointer)) ;

#ifdef hpux
static int GetKeyboardID           P(()) ;
#endif

static int event_is_keypad         P((XEvent *)) ;
static int get_next_event          P((Widget, int, XEvent *)) ;
static int is_window_showing       P((Widget)) ;

static KeySym keypad_keysym        P((XEvent *)) ;

static void modelineValueChanged   P((Widget, XtPointer, XtPointer)) ;
static void dtcalc_kkeyboard_create   P((Widget)) ;
static void dtcalc_kpanel_create      P((Widget)) ;
static void confirm_callback     P((Widget, XtPointer, XtPointer)) ;
static void create_cfframe       P(()) ;
static void create_menu          P((enum menu_type, Widget, int)) ;
static void do_button            P((Widget, XtPointer, XtPointer)) ;
static void do_confirm_notice    P((Widget, char *)) ;
static void do_continue_notice   P((Widget, char *)) ;
static void close_reg            P((Widget, XtPointer, XtPointer)) ;
static void event_proc           P((Widget, XtPointer, XEvent *, Boolean *)) ;
static void frame_interpose      P((Widget, XtPointer, XEvent *, Boolean *)) ;
static void menu_handler         P((Widget, XtPointer, XEvent *, Boolean *)) ;
static void popupHandler         P((Widget, XtPointer, XEvent *, Boolean *)) ;
static void make_mode_frame      P((enum mode_type)) ;
static void new_cf_value         P((Widget, XtPointer, XtPointer)) ;
static void do_memory            P((Widget, XtPointer, XtPointer)) ;
static void switch_mode          P((enum mode_type)) ;
static void update_cf_value      P(()) ;
static void xerror_interpose     P((Display *, XErrorEvent *)) ;

static Widget button_create      P((Widget, int, int, int, int)) ;
static void save_state           P((Widget, XtPointer, XtPointer)) ;
static void SaveSession          P(( char *, char * )) ;
static void setCalcHints         P(()) ;

static char * _DtcalcStripSpaces P(( char * )) ;
extern char **environ ;

extern char *base_str[] ;       /* Strings for each base value. */
extern char *calc_res[] ;       /* Calctool X resources. */
extern char *dtype_str[] ;      /* Strings for each display mode value. */
extern char *lstrs[] ;          /* Labels for various Motif items. */
extern char *mess[] ;           /* Message strings. */
extern char *mode_str[] ;       /* Strings for each mode value. */
extern char *pstrs[] ;          /* Property sheet strings. */
extern char *ttype_str[] ;      /* Strings for each trig type value. */
extern char *vstrs[] ;          /* Various strings. */

extern struct button buttons[] ;           /* Calculator button values. */
extern struct button mode_buttons[] ;      /* Calculator mode button values. */
extern struct menu cmenus[] ;              /* Calculator menus. */
extern struct menu_entry menu_entries[] ;  /* All the menu strings. */

extern Vars v ;                 /* Calctool variables and options. */

char translations_return[] = "<Key>Return:ManagerGadgetSelect()";
static Boolean NoDisplay=False;


Atom compound_text = (Atom) None;

/* added by brianc to re register the textfield as a drop site */

void RegisterDropSite(Widget widget, XtCallbackProc callback)
{

Arg args[20];
Cardinal	n;
Atom targets[10];

n = 0;
XtSetArg(args[n], XmNdropSiteOperations, XmDROP_COPY); 
n++;
XtSetArg(args[n], XmNdropProc, callback);
n++;
targets[0] = compound_text;

XtSetArg(args[n], XmNimportTargets, targets);
n++;
XtSetArg(args[n], XmNnumImportTargets, 1);
n++;
XmDropSiteRegister(widget, args, n);

}


/* this takes the text that was dropped, parses thru it
   and passes the valid chars off as if they were input
   from the keyboard.

*/

void TransferCB(Widget widget,
		XtPointer client_data,
		Atom* selection,
		Atom* type,
		XtPointer value,
		unsigned long Value_length,
		int format)
{
  XmString xmstring;
 size_t x, i,j;
 char dropped[MAX_DIGITS+1],passed[MAX_DIGITS+1],decpt;
 int dec_exists;


 decpt = '.'; 
 dec_exists = 0;


 x = strlen(value);
 j = 0;

 strncpy(dropped, value, MAX_DIGITS);
for( i = 0; i < MAX_DIGITS+1; i ++)
	passed[i] = '\0';


i = 0;
 while (i < x)
 {
	if ((i == 0) && (dropped[i] == '-'))
	{
                passed[j] = dropped[i];
                j++;
        }

	if (dropped[i] == decpt)
	{
		if(dec_exists == 0)
	   {
		     passed[j] = dropped[i];
			j++;
		dec_exists = 1;
	   }else
	   {
		beep();
	   }
 	}
	if(v->base == HEX)
	{
		if(isxdigit(dropped[i])!= 0)
		{
			passed[j] = dropped[i];
			j++;
		}else
			beep();
	}else if(v->base== DEC)
	{
                if(isdigit(dropped[i])!= 0)
                {
                        passed[j] = dropped[i];
                        j++;
                }else
                        beep();


        }else if(v->base == OCT)
	{
		if((isdigit(dropped[i])!= 0) && ((dropped[i] - '0') < 8))
                {
                        passed[j] = dropped[i];
                        j++;
                }else
                        beep();

	}else if(v->base == BIN)
        {
                if((isdigit(dropped[i])!= 0) && ((dropped[i] - '0') < 2))
                {
                        passed[j] = dropped[i];
                        j++;
                }else
                        beep();

	}

	i++; 
 }


  if(*type == compound_text)
  {

	MPstr_to_num(passed, v->base, v->MPdisp_val);
	

	show_display(v->MPdisp_val);


  }

}


void DropCB(Widget widget,
	XtPointer client_data,
	XtPointer call_data)
{
  XmDropProcCallbackStruct* ptr;
  XmDropTransferEntryRec entries[2];
  Arg args[20];
  Cardinal n;

  ptr = (XmDropProcCallbackStruct*) call_data;
  n = 0;

  /* only support DROP_COPY  */
  if(ptr->operation != XmDROP_COPY)
  {
	XtSetArg(args[n], XmNtransferStatus, XmTRANSFER_FAILURE);
	n++;

	XmDropTransferStart(ptr->dragContext, args, n);
	return;
  }


  entries[0].target = compound_text;
  entries[0].client_data = (XtPointer) widget; 

  XtSetArg(args[n], XmNdropTransfers, entries);
  n++;
  XtSetArg(args[n], XmNnumDropTransfers, 1);
  n++;
  XtSetArg(args[n], XmNtransferProc, TransferCB);
  n++;

  XmDropTransferStart(ptr->dragContext, args, n);

}

int
main(argc, argv)
int argc ;
char **argv ;
{
  char bind_home[MAXPATHLEN], **new_environ ;
  Pixmap pixmap;
  int i, j ;

  XtSetLanguageProc(NULL, NULL, NULL);
  _DtEnvControl(DT_ENV_SET);
  signal (SIGHUP, SIG_IGN);
#ifdef __osf__
  signal (SIGFPE, SIG_IGN);
#endif /* __osf__ */

  X       = (XVars) LINT_CAST(calloc(1, sizeof(XObject))) ;
  X->home = getenv("HOME") ;

  XtToolkitInitialize() ;
  X->app = XtCreateApplicationContext() ;

  X->dpy = XtOpenDisplay(X->app, NULL, argv[0], DTCALC_CLASS_NAME, 
                                            NULL, 0, &argc, argv) ;
  if (!X->dpy)
    {
      char *msg, *tmpStr;

      tmpStr = GETMESSAGE(2, 31, "Could not open display.\n");
      msg = XtNewString(tmpStr);
      FPRINTF(stderr, msg) ;
      exit(1) ;
    }

  X->screen = DefaultScreen(X->dpy) ;
  X->root   = RootWindow(X->dpy, X->screen) ;
  white_pixel = WhitePixel(X->dpy, X->screen);
  black_pixel = BlackPixel(X->dpy, X->screen);

  X->kframe = XtVaAppCreateShell(argv[0], DTCALC_CLASS_NAME,
                                 applicationShellWidgetClass,
                                 X->dpy,
                                 XtNiconName,   "dtcalc",
                                 XtNiconPixmap, X->icon,
                                 XtNiconMask, X->iconmask,
                                 NULL) ;
  X->dval  = NULL;
  X->vval  = NULL;
  X->cfval = NULL;
  X->helpMapped = False;

  if (DtInitialize (X->dpy, X->kframe, argv[0], DTCALC_CLASS_NAME) == False)
  {
     /* Fatal Error: could not connect to the messaging system. */
     /* DtInitialize() has already logged an appropriate error msg */
     exit(-1);
  }

  /*  Get the application defined resources */
  XtGetApplicationResources(X->kframe, &application_args, resources, 6, NULL,0);

  v = (Vars) LINT_CAST(calloc(1, sizeof(CalcVars))) ;

  /*  Get the dt path created and initialized  */
  dt_path = _DtCreateDtDirs (X->dpy);

#ifdef hpux
  v->keybdID = GetKeyboardID();
#endif

  init_colors() ;             /* get the pixels for the default colors in DT */
  if(pixels[0].bg == white_pixel || pixels[0].bg == black_pixel)
    BlackWhite = True;

  if(colorSrv && !BlackWhite)
  {
     /* first get the Calculator's Icon */
     pixmap = XmGetPixmap (DefaultScreenOfDisplay(X->dpy), CALC_ICON_NAME, 
                                                 pixels[1].fg, pixels[1].bg);
     if( pixmap != XmUNSPECIFIED_PIXMAP)
         X->icon = pixmap;
     else
         X->icon = XmUNSPECIFIED_PIXMAP;

     /* now let's get the mask for the Calculator */
     pixmap = _DtGetMask (DefaultScreenOfDisplay(X->dpy), CALC_ICON_NAME);
     if( pixmap != XmUNSPECIFIED_PIXMAP)
        X->iconmask = pixmap;
     else
         X->iconmask = XmUNSPECIFIED_PIXMAP;
  }
  else
  {
     /* first get the Calculator's Icon */
     pixmap = XmGetPixmap (DefaultScreenOfDisplay(X->dpy), CALC_ICON_NAME_BM, 
                                                 white_pixel, black_pixel);
     if( pixmap != XmUNSPECIFIED_PIXMAP)
         X->icon = pixmap;
     else
         X->icon = XmUNSPECIFIED_PIXMAP;

     /* now let's get the mask for the Calculator */
     pixmap = _DtGetMask (DefaultScreenOfDisplay(X->dpy), CALC_ICON_NAME_BM);
     if( pixmap != XmUNSPECIFIED_PIXMAP)
        X->iconmask = pixmap;
     else
         X->iconmask = XmUNSPECIFIED_PIXMAP;
  }


  do_dtcalc(argc, argv) ;
  exit(0) ;
/*NOTREACHED*/
}

void
beep()
{
  ds_beep(X->dpy) ;
}


static Widget
button_create(owner, row, column, maxrows, maxcols)
Widget owner ;
int row, column, maxrows, maxcols ;
{
  int n = row * maxcols + column ;
  int val ;
  enum menu_type mtype = buttons[n].mtype ;
  Widget button ;
  XmString lstr ;
  int result;
  Pixel bg;

  get_label(n) ;
  lstr = XmStringCreateSimple(v->pstr) ;

  if(colorSrv)
  {
     if (row < 3)
        bg = pixels[5].bg;
     else if(row >= 3  && row < 6)
        bg = pixels[7].bg;
     else if(((row == 7 && column == 3) || (row == 13 && column == 3)) && 
                                                                   !BlackWhite)
        bg = pixels[0].bg;
     else if((row >= 6  && row < 8) || (row >= 8  && row < 13 && column == 3) ||
                                         (row == 13 && column == 2))
        bg = pixels[6].bg;
     else
        bg = pixels[2].bg;

     button = XtVaCreateManagedWidget(buttons[n].resname,
                      xmPushButtonWidgetClass,
                      owner,
                      XmNtopAttachment,    XmATTACH_POSITION,
                      XmNtopPosition,      row * maxcols,
                      XmNleftAttachment,   XmATTACH_POSITION,
                      XmNleftPosition,     column * maxrows,
                      XmNrightAttachment,  XmATTACH_POSITION,
                      XmNrightPosition,    (column+1) * maxrows,
                      XmNbottomAttachment, XmATTACH_POSITION,
                      XmNbottomPosition,   (row+1) * maxcols,
                      XmNlabelString,      lstr,
                      XmNbackground,       bg,
                      XmNtraversalOn,      TRUE,
                      XmNalignment,        XmALIGNMENT_CENTER,
                      XmNrecomputeSize,    False,
                      XmNnavigationType,   XmNONE,
                      NULL) ;
  }
  else
     button = XtVaCreateManagedWidget(buttons[n].resname,
                      xmPushButtonWidgetClass,
                      owner,
                      XmNtopAttachment,    XmATTACH_POSITION,
                      XmNtopPosition,      row * maxcols,
                      XmNleftAttachment,   XmATTACH_POSITION,
                      XmNleftPosition,     column * maxrows,
                      XmNrightAttachment,  XmATTACH_POSITION,
                      XmNrightPosition,    (column+1) * maxrows,
                      XmNbottomAttachment, XmATTACH_POSITION,
                      XmNbottomPosition,   (row+1) * maxcols,
                      XmNlabelString,      lstr,
                      XmNtraversalOn,      TRUE,
                      XmNalignment,        XmALIGNMENT_CENTER,
                      XmNrecomputeSize,    False,
                      XmNnavigationType,   XmNONE,
                      NULL) ;

  XmStringFree(lstr) ;

  if (mtype != M_NONE) create_menu(mtype, button, n) ;
  val = (v->curwin << 16) + n ;
  XtAddCallback(button, XmNactivateCallback, do_button,  (XtPointer) val) ;
  XtAddCallback(button, XmNhelpCallback,   HelpRequestCB, (XtPointer) val) ;
#ifdef AIX_ILS
  XtAddEventHandler(button, KeyPressMask | KeyReleaseMask,
                    FALSE, event_proc, NULL) ;
#endif /* AIX_ILS */

  if( funBtn == NULL && strcmp(v->pstr, GETMESSAGE(3, 5, "Functions")) == 0)
     funBtn = button;
  else if( constBtn == NULL && strcmp(v->pstr, 
                                      GETMESSAGE(3, 6, "Constants")) == 0)
     constBtn = button;
  else if( accBtn == NULL && strcmp(v->pstr, 
                                    GETMESSAGE(3, 12, "Accuracy")) == 0)
     accBtn = button;
  else if( stoBtn == NULL && strcmp(v->pstr, GETMESSAGE(3, 13, "Store")) == 0)
     stoBtn = button;
  else if( rclBtn == NULL && strcmp(v->pstr, GETMESSAGE(3, 14, "Recall")) == 0)
     rclBtn = button;
  else if( exchBtn == NULL && strcmp(v->pstr, 
                                     GETMESSAGE(3, 15, "Exchange")) == 0)
     exchBtn = button;
     
  return(button) ;
}


static void
dtcalc_initialize_rframe(owner, type) 
Widget owner ;
int type ;
{
  char str[MAXLINE] ;
  int i ;
  Arg args[15];
  XmString label_string;
  Widget sep, button, frame, form;
 
  if(type == MEM)
  {
     if (X->rframe) return ;

     X->rframe = XmCreateFormDialog(owner, "rframe", NULL, 0) ;

     /*  Adjust the decorations for the dialog shell of the dialog  */
     XtSetArg (args[0], XmNmwmFunctions, MWM_FUNC_MOVE);
     XtSetArg (args[1], XmNmwmDecorations, MWM_DECOR_BORDER | MWM_DECOR_TITLE);
     XtSetValues (XtParent(X->rframe), args, 2);

     set_title(FCP_REG, lstrs[(int) L_MEMT]) ;

     frame = XmCreateFrame(X->rframe, "frame", NULL, 0);
     XtManageChild(frame);
     form = (Widget) XmCreateForm(frame, "form", NULL, 0) ;
     XtManageChild(form);

     XtVaSetValues(form,
                   XmNmarginWidth,      5,
                   XmNmarginHeight,     5,
                   XmNallowShellResize, TRUE,
                   XmNdefaultPosition,  FALSE,
                   NULL) ;

     SPRINTF(str, "register%1d", 0) ;
     X->registers[0] = XtVaCreateManagedWidget(str,
                                   xmLabelWidgetClass,
                                   form,
                                   XmNtopAttachment, XmATTACH_FORM,
                                   XmNtopOffset, 2,
                                   XmNleftAttachment, XmATTACH_FORM,
                                   XmNleftOffset, 2,
                                   XmNmarginWidth,      5,
                                   XmNmarginHeight,     1,
                                   NULL) ;

     for (i = 1; i < MAXREGS; i++)
       {
         SPRINTF(str, "register%1d", i) ;
         X->registers[i] = XtVaCreateManagedWidget(str,
                                       xmLabelWidgetClass,
                                       form,
                                       XmNtopAttachment, XmATTACH_WIDGET,
                                       XmNtopWidget,   X->registers[i - 1],
                                       XmNtopOffset, 2,
                                       XmNleftAttachment, XmATTACH_FORM,
                                       XmNleftOffset, 2,
                                       XmNmarginWidth,      5,
                                       XmNmarginHeight,     1,
                                       NULL) ;
       }
       
     XtSetArg (args[0], XmNtopAttachment, XmATTACH_WIDGET);
     XtSetArg (args[1], XmNtopWidget, X->registers[i - 1]);
     XtSetArg (args[2], XmNtopOffset, 3);
     XtSetArg (args[3], XmNleftAttachment, XmATTACH_FORM);
     XtSetArg (args[4], XmNrightAttachment, XmATTACH_FORM);
     sep = XmCreateSeparator(form, "sep", args, 5);
     XtManageChild(sep);

#ifdef AIX_ILS
     label_string = XmStringCreateLocalized (GETMESSAGE(2, 32, "Close") );
#else /* AIX_ILS */
     label_string = XmStringCreateLtoR (GETMESSAGE(2, 32, "Close"), 
                                                     XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
     XtSetArg (args[0], XmNmarginHeight, 0);
     XtSetArg (args[1], XmNmarginWidth, 10);
     XtSetArg (args[2], XmNlabelString, label_string);
     XtSetArg (args[3], XmNtopAttachment, XmATTACH_WIDGET);
     XtSetArg (args[4], XmNtopWidget, sep);
     XtSetArg (args[5], XmNtopOffset, 5);
     XtSetArg (args[6], XmNleftAttachment, XmATTACH_POSITION);
     XtSetArg (args[7], XmNleftPosition, 25);
     XtSetArg (args[8], XmNbottomAttachment, XmATTACH_FORM);
     XtSetArg (args[9], XmNbottomOffset, 5);
     XtSetArg (args[10], XmNshowAsDefault, True);
     button = XmCreatePushButton(form, "button", args, 11);
     XmStringFree(label_string);
     
     XtAddCallback(button, XmNactivateCallback, close_reg, (XtPointer)type) ;

     XtSetArg (args[0], XmNcancelButton, button);
     XtSetArg (args[1], XmNdefaultButton, button);
     XtSetValues (X->rframe, args, 2);

     XtManageChild(button);
  }
  else
  {
     if (X->frframe) return ;
     X->frframe = XmCreateFormDialog(owner, "frframe", NULL, 0) ;

     /*  Adjust the decorations for the dialog shell of the dialog  */
     XtSetArg (args[0], XmNmwmFunctions, MWM_FUNC_MOVE);
     XtSetArg (args[1], XmNmwmDecorations, MWM_DECOR_BORDER | MWM_DECOR_TITLE);
     XtSetValues (XtParent(X->frframe), args, 2);

#ifdef AIX_ILS
     set_title(FCP_FIN_REG, GETMESSAGE(2, 47, lstrs[(int) L_FINMEMT] ));
#else /* AIX_ILS */
     set_title(FCP_FIN_REG, lstrs[(int) L_FINMEMT]) ;
#endif /* AIX_ILS */
    
     frame = XmCreateFrame(X->frframe, "frame", NULL, 0);
     XtManageChild(frame);
     form = (Widget) XmCreateForm(frame, "form", NULL, 0) ;
     XtManageChild(form);

     XtVaSetValues(form,
                   XmNmarginWidth,      5,
                   XmNmarginHeight,     5,
                   XmNallowShellResize, TRUE,
                   XmNdefaultPosition,  FALSE,
                   NULL) ;

     SPRINTF(str, "fregister%1d", 0) ;
     X->fregisters[0] = XtVaCreateManagedWidget(str,
                                   xmLabelWidgetClass,
                                   form,
                                   XmNtopAttachment, XmATTACH_FORM,
                                   XmNtopOffset, 2,
                                   XmNleftAttachment, XmATTACH_FORM,
                                   XmNleftOffset, 2,
                                   XmNmarginWidth,      5,
                                   XmNmarginHeight,     1,
                                   NULL) ;

     for (i = 1; i < FINREGS; i++)
       {
         SPRINTF(str, "fregister%1d", i) ;
         X->fregisters[i] = XtVaCreateManagedWidget(str,
                                       xmLabelWidgetClass,
                                       form,
                                       XmNtopAttachment, XmATTACH_WIDGET,
                                       XmNtopWidget,   X->fregisters[i - 1],
                                       XmNtopOffset, 2,
                                       XmNleftAttachment, XmATTACH_FORM,
                                       XmNleftOffset, 2,
                                       XmNmarginWidth,      5,
                                       XmNmarginHeight,     1,
                                       NULL) ;
       }

     SPRINTF(str, "fregistervals%1d", 0) ;
     X->fregistersvals[0] = XtVaCreateManagedWidget(str,
                                   xmLabelWidgetClass,
                                   form,
                                   XmNtopAttachment, XmATTACH_FORM,
                                   XmNtopOffset, 2,
                                   XmNleftAttachment, XmATTACH_WIDGET,
                                   XmNleftWidget, X->fregisters[0],
                                   XmNleftOffset, 0,
                                   XmNrightAttachment, XmATTACH_FORM,
                                   XmNrightOffset, 2,
                                   XmNmarginWidth,      5,
                                   XmNmarginHeight,     1,
                                   NULL) ;

     for (i = 1; i < FINREGS; i++)
       {
         SPRINTF(str, "fregistervals%1d", i) ;
         X->fregistersvals[i] = XtVaCreateManagedWidget(str,
                                       xmLabelWidgetClass,
                                       form,
                                       XmNtopAttachment, XmATTACH_WIDGET,
                                       XmNtopWidget,   X->fregistersvals[i - 1],
                                       XmNtopOffset, 2,
                                       XmNleftAttachment, XmATTACH_WIDGET,
                                       XmNleftWidget, X->fregisters[i],
                                       XmNleftOffset, 0,
                                       XmNrightAttachment, XmATTACH_FORM,
                                       XmNrightOffset, 2,
                                       XmNmarginWidth,      5,
                                       XmNmarginHeight,     1,
                                       NULL) ;
       }

     XtSetArg (args[0], XmNtopAttachment, XmATTACH_WIDGET);
     XtSetArg (args[1], XmNtopWidget, X->fregisters[i - 1]);
     XtSetArg (args[2], XmNtopOffset, 3);
     XtSetArg (args[3], XmNleftAttachment, XmATTACH_FORM);
     XtSetArg (args[4], XmNrightAttachment, XmATTACH_FORM);
     sep = XmCreateSeparator(form, "sep", args, 5);
     XtManageChild(sep);

#ifdef AIX_ILS
     label_string = XmStringCreateLocalized (GETMESSAGE(2, 32, "Close") );
#else /* AIX_ILS */
     label_string = XmStringCreateLtoR (GETMESSAGE(2, 32, "Close"), 
                                                     XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
     XtSetArg (args[0], XmNmarginHeight, 0);
     XtSetArg (args[1], XmNmarginWidth, 10);
     XtSetArg (args[2], XmNlabelString, label_string);
     XtSetArg (args[3], XmNtopAttachment, XmATTACH_WIDGET);
     XtSetArg (args[4], XmNtopWidget, sep);
     XtSetArg (args[5], XmNtopOffset, 5);
     XtSetArg (args[6], XmNleftAttachment, XmATTACH_POSITION);
     XtSetArg (args[7], XmNleftPosition, 30);
     XtSetArg (args[8], XmNbottomAttachment, XmATTACH_FORM);
     XtSetArg (args[9], XmNbottomOffset, 5);
     XtSetArg (args[10], XmNshowAsDefault, True);
     button = XmCreatePushButton(form, "button", args, 11);
     XmStringFree(label_string);

     XtSetArg (args[0], XmNcancelButton, button);
     XtSetArg (args[1], XmNdefaultButton, button);
     XtSetValues (X->frframe, args, 2);

     XtAddCallback(button, XmNactivateCallback, close_reg, NULL) ;

     XtManageChild(button);
  }
}


static void
dtcalc_kkeyboard_create(owner)
Widget owner ;
{
  int column, row ;
  Widget buttonFrame;

  buttonFrame = XtVaCreateManagedWidget("buttonFrame",
                                       xmFrameWidgetClass,
                                       owner,
                                       XmNshadowThickness, 0,
                                       XmNmarginWidth, 0,
                                       XmNmarginHeight, 0,
                                       XmNtopAttachment,      XmATTACH_WIDGET,
                                       XmNtopWidget,          X->modeFrame,
                                       XmNrightAttachment,    XmATTACH_FORM,
                                       XmNleftAttachment,     XmATTACH_FORM,
                                       XmNbottomAttachment,   XmATTACH_FORM,
                                       NULL) ;

  X->kkeyboard = XtVaCreateManagedWidget("kkeyboard",
                                         xmFormWidgetClass,
                                         buttonFrame,
                                         XmNfractionBase, BROWS * BCOLS,
                                         XmNnavigationType,  XmSTICKY_TAB_GROUP,
                                         NULL) ;
 
  v->curwin = FCP_KEY ;
  for (row = 0; row < BROWS; row++)
    for (column = 0; column < BCOLS; column++)
      {  
        X->kbuttons[row][column] = button_create(X->kkeyboard, row, column,
                                                 BROWS, BCOLS) ;
        XtManageChild(X->kbuttons[row][column]) ;
      }  
  grey_buttons(v->base) ;
}


static void
dtcalc_kpanel_create(owner)
Widget owner ;
{
  static char *mnames[] = { "base", "ttype", "num", "hyp",
                            "inv",  "op",  "mode" } ;
  int i, n, val;
  Widget basePulldown, numPulldown, modePulldown, trigPulldown, dummyText;
  Arg args[10];
  XmString label_string;
  Pixel tmp_pixelbg, tmp_pixelfg;
  XtTranslations trans_table;

  trans_table = XtParseTranslationTable(translations_return); 

  if(colorSrv)
  {
    if(BlackWhite)
    {
       if(pixels[2].bg == black_pixel)
       {
          tmp_pixelbg = black_pixel;
          tmp_pixelfg = white_pixel;
       }
       else
       {
          tmp_pixelbg = white_pixel;
          tmp_pixelfg = black_pixel;
       }
    }
    else
    {
       tmp_pixelbg = pixels[6].bg;
       tmp_pixelfg = white_pixel;
    }
  }
  else
  {
    tmp_pixelbg = white_pixel;
    tmp_pixelfg = black_pixel;
  }

  X->mainWin = XtVaCreateManagedWidget("mainWin",
                                      xmMainWindowWidgetClass,
                                      owner,
                                      NULL) ;

  if(application_args.menuBar)
     create_menu_bar(X->mainWin);

  X->kFrame = XtVaCreateManagedWidget("kFrame", xmFrameWidgetClass,
                                      X->mainWin,
                                      XmNshadowThickness, 1,
                                      XmNshadowType, XmSHADOW_OUT,
                                      XmNmarginWidth, 5,
                                      XmNmarginHeight, 5,
                                      NULL) ;

  X->kpanel = XtVaCreateManagedWidget("kpanel",
                                      xmFormWidgetClass,
                                      X->kFrame,
                                      XmNshadowThickness, 0,
                                      XmNbackground,         tmp_pixelbg,
                                      XmNforeground,         tmp_pixelfg,
                                      NULL) ;

  create_popup(X->kpanel);

  X->textFrame = XtVaCreateManagedWidget("textFrame",
                                      xmFrameWidgetClass,
                                      X->kpanel,
                                      XmNshadowThickness, 2,
                                      XmNshadowType, XmSHADOW_IN,
                                      XmNmarginWidth, 0,
                                      XmNmarginHeight, 0,
                                      XmNtopAttachment,      XmATTACH_FORM,
                                      XmNrightAttachment,    XmATTACH_FORM,
                                      XmNleftAttachment,     XmATTACH_FORM,
                                      XmNnavigationType,     XmTAB_GROUP,
                                      NULL) ;
  XtAddCallback(X->textFrame, XmNhelpCallback, HelpRequestCB, 
                                                 (XtPointer)HELP_DISPLAY) ;

  X->textForm = XtVaCreateManagedWidget("textForm",
                                      xmFormWidgetClass,
                                      X->textFrame,
                                      XmNshadowThickness, 0,
                                      XmNbackground,         tmp_pixelbg,
                                      XmNforeground,         tmp_pixelfg,
                                      NULL) ;
  XtAddCallback(X->textForm, XmNhelpCallback, HelpRequestCB, 
                                                 (XtPointer)HELP_DISPLAY) ;

  X->modevals[(int) DISPLAYITEM] = XtVaCreateManagedWidget("display",
                                       xmTextWidgetClass,
                                       X->textForm,
                                       XmNtopAttachment,      XmATTACH_FORM,
                                       XmNrightAttachment,    XmATTACH_FORM,
                                       XmNbottomAttachment,   XmATTACH_FORM,
                                       XmNresizeWidth,        TRUE,
                                       XmNshadowThickness,    0,
                                       XmNhighlightThickness, 0,
                                       XmNeditable,           FALSE,
                                       XmNverifyBell,         FALSE,
                                       XmNbackground,         tmp_pixelbg,
                                       XmNforeground,         tmp_pixelfg,
                                       NULL) ;
  XtAddCallback(X->modevals[(int) DISPLAYITEM], XmNhelpCallback, HelpRequestCB,
                                                    (XtPointer) HELP_DISPLAY) ;


  XmDropSiteUnregister(X->modevals[(int) DISPLAYITEM]);

  compound_text = XmInternAtom(XtDisplay(X->modevals[(int) DISPLAYITEM]), "COMPOUND_TEXT", FALSE);

  RegisterDropSite(X->modevals[(int) DISPLAYITEM], (XtCallbackProc) DropCB);

#ifdef AIX_ILS
  XtAddEventHandler(X->modevals[(int) DISPLAYITEM],
                KeyPressMask | KeyReleaseMask, FALSE, event_proc, NULL) ;
#endif /* AIX_ILS */


  X->modeFrame = XtVaCreateManagedWidget("modeFrame",
                                      xmFrameWidgetClass,
                                      X->kpanel,
                                      XmNshadowThickness, 0,
                                      XmNmarginWidth, 0,
                                      XmNmarginHeight, 0,
                                      XmNtopAttachment,      XmATTACH_WIDGET,
                                      XmNtopWidget,          X->textFrame,
                                      XmNrightAttachment,    XmATTACH_FORM,
                                      XmNleftAttachment,     XmATTACH_FORM,
                                      XmNnavigationType,     XmTAB_GROUP,
                                      NULL) ;
  XtAddCallback(X->modeFrame, XmNhelpCallback,   HelpRequestCB,
                                                   (XtPointer) HELP_MODELINE) ;

  X->modeline = XtVaCreateManagedWidget("modeline",
                    xmFormWidgetClass,
                    X->modeFrame,
                    XmNshadowThickness, 0,
                    NULL) ;
  XtAddCallback(X->modeline, XmNhelpCallback,   HelpRequestCB,
                                                    (XtPointer) HELP_MODELINE) ;

#ifdef AIX_ILS
  label_string = XmStringCreateLocalized ("                   " );
#else /* AIX_ILS */
  label_string = XmStringCreateLtoR ("                   ", 
                                              XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
  i = (int) OPITEM;
  X->modevals[i] = XtVaCreateManagedWidget(mnames[i],
                       xmLabelWidgetClass,
                       X->modeline,
                       XmNtopAttachment, XmATTACH_FORM,
                       XmNrightAttachment, XmATTACH_FORM,
                       XmNrecomputeSize, False,
                       XmNalignment, XmALIGNMENT_CENTER,
                       XmNlabelString, label_string,
                       NULL) ;
  XtAddCallback(X->modevals[i], XmNhelpCallback,   HelpRequestCB,
                                                    (XtPointer) HELP_MODELINE) ;

  i = (int) HYPITEM;
  X->modevals[i] = XtVaCreateManagedWidget(mnames[i],
                       xmLabelWidgetClass,
                       X->modeline,
                       XmNrightAttachment, XmATTACH_WIDGET,
                       XmNrightWidget,  X->modevals[(int) OPITEM],
                       XmNtopAttachment, XmATTACH_FORM,
                       XmNrecomputeSize, False,
                       XmNalignment, XmALIGNMENT_CENTER,
                       XmNlabelString, label_string,
                       NULL) ;
  XtAddCallback(X->modevals[i], XmNhelpCallback,   HelpRequestCB,
                                                    (XtPointer) HELP_MODELINE) ;

  i = (int) INVITEM;
  X->modevals[i] = XtVaCreateManagedWidget(mnames[i],
                       xmLabelWidgetClass,
                       X->modeline,
                       XmNrightAttachment, XmATTACH_WIDGET,
                       XmNrightWidget,  X->modevals[(int) HYPITEM],
                       XmNtopAttachment, XmATTACH_FORM,
                       XmNrecomputeSize, False,
                       XmNalignment, XmALIGNMENT_CENTER,
                       XmNlabelString, label_string,
                       NULL) ;
  XtAddCallback(X->modevals[i], XmNhelpCallback,   HelpRequestCB,
                                                    (XtPointer) HELP_MODELINE) ;
  XmStringFree(label_string);

  {
    i = (int) MODEITEM;
    modePulldown = XmCreatePulldownMenu(X->modeline, "modePD", args, 0);

    XtSetArg(args[0], XmNmarginHeight, 0);
    XtSetArg(args[1], XmNmarginWidth, 0);
    val = (v->curwin << 16) +
             menu_entries[cmenus[(int) M_MODE].mindex + 1].val ;
#ifdef AIX_ILS
    label_string = XmStringCreateLocalized ( GETMESSAGE(2, 9, "Financial") );
#else /* AIX_ILS */
    label_string = XmStringCreateLtoR ((GETMESSAGE(2, 9, "Financial")),
                                                     XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
    XtSetArg(args[2], XmNlabelString, label_string);
    XtSetArg(args[3], XmNuserData, val);
    modeArry[0] = XmCreatePushButtonGadget(modePulldown, "fin", args, 4);
    XmStringFree(label_string);
    XtAddCallback(modeArry[0], XmNactivateCallback, modelineValueChanged,
                                                          (XtPointer) M_MODE);

    val = (v->curwin << 16) +
             menu_entries[cmenus[(int) M_MODE].mindex + 2].val ;
#ifdef AIX_ILS
    label_string = XmStringCreateLocalized ( GETMESSAGE(2, 10, "Logical") );
#else /* AIX_ILS */
    label_string = XmStringCreateLtoR ((GETMESSAGE(2, 10, "Logical")),
                                                     XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
    XtSetArg(args[2], XmNlabelString, label_string);
    XtSetArg(args[3], XmNuserData, val);
    modeArry[1] = XmCreatePushButtonGadget(modePulldown, "logic", args, 4);
    XmStringFree(label_string);
    XtAddCallback(modeArry[1], XmNactivateCallback, modelineValueChanged,
                                                          (XtPointer) M_MODE);

    val = (v->curwin << 16) +
             menu_entries[cmenus[(int) M_MODE].mindex + 3].val ;
#ifdef AIX_ILS
    label_string = XmStringCreateLocalized ( GETMESSAGE(2, 11, "Scientific") );
#else /* AIX_ILS */
    label_string = XmStringCreateLtoR ((GETMESSAGE(2, 11, "Scientific")),
                                                     XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
    XtSetArg(args[2], XmNlabelString, label_string);
    XtSetArg(args[3], XmNuserData, val);
    modeArry[2] = XmCreatePushButtonGadget(modePulldown, "Sci", args, 4);
    XmStringFree(label_string);
    XtAddCallback(modeArry[2], XmNactivateCallback, modelineValueChanged,
                                                          (XtPointer) M_MODE);

    XtManageChildren(modeArry, 3);

    /* create the Option Menu and attach it to the Pulldown MenuPane */
    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);            n++;
    XtSetArg (args[n], XmNtopWidget, X->modevals[(int)HYPITEM]);      n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM);           n++;
    XtSetArg(args[n], XmNsubMenuId, modePulldown);                    n++;
    XtSetArg(args[n], XmNmenuHistory, modeArry[(int)v->modetype]);    n++;
    X->modevals[i] = XmCreateOptionMenu(X->modeline, "mode", args, n);
    XtManageChild (X->modevals[i]);
    XtAddCallback(X->modevals[i], XmNhelpCallback, HelpRequestCB,
                                                       (XtPointer) HELP_MODE) ;

    XtOverrideTranslations(X->modevals[i], trans_table);

#ifdef AIX_ILS
    XtAddEventHandler(X->modevals[i], KeyPressMask | KeyReleaseMask,
		      FALSE, event_proc, NULL) ;
#endif /* AIX_ILS */

    i = (int) BASEITEM;
    basePulldown = XmCreatePulldownMenu(X->modeline, "basePD", args, 0);

    val = (v->curwin << 16) +
             menu_entries[cmenus[(int) M_BASE].mindex].val ;
#ifdef AIX_ILS
    label_string = XmStringCreateLocalized ( GETMESSAGE(2, 1, "Bin") );
#else /* AIX_ILS */
    label_string = XmStringCreateLtoR ((GETMESSAGE(2, 1, "Bin")),
                                                     XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
    XtSetArg(args[0], XmNmarginHeight, 0);
    XtSetArg(args[1], XmNmarginWidth, 0);
    XtSetArg(args[2], XmNlabelString, label_string);
    XtSetArg(args[3], XmNuserData, val);
    X->baseWidgArry[0] = XmCreatePushButtonGadget(basePulldown, "bin", args, 4);
    XmStringFree(label_string);
    XtAddCallback(X->baseWidgArry[0], XmNactivateCallback, 
                         modelineValueChanged, (XtPointer) M_BASE);

    val = (v->curwin << 16) +
             menu_entries[cmenus[(int) M_BASE].mindex + 1].val ;
#ifdef AIX_ILS
    label_string = XmStringCreateLocalized ( GETMESSAGE(2, 2, "Oct") );
#else /* AIX_ILS */
    label_string = XmStringCreateLtoR ((GETMESSAGE(2, 2, "Oct")),
                                                     XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
    XtSetArg(args[2], XmNlabelString, label_string);
    XtSetArg(args[3], XmNuserData, val);
    X->baseWidgArry[1] = XmCreatePushButtonGadget(basePulldown, "oct", args, 4);
    XmStringFree(label_string);
    XtAddCallback(X->baseWidgArry[1], XmNactivateCallback, 
                         modelineValueChanged, (XtPointer) M_BASE);

    val = (v->curwin << 16) +
             menu_entries[cmenus[(int) M_BASE].mindex + 2].val ;
#ifdef AIX_ILS
    label_string = XmStringCreateLocalized ( GETMESSAGE(2, 3, "Dec") );
#else /* AIX_ILS */
    label_string = XmStringCreateLtoR ((GETMESSAGE(2, 3, "Dec")),
                                                     XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
    XtSetArg(args[2], XmNlabelString, label_string);
    XtSetArg(args[3], XmNuserData, val);
    X->baseWidgArry[2] = XmCreatePushButtonGadget(basePulldown, "dec", args, 4);
    XmStringFree(label_string);
    XtAddCallback(X->baseWidgArry[2], XmNactivateCallback, 
                         modelineValueChanged, (XtPointer) M_BASE);

    val = (v->curwin << 16) +
             menu_entries[cmenus[(int) M_BASE].mindex + 3].val ;
#ifdef AIX_ILS
    label_string = XmStringCreateLocalized ( GETMESSAGE(2, 4, "Hex") );
#else /* AIX_ILS */
    label_string = XmStringCreateLtoR ((GETMESSAGE(2, 4, "Hex")),
                                                     XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
    XtSetArg(args[2], XmNlabelString, label_string);
    XtSetArg(args[3], XmNuserData, val);
    X->baseWidgArry[3] = XmCreatePushButtonGadget(basePulldown, "hex", args, 4);
    XmStringFree(label_string);
    XtAddCallback(X->baseWidgArry[3], XmNactivateCallback, 
                         modelineValueChanged, (XtPointer) M_BASE);

    XtManageChildren(X->baseWidgArry, 4);

    /* create the Option Menu and attach it to the Pulldown MenuPane */
    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);            n++;
    XtSetArg (args[n], XmNtopWidget, X->modevals[(int)HYPITEM]);      n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM);           n++;
    XtSetArg(args[n], XmNsubMenuId, basePulldown);                    n++;
    XtSetArg(args[n], XmNmenuHistory, X->baseWidgArry[(int)v->base]); n++;
    X->modevals[i] = XmCreateOptionMenu(X->modeline, "base", args, n);
    XtManageChild (X->modevals[i]);
    XtAddCallback(X->modevals[i], XmNhelpCallback, HelpRequestCB,
                                                      (XtPointer) HELP_BASE) ;

    XtOverrideTranslations(X->modevals[i], trans_table);
#ifdef AIX_ILS
  XtAddEventHandler(X->modevals[i], KeyPressMask | KeyReleaseMask,
                    FALSE, event_proc, NULL) ;
#endif /* AIX_ILS */


    i = (int) NUMITEM;
    numPulldown = XmCreatePulldownMenu(X->modeline, "numPD", args, 0);

    val = (v->curwin << 16) +
             menu_entries[cmenus[(int) M_NUM].mindex].val ;
#ifdef AIX_ILS
    label_string = XmStringCreateLocalized ( GETMESSAGE(2, 5, "Eng") );
#else /* AIX_ILS */
    label_string = XmStringCreateLtoR ((GETMESSAGE(2, 5, "Eng")),
                                                     XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
    XtSetArg(args[0], XmNmarginHeight, 0);
    XtSetArg(args[1], XmNmarginWidth, 0);
    XtSetArg(args[2], XmNlabelString, label_string);
    XtSetArg(args[3], XmNuserData, val);
    X->numWidgArry[0] = XmCreatePushButtonGadget(numPulldown, "eng", args, 4);
    XmStringFree(label_string);
    XtAddCallback(X->numWidgArry[0], XmNactivateCallback, modelineValueChanged, 
                                                          (XtPointer) M_NUM);

    val = (v->curwin << 16) +
             menu_entries[cmenus[(int) M_NUM].mindex + 1].val ;
#ifdef AIX_ILS
    label_string = XmStringCreateLocalized ( GETMESSAGE(2, 6, "Fix") );
#else /* AIX_ILS */
    label_string = XmStringCreateLtoR ((GETMESSAGE(2, 6, "Fix")),
                                                     XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
    XtSetArg(args[2], XmNlabelString, label_string);
    XtSetArg(args[3], XmNuserData, val);
    X->numWidgArry[1] = XmCreatePushButtonGadget(numPulldown, "fix", args, 4);
    XmStringFree(label_string);
    XtAddCallback(X->numWidgArry[1], XmNactivateCallback, modelineValueChanged, 
                                                          (XtPointer) M_NUM);

    val = (v->curwin << 16) +
             menu_entries[cmenus[(int) M_NUM].mindex + 2].val ;
#ifdef AIX_ILS
    label_string = XmStringCreateLocalized ( GETMESSAGE(2, 7, "Sci") );
#else /* AIX_ILS */
    label_string = XmStringCreateLtoR ((GETMESSAGE(2, 7, "Sci")),
                                                     XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
    XtSetArg(args[2], XmNlabelString, label_string);
    XtSetArg(args[3], XmNuserData, val);
    X->numWidgArry[2] = XmCreatePushButtonGadget(numPulldown, "sci", args, 4);
    XmStringFree(label_string);
    XtAddCallback(X->numWidgArry[2], XmNactivateCallback, modelineValueChanged, 
                                                          (XtPointer) M_NUM);

    XtManageChildren(X->numWidgArry, 3);

    /* create the Option Menu and attach it to the Pulldown MenuPane */
    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);             n++;
    XtSetArg (args[n], XmNtopWidget, X->modevals[(int)HYPITEM]);       n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM);            n++;
    XtSetArg(args[n], XmNsubMenuId, numPulldown);                      n++;
    XtSetArg(args[n], XmNmenuHistory, X->numWidgArry[(int)v->dtype]);  n++;
    X->modevals[i] = XmCreateOptionMenu(X->modeline, "num", args, n);
    XtManageChild (X->modevals[i]);
    XtAddCallback(X->modevals[i], XmNhelpCallback, HelpRequestCB,
                                                   (XtPointer) HELP_NOTATION) ;

    XtOverrideTranslations(X->modevals[i], trans_table);
#ifdef AIX_ILS
  XtAddEventHandler(X->modevals[i], KeyPressMask | KeyReleaseMask,
                    FALSE, event_proc, NULL) ;
#endif /* AIX_ILS */


    i = (int) TTYPEITEM;
    trigPulldown = XmCreatePulldownMenu(X->modeline, "trigPD", args, 0);

    XtSetArg(args[0], XmNmarginHeight, 0);
    XtSetArg(args[1], XmNmarginWidth, 0);
    val = (v->curwin << 16) +
             menu_entries[cmenus[(int) M_TRIG].mindex].val ;
#ifdef AIX_ILS
    label_string = XmStringCreateLocalized ( ttype_str[(int) DEG] );
#else /* AIX_ILS */
    label_string = XmStringCreateLtoR ( ttype_str[(int) DEG],
                                                     XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
    XtSetArg(args[2], XmNlabelString, label_string);
    XtSetArg(args[3], XmNuserData, val);
    X->ttypeWidgArry[0] =
                     XmCreatePushButtonGadget(trigPulldown, "deg", args, 4);
    XmStringFree(label_string);
    XtAddCallback(X->ttypeWidgArry[0], XmNactivateCallback, 
                         modelineValueChanged, (XtPointer) M_TRIG);

    val = (v->curwin << 16) +
             menu_entries[cmenus[(int) M_TRIG].mindex + 1].val ;
#ifdef AIX_ILS
    label_string = XmStringCreateLocalized ( ttype_str[(int) GRAD] );
#else /* AIX_ILS */
    label_string = XmStringCreateLtoR ( ttype_str[(int) GRAD],
                                                     XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
    XtSetArg(args[2], XmNlabelString, label_string);
    XtSetArg(args[3], XmNuserData, val);
    X->ttypeWidgArry[1] = 
                XmCreatePushButtonGadget(trigPulldown, "grd", args, 4);
    XmStringFree(label_string);
    XtAddCallback(X->ttypeWidgArry[1], XmNactivateCallback,
                          modelineValueChanged, (XtPointer) M_TRIG);

    val = (v->curwin << 16) +
             menu_entries[cmenus[(int) M_TRIG].mindex + 2].val ;
#ifdef AIX_ILS
    label_string = XmStringCreateLocalized ( ttype_str[(int) RAD] );
#else /* AIX_ILS */
    label_string = XmStringCreateLtoR ( ttype_str[(int) RAD],
                                                     XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
    XtSetArg(args[2], XmNlabelString, label_string);
    XtSetArg(args[3], XmNuserData, val);
    X->ttypeWidgArry[2] = 
                 XmCreatePushButtonGadget(trigPulldown, "rad", args, 4);
    XmStringFree(label_string);
    XtAddCallback(X->ttypeWidgArry[2], XmNactivateCallback,
                           modelineValueChanged, (XtPointer) M_TRIG);

    XtManageChildren(X->ttypeWidgArry, 3);

    /* create the Option Menu and attach it to the Pulldown MenuPane */
    n = 0;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);              n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);              n++;
    XtSetArg (args[n], XmNtopWidget, X->modevals[(int) HYPITEM]);       n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM);             n++;
    XtSetArg(args[n], XmNsubMenuId, trigPulldown);                      n++;
    XtSetArg(args[n], XmNmenuHistory, X->ttypeWidgArry[(int)v->ttype]); n++;
    X->modevals[i] = XmCreateOptionMenu(X->modeline, "trig", args, n);
    XtManageChild (X->modevals[i]);
    XtAddCallback(X->modevals[i], XmNhelpCallback, HelpRequestCB,
                                                      (XtPointer) HELP_TRIG) ;

    XtOverrideTranslations(X->modevals[i], trans_table);
    XtSetSensitive(X->modevals[i], True);

    XtSetArg (args[0], XmNrightAttachment, XmATTACH_WIDGET);
    XtSetArg (args[1], XmNrightWidget, X->modevals[(int)BASEITEM]);
    XtSetValues (X->modevals[(int)MODEITEM], args, 2);

    XtSetArg (args[0], XmNrightAttachment, XmATTACH_WIDGET);
    XtSetArg (args[1], XmNrightWidget, X->modevals[(int)NUMITEM]);
    XtSetValues (X->modevals[(int)BASEITEM], args, 2);

    XtSetArg (args[0], XmNrightAttachment, XmATTACH_WIDGET);
    XtSetArg (args[1], XmNrightWidget, X->modevals[(int)TTYPEITEM]);
    XtSetValues (X->modevals[(int)NUMITEM], args, 2);


#ifdef AIX_ILS
  XtAddEventHandler(X->modevals[i], KeyPressMask | KeyReleaseMask,
                    FALSE, event_proc, NULL) ;
#endif /* AIX_ILS */

  }
}

/*ARGSUSED*/
static void
confirm_callback(widget, client_data, call_data)
Widget widget ;
XtPointer client_data, call_data ;
{
   update_cf_value();
}


static void
create_cfframe()    /* Create auxiliary frame for CON/FUN key. */
{
  XmString tstr ;
  Arg args[15];
  Widget sep, frame, form;
  XmString label_string;

  X->CFframe = (Widget) XmCreateFormDialog(X->kframe, "cfframe", NULL, 0) ;
  tstr = XmStringCreateSimple(lstrs[(int) L_NEWCON]) ;
  XtVaSetValues(X->CFframe,
                XmNdialogTitle,     tstr,
                XmNautoUnmanage,    FALSE,
                XmNallowShellResize, FALSE,
                XmNdefaultPosition, FALSE,
                NULL) ;
  XmStringFree(tstr) ;

  /*  Adjust the decorations for the dialog shell of the dialog  */
  XtSetArg (args[0], XmNmwmFunctions, MWM_FUNC_MOVE);
  XtSetArg (args[1], XmNmwmDecorations, MWM_DECOR_BORDER | MWM_DECOR_TITLE);
  XtSetValues (XtParent(X->CFframe), args, 2);

  frame = (Widget) XmCreateFrame(X->CFframe, "frame", NULL, 0) ;
  XtManageChild(frame);
  form = (Widget) XmCreateForm(frame, "form", NULL, 0) ;
  XtManageChild(form);


  X->CFpi_cftext = make_textW(form, lstrs[(int) L_CONNO]) ;
  X->CFpi_dtext  = make_textW(form, lstrs[(int) L_DESC]) ;
  X->CFpi_vtext  = make_textW(form, lstrs[(int) L_VALUE]) ;

  XtSetArg (args[0], XmNtopAttachment, XmATTACH_FORM);
  XtSetArg (args[1], XmNtopOffset, 5);
  XtSetArg (args[2], XmNrightAttachment, XmATTACH_FORM);
  XtSetArg (args[3], XmNrightOffset, 5);
  XtSetValues(X->CFpi_cftext->manager, args, 4);

  XtSetArg (args[0], XmNtopAttachment, XmATTACH_WIDGET);
  XtSetArg (args[1], XmNtopWidget, X->CFpi_cftext->manager);
  XtSetArg (args[2], XmNtopOffset, 5);
  XtSetArg (args[3], XmNrightAttachment, XmATTACH_FORM);
  XtSetArg (args[4], XmNrightOffset, 5);
  XtSetValues(X->CFpi_dtext->manager, args, 5);
 
  XtSetArg (args[0], XmNtopAttachment, XmATTACH_WIDGET);
  XtSetArg (args[1], XmNtopWidget, X->CFpi_dtext->manager);
  XtSetArg (args[2], XmNtopOffset, 5);
  XtSetArg (args[3], XmNrightAttachment, XmATTACH_FORM);
  XtSetArg (args[4], XmNrightOffset, 5);
  XtSetValues(X->CFpi_vtext->manager, args, 5);
 
  XtSetArg (args[0], XmNtopAttachment, XmATTACH_WIDGET);
  XtSetArg (args[1], XmNtopWidget, X->CFpi_vtext->manager);
  XtSetArg (args[2], XmNtopOffset, 3);
  XtSetArg (args[3], XmNleftAttachment, XmATTACH_FORM);
  XtSetArg (args[4], XmNrightAttachment, XmATTACH_FORM);
  sep = XmCreateSeparator(form, "sep", args, 5);
  XtManageChild(sep);

#ifdef AIX_ILS
  label_string = XmStringCreateLocalized ( GETMESSAGE(4, 4, "OK") );
#else /* AIX_ILS */
  label_string = XmStringCreateLtoR (GETMESSAGE(4, 4, "OK"), 
                                                XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
  XtSetArg (args[0], XmNmarginHeight, 2);
  XtSetArg (args[1], XmNmarginWidth, 15);
  XtSetArg (args[2], XmNlabelString, label_string);
  XtSetArg (args[3], XmNtopAttachment, XmATTACH_WIDGET);
  XtSetArg (args[4], XmNtopWidget, sep);
  XtSetArg (args[5], XmNtopOffset, 5);
  XtSetArg (args[6], XmNleftAttachment, XmATTACH_FORM);
  XtSetArg (args[7], XmNleftOffset, 15);
  XtSetArg (args[8], XmNbottomAttachment, XmATTACH_FORM);
  XtSetArg (args[9], XmNbottomOffset, 5);
  XtSetArg (args[10], XmNshowAsDefault, True);
  XtSetArg (args[11], XmNtraversalOn, True);
  X->CFpi_butOK = XmCreatePushButton(form, "button", args, 12);
  XmStringFree(label_string);
  XtManageChild(X->CFpi_butOK);

#ifdef AIX_ILS
  label_string = XmStringCreateLocalized ( GETMESSAGE(3, 361, "Cancel") );
#else /* AIX_ILS */
  label_string = XmStringCreateLtoR (GETMESSAGE(3, 361, "Cancel"), 
                                                 XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
  XtSetArg (args[0], XmNmarginHeight, 2);
  XtSetArg (args[1], XmNmarginWidth, 10);
  XtSetArg (args[2], XmNlabelString, label_string);
  XtSetArg (args[3], XmNtopAttachment, XmATTACH_WIDGET);
  XtSetArg (args[4], XmNtopWidget, sep);
  XtSetArg (args[5], XmNtopOffset, 10);
  XtSetArg (args[6], XmNleftAttachment, XmATTACH_WIDGET);
  XtSetArg (args[7], XmNleftWidget, X->CFpi_butOK);
  XtSetArg (args[8], XmNleftOffset, 33);
  XtSetArg (args[9], XmNbottomAttachment, XmATTACH_FORM);
  XtSetArg (args[10], XmNbottomOffset, 10);
  XtSetArg (args[11], XmNtraversalOn, True);
  X->CFpi_butClose = XmCreatePushButton(form, "button", args, 12);
  XmStringFree(label_string);
  XtManageChild(X->CFpi_butClose);

#ifdef AIX_ILS
  label_string = XmStringCreateLocalized ( GETMESSAGE(2, 47, "Help") );
#else /* AIX_ILS */
  label_string = XmStringCreateLtoR (GETMESSAGE(2, 47, "Help"), 
                                                   XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
  XtSetArg (args[0], XmNmarginHeight, 2);
  XtSetArg (args[1], XmNmarginWidth, 10);
  XtSetArg (args[2], XmNlabelString, label_string);
  XtSetArg (args[3], XmNtopAttachment, XmATTACH_WIDGET);
  XtSetArg (args[4], XmNtopWidget, sep);
  XtSetArg (args[5], XmNtopOffset, 10);
  XtSetArg (args[6], XmNrightAttachment, XmATTACH_FORM);
  XtSetArg (args[7], XmNrightOffset, 15);
  XtSetArg (args[8], XmNleftAttachment, XmATTACH_WIDGET);
  XtSetArg (args[9], XmNleftWidget, X->CFpi_butClose);
  XtSetArg (args[10], XmNleftOffset, 33);
  XtSetArg (args[11], XmNbottomAttachment, XmATTACH_FORM);
  XtSetArg (args[12], XmNbottomOffset, 10);
  XtSetArg (args[13], XmNtraversalOn, True);
  X->CFpi_butHelp = XmCreatePushButton(form, "button", args, 14);
  XmStringFree(label_string);
  XtManageChild(X->CFpi_butHelp);

  XtAddCallback(X->CFpi_butOK, XmNactivateCallback, write_cf_value,
                                                   (XtPointer) X->CFpi_butOK) ;
  XtAddCallback(X->CFpi_butClose, XmNactivateCallback, close_cf,
                                                          (XtPointer) NULL) ;

  XtAddCallback(X->CFpi_cftext->textfield, XmNactivateCallback, move_cf,
                                                   (XtPointer) X->CFpi_butOK) ;
  XtAddCallback(X->CFpi_dtext->textfield, XmNactivateCallback, move_cf,
                                                   (XtPointer) X->CFpi_butOK) ;
  XtAddCallback(X->CFpi_vtext->textfield, XmNactivateCallback, move_cf,
                                                   (XtPointer) X->CFpi_butOK) ;

  XtAddCallback(X->CFpi_cftext->textfield, XmNfocusCallback, FocusInCB,
                                                   (XtPointer) NULL) ;
  XtAddCallback(X->CFpi_dtext->textfield, XmNactivateCallback, FocusInCB,
                                                   (XtPointer) NULL) ;

  XtSetArg (args[0], XmNcancelButton, X->CFpi_butClose);
  XtSetValues (X->CFframe, args, 1);

  /* Make the first prompt automatically get the focus. */
   XmProcessTraversal(X->CFpi_cftext->textfield, XmTRAVERSE_CURRENT);
}


static void
create_menu(mtype, button, n)   /* Create popup menu for dtcalc button. */
enum menu_type mtype ;
Widget button ;
int n ;
{
  char *mstr, *tmpStr, *ptr ;
  int i, invalid, val ;
  Widget menu, mi ;
  Boolean isAFunction = False;

  if ((mtype != M_CON && mtype != M_FUN) &&
      X->menus[(int) mtype] != NULL) return ;       /* Already created? */

  X->menus[(int) mtype] = menu = XmCreatePopupMenu(button,
                                                   "menu", NULL, 0) ;

  X->mrec[(int) mtype] = n ;

  XtCreateManagedWidget(cmenus[(int) mtype].title, xmLabelWidgetClass,
                        menu, NULL, 0) ;
  XtCreateManagedWidget("separator", xmSeparatorWidgetClass, menu, NULL, 0) ;
  XtCreateManagedWidget("separator", xmSeparatorWidgetClass, menu, NULL, 0) ;


  for (i = 0; i < cmenus[(int) mtype].total; i++)
    {
      invalid = 0 ;
      switch (mtype)
        {
          case M_CON : if (strlen(v->con_names[i]))        /* Constants. */
                         mstr = v->con_names[i] ;
                       else invalid = 1 ;
                       break ;
          case M_FUN : if (strlen(v->fun_names[i]))        /* Functions. */
                       {
                         isAFunction = True;
                         mstr = v->fun_names[i] ;
                       }
                       else invalid = 1 ;
                       break ;
          default    : mstr = menu_entries[cmenus[(int) mtype].mindex + i].str ;
        }
      if (!invalid)
        {
           
           tmpStr = GETMESSAGE(3, 45, ".");
           if(strcmp(tmpStr, ".") != 0)
           {
              /* if it's not a "." let's go change it to what it should be */
              ptr = DtStrchr(mstr, '.');
              if(ptr != NULL)
                 ptr[0] = tmpStr[0];
           }
          mi = XtCreateManagedWidget(mstr, xmPushButtonWidgetClass,
                                     menu, NULL, 0) ;
          val = (v->curwin << 16) +
                menu_entries[cmenus[(int) mtype].mindex + i].val ;
          XtAddCallback(mi, XmNactivateCallback, menu_proc, (XtPointer) val) ;
        }
    }
    if( !isAFunction && mtype == M_FUN)
    {
       mi = XtCreateManagedWidget(GETMESSAGE(2, 35, "No Functions Defined"),
                                      xmPushButtonWidgetClass, menu, NULL, 0) ;
    }
}


/*ARGSUSED*/
static void
do_button(widget, client_data, call_data)
Widget widget ;
XtPointer client_data, call_data ;
{
  char *str;
  XmString cstr ;
  Arg args[3];
  int n = ((int) client_data) & 0xFFFF ;

  XtSetArg(args[0], XmNlabelString, &cstr);
  XtGetValues(X->modevals[OPITEM], args, 1);

  XmStringGetLtoR (cstr, XmFONTLIST_DEFAULT_TAG, &str);

  if(strcmp(vstrs[(int) V_CLR], str) == 0 &&
              (buttons[n].value !=  KEY_CLR && buttons[n].value !=  KEY_QUIT))
  {
     beep();
     return;
  }

  v->curwin = ((int) client_data) >> 16 ;
  if (v->pending)
    {
      v->current = buttons[n].value ;
      do_pending() ;
    }
  else if (n >= 0 && n <= NOBUTTONS) process_item(n) ;
  v->error = FALSE;
}


/*ARGSUSED*/
static void
do_continue_notice(parent, str)
Widget parent ;
char *str ;
{
  XmString contstr, message, cstr ;
  char *tmpStr;

  X->notice = (Widget) XmCreateInformationDialog(X->kframe, "continue",
                                                 NULL, 0) ;
  contstr = XmStringCreateSimple(vstrs[(int) V_CONTINUE]) ;
#ifdef AIX_ILS
  message = XmStringCreateLocalized( str );
#else /* AIX_ILS */
  message = XmStringCreateLtoR(str, XmFONTLIST_DEFAULT_TAG) ;
#endif /* AIX_ILS */
  XtVaSetValues(X->notice,
                XmNautoUnmanage,      FALSE,
                XmNcancelLabelString, contstr,
                XmNmessageString,     message,
                XmNdialogStyle,       XmDIALOG_FULL_APPLICATION_MODAL,
                0) ;
  XmStringFree(contstr) ;
  XmStringFree(message) ;
  XtUnmanageChild((Widget) XmMessageBoxGetChild(X->notice,
                                                XmDIALOG_OK_BUTTON)) ;
  XtUnmanageChild((Widget) XmMessageBoxGetChild(X->notice,
                                                XmDIALOG_HELP_BUTTON)) ;

  tmpStr = GETMESSAGE(2, 36, "Continue Notice");
  cstr = XmStringCreateSimple(tmpStr) ;
  XtVaSetValues(X->notice, XmNdialogTitle, cstr, 0) ;
  XmStringFree(cstr) ;

  XtManageChild(X->notice) ;
}


/*ARGSUSED*/
static void
do_confirm_notice(parent, str)
Widget parent ;
char *str ;
{
  XmString confirm, cancel, message, cstr ;
  char *tmpStr;


  X->notice = (Widget) XmCreateInformationDialog(X->kframe, "continue",
                                                    NULL, 0) ;
  confirm = XmStringCreateSimple(vstrs[(int) V_CONFIRM]) ;
  cancel  = XmStringCreateSimple(vstrs[(int) V_CANCEL]) ;
#ifdef AIX_ILS
  message = XmStringCreateLocalized( str );
#else /* AIX_ILS */
  message = XmStringCreateLtoR(str, XmFONTLIST_DEFAULT_TAG) ;
#endif /* AIX_ILS */
  XtVaSetValues(X->notice,
                XmNautoUnmanage,      FALSE,
                XmNcancelLabelString, cancel,
                XmNokLabelString,     confirm,
                XmNmessageString,     message,
                XmNdialogStyle,       XmDIALOG_APPLICATION_MODAL,
                0) ;
  XmStringFree(confirm) ;
  XmStringFree(cancel) ;
  XmStringFree(message) ;
  XtUnmanageChild((Widget) XmMessageBoxGetChild(X->notice,
                                                XmDIALOG_HELP_BUTTON)) ;
  XtAddCallback(X->notice, XmNokCallback, confirm_callback, NULL) ;

  tmpStr = GETMESSAGE(2, 37, "Confirmation Notice");
  cstr = XmStringCreateSimple(tmpStr) ;
  XtVaSetValues(X->notice, XmNdialogTitle, cstr, 0) ;
  XmStringFree(cstr) ;

  XtManageChild(X->notice) ;

}


/*ARGSUSED*/
static void
close_reg(widget, client_data, call_data)
Widget widget ;
XtPointer client_data, call_data ;
{
  int type = (int)client_data;

  if(type == MEM)
  {
     XtUnmanageChild(X->rframe);
     XtUnmapWidget(XtParent(X->rframe));
     v->rstate = 0 ;
  }
  else
  {
     XtUnmanageChild(X->frframe);
     XtUnmapWidget(XtParent(X->frframe));
     v->frstate = 0 ;
  }
}


/*ARGSUSED*/
/* draw_button()
 *  This routine causes the calculator buttons to looked depressed (armed)
 *  and undepressed (disarmed) when the user uses the keyboard to select a
 *  button on the keyboard.  It's pretty straight forward:  
 *    The user pressess a key from the keyboard
 *    The routine event_proc() gets called.  
 *    It determines whether the key from the keyboard is a valid key.
 *    If it is, it sends that key to process_event() 
 *    This routines then determines whether the key is an active key.
 *    If it is then it calls this routine to set the key to either armed
 *     or disarmed.  This state is determined by whether it is a keyboard
 *     up event or a keyboard down event.  
 *  There is some special case code in here also.  There was a bug where
 *  if a user pressed a CNTL character key, the event order could cause
 *  the key to stay depressed.  The typical order is:
 *     Control key down
 *     KEY key down
 *     KEY key up
 *     Control key up
 *  When this typical order was pressed things worked fine.  But the problem
 *  arose when the order was:
 *     Control key down
 *     KEY key down
 *     Control key up
 *     KEY key up
 *  The problem was that when the last "KEY key up" was processed it is not
 *  recognized as a control key, so the calculator thought is was a "normal"
 *  key and disarmed that "normal" key button rather then the Control 
 *  key button.  This is what the lastArmed and count static variables are
 *  used for.  They check to see if the last "KEY key up" corresponds to  
 *  the last "Control key down" button that was armed.  If it doesn't then
 *  we know we need to disarm the "Control key" button rather then the
 *  "normal" button that was passed in. 
 *
 */
void
draw_button(n, fcptype, row, column, inverted)
int n;
enum fcp_type fcptype ;
int row, column, inverted ;
{
  char str[10] ;
  Widget widget ;
  XmPushButtonWidget pb;
  int i, j, row2, column2;
  static XtIntervalId timerId = 0;

  if(inverted)
  {
     widget = X->kbuttons[row][column] ;
     lastArmed[countArmed] = n;
     countArmed++;
  }	  
  else
  {
     if(countArmed == 0)
        return;
     for(i = 0; i < countArmed; i++)
     {
        if(lastArmed[i] == n)
           break;
     }
     if(countArmed == i && countArmed != 0)
        i = 0;
     
     row2 = lastArmed[i] / MAXCOLS ;
     column2 = lastArmed[i] - (row2 * MAXCOLS) ;
     widget = X->kbuttons[row2][column2] ;
     for(j = i; j < countArmed; j++)
        lastArmed[j] = lastArmed[j + 1];
     countArmed--;
  }

  if (inverted) 
     strcpy(str, "Arm");
  else          
     strcpy(str, "Disarm");

  /* go call the correct arm/disarm function from Motif */
  XtCallActionProc(widget, str, X->event, NULL, 0) ;

}


static int
event_is_keypad(xevent)
XEvent *xevent ;
{
  if (xevent->type != KeyPress && xevent->type != KeyRelease) return(0) ;
  return(X->kparray[xevent->xkey.keycode - X->kcmin] > 0) ;
}


/*
 * event_proc()
 *  This routine is called any time a key on the keyboard is pressed.  It 
 *  then determines whether the key is a valid key or whether it is to
 *  be ignore.  If it is a valid key it passes it on to be processed.
 *
 */
/*ARGSUSED*/
static void
event_proc(widget, client_data, event, continue_to_dispatch)
Widget widget ;
XtPointer client_data ;
XEvent *event ;
Boolean *continue_to_dispatch ;
{
  Widget w;
  Arg args[3];
  XmString label;
  String str ;
  int n;
  static char count = 0;

  /* If the ASCII convert dialog, or the Help dialog
     is up and managed, we don't want the keystrokes to go to the calculator
     main, just those dialogs */
  if ((X->Aframe != NULL && XtIsManaged(X->Aframe)) ||
     (X->helpDialog != NULL && X->helpMapped))
  {
     return;
  }

  if(ignore_event)
  {
     if(count == 0)
       count++;
     else {
        count = 0; 
        ignore_event = False;
        XtRemoveTimeOut(timerId);
     }
     return;
  }
  X->event = event ;
  v->event_type = get_next_event(widget, 0, event) ;

  /* first check to see if there is an error condition, if there is we
     want to beep and return.
  */
  XtSetArg(args[0], XmNlabelString, &label);
  XtGetValues(X->modevals[OPITEM], args, 1);

  XmStringGetLtoR (label, XmFONTLIST_DEFAULT_TAG, &str);

  if(strcmp(vstrs[(int) V_CLR], str) == 0 &&
                      (v->cur_ch !=  KEY_CLR && v->cur_ch !=  KEY_QUIT))
  {
     if(v->event_type == KEYBOARD_DOWN)
        beep();
     else
     {
        n = get_index(v->cur_ch);
        if(n != TITEMS)
           draw_button(n, v->curwin, v->row, v->column, FALSE);
     }
     return;
  }

  /* Now lets check to see if the input was for the Constants/Functions
     dialog.  If it was process it.
  */
  if(X->CFframe != NULL && XtIsManaged(X->CFframe))
  {
     w = XmGetFocusWidget(X->CFframe);
     if(w == X->CFpi_butOK)
     {
        if(v->cur_ch == KEY_EQ)
        {
           XtCallCallbacks(X->CFpi_butOK, XmNarmCallback, (XtPointer) NULL);
           XtCallCallbacks(X->CFpi_butOK, XmNactivateCallback, 
                                                            (XtPointer) NULL);
        }
        if(v->event_type == ARROW || v->event_type == TAB)
        {
           XtSetArg(args[0], XmNdefaultButton, X->CFpi_butOK);
           XtSetValues(X->CFframe, args, 1);
        }
     }
     else if(w == X->CFpi_butClose)
     {
        if(v->cur_ch == KEY_EQ)
        {
           XtCallCallbacks(X->CFpi_butClose, XmNarmCallback, (XtPointer) NULL);
           XtCallCallbacks(X->CFpi_butClose, XmNactivateCallback, 
                                                            (XtPointer) NULL);
        }
        if(v->event_type == ARROW || v->event_type == TAB)
        {
           XtSetArg(args[0], XmNdefaultButton, NULL);
           XtSetValues(X->CFframe, args, 1);
        }
     }
     else if(w == X->CFpi_butHelp)
     {
        if(v->cur_ch == KEY_EQ)
        {
           XtCallCallbacks(X->CFpi_butHelp, XmNarmCallback, (XtPointer) NULL);
           XtCallCallbacks(X->CFpi_butHelp, XmNactivateCallback, 
                                                   (XtPointer) HELP_CONSTANT);
        }
        if(v->event_type == ARROW || v->event_type == TAB)
        {
           XtSetArg(args[0], XmNdefaultButton, NULL);
           XtSetValues(X->CFframe, args, 1);
        }
     }
     return;
  }
 
  /* if the key is an "inactive" key (i.e. desensitized) then we want to
     ignore it. */ 
  switch(v->base)
  {
     case BIN:
        if(v->cur_ch == KEY_2 || 
           v->cur_ch == KEY_3 || 
           v->cur_ch == KEY_4 || 
           v->cur_ch == KEY_5 || 
           v->cur_ch == KEY_6 || 
           v->cur_ch == KEY_7)
            return;
     case OCT:
        if(v->cur_ch == KEY_8 || 
           v->cur_ch == KEY_9)
            return;
     case DEC:
        if(v->cur_ch == KEY_A || 
           v->cur_ch == KEY_B || 
           v->cur_ch == KEY_C || 
           v->cur_ch == KEY_D || 
           v->cur_ch == KEY_E || 
           v->cur_ch == KEY_F)
            return;
  }

  /* finally, check to see if it is a key we wish to ignore and motif
     will handle.
  */
  if(v->event_type == F4_PRESS || v->event_type == ARROW || 
     v->event_type == TAB || v->event_type == CONTROL || 
     v->event_type == SHIFT || v->event_type == SPACE ||
     v->event_type == ESCAPE || v->event_type == META || v->event_type == ALT)
     return;
  else if (v->cur_ch == CTL('/'))
     return;
  else
     process_event(v->event_type) ;
  v->error = False;
}


/*ARGSUSED*/
static void
frame_interpose(widget, clientData, event, continue_to_dispatch)
Widget widget ;
XtPointer clientData ;
XEvent *event ;
Boolean *continue_to_dispatch ;
{
  if (!v->started) return ;
  if (event->type == MapNotify)
    {
      if (v->rstate)            win_display(FCP_REG,  TRUE) ;
      else if (v->frstate)      win_display(FCP_FIN_REG,  TRUE) ;
      v->iconic = FALSE ;
    }
  else if (event->type == UnmapNotify) v->iconic = TRUE ;
}


/*ARGSUSED*/
static int
get_next_event(widget, ev_action, xevent)
Widget widget ;
int ev_action ;
XEvent *xevent ;
{
  char *tmpStr, chs[2] ;
  int cval, down, nextc, up ;
  KeySym ksym;
  XKeyPressedEvent *key_event ;

  if (!v->started) return(LASTEVENTPLUSONE) ;
  nextc = xevent->type ;

  down    = xevent->type == ButtonPress   || xevent->type == KeyPress ;
  up      = xevent->type == ButtonRelease || xevent->type == KeyRelease ;

  if (nextc == KeyPress || nextc == KeyRelease)
    {
      key_event = (XKeyPressedEvent *) xevent ;
      chs[0] = chs[1] = (char)0;
      XLookupString(key_event, chs, 1, &ksym, (XComposeStatus *) NULL) ;
           if (ksym == XK_F4 && down) return(F4_PRESS) ;
      else if (ksym == XK_F4 && up) return(F4_PRESS) ;
      else if (ksym == XK_Right  && down) 
      {
         if (!event_is_keypad(xevent))
            return(ARROW);
      }
      else if (ksym == XK_Left  && down) 
      {
         if (!event_is_keypad(xevent))
            return(ARROW);
      }
      else if (ksym == XK_Up  && down) 
      {
         if (!event_is_keypad(xevent))
            return(ARROW);
      }
      else if (ksym == XK_Down  && down) 
      {
         if (!event_is_keypad(xevent))
            return(ARROW);
      }
      else if (ksym == XK_F10  && down) return(F4_PRESS);
      else if (ksym == XK_F10  && up) return(F4_PRESS);
      else if (ksym == XK_Tab  && down) return(TAB);
      else if (ksym == XK_Tab  && up) return(TAB);
#ifdef hpux
      else if (ksym == XK_BackTab  && down) return(SHIFT);
      else if (ksym == XK_BackTab  && up) return(SHIFT);
      else if (ksym == hpXK_DeleteChar  && down) cval = 127;
      else if (ksym == hpXK_DeleteChar  && up) cval = 127;
#endif
      else if (ksym == XK_Return && down) cval = KEY_EQ;
      else if (ksym == XK_Return && up) cval = KEY_EQ;
      else if (ksym == XK_Escape && down) return(ESCAPE);
      else if (ksym == XK_Escape && up) return(ESCAPE);
      else if (ksym == XK_Control_L  && down) return(CONTROL);
      else if (ksym == XK_Control_L  && up) return(CONTROL);
      else if (ksym == XK_Control_R  && down) return(CONTROL);
      else if (ksym == XK_Control_R  && up) return(CONTROL);
      else if (ksym == XK_Meta_L  && down) return(META);
      else if (ksym == XK_Meta_R  && up) return(META);
      else if (ksym == XK_Meta_L  && down) return(META);
      else if (ksym == XK_Meta_R  && up) return(META);
      else if (ksym == XK_Alt_L  && down) return(ALT);
      else if (ksym == XK_Alt_R  && up) return(ALT);
      else if (ksym == XK_Alt_L  && down) return(ALT);
      else if (ksym == XK_Alt_R  && up) return(ALT);
      else if (ksym == XK_Select  && down) return(SPACE);
      else if (ksym == XK_Select  && up) return(SPACE);
      else if (ksym == XK_space  && down) return(SPACE);
      else if (ksym == XK_space  && up) return(SPACE);
      else if (ksym == XK_Shift_L || ksym == XK_Shift_R)
        return(LASTEVENTPLUSONE) ;
      else cval = chs[0] ;
    }

  tmpStr = GETMESSAGE(3, 45, ".");
  if (event_is_keypad(xevent))
    {
#ifdef _AIX
      /*
       * In keypad_keysym(), KeySym associated with xevent->xkey.keycode 
       * is extracted by calling XKeycodeToKeysym(). But the current
       * implementation doesn't care of the modifires state. In this moment,
       * we can still use ksym which is got in the above XLookupString().
       * Sun's implementation seems to need an attention for this. But AIX
       * doesn't.
       * Still JP kbd NumLock state cannot be handled correctly.
       */ 
      switch (ksym)
#else /* _AIX */
      switch (keypad_keysym(xevent))
#endif /* AIX_ILS */
        {
          case XK_KP_0        : v->cur_ch = '0' ;
                                break ;
          case XK_KP_1        : v->cur_ch = '1' ;
                                break ;
          case XK_KP_2        : v->cur_ch = '2' ;
                                break ;
          case XK_KP_3        : v->cur_ch = '3' ;
                                break ;
          case XK_KP_4        : v->cur_ch = '4' ;
                                break ;
          case XK_KP_5        : v->cur_ch = '5' ;
                                break ;
          case XK_KP_6        : v->cur_ch = '6' ;
                                break ;
          case XK_KP_7        : v->cur_ch = '7' ;
                                break ;
          case XK_KP_8        : v->cur_ch = '8' ;
                                break ;
          case XK_KP_9        : v->cur_ch = '9' ;
                                break ;
          case XK_KP_Add      : v->cur_ch = '+' ;
                                break ;
          case XK_KP_Subtract : v->cur_ch = '-' ;
                                break ;
          case XK_KP_Multiply : v->cur_ch = 'x' ;
                                break ;
          case XK_KP_Divide   : v->cur_ch = '/' ;
                                break ;
          case XK_KP_Equal    : 
          case XK_KP_Enter    : v->cur_ch = '=' ;
                                break ;
          case XK_KP_Decimal  : v->cur_ch = tmpStr[0] ;
        }
           if (down) return(KEYBOARD_DOWN) ;
      else if (up)   return(KEYBOARD_UP) ;
    }
  else if ((nextc == KeyPress || nextc == KeyRelease) &&
          (IS_KEY(cval, KEY_BSP) || IS_KEY(cval, KEY_CLR)))
    {
      v->cur_ch = cval ;        /* Delete and Back Space keys. */
           if (down) return(KEYBOARD_DOWN) ;
      else if (up)   return(KEYBOARD_UP) ;
    }

  if ((nextc == KeyPress || nextc == KeyRelease) && cval >= 0 && cval <= 127)
    {

/*  If this is a '*' or Return key press, then map to their better known
 *  equivalents, so that button highlighting works correctly.
 */

      if (cval == '*')      cval = KEY_MUL ;

/*  All the rest of the ASCII characters. */

      v->cur_ch = cval ;
           if (down) return(KEYBOARD_DOWN) ;
      else if (up)   return(KEYBOARD_UP) ;
    }

  return(LASTEVENTPLUSONE) ;
}


char *
get_resource(rtype)      /* Get dtcalc resource from merged database. */
enum res_type rtype ;
{
  char str[MAXLINE] ;

  STRCPY(str, calc_res[(int) rtype]) ;
  return(ds_get_resource(X->rDB, v->appname, str)) ;
}


void
grey_button(row, column, state)
int row, column, state ;
{
  XtSetSensitive(X->kbuttons[row][column], !state) ;
}


void
init_graphics()
{
  char *tmpStr, *tmpStr1;

  tmpStr = GETMESSAGE(2, 12, "Calculator");
  tmpStr1 = XtNewString(tmpStr);
  XtVaSetValues(X->kframe,
                XmNiconName, tmpStr1,
                XmNtitle,    tmpStr1,
                0) ;
  dtcalc_kpanel_create(X->kframe) ;
  dtcalc_kkeyboard_create(X->kpanel) ;
  make_modewin();
  XtSetMappedWhenManaged(X->kframe, False);
  XtRealizeWidget(X->kframe) ;
  XSync(X->dpy, False);
  setCalcHints();
  XtSetMappedWhenManaged(X->kframe, True);
#ifdef AIX_ILS
  XtMapWidget( X->kframe );
#else /* AIX_ILS */
  XtPopup (X->kframe, XtGrabNone);
#endif /* AIX_ILS */
  XSync(X->dpy, False);
  XtFree(tmpStr1);

  X->Aframe  = NULL ;
  X->CFframe = NULL ;
  X->Pframe  = NULL ;
}


static int
is_window_showing(widget)
Widget widget ;
{
  return(XtIsManaged(widget)) ;
}


void
key_init()
{
  int i, j ;
  KeySym *tmp ;
  KeySym ks ;

  XDisplayKeycodes(X->dpy, &X->kcmin, &X->kcmax) ;
  tmp = XGetKeyboardMapping(X->dpy, X->kcmin, 1, &X->keysyms_per_key) ;
  XFree((char *) tmp) ;

  X->kparray = (unsigned char *) malloc(X->kcmax - X->kcmin + 1) ;

/*  For each key, run through its list of keysyms.  If this keysym is a
 *  keypad keysym, we know this key is on the keypad.  Mark it as such in
 *  kparray[].
 */

  for (i = X->kcmin; i <= X->kcmax; ++i)
    {
      X->kparray[i - X->kcmin] = 0 ;
      for (j = 0; j < X->keysyms_per_key; ++j)
        {
          ks = XKeycodeToKeysym(X->dpy, i, j) ;
          if (IsKeypadKey(ks))
            {
              X->kparray[i - X->kcmin] = 1 ;
              break ;
            }
        }
    }
}


static KeySym
keypad_keysym(xevent)
XEvent *xevent ;
{
  int i ;
  int keycode = xevent->xkey.keycode ;
  KeySym ks ;

  for (i = 0; i < X->keysyms_per_key; ++i)
    {
      ks = XKeycodeToKeysym(X->dpy, keycode, i) ;
      if (IsKeypadKey(ks)) 
      {
#ifdef sun
         if(ks != XK_KP_Insert)
#endif
            return(ks) ;
      }
    }
  return(NoSymbol) ;
}

void
load_resources()
{
  X->rDB = ds_load_resources(X->dpy) ;
}


void
make_frames()
{
  char *tool_label = NULL ;
  int depth ;

  if (v->titleline == NULL)
    {
      tool_label = (char *) calloc(1, strlen(lstrs[(int) L_UCALC]) + 3);

      SPRINTF(tool_label, "%s", lstrs[(int) L_UCALC]);
    }
  else read_str(&tool_label, v->titleline) ;

  X->clipboard   = XInternAtom(X->dpy, "CLIPBOARD", FALSE) ;
  X->length_atom = XInternAtom(X->dpy, "LENGTH",    FALSE) ;
  XtAddEventHandler(X->kframe, StructureNotifyMask, FALSE,
                    frame_interpose, NULL) ;
  XtVaSetValues(X->kframe,
                XmNtitle, tool_label,
                0) ;

  XtVaGetValues(X->kframe,
                XmNdepth, &depth,
                0) ;

  if(v->titleline == NULL)
    free(tool_label);
}

void
make_modewin()             /* Draw special mode frame plus buttons. */
{
  switch_mode(v->modetype) ;
}

void
make_registers(type)           /* Calculate memory register frame values. */
int type;
{
  char line[MAXLINE] ;     /* Current memory register line. */
  char *ptr, *tmp, *tmpStr;
  int i, savAcc ;
  XmString str, numStr ;
  int MPtemp[MP_SIZE];

  if(type == MEM)
  {
     if (!v->rstate) return ;
  }
  else if(type == FIN)
  {
     if (!v->frstate) return ;
  }

  dtcalc_initialize_rframe(X->kframe, type) ;

  tmpStr = GETMESSAGE(3, 45, ".");
  if(type == MEM)
  {
     for (i = 0; i < MAXREGS; i++)
       {
         SPRINTF(line, "%s:   %s", menu_entries[i + 10].str, 
                                      make_number(v->MPmvals[i], FALSE))  ;
         
         {
            /* if it's not a "." let's go change it to what it should be */
            ptr = DtStrrchr(line, '.');
            if(ptr != NULL)
               ptr[0] = tmpStr[0];
         }
         str = XmStringCreateSimple(line) ;
         XtVaSetValues(X->registers[i], XmNlabelString, str, 0) ;
         XmStringFree(str) ;
       }
  }
  else
  {
     for (i = 0; i < FINREGS; i++)
       {
         if( i == 5)
         {
            mpcdm(&(v->MPfvals[i]), MPtemp);
            tmp = make_number(MPtemp, FALSE);
            ptr = DtStrchr(tmp, 'e');
            if(ptr == NULL)
            {
               ptr = DtStrchr(tmp, '.');
               if(ptr == NULL)
               {
                  ptr = DtStrchr(tmp, ',');
                  if(ptr != NULL)
                     *ptr = '\0';
               }
               else
                  *ptr = '\0';
            }
         }
         else if ( i == 1 )
         {
            savAcc = v->accuracy;
            v->accuracy = 2;
            mpcdm(&(v->MPfvals[i]), MPtemp);
            tmp = make_number(MPtemp, FALSE);
            v->accuracy = savAcc;
         }
         else
         {
            mpcdm(&(v->MPfvals[i]), MPtemp);
            tmp = make_number(MPtemp, FALSE);
         }
         if(strcmp(tmpStr, ".") != 0)
         {
            /* if it's not a "." let's go change it to what it should be */
            ptr = DtStrrchr(tmp, '.');
            if(ptr != NULL)
               ptr[0] = tmpStr[0];
         }
         str = XmStringCreateSimple(menu_entries[i + 34].str) ;
         numStr = XmStringCreateSimple(tmp);
         XtVaSetValues(X->fregisters[i], XmNlabelString, str, 0) ;
         XtVaSetValues(X->fregistersvals[i], XmNlabelString, numStr, 
                                            XmNalignment, XmALIGNMENT_END, 0) ;
         XmStringFree(str) ;
         XmStringFree(numStr) ;
       }
  }
}


/*ARGSUSED*/
static void
menu_handler(widget, client_data, event, continue_to_dispatch)
Widget widget ;
XtPointer client_data ;
XEvent *event ;
Boolean *continue_to_dispatch ;
{
  int button ;
  Widget menu ;

  X->mtype = (enum menu_type) client_data ;
  menu = X->menus[(int) X->mtype] ;
  XtVaGetValues(menu, XmNwhichButton, &button, 0) ;
  if (event->xbutton.button == button)
    {
      XmMenuPosition(menu, (XButtonPressedEvent *) event) ;
      XtManageChild(menu) ;
    }
}


/*ARGSUSED*/
void
menu_proc(widget, client_data, call_data)
Widget widget ;
XtPointer client_data, call_data ;
{
  int choice = ((int) client_data) & 0xFFFF ;

  v->curwin = ((int) client_data) >> 16 ;
  handle_menu_selection(X->mrec[(int) X->mtype], choice) ;
}


/*ARGSUSED*/
static void
new_cf_value(widget, client_data, call_data)
Widget widget ;
XtPointer client_data, call_data ;
{
  enum menu_type mtype = (enum menu_type) client_data;
  XmString cstr ;

  if (X->CFframe == NULL) create_cfframe() ;
  if (mtype ==  M_CON)
    {
      X->CFtype = M_CON ;
      set_text_str(X->CFpi_cftext, T_LABEL, lstrs[(int) L_CONNO]) ;

      cstr = XmStringCreateSimple(lstrs[(int) L_NEWCON]) ;
      XtVaSetValues(X->CFframe, XmNdialogTitle, cstr, 0) ;
      XmStringFree(cstr) ;

      XtRemoveAllCallbacks(X->CFpi_butHelp, XmNactivateCallback);
      XtAddCallback(X->CFpi_butHelp, XmNactivateCallback, HelpRequestCB,
                                                    (XtPointer) HELP_CONSTANT);
    }
  else
    {
      X->CFtype = M_FUN ;
      set_text_str(X->CFpi_cftext, T_LABEL, lstrs[(int) L_FUNNO]) ;

      cstr = XmStringCreateSimple(lstrs[(int) L_NEWFUN]) ;
      XtVaSetValues(X->CFframe, XmNdialogTitle, cstr, 0) ;
      XmStringFree(cstr) ;

      XtRemoveAllCallbacks(X->CFpi_butHelp, XmNactivateCallback);
      XtAddCallback(X->CFpi_butHelp, XmNactivateCallback, HelpRequestCB,
                                                    (XtPointer) HELP_FUNCTION);
    }
 
/* Clear text fields. */
 
  set_text_str(X->CFpi_cftext, T_VALUE, "") ;
  set_text_str(X->CFpi_dtext,  T_VALUE, "") ;
  set_text_str(X->CFpi_vtext,  T_VALUE, "") ;
  XmProcessTraversal(X->CFpi_cftext->textfield, XmTRAVERSE_CURRENT) ;

  if (!is_window_showing(X->CFframe))
    _DtGenericMapWindow (X->kframe, X->CFframe);

  XtManageChild(X->CFframe) ;

  /* lets set the focus in the first text widget */
  XmProcessTraversal(X->CFpi_cftext->textfield, XmTRAVERSE_CURRENT);
  ignore_event = True;
  timerId = XtAppAddTimeOut (XtWidgetToApplicationContext (X->kframe), 300,
                             TimerEvent, (XtPointer) NULL);
}


void
put_resource(rtype, value)   /* Put calc resource into database. */
enum res_type rtype ;
char *value ;
{
  ds_put_resource(&(X->dtcalcDB), v->appname, calc_res[(int) rtype], value) ;
}


void
redraw_buttons()
{
  enum fcp_type scurwin ;
  int column, n, row ;
  XmString lstr ;

  scurwin   = v->curwin ;
  v->curwin = FCP_KEY ;
  for (row = 0; row < BROWS; row++)
    for (column = 0; column < BCOLS; column++)
      {  
        n = row * MAXCOLS + column ;
        get_label(n);
        lstr = XmStringCreateSimple(v->pstr) ;
        XtVaSetValues(X->kbuttons[row][column], XmNlabelString, lstr, NULL);
        XmStringFree(lstr) ;
      }
  v->curwin = scurwin ;
}


void
save_cmdline(argc, argv)
int argc ;
char *argv[] ;
{
  ds_save_cmdline(X->dpy, XtWindow(X->kframe), argc, argv) ;
}


void
save_resources(filename)
char *filename;
{
  int reply = ds_save_resources(X->dtcalcDB, filename) ;

  if (reply) _DtSimpleError (v->appname, DtWarning, NULL, vstrs[(int) V_NORSAVE]);
}

void
ErrorDialog(string)
char *string;
{
   ErrDialog(string, X->mainWin);
}

void
set_item(itemno, str)
enum item_type itemno ;
char *str ;
{
  Widget w ;
  XmString cstr ;
  char *tmpStr, *ptr, displayStr[50];

  if (itemno == DISPLAYITEM)
    {
      if(!NoDisplay) 
      {
         if(str != NULL && (strcmp(str, "") != 0))
         {
            /* Let's get the decimal point, in some languages it's a , */
            strcpy(displayStr, str);
            tmpStr = GETMESSAGE(3, 45, ".");
            if(strcmp(tmpStr, ".") != 0)
            {
               /* if it's not a "." let's go change it to what it should be */
               ptr = DtStrrchr(displayStr, '.');
               if(ptr != NULL)
                  ptr[0] = tmpStr[0];
            }
            w = X->modevals[(int) DISPLAYITEM] ;
            XmTextSetString(w, displayStr) ;
            XmTextSetInsertionPosition(w, XmTextGetLastPosition(w)) ;
            XSync(X->dpy, False);
         }
      }
      return ;
    } 
  else
    {
       if(str != NULL)
       {
          cstr = XmStringCreateSimple(str) ;
          XtVaSetValues(X->modevals[(int) itemno], XmNlabelString, cstr, 0) ;
          XmStringFree(cstr) ;
       }
    }
}

void
set_title(fcptype, str)     /* Set new title for a window. */
enum fcp_type fcptype ;
char *str ;
{
  Widget w ;
  XmString cstr ;


  if (fcptype == FCP_KEY)  
    w = X->kframe ;
  else if (fcptype == FCP_REG)  
    w = X->rframe ;
  else if (fcptype == FCP_FIN_REG)  
    w = X->frframe ;
  else if (fcptype == FCP_MODE) 
    w = X->mframe[(int) v->modetype] ;

  if (fcptype == FCP_KEY) 
    XtVaSetValues(w, XmNtitle, str, 0) ;
  else
    {
#ifdef AIX_ILS
      cstr = XmStringCreateLocalized(str) ;
#else /* AIX_ILS */
      cstr = XmStringCreateSimple(str) ;
#endif /* AIX_ILS */
      XtVaSetValues(w, XmNdialogTitle, cstr, 0) ;
      XmStringFree(cstr) ;
    }

}

/*ARGSUSED*/
void
show_ascii(widget, client_data, call_data)
Widget widget ;
XtPointer client_data, call_data ;
{
  char *str ;
  int val ;

  str = XmTextFieldGetString(X->Api_text->textfield);
  val = str[strlen(str) - 1] ;
  mpcim(&val, v->MPdisp_val) ;
  show_display(v->MPdisp_val) ;
  XtFree(str);
}


void
show_ascii_frame()      /* Display ASCII popup. */
{
  XmString tstr ;
  Widget sep, frame, form;
  Arg args[15];
  XmString label_string;

  if (X->Aframe == NULL)
    {
      X->Aframe = (Widget) XmCreateFormDialog(X->kframe, "aframe", NULL, 0) ;
      tstr = XmStringCreateSimple(pstrs[(int) P_ASCIIT]) ;
      XtVaSetValues(X->Aframe,
                    XmNdialogTitle,     tstr,
                    XmNautoUnmanage,    FALSE,
                    XmNallowShellResize, TRUE,
                    XmNdefaultPosition, FALSE,
                    NULL) ;
      XmStringFree(tstr) ;

      /*  Adjust the decorations for the dialog shell of the dialog  */
      XtSetArg (args[0], XmNmwmFunctions, MWM_FUNC_MOVE);
      XtSetArg (args[1], XmNmwmDecorations, MWM_DECOR_BORDER | MWM_DECOR_TITLE);
      XtSetValues (XtParent(X->Aframe), args, 2);

      X->Api_text = make_textW(X->Aframe, pstrs[(int) P_CHAR]) ;

      XtSetArg (args[0], XmNtopAttachment, XmATTACH_FORM);
      XtSetArg (args[1], XmNtopOffset, 5);
      XtSetArg (args[2], XmNleftAttachment, XmATTACH_FORM);
      XtSetArg (args[3], XmNleftOffset, 5);
      XtSetValues(X->Api_text->manager, args, 4);

      XtSetArg (args[0], XmNmaxLength, 1);
      XtSetValues(X->Api_text->textfield, args, 1);

      XtSetArg (args[0], XmNtopAttachment, XmATTACH_WIDGET);
      XtSetArg (args[1], XmNtopWidget, X->Api_text->manager);
      XtSetArg (args[2], XmNtopOffset, 3);
      XtSetArg (args[3], XmNleftAttachment, XmATTACH_FORM);
      XtSetArg (args[4], XmNrightAttachment, XmATTACH_FORM);
      sep = XmCreateSeparator(X->Aframe, "sep", args, 5);
      XtManageChild(sep);

#ifdef AIX_ILS
      label_string = XmStringCreateLocalized ( GETMESSAGE(2, 33, "Apply") );
#else /* AIX_ILS */
      label_string = XmStringCreateLtoR (GETMESSAGE(2, 33, "Apply"),
                                                    XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
      XtSetArg (args[0], XmNmarginHeight, 2);
      XtSetArg (args[1], XmNmarginWidth, 15);
      XtSetArg (args[2], XmNlabelString, label_string);
      XtSetArg (args[3], XmNtopAttachment, XmATTACH_WIDGET);
      XtSetArg (args[4], XmNtopWidget, sep);
      XtSetArg (args[5], XmNtopOffset, 5);
      XtSetArg (args[6], XmNleftAttachment, XmATTACH_FORM);
      XtSetArg (args[7], XmNleftOffset, 10);
      XtSetArg (args[8], XmNbottomAttachment, XmATTACH_FORM);
      XtSetArg (args[9], XmNbottomOffset, 5);
      X->Api_butOK = XmCreatePushButton(X->Aframe, "button", args, 10);
      XmStringFree(label_string);
      XtManageChild(X->Api_butOK);

#ifdef AIX_ILS
      label_string = XmStringCreateLocalized ( GETMESSAGE(2, 32, "Close") );
#else /* AIX_ILS */
      label_string = XmStringCreateLtoR (GETMESSAGE(2, 32, "Close"), 
                                                    XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
      XtSetArg (args[0], XmNmarginHeight, 2);
      XtSetArg (args[1], XmNmarginWidth, 10);
      XtSetArg (args[2], XmNlabelString, label_string);
      XtSetArg (args[3], XmNtopAttachment, XmATTACH_WIDGET);
      XtSetArg (args[4], XmNtopWidget, sep);
      XtSetArg (args[5], XmNtopOffset, 5);
      XtSetArg (args[6], XmNleftAttachment, XmATTACH_WIDGET);
      XtSetArg (args[7], XmNleftWidget, X->Api_butOK);
      XtSetArg (args[8], XmNleftOffset, 10);
      XtSetArg (args[9], XmNbottomAttachment, XmATTACH_FORM);
      XtSetArg (args[10], XmNbottomOffset, 5);
      X->Api_butClose = XmCreatePushButton(X->Aframe, "button", args, 11);
      XmStringFree(label_string);
      XtManageChild(X->Api_butClose);
    
#ifdef AIX_ILS
      label_string = XmStringCreateLocalized ( GETMESSAGE(2, 47, "Help") );
#else /* AIX_ILS */
      label_string = XmStringCreateLtoR (GETMESSAGE(2, 47, "Help"), 
                                                  XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
      XtSetArg (args[0], XmNmarginHeight, 2);
      XtSetArg (args[1], XmNmarginWidth, 10);
      XtSetArg (args[2], XmNlabelString, label_string);
      XtSetArg (args[3], XmNtopAttachment, XmATTACH_WIDGET);
      XtSetArg (args[4], XmNtopWidget, sep);
      XtSetArg (args[5], XmNtopOffset, 5);
      XtSetArg (args[6], XmNleftAttachment, XmATTACH_WIDGET);
      XtSetArg (args[7], XmNleftWidget, X->Api_butClose);
      XtSetArg (args[8], XmNleftOffset, 10);
      XtSetArg (args[9], XmNrightAttachment, XmATTACH_FORM);
      XtSetArg (args[10], XmNrightOffset, 10);
      XtSetArg (args[11], XmNbottomAttachment, XmATTACH_FORM);
      XtSetArg (args[12], XmNbottomOffset, 5);
      X->Api_butHelp = XmCreatePushButton(X->Aframe, "button", args, 13);
      XmStringFree(label_string);
      XtManageChild(X->Api_butHelp);

      XtAddCallback(X->Api_text->textfield, XmNactivateCallback,
                                                       show_ascii, NULL) ;
      XtAddCallback(X->Api_butOK,  XmNactivateCallback, show_ascii, NULL) ;
      XtAddCallback(X->Api_butClose, XmNactivateCallback, close_ascii,
                                                          (XtPointer) NULL) ;
      XtAddCallback(X->Api_butHelp, XmNactivateCallback, HelpRequestCB,
                                                    (XtPointer) HELP_ASCII) ;

      XtSetArg (args[0], XmNcancelButton, X->Api_butClose);
      XtSetArg (args[1], XmNdefaultButton, X->Api_butOK);
      XtSetValues (X->Aframe, args, 2);
    }

  if (!is_window_showing(X->Aframe))
    _DtGenericMapWindow (X->kframe, X->Aframe);

  XtManageChild(X->Aframe) ;

  XmProcessTraversal(X->Api_text->textfield, XmTRAVERSE_CURRENT);
  ignore_event = True;
  timerId = XtAppAddTimeOut (XtWidgetToApplicationContext (X->kframe), 300,
			     TimerEvent, (XtPointer) NULL);
}


void
start_tool()
{
  saveatom = XmInternAtom(X->dpy, "WM_SAVE_YOURSELF", FALSE) ;
  command_atom = XmInternAtom (X->dpy, "WM_COMMAND", FALSE);
  wm_state_atom = XmInternAtom (X->dpy, "WM_STATE", False);

  XmAddWMProtocols(X->kframe, &saveatom, 1) ;
  XmAddWMProtocolCallback(X->kframe, saveatom, save_state, (XtPointer)NULL) ;

  v->started = 1 ;
  XSetErrorHandler((int (*)())xerror_interpose) ;
  XtAddEventHandler(X->kframe, KeyPressMask | KeyReleaseMask,
                    FALSE, event_proc, NULL) ;
#ifdef AIX_ILS
  XmProcessTraversal( X->kbuttons[0][0], XmTRAVERSE_CURRENT );
#else /* AIX_ILS */
  XtAddGrab(X->kframe, TRUE, TRUE) ;
#endif /* AIX_ILS */

  do_clear();

  XtAppMainLoop(X->app) ;
}

void
clear_buttons( start )
int start;
{
  int i, n;
  int row, column;
  Arg args[8];
  XmString lstr ;
  static Boolean first = True;

  if(first)
  {
     i = 0;
     if(buttons[i].str != NULL)
     {
        XtFree(buttons[i].str);
        XtFree(buttons[i].str2);
        XtFree(buttons[i].resname);
     }
     buttons[i].str = XtNewString("");
     buttons[i].str2 = XtNewString("");
     buttons[i].resname = XtNewString("blank");
     buttons[i].value = 0;
     buttons[i].opdisp = OP_SET;
     buttons[i].mtype = M_NONE;

     n = 0;
     lstr = XmStringCreateSimple(buttons[i].str);
     XtSetArg (args[n], XmNlabelString, lstr); n++;
     if(colorSrv)
     {
        XtSetArg (args[n], XmNbackground, pixels[1].bg); n++;
        XtSetArg (args[n], XmNforeground, pixels[1].fg); n++;
        XtSetArg (args[n], XmNtopShadowColor, pixels[1].ts); n++;
        XtSetArg (args[n], XmNbottomShadowColor, pixels[1].bs); n++;
        XtSetArg (args[n], XmNarmColor, pixels[1].sc); n++;
     }
     XtSetValues( X->kbuttons[0][0], args, n);
     XmStringFree(lstr) ;
     first = False;
  }

  row = (start / BCOLS) + 3;
  column = start % BCOLS;
  for(i = start + 12; i < 24; i++)
  {
     XtFree(buttons[i].str);
     XtFree(buttons[i].resname);
     buttons[i].str = XtNewString("");
     buttons[i].resname = XtNewString("blank");
     buttons[i].value = 0;
     buttons[i].opdisp = OP_SET;
     buttons[i].mtype = M_NONE;

     n = 0;
     lstr = XmStringCreateSimple(buttons[i].str);
     XtSetArg (args[n], XmNlabelString, lstr); n++;
     if(colorSrv)
     {
        XtSetArg (args[n], XmNbackground, pixels[1].bg); n++;
        XtSetArg (args[n], XmNforeground, pixels[1].fg); n++;
        XtSetArg (args[n], XmNtopShadowColor, pixels[1].ts); n++;
        XtSetArg (args[n], XmNbottomShadowColor, pixels[1].bs); n++;
        XtSetArg (args[n], XmNarmColor, pixels[1].sc); n++;
     }
     XtSetValues( X->kbuttons[row][column], args, n);
     XmStringFree(lstr) ;

     column++;
     if(column % BCOLS == 0)
     {
        row++;
        column = 0;
     }
  }
}

void
make_buttons_fin()
{
  int i, n;
  int row, column;
  Arg args[8];
  XmString lstr ;

  row = 3;
  column = 0;
  for(i = 12; i < 24; i++)
  {
     if(buttons[i].str != NULL)
     {
        XtFree(buttons[i].str);
        XtFree(buttons[i].str2);
        XtFree(buttons[i].resname);
     }
     buttons[i].str = XtNewString(mode_buttons[i - 12].str);
     buttons[i].str2 = XtNewString(mode_buttons[i - 12].str2);
     buttons[i].resname = XtNewString(mode_buttons[i - 12].resname);
     buttons[i].value = mode_buttons[i - 12].value;
     buttons[i].opdisp = mode_buttons[i - 12].opdisp;
     buttons[i].mtype = mode_buttons[i - 12].mtype;
     buttons[i].func = mode_buttons[i - 12].func;

     n = 0;
     if(v->tstate)
        lstr = XmStringCreateSimple(buttons[i].str2);
     else
        lstr = XmStringCreateSimple(buttons[i].str);
     XtSetArg (args[n], XmNlabelString, lstr); n++;
     if(colorSrv)
     {
        XtSetArg (args[n], XmNbackground, pixels[7].bg); n++;
        XtSetArg (args[n], XmNforeground, pixels[7].fg); n++;
        XtSetArg (args[n], XmNtopShadowColor, pixels[7].ts); n++;
        XtSetArg (args[n], XmNbottomShadowColor, pixels[7].bs); n++;
        XtSetArg (args[n], XmNarmColor, pixels[7].sc); n++;
     }
     XtSetValues( X->kbuttons[row][column], args, n);
     XmStringFree(lstr) ;

     column++;
     if(column % BCOLS == 0)
     {
        row++;
        column = 0;
     } 
  }
  clear_buttons(11);
}

void
make_buttons_log()
{
  int i, n;
  int row, column;
  Arg args[8];
  XmString lstr ;

  row = 3;
  column = 0;
  for(i = 12; i < 24; i++)
  {
     if(buttons[i].str != NULL)
     {
        XtFree(buttons[i].str);
        XtFree(buttons[i].str2);
        XtFree(buttons[i].resname);
     }
     buttons[i].str = XtNewString(mode_buttons[i + 4].str);
     buttons[i].str2 = XtNewString(mode_buttons[i + 4].str2);
     buttons[i].resname = XtNewString(mode_buttons[i + 4].resname);
     buttons[i].value = mode_buttons[i + 4].value;
     buttons[i].opdisp = mode_buttons[i + 4].opdisp;
     buttons[i].mtype = mode_buttons[i + 4].mtype;
     buttons[i].func = mode_buttons[i + 4].func;

     n = 0;
     if(v->tstate)
        lstr = XmStringCreateSimple(buttons[i].str2);
     else
        lstr = XmStringCreateSimple(buttons[i].str);
     XtSetArg (args[n], XmNlabelString, lstr);  n++;
     if(colorSrv)
     {
        XtSetArg (args[n], XmNbackground, pixels[7].bg); n++;
        XtSetArg (args[n], XmNforeground, pixels[7].fg); n++;
        XtSetArg (args[n], XmNtopShadowColor, pixels[7].ts); n++;
        XtSetArg (args[n], XmNbottomShadowColor, pixels[7].bs); n++;
        XtSetArg (args[n], XmNarmColor, pixels[7].sc); n++;
     }
     XtSetValues( X->kbuttons[row][column], args, n);
     XmStringFree(lstr) ;

     column++;
     if(column % BCOLS == 0)
     {
        row++;
        column = 0;
     }
  }
  clear_buttons(9);
}

void
make_buttons_sci()
{
  int i, n;
  int row, column;
  Arg args[8];
  XmString lstr ;

  row = 3;
  column = 0;
  for(i = 12; i < 24; i++)
  {
     if(buttons[i].str != NULL)
     {
        XtFree(buttons[i].str);
        XtFree(buttons[i].str2);
        XtFree(buttons[i].resname);
     }
     buttons[i].str = XtNewString(mode_buttons[i + 20].str);
     buttons[i].str2 = XtNewString(mode_buttons[i + 20].str2);
     buttons[i].resname = XtNewString(mode_buttons[i + 20].resname);
     buttons[i].value = mode_buttons[i + 20].value;
     buttons[i].opdisp = mode_buttons[i + 20].opdisp;
     buttons[i].mtype = mode_buttons[i + 20].mtype;
     buttons[i].func = mode_buttons[i + 20].func;

     n = 0;
     if(v->tstate)
        lstr = XmStringCreateSimple(buttons[i].str2);
     else
        lstr = XmStringCreateSimple(buttons[i].str);
     XtSetArg (args[n], XmNlabelString, lstr); n++;
     if(colorSrv)
     {
        XtSetArg (args[n], XmNbackground, pixels[7].bg); n++;
        XtSetArg (args[n], XmNforeground, pixels[7].fg); n++;
        XtSetArg (args[n], XmNtopShadowColor, pixels[7].ts); n++;
        XtSetArg (args[n], XmNbottomShadowColor, pixels[7].bs); n++;
        XtSetArg (args[n], XmNarmColor, pixels[7].sc); n++;
     }
     XtSetValues( X->kbuttons[row][column], args, n);
     XmStringFree(lstr) ;

     column++;
     if(column % BCOLS == 0)
     {
        row++;
        column = 0;
     }
  }
  clear_buttons(12);
}


static void
switch_mode(curmode)
enum mode_type curmode ;
{
  int i ;
  XmString lstr;
  Arg args[2];

  v->modetype = curmode ;
  XtSetArg(args[0], XmNmenuHistory, modeArry[(int)curmode]);
  XtSetValues( X->modevals[(int)MODEITEM], args, 1);
  if((int)curmode == (int)FINANCIAL)
  {
     make_buttons_fin();
     XtSetSensitive(X->modevals[(int)TTYPEITEM], False);
     set_item(HYPITEM, "    ") ;
     set_item(INVITEM, "    ") ;
     return;
  }
  else if((int)curmode == (int)LOGICAL)
  {
     make_buttons_log();
     XtSetSensitive(X->modevals[(int)TTYPEITEM], False);
     set_item(HYPITEM, "    ") ;
     set_item(INVITEM, "    ") ;
  }
  else
  {
     make_buttons_sci();
     XtSetSensitive(X->modevals[(int)TTYPEITEM], True);
     set_item(HYPITEM, "    ") ;
     set_item(INVITEM, "    ") ;
  }

}


static void
update_cf_value()
{
  char message[MAXLINE] ;
  char str[MAXLINE] ;          /* Temporary buffer for various strings. */
  char result[MAXLINE] ;
  char *ptr, *tmpStr;
  double tmp ;                 /* For converting constant value. */
  int i, len, inc ;
  int n ;                      /* Set to 1, if constant value is valid. */
  char cur_op, current, display[MAXLINE], old_cal_value, fnum[MAX_DIGITS+1];
  int cur_ch, toclear, tstate, pending, accuracy;
  int MPdisp_val[MP_SIZE], MPlast_input[MP_SIZE], MPresult[MP_SIZE] ;
 
  switch (X->CFtype)
    {
      case M_CON : tmpStr = GETMESSAGE(3, 45, ".");
                   if(strcmp(tmpStr, ".") != 0)
                   {
                      /* if it's not a "." let's go change it to what it 
                         should be */
                      ptr = DtStrchr(X->vval, tmpStr[0]);
                      while(ptr != NULL)
                      {
                         ptr[0] = '.';
                         ptr = DtStrchr(X->vval, tmpStr[0]);
                      }
                   }
                   /* need to run a "compute" of what was typed in */
                   len = strlen(X->vval) ;
                   STRCPY(str, X->vval);
                   if(X->vval[len - 1] != '=')
                   {
                      /* need to add an '=' at the end of the string so it
                         computes correctly */
                      strcat(str, "=");
                      len = strlen(str);
                   }
                     
                   if(strncmp(str, "-", 1) == 0)
                   {
                     inc = 1;
                     for(i=0; i < len -1; i++)
                     {
                        str[i] = str[i+inc];
                        if(inc == 0 || isdigit((int)str[i]) || str[i] == '.')
                          continue;
                        else
                        {
                          str[i] = 'C';
                          inc = 0;
                        }
                     }
                   }
                   /* now let's compute it, first save off some state */
                   cur_op = v->cur_op;
                   current = v->current;
                   old_cal_value = v->old_cal_value;
                   cur_ch = v->cur_ch;
                   toclear = v->toclear;
                   tstate = v->tstate;
                   pending = v->pending;
                   STRCPY(display, v->display);
                   STRCPY(fnum, v->fnum);
                   mpstr(v->MPdisp_val, MPdisp_val);
                   mpstr(v->MPlast_input, MPlast_input);
                   mpstr(v->MPresult, MPresult);
                   i = 0 ;
                   mpcim(&i, v->MPdisp_val) ;
                   mpcim(&i, v->MPlast_input) ;
                   mpcim(&i, v->MPresult) ;
                   v->cur_op = '?';

                   NoDisplay = True;
                   process_str(str, M_CON);
                   NoDisplay = False;

                   /* get the computed value */
                   accuracy = v->accuracy;
                   v->accuracy = 9 ;
                   STRCPY(result, make_number(v->MPresult, FALSE)) ;
                   v->accuracy = accuracy ;

                   /* return to previous state */
                   v->cur_op = cur_op;
                   v->current = current;
                   v->old_cal_value = old_cal_value;
                   v->cur_ch = cur_ch;
                   v->toclear = toclear;
                   v->tstate = tstate;
                   v->pending = pending;
                   STRCPY(v->display, display);
                   STRCPY(v->fnum, fnum);
                   mpstr(MPdisp_val, v->MPdisp_val);
                   mpstr(MPlast_input, v->MPlast_input);
                   mpstr(MPresult, v->MPresult);

                   set_item(DISPLAYITEM, v->display);

                   n = sscanf(result, "%lf", &tmp) ;
                   ptr = DtStrchr(result, 'e');
                   if (n != 1 || ptr != NULL || v->error == TRUE)
                     {
                       SPRINTF(message, "%s\n%s", vstrs[(int) V_INVCON],
                               vstrs[(int) V_NOCHANGE]) ;
                       do_continue_notice(X->CFframe, message) ;
                       set_item(OPITEM, "") ;
                       if(v->cur_op != '?')
                          set_item(OPITEM, buttons[get_index(v->cur_op)].str);
                       v->error = FALSE;
                       return ;
                     }

                   if(v->cur_op != '?')
                       set_item(OPITEM, buttons[get_index(v->cur_op)].str);

                   /* now let's compute it, first save off some state */
                   if(strncmp(result, "-", 1) == 0)
                   {
                      len = strlen(result);
                      
                      STRCPY(str, result);
                      for(i=0; i < len; i++)
                         str[i] = str[i+1];
                      MPstr_to_num(str, DEC, v->MPcon_vals[X->cfno]) ;
                      mpneg(v->MPcon_vals[X->cfno], v->MPcon_vals[X->cfno]) ;
                   }
                   else
                      MPstr_to_num(result, DEC, v->MPcon_vals[X->cfno]) ;
                    
                   if(strncmp(result, "0.", 2) == 0 ||
                                   strncmp(result, "-0.", 3) == 0 )
                   {
                      len = strlen(result);
                      while(result[len - 1] == '0')
                      {
                          result[len - 1] = '\0';
                          len--;
                      }
                   }
                   SPRINTF(v->con_names[X->cfno], "%1d: %s [%s]",
                           X->cfno, result, X->dval) ;
                   break ;
      case M_FUN : tmpStr = GETMESSAGE(3, 45, ".");
                   if(strcmp(tmpStr, ".") != 0)
                   {
                      /* if it's not a "." let's go change it to what it 
                         should be */
                      ptr = DtStrchr(X->vval, tmpStr[0]);
                      while(ptr != NULL)
                      {
                         ptr[0] = '.';
                         ptr = DtStrchr(X->vval, tmpStr[0]);
                      }
                   }
                   STRCPY(v->fun_vals[X->cfno], convert(X->vval)) ;
                   if(strcmp(X->vval, "") != 0)
                   {
                      SPRINTF(v->fun_names[X->cfno], "%1d: %s [%s]",
                              X->cfno, X->vval, X->dval) ;
                   }
                   else
                      STRCPY(v->fun_names[X->cfno], "");
    }
 
  XtDestroyWidget(X->menus[(int) X->CFtype]) ;
  for (i = 0; i < NOBUTTONS; i++)
    if (buttons[i].mtype == X->CFtype)
      create_menu(X->CFtype, X->kbuttons[i / BCOLS][i % BCOLS], i) ;
  switch (X->CFtype)
    {
      case M_CON :
                   write_rcfile(X->CFtype, X->cfexists, X->cfno,
                                                        result, X->dval) ;
                   break ;
      case M_FUN :
                   write_rcfile(X->CFtype, X->cfexists, X->cfno,
                                                        X->vval, X->dval) ;
    }

  ignore_event = True;
  timerId = XtAppAddTimeOut (XtWidgetToApplicationContext (X->kframe), 300,
			     TimerEvent, (XtPointer) NULL);

}


void
win_display(fcptype, state)
enum fcp_type fcptype ;
int state ;
{
  Widget widget ;
  Position newX, newY;
  Arg args[3];

       if (fcptype == FCP_REG)  widget = X->rframe ;
  else if (fcptype == FCP_MODE) widget = X->mframe[(int) v->modetype] ;
  else if (fcptype == FCP_FIN_REG) widget = X->frframe;

  if (widget == NULL) 
     return ;

  if (state && !is_window_showing(widget))
  {
       if (fcptype == FCP_REG || fcptype == FCP_FIN_REG)  
       {
          XtSetMappedWhenManaged(XtParent(widget), False);
          XSync(X->dpy, False);
      
          XtManageChild(widget) ;

          _DtChildPosition(widget, X->kframe, &newX, &newY);
          XtSetArg(args[0], XmNx, newX);
          XtSetArg(args[1], XmNy, newY);
          XtSetValues(widget, args, 2);

          XtSetMappedWhenManaged(XtParent(widget), True);
          XSync(X->dpy, False);

          XtMapWidget(XtParent(widget));
       }
       else if (fcptype == FCP_MODE) 
          _DtGenericMapWindow (X->kframe, widget);
  }
  if (state) XtManageChild(widget) ;
  else       XtUnmanageChild(widget) ;
}


/*ARGSUSED*/
void
write_cf_value(widget, client_data, call_data)
Widget widget ;
XtPointer client_data, call_data ;
{
  char message[MAXLINE] ;
  char str[MAXLINE] ;          /* Temporary buffer for various strings. */
  Widget focus_widget;
  Arg args[1];

  focus_widget = XmGetFocusWidget(widget);

  if(focus_widget != X->CFpi_vtext->textfield && focus_widget != X->CFpi_butOK)
     return;

  if(focus_widget == X->CFpi_vtext->textfield)
  {
     X->vval = XmTextFieldGetString(X->CFpi_vtext->textfield);
     if(strcmp(X->vval, "") == 0)
        return;
  }

  if(X->dval != NULL)
     XtFree(X->dval);
  if(X->vval != NULL)
     XtFree(X->vval);
  if(X->cfval != NULL)
     XtFree(X->cfval);
  X->dval = XmTextFieldGetString(X->CFpi_dtext->textfield);
  if(strlen(X->dval) > 41)
     X->dval[40] = '\0';
  X->vval = XmTextFieldGetString(X->CFpi_vtext->textfield);
  X->cfval = XmTextFieldGetString(X->CFpi_cftext->textfield);
  SSCANF(X->cfval, "%d", &X->cfno) ;
  if ((strcmp(X->cfval, "") == 0) || X->cfval[0] < '0' || X->cfval[0] > '9' ||
                                     X->cfno < 0 || X->cfno > 9)
    {
      SPRINTF(str, (X->CFtype == M_CON) ? vstrs[(int) V_LCON]
                                   : vstrs[(int) V_LFUN]) ;
      SPRINTF(message, "%s\n%s", str, vstrs[(int) V_RANGE]) ;
      do_continue_notice(X->CFframe, message) ;
      return ;
    }
 
  X->vval = (char *)_DtcalcStripSpaces((char *)X->vval);
  X->cfexists = 0 ;
  switch (X->CFtype)
    {
      case M_CON : X->cfexists = 1 ;    /* Always the default constants. */
                   break ;
      case M_FUN : if (strlen(v->fun_vals[X->cfno])) X->cfexists = 1 ;
    }
  if (X->cfexists)
    {
      SPRINTF(str, mess[(int) MESS_CON],
                   (X->CFtype == M_CON) ? vstrs[(int) V_UCON]
                                        : vstrs[(int) V_UFUN], X->cfno) ;
      SPRINTF(message, "%s\n%s", str, vstrs[(int) V_OWRITE]) ;
      XtUnmanageChild(X->CFframe) ;
      do_confirm_notice(X->CFframe, message) ;
    }    
  else 
  {
     update_cf_value() ;
     XtUnmanageChild(X->CFframe) ;
  }

  XtSetArg (args[0], XmNdefaultButton, NULL);
  XtSetValues (X->CFframe, args, 1);
}


static void
xerror_interpose(display, error)
Display *display ;
XErrorEvent *error ;
{
  char msg1[80];
  char msg[1024];
 
  XGetErrorText(display, error->error_code, msg1, 80) ;
  sprintf(msg, "\nX Error (intercepted): %s\nMajor Request Code   : %d\nMinor Request Code   : %d\nResource ID (XID)    : %u\nError Serial Number  : %u\n", msg1, error->request_code, error->minor_code, error->resourceid, error->serial);
  _DtSimpleError (v->appname, DtError, NULL, msg);
  abort() ;
}

static void
modelineValueChanged(widget, client_data, call_data)
Widget widget ;
XtPointer client_data, call_data ;
{
  Arg args[1];
  int position, val, choice;

  X->mtype = (enum menu_type) client_data ;
  XtSetArg (args[0], XmNuserData, &val);
  XtGetValues (widget, args, 1);

  choice =  val & 0xFFFF ;
  v->curwin = ((int) val) >> 16 ;
  if(X->mtype == M_BASE)
     handle_menu_selection(56, choice) ;
  else if(X->mtype == M_NUM)
     handle_menu_selection(57, choice) ;
  else if(X->mtype == M_MODE)
     handle_menu_selection(58, choice) ;
  else 
     handle_menu_selection(59, choice) ;
}

create_menu_bar(parent)
Widget parent;
{
   register int n;
   int i, j, val, count;
   Widget WidgList[10];
   Arg args[5];
   char *mnemonic;
   XmString labelString;
   Widget child, mem_reg;
   Widget lastCascadeButtonGadget;
   Widget lastCascadeButton;
   Widget lastMenuPane;
   Widget helpPulldown;
   Widget lastCascadeMenuPane;


   /*  Create the pulldown menu  */
   n = 0;
   XtSetArg(args[n], XmNorientation, XmHORIZONTAL);     n++;
   XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
   XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);        n++;
   XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
   X->menubar = XmCreateMenuBar(parent, "mainMenu", args, n);
   XtManageChild(X->menubar);
   XtAddCallback(X->menubar, XmNhelpCallback, HelpRequestCB,
                 (XtPointer)HELP_MENUBAR);

   mnemonic = GETMESSAGE(2, 13, "O");
   mnemonic = XtNewString(mnemonic);
   n = 0;
#ifdef AIX_ILS
   labelString = XmStringCreateLocalized ( GETMESSAGE(2, 14, "Options") );
#else /* AIX_ILS */
   labelString = XmStringCreateLtoR ((GETMESSAGE(2, 14, "Options")),
                                                     XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
   XtSetArg(args[n], XmNlabelString, labelString);   n++;
#ifdef AIX_ILS
   XtSetArg(args[n], XmNmnemonic, XStringToKeysym( mnemonic ) ); n++;
#else /* AIX_ILS */
   XtSetArg(args[n], XmNmnemonic, mnemonic[0]);   n++;
#endif /* AIX_ILS */
   XtSetArg(args[n], XmNmarginWidth, 6);   n++;
   lastCascadeButtonGadget = 
            XmCreateCascadeButtonGadget(X->menubar, "options", args, n);
   XtManageChild(lastCascadeButtonGadget);
   XmStringFree(labelString);
   XtFree(mnemonic);

   lastMenuPane = XmCreatePulldownMenu(X->menubar, "optionMenu", NULL, 0);

   XtSetArg(args[0], XmNsubMenuId, lastMenuPane);
   XtSetValues(lastCascadeButtonGadget, args, 1);

   mnemonic = GETMESSAGE(2, 40, "R");
   mnemonic = XtNewString(mnemonic);
#ifdef AIX_ILS
   labelString =
    	    XmStringCreateLocalized( GETMESSAGE(2, 15, "Memory Registers") );
#else /* AIX_ILS */
   labelString = XmStringCreateLtoR ((GETMESSAGE(2, 15, "Memory Registers")),
                                                        XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
   XtSetArg(args[0], XmNlabelString, labelString );
#ifdef AIX_ILS
   XtSetArg(args[1], XmNmnemonic, XStringToKeysym( mnemonic ) );
#else /* AIX_ILS */
   XtSetArg(args[1], XmNmnemonic, mnemonic[0]);
#endif /* AIX_ILS */
   mem_reg = XmCreatePushButtonGadget (lastMenuPane, "memR", args, 2);
   XtManageChild(mem_reg);
   XmStringFree(labelString);
   XtAddCallback(mem_reg, XmNactivateCallback, (XtCallbackProc)do_memory, 
                                                               (XtPointer)MEM);

   mnemonic = GETMESSAGE(2, 45, "g");
   mnemonic = XtNewString(mnemonic);
#ifdef AIX_ILS
   labelString =
    	XmStringCreateLocalized ( GETMESSAGE(3, 138, "Financial Registers") );
#else /* AIX_ILS */
   labelString = XmStringCreateLtoR (
        (GETMESSAGE(3, 138, "Financial Registers")), XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
   XtSetArg(args[0], XmNlabelString, labelString );
#ifdef AIX_ILS
   XtSetArg(args[1], XmNmnemonic, XStringToKeysym( mnemonic ) );
#else /* AIX_ILS */
   XtSetArg(args[1], XmNmnemonic, mnemonic[0]);
#endif /* AIX_ILS */
   mem_reg = XmCreatePushButtonGadget (lastMenuPane, "memR", args, 2);
   XtManageChild(mem_reg);
   XmStringFree(labelString);
   XtAddCallback(mem_reg, XmNactivateCallback, (XtCallbackProc)do_memory, 
                                                               (XtPointer)FIN);

   mnemonic = GETMESSAGE(2, 41, "A");
   mnemonic = XtNewString(mnemonic);
#ifdef AIX_ILS
   labelString=XmStringCreateLocalized ( GETMESSAGE(2, 16, "ASCII Convert") );
#else /* AIX_ILS */
   labelString = XmStringCreateLtoR ((GETMESSAGE(2, 16, "ASCII Convert")),
                                                        XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
   XtSetArg(args[0], XmNlabelString, labelString );
#ifdef AIX_ILS
   XtSetArg(args[1], XmNmnemonic, XStringToKeysym( mnemonic ) );
#else /* AIX_ILS */
   XtSetArg(args[1], XmNmnemonic, mnemonic[0]);
#endif /* AIX_ILS */
   child = XmCreatePushButtonGadget (lastMenuPane, "asc", args, 2);
   XtManageChild(child);
   XmStringFree(labelString);
   XtAddCallback(child, XmNactivateCallback, (XtCallbackProc)do_ascii, 
                                                         (XtPointer)NULL);

/* commented out by brianc to make ascii option visible in multibyte locales 
   if(_DtNl_is_multibyte)
     XtSetSensitive(child, False); */

   child = (Widget)XmCreateSeparatorGadget(lastMenuPane, "separator",args,0);
   XtManageChild(child);

   mnemonic = GETMESSAGE(2, 42, "F");
   mnemonic = XtNewString(mnemonic);
#ifdef AIX_ILS
   labelString = XmStringCreateLocalized ( vstrs[(int) V_FUNWNAME] );
#else /* AIX_ILS */
   labelString = XmStringCreateLtoR (vstrs[(int) V_FUNWNAME], 
                                                 XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
   XtSetArg(args[0], XmNlabelString, labelString );
#ifdef AIX_ILS
   XtSetArg(args[1], XmNmnemonic, XStringToKeysym( mnemonic ) );
#else /* AIX_ILS */
   XtSetArg(args[1], XmNmnemonic, mnemonic[0]);
#endif /* AIX_ILS */
   child = XmCreatePushButtonGadget (lastMenuPane, "enterFunc", args, 2);
   XtManageChild(child);
   XmStringFree(labelString);
   XtAddCallback(child, XmNactivateCallback, (XtCallbackProc)new_cf_value,
                                                      (XtPointer)(int)M_FUN);

   mnemonic = GETMESSAGE(2, 43, "C");
   mnemonic = XtNewString(mnemonic);
#ifdef AIX_ILS
   labelString = XmStringCreateLocalized ( vstrs[(int) V_CONWNAME] );
#else /* AIX_ILS */
   labelString = XmStringCreateLtoR (vstrs[(int) V_CONWNAME], 
                                                 XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
   XtSetArg(args[0], XmNlabelString, labelString );
#ifdef AIX_ILS
   XtSetArg(args[1], XmNmnemonic, XStringToKeysym( mnemonic ) );
#else /* AIX_ILS */
   XtSetArg(args[1], XmNmnemonic, mnemonic[0]);
#endif /* AIX_ILS */
   child = XmCreatePushButtonGadget (lastMenuPane, "enterCons", args, 2);
   XtManageChild(child);
   XmStringFree(labelString);
   XtAddCallback(child, XmNactivateCallback, (XtCallbackProc)new_cf_value,
                                                      (XtPointer)(int)M_CON);

   child = (Widget)XmCreateSeparatorGadget(lastMenuPane, "separator",args,0);
   XtManageChild(child);

   mnemonic = GETMESSAGE(2, 44, "x");
   mnemonic = XtNewString(mnemonic);
#ifdef AIX_ILS
   labelString = XmStringCreateLocalized ( GETMESSAGE(3, 403, "Exit") );
#else /* AIX_ILS */
   labelString = XmStringCreateLtoR ((GETMESSAGE(3, 403, "Exit")), 
                                                  XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
   XtSetArg(args[0], XmNlabelString, labelString );
#ifdef AIX_ILS
   XtSetArg(args[1], XmNmnemonic, XStringToKeysym( mnemonic ) );
#else /* AIX_ILS */
   XtSetArg(args[1], XmNmnemonic, mnemonic[0]);
#endif /* AIX_ILS */
   child = XmCreatePushButtonGadget (lastMenuPane, "exit", args, 2);
   XtManageChild(child);
   XmStringFree(labelString);
   XtAddCallback(child, XmNactivateCallback, (XtCallbackProc)do_frame,
                                                          (XtPointer) NULL);

   n = 0;
   mnemonic = GETMESSAGE(2, 17, "H");
   mnemonic = XtNewString(mnemonic);
#ifdef AIX_ILS
   labelString = XmStringCreateLocalized ( GETMESSAGE(2, 18, "Help") );
#else /* AIX_ILS */
   labelString = XmStringCreateLtoR ((GETMESSAGE(2, 18, "Help")),
                                                        XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
   XtSetArg(args[n], XmNlabelString, labelString);   n++;
#ifdef AIX_ILS
   XtSetArg(args[n], XmNmnemonic, XStringToKeysym( mnemonic ) ); n++;
#else /* AIX_ILS */
   XtSetArg(args[n], XmNmnemonic, mnemonic[0]);   n++;
#endif /* AIX_ILS */
   XtSetArg(args[n], XmNmarginWidth, 6);   n++;
   lastCascadeButtonGadget =
            XmCreateCascadeButtonGadget(X->menubar, "help", args, n);
   XmStringFree(labelString);
   XtFree(mnemonic);

   helpPulldown = XmCreatePulldownMenu(X->menubar, "helpMenu", NULL, 0);

   XtSetArg(args[0], XmNsubMenuId, helpPulldown);
   XtSetValues(lastCascadeButtonGadget, args, 1);

   XtSetArg(args[0], XmNmenuHelpWidget, lastCascadeButtonGadget);
   XtSetValues (X->menubar, args, 1);

   XtManageChild(lastCascadeButtonGadget);

   count =0;

   n = 0;
   mnemonic = GETMESSAGE(2, 19, "v");
   mnemonic = XtNewString(mnemonic);
#ifdef AIX_ILS
   XtSetArg(args[n], XmNmnemonic, XStringToKeysym( mnemonic ) ); n++;
#else /* AIX_ILS */
   XtSetArg(args[n], XmNmnemonic, mnemonic[0]); n++;
#endif /* AIX_ILS */
#ifdef AIX_ILS
   labelString = XmStringCreateLocalized ( GETMESSAGE(2, 20, "Overview") );
#else /* AIX_ILS */
   labelString = XmStringCreateLtoR ((GETMESSAGE(2, 20, "Overview")),
                                                        XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
   XtSetArg(args[n], XmNlabelString, labelString); n++;
   WidgList[count++]=
       XmCreatePushButton(helpPulldown, "introduction", args, n );
   XtAddCallback(WidgList[count-1], XmNactivateCallback,
       (XtCallbackProc)HelpRequestCB, (XtPointer)HELP_INTRODUCTION);
   XmStringFree(labelString);
   XtFree(mnemonic);

   WidgList[count++]= (Widget)XmCreateSeparatorGadget(helpPulldown,
                                                       "separator",args,0);

   n = 0;
   mnemonic = GETMESSAGE(2, 38, "C");
   mnemonic = XtNewString(mnemonic);
#ifdef AIX_ILS
   XtSetArg(args[n], XmNmnemonic, XStringToKeysym( mnemonic ) ); n++;
#else /* AIX_ILS */
   XtSetArg(args[n], XmNmnemonic, mnemonic[0]); n++;
#endif /* AIX_ILS */
#ifdef AIX_ILS
   labelString =
    	    XmStringCreateLocalized ( GETMESSAGE(2,39,"Table of Contents") );
#else /* AIX_ILS */
   labelString = XmStringCreateLtoR ((GETMESSAGE(2,39,"Table of Contents")),
                                                        XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
   XtSetArg(args[n], XmNlabelString, labelString);  n++;
   WidgList[count++]= XmCreatePushButton(helpPulldown, "toc", args, n );
   XtAddCallback(WidgList[count-1], XmNactivateCallback,
       (XtCallbackProc)HelpRequestCB, (XtPointer)HELP_TABLEOFCONTENTS);
   XmStringFree(labelString);
   XtFree(mnemonic);

   n = 0;
   mnemonic = GETMESSAGE(2, 21, "T");
   mnemonic = XtNewString(mnemonic);
#ifdef AIX_ILS
   XtSetArg(args[n], XmNmnemonic, XStringToKeysym( mnemonic ) ); n++;
#else /* AIX_ILS */
   XtSetArg(args[n], XmNmnemonic, mnemonic[0]); n++;
#endif /* AIX_ILS */
#ifdef AIX_ILS
   labelString = XmStringCreateLocalized ( GETMESSAGE(2, 22, "Tasks") );
#else /* AIX_ILS */
   labelString = XmStringCreateLtoR ((GETMESSAGE(2, 22, "Tasks")),
                                                        XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
   XtSetArg(args[n], XmNlabelString, labelString);  n++;
   WidgList[count++]= XmCreatePushButton(helpPulldown, "tasks", args, n );
   XtAddCallback(WidgList[count-1], XmNactivateCallback,
       (XtCallbackProc)HelpRequestCB, (XtPointer)HELP_TASKS);
   XmStringFree(labelString);
   XtFree(mnemonic);

   n = 0;
   mnemonic = GETMESSAGE(2, 23, "R");
   mnemonic = XtNewString(mnemonic);
#ifdef AIX_ILS
   XtSetArg(args[n], XmNmnemonic, XStringToKeysym( mnemonic ) ); n++;
#else /* AIX_ILS */
   XtSetArg(args[n], XmNmnemonic, mnemonic[0]); n++;
#endif /* AIX_ILS */
#ifdef AIX_ILS
   labelString = XmStringCreateLocalized ( GETMESSAGE(2, 24, "Reference") );
#else /* AIX_ILS */
   labelString = XmStringCreateLtoR ((GETMESSAGE(2, 24, "Reference")),
                                                        XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
   XtSetArg(args[n], XmNlabelString, labelString);  n++;
   WidgList[count++]= XmCreatePushButton(helpPulldown, "reference", args, n );
   XtAddCallback(WidgList[count-1], XmNactivateCallback,
       (XtCallbackProc)HelpRequestCB, (XtPointer)HELP_REFERENCE);
   XmStringFree(labelString);
   XtFree(mnemonic);

   n = 0;
   mnemonic = GETMESSAGE(2, 25, "O");
   mnemonic = XtNewString(mnemonic);
#ifdef AIX_ILS
   XtSetArg(args[n], XmNmnemonic, XStringToKeysym( mnemonic ) ); n++;
#else /* AIX_ILS */
   XtSetArg(args[n], XmNmnemonic, mnemonic[0]); n++;
#endif /* AIX_ILS */
#ifdef AIX_ILS
   labelString = XmStringCreateLocalized ( GETMESSAGE(2, 26, "On Item") );
#else /* AIX_ILS */
   labelString = XmStringCreateLtoR ((GETMESSAGE(2, 26, "On Item")),
                                                        XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
   XtSetArg(args[n], XmNlabelString, labelString);  n++;
   WidgList[count++]= XmCreatePushButton(helpPulldown, "onItem", args, n );
   XtAddCallback(WidgList[count-1], XmNactivateCallback,
       (XtCallbackProc)HelpModeCB, (XtPointer)NULL);
   XmStringFree(labelString);
   XtFree(mnemonic);

   WidgList[count++]= (Widget)XmCreateSeparatorGadget(helpPulldown,
                                                       "separator",args,0);

   n = 0;
   mnemonic = GETMESSAGE(2, 27, "U");
   mnemonic = XtNewString(mnemonic);
#ifdef AIX_ILS
   XtSetArg(args[n], XmNmnemonic, XStringToKeysym( mnemonic ) ); n++;
#else /* AIX_ILS */
   XtSetArg(args[n], XmNmnemonic, mnemonic[0]); n++;
#endif /* AIX_ILS */
#ifdef AIX_ILS
   labelString = XmStringCreateLocalized ( GETMESSAGE(2, 28, "Using Help") );
#else /* AIX_ILS */
   labelString = XmStringCreateLtoR ((GETMESSAGE(2, 28, "Using Help")),
                                                        XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
   XtSetArg(args[n], XmNlabelString, labelString); n++;
   WidgList[count++]=
       XmCreatePushButton(helpPulldown, "usingHelp", args, n );
   XtAddCallback(WidgList[count-1], XmNactivateCallback,
       (XtCallbackProc)HelpRequestCB, (XtPointer)HELP_USING);
   XmStringFree(labelString);
   XtFree(mnemonic);

   WidgList[count++]= (Widget)XmCreateSeparatorGadget(helpPulldown,
                                                       "separator",args,0);

   n = 0;
   mnemonic = GETMESSAGE(2, 29, "A");
   mnemonic = XtNewString(mnemonic);
#ifdef AIX_ILS
   XtSetArg(args[n], XmNmnemonic, XStringToKeysym( mnemonic ) ); n++;
#else /* AIX_ILS */
   XtSetArg(args[n], XmNmnemonic, mnemonic[0]); n++;
#endif /* AIX_ILS */
#ifdef AIX_ILS
   labelString =
    	    XmStringCreateLocalized ( GETMESSAGE(2, 30, "About Calculator") );
#else /* AIX_ILS */
   labelString = XmStringCreateLtoR ((GETMESSAGE(2, 30, "About Calculator")),
                                                        XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
   XtSetArg(args[n], XmNlabelString, labelString); n++;
   WidgList[count++]= XmCreatePushButton(helpPulldown, "about", args, n );
   XtAddCallback(WidgList[count-1], XmNactivateCallback,
       (XtCallbackProc)HelpRequestCB, (XtPointer)HELP_VERSION);
   XmStringFree(labelString);
   XtFree(mnemonic);

   XtManageChildren(WidgList, count);

   /* Fine tune the menubar */
   XtSetArg(args[0], XmNmarginWidth, 2);
   XtSetArg(args[1], XmNmarginHeight, 2);
   XtSetValues(X->menubar, args, 2);

}


init_colors()
{
    int         colorUse;
    short       act, inact, prim, second;

    _XmGetPixelData (X->screen, &colorUse, pixels, &act, &inact, 
                                                      &prim, &second);

    if(pixels[0].bg == pixels[1].bg && pixels[1].bg == pixels[2].bg && 
       pixels[2].bg == pixels[3].bg && pixels[3].bg == pixels[4].bg &&
       pixels[4].bg == pixels[5].bg && pixels[5].bg == pixels[6].bg &&
       pixels[6].bg == pixels[7].bg && pixels[7].bg == 0)
         colorSrv = False;
    else
         colorSrv = True;

}

static void
do_memory( w, client_data, call_data )
Widget w ;
XtPointer client_data ;
XtPointer call_data ;
{
  int type = (int)client_data;

  if(type == MEM)
  {
     if (!v->rstate)
       {
         v->rstate = 1 ;
         make_registers(MEM) ;
       }
     win_display(FCP_REG, TRUE) ;
  }
  else
  {
     if (!v->frstate)
       {
         v->frstate = 1 ;
         make_registers(FIN) ;
       }
     win_display(FCP_FIN_REG, TRUE) ;
  }
  ignore_event = True;
  timerId = XtAppAddTimeOut (XtWidgetToApplicationContext (X->kframe), 300,
			     TimerEvent, (XtPointer) NULL);
}

void
read_resources()    /* Read all possible resources from the database. */
{
  int boolval, i, intval ;
  char str[MAXLINE] ;
  char *msg;

  /* set the accuracy variable */
  if(application_args.accuracy > 9)
     v->accuracy = 9;
  else if(application_args.accuracy < 0)
     v->accuracy = 0;
  else
     v->accuracy = application_args.accuracy;

  /* set the base */
  if(strcmp(application_args.base, "binary") == 0 ||
         strcmp(application_args.base, "bin") == 0)
     v->base = (enum base_type) 0 ;
  else if(strcmp(application_args.base, "octal") == 0 ||
                  strcmp(application_args.base, "oct") == 0)
     v->base = (enum base_type) 1 ;
  else if(strcmp(application_args.base, "decimal") == 0 ||
                  strcmp(application_args.base, "dec") == 0)
     v->base = (enum base_type) 2 ;
  else if(strcmp(application_args.base, "hexadecimal") == 0 ||
                  strcmp(application_args.base, "hex") == 0)
     v->base = (enum base_type) 3 ;
  else
  {
     msg = (char *) XtMalloc(strlen( opts[(int) O_BASE]) + 3);
     sprintf(msg, opts[(int) O_BASE]);
     _DtSimpleError (v->appname, DtWarning, NULL, msg);
     XtFree(msg);
     v->base = (enum base_type) 2;
  }

  /* set the display numeration */
  if(strcmp(application_args.display, "fixed") == 0) 
     v->dtype = (enum base_type) 1 ;
  else if(strcmp(application_args.display, "eng") == 0 ||
                  strcmp(application_args.display, "engineering") == 0)
     v->dtype = (enum base_type) 0 ;
  else if(strcmp(application_args.display, "scientific") == 0 ||
                  strcmp(application_args.display, "sci") == 0)
     v->dtype = (enum base_type) 2 ;
  else
  {
     msg = (char *) XtMalloc(strlen( opts[(int) O_DISPLAY]) + strlen(str) + 3);
     sprintf(msg, opts[(int) O_DISPLAY], str);
     _DtSimpleError (v->appname, DtWarning, NULL, msg);
     XtFree(msg);
     v->dtype = (enum base_type) 1;
  }

  /* set the mode */
  if(strcmp(application_args.mode, "scientific") == 0)
     v->modetype = (enum base_type) 2 ;
  else if(strcmp(application_args.mode, "financial") == 0)
     v->modetype = (enum base_type) 0 ;
  else if(strcmp(application_args.mode, "logical") == 0)
     v->modetype = (enum base_type) 1 ;
  else
  {
     msg = (char *) XtMalloc(strlen( opts[(int) O_MODE]) + strlen(str) + 3);
     sprintf(msg, opts[(int) O_MODE], str);
     _DtSimpleError (v->appname, DtWarning, NULL, msg);
     XtFree(msg);
     v->modetype = (enum base_type) 2;
  }

  /* set the display numeration */
  if(strcmp(application_args.trigType, "deg") == 0 ||
                  strcmp(application_args.trigType, "degrees") == 0)
     v->ttype = (enum base_type) 0 ;
  else if(strcmp(application_args.trigType, "rad") == 0 ||
                  strcmp(application_args.trigType, "radians") == 0)
     v->ttype = (enum base_type) 2 ;
  else if(strcmp(application_args.trigType, "grad") == 0 ||
                  strcmp(application_args.trigType, "gradients") == 0)
     v->ttype = (enum base_type) 1 ;
  else
  {
     msg = (char *) XtMalloc(strlen( opts[(int) O_TRIG]) + strlen(str) + 3);
     sprintf(msg, opts[(int) O_TRIG], str);
     _DtSimpleError (v->appname, DtWarning, NULL, msg);
     XtFree(msg);
     v->ttype = (enum base_type) 0;
  }

}

void
close_cf(widget, client_data, call_data)
Widget widget ;
XtPointer client_data, call_data ;
{
  Arg args[1];

  XtSetArg (args[0], XmNdefaultButton, NULL);
  XtSetValues (X->CFframe, args, 1);

  XtUnmanageChild(X->CFframe) ;
  ignore_event = True;
  timerId = XtAppAddTimeOut (XtWidgetToApplicationContext (X->kframe), 300,
			     TimerEvent, (XtPointer) NULL);
}

void
close_ascii(widget, client_data, call_data)
Widget widget ;
XtPointer client_data, call_data ;
{
  XtUnmanageChild(X->Aframe) ;
  ignore_event = True;
  timerId = XtAppAddTimeOut (XtWidgetToApplicationContext (X->kframe), 300,
			     TimerEvent, (XtPointer) NULL);
}

void
FocusInCB(widget, client_data, call_data)
Widget widget ;
XtPointer client_data, call_data ;
{
  Arg args[1];

  XtSetArg (args[0], XmNdefaultButton, NULL);
  XtSetValues (X->CFframe, args, 1);

  XtSetArg(args[0], XmNshowAsDefault, True);
  XtSetValues(X->CFpi_butOK, args, 1);
}

void
move_cf(widget, client_data, call_data)
Widget widget ;
XtPointer client_data, call_data ;
{
  char *input;
  int value;
  Arg args[3];

  value = 0;
  if(widget == X->CFpi_cftext->textfield)
  {
     XtSetArg (args[0], XmNdefaultButton, NULL);
     XtSetValues (X->CFframe, args, 1);

     XtSetArg(args[0], XmNshowAsDefault, True);
     XtSetValues(X->CFpi_butOK, args, 1);

     input = XmTextFieldGetString(X->CFpi_cftext->textfield);
     if(strcmp(input, "") != 0)
        XmProcessTraversal(X->CFpi_dtext->textfield, XmTRAVERSE_CURRENT);
  }
  else if(widget == X->CFpi_dtext->textfield)
  {
     XtSetArg (args[0], XmNdefaultButton, NULL);
     XtSetValues (X->CFframe, args, 1);

     XtSetArg(args[0], XmNshowAsDefault, True);
     XtSetValues(X->CFpi_butOK, args, 1);

     XmProcessTraversal(X->CFpi_vtext->textfield, XmTRAVERSE_CURRENT);
     value = 1;
  }
  else if(widget == X->CFpi_vtext->textfield)
  {
     input = XmTextFieldGetString(X->CFpi_vtext->textfield);
     if(strcmp(input, "") != 0)
        write_cf_value(widget, client_data, call_data);
  }
  ignore_event = True;
  timerId = XtAppAddTimeOut (XtWidgetToApplicationContext (X->kframe), 300,
			     TimerEvent, (XtPointer) value);
}

create_popup(parent)
Widget parent;
{
  int n;
  char *mnemonic;
  Arg args[10];
  XmString label;
  Widget dummyHelp1, dummyHelp2, memRegs;
  Widget help, helpI, helpToc, helpT, helpR, helpO, helpU, helpV; 

  X->popupMenu = XmCreatePopupMenu(parent, "popup", NULL, 0) ;
  XtCreateManagedWidget(GETMESSAGE(2, 46,"Calculator Popup"), 
                        xmLabelWidgetClass, X->popupMenu, NULL, 0) ;
  XtCreateManagedWidget("separator", xmSeparatorWidgetClass,
                        X->popupMenu, NULL, 0) ;
  XtCreateManagedWidget("separator", xmSeparatorWidgetClass,
                        X->popupMenu, NULL, 0) ;


/*  The popup menu contains the following entries ......................*/
/* 1.  Memory Registers ...  */
#ifdef AIX_ILS
  memRegs = XtVaCreateManagedWidget( "memRegs1",
				    xmPushButtonWidgetClass,
				    X->popupMenu,
				    XmNlabelString, XmStringCreateLocalized(
	                               GETMESSAGE(2, 15, "Memory Registers") ),
				    XmNmnemonic, XStringToKeysym(
						      GETMESSAGE(2, 40, "R") ),
				    NULL);
#else /* AIX_ILS */
  memRegs = XtVaCreateManagedWidget(GETMESSAGE(2, 15, "Memory Registers"),
                                xmPushButtonWidgetClass,
                                X->popupMenu,
                                NULL);
#endif /* AIX_ILS */
  XtAddCallback(memRegs, XmNactivateCallback, (XtCallbackProc)do_memory,
                                                               (XtPointer)MEM);

/* 2.  Finacial Registers ...  */
#ifdef AIX_ILS
  memRegs = XtVaCreateManagedWidget( "memRegs2",
				    xmPushButtonWidgetClass,
				    X->popupMenu,
				    XmNlabelString, XmStringCreateLocalized(
                                   GETMESSAGE(3, 138, "Financial Registers") ),
				    XmNmnemonic, XStringToKeysym(
						GETMESSAGE( 2, 45, "g" ) ),
				    NULL);
#else /* AIX_ILS */
  memRegs = XtVaCreateManagedWidget(GETMESSAGE(3, 138, "Financial Registers"),
                                xmPushButtonWidgetClass,
                                X->popupMenu,
                                NULL);
#endif /* AIX_ILS */
  XtAddCallback(memRegs, XmNactivateCallback, (XtCallbackProc)do_memory,
                                                               (XtPointer)FIN);

/* 3.  Ascii Converter ...  */
#ifdef AIX_ILS
  X->asciiConv = XtVaCreateManagedWidget("asciiConv",
					 xmPushButtonWidgetClass,
					 X->popupMenu,
					 XmNlabelString,
                 XmStringCreateLocalized( GETMESSAGE(2, 16, "ASCII Convert") ),
					 XmNmnemonic, XStringToKeysym(
						      GETMESSAGE(2, 41, "A") ),
					 NULL);
#else /* AIX_ILS */
  X->asciiConv = XtVaCreateManagedWidget(GETMESSAGE(2, 16, "ASCII Convert"),
                                xmPushButtonWidgetClass,
                                X->popupMenu,
                                NULL);
#endif /* AIX_ILS */
  XtAddCallback(X->asciiConv, XmNactivateCallback, (XtCallbackProc)do_ascii,
                                                         (XtPointer)NULL);

/* Commented out by brianc to make ASCII option visible in multibyte locales
  if(_DtNl_is_multibyte)
     XtSetSensitive(X->asciiConv, False); */

  XtCreateManagedWidget("separator", xmSeparatorWidgetClass,
                                                  X->popupMenu, NULL, 0) ;

/* 4.  Enter Functions ...  */
#ifdef AIX_ILS
  X->enterFun = XtVaCreateManagedWidget("enterFun",
					xmPushButtonWidgetClass,
					X->popupMenu,
					XmNlabelString,
			    XmStringCreateLocalized( vstrs[(int) V_FUNWNAME] ),
					XmNmnemonic, XStringToKeysym(
						      GETMESSAGE(2, 42, "F") ),
                                NULL);
#else /* AIX_ILS */
  X->enterFun = XtVaCreateManagedWidget(vstrs[(int) V_FUNWNAME],
                                xmPushButtonWidgetClass,
                                X->popupMenu,
                                NULL);
#endif /* AIX_ILS */
  XtAddCallback(X->enterFun, XmNactivateCallback, (XtCallbackProc)new_cf_value,
                                                      (XtPointer)(int)M_FUN);

/* 5.  Enter Constants ...  */
#ifdef AIX_ILS
  X->enterConst = XtVaCreateManagedWidget("enterConst",
					  xmPushButtonWidgetClass,
					  X->popupMenu,
					  XmNlabelString,
			    XmStringCreateLocalized( vstrs[(int) V_CONWNAME] ),
					  XmNmnemonic, XStringToKeysym(
						      GETMESSAGE(2, 43, "C") ),
					  NULL);
#else /* AIX_ILS */
  X->enterConst = XtVaCreateManagedWidget(vstrs[(int) V_CONWNAME],
                                xmPushButtonWidgetClass,
                                X->popupMenu,
                                NULL);
#endif /* AIX_ILS */
  XtAddCallback(X->enterConst, XmNactivateCallback,(XtCallbackProc)new_cf_value,
                                                      (XtPointer)(int)M_CON);

  XtCreateManagedWidget("separator", xmSeparatorWidgetClass,
                                                  X->popupMenu, NULL, 0) ;


/* 6.   Help ... */

  dummyHelp1 = XtVaCreatePopupShell ("dummyHelp1",
                xmMenuShellWidgetClass,         
                X->popupMenu,
  		XmNwidth, 10,
		XmNheight, 10,
                NULL);

  dummyHelp2 = XtVaCreateWidget("dummyHelp2",
                xmRowColumnWidgetClass, dummyHelp1,
                XmNrowColumnType, XmMENU_PULLDOWN,
                NULL);

#ifndef AIX_ILS
  n = 0;
  mnemonic = GETMESSAGE(2, 19, "v");
  mnemonic = XtNewString (mnemonic);
  XtSetArg (args[n], XmNmnemonic, mnemonic[0]);
#endif /* AIX_ILS */

#ifdef AIX_ILS
  label = XmStringCreateLocalized ( GETMESSAGE(2, 20, "Overview") );
#else /* AIX_ILS */
  label = XmStringCreateLtoR (GETMESSAGE(2, 20, "Overview"), XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
  helpI = XtVaCreateManagedWidget ("introduction",
                xmPushButtonGadgetClass, dummyHelp2,
                XmNlabelString, label,
#ifdef AIX_ILS
		XmNmnemonic, XStringToKeysym( GETMESSAGE(2, 19, "v") ),
#endif /* AIX_ILS */
                NULL);


  XtAddCallback(helpI, XmNactivateCallback,
                 (XtCallbackProc)HelpRequestCB, (XtPointer)HELP_INTRODUCTION);
  XmStringFree (label);
#ifndef AIX_ILS
  XtFree(mnemonic);
#endif /* AIX_ILS */
  
  XtCreateManagedWidget("separator", xmSeparatorWidgetClass,
                        dummyHelp2, NULL, 0);

#ifndef AIX_ILS
  n = 0;
  mnemonic = GETMESSAGE(2, 38, "C");
  mnemonic = XtNewString(mnemonic);
  XtSetArg (args[n], XmNmnemonic, mnemonic[0]);
#endif /* AIX_ILS */
#ifdef AIX_ILS
  label = XmStringCreateLocalized ( GETMESSAGE(2, 39, "Table of Contents") );
#else /* AIX_ILS */
  label = XmStringCreateLtoR (GETMESSAGE(2, 39, "Table of Contents"), 
                                                    XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
  helpToc = XtVaCreateManagedWidget ("toc",
                xmPushButtonGadgetClass, dummyHelp2,
                XmNlabelString, label,
#ifdef AIX_ILS
		XmNmnemonic, XStringToKeysym( GETMESSAGE(2, 38, "C") ),
#endif /* AIX_ILS */
                NULL);

  XtAddCallback(helpToc, XmNactivateCallback,
                (XtCallbackProc)HelpRequestCB, (XtPointer)HELP_TABLEOFCONTENTS);

  XmStringFree(label);
#ifndef AIX_ILS 
  XtFree (mnemonic);
#endif /* AIX_ILS */

  
#ifndef AIX_ILS
  n = 0;
  mnemonic = GETMESSAGE(2, 21, "T");
  mnemonic = XtNewString (mnemonic);
  XtSetArg (args[n], XmNmnemonic, mnemonic[0]);
#endif /* AIX_ILS */
#ifdef AIX_ILS
  label = XmStringCreateLocalized ( GETMESSAGE(2, 22, "Tasks") );
#else /* AIX_ILS */
  label = XmStringCreateLtoR (GETMESSAGE(2, 22, "Tasks"), XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
  helpT = XtVaCreateManagedWidget ("tasks",
                xmPushButtonGadgetClass, dummyHelp2,
                XmNlabelString, label,
#ifdef AIX_ILS
		XmNmnemonic, XStringToKeysym( GETMESSAGE(2, 21, "T") ),
#endif /* AIX_ILS */
                NULL);

  XtAddCallback(helpT, XmNactivateCallback,
                 (XtCallbackProc)HelpRequestCB, (XtPointer)HELP_TASKS);

  XmStringFree(label);
#ifndef AIX_ILS
  XtFree (mnemonic);
#endif /* AIX_ILS */

#ifndef AIX_ILS
  n = 0;
  mnemonic = GETMESSAGE(2, 23, "R");
  mnemonic = XtNewString (mnemonic);
  XtSetArg (args[n], XmNmnemonic, mnemonic[0]);
#endif /* AIX_ILS */

#ifdef AIX_ILS
  label = XmStringCreateLocalized ( GETMESSAGE(2, 24, "Reference") );
#else /* AIX_ILS */
  label = XmStringCreateLtoR (GETMESSAGE(2, 24, "Reference"), XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
  helpR = XtVaCreateManagedWidget ("reference",
                xmPushButtonGadgetClass, dummyHelp2,
                XmNlabelString, label,
#ifdef AIX_ILS
		XmNmnemonic, XStringToKeysym( GETMESSAGE(2, 23, "R" ) ),
#endif /* AIX_ILS */
                NULL);


  XtAddCallback(helpR, XmNactivateCallback,
                 (XtCallbackProc)HelpRequestCB, (XtPointer)HELP_REFERENCE);

  XmStringFree(label);
#ifndef AIX_ILS 
  XtFree (mnemonic);
#endif /* AIX_ILS */


#ifndef AIX_ILS
  n = 0;
  mnemonic = GETMESSAGE(2, 25, "O");
  mnemonic = XtNewString (mnemonic);
  XtSetArg (args[n], XmNmnemonic, mnemonic[0]);
#endif /* AIX_ILS */
#ifdef AIX_ILS
  label = XmStringCreateLocalized ( GETMESSAGE(2, 26, "On Item") );
#else /* AIX_ILS */
  label = XmStringCreateLtoR (GETMESSAGE(2, 26, "On Item"), XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */

  helpO = XtVaCreateManagedWidget ("onItem",
                xmPushButtonGadgetClass, dummyHelp2,
                XmNlabelString, label,
#ifdef AIX_ILS
		XmNmnemonic, XStringToKeysym( GETMESSAGE(2, 25, "O") ),
#endif /* AIX_ILS */
                NULL);

  XtAddCallback(helpO, XmNactivateCallback,
                 (XtCallbackProc)HelpModeCB, (XtPointer)NULL);

  XtCreateManagedWidget("separator", xmSeparatorWidgetClass,
                        dummyHelp2, NULL, 0);
  XmStringFree (label);
#ifndef AIX_ILS 
  XtFree(mnemonic); 
#endif /* AIX_ILS */

#ifndef AIX_ILS
  n = 0;
  mnemonic = GETMESSAGE(2, 27, "U");
  mnemonic = XtNewString (mnemonic);
  XtSetArg (args[n], XmNmnemonic, mnemonic[0]);
#endif /* AIX_ILS */

#ifdef AIX_ILS
  label = XmStringCreateLocalized ( GETMESSAGE(2, 28, "Using Help") );
#else /* AIX_ILS */
  label = XmStringCreateLtoR (GETMESSAGE(2, 28, "Using Help"), XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
  helpU = XtVaCreateManagedWidget ("useHelp",
                xmPushButtonGadgetClass, dummyHelp2,
                XmNlabelString, label,
#ifdef AIX_ILS
		XmNmnemonic, XStringToKeysym( GETMESSAGE(2, 27, "U") ),
#endif /* AIX_ILS */
                NULL);


  XtAddCallback(helpU, XmNactivateCallback,
                 (XtCallbackProc)HelpRequestCB, (XtPointer)HELP_USING);

  XtCreateManagedWidget("separator", xmSeparatorWidgetClass,
                        dummyHelp2, NULL, 0);

  XmStringFree(label);
#ifndef AIX_ILS
  XtFree (mnemonic);
#endif /* AIX_ILS */
 
#ifndef AIX_ILS
  n = 0;
  mnemonic = GETMESSAGE(2, 29, "A");
  mnemonic = XtNewString (mnemonic);
  XtSetArg (args[n], XmNmnemonic, mnemonic[0]);
#endif /* AIX_ILS */

#ifdef AIX_ILS
  label = XmStringCreateLocalized (GETMESSAGE(2, 30, "About Calculator") );
#else /* AIX_ILS */
  label = XmStringCreateLtoR (GETMESSAGE(2, 30, "About Calculator"), 
                                                     XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
  helpV = XtVaCreateManagedWidget ("version",
                xmPushButtonGadgetClass, dummyHelp2,
                XmNlabelString, label,
#ifdef AIX_ILS
		XmNmnemonic, XStringToKeysym( GETMESSAGE(2, 29, "A" ) ),
#endif /* AIX_ILS */
                NULL);

  XtAddCallback(helpV, XmNactivateCallback,
                 (XtCallbackProc)HelpRequestCB, (XtPointer)HELP_VERSION);

  XmStringFree(label);
#ifndef AIX_ILS
  XtFree (mnemonic);
#endif /* AIX_ILS */
 
  
  mnemonic = GETMESSAGE(2, 17, "H");
#ifdef AIX_ILS
  label = XmStringCreateLocalized ( GETMESSAGE(2, 47, "Help") );
#else /* AIX_ILS */
  label = XmStringCreateLtoR (GETMESSAGE(2, 47, "Help"), XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
  help = XtVaCreateManagedWidget("help",
		xmCascadeButtonGadgetClass,  X->popupMenu,
		XmNsubMenuId, 		    dummyHelp2,
#ifdef AIX_ILS 
		XmNmnemonic,		    XStringToKeysym( mnemonic ),
#else /* AIX_ILS */
		XmNmnemonic,		    mnemonic,
#endif /* AIX_ILS */
		XmNlabelString,		    label,
		NULL);
  XmStringFree(label);
  
  XtCreateManagedWidget("separator", xmSeparatorWidgetClass,
                        X->popupMenu, NULL, 0);
  
/* 7.  Exit */
#ifdef AIX_ILS
  label = XmStringCreateLocalized ( GETMESSAGE(3, 403, "Exit") );
#else /* AIX_ILS */
  label = XmStringCreateLtoR ((GETMESSAGE(3, 403, "Exit")), 
                                                  XmFONTLIST_DEFAULT_TAG);
#endif /* AIX_ILS */
  X->Close = XtVaCreateManagedWidget("close",
                                xmPushButtonWidgetClass,
                                X->popupMenu,
		                XmNlabelString,		    label,
#ifdef AIX_ILS
				XmNmnemonic,
				      XStringToKeysym(GETMESSAGE(2, 44, "X" )),
#endif /* AIX_ILS */
                                NULL);
  XmStringFree(label);
  XtAddCallback(X->Close, XmNactivateCallback,(XtCallbackProc)do_frame,
                                                      (XtPointer)(int)M_FUN);

  XtAddEventHandler(parent, ButtonPressMask, FALSE,
                                     popupHandler, (XtPointer) NULL) ;
  XtAddCallback(X->popupMenu, XmNmapCallback,(XtCallbackProc)map_popup,
                                                      (XtPointer)NULL);
}

static void
popupHandler(widget, client_data, event, continue_to_dispatch)
Widget widget ;
XtPointer client_data ;
XEvent *event ;
Boolean *continue_to_dispatch ;
{
  XButtonPressedEvent *bevent;

  bevent = (XButtonPressedEvent *)event;

  if (event->type == ButtonPress && event->xbutton.button == Button3)
  {
     if(bevent->x >= funBtn->core.x && 
        bevent->x <= funBtn->core.x + funBtn->core.width && 
        bevent->y >= funBtn->core.y + 
                     X->modeline->core.height + X->textForm->core.height && 
        bevent->y <= funBtn->core.y +funBtn->core.height + 
                     X->modeline->core.height + X->textForm->core.height) 
     {
        menu_handler(widget, (XtPointer)M_FUN, event, continue_to_dispatch);
     }
     else if(bevent->x >= constBtn->core.x && 
        bevent->x <= constBtn->core.x + constBtn->core.width && 
        bevent->y >= constBtn->core.y + 
                     X->modeline->core.height + X->textForm->core.height && 
        bevent->y <= constBtn->core.y + constBtn->core.height + 
                     X->modeline->core.height + X->textForm->core.height) 
     {
        menu_handler(widget, (XtPointer)M_CON, event, continue_to_dispatch);
     }
     else if(bevent->x >= accBtn->core.x && 
        bevent->x <= accBtn->core.x + accBtn->core.width && 
        bevent->y >= accBtn->core.y + 
                     X->modeline->core.height + X->textForm->core.height && 
        bevent->y <= accBtn->core.y + accBtn->core.height + 
                     X->modeline->core.height + X->textForm->core.height) 
     {
        menu_handler(widget, (XtPointer)M_ACC, event, continue_to_dispatch);
     }
     else if(bevent->x >= rclBtn->core.x && 
        bevent->x <= rclBtn->core.x + rclBtn->core.width && 
        bevent->y >= rclBtn->core.y + 
                     X->modeline->core.height + X->textForm->core.height && 
        bevent->y <= rclBtn->core.y + rclBtn->core.height + 
                     X->modeline->core.height + X->textForm->core.height) 
     {
        menu_handler(widget, (XtPointer)M_RCL, event, continue_to_dispatch);
     }
     else if(bevent->x >= stoBtn->core.x && 
        bevent->x <= stoBtn->core.x + stoBtn->core.width && 
        bevent->y >= stoBtn->core.y + 
                     X->modeline->core.height + X->textForm->core.height && 
        bevent->y <= stoBtn->core.y + stoBtn->core.height + 
                     X->modeline->core.height + X->textForm->core.height) 
     {
        menu_handler(widget, (XtPointer)M_STO, event, continue_to_dispatch);
     }
     else if(bevent->x >= exchBtn->core.x && 
        bevent->x <= exchBtn->core.x + exchBtn->core.width && 
        bevent->y >= exchBtn->core.y + 
                     X->modeline->core.height + X->textForm->core.height && 
        bevent->y <= exchBtn->core.y + exchBtn->core.height + 
                     X->modeline->core.height + X->textForm->core.height) 
     {
        menu_handler(widget, (XtPointer)M_EXCH, event, continue_to_dispatch);
     }
     else
     {
        XmMenuPosition(X->popupMenu, (XButtonPressedEvent *) event) ;
        XtManageChild(X->popupMenu) ;
     }
  }
}

void
set_option_menu(type, base)
int type;
int base;
{
    Arg args[1];

    if(type == BASEITEM)
       XtSetArg(args[0], XmNmenuHistory, X->baseWidgArry[base]);
    else if(type == NUMITEM)
       XtSetArg(args[0], XmNmenuHistory, X->numWidgArry[base]);
    else
       XtSetArg(args[0], XmNmenuHistory, X->ttypeWidgArry[base]);
    XtSetValues( X->modevals[type], args, 1);
}

void
map_popup(widget, client_data, call_data)
Widget widget ;
XtPointer client_data, call_data ;
{
   XmAnyCallbackStruct * callback;
   XEvent * event;
   XKeyPressedEvent *key_event ;

   callback = (XmAnyCallbackStruct *) call_data;
   event = (XEvent *) callback->event;
   key_event = (XKeyPressedEvent *) event ;

   if(event->type != KeyRelease)
      return;

   if (v->event_type == F4_PRESS)
      XmMenuPosition(widget, (XButtonPressedEvent *) event) ;
}

/*ARGSUSED*/
static void
save_state(widget, client_data, call_data)
Widget widget ; 
XtPointer client_data ;
XtPointer call_data ;
{
   char * full_path=NULL;
   char * file_name=NULL;
   char * strPtr;
   int    restore=NORMAL_RESTORE;
   Boolean status=FALSE;
   char * command;
   int command_len;

   status = DtSessionSavePath(widget, &full_path, &file_name);

   if (status)
   {
      SaveSession (full_path, file_name);
   }
   else if(dt_path != NULL)
   {
      full_path = (char *) XtMalloc (sizeof (char) * MAX_PATH);

      (void) STRCPY (full_path, dt_path);
      (void) strcat (full_path, "/");
      (void) strcat (full_path, DTCALC_CLASS_NAME);
      SaveSession (full_path, full_path);
      status = True;
   }
   else
      status = False;

   command_len =
        strlen(v->appname) + strlen(" -session ")+ strlen(file_name) + 1;
   command = XtMalloc (sizeof (char) * command_len);
   (void) STRCPY (command, v->appname);
   if(status)
   {
      (void) strcat (command, " -session ");
      (void) strcat (command, file_name);
   }

   XChangeProperty (X->dpy, XtWindow (X->kframe), command_atom,
                    XA_STRING, 8, PropModeReplace, (unsigned char *)command, 
                    command_len);
   XSync(X->dpy, False);

   XtFree ((char *) full_path);
   XtFree ((char *) file_name);
}

static void
SaveSession( path, file_name )
        char *path ;
        char *file_name ;
{
   char workspaceNumber[5];
   int fd;
   Atom * ws_presence = NULL;
   char * workspace_name;
   unsigned long num_workspaces = 0;
   Atom actual_type;
   int  actual_format;
   unsigned long nitems;
   unsigned long leftover;
   WM_STATE * wm_state;

   char tempStr[50];
   Position x, y;
   Dimension width, height;

   char * command;
   int command_len;
   int i;
   int j;
   char * msg;
   int view_index;
   char * tmpStr;
   XmVendorShellExtObject vendorExt;
   XmWidgetExtData        extData;
   Window                 junkWindow;
   int                    t_x, t_y;
   Arg                    args[5];

     /*  Create the session file  */

   if ((fd = creat (path, S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP)) == -1)
   {
      tmpStr = GETMESSAGE(2, 34, "Could not open the session file.");
      msg = XtNewString(tmpStr);
      _DtSimpleError (v->appname, DtError, NULL, msg);
      XtFree(msg);
      return;
   }
   close(fd);

   /*  Getting the WM_STATE property to see if iconified or not */
   XGetWindowProperty (X->dpy, XtWindow (X->kframe),
                             wm_state_atom, 0L, (long) BUFSIZ, False,
                             wm_state_atom, &actual_type, &actual_format,
                             &nitems, &leftover, (unsigned char **) &wm_state);

   /* Write out if iconified our not */
   if (wm_state->state == IconicState)
        put_resource(R_ICON,     set_bool(True)) ;
   else
        put_resource(R_ICON,     set_bool(False)) ;

   /* does it have a menubar or not */
   if ( application_args.menuBar )
        put_resource(R_MENUBAR,     set_bool(True)) ;
   else
        put_resource(R_MENUBAR,     set_bool(False)) ;

   /* is keys set to on/off */
   if (v->tstate)
        put_resource(R_KEYS,     set_bool(True)) ;
   else
        put_resource(R_KEYS,     set_bool(False)) ;

   /*  Get the workspaces for this dt by accessing the property.  */

   if (DtWsmGetWorkspacesOccupied (X->dpy, XtWindow (X->kframe),
                                  &ws_presence, &num_workspaces) == Success)
   {
      char *string;

      string = (char *)XtMalloc(num_workspaces * 40);
      for (j = 0; j < num_workspaces; j++)
      {
         workspace_name = XGetAtomName (X->dpy, ws_presence[j]);
         if(j == 0)
            strcpy(string, workspace_name);
         else
            strcat(string, workspace_name);
         if(j + 1 != num_workspaces)
            strcat(string, " ");
         XtFree ((char *) workspace_name);
      }
      put_resource(R_WORKSPACE,     string) ;

      XFree((char *)ws_presence);
      XtFree((char *)string);
   }



   XtSetArg(args[0], XmNwidth, &width);
   XtSetArg(args[1], XmNheight, &height);
   XtGetValues(X->kframe, args, 2);

   XTranslateCoordinates(XtDisplay(X->kframe),
                       XtWindow(X->kframe),
                       RootWindowOfScreen(XtScreen(X->kframe)),
                       0, 0, &t_x, &t_y,
                       &junkWindow);
   x = (Position) t_x;
   y = (Position) t_y;

   /* Modify x & y to take into account window mgr frames */
   extData=_XmGetWidgetExtData(X->kframe, XmSHELL_EXTENSION);
   vendorExt = (XmVendorShellExtObject)extData->widget;
   x -= vendorExt->vendor.xOffset;
   y -= vendorExt->vendor.yOffset;

   sprintf(tempStr, "%d", width);
   put_resource(R_WIDTH,     tempStr) ;
   sprintf(tempStr, "%d", height);
   put_resource(R_HEIGHT,     tempStr) ;
   sprintf(tempStr, "%d", x);
   put_resource(R_X,     tempStr) ;
   sprintf(tempStr, "%d", y);
   put_resource(R_Y,     tempStr) ;

   write_resources(path);
}

void
RestoreSession()
{
   Boolean status=False;
   char *path, *msg;
   char str[MAXLINE] ;
   char temp[MAXLINE] ;
   char * full_path = NULL;
   XrmDatabase db;
   int boolval, i, intval ;
   double result;
   int MPtemp[MP_SIZE];

   status = DtSessionRestorePath(X->kframe, &full_path, 
                                 application_args.session);
   if (!status)
      return;

   path = XtNewString(full_path);

   db = XrmGetFileDatabase(path) ;
   XrmMergeDatabases(db, &(X->rDB)) ;

   if (get_int_resource(R_ACCURACY, &intval))
    {
      v->accuracy = intval ;
      if (v->accuracy < 0 || v->accuracy > 9)
        {
          msg = (char *) XtMalloc(strlen( opts[(int) O_ACCRANGE]) + 3);
          sprintf(msg, opts[(int) O_ACCRANGE]);
          _DtSimpleError (v->appname, DtWarning, NULL, msg);
          XtFree(msg);
          v->accuracy = 2 ;
        }
    }

   if ((full_path = get_resource(R_BASE)) != NULL)
    {
      for (i = 0; i < MAXBASES; i++)
        if (EQUAL(full_path, base_str[i])) break ;

      if (i == MAXBASES)
      {
          msg = (char *) XtMalloc(strlen( opts[(int) O_BASE]) + 3);
          sprintf(msg, opts[(int) O_BASE]);
          _DtSimpleError (v->appname, DtWarning, NULL, msg);
          XtFree(msg);
      }
      else
        {
          v->base = (enum base_type) i ;
        }
    }

   if (get_str_resource(R_DISPLAY, str))
    {
      for (i = 0; i < MAXDISPMODES; i++)
        if (EQUAL(str, dtype_str[i])) break ;

      if (i == MAXDISPMODES)
      {
          msg = (char *) XtMalloc(strlen( opts[(int) O_DISPLAY]) + 
                                                          strlen(str) + 3);
          sprintf(msg, opts[(int) O_DISPLAY], str);
          _DtSimpleError (v->appname, DtWarning, NULL, msg);
          XtFree(msg);
      }
      else v->dtype = (enum num_type) i ;
    }

   if (get_str_resource(R_MODE, str))
    {
      for (i = 0; i < MAXMODES; i++)
        if (EQUAL(str, mode_str[i])) break ;

      if (i == MAXMODES)
      {
          msg = (char *)XtMalloc(strlen( opts[(int) O_MODE]) + strlen(str) + 3);
          sprintf(msg, opts[(int) O_MODE], str);
          _DtSimpleError (v->appname, DtWarning, NULL, msg);
          XtFree(msg);
      }
      else v->modetype = (enum mode_type) i ;
    }

   if (get_str_resource(R_TRIG, str))
    {
      for (i = 0; i < MAXTRIGMODES; i++)
        if (EQUAL(str, ttype_str[i])) break ;

      if (i == MAXTRIGMODES)
      {
          msg = (char *)XtMalloc(strlen( opts[(int) O_TRIG]) + strlen(str) + 3);
          sprintf(msg, opts[(int) O_TRIG], str);
          _DtSimpleError (v->appname, DtWarning, NULL, msg);
          XtFree(msg);
      }
      else v->ttype = (enum trig_type) i ;
    }

   if (get_bool_resource(R_REGS,   &boolval)) v->rstate     = boolval ;

   /* Get the iconify state */
   if (get_bool_resource(R_ICON,   &boolval)) v->iconic    = boolval;

   /* Get the menubar state */
   if (get_bool_resource(R_MENUBAR,   &boolval)) 
      application_args.menuBar = boolval;

   /* Get the keys state */
   if (get_bool_resource(R_KEYS,   &boolval)) v->tstate    = boolval;

   /* Get the proper workspaces if needed */
   if ((full_path = get_resource(R_WORKSPACE)) != NULL)
      v->workspaces = XtNewString(full_path);

   /* Get the x, y width, and height */ 
   if (get_int_resource(R_WIDTH, &intval))
      v->width = intval ;
   if (get_int_resource(R_HEIGHT, &intval))
      v->height = intval ;
   if (get_int_resource(R_X, &intval))
      v->x = intval ;
   if (get_int_resource(R_Y, &intval))
      v->y = intval ;

   if (get_str_resource(R_DISPLAYED, str))
   {
      STRCPY(v->display, str);
      MPstr_to_num(str, v->base, v->MPdisp_val) ;
   }

   if (get_str_resource(R_REG0, str))
      MPstr_to_num(str, v->base, v->MPmvals[0]) ;
   if (get_str_resource(R_REG1, str))
      MPstr_to_num(str, v->base, v->MPmvals[1]) ;
   if (get_str_resource(R_REG2, str))
      MPstr_to_num(str, v->base, v->MPmvals[2]) ;
   if (get_str_resource(R_REG3, str))
      MPstr_to_num(str, v->base, v->MPmvals[3]) ;
   if (get_str_resource(R_REG4, str))
      MPstr_to_num(str, v->base, v->MPmvals[4]) ;
   if (get_str_resource(R_REG5, str))
      MPstr_to_num(str, v->base, v->MPmvals[5]) ;
   if (get_str_resource(R_REG6, str))
      MPstr_to_num(str, v->base, v->MPmvals[6]) ;
   if (get_str_resource(R_REG7, str))
      MPstr_to_num(str, v->base, v->MPmvals[7]) ;
   if (get_str_resource(R_REG8, str))
      MPstr_to_num(str, v->base, v->MPmvals[8]) ;
   if (get_str_resource(R_REG9, str))
      MPstr_to_num(str, v->base, v->MPmvals[9]) ;

   if (get_str_resource(R_FREG0, str))
   {
      MPstr_to_num(str, v->base, MPtemp) ;
      mpcmd(MPtemp, &(v->MPfvals[0]));
   }
   if (get_str_resource(R_FREG1, str))
   {
      MPstr_to_num(str, v->base, MPtemp) ;
      mpcmd(MPtemp, &(v->MPfvals[1]));
   }
   if (get_str_resource(R_FREG2, str))
   {
      MPstr_to_num(str, v->base, MPtemp) ;
      mpcmd(MPtemp, &(v->MPfvals[2]));
   }
   if (get_str_resource(R_FREG3, str))
   {
      MPstr_to_num(str, v->base, MPtemp) ;
      mpcmd(MPtemp, &(v->MPfvals[3]));
   }
   if (get_str_resource(R_FREG4, str))
   {
      MPstr_to_num(str, v->base, MPtemp) ;
      mpcmd(MPtemp, &(v->MPfvals[4]));
   }
   if (get_str_resource(R_FREG5, str))
   {
      MPstr_to_num(str, v->base, MPtemp) ;
      mpcmd(MPtemp, &(v->MPfvals[5]));
   }
      
}

static void
setCalcHints()
{
   char *ptr;
   Atom * workspace_atoms = NULL;
   int num_workspaces=0;
   Arg args[8];
   XWMHints   *wmhints;

   if(v->width != 0 && v->height != 0)
   {
      WMShellWidget wm = (WMShellWidget)(X->kframe);
      wm->wm.size_hints.flags |= USPosition;
      XtSetArg (args[0], XmNx, (Position)v->x);
      XtSetArg (args[1], XmNy, (Position)v->y);
      XtSetArg (args[2], XmNwidth, (Dimension)v->width);
      XtSetArg (args[3], XmNheight, (Dimension)v->height);
      XtSetValues (X->kframe, args, 4);
   }

   if (v->iconic)
   {
       /* add the iconify hint to the current shell */
       XtSetArg(args[0], XmNinitialState, IconicState);
       XtSetValues(X->kframe, args, 1);
   }
   else
   {
     /* Remove the iconify hint from the current shell */
     wmhints = XGetWMHints(X->dpy, XtWindow(X->kframe));
     wmhints->flags |= IconWindowHint;
     wmhints->initial_state = NormalState;
     XSetWMHints(X->dpy, XtWindow(X->kframe), wmhints);
     XFree(wmhints);
   }

   if (v->workspaces)
   {
      do
      {
         ptr = DtStrchr (v->workspaces, ' ');

         if (ptr != NULL) *ptr = NULL;

         workspace_atoms = (Atom *) XtRealloc ((char *)workspace_atoms,
                              sizeof (Atom) * (num_workspaces + 1));

         workspace_atoms[num_workspaces] = 
                              XmInternAtom (X->dpy, v->workspaces, True);

         num_workspaces++;

         if (ptr != NULL)
         {
            *ptr = ' ';
            v->workspaces = ptr + 1;
         }
      } while (ptr != NULL);

      DtWsmSetWorkspacesOccupied (X->dpy, XtWindow (X->kframe), 
                              workspace_atoms, num_workspaces);

      XtFree ((char *) workspace_atoms);
   }
}

XtTimerCallbackProc
TimerEvent( client_data, id )
        XtPointer client_data;
        XtIntervalId *id;
{
  Arg args[1];

  ignore_event = False;

  if(client_data)
  {
    XtSetArg (args[0], XmNdefaultButton, X->CFpi_butOK);
    XtSetValues (X->CFframe, args, 1);
  }
}

#ifdef hpux
static int
GetKeyboardID()
{
    XHPDeviceList *list, *slist;
    int ndevices = 0, i, kbd = 0;

    slist = XHPListInputDevices(X->dpy, &ndevices);
    for (i = 0, list = slist; i < ndevices; i++, list++)
    {
       if (list->type != KEYBOARD && strcmp(list->name, PS2_DIN_NAME))
          continue;
       if (list->detailed_id & (HP_ITF_KBD | HP_HIL) == (HP_ITF_KBD | HP_HIL))
       {
          kbd = HIL_ITF_KBD;
          return(kbd);
       }
       else if (list->detailed_id & (PC101_KBD|HP_HIL) == (PC101_KBD|HP_HIL))
       {
          kbd = HIL_PC101_KBD;
          return(kbd);
       }
       else if (list->detailed_id & (PC101_KBD|SERIAL) == (PC101_KBD|SERIAL))
       {
          kbd = SERIAL_PC101_KBD;
          return(kbd);
       }

       if (!kbd)
       {
          if (strcmp(list->name, PS2_DIN_NAME) == 0)
          {
             kbd = SERIAL_PC101_KBD;
             return(kbd);
          }
	  else if (list->hil_id >= FIRST_HIL_KBD && 
	                        	     list->hil_id <= LAST_HIL_KBD)
          {
             if (list->io_byte & LED_BITS)
	     {
	        kbd = HIL_PC101_KBD;
                return(kbd);
	     }
	     else
	     {
	        kbd = HIL_ITF_KBD;
                return(kbd);
 	     }
          }
       }
    }
    XHPFreeDeviceList (slist);
    return(0);
}
#endif

static char *
_DtcalcStripSpaces(file)
char *file;
{
   int i, j;

   if (file == NULL)
      return (file);

   for (i = 0; i < strlen(file); i++)
   {
      if (isspace(file[i]))
      {
         for(j = i; file[j] != NULL; j++)
            file[j] = file[j + 1];
      }
   }

   return (file);
}

