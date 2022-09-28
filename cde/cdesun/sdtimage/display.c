#ifndef lint
static char sccsid[] = "@(#)display.c 1.28 96/06/05";
#endif

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

/*
 * display.c - Functions dealing with the display of the images.
 */

#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Dt/SpinBox.h>

#include "display.h"
#include "image.h"
#include "imagetool.h"
#include "misc.h"
#include "props.h"
#include "ui_imagetool.h"
#include "state.h"

extern int  scroll_height;
extern int  scroll_width;
extern int  rendering_pages;
extern int  started_thumbnails;

DisplayInfo	*current_display;
DisplayInfo	*ps_display;
DisplayInfo	*image_display;

void
set_current_display ()
{
    if (((current_image->type_info)->type == POSTSCRIPT) ||
	((current_image->type_info)->type == EPSF))
       current_display = ps_display;
    else 
       current_display = image_display;
}

void
set_roi_on_display (roi)
    XilRoi roi;
{
    int	dx = abs (current_state->currentx - current_state->old_x);
    int	dy = abs (current_state->currenty - current_state->old_y);

    if ((dx == 0) && (dy == 0))
	return;

    if (current_state->old_x <= current_state->currentx) {
	if (dx != 0) {
	    (*xil->roi_add_rect) (roi, current_state->old_x, 
			current_state->old_y, dx, current_image->height);
	}

	if (dy != 0) {
	    if (current_state->old_y < current_state->currenty) {
		(*xil->roi_add_rect) (roi, current_state->currentx, 
			current_state->old_y, 
			abs (current_image->width - dx), dy);
	    } else {
		(*xil->roi_add_rect) (roi, current_state->currentx, 
			current_state->currenty+current_image->height,
			abs (current_image->width - dx), dy);
	    }
	}
    } else {
	if (dx != 0) {
	    (*xil->roi_add_rect) (roi, 
	                current_state->currentx + current_image->width,
 		        current_state->old_y, dx, current_image->height);
	}

	if (dy != 0) {
	    if (current_state->old_y < current_state->currenty) {
		(*xil->roi_add_rect) (roi, current_state->old_x, 
			current_state->old_y, 
			abs (current_image->width - dx), dy);
	    } else {
	      (*xil->roi_add_rect) (roi, current_state->old_x, 
			current_state->currenty+current_image->height,
			abs (current_image->width - dx), dy);
	    } 
	} 
    } 

    (*xil->set_roi) (current_display->display_win, roi);
}

void
update_page ()
{
     int  dx = abs (current_state->currentx - current_state->old_x);
     int  dy = abs (current_state->currenty - current_state->old_y);

/*
printf("NOTE: update_page\n");
printf("%d %d %d %d %d %d\n", current_display->pix_width,
current_display->pix_height, current_state->currentx, current_state->currenty,
current_state->old_x, current_state->old_y);
*/
    /*
     * Copy the ps file onto the canvas.
     */
    if (current_display->depth == 1 || prog->ps_mono == True) {
	XCopyPlane (current_display->xdisplay, current_display->pixmap1, 
		current_display->win, current_display->win_gc, 0, 0, 
		current_display->pix_width, current_display->pix_height, 
		current_state->currentx, current_state->currenty, 1L);
    } else {
	XCopyArea (current_display->xdisplay, current_display->pixmap1, 
		current_display->win, current_display->win_gc, 0, 0, 
		current_display->pix_width, current_display->pix_height, 
		current_state->currentx, current_state->currenty);
    }

    if (current_state->old_x <= current_state->currentx) {
	XFillRectangle (current_display->xdisplay, current_display->win, 
		current_display->fill_gc, current_state->old_x, 
		current_state->old_y, dx, current_display->pix_height);

	if (current_state->old_y < current_state->currenty) {
	    XFillRectangle (current_display->xdisplay, current_display->win, 
		current_display->fill_gc, current_state->currentx, 
		current_state->old_y, current_display->pix_width - dx, dy);
	} else {
	    XFillRectangle (current_display->xdisplay, current_display->win, 
		current_display->fill_gc, current_state->currentx, 
		current_state->currenty + current_display->pix_height,
		current_display->pix_width - dx, dy);
	}
    } else {
	XFillRectangle (current_display->xdisplay, current_display->win, 
		current_display->fill_gc, 
		current_state->currentx + current_display->pix_width, 
		current_state->old_y, dx, current_display->pix_height);

	if (current_state->old_y <= current_state->currenty) {
	    XFillRectangle (current_display->xdisplay, current_display->win, 
		current_display->fill_gc, current_state->old_x, 
		current_state->old_y, current_display->pix_width - dx, dy);
	} else {
	    XFillRectangle (current_display->xdisplay, current_display->win, 
		current_display->fill_gc, current_state->old_x, 
		current_state->currenty + current_display->pix_height,
		current_display->pix_width - dx, dy);
	} 
    }
    XFlush (current_display->xdisplay);
}
     
void
update_image ()
{
    XilRoi  roi = NULL;

    if (current_display == image_display) {

/*
 * Clear the backgound only when necessary
 */
	if ((((*xil->get_width) (current_display->display_win) >
	     (*xil->get_width) (current_image->view_image)) ||
	    ((*xil->get_height) (current_display->display_win) >
	     (*xil->get_height) (current_image->view_image))) &&
	    current_state->scrollbar_update != True) {

/* 
 * Set ROI on display win if panning.
 */
	    if (current_state->set_roi) {
		roi = (*xil->roi_create) (image_display->state);
		set_roi_on_display (roi);
		(*xil->set_value) (current_display->display_win,
		    current_display->background);
/*
 * Destroy ROI and set back to NULL.
 */
		(*xil->roi_destroy) (roi);
		(*xil->set_roi) (current_display->display_win, NULL);
		current_state->set_roi = FALSE;
	    } else {
		(*xil->set_value) (current_display->display_win,
		    current_display->background);
	    }

	}
	current_state->scrollbar_update = False;
	(*xil->set_origin) (current_image->view_image,
	    (float)(0 - current_state->currentx),
	    (float)(0 - current_state->currenty));

	(*xil->copy) (current_image->view_image, current_display->display_win);
	(*xil->set_origin) (current_image->view_image, 0.0, 0.0);
    }
    else
      update_page ();

}
 
