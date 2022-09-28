/*
 * @(#)misc.h 1.4 97/03/19
 *
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

#ifndef	_MISC_H
#define	_MISC_H

#include <Xm/Xm.h>

/*
**  Variable "types" to be passed to the dialog_popup function (that
**  function accepts a variable argument list with these attribute, value
**  pairs).
**
**  The BUTTON_IDENT attribute can be used to identify 1 or more buttons on
**  the dialog.  The int id value will be returned if that button is
**  pressed and the char* name value will be the button label.
*/
typedef enum {
	DIALOG_TYPE = 1,	/*  Followed by XmNDialogType */
        DIALOG_TITLE,           /*  Followed by char*  */
	DIALOG_STYLE,		/*  Followed by XmNDialogStyle */
	DIALOG_LIST,		/*  Followed by no of rows (XmNvisibleItemCount */
	DIALOG_LIST_TEXT,	/*  Followed by ERRSTK of strings for list */
        DIALOG_TEXT,            /*  Followed by char*  */
        BUTTON_IDENT,           /*  Followed by int, char*  */
        HELP_IDENT              /*  Followed by char*  */
} Dialog_create_op;
 

extern int AlertPrompt (Widget frame, ...);

#endif	/* _MISC_H */
