#ifndef lint
static char sccsid[] = "@(#)save.c 1.26 97/04/24";
#endif

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

/*
 * save.c - Functions used for saving...
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include "display.h"
#include "help.h"
#include "image.h"
#include "imagetool.h"
#include "misc.h"
#include "state.h"
#include "ui_imagetool.h"
#include "snap_dialog.h"

void
SaveCancelCallback (w, client_data, cbs)
     Widget     w;
     XtPointer  client_data;
     XmFileSelectionBoxCallbackStruct *cbs;
{
     XtPopdown (XtParent (save_dialog));
}

void
SaveOKCallback (w, client_data, cbs)
     Widget     w;
     XtPointer  client_data;
     XmFileSelectionBoxCallbackStruct *cbs;
{
     char  filename[MAXPATHLEN];
     int   status;

     ConvertCompoundToChar (cbs->value, &filename);

     status = save_newfile (filename, SAVE);

     /*
      * Dismiss Save popup if successful.
      */
     if (status >= 0)
       XtPopdown (XtParent (save_dialog));
}

void
SaveAsCancelCallback (w, client_data, call_data)
     Widget     w;
     XtPointer  client_data;
     XtPointer  call_data;
{
  saveas->button_selected = SAVE_CANCEL_BUTTON;
  XtPopdown (XtParent (saveas->dialog));
}

void
SaveAsOKCallback (w, client_data, cbs)
     Widget     w;
     XtPointer  client_data;
     XmFileSelectionBoxCallbackStruct *cbs;
{
     char  filename[MAXPATHLEN], *title;
     int   status;

     ConvertCompoundToChar (cbs->value, &filename);

     XtVaGetValues (saveas->shell, XmNtitle, &title, NULL);
     if (strcmp (title, catgets (prog->catd, 1, 288,
				 "Image Viewer - Save Page As Image")) == 0)
	 status = save_newfile (filename, SAVEPAGEASIMAGE);
     else if (strcmp (title, catgets (prog->catd, 1, 289,
				 "Image Viewer - Save Selection As")) == 0)
	 status = save_newfile (filename, SAVESELECTIONAS);
     else
	 status = save_newfile (filename, SAVEAS);
     /*
      * Dismiss Save As popup if successful.
      */
     if (status >= 0)
       {
         XtPopdown (XtParent (saveas->dialog));

         saveas->button_selected = SAVE_OK_BUTTON;
         dtb_snap_dlog_dialog.snap_needs_save = False;
       }
     else
       saveas->button_selected = SAVE_CANCEL_BUTTON;
}


int
check_save_file (fname, save_type)
char  	   *fname;
int         save_type;
{
    struct stat	file_info;
    FILE       *fp = NULL;
    int		status;
    char        error[MAXPATHLEN];


    if (strcmp (fname, "") == 0 || AllBlanks (basename(fname))) {
      AlertPrompt (base->top_level,
		   DIALOG_TYPE, XmDIALOG_ERROR,
                   DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		   DIALOG_TITLE, catgets (prog->catd, 1, 417,
					  "Image Viewer - Save"),
		   DIALOG_TEXT, catgets (prog->catd, 1, 290,
					 "Please enter a file name."),
		   BUTTON_IDENT, 0, catgets (prog->catd, 1, 291, "Continue"),
		   HELP_IDENT, HELP_SAVE_ITEM,
		   NULL);
      return (-2);
    }
    else if (stat (fname, &file_info) == 0) {
      if (S_ISDIR (file_info.st_mode)) {
	sprintf (error, catgets (prog->catd, 1, 292, "%s is a folder.\nPlease enter a file name."), fname);
	AlertPrompt (base->top_level,
		   DIALOG_TYPE,  XmDIALOG_ERROR,
                   DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		   DIALOG_TITLE, catgets (prog->catd, 1, 418,
					  "Image Viewer - Save"),
		   DIALOG_TEXT, error,
		   BUTTON_IDENT, 0, catgets (prog->catd, 1, 293, "Continue"),
		   HELP_IDENT, HELP_SAVE_ITEM,
		   NULL);
	return (-2);
      }
    }

/*
 * First try opening it for reading to check if file exists.
 */
    if ((fp = fopen (fname, "r")) == NULL) {
      /*
       * Couldn't open if for reading, but file can be
       * 044, 004, 000 permissions so do a stat.
       */
      if (access (fname, F_OK) == 0)
	  return (0);
      /*
       * File doesn't exist so open it for writing to check if
       * fname contains directory that doesn't exist. (/tmp/xxx/xxx/xx)
       */
      if ((fp = fopen (fname, "w")) == NULL) {
        sprintf (error, catgets (prog->catd, 1, 294,
				 "Cannot save to %s."), fname);
	AlertPrompt (base->top_level,
		   DIALOG_TYPE,  XmDIALOG_WARNING,
                   DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		   DIALOG_TITLE, catgets (prog->catd, 1, 419, "Image Viewer - Save"),
		   DIALOG_TEXT, error,
		   BUTTON_IDENT, 0, catgets (prog->catd, 1, 295, "Continue"),
		   HELP_IDENT, HELP_SAVE_ITEM,
		   NULL);
        return (-1);
      }
      else
	unlink (fname);
    }

    if (fp)
      fclose (fp);

    return (0);
}

