/* static char rcsid[] = 
	"$XConsortium: Control.c /main/cde1_maint/4 1995/10/09 09:34:09 pascale $";
*/
/**---------------------------------------------------------------------
***	
***	file:		Control.c
***
***	project:	MotifPlus Widgets
***
***	description:	Source code for DtControl class.
***	
***
***			(c) Copyright 1992 by Hewlett-Packard Company.
***
***
***-------------------------------------------------------------------*/


/*-------------------------------------------------------------
**	Include Files
*/

#ifdef SUN_CDE_PERF
/* Do this here so the rest of DtWidget can get by with
 * a simple _TS_ERRNO, so their semantics can't change
 * by threading this private widget.
 */
#define _REENTRANT

#include <fcntl.h>
#include <sys/param.h>

/* No libc stub for sema_destroy... */
/* Removed, this is a problem for the non-MT case */
/* sema_destroy currently doesn't do anything */
/* #pragma weak sema_destroy */
#endif

#include <stdio.h>
#ifdef __apollo
#include <sys/types.h>           /* needed for sys/stat.h on bsd4.3 */
#else
#include <stdlib.h>
#endif
#include <sys/stat.h>
#include <errno.h>
#include <Xm/GadgetP.h>
#include <Dt/Control.h>
#include <Dt/ControlP.h>
#include <Xm/ManagerP.h>
#include <Dt/MacrosP.h>
#include <time.h>
#ifdef AIX_ILS
#include <langinfo.h>
#endif /* AIX_ILS */


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget _DtCreateControl() ;

#else

extern Widget _DtCreateControl( 
                        Widget parent,
                        String name,
                        ArgList arglist,
                        int argcount) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static void ClickTimeout() ;
static void BusyTimeout() ;
static void DateTimeout() ;
static void FileCheckTimeout() ;
static void PushAnimationTimeout() ;
static void DropAnimationTimeout() ;
static void CheckFile() ;
static void ClassInitialize() ;
static void ClassPartInitialize() ;
static void Destroy() ;
static void Initialize() ;
static Boolean SetValues() ;
static void GetSize() ;
static void GetPositions() ;
static void Draw() ;
static void CallCallback() ;
static void UpdateGCs() ;
#ifdef AIX_ILS
static void ReplaceJPDate();
#endif /* AIX_ILS */

#else

static void ClickTimeout( 
                        XtPointer client_data,
                        XtIntervalId *id) ;
static void BusyTimeout( 
                        XtPointer client_data,
                        XtIntervalId *id) ;
static void DateTimeout( 
                        XtPointer client_data,
                        XtIntervalId *id) ;
static void PushAnimationTimeout( 
                        XtPointer client_data,
                        XtIntervalId *id) ;
static void DropAnimationTimeout( 
                        XtPointer client_data,
                        XtIntervalId *id) ;
static void FileCheckTimeout( 
                        XtPointer client_data,
                        XtIntervalId *id) ;
#ifdef SUN_CDE_PERF
static void CheckFile( 
		      DtControlGadget g,
		      struct stat* sbuf	/* can be NULL */
		);
#else
static void CheckFile( 
                        DtControlGadget g) ;
#endif
static void ClassInitialize( void ) ;
static void ClassPartInitialize(
                        WidgetClass cl) ;

static void Destroy (Widget);

static void Initialize( 
                        Widget request_w,
                        Widget new_w) ;
static Boolean SetValues(
        		Widget current_w,
                        Widget request_w,
                        Widget new_w) ;
static void GetSize( 
                        DtControlGadget g,
                        Dimension *w,
                        Dimension *h) ;
static void GetPositions( 
                        DtControlGadget g,
                        Position w,
                        Position h,
                        Dimension h_t,
                        Dimension s_t,
                        Position *pix_x,
                        Position *pix_y,
                        Position *str_x,
                        Position *str_y) ;
static void Draw( 
                        DtControlGadget g,
                        Drawable drawable,
                        Position x,
                        Position y,
                        Dimension w,
                        Dimension h,
                        Dimension h_t,
                        Dimension s_t,
                        unsigned char s_type,
                        unsigned char fill_mode) ;
static void CallCallback( 
                        DtControlGadget g,
                        XtCallbackList cb,
                        int reason,
                        XEvent *event) ;
static void UpdateGCs( 
                        DtControlGadget g) ;
#ifdef AIX_ILS
static void ReplaceJPDate(
                        char *date,
                        char *jpstr,
                        int wday) ;
#endif /* AIX_ILS */

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/


/*-------------------------------------------------------------
**	Public Interface
**-------------------------------------------------------------
*/

WidgetClass	dtControlGadgetClass;


#define UNSPECIFIED_DIMENSION	9999
#define UNSPECIFIED_CHAR	255

#ifdef _SUN_OS
static const char _DtMsgIndicator_0000[] = "/var/mail/";
#elif defined(__osf__)
static const char _DtMsgIndicator_0000[] = "/usr/spool/mail/";
#else
static const char _DtMsgIndicator_0000[] = "/usr/mail/";
#endif

#define	MAIL_DIR	_DtMsgIndicator_0000
#define Min(x, y)    (((x) < (y)) ? (x) : (y))
#define Max(x, y)    (((x) > (y)) ? (x) : (y))

#define G_Width(r)              (r -> rectangle.width)
#define G_Height(r)             (r -> rectangle.height)
#define G_Expose(r,e,reg) \
 (((RectObjClassRec *)r -> object.widget_class) -> rect_class.expose)(r,e,reg)


/*-------------------------------------------------------------
**	Resource List
*/
#define R_Offset(field) \
	XtOffset (DtControlGadget, control.field)
static XtResource resources[] = 
{
	{
		"_do_update",
		XmCBoolean, XmRBoolean, sizeof (Boolean),
		R_Offset (_do_update), XmRImmediate, (XtPointer) FALSE
	},
	{
		XmNuseEmbossedText,
		XmCBoolean, XmRBoolean, sizeof (Boolean),
		R_Offset (use_embossed_text), XmRImmediate, (XtPointer) TRUE
	},
	{
		XmNpushFunction,
		XmCPushFunction, XmRFunction, sizeof (XtPointer),
		R_Offset (push_function),
		XmRImmediate, (XtPointer) NULL
	},
	{
		XmNpushArgument,
		XmCPushArgument, XmRPointer, sizeof (XtPointer),
		R_Offset (push_argument),
		XmRImmediate, (XtPointer) NULL
	},
	{
		XmNdropAction,
		XmCDropAction, XmRString, sizeof (String),
		R_Offset (drop_action),
		XmRImmediate, (XtPointer) NULL
	},
	{
		"pushAction",
		"PushAction", XmRString, sizeof (String),
		R_Offset (push_action),
		XmRImmediate, (XtPointer) NULL
	},
	{
		XmNformat,
		XmCFormat, XmRString, sizeof (String),
		R_Offset (format),
		XmRImmediate, (XtPointer) "%b%n%e"
	},
#ifdef AIX_ILS
        {
                XmNformatJP,
                XmCFormatJP, XmRString, sizeof (String),
                R_Offset (format_jp),
                XmRImmediate, (XtPointer) NULL
        },
#endif /* AIX_ILS */
	{
		XmNsubpanel,
		XmCSubpanel, XmRWidget, sizeof (Widget),
		R_Offset (subpanel),
		XmRImmediate, (XtPointer) NULL
	},
	{
		XmNcontrolType,
		XmCControlType, XmRControlType, sizeof (unsigned char),
		R_Offset (control_type),
		XmRImmediate, (XtPointer) XmCONTROL_NONE
	},
	{
		XmNfileName,
		XmCString, XmRString, sizeof (String),
		R_Offset (file_name), XmRImmediate, (XtPointer) NULL
	},
	{
		XmNalternateImage,
		XmCAlternateImage, XmRString, sizeof (String),
		R_Offset (alt_image), XmRImmediate, (XtPointer) NULL
	},
	{
		XmNchime,
		XmCBoolean, XmRBoolean, sizeof (Boolean),
		R_Offset (chime), XmRImmediate, (XtPointer) FALSE
	},
	{
		XmNclientTimeoutInterval,
		XmCInterval, XmRInt, sizeof (int),
		R_Offset (max_blink_time), XmRImmediate, (XtPointer) 10000
	},
	{
		XmNwaitingBlinkRate,
		XmCInterval, XmRInt, sizeof (int),
		R_Offset (blink_time), XmRImmediate, (XtPointer) 500
	},
	{
		XmNmonitorTime,
		XmCInterval, XmRInt, sizeof (int),
		R_Offset (monitor_time), XmRImmediate, (XtPointer) 30000
	},
	{
		XmNpushButtonClickTime,
		XmCInterval, XmRInt, sizeof (int),
		R_Offset (click_time), 
		XmRImmediate, (XtPointer) 1000
	},
	{
		XmNuseLabelAdjustment,
		XmCBoolean, XmRBoolean, sizeof (Boolean),
		R_Offset (use_label_adj), XmRImmediate, (XtPointer) TRUE
	}
};
#undef	R_Offset


static XmBaseClassExtRec       controlBaseClassExtRec = {
    NULL,                                     /* Next extension       */
    NULLQUARK,                                /* record type XmQmotif */
    XmBaseClassExtVersion,                    /* version              */
    sizeof(XmBaseClassExtRec),                /* size                 */
    XmInheritInitializePrehook,               /* initialize prehook   */
    XmInheritSetValuesPrehook,                /* set_values prehook   */
    XmInheritInitializePosthook,              /* initialize posthook  */
    XmInheritSetValuesPosthook,               /* set_values posthook  */
    (WidgetClass)&dtControlCacheObjClassRec, /* secondary class      */
    XmInheritSecObjectCreate,                 /* creation proc        */
    XmInheritGetSecResData,                   /* getSecResData */
    {0},                                      /* fast subclass        */
    XmInheritGetValuesPrehook,                /* get_values prehook   */
    XmInheritGetValuesPosthook,               /* get_values posthook  */
    NULL,                                     /* classPartInitPrehook */
    NULL,                                     /* classPartInitPosthook*/
    NULL,                                     /* ext_resources        */
    NULL,                                     /* compiled_ext_resources*/
    0,                                        /* num_ext_resources    */
    FALSE,                                    /* use_sub_resources    */
    XmInheritWidgetNavigable,                 /* widgetNavigable      */
    XmInheritFocusChange,                     /* focusChange          */
};

