/**********************************<+>*************************************
***************************************************************************
**
**  File:        main.c
**
**  Project:     DT dtpad, a memo maker type editor based on the Dt Editor
**               widget.
**
**  Description:
**  -----------
**
**  Dtpad is basically a wrapper around the DT Editor widget providing:
**
**    o	a menu interface to Editor widget features
**    o	client/server architecture (via the DtSvc library)
**    o	session save/restore
**    o	wm_delete protocol and "panic save" signal handling
**    o	ToolTalk support of media exchange and desktop message sets
**    o	printing
**
**  This file contains the routines that control the client/server architecture
**  of dtpad and creation of the top level widgets.
**
**  See the comments preceeding main() for a description of the	client/server
**  architecture of dtpad as well as an overview of the routine hierarchy of
**  dtpad.  The widget hierarchy of dtpad goes something like this:
**
**	Creating Routine	Creating Function	Widget
**	----------------------	----------------------	-----------------------
**  (1)	CreateFirstPad:		XtInitialize()		topLevelWithWmCommand
**  (2)	CreateFirstPad:		XtAppCreateShell()	  pPad->app_shell
**  (3)	  RealizeNewPad:	XmCreateMainWindow()	    pPad->mainWindow
**     	    CreateMenuBar:	XmCreateMenuBar()	      pPad->menu_bar
**	      CreateFileMenu:	XmCreatePulldownMenu()		(file_pane)
**	      CreateEditMenu:	XmCreatePulldownMenu()		(edit_pane)
**	      CreateFormatMenu: XmCreatePulldownMenu()		(format_pane)
**	      CreateOptionsMenu:XmCreatePulldownMenu()		(options_pane)
**	      CreateHelpMenu:   XmCreatePulldownMenu()		(help_pane)
**          CreateEditorWidget:	DtCreateEditor()	      pPad->editor
**
**  (1)	The topLevelWithWmCommand is only created once whereas the others
**	are created and realized whenever a Editor window is opened.
**      The display is opened once but stored in the pPad Editor struct
**	associated with each opened edit window.
**  (2)	The pPad->app_shell is created initially (in CreateFirstPad) and
**	(in dtpad.c:FindOrCreatePad) whenever the server receives a message
**	(via messageCB.c:OpenNewPadCB) to start another edit session.
**  (3)	The pPad->mainWindow as well as the pPad->menu_bar and pPad->editor
**	are only created via RealizeNewPad.  RealizeNewPad is called
**	whenever an app_shell is created and also when a session is restored.
**
*******************************************************************
**  (c) Copyright Hewlett-Packard Company, 1990.  All rights are
**  reserved.  Copying or other reproduction of this program
**  except for archival purposes is prohibited without prior
**  written consent of Hewlett-Packard Company.
********************************************************************
**
********************************************************************
**  (c) Copyright 1993, 1994 Hewlett-Packard Company
**  (c) Copyright 1993, 1994 International Business Machines Corp.
**  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
**  (c) Copyright 1993, 1994 Novell, Inc.
********************************************************************
**
**
**************************************************************************
**********************************<+>*************************************/
#include "dtpad.h"
#include "stdlib.h"
#include "messageParam.h"
#include "signal.h"
#include <locale.h>
#include <sys/wait.h>
#include <Xm/MwmUtil.h>
#include <Dt/EnvControl.h>
#include <Dt/DtpadM.h>
#include "X11/Xutil.h"
#include <X11/StringDefs.h>
/* include <Xm/ToggleBP.h>	XXX - debug only */

#ifdef MESSAGE_CAT
#define TRUE 1
#define FALSE 0
#include <nl_types.h>
#if !defined(NL_CAT_LOCALE)
#define NL_CAT_LOCALE 0
#endif
#ifdef __ultrix
#define _DTPAD_CAT_NAME "dtpad.cat"
#else  /* __ultrix */
static const char catalogName[] = "dtpad";
#define _DTPAD_CAT_NAME catalogName
#endif /* __ultrix */
#endif /* MESSAGE_CAT */


/************************************************************************
 *			Global Definitions
 ************************************************************************/

Atom            xa_WM_DELETE_WINDOW;

/* bits for exclamation point in dialog */
unsigned char warningBits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00,
   0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00,
   0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00,
   0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00,
   0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00,
   0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00,
   0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x03, 0x00,
   0x00, 0xe0, 0x07, 0x00, 0x00, 0xf0, 0x0f, 0x00, 0x00, 0xf0, 0x0f, 0x00,
   0x00, 0xf0, 0x0f, 0x00, 0x00, 0xf0, 0x0f, 0x00, 0x00, 0xe0, 0x07, 0x00,
   0x00, 0xc0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00
};

static XtResource resources[] = {
  {"statusLine", "StatusLine", XmRBoolean, sizeof(Boolean),
      XtOffset(ApplicationDataPtr,statusLine), XmRImmediate, (XtPointer)False,
  },
  {"wrapToFit", "WrapToFit", XmRBoolean, sizeof(Boolean),
      XtOffset(ApplicationDataPtr,wordWrap), XmRImmediate, (XtPointer)False,
  },
  {"overstrike", "Overstrike", XmRBoolean, sizeof(Boolean),
      XtOffset(ApplicationDataPtr,overstrike), XmRImmediate, (XtPointer)False,
  },
  {"saveOnClose", "SaveOnClose", XmRBoolean, sizeof(Boolean),
      XtOffset(ApplicationDataPtr,saveOnClose), XmRImmediate, (XtPointer)False,
  },
  {"missingFileWarning", "MissingFileWarning", XmRBoolean, sizeof(Boolean),
      XtOffset(ApplicationDataPtr,missingFileWarning), XmRImmediate, (XtPointer)False,
  },
  {"readOnlyWarning", "ReadOnlyWarning", XmRBoolean, sizeof(Boolean),
      XtOffset(ApplicationDataPtr,readOnlyWarning), XmRImmediate, (XtPointer)True,
  },
  {"nameChange", "NameChange", XmRBoolean, sizeof(Boolean),
      XtOffset(ApplicationDataPtr,nameChange), XmRImmediate, (XtPointer)True,
  },
  {"viewOnly", "ViewOnly", XmRBoolean, sizeof(Boolean),
      XtOffset(ApplicationDataPtr,viewOnly), XmRImmediate, (XtPointer)False,
  },
  {"workspaceList", "WorkspaceList", XmRString, sizeof(char *),
      XtOffset(ApplicationDataPtr,workspaceList), XmRImmediate,(XtPointer)NULL,
  },
  {"session", "Session", XmRString, sizeof(char *),
      XtOffset(ApplicationDataPtr,session), XmRImmediate, (XtPointer)NULL,
  },

  {"standAlone", "StandAlone", XmRBoolean, sizeof(Boolean),
      XtOffset(ApplicationDataPtr,standAlone), XmRImmediate, (XtPointer)False,
  },
  {"blocking", "Blocking", XmRBoolean, sizeof(Boolean),
      XtOffset(ApplicationDataPtr,blocking), XmRImmediate, (XtPointer)True,
  },
  {"server", "Server", XmRBoolean, sizeof(Boolean),
      XtOffset(ApplicationDataPtr,server), XmRImmediate, (XtPointer)False,
  },
  {"exitOnLastClose", "ExitOnLastClose", XmRBoolean, sizeof(Boolean),
      XtOffset(ApplicationDataPtr,exitOnLastClose), XmRImmediate, (XtPointer)False,
  },

  {"idString", "IdString", XmRString, sizeof(char *),
      XtOffset(ApplicationDataPtr,idString), XmRImmediate, (XtPointer)NULL,
  },
  {"directory", "Directory", XmRString, sizeof(char *),
      XtOffset(ApplicationDataPtr,directory), XmRImmediate, (XtPointer)NULL,
  },
  {"saveAsDir", "SaveAsDir", XmRString, sizeof(char *),
      XtOffset(ApplicationDataPtr,saveAsDir), XmRImmediate, (XtPointer)NULL,
  },
  {"mainTitle", "MainTitle", XmRString, sizeof(char *),
      XtOffset(ApplicationDataPtr,mainTitle), XmRImmediate, (XtPointer)NULL,
  },
  {"nameInTitle", "NameInTitle", XmRBoolean, sizeof(Boolean),
      XtOffset(ApplicationDataPtr,nameInTitle), XmRImmediate, (XtPointer)True,
  },
  {"exitLabel", "ExitLabel", XmRString, sizeof(char *),
      XtOffset(ApplicationDataPtr,exitLabel), XmRImmediate, (XtPointer)NULL,
  },
  {"exitMnemonic", "ExitMnemonic", XmRString, sizeof (char *),
      XtOffset(ApplicationDataPtr,exitMnemonic), XmRImmediate, (XtPointer)NULL,
  },
  {"helpVol", "HelpVol", XmRString, sizeof (char *),
      XtOffset (ApplicationDataPtr,helpVol), XmRImmediate, (XtPointer)NULL,
  },
  {"menuBar", "MenuBar", XmRBoolean, sizeof(Boolean),
      XtOffset(ApplicationDataPtr,menuBar), XmRImmediate, (XtPointer)True,
  },
  {"confirmationString", "ConfirmationString", XmRString, sizeof(char *),
      XtOffset(ApplicationDataPtr,confirmationString), XmRImmediate, (XtPointer)NULL,
  },
  {"allowExit", "allowExit", XmRBoolean, sizeof(Boolean),
      XtOffset(ApplicationDataPtr,allowExit), XmRImmediate, (XtPointer)True,
  },
  {"closeImmediate", "CloseImmediate", XmRBoolean, sizeof(Boolean),
      XtOffset(ApplicationDataPtr,closeImmediate), XmRImmediate, (XtPointer)False,
  },
};