int
colors_to_depth (colors)
    char *colors;
{
    int depth = 8;

/* I18N_STRING
   B&W = short for black and white
 */
    if (strcmp (colors, catgets (prog->catd, 1, 296, "B&W")) == 0)
      depth = 1;
    else if (strcmp (colors, catgets (prog->catd, 1, 297, "256")) == 0)
      depth = 8;
    else if (strcmp (colors, catgets (prog->catd, 1, 298, "Millions")) == 0)
      depth = 24;

   return depth;
}


CompTypes
string_to_compression (comp_str)
    char  *comp_str;
{
    CompTypes compression = NO_COMPRESS;

    if (strcmp (comp_str, catgets (prog->catd, 1, 299, "None")) == 0)
      compression = NO_COMPRESS;
    else if (strcmp (comp_str, catgets (prog->catd, 1, 300, "Unix")) == 0)
      compression = UNIX;
    else if (strcmp (comp_str, catgets (prog->catd, 1, 301, "Encoded")) == 0)
      compression = RUN_LENGTH;
    else if (strcmp (comp_str, catgets (prog->catd, 1, 302, "LZW")) == 0)

      compression = LZW;
    else if (strcmp (comp_str, catgets (prog->catd, 1, 303, "JPEG")) == 0)
      compression = JPEG;

    return compression;
}

/*
 * 1 - continue
 * 0 - don't continue.
 */
static int
confirm_data_loss (fname)
    char *fname;
{
    int status;

    status = AlertPrompt (base->top_level,
	       DIALOG_TYPE, XmDIALOG_QUESTION,
               DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
	       DIALOG_TITLE, catgets (prog->catd, 1, 420, "Image Viewer - Save"),
	       DIALOG_TEXT, catgets (prog->catd, 1, 304,
		 "Saving to the JFIF (JPEG) file format\nmay result in a loss of data.\nDo you want to continue?"),
	       BUTTON_IDENT, 0, catgets (prog->catd, 1, 305, "Yes"),
	       BUTTON_IDENT, 1, catgets (prog->catd, 1, 306, "No"),
	       HELP_IDENT, HELP_SAVE_ITEM,
	       NULL);

    if (status == 1)
      return 0;
    else {
      if (access (fname, F_OK)  == 0)
        unlink (fname);
    }
      return 1;
}

/*
 * 1 - continue
 * 0 - don't continue.
 */