/* ext rec static initialization */
externaldef (xmcontrolcacheobjclassrec)
DtControlCacheObjClassRec dtControlCacheObjClassRec =
{
  {
      /* superclass         */    (WidgetClass) &xmExtClassRec,
      /* class_name         */    "DtControl",
      /* widget_size        */    sizeof(DtControlCacheObjRec),
      /* class_initialize   */    NULL,
      /* chained class init */    NULL,
      /* class_inited       */    False,
      /* initialize         */    NULL,
      /* initialize hook    */    NULL,
      /* realize            */    NULL,
      /* actions            */    NULL,
      /* num_actions        */    0,
      /* resources          */    NULL,
      /* num_resources      */    0,
      /* xrm_class          */    NULLQUARK,
      /* compress_motion    */    False,
      /* compress_exposure  */    False,
      /* compress enter/exit*/    False,
      /* visible_interest   */    False,
      /* destroy            */    NULL,
      /* resize             */    NULL,
      /* expose             */    NULL,
      /* set_values         */    NULL,
      /* set values hook    */    NULL,
      /* set values almost  */    NULL,
      /* get values hook    */    NULL,
      /* accept_focus       */    NULL,
      /* version            */    XtVersion,
      /* callback offsetlst */    NULL,
      /* default trans      */    NULL,
      /* query geo proc     */    NULL,
      /* display accelerator*/    NULL,
      /* extension record   */    (XtPointer)NULL,
   },

   {
      /* synthetic resources */   NULL,
      /* num_syn_resources   */   0,
      /* extension           */   NULL,
   }
};
	

/*-------------------------------------------------------------
**	Class Record
*/
DtControlClassRec dtControlClassRec =
{
	/*	Core Part
	*/
	{	
		(WidgetClass) &dtIconClassRec, /* superclass		*/
		"Control",			/* class_name		*/
		sizeof (DtControlRec),  	/* widget_size		*/
		ClassInitialize,		/* class_initialize	*/
		ClassPartInitialize,		/* class_part_initialize*/
		False,				/* class_inited		*/
		(XtInitProc) Initialize,	/* initialize		*/
		NULL,				/* initialize_hook	*/
		NULL,				/* realize		*/
		NULL,				/* actions		*/
		0,				/* num_actions		*/
		resources,			/* resources		*/
		XtNumber (resources),		/* num_resources	*/

		NULLQUARK,			/* xrm_class		*/
		True,				/* compress_motion	*/
		True,				/* compress_exposure	*/
		True,				/* compress_enterleave	*/
		False,				/* visible_interest	*/	
		Destroy,		 	/* destroy		*/	
		XtInheritResize,		/* resize		*/
		XtInheritExpose,		/* expose		*/	
		(XtSetValuesFunc)SetValues,	/* set_values		*/	
		NULL,				/* set_values_hook	*/
		XtInheritSetValuesAlmost,	/* set_values_almost	*/
		NULL,				/* get_values_hook	*/
		NULL,				/* accept_focus		*/	
		XtVersion,			/* version		*/
		NULL,				/* callback private	*/
		NULL,				/* tm_table		*/
		NULL,				/* query_geometry	*/
		NULL,				/* display_accelerator	*/
		(XtPointer)&controlBaseClassExtRec, /* extension	*/

	},

	/*	XmGadget Part
	*/
	{
(XtWidgetProc)	XmInheritBorderHighlight,	/* border_highlight	*/
(XtWidgetProc)	XmInheritBorderUnhighlight,	/* border_unhighlight	*/
(XtActionProc)	XmInheritArmAndActivate,	/* arm_and_activate	*/
(XmWidgetDispatchProc)	XmInheritInputDispatch,	/* input_dispatch	*/
(XmVisualChangeProc)	XmInheritVisualChange,	/* visual_change	*/
		NULL,				/* get_resources	*/
		0,				/* num_get_resources	*/
		XmInheritCachePart,		/* class_cache_part	*/
		NULL,	 			/* extension		*/
	},

	/*	DtIcon Part
	*/
	{
(GetSizeProc)	GetSize,			/* get_size		*/
(GetPositionProc)	GetPositions,			/* get_positions	*/
(DrawProc)	Draw,				/* draw			*/
(CallCallbackProc)	CallCallback,			/* call_callback	*/
(UpdateGCsProc)	UpdateGCs,			/* update_gcs		*/
		True,				/* optimize_redraw	*/
		NULL,				/* class_cache_part	*/
		NULL,				/* extension		*/
	},

	/*	DtControl Part
	*/
	{
		NULL,				/* class_cache_part	*/
		NULL,				/* extension		*/
	}
};


WidgetClass dtControlGadgetClass = (WidgetClass) &dtControlClassRec;



/*-------------------------------------------------------------
**	Private Functions
**-------------------------------------------------------------
*/

#ifdef SUN_CDE_PERF

/* Parameters passed from main thread into monitor thread */
struct thread_param {
    sema_t	sema;
    int		monitor_time;
    int		rfd;
    int		wfd;
    char*	file_name;
};

struct stat_bufffer {
        int     err_type;
        struct stat sbuf;
};

static void
free_thread_param(struct thread_param* p)
{
    /* cosmetic, sema_destroy doesn't do anything */
    /* sema_destroy(&p->sema); */
    free(p->file_name);
    free(p);
}


/*-------------------------------------------------------------
**	UpdateGadget
**		Read stat structure off from thread, check to
**		see if we need updating.
*/
static void
UpdateGadget(XtPointer w, int* fdp, XtInputId* xtid)
{
    struct stat_bufffer stat_data;

   if (read(*fdp, &stat_data, sizeof(struct stat_bufffer)) 
						== sizeof(struct stat_bufffer))
   {
	if(!stat_data.err_type)
          CheckFile((DtControlGadget)w, &stat_data.sbuf);
        else
          CheckFile((DtControlGadget)w, NULL);
   }
}

/*-------------------------------------------------------------
**	ThreadCheckTimeout
**		stat file every so often and return the
**		information to the gadget.
**
**		Note:  calls stat *before* polling in order to
**		elmimate the need to call stat from the main
**		thread in Initialize().
**
**		All the business with the local copies of
**		instance variables is in order to guarantee
**		that the thread doesn't get hosed in between
**		the time the widget gets destroyed and when
**		the thread tests the semaphore.
**
**		Also note, we want to perform a single atomic
**		write.  This avoids possible confusion if the
**		client forks and the thread in the child tries
**		to write before the child exec's.  This way,
**		the parent may get 2 notifications, but it 
**		isn't really a significant problem.
*/
static void*
ThreadCheckTimeout(void* arg)
{
    struct thread_param* param = (struct thread_param*) arg;
    struct stat_bufffer stat_data;
    int errno;

    while (sema_trywait(&param->sema) != 0) {
        if (stat(param->file_name, &stat_data.sbuf) == 0) {
	    stat_data.err_type = 0;
	    write(param->wfd, &stat_data, sizeof(struct stat_bufffer));
	}
 	else {
	    stat_data.err_type = errno; 
	    write(param->wfd, &stat_data, sizeof(struct stat_bufffer));
  	}

	(void) poll(0, 0, param->monitor_time);
    }

    /* Cleanup, widget is getting nuked. */
    close(param->rfd);
    close(param->wfd);
    free_thread_param(param);
    thr_exit(NULL);

/* NOTREACHED */
}

#endif

/*-------------------------------------------------------------
**	FileCheckTimeout
**		Check for change in file, reset timeout.
**		An XtTimerCallbackProc.
*/
static void
#ifdef _NO_PROTO
FileCheckTimeout( client_data, id )
        XtPointer client_data ;
        XtIntervalId *id ;
#else
FileCheckTimeout(
        XtPointer client_data,
        XtIntervalId *id )
#endif /* _NO_PROTO */
{
	DtControlGadget	g = (DtControlGadget) client_data;
	XtAppContext		app_context =
			XtWidgetToApplicationContext ((Widget) g);

#ifdef SUN_CDE_PERF
	CheckFile(g, NULL);
#else
	CheckFile (g);
#endif

	G_MonitorTimer (g) = 
		XtAppAddTimeOut (app_context, G_MonitorTime (g),
				FileCheckTimeout, (XtPointer) g);
}


/*-------------------------------------------------------------
**	CheckFile
**		Check for change in file size
*/
#ifdef SUN_CDE_PERF
static void
CheckFile(
	  DtControlGadget g,
	  struct stat* sbuf
)
#else
static void 
#ifdef _NO_PROTO
CheckFile( g )
        DtControlGadget g ;
#else
CheckFile(
        DtControlGadget g )
