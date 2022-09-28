#ifndef lint
static char sccsid[] = "@(#)open.c 1.29 97/04/24";
#endif

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <Xm/FileSB.h>

#include "display.h"
#include "help.h"
#include "image.h"
#include "imagetool.h"
#include "misc.h"
#include "state.h"
#include "props.h"
#include "ui_imagetool.h"
#include "snap_dialog.h"

extern int  choice_rows;
extern int  first_open;

int 
check_open_file (fname)
     char *fname;
{
    struct stat file_info;
    char error[MAXPATHLEN];

    if (strcmp (fname, "") == 0 || AllBlanks (basename(fname))) {
      AlertPrompt (base->top_level,
		   DIALOG_TYPE,  XmDIALOG_ERROR,
                   DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		   DIALOG_TITLE, catgets (prog->catd, 1, 398, "Image Viewer - Open"),
		   DIALOG_TEXT, catgets (prog->catd, 1, 107,
					 "Please enter a file name."),
		   BUTTON_IDENT, 0, catgets (prog->catd, 1, 108, "Continue"),
		   HELP_IDENT, HELP_OPEN_ITEM,
		   NULL);
      return -2;		  
    }   
    else if (stat (fname, &file_info) == 0) {
	if (S_ISDIR (file_info.st_mode)) {
	    sprintf (error, catgets (prog->catd, 1, 109, 
		     "%s is a folder.\nUse File->Open to choose a single file."), fname);
	    AlertPrompt (base->top_level,
		   DIALOG_TYPE,  XmDIALOG_ERROR,
                   DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		   DIALOG_TITLE, catgets (prog->catd, 1, 399, "Image Viewer - Open"),
		   DIALOG_TEXT, error,
		   BUTTON_IDENT, 0, catgets (prog->catd, 1, 110, "Continue"),
		   HELP_IDENT, HELP_OPEN_ITEM,
		   NULL);
	    free (prog->directory);
	    prog->directory = malloc (strlen (fname) + 1);
	    strcpy (prog->directory, fname);
	    return (-2);
	}
	if (!S_ISREG (file_info.st_mode)) {
	    sprintf (error, catgets (prog->catd, 1, 111, 
				     "%s is not a regular file."), fname);
	    AlertPrompt (base->top_level,
		   DIALOG_TYPE,  XmDIALOG_ERROR,
                   DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		   DIALOG_TITLE, catgets (prog->catd, 1, 400, "Image Viewer - Open"),
		   DIALOG_TEXT, error,
		   BUTTON_IDENT, 0, catgets (prog->catd, 1, 112, "Continue"),
		   HELP_IDENT, HELP_OPEN_ITEM,
		   NULL);
	    return (-2);
	}

	/* Fix for Bug# 1223593. Added the 'access' call because original
	 * ignored secondary groups, etc. But, left in the original checks for 
	 * uid and gid because 'access' checks the REAL u/gid but prog->u/gid 
	 * is the EFFECTIVE u/gid
	 */
	if ( !( (file_info.st_uid == prog->uid && file_info.st_mode & S_IRUSR)
	       || (file_info.st_gid == prog->gid && file_info.st_mode & S_IRGRP)
	       || (access(fname, R_OK ) == 0)
	       )
	    ) {
	    sprintf (error, catgets (prog->catd, 1, 113,
			    "You do not have read permission for %s."), fname);
	    AlertPrompt (base->top_level,
		   DIALOG_TYPE,  XmDIALOG_ERROR,
                   DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		   DIALOG_TITLE, catgets (prog->catd, 1, 401, "Image Viewer - Open"),
		   DIALOG_TEXT, error,
		   BUTTON_IDENT, 0, catgets (prog->catd, 1, 114, "Continue"),
		   HELP_IDENT, HELP_OPEN_ITEM,
		   NULL);
	    return (-2);
	}

	if (file_info.st_size == 0) {
	    sprintf (error, catgets (prog->catd, 1, 115,
				     "%s is an empty file."), fname);
	    AlertPrompt (base->top_level,
		   DIALOG_TYPE,  XmDIALOG_ERROR,
                   DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		   DIALOG_TITLE, catgets (prog->catd, 1, 402, "Image Viewer - Open"),
		   DIALOG_TEXT, error,
		   BUTTON_IDENT, 0, catgets (prog->catd, 1, 116, "Continue"),
		   HELP_IDENT, HELP_OPEN_ITEM,
		   NULL);
	    return (-2);
	}
    }
    else {
      sprintf (error, catgets (prog->catd, 1, 117, "%s does not exist."), fname);
      AlertPrompt (base->top_level,
		   DIALOG_TYPE,  XmDIALOG_ERROR,
                   DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		   DIALOG_TITLE, catgets (prog->catd, 1, 403, "Image Viewer - Open"),
		   DIALOG_TEXT, error,
		   BUTTON_IDENT, 0, catgets (prog->catd, 1, 118, "Continue"),
		   HELP_IDENT, HELP_OPEN_ITEM,
		   NULL);
      return (-1);	       
    }

    /*
     * Return the number of bytes in the file.
     */

    return (file_info.st_size);
}

