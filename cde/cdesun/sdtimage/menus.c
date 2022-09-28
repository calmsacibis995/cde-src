#ifndef lint
static char sccsid[] = "@(#)menus.c 1.25 97/05/29";
#endif

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

#include <Xm/RowColumn.h>
#include "display.h"
#include "help.h"
#include "image.h"
#include "imagetool.h"
#include "props.h"
#include "state.h"
#include "tt.h"
#include "misc.h"
#include "ui_imagetool.h"
#include "snap_dialog.h"

/*
 * menus.c - callback routines for menu items.
 */

Boolean     open_showing = FALSE;
Boolean     save_showing = FALSE;


/*
 * General event handler for all popups to
 * catch Maps/Unmaps of the dialog and set flag.
 */
void
PopupEventHandler (w, client_data, event, dispatch)
     Widget     w;
     XtPointer  client_data;
     XEvent    *event;
     Boolean   *dispatch;
{
     Widget     dialog = (Widget) client_data;

     if (open_dialog && dialog == open_dialog) {
       if (event->type == MapNotify)
	 open_showing = TRUE;
       else if (event->type == UnmapNotify)
	 open_showing = FALSE;
     }
     else if (openas && dialog == openas->dialog) {
       if (event->type == MapNotify)
	 openas->showing = TRUE;
       else if (event->type == UnmapNotify)
	 openas->showing = FALSE;
     }
     else if (save_dialog && dialog == save_dialog) {
       if (event->type == MapNotify)
	 save_showing = TRUE;
       else if (event->type == UnmapNotify)
	 save_showing = FALSE;
     }
     else if (saveas && dialog == saveas->dialog) {
       if (event->type == MapNotify)
	 saveas->showing = TRUE;
       else if (event->type == UnmapNotify)
	 saveas->showing = FALSE;
     }
     else if (palette && dialog == palette->dialog) {
       if (event->type == MapNotify)
	 palette->showing = TRUE;
       else if (event->type == UnmapNotify)
	 palette->showing = FALSE;
     }
     else if (info && dialog == info->dialog) {
       if (event->type == MapNotify)
	 info->showing = TRUE;
       else if (event->type == UnmapNotify)
	 info->showing = FALSE;
     }
     else if (pg_overview && dialog == pg_overview->dialog) {
       if (event->type == MapNotify)
	 pg_overview->showing = TRUE;
       else if (event->type == UnmapNotify)
	 pg_overview->showing = FALSE;
     }
     else if (pg_controls && dialog == pg_controls->dialog) {
       if (event->type == MapNotify)
	 pg_controls->showing = TRUE;
       else if (event->type == UnmapNotify)
	 pg_controls->showing = FALSE;
     }
     else if (properties && dialog == properties->dialog) {
       if (event->type == MapNotify)
	 properties->showing = TRUE;
       else if (event->type == UnmapNotify)
	 properties->showing = FALSE;
     }
     else if (print && dialog == print->dialog) {
       if (event->type == MapNotify)
	 print->showing = TRUE;
       else if (event->type == UnmapNotify)
	 print->showing = FALSE;
     }
     else if (print_prev && dialog == print_prev->dialog) {
       if (event->type == MapNotify)
	 print_prev->showing = TRUE;
       else if (event->type == UnmapNotify)
	 print_prev->showing = FALSE;
     }
     else if (about && dialog == about->dialog) {
       if (event->type == MapNotify)
	 about->showing = TRUE;
       else if (event->type == UnmapNotify)
	 about->showing = FALSE;
     }
}

/*
 * Menu handler for Open...
 */
void
OpenCallback (w, client_data, call_data)
    Widget     w;
    XtPointer  client_data;
    XtPointer  call_data;
{
    Boolean    set_min = False;

    if (dtb_snap_dlog_dialog.snap_needs_save)
      {
        Boolean cont = SaveUnsavedSnapshot();
        if (cont == False)
          return;
      }

    setbusy ();

    /*
     * Create File Selection Box if not already created.
     */
    if (!open_dialog) {
      open_dialog = OpenObjectsInitialize (base->top_level);
      set_min = True;
    }

    /*
     * Highlight current file and fill in filename in text field.
     */

    if (current_image)
      SetCurrentDoc (open_dialog, basename (current_image->file));

    ShowDialog (open_dialog, open_showing);
#ifdef NEVER
    if (set_min == True) {
      Dimension  width, height;

      XtVaGetValues (XtParent (open_dialog), XmNwidth, &width,
		     XmNheight, &height, NULL);
      XtVaSetValues (XtParent (open_dialog), XmNminWidth, width,
		     XmNminHeight, height, NULL);
    }
#endif
    setactive ();

}

