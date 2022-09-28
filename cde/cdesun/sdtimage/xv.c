#ifndef lint
static char sccsid[] = "@(#)xv.c 1.17 96/04/25";
#endif

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

/*
 * xv.c - I think that some of these functions can go away...
 */

#define  XK_MISCELLANY
#define  XK_LATIN1
#include <X11/keysymdef.h>

#include "image.h"
#include "imagetool.h"
#include "display.h"
#include "state.h"
#include "ui_imagetool.h"
#include "props.h"


extern int			started_thumbnails;
int		 max_x;
int		 max_y;
int		 min_x;
int		 min_y;
static int prevx;
static int prevy;
static int erase_box = False;
static int top_left_x, top_left_y;


void
display_error (window, error)
    Window     window;
    char      *error;
{

    SetFooter (base->left_footer, error);
    XBell (XtDisplay (base->top_level), 0);

}

void
MainWEventHandler (Widget     w,
		   XtPointer  client_data,
		   XEvent    *event,
		   Boolean   *dispatch)
{

     switch (event->type) {
     case ConfigureNotify:
       break;

     case MapNotify:
/*
      if ((prog->frame_mapped == FALSE) && (current_display == ps_display) &&
          (current_state != NULL) && (prog->dps == TRUE)) {
	 started_thumbnails = TRUE;
         do_dps_op (0);
	 }
*/

       prog->tt_timer_set = FALSE;
       prog->frame_mapped = TRUE;
       break;
       
     default:
       break;  /* end switch event */
     }
}

void
CanvasEventHandler (Widget     w,
		    XtPointer  client_data,
		    XEvent    *event,
		    Boolean   *dispatch)
{
    switch (event->type) {
    case ClientMessage:
      dps_event_func (event);
      break;
      
    default:
      break;
    }
  
}

void
KeyEventHandler (Widget     w,
		 XtPointer  client_data,
		 XEvent    *event,
		 Boolean   *dispatch)
{
     KeySym  keysym;

     if (event->type != KeyPress)
       return;

     keysym = XKeycodeToKeysym (XtDisplay (w), event->xkey.keycode, 0);

     switch (keysym) {
/*
 * Page Up
 */
     case XK_KP_Prior:
     case XK_Prior:
     case XK_R9:
     case XK_Left:
     case XK_Up:
     case XK_BackSpace:
     case XK_Delete:
       prev_page();
       break;
/*
 * Page Down
 */       
     case XK_KP_Next:
     case XK_Next:
     case XK_R15: 
     case XK_KP_Enter:
     case XK_Return:
     case XK_space:
     case XK_Right:
     case XK_Down:
       next_page();
       break;
/*
 * Home
 */       
     case XK_KP_Home:
     case XK_Home:
     case XK_R7:
       first_page();
       break;
/*
 * End
 */       
     case XK_KP_End:
     case XK_End:
     case XK_R13:
       last_page();
       break;
/*
 * Props key
 */
     case XK_L3:
       break;
       
     default:
       break;
     }
}

/*
 * Catch Props key to display Properties popup.
 */
void
PropsKeyEventHandler (Widget     w,
		      XtPointer  client_data,
		      XEvent    *event,
		      Boolean   *dispatch)
{
     KeySym  keysym;

     if (event->type != KeyPress)
       return;

     keysym = XKeycodeToKeysym (XtDisplay (w), event->xkey.keycode, 0);

     switch (keysym) {
     case XK_L3:
       PropsCallback (NULL, NULL, NULL);
       break;
     default:
       break;
     }
}

