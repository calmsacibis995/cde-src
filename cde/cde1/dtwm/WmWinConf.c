/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: WmWinConf.c /main/cde1_maint/2 1995/10/03 17:26:38 lehors $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */
#include "WmGlobal.h"	/* This should be the first include */
#include <X11/X.h>

#define XK_MISCELLANY
#include <X11/keysymdef.h>


#define MOVE_OUTLINE_WIDTH	2

#define CONFIG_MASK (KeyPressMask|ButtonPressMask|\
			 ButtonReleaseMask|PointerMotionMask)
#define PGRAB_MASK (ButtonPressMask|ButtonReleaseMask|\
		    PointerMotionMask|PointerMotionHintMask)

/* grab types */

#define NotGrabbed	0
#define ResizeGrab	1
#define MoveGrab	2
#ifdef WSM

/* Anchors */
#define ANCHOR_NONE	0
#define ANCHOR_NW	1
#define ANCHOR_NE	2
#define ANCHOR_SE	3
#define ANCHOR_SW	4

#ifndef ABS
#define ABS(x) ((x)>0?(x):(-(x)))
#endif /* ABS */
#endif /* WSM */

/* number of times to poll before blocking on a config event */

#define CONFIG_POLL_COUNT	300

/* mask for all buttons */
#define ButtonMask	\
    (Button1Mask|Button2Mask|Button3Mask|Button4Mask|Button5Mask)

/*
 * include extern functions
 */
#include "WmWinConf.h"
#include "WmCDInfo.h"
#include "WmCDecor.h"
#include "WmCPlace.h"
#include "WmEvent.h"
#include "WmFeedback.h"
#include "WmFunction.h"
#include "WmIDecor.h"
#include "WmIPlace.h"
#include "WmIconBox.h"
#include "WmKeyFocus.h"
#include "WmProtocol.h"
#include "WmWinInfo.h"



/*
 * Global Variables:
 *
 * These statics are set up at the initiation of a configuration 
 * operation and used for succeeding events.
 */

static int pointerX = -1;
static int pointerY = -1;

static int offsetX = 0;
static int offsetY = 0;

static int resizeX, resizeY;	/* root coords of UL corner of frame */
static unsigned int resizeWidth, resizeHeight;	/* size of frame */
static unsigned int resizeBigWidthInc, resizeBigHeightInc;
static int startX, startY; 
static unsigned int startWidth, startHeight;
static unsigned int minWidth, minHeight, maxHeight, maxWidth;
#ifdef WSM
static int marqueeX, marqueeY;	/* root coords of UL corner of are */
static long marqueeWidth, marqueeHeight;	/* size of area */
static unsigned int marqueeAnchor;	/* id of anchor corner */
static long marqueeWidth0, marqueeHeight0;	/* old size of area */
#endif /* WSM */

static int opaqueMoveX = 0;    /* for cancel request on opaque moves */
static int opaqueMoveY = 0;
static int moveX = 0;		/* root coords of UL corner of frame */
static int moveY = 0;
static int moveIBbbX = 0;	/* root coords of icon box bulletin board */
static int moveIBbbY = 0;
static unsigned int moveWidth = 0;	/* size of frame */
static unsigned int moveHeight = 0;
static int moveLastPointerX = 0;	/* last pointer position */
static int moveLastPointerY= 0;

static Boolean anyMotion = FALSE;
static Boolean configGrab = FALSE;

Dimension clipWidth = 0;
Dimension clipHeight = 0;
Position clipX = 0;
Position clipY = 0;

 /*
  * Types and state related to edgeMoveThreshold resource
  */

 typedef struct EdgeMoveState_ {
     int       pendDx;         /* X and Y amounts accumulated while stuck */
     int       pendDy;
 } EdgeMoveState;

 EdgeMoveState emState;




/*************************************<->*************************************
 *
 *  GetClipDimensions (pcd, fromRoot)
 *
 *
 *  Description:
 *  -----------
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *************************************<->***********************************/
#ifdef _NO_PROTO
void GetClipDimensions (pCD, fromRoot)
	ClientData *pCD;
	Boolean fromRoot;
#else /* _NO_PROTO */
void GetClipDimensions (ClientData *pCD, Boolean fromRoot)
#endif /* _NO_PROTO */
{

    int i;
    Arg getArgs[5];
    Position tmpX, tmpY;

    i=0;
    XtSetArg (getArgs[i], XmNwidth, (XtArgVal) &clipWidth ); i++;
    XtSetArg (getArgs[i], XmNheight, (XtArgVal) &clipHeight ); i++;
    XtSetArg (getArgs[i], XmNx, (XtArgVal) &tmpX ); i++;
    XtSetArg (getArgs[i], XmNy, (XtArgVal) &tmpY ); i++;

    XtGetValues (P_ICON_BOX(pCD)->clipWidget, getArgs, i);

    if (fromRoot)
    {
	XtTranslateCoords(P_ICON_BOX(pCD)->scrolledWidget,
			tmpX, tmpY,
			&clipX, &clipY);
    }
    else
    {
	clipX = tmpX;
	clipY = tmpY;      
    }			

} /* END OF FUNCTION GetClipDimensions */



/*************************************<->*************************************
 *
 *  HandleClientFrameMove (pcd, pev)
 *
 *
 *  Description:
 *  -----------
 *  Provide visual feedback of interactive moving of the window.
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *  pev		- pointer to event
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *************************************<->***********************************/
#ifdef _NO_PROTO
void HandleClientFrameMove (pcd, pev)

	ClientData *pcd;
	XEvent *pev;
#else /* _NO_PROTO */
void HandleClientFrameMove (ClientData *pcd, XEvent *pev)
#endif /* _NO_PROTO */
{
    int tmpX, tmpY, warpX, warpY;
    Window grab_win;
    KeySym keysym;
    Boolean control;
    Boolean firstTime;
    int big_inc, keyMultiplier;
    int newX, newY;
    XEvent event, KeyEvent;

    if (pev) {
	firstTime = True;
    }
    else {
	firstTime = False;
    }

    big_inc = DisplayWidth (DISPLAY, SCREEN_FOR_CLIENT(pcd)) / 20;


    /*
     *	Do our grabs and initial setup if we're just starting out
     */
    if (!configGrab) {
	if (!StartClientMove (pcd, pev))
	{
	    /* configuration was not initiated */
	    return;
	}
    }

    grab_win = GrabWin (pcd, pev);

    
    if (pcd->pSD->useIconBox && P_ICON_BOX(pcd))
    {
	GetClipDimensions (pcd, True);
    }

    for ( ;; )
    {
	tmpX = tmpY = 0;

	if (firstTime) {
	    /* handle the event we were called with first */
	    firstTime = False;
	}
	else 
	{
	    pev = &event;
	    GetConfigEvent(DISPLAY, grab_win, CONFIG_MASK, 
		moveLastPointerX, moveLastPointerY, moveX, moveY,
		moveWidth, moveHeight, &event);
	}

	if (pev->type == KeyPress) 
	{
	    keyMultiplier = 1;
	    while (keyMultiplier <= big_inc && 
		      XCheckIfEvent (DISPLAY, &KeyEvent, IsRepeatedKeyEvent, 
		      (char *) pev))
	    {
		  keyMultiplier++;
	    }

	    keysym = XKeycodeToKeysym (DISPLAY, pev->xkey.keycode, 0);
	    control = (pev->xkey.state & ControlMask) != 0;
	    tmpX = tmpY = 0;

	    switch (keysym) {
		case XK_Left:
		    tmpX = keyMultiplier * ((control) ? (-big_inc) : (-1));
		    break;

		case XK_Up:
		    tmpY = keyMultiplier * ((control) ? (-big_inc) : (-1));
		    break;

		case XK_Right:
		    tmpX = keyMultiplier * ((control) ? big_inc : 1);
		    break;

		case XK_Down:
		    tmpY = keyMultiplier * ((control) ? big_inc : 1);
		    break;

		case XK_Return:
		    CompleteFrameConfig (pcd, pev);
		    return;

		case XK_Escape:
		    CancelFrameConfig (pcd);
		    CheckEatButtonRelease (pcd, pev);
		    return;

		default:
		    break;
	    }

            CheckEdgeMove (pcd, False, moveX, moveY, moveWidth, moveHeight,
                       &tmpX, &tmpY);
            if (tmpX == 0 && tmpY == 0)
               continue;

            warpX = moveLastPointerX + tmpX;
            warpY = moveLastPointerY + tmpY;
 
            ForceOnScreen(SCREEN_FOR_CLIENT(pcd), &warpX, &warpY);

            if ((warpX != moveLastPointerX) || (warpY != moveLastPointerY))
            {
                SetPointerPosition (warpX, warpY, &newX, &newY);
 
                tmpX = newX - moveLastPointerX;
                tmpY = newY - moveLastPointerY;
                moveLastPointerX = newX;
                moveLastPointerY = newY;
                moveX += tmpX;
                moveY += tmpY;
            }
	    else 
	    {
		/*
		 * make like motion event and move frame.
		 */
		 moveX += tmpX;
		 moveY += tmpY;
	   }
	}
	else if (pev->type == ButtonRelease) 
	{
	    /*
	     *  Update (x,y) to the location of the button release
	     */
            tmpX = pev->xmotion.x_root - moveLastPointerX;
            tmpY = pev->xmotion.y_root - moveLastPointerY;
            CheckEdgeMove (pcd, False, moveX, moveY, moveWidth, moveHeight,
                        &tmpX, &tmpY);
            moveX += tmpX;
            moveY += tmpY;

	    CompleteFrameConfig (pcd, pev);
	    return;
	}
	else if (pev->type == MotionNotify) 
	{	
	    tmpX = pev->xmotion.x_root - moveLastPointerX;
	    tmpY = pev->xmotion.y_root - moveLastPointerY;
            CheckEdgeMove (pcd, False, moveX, moveY, moveWidth, moveHeight,
                        &tmpX, &tmpY);

	    moveLastPointerX = pev->xmotion.x_root;
	    moveLastPointerY = pev->xmotion.y_root;

            if (tmpX == 0 && tmpY == 0)
               continue;

	    moveX += tmpX;
	    moveY += tmpY;
	    anyMotion = True;
	}

	/* draw outline if there is something to draw */
	if (tmpX || tmpY) {
	    FixFrameValues (pcd, &moveX, &moveY, &moveWidth, &moveHeight,
			    FALSE /* no size checks */);
	    if (pcd->pSD->moveOpaque)
	    {
		MoveOpaque (pcd, moveX, moveY, moveWidth, moveHeight);
	    }
	    else
	    {
		MoveOutline(moveX, moveY, moveWidth, moveHeight);
	    }
	    
	    if ( !wmGD.movingIcon &&
		 (wmGD.showFeedback & WM_SHOW_FB_MOVE))
	    {
		DoFeedback (pcd, moveX, moveY, moveWidth, moveHeight, 
			    (unsigned long) 0, FALSE /* no size checks */);
	    }
	}
    }

} /* END OF FUNCTION HandleClientFrameMove */



/*************************************<->*************************************
 *
 *  UpdateAndDrawResize ()
 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *************************************<->***********************************/
#ifdef _NO_PROTO
void UpdateAndDrawResize (pcd)
	ClientData *pcd;
#else /* _NO_PROTO */
void UpdateAndDrawResize (ClientData *pcd)
#endif /* _NO_PROTO */
{
    int tmpHeight, tmpWidth;
    
    /* 
     * Handle a motion event or a keypress that's like a motion 
     * event
     */
    
    /* set height */
    
    switch (wmGD.configPart) {
      case FRAME_RESIZE_NW:
      case FRAME_RESIZE_N:
      case FRAME_RESIZE_NE:
	tmpHeight = (int) startHeight + (startY - pointerY);
	if (tmpHeight < (int) minHeight)
	{
	    resizeHeight = minHeight;
	    resizeY = startY + startHeight - minHeight;
	}
	else if (pcd->pSD->limitResize 
		 && (tmpHeight > (int) maxHeight)
		 && (!(pcd->clientFlags & ICON_BOX)))
	{
	    resizeHeight = maxHeight;
	    resizeY = startY + startHeight - maxHeight;
	}
	else
	{
	    resizeHeight = (unsigned int) tmpHeight;
	    resizeY = pointerY;
	}
	break;
	
      case FRAME_RESIZE_SW:
      case FRAME_RESIZE_S:
      case FRAME_RESIZE_SE:
	resizeY = startY;
	tmpHeight = pointerY - startY + 1;
	if (tmpHeight < (int) minHeight)
	{
	    resizeHeight = minHeight;
	}
	else if (pcd->pSD->limitResize 
		 && (tmpHeight > (int) maxHeight)
		 && (!(pcd->clientFlags & ICON_BOX)))
	{
	    resizeHeight = maxHeight;
	}
	else
	{
	    resizeHeight = (unsigned int) tmpHeight;
	}
	break;
	
      default:
	resizeY = startY;
	resizeHeight = startHeight;
	break;
	
    }
    
    /* set width */
    
    switch (wmGD.configPart) {
      case FRAME_RESIZE_NW:
      case FRAME_RESIZE_W:
      case FRAME_RESIZE_SW:
	tmpWidth = (int) startWidth + (startX - pointerX);
	if (tmpWidth < (int) minWidth)
	{
	    resizeWidth = minWidth;
	    resizeX = startX + startWidth - minWidth;
	}
	else if (pcd->pSD->limitResize 
		 && (tmpWidth > (int) maxWidth)
		 && (!(pcd->clientFlags & ICON_BOX)))
	{
	    resizeWidth = maxWidth;
	    resizeX = startX + startWidth - maxWidth;
	}
	else
	{
	    resizeWidth = (unsigned int) tmpWidth;
	    resizeX = pointerX;
	}
	break;
	
      case FRAME_RESIZE_NE:
      case FRAME_RESIZE_E:
      case FRAME_RESIZE_SE:
	resizeX = startX;
	tmpWidth = pointerX - startX + 1;
	if (tmpWidth < (int) minWidth)
	{
	    resizeWidth = minWidth;
	}
	else if (pcd->pSD->limitResize 
		 && (tmpWidth > (int) maxWidth)
		 && (!(pcd->clientFlags & ICON_BOX)))
	{
	    resizeWidth = maxWidth;
	}
	else
	{
	    resizeWidth = (unsigned int) tmpWidth;
	}
	break;
	
      default:
	resizeX = startX;
	resizeWidth = startWidth;
	break;
    }
    
    FixFrameValues (pcd, &resizeX, &resizeY, &resizeWidth, 
		    &resizeHeight, TRUE /* do size checks */);
    MoveOutline (resizeX, resizeY, resizeWidth, resizeHeight);
    if (wmGD.showFeedback & WM_SHOW_FB_RESIZE)
    {
	DoFeedback(pcd, resizeX, resizeY, resizeWidth, resizeHeight,
		   (unsigned long) 0, TRUE /* do size checks */);
    }
}


/*************************************<->*************************************
 *
 *  HandleClientFrameResize (pcd, pev)
 *
 *
 *  Description:
 *  -----------
 *  Provide visual feedback of interactive resizing of the window.
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *  pev		- pointer to event
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  o The window sizes refer to the frame, not the client window.
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void HandleClientFrameResize (pcd, pev)

	ClientData *pcd;
	XEvent *pev;

#else /* _NO_PROTO */
void HandleClientFrameResize (ClientData *pcd, XEvent *pev)
#endif /* _NO_PROTO */
{
    Window grab_win;
    Boolean resizeDone;
    XEvent event;


    /*
     * Do our grabs the first time through
     */
    if (!configGrab) {
	if (StartResizeConfig (pcd, pev))
	{
	    configGrab = TRUE;
	}
	else
	{
	    /* resize could not be initiated */
	    return;
	}
    }

    grab_win = GrabWin (pcd, pev);

    resizeDone = False;
    while (!resizeDone) 
    {
	if (!pev) 	/* first time through will already have event */
	{
	    pev = &event;

	    GetConfigEvent(DISPLAY, grab_win, CONFIG_MASK, 
		pointerX, pointerY, resizeX, resizeY,
		resizeWidth, resizeHeight, &event);
	}

	if (pev->type == MotionNotify)
	{
	    pointerX = pev->xmotion.x_root;
	    pointerY = pev->xmotion.y_root;
	    anyMotion = TRUE;

	    /*
	     * Really start resizing once the pointer hits a resize area
	     * (This only applies to accelerator and keyboard resizing!)
	     */
	    if (!wmGD.configSet && !SetPointerResizePart (pcd, pev)) {
		pev = NULL;
		continue;		/* ignore this event */
	    }
	}
	else if (pev->type == KeyPress) {

	    /* 
	     * Handle key event. 
	     */
	    resizeDone = HandleResizeKeyPress (pcd, pev);
	}
	else if (pev->type == ButtonRelease) {

	    /*
	     *  Update (x,y) to the location of the button release
	     */
	    pointerX = pev->xbutton.x_root;
	    pointerY = pev->xbutton.y_root;
	    UpdateAndDrawResize(pcd);

	    CompleteFrameConfig (pcd, pev);
	    resizeDone = True;
	}
	else  {
	    pev = NULL;
	    continue;			/* ignore this event */
	}

	if (!resizeDone)
	{
	    UpdateAndDrawResize(pcd);
	}

	pev = NULL;	/* reset event pointer */

    }  /* end while */

} /* END OF FUNCTION HandleClientFrameResize */