void
CanvasResizeCallback (w, client_data, cbs)
     Widget                        w;
     XtPointer                     client_data;
     XmDrawingAreaCallbackStruct  *cbs;
{
     BaseObjects      *b = (BaseObjects *)client_data;
     Dimension         width, height;

/*
 * Save "extra_height" for calculating the frame size in
 * fit_frame_to_image() later.
 */
     if (b->extra_height == 0) {
       Dimension   height1, height2;
       XtVaGetValues (b->mainw, XmNheight, &height1, NULL);
       XtVaGetValues (b->scrolledw, XmNheight, &height2, NULL);
	 b->extra_height = height1 - height2;
     }

}

void
set_zoom_and_rotate ()
{
    if (palette != NULL) {
      if (current_display == ps_display) 
	XtVaSetValues (palette->zoom_value, DtNposition, prog->def_ps_zoom, NULL);
      else 
	XtVaSetValues (palette->zoom_value, DtNposition, DEFAULT_ZOOM, NULL);

      XtVaSetValues (palette->rotate_value, DtNposition, DEFAULT_ANGLE, NULL);
    }
}


void
CanvasRepaintCallback (w, client_data, cbs)
     Widget                        w;
     XtPointer                     client_data;
     XmDrawingAreaCallbackStruct  *cbs;
{
     XRectangle     xr;
     XilRoi         roi = NULL;
     XEvent         ev;
     XRectangle     xrects[20];
     int            i, nrects = 0;
     static Boolean startup = True;
     static int     tries = 0;
/*
 * Return if not an ExposeEvent
 */
     if (cbs && cbs->reason != XmCR_EXPOSE) {
       return;
     }
     else if (current_display == NULL)
       return;
/*
 * 
 */
     tries++;
     if (startup == True && current_props->show_palette == True) {
       Position x, y;
       XtVaGetValues (base->top_level, XmNx, &x, XmNy, &y, NULL);
       if ((x != 0 || y != 0 || tries >= 2) && current_image != NULL) {
	 PaletteCallback (NULL, NULL, NULL);
	 set_zoom_and_rotate();
	 startup = False;
       }
     }

     /*
      * Set first clip rectangle.
      */
     if (cbs) {
       xrects[nrects].x = cbs->event->xexpose.x;
       xrects[nrects].y = cbs->event->xexpose.y;
       xrects[nrects].width = cbs->event->xexpose.width;
       xrects[nrects].height = cbs->event->xexpose.height;
       nrects++;
     
       /*
	* Compress Expose Events and add them
	* to the clip rectangles.
	*/
       while (XEventsQueued (current_display->xdisplay, QueuedAfterFlush) &&
	      (XPeekEvent (current_display->xdisplay, &ev), 
	       (ev.xexpose.type == Expose), 
	       (ev.xexpose.window == XtWindow (w)))) {
	 XNextEvent (current_display->xdisplay, &ev);
	 xrects[nrects].x = ev.xexpose.x;
	 xrects[nrects].y = ev.xexpose.y;
	 xrects[nrects].width = ev.xexpose.width;
	 xrects[nrects].height = ev.xexpose.height;
	 nrects++;
       }
     }

     if ((current_image == NULL) && (current_display == image_display) &&
	 (current_display->display_win != NULL)) {
       /*
	* Case where no image on canvas and repaint occurs.
	*/
       (*xil->set_value) (current_display->display_win,
		      current_display->background);
     } 
     else if (current_display == ps_display) {
       /*
	* PS repaint. 
	*/
       XSetClipRectangles (current_display->xdisplay, 
			   current_display->fill_gc, 0, 0, xrects, nrects, 
			   Unsorted);
       XFillRectangle (current_display->xdisplay, current_display->win,
		       current_display->fill_gc, 0, 0, 
		       current_display->width, 
		       current_display->height); 
     } 
     else if ((current_display == image_display) &&
	      (current_image != (ImageInfo *) NULL) && nrects > 0) {
       /*
	* Image repaint.
	*/
       roi = (*xil->roi_create) (image_display->state);

       if (current_state && current_state->image_selected) {
	 make_rectangle ((current_state->save_rect).top_x,
			 (current_state->save_rect).top_y,
			 (current_state->save_rect).new_x,
			 (current_state->save_rect).new_y, &xr);
	 (*xil->roi_add_rect) (roi, xr.x - 1, xr.y - 1,
			   xr.width + 2, xr.height + 2);
	 (*xil->set_roi) (current_display->display_win, roi); 
       }

       for (i = 0; i < nrects; i++) 
	 (*xil->roi_add_rect) (roi, xrects[i].x, xrects[i].y,
			   xrects[i].width, xrects[i].height);

       (*xil->set_roi) (current_display->display_win, roi);
       (*xil->roi_destroy) (roi);
     }
     
     if (current_image != (ImageInfo *) NULL)
       update_image ();
/*
 * Repaint box if needed.
 */
     if (current_state && (current_state->image_selected == TRUE))
       drawbox ((current_state->save_rect).top_x,
		(current_state->save_rect).top_y,
		(current_state->save_rect).new_x,
		(current_state->save_rect).new_y,
		&xr, FALSE );      
/*
 * Reset the clip mask to None
 */     
     if (current_display == ps_display)
       XSetClipMask (current_display->xdisplay, current_display->fill_gc,
		     (Pixmap) None);
/*
 * Reset the ROI to None
 */
     XFlush (current_display->xdisplay);
     if (current_display->display_win)
       (*xil->set_roi) (current_display->display_win, NULL);
     
}

