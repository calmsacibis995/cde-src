/* $XConsortium: HyperText.c /main/cde1_maint/1 1995/07/17 17:42:57 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        HyperText.c
 **
 **   Project:     Text Graphic Display Library
 **
 **   Description: This body of code does all the work for hypertext links
 **
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

/*
 * system includes
 */
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#ifdef __hpux
#include <time.h>
#else /* SUN and IBM */
#ifdef	_AIX
#include <sys/select.h>
#endif
#include <sys/time.h>
#include <sys/types.h>
#endif
#include <unistd.h>
#include <sys/wait.h>
#include <Xm/DialogS.h>


/*
 * private includes
 */
#include "Canvas.h"
#include "HelpErrorP.h"
#include "DisplayAreaP.h"
#include "FontI.h"
#include "HourGlassI.h"
#include "HyperTextI.h"
#include "HelposI.h"

#ifdef NLS16
#endif

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO
static	void	ProcessEvents ();
#else
static	void	ProcessEvents (
			DtHelpDispAreaStruct	*pDAS,
			pid_t			 child_pid,
			Boolean			 child_flag);
#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/

/*********************************************************************
 *		Private Variables
 *********************************************************************/

typedef	struct {
	char	*link_spec;
	int	 type;
	char	*descrip;
} HyperList;

/*********************************************************************
 *		Private Functions
 *********************************************************************/
/*********************************************************************
 * ProcessEvents
 * 
 *    ProcessEvents will do a select on the socket until the child
 *       dies.
 *
 *********************************************************************/
static void
#ifdef _NO_PROTO
ProcessEvents ( pDAS, child_pid, child_flag )
    DtHelpDispAreaStruct	*pDAS;
    pid_t			 child_pid;
    Boolean			 child_flag;
#else
ProcessEvents (
    DtHelpDispAreaStruct	*pDAS,
    pid_t			 child_pid,
    Boolean			 child_flag)
#endif /* _NO_PROTO */
{
    int    result;

    int   rMask;
    int   myFd;
    pid_t pid;
    struct timeval *topPtr;
    struct timeval  toStruct;

    Display *dpy = XtDisplay (pDAS->dispWid);
    XEvent   event;


    /*
     * set a small time out.
     */
    toStruct.tv_usec = 500000;		/* 500 miliseconds */
    toStruct.tv_sec  = 0;
    topPtr = &toStruct;

    /*
     * get the socket's file descriptor
     */
    myFd = ConnectionNumber (dpy);

    do
      {
	rMask = (1 << (myFd % 32));		/* on t.o. select will clear */

	if (!XPending(dpy))
	  {
#ifdef	__hpux
	    result = select(myFd+1, &rMask, 0, 0, topPtr);
#else
	    result = select(myFd+1, ((fd_set *)&rMask), 0, 0, topPtr);
#endif

	    /*
	     * check to see if the select happened because of
	     * a system interrupt.
             */
	    /*
	     * otherwise an exposure event happened.
	     * fall through and XPending will be true,
	     * forcing us to go to the XmUpdateDisplay call.
	     */
	  }
	else
	  {
/*
 * TODO
 * Use XtDisplatchEvent????
 */
	    XNextEvent (dpy, &event);
	    if (event.type == Expose)
	        XmUpdateDisplay (pDAS->dispWid);
	  }

	/*
	 * check to see if the child is still going
	 */
	pid = waitpid (child_pid, (int *) 0, WNOHANG);
	if (pid == child_pid || pid == -1)
	    child_flag = True;

      } while (!child_flag);
}

/*********************************************************************
 *		Internal Public Functions
 *********************************************************************/
/*********************************************************************
 * _DtHelpExecProcedure
 * 
 *    _DtHelpExecProcedure will fork/exec the command passed in. It will
 *       then allow only exposure events to be processed until the
 *       child dies.
 *
 *********************************************************************/
void
#ifdef _NO_PROTO
_DtHelpExecProcedure ( pDAS, cmd )
	DtHelpDispAreaStruct	*pDAS;
	char *cmd;
#else
_DtHelpExecProcedure (
	DtHelpDispAreaStruct	*pDAS,
	char *cmd )