#endif /* _NO_PROTO */
#endif /* SUN_CDE_PERF */
{
Boolean		result;
Boolean		file_changed;
off_t		file_size = 0;    /* largefile change */
struct stat	stat_buf;
Atom            _SUN_MAIL_STATUS;


#ifdef SUN_CDE_PERF
if (sbuf == NULL) {
    sbuf = &stat_buf;
    if (stat(G_FileName (g), sbuf) == 0) {
	file_size = sbuf->st_size;
    }
}
else {
    file_size = sbuf->st_size;
}
#else
if (stat (G_FileName (g), &stat_buf) == 0)
    {
    file_size = stat_buf.st_size;
    }
#endif /* SUN_CDE_PERF */


switch (G_ControlType (g))
    {
    case XmCONTROL_MONITOR:
	file_changed = (file_size == 0) ? False : True;
	break;

    case XmCONTROL_MAIL:
        _SUN_MAIL_STATUS = XInternAtom(XtDisplay(g), "_SUN_MAIL_STATUS", False);
        if(XGetSelectionOwner(XtDisplay(g), _SUN_MAIL_STATUS) != None) {
                G_FileSize (g) = file_size;
                return;
        }
	if (file_size == 0)
	    file_changed = False;
#ifdef SUN_CDE_PERF
	else if (sbuf->st_atime > sbuf->st_mtime)
#else
	else if (stat_buf.st_atime > stat_buf.st_mtime)
#endif
	    file_changed = False;
	else if (file_size == G_FileSize (g)) {
	    file_changed =  G_FileChanged (g);
 	}
	else if (file_size > G_FileSize (g)) {
	    file_changed = True;
#ifdef SUN_CDE_PERF
	    G_ModifiedTime (g) = sbuf->st_mtime;
        }
	else if (sbuf->st_mtime > G_ModifiedTime (g))
#else
	    G_ModifiedTime (g) = stat_buf.st_mtime;
	}
	else if (stat_buf.st_mtime > G_ModifiedTime (g))
#endif
	    file_changed = True;
	else
	    file_changed = False;
	break;
    }

G_FileSize (g) = file_size;

if (file_changed != G_FileChanged (g))
    {
    G_FileChanged (g) = file_changed;
    G_CallCallback ((DtIconGadget) g, G_Callback (g), XmCR_MONITOR, NULL);
    G_Expose ((Widget) g, NULL, NULL);
    XmUpdateDisplay ((Widget) g);
    }
}



/*-------------------------------------------------------------
**	ClickTimeout
**		An XtTimerCallbackProc.
**		
*/
static void 
#ifdef _NO_PROTO
ClickTimeout( client_data, id )
        XtPointer client_data ;
        XtIntervalId *id ;
#else
ClickTimeout(
        XtPointer client_data,
        XtIntervalId *id )
#endif /* _NO_PROTO */
{
DtControlGadget	g = 	(DtControlGadget) client_data;

G_ClickTimer (g) = 0;
}



/*-------------------------------------------------------------
**	BusyTimeout
**		Switch pixmap.
**              An XtTimerCallbackProc.
*/
static void 
#ifdef _NO_PROTO
BusyTimeout( client_data, id )
        XtPointer client_data ;
        XtIntervalId *id ;
#else
BusyTimeout(
        XtPointer client_data,
        XtIntervalId *id )
#endif /* _NO_PROTO */
{
DtControlGadget	g = 	(DtControlGadget) client_data;
XtAppContext		app_context = XtWidgetToApplicationContext((Widget) g);

if ((G_Busy (g) > 0) && (G_BlinkElapsed (g) < G_MaxBlinkTime (g)))
    {
    G_Set (g) = ! G_Set (g);
    G_BlinkTimer (g) = 
	XtAppAddTimeOut (app_context, G_BlinkTime (g),
			 BusyTimeout, (XtPointer) g);
    G_BlinkElapsed (g) += G_BlinkTime (g);
    }
else
    {
    G_Set (g) = False;
    G_BlinkTimer (g) = NULL;
    G_BlinkElapsed (g) = 0;
    G_Busy (g) = 0;
    G_CallCallback ((DtIconGadget) g, G_Callback (g), XmCR_BUSY_STOP, NULL);
    }

G_Expose ((Widget) g, NULL, False);
}



/*-------------------------------------------------------------
**	DateTimeout
**		Update date strings.
**              An XtTimerCallbackProc.
*/
static void 
#ifdef _NO_PROTO
DateTimeout( client_data, id )
        XtPointer client_data ;
        XtIntervalId *id ;
#else
DateTimeout(
        XtPointer client_data,
        XtIntervalId *id )
#endif /* _NO_PROTO */
{
DtControlGadget	g = 	(DtControlGadget) client_data;
XtAppContext		app_context =
				XtWidgetToApplicationContext ((Widget) g);
XmString		xs1, xs2;
char			sTime[128];
struct tm		*timeptr;
time_t	 		tse;
String	 		s1, s2, s3;
unsigned long		tilMidnight;
Boolean			old_format = False;

/*	Get time string.
 */
tse = time (NULL);
timeptr = localtime (&tse);
strftime ((char*) sTime, 128, G_Format (g), timeptr);
#ifdef AIX_ILS
if(g->control.format_jp != NULL && strlen(g->control.format_jp) > 0)
    ReplaceJPDate(sTime, g->control.format_jp, timeptr->tm_wday);
#endif /* AIX_ILS */

/*	Break time string between date and day of week.
 */
s1 = sTime;
s2 = strchr (sTime, '\n');

if (s2 == NULL)
   s3 = NULL;
else
   {
   s3 = strchr (s2+1, '\n');

   if (s3 != NULL)
      {
      s2[0] = ' ';
      s2 = s3;
      }
	
   s2[0] = '\0';
   s2++;
   }

/*	Create date string.
 */
if (s1 != NULL)
   {
   if (s2 == NULL)
      {
      xs1 = XmStringCreate (s1, XmFONTLIST_DEFAULT_TAG);
      G_String (g) = _XmStringCreate (xs1);
      _XmStringExtent (G_FontList (g), G_String (g),
			&(G_StringWidth (g)), &(G_StringHeight (g)));
      XmStringFree (xs1);
      if (G_AltString (g) != NULL)
         {
	 _XmStringFree (G_AltString (g));
    	 G_AltString (g) = NULL;
         G_AltStringHeight (g) = 0;
         G_AltStringWidth (g) = 0;
         }
      }
   else
      {
      if (!G_UseLabelAdjustment (g))
         {
         /* swap s1 and s2 */
	 s3 = s1;
         s1 = s2;
         s2 = s3;
         }
      xs1 = XmStringCreate (s1, XmFONTLIST_DEFAULT_TAG);
      if (G_AltString (g) != NULL)
	  _XmStringFree (G_AltString (g));
      G_AltString (g) = _XmStringCreate (xs1);
      _XmStringExtent (G_FontList (g), G_AltString (g),
			  &(G_AltStringWidth (g)), &(G_AltStringHeight (g)));
      XmStringFree (xs1);
      xs2 = XmStringCreate (s2, XmFONTLIST_DEFAULT_TAG);
      G_String (g) = _XmStringCreate (xs2);
      _XmStringExtent (G_FontList (g), G_String (g),
			&(G_StringWidth (g)), &(G_StringHeight (g)));
      XmStringFree (xs2);
      }
   }

/*	Update display.
 */
G_Expose ((Widget) g, NULL, False);
XmUpdateDisplay ((Widget) g);

/*	Compute milliseconds until midnight:
 *	    a. compute current time in seconds
 *	    b. subtract that from number of seconds in a day.
 *	    c. multiply by 1000 to get milliseconds.
 *	Add timeout.
 */
tilMidnight = 1000 * (86400 - ((3600 * timeptr->tm_hour) +
                           (60 * timeptr->tm_min) +
                           (timeptr->tm_sec)));

if (G_Format (g) != NULL)
   {
   G_DateTimer (g) = 
      XtAppAddTimeOut (app_context, tilMidnight, 
		       DateTimeout, (XtPointer) g);
   }
else
   {
   G_DateTimer (g) = 0;
   }
}



/*-------------------------------------------------------------
**	Action Procs
**-------------------------------------------------------------
*/



/*-------------------------------------------------------------
**	Core Procs
**-------------------------------------------------------------
*/


/*-------------------------------------------------------------
**	ClassInitialize
*/

static void
#ifdef _NO_PROTO
ClassInitialize()
#else
ClassInitialize( void )
#endif /* _NO_PROTO */
{
  controlBaseClassExtRec.record_type = XmQmotif;
}


/*-------------------------------------------------------------
**	ClassPartInitialize
*/

static void
#ifdef _NO_PROTO
ClassPartInitialize( cl )
        WidgetClass cl ;
#else
ClassPartInitialize(
        WidgetClass cl )
#endif /* _NO_PROTO */
{
    dtControlCacheObjClassRec.object_class.resources =
				 dtIconCacheObjClassRec.object_class.resources;

    dtControlCacheObjClassRec.object_class.num_resources =
			 dtIconCacheObjClassRec.object_class.num_resources;
}



/*-------------------------------------------------------------
**	Initialize
**		Initialize a new gadget instance.
*/
static void 
#ifdef _NO_PROTO
Initialize( request_w, new_w )
        Widget request_w ;
        Widget new_w ;
#else
Initialize(
        Widget request_w,
        Widget new_w )
