/* 
 * (C) Copyright 1995 Sun Microsystems, Inc.
 * ALL RIGHTS RESERVED 
 */ 

/*
 * Included Files:
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/Xatom.h>
#include <unistd.h>
#include <Dt/Wsm.h>
#include <X11/StringDefs.h>

/*
 * Include extern functions:
 */


/*
 * Macros:
 */

#define DEFAULT_MASK ( KeyPressMask | KeyReleaseMask | ButtonPressMask | \
                       ButtonReleaseMask | EnterWindowMask |		\
                       LeaveWindowMask | PointerMotionMask |		\
                       Button1MotionMask |				\
                       Button2MotionMask | Button3MotionMask |		\
                       Button4MotionMask | Button5MotionMask |		\
                       ButtonMotionMask | KeymapStateMask |		\
                       ExposureMask | VisibilityChangeMask | 		\
                       StructureNotifyMask | /* ResizeRedirectMask | */	\
                       SubstructureNotifyMask | SubstructureRedirectMask | \
                       FocusChangeMask | PropertyChangeMask |		\
                       ColormapChangeMask | OwnerGrabButtonMask )

/*
 * Global Variables:
 */


/*
 * Static Variables:
 */

typedef struct winList {
	XConfigureEvent config;
	int             moveX;
	int             moveY;
	char            *name;
	Bool		unseen;
	Bool		mapped;
	struct winList  *next;
	struct winList  *prev;
} WinList;

static WinList *winHead;


static void CreateWindow (Display *display, int screen, WinList *winList);

static Bool allWorkspaces = False;
static Bool haveFocus = False;
static WinList *mainWindow;

static Widget        toplevel;
static XtAppContext  appContext;
static Atom          fwsIconAtom;

static Atom FWS_CLIENT;
static Atom FWS_COMM_WINDOW;
static Atom FWS_PROTOCOLS;
static Atom FWS_STACK_UNDER;
static Atom FWS_PARK_ICONS;
static Atom FWS_PASS_ALL_INPUT;
static Atom FWS_PASSES_INPUT;
static Atom FWS_HANDLES_FOCUS;

static Atom FWS_REGISTER_WINDOW;
static Atom FWS_STATE_CHANGE;
static Atom FWS_UNSEEN_STATE;
static Atom FWS_NORMAL_STATE;
static Atom WM_PROTOCOLS;
static Atom WM_CHANGE_STATE;

static Window fwsCommWindow;
static Bool   fwsStackUnder;
static Bool   fwsParkIcons;
static Bool   fwsPassesInput;
static Bool   fwsHandlesFocus;

static XFontStruct   *titleFont;
static XFontStruct   *msgFont;

typedef struct {
	Bool     debug;
	Bool     noFws;
	Bool     allWs;
	Bool     ws0;
	Bool     ws1;
	Bool     ws2;
	Bool     ws3;
	Bool     help;
} OptionData;

static OptionData options;

static XtResource resources[] = {
	{ "debug", NULL, XtRBool,  sizeof(Bool), XtOffsetOf(OptionData,debug),
	  XtRImmediate, False },
	{ "noFws", NULL, XtRBool,  sizeof(Bool), XtOffsetOf(OptionData,noFws),
	  XtRImmediate, False },
	{ "all",   NULL, XtRBool,  sizeof(Bool), XtOffsetOf(OptionData,allWs),
	  XtRImmediate, False },
	{ "ws0",   NULL, XtRBool,  sizeof(Bool), XtOffsetOf(OptionData,ws0),
	  XtRImmediate, False },
	{ "ws1",   NULL, XtRBool,  sizeof(Bool), XtOffsetOf(OptionData,ws1),
	  XtRImmediate, False },
	{ "ws2",   NULL, XtRBool,  sizeof(Bool), XtOffsetOf(OptionData,ws2),
	  XtRImmediate, False },
	{ "ws3",   NULL, XtRBool,  sizeof(Bool), XtOffsetOf(OptionData,ws3),
	  XtRImmediate, False },
	{ "help",   NULL, XtRBool,  sizeof(Bool), XtOffsetOf(OptionData,help),
	  XtRImmediate, False }
};

static int numResources = sizeof resources / sizeof resources[0];
	