#ifdef OLD_TOOLTALK
int
geometry_tt (ttmsg, key, status, w, h, x, y)
Tt_message  	 ttmsg;
void		*key;
int         	 status;
int         	 w, h, x, y;
{
   Rect   rect;

   rect.r_width = w;
   rect.r_height = h;
   rect.r_left = x;
   rect.r_top = y;

   ds_position_popup_rect (&rect, base_window->base_window, DS_POPUP_RIGHT);

   xv_set (base_window->base_window, XV_SHOW, TRUE, NULL);

   return (0);
}
#endif


void
fit_frame_to_image (image)
    ImageInfo  *image;
{
    Dimension  frame_width, frame_height;
    int        min_width, min_height, max_width, max_height;
    Boolean    vsb = False, hsb = False;

/*
 * Save "extra_height" for calculating the frame size in
 * fit_frame_to_image() later.
 */
     if (base->extra_height == 0) {
       Dimension   height1, height2;
       XtVaGetValues (base->mainw, XmNheight, &height1, NULL);
       XtVaGetValues (base->scrolledw, XmNheight, &height2, NULL);
	 base->extra_height = height1 - height2;
     }
/*
 * Determine what the frame size should be base on the image size:
 *   frame_width  = image_width + width_of_scrollbar;
 *   frame_height = image_height + extra_height + width_of_scrollbar;
 */
    frame_width = (Dimension) image->width + base->sb_width;
    frame_height = (Dimension) (image->height + base->extra_height + base->sb_width);

    XtVaGetValues (base->top_level, XmNminWidth, &min_width,
		                    XmNminHeight, &min_height, 
		                    XmNmaxWidth, &max_width,
		                    XmNmaxHeight, &max_height, NULL);
    if ((int) frame_width < min_width) 
      frame_width = (Dimension) min_width;
    else if ((int) frame_width > max_width) {
      frame_width = (Dimension) max_width;
      hsb = True;
    }

    if ((int) frame_height < min_height)
      frame_height = (Dimension) min_height;
    else if ((int) frame_height > max_height) {
      frame_height = (Dimension) max_height;
      vsb = True;
    }

    if ((vsb == False) && (frame_width != max_width) &&
	((int)(frame_width - base->sb_width) > min_width))
      frame_width = frame_width - base->sb_width;

    if ((hsb == False) && (frame_height != max_height) &&
	((int)(frame_height - base->sb_width) > min_height))
      frame_height = frame_height - base->sb_width;

    XtVaSetValues (base->top_level, XmNwidth, frame_width,
		                    XmNheight, frame_height, NULL);
/*
 * If we were started by tooltalk, ask for the geometry...
 * NOTE: Removed 7/12/93 - took too much time, and was useless since
 *  	 the window is so large that it's almost impossible for it
 *	 to appear next to the launching application.
 */
#ifdef LATER
    if ((prog->tt_sender != (char *) NULL) && 
        (xv_get (base_window->base_window, XV_SHOW) == FALSE)) 
       xv_set (base_window->base_window, XV_SHOW, TRUE, 
					 XV_X, 20,
					 XV_Y, 0,
					 NULL);
#endif
}

#ifdef LATER
/*
 * Done proc.. need to tell snapshot we're exiting (if we were
 * started via tooltalk.
 */

Notify_value
imagetool_done_proc( client, status )
Notify_client	client;
Destroy_status	status;
{
    Frame	frame;
    int		n = 1;

/*
 * If we were started by tooltalk, then set the timer and don't
 * really go away.
 */

    if ((status == DESTROY_CHECKING) && (prog->tt_started == TRUE)) {
#ifdef OLD_TOOLTALK
       set_tt_timer ();
#endif
       notify_veto_destroy (client);
       xv_set (base_window->base_window, XV_SHOW, FALSE, NULL);
       while (frame = xv_get (base_window->base_window, FRAME_NTH_SUBFRAME, n++))
          xv_set (frame, FRAME_CMD_PUSHPIN_IN, FALSE,
                         XV_SHOW,              FALSE,
                         NULL);

       return (NOTIFY_DONE);
       }

    return (notify_next_destroy_func (client, status));
}
#endif

/*
 * XErrorHandler
 */
