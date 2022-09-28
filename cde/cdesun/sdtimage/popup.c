#ifndef lint
static char sccsid[] = "@(#)popup.c 1.4 94/11/14";
#endif

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

#include <X11/X.h>
#include <X11/Intrinsic.h>
#include <X11/Xutil.h>
#include <Xm/Xm.h>
#include "popup.h"

#define TITLE_LINE_HEIGHT    20
#define BORDER		      5


typedef enum {
        POPUP_RIGHT,       /* Place popup to right of baseframe */
        POPUP_LEFT,        /* Place popup to left of baseframe */
        POPUP_ABOVE,       /* Place popup above baseframe */
        POPUP_BELOW,       /* Place popup below baseframe */
        POPUP_NONE	   /* place holder! */
} PopupLoc;


/*  Function:       ScreenSize
 *
 *  Description:    Get the width and height of the screen in pixels
 *
 *  Parameters:     width_p         Pointer to an integer to place width
 *                  height_p        Pointer to an integer to place height
 *
 *  Returns:        None.
 */
static void
ScreenSize(Widget widget, int *width_p, int *height_p) 
{
	Display *dpy  = XtDisplay(widget) ;
	int screen    = DefaultScreen(dpy) ;

	*width_p  = DisplayWidth(dpy, screen) ;
	*height_p = DisplayHeight(dpy, screen) ;
}


/*  Function:       ForcePopupOnScreen
 *
 *  Description:    Make sure that the specified widget appears entirely
 *                  on the screen.
 *
 *                  You specify the x and y where you would like the
 *                  popup to appear.  If this location would cause any
 *                  portion of the popup to appear off of the screen
 *                  then the routine makes the minimum adjustments
 *                  necessary to move it onto the screen.
 *
 *                  NOTE:   The following coordinates must be specified
 *                          relative to the screen origin *not* the
 *                          parent widget!
 *
 *  Parameters:     popup_x_p       Pointer to x location where you would
 *                                  like the popup to appear.  If the popup
 *                                  is moved this is updated to reflect
 *                                  the new position.
 *                  popup_y_p       Pointer to y location where you would
 *                                  like the popup to appear.  If the popup
 *                                  is moved this is updated to reflect
 *                                  the new position.
 *
 *                  popup           Popup`s widget.
 *
 *  Returns:        TRUE    The popup was moved
 *                  FALSE   The popup was not moved
 */
static int
ForcePopupOnScreen(Widget popup, int *popup_x_p, int *popup_y_p)
{
	Dimension popup_width, popup_height ;
	Position left, top ;
	int popup_x, popup_y ;
	int n, rcode, screen_width, screen_height ;

	popup_x = *popup_x_p ;
  	popup_y = *popup_y_p ;
 
	/* Get the screen size */
 
  	ScreenSize(popup, &screen_width, &screen_height) ;

  	XtVaGetValues(popup,
		XmNwidth,  &popup_width,
                XmNheight, &popup_height,
                0) ;

 
	/* Make sure frame does not go off side of screen */
 
	n = popup_x + (int) popup_width ;
	if (n > screen_width) 
		popup_x -= (n - screen_width) ;
	else 
		if (popup_x < 0) popup_x = 0 ;
 

	/* Make sure frame doen't go off top or bottom */
 
	n = popup_y + (int) popup_height ;
	if (n > screen_height) 
		popup_y -= n - screen_height ;
  	else 
		if (popup_y < 0) popup_y = 0 ;
 

	/* Set location and return */
 
	left = (Position) popup_x ;
	top  = (Position) popup_y ;

	XtVaSetValues(popup,
		XmNx, left,
		XmNy, top,
		0) ;
 
	if (popup_x != *popup_x_p || popup_y != *popup_y_p) 
		rcode = TRUE ;
	else
		rcode = FALSE ;

  	*popup_x_p = popup_x ;
	*popup_y_p = popup_y ;

	return(rcode) ;
}

 
/*
 * Function:       PositionPopup
 *
 * Description:    Position a popup relative to the parent frame
 *                 making sure it doesn't go off of the screen.
 *
 * Parameters:     base            Popup's parent widget
 *                 popup           Popup widget
 *  
 *  When choosing where to place the  popup relative to the
 *  primary window, the priority used is:
 *
 *  1. To the right
 *  2. beneath
 *  3. left
 *  4. above
 *
 */

