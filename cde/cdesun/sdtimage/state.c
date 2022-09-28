#ifndef lint
static char sccsid[] = "@(#)state.c 1.12 96/05/29";
#endif

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

#include <math.h>
#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <Dt/SpinBox.h>

#include "display.h"
#include "image.h"
#include "imagetool.h"
#include "state.h"
#include "ui_imagetool.h"

extern int       scroll_height;
extern int       scroll_width;
extern XColor    white_color;

StateInfo	*current_state;

void	identity_xform(), rotate_xform(), zoom_xform(), swap_matrix(),
	hflip_xform(), vflip_xform(), matrix_mul(), matrix_copy();


StateInfo *
init_state (type)
    FileTypes  type;
{
    StateInfo	*tmp_state = (StateInfo *) calloc (1, sizeof (StateInfo));

    if (type == POSTSCRIPT || type == EPSF)
      tmp_state->zoom = ((float) prog->def_ps_zoom) / 100.0;
    else
      tmp_state->zoom = 1.0;
    tmp_state->rotate_amt = 0;
    tmp_state->angle = 0;
    tmp_state->frontside = TRUE;
    tmp_state->hflip = OFF;
    tmp_state->vflip = OFF;
    (tmp_state->undo).op = NO_UNDO;
    (tmp_state->undo).zoom_amt = 0.0;
    tmp_state->pan = OFF;
    tmp_state->select = OFF;
    tmp_state->image_selected = FALSE;
    tmp_state->set_roi = FALSE;
    (tmp_state->sel_rect).x = 0;
    (tmp_state->sel_rect).y = 0;
    (tmp_state->sel_rect).width = 0;
    (tmp_state->sel_rect).height = 0;
    (tmp_state->save_rect).top_x = 0;
    (tmp_state->save_rect).top_y = 0;
    (tmp_state->save_rect).new_x = 0;
    (tmp_state->save_rect).new_y = 0;
    (tmp_state->save_rect).xrect.y = 0;
    (tmp_state->save_rect).xrect.x = 0;
    (tmp_state->save_rect).xrect.width = 0;
    (tmp_state->save_rect).xrect.height = 0;
    tmp_state->old_x = 0;
    tmp_state->old_y = 0;
    tmp_state->currentx = 0;
    tmp_state->currenty = 0;
    XtVaSetValues (base->vscroll, XmNvalue, 0, NULL);
    XtVaSetValues (base->hscroll, XmNvalue, 0, NULL);
    tmp_state->current_page = 1;
    tmp_state->next_page = 0;
    tmp_state->reversed = FALSE;
    tmp_state->using_dsc = FALSE;
    tmp_state->timeout_hit = FALSE;
    identity_xform(tmp_state->xform);
    identity_xform(tmp_state->old_xform);

/*
 * Keep same state as what's selected
 * on the palette.
 */
    if (palette) {
      unsigned char shadow;

      XtVaGetValues (palette->select_button, XmNshadowType, &shadow, NULL);
      if (shadow == XmSHADOW_ETCHED_IN || shadow == XmSHADOW_IN)
	tmp_state->select = ON;

      XtVaGetValues (palette->pan_button, XmNshadowType, &shadow, NULL);
      if (shadow == XmSHADOW_ETCHED_IN || shadow == XmSHADOW_IN)
        tmp_state->pan = ON;
    }
    
    return (tmp_state);
}

void
revert_state ()
{
    if (current_display == ps_display)
      current_state->zoom = ((float) prog->def_ps_zoom) / 100.0;
    else
      current_state->zoom = 1.0;

    if(current_image->orient == LANDSCAPE &&
       (current_image->type_info)->type == POSTSCRIPT)
      {
	current_state->rotate_amt = -90;
	current_state->angle = 90;
      }
    else
      {
	current_state->rotate_amt = 0;
	current_state->angle = 0;
      }

    current_state->frontside = TRUE;
    current_state->hflip = OFF;
    current_state->vflip = OFF;
    (current_state->undo).op = NO_UNDO;
    (current_state->undo).zoom_amt = 0.0;
    current_state->image_selected = FALSE;
    (current_state->save_rect).top_x = 0;
    (current_state->save_rect).top_y = 0;
    (current_state->save_rect).new_x = 0;
    (current_state->save_rect).new_x = 0;
    (current_state->save_rect).xrect.x = 0;
    (current_state->save_rect).xrect.y = 0;
    (current_state->save_rect).xrect.width = 0;
    (current_state->save_rect).xrect.height = 0;
    current_state->old_x = 0;
    current_state->old_y = 0;
    current_state->currentx = 0;
    current_state->currenty = 0;
/*
    XtVaSetValues (base->vscroll, XmNvalue, 0, NULL);
    XtVaSetValues (base->hscroll, XmNvalue, 0, NULL);
*/
    identity_xform(current_state->xform);
    identity_xform(current_state->old_xform);
}

 
static void
rotate_matrix( angle, rmatrix )
    int angle;            /* Rotation angle in degrees */
    float rmatrix[3][3];
{
    rmatrix[0][0] = cos( RADIANS(angle) * -1.0);
    rmatrix[0][1] = sin( RADIANS(angle) * -1.0);
    rmatrix[0][2] = 0.0;
 
    rmatrix[1][0] = -sin( RADIANS(angle) * -1.0 );
    rmatrix[1][1] = cos( RADIANS(angle)  * -1.0);
    rmatrix[1][2] = 0.0;
 
    rmatrix[2][0] = 0.0;
    rmatrix[2][1] = 0.0;
    rmatrix[2][2] = 1.0;
}