static int
confirm_overwrite_file (fname, compression)
    char       *fname;
    CompTypes  compression;
{
    int status;
    int value = 1;
    int base_length = strlen (basename (fname));
    int length = strlen (fname);
    char msg[2048];

    /*
     * Check if file already exists.
     */
    if (access (fname, F_OK) == 0) {
      sprintf (msg, catgets (prog->catd, 1, 307, "The file \"%s\" already exists.\nDo you want to overwrite the existing file \"%s\"?"), basename (fname), basename (fname));
      status = AlertPrompt (base->top_level,
		   DIALOG_TYPE,  XmDIALOG_QUESTION,
                   DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		   DIALOG_TITLE, catgets (prog->catd, 1, 421, "Image Viewer - Save"),
		   DIALOG_TEXT, msg,
		   BUTTON_IDENT, 0, catgets (prog->catd, 1, 308, "Cancel"),
		   BUTTON_IDENT, 1, catgets (prog->catd, 1, 309, "Overwrite Existing File"),
		   HELP_IDENT, HELP_SAVE_ITEM,
		   NULL);
      if (status == 0)
	value = 0;
      else
	value = 1;
    }
    if (value == 0)
      return value;

    /*
     * If doing UNIX compression, check both cases.
     *  1.  If user already entered .Z extension, check if basename exists
     *        (e.g. file.ras.Z  and file.ras)
     *  2.  If user didn't enter .Z, check if .Z exists.
     *        (e.g., file.ras and file.ras.Z)
     */
    if (compression == UNIX) {

      if ((base_length > 2) &&
	  (fname[length - 1] == 'Z') && (fname[length - 2] == '.')) {
	fname[length - 2] = '\0';
	if (access (fname, F_OK) == 0) {
	  sprintf (msg, catgets (prog->catd, 1, 310, "The file \"%s\" already exists.\nSaving a file using UNIX compression will overwrite this file.\nDo you want to overwrite the existing file \"%s\"?"), basename (fname), basename (fname));
	  status = AlertPrompt (base->top_level,
		     DIALOG_TYPE,  XmDIALOG_QUESTION,
                     DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		     DIALOG_TITLE, catgets (prog->catd, 1, 422, "Image Viewer - Save"),
		     DIALOG_TEXT, msg,
		     BUTTON_IDENT, 0, catgets (prog->catd, 1, 311, "Cancel"),
		     BUTTON_IDENT, 1, catgets (prog->catd, 1, 312, "Overwrite Exising File"),
		     HELP_IDENT, HELP_SAVE_ITEM,
		     NULL);
	  if (status == 0)
	    value = 0;
	  else
	    value = 1;
	}
	fname[length - 2] = '.';
      }
      else {
	char cname[MAXPATHLEN];
	sprintf (cname, "%s.Z", fname);
	if (access (cname, F_OK) == 0) {
	  sprintf (msg, catgets (prog->catd, 1, 313, "The file \"%s\" already exists.\nSaving a file using UNIX compression will overwrite this file.\nDo you want to overwrite the existing file \"%s\"?"), basename (cname), basename (cname));
	  status = AlertPrompt (base->top_level,
		     DIALOG_TYPE,  XmDIALOG_QUESTION,
                     DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		     DIALOG_TITLE, catgets (prog->catd, 1, 423, "Image Viewer - Save"),
		     DIALOG_TEXT, msg,
		     BUTTON_IDENT, 0, catgets (prog->catd, 1, 314, "Cancel"),
		     BUTTON_IDENT, 1, catgets (prog->catd, 1, 315, "Overwrite Existing File"),
		     HELP_IDENT, HELP_SAVE_ITEM,
		     NULL);
	  if (status == 0)
	    value = 0;
	  else
	    value = 1;
	}
      }
    }

    return value;

}

int
save_entire_ps_file (new_image, old_image)
     ImageInfo	*new_image;
     ImageInfo	*old_image;
{
     mmap_handle_t	*file_ptr;
     int		 status = 0;

    if (strcmp (old_image->realfile, new_image->file) == 0)
      return (status);

     if (access (new_image->file, F_OK) == 0) {
       status = unlink (new_image->file);
       if (status != 0) {
	 AlertPrompt (base->top_level,
		      DIALOG_TYPE,  XmDIALOG_WARNING,
                      DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		      DIALOG_TITLE, catgets (prog->catd, 1, 424, "Image Viewer - Save"),
		      DIALOG_TEXT, catgets (prog->catd, 1, 316,
				   "Cannot save file.\nCheck permissions."),
		      BUTTON_IDENT, 0, catgets (prog->catd, 1, 317, "Continue"),
		      HELP_IDENT, HELP_SAVE_ITEM,
		      NULL);
	return (-2);
       }
     }
     file_ptr = fast_read (old_image->realfile);
     status = fast_write (new_image->file, file_ptr->addr, file_ptr->len, 0);
     fast_close (file_ptr);
     return (status);
}