/*************************************<->*************************************
 *
 *  HandleResizeKeyPress (pcd, pev)
 *
 *
 *  Description:
 *  -----------
 *  Handles keypress events during resize of window
 *
 *
 *  Inputs:
 *  ------
 *  pcd 	- pointer to client data
 *  pev		- pointer to event
 *
 * 
 *  Outputs:
 *  -------
 *  Return	- True if this event completes (or cancels) resizing 
 * 
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
Boolean HandleResizeKeyPress (pcd, pev) 

	ClientData *pcd;
	XEvent *pev;
#else /* _NO_PROTO */
Boolean HandleResizeKeyPress (ClientData *pcd, XEvent *pev)
#endif /* _NO_PROTO */
{
    KeySym keysym;
    Boolean control;
    int warpX, warpY, currentX, currentY, newX, newY;
    int junk, keyMult;
    Window junk_win;
    XEvent KeyEvent;

    /*
     * Compress repeated keys 
     */
    keyMult = 1;
    while (keyMult <= 10 && 
	      XCheckIfEvent (DISPLAY, &KeyEvent, IsRepeatedKeyEvent, 
	      (char *) pev))
    {
	  keyMult++;
    }

    keysym = XKeycodeToKeysym (DISPLAY, pev->xkey.keycode, 0);
    control = (pev->xkey.state & ControlMask) != 0;

    switch (keysym) {
	case XK_Left:
	    switch (wmGD.configPart) {
		case FRAME_NONE:
		    wmGD.configPart = FRAME_RESIZE_W;
		    ReGrabPointer(pcd->clientFrameWin, pev->xkey.time);
		    warpY = resizeY + resizeHeight/2;
		    warpX = resizeX + ((control) ? 
					  (-resizeBigWidthInc) : 
					  (-pcd->widthInc));
		    break;

		case FRAME_RESIZE_N:
		    wmGD.configPart = FRAME_RESIZE_NW;
		    ReGrabPointer(pcd->clientFrameWin, pev->xkey.time);
		    warpX = resizeX + ((control) ? 
					  (-resizeBigWidthInc) : 
					  (-pcd->widthInc));
		    warpY = pointerY;
		    break;

		case FRAME_RESIZE_S:
		    wmGD.configPart = FRAME_RESIZE_SW;
		    ReGrabPointer(pcd->clientFrameWin, pev->xkey.time);
		    warpX = resizeX + ((control) ? 
					  (-resizeBigWidthInc) : 
					  (-pcd->widthInc));
		    warpY = pointerY;
		    break;
		
		default:
		    warpX = pointerX + ((control) ? 
					(-resizeBigWidthInc * keyMult) : 
					(-pcd->widthInc * keyMult));
		    warpY = pointerY;
		    break;
	    }
	    break;

	case XK_Up:
	    switch (wmGD.configPart) {
		case FRAME_NONE:
		    wmGD.configPart = FRAME_RESIZE_N;
		    warpX = resizeX + resizeWidth/2;
		    warpY = resizeY + ((control) ? 
					  (-resizeBigHeightInc) : 
					  (-pcd->heightInc));
		    ReGrabPointer(pcd->clientFrameWin, pev->xkey.time);
		    break;

		case FRAME_RESIZE_W:
		    wmGD.configPart = FRAME_RESIZE_NW;
		    ReGrabPointer(pcd->clientFrameWin, pev->xkey.time);
		    warpX = pointerX;
		    warpY = resizeY + ((control) ? 
					  (-resizeBigHeightInc) : 
					  (-pcd->heightInc));
		    break;

		case FRAME_RESIZE_E:
		    wmGD.configPart = FRAME_RESIZE_NE;
		    ReGrabPointer(pcd->clientFrameWin, pev->xkey.time);
		    warpX = pointerX;
		    warpY = resizeY + ((control) ? 
					      (-resizeBigHeightInc) : 
					      (-pcd->heightInc));
		    break;

		default: 
		    warpX = pointerX;
		    warpY = pointerY + ((control) ? 
					(-resizeBigHeightInc * keyMult) : 
					(-pcd->heightInc * keyMult));
		    break;
	    }
	    break;

	case XK_Right:
	    switch (wmGD.configPart) {
		case FRAME_NONE:
		    wmGD.configPart = FRAME_RESIZE_E;
		    warpY = resizeY + resizeHeight/2;
		    warpX = resizeX + resizeWidth - 1 + 
			       ((control) ? resizeBigWidthInc : 
					    pcd->widthInc);
		    ReGrabPointer(pcd->clientFrameWin, pev->xkey.time);
		    break;

		case FRAME_RESIZE_N:
		    wmGD.configPart = FRAME_RESIZE_NE;
		    ReGrabPointer(pcd->clientFrameWin, pev->xkey.time);
		    warpX = resizeX + resizeWidth - 1 + 
			       ((control) ? resizeBigWidthInc : 
					    pcd->widthInc);
		    warpY = pointerY;
		    break;

		case FRAME_RESIZE_S:
		    wmGD.configPart = FRAME_RESIZE_SE;
		    ReGrabPointer(pcd->clientFrameWin, pev->xkey.time);
		    warpX = resizeX + resizeWidth - 1 + 
			       ((control) ? resizeBigWidthInc : 
					    pcd->widthInc);
		    warpY = pointerY;
		    break;

		default:
		    warpX = pointerX + ((control) ? 
				 	(resizeBigWidthInc * keyMult) : 
				  	(pcd->widthInc * keyMult));
		    warpY = pointerY;
		    break;
	    }
	    break;

	case XK_Down:
	    switch (wmGD.configPart) {
		case FRAME_NONE:
		    wmGD.configPart = FRAME_RESIZE_S;
		    warpX = resizeX + resizeWidth/2;
		    warpY = resizeY + resizeHeight - 1 + 
			       ((control) ? resizeBigHeightInc : 
					    pcd->heightInc);
		    ReGrabPointer(pcd->clientFrameWin, pev->xkey.time);
		    break;

		case FRAME_RESIZE_E:
		    wmGD.configPart = FRAME_RESIZE_SE;
		    ReGrabPointer(pcd->clientFrameWin, pev->xkey.time);
		    warpX = pointerX;
		    warpY = resizeY + resizeHeight - 1 + 
			       ((control) ? resizeBigHeightInc : 
					    pcd->heightInc);
		    break;

		case FRAME_RESIZE_W:
		    wmGD.configPart = FRAME_RESIZE_SW;
		    ReGrabPointer(pcd->clientFrameWin, pev->xkey.time);
		    warpX = pointerX;
		    warpY = resizeY + resizeHeight - 1 + 
			       ((control) ? resizeBigHeightInc : 
					    pcd->heightInc);
		    break;

		default:
		    warpX = pointerX;
		    warpY = pointerY + ((control) ? 
					(resizeBigHeightInc * keyMult) : 
					(pcd->heightInc * keyMult));
		    break;
	    }
	    break;

	case XK_Return:
	    CompleteFrameConfig (pcd, pev);
	    return (True);

	case XK_Escape:
	    CancelFrameConfig (pcd);
	    CheckEatButtonRelease (pcd, pev);
	    return (True);

	default:
	    return (False);		/* ignore this key */

    } /* end switch(keysym) */

    /*
     * Make sure the new pointer position is on screen before doing
     * the warp. Warp only if the pointer position changes.
     */
    pointerX = warpX;
    pointerY = warpY;

    ForceOnScreen(SCREEN_FOR_CLIENT(pcd), &warpX, &warpY);

    /*
     * Don't query pointer if enable warp is off.
     */
    if (!wmGD.enableWarp ||
	XQueryPointer (DISPLAY, ROOT_FOR_CLIENT(pcd), &junk_win, &junk_win,
	       &currentX, &currentY, &junk, &junk, (unsigned int *)&junk))
    {
	if ( (warpX != currentX) || (warpY != currentY) ) 
	{
	    SetPointerPosition (warpX, warpY, &newX, &newY);
	    return (False);
	}
    }
    return (False);

} /* END OF FUNCTION HandleResizeKeyPress */


/*************************************<->*************************************
 *
 *  DoFeedback (pcd, x, y, width, height, newStyle, resizing)
 *
 *
 *  Description:
 *  -----------
 *  Start or update feedback of size/position info
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *  x		-
 *  y		-
 *  width	- 
 *  height	-
 *  newStyle	- style flags. 
 *  resizing    - check size constraints iff TRUE
 *  
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  o If newStyle has FB_POSITION  and/or FB_SIZE bits set, then it is
 *    assumed that this is an initial call and a feedback window of the
 *    desired style should be popped up. If newStyle is zero, then it
 *    is assumed that the feedback window is already up and the values
 *    passed in are updates.
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void DoFeedback(pcd, x, y, width, height, newStyle, resizing)

    ClientData *pcd;
    int x, y; 
    unsigned int width, height;
    unsigned long newStyle;
    Boolean     resizing;
#else /* _NO_PROTO */
void DoFeedback (ClientData *pcd, int x, int y, unsigned int width, unsigned int height, unsigned long newStyle, Boolean resizing)
#endif /* _NO_PROTO */
{
    int cx = x;
    int cy = y;
    unsigned int cwidth, cheight;

    /* compute client window coordinates from frame coordinates */
    FrameToClient (pcd, &cx, &cy, &width, &height);

    /* use frame (not client) position if user wishes it */
    if (wmGD.positionIsFrame) {
	cx = x;
	cy = y;
    }

    /* If resizing, make sure configuration is valid. */
    if (resizing)
    {
        FixWindowConfiguration (pcd, &width, &height,
				 (unsigned int) pcd->widthInc, 
				 (unsigned int) pcd->heightInc);
    }

    /*
     * Put size in client specific units.  Do not include base into calculations
     * when increment is not specified (i.e. = 1).
     */
    cwidth = (width - ((pcd->widthInc==1) ? 0 : pcd->baseWidth))
		/ pcd->widthInc;
    cheight = (height - ((pcd->heightInc==1) ? 0 : pcd->baseHeight))
		/ pcd->heightInc;

    if (newStyle) {
	ShowFeedbackWindow (pcd->pSD, cx, cy, cwidth, cheight, newStyle);
    }
    else {
	UpdateFeedbackInfo (pcd->pSD, cx, cy, cwidth, cheight);
    }
} /* END OF FUNCTION DoFeedback  */



/*************************************<->*************************************
 *
 *  CheckVisualPlace
 *
 *
 *  Description:
 *  -----------
 *  Prevents icons in the icon box from being moved outside the clip window
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data

 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean CheckVisualPlace (pCD, tmpX, tmpY)
	ClientData *pCD;
	int tmpX;
	int tmpY;

#else /* _NO_PROTO */
Boolean CheckVisualPlace (ClientData *pCD, int tmpX, int tmpY)
#endif /* _NO_PROTO */
{
    Boolean rval = True;
    Window child;
    int newX;
    int newY;
    
    GetClipDimensions(pCD, True);


    /* 
     * Get root coordinates of X and Y for icon.
     * We use root coordinates of clip window since clipX and
     * clipY are not 0, but the icon X and Y may be 0 in
     * local coordinates
     */
     
    XTranslateCoordinates(DISPLAY, XtWindow(P_ICON_BOX(pCD)->bBoardWidget),
    			  ROOT_FOR_CLIENT(pCD), tmpX, tmpY,
			  &newX, &newY, &child);


    if (newX < clipX) 
    {
        return(False);
    }
    if (newY < clipY) 
    {
        return(False);
    }


    if (((int)newX) > ((int)clipX + 
    	(int)clipWidth - ((int)ICON_WIDTH(pCD)))) 
    {
        return(False);
    }
    if (((int)newY) > ((int)clipY + 
    	(int)clipHeight - ((int)ICON_HEIGHT(pCD))))
    {
        return(False);
    }
    
    return (rval);
 
} /* END OF FUNCTION CheckVisualPlace */