static void
fmult( a, b, c )
   float  a[];
   float  b[][3];
   float  c[];
{
   c[0] = ( a[0] * b[0][0] ) + ( a[1] * b[1][0] ) + ( a[2] * b[2][0] );
   c[1] = ( a[0] * b[0][1] ) + ( a[1] * b[1][1] ) + ( a[2] * b[2][1] );   
   c[2] = ( a[0] * b[0][2] ) + ( a[1] * b[1][2] ) + ( a[2] * b[2][2] );    
}

static u_short
y_magnitude( a, b )
    float a[3],b[3];
{
    return( abs(a[1] - b[1]) );
}

static u_short
x_magnitude( a, b )
    float a[3],b[3];
{
    return( abs(a[0] - b[0]) );
}

void
get_dimensions (old_width, old_height, new_width, new_height)
    unsigned int   old_width;
    unsigned int   old_height;
    unsigned int  *new_width;
    unsigned int  *new_height;
{
    unsigned int   width,
		   height;
    float        t[3][3];
    float        corner1[3], new_corner1[3],
                 corner2[3], new_corner2[3],
                 corner3[3], new_corner3[3],
                 corner4[3], new_corner4[3];

    rotate_matrix(current_state->angle, &t[0][0]); 

    corner1[0] = corner1[1] = corner1[2] = 0.0;
    corner2[0] = (float)old_width; corner2[1] = corner2[2] = 0.0;
    corner3[0] = (float)old_width; corner3[1] = (float)old_height; corner3[2] = 0.0;
    corner4[0] = 0.0; corner4[1] = (float)old_height; corner4[2] = 0.0;
    
    fmult( corner1, &t, new_corner1 );	
    fmult( corner2, &t, new_corner2 );	
    fmult( corner3, &t, new_corner3 );	
    fmult( corner4, &t, new_corner4 );	
    
    if ( (current_state->angle == 0) || 
	((current_state->angle > 90 ) && (current_state->angle <= 180)) ||
	((current_state->angle > 270) && (current_state->angle <= 360)) ) {
      width  = x_magnitude (new_corner2, new_corner4);
      height = y_magnitude (new_corner1, new_corner3);
    }
    else if ( ((current_state->angle > 0) && (current_state->angle <= 90)) ||
	      ((current_state->angle > 180) && (current_state->angle <= 270)) ) {
      width =  x_magnitude (new_corner1, new_corner3);
      height = y_magnitude (new_corner2, new_corner4);
    }

    *new_width  = width;
    *new_height = height;
}