int
save_newfile (tmpfname, save_type)
char		*tmpfname;
int		 save_type;
{
    char	       colors_str[MAXNAMELEN];
    char	       comp_str[MAXNAMELEN];
    int		       row;
    int		       type;
    int 	       status;
    int		       save_depth;
    ImageInfo	      *tmp_image = NULL;
    StateInfo	      *new_state;
    struct stat	       file_info;
    XilMemoryStorage  *storage;
    XilDataType        datatype;
    unsigned int       width, height, nbands;
    unsigned int       scanline_stride, pixel_stride;
    unsigned char     *data;
    unsigned char     *tmp_data;
    float 	       mult[1], offset[1];
    int		       do_compression = TRUE;
    XmString           xmstring;

    setbusy ();
/*
 * Make sure the fname is not a directory.
 */
    status = check_save_file (tmpfname, save_type);
    if (status != 0) {
      setactive();
      return (status);
    }
/*
 * Need to check some things on SaveAs.
 */

    switch (save_type) {
    case SAVEAS:
    case SAVESELECTIONAS:
    case SAVEPAGEASIMAGE:
/*
 * Get saving file format and
 * confirm data loss if JFIF/JPEG.
 */
      row = GetListSelection (saveas->format_list);
      type = GetFileFormat (saveas->format_list, row);
      if ((&all_types [type])->type == JFIF) {
        if ((status = confirm_data_loss (tmpfname)) == 0) {
	   setactive();
	   return (-1);
        }
      }
/*
 * Create a new ImageInfo with new filename.
 */
      tmp_image = init_image (tmpfname, tmpfname, 0, &all_types [type],
				 (char *) NULL);
/*
 * Get the depth to save in.
 */
      XtVaGetValues (XmOptionButtonGadget (saveas->colors_menu),
	  	     XmNlabelString, &xmstring, NULL);
      ConvertCompoundToChar (xmstring, &colors_str);
      XmStringFree (xmstring);
      tmp_image->depth = colors_to_depth (colors_str);

/*
 * Get the compression method.
 */
      XtVaGetValues (XmOptionButtonGadget (saveas->compression_menu),
	  	     XmNlabelString, &xmstring, NULL);
      ConvertCompoundToChar (xmstring, &comp_str);
      XmStringFree (xmstring);
      tmp_image->compression = string_to_compression (comp_str);
      break;

/*
 * Regular SAVE.
 */
    case SAVE:
/*
 * Confirm data loss if JFIF/JPEG.
 */
       if ((current_image->type_info)->type == JFIF) {
         if ((status = confirm_data_loss()) == 0) {
	   setactive();
	   return (-1);
         }
       }
/*
 * Make a copy of the current image, using same
 * depth, compression, file format.
 * Insert the new file name in case it changed.
 */
	tmp_image = init_image(tmpfname, tmpfname, 0, current_image->type_info,
	    (char *)NULL);
	tmp_image->compression = current_image->compression;
	tmp_image->depth = current_image->depth;
	tmp_image->nbands = current_image->nbands;

      break;

    default:
      break;

    }  /* end switch */
/*
    tmp_image->cmap = current_image->cmap;
*/

/*
 * Confirm file overwritten.
 */
    if ((status = confirm_overwrite_file (tmpfname, tmp_image->compression)) == 0) {
      if (tmp_image)
	destroy_image (tmp_image);
      setactive();
      return (-1);
    }

/*
 * If current view is PS, save out to XIL image.
 * Fix later, PS not saving from original.
 * else make a copy of current_image->orig_image.
 * Note, if we're doing a SAVE of a ps file, then bypass all of
 * this stuff...
 */
    if ((current_display != ps_display) || (save_type != SAVE)) {
      /*
       * strip off ".Z" if any from file name if file saves to
       * UNIX compress format
       */
      if (tmp_image->compression == UNIX) {
	int len = strlen(tmp_image->file);

	if (len > 2)
	  if (tmp_image->file[len-1] == 'Z')
	  if (tmp_image->file[len-2] == '.')
	  tmp_image->file[len-2] = '\0';
      }

      if (current_display == ps_display) {
	if (prog->xil_opened == FALSE) {
	  BaseImageCanvasObjectsCreate (base);
	  if (prog->xil_opened == False) {
            if (tmp_image)
	      destroy_image (tmp_image);
            setactive();
            return (-1);
	  }
        }

	current_image->orig_image = create_image_from_ps(current_image);

	current_image->saving = TRUE;
	assign_display_func(current_image, tmp_image->depth);
	current_image->saving = FALSE;

	if (save_type == SAVESELECTIONAS) {
	  XilImage	tmp_xilimage;

	  /* the image return from crop_region is a child image */
	  tmp_xilimage = crop_region(current_image,
				     current_image->orig_image);
	  if (tmp_xilimage == NULL) {
	    setactive();
	    return (-1);
	  }

	  tmp_image->orig_image = (current_image->display_func)
	    (tmp_xilimage);
	}
	else {
	  tmp_image->orig_image = (current_image->display_func)
	    (current_image->orig_image);
	}
      }
      else {
	current_image->saving = TRUE;
	assign_display_func(current_image, tmp_image->depth);
	current_image->saving = FALSE;

	/* create a copy that only contains selected region */
	if (save_type == SAVESELECTIONAS) {
	  XilImage	tmp_xilimage;

	  /* the image return from crop_region is a child image */
	  tmp_xilimage = crop_region(current_image,
				     current_image->dest_image);

	  if (tmp_xilimage == NULL) {
	    setactive();
	    return (-1);
	  }

	  tmp_image->orig_image = (current_image->display_func)
	    (tmp_xilimage);
	}
	else {
	  tmp_image->orig_image = (current_image->display_func)
	    (current_image->dest_image);
	}
      }

      /*
       * Copy the cmap, width, height, nbands, offset, colors_compressed,
       * bytes_per_line and rgborder from current image.
       *
       */
      copy_image_data(tmp_image, current_image);

      /* set the correct dimension */
      tmp_image->width = (*xil->get_width) (tmp_image->orig_image);
      tmp_image->height = (*xil->get_height) (tmp_image->orig_image);
      tmp_image->nbands = (*xil->get_nbands) (tmp_image->orig_image);
      tmp_image->datatype = (*xil->get_datatype) (tmp_image->orig_image);

      /* adjust bytes_per_line! */
      tmp_data = retrieve_data (tmp_image->orig_image, &pixel_stride,
				&scanline_stride);
      reimport_image(tmp_image->orig_image, FALSE);
      tmp_image->bytes_per_line = scanline_stride;

      /*
       * Rescale view_image back by multiplying the offset by -1.0.
       * Can ps files have an offset? If so, then this is wrong!
       */
      if (current_image->dithered24to8) {
	mult[0] = 1.0;
	offset[0] = (float) tmp_image->offset * -1.0;
	(*xil->rescale) (tmp_image->orig_image, tmp_image->orig_image, mult,
		     offset);
      }

      /*
       * At this point the image we're saving should be in
       * tmp_image->orig_image.
       * The save_funcs should save whatever's in orig_image.
       *
       * Delete the file (if exists) even if it's permissions are 444
       * because at this point the user said to overwrite even
       * if the file is read-only.
       */
      if (access(tmp_image->file, F_OK)==0 && tmp_image->compression!=UNIX) {
	status = unlink (tmp_image->file);
	if (status != 0) {
	  AlertPrompt (base->top_level,
		       DIALOG_TYPE,  XmDIALOG_WARNING,
                       DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		       DIALOG_TITLE, catgets (prog->catd, 1, 425, "Image Viewer - Save"),
		       DIALOG_TEXT, catgets (prog->catd, 1, 318,
					     "Cannot overwrite file."),
		       BUTTON_IDENT, 0, catgets (prog->catd, 1, 319, "Continue"),
		       HELP_IDENT, HELP_SAVE_ITEM,
		       NULL);
	  setactive ();
	  return (-2);
	}
      }
      status = (*tmp_image->type_info->save_func) (tmp_image);

    }
    else {

      int length = strlen (tmp_image->file);
      if ((length > 2) && ((tmp_image->file[length - 1] == 'Z') &&
			   (tmp_image->file[length - 2] == '.'))) {
	char *buf;

	buf = (char *) malloc (strlen (tmp_image->file) + 3);
	sprintf(buf, "%s%s", tmp_image->file, ".Z");
	free (tmp_image->file);
	tmp_image->file = buf;
	tmp_image->realfile = buf;
      }
      status = save_entire_ps_file (tmp_image, current_image);

      /*
       * If we saved the entire file, it is already compressed, so set
       * a flag to not do the compression.
       */
      do_compression = FALSE;
    }

    if (status != 0) {
      AlertPrompt (base->top_level,
		   DIALOG_TYPE,  XmDIALOG_WARNING,
                   DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		   DIALOG_TITLE, catgets (prog->catd, 1, 426, "Image Viewer - Save"),
		   DIALOG_TEXT, catgets (prog->catd, 1, 320,
					 "Cannot save file."),
		   BUTTON_IDENT, 0, catgets (prog->catd, 1, 321, "Continue"),
		   HELP_IDENT, HELP_SAVE_ITEM,
		   NULL);
      destroy_image (tmp_image);
      setactive ();
      return (-2);
    }

    if ((tmp_image->compression == UNIX) && (do_compression == TRUE)) {
      char command[80];
      char filename[MAXPATHLEN];
      char *error_string;
      int status;

      /*
       * Delete the *.Z file before compressing or compress will prompt user
       * asking if you wish to overwrite *.Z file.
       */
      sprintf (filename, "%s.Z", tmp_image->file);
      if (access (filename, F_OK) == 0) {
	status = unlink (filename);
	if (status != 0) {
	  AlertPrompt (base->top_level,
		   DIALOG_TYPE,  XmDIALOG_WARNING,
                   DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		   DIALOG_TITLE, catgets (prog->catd, 1, 427, "Image Viewer - Save"),
		   DIALOG_TEXT, catgets (prog->catd, 1, 322,
					 "Cannot save file."),
		   BUTTON_IDENT, 0, catgets (prog->catd, 1, 323, "Continue"),
		   HELP_IDENT, HELP_SAVE_ITEM,
		   NULL);
	  setactive ();
	  return (-2);
	}
      }
      sprintf(command, "%s %s", "/usr/bin/compress", tmp_image->file);
      if ((status = system(command)) == 0) {
	char *buf;

	if ((buf = (char*)malloc(strlen(tmp_image->file) + 3)) == NULL) {
	  if (prog->verbose) {
	    fprintf( stderr, catgets (prog->catd, 1, 324, "cannot malloc memory\n"));
	  }
	} else {
	  sprintf(buf, "%s%s", tmp_image->file, ".Z");
	  free(tmp_image->file);
	  tmp_image->file = buf;
	  tmp_image->realfile = buf;
	}
      } else if (status == 512) {
	/* not sure this status return all the time for this type
	 * compress - it did return from gif file
	 */
	char    error[1024];
	sprintf (error, catgets (prog->catd, 1, 325,
				 "%s not compressed.\nFile would be larger if compressed."), tmpfname);
	AlertPrompt (base->top_level,
		     DIALOG_TYPE, XmDIALOG_WARNING,
		     DIALOG_TITLE, catgets (prog->catd, 1, 428,
					    "Image Viewer - Save"),
		     DIALOG_TEXT, error,
		     BUTTON_IDENT, 0, catgets (prog->catd, 1, 326, "Continue"),
		     HELP_IDENT, HELP_SAVE_ITEM,
		     NULL);
      }
      else {
	AlertPrompt (base->top_level,
		     DIALOG_TYPE, XmDIALOG_WARNING,
		     DIALOG_TITLE, catgets (prog->catd, 1, 429,
					    "Image Viewer - Save"),
		     DIALOG_TEXT, catgets (prog->catd, 1, 430, "File compression failed"),
		     BUTTON_IDENT, 0, catgets (prog->catd, 1, 327, "Continue"),
		     HELP_IDENT, HELP_SAVE_ITEM,
		     NULL);
      }
    }

    /*
     * New state.
     */
    new_state = init_state ();

    stat (tmp_image->file, &file_info);
    tmp_image->file_size = file_info.st_size;

    /*
     * Need to load in the `new' postscript file before doing the
     * set_current_display, so do it now.
     * Also, check here, if old image was ps, then turn of ps stuff.
     * If new image is ps, then we kill pagecounter when we load in
     * the new one. If not, then we have to kill it now.
     */
    if (((tmp_image->type_info)->type == POSTSCRIPT) ||
	((tmp_image->type_info)->type == EPSF)) {
      ps_load (tmp_image);
    } else if (((current_image->type_info)->type == POSTSCRIPT) ||
	       ((current_image->type_info)->type == EPSF)) {
      close_ps (TRUE, TRUE);
    }

    set_tool_options (current_image, tmp_image);

    if (current_image != (ImageInfo *) NULL) {
      destroy_image (current_image);
    }
    current_image = tmp_image;

    if (current_state != (StateInfo *) NULL) {
      free (current_state);
    }
    current_state = new_state;

    set_current_display ();

    if (((current_image->type_info)->type != POSTSCRIPT) &&
	((current_image->type_info)->type != EPSF)) {
      assign_display_func(current_image, current_display->depth);
      check_canvas (current_image->width, current_image->height, image_display);

    }
    else { 	/* Is a postscript file */
      current_state->reversed = initial_reverse (TRUE);
      current_state->using_dsc = initial_dsc ();
      current_state->timeout_hit = initial_timeout ();
      current_state->zoom = prog->def_ps_zoom / 100.0;
    }

    resize_canvas ();

    if (current_display == ps_display) {
      fit_frame_to_image (current_image);
    }

    display_new_image();

    /*
     * If we were started via tooltalk, tell other application that file
     * was saved.
     */
#ifdef OLD_TOOLTALK
    if (prog->tt_started == TRUE)
      save_file_tt ();
#endif
    /*
     * Reset the scroll list on the saveas.
     */
    reread_file_choosers (save_type);

    check_popups (FALSE);

    setactive();

    if (palette != NULL) {
      reset_select ();
      reset_pan ();
    }

    return 0;
}