static XrmOptionDescRec clOptions[] = {
	{ "-display",	".display",	XrmoptionSepArg,	":0.0" },
	{ "-debug",	".debug",	XrmoptionNoArg,		"on"   },
	{ "-noFWS",	".noFWS",	XrmoptionNoArg,		"off"  },
	{ "-all",       ".all",         XrmoptionNoArg,         "true" },
	{ "-ws0",       ".ws0",         XrmoptionNoArg,         "true" },
	{ "-ws1",       ".ws1",         XrmoptionNoArg,         "true" },
	{ "-ws2",       ".ws2",         XrmoptionNoArg,         "true" },
	{ "-ws3",       ".ws3",         XrmoptionNoArg,         "true" },
	{ "-help",      ".help",        XrmoptionNoArg,         "true" }
};

static int numClOptions = sizeof clOptions / sizeof clOptions[0];


/*************************************<->***********************************
 *
 * Usage() -
 *
 * Print basic instructions.
 *
 *************************************<->***********************************/

static void Usage (void)
{
	printf
("\n"
 "--- FWS protocol test program. ---\n"
 "Drag the test window(s) with the mouse button 1.\n"
 "\n"
 "Hit a key to test a function.\n"
 " q - quits the program.\n"
 " f - raise the window (front).\n"
 " b - lower the window (back).\n"
 " u - unmap the window, wait 3 seconds, map it.\n"
 " a - toggle the window between occupying all workspaces and\n"
 "     occupying ony the current workspace.\n"
 " i - create icon-like test window.\n"
 " k - destroy (kill) the test window.\n"
 " h - print this message.\n"
 "Other keys should beep.\n"
 "\n"
 "Command line options:\n"
 "  -debug    be verbose.\n"
 "  -noFWS    don't use FWS protocols, just override-redirect.\n"
 "  -all      occupy all workspaces.\n"
 "  -ws0, -ws1, -ws2, -ws3   occupy one or more of the first four workspaces.\n"
 "  -help     print this message.\n"
 "\n"
	);
}


/*************************************<->***********************************
 *
 * NewWindow() -
 *
 * Create a new entry in our window list.
 *
 *************************************<->***********************************/

static WinList *NewWindow (void)
{
	WinList *result;

	result = (WinList *) malloc (sizeof (WinList));
	memset (result, 0, sizeof (WinList));
	result->next = winHead;
	result->prev = NULL;
	if (winHead != NULL)
		winHead->prev = result;
	winHead = result;
	return result;
}


/*************************************<->***********************************
 *
 * FindListEntry() -
 *
 * Find the list entry for a window given the window id.
 *
 *************************************<->***********************************/

WinList *FindListEntry (Window window)
{
	WinList *winList;

	for (winList = winHead;
	     winList != NULL;
	     winList = winList->next)

		if (winList->config.window == window)
			break;
	return winList;
}


/*************************************<->***********************************
 *
 *  DrawFocusButton ()
 *
 *  This function paints a circle in the center of the main window, solid if
 *  the window has focus, an outline otherwise.
 *
 *************************************<->***********************************/

static void DrawFocusButton (Bool solid)
{
	int         x, y;
	GC          gc;
	XGCValues   gcValues;
	Display     *display;
	Window      window;
	int         screen;

	display = mainWindow->config.display;
	window = mainWindow->config.window;
	screen = DefaultScreen (display);
	x = mainWindow->config.width / 2;
	y = mainWindow->config.height / 2;

	if (solid)
	{
		gcValues.foreground = BlackPixel (display, screen);
		gc = XCreateGC (display, window, GCForeground, &gcValues);
		XFillArc (display, window, gc, x, y, 20, 20, 0, 360 * 64);
		XFreeGC (display, gc);
	}
	else
	{
		gcValues.foreground = WhitePixel (display, screen);
		gc = XCreateGC (display, window, GCForeground, &gcValues);
		XFillArc (display, window, gc, x, y, 20, 20, 0, 360 * 64);
		XFreeGC (display, gc);
		gcValues.foreground = BlackPixel (display, screen);
		gc = XCreateGC (display, window, GCForeground, &gcValues);
		XDrawArc (display, window, gc, x, y, 20, 20, 0, 360 * 64);
		XFreeGC (display, gc);
	}
}


/*************************************<->***********************************
 *
 *  Draw ()
 *
 *  This function paints the window with a test pattern.  The window is White
 *  with a black text string in font titleFont.  If this is the main window,
 *  then also draw the focus button.
 *
 *************************************<->***********************************/

