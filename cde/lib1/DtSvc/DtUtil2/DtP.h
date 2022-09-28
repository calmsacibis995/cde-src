/*
 * File:         DtP.h $XConsortium: DtP.h /main/cde1_maint/1 1995/07/14 20:32:23 drk $
 * Language:     C
 *
 * (c) Copyright 1990, Hewlett-Packard Company, all rights reserved.
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc.				*
 */

#ifndef _DtP_h
#define _DtP_h

#ifdef _SUN_OS
/* This is a hack to satisfy the sun xpg2lib requirements of a definition
   for the assert macro */
#include <assert.h>
#endif /* _SUN_OS */


/* If <X11/Intrinsic.h> is included along with this file, <X11/Intrinsic.h>
   MUST be included first due to "typedef Boolean ..." problems.  The following
   conditional #define is also part of the solution to this problem. */
#ifdef _XtIntrinsic_h
#	define TOOLKIT
#endif

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xmd.h>    /* for protocol typedefs */

#ifndef _NO_PROTO
#include <X11/Intrinsic.h>
#endif

#ifndef __STDC__
#  ifndef const
#    define const
#  endif
#endif
#include <bms/sbport.h>
#include <bms/bms.h>

#include <Dt/DtPStrings.h>

/*********************************
 *
 * Miscellaneous Data Types
 *
 *********************************/

#define DtChar		XeChar
#define DtString	XeString

/*********************************
 *
 * Initalization
 *
 *********************************/

#define DtToolClass			XeToolClass
extern XrmDatabase _DtResourceDatabase;

/*********************************
 *
 * Global variables
 *
 *********************************/
extern Display *_dtDisplay;
extern char *_dtApplicationName;
extern char *_dtApplicationClass;
extern char * _dtToolClass;

/**********************************
 *
 * Stack Alloc macros
 *
 **********************************/
#define DtStackAlloc(size, stack_cache_array)     \
    ((size) <= sizeof(stack_cache_array)          \
    ?  (XtPointer)(stack_cache_array)             \
    :  XtMalloc((unsigned)(size)))

#define DtStackFree(pointer, stack_cache_array) \
    if ((pointer) != ((XtPointer)(stack_cache_array))) XtFree(pointer);


#ifdef _NO_PROTO
extern void   _DtAddToResource();
extern void   _DtAddResString();
extern char * _DtGetResString( Display *dpy, unsigned int);
#else
extern void   _DtAddToResource( Display *, char * );
extern void   _DtAddResString( Display *, char *, unsigned int);
extern char * _DtGetResString( Display *dpy, unsigned int);

#endif

#define _DT_ATR_RESMGR   (1 << 0)
#define _DT_ATR_PREFS    (1 << 1)

/*
    DESCRIPTION:

	Add strings to XA_RESOURCE_MANAGER property on the default root
	window.  Correctly merges resource specifications with the same
	name and different values.  The new value overwrites the old.

        _DtAddToResource() may be used where you would have used xrdb to 
        add a resource.

    SYNOPSIS:

        void _DtAddToResource(dpy,data)

        Display *dpy;       The application's display structure.

        char *data;          The string to be added to the
                             XA_RESOURCE_MANAGER property.

*/

#ifdef _NO_PROTO
extern char *_DtCreateDtDirs();
#else
extern char *_DtCreateDtDirs( Display * );
#endif
/*
    DESCRIPTION:

	Creates the directories needed for dt to operate in.  When an
        application saves its state inside a file, it should call this
        routine to set up the directories before saving any files.  The
        routine constructs the path to which all save files should be saved
        to when responding the the WM_SAVE_YOURSELF message issued by the
        session manager.  The routine  returns the path to save to.  It also
        allocates the memory for the path so when you are done with it you
        should free() it.

        WARNING:  If it can't create the directory it returns NULL

    SYNOPSIS:

	dirName = _DtCreateDtDirs (display);

	char *dirName;		The path to save to.

	Display *display;	The application's display structure.
*/


#ifdef _NO_PROTO
extern Boolean DtInitialize ();
extern Boolean DtAppInitialize ();
#else
extern Boolean DtInitialize ( Display *, Widget, char *, char * );
extern Boolean DtAppInitialize ( XtAppContext, Display *, Widget, char *, char * );
#endif
/*
    DESCRIPTION:
    
	The following initialization functions for libDtSvc are supplied.
	Your application should use one of these functions based on the
	following rules:
	
		DtInitialize		Applications that use the default
					Xt app-context created by
					XtInitialize.
					
		DtAppInitialize		Applications that create their own
					Xt app-context.  [NOTE: Multiple
					app-contexts are not supported.]

	Responsible for initializing:
	
		- _DtResourceDatabase (used by the Action Library and
		  Datatyping library)
		- DtProgName
		- DtToolClass

	The appropriate Dt[App]Initialize function must be called before any
	of the other Dt functions or global variables are referenced.
	After calling the Dt[App]Initialize function you should do any other 
	initialization you have, including using DtAdd...Callback to register 
	for messages you want to receive.
		
    SYNOPSIS:
    
	statu = DtInitialize (display, Widget, name, toolClass);
	statu = DtAppInitialize (appContext, display, Widget, name, toolClass);
	
	Boolean statu;			Returns TRUE if the library is 
					correctly initialized, FALSE otherwise.
					
	XtAppContext appContext;	The application context obtained
					from XtCreateApplicationContext.
					
	Display *display;		The display where this application
					is displaying.  This is used to find
					the BMS and the appropriate Xrm
					database.
					
	char *name;			The name used to invoke this
					application.  Typically argv[0].
					
	char *toolClass;		The tool class that this application
					is known by.  E.g. EDIT or FILEMGR.
*/


/*
 * The following string globals are available for use by any DT
 * component.  They represent the button labels in most dialogs,
 * and will be automatically localized by DtInitialize().
 */
extern char * _DtOkString;
extern char * _DtCancelString;
extern char * _DtHelpString;
extern char * _DtApplyString;
extern char * _DtCloseString;

/* Global copyright string for use by all Dt components */
extern char * DtCopyRightString;

/*
 * NLS macros and externs
 */
#ifdef _NO_PROTO
extern char *Dt11GetMessage ();
#else
extern char *Dt11GetMessage (
	char *filename,
	int set,
	int number,
	char *string);
#endif /* _NO_PROTO */

#ifdef MESSAGE_CAT
#define _MESSAGE_CAT_NAME	"dt"
#define Dt11GETMESSAGE(set, number, string)\
    Dt11GetMessage(_MESSAGE_CAT_NAME, set, number, string)
#else /* MESSAGE_CAT */
#define Dt11GETMESSAGE(set, number, string)\
    string
#endif /* MESSAGE_CAT */

/* Do not add anything after this endif. */
#endif /* _DtP_h */