int
xerror_handler( dpy, event )
    Display       *dpy;
    XErrorEvent   *event;
{
    char buffer[128];
    char mesg[128];
    char number[32];
    char *mtype = "XlibMessage";
    FILE *fp = stderr;
 
    XGetErrorText (dpy, event->error_code, buffer, 128 );
    XGetErrorDatabaseText (dpy, mtype,  "XError",  
			   "X Error (intercepted)", mesg, 128);
    ( void )fprintf (fp, "%s:  %s\n  ", mesg, buffer);
    XGetErrorDatabaseText (dpy, mtype,  "MajorCode",  
			   "Request Major code %d", mesg, 128);
    ( void )fprintf (fp, mesg, event->request_code);
    sprintf( number, "%d", event->request_code );
    XGetErrorDatabaseText (dpy, "XRequest", number, "", buffer, 128 );
    ( void )fprintf(fp,  " (%s)" , buffer);
    fputs ("\n", fp);
    XGetErrorDatabaseText (dpy, mtype, "MinorCode",  
			   "Request Minor code", mesg, 128 );
    ( void )fprintf (fp, mesg, event->minor_code );
    fputs ("\n", fp);
    XGetErrorDatabaseText (dpy, mtype,  "ResourceID",  
			   "ResourceID 0x%x", mesg, 128);
    ( void )fprintf (fp, mesg, event->resourceid );
    fputs ("\n", fp);
    XGetErrorDatabaseText (dpy, mtype, "ErrorSerial", 
			   "Error Serial #%d", mesg, 128);
    ( void )fprintf (fp, mesg, event->serial);
    fputs ("\n", fp);
    XGetErrorDatabaseText (dpy, mtype, "CurrentSerial", 
			   "Current Serial #%d", mesg, 128);
    ( void )fprintf (fp, mesg, NextRequest (dpy));
    fputs ("\n", fp);

    if (event->error_code == BadAlloc) {
      AlertPrompt (base->top_level, 
		   DIALOG_TYPE, XmDIALOG_WARNING,
		   DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		   DIALOG_TITLE, catgets (prog->catd, 1, 386, "Image Viewer"),
		   DIALOG_TEXT, catgets (prog->catd, 1, 1,
			       "Unable to allocate sufficient memory.\nImage Viewer cannot continue."),
		   BUTTON_IDENT, 0, catgets (prog->catd, 1, 2, "Close"),
		   NULL);
      exit (1);
    }
/*
 * Force a core dump for stack trace, only if running in verbose mode.
 */
    if (prog->verbose == TRUE)
       abort();
}

/*
 * XILErrorHandler 
 */
Xil_boolean
xil_error_handler (error)
    XilError  error;
{
    if ((*xil->error_get_category) (error) == XIL_ERROR_RESOURCE) {	
      AlertPrompt (base->top_level, 
		   DIALOG_TYPE, XmDIALOG_WARNING,
		   DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		   DIALOG_TITLE, catgets (prog->catd, 1, 387, "Image Viewer"),
		   DIALOG_TEXT, catgets (prog->catd, 1, 3,
			       "Unable to allocate sufficient memory.\nImage Viewer cannot continue."), NULL,
		   BUTTON_IDENT, 0, catgets (prog->catd, 1, 4, "Close"),
		   NULL);
      exit (1);
    } 
    else {
      if ((*xil->error_get_primary) (error)) {
	fprintf (stderr, catgets (prog->catd, 1, 5, "XIL Error: %s\n"), (*xil->error_get_string) (error));
      }
    }

    return (TRUE);
}

void
SetFooter (footer, text)
     Widget   footer;
     char    *text;
{
     XmString  xmstr;

     if (text[0] == '\0') {
       xmstr = XmStringCreateLocalized (" ");
       prog->footer_priority = 0;
     }
     else
       xmstr = XmStringCreateLocalized (text);
     XtVaSetValues (footer, XmNlabelString, xmstr, NULL);
     XmStringFree (xmstr);
}

void
setbusy ()
{

     _DtTurnOnHourGlass (base->top_level);
     XSync (image_display->xdisplay, 0);

}

void
setactive ()
{

     _DtTurnOffHourGlass (base->top_level);
     XSync (image_display->xdisplay, 0);

     if ((prog->frame_mapped == TRUE) && (current_display == ps_display) &&
	 (current_state != (StateInfo *) NULL) && (prog->dps == TRUE) &&
	 (started_thumbnails == FALSE)) {
       started_thumbnails = TRUE;
       do_dps_op (0);
     }

}


void 
set_labels (file)
char	*file;
{
    char	 label [256];
    char	*s = basename (file);

/*
 * Set the Frame title if not specified on command line. 
 */
    if (appresources->title == 0) {
       if (prog->standardin) 
         sprintf (label, "%s - %s", prog->name, catgets (prog->catd, 1, 434,
					"Standard Input"));
       else
         sprintf (label, "%s - %s", prog->name, s);
       XtVaSetValues (base->top_level, XmNtitle, label, NULL);
    }

/*
 * Set the Icon Name if not specified on the command line.
 */
    if (appresources->icon_name == 0) {
      if (strcmp (s, catgets (prog->catd, 1, 388, "(None)")) == 0)
        XtVaSetValues (base->top_level, 
		       XmNiconName, catgets (prog->catd, 1, 6, "Image Viewer"),
		       NULL);
      else
	XtVaSetValues (base->top_level, XmNiconName, s, NULL);
    }

}

void
set_image_print_options ()
{
     Boolean   set;
/*
 * Should really check # pages, because it seems that some file formats
 * (specifically tiff) might have more than one image in them...
 * REMEMBER TO PUT THIS IN LATER!!!

    if (current_image->pages < 1) 
       xv_set (print->page_range, PANEL_INACTIVE, TRUE, NULL);
    else 
       xv_set (print->page_range, PANEL_INACTIVE, FALSE, NULL);
 */

     XtVaSetValues (print->range_label, XmNsensitive, False, NULL);
     XtVaSetValues (print->this_page_toggle, XmNsensitive, False, NULL);
     XtVaSetValues (print->orig_toggle, XmNsensitive, False, NULL);

     XtVaSetValues (print->orientation_label, XmNsensitive, True, NULL);
     XtVaSetValues (print->portrait_toggle, XmNsensitive, True, NULL);
     XtVaSetValues (print->landscape_toggle, XmNsensitive, True, NULL);

     XtVaSetValues (print->image_size_label, XmNsensitive, True, NULL);
     XtVaSetValues (print->image_size_value, XmNsensitive, True, NULL);
     XtVaSetValues (print->image_size_text, XmNsensitive, True, NULL);
     XtVaSetValues (print->percent_label, XmNsensitive, True, NULL);
     XtVaSetValues (print->image_size_slider, XmNsensitive, True, NULL);

     XtVaSetValues (print->position_label, XmNsensitive, True, NULL);
     XtVaSetValues (print->centered_toggle, XmNsensitive, True, NULL);
     XtVaSetValues (print->margin_toggle, XmNsensitive, True, NULL);
     XtVaGetValues (print->margin_toggle, XmNset, &set, NULL);
     if (set == True) {
       XtVaSetValues (print->top_text, XmNsensitive, True, NULL);
       XtVaSetValues (print->left_text, XmNsensitive, True, NULL);
       XtVaSetValues (print->top_inches_label, XmNsensitive, True, NULL);
       XtVaSetValues (print->left_inches_label, XmNsensitive, True, NULL);
       XtVaSetValues (print->top_cm_label, XmNsensitive, True, NULL);
       XtVaSetValues (print->left_cm_label, XmNsensitive, True, NULL);
       XtVaSetValues (XmOptionButtonGadget(print->units_menu), XmNsensitive, True, NULL);
     }

}