void
PositionPopup(Widget base, Widget popup)
{
	int bh, bw, bx, by, ph, pw, px, py;
	int screen_width, screen_height;
	int l_overlap = 0, r_overlap = 0, a_overlap = 0, b_overlap = 0;
	Position base_x, base_y, popup_x, popup_y;
	Dimension base_width, base_height, popup_width, popup_height;
	PopupLoc location_op = POPUP_NONE;
	

	XtVaGetValues(base,
		XmNx,	&base_x,
		XmNy, 	&base_y,
		XmNwidth, &base_width,
		XmNheight, &base_height,
                0);

	bx = (int) base_x;
	by = (int) base_y;
	bw = (int) base_width;
	bh = (int) base_height;

	XtVaGetValues(popup,
		XmNx,	  &popup_x,
		XmNy,  	  &popup_y,
		XmNwidth, &popup_width,
		XmNheight,&popup_height,
		0);

	px = (int) popup_x;
	py = (int) popup_y;
	pw = (int) popup_width;
	ph = (int) popup_height;


	ScreenSize(popup, &screen_width, &screen_height);


	if (pw + BORDER + BORDER <= (screen_width - bw - bx)) {
	    if (location_op == POPUP_NONE)
		location_op = POPUP_RIGHT ;
	}
	else
	    r_overlap = pw + BORDER + BORDER - (screen_width - bw - bx);

	if (bx >= pw) {
	    if (location_op == POPUP_NONE)
		location_op = POPUP_LEFT ;
	}
	else
	    l_overlap = pw + BORDER + BORDER - bx;
/*
	if (ph + TITLE_LINE_HEIGHT + BORDER + BORDER <= (screen_height - bh - by)) {
	   if (location_op == POPUP_NONE)
		location_op = POPUP_BELOW ;
	}
	else
	    b_overlap = ph + TITLE_LINE_HEIGHT + BORDER + BORDER - (screen_height - bh - by);

	if (by >= ph + TITLE_LINE_HEIGHT + BORDER + BORDER) {
	    if (location_op == POPUP_NONE)
		location_op = POPUP_ABOVE ;
	}
	else 	
	    a_overlap = ph + TITLE_LINE_HEIGHT + BORDER + BORDER - by;

	if (location_op == POPUP_NONE)
		if (r_overlap <= b_overlap)
			if (r_overlap <= l_overlap)
				if (r_overlap <= a_overlap)
					location_op = POPUP_RIGHT;	
				else
					location_op = POPUP_ABOVE;
			else
				if (l_overlap <= a_overlap)
					location_op = POPUP_LEFT;
				else
					location_op = POPUP_ABOVE;
		else
			if (b_overlap <= l_overlap)
				if (b_overlap <= a_overlap)
					location_op = POPUP_BELOW;
				else
					location_op = POPUP_ABOVE;	
			else
				if (l_overlap <= a_overlap)
					location_op  = POPUP_LEFT;
				else
					location_op = POPUP_ABOVE
 */
	if (location_op == POPUP_NONE) {
	  if (r_overlap <= l_overlap) 
	    location_op = POPUP_RIGHT;	
	  else 
	    location_op = POPUP_LEFT;
	}

	switch (location_op)
    	{
		case POPUP_RIGHT: 
				px = bx + bw + BORDER;
               	                py = by - TITLE_LINE_HEIGHT - BORDER;
                               	break;

      		case POPUP_LEFT: 
				px = bx - pw - (3 * BORDER) - 1;
                               	py = by - TITLE_LINE_HEIGHT - BORDER;
                              	break ;

      		case POPUP_ABOVE: 
				px = bx - BORDER;
                               	py = by - ph - (2 * TITLE_LINE_HEIGHT) - (3 * BORDER);
                               	break ;

      		case POPUP_BELOW: 
				px = bx - BORDER;
                               	py = by + bh + BORDER;
                               	break ;

      		default : 
				px = bx + (bw - pw) / 2;
                               	py = by + (bh - ph) / 2;
    	}
	ForcePopupOnScreen(popup, &px, &py) ; 
	return;
}