void
resize_canvas ()
{
    int  frame_width, frame_height, panel_height;
    int  display_width, display_height;
    int  length;
    static int first_time = TRUE;
    int  size_changed = FALSE;

    Dimension  sw_height, sw_width;
    Dimension  disp_height, disp_width;
    Dimension  canvas_width, canvas_height;
    Dimension  can_height = 0, can_width = 0;
    Dimension  len;

/*
 * Determine the height/width of the viewing area:
 *   disp_height = scrolled_window_height - width_of_scrollbar
 *   disp_width  = scrolled_window_width  - width_of_scrollbar
 */
/*
    XtVaGetValues (base->scrolledw, XmNheight, &sw_height,
		                    XmNwidth, &sw_width, NULL);
    disp_height = sw_height - base->sb_width;
    disp_width = sw_width - base->sb_width;

    if (current_display->display_win) {
*/
    if (current_display->display_win) {
      can_height = (*xil->get_height) (current_display->display_win);
      can_width  = (*xil->get_width) (current_display->display_win);
    }

    XtVaGetValues (current_display->new_canvas, XmNheight, &disp_height,
		                                XmNwidth, &disp_width, NULL);

    /*
     * The canvas width needs to be larger.
     */
    if (current_image->width + current_state->currentx > (int) can_width) {
      if ((int) disp_width <= current_image->width + current_state->currentx) {
	current_state->currentx = (int) can_width - current_image->width;
	if (current_state->currentx < 0)
	  current_state->currentx = 0;
      }
      XtVaSetValues (current_display->new_canvas,
        XmNwidth, (Dimension) (current_image->width + current_state->currentx),
        NULL);
      size_changed = True;
    }

    /*
     * The canvas width needs to be smaller.
     */
    else if (current_image->width + current_state->currentx < (int) can_width) {

      len = (Dimension) (current_image->width + current_state->currentx);
      if (len <= disp_width)
	len = disp_width;

      XtVaSetValues (current_display->new_canvas, XmNwidth, len, NULL);
      size_changed = TRUE;
    }


    /*
     * The canvas height needs to be larger.
     */
    if (current_image->height + current_state->currenty > (int) can_height) {
      if ((int) disp_height <= current_image->height + current_state->currenty) {
        current_state->currenty = (int) can_height - current_image->height;
        if (current_state->currenty < 0)
          current_state->currenty = 0;
      }

      XtVaSetValues (current_display->new_canvas,
		     XmNheight, (Dimension) (current_image->height + 
					     current_state->currenty),
		     NULL);
      size_changed = TRUE;
    }

    /*
     * The canvas height needs to be smaller.
     */
    else if (current_image->height + current_state->currenty < (int) can_height) {
      len = (Dimension) (current_image->height + current_state->currenty);
      if (len <= disp_height)
        len = disp_height;
      XtVaSetValues (current_display->new_canvas, XmNheight, len, NULL);
      size_changed = TRUE;
    }

    if ((size_changed == TRUE || first_time == TRUE) && 
	(current_display == image_display)) {

      if (image_display->display_win) 
        (*xil->destroy) (image_display->display_win);

      image_display->display_win = 
	(*xil->create_from_window) (image_display->state, 
				image_display->xdisplay, image_display->win);
      first_time = FALSE;
    }

/*
 * Set top left back to 0
 */
    XtVaSetValues (base->vscroll, 
		   XmNvalue, 0, 
		   XmNmaximum, (disp_height > current_image->height) ?
		   disp_height : current_image->height,
		   XmNsliderSize, disp_height, 
		   NULL);
    
    XtVaSetValues (base->hscroll, 
		   XmNvalue, 0, 
		   XmNmaximum, (disp_width > current_image->width) ?
		   disp_width : current_image->width,
		   XmNsliderSize, disp_width,
		   NULL);
    current_state->currentx = 0;
    current_state->currenty = 0;
}

void
image_turnover_func (image)
    XilImage     image;
{
    XilImage      tmp_image;
    XilDataType   datatype;
    unsigned int  width, height, nbands;

/*
    if (current_state->frontside)
      return;

    (*xil->get_info) (image, &width, &height, &nbands, &datatype);
    tmp_image = (*xil->create) (current_display->state, width, height, nbands, datatype);
    (*xil->set_value) (tmp_image, current_display->background);
    (*xil->transpose) (image, tmp_image, XIL_FLIP_Y_AXIS); 

    if (current_image->view_image != NULL)
      (*xil->destroy) (current_image->view_image);

    current_image->view_image = tmp_image;
*/
}

void
image_zoom_func ()
{
    float	  tmp_m[6];

    if (current_state->zoom != 1.0) {
	zoom_xform(current_state->zoom_amt, current_state->zoom_amt, tmp_m);
	matrix_copy(current_state->xform, current_state->old_xform);
	matrix_mul(tmp_m, current_state->xform, current_state->xform);
    }
}

void
image_rotate_func()
{
    float	  tmp_m[6];

    rotate_xform(current_state->rotate_amt, tmp_m);
    matrix_copy(current_state->xform, current_state->old_xform);
    matrix_mul(tmp_m, current_state->xform, current_state->xform);
}

void
image_vflip_func ()
{
    float	  tmp_m[6];

    vflip_xform(tmp_m);
    matrix_copy(current_state->xform, current_state->old_xform);
    matrix_mul(tmp_m, current_state->xform, current_state->xform);
}

void
image_hflip_func ()
{
    float	  tmp_m[6];

    hflip_xform(tmp_m);
    matrix_copy(current_state->xform, current_state->old_xform);
    matrix_mul(tmp_m, current_state->xform, current_state->xform);
}