#endif /* _NO_PROTO */
{
	DtControlGadget	request = (DtControlGadget) request_w,
				new = (DtControlGadget) new_w;
	XmManagerWidget		mw = (XmManagerWidget) XtParent (new);
	XtAppContext	app_context =
				XtWidgetToApplicationContext (new_w);
	String		str;
	int		file_name_size;				
	XrmValue	pixVal;

G_MonitorTimer (new) = 0;
G_ClickTimer (new) = 0;
G_DateTimer (new) = 0;
G_BlinkTimer (new) = 0;
G_PushAnimationTimer (new) = 0;
G_DropAnimationTimer (new) = 0;

G_FileSize (new) = 0;
G_FileChanged (new) = False;
G_ModifiedTime (new) = 0;

G_Busy (new) = 0;
G_AltString (new) = NULL;
G_AltStringWidth (new) = 0;
G_AltStringHeight (new) = 0;

/*	Initialize animation data.
 */
G_NumPushImages (new) = 0;
G_MaxPushImages (new) = 0;
G_PushImagePosition (new) = 0;
G_PushDelays (new) = NULL;
G_PushPixmaps (new) = NULL;
G_PushMasks (new) = NULL;

G_NumDropImages (new) = 0;
G_MaxDropImages (new) = 0;
G_DropImagePosition (new) = 0;
G_DropDelays (new) = NULL;
G_DropPixmaps (new) = NULL;
G_DropMasks (new) = NULL;

#ifdef SUN_CDE_PERF
G_MonitorThread (new) = 0;
#endif

/*	Copy drop argument.
*/
if (G_DropAction (new) != NULL)
    {
    String	str = G_DropAction (new);
    G_DropAction (new) = XtMalloc (strlen (str) + 1);
    strcpy (G_DropAction (new), str);
    }

if (G_PushAction (new) != NULL)
    {
    String    str = G_PushAction (new);
    G_PushAction (new) = XtMalloc (strlen (str) + 1);
    strcpy (G_PushAction (new), str);
    }

/*	Validate control type.
 */
if (G_ControlType (new) != XmCONTROL_NONE &&
    G_ControlType (new) != XmCONTROL_SWITCH &&
    G_ControlType (new) != XmCONTROL_BUTTON &&
    G_ControlType (new) != XmCONTROL_BLANK &&
    G_ControlType (new) != XmCONTROL_DATE &&
    G_ControlType (new) != XmCONTROL_BUSY &&
    G_ControlType (new) != XmCONTROL_CLIENT &&
    G_ControlType (new) != XmCONTROL_MAIL &&
    G_ControlType (new) != XmCONTROL_MONITOR)
    {
    G_ControlType (new) == XmCONTROL_NONE;
    }

if (G_ControlType (new) == XmCONTROL_SWITCH)
   G_ShadowType (new) = (G_Set (new)) ? XmSHADOW_IN : XmSHADOW_OUT;

if ( ((G_ControlType (new) == XmCONTROL_MONITOR) ||
      (G_ControlType (new) == XmCONTROL_MAIL) ||
      (G_ControlType (new) == XmCONTROL_BUSY)) &&
     (G_AltImage (new) != NULL))
    {

/*	Copy string or default to user mail directory.
 */
    if ((G_ControlType (new) == XmCONTROL_MONITOR) ||
        (G_ControlType (new) == XmCONTROL_MAIL))
    {
	if (G_FileName (new) == NULL)
	{
	    if (G_ControlType (new) == XmCONTROL_MAIL)
	    {
                if ((str = getenv ("MAIL")) == NULL)
                {
		   str = getenv ("LOGNAME");
		   G_FileName (new) = 
		      XtMalloc (strlen (MAIL_DIR) + strlen (str) + 1);
		   strcpy (G_FileName (new), MAIL_DIR);
		   strcat (G_FileName (new), str);
                }
                else
                {
		   G_FileName (new) = XtMalloc (strlen (str) + 1);
		   strcpy (G_FileName (new), str);
		}
            }
	}
	else
	{
	    str = G_FileName (new);
	    G_FileName (new) = XtMalloc (strlen (str) + 1);
	    strcpy (G_FileName (new), str);
	}

	G_FileChanged (new) = False;
   }
	
/*	Get alternate pixmap and mask.
 */
    G_AltPix (new) =
		XmGetPixmap (XtScreen (new), G_AltImage (new),
				G_PixmapForeground (new),
				G_PixmapBackground (new));
    G_AltMask (new) =
		_DtGetMask (XtScreen (new), G_AltImage (new));
    G_AltImage (new) = NULL;

    if ((G_FileName (new) != NULL) &&
	(G_AltPix (new) != XmUNSPECIFIED_PIXMAP) &&
	((G_ControlType (new) == XmCONTROL_MONITOR) ||
	 (G_ControlType (new) == XmCONTROL_MAIL)))
#ifdef SUN_CDE_PERF
	{
	    int status = -1;
	    int tstatus = 0;
	    struct thread_param* param;

	    if ((status = pipe(G_FilePipe (new))) != -1) {
	        sigset_t nset;
		sigset_t oset;

		/* set close-on-exec flag */
		fcntl(G_FileRPipe(new), F_SETFD, 1);
		fcntl(G_FileWPipe(new), F_SETFD, 1);
	

		/* install callback to communicate with thread */
		G_ThreadInputId(new)
		    = XtAppAddInput(app_context,
			      G_FileRPipe(new), 
			      (XtPointer)XtInputReadMask,
			      UpdateGadget, new
		      );


		/* flag to tell thread when to exit.
		 * "param" gets freed by the thread.
		 */
		param = (struct thread_param*) malloc(sizeof(struct thread_param));
		if (param != NULL) {
		    param->file_name = strdup(G_FileName(new));
		    if (param->file_name == NULL) {
		        free(param);
			param = NULL;
		    }
		}

		if ((param != NULL) || (param->file_name != NULL)) {
		    G_Sema(new) = &param->sema;
		    sema_init(&param->sema, 0, USYNC_THREAD, NULL);

		    /* Spawn monitor thread being careful to mask out signals */
		    param->monitor_time	 = G_MonitorTime(new);
		    param->rfd		 = G_FileRPipe(new);
		    param->wfd		 = G_FileWPipe(new);

		    sigfillset(&nset);
		    thr_sigsetmask(SIG_SETMASK, &nset, &oset);

		    tstatus 
		        = thr_create(NULL, 0, ThreadCheckTimeout, param,
				     THR_DETACHED, &G_MonitorThread(new)
			     );

		    thr_sigsetmask(SIG_SETMASK, &oset, NULL);
		}
	    }

	    if ((status == -1) || (param == NULL) || (tstatus != 0)) {
		/* failure, fallback to the old way of monitoring */

	        if (param != NULL) {
		    free_thread_param(param);
		    G_Sema(new) = NULL;
		}

		if (tstatus != 0) {
		    /* pipe succeeded, thread failed.  close pipe. */
		    close(G_FileRPipe(new));
		    close(G_FileWPipe(new));
		}

		/* Remove input handler, put in fallback timer */
		if (status != -1) {
		    XtRemoveInput(G_ThreadInputId(new));
		}
		G_MonitorTimer (new) =
		    XtAppAddTimeOut(app_context, G_MonitorTime (new),
				    (XtTimerCallbackProc) FileCheckTimeout,
				    (XtPointer) new);
	    }
	}
#else
	{
	    CheckFile (new);
	    G_MonitorTimer (new) =
		XtAppAddTimeOut (app_context, G_MonitorTime (new),
				 (XtTimerCallbackProc) FileCheckTimeout,
				 (XtPointer) new);
	}
#endif

/*	Check for Control state change.
 */
    }
else
    {
    G_AltPix (new) = XmUNSPECIFIED_PIXMAP;
    G_AltMask (new) = XmUNSPECIFIED_PIXMAP;
    G_MonitorTimer (new) = 0;
    }

if (G_ControlType (new) == XmCONTROL_DATE)
    {
	Dimension	w, h;

    DateTimeout (new_w, NULL);

        if (G_Width (request) == 0 || G_Height (request) == 0)
        {
                G_GetSize ((DtIconGadget) new, &w, &h);
                if (G_Width (request) == 0)
                        G_Width (new) = w;
                if (G_Height (request) == 0)
                        G_Height (new) = h;
        }
    }

G_ArmedGC (new) = NULL;

G_TopShadowGC (new) = NULL;
G_BottomShadowGC (new) = NULL;
G__DoUpdate (new) = True;
G_UpdateGCs ((DtIconGadget) new);
G__DoUpdate (new) = False;
}



/*-------------------------------------------------------------
**	Destroy
**		Release resources allocated for gadget.
*/

static void 
#ifdef _NO_PROTO
Destroy( w )
        Widget w ;
#else
Destroy(
        Widget w )
#endif /* _NO_PROTO */
{
	DtControlGadget	g =	(DtControlGadget) w;

#ifdef SUN_CDE_PERF
	if (G_MonitorThread(g) != 0) {
	    /* tell monitor thread to go away. */
	    XtRemoveInput(G_ThreadInputId(g));
	    sema_post(G_Sema(g));
	}
#endif

if (G_FileName (g) != NULL)
    XtFree (G_FileName (g));

if (G_NumPushImages (g) > 0)
    {
    XtFree ((char*) G_PushDelays (g));
    XtFree ((char*) G_PushPixmaps (g));
    XtFree ((char*) G_PushMasks (g));
    }
if (G_NumDropImages (g) > 0)
    {
    XtFree ((char*) G_DropDelays (g));
    XtFree ((char*) G_DropPixmaps (g));
    XtFree ((char*) G_DropMasks (g));
    }
if (G_DropAction (g) != NULL)
    XtFree (G_DropAction (g));
if (G_PushAction (g) != NULL)
    XtFree (G_PushAction (g));

if (G_AltString (g) != NULL)
    _XmStringFree (G_AltString (g));

if (G_ClickTimer (g))
    XtRemoveTimeOut (G_ClickTimer (g));
if (G_BlinkTimer (g))
    XtRemoveTimeOut (G_BlinkTimer (g));
if (G_MonitorTimer (g))
    XtRemoveTimeOut (G_MonitorTimer (g));
if (G_DateTimer (g))
    XtRemoveTimeOut (G_DateTimer (g));
if (G_PushAnimationTimer (g))
    XtRemoveTimeOut (G_PushAnimationTimer (g));
if (G_DropAnimationTimer (g))
    XtRemoveTimeOut (G_DropAnimationTimer (g));
}



/*-------------------------------------------------------------
**	SetValues
**		
*/
static Boolean 
#ifdef _NO_PROTO
SetValues( current_w, request_w, new_w )
        Widget current_w ;
        Widget request_w ;
        Widget new_w ;
