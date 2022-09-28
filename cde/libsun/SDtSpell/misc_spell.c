/* 
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */ 

#pragma ident "@(#)misc_spell.c	1.12 96/06/19 SMI"

#include <Xm/Xm.h>
#include <Xm/MessageB.h>

#include "spelldefs.h"
#include "spelltypes.h"
#include "globals.h"
#include "engine.h"

/*--------------------------- FUNCTION DEFINITIONS ---------------------------*/



/* Function	:	_SDtSpellStripCopyWord()
 |
 | Objective    :       Strip the leading and trailing punctuation from
 |			the input word and copy the result to the misspelled
 |			member of the spell checker structure..
 |
 | Arguments    :       SplCheckP - spell checker structure
 |			char *    - word source
 |
 | Return Value	:	char *allocated word.
 |
 */

char *
 _SDtSpellStripCopyWord(SpellEngP spellEngine, char *word, int *start, int *end)
{
	char *misspelled;

	/* The leading and trailing punctuations. */
	char *pre =
		(char *)(spellEngine -> mainDict -> icBuff -> prestrip);
	char *post = 
		(char *)(spellEngine -> mainDict -> icBuff -> poststrip);


	/* Calculated indices to the ending location of the leading 
	 | punctuation and the beginning location of the trailing
	 | punctuation in the word.
	 */
	int	loc_pre, loc_post;
	

	char *p;	/* Pointer to access the string. */


	/* Allocate space for the misspelled word.  It should be the
	 | same size as the word (in case no stripping occurs), plus
	 | space for the null value.
	 */
	misspelled = (char *) calloc(strlen(word) + 2, sizeof(char));


	/* Get the locations of the leading and trailing punctuation.
	 | For example, in the string
	 |	($Four!*%)
	 | loc_pre will get a value of 2 (the $), while loc_post will get
	 | a value of 6 (the r before the !).  The values will be used to
	 | get the string "Four" out of the string "($Four!*%)" and
	 | will be put inmisspelled.
	 */
	loc_pre = strlen(pre);
	loc_post = strlen(word) - strlen(post);
	*start += loc_pre;
	*end -= strlen(post);


	/* Now copy the substring that we're really interested in. */
        p = &(word[loc_pre]);
	strncpy(misspelled, p, loc_post - loc_pre);

	return misspelled;
}


/* This function returns a char * of the error message. */

char *
_SDtSpellErrorTypes(int error_flags)
{
	/* NOTE this wont work which motif is MT safe */
	/* this doesnt handle wide characters */

	static char returnstr[255];
	char *msg;

	if (error_flags & ICCAPERR)
		 msg = catgets(_spellCat, 3, 6, "Capitalization.");
	else if ( error_flags & ICORDERR)
		msg = catgets(_spellCat, 3, 7, "Number misspelled.");
	else if ( error_flags & ICACCERR)
		msg = catgets(_spellCat, 3, 8, "Accent misplaced.");
	else if ( error_flags & ICAPOSALT)	 
		msg = catgets(_spellCat, 3, 9, "Apostrophe misplaced.");
	else if ( error_flags & ICCMPERR)
		msg = catgets(_spellCat, 3, 5, "Unknown words.");
	else 
		msg = catgets(_spellCat, 3, 4, "Unknown word.");

	strcpy(returnstr, msg);

	return returnstr;
}


/* Function	: _SDtSpellShowDialog()
 |
 | Objective	: Generic method to bing up a dialog for errors, 
 |		  messages, and special conditions.
 |
 | Arguments	: Widget		- parent of dialog
 |		  char *		- dialog title
 |		  int			- dialog type, e.g., error, info, etc.
 |		  int			- button mask to inform what buttons
 |					  to hide
 |		  int			- default button
 |		  char *		- message to display in dialog
 |		  char *		- alternative OK button label
 |		  char *		- alternative Cancel button label
 |		  char *		- alternative Help button label.
 |
 | Return value : Widget		- widget id of dialog
 */

