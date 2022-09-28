/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:     WsmP.h
 **
 **   RCS:	$XConsortium: WsmP.h /main/cde1_maint/1 1995/07/14 20:30:08 drk $
 **   Project:  DT Workspace Manager
 **
 **   Description: Defines PRIVATE properties, structures, and 
 **		   parameters used for communication with the 
 **		   workspace manager. This also includes some 
 **		   backward compatibility stuff for HP_VUE.
 **
 ** (c) Copyright 1993, 1994 Hewlett-Packard Company
 ** (c) Copyright 1993, 1994 International Business Machines Corp.
 ** (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 ** (c) Copyright 1993, 1994 Novell, Inc.
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _Dt_WsmP_h
#define _Dt_WsmP_h
#include <Dt/Wsm.h>
#include <X11/Intrinsic.h>
#include <Dt/Service.h>
#include <Tt/tt_c.h>

/**********************************************************************
 * Workspace atom names 
 **********************************************************************/
#define _XA_DT_MARQUEE_SELECTION	"_DT_MARQUEE_SELECTION"
#define _XA_DT_WORKSPACE_EMBEDDED_CLIENTS	\
					"_DT_WORKSPACE_EMBEDDED_CLIENTS"
#define _XA_DT_WM_REQUEST		"_DT_WM_REQUEST"
#define _XA_DT_WORKSPACE_HINTS		"_DT_WORKSPACE_HINTS"
#define _XA_DT_WORKSPACE_PRESENCE	"_DT_WORKSPACE_PRESENCE"
#define _XA_DT_WORKSPACE_INFO		"_DT_WORKSPACE_INFO"
#define _XA_DT_WM_HINTS			"_DT_WM_HINTS"
#define _XA_DT_WORKSPACE_LIST		"_DT_WORKSPACE_LIST"
#define _XA_DT_WORKSPACE_CURRENT	"_DT_WORKSPACE_CURRENT"


/**********************************************************************
 * Name to request ``all'' workspaces (for a persistent window)
 **********************************************************************/

#define _XA_DT_WORKSPACE_ALL		"all"


/**********************************************************************
 * Workspace function definitions 
 **********************************************************************/
#define DtWM_FUNC_OCCUPY_WS       	DtWM_FUNCTION_OCCUPY_WS
#define DtWM_FUNC_ALL			DtWM_FUNC_OCCUPY_WS

/**********************************************************************
 * Workspace property information
 **********************************************************************/

/* 
 *
 *  NOTE: The "(client -> dtwm)" indication calls out the direction
 *        of information flow. In this case, the client writes the
 *        property and dtwm reads the property.
 *
 *
 *  _DT_WORKSPACE_HINTS		(client -> dtwm)
 *
 *  This property is a list of atoms placed by a client on its 
 *  top level window(s). Each atom is an "interned" string name
 *  for a workspace. The workspace manager looks at this property
 *  when it manages the window (e.g. when the window is mapped)
 *  and will place the window in the workspaces listed.
 *
 *  _DT_WORKSPACE_PRESENCE	(dtwm -> client)
 *
 *  This property is a list of atoms placed on a client by dtwm.
 *  Each atom is an "interned" string name for a workspace. This 
 *  property lists the workspaces that this client lives in.
 *
 *  _DT_WORKSPACE_LIST		(dtwm -> clients)
 *
 *  This property is a list of atoms. Each atom represents a 
 *  name of a workspace.  The list is in "order" such that
 *  the first element is for the first workspace and so on.
 *  This proeprty is placed on the mwm ("wmWindow") window.
 *
 *  _DT_WORKSPACE_CURRENT	(dtwm -> clients)
 *
 *  This property is a single atom, representing the current 
 *  workspace. It is updated each time the workspace changes.
 *  This proeprty is placed on the mwm window.
 *
 *  _DT_WORKSPACE_INFO_<name>	(dtwm -> clients)
 *
 *  There is one property of this form for each workspace in
 *  _DT_WORKSPACE_LIST. This property is a sequence of ISO-LATIN1
 *  NULL-terminated strings representing the elements in a
 *  structure. This information was formerly in _DT_WORKSPACE_INFO
 *  but was broken out to allow for extensibility.
 *  This proeprty is placed on the mwm window.
 *
 *  _DT_WM_HINTS	(client -> dtwm)
 *
 *  This property requests specific window/workspace management behavior.
 *  The functions member of the property allows a client to enable or
 *  disable workspace management functions. The behavior member is
 *  used to indicate front panels and slide-ups.
 *
 */


/**********************************************************************
 * Property structures
 **********************************************************************/

typedef struct _DtWmHints
{
    long	flags;			/* marks valid fields */
    long	functions;		/* special dtwm functions */
    long	behaviors;		/* special dtwm behaviors */
    Window	attachWindow;		/* (reserved) */
} DtWmHints;

/* DtWmHints "flags" definitions */
#define DtWM_HINTS_FUNCTIONS		(1L << 0)
#define DtWM_HINTS_BEHAVIORS		(1L << 1)
#define DtWM_HINTS_ATTACH_WINDOW	(1L << 2)	/* (reserved) */

/* DtWmHints "functions" definitions */
#define DtWM_FUNCTION_ALL		(1L << 0)
#define DtWM_FUNCTION_OCCUPY_WS		(1L << 16)

/* DtWmHints "behaviors" definitions */
#define DtWM_BEHAVIOR_PANEL		(1L << 1)
#define DtWM_BEHAVIOR_SUBPANEL		(1L << 2)
#define DtWM_BEHAVIOR_SUB_RESTORED	(1L << 3)


#ifdef HP_VUE
/*
 * For Compatibility with some old HP VUE clients
 */
#define _XA_VUE_WORKSPACE_INFO		"_VUE_WORKSPACE_INFO"
#define _XA_VUE_WORKSPACE_HINTS		"_VUE_WORKSPACE_HINTS"
#define _XA_VUE_WORKSPACE_PRESENCE	"_VUE_WORKSPACE_PRESENCE"
#define _XA_VUE_WM_REQUEST		"_VUE_WM_REQUEST"
#define _XA_VUE_WM_HINTS		"_VUE_WM_HINTS"
#endif /* HP_VUE */



/**********************************************************************
 * Session atom names 
 **********************************************************************/
#define _XA_DT_SESSION_HINTS		"_DT_SESSION_HINTS"
#define _XA_DT_SAVE_MODE		"_DT_SAVE_MODE"
#define _XA_DT_RESTORE_MODE		"_DT_RESTORE_MODE"
#define _XA_DT_SM_WM_PROTOCOL		"_DT_SM_WM_PROTOCOL"
#define _XA_DT_SM_START_ACK_WINDOWS	"_DT_SM_START_ACK_WINDOWS"
#define _XA_DT_SM_STOP_ACK_WINDOWS	"_DT_SM_STOP_ACK_WINDOWS"
#define _XA_DT_WM_WINDOW_ACK		"_DT_WM_WINDOW_ACK"
#define _XA_DT_WM_EXIT_SESSION		"_DT_WM_EXIT_SESSION"
#define _XA_DT_WM_LOCK_DISPLAY		"_DT_WM_LOCK_DISPLAY"
#define _XA_DT_WM_READY		"_DT_WM_READY"

/**********************************************************************
 * Workspace special character definitions 
 **********************************************************************/
#define DTWM_CH_ESC_NEXT		"\\"

/**********************************************************************
 * Marquee selection callback prototype
 **********************************************************************/
typedef void (*DtWsmMarqueeSelectionProc) ();
/*
    Widget		widget;
    int			type;
    Position		x, y;
    Dimension		width, height;
    XtPointer		client_data;
*/

/**********************************************************************
 * Marquee Select
 **********************************************************************/

#define DT_WSM_MARQUEE_SELECTION_TYPE_BEGIN	1
#define DT_WSM_MARQUEE_SELECTION_TYPE_CONTINUE	2
#define DT_WSM_MARQUEE_SELECTION_TYPE_END	3
#define DT_WSM_MARQUEE_SELECTION_TYPE_CANCEL	4

/**********************************************************************
 * Workspace request definitions 
 *
 * NOTE: These functions do not necessarily match the other
 *       window manager f.* functions!
 **********************************************************************/
#define DTWM_REQ_CHANGE_BACKDROP	"f.change_backdrop"
#define DTWM_REQ_RESTART		"f.restart"

/**********************************************************************
 * Workspace request parameter definitions 
 **********************************************************************/
#define DTWM_REQP_BACKDROP_NONE	"NoBackdrop"
#define DTWM_REQP_NO_CONFIRM	"-noconfirm"

/**********************************************************************
 * Resource converter definitions
 *
 * NOTE: This has been lifted from mwm. 
 * Please keep syncronized with the current version of mwm/dtwm.
 * (See WmGlobal.h)
 **********************************************************************/

/* icon placement values (iconPlacement, ...): */
#define ICON_PLACE_LEFT_PRIMARY		(1L << 0)
#define ICON_PLACE_RIGHT_PRIMARY	(1L << 1)
#define ICON_PLACE_TOP_PRIMARY		(1L << 2)
#define ICON_PLACE_BOTTOM_PRIMARY	(1L << 3)
#define ICON_PLACE_LEFT_SECONDARY	(1L << 4)
#define ICON_PLACE_RIGHT_SECONDARY	(1L << 5)
#define ICON_PLACE_TOP_SECONDARY	(1L << 6)
#define ICON_PLACE_BOTTOM_SECONDARY	(1L << 7)
#define ICON_PLACE_EDGE			(1L << 8)
#define ICON_PLACE_TIGHT		(1L << 9)
#define ICON_PLACE_RESERVE		(1L << 10)

/**********************************************************************
 * Workspace property information
 **********************************************************************/

/* 
 *
 *  NOTE: The "(client -> dtwm)" indication calls out the direction
 *        of information flow. In this case, the client writes the
 *        property and dtwm reads the property.
 *
 */
#ifdef HP_VUE
/*
 *  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *  % _VUE_WORKSPACE_INFO is obsolete, but maintained for      %
 *  %     backward compatiblity. Its function has been         %
 *  %     replaced by                                          %
 *  %         _DT_WORKSPACE_LIST                              %
 *  %         _DT_WORKSPACE_INFO_<name>                       %
 *  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *  _VUE_WORKSPACE_INFO		(dtwm -> client)
 *
 *  This property is a list of structures that contain information 
 *  on each workspace. Each structure contains the atom representing
 *  the workspace string name and the window id of the background
 *  window for the workspace (if any). The first workspace in the 
 *  list is the "active" workspace for the screen. This property is
 *  placed on the window identified by the "wmWindow" member of 
 *  the _MOTIF_WM_INFO property.
 */
#endif /* HP_VUE */
/*
 *
 *  _DT_WM_REQUEST	(client -> dtwm)
 *
 *  This property of type string that is used to communication 
 *  function requests to dtwm. This property is placed on the mwm 
 *  window. Dtwm listens for changes to this property and dequeues 
 *  requests off the top of the list. Requests are NULL-terminated
 *  strings in the format:
 *
 *    <req_type> <req_parms> 
 *
 *  Each request ends with a literal '\0' character to insure
 *  separation from the next request. 
 *
 *  Clients must always add requests to the end of the property 
 *  (mode=PropModeAppend). Use of convenience routines is 
 *  recommended since they take care of proper formatting of the 
 *  requests.
 *
 */


/**********************************************************************
 * Property structures
 **********************************************************************/

/* Internal form of this property */
typedef struct _DtWorkspaceHints
{
    long	version;	/* indicates structure changes */
    long	flags;		/* marks valid fields */
    long	wsflags;	/* special workspace information */
    long	numWorkspaces;	/* number of workspaces */
    Atom *	pWorkspaces;	/* list of atoms for workspaces */
} DtWorkspaceHints;

/* DtWorkspaceHints "flags" definitions */
#define DT_WORKSPACE_HINTS_WSFLAGS		(1L << 0)
	/* The following definitions enables both the numWorkspaces
	   and pWorkspaces fields */
#define DT_WORKSPACE_HINTS_WORKSPACES		(1L << 1)

/* DtWorkspaceHints "wsflags" definitions */
#define DT_WORKSPACE_FLAGS_OCCUPY_ALL		(1L << 0)


#ifdef HP_VUE
/*
 * Obsolete WorkspaceInfo structure
 */
typedef struct _WorkspaceInfo
{
    Atom	   workspace;		/* workspace "name" */
    Window	   backgroundWindow;	/* backdrop window ID (if any) */
    unsigned long  bg;			/* backdrop background pixel */
    unsigned long  fg;			/* backdrop foreground pixel */
    Atom	   backdropName;	/* atomized name for backdrop */
} WorkspaceInfo;
#endif /* HP_VUE */

/**********************************************************************
 * Marquee Selection Structures
 **********************************************************************/
typedef struct _DtMarqueeSelectData
{
    long	state;			/* current property state */
    Position	x;			/* NW corner of select area */
    Position	y;			/* NW corner of select area */
    Dimension	width;			/* size of select area */
    Dimension	height;			/* size of select area */
} DtMarqueeSelectData;

typedef struct _DtMarqueeSelectProp
{
    long	state;			/* current property state */
    long	x;			/* NW corner of select area */
    long	y;			/* NW corner of select area */
    long	width;			/* size of select area */
    long	height;			/* size of select area */
} DtMarqueeSelectProperty;

/**********************************************************************
 * Marquee Selection Definitions
 **********************************************************************/
#define DT_MARQUEE_SELECT_BEGIN	1
#define DT_MARQUEE_SELECT_CONTINUE	2
#define DT_MARQUEE_SELECT_END		3
#define DT_MARQUEE_SELECT_CANCEL	4

/**********************************************************************
 * Workspace change callback context (opaque)
 **********************************************************************/
typedef void (*DtWsmCBProc) ();
struct _DtWsmCBContext
{
    Tt_pattern		pattern;
    Widget		widget;
    DtWsmCBProc		ws_cb;
    XtPointer		client_data;
    XtPointer		nested_context;
};

/**********************************************************************
 * Convenience Routines
 **********************************************************************/

#ifdef HP_VUE
/*************************************<->*************************************
 *
 *  status DtGetWorkspaceInfo (display, root, ppWsInfo, pNumInfo)
 *
 *
 *  Description:
 *  -----------
 *  Get the contents of the _DT_WORKSPACE_INFO property on a window
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  root	- root window of screen being managed
 *  ppWsInfo	- pointer to a pointer (to be returned)
 *  pNumInfo	- pointer to a number (to be returned)
 *
 *  Outputs:
 *  -------
 *  *ppWsInfo	- pointer to a list of workspace info structures
 *  *pNumInfo	- number of workspace info structures in list
 *  Return	- Success if property fetched ok.
 *		  Failure returns are from XGetWindowProperty
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
extern Status DtGetWorkspaceInfo ();
#else
extern Status DtGetWorkspaceInfo( 
                        Display *display,
                        Window root,
                        WorkspaceInfo **ppWsInfo,
                        unsigned long *pNumInfo) ;
#endif
#endif /* HP_VUE */


/*************************************<->*************************************
 *
 *  status _DtGetMwmWindow (display, root, pMwmWindow)
 *
 *
 *  Description:
 *  -----------
 *  Get the Motif Window manager window
 *
 *
 *  Inputs:
 *  ------
 *  display		- display 
 *  root		- root window of screen
 *  pMwmWindow		- pointer to a window (to be returned)
 *
 *  Outputs:
 *  --------
 *  *pMwmWindow		- mwm window id, if successful
 *  Return		- status from XGetWindowProperty
 *
 *  Comments:
 *  --------
 *  This can fail if mwm is not managing the screen for the root window
 *  passed in.
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
extern Status _DtGetMwmWindow();
#else
extern Status _DtGetMwmWindow( 
                        Display *display,
                        Window root,
                        Window *pMwmWindow) ;
#endif


/*************************************<->*************************************
 *
 *  status DtGetWorkspacePresence (display, window, ppWsPresence, 
 *					pNumPresence)
 *
 *
 *  Description:
 *  -----------
 *  Get the contents of the _DT_WORKSPACE_PRESENCE property on a window
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  window	- window to get info from
 *  ppWsPresence	- pointer to a pointer (to be returned)
 *  pNumPresence	- pointer to a number (to be returned)
 *
 *  Outputs:
 *  -------
 *  *ppWsPresence	- pointer to a list of workspace info structures
 *                 	  (NOTE: This should be freed using XFree)
 *  *pNumPresence	- number of workspace info structures in list
 *  Return		- Success if property fetched ok.
 *		  	  Failure returns are from XGetWindowProperty
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
extern Status DtGetWorkspacePresence();
#else
extern Status DtGetWorkspacePresence( 
                        Display *display,
                        Window window,
                        Atom **ppWsPresence,
                        unsigned long *pNumPresence) ;
#endif


/*************************************<->*************************************
 *
 *  status _DtGetEmbeddedClients (display, root, ppEmbeddedClients, 
 *					pNumEmbeddedClients)
 *
 *
 *  Description:
 *  -----------
 *  Get the contents of the _DT_WORKSPACE_EMBEDDED_CLIENTS property 
 *  from a root window. This is a list (array) of top-level windows that 
 *  are embedded in the front panel of the window manager. They would
 *  not be picked up ordinarily by a session manager in a normal
 *  search for top-level windows because they are reparented to 
 *  the front panel which itself is a top-level window.
 *
 *
 *  Inputs:
 *  ------
 *  display		- display 
 *  root		- root window to get info from
 *  ppEmbeddedClients	- pointer to a pointer (to be returned)
 *  pNumEmbeddedClients	- pointer to a number (to be returned)
 *
 *  Outputs:
 *  -------
 *  *ppEmbeddedClients	- pointer to a array of window IDs (top-level
 *			  windows for embedded clients)
 *                 	  (NOTE: This should be freed using XFree)
 *  *pNumEmbeddedClients- number of window IDs in array
 *  Return		- Success if property fetched ok.
 *		  	  Failure returns are from XGetWindowProperty
 * 
 *  Comments:
 *  --------
 *  Use XFree to free the returned data.
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
extern Status _DtGetEmbeddedClients();
#else
extern Status _DtGetEmbeddedClients(
        Display *display,
        Window root,
        Atom **ppEmbeddedClients,
        unsigned long *pNumEmbeddedClients );
#endif


/*************************************<->*************************************
 *
 *  Boolean DtIsBackdropWindow (display, screen_num, window)
 *
 *
 *  Description:
 *  -----------
 *  Returns true if the window passed in is a backdrop window.
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  screen_num	- number of screen we're interested in
 *  window	- window we want to test
 *
 *  Outputs:
 *  -------
 *  Return	- True if window is a backdrop window
 *                False otherwise.
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
extern Boolean DtIsBackdropWindow ();
#else
extern Boolean DtIsBackdropWindow( 
                        Display *display,
                        int screen_num,
                        Window window) ;
#endif

/*************************************<->*************************************
 *
 *  Status _DtWmRestart (display, root)
 *
 *
 *  Description:
 *  -----------
 *  Requests the window manager to restart itself
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  root	- root window for screen 
 *
 *  Returns:
 *  --------
 *  Success if request was sent
 *
 *  Comments:
 *  ---------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
extern Status 
_DtWmRestart();
#else
extern Status 
_DtWmRestart( 
	Display *display,
	Window root) ;
#endif

/*************************************<->*************************************
 *
 *  status DtWmGetMarqueeSelection (display, root, pMarqueeData)
 *
 *
 *  Description:
 *  -----------
 *  Get the contents of the _DT_WORKSPACE_LIST property 
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  root	- root window of screen of interest
 *  pNumWorkspaces - the number of workspaces returned
 *  ppWorkspaceList - pointer to a pointer to return
 *
 *  Outputs:
 *  --------
 *  *ppDtWmHints-points to the Workspace List
 *		  (NOTE: This should be freed using XFree)
 *
 *  Comments:
 *  ---------
 * 
 *************************************<->***********************************/
extern Status
#ifdef _NO_PROTO
DtWmGetMarqueeSelection();
#else
DtWmGetMarqueeSelection(
        Display *display,
        Window root,
        DtMarqueeSelectData *pMarqueeData);
#endif /* _NO_PROTO */


/*************************************<->*************************************
 *
 *  _DtWsmChangeBackdrop (display, root, path, pixmap)
 *
 *
 *  Description:
 *  -----------
 *  Request the HP DT workspace manager to change the backdrop
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  root	- root window of screen
 *  path	- file path to bitmap file
 *  pixmap	- pixmap id of backdrop pixmap
 *
 *  Returns:
 *  --------
 *  Success if request sent 
 * 
 *************************************<->***********************************/
extern Status 
#ifdef _NO_PROTO
_DtWsmChangeBackdrop ();
#else
_DtWsmChangeBackdrop (
	Display *display, 
	Window root, 
	char *path, 
	Pixmap pixmap);
#endif /* _NO_PROTO */

/*************************************<->*************************************
 *
 *  _DtWmCvtStringToIPlace (args, numArgs, fromVal, toVal)
 *
 *
 *  Description:
 *  -----------
 *  This function converts a string to an icon placement scheme description.
 *
 *
 *  Inputs:
 *  ------
 *  args = NULL (don't care)
 *
 *  numArgs = 0 (don't care)
 *
 *  fromVal = resource value to convert
 *
 * 
 *  Outputs:
 *  -------
 *  toVal = descriptor to use to return converted value
 *
 *************************************<->***********************************/

extern void
#ifdef _NO_PROTO
_DtWmCvtStringToIPlace () ;
#else /* _NO_PROTO */
_DtWmCvtStringToIPlace (
	XrmValue *args, 
	Cardinal numArgs, 
	XrmValue *fromVal, 
	XrmValue *toVal);
#endif /* _NO_PROTO */

/*************************************<->*************************************
 *
 *  status DtWmGetMarqueeSelection (display, root, pMarqueeData)
 *
 *
 *  Description:
 *  -----------
 *  Get the contents of the _DT_WORKSPACE_LIST property 
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  root	- root window of screen of interest
 *  pNumWorkspaces - the number of workspaces returned
 *  ppWorkspaceList - pointer to a pointer to return
 *
 *  Outputs:
 *  --------
 *  *ppDtWmHints-points to the Workspace List
 *		  (NOTE: This should be freed using XFree)
 *
 *  Comments:
 *  ---------
 * 
 *************************************<->***********************************/
extern Status
#ifdef _NO_PROTO
DtWmGetMarqueeSelection();
#else
DtWmGetMarqueeSelection(
        Display *display,
        Window root,
        DtMarqueeSelectData *pMarqueeData);
#endif /* _NO_PROTO */

/*************************************<->*************************************
 *
 *  Boolean _DtWsmIsBackdropWindow (display, screen_num, window)
 *
 *
 *  Description:
 *  -----------
 *  Returns true if the window passed in is a backdrop window.
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  screen_num	- number of screen we're interested in
 *  window	- window we want to test
 *
 *  Outputs:
 *  -------
 *  Return	- True if window is a backdrop window
 *                False otherwise.
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
Boolean 
#ifdef _NO_PROTO
_DtWsmIsBackdropWindow( );
#else
_DtWsmIsBackdropWindow(
        Display *display,
        int screen_num,
        Window window );
#endif /* _NO_PROTO */

/*************************************<->*************************************
 *
 *  status DtWsmSetWorkspaceTitle (widget, aWs, pchNewName)
 *
 *
 *  Description:
 *  -----------
 *  Rename a workspace
 *
 *
 *  Inputs:
 *  ------
 *  widget	- a widget
 *  aWs  	- atom of workspace 
 *  pchNewName	- new name for the workspace
 *
 *  Outputs:
 *  --------
 *  Return	- Success if communication to workspace manager
 *		  was successful.
 *
 *  Comments:
 *  ---------
 * 
 *************************************<->***********************************/
Status
#ifdef _NO_PROTO
_DtWsmSetWorkspaceTitle ();
#else
_DtWsmSetWorkspaceTitle (
	Widget widget,
        Atom aWs,
	char * pchNewName);
#endif /* _NO_PROTO */

/*************************************<->*************************************
 *
 *  status _DtWsmDeleteWorkspace (widget, aWs)
 *
 *
 *  Description:
 *  -----------
 *  Delete a workspace
 *
 *
 *  Inputs:
 *  ------
 *  widget	- a widget (with a window!)
 *  aWs  	- atom of workspace to delete
 *
 *  Outputs:
 *  --------
 *  Return	- Success if communication to workspace manager
 *		  was successful.
 *
 *  Comments:
 *  ---------
 * 
 *************************************<->***********************************/
Status
#ifdef _NO_PROTO
_DtWsmDeleteWorkspace ();
#else
_DtWsmDeleteWorkspace (
	Widget widget,
        Atom aWs);
#endif /* _NO_PROTO */


/*************************************<->*************************************
 *
 *  status _DtWsmCreateWorkspace (widget, pchTitle)
 *
 *
 *  Description:
 *  -----------
 *  Add a workspace
 *
 *
 *  Inputs:
 *  ------
 *  widget	- a widget (with a window!)
 *  pchTitle	- user-visible title of the workspace
 *
 *  Outputs:
 *  --------
 *  Return	- Success if communication to workspace manager
 *		  was successful.
 *
 *  Comments:
 *  ---------
 * 
 *************************************<->***********************************/
Status
#ifdef _NO_PROTO
_DtWsmCreateWorkspace ();
#else
_DtWsmCreateWorkspace (Widget widget, char * pchTitle);
#endif /* _NO_PROTO */

/*************************************<->*************************************
 *
 *  _DtWsmSetWorkspaceHints (display, window, pWsHints)
 *
 *
 *  Description:
 *  -----------
 *  Set the contents of the _DT_WORKSPACE_HINTS property on a window
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  window	- window to get hints from
 *  pWsHints	- pointer to workspace hints
 *
 *  Comments:
 *  ---------
 *  The internal form of the property is a structure. The structure
 *  must be unwound and turned into a simple array of "long"s before
 *  being written out.
 *
 *  This function currently only deals with version 1 of the property
 *  structure. The passed in pWsHints->version is ignored.
 * 
 *************************************<->***********************************/
extern void 
#ifdef _NO_PROTO
_DtWsmSetWorkspaceHints();
#else
_DtWsmSetWorkspaceHints(
        Display *display,
        Window window,
        DtWorkspaceHints *pWsHints);
#endif /* _NO_PROTO */


/*************************************<->*************************************
 *
 *  _DtWsmGetWorkspaceHints (display, window, ppWsHints)
 *
 *
 *  Description:
 *  -----------
 *  Get the contents of the _DT_WORKSPACE_HINTS property from a window
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  window	- window to get hints from
 *  ppWsHints	- pointer to pointer to workspace hints
 *
 *  Outputs:
 *  *ppWsHints	- allocated workspace hints data.
 *
 *  Comments:
 *  ---------
 *  The internal form of the property is a structure. The property
 *  is read in and packed into data allocated for the structure.
 *  Free the workspace hints by calling _DtWsmFreeWorkspaceHints.
 *
 *  This function currently only deals with version 1 of the property
 *  structure. 
 * 
 *************************************<->***********************************/
extern Status 
#ifdef _NO_PROTO
_DtWsmGetWorkspaceHints();
#else
_DtWsmGetWorkspaceHints(
        Display *display,
        Window window,
        DtWorkspaceHints **ppWsHints);
#endif /* _NO_PROTO */

/*************************************<->*************************************
 *
 *  _DtWsmFreeWorkspaceHints (pWsHints)
 *
 *
 *  Description:
 *  -----------
 *  Free  a workspace hints structure returned from _DtWsmGetWorkspaceHints
 *
 *  Inputs:
 *  ------
 *  pWsHints	- pointer to workspace hints
 *
 *  Outputs:
 *
 *  Comments:
 *  ---------
 *************************************<->***********************************/
extern void 
#ifdef _NO_PROTO
_DtWsmFreeWorkspaceHints();
#else
_DtWsmFreeWorkspaceHints(
        DtWorkspaceHints *pWsHints);
#endif /* _NO_PROTO */

/*************************************<->*************************************
 *
 *  DtWsmCBContext * _DtWsmAddMarqueeSelectionCallback (widget, 
 *						 	marquee_select, 
 *							client_data)
 *
 *
 *  Description:
 *  -----------
 *  Register a function to be called when a marquee selection is made
 *
 *
 *  Inputs:
 *  ------
 *  widget		- widget for this client
 *  marquee_select	- function to call for marquee select
 *  client_data		- additional data to pass back to client when called.
 *
 *  Outputs:
 *  --------
 *  Return	- ptr to callback context data (opaque) 
 *
 *  Comments:
 *  ---------
 *  The callback context data ptr should be saved if you intend to
 *  removed this callback at some point in the future. 
 * 
 *************************************<->***********************************/
DtWsmCBContext 
#ifdef _NO_PROTO
_DtWsmAddMarqueeSelectionCallback ();
#else
_DtWsmAddMarqueeSelectionCallback (
	Widget				widget,
	DtWsmMarqueeSelectionProc	marquee_select,
	XtPointer			client_data);
#endif /* _NO_PROTO */

/*************************************<->*************************************
 *
 *  _DtWsmSetDtWmHints (display, window, pHints)
 *
 *
 *  Description:
 *  -----------
 *  Set the contents of the _DT_WM_HINTS property on a window
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  window	- window to set hints on
 *  pHints	- pointer the hints to set
 *
 *  Comments:
 *  ---------
 *  No error checking
 * 
 *************************************<->***********************************/
extern void 
#ifdef _NO_PROTO
_DtWsmSetDtWmHints();
#else
_DtWsmSetDtWmHints( Display *display, 
		Window window, 
		DtWmHints *pHints);
#endif /* _NO_PROTO */

/*************************************<->*************************************
 *
 *  status _DtWsmGetDtWmHints (display, window, ppDtWmHints)
 *
 *
 *  Description:
 *  -----------
 *  Get the contents of the _DT_WM_HINTS property on a window
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  window	- window to get hints from
 *  ppDtWmHints- pointer to a pointer to return
 *
 *  Outputs:
 *  --------
 *  *ppDtWmHints-points to the DtWmHints structure retrieved from
 *		  the window (NOTE: This should be freed using XFree)
 *
 *  Comments:
 *  ---------
 * 
 *************************************<->***********************************/
extern Status
#ifdef _NO_PROTO
_DtWsmGetDtWmHints();
#else
_DtWsmGetDtWmHints(
        Display *display,
        Window window,
        DtWmHints **ppDtWmHints);
#endif /* _NO_PROTO */

/*************************************<->*************************************
 *
 *  _DtWsmSelectionNameForScreen (scr)
 *
 *  Description:
 *  -----------
 *  Returns a string containing the selection name used for
 *  communication with the workspace manager on this screen
 *
 *
 *  Inputs:
 *  ------
 *  scr		- number of screen
 *
 *  Outputs:
 *  --------
 *  Return	- ptr to string with selection name (free with XtFree)
 *
 *  Comments:
 *  ---------
 *  Assumes the screen number is < 1000.
 * 
 *************************************<->***********************************/
extern String
#ifdef _NO_PROTO
_DtWsmSelectionNameForScreen ();
#else /* _NO_PROTO */
_DtWsmSelectionNameForScreen (int scr);
#endif /* _NO_PROTO */

extern Tt_callback_action
#ifdef _NO_PROTO
_DtWsmConsumeReply ();
#else
_DtWsmConsumeReply (
        Tt_message msg,
        Tt_pattern pat );
#endif /* _NO_PROTO */

#endif /* _Dt_WsmP_h */
/* Do not add anything after this endif. */