void
OpenCancelCallback (w, client_data, call_data)
     Widget     w;
     XtPointer  client_data;
     XtPointer  call_data;
{
     XtPopdown (XtParent (open_dialog));
}

void
OpenOKCallback (w, client_data, cbs)
     Widget     w;
     XtPointer  client_data;
     XmFileSelectionBoxCallbackStruct *cbs;
{
     char  filename[MAXPATHLEN];
     int   status;

     ConvertCompoundToChar (cbs->value, &filename);

     status = open_newfile (filename, filename, -1, (char *) NULL, 0);
     /*
      * Dismiss Open popup if successful.
      */
     if (status >= 0)
       {
         XtPopdown (XtParent (open_dialog));
         dtb_snap_dlog_dialog.snap_needs_save = False;
       }

     /*
      * If started via tooltalk, but just loaded in a new file,
      * break tooltalk connection.
      */
     if ((prog->tt_started == TRUE) && (status == 0)) 
       TTBreak ();
}

int
OpenAtStart (path, type)
     char *path;
     int   type;
{
     int   status;

     status = open_newfile (path, path, type, (char *)NULL, 0);

     return (status);

}

void
OpenAsCancelCallback (w, client_data, call_data)
     Widget     w;
     XtPointer  client_data;
     XtPointer  call_data;
{
     XtPopdown (XtParent (openas->dialog));
}

int
GetFileFormat (list, row)
     Widget list;
     int    row;
{
     int            i = 0;
     XmStringTable  strs;
     char           selected[MAXNAMELEN];

     XtVaGetValues (list, XmNitems, &strs, NULL);
     ConvertCompoundToChar (strs[row], &selected);
     
     for (i = 0; i < ntypes; i++) {
       if (strcmp (selected, all_types[i].popup_list_name) == 0)
	 break;
     }

     return i;
}

void
OpenAsOKCallback (w, client_data, cbs)
     Widget     w;
     XtPointer  client_data;
     XmFileSelectionBoxCallbackStruct *cbs;
{
     char  filename[MAXPATHLEN];
     int   status, row, open_type;

/*
 * Get the file format.
 */
     row = GetListSelection (openas->format_list);
     open_type = GetFileFormat (openas->format_list, row);
/*
 * Get the file name.
 */
     ConvertCompoundToChar (cbs->value, &filename);

     status = open_newfile (filename, filename, open_type, (char *) NULL, 0);
/*
 * If started via tooltalk, but just loaded in a new file,
 * break tooltalk connection.
 */
    if ((prog->tt_started == TRUE) && (status == 0)) 
      TTBreak ();

     /*
      * Dismiss Open As popup if successful.
      */
     if (status >= 0)
       {
         XtPopdown (XtParent (openas->dialog));
         dtb_snap_dlog_dialog.snap_needs_save = False;
       }

}