int
save_format_supported (type_info)
    TypeInfo *type_info;
{
    int  i;
    int  value = FALSE;

    for (i = 0; i < ntypes; i++) {
      if ((all_types[i].save_func != NULL) &&
          (type_info->type == all_types[i].type)) {
        value = TRUE;
        break;
      }
    }

    return value;

}

void
set_saveas_list (list)
     Widget    list;
{
     int       i, row, nitems = 0;
     XmString  xmstring;

/*
 * Set the saveas list, to the list in all_types, adding client
 * data, so we can easily determine the choice when the user selects
 * something.
 */

     for (i = 0; i < ntypes; i++) {
       if (all_types[i].save_func != NULL) {
	 xmstring = XmStringCreateLocalized (all_types[i].popup_list_name);
	 row = InsertInList (list, nitems, all_types[i].popup_list_name);
	 nitems++;
	 XmListAddItemUnselected (list, xmstring, row);
	 XmStringFree (xmstring);
       }
     }

/*
 * Select SunRaster.
 * Set this selection to be at the top of the list.
 */
     xmstring = XmStringCreateLocalized (all_types[0].popup_list_name);
     XmListSelectItem (list, xmstring, True);
     XmListSetItem (list, xmstring);
     XmStringFree (xmstring);

}

static void
set_colors_value (ftype)
     FileTypes    ftype;
{
     XmString  xmstring;
/*
 * Set all menu items inactive.
 * Then turn on appropriate items per format type.
 */
     XtVaSetValues (saveas->bw_item, XmNsensitive, False, NULL);
     XtVaSetValues (saveas->colors_256_item, XmNsensitive, False, NULL);
     XtVaSetValues (saveas->colors_millions_item, XmNsensitive, False, NULL);
     xmstring = XmStringCreateLocalized (catgets (prog->catd, 1, 328, "256"));
     XtVaSetValues (XmOptionButtonGadget (saveas->colors_menu),
		    XmNlabelString, xmstring, NULL);
     XmStringFree (xmstring);

     switch (ftype) {
     /*
      * 1, 8, 24-bit
      */
     case RASTER:
     case EPSF:
     case POSTSCRIPT:
     case TIF:
       XtVaSetValues (saveas->bw_item, XmNsensitive, True, NULL);
       XtVaSetValues (saveas->colors_256_item, XmNsensitive, True, NULL);
       XtVaSetValues (saveas->colors_millions_item, XmNsensitive, True, NULL);
       xmstring = XmStringCreateLocalized (catgets (prog->catd, 1, 329, "256"));
       XtVaSetValues (XmOptionButtonGadget (saveas->colors_menu),
		      XmNlabelString, xmstring, NULL);
       XmStringFree (xmstring);
       break;
     /*
      * 24-bit only
      */
     case JFIF:
       XtVaSetValues (saveas->colors_millions_item, XmNsensitive, True, NULL);
       xmstring = XmStringCreateLocalized (catgets (prog->catd, 1, 330,
						    "Millions"));
       XtVaSetValues (XmOptionButtonGadget (saveas->colors_menu),
		      XmNlabelString, xmstring, NULL);
       XmStringFree (xmstring);
       break;

    /*
     * 8-bit only.
     */
     default:
       XtVaSetValues (saveas->colors_256_item, XmNsensitive, True, NULL);
       xmstring = XmStringCreateLocalized (catgets (prog->catd, 1, 331, "256"));
       XtVaSetValues (XmOptionButtonGadget (saveas->colors_menu),
		      XmNlabelString, xmstring, NULL);
       XmStringFree (xmstring);
       break;
    }
}