#endif /* _NO_PROTO */
{
    pid_t pid;
    XWindowAttributes attr;
    char	*pShell = "sh";
    Widget	 shellWidget;
    Boolean	 childFlag = False;
    pid_t	 childPid;

    /*
     * Turn on the wait cursor.
     */
    shellWidget = pDAS->dispWid;
    while (!XtIsSubclass(shellWidget, xmDialogShellWidgetClass))
    	shellWidget = XtParent(shellWidget);
    
     _DtHelpTurnOnHourGlass(shellWidget);
    /*
     * Get the window event mask via the window attributes. Remember it.
     * Set the Input to only Exposure events.
     */
    XGetWindowAttributes (XtDisplay(pDAS->dispWid), XtWindow (pDAS->dispWid),
								&attr);
    XSelectInput (XtDisplay(pDAS->dispWid), XtWindow (pDAS->dispWid),
								ExposureMask);

    XSync (XtDisplay(pDAS->dispWid), 0);

    /*
     * initialize the global flag and variable.
     */
    childPid  = -1;
    childFlag = False;

    /*
     * fork a child process.
     */
#ifdef __hpux
    childPid = vfork ();
#else
    childPid = fork ();
#endif /* __hpux */

    /*
     * If the child, exec the cmd with a shell parent
     * so if the cmd ends in an ampersand, the command
     * will be put in the background and the shell will
     * die and return, creating a SIGCLD for us to catch.
     */
    if (childPid == 0)
      {
	execlp (pShell, pShell, "-c", cmd, ((char *) 0));
	_exit (1);
      }

    /*
     * Check to make sure the vfork was successful.
     */
    if (childPid != -1)
      {
	/*
	 * check to see if the child is still going
	 */
	pid = waitpid (childPid, (int *) 0, WNOHANG);
	if (!(pid == childPid || pid == -1))
	    /*
	     * process the exposure events in a special routine.
	     */
	    ProcessEvents (pDAS, childPid, childFlag);
      }

    /*
     * reset the input mask
     */
    XSelectInput (XtDisplay(pDAS->dispWid), XtWindow (pDAS->dispWid),
						(attr.your_event_mask));
    /*
     * turn off the wait cursor
     */
    _DtHelpTurnOffHourGlass(shellWidget);

} /* End _DtHelpExecProcedure */

/*********************************************************************
 * Function: _DtHelpProcessHyperSelection
 *
 *    Determine if the user selected a segment that is a hypertext
 *        link. If so, call the appropriate function to process it.
 *
 *********************************************************************/
void
#ifdef _NO_PROTO
_DtHelpProcessHyperSelection (pDAS, downX, downY, event)
	DtHelpDispAreaStruct	*pDAS;
	int		 downX;
	int		 downY;
	XEvent		*event;
#else
_DtHelpProcessHyperSelection (
	DtHelpDispAreaStruct	*pDAS,
	int		 downX,
	int		 downY,
	XEvent		*event )
#endif /* _NO_PROTO */
{

    int	   upX;
    int	   upY;

    CEHyperTextCallbackStruct  ceHyper;
    DtHelpHyperTextStruct callData;

    if (pDAS->hyperCall == NULL)
	return;

    downY = downY + pDAS->firstVisible - pDAS->decorThickness;
    downX = downX + pDAS->virtualX     - pDAS->decorThickness;
    upX   = event->xbutton.x + pDAS->virtualX     - pDAS->decorThickness;
    upY   = event->xbutton.y + pDAS->firstVisible - pDAS->decorThickness;

    /*
     * turn off the old traversal
     */
    if (pDAS->neededFlags & _DT_HELP_FOCUS_FLAG)
      {
        pDAS->traversal |= _DT_HELP_DRAW_TOC_IND;
        _DtHelpCeMoveTraversal(pDAS->canvas, CETraversalOff, False,
				(XtIsRealized(pDAS->dispWid) ? True : False),
					NULL, NULL, NULL, NULL);
      }

    if (_DtHelpCeProcessLink(pDAS->canvas, downX, downY, upX, upY, &ceHyper))
      {
	/*
	 * turn the traversal on for the selected hyptext link.
	 */
	if (pDAS->neededFlags & _DT_HELP_FOCUS_FLAG)
	  {
	    pDAS->traversal |= _DT_HELP_DRAW_TOC_IND;
            _DtHelpCeMoveTraversal(pDAS->canvas, CETraversalOn, False,
				(XtIsRealized(pDAS->dispWid) ? True : False),
					NULL, NULL, NULL, NULL);
          }

	callData.reason        = XmCR_ACTIVATE;
	callData.event         = event;
	callData.window        = XtWindow (pDAS->dispWid);
	callData.specification = ceHyper.specification;
	callData.hyper_type    = ceHyper.hyper_type;
	callData.window_hint   = ceHyper.win_hint;

	(*(pDAS->hyperCall)) (pDAS, pDAS->clientData, &callData);
      }
    else if (pDAS->neededFlags & _DT_HELP_FOCUS_FLAG)
	/*
	 * turn the traversal back on
	 */
        _DtHelpCeMoveTraversal(pDAS->canvas, CETraversalOn, False,
				(XtIsRealized(pDAS->dispWid) ? True : False),
					NULL, NULL, NULL, NULL);

}  /* End _DtHelpProcessHyperSelection */