static XrmOptionDescRec option_list[] =
{
   {"-statusLine", "statusLine", XrmoptionNoArg,(caddr_t)"TRUE"},
   {"-wrapToFit", "wrapToFit", XrmoptionNoArg, (caddr_t)"TRUE"},
   {"-overstike", "overstike", XrmoptionNoArg, (caddr_t)"TRUE"},
   {"-saveOnClose", "saveOnClose", XrmoptionNoArg, (caddr_t)"TRUE"},
   {"-missingFileWarning", "missingFileWarning", XrmoptionNoArg, (caddr_t)"TRUE"},
   {"-noReadOnlyWarning", "readOnlyWarning", XrmoptionNoArg, (caddr_t)"FALSE"},
   {"-noNameChange", "nameChange", XrmoptionNoArg, (caddr_t)"FALSE"},
   {"-viewOnly", "viewOnly", XrmoptionNoArg, (caddr_t)"TRUE"},
   {"-workspaceList", "workspaceList", XrmoptionSepArg, (caddr_t)NULL},
   {"-session", "session", XrmoptionSepArg, (caddr_t)NULL},

   {"-standAlone", "standAlone", XrmoptionNoArg, (caddr_t)"TRUE"},
   {"-noBlocking", "blocking", XrmoptionNoArg, (caddr_t)"FALSE"},
   {"-server", "server", XrmoptionNoArg, (caddr_t)"TRUE"},
   {"-exitOnLastClose", "exitOnLastClose", XrmoptionNoArg, (caddr_t)"TRUE"},

   {"-idString", "idString", XrmoptionSepArg, (caddr_t)NULL},
   {"-directory", "directory", XrmoptionSepArg,(caddr_t)NULL},
   {"-saveAsDir", "saveAsDir", XrmoptionSepArg,(caddr_t)NULL},
   {"-mainTitle", "mainTitle", XrmoptionSepArg, (caddr_t)NULL},
   {"-noNameInTitle", "nameInTitle", XrmoptionNoArg,(caddr_t)"FALSE"},
   {"-exitLabel", "exitLabel", XrmoptionSepArg,(caddr_t)NULL},
   {"-exitMnemonic", "exitMnemonic", XrmoptionSepArg,(caddr_t)NULL},
   {"-helpVol", "helpVol", XrmoptionSepArg,(caddr_t)NULL},
   {"-noMenuBar", "menuBar", XrmoptionNoArg,(caddr_t)"FALSE"},
   {"-confirmationString", "confirmationString", XrmoptionSepArg,(caddr_t)NULL},
   {"-noExit", "allowExit", XrmoptionNoArg,(caddr_t)"FALSE"},
   {"-closeImmediate", "closeImmediate", XrmoptionNoArg,(caddr_t)"TRUE"},
};

ApplicationData initialServerResources;
char *initialServerGeometry;
Editor *pPadList = (Editor *)NULL;
int numActivePads = 0;
Widget topLevelWithWmCommand;
Boolean ActionDBInitialized = False;

#ifdef USE_SDTSPELL
/* Function pointer to the Solaris CDE speling checker. */
void *SDtSpellLibHandle; 
#endif

/************************************************************************
 *			Forward Declarations
 ************************************************************************/

static Editor *CreateFirstPad(
        int *pArgc,
	char **argv);
static void SendMessageToServer(
	Editor *pPad,
	int argc,
	char **argv);
static void InitEditSessionAndFirstPad(
        Editor *pPad,
	int argc,
	char **argv);
static void SetStateFromResources(
	Editor *pPad);
static void SetDialogTitleString(
        Editor *pPad);


/************************************************************************
 * main - controls the high level client/server processing of dtpad based 
 *	on the three modes of operation:
 *
 *	1) requestor mode -	send message to dtpad server to do actual
 *				editing
 *	2) server mode -	do editing for one or more dtpad requestors
 *				(via multiple edit windows)
 *	3) standAlone mode -	do all editing alone (via a single
 *				edit window)
 *
 ************************************************************************/ 
void 
main(
        int argc,
        char **argv )
{
    char *foo;
    Editor *pPad;

    int ttFd;
    char *sessId;
    Tt_pattern requests2Handle;
    Tt_message msg;

    setlocale(LC_ALL, "");

    /* Set up the environment to allow relocation of the various bitmaps
     * and message catalogs. */
    (void)_DtEnvControl(DT_ENV_SET);

    foo = ((char *)GETMESSAGE(7, 1, ""));

    (void) XtSetLanguageProc((XtAppContext)NULL, (XtLanguageProc)NULL, 
			     (XtPointer)NULL);
    /*
    while(1)
      ttFd = 0;
      */
    pPad = CreateFirstPad(&argc, argv);

    if (DtInitialize(pPad->display, topLevelWithWmCommand, pPad->progname,
                     DTPAD_TOOL_CLASS) != True ) {
	XtError((char *) GETMESSAGE(7, 4,
	  "Unable to initialize Desktop services ... exiting Text Editor."));
    }

    /* -----> Resolve conficts between -standAlone, -session and -server */
    if (pPad->xrdb.standAlone) {	/* standAlone takes precedence */
	pPad->xrdb.session = (char *) NULL;
	pPad->xrdb.server = False;
    } else if (pPad->xrdb.session) {	/* session is 2nd in precedence */
 	pPad->xrdb.server = False;
 	pPad->xrdb.blocking = False;
    }

    if (pPad->xrdb.standAlone) {

	/*--------------------------------------------------------
	 * StandAlone Mode
	 *
	 *	Initialize self as an editing session.
	 *
	 *-------------------------------------------------------*/
	InitDtEditSession(pPad, argc, argv);	/* do actual editing */

    } else if (pPad->xrdb.server) {
	/*--------------------------------------------------------
	 * Server Mode
	 *
	 *	Indicate the types of messages interested in and
 	 *	callbacks to process each message type, then
	 *	initialize self as an editing session.
	 *
	 *-------------------------------------------------------*/      
	InitDtEditSession(pPad, argc, argv); /*do actual editing*/
      
	/* -----> Create and register pattern to handle "DtPadSession_Run"
	 *        requests from a requestor dtpad */
	requests2Handle = tt_pattern_create();
	tt_pattern_category_set(requests2Handle, TT_HANDLE);
	tt_pattern_class_add(requests2Handle, TT_REQUEST);
	tt_pattern_scope_add(requests2Handle, TT_SESSION);
	sessId = tt_default_session();
	tt_pattern_session_add(requests2Handle, sessId);
	tt_free(sessId);
	tt_pattern_op_add(requests2Handle, "DtPadSession_Run");
	tt_pattern_callback_add(requests2Handle, HandleTtRequest);
	tt_pattern_register(requests2Handle);
      
    } else {

	/*--------------------------------------------------------
	 * Requestor Mode
	 *
	 *	Just tell server to do actual editing and wait
	 *	for DTPAD_DONE msg from server to exit (or exit
	 *	immediately if -noBlocking).
	 *
	 *-------------------------------------------------------*/

	/* Send message to dtpad server to either open-and-edit a
	 * file or to restart a session.  Also set up callbacks to
	 * handle success, failure and done-editing notification.
	 */
	SendMessageToServer(pPad, argc, argv);
    }

    XtAppMainLoop(pPad->app_context);
    exit(0);

}


