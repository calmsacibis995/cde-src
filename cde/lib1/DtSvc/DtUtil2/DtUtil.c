/* $XConsortium: DtUtil.c /main/cde1_maint/2 1995/11/05 12:19:15 drk $ */
/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/******************************************************************************
 *
 * File Name: DtUtil.c
 *
 *  Contains the DT functions used by an application to connect to the
 *  underlying communications mechanism (in this case, the BMS).
 *
 *****************************************************************************/
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Dt/DtP.h>
#include <Dt/UserMsg.h>
#include <Dt/DtNlUtils.h>
#include <Dt/EnvControl.h>

/*****************************************
 *
 * External functions not defined in header files.
 *
 *****************************************/

#ifdef _NO_PROTO
extern void DtStatusResponse ();
extern void DtBlockNonfatalErrors ();
extern void _DtReleaseLock ();
extern void DtRestoreAllErrors ();
extern Boolean _DtInitializeToolTalk();
#else
extern void DtStatusResponse ( void );
extern void DtBlockNonfatalErrors ( void );
extern void _DtReleaseLock ( Display *, char * );
extern void DtRestoreAllErrors ( void );
extern Boolean _DtInitializeToolTalk(Widget widget);
#endif


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Boolean DtAppInitialize() ;
extern Boolean DtInitialize() ;

#else

extern Boolean DtAppInitialize( 
                        XtAppContext app_context,
                        Display *display,
                        Widget widget,
                        char *name,
                        char *toolClass) ;
extern Boolean DtInitialize( 
                        Display *display,
                        Widget widget,
                        char *name,
                        char *toolClass) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static Boolean DtBigInitialize() ;
static void InitButtonLabels() ;
static void DtGlobalsInitialize() ;

#else

static Boolean DtBigInitialize( 
                        Display *display,
                        Widget widget,
                        char *name,
                        char *toolClass,
                        Boolean bms_connection,
                        Boolean use_app_context,
                        XtAppContext app_context) ;
static void InitButtonLabels( void ) ;
static void DtGlobalsInitialize( 
                        Display *display,
                        char *name,
                        char *toolClass) ;

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/


/*****************************************
 *
 * Global variables 
 *
 *****************************************/

XrmDatabase _DtResourceDatabase = NULL;	/* This Dt global indicates which
					   Xrm database should be read for
					   resources. */

XtAppContext _DtAppContext;		/* This Dt global keeps track of the
					   app-context, if one has been 
					   specified.  Note that libXv does
					   not yet support multiple app-
					   contexts. */
XtAppContext *_DtInitAppContextp = NULL;
Widget   _DtInitTtContextWidget = NULL;

Display *_dtDisplay = NULL;		/* This global variable is saved
					   when a client invokes "DtInitialize"
					   It is used later to get resources
					   when the DT databases are loaded.*/
char *_dtApplicationName = NULL;	/* This global variable is the
					   client's "ApplicationName". */
char *_dtApplicationClass = NULL;	/* This global variable is the
					   client's "ApplicationClass". */
char *_dtToolClass = NULL;		/* Tool class passed to _DtInit...() */


/* Localizable button labels */
char * _DtOkString = NULL;
char * _DtCancelString = NULL;
char * _DtHelpString = NULL;
char * _DtApplyString = NULL;
char * _DtCloseString = NULL;


/*********************************************
 *
 * Initialization Functions
 *
 *********************************************/

Boolean 
#ifdef _NO_PROTO
DtAppInitialize( app_context, display, widget, name, toolClass )
        XtAppContext app_context ;
        Display *display ;
        Widget widget;
        char *name ;
        char *toolClass ;
#else
DtAppInitialize(
        XtAppContext app_context,
        Display *display,
        Widget widget,
        char *name,
        char *toolClass )
#endif /* _NO_PROTO */
{
   /* We need both a BMS connection and an app-context. */
   return (DtBigInitialize (display, widget, name, toolClass, TRUE, TRUE, 
			    app_context));
}

Boolean 
#ifdef _NO_PROTO
DtInitialize( display, widget, name, toolClass )
        Display *display ;
        Widget widget;
        char *name ;
        char *toolClass ;
#else
DtInitialize(
        Display *display,
        Widget widget,
        char *name,
        char *toolClass )
#endif /* _NO_PROTO */
{
   /* We need a BMS connection but no app-context. */
   return (DtBigInitialize (display, widget, name, toolClass, TRUE, FALSE, 
                             NULL));
}

static Boolean 
#ifdef _NO_PROTO
DtBigInitialize( display, widget, name, toolClass, bms_connection, use_app_context, app_context )
        Display *display ;
        Widget widget;
        char *name ;
        char *toolClass ;
        Boolean bms_connection ;
        Boolean use_app_context ;
        XtAppContext app_context ;
#else
DtBigInitialize(
        Display *display,
        Widget widget,
        char *name,
        char *toolClass,
        Boolean bms_connection,
        Boolean use_app_context,
        XtAppContext app_context )
#endif /* _NO_PROTO */
   
{
   static Boolean initialized = False;
   static Boolean init_ret_status;


   /* Initialization can only be performed once. */
   if (initialized)
      return (init_ret_status);


   /* Preserve the pre-Dt environ and add Dt-specifics to environ */
   (void) _DtEnvControl (DT_ENV_SET); 

   /* Initialize a bunch of miscellaneous things. */
   DtNlInitialize();
   InitButtonLabels();
   DtGlobalsInitialize (display, name, toolClass);

   if ( XmIsGadget(widget) )
        _DtInitTtContextWidget = XtParent(widget);
   else
        _DtInitTtContextWidget = widget;

   _DtAppContext =  XtWidgetToApplicationContext(_DtInitTtContextWidget);
   _DtInitAppContextp = &_DtAppContext;

   init_ret_status = initialized = TRUE;

   return (init_ret_status);
}


/* Initialize the global button labels */

static void 
#ifdef _NO_PROTO
InitButtonLabels()
#else
InitButtonLabels( void )
#endif /* _NO_PROTO */

{
   _DtOkString = strdup(((char *)Dt11GETMESSAGE(28, 1, "OK")));
   _DtCancelString = strdup(((char *)Dt11GETMESSAGE(28, 2, "Cancel")));
   _DtHelpString = strdup(((char *)Dt11GETMESSAGE(28, 3, "Help")));
   _DtApplyString = strdup(((char *)Dt11GETMESSAGE(28, 4, "Apply")));
   _DtCloseString = strdup(((char *)Dt11GETMESSAGE(28, 5, "Close")));
}

static void 
#ifdef _NO_PROTO
DtGlobalsInitialize( display, name, toolClass )
        Display *display ;
        char *name ;
        char *toolClass ;
#else
DtGlobalsInitialize(
        Display *display,
        char *name,
        char *toolClass )
#endif /* _NO_PROTO */
{
   _DtResourceDatabase = (XrmDatabase) XtDatabase (display);
   DtProgName = name;
   _dtToolClass = XtNewString(toolClass);

   XeToolClass = XtNewString (toolClass);

   /*
    * Save the application name and application class.
    */
   _dtDisplay = (Display *) display;
   XtGetApplicationNameAndClass (display,
				 &_dtApplicationName,
				 &_dtApplicationClass);
}   
