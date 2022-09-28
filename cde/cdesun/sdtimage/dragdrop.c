#ifndef lint
static char sccsid[] = "@(#)dragdrop.c 1.16 97/04/24";
#endif

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

#include <dirent.h>
#include <stdio.h>
#include <Dt/Dnd.h>
#include <Dt/Dts.h>
#include <Dt/IconFile.h>

#include "help.h"
#include "image.h"
#include "imagetool.h"
#include "misc.h"
#include "ui_imagetool.h"

#define DRAG_THRESHOLD   4

static Boolean        doingDrag = False;
static char          *drag_data;
static char          *drag_name;

static void
DndProcessFilename (cbs)
     DtDndTransferCallbackStruct  *cbs;
{
     char  *path;
     int    status;
/*
 * Check for multiple filenames.
 */
     if (cbs->dropData->numItems != 1) {
       AlertPrompt (base->top_level,
		    DIALOG_TYPE,  XmDIALOG_ERROR,
                    DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		    DIALOG_TITLE, catgets (prog->catd, 1, 389, "Image Viewer - Drag and Drop"),
		    DIALOG_TEXT, catgets (prog->catd, 1, 24,
				 "Please drag only one file at a time onto Image Viewer."),
		    BUTTON_IDENT, 0, catgets (prog->catd, 1, 25, "Continue"),
		    HELP_IDENT, HELP_DND_ITEM,
		    NULL);
       cbs->status = DtDND_FAILURE;
       return;
     }

     path = cbs->dropData->data.files[0];
       
     if (path) 
       status = open_newfile (path, path, -1, (char *)NULL, 0);
     else
       cbs->status = DtDND_FAILURE;

}


static void
DndProcessBuffer (cbs)
     DtDndTransferCallbackStruct  *cbs;
{
     DtDndBuffer  *buffers;
     char          current_filename [MAXNAMLEN];
     FILE         *dnd_file;
     int           status;
/*
 * Use abbreviated method of referring to the data buffers.
 */
     if (cbs->dropData != NULL)
       buffers = cbs->dropData->data.buffers;
     else {
       cbs->status = DtDND_FAILURE;
       return;
     }

/*
 * Check for multiple buffers.
 */
     if (cbs->dropData->numItems != 1) { 
       AlertPrompt (base->top_level,
		    DIALOG_TYPE,  XmDIALOG_ERROR,
                    DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		    DIALOG_TITLE, catgets (prog->catd, 1, 390, "Image Viewer - Drag and Drop"),
		    DIALOG_TEXT, catgets (prog->catd, 1, 26,
				 "Please drag one file at a time onto Image Viewer."),
		    BUTTON_IDENT, 0, catgets (prog->catd, 1, 27, "Continue"),
		    HELP_IDENT, HELP_DND_ITEM,
		    NULL);
       cbs->status = DtDND_FAILURE;
       return;
     }

     if (buffers[0].size < 1) {
       AlertPrompt (base->top_level,
		    DIALOG_TYPE,  XmDIALOG_ERROR,
                    DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		    DIALOG_TITLE, catgets (prog->catd, 1, 391, "Image Viewer - Drag and Drop"),
		    DIALOG_TEXT, catgets (prog->catd, 1, 28,
					  "File is empty."),
		    BUTTON_IDENT, 0, catgets (prog->catd, 1, 29, "Continue"),
		    HELP_IDENT, HELP_DND_ITEM,
		    NULL);
       cbs->status = DtDND_FAILURE;
       return;
     }
/* 
 * The length is non-zero, so we got data.
 * If dnd_file is NULL, then we haven't opened the tmp file for writing.
 */
     if (buffers[0].name != (char *) NULL)
       strcpy (current_filename, basename (buffers[0].name));
     else
       strcpy (current_filename, catgets (prog->catd, 1, 30, "(Untitled)"));
		 
     make_tmpfile (&(prog->datafile), current_filename);
     dnd_file = fopen (prog->datafile, "w");
     if (dnd_file == (FILE *) NULL) {
       AlertPrompt (base->top_level,
		    DIALOG_TYPE,  XmDIALOG_ERROR,
                    DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		    DIALOG_TITLE, catgets (prog->catd, 1, 392, "Image Viewer - Drag and Drop"),
		    DIALOG_TEXT, catgets (prog->catd, 1, 31,
					  "Could not open temporary file."),
		    BUTTON_IDENT, 0, catgets (prog->catd, 1, 32, "Continue"),
		    HELP_IDENT, HELP_DND_ITEM,
		    NULL);
       cbs->status = DtDND_FAILURE;
       return;
     }
     
     if (fwrite (buffers[0].bp, buffers[0].size, 1, dnd_file) != 1) {
       AlertPrompt (base->top_level,
		    DIALOG_TYPE,  XmDIALOG_ERROR,
                    DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		    DIALOG_TITLE, catgets (prog->catd, 1, 393, "Image Viewer - Drag and Drop"),
		    DIALOG_TEXT, catgets (prog->catd, 1, 33,
					  "Could not write to temporary file."),
		    BUTTON_IDENT, 0, catgets (prog->catd, 1, 34, "Continue"),
		    HELP_IDENT, HELP_DND_ITEM,
		    NULL);
       fclose (dnd_file);
       cbs->status = DtDND_FAILURE;
       return;
     }
     
     fclose (dnd_file);
/*
 * Display the file.
 */
     status = open_newfile (current_filename, prog->datafile, -1, NULL, 0);

}