/************************************************************************
 * CreateFirstPad - Performs initialization common to all modes of
 *	operation (i.e. to requestor mode, server mode and standAlone
 *	mode).
 *
 *   This routine initializes the primary sturcture used by dtpad, pPadList
 *   which is a list of Editor structs (pPad) with one entry per edit request.
 *
 *   Specifically, this routine:
 *
 *   1) creates the first entry in pPadList and returns a pointer to it
 *   2) initializes Xt and the display, and creates the top level widget
 *   3) adds the "save yourself" protocol to the top level widget
 *   4) creates the applic shell widget and applic context
 *   5) loads the application resources to the initial pPad and sets some
 *	initial editing process values
 *
 *   Creation of subsequent entries in the pPadList (1) and creation of
 *   corresponding applic shells and contexts (4) are also done by the server
 *   (via TTmedia_ptype_declareCB) for each new edit request that it receives.
 *   This means that there is one top level widget and display connection
 *   per process and one applic shell widget and applic context PER EDIT
 *   REQUEST (i.e. per entry in pPadList).
 *
 *   Note that CreateFirstPad is run in all modes the first time the requestor
 *   or server starts up and that resource are only loaded via this routine.
 *   This means that resources are obtained (via XtGetApplicationResources)
 *   on both the requestor and the server and saved in the initial pPad Editor
 *   on the requestor or server.  This also means that for requestor dtpads, a 
 *   pPad struct is created on both the requestor and the server (the server
 *   pPad may be the initial struct created by CreateFirstPad or it may be a
 *   subsequent pPad created by TTmedia_ptype_declareCB but with the initial
 *   server resource values).  For requestor mode, the dtpad pPad resources are
 *   shipped to the server (in the TTDT_EDIT request) where they override
 *   resources in the corresponding pPad struct on the server.
 *
 *   XXX - Some of the stuff in this routine probably does not need to be
 *	   done in all modes of operation - primarily in requestor mode.
 *	   Note that resources need to be loaded in all modes.
 *
 * [SuG 5/12/95] Modified for fishy argument checking.
 ************************************************************************/
static Editor * 
CreateFirstPad(
        int *pArgc,
	char **argv)
{
    Editor *pPad;
    Arg al[10];
    register int ac;
    char *tmpGeometry ;
    Widget foo;
	int i;
	/* Added for the argument fix*/
	char* inS;
	int in;
    Atom xa_WM_SAVE_YOURSELF;
	extern char *optarg;
	extern int optind, opterr, optind;
	char c;

    /* Create the 1st Editor entry in list of Editor instances.
     * This is also done in FindOrCreatePad() for each Editor pad instance */
    CreatePad(&pPad);

    if (pPad->progname=MbStrrchr(argv[0], '/'))
	pPad->progname++;
    else
	pPad->progname = argv[0];

    /* Initialize the toolkit, open the display and create top level widget */
    topLevelWithWmCommand = XtInitialize(argv[0], DTPAD_CLASS_NAME, 
				   option_list, XtNumber(option_list), 
			           (int *) pArgc, argv);

	/*
	/*[At this point Xt has stripped for us all valid arguments.* /
	/*[If number of arguments is <= 2, first one is program name and second
       one will be file name always (we don't care if it has a '-').* /
	if (*pArgc > 2) {
		/*[Now we check for illegal arguments, note argv[0] is progname, so
	   	   we skip that, argv[last] should be file name so we check for '-'
	       character in (i = 1 to i = (n-1)) to catch inbetween invalids.
		   Any extras with '-' as first characters are treated as illegal
		   options and ones without it as superfluous arguments.* /
		for (i = 1; i < ((*pArgc) - 1); i++) {
			inS = MbStrchri(argv[i], '-', &in);
			if (inS != NULL) {
				if (in == 0) {
					printf(GETMESSAGE(2, 2, "dtpad: illegal option: %s"),
					       argv[i]);
					printf("\n");
					exit(1);
				}
			}
			else {
				printf(GETMESSAGE(2, 3, "dtpad: extra argument: %s"),
				       argv[i]);
				printf("\n");
				exit(1);
			}
		}
	}
	*/

    /* At this point of time, declare an error if there is more than one argument
       or if it has only one argument (file name) that starts with '-' but not '--'.
	(Fix for bug 1204858)
    */

    if ((*pArgc > 2) || (*pArgc == 2 && argv[1][0] == '-' && argv[1][1] != '-')) {
        printf(GETMESSAGE(2,4,"Usage: dtpad [-options] [file]"));
        printf("\n");
        exit(1);
    }

    ac = 0;
    XtSetArg(al[ac], XmNgeometry, &tmpGeometry); ac++;
    XtGetValues(topLevelWithWmCommand, al, ac);

    pPad->geometry = XtNewString(tmpGeometry) ;/* Bug fix 4024518/Escalation 507988 */

    /* Realize the top level widget */
    XtSetMappedWhenManaged(topLevelWithWmCommand, False);
    XtRealizeWidget(topLevelWithWmCommand);

    /* Determine the display connection */
    if (!(pPad->display = XtDisplay(topLevelWithWmCommand))) {
        XtError((char *) GETMESSAGE(7, 5, 
          "Can not open display ... exiting Text Editor."));
    }

    /* TTT - ToolTalk Debug
     * XSynchronize(pPad->display, True);
     */

    /* Add the Save-Yourself protocol to the top level widget.
     * Application shell widgets (one per Editor request) don't need it,
     * since they don't have a WM_COMMAND property on them. */
    xa_WM_SAVE_YOURSELF = XInternAtom(pPad->display, 
                                             "WM_SAVE_YOURSELF", False);
    XmAddWMProtocolCallback(topLevelWithWmCommand, xa_WM_SAVE_YOURSELF, 
                           (XtCallbackProc)SaveSessionCB, (XtPointer)pPadList);

     
    /* Create the application shell widget and obtain the application context.
     * This is also done in FindOrCreatePad() for each Editor pad instance */
    ac = 0;
    pPad->app_shell = XtAppCreateShell(pPad->progname, DTPAD_CLASS_NAME,
                                           applicationShellWidgetClass,
                                           pPad->display, al, ac);
    pPad->app_context = XtWidgetToApplicationContext(pPad->app_shell);

    /* Get the application resources */
    XtGetApplicationResources(pPad->app_shell, &pPad->xrdb, resources,
                              XtNumber(resources), NULL, 0);
    if (pPad->xrdb.server) {
	initialServerResources = pPad->xrdb;
	initialServerResources.server = False; /* so window will be mapped */ 
	/* initialServerGeometry = pPad->geometry; */
	initialServerGeometry = strdup(pPad->geometry); /* fix for bug 1252959 */
    }

    /* Set some initial Editor instance values */
    if (pPad->xrdb.server == False) {
        pPad->inUse = True;
    }
    /* XXX - action procs and translations will be handled by the Editor widget
     * AddActionProcs(pPad);
     * CreateTranslationTable();
     */

    return pPad;
}


/************************************************************************
 * CreatePad - creates a new entry in the pPadList of Editor structs.
 *	Both the top of the list (the global, pPadList) and the current
 *	entry (whose address is passed as a parameter) are updated to
 *	point to the newly created entry.
 ************************************************************************/
/* ARGSUSED */
void 
CreatePad(
        Editor **ppPad)
{
    Editor *pPad;

    pPad = *ppPad = (Editor *) XtCalloc(1, sizeof(Editor));
    pPad->pPrevPad = (Editor *)NULL;

    if (pPadList == (Editor *)NULL) {
        pPadList = pPad;
        pPad->pNextPad = (Editor *)NULL;
    } else {
	Editor *pTemp = pPadList;
	pPadList = pPad;
	pPad->pNextPad = pTemp;
	pTemp->pPrevPad = pPad;
    }
}

/************************************************************************
 * SetupLoadArgs - pack requestor options into the Edit request
 ************************************************************************/