void
set_ps_print_options ()
{
     Boolean   set, mset;

     XtVaSetValues (print->range_label, XmNsensitive, True, NULL);
     XtVaSetValues (print->this_page_toggle, XmNsensitive, True, NULL);
     XtVaSetValues (print->orig_toggle, XmNsensitive, True, NULL);
/*
 * If print page as image selected...
 */
     XtVaGetValues (print->this_page_toggle, XmNset, &set, NULL);
     if (set == True) {
       XtVaSetValues (print->image_size_label, XmNsensitive, True, NULL);
       XtVaSetValues (print->image_size_value, XmNsensitive, True, NULL);
       XtVaSetValues (print->image_size_text, XmNsensitive, True, NULL);
       XtVaSetValues (print->percent_label, XmNsensitive, True, NULL);
       XtVaSetValues (print->image_size_slider, XmNsensitive, True, NULL);

       XtVaSetValues (print->orientation_label, XmNsensitive, True, NULL);
       XtVaSetValues (print->portrait_toggle, XmNsensitive, True, NULL);
       XtVaSetValues (print->landscape_toggle, XmNsensitive, True, NULL);

       XtVaSetValues (print->position_label, XmNsensitive, True, NULL);
       XtVaSetValues (print->centered_toggle, XmNsensitive, True, NULL);
       XtVaSetValues (print->margin_toggle, XmNsensitive, True, NULL);
       XtVaGetValues (print->margin_toggle, XmNset, &mset, NULL);
       if (mset == True) {
	 XtVaSetValues (print->top_text, XmNsensitive, True, NULL);
	 XtVaSetValues (print->left_text, XmNsensitive, True, NULL);
	 XtVaSetValues (print->top_inches_label, XmNsensitive, True, NULL);
	 XtVaSetValues (print->left_inches_label, XmNsensitive, True, NULL);
	 XtVaSetValues (print->top_cm_label, XmNsensitive, True, NULL);
	 XtVaSetValues (print->left_cm_label, XmNsensitive, True, NULL);
	 XtVaSetValues (XmOptionButtonGadget(print->units_menu), XmNsensitive, True, NULL);
       }
     }

/*
 * Else print all or specific pages selected, to turn off most items.
 */

     else {
       XtVaSetValues (print->orientation_label, XmNsensitive, False, NULL);
       XtVaSetValues (print->portrait_toggle, XmNsensitive, False, NULL);
       XtVaSetValues (print->landscape_toggle, XmNsensitive, False, NULL);
       
       XtVaSetValues (print->image_size_label, XmNsensitive, False, NULL);
       XtVaSetValues (print->image_size_value, XmNsensitive, False, NULL);
       XtVaSetValues (print->image_size_text, XmNsensitive, False, NULL);
       XtVaSetValues (print->percent_label, XmNsensitive, False, NULL);
       XtVaSetValues (print->image_size_slider, XmNsensitive, False, NULL);

       XtVaSetValues (print->position_label, XmNsensitive, False, NULL);
       XtVaSetValues (print->centered_toggle, XmNsensitive, False, NULL);
       XtVaSetValues (print->margin_toggle, XmNsensitive, False, NULL);
 
       XtVaSetValues (print->top_text, XmNsensitive, False, NULL);
       XtVaSetValues (print->left_text, XmNsensitive, False, NULL);
       XtVaSetValues (print->top_inches_label, XmNsensitive, False, NULL);
       XtVaSetValues (print->left_inches_label, XmNsensitive, False, NULL);
       XtVaSetValues (print->top_cm_label, XmNsensitive, False, NULL);
       XtVaSetValues (print->left_cm_label, XmNsensitive, False, NULL);
       XtVaSetValues (XmOptionButtonGadget(print->units_menu), XmNsensitive, False, NULL);

/*
 * If only one page, and Pages was selected, turn it off, and turn on
 * This Page (as image), and also turn on all of the image ops....
 */

       }

}

