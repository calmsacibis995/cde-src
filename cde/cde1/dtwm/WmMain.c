/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.1
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: WmMain.c /main/cde1_maint/2 1995/10/09 13:44:37 pascale $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include "WmGlobal.h"

#ifndef NO_MULTIBYTE
#include <locale.h>
#endif
#ifdef WSM
#include <Dt/Message.h>
#include <Dt/EnvControl.h>
#endif /* WSM */
/*
 * include extern functions
 */

#include "WmCEvent.h"
#include "WmEvent.h"
#include "WmInitWs.h"
#ifdef WSM
#include "WmIPC.h"
#include "WmBackdrop.h"
#endif /* WSM */


/*
 * Function Declarations:
 */
#ifdef WSM
#ifdef _NO_PROTO
int WmReturnIdentity();
#else /* _NO_PROTO */
int WmReturnIdentity (int argc, char *argv[], char *environ[]);
#endif /* _NO_PROTO */
#define ManagedRoot(w) (!XFindContext (DISPLAY, (w), wmGD.screenContextType, \
(caddr_t *)&pSD) ? (SetActiveScreen (pSD), True) : \
(IsBackdropWindow (ACTIVE_PSD, (w))))
#else /* WSM */
#define ManagedRoot(w) (!XFindContext (DISPLAY, (w), wmGD.screenContextType, \
(caddr_t *)&pSD) ? (SetActiveScreen (pSD), True) : False)
#endif /* WSM */

WmScreenData *pSD;

/*
 * Global Variables:
 */

WmGlobalData wmGD;
#ifdef MESSAGE_CAT
NlsStrings wmNLS;
#endif /* MESSAGE_CAT */
#ifdef WSM
int WmIdentity;
#endif /* WSM */



/*************************************<->*************************************
 *
 *  main (argc, argv, environ)
 *
 *
 *  Description:
 *  -----------
 *  This is the main window manager function.  It calls window manager
 *  initializtion functions and has the main event processing loop.
 *
 *
 *  Inputs:
 *  ------
 *  argc = number of command line arguments (+1)
 *
 *  argv = window manager command line arguments
 *
 *  environ = window manager environment
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
int
main (argc, argv, environ)
    int argc;
    char *argv[];
    char *environ[];

#else /* _NO_PROTO */
int
main (int argc, char *argv [], char *environ [])
#endif /* _NO_PROTO */
{
    XEvent	event;
    Boolean	dispatchEvent;

    setlocale(LC_ALL, "");

#ifdef __osf__
    svc_init();
#endif

#ifndef NO_MULTIBYTE
#ifdef WSM
    /*
     * Set up environment variables for this HP DT client
     */
    _DtEnvControl(DT_ENV_SET);

    /*
     * Force LANG lookup early. 
     * (Front end may change $LANG to 'C' as part
     *  of string space reduction optimization.)
     */
     {
	 char * foo = ((char *)GETMESSAGE(44, 1, ""));
     }
#endif /* WSM */
    XtSetLanguageProc (NULL, (XtLanguageProc)NULL, NULL);
#endif
#ifdef WSM
    /*  
     * Get Identity
     */
    WmIdentity = WmReturnIdentity(argc, argv, environ);
#endif /* WSM */

    /*
     * Initialize the workspace:
     */

    InitWmGlobal (argc, argv, environ);
#ifdef WSM

    /*
     * Set up PATH variable if it must run as standalone command
     * invoker
     */
    if (wmGD.dtLite)
    {
	_DtEnvControl(DT_ENV_SET_BIN);
    }
#endif /* WSM */
    
    /*
     * MAIN EVENT HANDLING LOOP:
     */

    for (;;)
    {
        XtAppNextEvent (wmGD.mwmAppContext, &event);


        /*
	 * Check for, and process non-widget events.  The events may be
	 * reported to the root window, to some client frame window,
	 * to an icon window, or to a "special" window management window.
	 * The lock modifier is "filtered" out for window manager processing.
	 */

	wmGD.attributesWindow = 0L;

#ifdef WSM
	if ((event.type == ButtonPress) || 
	    (event.type == ButtonRelease))
	{
	    if ((wmGD.evLastButton.button != 0) &&
		ReplayedButtonEvent (&(wmGD.evLastButton), 
				     &(event.xbutton)))
	    {
		wmGD.bReplayedButton = True;
	    }
	    else
	    {
		/* save this button for next comparison */
		memcpy (&wmGD.evLastButton, &event, sizeof (XButtonEvent));
		wmGD.bReplayedButton = False;
	    }
	}
#endif /* WSM */
	dispatchEvent = True;
	if (wmGD.menuActive)
	{
	    /*
	     * Do special menu event preprocessing.
	     */

	    if (wmGD.checkHotspot || wmGD.menuUnpostKeySpec ||
		wmGD.menuActive->accelKeySpecs)
	    {
	        dispatchEvent = WmDispatchMenuEvent ((XButtonEvent *) &event);
	    }
	}

	if (dispatchEvent)
	{
	    if (ManagedRoot(event.xany.window))
	    {
	        dispatchEvent = WmDispatchWsEvent (&event);
	    }
	    else
	    {
	        dispatchEvent = WmDispatchClientEvent (&event);
	    }

	    if (dispatchEvent)
	    {
                /*
                 * Dispatch widget related event:
                 */

                XtDispatchEvent (&event);
	    }
	}
    }

} /* END OF FUNCTION main */

#ifdef WSM
/******************************<->*************************************
 *
 *  WmReturnIdentity (argc, argv, environ)
 *
 *
 *  Description:
 *  -----------
 *  This function checks the last component of the (path)name
 *  contained in argv[0] and makes a global decision as to whether
 *  it should fetch resources as mwm or dtwm.
 *
 *  Inputs:
 *  ------
 *  argc = number of command line arguments (+1)
 *
 *  argv = window manager command line arguments
 *
 *  environ = window manager environment
 *
 ******************************<->***********************************/

#ifdef _NO_PROTO
int WmReturnIdentity (argc, argv, environ)
int argc;
char *argv[];
char *environ[];
#else /* _NO_PROTO */
int WmReturnIdentity ( int argc, char *argv[], char *environ[]) 
#endif /* _NO_PROTO */
{
	char *tempString;
	char *origPtr;

	/* assume it's dtwm until proven differently */

 	int retVal = DT_MWM;

	if (!(tempString = 
	      (char *)(XtMalloc ((unsigned int)(strlen (argv[0]) + 1)))))
	{
		Warning(((char *)GETMESSAGE(44, 2, "Insufficient memory for name of window manager")));
		exit(WM_ERROR_EXIT_VALUE);
	}

	origPtr = tempString;

	if (strrchr(argv[0], '/'))
	{
		
		strcpy(tempString, (strrchr(argv[0], '/')));

		tempString++;
	}
	else
		strcpy(tempString, argv[0]);

	if (!(strcmp(tempString, WM_RESOURCE_NAME)))
	/*
	 *
	 *   If it's explicity "mwm", then set our identity anew.
	 *
	 */
	{
		retVal = MWM;
	}

	XtFree((char *)origPtr);

	return(retVal);

} /* END OF FUNCTION WmReturnIdentity */
#endif /* WSM */
#ifdef WSM
/*************************     eof   ******************************/
#endif /* WSM */