void
SetCurrentDoc (dialog, fname)
   Widget        dialog;
   char         *fname;
{
   Widget        list, text;
   XmString      str;
   int           row;

/*
 * Set the filename in the text field.
 */
   text = XmFileSelectionBoxGetChild (dialog, XmDIALOG_TEXT);
   XmTextSetString (text, fname);
/*
 * Select the filename in the file scrolled list.
 */
   str = XmStringCreateLocalized (fname);
   list = XmFileSelectionBoxGetChild (dialog, XmDIALOG_LIST);
   XmListSelectItem (list, str, False);
/*
 * Set the selected item to show at the top of the list.
 */
   row = GetListSelection (list);
   if (row != -1)
     XmListSetPos (list, ++row);

}

void
reread_file_choosers (op)
     int   op;
{
     char      save_dir[MAXPATHLEN], open_dir[MAXPATHLEN];
     Widget    dialog;
     XmString  dir;

     if (op == SAVE)
       dialog = save_dialog;
     else
       dialog = saveas->dialog;
/*
 * Get the current directory of the Save/SaveAs dialog.
 */
     XtVaGetValues (dialog, XmNdirectory, &dir, NULL);
     ConvertCompoundToChar (dir, &save_dir);

/*
 * If the open dialog is in the same directory as
 * the file just saved, then force an update and
 * set the file just saved to be selected.
 */
     if (save_dir[0] != '\0' && open_dialog) {
       XtVaGetValues (open_dialog, XmNdirectory, &dir, NULL);
       ConvertCompoundToChar (dir, &open_dir);
       if (strcmp (save_dir, open_dir) == 0) {
	 XtVaSetValues (open_dialog, XtVaTypedArg, XmNdirectory, XtRString,
			open_dir, strlen (open_dir) + 1, NULL);
	 if (current_image)
	   SetCurrentDoc (open_dialog, basename (current_image->file));
       }
     }
     
     if (save_dir[0] != '\0'  && openas) {
       XtVaGetValues (openas->dialog, XmNdirectory, &dir, NULL);
       ConvertCompoundToChar (dir, &open_dir);
       if (strcmp (save_dir, open_dir) == 0) {
	 XtVaSetValues (openas->dialog, XtVaTypedArg, XmNdirectory, XtRString,
			open_dir, strlen (open_dir) + 1, NULL);
	 if (current_image)
	   SetCurrentDoc (openas->dialog, basename (current_image->file));
       }
     }

}

/*
 * Things to check just after loading in a new image.
 * prev_null_image = true is the prev image was null.
 */
void
check_popups (prev_null_image)
   int  prev_null_image;
{
   int        ps_file;
   Widget     text;
   Window     new_wins[2];

   if (current_display == ps_display)
     ps_file = TRUE;
   else
     ps_file = FALSE;
  
/*
 * This is a kludge if prev_image is NULL and
 * just loaded ps_file because we do not get a
 * WIN_VIS event in this case.
 */
#ifdef NEVER
no longer needed because caught in MainWEventHandler now.
/* why ps file only?
   if (prev_null_image && ps_file && current_props->show_palette) { 

   if (prev_null_image && current_props->show_palette) {
     XSync (current_display->xdisplay, 0);
     if (prog->frame_mapped == True)
       PaletteCallback (NULL, NULL, NULL);
     setbusy(); /* Undone in PaletteCallback() */
     first_open = FALSE; 
   }
#endif

/*
 * No matter what the type, display image info.
 */
   if (info && info->showing == TRUE)
       update_imageinfo (current_image);
/*
 * If print preview popup is showing, update the display.
 */
    if (print_prev && print_prev->showing == True) 
	position_image();