/*************************************<->*************************************
 *
 *  CompleteFrameConfig (pcd, pev)
 *
 *
 *  Description:
 *  -----------
 *  Clean up graphic feedback when user stops configuring.
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *  pev		- pointer to event
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  o This routine assumes that it is called in response to a button release
 *    event.
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void CompleteFrameConfig (pcd, pev)

	ClientData *pcd;
	XEvent *pev;
#else /* _NO_PROTO */
void CompleteFrameConfig (ClientData *pcd, XEvent *pev)
#endif /* _NO_PROTO */
{
    unsigned int tmpWidth, tmpHeight;
    int tmpX, tmpY;
    Boolean inIconBox;
    
    
    if (wmGD.configAction == RESIZE_CLIENT) {
	/* release the grabs */
	UndoGrabs();

	/*
	 * Honor the implied constrained anchor points on the window
	 * so that the resize doesn't cause the window to move 
	 * unexpectedly.
	 */

#ifndef CONFIG_RELATIVE_TO_CLIENT

	tmpX = resizeX;
	tmpY = resizeY;

	/* Use dummy x,y so we don't add frame offset to client location */
	FrameToClient (pcd, &tmpX, &tmpY, &resizeWidth, &resizeHeight);
#else
	FrameToClient (pcd, &resizeX, &resizeY, &resizeWidth, &resizeHeight);
#endif

	tmpWidth = resizeWidth;
	tmpHeight = resizeHeight;

	FixWindowConfiguration (pcd, &tmpWidth, &tmpHeight,
				     (unsigned int) pcd->widthInc, 
				     (unsigned int) pcd->heightInc);

        AdjustPos (&resizeX, &resizeY,
		   resizeWidth, resizeHeight, tmpWidth, tmpHeight);

	/* reconfigure the window(s) */
	ProcessNewConfiguration (pcd, resizeX, resizeY, 
				 resizeWidth, resizeHeight, FALSE);

    }
    else if (wmGD.configAction == MOVE_CLIENT)
    {
	/* release the grabs */
	UndoGrabs();

	/* make sure title bar is popped out */
	if ((wmGD.configAction == MOVE_CLIENT) &&
	    (wmGD.gadgetClient == pcd) && 
	    (wmGD.gadgetDepressed == FRAME_TITLE))
	{
	    PopGadgetOut (pcd, FRAME_TITLE);
	    FrameExposureProc(pcd);			/* repaint frame */
	}

	/* handle both icon and normal frames */
	if (wmGD.movingIcon)
	{

	    inIconBox = (pcd->pSD->useIconBox && P_ICON_BOX(pcd));

	    /* only need to move the icon */
	    if (wmGD.iconAutoPlace || inIconBox)
	    {
		int centerX;
		int centerY;
		int place;
		IconPlacementData *pIPD;

		/* 
		 * Get correct icon placement data
		 */
		if (inIconBox) 
		{
		    pIPD = &P_ICON_BOX(pcd)->IPD;
		    moveX -= moveIBbbX;
		    moveY -= moveIBbbY;
		}
		else
		{
		    pIPD = &(ACTIVE_WS->IPData);
		}

                /*
		 * Check to make sure that there is an unoccupied place
		 * where the icon is being moved to:
		 */

		centerX = moveX + ICON_WIDTH(pcd) / 2;
		centerY = moveY + ICON_HEIGHT(pcd) / 2;
		place = CvtIconPositionToPlace (pIPD, centerX, centerY);

		if (place != ICON_PLACE(pcd))
		{
		    if (pIPD->placeList[place].pCD)
		    {
			/*
			 * Primary place occupied, try to find an unoccupied
			 * place in the proximity.
			 */

#ifdef FWS
			place = FindIconPlace (ACTIVE_WS, pcd, pIPD,
					       centerX, centerY);
#else
			place = FindIconPlace (pcd, pIPD, centerX, centerY);
#endif /* FWS */
			if (place == NO_ICON_PLACE)
			{
			    /*
			     * Can't find an unoccupied icon place.
			     */

			    F_Beep (NULL, pcd, (XEvent *)NULL);

			    if (pcd->pSD->moveOpaque && !inIconBox)
			    {
				/*
				 * Replace icon into same place - as if it
				 * didn't move.
				 */
				
				XMoveWindow (DISPLAY, ICON_FRAME_WIN(pcd),
					     ICON_X(pcd), ICON_Y(pcd));
				if ((ICON_DECORATION(pcd) & 
				     ICON_ACTIVE_LABEL_PART) &&
				    (wmGD.keyboardFocus == pcd))
				{
				    MoveActiveIconText(pcd);
				    ShowActiveIconText(pcd);
				}
			    }
			}
		    }
		    if ((place != NO_ICON_PLACE) && 
			(place != ICON_PLACE(pcd)))
		    {
			if (inIconBox)
			{
			    CvtIconPlaceToPosition (pIPD, place,
			    			&tmpX, &tmpY);
	    		    if( (CheckIconBoxSize (P_ICON_BOX(pcd))) &&
				(CheckVisualPlace(pcd, tmpX, tmpY)))
			    {
				/*
				 * Move the icon to the new place.
				 */

				MoveIconInfo (pIPD, ICON_PLACE(pcd), place);
				CvtIconPlaceToPosition (pIPD, place, 
						&ICON_X(pcd), &ICON_Y(pcd));


				XtMoveWidget (
				    pIPD->placeList[ICON_PLACE(pcd)].theWidget,
				    ICON_X(pcd), ICON_Y(pcd));

				SetNewBounds (P_ICON_BOX(pcd));
				if (ICON_DECORATION(pcd) &
                                    ICON_ACTIVE_LABEL_PART)
                                {
                                    MoveActiveIconText(pcd);
                                }
			    }
			    else
			    {
				F_Beep (NULL, pcd, (XEvent *)NULL);
			    }
			}
			else 
			{
#ifdef FWS
			    /*
			     * If this slot is the one which we have offered
			     * for FWS client use then attempt to withdraw the
			     * offer, so that we can use the slot.  Offer a
			     * new slot for FWS client use (if one is not
			     * already available).  Make the slot we are about
			     * to return temporarily unavailable.
			     */
			    if (place == pIPD->fwsNextIcon)
			    {
				FwsUnreserveIconSpace (ACTIVE_WS, pIPD);
				pIPD->placeList[place].pCD = fwsReserved;
				FwsReserveIconSpace (ACTIVE_WS, pIPD);
				pIPD->placeList[place].pCD = (ClientData *)NULL;
			    }
#endif /* FWS */
			    /*
			     * Move the icon to the new place.
			     */
			    MoveIconInfo (pIPD, ICON_PLACE(pcd), place);
			    CvtIconPlaceToPosition (pIPD, place, &ICON_X(pcd), 
						    &ICON_Y(pcd));

			    XMoveWindow (DISPLAY, ICON_FRAME_WIN(pcd), 
					 ICON_X(pcd), ICON_Y(pcd));

			    if (pcd->pSD->moveOpaque &&
				(ICON_DECORATION(pcd) & 
				 ICON_ACTIVE_LABEL_PART) &&
				(wmGD.keyboardFocus == pcd))
			    {
				MoveActiveIconText(pcd);
				ShowActiveIconText(pcd);
			    }
			}
		    }
		}
		else if (pcd->pSD->moveOpaque && !inIconBox)
                {
		    /*
		     * Replace icon into same place - as if it
		     * didn't move.
		     */
		    XMoveWindow (DISPLAY, ICON_FRAME_WIN(pcd),
				 ICON_X(pcd), ICON_Y(pcd));
		    if ((ICON_DECORATION(pcd) & 
			 ICON_ACTIVE_LABEL_PART) &&
			(wmGD.keyboardFocus == pcd))
		    {
			MoveActiveIconText(pcd);
			ShowActiveIconText(pcd);
		    }
                }
            }
	    else
	    {
		XMoveWindow (DISPLAY, ICON_FRAME_WIN(pcd), moveX, moveY);
		ICON_X(pcd) = moveX;
		ICON_Y(pcd) = moveY;
	    }
	    if ((ICON_DECORATION(pcd) & ICON_ACTIVE_LABEL_PART) &&
		(wmGD.keyboardFocus == pcd))
	    {
		MoveActiveIconText(pcd);
	    }
	}
	else {	/* assume normal window frame */
	    /* reconfigure the window(s) */
	    ProcessNewConfiguration (pcd, 
#ifndef CONFIG_RELATIVE_TO_CLIENT
				     moveX,
				     moveY,
#else
				     moveX + offsetX,
				     moveY + offsetY,
#endif
				     (unsigned int) 
					 (moveWidth - 2*offsetX),
				     (unsigned int) 
					 (moveHeight - offsetX - offsetY),
				     FALSE);
	}
    }
#ifdef WSM
    else if (wmGD.configAction == MARQUEE_SELECT)
    {
	WmScreenData *pSD;

	UndoGrabs();

	pSD = pcd ? pcd->pSD : ACTIVE_PSD;

	dtSendMarqueeSelectionNotification(pSD, DT_MARQUEE_SELECT_END, 
			marqueeX, marqueeY, marqueeWidth, marqueeHeight);
    }
#endif /* WSM */

    /*
     * Clear configuration flags and data.
     */

    wmGD.configAction = NO_ACTION;
    wmGD.configPart = FRAME_NONE;
    wmGD.configSet = False;
    configGrab = FALSE;
    anyMotion = FALSE;
    wmGD.movingIcon = FALSE;

#ifdef WSM
    if (pcd)
    {
#endif /* WSM */
    /* hide the move/resize config data */
    HideFeedbackWindow(pcd->pSD);

    /*
     * Set the focus back to something reasonable
     */
    RepairFocus ();	
#ifdef WSM
    }
#endif /* WSM */

} /* END OF FUNCTION CompleteFrameConfig */


/*************************************<->*************************************
 *
 *  MoveOpaque (pcd, x, y, width, height)
 *
 *
 *  Description:
 *  -----------
 *  Move a window or icon on the root or icon box.
 *
 *
 *  Inputs:
 *  ------
 *  pcd         - client data pointer
 *  x		- x coordinate (on root)
 *  y		- y coordinate (on root)
 *  width	- pixel width of frame
 *  height	- pixel height of frame
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  o use MoveOutline() for icons in an icon box.
 *  
 *************************************<->***********************************/
#ifdef _NO_PROTO
void
MoveOpaque (pcd, x, y, width, height)

    ClientData *pcd;
    int x, y; 
    unsigned int width, height;
#else /* _NO_PROTO */
void MoveOpaque (ClientData *pcd, int x, int y,
		 unsigned int width, unsigned int height)
#endif /* _NO_PROTO */


{
    /* Check if moving icon */
    if (wmGD.movingIcon)
    {
	if (pcd->pSD->useIconBox && P_ICON_BOX(pcd))
	{
	    /*
	     * For now just fall back to move outline when the
	     * icon is in the icon box
	     */   
	    
	    MoveOutline (x, y, width, height);
	}
	else
	{
	    XMoveWindow (DISPLAY,ICON_FRAME_WIN(pcd) , x, y);
	}
    }
    else
    {
	/* This is a window */
	XMoveWindow (DISPLAY, pcd->clientFrameWin, x, y);
	
    }
    
    /* cleanup exposed frame parts */
    PullExposureEvents ();
    
} /* END OF FUNCTION MoveOpaque */



/* thickness of outline */
#define OUTLINE_WIDTH	2

/* number of points to draw outline once */
#define SEGS_PER_DRAW	(4*OUTLINE_WIDTH)

/* number of points to flash outline (draw then erase) */
#define SEGS_PER_FLASH	(2*SEGS_PER_DRAW)


/*************************************<->*************************************
 *
 *  DrawSegments (dpy, win, gc, outline, nsegs)
 *
 *  Description:
 *  -----------
 *  Draw segments using either using normal X or using the ALLPLANES
 *  extension, depending on #ifdef ALLPLANES and whether the server actually
 *  supports the extension.  This is a thin wrapper around the Xlib
 *  XDrawSegments() call.
 *
 *  Inputs:
 *  ------
 *  dpy		- the X display
 *  win		- the window on which to draw
 *  gc		- the gc to use, typically whose function is GXxor
 *  outline	- array of segments
 *  nsegs	- number of segments in the outline array
 * 
 *  Outputs:
 *  -------
 *  (none)
 *
 *  Comments:
 *  --------
 *  Note: no GC is used when drawing with the ALLPLANES extension;
 *  therefore, the GC parameter is ignored in that case.
 * 
 *************************************<->***********************************/


#ifdef _NO_PROTO
static void
DrawSegments (dpy, win, gc, outline, nsegs)
	Display *dpy;
	Window win;
	GC gc;
	XSegment *outline;
	int nsegs;
#else /* _NO_PROTO */
static void
DrawSegments (Display *dpy, Window win, GC gc, XSegment *outline, int nsegs)
#endif /* _NO_PROTO */
{
#if defined(sun) && defined(ALLPLANES)
	if (wmGD.allplanes)
	    XAllPlanesDrawSegments(dpy, win, outline, nsegs);
	else
#endif /* defined(sun) && defined(ALLPLANES) */
	    XDrawSegments(dpy, win, gc, outline, nsegs);
} /* END OF FUNCTION  DrawSegments */


/*************************************<->*************************************
 *
 *  MoveOutline (x, y, width, height)
 *
 *
 *  Description:
 *  -----------
 *  Draw a window outline on the root window.
 *
 *
 *  Inputs:
 *  ------
 *  x		- x coordinate (on root)
 *  y		- y coordinate (on root)
 *  width	- pixel width of frame
 *  height	- pixel height of frame
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  o get display, root window ID, and xorGC out of global data.  
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void MoveOutline (x, y, width, height)

	int x, y; 
	unsigned int width, height;
#else /* _NO_PROTO */
void MoveOutline (int x, int y, unsigned int width, unsigned int height)
#endif /* _NO_PROTO */
{
    if (wmGD.freezeOnConfig)
    {
	DrawOutline (x, y, width, height);
    }
    else
    {
#ifdef WSM
      if (wmGD.useWindowOutline)
	WindowOutline(x,y,width,height);
      else
#endif
	FlashOutline(x, y, width, height);
    }
} /* END OF FUNCTION  MoveOutline */



/*************************************<->*************************************
 *
 *  FlashOutline ()
 *
 *
 *  Description:
 *  -----------
 *  flash a window outline on the root window.
 *
 *
 *  Inputs:
 *  ------
 *  x		- x coordinate (on root)
 *  y		- y coordinate (on root)
 *  width	- pixel width of frame
 *  height	- pixel height of frame
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  o get display, root window ID, and xorGC out of global data.  
 *  o draw on root and erase "atomically"
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void FlashOutline(x, y, width, height)

	int x, y; 
	unsigned int width, height;
#else /* _NO_PROTO */
void FlashOutline (int x, int y, unsigned int width, unsigned int height)
#endif /* _NO_PROTO */
{
    static XSegment  outline[SEGS_PER_FLASH];

    /*
     * Do nothing if no box to draw 
     */
    if (x == 0 && y == 0 &&
	width == 0 && height == 0)
	return;

    /*
     * Draw outline an even number of times (draw then erase)
     */
    SetOutline (outline, x, y, width, height, OUTLINE_WIDTH);
    memcpy ( (char *) &outline[SEGS_PER_DRAW], (char *) &outline[0], 
	SEGS_PER_DRAW*sizeof(XSegment));

    /*
     * Flash the outline at least once, then as long as there's 
     * nothing else going on
     */
    DrawSegments(DISPLAY, ACTIVE_ROOT, ACTIVE_PSD->xorGC,
			outline, SEGS_PER_FLASH);
    XSync(DISPLAY, FALSE);

    while (!XtAppPending(wmGD.mwmAppContext)) {
    	DrawSegments(DISPLAY, ACTIVE_ROOT, ACTIVE_PSD->xorGC, 
			outline, SEGS_PER_FLASH);
	XSync(DISPLAY, FALSE);
    }
} /* END OF FUNCTION  FlashOutline */

#ifdef WSM

/*************************************<->*************************************
 *
 *  CreateOutlineWindows (pSD)
 *
 *
 *  Description:
 *  -----------
 *  create the outline windows
 *
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  variables are affected:
 *      woN
 *      woS
 *      woE
 *      woW
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
CreateOutlineWindows(pSD)
    WmScreenData *pSD;
#else /* _NO_PROTO */
CreateOutlineWindows (WmScreenData *pSD)
#endif /* _NO_PROTO */
{
    XSetWindowAttributes xswa;
    unsigned int xswamask;
    int x, y, width, height;


    x = -10;
    y = -10;
    width = OUTLINE_WIDTH;
    height = OUTLINE_WIDTH;

    xswa.override_redirect = True;    
    xswa.backing_store = NotUseful;
    xswa.save_under = True;
    xswa.background_pixmap = XmGetPixmap(
				XtScreen(pSD->screenTopLevelW),
				"50_foreground", 
				pSD->clientAppearance.foreground,
				pSD->clientAppearance.background);

    xswamask = (CWOverrideRedirect | 
		CWBackingStore | 
		CWBackPixmap |
		CWSaveUnder);
    
    pSD->woN = XCreateWindow(DISPLAY, pSD->rootWindow, 
		       x, y, width, height,
		       0, 
		       XDefaultDepth(DISPLAY,pSD->screen), 
		       CopyFromParent,
		       CopyFromParent, 
		       xswamask, 
		       &xswa);

    pSD->woS = XCreateWindow(DISPLAY, pSD->rootWindow, 
		       x, y, width, height,
		       0, 
		       XDefaultDepth(DISPLAY,pSD->screen), 
		       CopyFromParent,
		       CopyFromParent, 
		       xswamask, 
		       &xswa);

    pSD->woE = XCreateWindow(DISPLAY, pSD->rootWindow, 
		       x, y, width, height,
		       0, 
		       XDefaultDepth(DISPLAY,pSD->screen), 
		       CopyFromParent,
		       CopyFromParent, 
		       xswamask, 
		       &xswa);

    pSD->woW = XCreateWindow(DISPLAY, pSD->rootWindow, 
		       x, y, width, height,
		       0, 
		       XDefaultDepth(DISPLAY,pSD->screen), 
		       CopyFromParent,
		       CopyFromParent, 
		       xswamask, 
		       &xswa);

} /* END OF FUNCTION  CreateOutlineWindows */


/*************************************<->*************************************
 *
 *  WindowOutline ()
 *
 *
 *  Description:
 *  -----------
 *  show an outline on the root window using windows.
 *
 *
 *  Inputs:
 *  ------
 *  x		- x coordinate (on root)
 *  y		- y coordinate (on root)
 *  width	- pixel width of frame
 *  height	- pixel height of frame
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  Always unmap during move/resize of outline windows to let saveunder
 *  stuff work. HP server's toss saveunder stuff for windows that
 *  configure themselves while mapped.
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void WindowOutline(x, y, width, height)

	int x, y; 
	unsigned int width, height;
#else /* _NO_PROTO */
void WindowOutline (int x, int y, unsigned int width, unsigned int height)
#endif /* _NO_PROTO */
{
    static int  lastOutlineX = 0;
    static int  lastOutlineY = 0;
    static int  lastOutlineWidth = 0;
    static int  lastOutlineHeight = 0;
    WmScreenData *pSD = ACTIVE_PSD;
    int iX, iY;
    int iW, iH;

    if (pSD->woN == (Window)0L)
    {
	CreateOutlineWindows(pSD);
    }

    if (x == lastOutlineX && y == lastOutlineY && 
	width == lastOutlineWidth && height == lastOutlineHeight)
    {
	return;		/* no change */
    }

    XUnmapWindow(DISPLAY, pSD->woN);
    XUnmapWindow(DISPLAY, pSD->woS);
    XUnmapWindow(DISPLAY, pSD->woE);
    XUnmapWindow(DISPLAY, pSD->woW);

    if ((width == 0) && (height == 0))
    {
	lastOutlineWidth = lastOutlineHeight = 0;
	lastOutlineX = lastOutlineY = 0;
    }
    else
    {
	/* North */
	iX = x;
	iY = y;
	iW = (int) width;
	iH = OUTLINE_WIDTH;
	if (iW < 0) iW = 1;
	XMoveResizeWindow (DISPLAY, pSD->woN, iX, iY, iW, iH);

	/* West */
	iX = x;
	iY = y + OUTLINE_WIDTH;
	iW = OUTLINE_WIDTH;
	iH = (int) height - OUTLINE_WIDTH;
	if (iH < 0) iH = 1;
	XMoveResizeWindow (DISPLAY, pSD->woW, iX, iY, iW, iH);

	/* East */
	iX = x + (int)width - OUTLINE_WIDTH;
	iY = y + OUTLINE_WIDTH;
	iW = OUTLINE_WIDTH;
	iH = (int)height - OUTLINE_WIDTH;
	if (iH < 0) iH = 1;
	XMoveResizeWindow (DISPLAY, pSD->woE, iX, iY, iW, iH);

	/* South */
	iX = x + OUTLINE_WIDTH;
	iY = y + (int)height - OUTLINE_WIDTH;
	iW = (int)width - 2*OUTLINE_WIDTH;
	iH = OUTLINE_WIDTH;
	if (iW < 0) iW = 1;
	XMoveResizeWindow (DISPLAY, pSD->woS, iX, iY, iW, iH);

	lastOutlineX = x;
	lastOutlineY = y;
	lastOutlineWidth = width;
	lastOutlineHeight = height;

	XMapRaised (DISPLAY, pSD->woN);
	XMapRaised (DISPLAY, pSD->woS);
	XMapRaised (DISPLAY, pSD->woE);
	XMapRaised (DISPLAY, pSD->woW);

	/* cleanup exposed frame parts */
	PullExposureEvents ();
    }

} /* END OF FUNCTION  WindowOutline */