static void Draw (WinList         *winList)
{
	int         stringlen;
	int         direction, ascent, descent;
	XCharStruct overall;
	int         x, y;
	GC          gc;
	XGCValues   gcValues;
	Display     *display;

	stringlen = strlen (winList->name);
	XTextExtents (titleFont, winList->name, stringlen, &direction,
		      &ascent, &descent, &overall);
	x = (winList->config.width - overall.width)/2;
	y = (winList->config.height + overall.ascent - overall.descent)/2;
	display = winList->config.display;
	gcValues.foreground = BlackPixel (display, DefaultScreen (display));
	gcValues.font = titleFont->fid;
	gc = XCreateGC (display, winList->config.window,
			GCForeground | GCFont, &gcValues);
	XDrawString (display, winList->config.window, gc, x, y,
		     winList->name, stringlen);
	XFreeGC (display, gc);
	if (winList == mainWindow)
		DrawFocusButton (haveFocus);
}


/*************************************<->***********************************
 *
 *  DrawString
 *
 * Draw a string in the lower left corner of a given window in
 * font msgFont.
 *
 *************************************<->***********************************/

static void DrawString (char    *buf,
			WinList *winList)
{
	int         stringlen;
	int         direction, ascent, descent;
	XCharStruct overall;
	int         x, y;
	GC          gc;
	XGCValues   gcValues;
	Display     *display;

	stringlen = strlen (buf);
	XTextExtents (msgFont, buf, stringlen, &direction,
		      &ascent, &descent, &overall);
	x = 5;
	y = winList->config.height - msgFont->descent - 5;
	display = winList->config.display;
	XClearArea (display, winList->config.window, x, y - msgFont->ascent,
		    winList->config.width / 2,
		    msgFont->ascent + msgFont->descent + 1, False);
	gcValues.foreground = BlackPixel (display, DefaultScreen (display));
	gcValues.font = msgFont->fid;
	gc = XCreateGC (display, winList->config.window,
			GCForeground | GCFont, &gcValues);
	XDrawString (display, winList->config.window, gc, x, y, buf, stringlen);
	XFreeGC (display, gc);
}


/*************************************<->***********************************
 *
 *  DrawKeysym
 *
 * Draw the name of a keysym and key state in the given window.
 *
 *************************************<->***********************************/

static void DrawKeysym (char    *keysym,
			int     state,
			WinList *winList)
{
	char        buf[100];

	sprintf (buf, "%s [0x%02x]", keysym, state);
	DrawString (buf, winList);
}


/*************************************<->***********************************
 *
 *  DrawButtonMsg - 
 *
 * Draw the button pressed mask and modifier state in the given window.
 *
 *************************************<->***********************************/

static void DrawButtonMsg (int     buttons,
			   int     state,
			   WinList *winList)
{
	char        buf[100];

	sprintf (buf, "button 0x%02x [0x%02x]", buttons, state);
	DrawString (buf, winList);
}


/*************************************<->***********************************
 *
 * AddWMHints -
 *
 * Set up the WM hints for this window, especially input=True.
 *
 *************************************<->***********************************/

static void AddWMHints (Display *display,
			Window  window)
{
	XWMHints *hints;

	hints = XAllocWMHints();
	hints->flags = InputHint;
	hints->input = True;
	XSetWMHints (display, window, hints);
	XFree (hints);
}


/*************************************<->***********************************
 *
 * AddFwsProtocols -
 *
 * Add the FWS protocol atoms to the WMProtocols property for the window.
 *
 *************************************<->***********************************/

static void AddFwsProtocols (Display *display,
			     Window window)
{
#define MAX_FWS_PROTOS 10
	Atom       fwsProtocols[MAX_FWS_PROTOS];
	int        nProtos;

	nProtos = 0;
	fwsProtocols[nProtos++] = FWS_CLIENT;
	fwsProtocols[nProtos++] = FWS_STACK_UNDER;
	fwsProtocols[nProtos++] = FWS_STATE_CHANGE;
	fwsProtocols[nProtos++] = FWS_PASS_ALL_INPUT;
	XChangeProperty (display, window, WM_PROTOCOLS,
			 XA_ATOM, 32, PropModeAppend,
			 (unsigned char *) fwsProtocols, nProtos);
}


/*************************************<->***********************************
 *
 * AddWorkspaces() -
 *
 * Set up the initial workspace list for this window based on the command
 * line options (or resources). 
 *
 *************************************<->***********************************/