/*
 * If Save Selection As showing, unshow it because
 * nothing is selected now.
 */
    if (saveas != NULL && saveas->showing == TRUE) {
      char  *title;
      XtVaGetValues (saveas->shell, XmNtitle, &title, NULL);
      if (strcmp (title, catgets (prog->catd, 1, 119,
				  "Image Viewer - Save Selection As")) == 0)
        DismissDialog (saveas->dialog);
    }

/*
 * Just loaded image file...
 *   Turn off scale
 *   Unshow PS Options.
 *   Unshow Page Overview
 *   If Save Page As Image showing, change to Save As
 * 
 */
   if (ps_file == FALSE) {
     XmString   xmstring;
     char       value[80];

     XtVaSetValues (base->goto_scale, XmNsensitive, False, 
		                      XmNvalue, 1, NULL);
     sprintf (value, "%-d", 1);
     xmstring = XmStringCreateLocalized (value);
     XtVaSetValues (base->goto_pgno, XmNsensitive, False, 
		                     XmNlabelString, xmstring, NULL);
     XmStringFree (xmstring);

     if (pg_controls && pg_controls->showing == TRUE)
       DismissDialog (pg_controls->dialog);

     if (pg_overview && pg_overview->showing)
       DismissDialog (pg_overview->dialog);

     if (saveas && saveas->showing == TRUE) {
       char *title;
       XtVaGetValues (saveas->shell, XmNtitle, &title, NULL);
       if (strcmp (title, catgets (prog->catd, 1, 120, 
				   "Image Viewer - Save Page As Image")) == 0)
	 XtVaSetValues (saveas->shell, XmNtitle, 
			catgets (prog->catd, 1, 404, "Image Viewer - Save As"),
			NULL);
     }

   }

/*
 * Just loaded PS file...
 *   Turn on scale.
 *   If Save As showing, change to Save Page As Image
 *   If PS Options showing, update the page overview if npages > 1.
 */
   else {

     if (saveas && saveas->showing == TRUE) {
       char *title; 
       XtVaGetValues (saveas->shell, XmNtitle, &title, NULL);
       if (strcmp (title, catgets (prog->catd, 1, 121,
				   "Image Viewer - Save As")) == 0)
	 XtVaSetValues (saveas->shell, 
			XmNtitle, catgets (prog->catd, 1, 122,
					   "Image Viewer - Save Page As Image"),
			NULL);
     }

     if (pg_overview && pg_overview->showing == TRUE  &&
         current_image->pages <= 1)
       DismissDialog (pg_overview->dialog);
     XmProcessTraversal (ps_display->new_canvas, XmTRAVERSE_CURRENT);

   }

/*
 * If Save/Save As is created, then
 * update the document name with the newly
 * loaded document wheather showing or not.
 */
   if (save_dialog) {
     text = XmFileSelectionBoxGetChild (save_dialog, XmDIALOG_TEXT);
     XmTextSetString (text, basename (current_image->file));
   }
   if (saveas && saveas->dialog) {
/*
Don't set filename for saveas until we use templates.
     text = XmFileSelectionBoxGetChild (saveas->dialog, XmDIALOG_TEXT);
     XmTextSetString (text, basename (current_image->file));
*/
#ifdef LATER
     char        newname [MAXPATHLEN];
     CompTypes   compression;
     char       *comp_str;
     int         length, null_set = FALSE;

     
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
#endif
   }

/*
 * Reset the zoom and angle choices
 */
    set_zoom_and_rotate ();

/*
 * Set WM_COLORMAP_WINDOWS property on base canvas.
 */
    new_wins[0] = current_display->win;
    new_wins[1] = XtWindow (base->top_level);
    XSetWMColormapWindows (current_display->xdisplay, 
			   XtWindow (base->top_level),
		           new_wins, 2);

    if (ps_file == TRUE && 
	pg_overview && pg_overview->showing == TRUE &&
        current_image->pages <= 1)
      DismissDialog (pg_overview->dialog);