#endif /* WSM */


/*************************************<->*************************************
 *
 *  DrawOutline (x, y, width, height)
 *
 *
 *  Description:
 *  -----------
 *  Draw a window outline on the root window.
 *
 *
 *  Inputs:
 *  ------
 *  x		- x coordinate (on root)
 *  y		- y coordinate (on root)
 *  width	- pixel width of frame
 *  height	- pixel height of frame
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  o get display, root window ID, and xorGC out of global data.  
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void DrawOutline(x, y, width, height)

	int x, y; 
	unsigned int width, height;
#else /* _NO_PROTO */
void DrawOutline (int x, int y, unsigned int width, unsigned int height)
#endif /* _NO_PROTO */
{
    static int  lastOutlineX = 0;
    static int  lastOutlineY = 0;
    static int  lastOutlineWidth = 0;
    static int  lastOutlineHeight = 0;
    XSegment  outline[SEGS_PER_DRAW];


    if (x == lastOutlineX && y == lastOutlineY && 
	width == lastOutlineWidth && height == lastOutlineHeight)
    {
	return;		/* no change */
    }

    if (lastOutlineWidth || lastOutlineHeight) {
	SetOutline (outline, lastOutlineX, lastOutlineY, lastOutlineWidth,
	    lastOutlineHeight, OUTLINE_WIDTH);

    	DrawSegments(DISPLAY, ACTIVE_ROOT, ACTIVE_PSD->xorGC, 
			outline, SEGS_PER_DRAW);
    }

    lastOutlineX = x;
    lastOutlineY = y;
    lastOutlineWidth = width;
    lastOutlineHeight = height;

    if (lastOutlineWidth || lastOutlineHeight) {

	SetOutline (outline, lastOutlineX, lastOutlineY, lastOutlineWidth,
	    lastOutlineHeight, OUTLINE_WIDTH);

    	DrawSegments(DISPLAY, ACTIVE_ROOT, ACTIVE_PSD->xorGC, 
			outline, SEGS_PER_DRAW);
    }
} /* END OF FUNCTION  DrawOutline */



/*************************************<->*************************************
 *
 *  WindowIsOnScreen (pCD, dx, dy)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to check if a window is atleast partially on the
 *  screen or not.  If the window is completely off the screen, dx and dy
 *  will contain the minimum distance to move some part of the window's frame
 *  back onto the screen.
 *
 *
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *  dx		- minimum x distance to move the window back to the screen
 *  dy		- minimum y distance to move the window back to the screen
 *
 *
 *  Returns:
 *  --------
 *  true	- if the window has some part on the screen
 *  false       - if the window is completely off the screen
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean WindowIsOnScreen (pCD, dx, dy)
     ClientData *pCD;
     int *dx;
     int *dy;
#else /* _NO_PROTO */
Boolean WindowIsOnScreen (ClientData *pCD, int *dx, int *dy)
#endif /* _NO_PROTO */
{
  int x1 = pCD->clientX;
  int x2 = pCD->clientX + pCD->clientWidth;
  int y1 = pCD->clientY;
  int y2 = pCD->clientY + pCD->clientHeight;
  int screenW = DisplayWidth (DISPLAY, SCREEN_FOR_CLIENT(pCD));
  int screenH = DisplayHeight(DISPLAY, SCREEN_FOR_CLIENT(pCD));

  *dx = *dy = 0;
  
  if (x2 < 0)			/* right frame border off left side of screen. */
    *dx =  -x2;
  else if (x1 > screenW)	/* left frame border off right side of screen. */
    *dx = screenW - x1;
  
  if (y2 < 0)			/* bottom frame border off top of screen. */
    *dy = -y2;
  else if (y1 > screenH)	/* top frame border off bottom of screen. */
    *dy = screenH - y1;
  
  return ((*dx == 0) && (*dy == 0));
}



/*************************************<->*************************************
 *
 *  InitEdgeMove ()
 *
 *  Description:
 *  -----------
 *  This function resets the global emState variable to a value appropriate
 *  for the start of a window configuration action.
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *		- side effects to emState
 *
 *  Returns:
 *  --------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

void
#ifdef _NO_PROTO
InitEdgeMove ()
#else /* _NO_PROTO */
InitEdgeMove (void)
#endif /* _NO_PROTO */
{
  emState.pendDx = 0;
  emState.pendDy = 0;
} /* END OF FUNCTION	InitEdgeMove */



/*************************************<->*************************************
 *
 *  CheckEdgeMove (pCD, initPlacement, curx, cury, width, height, dx, dy)
 *
 *  Description:
 *  -----------
 *  This function adds dx and dy to the current window position and checks
 *  whether the resulting position would cross the edge move threshold.  If
 *  moving into the near side of the threshold area, the proposed motion is
 *  stashed away but not acted on immediately.  If moving out of the far side
 *  of the threshold area, the threshold is disabled for the remainder of the
 *  configuration action.
 *
 *  XXX: Consider modifying behavior so that crossing the far side of the
 *  threshold doesn't apply all of the accumulated motion, but only the last
 *  increment, so that windows don't make sudden jumps.  (See how olwm does
 *  it...)  After making this change, it would probably make sense to allow
 *  stickyness to reapply each time the window moves back completely within
 *  the screen area.
 *
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data
 *  curx	- current x position to move to
 *  cury	- current y position to move to
 *  dx		- incremental x movement proposed
 *  dy		- incremental y movement proposed
 * 
 *  Outputs:
 *  -------
 *		- side effects to emState
 *
 *  Returns:
 *  --------
 *
 *  Comments:
 *  --------
 *  CheckHorEdgeMove and CheckVertEdgeMove are helper functions that do the
 *  bulk of the real work, handling respectively the horizontal and vertical
 *  compoments of the proposed motion.
 * 
 *************************************<->***********************************/

static void
#ifdef	_NO_PROTO
CheckHorEdgeMove (pCD, curx, width, dx)
    ClientData *pCD;
    int curx;
    unsigned int width;
    int *dx;
#else	/* _NO_PROTO */
CheckHorEdgeMove (ClientData *pCD, int curx, unsigned int width, int *dx)
#endif	/* _NO_PROTO */
{
    int xl = curx + emState.pendDx + *dx;
    int xr = xl + width - 1;
    int screenW = DisplayWidth (DISPLAY, SCREEN_FOR_CLIENT(pCD));
    int edgeMoveThreshold = pCD->pSD->edgeMoveThreshold;

    if (*dx < 0) {
	/*
	 * Moving left:  If the old position was in the right threshold, use
	 * *dx to cancel out pending motion to the right.
	 */
	if (emState.pendDx > 0) {
	    if (*dx + emState.pendDx >= 0) {
		emState.pendDx += *dx, *dx = 0;
		/* No residual motion left after cancellation. */
		return;
	    } else
		*dx += emState.pendDx, emState.pendDx = 0;
	}
	/*
	 * Now perform checks against the left window edge.
	 */
	if (xl < 0 && xl >= -edgeMoveThreshold) {
	    /*
	     * The proposed movement lands inside the threshold area.  If the
	     * current position is already inside the threshold (which can
	     * happen by moving back into it after having gone all the way
	     * through it), accumulate the motion into pendDx.  Otherwise,
	     * allow only the part of it that brings the window to the edge of
	     * the threshold and accumulate the rest.
	     */
	    if (curx >= 0 && emState.pendDx == 0)
		emState.pendDx = xl, *dx -= emState.pendDx;
	    else
		emState.pendDx += *dx, *dx = 0;
	    return;
	}
    } else {
	/* Moving right; behave symmetrically to the moving left case. */
	if (emState.pendDx < 0) {
	    if (*dx + emState.pendDx <= 0) {
		emState.pendDx += *dx, *dx = 0;
		/* No residual motion left after cancellation. */
		return;
	    } else
		*dx += emState.pendDx, emState.pendDx = 0;
	}
	/*
	 * Now perform checks against the right window edge.
	 */
	if (xr >= screenW && xr < screenW + edgeMoveThreshold) {
	    /*
	     * The proposed movement lands inside the threshold area.  If the
	     * current position is already inside the threshold (which can
	     * happen by moving back into it after having gone all the way
	     * through it), accumulate the motion into pendDx.  Otherwise,
	     * allow only the part of it that brings the window to the edge of
	     * the threshold and accumulate the rest.
	     */
	    if (curx + width - 1 < screenW && emState.pendDx == 0) {
		emState.pendDx = xr - (screenW - 1);
		*dx -= emState.pendDx;
	    } else
		emState.pendDx += *dx, *dx = 0;
	    return;
	}
    }

    /*
     * The new position either falls inside the screen or beyond the far edge
     * of the threshold.  In either case, the motion should be allowed (adding
     * in pending motion as well).
     */
    *dx += emState.pendDx, emState.pendDx = 0;
}

static void
#ifdef	_NO_PROTO
CheckVertEdgeMove (pCD, cury, height, dy)
    ClientData *pCD;
    int cury;
    unsigned int height;
    int *dy;
#else	/* _NO_PROTO */
CheckVertEdgeMove (ClientData *pCD, int cury, unsigned int height, int *dy)
#endif	/* _NO_PROTO */
{
    int yt = cury + emState.pendDy + *dy;
    int yb = yt + height - 1;
    int screenH = DisplayHeight (DISPLAY, SCREEN_FOR_CLIENT(pCD));
    int edgeMoveThreshold = pCD->pSD->edgeMoveThreshold;

    if (*dy < 0) {
	/*
	 * Moving up:  If the old position was in the bottom threshold, use
	 * *dy to cancel out pending downward motion.
	 */
	if (emState.pendDy > 0) {
	    if (*dy + emState.pendDy >= 0) {
		emState.pendDy += *dy, *dy = 0;
		/* No residual motion left after cancellation. */
		return;
	    } else
		*dy += emState.pendDy, emState.pendDy = 0;
	}
	/*
	 * Now perform checks against the top window edge.
	 */
	if (yt < 0 && yt >= -edgeMoveThreshold) {
	    /*
	     * The proposed movement lands inside the threshold area.  If the
	     * current position is already inside the threshold (which can
	     * happen by moving back into it after having gone all the way
	     * through it), accumulate the motion into pendDy.  Otherwise,
	     * allow only the part of it that brings the window to the edge of
	     * the threshold and accumulate the rest.
	     */
	    if (cury >= 0 && emState.pendDy == 0) {
		emState.pendDy = yt, *dy -= emState.pendDy;
	    } else {
		emState.pendDy += *dy, *dy = 0;
	    }
	    return;
	}
    } else {
	/* Moving down; behave symmetrically to the moving up case. */
	if (emState.pendDy < 0) {
	    if (*dy + emState.pendDy <= 0) {
		emState.pendDy += *dy, *dy = 0;
		/* No residual motion left after cancellation. */
		return;
	    } else
		*dy += emState.pendDy, emState.pendDy = 0;
	}
	/*
	 * Now perform checks against the bottom window edge.
	 */
	if (yb >= screenH && yb < screenH + edgeMoveThreshold) {
	    /*
	     * The proposed movement lands inside the threshold area.  If the
	     * current position is already inside the threshold (which can
	     * happen by moving back into it after having gone all the way
	     * through it), accumulate the motion into pendDy.  Otherwise,
	     * allow only the part of it that brings the window to the edge of
	     * the threshold and accumulate the rest.
	     */
	    if (cury + height - 1 < screenH && emState.pendDy == 0) {
		emState.pendDy = yb - (screenH - 1);
		*dy -= emState.pendDy;
	    } else
		emState.pendDy += *dy, *dy = 0;
	    return;
	}
    }

    /*
     * The new position either falls inside the screen or beyond the far edge
     * of the threshold.  In either case, the motion should be allowed (adding
     * in pending motion as well).
     */
    *dy += emState.pendDy, emState.pendDy = 0;
}

void
#ifdef	_NO_PROTO
CheckEdgeMove (pCD, initPlacement, curx, cury, width, height, dx, dy)
    ClientData *pCD;
    Boolean initPlacement;
    int curx;
    int cury;
    unsigned int width;
    unsigned int height;
    int *dx;
    int *dy;
#else	/* _NO_PROTO */
CheckEdgeMove (ClientData *pCD, Boolean initPlacement, int curx, int cury,
	unsigned int width, unsigned int height, int *dx, int *dy)
#endif	/* _NO_PROTO */
{
    /*
     * If dealing with an icon use the supplied width and height directly.
     * However, if dealing with a regular client frame, use the frameInfo
     * dimensions rather than the client dimensions given as argument, since
     * the intent is to make sure that the entire frame respects the threshold
     * constraints.  Unfortunately, during the initial placement, the frame
     * hasn't been set up yet, so the outilne window dimensions have to
     * suffice.
     */
     if (!(initPlacement || wmGD.movingIcon ||
		(pCD->pSD->useIconBox && P_ICON_BOX(pCD)))) {
	width = pCD->frameInfo.width;
	height = pCD->frameInfo.height;
     }

    if (*dx != 0 && width <= DisplayWidth (DISPLAY, SCREEN_FOR_CLIENT(pCD)))
	CheckHorEdgeMove(pCD, curx, width, dx);
    if (*dy != 0 && height <= DisplayHeight (DISPLAY, SCREEN_FOR_CLIENT(pCD)))
	CheckVertEdgeMove(pCD, cury, height, dy);
}