/*
 * Menu handler for Open As...
 */
void
OpenAsCallback (w, client_data, call_data)
    Widget     w;
    XtPointer  client_data;
    XtPointer  call_data;
{
    Boolean    set_min = False;

    if (dtb_snap_dlog_dialog.snap_needs_save)
      {
        Boolean cont = SaveUnsavedSnapshot();
        if (cont == False)
          return;
      }

    setbusy ();
    /*
     * Create File Selection Box if not already created.
     */
    if (!openas) {
      openas = OpenAsObjectsInitialize (base->top_level);
      set_min = True;
      set_openas_list (openas->format_list);
    }

    /*
     * Highlight current file and fill in filename in text field.
     */
    if (current_image)
      SetCurrentDoc (openas->dialog, basename (current_image->file));

    ShowDialog (openas->dialog, openas->showing);
#ifdef NEVER
    if (set_min == True) {
      Dimension  width, height;

      XtVaGetValues (XtParent (openas->dialog), XmNwidth, &width,
		     XmNheight, &height, NULL);
      XtVaSetValues (XtParent (openas->dialog), XmNminWidth, width,
		     XmNminHeight, height, NULL);
    }
#endif
    setactive ();

}

/*
 * Menu handler for Save...
 */
void
SaveCallback (w, client_data, call_data)
    Widget     w;
    XtPointer  client_data;
    XtPointer  call_data;
{
    Boolean    set_min = False;

    setbusy ();
    /*
     * Create File Selection Box if not already created.
     */
    if (!save_dialog) {
      save_dialog = SaveObjectsInitialize (base->top_level);
      set_min = True;
    }
    /*
     * Highlight current file and fill in filename in text field.
     */
    if (current_image)
      SetCurrentDoc (save_dialog, basename (current_image->file));

    ShowDialog (save_dialog, save_showing);
#ifdef NEVER
    if (set_min == True) {
      Dimension  width, height;

      XtVaGetValues (XtParent (save_dialog), XmNwidth, &width,
		     XmNheight, &height, NULL);
      XtVaSetValues (XtParent (save_dialog), XmNminWidth, width,
		     XmNminHeight, height, NULL);
    }
#endif
    setactive ();

}

/*
 * Menu handler for Save As...
 */
void
SaveAsCallback (w, client_data, call_data)
    Widget     w;
    XtPointer  client_data;
    XtPointer  call_data;
{
    Boolean    set_min = False;
    int        option = (int) client_data;

    setbusy ();
    /*
     * Create File Selection Box if not already created.
     */
    if (!saveas) {
      saveas = SaveAsObjectsInitialize (base->top_level);
      set_min = True;
      set_saveas_list (saveas->format_list);
    }
    /*
     * Set correct title label
     */
    if (option == SAVEAS)
      XtVaSetValues (saveas->shell, XmNtitle, catgets (prog->catd, 1, 395,
					      "Image Viewer - Save As"), NULL);
    else if (option == SAVEPAGEASIMAGE)
      XtVaSetValues (saveas->shell, XmNtitle, catgets (prog->catd, 1, 396,
			            "Image Viewer - Save Page As Image"), NULL);
    else
      XtVaSetValues (saveas->shell, XmNtitle, catgets (prog->catd, 1, 397,
				    "Image Viewer - Save Selection As"), NULL);

    /*
     * Highlight current file and fill in filename in text field.
commenting this out because it the filename extension will be preserved
but we don't want the same filename extension for other file formats.
fix to use templates later.
    if (current_image)
      SetCurrentDoc (saveas->dialog, basename (current_image->file));
     */

    ShowDialog (saveas->dialog, saveas->showing);
    if (set_min == True) {
      XmListCallbackStruct  cbs;
#ifdef NEVER
      Dimension             width, height;

      XtVaGetValues (XtParent (saveas->dialog), XmNwidth, &width,
		                                XmNheight, &height, NULL);
      XtVaSetValues (XtParent (saveas->dialog), XmNminWidth, width,
		                                XmNminHeight, height, NULL);
#endif
      /*
       * This has to be done after the dialog has been managed/realized.
       */
      cbs.reason = XmCR_BROWSE_SELECT;
      SaveFormatListCallback (saveas->format_list, NULL, &cbs);
    }

    setactive ();
#ifdef LATER

		if (current_image) {
		  CompTypes   compression;
		  char       *comp_str;
		  int         length, null_set = FALSE;

		  doc_name = xv_get (saveasfc->saveasfc, FILE_CHOOSER_CHILD,
	     			     FILE_CHOOSER_DOCUMENT_NAME_CHILD);

		  comp_str = (char *) xv_get (saveasfc->compression_value,
					      PANEL_LABEL_STRING);
		  compression = string_to_compression (comp_str);
		  length = strlen (current_image->file);
		  if ((length > 2) &&
		      (current_image->file [length - 1] == 'Z') &&
		      (current_image->file [length - 2] == '.') &&
		      (compression != UNIX)) {
		    current_image->file [length - 2] = '\0';
		    null_set = TRUE;
		  }

		  unique_filename (current_image->file, newname);
		  if (null_set)
		    current_image->file [length - 2] = '.';

                  xv_set (doc_name, PANEL_VALUE, basename (newname), NULL);
		  save_button = xv_get (saveasfc->saveasfc, FILE_CHOOSER_CHILD,
					FILE_CHOOSER_SAVE_BUTTON_CHILD);
		  xv_set (save_button, PANEL_INACTIVE, FALSE, NULL);
 	        }

#endif
}