/* ARGSUSED */
void
SetupLoadArgs(Tt_message m, Editor *pPad)
{
    ApplicationDataPtr pArgs = &pPad->xrdb;
    
    /* the netfile name corresponding to the filename is in the */
    /* file arg of the message */
    if (pArgs->idString != (char *)NULL) {
      tt_message_context_set(m, "IDSTRING", pArgs->idString);
    }

    /* -----> Ignore all other resources if restoring a session.
     * XXX  - Currently, an Edit request is not used to restore a session.
     *        Instead a "private" ToolTalk request, DtPadSession_Run is
     *        used (which is sent in SendMessageToServer() and handled in
     *        HandleTtRequest() - the handler is setup in main()) */
    if (pPad->xrdb.session != (char *)NULL) {
      tt_message_context_set(m, "SESSION", pArgs->session);
      return;
    }

    /* -----> The following is the only resource passed to the text editor
     *        server which is not a text editor specific resource.
     *        It is always passed. */
    if (pPad->geometry != (char *)NULL) {
      tt_message_context_set(m, "GEOMETRY", pPad->geometry);
    }

    /* -----> Basic options */
    tt_message_context_set(m, "STATUSLINE", pArgs->statusLine ? "t" : "f");
    tt_message_context_set(m, "WINDOWWORDWRAP", pArgs->wordWrap ? "t" : "f");
    tt_message_context_set(m, "OVERSTRIKE", pArgs->overstrike ? "t" : "f");
    tt_message_context_set(m, "SAVEONCLOSE", pArgs->saveOnClose ? "t" : "f");
    tt_message_context_set(m, "MISSINGFILEWARNING", pArgs->missingFileWarning ? "t" : "f");
    tt_message_context_set(m, "NOREADONLYWARNING",
				pArgs->readOnlyWarning ? "t" : "f");
    tt_message_context_set(m, "NONAMECHANGE",
				pArgs->nameChange ? "t" : "f");
    tt_message_context_set(m, "VIEWONLY", pArgs->viewOnly ? "t" : "f");
    tt_message_context_set(m, "WORKSPACELIST", pArgs->workspaceList);

    /* -----> client/server control options */
    if (pArgs->blocking == True) {
	char *buf = (char *)XtMalloc(32);
	sprintf(buf, "%d", (int)getpid());
	tt_message_context_set(m, "BLOCKING", buf);
	pPad->blockChannel = buf;
    }

    /* -----> XXX - application server support options are no longer supported */
    if (pArgs->directory != (char *)NULL) {
	tt_message_context_set(m, "DIRECTORY", pArgs->directory);
    }
    if (pArgs->saveAsDir != (char *)NULL) {
	tt_message_context_set(m, "SAVEASDIR", pArgs->saveAsDir);
    }
    if (pArgs->mainTitle != (char *)NULL) {
	tt_message_context_set(m, "MAINTITLE", pArgs->mainTitle);
    }
    if (pArgs->nameInTitle != True) {
	tt_message_context_set(m, "NONAMEINTITLE", "t");
    }
    if (pArgs->exitLabel != (char *)NULL) {
	tt_message_context_set(m, "EXITLABEL", pArgs->exitLabel);
    }
    if (pArgs->exitMnemonic != (char *)NULL) {
	tt_message_context_set(m, "EXITMNEMONIC", pArgs->exitMnemonic);
    }
    if (pArgs->helpVol != (char *)NULL) {
	tt_message_context_set(m, "HELPVOL", pArgs->helpVol);
    }
    if (pArgs->menuBar != True) {
	tt_message_context_set(m, "NOMENUBAR", "t");
    }
    if (pArgs->confirmationString != (char *)NULL) {
	tt_message_context_set(m, "CONFIRMATIONSTRING", pArgs->confirmationString);
    }
    if (pArgs->allowExit != True) {
	tt_message_context_set(m, "NOEXIT", "t");
    }
    if (pArgs->closeImmediate == True) {
	tt_message_context_set(m, "CLOSEIMMEDIATE", "t");
    }
}


/************************************************************************
 * pLoadMsgCB - requestor side ttmedia_load() callback setup in
 *	SendMessageToServer() to handle Replies from the dtpad server.
 *
 *	Replies are sent by the server via the following functions:
 *	
 *	ttMediaLoadReply (in TTmediaReply).  TTmediaReply is executed when:
 *
 *	  1) an Edit or Display request is being processed and either the
 *	     edit window is closed or a new file is selected by the user
 *	  2) an Instantiate request is being processed and the edit window
 *	     is closed
 *	  3) a Quit request is handled which matches the Edit request
 *
 *	tttk_message_fail.  tttk_message_fail is normally called whenever
 *
 *	  1) the edit/display/instantiate request can't be handled
 *	  2) a buffer is being edited and the user closed the window
 *	     (XXX - actually, this should only be done if a buffer
 *		    is being edited and there are unsaved changes
 *		    - otherwise ttMediaLoadReply should be used)
 *	  3) a Quit request is handled for the window and there are
 *	     unsaved changed  
 *
 ************************************************************************/
Tt_message
pLoadMsgCB(Tt_message msg,
	   void *clientData,
	   Tttk_op op,
	   unsigned char *contents,
	   int len,
	   char *file)
{
    Tt_state state = tt_message_state(msg);
    StartupStruct *pStruct = (StartupStruct *)tt_message_user(msg, 0);

    if (state == TT_HANDLED) {
	tttk_message_destroy(msg);
	exit(0);
    } else if (state == TT_FAILED) {
	if (pStruct->pPad->xrdb.closeImmediate == True) {
	    exit(1);
	}
	pStruct->pPad->inUse = True;
	pStruct->pPad->xrdb.standAlone = True;
	InitDtEditSession(pStruct->pPad, pStruct->argc, pStruct->argv);
	return 0;
    } else {
	return 0;
    }
}


/************************************************************************
 * pStatusCB - requestor side ttdt_subcontract_manage() callback setup in
 *	SendMessageToServer() to handle Status messages from the dtpad
 *	server.
 *
 *	The server sends a Status message via ttdt_message_accept() when
 *	it determines that it can actually handle the EDIT request.
 *
 ************************************************************************/
Tt_message
pStatusCB(Tt_message msg,
	  void *clientdata,
	  Tt_message contract)
{
    if (clientdata) {
	/* This is the blocking case, just return 0 (we'll exit in
	 * pLoadMsgCB when we get a Reply from the dtpad server when it's
	 * done handling the request).*/
	return 0;
    } else {
	/* This is the non-blocking case, exit if this is a Status notice */
	if (tttk_string_op(tt_message_op(msg)) == TTDT_STATUS) {
	    exit(0);
	} else {
	    return 0;
	}
    }
}
	  

/************************************************************************
 * SendMessageToServer - requestor mode routine to setup callbacks and send
 *	a message to the dtpad "server" to do one of the following:
 *
 *   case 1) restore dtpad session & exit dtpad requestor immediately
 *   case 2) edit a file & exit dtpad requestor immediately
 *   case 3) edit a file & and wait till editing is done before exiting
 *	     requestor dtpad
 *
 ************************************************************************/