/*************************************<->*************************************
 *
 *  ProcessNewConfiguration (pCD, x, y, width, height, clientRequest)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to configure a client window following receipt of
 *  a client request or an interactive configuration action.
 *
 *
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data
 *  x		- x coord of client window
 *  y		- y coord of client window
 *  width	- width of client window
 *  height	- height of client window
 *  clientRequest	- true if configuration requested by client program
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void ProcessNewConfiguration (pCD, x, y, width, height, clientRequest)
	ClientData *pCD;
	int x;
	int y;
	unsigned int width;
	unsigned int height;
	Boolean clientRequest;

#else /* _NO_PROTO */
void ProcessNewConfiguration (ClientData *pCD, int x, int y, unsigned int width, unsigned int height, Boolean clientRequest)
#endif /* _NO_PROTO */
{
    unsigned int changedValues = 0;
    int          xoff = 0, yoff = 0;
    int          dx, dy;
    Boolean      originallyOnScreen = WindowIsOnScreen(pCD, &dx, &dy);
    Boolean newMax = False;
    Boolean toNewMax = False;

    /*
     * Fix the configuration values to be compatible with the configuration
     * constraints for this class of windows.
     */

    

    FixWindowConfiguration (pCD, &width, &height,
				 (unsigned int) pCD->widthInc, 
				 (unsigned int) pCD->heightInc);

    if ((pCD->maxWidth != pCD->oldMaxWidth) ||
	 (pCD->maxHeight != pCD->oldMaxHeight))
    {
	/*
	 * We've got a new maximum size.
	 */
	newMax = True;
    }

    /*
     * If the configuration has changed, update client data
     * 
     * Changes in width or height cause maximized windows to return to
     * normal state and update normal geometry (x, y, width, height)
     */
    if (pCD->maxConfig) 
    {
	if (newMax &&
	    (pCD->maxWidth == width) &&
	    (pCD->maxHeight == height))
	{
	    /* we're changing to the new max size */
	    toNewMax = True;
	}

	changedValues |= (width != pCD->oldMaxWidth) ? CWWidth : 0;
	changedValues |= (height != pCD->oldMaxHeight) ? CWHeight : 0;

	if (!toNewMax && (changedValues & CWWidth)) {
	    /*
	     * Hacked to update maxWidth for 'vertical' max clients
	     */
	    if (IS_MAXIMIZE_VERTICAL(pCD)) {
		pCD->maxWidth = width;
	    }
	    pCD->clientWidth = width;
	    if (changedValues & CWHeight) {
		/*
		 * Hacked to update maxHeight for 'horizontal' max client
		 */
		if (IS_MAXIMIZE_HORIZONTAL(pCD)) {
		    pCD->maxHeight = height;
		}
		pCD->clientHeight = height;
	    }
	    else {
		pCD->clientHeight = pCD->oldMaxHeight;
	    }
	}
	else if (!toNewMax && (changedValues & CWHeight)) {
	    /*
	     * Hacked to update maxHeight for 'horizontal' max client
	     */
	    if (IS_MAXIMIZE_HORIZONTAL(pCD)) {
		pCD->maxHeight = height;
	    }
	    pCD->clientHeight = height;
	    pCD->clientWidth = pCD->oldMaxWidth;
	}
    }
    else {
	if (width != pCD->clientWidth)
	{
	    /*
	     * Hacked to update maxWidth for 'vertical' max clients
	     */
	    if (IS_MAXIMIZE_VERTICAL(pCD)) {
		pCD->maxWidth = width;
	    }

	    changedValues |= CWWidth;
	    pCD->clientWidth = width;

	}

	if (height != pCD->clientHeight)
	{
	    /*
	     * Hacked to update maxHeight for 'horizontal' max client
	     */
	    if (IS_MAXIMIZE_HORIZONTAL(pCD)) {
		pCD->maxHeight = height;
	    }

	    changedValues |= CWHeight;
	    pCD->clientHeight = height;
	}
    }

#ifndef CONFIG_RELATIVE_TO_CLIENT
    /*
     * If positionIsFrame or user initiated configuration request,
     * then adjust client position to by frame_width and frame_height.
     */
    if (wmGD.positionIsFrame || (!clientRequest))
    {
	xoff = pCD->clientOffset.x;
	yoff = pCD->clientOffset.y;
    }
#endif

    /*
     * Changes in position update maximum geometry on maximized windows
     * if there was no change in size.
     */
    if (pCD->maxConfig) {
	if (x != pCD->maxX) {
	    changedValues |= CWX;

            if (IS_MAXIMIZE_VERTICAL(pCD)) {
                pCD->clientX = x + xoff;
	    }

	    if (!toNewMax && (changedValues & (CWWidth | CWHeight)))
		pCD->clientX = x + xoff;
	    else
		pCD->maxX = x + xoff;

	}
	else if (!toNewMax && (changedValues & (CWWidth | CWHeight)))
	{
	    pCD->clientX = pCD->maxX;
	}

	if (y != pCD->maxY) {
	    changedValues |= CWY;

            if (IS_MAXIMIZE_HORIZONTAL(pCD)) {
	        pCD->clientY = y + yoff;
	    }

	    if (!toNewMax && (changedValues & (CWWidth | CWHeight)))
		pCD->clientY = y + yoff;
	    else
		pCD->maxY = y + yoff;

	}
	else if (!toNewMax && (changedValues & (CWWidth | CWHeight))) 
	{
	    pCD->clientY = pCD->maxY;
	}
    }
    else {
	if (x != pCD->clientX) {
	    changedValues |= CWX;
	    pCD->clientX = x + xoff;

            if (IS_MAXIMIZE_VERTICAL(pCD)) {
                pCD->maxX = x + xoff;
	    }
	}

	if (y != pCD->clientY) {
	    changedValues |= CWY;
	    pCD->clientY = y + yoff;

            if (IS_MAXIMIZE_HORIZONTAL(pCD)) {
                pCD->maxY = y + yoff;
	    }
	}
    }

    /* check if the window has reconfigured itself off the screen. */
    if (originallyOnScreen && !WindowIsOnScreen(pCD, &dx, &dy))
      {
	if (dx != 0)
	  {
	    changedValues |= CWX;
	    pCD->clientX += dx;
	  }

	if (dy != 0)
	  {
	    changedValues |= CWY;
	    pCD->clientY += dy;
	  }
      }


    /*
     * Resize the client window if necessary:
     */

    if (changedValues & (CWWidth | CWHeight))
    {
	if (pCD->maxConfig) 
	{
	    if (!toNewMax)
	    {
		/* maximized window resized, return to normal state */
		pCD->maxConfig = FALSE;
		pCD->clientState = NORMAL_STATE;
	    }
	}

	XResizeWindow (DISPLAY, pCD->client, width, height);
	RegenerateClientFrame(pCD);
    }
    if (changedValues & (CWX | CWY)) {
	if (pCD->maxConfig)
	{
	  /*
	   * Fix for 5217 - If the request is from the client, use the clients
	   *                offsets instead of the static offsets
	   */
	  if (clientRequest)
	    {
	      XMoveWindow (DISPLAY, pCD->clientFrameWin,
			   pCD->maxX - pCD->clientOffset.x,
			   pCD->maxY - pCD->clientOffset.y);
	    }
	  else
	    {
	      XMoveWindow (DISPLAY, pCD->clientFrameWin, 
			   pCD->maxX - offsetX,
			   pCD->maxY - offsetY);
	    }
	  /* End fix 5217 */
	}
	else 
	{
            if (clientRequest)
            {
                XMoveWindow (DISPLAY, pCD->clientFrameWin,
                             pCD->clientX - pCD->clientOffset.x,
                             pCD->clientY - pCD->clientOffset.y);
            }
            else
            {
                XMoveWindow (DISPLAY, pCD->clientFrameWin,
                             pCD->clientX - offsetX,
                             pCD->clientY - offsetY);
            }
	}
	SetFrameInfo (pCD);
#ifdef PANELIST
	if (pCD->dtwmBehaviors & DtWM_BEHAVIOR_SUBPANEL)
	{
	    /* turn off subpanel behavior if moved */
	    pCD->dtwmBehaviors &= ~DtWM_BEHAVIOR_SUBPANEL;
	}
#endif /* PANELIST */
    }

    /*
     * Send a configure notify  message if appropriate:
     *   1. rejected client configuration request.
     *   2. client request and move without resize 
     */


    if ((!changedValues && clientRequest) ||
	(changedValues && !(changedValues & (CWWidth | CWHeight))))
    {
	SendConfigureNotify (pCD);
    }

    /*
     * Try to send notice directly to icon box that the window
     * has changed size
     */

    if ((pCD->clientFlags & ICON_BOX) && 
	(changedValues & (CWWidth | CWHeight)))
    {
	CheckIconBoxResize(pCD, changedValues, width, height);
    }

} /* END OF FUNCTION ProcessNewConfiguration */



/*************************************<->*************************************
 *
 *  StartResizeConfig (pcd, pev)
 *
 *
 *  Description:
 *  -----------
 *  Start resize of client window 
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *  pev		- pointer to event
 * 
 *  Outputs:
 *  -------
 *  return	- true if configuration can begin, else false
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean StartResizeConfig(pcd, pev)
	ClientData *pcd;
	XEvent *pev;

#else /* _NO_PROTO */
Boolean StartResizeConfig (ClientData *pcd, XEvent *pev)
#endif /* _NO_PROTO */
{
    Window grab_win, junk_win;
    Boolean grabbed;
    int big_inc, tmp_inc;
    int junk, junkX, junkY;

    /*
     *	Do our grabs 
     */
    if (!configGrab)
    {
	grab_win = GrabWin (pcd, pev);

	if (pev)
	{
	    grabbed = DoGrabs (grab_win, ConfigCursor((int) wmGD.configPart), 
			PGRAB_MASK, pev->xbutton.time, pcd, True);
	}
	else
	{
	    grabbed = DoGrabs (grab_win, ConfigCursor((int) wmGD.configPart), 
			PGRAB_MASK, CurrentTime, pcd, True);
	}
	if (!grabbed)
	{
	    return (False);
	}
	configGrab = TRUE;
    }
    else
    {
	/* continue with the configuration in progress (!!!) */
	return (True);
    }

    /* 
     * Set up static variables for succeeding events 
     */
    if (!XQueryPointer (DISPLAY, ROOT_FOR_CLIENT(pcd), &junk_win, &junk_win,
		   &pointerX, &pointerY, &junk, &junk, (unsigned int *)&junk))
    {
	CancelFrameConfig (pcd);	/* release grabs */
	return (False);
    };
    wmGD.preMoveX = pointerX;
    wmGD.preMoveY = pointerY;
    anyMotion = FALSE;

    offsetX = pcd->clientOffset.x;
    offsetY = pcd->clientOffset.y;

    /* 
     * get window geometry information and convert to frame coordinates
     */
    if (pcd->maxConfig) {
	resizeX = pcd->maxX;
	resizeY = pcd->maxY;
	resizeWidth = pcd->maxWidth;
	resizeHeight = pcd->maxHeight;
    }
    else {
	resizeX = pcd->clientX;
	resizeY = pcd->clientY;
	resizeWidth = pcd->clientWidth;
	resizeHeight = pcd->clientHeight;
    }
    ClientToFrame(pcd, &resizeX, &resizeY, &resizeWidth, &resizeHeight);

    /* save start values to see where we came from */
    startX = resizeX;
    startY = resizeY;
    startWidth = resizeWidth;
    startHeight = resizeHeight;

    /* get min and max frame sizes */
    minWidth = pcd->minWidth;
    minHeight = pcd->minHeight;
    junkX = junkY = 0;
    ClientToFrame(pcd, &junkX, &junkY, &minWidth, &minHeight);

    /*
     * Hack to use maxHeightLimit and maxWidthLimit as the real max when
     * maximumClientSize is set to 'horizontal' or 'vertical', since
     * pCD->maxHeight and pCD->maxWidth is fiddle to on reconfiguration.
     */
    maxWidth = pcd->maxWidthLimit;
    maxHeight = pcd->maxHeightLimit;
    junkX = junkY = 0;
    ClientToFrame(pcd, &junkX, &junkY, &maxWidth, &maxHeight);

    /* compute big increment values */
    big_inc = DisplayWidth (DISPLAY, SCREEN_FOR_CLIENT(pcd)) / 20;

    tmp_inc = big_inc - big_inc%pcd->widthInc;
    if (tmp_inc > 5*pcd->widthInc)
	resizeBigWidthInc = tmp_inc;
    else 
	resizeBigWidthInc = 5*pcd->widthInc;

    tmp_inc = big_inc - big_inc%pcd->heightInc;
    if (tmp_inc > 5*pcd->heightInc)
	resizeBigHeightInc = tmp_inc;
    else 
	resizeBigHeightInc = 5*pcd->heightInc;

    /* pop up feedback window */
    if (wmGD.showFeedback & WM_SHOW_FB_RESIZE)
    {
	DoFeedback (pcd, resizeX, resizeY, resizeWidth, resizeHeight, 
		    FB_SIZE, TRUE /* do size checks */);
    }

    /* set configuring data */
    wmGD.configAction = RESIZE_CLIENT;
    wmGD.configButton = pev ? pev->xbutton.button: 0;

    return (True);

} /* END OF FUNCTION StartResizeConfig */



/*************************************<->*************************************
 *
 *  StartClientResize (pcd, pev)
 *
 *
 *  Description:
 *  -----------
 *  Start resize of client window as invoked from menu
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *  pev		- pointer to event
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  o This should only be called as the result of a Resize function 
 *    being selected from the system menu.
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void StartClientResize(pcd, pev)

	ClientData *pcd;
	XEvent *pev;
#else /* _NO_PROTO */
void StartClientResize (ClientData *pcd, XEvent *pev)
#endif /* _NO_PROTO */
{

    /* do initial setup for resize */
    wmGD.configPart = FRAME_NONE;	/* determined by later action */
    wmGD.configSet = False;		/* don't know what it is yet */
    if (!StartResizeConfig (pcd, pev))
    {
	/* resize could not be initiated */
	return;
    }


    /*
     *  Warp pointer to middle of window if started from the keyboard
     *  or menu (no event).
     */
    if ( !pev || pev->type == KeyPress )
    {
	pointerX = resizeX + resizeWidth/2;
	pointerY = resizeY + resizeHeight/2;

	ForceOnScreen(SCREEN_FOR_CLIENT(pcd), &pointerX, &pointerY);
	if (wmGD.enableWarp)
	{
	    XWarpPointer(DISPLAY, None, ROOT_FOR_CLIENT(pcd), 
		     0, 0, 0, 0, pointerX, pointerY);
	}
    }

} /* END OF FUNCTION StartClientResize  */


/*************************************<->*************************************
 *
 *  StartClientMove (pcd, pev)
 *
 *
 *  Description:
 *  -----------
 *  Handle move of client window as invoked from menu
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *  pev		- pointer to event
 * 
 *  Outputs:
 *  -------
 *  Return	- True if configuration was initiated, else False
 *
 *
 *  Comments:
 *  --------
 *  o This should only be called as the result of a Move function 
 *    being selected from the system menu.
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
Boolean StartClientMove(pcd, pev)

	ClientData *pcd;
	XEvent *pev;
#else /* _NO_PROTO */
Boolean StartClientMove (ClientData *pcd, XEvent *pev)
#endif /* _NO_PROTO */
{
    Window grab_win, junk_win;
    Boolean grabbed;
    int junk;
    Window child;

    /*
     *	Do our grabs if we're just starting out
     */
    if (!configGrab)
    {
	grab_win = GrabWin (pcd, pev);
	if (grab_win == ICON_FRAME_WIN(pcd))
	{
	    wmGD.movingIcon = True;
	}

	if (pev)
	{
	    grabbed = DoGrabs (grab_win, wmGD.configCursor, 
			PGRAB_MASK, pev->xbutton.time, pcd, False);
	}
	else
	{
	    grabbed = DoGrabs (grab_win, wmGD.configCursor, 
			PGRAB_MASK, CurrentTime, pcd, False);
	}
	if (!grabbed)
	{
	    wmGD.movingIcon = False;
	    return (False);
	}
	configGrab = TRUE;
    }

    /* 
     * Set up static variables for succeeding events if we're not 
     * entering with a motion event. If we are, we assume that the
     * preMove variables have been setup.
     */
    if (pev && ((pev->type == ButtonPress) || (pev->type == ButtonRelease)))
    {
	wmGD.preMoveX = pev->xbutton.x_root;
	wmGD.preMoveY = pev->xbutton.y_root;
    }
    else if ((pev && (pev->type != MotionNotify)) || !pev)
    {
	if (!XQueryPointer (DISPLAY, ROOT_FOR_CLIENT(pcd), 
		   &junk_win, &junk_win,
		   &(wmGD.preMoveX), &(wmGD.preMoveY),
		   &junk, &junk, (unsigned int *)&junk))
	{
	    CancelFrameConfig (pcd);
	    return (False);
	}
    }

    offsetX = pcd->clientOffset.x;
    offsetY = pcd->clientOffset.y;

    anyMotion = FALSE;
    moveLastPointerX = wmGD.preMoveX;
    moveLastPointerY = wmGD.preMoveY;

    /* get frame window geometry */
    if (wmGD.movingIcon)
    {
	moveWidth = ICON_WIDTH(pcd);
	moveHeight = ICON_HEIGHT(pcd);

	moveX = ICON_X(pcd);
	moveY = ICON_Y(pcd);

	if (pcd->pSD->useIconBox && P_ICON_BOX(pcd))
	{
	    /* get root coords of icon box bulletin board */
	    XTranslateCoordinates(DISPLAY, 
	        XtWindow(P_ICON_BOX(pcd)->bBoardWidget), ROOT_FOR_CLIENT(pcd), 
	        0, 0, &moveIBbbX, &moveIBbbY, &child);

	    moveX += moveIBbbX;
	    moveY += moveIBbbY;
	}
	else if (pcd->pSD->moveOpaque &&
		 (ICON_DECORATION(pcd) & ICON_ACTIVE_LABEL_PART) &&
		 (wmGD.keyboardFocus == pcd))
	{
	    HideActiveIconText ((WmScreenData *)NULL);
	}
    }
    else 
    {
	if (pcd->maxConfig) {	/* maximized */
	    moveWidth = pcd->maxWidth;
	    moveHeight = pcd->maxHeight;
	    moveX = pcd->maxX;
	    moveY = pcd->maxY;
	}
	else {			/* normal */
	    moveWidth = pcd->clientWidth;
	    moveHeight = pcd->clientHeight;
	    moveX = pcd->clientX;
	    moveY = pcd->clientY;
	}
	ClientToFrame (pcd, &moveX, &moveY, &moveWidth, &moveHeight);
    }

    if (pcd->pSD->moveOpaque)
    {
	opaqueMoveX = moveX;
	opaqueMoveY = moveY;
    }

    /*
     *  Warp pointer to middle of window if started from the menu (no event).
     */
    if ( !pev || pev->type == KeyPress )
    {
	moveLastPointerX = moveX + moveWidth/2;
	moveLastPointerY = moveY + moveHeight/2;

	ForceOnScreen (SCREEN_FOR_CLIENT(pcd), 
		       &moveLastPointerX, &moveLastPointerY);
	if (wmGD.enableWarp)
	{
	    XWarpPointer(DISPLAY, None, ROOT_FOR_CLIENT(pcd), 0, 0, 0, 0, 
		moveLastPointerX, moveLastPointerY);
	}
    }

    /* pop up feedback window */
    if ( !wmGD.movingIcon && (wmGD.showFeedback & WM_SHOW_FB_MOVE))
    {
	DoFeedback (pcd, moveX, moveY, moveWidth, moveHeight, 
		    FB_POSITION, FALSE /* no size checks */);
    }
    
    /* set configuring data */
    wmGD.configAction = MOVE_CLIENT;
    if (pev && pev->type != KeyPress)
	wmGD.configButton = pev->xbutton.button;
    else 
	wmGD.configButton = 0;

    InitEdgeMove();

    return (True);


} /* END OF FUNCTION StartClientMove */



