/*
 * (c) Copyright 1996 Sun Microsystems, Inc.
 */


#pragma ident "@(#)utils.h	1.2 96/06/20 SMI"


#ifndef _UTILS_H_
#define _UTILS_H_

/*
 * Variable "types" to be passed to the AlertPrompt function (that
 * function accepts a variable argument list with these attribute,
 * value pairs).
 *
 * The BUTTON_IDENT attribute can be used to identify 1 or more buttons
 * on the dialog.  The int id value will be returned if that button is
 * pressed and the char* name value will be the button label.  Button id
 * has to be greater than 0, and if the user close the window through the
 * window manager, a value of -1 will be return.
 *  
 */
enum {	DIALOG_TYPE = 1,	/* Followed by XmNdialogType */
	DIALOG_TITLE,		/* Followed by char* */
	DIALOG_STYLE,		/* Followed by XmNdialogStyle */
	DIALOG_TEXT,		/* Followed by char* */
	BUTTON_IDENT,		/* Followed by int, char* */
	HELP_IDENT		/* Followed by char* */
};

/*
 * Defines button id values for Default Message Dialog.
 */
enum {	ANSWER_NONE = 0,
	ANSWER_ACTION_1,
	ANSWER_ACTION_2,
	ANSWER_ACTION_3,
	ANSWER_CANCEL,
	ANSWER_HELP
};

void MessageDialogQuitHandler( Widget, XtPointer, XtPointer );
void MessageDialogResponse_CB( Widget, XtPointer, XtPointer );
void AlertPromptHelp_CB( Widget, XtPointer, XtPointer );
int AlertPrompt( Widget, ... );

int dtb_set_label_from_image_file(Widget  widget, String  fileName);

int dtb_cvt_image_file_to_pixmap(Widget  widget,
				 String  fileName, 
				 Pixmap  *pixmap);

int dtb_set_label_pixmaps(Widget      widget,
			  Pixmap      labelPixmap,
			  Pixmap      labelInsensitivePixmap);

Pixmap dtb_create_greyed_pixmap(Widget      widget,
				Pixmap      pixmap);

Boolean dtb_file_has_extension(String      fileName,
			       String      extension);

int dtb_cvt_filebase_to_pixmap(Widget      widget,
			       String      fileBase,
			       String      extension,
			       Pixmap      *pixmap_ptr);
int dtb_cvt_file_to_pixmap(
			   String      fileName,
			   Widget      widget,
			   Pixmap      *pixmapReturnPtr);

String dtb_get_exe_dir(void);

void dtb_save_command(char        *argv0);
static int determine_exe_dir(char        *argv0,
			     char        *buf,
			     int         bufSize);
static int determine_exe_dir_from_argv(char        *argv0,
				       char        *buf,
				       int         bufSize);
static int determine_exe_dir_from_path (char        *argv0,
					char        *buf,
					int         bufSize);
static Boolean path_is_executable(char        *path,
				  uid_t       euid,
				  gid_t       egid);


#endif				/* _UTILS_H_ */