#else
SetValues(
        Widget current_w,
        Widget request_w,
        Widget new_w )
#endif /* _NO_PROTO */
{


    DtControlGadget	request_c =	(DtControlGadget) request_w,
			current_c =	(DtControlGadget) current_w,
			new_c =		(DtControlGadget) new_w;
#if 0
	String		file_name =	G_FileName (new);
	int		file_name_size;				
	Boolean		redraw_flag =	False;
#endif /*  0 */

    if (strcmp(G_Format(new_c),G_Format(current_c)) &&
        G_ControlType (new_c) == XmCONTROL_DATE)
    {
       if (G_DateTimer (new_c))
    	  XtRemoveTimeOut (G_DateTimer (new_c));

       if (G_Format(new_c) != NULL)
          DateTimeout (new_w, NULL);
    }

    if (G_ImageName (new_c) && (G_ImageName (current_c) != G_ImageName (new_c)))
       return True;
    else
       return False;

#if 0

/*	Copy drop argument.
*/
if (G_DropAction (new) != G_DropAction (current))
    {
    String	str = G_DropAction (new);
    if (G_DropAction (current) != NULL)
	XtFree (G_DropAction (current));
    G_DropAction (new) = XtMalloc (strlen (str) + 1);
    strcpy (G_DropAction (new), str);
    }

if (G_PushAction (new) != G_PushAction (current))
    {
    String    str = G_PushAction (new);
    if (G_PushAction (current) != NULL)
      XtFree (G_PushAction (current));
    G_PushAction (new) = XtMalloc (strlen (str) + 1);
    strcpy (G_PushAction (new), str);
    }

/*	Copy string or default to user mail directory.
*/
if ((G_ControlType (new) == XmCONTROL_MONITOR) ||
    (G_ControlType (new) == XmCONTROL_MAIL))
    {
	if (G_FileName (new) != G_FileName (current))
	{
		if (G_FileName (current) != NULL)
			XtFree (G_FileName (current));
		if (G_FileName (new) != NULL)
		{
			file_name_size = strlen (G_FileName (new));
			G_FileName (new) = XtMalloc (file_name_size);
			strcpy (G_FileName (new), file_name);
		}
	}
    }

	return (redraw_flag);
#endif /* 0 */
}


/*-------------------------------------------------------------
**	Gadget Procs
**-------------------------------------------------------------
*/


/*-------------------------------------------------------------
**	Icon Procs
**-------------------------------------------------------------
*/

/*-------------------------------------------------------------
**	GetSize
**		Compute size.
*/
static void 
#ifdef _NO_PROTO
GetSize( g, w, h )
        DtControlGadget g ;
        Dimension *w ;
        Dimension *h ;
#else
GetSize(
        DtControlGadget g,
        Dimension *w,
        Dimension *h )
#endif /* _NO_PROTO */
{
	Dimension	s_t = G_ShadowThickness (g),
			h_t = G_HighlightThickness (g),
			p_w = G_PixmapWidth (g),
			p_h = G_PixmapHeight (g),
			m_w = G_MarginWidth (g),
			m_h = G_MarginHeight (g),
			s_w = G_StringWidth (g),
			s_h = G_StringHeight (g),
			v_pad = 2 * (s_t + h_t + m_h),
			h_pad = 2 * (s_t + h_t + m_w),
			spacing = G_Spacing (g);

	if (((p_w == 0) && (p_h == 0)) || ((s_w == 0) && (s_h == 0)))
		spacing = 0;

if (G_ControlType (g) == XmCONTROL_DATE)
   {
/*    Adjust size based on second date string.
 */
    if ( (G_StringWidth (g) > 0) &&
       (G_AltStringWidth (g) > G_StringWidth (g)) )
      s_w = G_AltStringWidth (g);
    if ( (G_StringWidth (g) > 0) && (G_AltStringWidth (g) > 0) )
      s_h += spacing + G_AltStringHeight (g);
    }
else
    {
/*    Add left and right spacing for string.
 */
    if (s_w > 0)
      s_w += 4;
    }

/*	Get width and height.
*/
	switch ((int) G_PixmapPosition (g))
	{
		case XmPIXMAP_TOP:
		case XmPIXMAP_BOTTOM:
			*w = Max (p_w, s_w) + h_pad;
			*h = p_h + s_h + v_pad + spacing;
			break;
		case XmPIXMAP_LEFT:
		case XmPIXMAP_RIGHT:
			*w = p_w + s_w + h_pad + spacing;
			*h = Max (p_h, s_h) + v_pad;
			break;
		case XmPIXMAP_MIDDLE:
			*w = Max (p_w, s_w) + h_pad;
			*h = Max (p_h, s_h) + v_pad;
			break;
	}
}



/*-------------------------------------------------------------
**	GetPositions
**		Get positions of string and pixmap.
*/
static void 
#ifdef _NO_PROTO
GetPositions( g, w, h, h_t, s_t, pix_x, pix_y, str_x, str_y )
        DtControlGadget g ;
        Position w ;
        Position h ;
        Dimension h_t ;
        Dimension s_t ;
        Position *pix_x ;
        Position *pix_y ;
        Position *str_x ;
        Position *str_y ;
#else
GetPositions(
        DtControlGadget g,
        Position w,
        Position h,
        Dimension h_t,
        Dimension s_t,
        Position *pix_x,
        Position *pix_y,
        Position *str_x,
        Position *str_y )
#endif /* _NO_PROTO */
{
	Dimension	p_w =		G_PixmapWidth (g),
			p_h =		G_PixmapHeight (g),
			s_w =		G_StringWidth (g),
			s_h =		G_StringHeight (g),
			m_w =		G_MarginWidth (g),
			m_h =		G_MarginHeight (g),
			spacing =	G_Spacing (g),
			h_pad =		s_t + h_t + m_w,
			v_pad =		s_t + h_t + m_h,
			width =		w - 2 * h_pad,
			height =	h - 2 * v_pad;
	Position	p_x =		h_pad,
			p_y =		v_pad,
			s_x =		h_pad,
			s_y =		v_pad;
	unsigned char	align =		G_Alignment (g);

	if (((p_w == 0) && (p_h == 0)) || ((s_w == 0) && (s_h == 0)))
		spacing = 0;

/*	Set positions
*/
	switch ((int) G_PixmapPosition (g))
	{
		case XmPIXMAP_TOP:
			if (align == XmALIGNMENT_CENTER)
			{
				if (p_w && width > p_w)
					p_x += (width - p_w)/2;
				if (s_w && width > s_w)
					s_x += (width - s_w)/2;
			}
			else if (align == XmALIGNMENT_END)
			{
				if (p_w && width > p_w)
					p_x += width - p_w;
				if (s_w && width > s_w)
					s_x += width - s_w;
			}
			if (p_h && height > p_h + s_h + spacing)
				p_y += (height - p_h - s_h - spacing)/2;
			if (p_h)
				s_y = p_y + p_h + spacing;
			else
				s_y += (height - s_h)/2;
			break;
		case XmPIXMAP_BOTTOM:
			if (align == XmALIGNMENT_CENTER)
			{
				if (p_w && width > p_w)
					p_x += (width - p_w)/2;
				if (s_w && width > s_w)
					s_x += (width - s_w)/2;
			}
			else if (align == XmALIGNMENT_END)
			{
				if (p_w && width > p_w)
					p_x += width - p_w;
				if (s_w && width > s_w)
					s_x += width - s_w;
			}
			if (s_h && height > p_h + s_h + spacing)
				s_y += (height - p_h - s_h - spacing)/2;
			if (s_h)
				p_y = s_y + s_h + spacing;
			else
				p_y += (height - p_h)/2;
			break;
		case XmPIXMAP_LEFT:
			if (p_h && height > p_h)
				p_y += (height - p_h)/2;
			s_x += p_w + spacing;
			if (s_h && height > s_h)
				s_y += (height - s_h)/2;
/*
			if (p_w && width > p_w + spacing + s_w)
				p_x += (width - p_w - spacing - s_w)/2;
			if (s_w && width > p_w + spacing + s_w)
				s_x += (width - p_w - spacing - s_w)/2;
*/
			break;
		case XmPIXMAP_RIGHT:
			if (s_h && height > s_h)
				s_y += (height - s_h)/2;
			p_x += s_w + spacing;
			if (p_w && width > p_w + spacing + s_w)
				p_x += (width - p_w - spacing - s_w)/2;
			if (s_w && width > p_w + spacing + s_w)
				s_x += (width - p_w - spacing - s_w)/2;
			if (p_h && height > p_h)
				p_y += (height - p_h)/2;
			break;
		case XmPIXMAP_MIDDLE:
			if (p_w && width > p_w)
		 	   p_x += (width - p_w)/2;
			if (s_w && width > s_w)
			   s_x += (width - s_w)/2;
			if (s_h && height > s_h)
				s_y += (height - s_h)/2;
			if (p_h && height > p_h)
				p_y += (height - p_h)/2;
                        break;
	}

	*pix_x = p_x;
	*pix_y = p_y;
	*str_x = s_x;
	*str_y = s_y;
}



/*-------------------------------------------------------------
**	Draw
**		Draw gadget to drawable.
*/
static void 
#ifdef _NO_PROTO
Draw( g, drawable, x, y, w, h, h_t, s_t, s_type, fill_mode )
        DtControlGadget g ;
        Drawable drawable ;
        Position x ;
        Position y ;
        Dimension w ;
        Dimension h ;
        Dimension h_t ;
        Dimension s_t ;
        unsigned char s_type ;
        unsigned char fill_mode ;