/*************************************<->*************************************
 *
 *  DoGrabs (grab_win, cursor, pmask, grabTime, alwaysGrab)
 *
 *
 *  Description:
 *  -----------
 *  Do the grabs for window configuration
 *
 *
 *  Inputs:
 *  ------
 *  grab_win	- window to grab on
 *  cursor	- cursor shape to attach to the pointer
 *  pmask	-
 *  grabTime	- time stamp
 *  alwaysGrab  - 
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
Boolean DoGrabs (grab_win, cursor, pmask, grabTime, pCD, alwaysGrab)

	Window grab_win;
	Cursor cursor;
	unsigned int pmask;
	Time grabTime;
	ClientData *pCD;
        Boolean alwaysGrab;
#else /* _NO_PROTO */
Boolean DoGrabs (Window grab_win, Cursor cursor, unsigned int pmask, Time grabTime, ClientData *pCD, Boolean alwaysGrab)
#endif /* _NO_PROTO */
{

#ifdef WSM
    Window root;

    if (pCD)
	root = ROOT_FOR_CLIENT(pCD);
    else
	root = RootWindow (DISPLAY, ACTIVE_PSD->screen);

    if (pCD && pCD->pSD->useIconBox && wmGD.movingIcon && P_ICON_BOX(pCD))
#else
    if (pCD->pSD->useIconBox && wmGD.movingIcon && P_ICON_BOX(pCD))
#endif
    {
	/*
	 * Confine the pointer to the icon box clip window
	 */
	if (XGrabPointer(DISPLAY, 
			 grab_win,
			 FALSE,			/* owner_events */
			 pmask,
			 GrabModeAsync,		/* pointer_mode */
			 GrabModeAsync,		/* keyboard_mode */
						/* confine_to window */
			 XtWindow(P_ICON_BOX(pCD)->clipWidget),
			 cursor,
			 grabTime) != GrabSuccess)
	{	
	    return(FALSE);
	}
    }
    else
    {
	/*
	 * Just confine the pointer to the root window
	 */
	if (XGrabPointer(DISPLAY, 
			 grab_win,
			 FALSE,			/* owner_events */
			 pmask,
			 GrabModeAsync,		/* pointer_mode */
			 GrabModeAsync,		/* keyboard_mode */
#ifdef WSM
			 root,
#else
			 ROOT_FOR_CLIENT(pCD),		/* confine_to window */
#endif
			 cursor,
			 grabTime) != GrabSuccess)
	{
	    return(FALSE);
	}
    }

    /*
     * Don't grab keyboard away from menu widget to prevent 
     * hosing of traversal.
     */
    if (!wmGD.menuActive) 
    {
	if ((XGrabKeyboard(DISPLAY, 
			   grab_win,
			   FALSE,			/* owner_events */
			   GrabModeAsync,		/* pointer_mode */
			   GrabModeAsync,		/* keyboard_mode */
			   grabTime)) != GrabSuccess)
	{
	    XUngrabPointer (DISPLAY, CurrentTime);
	    return(FALSE);
	}
    }
    
    
	/* 
	 * If running automation version of mwm, do not grab the server, since
	 * this will confuse the automation input synthesis code.
	 */
#ifndef AUTOMATION

    if (wmGD.freezeOnConfig) 
	
    {
#ifdef WSM
	if (!pCD || ((pCD->pSD->moveOpaque && alwaysGrab) ||
	           (!(pCD->pSD->moveOpaque))))
#else /* WSM */
	if ((pCD->pSD->moveOpaque && alwaysGrab) ||
	    (!(pCD->pSD->moveOpaque)))
#endif /* WSM */
	{
	    XGrabServer(DISPLAY);
        }
    }

#endif /* AUTOMATION */
    
    return(TRUE);
} /* END OF FUNCTION DoGrabs   */


/*************************************<->*************************************
 *
 *  UndoGrabs ()
 *
 *
 *  Description:
 *  -----------
 *  Release the grabs
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void UndoGrabs ()
#else /* _NO_PROTO */
void UndoGrabs (void)
#endif /* _NO_PROTO */
{
    /* erase outline */
    MoveOutline(0, 0, 0, 0);
    XSync (DISPLAY, FALSE /*don't discard events*/);

    /* give up grabs */
    if (wmGD.freezeOnConfig) {
	XUngrabServer(DISPLAY);
    }

    /*
     * Don't Ungrab keyboard away from menu widget to prevent 
     * hosing of traversal.
     */
    if (!wmGD.menuActive)
	XUngrabKeyboard (DISPLAY,CurrentTime);

    XUngrabPointer (DISPLAY, CurrentTime);	/* event time NOT used */
    XFlush (DISPLAY);

} /* END OF FUNCTION UndoGrabs  */



/*************************************<->*************************************
 *
 *  CancelFrameConfig (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Cance a frame configuration (move/resize) operation.
 *
 *
 *  Inputs:
 *  ------
 *  pcd 	- pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void CancelFrameConfig (pcd)

	ClientData *pcd;
#else /* _NO_PROTO */
void CancelFrameConfig (ClientData *pcd)
#endif /* _NO_PROTO */
{

    /* remove keyboard, pointer, and server grabs */
    UndoGrabs();

    /* turn off feedback window */
#ifdef WSM
    if (pcd)
    {
#endif /* WSM */
    HideFeedbackWindow(pcd->pSD);

    /* make sure title bar is popped out */
    if ((wmGD.configAction == MOVE_CLIENT) &&
	(wmGD.gadgetClient == pcd) && (wmGD.gadgetDepressed == FRAME_TITLE))
    {
	PopGadgetOut (pcd, FRAME_TITLE);
	FrameExposureProc(pcd);			/* repaint frame */
    }
    if ((pcd->pSD->moveOpaque) &&
	(wmGD.configAction == MOVE_CLIENT))
	
    {
	if ((pcd->clientState == MINIMIZED_STATE) &&
	    (!(pcd->pSD->useIconBox && P_ICON_BOX(pcd))))
	{
	    /*
	     * Replace icon into pre-move position
	     */

	    XMoveWindow (DISPLAY, ICON_FRAME_WIN(pcd),
			 ICON_X(pcd),  ICON_Y(pcd));
	    if ((ICON_DECORATION(pcd) & ICON_ACTIVE_LABEL_PART))
	    {
		ShowActiveIconText(pcd);
	    }
	}
	else if (! wmGD.movingIcon) /* we are not moving in the iconbox */
	{
	    XMoveWindow (DISPLAY, pcd->clientFrameWin, 
			 opaqueMoveX, opaqueMoveY);
	}
    }
#ifdef WSM
    }
    if (wmGD.configAction == MARQUEE_SELECT)
    {
       dtSendMarqueeSelectionNotification(ACTIVE_PSD, DT_MARQUEE_SELECT_CANCEL, 
			    marqueeX, marqueeY, 0, 0);
    }
#endif /* WSM */

    /* replace pointer if no motion events received */
#ifdef WSM
    if (pcd)
#endif /* WSM */
    if (!anyMotion && wmGD.enableWarp) {
	XWarpPointer(DISPLAY, None, ROOT_FOR_CLIENT(pcd), 
			 0, 0, 0, 0, wmGD.preMoveX, wmGD.preMoveY);
    }
    anyMotion = FALSE;

    /* Clear configuration flags and data */
    wmGD.configAction = NO_ACTION;
    wmGD.configPart = FRAME_NONE;
    wmGD.configSet = False;
    configGrab = FALSE;
    wmGD.movingIcon = FALSE;
    
    /* set the focus back to a reasonable window */
    RepairFocus ();	
} /* END OF FUNCTION  CancelFrameConfig */



/*************************************<->*************************************
 *
 *  CheckEatButtonRelease (pcd, pev)
 *
 *
 *  Description:
 *  -----------
 *  Set up to eat button releases if buttons are down.
 *
 *
 *  Inputs:
 *  ------
 *  pcd  - pointer to client data
 *  pev	 - pointer to key event that caused cancel
 * 
 *  Outputs:
 *  -------
 *  none
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void
CheckEatButtonRelease (pcd, pev)
    ClientData *pcd;
    XEvent *pev;
#else /* _NO_PROTO */
void
CheckEatButtonRelease (ClientData *pcd, XEvent *pev)
#endif /* _NO_PROTO */
{
    Window grab_win;
#ifdef WSM
    Window root;

    if (pcd != (ClientData *)NULL)
	root = ROOT_FOR_CLIENT(pcd);
    else
	root = RootWindow (DISPLAY, ACTIVE_PSD->screen);
#endif /* WSM */

#ifdef WSM
    if (pcd == (ClientData *) NULL)
	grab_win = root;
    else
#endif /* WSM */
    grab_win = GrabWin(pcd, pev);

    if ((pev->type == KeyPress || pev->type == KeyRelease) &&
	(pev->xbutton.state & ButtonMask))
    {
	/*
	 * Some buttons are down... 
	 * Set up conditions to wait for these buttons to go up.
	 */
	if (XGrabPointer(DISPLAY, 
			 grab_win,
			 False,			/* owner_events */
			 ButtonReleaseMask,
			 GrabModeAsync,		/* pointer_mode */
			 GrabModeAsync,		/* keyboard_mode */
#ifdef WSM
			 root,			/* confine_to window */
#else /* WSM */
			 ROOT_FOR_CLIENT(pcd),	/* confine_to window */
#endif /* WSM */
			 wmGD.configCursor,
			 pev->xbutton.time) == GrabSuccess)
	{
	    EatButtonRelease (pev->xbutton.state & ButtonMask);
	}
    }
}


/*************************************<->*************************************
 *
 *  EatButtonRelease (releaseButtons)
 *
 *
 *  Description:
 *  -----------
 *  Eat up button release events
 *
 *
 *  Inputs:
 *  ------
 *  releaseButtons = button mask of button releases to eat
 * 
 *  Outputs:
 *  -------
 *  none
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void
EatButtonRelease (releaseButtons)
    unsigned int releaseButtons;
#else /* _NO_PROTO */
void
EatButtonRelease (unsigned int releaseButtons)
#endif /* _NO_PROTO */
{
    unsigned int new_state;
    XEvent event;

    while (releaseButtons)
    {
#ifdef WSM
	PullExposureEvents ();
#endif /* WSM */
	XMaskEvent (DISPLAY, ButtonReleaseMask, &event);

	if (event.type == ButtonRelease)
	{
	    /* look at the state after this button is released */
	    new_state = 
		event.xbutton.state & ~ButtonStateBit(event.xbutton.button);

	    if (!(new_state & releaseButtons))
	    {
		/* all the buttons we were waiting for have been
		 * released.
		 */

		XUngrabPointer (DISPLAY, event.xbutton.time);
		releaseButtons = 0;
	    }
	}
    }
}



/*************************************<->*************************************
 *
 *  ButtonStateBit (button)
 *
 *
 *  Description:
 *  -----------
 *  Converts a button number to a button state bit
 *
 *
 *  Inputs:
 *  ------
 *  button = button number (Button1, Button2, etc.)
 * 
 *  Outputs:
 *  -------
 *  Return = bit used in xbutton state field 
 *		(Button1Mask, Button2Mask,...)
 *
 *
 *  Comments:
 *  --------
 *  
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
unsigned int
ButtonStateBit (button)
    unsigned int button;
#else /* _NO_PROTO */
unsigned int
ButtonStateBit (unsigned int button)
#endif /* _NO_PROTO */
{
#define MAX_BUTTON 5
    typedef struct {
	unsigned int button;
	unsigned int maskbit;
    } ButtonAssoc;

    static ButtonAssoc bmap[MAX_BUTTON] = {
	 {Button1, Button1Mask},
	 {Button2, Button2Mask},
	 {Button3, Button3Mask},
	 {Button4, Button4Mask},
	 {Button5, Button5Mask},
    };

    int i;
    unsigned int rval = 0;

    for (i = 0; i < MAX_BUTTON; i++)
    {
	if (bmap[i].button == button)
	{
	    rval = bmap[i].maskbit;
	    break;
	}
    }

    return (rval);

}

/*************************************<->*************************************
 *
 *  ConfigCursor (frame_part)
 *
 *
 *  Description:
 *  -----------
 *  return the config cursor that goes with the config part specified
 *
 *
 *  Inputs:
 *  ------
 *  frame_part	- frame part id
 * 
 *  Outputs:
 *  -------
 *  return	- cursor to use 
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
Cursor ConfigCursor (frame_part)

	int frame_part;
#else /* _NO_PROTO */
Cursor ConfigCursor (int frame_part)
#endif /* _NO_PROTO */
{
    Cursor cursor;

    switch (frame_part) {
	case FRAME_RESIZE_NW:
	    cursor = wmGD.stretchCursors[STRETCH_NORTH_WEST];
	    break;
	case FRAME_RESIZE_N:
	    cursor = wmGD.stretchCursors[STRETCH_NORTH];
	    break;
	case FRAME_RESIZE_NE:
	    cursor = wmGD.stretchCursors[STRETCH_NORTH_EAST];
	    break;
	case FRAME_RESIZE_E:
	    cursor = wmGD.stretchCursors[STRETCH_EAST];
	    break;
	case FRAME_RESIZE_SE:
	    cursor = wmGD.stretchCursors[STRETCH_SOUTH_EAST];
	    break;
	case FRAME_RESIZE_S:
	    cursor = wmGD.stretchCursors[STRETCH_SOUTH];
	    break;
	case FRAME_RESIZE_SW:
	    cursor = wmGD.stretchCursors[STRETCH_SOUTH_WEST];
	    break;
	case FRAME_RESIZE_W:
	    cursor = wmGD.stretchCursors[STRETCH_WEST];
	    break;
	default:
	    cursor = wmGD.configCursor;
    }

    return(cursor);

} /* END OF FUNCTION ConfigCursor  */


/*************************************<->*************************************
 *
 *  ReGrabPointer (grab_win, grabTime)
 *
 *
 *  Description:
 *  -----------
 *  Grab the pointer again to change the cursor
 *
 *
 *  Inputs:
 *  ------
 *  grab_win	- 
 *  grabTime	- time stamp
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void ReGrabPointer (grab_win, grabTime)

	Window grab_win;
	Time grabTime;
#else /* _NO_PROTO */
void ReGrabPointer (Window grab_win, Time grabTime)
#endif /* _NO_PROTO */
{
    XGrabPointer(DISPLAY, 
		 grab_win,
		 FALSE,			/* owner_events */
		 PGRAB_MASK,
		 GrabModeAsync,		/* pointer_mode */
		 GrabModeAsync,		/* keyboard_mode */
		 ACTIVE_ROOT,		/* confine_to window */
		 ConfigCursor((int)wmGD.configPart),
		 grabTime);

} /* END OF FUNCTION  ReGrabPointer */



/*************************************<->*************************************
 *
 *  SetPointerResizePart (pcd, pev)
 *
 *
 *  Description:
 *  -----------
 *  Sets the global configuration part for resize based on the current
 *  configuration part and the location of the event
 *
 *
 *  Inputs:
 *  ------
 *  pcd 	- pointer to client data
 *  pev		- pointer to event
 *
 * 
 *  Outputs:
 *  -------
 *  Return	- TRUE if wmGD.configPart is a valid resize part
 *
 *
 *  Comments:
 *  --------
 *  o Assumes the static data for resizing has been set up.
 *************************************<->***********************************/
#ifdef _NO_PROTO
Boolean SetPointerResizePart (pcd, pev)

	ClientData *pcd;
	XEvent *pev;