void
set_tool_options (old_image, new_image)
ImageInfo	*old_image;
ImageInfo	*new_image;
{
    int 	i;
    FileTypes	old_type;
    extern int  pg_overview_showing;

/* 
 * No matter what the type, if the old type was NO_TYPE (no previously
 * loaded image), then let certain menu options appear.
 */ 

    if (old_image == (ImageInfo *) NULL) {
      XtVaSetValues (base->saveas_item, XmNsensitive, True, NULL);
      XtVaSetValues (base->printprev_item, XmNsensitive, True, NULL);
      XtVaSetValues (base->printone_item, XmNsensitive, True, NULL);
      XtVaSetValues (base->print_item, XmNsensitive, True, NULL);
      XtVaSetValues (base->imageinfo_item, XmNsensitive, True, NULL);
      XtVaSetValues (base->palette_item, XmNsensitive, True, NULL);
#ifdef LATER
      xv_set (base_window->drop_target, PANEL_DROP_FULL, TRUE, NULL);
#endif
      old_type = NO_TYPE;
    }
    else 
      old_type = (old_image->type_info)->type;

/*
 * If PS file just loaded, and old file wasn't PS, then 
 * unshow certain menu items...
 */

    switch ((new_image->type_info)->type) {
    case POSTSCRIPT:
    case EPSF:
      if (print != (PrintObjects *) NULL)
	set_ps_print_options ();
      
      if (new_image->pages > 1) {
	if (prog->dps == TRUE)
	  XtVaSetValues (base->overview_item, XmNsensitive, False, NULL);
	else	
	  XtVaSetValues (base->overview_item, XmNsensitive, True, NULL);
	
	XtVaSetValues (base->pgfwd_button, XmNsensitive, True, NULL);
	XtVaSetValues (base->pgback_button, XmNsensitive, False, NULL);
      }
      else {
	XtVaSetValues (base->overview_item, XmNsensitive, False, NULL);	
	XtVaSetValues (base->pgfwd_button, XmNsensitive, False, NULL);
	XtVaSetValues (base->pgback_button, XmNsensitive, False, NULL);
      }
      
      if (base->canvas != NULL)
	XLowerWindow (XtDisplay (base->canvas), XtWindow (base->canvas));

      if ((old_type != POSTSCRIPT) && (old_type != EPSF)) {
	XtVaSetValues (base->saveas_item, XmNsensitive, False, NULL);
	XtVaSetValues (base->savepage_item, XmNsensitive, True, NULL);
	XtVaSetValues (base->savesel_item, XmNsensitive, False, NULL);
	XtVaSetValues (base->save_item, XmNsensitive, True, NULL);
	XtVaSetValues (base->controls_item, XmNsensitive, True, NULL);
	
      }
      
/*
 * If we were displaying a postscript file, and the page overview pop up
 * is up, then render the small pages. If there is only one page, the
 * take down the page overview pop up.
 */

      else {
	XtVaSetValues (base->savesel_item, XmNsensitive, False, NULL);
	XtVaSetValues (base->savepage_item, XmNsensitive, True, NULL);

/*
 * If dps is true, then we're going to render the thumbnails, so don't do
 * this!
 */
	if ((pg_overview != NULL) && (prog->dps == FALSE) &&
	    (pg_overview->showing == True)) {

	  if (new_image->pages > 1) {
	    pageview_pixmaps_create (new_image->pages);
	    render_small_pages (new_image, FALSE);
	    pg_overview->buttons = (Widget *) PgOverviewPagesCreate (pg_overview,
							 current_image->pages);
	    if (initial_reverse (FALSE) == TRUE)
	      ReversePgOverviewPages (pg_overview, new_image->pages, TRUE);
	  }
	  else 
	    DismissDialog (pg_overview->dialog);

	}

/*
 * If the ps options pop up has been created, and only one page then make 
 * the panel item for reverse page order inactive.
 * Also, if we think the pages are in reverse page order, then set order
 * to Last page first
 */
	if (pg_controls != NULL) {
	  XtVaSetValues (pg_controls->right_toggle, XmNset, True, NULL);
	  XtVaSetValues (pg_controls->left_toggle, XmNset, False, NULL);

/*
 * Note that we set the value here, but then may turn the option off
 * for a while, until we get the total number of pages... 
 * image->pages may be less than zero.
 */

	  if (initial_reverse (FALSE) == TRUE) {
	    XtVaSetValues (pg_controls->lpf, XmNset, True, NULL);
	    XtVaSetValues (pg_controls->fpf, XmNset, False, NULL);
	  }
	  else {
	    XtVaSetValues (pg_controls->fpf, XmNset, True, NULL);
	    XtVaSetValues (pg_controls->lpf, XmNset, False, NULL);
	  }
	  
	  if (new_image->pages > 1) {
	    XtVaSetValues (pg_controls->order_label, XmNsensitive, True, NULL);
	    XtVaSetValues (pg_controls->fpf, XmNsensitive, True, NULL);
	    XtVaSetValues (pg_controls->lpf, XmNsensitive, True, NULL);
	  }
	  else {
	    XtVaSetValues (pg_controls->order_label, XmNsensitive, False, NULL);
	    XtVaSetValues (pg_controls->fpf, XmNsensitive, False, NULL);
	    XtVaSetValues (pg_controls->lpf, XmNsensitive, False, NULL);
	  }
	}
      }

      break;

    default:
      if (print != (PrintObjects *) NULL)
	set_image_print_options ();
      
      if ((old_type == POSTSCRIPT) || (old_type == EPSF) ||
	  (old_type == NO_TYPE)) {
	
	/*
	 * Check to see if pagecounter is running, and if so, stop it.
	 */
	
	if (old_type != NO_TYPE)
	  check_pagecounter ();
	
	XtVaSetValues (base->saveas_item, XmNsensitive, True, NULL);
	XtVaSetValues (base->savesel_item, XmNsensitive, False, NULL);
	XtVaSetValues (base->savepage_item, XmNsensitive, False, NULL);
	XtVaSetValues (base->overview_item, XmNsensitive, False, NULL);
	XtVaSetValues (base->pgfwd_button, XmNsensitive, False, NULL);
	XtVaSetValues (base->pgback_button, XmNsensitive, False, NULL);
	XtVaSetValues (base->controls_item, XmNsensitive, False, NULL);
	
	if (pg_overview != NULL)
	  DismissDialog (pg_overview->dialog);
	
	if (pg_controls != NULL)
	  DismissDialog (pg_controls->dialog);

/*
 * Turn on Save only if we support this format.
 */
	XtVaSetValues (base->save_item, XmNsensitive, True, NULL);
/*
 * Clear out right footer.
 */
	SetFooter (base->right_footer, "");

      }
      else {
	XtVaSetValues (base->save_item, XmNsensitive, True, NULL);
	XtVaSetValues (base->saveas_item, XmNsensitive, True, NULL);
	XtVaSetValues (base->savesel_item, XmNsensitive, False, NULL);
      }
      if (base->ps_canvas != NULL)
	XLowerWindow (XtDisplay (base->ps_canvas), XtWindow (base->ps_canvas));
    }

/*
 * Turn on Save only if this format is supported.
 */
    if (save_format_supported (new_image->type_info) == TRUE)
      XtVaSetValues (base->save_item, XmNsensitive, True, NULL);
    else
      XtVaSetValues (base->save_item, XmNsensitive, False, NULL);
      
/*
 * Undo turned off... since we haven't done anything yet to the
 * newly loaded file.
 */
    XtVaSetValues (base->undo_item, XmNsensitive, False, NULL);

/*
 * Set the labels.. for the frame and icon.
 */
    set_labels (new_image->file);

}