/*
 * Menu handler for 'snapshot' menu
 */
void
PopupSnapshotCallback (Widget w, XtPointer client_data, XtPointer call_data)
{
  Boolean    set_min = False;
  BaseObjects *b = (BaseObjects *)client_data;

  setbusy();
  DtbCreateSnapDialog(&dtb_snap_dlog_dialog, b->top_level);
  setactive();

  return;
}

/*
 * Menu handler for `file_menu (Print Preview...)'.
 */
void
PrintPrevCallback (w, client_data, call_data)
     Widget     w;
     XtPointer  client_data;
     XtPointer  call_data;
{
     Boolean    set_extra = False;

     setbusy ();
/*
 * Create print popup when print_preview selected
 * so we can update print options if panning on preview.
 */
     if (!print)
       print = PrintObjectsInitialize (base->top_level);

     if ((current_image->type_info->type == POSTSCRIPT) ||
	 (current_image->type_info->type == EPSF))
       set_ps_print_options ();
     else
       set_image_print_options ();

     if (!print_prev) {
       print_prev = PrintPrevObjectsInitialize (base->top_level);
       set_extra = True;
     }

     ShowDialog (print_prev->dialog, print_prev->showing);
/*
 * Calculate excess height, width of canvas to shell
 * to determine dimensions later.
 */
     if (set_extra) {

       Dimension  frame_width, frame_height;
       Dimension  canvas_width, canvas_height;

       XtVaGetValues (print_prev->shell, XmNwidth, &frame_width,
		                         XmNheight, &frame_height, NULL);
       XtVaGetValues (print_prev->page, XmNwidth, &canvas_width,
		                        XmNheight, &canvas_height, NULL);
       print_prev->extra_width = frame_width - canvas_width;
       print_prev->extra_height = frame_height - canvas_height;
     }

/*
 * Show the popup, then position the image.
 */
     position_image ();

     setactive ();
}

/*
 * Menu handler for `file_menu (Print One)'.
 */

/*
 * Menu handler for Print One
 */
void
PrintOneCallback (w, client_data, call_data)
    Widget     w;
    XtPointer  client_data;
    XtPointer  call_data;
{
    if (printer_exists () == TRUE)
      PrintButtonCallback (NULL, NULL, NULL);

}

/*
 * Menu handler for `file_menu (Print...)'.
 */