#else /* _NO_PROTO */
Boolean SetPointerResizePart (ClientData *pcd, XEvent *pev)
#endif /* _NO_PROTO */
{
    int newPart;
    Time grabTime;

    newPart = ResizeType(pcd, pev);	/* get part id for this event */
    grabTime = (pev) ? pev->xmotion.time : CurrentTime;

    switch (wmGD.configPart) {
	case FRAME_NONE:
	    if (newPart == FRAME_NONE)
		return(FALSE);		/* still not valid */

	    wmGD.configPart = newPart;
	    ReGrabPointer(pcd->clientFrameWin, grabTime);
	    return(TRUE);

	case FRAME_RESIZE_N:
	    switch (newPart) {
		case FRAME_RESIZE_W:
		case FRAME_RESIZE_NW:
		    wmGD.configPart = FRAME_RESIZE_NW;
		    ReGrabPointer(pcd->clientFrameWin, grabTime);
		    break;

		case FRAME_RESIZE_E:
		case FRAME_RESIZE_NE:
		    wmGD.configPart = FRAME_RESIZE_NE;
		    ReGrabPointer(pcd->clientFrameWin, grabTime);
		    break;

		default:
		    break;
	    }
	    break;

	case FRAME_RESIZE_E:
	    switch (newPart) {
		case FRAME_RESIZE_N:
		case FRAME_RESIZE_NE:
		    wmGD.configPart = FRAME_RESIZE_NE;
		    ReGrabPointer(pcd->clientFrameWin, grabTime);
		    break;

		case FRAME_RESIZE_S:
		case FRAME_RESIZE_SE:
		    wmGD.configPart = FRAME_RESIZE_SE;
		    ReGrabPointer(pcd->clientFrameWin, grabTime);
		    break;

		default:
		    break;
	    }
	    break;

	case FRAME_RESIZE_S:
	    switch (newPart) {
		case FRAME_RESIZE_E:
		case FRAME_RESIZE_SE:
		    wmGD.configPart = FRAME_RESIZE_SE;
		    ReGrabPointer(pcd->clientFrameWin, grabTime);
		    break;

		case FRAME_RESIZE_W:
		case FRAME_RESIZE_SW:
		    wmGD.configPart = FRAME_RESIZE_SW;
		    ReGrabPointer(pcd->clientFrameWin, grabTime);
		    break;

		default:
		    break;
	    }
	    break;

	case FRAME_RESIZE_W:
	    switch (newPart) {
		case FRAME_RESIZE_N:
		case FRAME_RESIZE_NW:
		    wmGD.configPart = FRAME_RESIZE_NW;
		    ReGrabPointer(pcd->clientFrameWin, grabTime);
		    break;

		case FRAME_RESIZE_S:
		case FRAME_RESIZE_SW:
		    wmGD.configPart = FRAME_RESIZE_SW;
		    ReGrabPointer(pcd->clientFrameWin, grabTime);
		    break;

		default:
		    break;
	    }
	    break;

	case FRAME_RESIZE_NW:
	case FRAME_RESIZE_NE:
	case FRAME_RESIZE_SW:
	case FRAME_RESIZE_SE:
	    break;

	default:
	    return(FALSE);	/* not a valid resize part */
    }
    return(TRUE);

} /* END OF FUNCTION  SetPointerResizePart */


/*************************************<->*************************************
 *
 *  ResizeType (pcd, pev)
 *
 *
 *  Description:
 *  -----------
 *  Returns a resize part ID for an event outside of the current 
 *  resize area.
 *
 *
 *  Inputs:
 *  ------
 *  pcd 	- pointer to client data
 *  pev		- pointer to event
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  o Assumes the static data for resizing has been set up.
 *************************************<->***********************************/
#ifdef _NO_PROTO
int ResizeType(pcd, pev)

	ClientData *pcd;
	XEvent *pev;
#else /* _NO_PROTO */
int ResizeType (ClientData *pcd, XEvent *pev)
#endif /* _NO_PROTO */
{
    int x, y;

    if (!pev) return(FRAME_NONE);

    x = pev->xmotion.x_root;
    y = pev->xmotion.y_root;

    /* if inside all resize areas, then forget it */
    if ( (x > resizeX) &&
	 (y > resizeY) &&
	 (x < (resizeX + resizeWidth - 1)) &&
	 (y < (resizeY + resizeHeight - 1)) )
    {
	return(FRAME_NONE);
    }

    /* left side */
    if (x <= resizeX) {
	if (y < resizeY + (int)pcd->frameInfo.cornerHeight)
	    return (FRAME_RESIZE_NW);
	else if (y >= resizeY + resizeHeight -(int)pcd->frameInfo.cornerHeight)
	    return (FRAME_RESIZE_SW);
	else 
	    return (FRAME_RESIZE_W);
    } 

    /* right side */
    if (x >= resizeX + resizeWidth - 1) {
	if (y < resizeY + (int)pcd->frameInfo.cornerHeight)
	    return (FRAME_RESIZE_NE);
	else if (y >= resizeY + resizeHeight -(int)pcd->frameInfo.cornerHeight)
	    return (FRAME_RESIZE_SE);
	else 
	    return (FRAME_RESIZE_E);
    } 

    /* top side */
    if (y <= resizeY) {
	if (x < resizeX + (int)pcd->frameInfo.cornerWidth)
	    return (FRAME_RESIZE_NW);
	else if (x >= resizeX + resizeWidth - (int)pcd->frameInfo.cornerWidth)
	    return (FRAME_RESIZE_NE);
	else 
	    return (FRAME_RESIZE_N);
    } 

    /* bottom side */
    if (y >= resizeY + resizeHeight - 1) {
	if (x < resizeX + (int)pcd->frameInfo.cornerWidth)
	    return (FRAME_RESIZE_SW);
	else if (x >= resizeX + resizeWidth - (int)pcd->frameInfo.cornerWidth)
	    return (FRAME_RESIZE_SE);
	else 
	    return (FRAME_RESIZE_S);
    } 

    return(FRAME_NONE);

} /* END OF FUNCTION  ResizeType */



/*************************************<->*************************************
 *
 *  FixFrameValues (pcd, pfX, pfY, pfWidth, pfHeight, resizing)
 *
 *
 *  Description:
 *  -----------
 *  Fix up the frame values so that they do not exceed maximum or minimum
 *  size and that at least part of the frame is on screen
 *
 *
 *  Inputs:
 *  ------
 *  pcd 	- pointer to client data
 *  pfX		- pointer to frame x-coord
 *  pfY		- pointer to frame y-coord
 *  pfWidth	- pointer to frame width
 *  pfHeight	- pointer to frame height
 *  resizing      - check size constraints iff TRUE
 *
 * 
 *  Outputs:
 *  -------
 *  *pfX	- fixed up frame x-coord
 *  *pfY	- fixed up frame y-coord
 *  *pfWidth	- fixed up frame width
 *  *pfHeight	- fixed up frame height
 *
 *
 *  Comments:
 *  --------
 *  1. This could be more efficient
 *  2. Interactive resize with aspect ratio constraints may cause part of the
 *     outline to disappear off screen.  The critical case is when the title 
 *     bar disappears ABOVE the screen. 
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void FixFrameValues (pcd, pfX, pfY, pfWidth, pfHeight, resizing)

    ClientData *pcd;
    int        *pfX, *pfY; 
    unsigned int *pfWidth, *pfHeight;
    Boolean    resizing;

#else /* _NO_PROTO */
void FixFrameValues (ClientData *pcd, int *pfX, int *pfY, unsigned int *pfWidth, unsigned int *pfHeight, Boolean resizing)
#endif /* _NO_PROTO */
{
    unsigned int lswidth;
    unsigned int oWidth, oHeight;


    /* 
     * Fix size if resizing and not icon.
     */

    if (resizing && !wmGD.movingIcon)
    {
	FrameToClient(pcd, pfX, pfY, pfWidth, pfHeight);
    
	oWidth = *pfWidth;
	oHeight = *pfHeight;

	FixWindowSize (pcd, pfWidth, pfHeight, 1, 1);

        AdjustPos (pfX, pfY, oWidth, oHeight, *pfWidth, *pfHeight);

	ClientToFrame(pcd, pfX, pfY, pfWidth, pfHeight);
    }

    /* 
     * Don't move if we'd end up totally offscreen 
     */

    if (wmGD.movingIcon)
    {
	lswidth = FRAME_BORDER_WIDTH(pcd);
    }
    else
    {
        lswidth = pcd->frameInfo.lowerBorderWidth;
    }
    if (lswidth < 5) lswidth = 5;

    if (wmGD.movingIcon && P_ICON_BOX(pcd))
    {
	/* 
	 *  Constrain outline to icon box
	 */
	/* left edge of outline */
	if (*pfX < clipX)
	{
	    *pfX = clipX;
	}

	/* top of outline */
	if (*pfY < clipY)
	{
	    *pfY = clipY;
	}

	/* right edge of outline */
	if (((int)*pfX) > ((int)clipX + (int)clipWidth - ((int)*pfWidth)))
	{
	    *pfX = clipX + clipWidth - *pfWidth;
	}

	/* bottom edge of outline */
	if (((int)*pfY) > ((int)clipY + (int)clipHeight - ((int)*pfHeight)))
	{
	    *pfY = clipY + clipHeight - *pfHeight;
	}

    }
    else
    {
	/* 
	 * keep outline on screen 
	 */


	/* keep right border on screen */
	if (*pfX < ((int) lswidth - (int) *pfWidth))
	{
	    *pfX = (int) lswidth - (int) *pfWidth;
	}

	/* keep bottom border on screen */
	if (*pfY < ((int) lswidth - (int) *pfHeight))
	{
	    *pfY = (int) lswidth - (int) *pfHeight;
	}

	/* keep left border on screen */
	if (*pfX > (DisplayWidth(DISPLAY, SCREEN_FOR_CLIENT(pcd)) - 
	    (int) lswidth))
	{
	    *pfX = DisplayWidth(DISPLAY, SCREEN_FOR_CLIENT(pcd)) - 
		(int) lswidth;
	}

	/* keep top border on screen */
	if (*pfY > (DisplayHeight(DISPLAY,SCREEN_FOR_CLIENT(pcd)) - 
	    (int) lswidth))
	{
	    *pfY = DisplayHeight(DISPLAY, SCREEN_FOR_CLIENT(pcd)) - 
		 (int) lswidth;
	}
    }

} /* END OF FUNCTION FixFrameValues */



/*************************************<->*************************************
 *
 *  ForceOnScreen (screen, pX, pY)
 *
 *
 *  Description:
 *  -----------
 *  Correct (if necessary) the coords specified to make them on screen
 *
 *
 *  Inputs:
 *  ------
 *  screen	- screen number
 *  pX		- pointer to x-coord
 *  pY		- pointer to y-coord
 * 
 *  Outputs:
 *  -------
 *  *pX		- x-coord (on screen)
 *  *pY		- y-coord (on screen)
 *
 *
 *  Comments:
 *  --------
 *  XXComments ...
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void ForceOnScreen(screen, pX,pY)

	int screen, *pX, *pY;
#else /* _NO_PROTO */
void ForceOnScreen (int screen, int *pX, int *pY)
#endif /* _NO_PROTO */
{
    if (*pX >= (DisplayWidth(DISPLAY, screen)))
	*pX = DisplayWidth(DISPLAY, screen) - 1;
    else if (*pX < 0)
	*pX = 0;

    if (*pY >= (DisplayHeight(DISPLAY, screen)))
	*pY = DisplayHeight(DISPLAY, screen) - 1;
    else if (*pY < 0)
	*pY = 0;

} /* END OF FUNCTION  ForceOnScreen  */


/*************************************<->*************************************
 *
 *  SetPointerPosition (newX, newY, actualX, actualY)
 *
 *
 *  Description:
 *  -----------
 *  Attempt to set the pointer to position at newX, newY. 
 *
 *
 *  Inputs:
 *  ------
 *  newX	- X-coordinate to set pointer at
 *  newY	- Y-coordinate to set pointer at
 *
 * 
 *  Outputs:
 *  -------
 *  *actualX	- actual X-coord of pointer on return
 *  *actualY	- actual Y-coord of pointer on return
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void SetPointerPosition (newX, newY, actualX, actualY)

	int newX, newY;
	int *actualX, *actualY;
#else /* _NO_PROTO */
void SetPointerPosition (int newX, int newY, int *actualX, int *actualY)
#endif /* _NO_PROTO */
{
    int junk;
    Window junk_win;

    /*
     * Warp pointer ...
     */
    if (wmGD.enableWarp)
    {
	XWarpPointer(DISPLAY, None, ACTIVE_ROOT, 
	     0, 0, 0, 0, newX, newY);
    }


    /*
     * Get pointer position
     * NOTE: if we are not warping, we don't want to do the Query pointer,
     *       hence enableWarp is tested first.
     */
    if (!wmGD.enableWarp || 
        !XQueryPointer (DISPLAY, ACTIVE_ROOT, &junk_win, &junk_win, 
		actualX, actualY, &junk, &junk, (unsigned int *)&junk))

    {
	/* failed to get pointer position or not warping, return something */
	*actualX = newX;
	*actualY = newY;
    }

} /* END OF FUNCTION SetPointerPositio  */



/*************************************<->*************************************
 *
 *  GetConfigEvent (display, window, mask, curX, curY, oX, oY, 
 *     oWidth, oHeight, pev,)
 *
 *
 *  Description:
 *  -----------
 *  Get next configuration event
 *
 *
 *  Inputs:
 *  ------
 *  display	- pointer to display
 *  window	- window to get event relative to
 *  mask	- event mask - acceptable events to return
 *  pev		- pointer to a place to put the event
 *  curX	- current X value of pointer
 *  curY	- current Y value of pointer
 *  oX		- X value of outline
 *  oY		- Y value of outline
 *  oWidth	- width of outline
 *  oHeight	- height of outline
 * 
 *  Outputs:
 *  -------
 *  *pev	- event returned.
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void GetConfigEvent (display, window, mask, curX, curY, oX, oY, oWidth, oHeight, pev)

	Display *display;
	Window window;
	unsigned long mask;
	XEvent *pev;
	int curX, curY;
	int oX, oY;
	unsigned oWidth, oHeight;
#else /* _NO_PROTO */
void GetConfigEvent (Display *display, Window window, unsigned long mask, int curX, int curY, int oX, int oY, unsigned oWidth, unsigned oHeight, XEvent *pev)
#endif /* _NO_PROTO */
{
    Window root_ret, child_ret;
    int root_x, root_y, win_x, win_y;
    unsigned int mask_ret;
    Boolean polling;
    int pollCount;
    Boolean gotEvent;
    Boolean eventToReturn = False;

    while (!eventToReturn)
    {
	/* 
	 * Suck up pointer motion events 
	 */
	gotEvent = False;
	while (XCheckWindowEvent(display, window, mask, pev))
	{
	    gotEvent = True;
	    if (pev->type != MotionNotify)
		break;
	}

	/*
	 * Only poll if we are warping the pointer. 
	 * (uses PointerMotionHints exclusively).
	 */
	polling = wmGD.enableWarp;
	pollCount = CONFIG_POLL_COUNT;

	if (!gotEvent && (polling || !wmGD.freezeOnConfig))
	{
	    /*
             * poll for events and flash the frame outline 
	     * if not move opaque
	     */

	    while (True)
	    {
		if (XCheckWindowEvent(display, window, 
				      (mask & ~PointerMotionMask), pev))
		{
		    gotEvent = True;
		    break;
		}
		
		if (!wmGD.freezeOnConfig && !wmGD.pActiveSD->moveOpaque)
		{
                    /* flash the outline if server is not grabbed */
		    MoveOutline (oX, oY, oWidth, oHeight);
		}

		if (!XQueryPointer (display, window, &root_ret, &child_ret, 
			&root_x, &root_y, &win_x, &win_y, &mask_ret))
		{
		    continue;	/* query failed, try again */
		}

		if ((root_x != curX) || (root_y != curY))
		{
		    /* 
		     * Pointer moved to a new position.
		     * Cobble a motion event together. 
		     * NOTE: SOME FIELDS NOT SET !!! 
		     */

		    pev->type = MotionNotify;
		    /* pev->xmotion.serial = ??? */
		    pev->xmotion.send_event = False;
		    pev->xmotion.display = display;
		    pev->xmotion.window = root_ret;
		    pev->xmotion.subwindow = child_ret;
		    pev->xmotion.time = CurrentTime;		/* !!! !!! */
		    pev->xmotion.x = root_x;
		    pev->xmotion.y = root_y;
		    pev->xmotion.x_root = root_x;
		    pev->xmotion.y_root = root_y;
		    /* pev->xmotion.state = ??? */
		    /* pev->xmotion.is_hint  = ???? */
		    /* pev->xmotion.same_screen = ??? */

		    eventToReturn = True;
		    break;	/* from while loop */
		}
		else if (wmGD.freezeOnConfig)
		{
		    if (!(--pollCount))
		    {
			/* 
			 * No pointer motion in some time. Stop polling
			 * and wait for next event.
			 */
			polling = False;
			break; /* from while loop */ 
		    }
		}
	    }  /* end while */
	}

	if (!gotEvent && !polling && wmGD.freezeOnConfig) 
	{
	    /* 
	     * Wait for next event on window 
	     */

	    XWindowEvent (display, window, mask, pev);
	    gotEvent = True;
	}

	if (gotEvent)
	{
	    eventToReturn = True;
	    if (pev->type == MotionNotify &&
		pev->xmotion.is_hint == NotifyHint)
	    {
		/* 
		 * "Ack" the motion notify hint 
		 */
		if ((XQueryPointer (display, window, &root_ret, 
			&child_ret, &root_x, &root_y, &win_x, 
			&win_y, &mask_ret)) &&
		    ((root_x != curX) ||
		     (root_y != curY)))
		{
		    /*
		     * The query pointer values say that the pointer
		     * moved to a new location.
		     */
		    pev->xmotion.window = root_ret;
		    pev->xmotion.subwindow = child_ret;
		    pev->xmotion.x = root_x;
		    pev->xmotion.y = root_y;
		    pev->xmotion.x_root = root_x;
		    pev->xmotion.y_root = root_y;

		}
		else {
		    /*
		     * Query failed. Change curX to force position
		     * to be returned on first sucessful query.
		     */
		    eventToReturn = False;
		    curX++;
		}
	    }
	}
    } /* end while */

} /* END OF FUNCTION GetConfigEvent  */