void
set_undo_option (status)
     int	status;
{
     if (status == ON)
       XtVaSetValues (base->undo_item, XmNsensitive, True, NULL);
    else
       XtVaSetValues (base->undo_item, XmNsensitive, False, NULL);
}


void
set_save_selection (status)
int	status;
{

/*
 * Turn on Save Selection As.
 *   If the Save As or Save Page As Image is showing, change to
 *     Save Selection As.
 */
    if (status == ON) {
      XtVaSetValues (base->savesel_item, XmNsensitive, True, NULL);
      XtVaSetValues (base->save_item, XmNsensitive, False, NULL);
      XtVaSetValues (base->saveas_item, XmNsensitive, False, NULL);
      XtVaSetValues (base->savepage_item, XmNsensitive, False, NULL);

      if (saveas && saveas->showing == True) {
	char  *title;
	XtVaGetValues (saveas->shell, XmNtitle, &title, NULL);
	if (strcmp (title, catgets (prog->catd, 1, 7, 
				    "Image Viewer - Save As")) == 0 ||
	    strcmp (title, catgets (prog->catd, 1, 8,
				    "Image Viewer - Save Page As Image")) == 0)
	  XtVaSetValues (saveas->shell, 
			 XmNtitle, catgets (prog->catd, 1, 9,
					    "Image Viewer - Save Selection As"),
			 NULL);
      }
    }
 
/*
 * Turn OFF Save Selection As.
 *   If the Save Selection As is showing, change to:
 *      Save As if image  or
 *      Save Page As Image if PostScript.
 */
    else {
      XtVaSetValues (base->savesel_item, XmNsensitive, False, NULL);
      XtVaSetValues (base->save_item, XmNsensitive, True, NULL);

      if (current_display == ps_display)
	XtVaSetValues (base->savepage_item, XmNsensitive, True, NULL);
      else
	XtVaSetValues (base->saveas_item, XmNsensitive, True, NULL);

      if (saveas && saveas->showing == True) {
	char  *title;
        
	XtVaGetValues (saveas->shell, XmNtitle, &title, NULL);
	if (strcmp (title, catgets (prog->catd, 1, 10,
				    "Image Viewer - Save Selection As")) == 0) {
	  
	  if (current_display == ps_display)
	    XtVaSetValues (saveas->shell,
			   XmNtitle, catgets (prog->catd, 1, 11,
				       "Image Viewer - Save Page As Image"),
			   NULL);
	  else
	    XtVaSetValues (saveas->shell,
			   XmNtitle, catgets (prog->catd, 1, 12,
				       "Image Viewer - Save As"),
			   NULL);
        }
      }
    }

}

#ifdef LATER
void
canvas_event_proc (xvwin, event)
Xv_window	 xvwin;
Event		*event;
{

/*
 * NOTE:
 * These events are also caught in base_window_event_proc().
 * Any changes to this code below will also need to be changed
 * in base_window_event_proc() to catch cases where
 * "click to type" is set.
 */
    if ((event_is_ascii(event) || event_is_key_right(event) ||
        (event_action (event) == ACTION_ERASE_CHAR_BACKWARD)) &&
         event_is_down(event) && (current_image != NULL) &&
         (current_display == ps_display)) {
        switch (event_action(event)) {
        case ACTION_GO_DOCUMENT_START:          /* Home */
	case ACTION_GO_LINE_BACKWARD:
            first_page();
            break;
        case ACTION_GO_DOCUMENT_END:            /* End */
 	case ACTION_GO_LINE_END:
            last_page();
            break;
        case '\03':                             /* Cntrl-C */
            exit (3);
        case '\r':                              /* Return */
        case '\12':                             /* Line Feed */
        case ' ':                               /* Space */
        case ACTION_GO_PAGE_FORWARD:            /* PageDown */
            next_page();
            break;
        case '\b':                              /* Cntrl-H */
        case ACTION_ERASE_CHAR_BACKWARD:        /* Backspace */
        case ACTION_GO_PAGE_BACKWARD:           /* PageUp */
            prev_page();
            break;
        default:
            if (prog->verbose)
                fprintf(stderr, 
			catgets (prog->catd, 1, 13, 
			  "Keyboard: key '%c' %d pressed at %d,%d\n"), 
	                event_action(event), event_action(event),
                        event_x(event), event_y(event));
        }
    } 
/*
    else if (event_is_down (event) && (event_action (event) == ACTION_PROPS)) {
            (void)props_callback (NULL, MENU_NOTIFY);
    }
*/
    else if (event_is_down (event) && (event_action (event) == ACTION_UNDO)) {
            if (xv_get (xv_get (base_window->edit_menu, MENU_NTH_ITEM, UNDO),
                  MENU_INACTIVE) == FALSE)
              undo_callback (NULL, NULL);
    }
    else if ((event_action (event) == WIN_CLIENT_MESSAGE) &&
	     (current_display == ps_display))
       dps_event_func (event->ie_xevent);

/*
    else {
        switch (event_action(event)) {
 *
 * Fix this later...
 * if NOT olwm then...
 *
	case LOC_WINENTER:
	  XInstallColormap (current_display->xdisplay, current_image->cmap);
	  break;

        default:
            return;
        }      
    }
*/

}
#endif