static void
find_bg_color (image)
  ImageInfo *image;
{
  int     i, index = 0;
  XColor  colors[256];
  
  if (image->depth == 24) {
    image_display->background[0] = 255.0;
    image_display->background[1] = 255.0;
    image_display->background[2] = 255.0;
  }
  else if (image->depth == 8) {

    for (i = image->offset; i < 256; i++, index++) {
      colors[index].pixel = i;
      colors[index].red = (image->red[index] << 8) + image->red[index];
      colors[index].green = (image->green[index] << 8) + image->green[index];
      colors[index].blue = (image->blue[index] << 8) + image->blue[index];
    }

/*
 * If saving to 8, get the index.
 * else put closest to 255, 255, 255.
 */
    image_display->background[0] = 
      (float) closest_match (&white_color, colors, 256 - image->offset);
  }
  else if (image->depth == 1) 
    image_display->background[0] = 0.0;

}

void
regenerate_image (image)
    ImageInfo  *image;
{
    XilDataType   datatype;
    unsigned int  width, height, nbands;
    XilImage      save_image;
    int           save_width, save_height;
    float         bg0, bg1, bg2;
    unsigned int  scanline_stride, pixel_stride;
    u_char       *tmp_data;
/*
 * Save a copy of current_image->view_image beause
 * it gets destroyed in the turnover funcs.
 */
    save_width = current_image->width;
    save_height = current_image->height;
    bg0 = current_display->background[0];
    bg1 = current_display->background[1];
    bg2 = current_display->background[2];

    if (current_state->angle != 0) 
      find_bg_color (image);
  
/*
 * Save the view image.
 */
    (*xil->get_info) (current_image->view_image, &width, &height, &nbands, 
		  &datatype);
    save_image = (*xil->create) (image_display->state, width, height, nbands,
			     datatype);
    (*xil->copy) (current_image->view_image, save_image);
    
/*
 * Using current_image->view_image as placeholder here.
 * Assuming current_image->view is not needed
 * when this is called.
 */

    (*current_image->zoom_func) (image->revert_image);
    (*current_image->turnover_func) (current_image->view_image);
    (*current_image->rotate_func) (current_image->view_image);
 
    image->width = current_image->width;
    image->height = current_image->height;
 
/*
 * Copy into image->view_image.
 */
    (*xil->get_info) (current_image->view_image, &width, &height, &nbands, &datatype);
    image->view_image = (*xil->create) (image_display->state, width, height, 
				    nbands, datatype);
    (*xil->copy) (current_image->view_image, image->view_image);
/* 
 * Recalculate bytes_per_line.
 */
    tmp_data = retrieve_data (image->view_image, &pixel_stride,
			      &scanline_stride);
    image->bytes_per_line = scanline_stride;
    reimport_image(image->view_image, FALSE);

/* 
 * Restore.
 */
    current_image->width = save_width;
    current_image->height = save_height;
    current_image->view_image = save_image;
    current_display->background[0] = bg0;
    current_display->background[1] = bg1;
    current_display->background[2] = bg2;

}
 
void
reset_select()
{
   XRectangle  xr;
  
   XtVaSetValues (palette->select_button, XmNshadowType, XmSHADOW_ETCHED_OUT, NULL);
/*
 * Erase the ROI.
 */
    if (current_state && (current_state->image_selected == TRUE)) 
      drawbox ((current_state->save_rect).top_x, 
	       (current_state->save_rect).top_y, 
	       (current_state->save_rect).new_x, 
	       (current_state->save_rect).new_y, 
	       &xr, FALSE );

   current_state->select = OFF;
   reset_cursor();
   if ( current_state->image_selected == TRUE) {
     current_state->image_selected = FALSE;
     set_save_selection( OFF );
   }

   if (saveas && saveas->showing == True) {
     char  *title;
     XtVaGetValues (saveas->shell, XmNtitle, &title, NULL);
     if (strcmp (title, catgets (prog->catd, 1, 342, 
				 "Image Viewer - Save Selection As")) == 0) {
       if (current_display == ps_display)
	 XtVaSetValues (saveas->shell,
			XmNtitle, catgets (prog->catd, 1, 343,
				  "Image Viewer - Save Page As Image"), 
			NULL);
       else
	 XtVaSetValues (saveas->shell,
			XmNtitle, catgets (prog->catd, 1, 344,
				  "Image Viewer - Save As"), 
			NULL);
     }
   }

   SetFooter (base->left_footer, "");
}

void
reset_pan()
{

  XtVaSetValues (palette->pan_button, XmNshadowType, XmSHADOW_ETCHED_OUT, NULL);
  current_state->pan = OFF;
  reset_cursor();
  SetFooter (base->left_footer, "");

}


void
image_revert_func ()
{
    XilDataType   datatype;
    unsigned int  width, height, nbands;

    (*xil->get_info) (current_image->orig_image, &width, &height, 
		  &nbands, &datatype);
    if (current_image->dest_image) {
	(*xil->destroy) (current_image->dest_image);
    }
    current_image->dest_image = (*xil->create_copy) (current_image->orig_image,
	0, 0, width, height, 0, nbands);

    if (current_image->view_image) {
	(*xil->destroy) (current_image->view_image);
    }
    current_image->view_image = (current_image->display_func)(
	current_image->dest_image);

    current_image->width = width;
    current_image->height = height;
    current_image->nbands = nbands;
    current_image->datatype = datatype;
}