void
PositionFrame(int x, int y, int w, int h, Widget popup)
{
	int bh, bw, bx, by, ph, pw, px, py;
	int screen_width, screen_height;
	int l_overlap = 0, r_overlap = 0, a_overlap = 0, b_overlap = 0;
	Position popup_x, popup_y;
	Dimension popup_width, popup_height;
	PopupLoc location_op = POPUP_NONE;
	
	bx = (int) x;
	by = (int) y;
	bw = (int) w;
	bh = (int) h;

	XtVaGetValues(popup,
		XmNx,	  &popup_x,
		XmNy,  	  &popup_y,
		XmNwidth, &popup_width,
		XmNheight,&popup_height,
		0);

	px = (int) popup_x;
	py = (int) popup_y;
	pw = (int) popup_width;
	ph = (int) popup_height;


	ScreenSize(popup, &screen_width, &screen_height);


	if (pw + BORDER + BORDER <= (screen_width - bw - bx)) {
	    if (location_op == POPUP_NONE)
		location_op = POPUP_RIGHT ;
	}
	else
	    r_overlap = pw + BORDER + BORDER - (screen_width - bw - bx);

	if (bx >= pw) {
	    if (location_op == POPUP_NONE)
		location_op = POPUP_LEFT ;
	}
	else
	    l_overlap = pw + BORDER + BORDER - bx;
/*
 	if (ph + TITLE_LINE_HEIGHT + BORDER + BORDER <= (screen_height - bh - by)) {
	   if (location_op == POPUP_NONE)
		location_op = POPUP_BELOW ;
	}
	else
	    b_overlap = ph + TITLE_LINE_HEIGHT + BORDER + BORDER - (screen_height - bh - by);

	if (by >= ph + TITLE_LINE_HEIGHT + BORDER + BORDER) {
	    if (location_op == POPUP_NONE)
		location_op = POPUP_ABOVE ;
	}
	else 	
	    a_overlap = ph + TITLE_LINE_HEIGHT + BORDER + BORDER - by;

 
	if (location_op == POPUP_NONE)
		if (r_overlap <= b_overlap)
			if (r_overlap <= l_overlap)
				if (r_overlap <= a_overlap)
					location_op = POPUP_RIGHT;	
				else
					location_op = POPUP_ABOVE;
			else
				if (l_overlap <= a_overlap)
					location_op = POPUP_LEFT;
				else
					location_op = POPUP_ABOVE;
		else
			if (b_overlap <= l_overlap)
				if (b_overlap <= a_overlap)
					location_op = POPUP_BELOW;
				else
					location_op = POPUP_ABOVE;	
			else
				if (l_overlap <= a_overlap)
					location_op  = POPUP_LEFT;
				else
					location_op = POPUP_ABOVE;
*/
	if (location_op == POPUP_NONE) {
	  if (r_overlap <= l_overlap) 
	    location_op = POPUP_RIGHT;	
	  else 
	    location_op = POPUP_LEFT;
	}

	switch (location_op)
    	{
		case POPUP_RIGHT: 
				px = bx + bw + BORDER;
               	                py = by - TITLE_LINE_HEIGHT - BORDER;
                               	break;

      		case POPUP_LEFT: 
				px = bx - pw - (3 * BORDER) - 1;
                               	py = by - TITLE_LINE_HEIGHT - BORDER;
                              	break ;

      		case POPUP_ABOVE: 
				px = bx - BORDER;
                               	py = by - ph - (2 * TITLE_LINE_HEIGHT) - (3 * BORDER);
                               	break ;

      		case POPUP_BELOW: 
				px = bx - BORDER;
                               	py = by + bh + BORDER;
                               	break ;

      		default : 
				px = bx + (bw - pw) / 2;
                               	py = by + (bh - ph) / 2;
    	}
	ForcePopupOnScreen(popup, &px, &py) ; 
	return;
}