static void
set_compression_value( ftype )
     FileTypes  ftype;
{
     XmString   xmstring;
/*
 * Set all menu items inactive.
 * Then turn on appropriate items per format type.
 */
     XtVaSetValues (saveas->none_item, XmNsensitive, False, NULL);
     XtVaSetValues (saveas->unix_item, XmNsensitive, False, NULL);
     XtVaSetValues (saveas->encoded_item, XmNsensitive, False, NULL);
     XtVaSetValues (saveas->lzw_item, XmNsensitive, False, NULL);
     XtVaSetValues (saveas->jpeg_item, XmNsensitive, False, NULL);

     switch (ftype) {
     case TIF:
       XtVaSetValues (saveas->none_item, XmNsensitive, True, NULL);
       XtVaSetValues (saveas->unix_item, XmNsensitive, True, NULL);
       XtVaSetValues (saveas->lzw_item, XmNsensitive, True, NULL);
       xmstring = XmStringCreateLocalized (catgets (prog->catd, 1, 332, "LZW"));
       XtVaSetValues (XmOptionButtonGadget (saveas->compression_menu),
		     XmNlabelString, xmstring, NULL);
       XmStringFree (xmstring);
       break;
     case RASTER:
       XtVaSetValues (saveas->none_item, XmNsensitive, True, NULL);
       XtVaSetValues (saveas->unix_item, XmNsensitive, True, NULL);
       XtVaSetValues (saveas->encoded_item, XmNsensitive, True, NULL);
       xmstring = XmStringCreateLocalized (catgets (prog->catd, 1, 333, "None"));
       XtVaSetValues (XmOptionButtonGadget (saveas->compression_menu),
		     XmNlabelString, xmstring, NULL);
       XmStringFree (xmstring);
       break;
     case JFIF:
       XtVaSetValues (saveas->jpeg_item, XmNsensitive, True, NULL);
       xmstring = XmStringCreateLocalized (catgets (prog->catd, 1, 334, "JPEG"));
       XtVaSetValues (XmOptionButtonGadget (saveas->compression_menu),
		     XmNlabelString, xmstring, NULL);
       XmStringFree (xmstring);
       break;
     case GIF:
       XtVaSetValues (saveas->none_item, XmNsensitive, True, NULL);
       xmstring = XmStringCreateLocalized (catgets (prog->catd, 1, 335, "None"));
       XtVaSetValues (XmOptionButtonGadget (saveas->compression_menu),
		     XmNlabelString, xmstring, NULL);
       XmStringFree (xmstring);
       break;
     default:
       XtVaSetValues (saveas->none_item, XmNsensitive, True, NULL);
       XtVaSetValues (saveas->unix_item, XmNsensitive, True, NULL);
       xmstring = XmStringCreateLocalized (catgets (prog->catd, 1, 336, "None"));
       XtVaSetValues (XmOptionButtonGadget (saveas->compression_menu),
		     XmNlabelString, xmstring, NULL);
       XmStringFree (xmstring);
     }
}

void
SaveFormatListCallback (list, client_data, cbs)
     Widget                list;
     XtPointer             client_data;
     XmListCallbackStruct *cbs;
{
     int        format, row;
     TypeInfo  *ftype;

     switch (cbs->reason) {
     /*
      * Double-click event.
      */
     case XmCR_DEFAULT_ACTION:
     /*
      * Select event.
      */
     case XmCR_BROWSE_SELECT:
       row = GetListSelection (saveas->format_list);
       format = GetFileFormat (saveas->format_list, row);
       ftype = &all_types[format];
       set_colors_value (ftype->type);
       set_compression_value (ftype->type);
       break;
     default:
       break;
     }

}