/*
 * If file was loaded from stdin, rm temp file.
 */
     if (prog->standardinfile != NULL) {
       unlink (prog->standardinfile);
       free (prog->standardinfile);
       prog->standardinfile = NULL;
     }
}

/*
 * Insert new_str in alphabetical order into list.
 */
int
InsertInList (list, nitems, new_str)
     Widget  list;
     int     nitems;
     char   *new_str;
{
     int            i;
     XmStringTable  strs;
     char           str[MAXNAMELEN];
     
     XtVaGetValues (list, XmNitems, &strs, NULL);
     
     for (i = 0; i < nitems; i++) {
       ConvertCompoundToChar (strs[i], &str);
       if (str[0] != '\0' && strcoll (str, new_str) > 0)
	 break;
     }

     return i+1;
}

void
set_openas_list (list)
     Widget  list;
{
     int        i, row;
     XmString   xmstring;
/*
 * Set the openas list, to the list in all_types, adding client
 * data, so we can easily determine the choice when the user selects
 * something.
 */

     for (i = 0; i < ntypes; i++) {
       xmstring = XmStringCreateLocalized (all_types[i].popup_list_name);
       row = InsertInList (list, i, all_types[i].popup_list_name);
       XmListAddItemUnselected (list, xmstring, row);
       XmStringFree (xmstring);
     }
/*
 * Select SunRaster.
 * Set this selection to be at the top of the list.
 */
     xmstring = XmStringCreateLocalized (all_types[0].popup_list_name);
     XmListSelectItem (list, xmstring, False);
     XmListSetItem (list, xmstring);
     XmStringFree (xmstring);
}

/*
 * Generic open for open and openas...
 */