static void AddWorkspaces (Display *display,
			   Window window)
{
#define MAX_FWS_PROTOS 10
	Atom       *pWorkspaces;
	int        nWorkspaces;
	Atom       wsList[4];
	int        nList;

	nWorkspaces = 0;
	if (DtWsmGetWorkspaceList (display, DefaultRootWindow (display),
				   &pWorkspaces, &nWorkspaces) == Success)
	{
		nList = 0;
		if (options.ws0)
			wsList[nList++] = pWorkspaces[0];
		if (options.ws1)
			wsList[nList++] = pWorkspaces[1];
		if (options.ws2)
			wsList[nList++] = pWorkspaces[2];
		if (options.ws3)
			wsList[nList++] = pWorkspaces[3];

		XFree (pWorkspaces);
	}
	else
		printf ("DtWsmGetWorkspaceList() fails.\n");

	if (options.allWs)
	{
		DtWsmOccupyAllWorkspaces (display, window);
		allWorkspaces = True;
	}

	else if (nList > 0)
		DtWsmSetWorkspacesOccupied (display, window, wsList, nList);
}


/*************************************<->***********************************
 *
 * newHandler() -
 *
 * Handle X errors (temporarily) to record the occurance of BadWindow
 * errors without crashing.  Used to detect the FWS_COMM_WINDOW root window
 * property containing an old or obsolete window id.
 *
 *************************************<->***********************************/

static Bool badWindowFound;
static int (* oldHandler) (Display *, XErrorEvent *);

static int newHandler (Display *display,
		       XErrorEvent *xerror)
{
	if (xerror->error_code != BadWindow)
		(*oldHandler)(display, xerror);
	else
		badWindowFound = True;
	return 0;
}


/*************************************<->***********************************
 *
 * RegisterFwsWindow() -
 *
 * Send a client message to the FWS_COMM_WINDOW indicating the existance
 * of a new FWS client window.  Be careful to avoid BadWindow errors on
 * the XSendEvent in case the FWS_COMM_WINDOW root window property had
 * old/obsolete junk in it.
 *
 *************************************<->***********************************/

static Bool RegisterFwsWindow (Display *display,
			       Window  window)
{
	XClientMessageEvent  msg;

	msg.type = ClientMessage;
	msg.window = fwsCommWindow;
	msg.message_type = FWS_REGISTER_WINDOW;
	msg.format = 32;
	msg.data.l[0] = window;
	XSync (display, False);
	badWindowFound = False;
	oldHandler = XSetErrorHandler (newHandler);
	XSendEvent (display, fwsCommWindow, False, NoEventMask,
		    (XEvent *) &msg);
	XSync (display, False);
	XSetErrorHandler (NULL);
	if (badWindowFound)
		printf ("No FWS client window to register with.\n");
	return !badWindowFound;
}


/*************************************<->***********************************
 *
 * Minimize() -
 *
 * Create an icon-like window, positioned using the park icons feature of the
 * the FWS spec.  The window should have a number written in it.  The window
 * takes up the entire space offered by the window manager.
 *
 ************************************<->***********************************/

static void Minimize (Display *display)
{
	Atom            propType;
	int             propFormat;
	unsigned long   propItems;
	unsigned long   propBytesAfter;
	unsigned char   *propData;
	int             *nextIcon;
	WinList         *winList;
	char		buff[10];
	static int      count = 0;
	XClientMessageEvent  msg;

	/*
	 * Get the icon position property from the window manager.  If
	 * successful, then create a window in that position.
	 */

	XGrabServer (display);
	if (XGetWindowProperty (display, fwsCommWindow,
				fwsIconAtom, 0, 5, False,
				AnyPropertyType, &propType, &propFormat,
				&propItems, &propBytesAfter, 
				&propData) == Success
	    && propData != NULL
	    && propFormat == 32
	    && propBytesAfter == 0)
	{
		winList = NewWindow();
		nextIcon = (int *) propData;
		winList->config.x =      nextIcon[0];
		winList->config.y =      nextIcon[1];
		winList->config.width =  nextIcon[2];
		winList->config.height = nextIcon[3];
		winList->config.display = display;
		sprintf (buff, "%d", ++count);
		winList->name = strdup (buff);
		XFree (propData);

		CreateWindow (display, DefaultScreen (display), winList);

		/*
		 * Send a client message to the window manager to let it know
		 * which window was assigned to that icon position, so that it
		 * can recover the space later on.
		 */

		msg.type = ClientMessage;
		msg.window = fwsCommWindow;
		msg.message_type = fwsIconAtom;
		msg.format = 32;
		msg.data.l[0] = winList->config.window;
		msg.data.l[1] = nextIcon[4];
		XSendEvent (display, fwsCommWindow, False, NoEventMask,
			    (XEvent *) &msg);
		XDeleteProperty (display, fwsCommWindow, fwsIconAtom);

		/*
		 * Process input from the new window, map it, etc.
		 */

		XSelectInput (display, winList->config.window, DEFAULT_MASK);
		AddWMHints (display, winList->config.window);

		if (!options.noFws) {
			AddFwsProtocols (display, winList->config.window);
			AddWorkspaces (display, winList->config.window);
			RegisterFwsWindow (display, winList->config.window);
		}

		if (options.debug)
			printf ("Mapping icon window.\n");

		winList->mapped = True;
		if (!winList->unseen && winList->mapped)
			XMapWindow (display, winList->config.window);
	}
	XUngrabServer (display);
}