void
CanvasInputCallback (widget, client_data, cbs)
     Widget    widget;
     XtPointer client_data;
     XmDrawingAreaCallbackStruct  *cbs;
{
     XEvent *event =  cbs->event;

     if (current_state == NULL)
	return;

     if (cbs->reason == XmCR_INPUT && event->type == ButtonPress) {
       /*
	* Save starting point for panning.
	*/
       if (current_state->pan == TRUE) {
	 prevx = event->xbutton.x;
	 prevy = event->xbutton.y;
       }

       /*
	* Selecting Image for crop:
	* - Erase old box if exists.
	* - Save new corner.
	* - Reset select region to 0.
	*/
       else if (current_state->select == TRUE) {

	 if (current_state->image_selected == TRUE) {
	    
	   drawbox ((current_state->save_rect).top_x, 
		    (current_state->save_rect).top_y, 
		    (current_state->save_rect).new_x, 
		    (current_state->save_rect).new_y, 
		    &(current_state->save_rect), FALSE);
	   current_state->image_selected = FALSE;
	 }

	 top_left_x = event->xbutton.x;
	 top_left_y = event->xbutton.y;

	 erase_box = FALSE;

	 (current_state->sel_rect).x = 0;
	 (current_state->sel_rect).y = 0;
	 (current_state->sel_rect).width = 0;
	 (current_state->sel_rect).height = 0;
       }
       
     }

     else if (cbs->reason == XmCR_INPUT && event->type == ButtonRelease) {
       /*
	* Done drawing box:
	* - Erase old box
	* - Save current box.
	* - Set flags.
	*/
       if (current_state->select == TRUE) {
	 drawbox (top_left_x, top_left_y, event->xbutton.x,
		  event->xbutton.y, &(current_state->sel_rect), erase_box);
	 (current_state->save_rect).top_x = top_left_x;
	 (current_state->save_rect).top_y = top_left_y;
	 (current_state->save_rect).new_x = event->xbutton.x;
	 (current_state->save_rect).new_y = event->xbutton.y;
	 (current_state->save_rect).xrect = current_state->sel_rect;
	 if ((current_state->sel_rect).width == 0 ||
	     (current_state->sel_rect).height == 0) {
	   current_state->image_selected = TRUE;
	   current_state->sel_rect.width = 2;
	   set_save_selection (ON);
	 }  
	 else {
	   current_state->image_selected = TRUE;
	   set_save_selection (ON); 
	 }  
	 erase_box = FALSE;
       }
     }
     
}

void
SelectEventHandler (Widget      w,
		    XtPointer   client_data,
		    XEvent     *event)
{
     static int    x, y;
     XEvent        ev;
  
     if (current_state == NULL)
       return;

     /*
      * Compress Motion Notify Events
      */
     while (XEventsQueued (current_display->xdisplay, QueuedAfterFlush) &&
	    (XPeekEvent (current_display->xdisplay, &ev), (ev.xmotion.type == MotionNotify))) {
       XNextEvent (current_display->xdisplay, &ev);
     }
     x = event->xmotion.x;
     y = event->xmotion.y;

     /***********
      * Panning...
      ***********/
     if (current_state->pan == TRUE) {

       current_state->old_x = current_state->currentx;
       current_state->old_y = current_state->currenty;
       current_state->currentx += x - prevx;
       current_state->currenty += y - prevy;
	 
       if (current_state->currentx > max_x)
	 current_state->currentx = max_x;
       if (current_state->currentx < min_x)
	 current_state->currentx = min_x;
       if (current_state->currenty > max_y)
	 current_state->currenty = max_y;
       if (current_state->currenty < min_y)
	 current_state->currenty = min_y;
       
       if ((current_state->old_x != current_state->currentx) ||
	   (current_state->old_y != current_state->currenty)) {
	 adjust_scrollbars ();
	 current_state->set_roi = TRUE;
	 update_image();
       }
       prevx = x;
       prevy = y;
     }

     /***********
      * Selecting...
      ***********/
     else if (current_state->select == TRUE) {
       drawbox (top_left_x, top_left_y, event->xmotion.x, event->xmotion.y,
		&(current_state->sel_rect), erase_box);
       erase_box = TRUE; 
     } 

}

void
DetachScrollbar (sb)
     Widget  sb;
{
     XtUnmanageChild (sb);
     XtVaSetValues (sb, XmNvalue, 0, NULL);
}

void
AttachScrollbar (sb)
     Widget  sb;
{
     XtManageChild (sb);
}