void
PrintCallback (w, client_data, call_data)
    Widget     w;
    XtPointer  client_data;
    XtPointer  call_data;
{
    Boolean    set_min = False;

    if (printer_exists () == True) {

      setbusy();

      if (!print) {
        print = PrintObjectsInitialize (base->top_level);
	set_min = True;
      }

      if (current_display == ps_display)
	set_ps_print_options ();
      else
	set_image_print_options ();

      if (print_prev != (PrintPrevObjects *) NULL)
	update_margins ();

      ShowDialog (print->dialog, print->showing);
      if (set_min == True) {
	Dimension  width, height;

	XtVaGetValues (print->shell, XmNwidth, &width,
		                     XmNheight, &height, NULL);
	XtVaSetValues (print->shell, XmNminWidth, width,
		                     XmNminHeight, height, NULL);
      }
      setactive ();
      XmProcessTraversal (print->copies_text, XmTRAVERSE_CURRENT);
    }
}


/*
 * Menu handler for `Palette...'.
 */
void
PaletteCallback (w, client_data, call_data)
    Widget     w;
    XtPointer  client_data;
    XtPointer  call_data;
{
    setbusy();

    if (!palette) {
      palette = PaletteObjectsInitialize (base->top_level);
      set_zoom_and_rotate ();
    }

    ShowDialog (palette->dialog, palette->showing);

    setactive();
}

/*
 * Menu handler for `view_menu (Image Info...)'.
 */

void
ImageInfoCallback (w, client_data, call_data)
    Widget     w;
    XtPointer  client_data;
    XtPointer  call_data;
{
    Boolean    set_min = False;

    setbusy ();

    if (!info) {
      info = InfoObjectsInitialize (base->top_level);
      set_min = True;
    }

    update_imageinfo (current_image);
    ShowDialog (info->dialog, info->showing);

    if (set_min == True) {
      Dimension  width, height;

      XtVaGetValues (info->shell, XmNwidth, &width,
		                  XmNheight, &height, NULL);
      XtVaSetValues (info->shell, XmNminWidth, width,
		                  XmNminHeight, height, NULL);
    }

    setactive ();
}

/*
 * Menu handler for `view_menu (Page View...)'.
 */
void
PgOverviewCallback (w, client_data, call_data)
    Widget     w;
    XtPointer  client_data;
    XtPointer  call_data;
{
    Boolean    set_extra = False;
    Dimension  form_height;
    Dimension  width1, width2, width3;
    int        left_offset;

    setbusy();

    if (!pg_overview) {
      pg_overview = PgOverviewObjectsInitialize (base->top_level,
						 current_image->pages);

      if (current_state->reversed == True)
	ReversePgOverviewPages (pg_overview, current_image->pages, TRUE);

      set_pageview_pages (current_state->current_page);
      set_extra = True;
    }
    else if (pg_overview->thumbnails_created == False) {
      pg_overview->buttons = (Widget *) PgOverviewPagesCreate (pg_overview,
						    current_image->pages);
      if (current_state->reversed == TRUE)
	ReversePgOverviewPages (pg_overview, current_image->pages, TRUE);

      set_pageview_pages (current_state->current_page);
    }

    pg_overview->thumbnails_created = True;
    ShowDialog (pg_overview->dialog, pg_overview->showing);

    if (set_extra == True) {
      Dimension  shell_width, shell_height;
      Dimension  sw_width, sw_height;

      XtVaGetValues (pg_overview->shell, XmNwidth, &shell_width,
		                         XmNheight, &shell_height, NULL);
      XtVaGetValues (pg_overview->scrolledw, XmNwidth, &sw_width,
		                             XmNheight, &sw_height, NULL);
      pg_overview->extra_width = (int)shell_width - sw_width;
      if (pg_overview->nrows > 4)
	pg_overview->extra_width -= base->sb_width;
      pg_overview->extra_height = shell_height - sw_height;
      XtVaGetValues (pg_overview->dialog,
		     XmNwidth, &pg_overview->dialog_width, NULL);
    }
/*
 * Center Go To Page widget.
 */
    XtVaGetValues (pg_overview->goto_label, XmNwidth, &width1, NULL);
    XtVaGetValues (pg_overview->goto_value, XmNwidth, &width2, NULL);
    XtVaGetValues (pg_overview->scrolledw, XmNwidth, &width3, NULL);
    if ((((int) width1 + (int) width2) / 2) >= (int) (width3 / 2))
      left_offset = 0;
    else
      left_offset = ((int) width3 / 2) - (((int) width1 + (int) width2) / 2);

    XtVaSetValues (pg_overview->goto_label, XmNleftOffset, left_offset, NULL);

/*
 * Set scrollbar values.
 */
    if (pg_overview->nrows > 4) {
      Dimension  button_height;
      XtVaGetValues (pg_overview->buttons[0], XmNheight, &button_height, NULL);
      XtVaGetValues (pg_overview->pix_form, XmNheight, &form_height, NULL);
      XtVaSetValues (pg_overview->vscroll, XmNmaximum, (form_height + 1),
		     XmNsliderSize, (int) (button_height * 4),
		     XmNincrement, (int) button_height,
		     XmNpageIncrement, (int) (button_height * 3),
		     NULL);
    }

    XmProcessTraversal (pg_overview->buttons[current_state->current_page-1],
                        XmTRAVERSE_CURRENT);

    setactive();

}