/*************************************<->***********************************
 *
 * TimeoutMapWindow() -
 *
 * Map the window when called by the Xt timeout handler.  This is just used to
 * schedule an XMapWindow in the future, possibly after the workspace has
 * changed, to insure that we have caught the fact that the window should be
 * marked unseen in that case.
 *
 *************************************<->***********************************/

static void TimeoutMapWindow (XtPointer client_data,
			      XtIntervalId *id)
{
	WinList *winList = (WinList *) client_data;
	if (options.debug)
		printf ("Mapping window on keyboard command.\n");
	winList->mapped = True;
	if (!winList->unseen && winList->mapped)
		XMapWindow (winList->config.display, winList->config.window);
}


/*************************************<->***********************************
 *
 * CharacterCommand() -
 *
 * Decode the key pressed by the user and execute the appropriate test code.
 *
 *************************************<->***********************************/

static Bool CharacterCommand (Display *display,
			      WinList *winList,
			      char    cmd)
{
	Atom            currWs;
	Bool            done;
	XWindowChanges  wc;

	done = False;
	switch (cmd) {
	case 'q':
	case 'Q':
		done = True;
		break;
	case 'f':
	case 'F':
		XRaiseWindow (display, winList->config.window);
		break;
	case 'b':
	case 'B':
		if (options.noFws)
			XLowerWindow (display, winList->config.window);
		else
		{
			wc.stack_mode = Above;
			wc.sibling = fwsCommWindow;
			XConfigureWindow (display, winList->config.window,
					  CWStackMode | CWSibling, &wc);
		}
		XSync (display, False);
		break;
	case 'u':
	case 'U':
		if (options.debug)
			printf ("Unmapping window on keyboard command.\n");
		XUnmapWindow (display, winList->config.window);
		winList->mapped = False;
 		XtAppAddTimeOut (appContext, 3000, TimeoutMapWindow, winList);
		break;

	case 'a':
	case 'A':
		if (allWorkspaces) {
			DtWsmGetCurrentWorkspace (display,
						  winList->config.window,
						  &currWs);
			DtWsmSetWorkspacesOccupied (display,
						    winList->config.window, 
						    &currWs, 1);
			allWorkspaces = False;
		}
		else {
			DtWsmOccupyAllWorkspaces (display,
						  winList->config.window);
			allWorkspaces = True;
		}
		break;

	case 'h':
	case 'H':
		Usage();
		break;

	case 'i':
	case 'I':
		Minimize (display);
		break;

	case 'k':
	case 'K':
		XDestroyWindow (display, winList->config.window);

		if (winList->prev != NULL)
			winList->prev->next = winList->next;
		else
			winHead = winList->next;

		if (winList->next != NULL)
			winList->next->prev = winList->prev;
		free (winList);

		if (winHead == NULL)
			exit (0);
		break;

	default:
		XBell (display, 50);
	}
	return done;
}


/*************************************<->***********************************
 *
 * HandleClientMessage() -
 *
 * Process an incoming client message.  Change our window state to unseen
 * or normal as appropriate, and map or unmap the window.
 *
 *************************************<->***********************************/