void
FormEventHandler (Widget     form,
		  XtPointer  client_data,
		  XEvent    *event,
		  Boolean   *dispatch)
{
    BaseObjects      *b = (BaseObjects *)client_data;
    Dimension         width, height;
    Dimension         disp_height, disp_width;
    static Dimension  prev_width, prev_height;
    int               size_changed = False;
    int               vinc, hinc;

    switch (event->type) {
    case ConfigureNotify:
      DetachScrollbar (base->hscroll);
      DetachScrollbar (base->vscroll);
      /*
       * Determine the height/width of the viewing area:
       */
      if (current_display && current_display->new_canvas && current_image) {
	XtVaGetValues (current_display->new_canvas, 
		       XmNheight, &disp_height,
		       XmNwidth, &disp_width, 
		       NULL);
	disp_width += 2;
	disp_height += 2;
 	if (((int)disp_height - (int) base->sb_width - 10) > 0)
	   vinc = disp_height - base->sb_width - 10;
        else
	   vinc = disp_height;
 	if (((int)disp_width - (int) base->sb_width - 10) > 0)
	   hinc = disp_width - base->sb_width - 10;
        else
	   hinc = disp_width;
	XtVaSetValues (base->vscroll, XmNpageIncrement, vinc, NULL);
	XtVaSetValues (base->hscroll, XmNpageIncrement, hinc, NULL);
      }
      else 
	return;
      
      /*
       * Determine if scrollbars are needed. 
       */
      if (disp_height >= current_image->height)
	DetachScrollbar (base->vscroll);
      else {
	AttachScrollbar (base->vscroll);
	disp_width -= base->sb_width;
      }

      if (disp_width >= current_image->width)
	DetachScrollbar (base->hscroll);
      else {
	AttachScrollbar (base->hscroll);
	disp_height -= base->sb_width;
      }
	
      /*
       * Return if no size change.
       */
      if ((disp_width == prev_width) && (disp_height == prev_height))
	return;
      
      /*
       * Make sure current_display and image are set.
       */
      if ((current_display != NULL) && (current_image != NULL)) {

	if (current_display == image_display)
	  if (disp_height != (*xil->get_height) (image_display->display_win) ||
	      disp_width != (*xil->get_width) (image_display->display_win))
 	    size_changed = True;
	
	/*
	 * Re-Create the xil window for image_display
	 */
	if (current_display == image_display && size_changed == True) {  
	  if (image_display->display_win) 
	    (*xil->destroy) (image_display->display_win);
	  image_display->display_win = 
	    (*xil->create_from_window) (image_display->state, 
				    image_display->xdisplay, 
			            image_display->win);
	}

/*
 * Set the new display height and width.
*/
	XtVaGetValues (current_display->new_canvas, 
			 XmNheight, &current_display->height,
			 XmNwidth, &current_display->width,
			 NULL);
	disp_width = current_display->width;
	disp_height = current_display->height;
      }
      
      if (current_image != NULL) {
	current_state->currentx = 0;
	current_state->currenty = 0;
	check_canvas (current_image->width, current_image->height, 
		      current_display);
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
      update_image();

      /*
       * Save values for next time.
       */
      prev_width = disp_width;
      prev_height = disp_height;
      break;
      
    default:
      break;
    }  /* end switch */

}

#ifdef NEVER
Notify_value
base_window_resize (frame, event, arg, type)
    Frame               frame;
    Event              *event;
    Notify_arg          arg;
    Notify_event_type   type;
{ 
    Rect                rect;
    Xv_drop_site        frame_dropsite;
    int                 panel_height;
    int                 curr_width, curr_height;
    static int          old_width = 0, old_height = 0;

/*
 * Clear out the left footer, just in case we'll be displaying some
 * text there.
    xv_set (base_window->base_window, FRAME_LEFT_FOOTER, "", NULL);
 */

/*
 * Return if no size change.
 */
    curr_width  = xv_get (frame, XV_WIDTH);
    curr_height = xv_get (frame, XV_HEIGHT);

    if ((curr_width == old_width) && (curr_height == old_height))
      return (notify_next_event_func (frame, (Notify_event)event, arg, type));
/*                                                       
 * User resized the window. Reposition the drop site.
 */
    place_drop_site (base_window->drop_target, curr_width);

/*
 * Check only if the canvas needs to be larger.
 * "2" below comes from left and right border of 1.
 */
    panel_height = xv_get (base_window->base_panel, XV_HEIGHT);

    if ((current_display != NULL) && (current_image != NULL)) {
      if ((curr_height - panel_height - scroll_height) >= current_image->height)
        xv_set (current_display->canvas, 
		CANVAS_HEIGHT, curr_height - panel_height - scroll_height - 2,
		NULL);
      else
	xv_set (current_display->canvas, CANVAS_HEIGHT, current_image->height, NULL);
  
      if (curr_width - scroll_width >= current_image->width)
	xv_set (current_display->canvas, 
		CANVAS_WIDTH, curr_width - scroll_width - 2,
		NULL);
      else
	xv_set (current_display->canvas, CANVAS_WIDTH, current_image->width, NULL);

      XSync (image_display->xdisplay, 0);
    
      if (prog->xil_opened && current_display == image_display) {  
        if (image_display->display_win) 
	  (*xil->destroy) (image_display->display_win);
        image_display->display_win = 
	    (*xil->create_from_window) (image_display->state, 
				    image_display->xdisplay, 
			            image_display->win);
      }
    }
    else if (current_display == ps_display) {
      xv_set (current_display->canvas,
	      CANVAS_HEIGHT, curr_height - panel_height - scroll_height - 2,
	      CANVAS_WIDTH, curr_width - scroll_width - 2,
	      NULL);
      current_display->height = xv_get (current_display->canvas, CANVAS_HEIGHT);
      current_display->width = xv_get (current_display->canvas, CANVAS_WIDTH);
    }


    if (current_image != NULL) {
       if ((current_image->type_info->type == POSTSCRIPT) ||
	   (current_image->type_info->type == EPSF))
          check_canvas (current_image->width, current_image->height, current_display);
       else
          check_canvas (current_image->width, current_image->height, current_display);
    }
    old_width = curr_width;
    old_height = curr_height;

    frame_dropsite = xv_get (base_window->base_window, XV_KEY_DATA, DROPSITE);
    rect.r_left = 0;
    rect.r_top = xv_get (base_window->base_panel, XV_HEIGHT);
    rect.r_width = xv_get (base_window->base_window, XV_WIDTH);
    rect.r_height = xv_get (base_window->base_window, XV_HEIGHT);
    
    xv_set (frame_dropsite,
		DROP_SITE_DELETE_REGION,	NULL,
		DROP_SITE_REGION,		&rect,
		NULL);

    return (notify_next_event_func (base_window->base_window, (Notify_event)event, 
				    arg, type));
}  
 
static int
adjust_position (x, y)
    int *x, *y;
{
    int display_height, display_width;

    display_height = xv_get (base_window->base_window, XV_HEIGHT) -
                     xv_get (base_window->base_panel, XV_HEIGHT) -
		     scroll_height - 2;

    display_width = xv_get (base_window->base_window, XV_WIDTH) -
                    scroll_width - 2;

    if (*x > display_width)
      *x = display_width;
    else if (*x < 0)
      *x = 0;

    if (*y > display_height)
      *y = display_height;
    else if (*y < 0)
      *y = 0;
}

Notify_value
base_window_event_proc (window, event, arg, type)
Xv_opaque                window;
Event                   *event;
Notify_arg               arg;
Notify_event_type        type;
{
    Notify_value      value;
    static int        erase = FALSE;
    static int        panel_height;
    static int        x, y, prevx, prevy;
    static int        top_left_x, top_left_y;
    int               frame_x, frame_y;
    static int        x_view_start, y_view_start;
    static Scrollbar  hscroll;
    static Scrollbar  vscroll;
    XEvent            ev;

/*
 * NOTE:
 * These events are also caught in canvas_event_proc().
 * Any changes to this code below will also need to be changed
 * in canvas_event_proc() to catch cases where
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
                fprintf(stderr, MGET("Keyboard: key '%c' %d pressed at %d,%d\n"),                       event_action(event), event_action(event),
                       event_x(event), event_y(event));
        }
  } 
#ifdef LATER
  else if (event_is_down (event) && (event_action (event) == ACTION_PROPS)) {
            (void)props_callback (NULL, MENU_NOTIFY);
}
#endif
  else if (event_is_down (event) && (event_action (event) == ACTION_UNDO)) {
    if (xv_get (xv_get (base_window->edit_menu, MENU_NTH_ITEM, UNDO),
		MENU_INACTIVE) == FALSE) 
      undo_callback (NULL, NULL);
  }
  else {

    switch (event_action (event)) {
    case MS_LEFT:
    case ACTION_SELECT:
      if (current_state != (StateInfo *) NULL) {
         frame_x = event_x (event);
         frame_y = event_y (event);
         panel_height = xv_get (base_window->base_panel, XV_HEIGHT);
     if (prog->sb_right) 
        x = frame_x - 1;
      else
        x = frame_x - scroll_width - 1;
	
      y = frame_y - panel_height - 1;
      adjust_position (&x, &y);

/*
 * Selecting...
 * o Erase old box if something selected.
 * o Save new top left.
 * o Reset select region to 0.
 */
         if (current_state->select == TRUE) {

	   if (event_is_down (event)) {

	     if (current_display == ps_display) {
	       hscroll = base_window->ps_hscroll;
	       vscroll = base_window->ps_vscroll;
  	     }
	     else {
	       hscroll = base_window->hscroll;
	       vscroll = base_window->vscroll;
	     }
	     x_view_start = xv_get (hscroll, SCROLLBAR_VIEW_START);
	     y_view_start = xv_get (vscroll, SCROLLBAR_VIEW_START);
   
	     if (current_state->image_selected == TRUE) {
	    
	       drawbox ((current_state->save_rect).top_x, 
		        (current_state->save_rect).top_y, 
		        (current_state->save_rect).new_x, 
		        (current_state->save_rect).new_y, 
		        &(current_state->save_rect), FALSE);
               current_state->image_selected = FALSE;
	     }
             top_left_x = x + x_view_start;
	     top_left_y = y + y_view_start;
	     erase = FALSE;
	     (current_state->sel_rect).r_left = 0;
	     (current_state->sel_rect).r_top = 0;
  	     (current_state->sel_rect).r_width = 0;
	     (current_state->sel_rect).r_height = 0;
           }
           else if (event_is_up (event)) {
             drawbox (top_left_x, top_left_y, x + x_view_start, 
		      y + y_view_start, &(current_state->sel_rect), erase);
             (current_state->save_rect).top_x = top_left_x;
	     (current_state->save_rect).top_y = top_left_y;
	     (current_state->save_rect).new_x = x + x_view_start;
  	     (current_state->save_rect).new_y = y + y_view_start;
	     (current_state->save_rect).rect  = current_state->sel_rect;
	     if ((current_state->sel_rect).r_width == 0 ||
	         (current_state->sel_rect).r_height == 0) {
	       current_state->image_selected = FALSE;
	       set_save_selection (OFF);
	     }  
	     else {
	       current_state->image_selected = TRUE;
	       set_save_selection (ON); 
             }  
	     erase = FALSE;
           }
         } /* end if select */
/*
 * Panning...
 */
         else if ((current_state->pan == TRUE) && (event_is_down (event))) {
	   prevx = x;
	   prevy = y;
         }
         else if ((current_state->pan == TRUE) && (event_is_up (event))) {
         }
      }
      break;

    case LOC_DRAG:
      if (current_state != (StateInfo *) NULL) {
         while (XEventsQueued (current_display->xdisplay, QueuedAfterFlush) &&
	       (XPeekEvent (current_display->xdisplay, &ev), (ev.xmotion.type == MotionNotify))) {
	   XNextEvent (current_display->xdisplay, &ev);
         }
         frame_x = event_x (event);
         frame_y = event_y (event);
	 if (prog->sb_right)
	   x = frame_x - 1;
	 else
	   x = frame_x - scroll_width - 1;
         y = frame_y - panel_height - 1;
         adjust_position (&x, &y);
/*
 * Panning...
 */
         if (current_state->pan == TRUE) {
           int  diffx, diffy;

	   current_state->old_x = current_state->currentx;
	   current_state->old_y = current_state->currenty;
	   current_state->currentx += x - prevx;
	   current_state->currenty += y - prevy;
   
	   diffx = prevx - x;
	   diffy = prevy - y;
	       
	   if (current_state->currentx > max_x)
	     current_state->currentx = max_x;
   	   if (current_state->currentx < min_x)
	     current_state->currentx = min_x;
	   if (current_state->currenty > max_y)
	     current_state->currenty = max_y;
	   if (current_state->currenty < min_y)
	     current_state->currenty = min_y;

	   adjust_scrollbars (diffx, diffy);
           
	   if ((current_state->old_x != current_state->currentx) ||
	       (current_state->old_y != current_state->currenty)) {
	     current_state->set_roi = TRUE;
	     update_image();
	   }
           prevx = x;
           prevy = y;
         }
/*
 * Loc Drag with Selection Tool.
 */
         else if (current_state->select == TRUE) {
	   x_view_start = xv_get (hscroll, SCROLLBAR_VIEW_START);
           y_view_start = xv_get (vscroll, SCROLLBAR_VIEW_START);
	   drawbox (top_left_x, top_left_y, x + x_view_start, y + y_view_start,
		    &(current_state->sel_rect), erase);
	   erase = TRUE;
         }
      }
      break;
    case ACTION_DRAG_COPY:
    case ACTION_DRAG_MOVE:
      
/*
 * Get the current selection object and set the Current_Op to DND_CANVAS_OP
 * so we know we're dealing with a dnd operation to the pageview canvas.
 */

      current_sel = (Selection_requestor)
	xv_get (window, XV_KEY_DATA, SELECTOR);
      
      current_dnd_object = (Dnd) xv_get (window, XV_KEY_DATA, DROPSITE);
      
/*
 * Clear out the left footer, just in case we'll be displaying some
 * text there.
 */
 
      xv_set (base_window->base_window, 
	      FRAME_LEFT_FOOTER, "", NULL);

/* If the user dropped over an acceptable                     
 * drop site, the owner of the drop site will
 * be sent an ACTION_DROP_{COPY, MOVE} event.
 */
 
/* To acknowledge the drop and to associate the
 * rank of the source's selection to our
 * requestor selection object, we call
 * dnd_decode_drop().
 */

      if (dnd_decode_drop (current_sel, event) != XV_ERROR) {

 
/* We can use the macro dnd_site_id() to access
 * the site id of the drop site that was
 * dropped on.
 * In this case, must be site 1 (frame).
 */
 
	int site_id = dnd_site_id (event);
	
	if (site_id == 1) 
	  load_from_dragdrop();
	
      }
      
      else {
	if (prog->verbose)
	  fprintf (stderr, MGET("drop error\n"));
      }
      
      break;

/*
 * Frame resize.
 */
    case WIN_RESIZE:
      value = base_window_resize (window, event, arg, type);
      break;
/*
 * This is to tell us when to display the palette.
 * We only do this on the first WIN_VIS notify event and
 * then set a flag.
 */
    case WIN_VISIBILITY_NOTIFY:
/*
      if ((current_image != NULL) && (current_props->show_palette) &&
	  (prog->tt_timer_set == FALSE)) {
 * Place palette is already showing.
        if (palette) {
	    ds_position_popup (base_window->base_window, palette->palette, 
			       DS_POPUP_RIGHT);
	    xv_set (palette->palette, XV_SHOW, TRUE, 
		    FRAME_CMD_PUSHPIN_IN, TRUE, 
		    NULL);
	}
        else
	  palette_callback (NULL, MENU_NOTIFY);

        set_zoom_and_rotate();
      }
*/
      if ((prog->frame_mapped == FALSE) && (current_display == ps_display) &&
          (current_state != NULL) && (prog->dps == TRUE)) {
	 started_thumbnails = TRUE;
         do_dps_op (0);
	 }
      prog->tt_timer_set = FALSE;
      prog->frame_mapped = TRUE;
      break;

    default:
      break;
    }
 
  }
    value = notify_next_event_func (base_window->base_window, 
				    (Notify_event)event, arg, type);
    return (value);
}
#endif

#ifdef LATER

void
show_list_selection (list)
    Panel_item  list;
{
    Scrollbar  sb = (Scrollbar) xv_get (list, PANEL_LIST_SCROLLBAR);
    int        drows, nrows, greatest, view_start;

    view_start = current_list_selection (list);
    if (view_start == -1) 
      return;

    drows = xv_get (list, PANEL_LIST_DISPLAY_ROWS);    
    nrows = xv_get (list, PANEL_LIST_NROWS);    
    greatest = nrows - drows;
    if (view_start > greatest)
      view_start = greatest;
    xv_set (sb, SCROLLBAR_VIEW_START, view_start, NULL);
    
}
#endif