static void 
SendMessageToServer(
	Editor *pPad,
	int argc,
	char **argv)
{
    StartupStruct *pStruct = (StartupStruct *)XtMalloc(sizeof(StartupStruct));
    char *fileName, *pArgs;
    Tt_message msg;
    Tttk_op op;

    if (pPad->xrdb.session != (char *)NULL) {
	fileName = pPad->xrdb.session;
	pPad->saveRestore = True;
    } else if (argc > 1) {
	fileName = argv[argc - 1];
    } else {
	fileName = "*";
    }

    /* -----> Obtain the netfile name of the file to be edited.  If the
     *        directory can't be accessed via the requestor's host, don't
     *        use the server since we can't pass it a netfile name and we
     *        can't assume the filename is appropriate on the server's host.
     */
    pPad->fileStuff.netfile = tt_file_netfile(fileName);
    if (tt_ptr_error(pPad->fileStuff.netfile) != TT_OK) {
	tt_free(pPad->fileStuff.netfile);
	pPad->fileStuff.netfile = (char *) NULL;
	if (pPad->xrdb.closeImmediate == True)
	    exit(1);
	pPad->xrdb.standAlone = True;
	InitDtEditSession(pPad, argc, argv);	/* start standAlone  */
	return;
    }

    /*
     * Keep from saving our session if we've sent a message to the server.
     * We'll restore this to true iff the server sends a failure notification.
     */
    pPad->inUse = False;

    pStruct->pPad = pPad;
    pStruct->argc = argc;
    pStruct->argv = argv;

    TTstartDesktopMediaExchange(topLevelWithWmCommand, pPad);

    if (pPad->xrdb.session != NULL) {

	/*--------------------------------------------------------
	 * case 1 (restore session) - exit if successful.
	 *-------------------------------------------------------*/

	msg = tttk_message_create(
		0,				/* no inherited context */
		TT_REQUEST,			/* class */
		TT_SESSION,			/* scope */
		0,				/* TT_PROCEDURE addr */
		"DtPadSession_Run",		/* op */
		(Tt_message_callback) RequestCB); /* msg callback */
	tt_message_file_set(msg, fileName);
	tt_message_user_set(msg, 0, pStruct);
	tt_message_send(msg);

    } else {

	/*--------------------------------------------------------
	 * case 2 (noBlocking) - exit if the request is successfully
	 *		handled by the dtpad server.
	 * case 3 (Blocking) - do nothing if the request is successfully
	 *		handled, then wait for a Reply from the dtpad
	 *		server to exit.
	 *
	 * Build and send either a TTME_EDIT request if a file is specified
	 * or TTME_INSTANTIATE if no file is specified.  These requests
	 * are handled via TTmedia_ptype_declareCB() on the dtpad server.
	 *
	 * 1. If server can not handle the request, it will fail the request
	 *    immediately.  pLoadMsgCB will receive the reply message and
	 *    start a standAlone dtpad.
	 * 2. If server can handle the request, it will send a Status notice.
	 *    pStatusCB will receive the notice and either exit (noBlocking)
	 *    or do nothing (Blocking).  The server will then reply to a
	 *    TTME_EDIT request when editing for the specified file/buffer
	 *    is done (the edit window may still remain open though) and
	 *    to a TTME_INSTANTIATE request when the edit window is closed.
	 *-------------------------------------------------------*/

	if (strcmp(fileName, "*") == 0) {
	    op = TTME_INSTANTIATE;
	} else {
	    op = TTME_EDIT;
	}

	msg = ttmedia_load(0,
			pLoadMsgCB,
			NULL,
			op,
			"C_STRING",
			NULL,
			0,
			fileName,
			NULL,
			0);

	ttdt_subcontract_manage(msg,
			pStatusCB,
			NULL,
			(void *) pPad->xrdb.blocking);	/* client data */

	/* Add pArgs to msg using tt_message_context_set */
	SetupLoadArgs(msg, pPad);
	tt_message_user_set(msg, 0, pStruct);
	tt_message_send(msg);

    }
}


/************************************************************************
 * InitDtEditSession - sets up ToolTalk notifications and callbacks related
 *	to starting the editing session then calls InitEditSessionAnd-
 *	FirstPad.
 *
 *	(was StartServerOrStandAlone)		
 ***********************************************************************/
void 
InitDtEditSession(
	Editor *pPad,
	int argc,
	char **argv)
{
    InitEditSessionAndFirstPad(pPad, argc, argv);

    /* -----> Initialize TT connection & process start message */
    if (pPad->xrdb.server == True) {
      if (TTstartDesktopMediaExchange(topLevelWithWmCommand, pPad))
	tttk_Xt_input_handler(0, 0, 0); /* process TT msg that started us */
    }

}


/************************************************************************
 * InitEditSessionAndFirstPad - performs initialization related to
 *	starting the edit session and then calls the routine that creates
 *	the first pPad Editor.
 *
 * XXX - This routine can be incorporated into InitDtEditSession since it
 *	 will no longer be called directly from main() if DtInitialize fails.
 *
 * (was InitGlobalAndFirstPad)		
 ************************************************************************/
/* ARGSUSED */
static void 
InitEditSessionAndFirstPad(
        Editor *pPad,
	int argc,
	char **argv)
{
    char *pTmp = (char *)NULL;

    CreateDefaultImage (warningBits, 32, 32, "warn_image");

    xa_WM_DELETE_WINDOW = XInternAtom(pPad->display, 
                                             "WM_DELETE_WINDOW", False);

    /* get the users group and user id for file type checking */
    pPad->group_id = getgid();
    pPad->user_id = getuid();

    /* Set up file name */
    if (pPad->saveRestore == True && argv[3] != NULL) {
	pTmp = argv[3];
    } else if (argc > 1) {
	pTmp = argv[argc - 1];
    }

    if (pTmp != (char *)NULL) {
        pPad->fileStuff.fileName = (char *) XtMalloc(strlen(pTmp) + 1);
        strcpy(pPad->fileStuff.fileName, pTmp);
    } else {
	pPad->fileStuff.fileName = (char *)NULL;
    }
    RealizeNewPad(pPad);	/* display first pPad Editor window */

    setPanicConditions();	/* Set up emergency calls to PanicSave */

    /*
     * Set up a handler for SIGCLD so that the command invoker library
     * doesn't leave zombies.
     */
    {
        struct sigaction act;
        sigfillset(&act.sa_mask);
        act.sa_flags = 0;
        act.sa_handler = SigcldHndlr;
        sigaction(SIGCLD, &act, (struct sigaction *)NULL);
    }
}


/************************************************************************
 * RealizeNewPad - creates the GUI components of a pPad Editor, sets
 *	pPad resources, maps the window and loads the file to be edited.
 *
 *  NOTE: This routine in normally called only when a new edit window is to
 *	  be displayed.  However, it is also called when the server is first
 *	  started (when no edit requests have been received).  In this
 *	  situation, the window is not mapped and the file is not loaded.
 *	  These actions are done when the dummy pPad is "reused" when the
 *	  actual edit request is received from the requestor dtpad.
 *
 ************************************************************************/
void
RealizeNewPad(
        Editor *pPad)
{
    Arg al[10];			/* arg list */
    register int ac;		/* arg count */

    /* Create the title */
    SetDialogTitleString(pPad);

    /* Set some applic shell resources */
    ac = 0;
    XtSetArg(al[ac], XmNuseAsyncGeometry, True); ac++;
    XtSetArg(al[ac], XmNdeleteResponse, XmDO_NOTHING); ac++;
    XtSetArg(al[ac], XmNgeometry, NULL); ac++;
    XtSetValues(pPad->app_shell, al, ac);

    if (pPad->xrdb.session != NULL) {
       restoreSession(pPad);
    }
    pPad->nodo = False;

    /* Create Main Window */
    ac = 0;
    /* XXX
     * XtSetArg (al[ac], XmNwidth, pPad->width);  ac++;
     * XtSetArg (al[ac], XmNheight, pPad->height);  ac++;
     * XtSetArg (al[ac], XmNshadowThickness, 0);  ac++;
     * XtSetArg (al[ac], XmNcommandWindowLocation, XmCOMMAND_BELOW_WORKSPACE);ac++;
     */
    pPad->mainWindow = XmCreateMainWindow (pPad->app_shell, "main", al, ac);
    XtManageChild (pPad->mainWindow);

    /* Create MenuBar in Main Window */
    CreateMenuBar(pPad);
    if (pPad->xrdb.menuBar == True) {
        XtManageChild (pPad->menu_bar);
    }

    /* Create the DT Editor Widget in the Main Window. */
    CreateEditorWidget (pPad);
    XtManageChild (pPad->editor);

    /* Associate menu bar and editor widgets created above with menu bar and
     * work area of the main window (the main window command line area and
     * the horizontal/vertical scroll bars are not used) */
    XmMainWindowSetAreas(pPad->mainWindow, pPad->menu_bar,
    		(Widget) NULL, (Widget) NULL, (Widget) NULL,
		pPad->editor);

    /* Realize toplevel widgets. */
    XtSetMappedWhenManaged(pPad->app_shell, False);
    XtRealizeWidget (pPad->app_shell);

    /* Set the window title */
    ChangeMainWindowTitle(pPad);

    XSync(pPad->display, 0);

    /* setup where a close by the window manager will be handled */
    XmAddWMProtocolCallback(pPad->app_shell, xa_WM_DELETE_WINDOW,
                            (XtCallbackProc) closeCB, (XtPointer)pPad);

    /* reset the "save with newLines" flag */
    pPad->fileStuff.saveWithNewlines = True;

    /* XXX - don't allow menu bar height to be changed
     * XtSetArg(al[0], XmNresizeHeight, False);
     * XtSetValues(pPad->menu_bar, al, 1);
     */

    /* set app shell geometry (pixels), resize hints, position & size hints */
    SetWindowSize(pPad);

    /* set the state of the pPad based on its resources */
    SetStateFromResources(pPad);
    
    /* set File menu SaveAs label/mnemonic based on xrdb.nameChange */
    SetSaveAsLabelAndDialog(pPad);

    /* If we're the server initially starting up, this is a "dummy" edit
     * request and we really don't want to map anything to the display yet
     * (this will be done when the actual edit request comes in and this
     * dummy pPad is reused and mapped). */
    if (pPad->xrdb.server == False) {
        XtSetMappedWhenManaged(pPad->app_shell, True);
	XtPopup(pPad->app_shell, XtGrabNone);
	XtMapWidget(pPad->app_shell);
        numActivePads++;
    }

    /* load the file (if there's actually a file name specified) */
    LoadFile(pPad, NULL);

}