/*************************************<->*************************************
 *
 *  SetOutline (pOutline, x, y, width, height, fatness)
 *
 *
 *  Description:
 *  -----------
 *  Sets the outline of for config/move/placement operations
 *
 *
 *  Inputs:
 *  ------
 *  pOutline	- ptr to outline structure to fill in
 *  x		- x of upper-left corner of outline
 *  y		- y of upper-left corner of outline
 *  width	- width of outline.
 *  height	- height of outline.
 *  fatness	- pixel-width of outline
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  o Be sure that pOutline points to a big enough area of memory 
 *    for the outline to be set!
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void SetOutline (pOutline, x, y, width, height, fatness)

	XSegment *pOutline;
	int x,y,fatness;
	unsigned int width, height;
#else /* _NO_PROTO */
void SetOutline (XSegment *pOutline, int x, int y, unsigned int width, unsigned int height, int fatness)
#endif /* _NO_PROTO */
{
    int i;

    for (i=0; i<fatness; i++)
    {
	pOutline->x1 = x;
	pOutline->y1 = y;
	pOutline->x2 = x + width -1;
	pOutline++->y2 = y;

	pOutline->x1 = x + width -1;
	pOutline->y1 = y;
	pOutline->x2 = x + width -1;
	pOutline++->y2 = y + height - 1;

	pOutline->x1 = x + width -1;
	pOutline->y1 = y + height - 1;
	pOutline->x2 = x;
	pOutline++->y2 = y + height - 1;

	pOutline->x1 = x;
	pOutline->y1 = y + height - 1;
	pOutline->x2 = x;
	pOutline++->y2 = y;

	/*
	 * Modify values for next pass (if any)
	 * Next outline will be on inside of current one.
	 */
        x += 1;
	y += 1;
	width -= 2;
	height -= 2;
    }

} /* END OF FUNCTION SetOutline  */


/*************************************<->*************************************
 *
 *  AdjustPos (pX, pY, oWidth, oHeight, nWidth, nHeight)
 *
 *
 *  Description:
 *  -----------
 *  Adjusts the position according to wmGD.configPart and any change in
 *  client size.
 *
 *
 *  Inputs:
 *  ------
 *  pX, pY -- pointers to positions
 *  oWidth, oHeight -- original dimensions
 *  nWidth, nHeight -- new dimensions
 *  wmGD.configPart
 * 
 *  Outputs:
 *  -------
 *  pX, pY -- pointers to adjusted positions
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void AdjustPos (pX, pY, oWidth, oHeight, nWidth, nHeight)

    int          *pX, *pY;
    unsigned int  oWidth, oHeight;
    unsigned int  nWidth, nHeight;

#else /* _NO_PROTO */
void AdjustPos (int *pX, int *pY, unsigned int oWidth, unsigned int oHeight, unsigned int nWidth, unsigned int nHeight)
#endif /* _NO_PROTO */
{
    switch (wmGD.configPart) 
    {
        case FRAME_RESIZE_NW:
	    /* anchor lower right corner */
	    *pX += oWidth - nWidth;
	    *pY += oHeight - nHeight;
	    break;

	case FRAME_RESIZE_N:
	    /* anchor bottom */
	    *pY += oHeight - nHeight;
	    break;

	case FRAME_RESIZE_NE:
	    /* anchor lower left corner */
	    *pY += oHeight - nHeight;
	    break;

	case FRAME_RESIZE_E:
	    /* anchor left side */
	    break;

	case FRAME_RESIZE_SE:
	    /* anchor upper left corner */
	    break;

	case FRAME_RESIZE_S:
	    /* anchor top */
	    break;

	case FRAME_RESIZE_SW:
	    /* anchor upper right corner */
	    *pX += oWidth - nWidth;
	    break;

	case FRAME_RESIZE_W:
	    /* anchor right side */
	    *pX += oWidth - nWidth;
	    break;

	default:
	    break;
    }

} /* END OF FUNCTION AdjustPos */



/*************************************<->*************************************
 *
 *  GrabWin (pcd, pev)
 *
 *
 *  Description:
 *  -----------
 *  return window to do grab on for config operation
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- ptr to client data
 *  pev		- ptr to event
 *
 *  Outputs:
 *  -------
 *  Return 	- window 
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
Window GrabWin (pcd, pev)

	ClientData *pcd;
	XEvent *pev;
#else /* _NO_PROTO */
Window GrabWin (ClientData *pcd, XEvent *pev)
#endif /* _NO_PROTO */
{
    Window grab_win;
    
    /*
     * The grab window is the icon if the client is minimized
     * or if the event was on a "normalized" icon in the icon box.
     */

    if ((pcd->clientState == MINIMIZED_STATE) ||
        (pcd->pSD->useIconBox && pev &&
         ((pev->xany.window == ICON_FRAME_WIN(pcd)) ||
          (pev->xany.window == ACTIVE_ICON_TEXT_WIN))))
    {
        grab_win = ICON_FRAME_WIN(pcd);
    }
    else if (pev &&
             (pev->xany.window == pcd->clientFrameWin ||
              pev->xany.window == pcd->clientBaseWin ))
    {
        grab_win = pcd->clientFrameWin;
    }
    else if (pcd->pSD->useIconBox  &&
             P_ICON_BOX(pcd) &&
             wmGD.grabContext == F_SUBCONTEXT_IB_WICON)
    {
        grab_win = ICON_FRAME_WIN(pcd);
    }
    else
    {
        grab_win = pcd->clientFrameWin;
    }

    return (grab_win);

} /* END OF FUNCTION GrabWin */
#ifdef WSM

/*************************************<->*************************************
 *
 *  HandleMarqueeSelect (pSD, event)
 *
 *
 *  Description:
 *  -----------
 *  Does a marquee selection on the root window
 *
 *
 *  Inputs:
 *  ------
 *
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  Selection info is dumped into a root window property:
 *    _DT_MARQUEE_SELECTION
 * 
 *************************************<->***********************************/
void
#ifdef _NO_PROTO
HandleMarqueeSelect (pSD, pev)
    WmScreenData *pSD;
    XEvent *pev;
#else /* _NO_PROTO */
HandleMarqueeSelect (WmScreenData *pSD, XEvent *pev)
#endif /* _NO_PROTO */
{
    Window grab_win;
    Boolean bDone;
    XEvent event;

    grab_win = RootWindow (DISPLAY, pSD->screen);
    bDone = False;

    while (!bDone && (wmGD.configAction == MARQUEE_SELECT))
    {
	if (!pev) 	/* first time through will already have event */
	{
	    pev = &event;

	    GetConfigEvent(DISPLAY, grab_win, CONFIG_MASK, 
		pointerX, pointerY, resizeX, resizeY,
		resizeWidth, resizeHeight, &event);
	}

	if (pev->type == MotionNotify)
	{
	    pointerX = pev->xmotion.x_root;
	    pointerY = pev->xmotion.y_root;
	    UpdateMarqueeSelectData (pSD);
	}
	else if (pev->type == KeyPress) {

	    /* 
	     * Handle key event. 
	     */
	    bDone = HandleMarqueeKeyPress (pSD, pev);
	}
	else if (pev->type == ButtonRelease) {

	    /*
	     *  Update (x,y) to the location of the button release
	     */
	    pointerX = pev->xbutton.x_root;
	    pointerY = pev->xbutton.y_root;
	    UpdateMarqueeSelectData (pSD);
	    CompleteFrameConfig ((ClientData *)NULL, pev);
	    bDone = True;
	}
	else  {
	    pev = NULL;
	    continue;			/* ignore this event */
	}

	if (!bDone)
	{
	    MoveOutline (marqueeX, marqueeY, marqueeWidth, marqueeHeight);
	}

	pev = NULL;	/* reset event pointer */

    }  /* end while */

} /* END OF FUNCTION HandleMarqueeSelect */


/*************************************<->*************************************
 *
 *  StartMarqueeSelect ()
 *
 *
 *  Description:
 *  -----------
 *  Start marquee selection on root window
 *
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void
#ifdef _NO_PROTO
StartMarqueeSelect(pSD, pev)
	WmScreenData *pSD;
	XEvent *pev;
#else /* _NO_PROTO */
StartMarqueeSelect(WmScreenData *pSD, XEvent *pev)
#endif /* _NO_PROTO */
{
    Window grab_win, junk_win;
    Boolean grabbed;
    int big_inc;
    int junk, junkX, junkY;

    if (!pSD->bMarqueeSelectionInitialized)
    {
	/* 
	 * If we haven't initialized the marquee selection messaging
	 * then do so here before we do any grabs. Sending a dummy
	 * message will do.
	 *
	 * (If we move off of ICCCM messaging, then this can go away.)
	 */
	dtSendMarqueeSelectionNotification(pSD, DT_MARQUEE_SELECT_END, 
				0, 0, 0, 0);
	pSD->bMarqueeSelectionInitialized = True;
    }

    /*
     *	Do our grabs 
     */
    if (!configGrab)
    {
	grab_win = RootWindow (DISPLAY, pSD->screen);

	if (pev)
	{
	    grabbed = DoGrabs (grab_win, wmGD.configCursor, 
			PGRAB_MASK, pev->xbutton.time, NULL, True);
	}
	else
	{
	    grabbed = DoGrabs (grab_win, wmGD.configCursor, 
			PGRAB_MASK, CurrentTime, NULL, True);
	}
	if (!grabbed)
	{
	    return;
	}
	configGrab = TRUE;
    }
    else
    {
	/* continue with the configuration in progress (!!!) */
	return;
    }

    /* 
     * Set up static variables for succeeding events 
     */
    if ((pev->type == ButtonPress) || (pev->type == ButtonRelease))
    {
	pointerX = pev->xbutton.x_root;
	pointerY = pev->xbutton.y_root;
    }
    else if (!XQueryPointer (DISPLAY, pSD->rootWindow,
			&junk_win, &junk_win,
		   	&pointerX, &pointerY, 
			&junk, &junk, (unsigned int *)&junk))
    {
	CancelFrameConfig ((ClientData *)NULL);	/* release grabs */
	return;
    }

    /* save start values to see where we came from */
    marqueeX = startX = pointerX;
    marqueeY = startY = pointerY;
    marqueeWidth0 = marqueeWidth = 0;
    marqueeHeight0 = marqueeHeight = 0;
    marqueeAnchor = ANCHOR_NW;

    /* compute increment value for dynamic update */
    big_inc = DisplayWidth (DISPLAY, pSD->screen) / 20;

    /* set configuring data */
    wmGD.configAction = MARQUEE_SELECT;
    wmGD.configButton = pev ? pev->xbutton.button: 0;

    dtSendMarqueeSelectionNotification(pSD, DT_MARQUEE_SELECT_BEGIN, 
	    marqueeX, marqueeY, marqueeWidth, marqueeHeight);

} /* END OF FUNCTION StartMarqueeSelect  */


/*************************************<->*************************************
 *
 *  UpdateMarqueeSelectData ()
 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 *  pSD		- pointer to screen data
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *************************************<->***********************************/
#ifdef _NO_PROTO
void UpdateMarqueeSelectData (pSD)
	WmScreenData *pSD;
#else /* _NO_PROTO */
void UpdateMarqueeSelectData (WmScreenData *pSD)
#endif /* _NO_PROTO */
{
    /* validate and update anchor point and marquee data */
    
    switch (marqueeAnchor) 
    {
      case ANCHOR_NW:
	marqueeWidth = pointerX - marqueeX;
	marqueeHeight = pointerY - marqueeY;

	if (marqueeWidth < 0)
	{
	    marqueeWidth = -marqueeWidth;
	    marqueeX = pointerX;

	    if (marqueeHeight < 0)
	    {
		marqueeHeight = -marqueeHeight;
		marqueeY = pointerY;
		marqueeAnchor = ANCHOR_SE;
	    }
	    else
	    {
		marqueeAnchor = ANCHOR_NE;
	    }
	}
	else if (marqueeHeight < 0)
	{
	    marqueeHeight = -marqueeHeight;
	    marqueeY = pointerY;
	    marqueeAnchor = ANCHOR_SW;
	}
	break;

      case ANCHOR_NE:
	marqueeWidth += marqueeX - pointerX;
	marqueeHeight = pointerY - marqueeY;
	marqueeX = pointerX;

	if (marqueeWidth < 0)
	{
	    marqueeWidth = -marqueeWidth;
	    marqueeX = pointerX - marqueeWidth;

	    if (marqueeHeight < 0)
	    {
		marqueeHeight = -marqueeHeight;
		marqueeY = pointerY;
		marqueeAnchor = ANCHOR_SW;
	    }
	    else
	    {
		marqueeAnchor = ANCHOR_NW;
	    }
	}
	else if (marqueeHeight < 0)
	{
	    marqueeHeight = -marqueeHeight;
	    marqueeY = pointerY;
	    marqueeAnchor = ANCHOR_SE;
	}
	break;

      case ANCHOR_SE:
	marqueeWidth += marqueeX - pointerX;
	marqueeHeight += marqueeY - pointerY;
	marqueeX = pointerX;
	marqueeY = pointerY;

	if (marqueeWidth < 0)
	{
	    marqueeWidth = -marqueeWidth;
	    marqueeX = pointerX - marqueeWidth;

	    if (marqueeHeight < 0)
	    {
		marqueeHeight = -marqueeHeight;
		marqueeY = pointerY - marqueeHeight;
		marqueeAnchor = ANCHOR_NW;
	    }
	    else
	    {
		marqueeAnchor = ANCHOR_SW;
	    }
	}
	else if (marqueeHeight < 0)
	{
	    marqueeHeight = -marqueeHeight;
	    marqueeY = pointerY - marqueeHeight;
	    marqueeAnchor = ANCHOR_NE;
	}
	break;

      case ANCHOR_SW:
	marqueeWidth = pointerX - marqueeX;
	marqueeHeight += marqueeY - pointerY;
	marqueeY = pointerY;

	if (marqueeWidth < 0)
	{
	    marqueeWidth = -marqueeWidth;
	    marqueeX = pointerX;

	    if (marqueeHeight < 0)
	    {
		marqueeHeight = -marqueeHeight;
		marqueeY = pointerY - marqueeHeight;
		marqueeAnchor = ANCHOR_NE;
	    }
	    else
	    {
		marqueeAnchor = ANCHOR_SE;
	    }
	}
	else if (marqueeHeight < 0)
	{
	    marqueeHeight = -marqueeHeight;
	    marqueeY = pointerY - marqueeHeight;
	    marqueeAnchor = ANCHOR_NW;
	}
	break;
    }

    if ((wmGD.marqueeSelectGranularity > 0) &&
        ((ABS(marqueeWidth-marqueeWidth0) > wmGD.marqueeSelectGranularity) ||
	 (ABS(marqueeHeight-marqueeHeight0)>wmGD.marqueeSelectGranularity)))
    {
	dtSendMarqueeSelectionNotification(pSD, DT_MARQUEE_SELECT_CONTINUE, 
		marqueeX, marqueeY, marqueeWidth, marqueeHeight);

	marqueeWidth0 = marqueeWidth;
	marqueeHeight0 = marqueeHeight;
    }
}


/*************************************<->*************************************
 *
 *  HandleMarqueeKeyPress (pSD, pev)
 *
 *
 *  Description:
 *  -----------
 *  Handles keypress events during resize of window
 *
 *
 *  Inputs:
 *  ------
 *  pSD 	- pointer to screen data
 *  pev		- pointer to event
 *
 * 
 *  Outputs:
 *  -------
 *  Return	- True if this event completes (or cancels) resizing 
 * 
 * 
 *  Comments: 
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
Boolean HandleMarqueeKeyPress (pSD, pev) 

	WmScreenData *pSD;
	XEvent *pev;
#else /* _NO_PROTO */
Boolean HandleMarqueeKeyPress (WmScreenData *pSD, XEvent *pev)
#endif /* _NO_PROTO */
{
    KeySym keysym;
    Boolean control;
    int keyMult;
    XEvent KeyEvent;

    /*
     * Compress repeated keys 
     */
    keyMult = 1;
    while (keyMult <= 10 && 
	      XCheckIfEvent (DISPLAY, &KeyEvent, IsRepeatedKeyEvent, 
	      (char *) pev))
    {
	  keyMult++;
    }

    keysym = XKeycodeToKeysym (DISPLAY, pev->xkey.keycode, 0);
    control = (pev->xkey.state & ControlMask) != 0;

    switch (keysym) {

	case XK_Return:
	    CompleteFrameConfig ((ClientData *)NULL, pev);
	    return (True);

	case XK_Escape:
	    CancelFrameConfig ((ClientData *)NULL);
	    CheckEatButtonRelease ((ClientData *)NULL, pev);
	    return (True);

	default:
	    return (False);		/* ignore this key */

    } /* end switch(keysym) */

} /* END OF FUNCTION HandleResizeKeyPress */
#endif /* WSM */