void
PaletteCancelCallback (w, client_data, call_data)
     Widget     w;
     XtPointer  client_data;
     XtPointer  call_data;
{
     PaletteObjects  *p = (PaletteObjects *)client_data;

     XtPopdown (XtParent (p->dialog));
}

/*
 * Callback for Revert.
 */
void
RevertCallback (w, client_data, call_data)
    Widget     w;
    XtPointer  client_data;
    XtPointer  call_data;
{
    SetFooter (base->left_footer, "");

    setbusy();

    revert_state ();
    set_undo_option (OFF);
    set_zoom_and_rotate ();

    (*current_image->revert_func)();

    if (current_display == ps_display) 
       fit_frame_to_image (current_image);

    display_new_image();

    setactive();
}


void 
UndoCallback (w, client_data, call_data)
    Widget     w;
    XtPointer  client_data;
    XtPointer  call_data;
{
    int   i;
    float tmp;
    extern void ZoomScaleCallback();
    extern void VflipCallback();
    extern void HflipCallback();

    if (current_display == ps_display) {
	switch ((current_state->undo).op) {
	   case HFLIP:    HflipCallback (NULL, palette, NULL);
			  break;
	   case VFLIP:    VflipCallback (NULL, palette, NULL);
			  break;
	   case ROTATE_R: 
	   case ROTATE_L: setbusy ();

			  if ((current_state->undo).op == ROTATE_R) {
			    current_state->angle -= current_state->rotate_amt;
			    if (current_state->angle >= 360)
			      current_state->angle -= 360;
			    (current_state->undo).op = ROTATE_L;
			  }
			  else {
			    current_state->angle += current_state->rotate_amt;
			    if (current_state->angle < 0)
			      current_state->angle += 360;
			    (current_state->undo).op = ROTATE_R;
			  }

			  (*current_image->zoom_func) (current_image->orig_image);
			  (*current_image->turnover_func) 
					    (current_image->view_image);
			  (*current_image->rotate_func) (current_image->view_image);
			  display_new_image();
			  setactive ();
			  break;
	   case ZOOM:     setbusy ();
			  tmp = (current_state->undo).zoom_amt;
			  (current_state->undo).zoom_amt = current_state->zoom;
			  current_state->zoom = tmp;

			  (*current_image->zoom_func) (current_image->orig_image);
			  (*current_image->turnover_func) 
					    (current_image->view_image);
			  (*current_image->rotate_func) (current_image->view_image);
	     
	                  /*
			   * Setting the slider will update the spin box.
			   */
	                  XtVaSetValues (palette->zoom_scale, XmNvalue,
				    (int) (current_state->zoom * 100.0), NULL);
			  display_new_image();
			  setactive ();
	   }
	} else {
	    swap_matrix(current_state->xform, current_state->old_xform);
	    display_new_image();
	}

}

void
PanCallback (w, client_data, call_data)
    Widget     w;
    XtPointer  client_data;
    XtPointer  call_data;
{
    unsigned char shadow;

    SetFooter (base->left_footer, "");

    reset_select();
/*
 * If button is selected, unselect it.
 */
    XtVaGetValues (palette->pan_button, XmNshadowType, &shadow, NULL);
    if (shadow == XmSHADOW_ETCHED_IN || shadow == XmSHADOW_IN) {
      reset_pan ();
    }
    else {
/*
 * Button is unselected, so select it.
 */
      XtVaSetValues (palette->pan_button, XmNshadowType, XmSHADOW_ETCHED_IN, NULL);
/*
 * Selected pan so display message in frame.
 */
      SetFooter (base->left_footer, catgets (prog->catd, 1, 345,
	         "Drag SELECT in View window to pan image."));

/*
 * Change the cursor to pan.
 */
      XDefineCursor (current_display->xdisplay,
		     XtWindow (current_display->new_canvas),
		     cursor->pan_cursor);
    
      current_state->pan = ON;
    }

}

void
RotateScaleCallback (w, client_data, cbs)
     Widget                 w;
     XtPointer              client_data;
     XmScaleCallbackStruct *cbs;
{
/*
 * Change only if necessary or you'll
 * get too many XmCR_VALUE_CHANGED events.
 */
     if (cbs) {
       int  old_value;

       XtVaGetValues (palette->rotate_value, DtNposition, &old_value, NULL);
       if (cbs->value != old_value)
	 XtVaSetValues (palette->rotate_value, DtNposition, cbs->value, NULL);
     }
}