static void HandleClientMessage (Display        *display,
				 Window         window,
				 XClientMessageEvent *event)
{
	WinList *winList;

	if (event->message_type == FWS_STATE_CHANGE)
	{
		winList = FindListEntry (window);
		if (winList == NULL)
			return;

		if (event->data.l[0] == FWS_UNSEEN_STATE)
		{
			if (options.debug)
				printf ("Unmapping window - client message.\n");
			XUnmapWindow (display, window);
			winList->unseen = True;
		}
		else if (event->data.l[0] == NormalState)
		{
			if (options.debug)
				printf ("Mapping window - client message.\n");
			winList->unseen = False;
			if (!winList->unseen && winList->mapped)
				XMapWindow (display, window);
		}
	}
}


/*************************************<->***********************************
 *
 * MainLoop() -
 *
 * Handle all X events delivered to this program.
 *
 *************************************<->***********************************/

static void MainLoop (int     screen)
{
	Bool            movingWindow;
	XEvent          event;
	XWindowChanges  changes;
	Bool		done;
	char		buff[4];
	KeySym		keysym;
	WinList         *winList;
	int		len;
	Display         *display;

	display = XtDisplay (toplevel);
	movingWindow = False;
	done = False;
	while (!done) {

		/*
		 * Get an event from the server.
		 * Determine which of our windows it is.
		 * Preform an action depending on the event.
		 */

		XtAppNextEvent (XtWidgetToApplicationContext(toplevel), &event);
		winList = FindListEntry (event.xany.window);

		if (winList != NULL) switch (event.type) {

		case ReparentNotify:
			if (options.debug) printf ("ReparentNotify.\n");
			break;
		case ConfigureNotify:
			if (options.debug) printf ("ConfigureNotify.\n");
			winList->config = event.xconfigure;
			break;
		case PropertyNotify:
			if (options.debug) printf ("PropertyNotify.\n");
			break;
		case MapNotify:
			if (options.debug) printf ("MapNotify.\n");
			break;
		case VisibilityNotify:
			if (options.debug) printf ("VisibilityNotify.\n");
			break;
		case Expose:
			if (options.debug) printf ("Expose.\n");
			Draw (winList);
			break;
		case MotionNotify:
			if (options.debug) printf ("MotionNotify.\n");
			if (!movingWindow)
				break;
			while (XCheckTypedWindowEvent (display,
						       event.xany.window,
						       MotionNotify, &event))
				;
			changes.x = event.xmotion.x_root - winList->moveX;
			changes.y = event.xmotion.y_root - winList->moveY;
			XReconfigureWMWindow (event.xmotion.display,
					      event.xmotion.window,
					      screen, CWX | CWY, &changes);
			break;
		case ButtonPress:
			if (options.debug) printf ("ButtonPress.\n");
			DrawButtonMsg (event.xbutton.button,
				       event.xbutton.state, winList);
			XSetInputFocus (event.xbutton.display,
					event.xbutton.window,
					RevertToNone, CurrentTime);
			if (event.xbutton.button == Button1) {
				winList->moveX = event.xbutton.x;
				winList->moveY = event.xbutton.y;
				movingWindow = True;
			}
			break;
		case ButtonRelease:
			if (options.debug) printf ("ButtonRelease.\n");
			if (event.xbutton.button == Button1) {
				movingWindow = False;
				changes.x = event.xbutton.x_root
					- winList->moveX;
				changes.y = event.xbutton.y_root
					- winList->moveY;
				XReconfigureWMWindow (event.xbutton.display,
						      event.xbutton.window,
						      screen, CWX | CWY,
						      &changes);
			}
			break;
		case KeyPress:
			if (options.debug) printf ("KeyPress.\n");
			len = XLookupString (&event.xkey, buff, sizeof buff,
					     &keysym, NULL);
			DrawKeysym (XKeysymToString (keysym),
				    event.xkey.state, winList);
			if (len == 1)
				done = CharacterCommand (display, winList,
							 buff[0]);
			event.xkey.window = DefaultRootWindow (display);
			XSendEvent (display, DefaultRootWindow (display),
				    False, KeyPressMask | KeyReleaseMask,
				    &event);
			break;
		case KeyRelease:
			if (options.debug) printf ("KeyRelease.\n");
			event.xkey.window = DefaultRootWindow (display);
			XSendEvent (display, DefaultRootWindow (display),
				    False, KeyPressMask | KeyReleaseMask,
				    &event);
			break;
		case EnterNotify:
			if (options.debug) printf ("EnterNotify.\n");
			break;
		case LeaveNotify:
			if (options.debug) printf ("LeaveNotify.\n");
			break;
		case KeymapNotify:
			if (options.debug) printf ("KeymapNotify.\n");
			break;
		case ClientMessage:
			if (options.debug) printf ("ClientMessage.\n");
			HandleClientMessage (display, event.xany.window,
					     &event.xclient);
			break;
		case FocusIn:
			if (event.xfocus.window == mainWindow->config.window)
			{
				haveFocus = True;
				XSetInputFocus (event.xfocus.display,
						event.xfocus.window,
						RevertToNone, CurrentTime);
				DrawFocusButton (haveFocus);
			}
			break;
		case FocusOut:
			if (event.xfocus.window == mainWindow->config.window)
			{
				haveFocus = False;
				DrawFocusButton (haveFocus);
			}
			break;
		default:
			if (options.debug) printf ("Event type %d returned.\n",
					   event.type);
		}
	}
}