/************************************************************************
 * ManageOldPad - opens a new file in an exiting Editor pad "window".
 * 
 *	Existing windows have their widget structure already created but
 *	may or may not be managed/visible.   The existing window could be
 *	any of the following:
 *
 *	  o the initial "dummy" window created when the server was started
 *	    up (this window is not managed until the actual edit message
 *	    from the requestor is received)
 *	  o a window which has been closed previously
 *	  o a window that is currently managed/visible and is to be reused
 *	    (and is identified by the window id resource)
 ************************************************************************/
void
ManageOldPad(
        Editor *pPad,
	Boolean isCurrentlyVisible)
{
    if(pPad->xrdb.session != NULL)
       restoreSession(pPad);

    /* set app shell geometry (pixels), resize hints, position & size hints */
    SetWindowSize(pPad);

    /*.. create the title */
    ChangeMainWindowTitle(pPad);
    SetStateFromResources(pPad);  /* set the pPad state based on resources */
    pPad->nodo = False;
    
    /* set File menu SaveAs label/mnemonic based on xrdb.nameChange */
    SetSaveAsLabelAndDialog(pPad);

    /* If the window is iconified and autoRaise is disabled, then
     * we don't what to deiconify the window.
     */
    if (!isCurrentlyVisible) {
       XtSetMappedWhenManaged(pPad->app_shell, True);
       XtPopup(pPad->app_shell, XtGrabNone);
       XtMapWidget(pPad->app_shell);
    }
    if (pPad->xrdb.idString != (char *) NULL) {
	XRaiseWindow(pPad->display, XtWindow(pPad->app_shell));
    }

    LoadFile(pPad, NULL);

}


/************************************************************************
 * StartDbUpdate - 
 ************************************************************************/
/* ARGSUSED */
void
StartDbUpdate(
        XtPointer clientData)
{

   /* Read in the new databases */
   DtDbLoad();
   ActionDBInitialized = True;
}


/************************************************************************
 * exitCB - 
 ************************************************************************/
/* ARGSUSED */
void
exitCB(
        DtSvcHandle service,
        DtSvcMsgContext replyContext,
        XtPointer clientData,
        String * fields,
        int numFields)
{
  exit(0);
}


/************************************************************************
 * setPanicConditions - 
 ************************************************************************/
void
setPanicConditions(void)
{
    struct sigaction act;

    sigfillset(&act.sa_mask);
    /*
    signal(SIGHUP, (void (*)(int))PanicSave);
    signal(SIGINT, (void (*)(int))PanicSave);
    signal(SIGQUIT, (void (*)(int))PanicSave);
    signal(SIGILL, (void (*)(int))PanicSave);
    signal(SIGABRT, (void (*)(int))PanicSave);
    signal(SIGIOT, (void (*)(int))PanicSave);
    signal(SIGEMT, (void (*)(int))PanicSave);
    signal(SIGFPE, (void (*)(int))PanicSave);
    signal(SIGBUS, (void (*)(int))PanicSave);
    signal(SIGSEGV, (void (*)(int))PanicSave);
    signal(SIGSYS, (void (*)(int))PanicSave);
    signal(SIGPIPE, (void (*)(int))PanicSave);
    signal(SIGTERM, (void (*)(int))PanicSave);
    */
    sigfillset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = PanicSave;
    sigaction(SIGHUP, &act, (struct sigaction *)NULL);
    sigaction(SIGINT, &act, (struct sigaction *)NULL);
    sigaction(SIGQUIT, &act, (struct sigaction *)NULL);
    sigaction(SIGILL, &act, (struct sigaction *)NULL);
    sigaction(SIGABRT, &act, (struct sigaction *)NULL);
    sigaction(SIGIOT, &act, (struct sigaction *)NULL);
    sigaction(SIGEMT, &act, (struct sigaction *)NULL);
    sigaction(SIGFPE, &act, (struct sigaction *)NULL);
    sigaction(SIGBUS, &act, (struct sigaction *)NULL);
    sigaction(SIGSEGV, &act, (struct sigaction *)NULL);
    sigaction(SIGSYS, &act, (struct sigaction *)NULL);
    sigaction(SIGPIPE, &act, (struct sigaction *)NULL);
    sigaction(SIGTERM, &act, (struct sigaction *)NULL);

    /* TTT - comment out for debugging TookTalk code */
    XSetErrorHandler((int (*)()) PanicSave);
    XSetIOErrorHandler((int (*)())PanicSave);
}


/************************************************************************
 * SetDialogTitleString - 
 ************************************************************************/
static void
SetDialogTitleString(
        Editor *pPad)
{
    /*.. create the title */
    if (pPad->xrdb.mainTitle != (String) NULL)  {
	if (pPad->xrdb.nameInTitle == True) {
	    pPad->dialogTitle = (char *)XtMalloc(strlen(pPad->xrdb.mainTitle)
						 + 4);
	    sprintf(pPad->dialogTitle, "%s - ", pPad->xrdb.mainTitle);
	} else {
	    pPad->dialogTitle = (char *)XtMalloc(strlen(pPad->xrdb.mainTitle)
				                 + 1); 
	    sprintf(pPad->dialogTitle, "%s", pPad->xrdb.mainTitle);
	}
    } else {
#define MAIN_TITLE ((char *)GETMESSAGE(7, 2, "Text Editor - "))
	pPad->dialogTitle = strdup(MAIN_TITLE);
    }
}


/************************************************************************
 * GetAdjustedResizeHints - returns the size hints for the pPad Dt Editor
 *	widget with its min_height incremented by the height of dtpad's
 *	menu bar.
 ************************************************************************/
static void
GetAdjustedResizeHints(
	Editor *pPad,
        XSizeHints *pHints)
{
    Arg al[2];			/* arg list */
    register int ac;		/* arg count */
    Dimension MBheight;

    /* get Dt Editor widget size hints */
    DtEditorGetSizeHints(pPad->editor, pHints);
    
    /* get the menu bar height */
    if (pPad->xrdb.menuBar == True) {
	ac = 0;
        XtSetArg(al[0], XmNheight, &MBheight); ac++;
        XtGetValues(pPad->menu_bar, al, ac);
    } else {
	MBheight = 0;
    }
    /* adjust the editor widget min height by menu bar height */
    pHints->min_height += MBheight;
    pHints->base_height += MBheight;

}


/************************************************************************
 * SetAppShellResizeHints - sets the app_shell's normal resize hints
 *	(resize increment, minimum window size, etc.) based on the resize
 *	hints returned from the Dt Editor widget but with the dtpad menu
 *	bar height added to the min_height.
 ************************************************************************/
void
SetAppShellResizeHints(
        Editor *pPad)
{
    XSizeHints  size_hints;
    long supplied_return;

    /* make sure stuff not set by GetAdjustedResizeHints is not lost */
    XGetWMSizeHints(pPad->display, XtWindow(pPad->app_shell), &size_hints,
                                          &supplied_return, XA_WM_NORMAL_HINTS);

    GetAdjustedResizeHints(pPad, &size_hints);	/* consider the menu bar */

    XSetWMSizeHints(pPad->display, XtWindow(pPad->app_shell), &size_hints,
                    XA_WM_NORMAL_HINTS);
}


/************************************************************************
 * SetPadGeometry - sets the pad's width, height and x & y IN PIXELS
 *	based on the pad's geometry string.  Returns a flag
 *	indicating which geometry components were specified.
 ************************************************************************/