void
ZoomScaleCallback (w, client_data, cbs)
     Widget                 w;
     XtPointer              client_data;
     XmScaleCallbackStruct *cbs;
{
     if (cbs) {
       int  old_value;

       XtVaGetValues (palette->zoom_value, DtNposition, &old_value, NULL);
       if (cbs->value != old_value)
         XtVaSetValues (palette->zoom_value, DtNposition, cbs->value, NULL);
     }
}


void
identity_xform(m)
    float *m;
{
    m[0] = m[3] = 1.0;
    m[1] = m[2] = m[4] = m[5] = 0.0;
}

void
rotate_xform(angle, m)
    int angle;		/* rotation angle in degrees */
    float *m;
{
    double r = RADIANS(angle);

    m[0] = m[3] = (float)cos(r);
    m[1] = (float)-sin(r);
    m[2] = -m[1];
    m[4] = m[5] = 0.0;
}

void
zoom_xform(xfac, yfac, m)
    float xfac, yfac;
    float *m;
{
    m[0] = xfac;
    m[3] = yfac;
    m[1] = m[2] = m[4] = m[5] = 0.0;
}

void
hflip_xform(m)
    float *m;
{
    m[0] = 1.0;
    m[3] = -1.0;
    m[1] = m[2] = m[4] = m[5] = 0.0;
}

void
vflip_xform(m)
    float *m;
{
    m[0] = -1.0;
    m[3] = 1.0;
    m[1] = m[2] = m[4] = m[5] = 0.0;
}

void
matrix_mul(m1, m2, m3)
    float *m1, *m2, *m3;	/* 3x2 transforms */
{
    float m[6];

    m[0] = m1[0] * m2[0] + m1[2] * m2[1];
    m[1] = m1[1] * m2[0] + m1[3] * m2[1];
    m[2] = m1[0] * m2[2] + m1[2] * m2[3];
    m[3] = m1[1] * m2[2] + m1[3] * m2[3];
    m[4] = m1[0] * m2[4] + m1[2] * m2[5] + m1[4];
    m[5] = m1[1] * m2[4] + m1[3] * m2[5] + m1[5];

    memcpy(m3, m, sizeof(float)*6);
}

void
matrix_copy(src_m, dst_m)
    float *src_m, *dst_m;
{
    memcpy(dst_m, src_m, sizeof(float)*6);
}

void
swap_matrix(m1, m2)
    float *m1, *m2;
{
    float tmp[6];

    matrix_copy(m1, tmp);
    matrix_copy(m2, m1);
    matrix_copy(tmp, m2);
}

dump_matrix(m)
    float *m;
{
    printf("%fX + %fY + %f\n", m[0], m[2], m[4]);
    printf("%fX + %fY + %f\n", m[1], m[3], m[5]);
    printf("\n");
}

/*
 * Callback from rotate menu.
 */
void
RotateCallback (w, client_data, cbs)
    Widget                       w;
    XtPointer                    client_data;
    XmDrawnButtonCallbackStruct *cbs;
{
    PaletteObjects *p = (PaletteObjects *) client_data;
    int             text_value, curr_value, rotate_value, value;
    int             min, max;
    char            *text, new_text[32];

/*
 * User selected button, push button in.
 */
    if (cbs && cbs->reason == XmCR_ARM) {
      XtVaSetValues (w, XmNshadowType, XmSHADOW_ETCHED_IN, NULL);
      return;
    }
/*
 * Set value depending on which widget was
 * selected:  value = 0 -> rotate right    
 *            value = 1 -> rotate left
 */
    if (w == p->rotate_r_button)
      value = 0;
    else
      value = 1;

/*
 * Verify a valid rotate value in case they
 * didn't hit return on the text line.
 */
    XtVaGetValues (p->rotate_value, DtNposition, &curr_value,
		                    DtNminimumValue, &min,
	                            DtNmaximumValue, &max, NULL);

    text = XmTextGetString (p->rotate_text);
    text_value = atoi (text);

    if (text_value < min) 
      rotate_value = min;
    else if (text_value > max) 
      rotate_value = max;
    else
      rotate_value = text_value;

    if (rotate_value != curr_value)
      XtVaSetValues (p->rotate_value, DtNposition, rotate_value, NULL);
/*
 * Here because of a spin box bug.
 * Spin box doesn't change text if the value of spin box is 
 * already what DtNposition is being set to.
 */
    if (text_value != rotate_value) {
      sprintf (new_text, "%-d", rotate_value);
      XmTextSetString (p->rotate_text, new_text);
    }		    
/*
 * Return if 0 degrees.
    if (rotate_value == 0) {
	xv_set(p->rotate, PANEL_TOGGLE_VALUE, value, FALSE, NULL);
	return;
    } else {
	xv_set (p->palette, FRAME_LEFT_FOOTER, "", NULL);
	SetFooter (base->left_footer, "");
    }
 */

/*
 * Reset Select/Pan.
 * This has to be before setbusy() or resetting
 * the cursors won't work.
 */
    reset_select();
    reset_pan();

    setbusy();
    set_undo_option (ON);

/*
 * Compute the angle of rotation.
 * value == 0  Rotate Right
 * value == 1  Rotate Left
 */
    if (value == 0) {   
	(current_state->undo).op = ROTATE_L;
	current_state->angle += rotate_value;
	current_state->rotate_amt = -rotate_value;
	if (current_state->angle >= 360)
	    current_state->angle -= 360;
    } else if (value == 1) {
	(current_state->undo).op = ROTATE_R;
	current_state->angle -= rotate_value;
	current_state->rotate_amt = rotate_value;
	if (current_state->angle < 0)
	    current_state->angle += 360;
    }

    if (current_display == ps_display) {
	(*current_image->zoom_func) (current_image->revert_image);
	(*current_image->turnover_func) (current_image->view_image);
	(*current_image->rotate_func) (current_image->view_image);

	resize_canvas ();

	if (rotate_value == 90)
	   fit_frame_to_image (current_image);
    } else {
	(*current_image->rotate_func) (current_image->view_image);
    }

    display_new_image();
/*
 * Force button out
 */
    XtVaSetValues (w, XmNshadowType, XmSHADOW_ETCHED_OUT, NULL);
    setactive();

}