/*************************************<->***********************************
 *
 * WMSupportsFWS() -
 *
 * Initialize our atoms and determine if the current window manager is
 * providing FWS extension support.
 *
 *************************************<->***********************************/

static Bool WMSupportsFWS (Display *display,
			   int     screen)
{
	int             i;
	Atom            protocol;
	Atom            propType;
	int             propFormat;
	unsigned long   propItems;
	unsigned long   propBytesAfter;
	unsigned char   *propData;
	char            propName[30];

	FWS_CLIENT          = XInternAtom(display, "_SUN_FWS_CLIENT", False);
	FWS_COMM_WINDOW     = XInternAtom(display, "_SUN_FWS_COMM_WINDOW", False);
	FWS_PROTOCOLS       = XInternAtom(display, "_SUN_FWS_PROTOCOLS", False);
	FWS_STACK_UNDER     = XInternAtom(display, "_SUN_FWS_STACK_UNDER", False);
	FWS_PARK_ICONS      = XInternAtom(display, "_SUN_FWS_PARK_ICONS", False);
	FWS_PASS_ALL_INPUT  = XInternAtom(display, "_SUN_FWS_PASS_ALL_INPUT", False);
	FWS_PASSES_INPUT    = XInternAtom(display, "_SUN_FWS_PASSES_INPUT", False);
	FWS_HANDLES_FOCUS   = XInternAtom(display, "_SUN_FWS_HANDLES_FOCUS", False);
	FWS_REGISTER_WINDOW = XInternAtom(display, "_SUN_FWS_REGISTER_WINDOW",False);
	FWS_STATE_CHANGE    = XInternAtom(display, "_SUN_FWS_STATE_CHANGE", False);
	FWS_UNSEEN_STATE    = XInternAtom(display, "_SUN_FWS_UNSEEN_STATE", False);
	FWS_NORMAL_STATE    = XInternAtom(display, "_SUN_FWS_NORMAL_STATE", False);
	WM_PROTOCOLS        = XInternAtom(display, "WM_PROTOCOLS",False);
	WM_CHANGE_STATE     = XInternAtom(display, "WM_CHANGE_STATE",False);
	sprintf (propName, "_SUN_FWS_NEXT_ICON_%d", screen);
	fwsIconAtom = XInternAtom (display, propName, False);

	if (XGetWindowProperty (display, DefaultRootWindow (display),
				FWS_COMM_WINDOW, 0, 1,
				False, AnyPropertyType, &propType, 
				&propFormat, &propItems,
				&propBytesAfter, &propData) != Success)
		return False;

	if (propFormat != 32 ||
	    propItems != 1 ||
	    propBytesAfter != 0) {
		printf ("Bad FWS_COMM_WINDOW property on root window.\n");
		XFree (propData);
		return False;
	}

	fwsCommWindow = *(Window *) propData;
	printf ("Using fwsCommWindow = 0x%lx.\n", fwsCommWindow);
	XFree (propData);


	if (XGetWindowProperty (display, DefaultRootWindow (display),
				FWS_PROTOCOLS, 0, 10,
				False, AnyPropertyType, &propType, 
				&propFormat, &propItems,
				&propBytesAfter, &propData) != Success)
		return False;

	if (propFormat != 32 ||
	    propBytesAfter != 0) {
		printf ("Bad FWS_PROTOCOLS property on root window.\n");
		XFree (propData);
		return False;
	}

	for (i = 0; i < propItems; ++i) {
		protocol = ((Atom *) propData)[i];
		if (protocol == FWS_STACK_UNDER) {
			fwsStackUnder = True;
			printf ("Using fwsStackUnder.\n");
		}
		else if (protocol == FWS_PARK_ICONS) {
			fwsParkIcons = True;
			printf ("Using fwsParkIcons.\n");
		}
		else if (protocol == FWS_PASSES_INPUT) {
			fwsPassesInput = True;
			printf ("Using fwsPassesInput.\n");
		}
		else if (protocol == FWS_HANDLES_FOCUS) {
			fwsHandlesFocus = True;
			printf ("Using fwsHandlesFocus.\n");
		}
	}

	XFree (propData);
	return True;
}