/*
 * Menu handler for `Properties...'.
 */
void
PropsCallback (w, client_data, call_data)
    Widget     w;
    XtPointer  client_data;
    XtPointer  call_data;
{
    Boolean    set_min = False;

    setbusy();

    if (!properties) {
      properties = PropertiesObjectsInitialize (base->top_level);
      set_min = True;
    }

    PropsSetValues (properties);
    ShowDialog (properties->dialog, properties->showing);

    if (set_min == True) {
      Dimension  width, height;

      XtVaGetValues (properties->shell, XmNwidth, &width,
		                  XmNheight, &height, NULL);
      XtVaSetValues (properties->shell, XmNminWidth, width,
		                  XmNminHeight, height, NULL);
    }

    XmProcessTraversal (properties->radio1, XmTRAVERSE_CURRENT);
    setactive();

}

/*
 * Menu handler for `Page Viewing Controls...'
 */
void
PgControlsCallback (w, client_data, call_data)
    Widget     w;
    XtPointer  client_data;
    XtPointer  call_data;
{
    Boolean    set_min = False;
    XmString   xmstring;

    setbusy();

    if (!pg_controls) {
      pg_controls = PgControlsObjectsInitialize (base->top_level);
      set_min = True;
    }
/*
 * Reset degrees back to 0 if not showing.
 */
    if (pg_controls->showing == False) {
      xmstring = XmStringCreateLocalized (catgets (prog->catd, 1, 438, "0"));
      XtVaSetValues (XmOptionButtonGadget(pg_controls->degrees_menu), XmNlabelString, xmstring, NULL);
      XmStringFree (xmstring);
    }

    ShowDialog (pg_controls->dialog, pg_controls->showing);
    if (set_min == True) {
      Dimension  width, height;

      XtVaGetValues (pg_controls->shell, XmNwidth, &width,
		                  XmNheight, &height, NULL);
      XtVaSetValues (pg_controls->shell, XmNminWidth, width,
		                  XmNminHeight, height, NULL);
    }

    XmProcessTraversal (pg_controls->radio1, XmTRAVERSE_CURRENT);
    setactive();
}

/*
 * Menu handler for Exit...
 */
void
ExitCallback (w, client_data, call_data)
     Widget     w;
     XtPointer  client_data;
     XtPointer  call_data;
{
  Boolean    status;

  /* (DKenny - 27 May, 1997)
   * Do things before calling TTFailMessage, as it is possible after calling
   * this that we may not exit if another TT open request comes in.
   *
   * Close the PS_files, as if someone opens two files in succession, and
   * say the first file had 23 pages, but the second had only one, sdtimage
   * reports, when viewing the second PS file, that there are 23 pages when
   * there is only one, and naturally any attempt to go to the next page
   * causes a coredump.
   */
  if ( current_image && current_image->type_info &&
       ( current_image->type_info->type == POSTSCRIPT ||
         current_image->type_info->type == EPSF )
     )
      close_ps(TRUE,TRUE);

  /* (DKenny - 27 May, 1997)
   * Similarly for snap_dialog, need to check if a save is necessary now,
   * and not after TTFailMessage.
   */

  /*
   * ShutdownImagetool is called when
   * exit() is called because atexit() is set.
   */

  if (dtb_snap_dlog_dialog.snap_needs_save)
    {
      Boolean cont = SaveUnsavedSnapshot();
      if (cont == False)
        return;
    }

  /*
   * Imagetool is going away, tell the app who launched us if
   * started by another app.
   *
   */
  status = TTFailMessage ();
  if (status == False)   /* Don't quit, got another display message */
    return;

  exit (0);
}