int
open_newfile (path, realpath, open_type, data, size)
char		*path;
char		*realpath;
int		 open_type;
char		*data;
int		 size;
{
    char	*type = NULL;
    int          retry = FALSE;
    int		 file_size = size;
    TypeInfo	*file_type;
    TypeInfo	*save_file_type;
    int	         status;
    int          second_try = FALSE;
    ImageInfo	*new_image;
    StateInfo	*new_state;
    XilImage	 tmp_image;
    int		 compressed = FALSE;
    int          prev_null_image = FALSE;
    char         error[MAXPATHLEN];
    Boolean      manage_overview = False;
    DisplayInfo *prev_display;    
    ImageInfo   *prev_image;
   
    setbusy();
/*
 * Clear footer
 */
    SetFooter (base->left_footer, "");
/*
 * If Page Overview showing, umap the buttons.
 */
    if (pg_overview && pg_overview->showing == True) {
      XtUnmanageChild (pg_overview->scrolledw);
      XFlush (ps_display->xdisplay);
      manage_overview = True;  /* If error, manage these again */
    }
/*
 * Check if file really exists.
 */

/*
 * Determine if we can load in the file, if we have a file.
 */

    if (data == (char *) NULL) {
      if ((file_size = check_open_file (realpath)) < 0) {
	if (manage_overview == True)
	   XtManageChild (pg_overview->scrolledw);
	setactive ();
	return (-1);
      }
    }

RETRY:
/*
 * If open_type is -1, then we need to type the file.
 * Otherwise, the user used the Open As popup, and we already
 * know the type.
 */

    if (open_type != -1) {
       int length;
/*
 * File type was passed in...
 * First check if *.Z file.
 */
       file_type = &all_types [open_type];
       if ((length = strlen (realpath)) > 2) {
         if ((realpath[length - 1] == 'Z') && (realpath[length - 2] == '.'))
           compressed = TRUE;
       }
     }
    else {
/*
 * We don't know the type yet..
 * if ce_okay == -1, need to init types database.
 * if ce_okay == FALSE, use open as.
 * else type the file.
 */
      if (prog->ce_okay == -1) {
	DtDbLoad ();
	prog->ce_okay = True;
      }

      type = (char *) type_file (realpath, &compressed, data);

/*
 * If file_type is unknown, then we don't know the type, so ask the user
 * to use the open as pop up instead.
 */

      if (strcmp (type, catgets (prog->catd, 1, 123, "DATA")) == 0) {
	AlertPrompt (base->top_level,
		     DIALOG_TYPE,  XmDIALOG_ERROR,
                     DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		     DIALOG_TITLE, "Image Viewer - Open As",
		     DIALOG_TEXT, catgets (prog->catd, 1, 124,
 	               "File format not supported.\nUse the Open As option to choose a file format."),
		     BUTTON_IDENT, 0, catgets (prog->catd, 1, 125, "Continue"),
		     HELP_IDENT, HELP_OPENAS_ITEM,
		     NULL);
	if (manage_overview == True)
	   XtManageChild (pg_overview->scrolledw);
	setactive ();
	return  (-1);
      }
	
/*
 * Check if we even support loading this type of file.
 */
      file_type = str_to_ftype (type);
      if (file_type == NULL) {
	AlertPrompt (base->top_level,
		     DIALOG_TYPE,  XmDIALOG_ERROR,
                     DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		     DIALOG_TITLE, "Image Viewer - Open",
		     DIALOG_TEXT, catgets (prog->catd, 1, 126,
		       "File not opened:  Unsupported format."),
		     BUTTON_IDENT, 0, catgets (prog->catd, 1, 127, "Continue"),
		     HELP_IDENT, HELP_OPEN_ITEM,
		     NULL);
	if (manage_overview == True)
	   XtManageChild (pg_overview->scrolledw);
	setactive ();
	return  (-1);
      }
    }

/*
 * Init image objects if not already done so and not a PS file.
 */
    if ((file_type->type != POSTSCRIPT) && (file_type->type != EPSF) && 
	(prog->xil_opened == FALSE)) {
	 BaseImageCanvasObjectsCreate (base);
	 if (prog->xil_opened == False) {
	   setactive ();
	   return (-1);
	 }
    }

/*
 * If we were reading a PostScript file, we can stop now...
 */

    if ((current_image != (ImageInfo *) NULL) && 
        (current_display != (DisplayInfo *) NULL) &&
	(current_display == ps_display))
       close_ps (TRUE, FALSE);

/*
 * Check for unknown file type here... Here's what we should be doing...
 * If the file isn't one of the `known' ones (postscript, gif, raster, tiff),
 * then we should check the classing engine for a IMAGE_LOAD value 
 * and attempt to load the file using that.. 
 */

/*
    if (file_type == (TypeInfo *) NULL) {
      
      create a new file_type structure (do a realloc).. add to list.

      add it to saveas list...
*/
    
/*
 * At this point, it looks like we probably will read in the file,
 * so create a new ImageInfo structure, and pass to appropriate
 * load function.
 */

    new_image = init_image (path, realpath, file_size, file_type, data);
    if (compressed == TRUE)
       new_image->compression = UNIX;

/*
 * We need a new state.. mainly for postscript files. Create one here.
 */

    new_state = init_state (( *new_image->type_info).type);

    status = (*new_image->type_info->load_func) (new_image);

/*
 * If status isn't OK, then we hit a problem, so, leave things the
 * the way they are and return.
 */
 
    if (status != 0) {

/*
 * open_type was given to us (Open As) and first time retrying, but failed.
 * Type ourself and retry if our open_type is different than given open_type.
 */
       if ((status == -1) && (open_type != -1) && (retry == FALSE)) {	 
	 save_file_type = file_type;

	 if (prog->ce_okay == TRUE) {

	   /* Type again */
	   type = (char *) type_file (realpath, &compressed, data);
	   retry = TRUE;
	   if (strcmp (type, catgets (prog->catd, 1, 128, "DATA")) == 0) 
	     retry = FALSE;
	   
	   /* Check if unsupported format */
	   if (retry) {
  	     file_type = str_to_ftype (type);
	     if (file_type == NULL)
	       retry = FALSE;
           }

	   /* Check if we already tried this format */
	   if ((retry) && (open_type == (int) file_type->type))
	     retry = FALSE;

           /* Set the open_type to what we think it is */
	   if (retry) {
	     open_type = (int) file_type->type;
	   }

	 }
	 else 
	   retry = FALSE;

	 if (retry == FALSE) {
	   sprintf (error, catgets (prog->catd, 1, 129, "Error opening file as a %s."), save_file_type->popup_list_name);
	   AlertPrompt (base->top_level,
		     DIALOG_TYPE,  XmDIALOG_ERROR,
                     DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		     DIALOG_TITLE, catgets (prog->catd, 1, 405, "Image Viewer - Open As"),
		     DIALOG_TEXT, error,
		     BUTTON_IDENT, 0, catgets (prog->catd, 1, 130, "Continue"),
		     HELP_IDENT, HELP_OPENAS_ITEM,
		     NULL);
	 }
       }
	 
/*
 * If status is -1 then no notice appeared, yet so put one up now.
 */
       else if (status == -1) {
	 sprintf (error, catgets (prog->catd, 1, 131, "Error opening %s."), realpath);
	 AlertPrompt (base->top_level,
		      DIALOG_TYPE,  XmDIALOG_ERROR,
                      DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		      DIALOG_TITLE, catgets (prog->catd, 1, 406, "Image Viewer - Open"),
		      DIALOG_TEXT, error,
		      BUTTON_IDENT, 0, catgets (prog->catd, 1, 132, "Continue"),
		      HELP_IDENT, HELP_OPEN_ITEM,
		      NULL);
	 if (second_try)
	   retry = FALSE;
       }

       destroy_image (new_image);
       free (new_state);

       if (retry) {
	 second_try = TRUE;
         goto RETRY;
       }

/*
 * If we had a ps file open, go back to it. Note that if we
 * return -1 from open_psfile, we're screwed!
 */
       if ((current_image == NULL) && (ps_display->new_canvas != NULL))
          current_display = ps_display;

       if ((current_image != (ImageInfo *) NULL) && 
           (current_display != (DisplayInfo *) NULL) &&
	   (current_display == ps_display)) 
	  restart_ps ();
       else if (current_display == image_display)
	  XRaiseWindow (current_display->xdisplay, XtWindow (base->canvas));
/*
       if (status == -1) {
	 sprintf (error, catgets (prog->catd, 1, -1, "Error opening %s."), realpath);
	 AlertPrompt (base->top_level,
		      DIALOG_TYPE,  XmDIALOG_ERROR,
                      DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		      DIALOG_TITLE, catgets (prog->catd, 1, -1, "Image Viewer - Open"),
		      DIALOG_TEXT, error,
		      BUTTON_IDENT, 0, catgets (prog->catd, 1, -1, "Continue"),
		      HELP_IDENT, HELP_OPEN_ITEM,
		      NULL);
       }
*/
       
       if (manage_overview == True)
	 XtManageChild (pg_overview->scrolledw);
       setactive ();
       return (-2);
       }

/*
 * Display message if we opened the file as something else.
 */
    if ((status == 0) && (retry == TRUE) && (file_type != NULL)) {
      sprintf (error, catgets (prog->catd, 1, 133, "Error opening file as a %s.\nOpening file as a %s."), save_file_type->popup_list_name, file_type->popup_list_name);
      AlertPrompt (base->top_level,
		   DIALOG_TYPE,  XmDIALOG_ERROR,
                   DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		   DIALOG_TITLE, catgets (prog->catd, 1, 407, "Image Viewer - Open As"),
		   DIALOG_TEXT, error,
		   BUTTON_IDENT, 0, catgets (prog->catd, 1, 134, "Continue"),
		   HELP_IDENT, HELP_OPENAS_ITEM,
		   NULL);
    }

    set_tool_options (current_image, new_image);
/*
 * Set the new image, but don't destroy old one until 
 * assign_display_func if successful.
 */
    if (current_image == NULL)
      prev_null_image = True;
    prev_image = current_image;
    current_image = new_image;

    prev_display = current_display;
    set_current_display ();
/*
 * Make image available for XIL processing.
 * Dither image for current depth.
 */
    if (((new_image->type_info)->type != POSTSCRIPT) &&
	((new_image->type_info)->type != EPSF)) {
	extern int assign_display_func();

	if (assign_display_func (new_image, image_display->depth) == -1) {
	  sprintf (error, catgets (prog->catd, 1, 131, "Error opening %s."), realpath);
	  AlertPrompt (base->top_level,
		      DIALOG_TYPE,  XmDIALOG_ERROR,
                      DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		      DIALOG_TITLE, catgets (prog->catd, 1, 406, "Image Viewer - Open"),
		      DIALOG_TEXT, error,
		      BUTTON_IDENT, 0, catgets (prog->catd, 1, 132, "Continue"),
		      HELP_IDENT, HELP_OPEN_ITEM,
		      NULL);
          if (manage_overview == True)
	    XtManageChild (pg_overview->scrolledw);
   	  current_display = prev_display;
 	  current_image = prev_image;
          if ((current_image != (ImageInfo *) NULL) && 
              (current_display != (DisplayInfo *) NULL) &&
	      (current_display == ps_display)) 
	    restart_ps ();
          set_tool_options (NULL, prev_image);
    	  setactive ();
	  return (-1);
        }

        if (prev_null_image == False)
          destroy_image (prev_image);

        if (current_state != (StateInfo *) NULL) 
          free (current_state);
        current_state = new_state;

	(current_image->revert_func)();

	XmScrolledWindowSetAreas (base->scrolledw, base->hscroll,
				  base->vscroll, base->canvas);
    } 
    else {
        if (prev_null_image == False)
          destroy_image (prev_image);

        if (current_state != (StateInfo *) NULL) 
           free (current_state);
        current_state = new_state;

	current_state->reversed = initial_reverse (TRUE);       
	current_state->using_dsc = initial_dsc ();
	current_state->timeout_hit = initial_timeout ();
	current_state->zoom = prog->def_ps_zoom / 100.0;
	XmScrolledWindowSetAreas (base->scrolledw, base->hscroll,
				  base->vscroll, base->ps_canvas);
    }
    check_canvas (current_image->width, current_image->height, current_display);

    /*
     * Now try to rotate the image?
     * (only if landscape!)
     */
    if(current_image->orient == LANDSCAPE &&
       (current_image->type_info)->type == POSTSCRIPT)
      {
	/*	reset_select();
		reset_pan();

		setbusy();
	 */
	/* set_undo_option (ON); */

/*
 * Compute the angle of rotation.
 * value == 0  Rotate Right
 * value == 1  Rotate Left
 */
/* 	(current_state->undo).op = ROTATE_L; */
	current_state->angle += 90;
	current_state->rotate_amt = -(90);

	(*current_image->zoom_func) (current_image->revert_image);
	(*current_image->turnover_func) (current_image->view_image);
	(*current_image->rotate_func) (current_image->view_image);

	resize_canvas ();
	/*	current_state->angle -= 90;
		current_state->rotate_amt = 0; */
      }
    /*
     * END ROTATE
     */

/*
 * Fit frame to image then re-position palette if showing.
 */
    fit_frame_to_image (current_image);
    resize_canvas ();
    CanvasRepaintCallback (NULL, NULL, NULL);
/*
 * Check which popups should be showing with
 * this newly loaded image.
 */
    check_popups (prev_null_image);

    setactive();

    return (0);
}