Pixmap
create_pixmap (dsp, width, height, depth, set_display)
DisplayInfo *dsp;
int width;
int height;
int depth;
int set_display;
{
    XGCValues   gc_vals;
    Pixmap      pixmap;
    GC          pixmap_gc;
    unsigned long gc_mask;

    if (set_display == TRUE) {
       dsp->pix_height = height;
       dsp->pix_width = width;
/*       dsp->depth = depth; */
       }
    pixmap = XCreatePixmap(dsp->xdisplay, RootWindow(dsp->xdisplay,
                                          DefaultScreen(dsp->xdisplay)),
                           width, height, depth ); 

    gc_vals.function = GXclear;
    gc_mask = GCFunction;
    pixmap_gc = XCreateGC (dsp->xdisplay, pixmap, gc_mask, &gc_vals);
    XFillRectangle (dsp->xdisplay, pixmap, pixmap_gc, 0, 0, width, height);

    XFreeGC (dsp->xdisplay, pixmap_gc);
    XFlush (dsp->xdisplay);

    return (pixmap);
}

     
void
update_page_number (current, last, old)
int	current;
int	last;
int	old;
{
    static char right[80];
    char        value[80];
    int         len, max;
    Boolean     sensitive;
    XmString    xmstring;

    if (last < 0) {
/*
 * Updating page number for "current of x".
 */
       sprintf (right, catgets (prog->catd, 1, 14, "Page: %d of %d"), 
		current, 0);
       len = strlen (right);
       right [len - 1] = 'x';
       XtVaGetValues (base->goto_scale, XmNmaximum, &max, NULL);
/*
 * Make sure current page number has maximum set
 * before setting or Xtwarning will occur.
 */
       if (current > max)
         XtVaSetValues (base->goto_scale, XmNvalue, current, 
					  XmNmaximum, current,
		                          XmNsensitive, False, NULL);
       else
         XtVaSetValues (base->goto_scale, XmNvalue, current, 
		                          XmNsensitive, False, NULL);
       XtVaSetValues (base->goto_label, XmNsensitive, False, NULL);
       sprintf (value, "%-d", current);
       xmstring = XmStringCreateLocalized (value);
       XtVaSetValues (base->goto_pgno, XmNsensitive, False, 
		                       XmNlabelString, xmstring, NULL);
       XmStringFree (xmstring);
     }
     else {
       if (last > 1) {	
	 XtVaSetValues (base->goto_scale, XmNvalue, current,
	 		                  XmNmaximum, last, 
			                  XmNsensitive, True, NULL);
	 XtVaSetValues (base->goto_label, XmNsensitive, True, NULL);
 	 sprintf (value, "%-d", current);
 	 xmstring = XmStringCreateLocalized (value);
	 XtVaSetValues (base->goto_pgno, XmNsensitive, True,
			                 XmNlabelString, xmstring, NULL);
	 XmStringFree (xmstring);
       }
       else {
	 XtVaSetValues (base->goto_scale, XmNvalue, 1,
			                  XmNsensitive, False, NULL);
	 XtVaSetValues (base->goto_label, XmNsensitive, False, NULL);
	 sprintf (value, "%-d", 1);
	 xmstring = XmStringCreateLocalized (value);
	 XtVaSetValues (base->goto_pgno, XmNsensitive, False,
			                 XmNlabelString, xmstring, NULL);
	 XmStringFree (xmstring);
       }
       sprintf (right, catgets (prog->catd, 1, 15, "Page: %d of %d"), 
		current, last);
     }

    SetFooter (base->right_footer, right);
/*
 * Not last page, set page fwd active.
 */
    if (old == abs (last)) {
      XtVaGetValues (base->pgfwd_button, XmNsensitive, &sensitive, NULL);
      if (sensitive == False)
	XtVaSetValues (base->pgfwd_button, XmNsensitive, True, NULL);
    }
/*
 * Not page 1, set page back active.
 */
    else if (old == 1) {
      XtVaGetValues (base->pgback_button, XmNsensitive, &sensitive, NULL);
      if (sensitive == False)
        XtVaSetValues (base->pgback_button, XmNsensitive, True, NULL);
       }
/*
 * At end, set page fwd inactive.
 */
    if (current == abs (last)) {
      XtVaGetValues (base->pgfwd_button, XmNsensitive, &sensitive, NULL);
      if (sensitive == True)
	XtVaSetValues (base->pgfwd_button, XmNsensitive, False, NULL);
       }
/*
 * At beginning, set page back inactive.
 */
    else if (current == 1) {
      XtVaGetValues (base->pgback_button, XmNsensitive, &sensitive, NULL);
      if (sensitive == True)
	XtVaSetValues (base->pgback_button, XmNsensitive, False, NULL);
       }
}

/*
 * Return current selection on panel list.
 */

int
GetListSelection (list)
     Widget   list;
{
     Boolean  success;
     int     *pos_list;
     int      pos_cnt;

     success = XmListGetSelectedPos (list, &pos_list, &pos_cnt);
     if (success == False) 
       return -1;
     else if (pos_cnt > 0)
       return (pos_list[0] - 1);
     else
       return -1;
}