/*
 * Handles the transfer of a file or appointment to the draw area.
 * Adds the appropriate icon to the list of icons on the draw area.
 */
void
DndTransferCallback(
     Widget	w,
     XtPointer	clientData,
     XtPointer	callData)
{
     DtDndTransferCallbackStruct  *cbs =
     				(DtDndTransferCallbackStruct *)callData;

     if (cbs == NULL) {
       cbs->status = DtDND_FAILURE;
       return;
     }
       
     switch (cbs->dropData->protocol) {
     case DtDND_FILENAME_TRANSFER:
       DndProcessFilename (cbs);
       break;
     case DtDND_BUFFER_TRANSFER:
       DndProcessBuffer (cbs);
       break;
     default:
       cbs->status = DtDND_FAILURE;
       break;
     }
}

/*
 * DndConvertCallback
 *
 * Fills in buffer object with calendar appointment string based on which
 * appointments are selected in the scrolled list when the drag is started.
 * When no appointments are selected, the appointment under the pointer is
 * used. Supply a label for the calendar appointment based on the contents
 * of the appointment.
 */
static void
DndConvertCallback (
     Widget	dragContext,
     XtPointer	clientData,
     XtPointer	callData)
{
     DtDndConvertCallbackStruct *convertInfo =
                                (DtDndConvertCallbackStruct *) callData;
     DtDndBuffer	*buffers = convertInfo->dragData->data.buffers;
     FILE		*dnd_fp;
     size_t              size;

     if (convertInfo == NULL)
       return;

     /*
      * Verify the validity of the callback reason
      */
     if (convertInfo->reason != DtCR_DND_CONVERT_DATA &&
	 convertInfo->reason != DtCR_DND_CONVERT_DELETE) {
       convertInfo->status = DtDND_FAILURE;
       return;
     }
     
     /*
      * Supply the data for transfer into buffer.
      */
     if (convertInfo->reason == DtCR_DND_CONVERT_DATA) {
       /*
	* Allocate space for the buffer data.
        */
       drag_data = malloc (current_image->file_size);
       if (drag_data == NULL) {
	 convertInfo->status = DtDND_FAILURE;
	 return;
       }
       
       /*
	* Get the data from the disk file.
	*/
       if (current_image->data == (char *) NULL) {
	 dnd_fp = fopen (current_image->realfile, "r");
         if (dnd_fp == (FILE *) NULL) {
           convertInfo->status = DtDND_FAILURE;
           return;
         }
         size = fread (drag_data, 1, current_image->file_size, dnd_fp);
	 if ((int) size != current_image->file_size) {
           convertInfo->status = DtDND_FAILURE;
	   fclose (dnd_fp);
           return;
	 }
	 fclose (dnd_fp);
       }
       /*
	* or get the data from the image structure in memory.
	*/
       else {
         memcpy (drag_data, current_image->data, current_image->file_size);
       }
       
       /*
	* Store the size in the buffer info.
	*/
       buffers[0].bp = drag_data;
       buffers[0].size = current_image->file_size;

       /* 
	* Supply the name of the file 
	*/
       if (current_image->file) 
	 drag_name = malloc (strlen (basename (current_image->file)) + 1);
       else
	 drag_name = malloc (strlen (catgets (prog->catd, 1, 35, "(Untitled)")) + 1);
       strcpy (drag_name, basename (current_image->file));
       buffers[0].name = drag_name;
     } 
#ifdef LATER
     /*
      * Delete the data
      */
     else if (convertInfo->reason == DtCR_DND_CONVERT_DELETE) {
       printf("Delete appointment for %s\n", appt->what);
     }
#endif

}