#else
Draw(
        DtControlGadget g,
        Drawable drawable,
        Position x,
        Position y,
        Dimension w,
        Dimension h,
        Dimension h_t,
        Dimension s_t,
        unsigned char s_type,
        unsigned char fill_mode )
#endif /* _NO_PROTO */
{
XmManagerWidget	mgr =	(XmManagerWidget) XtParent (g);
Display *	d = 	XtDisplay (g);
GC		gc, gcTS, gcBS;
XRectangle	clip;
Position	p_x, p_y, s_x, s_y;
Dimension	width = 0, height = 0;
unsigned char	behavior =	G_Behavior (g);
Position	adj_x, adj_y;
Pixmap		pix;
Pixmap		mask;
int		index;
Dimension	left = 0, right = 0, top = 0, bottom = 0;
Dimension	v_pad;
Boolean		bClearArea = False;
Boolean		bClearShadow = False;
Boolean		bMono;

bMono = ( ((G_Foreground (g) == BlackPixelOfScreen (XtScreen (g))) ||
	   (G_Foreground (g) == WhitePixelOfScreen (XtScreen (g)))) &&
	  ((G_Background (g) == BlackPixelOfScreen (XtScreen (g))) ||
	   (G_Background (g) == WhitePixelOfScreen (XtScreen (g)))) );

/*	Select gc to fill background.
 */
if (G_ControlType (g) == XmCONTROL_SWITCH)
    {
    if (bMono)
      	gc = M_BackgroundGC (mgr);
    else
        gc = G_BackgroundGC (g);
    }
else if ((G_Armed (g)) && (fill_mode != XmFILL_PARENT))
    gc = G_ArmedBackgroundGC (g);
else
    gc = M_BackgroundGC (mgr); 

if ((fill_mode != XmFILL_NONE) && (fill_mode != XmFILL_TRANSPARENT))
	bClearArea = True;

/*	Select pixmap and mask.
 */
if (G_Pixmap (g))
    {
/*	Terminate animation sequence.
 */
    if (G_PushImagePosition (g) > G_NumPushImages (g))
	G_PushImagePosition (g) = 0;
    if (G_DropImagePosition (g) > G_NumDropImages (g))
	G_DropImagePosition (g) = 0;

/*	Use animation image.
 */
    if (G_PushImagePosition (g) > 0)
	{
	index = (G_PushImagePosition (g)) - 1;    
	if ((index == 0) || (bMono))
	    bClearArea = True;
	else
	    bClearArea = False;
	pix = G_PushPixmaps (g)[index];
	mask = G_PushMasks (g)[index];
	++G_PushImagePosition (g);
	}
    else if (G_DropImagePosition (g) > 0)
	{
	index = G_DropImagePosition (g) - 1;
	if (bMono)
	    bClearArea = True;
	else
	    bClearArea = False;
	pix = G_DropPixmaps (g)[index];
	mask = G_DropMasks (g)[index];
	++G_DropImagePosition (g);
	}

/*	Use alternate image.
 */
    else if ( ( ((G_ControlType (g) == XmCONTROL_MONITOR) ||
		 (G_ControlType (g) == XmCONTROL_MAIL)) &&
		(G_AltPix (g) != XmUNSPECIFIED_PIXMAP) &&
			G_FileChanged (g) ) ||
	      ( (G_ControlType (g) == XmCONTROL_BUSY) &&
		(G_AltPix (g) != XmUNSPECIFIED_PIXMAP) &&
		G_Busy (g) && G_Set (g)) )
	{
	pix = G_AltPix (g);
	mask = G_AltMask (g);
	}

/*	Use normal image.
 */
    else
	{
	pix = G_Pixmap (g);
	mask = G_Mask (g);
	}
    }

if (bClearShadow)
    {
/*	Erase shadow - top, left, right, bottom.
 */
    XFillRectangle (d, drawable, gc, x + h_t, y + h_t,
				w - 2 * h_t, s_t);
    XFillRectangle (d, drawable, gc, x + h_t, y + h_t + s_t,
				s_t, h - 2 * (h_t + s_t));
    XFillRectangle (d, drawable, gc, x + w - h_t - s_t, y + h_t + s_t,
				s_t, h - 2 * (h_t + s_t));
    XFillRectangle (d, drawable, gc, x + h_t, y + h - h_t - s_t,
				w - 2 * h_t, s_t);
    }
else if (bClearArea)
/*	Fill background.
 */
    XFillRectangle (d, drawable, gc, x + h_t, y + h_t,
				w - 2 * h_t, h - 2 * h_t);


/*	Get pixmap and string positions.
 */
G_GetPositions ((DtIconGadget) g, w, h, h_t, s_t, &p_x, &p_y, &s_x, &s_y);

/*	Select and display pixmap.
 */
if (G_Pixmap (g))
    {

/*	Compute display region.
 */
    if ((width == 0) && (height == 0))
	{	   
	width = (p_x + s_t + h_t >= G_Width (g))
			? 0 : Min (G_PixmapWidth (g),
				G_Width (g) - p_x - s_t - h_t);
	height = (p_y + s_t + h_t >= G_Height (g))
			? 0 : Min (G_PixmapHeight (g),
				G_Height (g) - p_y - s_t - h_t);
	}
/*	Update clip gc.
 */
    if (mask != XmUNSPECIFIED_PIXMAP)
	{
	gc = G_ClipGC (g);
	XSetClipMask (XtDisplay(g), gc, mask);
	XSetClipOrigin (XtDisplay(g), gc, x + p_x, y + p_y);
	}
    else
	gc = G_NormalGC (g);
	
/*	Paint pixmap.
 */
    if ((width > 0 && height > 0) &&
	(gc != NULL) && (pix != XmUNSPECIFIED_PIXMAP))
	XCopyArea (d, pix, drawable, gc, 0, 0,
				width, height, x + p_x, y + p_y);
    }

/*	Draw string.
 */
clip.x = x + h_t + s_t;
clip.y = y + h_t + s_t;
clip.width = G_Width (g) - (2 * (h_t + s_t));
clip.height = G_Height (g) - (2 * (h_t + s_t));

if ((G_String (g)) && (clip.width > 0 && clip.height > 0))
    {
    if (G_ControlType (g) == XmCONTROL_DATE)
	{
        if ( (G_StringHeight (g) > 0) && (G_AltStringHeight (g) > 0) )
            {
               int pad_factor;

               if (G_UseLabelAdjustment (g))
                  pad_factor = 3;
               else
                  pad_factor = 4;

	       v_pad = (G_Height(g) > G_StringHeight(g) + G_AltStringHeight(g))
		       ? (G_Height (g) - G_StringHeight (g) -
					  G_AltStringHeight (g))/pad_factor
		       : 0;
	       s_y = (2 * v_pad);
            }

	if (bMono)
	    {
	    _XmStringDrawImage (d, drawable, G_FontList (g), G_String (g),
				G_NormalGC (g), x + s_x, y + s_y,
				clip.width, XmALIGNMENT_BEGINNING,
				XmSTRING_DIRECTION_L_TO_R, &clip);
	    }
	else
	    {
	    if (G_UseEmbossedText (g))
		_XmStringDraw (d, drawable, G_FontList (g), G_String (g),
				G_BottomShadowGC (g), x + s_x + 1, y + s_y + 1,
				clip.width, XmALIGNMENT_BEGINNING,
				XmSTRING_DIRECTION_L_TO_R, &clip);
	    _XmStringDraw (d, drawable, G_FontList (g), G_String (g),
				G_NormalGC (g), x + s_x, y + s_y,
				clip.width, XmALIGNMENT_BEGINNING,
				XmSTRING_DIRECTION_L_TO_R, &clip);
	    }

	if (G_Width (g) > G_AltStringWidth (g))
	    s_x = (G_Width (g) - G_AltStringWidth (g)) / 2;

        if (G_UseLabelAdjustment (g))
 	   s_y = ((G_Height (g) - G_AltStringWidth (g)) + 5);
        else
 	   s_y += G_StringHeight (g);

	if (bMono)
	    {
	    _XmStringDrawImage (d, drawable, G_FontList (g),
				G_AltString (g),
				G_NormalGC (g), x + s_x, y + s_y,
				clip.width, XmALIGNMENT_BEGINNING,
				XmSTRING_DIRECTION_L_TO_R, &clip);
	    }
	else
	    {
	    if (G_UseEmbossedText (g))
		_XmStringDraw (d, drawable, G_FontList (g),
				G_AltString (g),
				G_BottomShadowGC (g), x + s_x + 1, y + s_y + 1,
				clip.width, XmALIGNMENT_BEGINNING,
				XmSTRING_DIRECTION_L_TO_R, &clip);
	    _XmStringDraw (d, drawable, G_FontList (g),
				G_AltString (g),
				G_NormalGC (g), x + s_x, y + s_y,
				clip.width, XmALIGNMENT_BEGINNING,
				XmSTRING_DIRECTION_L_TO_R, &clip);
	    }
	}
    else
	{
	if (bMono)
	    {
	    if ((s_x - 2) >= (h_t + s_t))
		XFillRectangle (d, drawable, G_ArmedBackgroundGC (g),
				x + s_x - 2, y + s_y,
				2, G_StringHeight (g));
	    _XmStringDrawImage (d, drawable, G_FontList (g), G_String (g),
				G_NormalGC (g), x + s_x, y + s_y,
				clip.width, XmALIGNMENT_BEGINNING,
				XmSTRING_DIRECTION_L_TO_R, &clip);
	    if ((s_x + G_StringWidth (g) + 2) <= (G_Width (g) - h_t - s_t))
		XFillRectangle (d, drawable, G_ArmedBackgroundGC (g),
				x + s_x + G_StringWidth (g), y + s_y,
				2, G_StringHeight (g));
	    }
	else
	    {
	    if (G_UseEmbossedText (g))
		_XmStringDraw (d, drawable, G_FontList (g), G_String (g),
				G_BottomShadowGC (g), x + s_x + 1, y + s_y + 1,
				clip.width, XmALIGNMENT_BEGINNING,
				XmSTRING_DIRECTION_L_TO_R, &clip);
	    _XmStringDraw (d, drawable, G_FontList (g), G_String (g),
				G_NormalGC (g), x + s_x, y + s_y,
				clip.width, XmALIGNMENT_BEGINNING,
				XmSTRING_DIRECTION_L_TO_R, &clip);
	    }
	}
    }

/*	Draw shadow.
 */
switch (G_ControlType (g))
    {
    case XmCONTROL_BLANK:
    case XmCONTROL_DATE:
	break;

    case XmCONTROL_CLIENT:
/*	Get insets.
 */
	if (bMono)
/*	Use black and white.
 */
	    {
	    if (G_Foreground (g) == WhitePixelOfScreen (XtScreen (g)))
		{
		gcTS = G_BackgroundGC (g);
		gcBS = G_NormalGC (g);
		}
	    else
		{
		gcBS = G_BackgroundGC (g);
		gcTS = G_NormalGC (g);
		}
	    }
	else
	    {
	    gcTS = M_BottomShadowGC (mgr);
	    gcBS = M_TopShadowGC (mgr);
	    }
/*	Drop inner shadow if secondary since no fill.
 */
	    _DtDrawEtchedShadow (d, drawable, gcTS, gcBS, s_t,
				x + left - s_t, y + top - s_t,
				w - left - right + (2 * s_t),
				h - top - bottom + (2 * s_t));
	break;

    default:
    case XmCONTROL_BUTTON:
    case XmCONTROL_MONITOR:
    case XmCONTROL_MAIL:
	switch (G_Behavior (g))
	    {
	    case XmICON_BUTTON:
		if (G_Armed (g))
		    _DtManagerDrawShadow (mgr, drawable, x, y, w, h,
						h_t, s_t, XmSHADOW_IN);
		break;
	    case XmICON_TOGGLE:
		if ( (G_Armed (g) && !G_Set (g)) ||
		     (!G_Armed (g) && G_Set (g)) )
		    _DtManagerDrawShadow (mgr, drawable, x, y, w, h,
						h_t, s_t, XmSHADOW_IN);
		break;
	    }
	break;
    case XmCONTROL_SWITCH:
	if (!G_FillOnArm (g))
	    {
	    gcTS = G_BottomShadowGC (g);
	    gcBS = G_TopShadowGC (g);
	    }
	else
	    {
	    if (G_Foreground (g) == WhitePixelOfScreen (XtScreen (g)))
		{
		gcTS = G_BackgroundGC (g);
		gcBS = G_NormalGC (g);
		}
	    else
		{
		gcTS = G_NormalGC (g);
		gcBS = G_BackgroundGC (g);
		}
	    }
	_XmDrawShadows (d, drawable, gcTS, gcBS,
			x + h_t, y + h_t, w - 2*h_t, h - 2*h_t, 1, 0);

	++x;  ++y;  w -= 2;  h -= 2;  --s_t;
	if (s_t > 0)
	    {
	    if ( (G_Armed (g) && !G_Set (g)) ||
	         (!G_Armed (g) && G_Set (g)) )
		{
		gcTS = G_BottomShadowGC (g);
		gcBS = G_TopShadowGC (g);
		}
	    else
		{
		gcTS = G_TopShadowGC (g);
		gcBS = G_BottomShadowGC (g);
		}
	    _XmDrawShadows (d, drawable, gcTS, gcBS,
			    x + h_t, y + h_t, w - 2*h_t, h - 2*h_t, s_t, 0);

	    }
	break;
    }
}