Widget
_SDtSpellShowDialog(Widget parent, char *dlg_title, int dlg_type, 
		int btn_mask_out, int default_btn, char *dlg_msg, 
		char *ok_msg, char *cancel_msg, char* help_msg)
{
	Widget		dialog;
	XmString	xms;


	/* Create the dialog. */
	dialog = XmCreateMessageDialog(parent, "_SDtSpellShowDialog", NULL, 0);
	if (dialog == (Widget) NULL)
		return((Widget) NULL);


	/* Setup the dialog title, dialog type, modal style, and
	 | default button.
	 */
	if (dlg_title == (char *) NULL)
		/* Use default title if null. */
		xms = XmStringCreateLocalized(catgets(_spellCat, 1, 14, 
					      "Spelling Checker Dialog"));
	else
		xms = XmStringCreateLocalized(dlg_title);
	XtVaSetValues(dialog, 
		      XmNdialogTitle, xms,
		      XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL,
		      XmNdefaultButtonType, default_btn,
		      XmNdialogType, dlg_type,
		      NULL);
	XmStringFree(xms);


	/* Set up dialog message. */
	if (dlg_msg != (char *) NULL) {
		xms = XmStringCreateLocalized(dlg_msg);
		XtVaSetValues(dialog, 
				 XmNmessageString, xms,
				 NULL); 
		XmStringFree(xms);
	}


	/* Override the OK button label. */
	if (ok_msg != (char *) NULL) {
		xms = XmStringCreateLocalized(ok_msg);
		XtVaSetValues(dialog, 
				 XmNokLabelString, xms,
				 NULL); 
		XmStringFree(xms);
	}


	/* Override the Cancel button label. */
	if (cancel_msg != (char *) NULL) {
		xms = XmStringCreateLocalized(cancel_msg);
		XtVaSetValues(dialog, 
				 XmNcancelLabelString, xms,
				 NULL); 
		XmStringFree(xms);
	}


	/* Override the Help button label. */
	if (help_msg != (char *) NULL) {
		xms = XmStringCreateLocalized(help_msg);
		XtVaSetValues(dialog, 
				 XmNhelpLabelString, xms,
				 NULL); 
		XmStringFree(xms);
	}



	/* Now unmanage the buttons based on the btn_mask_out,
	 | and then manage the dialog.
	 */
	if (btn_mask_out & ERR_MASK_OK)
		XtUnmanageChild(XmMessageBoxGetChild(dialog, 
						     XmDIALOG_OK_BUTTON));
	if (btn_mask_out & ERR_MASK_CANCEL)
		XtUnmanageChild(XmMessageBoxGetChild(dialog,
						     XmDIALOG_CANCEL_BUTTON));
	if (btn_mask_out & ERR_MASK_HELP)
		XtUnmanageChild(XmMessageBoxGetChild(dialog,
						     XmDIALOG_HELP_BUTTON));
	XtManageChild(dialog);


	return(dialog);
}


/* This function returns a char * of the error message. */

char *
_SDtSpellLangToLocale(SPL_LANGUAGE lang)
{
	/* Set the spell locale. */
	switch(lang) {


/* Catalan turned off, since it is temporarily not supported, per Jean
 | Cunnington, since problems with it as a partial locale.
	case CA:
		return(LOCALE_CA);
		break;
 */


	case EN_AU:
		return(LOCALE_EN_AU);
		break;

	case EN_UK:
		return(LOCALE_EN_UK);
		break;

	case DE:
		return(LOCALE_DE);
		break;

	case DE_CH:
		return(LOCALE_DE_CH);
		break;

	case ES:
		return(LOCALE_ES);
		break;

	case FR:
		return(LOCALE_FR);
		break;

	case IT:
		return(LOCALE_IT);
		break;

	case SV:
		return(LOCALE_SV);
		break;

	case EN_US:
	case UNSUPPORTED:
	default:
		return(LOCALE_EN_US);
		break;
	}
}