static int
SetPadGeometry(
        Editor *pPad)
{
    int                  geometryBits;
    int                  geometryX;
    int                  geometryY;
    unsigned int         geometryWidth;
    unsigned int         geometryHeight;
    XSizeHints  size_hints;


    /* -----> convert geometry string to integers */
    geometryBits = XParseGeometry(pPad->geometry,   /* geometry string  */
                                  &geometryX,       /* x position return*/
                                  &geometryY,       /* y position return*/
                                  &geometryWidth,   /* width return     */
                                  &geometryHeight); /* height return    */

    if (!(geometryBits & WidthValue))
	geometryWidth  = DEFAULT_COLS;
    if (!(geometryBits & HeightValue))
	geometryHeight = DEFAULT_ROWS;

    /* -----> Get width/height increments and minimum sizes in pixels
     *        and then compute window width/height in pixels. 
     *        The min_width & min_height size hints include a character. */
    GetAdjustedResizeHints(pPad, &size_hints);	/* consider the menu bar */
    geometryWidth = geometryWidth * size_hints.width_inc +
			size_hints.min_width - size_hints.width_inc;
    geometryHeight = geometryHeight * size_hints.height_inc +
			size_hints.min_height - size_hints.height_inc;

     /* -----> allow for negative X and Y values */
    if (geometryBits & XValue) {
	if (geometryBits & XNegative) {
            geometryX += DisplayWidth(pPad->display,
                                      DefaultScreen(pPad->display)) -
                                      geometryWidth -
                                      2 * pPad->app_shell->core.border_width;
	}
    } else {
	geometryX = 0;
    }
    if (geometryBits & YValue) {
	if (geometryBits & YNegative) {
            geometryY += DisplayHeight(pPad->display,
                                       DefaultScreen(pPad->display)) -
                                       geometryHeight -
                                       2 * pPad->app_shell->core.border_width;
	}
    } else {
	geometryY = 0;
    }

    /* -----> set pad's pixel geometry */
    pPad->width = geometryWidth;
    pPad->height = geometryHeight;
    pPad->x =  geometryX;
    pPad->y = geometryY;

    return geometryBits;
}


/************************************************************************
 * SetWindowSize - sets the following for the pad's application shell:
 *
 *	1) geometry (in pixels) - after converting the geometry string
 *			to pixel values and storing in the pad struct
 *	2) resize hints (PMinSize, PResizeInc and PBaseSize)
 *	3) position and size hints (PPosition and PSize)
 *
 ************************************************************************/
void
SetWindowSize(
        Editor *pPad)
{
    Arg al[10];			/* arg list */
    register int ac;		/* arg count */
    XSizeHints  size_hints;
    XWMHints   *wmhints;
    long supplied_return;
    int geometryBits;
    XWindowChanges changesStruct;

    /* -----> set pad's app_shell geometry (in pixels) */
    ac = 0;
    if (pPad->saveRestore == True) {
	/* if we're restoring a session, the geometry is already in
	 * pixel values so set the app_shell geometry directly */
        XtSetArg (al[ac], XmNx, pPad->x);  ac++;
        XtSetArg (al[ac], XmNy, pPad->y);  ac++;
        XtSetArg (al[ac], XmNwidth, pPad->width);  ac++;
        XtSetArg (al[ac], XmNheight, pPad->height);  ac++;
        XtSetValues(pPad->app_shell, al, ac);
    }  else {
	/* parse the pPad->geometry STRING into 
	 * pPad->geometry[Width|Height|X|Y] PIXELS values and
	 * return flags indicating which geometry parts were specified */
	geometryBits = SetPadGeometry(pPad); /* parse geometry str to pixels */
	/* set only the geometry parts that were specified */
        if (geometryBits & XValue) {
	    XtSetArg (al[ac], XmNx, pPad->x);  ac++; 
	}
        if (geometryBits & YValue) {
	    XtSetArg (al[ac], XmNy, pPad->y);  ac++; 
	}
        if (geometryBits & WidthValue) {
	    XtSetArg (al[ac], XmNwidth, pPad->width);  ac++;
	}
        if (geometryBits & HeightValue) {
            XtSetArg (al[ac], XmNheight, pPad->height);  ac++;
	}
	if (ac > 0) {
            XtSetValues(pPad->app_shell, al, ac);
        }
    }

    /* -----> set app_shell resize hints based on resize hints returned
     *        from the Dt Editor widget (PMinSize, PResizeInc and PBaseSize)
     *        and adjusted by the dtpad menu bar */ 
    SetAppShellResizeHints(pPad);

    /* -----> set app_shell position and size hints
     *        (and whether user specified (US) or program specified (P) */
    XGetWMSizeHints(pPad->display, XtWindow(pPad->app_shell), &size_hints,
                                          &supplied_return, XA_WM_NORMAL_HINTS);
    size_hints.flags = PMinSize|PResizeInc|PBaseSize;  /* set previously */
    size_hints.x = pPad->x;
    size_hints.y = pPad->y;
    size_hints.width = pPad->width;
    size_hints.height = pPad->height;
    if (pPad->saveRestore == True) {
        size_hints.flags |= USSize|USPosition;
    } else {
        if (geometryBits & XValue && geometryBits & YValue) {
            size_hints.flags |= USPosition;
	} else {
            size_hints.flags |= PPosition;
        }
        if (geometryBits & WidthValue && geometryBits & HeightValue) {
            size_hints.flags |= USSize;
	} else {
            size_hints.flags |= PSize;
        }
    }
    XSetWMSizeHints(pPad->display, XtWindow(pPad->app_shell), &size_hints,
                    XA_WM_NORMAL_HINTS);

   /* set iconic state */
   if (pPad->iconic)
   {
     /* add the iconify hint to the current shell */
     XtSetArg(al[ac], XmNinitialState, IconicState); ac++;

     if (pPad->iconicX)
       { XtSetArg(al[ac], XmNiconX, pPad->iconX); ac++; }

     if (pPad->iconicY)
       { XtSetArg(al[ac], XmNiconY, pPad->iconY); ac++; }

     XtSetValues(pPad->app_shell, al, ac);
   }
   else
   {
     /* Remove the iconify hint from the current shell */
     wmhints = XGetWMHints(XtDisplay(pPad->app_shell), 
                           XtWindow(pPad->app_shell));
     wmhints->flags |= IconWindowHint;
     wmhints->initial_state = NormalState;
     XSetWMHints(XtDisplay(pPad->app_shell), 
                 XtWindow(pPad->app_shell), wmhints);
   }

    changesStruct.x = pPad->x;
    changesStruct.y = pPad->y;
    changesStruct.width = pPad->width;
    changesStruct.height = pPad->height;
    XConfigureWindow(pPad->display, XtWindow(pPad->app_shell), 
		     CWWidth | CWHeight, &changesStruct);
}


/************************************************************************
 * SetInitialMenuSensitivity - sets the initial sensitivity of various menu
 	items primarily based on whether or not we're in viewOnly mode.
 ************************************************************************/
static void
SetInitialMenuSensitivity(
        Editor *pPad)
{
    Boolean editable = ! pPad->xrdb.viewOnly;

    /* -----> set sensitivity of File menu items */
    XtSetSensitive(pPad->fileStuff.fileWidgets.newBtn, editable);

    /* The following commented line was uncommented between 1.39 & 1.40
    without any comments. We need to comment it out to fix 1203626
    (-viewOnly option should not disable 'Open' menu)
    */
	/****Begin Change - SuG 5/5/95 
    XtSetSensitive(pPad->fileStuff.fileWidgets.openBtn, editable);
	*/
    XtSetSensitive(pPad->fileStuff.fileWidgets.includeBtn, editable);
    XtSetSensitive(pPad->fileStuff.fileWidgets.saveBtn, editable);

    /* -----> set sensitivity of Edit menu items
     * Edit Menu items that deal with the current selection (Cut, Copy,
     * Clear and Delete) are made sensitive/insensitive via the DtEditor
     * widget's DtNtextSelectCallback and DtNtextDeselectCallback.
     * Currently nothing is being done with Paste based on text
     * selected/deselected outside the edit window.
     * findChangeBtn is left sensitive since the DtEditor widget will
     * enable/disable changes in the Find/Change dialog based on the
     * DtNeditable resource. */
    XtSetSensitive(pPad->editStuff.widgets.undoBtn, editable);
    XtSetSensitive(pPad->editStuff.widgets.pasteBtn, editable);
    if (pPad->editStuff.widgets.checkSpellingBtn != (Widget) NULL) {
	XtSetSensitive(pPad->editStuff.widgets.checkSpellingBtn, editable);
    }
    SetSelectionMenuItems(pPad, False);	/* set Cut, Copy, Clear & Delete */

     /* -----> set sensitivity of Format menu */
    XtSetSensitive(pPad->formatStuff.widgets.formatBtn, editable);

    /* -----> set sensitivity of Options menu items */
    XtSetSensitive(pPad->optionsStuff.widgets.overstrikeBtn, editable);
    XtSetSensitive(pPad->optionsStuff.widgets.wordWrapBtn, editable);
    
}


/************************************************************************
 * SetExitButtons - 
 ************************************************************************/