/*-------------------------------------------------------------
**	CallCallback
**		Call callback, if any, with reason and event.
*/
static void 
#ifdef _NO_PROTO
CallCallback( g, cb, reason, event )
        DtControlGadget g ;
        XtCallbackList cb ;
        int reason ;
        XEvent *event ;
#else
CallCallback(
        DtControlGadget g,
        XtCallbackList cb,
        int reason,
        XEvent *event )
#endif /* _NO_PROTO */
{
DtControlCallbackStruct	cb_data;
XtAppContext	app_context = XtWidgetToApplicationContext ((Widget) g);

if ((reason == XmCR_ACTIVATE) && (G_Behavior (g) == XmICON_BUTTON))
    {
    if (G_ClickTimer (g) != 0)
	return;
    else
	G_ClickTimer (g) = 
	    XtAppAddTimeOut (app_context, G_ClickTime (g),
			     ClickTimeout, (XtPointer) g);
    }

	if (cb != NULL)
	{
		cb_data.reason = reason;
		cb_data.event = event;
		cb_data.control_type = G_ControlType (g);
		cb_data.set = G_Set (g);
		cb_data.push_function = G_PushFunction (g);
		cb_data.push_argument = G_PushArgument (g);
		cb_data.subpanel = G_Subpanel (g);
		cb_data.file_size = G_FileSize (g);
		XtCallCallbackList ((Widget) g, cb, &cb_data);

	}
}



/*-------------------------------------------------------------
**	UpdateGCs
**		Get normal and background graphics contexts.
**		Use standard mask to maximize caching opportunities.
*/
static void 
#ifdef _NO_PROTO
UpdateGCs( g )
        DtControlGadget g ;
#else
UpdateGCs(
        DtControlGadget g )
#endif /* _NO_PROTO */
{
XGCValues	values;
XtGCMask	value_mask;
XmManagerWidget	mw = (XmManagerWidget) XtParent(g);
XFontStruct *	font;
int		index;
Boolean		font_rtn;

if (!G__DoUpdate (g))
    return;

if (G_NormalGC (g))
    XtReleaseGC ((Widget)mw, G_NormalGC (g));
if (G_ClipGC (g))
    XtReleaseGC ((Widget)mw, G_ClipGC (g));
if (G_BackgroundGC (g))
    XtReleaseGC ((Widget)mw, G_BackgroundGC (g));
if (G_ArmedBackgroundGC (g))
    XtReleaseGC ((Widget)mw, G_ArmedBackgroundGC (g));
if (G_TopShadowGC (g))
    XtReleaseGC ((Widget)mw, G_TopShadowGC (g));
if (G_BottomShadowGC (g))
    XtReleaseGC ((Widget)mw, G_BottomShadowGC (g));

/*	Get normal GC.
*/
font_rtn = _XmFontListSearch (G_FontList (g), "default", (short *)&index,
					&font);
value_mask = GCForeground | GCBackground | GCFont | GCFillStyle;
if (G_UseEmbossedText (g))
    values.foreground = WhitePixelOfScreen (XtScreen (g));
else
    values.foreground = G_Foreground (g);
values.background = G_Background (g);

values.fill_style = FillSolid;
values.font = font->fid;
G_NormalGC (g) = XtGetGC ((Widget)mw, value_mask, &values);

/*	Get top shadow GC.
 */
if (G_ControlType (g) == XmCONTROL_SWITCH)
    values.foreground = G_PixmapBackground (g);
else
    values.foreground = mw -> manager.top_shadow_color;
values.background = G_Background (g);
G_TopShadowGC (g) = XtGetGC ((Widget)mw, value_mask, &values);

/*	Get bottom shadow GC.
 */
if (G_ControlType (g) == XmCONTROL_SWITCH)
    values.foreground = G_PixmapForeground (g);
else
    values.foreground = mw -> manager.bottom_shadow_color;
values.background = G_Background (g);
G_BottomShadowGC (g) = XtGetGC ((Widget)mw, value_mask, &values);

/*	Get background GC.
 */
values.foreground = G_Background (g);
values.background = G_Foreground (g);
G_BackgroundGC (g) = XtGetGC ((Widget)mw, value_mask, &values);

/*	Get armed background GC.
 */
values.foreground = G_ArmColor (g);
values.background = G_Background (g);
G_ArmedBackgroundGC (g) = XtGetGC ((Widget)mw, value_mask, &values);

/*	Get Clip GC
 */
if (G_Mask(g) != XmUNSPECIFIED_PIXMAP)
    {
    value_mask |= GCClipMask;
    values.clip_mask = G_Mask(g);
    }
values.foreground = G_Foreground (g);
values.background = G_Background (g);
G_ClipGC (g) = XtGetGC ((Widget)mw, value_mask, &values);
}


/*-------------------------------------------------------------
**	Public Entry Points
**-------------------------------------------------------------
*/


/*-------------------------------------------------------------
**	DtControlCallCallback
**		Call callback with reason and event.
**-------------------------------------------------------------
*/
#if 0
void 
#ifdef _NO_PROTO
DtControlCallCallback( w, reason, event )
	Widget w;
        int reason;
        XEvent *event;
#else
DtControlCallCallback(
	Widget w,
        int reason,
        XEvent *event )
#endif /* _NO_PROTO */
{
        DtControlGadget		g = (DtControlGadget) w;
	G_CallCallback ((DtIconGadget) g, G_Callback (g), reason, event);
}
#endif /* 0 */


/*-------------------------------------------------------------
**	Push Animation Timeout
**              An XtTimerCallbackProc.
*/
static void 
#ifdef _NO_PROTO
PushAnimationTimeout( client_data, id )
        XtPointer client_data;
        XtIntervalId *id ;
#else
PushAnimationTimeout(
        XtPointer client_data,
        XtIntervalId *id )
#endif /* _NO_PROTO */
{
DtControlGadget	g = (DtControlGadget) client_data;
XtAppContext    app_context = XtWidgetToApplicationContext ((Widget) g);

if ((G_PushImagePosition (g) > 0) &&
    (G_PushImagePosition (g) <= G_NumPushImages (g)))
    {
    G_PushAnimationTimer (g) = 
	XtAppAddTimeOut (app_context,
				G_PushDelays (g)[G_PushImagePosition (g) - 1],
				PushAnimationTimeout, (XtPointer) g);
    }
if (G_PushImagePosition (g) > 1)
    G_Expose ((Widget) g, NULL, NULL);
}