/*
 * Notify callback function for changing rotate_value text.
 */
/*ARGSUSED*/
void
RotateValueCallback (w, client_data, cbs)
     Widget                      w;
     XtPointer                   client_data;
     XmTextVerifyCallbackStruct *cbs;
{
     int  rotate_value, curr_value;
     int  min, max;

     if (cbs == NULL)
       return;
     else if (cbs->reason == XmCR_MODIFYING_TEXT_VALUE) {
/*
 * Return if backspace hit.
 */
       if (cbs->text->ptr == NULL)
	 return;
/*
 * Do not allow non-digits.
 */
       if (!isdigit (cbs->text->ptr[0])) {
	 cbs->doit = False;
	 return;
       }

     }
     else if (cbs->reason == XmCR_VALUE_CHANGED) {
       XtVaGetValues (palette->rotate_value, DtNposition, &rotate_value, NULL);
       /*
	* Change value on scale only if necessary or we'll
	* get too many ValueChanged events.
	*/
       XtVaGetValues (palette->rotate_scale, XmNvalue, &curr_value, NULL);
       if (rotate_value != curr_value)
         XtVaSetValues (palette->rotate_scale, XmNvalue, rotate_value, NULL);
     }

/*
 * User hit return in text field of spin box.
 * Default to rotate right.
 */
     else if (cbs->reason == XmCR_ACTIVATE) {
       RotateCallback (palette->rotate_r_button, palette, NULL);
     }
}

/*
 * Callback for vertical flip.
 */
void
VflipCallback (w, client_data, cbs)
    Widget                       w;
    XtPointer                    client_data;
    XmDrawnButtonCallbackStruct *cbs;
{
    PaletteObjects *p = (PaletteObjects *) client_data;
  
/*
 * User selected button, push button in.
 */
    if (cbs && cbs->reason == XmCR_ARM) {
      XtVaSetValues (p->vflip_button, XmNshadowType, XmSHADOW_ETCHED_IN, NULL);
      return;
    }

/*
 * Reset Select/Pan.
 * This has to be before setbusy() or resetting
 * the cursors won't work.
 */
    reset_select();
    reset_pan();

    setbusy();
    set_undo_option(ON);

    (current_state->undo).op = VFLIP;

    if (current_state->vflip == ON) {
	current_state->vflip = OFF;
    } else {
	current_state->vflip = ON;
    }

    if (current_display == ps_display) {
	(*current_image->zoom_func) (current_image->revert_image);
	(*current_image->vflip_func) (current_image->view_image);
	(*current_image->rotate_func) (current_image->view_image);
    } else {
	(*current_image->vflip_func) (current_image->view_image);
    }
/*
 * Force button out
 */
    XtVaSetValues (p->vflip_button, XmNshadowType, XmSHADOW_ETCHED_OUT, NULL);
    display_new_image();

    setactive();
}

/*
 * Callback for horizontal flip.
 */