/*
 * DndFinishDragCallback
 *
 * Resets drag state to indicate the drag is over. Free memory allocated 
 * with the drag.
 */
static void
DndFinishDragCallback(
     Widget	widget,
     XtPointer	clientData,
     XtPointer	callData)
{
     DtDndDragFinishCallbackStruct *cbs = 
				(DtDndDragFinishCallbackStruct *)callData;
     DtDndBuffer	 *buffers;
     static XtCallbackRec transfer_rec [] = {{DndTransferCallback, NULL},
                                             {NULL, NULL}};
     if (cbs && cbs->reason == DtCR_DND_DRAG_FINISH) {

       buffers = cbs->dragData->data.buffers;
       /*
	* Reset drag flag.
	*/
       doingDrag = False;

       /*
	* Free any data allocated for the drag out.
	* Using saved pointers instead of pointer from cbs 
	* because buffers == NULL when dropped on invalid site.
	*/
       if (drag_data) {
	 free (drag_data);
	 drag_data = NULL;
       }
       if (drag_name) {
	 free (drag_name);
	 drag_name = NULL;
       }
       
       /*
	* Set Scrolled Window to be drop site again.
	*/
       DtDndVaDropRegister (base->scrolledw,
			    DtDND_FILENAME_TRANSFER | DtDND_BUFFER_TRANSFER,
			    XmDROP_COPY, transfer_rec,
			    NULL);
     }

}


