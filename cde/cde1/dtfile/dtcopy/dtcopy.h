/* $XConsortium: dtcopy.h /main/cde1_maint/3 1995/10/09 14:53:26 pascale $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           dtcopy.h
 *
 *
 *   DESCRIPTION:    Header file used in copydialog.c,errordialog.c
 *                   main_dtcopy.c,overwrtdialog.c,utils.c
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

/* Macros */
#define MAX_PATH      1024

#define G_OVERWRITE_EXISTING		0
#define G_SKIP      			1
#define G_RENAME_EXISTING      		2
#define N_OVERWRITE_RADIO_BUTTONS	3

/* Help numbers that are used locally in dtcopy
 * to indentify what help volume should be used
 * for a particular callback.  Look to utils.c
 * in help_callback for code.  MJT
 */
#define HELP_HELP_MENU			1
#define HELP_INTRODUCTION		2
#define HELP_DIR_COPY			3
#define HELP_DIR_ERROR			4
#define HELP_OVERWRITE			5
#define HELP_USING			6
#define HELP_VERSION			7
#define STOPW_HELP_DIALOG		9

/*  debugging output  */
#ifdef DEBUG
extern int debug;
#define DPRINTF(p)  if (debug) printf p
#define DPRINTF2(p) if (debug >= 2) printf p
#else
#define DPRINTF(p)  /* noop */
#define DPRINTF2(p) /* noop */
#endif

/* _DtMessage Dialog build defines */
#define ERROR_DIALOG  1
#define WARNING_DIALOG  2
#define QUESTION_DIALOG 3

/* Global Variables */
extern Widget G_toplevel;
extern Widget G_overwrite_radio;
extern Widget G_summary_text;
extern Widget G_status_text;
extern Widget G_status_text2;
extern Widget G_error_count;
extern Widget G_error_msgs;
extern Widget G_copy_dialog;
extern Widget G_error_dialog;
extern Widget G_over_dialog;
extern Widget G_over_confirm;
extern Widget G_copy_action_area_pbuttons[4];
extern Widget G_help_dialog;
extern Widget G_rename_text;
extern Widget G_toggle_main;
extern Widget G_toggle_error;

extern char  G_rename_oldname[];
extern char  G_source_dir[];
extern int G_move;
extern int G_do_copy;
extern int G_pause_copy;
extern int G_user_pause_copy;
extern int G_wait_on_error;
extern int G_error_status;
extern int G_ignore_errors;
extern int G_overwrite_selection;
extern int G_overwrite_confirm;
extern int G_wait_on_input;
extern int G_dialog_closed;

/* Types */


/* External Routines */

#ifdef _NO_PROTO
extern void sensitize_copy_action_area();
extern void desensitize_copy_action_area();
extern void create_copydir_dialog();
extern void create_error_dialog();
extern void create_overwrite_dialog();
extern void _DtChildPosition();
extern void help_callback();
extern void display_cancel_warning();
extern void cancel_callback();
extern void toggle_callback();
extern int  touch();
#else
extern void help_callback(
	Widget widget,
	XtPointer client_data,
	XtPointer call_data);
extern void sensitize_copy_action_area(
		Widget *pushbutton_array);
extern void desensitize_copy_action_area(
		Widget *pushbutton_array);
extern void create_copydir_dialog(
		Widget parent_widget,
		char *source,
		char *target);
extern void create_error_dialog(
		Widget parent_widget,
		char *op,
		char *filename,
		int errno);
extern void create_overwrite_dialog(
		Widget parent_widget,
		char *sname,
		char *tname,
		Boolean isdir);
extern void _DtChildPosition(
		Widget w,
		Widget parent,
		Position *newX,
		Position *newY);
extern void cancel_callback(
 		Widget w,
		XtPointer client_data,
		XtPointer call_data);
extern void display_cancel_warning(Widget parent);
extern void toggle_callback(
		Widget w,
		XtPointer client_data,
		XtPointer call_data);
extern int  touch(String path);
#endif

/*
 * macro to get message catalog strings
 */
#ifdef MESSAGE_CAT
extern const char* _CLIENT_CAT_NAME;
/*
 * Without this proto, standard C says that _DtGetMessage() returns
 * an int, even though it really returns a pointer.  The compiler is
 * then free to use the high 32-bits of the return register for
 * something else (like scratch), and that can garble the pointer.
 */
#ifdef _NO_PROTO
extern char *_DtGetMessage();
#else  /* _NO_PROTO */
extern char *_DtGetMessage(
			   const char *filename,
			   int set,
			   int n,
			   char *s );
#endif /* _NO_PROTO */
#define GETMESSAGE(set, number, string)\
    (_DtGetMessage(_CLIENT_CAT_NAME, set, number, string))
#else /* MESSAGE_CAT */
#define GETMESSAGE(set, number, string)\
    string
#endif /* MESSAGE_CAT */