void
HflipCallback (w, client_data, cbs)
    Widget                       w;
    XtPointer                    client_data;
    XmDrawnButtonCallbackStruct *cbs;
{ 
    PaletteObjects *p = (PaletteObjects *) client_data;
  
/*
 * User selected button, push button in.
 */
    if (cbs && cbs->reason == XmCR_ARM) {
      XtVaSetValues (p->hflip_button, XmNshadowType, XmSHADOW_ETCHED_IN, NULL);
      return;
    }

    SetFooter (base->left_footer, "");
    setbusy();
    set_undo_option(ON);

    (current_state->undo).op = HFLIP;

    if (current_state->hflip == ON) {
	current_state->hflip = OFF;
    } else {
	current_state->hflip = ON;
    }

    if (current_display == ps_display) {
	(*current_image->zoom_func) (current_image->revert_image);
	(*current_image->hflip_func) (current_image->view_image);
	(*current_image->rotate_func) (current_image->view_image);
    } else {
	(*current_image->hflip_func) (current_image->view_image);
    }

    display_new_image();
/*
 * Set button out.
 */
    XtVaSetValues (p->hflip_button, XmNshadowType, XmSHADOW_ETCHED_OUT, NULL);
    setactive();
}

/*
 * Callback from zoom menu.
 */
void
ZoomCallback (w, client_data, cbs)
    Widget                       w;
    XtPointer                    client_data;
    XmDrawnButtonCallbackStruct *cbs;

{
   int             text_value, curr_value, zoom_value;
   int             min, max;
   char           *text, new_text[32];
   PaletteObjects *p = (PaletteObjects *)client_data;

/*
 * User selected zoom, push button in.
 */
   if (cbs && cbs->reason == XmCR_ARM) {
     XtVaSetValues (p->zoom_button, XmNshadowType, XmSHADOW_ETCHED_IN, NULL);
     return;
   }
/*
 * Verify a valid zoom value in case user
 * didn't hit return on the text line.
 */
   SetFooter (base->left_footer, "");

   XtVaGetValues (p->zoom_value, DtNposition, &curr_value,
		                 DtNminimumValue, &min,
		                 DtNmaximumValue, &max, NULL);
   text = XmTextGetString (p->zoom_text);
   text_value = atoi (text);

   if (text_value < min) 
     zoom_value = min;
   else if (text_value > max) 
     zoom_value = max;
   else
     zoom_value = text_value;

   if (zoom_value != curr_value)
     XtVaSetValues (p->zoom_value, DtNposition, zoom_value, NULL);
/*
 * Here because of a spin box bug.
 * Spin box doesn't change text if the value of spin box is 
 * already what DtNposition is being set to.
 */
    if (text_value != zoom_value) {
      sprintf (new_text, "%-d", zoom_value);
      XmTextSetString (p->zoom_text, new_text);
    }		    

/*
 * Reset Select/Pan.
 * This has to be before setbusy() or resetting
 * the cursors won't work.
 */
    reset_select();
    reset_pan();
        
    setbusy();
    set_undo_option(ON);

    (current_state->undo).op = ZOOM;
    (current_state->undo).zoom_amt = current_state->zoom;
    current_state->zoom_amt = zoom_value * 0.01;
    current_state->zoom *= current_state->zoom_amt;

    (*current_image->zoom_func)(current_image->revert_image);

    if (current_display == ps_display) {
	(*current_image->turnover_func)(current_image->view_image);
	(*current_image->rotate_func)(current_image->view_image);
	resize_canvas ();
    }

    display_new_image();
/*
 * Force button to be out.
 */
    XtVaSetValues (p->zoom_button, XmNshadowType, XmSHADOW_ETCHED_OUT, NULL);
    setactive();
}

/*
 * Notify callback function for changing rotate_value text.
 */
/*ARGSUSED*/
void
ZoomValueCallback (w, client_data, cbs)
     Widget                      w;
     XtPointer                   client_data;
     XmTextVerifyCallbackStruct *cbs;
{
     int  zoom_value, curr_value;
     int  min, max;

     if (cbs == NULL)
       return;
     else if (cbs->reason == XmCR_MODIFYING_TEXT_VALUE) {
/*
 * Return if backspace hit.
 */
       if (cbs->text->ptr == NULL)
	 return;
/*
 * Do not allow non-digits.
 */
       if (!isdigit (cbs->text->ptr[0])) {
	 cbs->doit = False;
	 return;
       }

     }
     else if (cbs->reason == XmCR_VALUE_CHANGED) {
       XtVaGetValues (palette->zoom_value, DtNposition, &zoom_value, NULL);
       /*
	* Change value on scale only if necessary or we'll
	* get too many ValueChanged events.
	*/
       XtVaGetValues (palette->zoom_scale, XmNvalue, &curr_value, NULL);
       if (zoom_value != curr_value)
	 XtVaSetValues (palette->zoom_scale, XmNvalue, zoom_value, NULL);
     }
     else if (cbs->reason == XmCR_ACTIVATE) {
       ZoomCallback (NULL, palette, NULL);
     }

}