static void
DndBufferDrag (w, event)
     Widget   w;
     XEvent  *event;
{
     static XtCallbackRec convertCBRec[] = { {DndConvertCallback, NULL},
					     {NULL, NULL} };
     static XtCallbackRec dragFinishCBRec[] =  { {DndFinishDragCallback, NULL},
						 {NULL, NULL} };
     char          *host_prefix;
     char          *icon_name = NULL;
     char          *icon_filename = NULL;
     Pixmap         icon = NULL, icon_mask = NULL;
     static Widget  drag_icon = NULL;
     Pixel          fg, bg;
     static int     source_type = NO_TYPE;
/*
 * Unregister current imagetool's drop site during the drag.
 */
     DtDndDropUnregister (base->scrolledw);
/*
 * Create a new source drag icon only if needed:
 * Either this is the first DND or if the type of file changed.
 */
     if (source_type != current_image->type_info->type) {
       /*
	* Get the icon from the typing database.
	*/
       host_prefix = DtDtsDataTypeToAttributeValue 
                       (current_image->type_info->type_name, 
			DtDTS_DA_DATA_HOST, NULL);
            
       icon_name = DtDtsDataTypeToAttributeValue 
                     (current_image->type_info->type_name, 
		      DtDTS_DA_ICON, NULL);

       if (icon_name != NULL)
	 icon_filename = (char *) XmGetIconFileName 
                           (XtScreen (base->top_level), NULL, icon_name, 
		    	    host_prefix, XmMEDIUM_ICON_SIZE);
       /*
	* Destroy previous drag icon and pixmaps, if created.
	* Check to make sure we're not destroying textualdragicon.
	*/
       if (drag_icon && (drag_icon != (Widget) _XmGetTextualDragIcon (w))) {
	 if (icon) {
	   XFreePixmap (XtDisplay (base->top_level), icon);
	   icon = NULL;
	 }
	 if (icon_mask) {
	   XFreePixmap (XtDisplay (base->top_level), icon_mask);
	   icon_mask = NULL;
	 }
	 XtDestroyWidget (drag_icon);
       }

       /*
	* Create the icon and mask from the icon file name.
	*/
       if (icon_filename != NULL) {
	 XtVaGetValues (base->form, XmNforeground, &fg,
		                    XmNbackground, &bg, NULL);
	 icon = XmGetPixmap (XtScreen (base->top_level), icon_filename, 
			     fg, bg); 
	 if (icon != XmUNSPECIFIED_PIXMAP)
	   icon_mask = _DtGetMask (XtScreen (base->top_level), icon_filename);
       }

       /*
	* Create new source icon.
	* Default to textual icon if icon and icon_mask not found for
	* this image type.
	*/
       if (icon && icon_mask)
	 drag_icon = DtDndCreateSourceIcon (w, icon, icon_mask);
       else
	 drag_icon = (Widget) _XmGetTextualDragIcon (w);
       /*
	* Save the type of icon just created so next time we might
	* not have to create it again.
	*/
       source_type = current_image->type_info->type;

       /*
	* Free data from typing database.
	*/
       if (host_prefix) {
	 DtDtsFreeAttributeValue (host_prefix);
	 host_prefix = NULL;
       }
       if (icon_name) {
	 DtDtsFreeAttributeValue (icon_name);
	 icon_name = NULL;
       }
       if (icon_filename) {
	 free (icon_filename);
	 icon_filename = NULL;
       }
     }
       
     /*
      * Start the drag and drop.
     convertCBRec[0].closure = (XtPointer) w;
     dragFinishCBRec[0].closure = (XtPointer) w;
      */

     if (drag_icon)
       DtDndVaDragStart (w, event, DtDND_BUFFER_TRANSFER, 1,
			 XmDROP_COPY,
			 convertCBRec, dragFinishCBRec,
			 DtNsourceIcon, drag_icon, NULL);

}

/*
 * Drag Motion Handler 
 * Determine if the pointer has moved beyond the drag threshold while button 1
 * was being held down.
 */
void
DndEventHandler (dragInitiator, client_data, event)
     Widget		dragInitiator;
     XtPointer	        client_data;
     XEvent	       *event;
{
     static int	initialX = -1;
     static int	initialY = -1;
     int		diffX, diffY;
     long		dragProtocol = (long)client_data;
     
     if (current_image == NULL)
       return;

     if (!doingDrag) {
       /*
	* If the drag is just starting, set initial button down coords
	*/
       if (initialX == -1 && initialY == -1) {
	 initialX = event->xmotion.x;
	 initialY = event->xmotion.y;
       }
       /*
	* Find out how far pointer has moved since button press
	*/
       diffX = initialX - event->xmotion.x;
       diffY = initialY - event->xmotion.y;
       
       if ((abs(diffX) >= DRAG_THRESHOLD) ||
	   (abs(diffY) >= DRAG_THRESHOLD)) {
	 doingDrag = True;
	 switch (dragProtocol) {
	 case DtDND_BUFFER_TRANSFER:
	   DndBufferDrag (dragInitiator, event);
	   break;
	 }
	 initialX = -1;
	 initialY = -1;
       }
     }
}