static void
SetExitButtons(
        Editor *pPad)
{
    Arg al[5];
    int ac, flags;
    
    if (pPad->xrdb.allowExit != True) {
	flags = MWM_FUNC_RESIZE | MWM_FUNC_MOVE | MWM_FUNC_MINIMIZE |
		MWM_FUNC_MAXIMIZE;
	XtUnmanageChild(pPad->fileStuff.fileWidgets.exitBtn);
    } else {
	flags = MWM_FUNC_ALL;
    	XtManageChild(pPad->fileStuff.fileWidgets.exitBtn);
    }
    ac = 0;
    XtSetArg(al[ac], XmNmwmFunctions, flags); ac ++;
    XtSetValues(pPad->editor, al, ac);
}


/************************************************************************
 * SetStateFromResources - set pPad's state (e.g. GUI components) based on
 *	current pPad->xrdb resource values.  The resources are initially
 *	obtained from either:
 *
 *	1) the dtpad requestor's resource data base and command line
 *	   options - if running in standAlone mode or in server mode and
 *	   handling an Edit request containing resource context fields
 *	2) the server's data base - if running in server mode and
 *	   handling an Edit request without resource context fields
 *
 ************************************************************************/
static void
SetStateFromResources(
        Editor *pPad)
{
    Arg al[1];			/* arg list */
    OptionsMenuWidgets *pWidg = &pPad->optionsStuff.widgets;
    
    SetWorkSpaceHints(pPad->app_shell, pPad->xrdb.workspaceList);
    SetDialogTitleString(pPad);

    /* -----> set window title */
    if (pPad->xrdb.mainTitle != (char *)NULL) {
	SetWindowTitles(pPad);
    }

    /* -----> Set the overstrike, wordWrap and statusLine for the Editor
     *        widget and corresponding Options menu toggle buttons.
     *        This is done in one place - in the valueChanged callback for
     *        each option - which is called only if parameter 3 is True AND
     *        the state being set is different than what it currently is.
     *        The first XmToggleButtonSetState (with parm 3 = False) ensures
     *        that the state is opposite of what we want it to be so that
     *        when we set it, the valueChanged callback will be called. */

    if (pPad->xrdb.overstrike == XmToggleButtonGetState(pWidg->overstrikeBtn))
	XmToggleButtonSetState(pWidg->overstrikeBtn,
				!pPad->xrdb.overstrike, False);
    XmToggleButtonSetState(pWidg->overstrikeBtn,
				pPad->xrdb.overstrike, True);

    if (pPad->xrdb.wordWrap == XmToggleButtonGetState(pWidg->wordWrapBtn))
	XmToggleButtonSetState(pWidg->wordWrapBtn,
				!pPad->xrdb.wordWrap, False);
    XmToggleButtonSetState(pWidg->wordWrapBtn,
				pPad->xrdb.wordWrap, True);

    if (pPad->xrdb.statusLine == XmToggleButtonGetState(pWidg->statusLineBtn))
	XmToggleButtonSetState(pWidg->statusLineBtn,
				!pPad->xrdb.statusLine, False);
    XmToggleButtonSetState(pWidg->statusLineBtn,
				pPad->xrdb.statusLine, True);

    /* -----> Set whether text can be edited */
    XtSetArg(al[0], DtNeditable, !pPad->xrdb.viewOnly);
    XtSetValues(pPad->editor, al, 1);
    SetInitialMenuSensitivity(pPad);

}


/************************************************************************
 * SigcldHndlr - 
 ************************************************************************/
/* ARGSUSED */
void SigcldHndlr (int dummy)
{
    int   status, w;
    struct sigaction act;
    sigfillset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = SigcldHndlr;

    w = wait (&status);

    /*
     * Is this really necessary?
     */
    sigaction(SIGCLD, &act, (struct sigaction *)NULL);
}


/************************************************************************
 * UnmanageAllDialogs - unmanages all the dialogs which may be managed
 *	when a pad gets closed or withdrawn.
 ************************************************************************/
void
UnmanageAllDialogs(
    Editor *pPad)
{
    HelpStruct *pHelp;
    /*
     * Unmanage all Modal dialogs that might possibly be up.
     */
    if (pPad->fileStuff.fileWidgets.saveAs.alrdy_exist != (Widget)NULL &&
	pPad->fileStuff.fileWidgets.saveAs.alrdy_exist->core.managed == True)
    {
        XtUnmanageChild(pPad->fileStuff.fileWidgets.saveAs.alrdy_exist);
    }

    if (pPad->fileStuff.fileWidgets.select.save_warning != (Widget)NULL &&
	pPad->fileStuff.fileWidgets.select.save_warning->core.managed == True)
    {
        XtUnmanageChild(pPad->fileStuff.fileWidgets.select.save_warning);
    }

    if (pPad->fileStuff.fileWidgets.gen_warning != (Widget)NULL &&
	pPad->fileStuff.fileWidgets.gen_warning->core.managed == True)
    {
        XtUnmanageChild(pPad->fileStuff.fileWidgets.gen_warning);
    }
    
    /*
     * Unmanage File menu dialogs that might be posted.
     * The Edit and Format menu dialogs are managed by the DtEditor widget.
     * XXX - unmanage print dialog too.
     */
    if (pPad->fileStuff.fileWidgets.select.file_dialog != (Widget)NULL &&
	pPad->fileStuff.fileWidgets.select.file_dialog->core.managed == True)
    {
        XtUnmanageChild(pPad->fileStuff.fileWidgets.select.file_dialog);
    }

    if (pPad->fileStuff.fileWidgets.saveAs.saveAs_form != (Widget)NULL &&
	pPad->fileStuff.fileWidgets.saveAs.saveAs_form->core.managed == True)
    {
        XtUnmanageChild(pPad->fileStuff.fileWidgets.saveAs.saveAs_form);
    }

    /*
     * Unmanage help if it's up.
     */
    if (pPad->MainHelp != (Widget)NULL && pPad->MainHelp->core.managed == True)
    {
        XtUnmanageChild(pPad->MainHelp);
    }
    for (pHelp = pPad->pHelpCache; pHelp != (HelpStruct *)NULL;
	 pHelp = pHelp->pNext)
    {
	if (pHelp->dialog != (Widget)NULL &&
	    pHelp->dialog->core.managed == True)
	{
            XtUnmanageChild(pHelp->dialog);
	}
    }
}


/************************************************************************
 * RestoreInitialServerResources - 
 ************************************************************************/
void
RestoreInitialServerResources(
        Editor *pPad)
{
    /* -----> free resource strings */
    /* Bug fix 4024518/ Escalation 507988 */

    if (pPad->xrdb.workspaceList != (char *)NULL) {
	XtFree(pPad->xrdb.workspaceList);
	pPad->xrdb.workspaceList = (char *)NULL;
    }
    if (pPad->xrdb.session != (char *)NULL) {
	XtFree(pPad->xrdb.session);
	pPad->xrdb.session = (char *)NULL;
    }
    if (pPad->geometry != (char *)NULL) {
	XtFree(pPad->geometry );
	pPad->geometry = (char *)NULL;
    }

    pPad->xrdb = initialServerResources;
    /* pPad->geometry = initialServerGeometry; */

    /* -----> duplicate resource strings */
    if (initialServerResources.workspaceList != (char *)NULL)
	pPad->xrdb.workspaceList = strdup(initialServerResources.workspaceList);
    if (initialServerResources.session != (char *)NULL)
	pPad->xrdb.session = strdup(initialServerResources.session);
    if (initialServerGeometry != (char *)NULL)
	pPad->geometry = XtNewString(initialServerGeometry);

}


#ifdef MESSAGE_CAT
/*****************************************************************************
 *  _DtpadGetMessage - obtains a localized message from dtpad's message catalog
 *
 * Parameters:
 *
 *   int	set -		The message catalog set number.
 *   int	number -	The message number.
 *   char	*string -	The default message if the message is not
 * 				retrieved from a message catalog.
 *
 * Returns: the message string for set 'set' and number 'number'.
 *
 *****************************************************************************/
char *
_DtpadGetMessage(
    int set,
    int number,
    char *string )
{
    char *msg;
    nl_catd catopen();
    char *catgets();
    static int first = 1;
    static nl_catd nlmsg_fd;

    if ( first ) {
	first = 0;
	nlmsg_fd = catopen(_DTPAD_CAT_NAME, NL_CAT_LOCALE);
    }
    msg=catgets(nlmsg_fd,set,number,string);
    return (msg);
}
#endif /* MESSAGE_CAT */