/*************************************<->***********************************
 *
 * CreateWindow() -
 *
 * Create a test window with en entry in the window list.
 *
 *************************************<->***********************************/


static void CreateWindow (Display *display,
			  int     screen,
			  WinList *winList)
{
	Window          window;
	static char     *wmNameString = "FWSTest";
	XTextProperty   wmName;
	XSetWindowAttributes attributes;

	attributes.override_redirect = True;
	attributes.background_pixel = WhitePixel (display, screen);
	window = XCreateWindow (display, DefaultRootWindow (display),
				winList->config.x, winList->config.y,
				winList->config.width, winList->config.height,
				0, CopyFromParent, InputOutput,
				CopyFromParent,
				CWOverrideRedirect | CWBackPixel,
				&attributes);
	XStringListToTextProperty (&wmNameString, 1, &wmName);
	XSetWMName (display, window, &wmName);
	winList->config.window = window;
	winList->config.display = display;
#if 0
	printf ("Creating window 0x%x.\n", window);
#endif
	winList->moveX = winList->config.x;
	winList->moveY = winList->config.y;

}



/*************************************<->***********************************
 *
 *  main ()
 *
 *
 *  Description:
 *  -----------
 *  This program opens an override redirect window and draws to it.
 *  FWS protocols are tested.
 *
 *
 *  Inputs:
 *  ------
 *  none.
 *
 *************************************<->***********************************/

void main (int argc,
	   char **argv)
{
	int           screen;
	Display       *display;
	Font          font;
	static char   titleName[] = "-*-lucidabright-*-*-*-*-34-*-*-*-*-*-*-*";
	static char   msgName[]   = "-*-fixed-*-*-*-*-10-*-*-*-*-*-*-*";
	WinList       *winList;
#ifdef SUBWINDOW
	Window        subWindow;
#endif

	toplevel = XtAppInitialize (&appContext, "fws", clOptions,
				    numClOptions, &argc, argv, NULL, NULL, 0);
	XtGetApplicationResources (toplevel, (XtPointer) &options,
				   resources, numResources, NULL, 0);
	if (options.help)
		Usage();
	display = XtDisplay (toplevel);
	screen = DefaultScreen (display);

	winList = NewWindow();
	winList->config.display = display;
	winList->name = strdup ("FwsTest");

	if (!options.noFws)
		options.noFws = !WMSupportsFWS (display, screen);

	winList->config.x = 100;
	winList->config.y = 100;
	winList->config.width = 600;
	winList->config.height = 400;

	CreateWindow (display, screen, winList);
	mainWindow = winList;
	XSelectInput (display, winList->config.window, DEFAULT_MASK);
	AddWMHints (display, winList->config.window);

	if (!options.noFws) {
		AddFwsProtocols (display, winList->config.window);
		AddWorkspaces (display, winList->config.window);
		RegisterFwsWindow (display, winList->config.window);
	}

	font = XLoadFont (display, titleName);
	titleFont = XQueryFont (display, font);
	font = XLoadFont (display, msgName);
	msgFont = XQueryFont (display, font);

#ifdef SUBWINDOW
	subWindow = XCreateSimpleWindow (display, winList->config.window,
					 195, 195, 210, 10, 3,
					 BlackPixel (display, screen),
					 WhitePixel (display, screen));
	XSelectInput (display, subWindow, DEFAULT_MASK);
	printf ("SubWindow 0x%x.\n", subWindow);
	XMapWindow (display, subWindow);
#endif /* SUBWINDOW */

	winList->mapped = True;
	if (!winList->unseen && winList->mapped)
	if (options.debug)
		printf ("Mapping initial window.\n");
	XMapWindow (display, winList->config.window);

	MainLoop (screen);

	XFreeFont (display, titleFont);
	XFreeFont (display, msgFont);
	XDestroyWindow (display, winList->config.window);
	XCloseDisplay (display);
	exit (0);
}
