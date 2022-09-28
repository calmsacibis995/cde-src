#ifndef lint
static char sccsid[] = "@(#)select.c 1.9 97/04/24";
#endif

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

#include "display.h"
#include "help.h"
#include "image.h"
#include "imagetool.h"
#include "misc.h"
#include "state.h"
#include "ui_imagetool.h"

void
make_rectangle( x, y, x1, y1, r )
    int          x, y, x1, y1;
    XRectangle  *r;
{
    if ( x > x1 )
       r->x = x1;
    else
       r->x = x;

    if ( y1 > y )
       r->y = y;
    else
       r->y = y1;

    r->width = abs( x - x1 );
    r->height = abs( y - y1 );
          
}

void
drawbox( top_left_x, top_left_y, new_x, new_y, r, erase )
    int         top_left_x, top_left_y, new_x, new_y;
    XRectangle *r;
    int         erase;
{

/*
 * Erase old box.
 */
    if ( erase ) 
      XDrawRectangle( current_display->xdisplay, current_display->win, 
		      current_display->select_gc, r->x, r->y,
		      r->width, r->height );
    
/*
 * Make new Rect and draw it.
 */ 
    make_rectangle( top_left_x, top_left_y, new_x, new_y, r );
    XDrawRectangle( current_display->xdisplay, current_display->win, 
		    current_display->select_gc, r->x, r->y, 
		    r->width, r->height );
}

void
reset_cursor()
{
/*
 * Resetting cursor back to None will use parent's cursor.
 */
     XDefineCursor (current_display->xdisplay,
		    XtWindow (current_display->new_canvas),
		    None);
}

void
SelectCallback (w, client_data, call_data)
    Widget     w;
    XtPointer  client_data;
    XtPointer  call_data;
{
    unsigned char  shadow;

    reset_pan();

/*
 * If deselecting select, reset it.
 */
    XtVaGetValues (palette->select_button, XmNshadowType, &shadow, NULL);
    if (shadow == XmSHADOW_ETCHED_IN || shadow == XmSHADOW_IN) {
      reset_select();
    }

    else {
/*
 * Button is unselected, so select it.
 */
      XtVaSetValues (palette->select_button, 
		     XmNshadowType, XmSHADOW_ETCHED_IN, NULL);
/*
 * Selected select so display message in frame and set cursor.
 */
      SetFooter (base->left_footer, catgets (prog->catd, 1, 337,
	         "Drag SELECT to select a region within the image."));
      XDefineCursor (current_display->xdisplay, 
		     XtWindow (current_display->new_canvas),
		     None);

      current_state->select = ON;
    }

}

XilImage 
crop_region (image, xilimage)
ImageInfo	*image;
XilImage	xilimage;
{
  XilImage    tmp_image = NULL;
  XRectangle  xrect;

/*
 * Make a copy of current rect in case we need to go back.
 */
  xrect.x = current_state->sel_rect.x;
  xrect.y = current_state->sel_rect.y;
  xrect.width = current_state->sel_rect.width;
  xrect.height = current_state->sel_rect.height;

  SetFooter (base->left_footer, "");

/*
 * Check for crop region width, height > 0.
 */

  if (xrect.width <= 0 || xrect.height <= 0) {
    AlertPrompt (base->top_level,
		 DIALOG_TYPE,  XmDIALOG_ERROR,
                 DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		 DIALOG_TITLE, catgets (prog->catd, 1, 431, 
					"Image Viewer - Palette"),
		 DIALOG_TEXT, catgets (prog->catd, 1, 338,
				 "Selected region too small.\nPlease select a larger area."),
		 BUTTON_IDENT, 0, catgets (prog->catd, 1, 339, "Continue"),
		 HELP_IDENT, HELP_PALETTE_ITEM,
		 NULL);
    return ((XilImage)NULL);
  }

/*
 * Check if entire rect is on image.
 */

  if (xrect.x > current_state->currentx + image->width || 
      xrect.y > current_state->currenty + image->height ||
      (int) (xrect.x + xrect.width) < current_state->currentx || 
      (int) (xrect.y + xrect.height) < current_state->currenty) {
    AlertPrompt (base->top_level,
		 DIALOG_TYPE,  XmDIALOG_ERROR,
                 DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		 DIALOG_TITLE, catgets (prog->catd, 1, 432, "Image Viewer - Palette"),
		 DIALOG_TEXT, catgets (prog->catd, 1, 340,
				       "Select a region within the image."),
		 BUTTON_IDENT, 0, catgets (prog->catd, 1, 341, "Continue"),
		 HELP_IDENT, HELP_PALETTE_ITEM,
		 NULL);
    return ((XilImage)NULL);
  }


/*
 * Adjust the region's top_y left_x if region
 * is larger thant he image.
 */
  if (xrect.x < current_state->currentx) {
    xrect.width = xrect.width - (current_state->currentx - xrect.x);
    xrect.x = current_state->currentx;
  }
  if (xrect.y < current_state->currenty) {
    xrect.height = xrect.height - (current_state->currenty - xrect.y);
    xrect.y = current_state->currenty;
  }

/*
 * Adjust top left of rect relative to currentx and currenty.
 */
  xrect.y = xrect.y - current_state->currenty;
  xrect.x = xrect.x - current_state->currentx;
  
/*
 * Adjust the region's width/height if region
 * is larger than the image.
 */
  if ((xrect.x + xrect.width) > image->width)
    xrect.width = image->width - xrect.x;
  if ((xrect.y + xrect.height) > image->height)
    xrect.height = image->height - xrect.y;

  tmp_image = (*xil->create_child) (xilimage, 
        xrect.x, xrect.y, xrect.width, xrect.height,
	0, (*xil->get_nbands) (xilimage)); 

  return(tmp_image);
}