/*-------------------------------------------------------------
**	_DtControlDoPushAnimation
**		Do Push animation.
**-------------------------------------------------------------
*/
void 
#ifdef _NO_PROTO
_DtControlDoPushAnimation( w )
	Widget w;
#else
_DtControlDoPushAnimation(
	Widget w )
#endif /* _NO_PROTO */
{
DtControlGadget	 g = (DtControlGadget) w;
XtAppContext	 app_context = XtWidgetToApplicationContext (w);

if ((G_NumPushImages (g) > 0) && (G_PushImagePosition (g) == 0))
    {
    G_PushImagePosition (g) = 1;
    PushAnimationTimeout ((XtPointer) g, NULL);
    }
}


/*-------------------------------------------------------------
**	_DtControlAddPushAnimationImage
**		Add Push animation image.
**-------------------------------------------------------------
*/
void 
#ifdef _NO_PROTO
_DtControlAddPushAnimationImage( w, image, delay )
	Widget w;
	String image;
        int delay;
#else
_DtControlAddPushAnimationImage(
	Widget w,
	String image,
        int delay )
#endif /* _NO_PROTO */
{
DtControlGadget	 g = (DtControlGadget) w;
XmManagerWidget		 mw = (XmManagerWidget) XtParent (w);
int			 i;
XGCValues		 values;
XtGCMask		 value_mask;

/*	Allocate blocks of animation data.
 */
if (G_NumPushImages (g) == G_MaxPushImages (g))
    {
    G_MaxPushImages (g) += NUM_LIST_ITEMS;
    G_PushDelays (g) = (int*)
	XtRealloc ((char*) G_PushDelays (g),
                        G_MaxPushImages (g) *sizeof (int));
    G_PushPixmaps (g) = (Pixmap*)
	XtRealloc ((char*) G_PushPixmaps (g),
                        G_MaxPushImages (g) *sizeof (Pixmap));
    G_PushMasks (g) = (Pixmap*)
	XtRealloc ((char*) G_PushMasks (g),
                        G_MaxPushImages (g) *sizeof (Pixmap));
    }

/*	Allocate animation image.
 */
i = G_NumPushImages (g)++;
G_PushDelays (g)[i] = delay;
G_PushPixmaps (g)[i] = XmGetPixmap (XtScreen (g), image,
				G_PixmapForeground (g), G_PixmapBackground (g));
G_PushMasks (g)[i] = _DtGetMask (XtScreen (g), image);
}


/*-------------------------------------------------------------
**	Drop Animation Timeout
**              An XtTimerCallbackProc.
*/
static void
#ifdef _NO_PROTO
DropAnimationTimeout( client_data, id )
	XtPointer client_data ;
        XtIntervalId *id ;
#else
DropAnimationTimeout(
	XtPointer client_data,
        XtIntervalId *id )
#endif /* _NO_PROTO */
{
DtControlGadget	g = (DtControlGadget) client_data;
XtAppContext	app_context = XtWidgetToApplicationContext ((Widget) g);

if ((G_DropImagePosition (g) > 0) &&
    (G_DropImagePosition (g) <= G_NumDropImages (g)))
    {
    G_DropAnimationTimer (g) = 
	 XtAppAddTimeOut (app_context,
				G_DropDelays (g)[G_DropImagePosition (g) - 1],
				DropAnimationTimeout, (XtPointer) g);
    }
G_Expose ((Widget) g, NULL, NULL);
}


/*-------------------------------------------------------------
**	_DtControlDoDropAnimation
**		Do drop animation.
**-------------------------------------------------------------
*/
void 
#ifdef _NO_PROTO
_DtControlDoDropAnimation( w )
	Widget w;
#else
_DtControlDoDropAnimation(
	Widget w )
#endif /* _NO_PROTO */
{
DtControlGadget	 g = (DtControlGadget) w;
XtAppContext	 app_context = XtWidgetToApplicationContext (w);

if ((G_NumDropImages (g) > 0) && (G_DropImagePosition (g) == 0))
    {
    G_DropImagePosition (g) = 1;
    DropAnimationTimeout ((XtPointer) g, NULL);
    }
}


/*-------------------------------------------------------------
**	_DtControlAddDropAnimationImage
**		Add drop animation image.
**-------------------------------------------------------------
*/
void 
#ifdef _NO_PROTO
_DtControlAddDropAnimationImage( w, image, delay )
	Widget w;
	String image;
        int delay;
#else
_DtControlAddDropAnimationImage(
	Widget w,
	String image,
        int delay )
#endif /* _NO_PROTO */
{
DtControlGadget	 g = (DtControlGadget) w;
XmManagerWidget		 mw = (XmManagerWidget) XtParent (w);
int			 i;
XGCValues		 values;
XtGCMask		 value_mask;

/*	Allocate blocks of animation data.
 */
if (G_NumDropImages (g) == G_MaxDropImages (g))
    {
    G_MaxDropImages (g) += NUM_LIST_ITEMS;
    G_DropDelays (g) = (int*)
	XtRealloc ((char*) G_DropDelays (g),
                        G_MaxDropImages (g) *sizeof (int));
    G_DropPixmaps (g) = (Pixmap*)
	XtRealloc ((char*) G_DropPixmaps (g),
                        G_MaxDropImages (g) *sizeof (Pixmap));
    G_DropMasks (g) = (Pixmap*)
	XtRealloc ((char*) G_DropMasks (g),
                        G_MaxDropImages (g) *sizeof (Pixmap));
    }

/*	Allocate animation image.
 */
i = G_NumDropImages (g)++;
G_DropDelays (g)[i] = delay;
G_DropPixmaps (g)[i] = XmGetPixmap (XtScreen (g), image,
				G_PixmapForeground (g), G_PixmapBackground (g));
G_DropMasks (g)[i] = _DtGetMask (XtScreen (g), image);
}


/*-------------------------------------------------------------
**	_DtControlSetFileChanged
**		Set file changed.
**-------------------------------------------------------------
*/
void 
#ifdef _NO_PROTO
_DtControlSetFileChanged( w, b )
	Widget w;
	Boolean b;
#else
_DtControlSetFileChanged(
	Widget w,
	Boolean b )
#endif /* _NO_PROTO */
{
DtControlGadget	 g = (DtControlGadget) w;

if ((G_ControlType (g) == XmCONTROL_MONITOR) ||
    (G_ControlType (g) == XmCONTROL_MAIL))
    {
    G_FileChanged (g) = b;
    G_Expose ((Widget) g, NULL, False);
    }
}


/*-------------------------------------------------------------
**	_DtControlSetBusy
**		Set busy.
**-------------------------------------------------------------
*/
void 
#ifdef _NO_PROTO
_DtControlSetBusy( w, b )
	Widget w;
	Boolean b;
#else
_DtControlSetBusy(
	Widget w,
	Boolean b )
#endif /* _NO_PROTO */
{
DtControlGadget	 g = (DtControlGadget) w;

if (G_ControlType (g) != XmCONTROL_BUSY)
    return;

if (b)
    {
    G_BlinkElapsed (g) = 0;
    ++ G_Busy (g);
    if (G_Busy (g) == 1)
	{
	G_CallCallback ((DtIconGadget) g, G_Callback (g), XmCR_BUSY_START,
				NULL);
	BusyTimeout (w, NULL);
	}
    }
else
    {
    if (G_Busy (g) > 0)
	--G_Busy (g);
    }
}


/*-------------------------------------------------------------
**	_DtCreateControl
**		Create a new gadget instance.
**-------------------------------------------------------------
*/
Widget 
#ifdef _NO_PROTO
_DtCreateControl( parent, name, arglist, argcount )
        Widget parent ;
        String name ;
        ArgList arglist ;
        int argcount ;
#else
_DtCreateControl(
        Widget parent,
        String name,
        ArgList arglist,
        int argcount )
#endif /* _NO_PROTO */
{
	return (XtCreateWidget (name, dtControlGadgetClass, 
			parent, arglist, argcount));
}

#ifdef AIX_ILS
static void
#ifdef _NO_PROTO
ReplaceJPDate(date, jpstr, wday)
        char *date;
        char *jpstr;
        int wday;
#else
ReplaceJPDate(char *date, char *jpstr, int wday)
#endif /* _NO_PROTO */
{
    char *s, *rp, *sp;
    char *p = NULL;
    char abday[5];
    char newdate[128];
    int i, j, k;

    s = (char *)malloc((strlen(jpstr) + 1) * sizeof(char));
    strcpy(s, jpstr);
    for(p = strtok(s, ",") ,i = 0; p != NULL && i < wday ;
        p = strtok(NULL, ","), i++);
    if(p == NULL) {
        free(s);
        return;
    }
    strcpy(abday, nl_langinfo(ABDAY_1 + wday));
    if((rp = strstr(date, abday)) != NULL) {
        for(i = 0, j = 0, sp = date; date[j] != '\0'; sp++) {
            if(sp == rp) {
                for(k = 0; k < strlen(p); k++)
                    newdate[i++] = p[k];
                j += strlen(abday);
            }
            else
                newdate[i++] = date[j++];
        }
        newdate[i] = '\0';
        strcpy(date, newdate);
    }
    free(s);
    return;
}
#endif /* AIX_ILS */

char
_DtControlGetMonitorState(Widget w)
{
    DtControlGadget	 g = (DtControlGadget) w;

    if (G_FileChanged (g))
       return(DtMONITOR_ON);
    else
       return(DtMONITOR_OFF);
}
