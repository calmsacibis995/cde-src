/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

#pragma ident "@(#)SpellOpt.c	1.16 96/09/24 SMI"


/*--------------------------------------------------------------
 |  General Notes:
 | 
 | This file contains code to handle creation and operation of the
 | spell widget's option dialog.  Spell-specific backend code has
 | been placed in options.c
 ---------------------------------------------------------------*/


/*--------------------------------------------------------------*/
/* 			INCLUDE FILES 				*/
/*--------------------------------------------------------------*/

#include <errno.h>
#include <nl_types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <unistd.h>


#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#endif				/* X_NOT_STDC_ENV */

#include <Xm/ArrowB.h>
#include <Xm/AtomMgr.h>
#include <Xm/DialogS.h>
#include <Xm/DragC.h>
#include <Xm/DropSMgr.h>
#include <Xm/FileSB.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/GadgetP.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/Protocols.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumnP.h>
#include <Xm/Scale.h>
#include <Xm/MessageB.h>
#include <Xm/Separator.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include <Xm/VendorSEP.h>

#include <SDt/SpellP.h>

#include "spellLib.h"
#include "globals.h"
#include "engine.h"
#include "list.h"
#include "misc_spell.h"
#include "defaults.h"
#include "help.h"


/* External function. */
extern char *_updateDict(SDtSpellSelectionBoxWidget);


/* CREATION FUNCTION */
Widget _SDtSpellCreateDictOptMenu(SDtSpellSelectionBoxWidget, 
				  LangMenuP *, Widget);


/* CALLBACKS */
void _SDtSpellOptionApply(Widget wid, XtPointer client, XtPointer call);
void _SDtSpellOptionOk(Widget wid, XtPointer client, XtPointer call);
void _SDtSpellOptionReset(Widget wid, XtPointer client, XtPointer call);
void _SDtSpellOptionClose(Widget wid, XtPointer client, XtPointer call);
void _SDtExportDictionary(Widget wid, XtPointer client, XtPointer call);
void _SDtImportDictionary(Widget wid, XtPointer client, XtPointer call);
void _SDtSpellSetCurrentLang(Widget wid, XtPointer client , XtPointer call);
void _SDtOptionWordListCB(Widget wid, XtPointer client , XtPointer call);
void _SDtRemoveWord(Widget wid, XtPointer client, XtPointer call);
void _SDtSpellDismissFSB(Widget, XtPointer, XtPointer);
void _SDtSpellCopyPersonal(Widget, XtPointer, XtPointer);
void _SDtSpellUpdatePersonal(Widget, XtPointer, XtPointer);


/* Other Functions */
Boolean		_SDtApply(SDtSpellSelectionBoxWidget widptr);
SPL_LANGUAGE	_SDtSpellSetDictMenuItem(Widget, LangMenuP, SPL_LANGUAGE);



/*-----------------------------------------------------------------
 |		Create Functions
 |-----------------------------------------------------------------*/


/* Function	:	_SDtSpellManageOptions()
 |
 | Objective	:	Manage the options window after the user has
 |			modified or reset the options.
 |
 | Arguments	:	SDtSpellSelectionBoxWidget wid
 |
 | Return value :	Boolean true if the options window and its children 
 |			are managed correctly, false otherwise.
 */

Boolean
_SDtSpellManageOptions(SDtSpellSelectionBoxWidget wid)
{
	char *error;


	/* No option panel present - how can we manage if it's not there! */
	if (SPELL_option_panel(wid) == NULL) 
		return(False);


	/* NO scrolling list widget present!  Same thing as above! */
	if (SPELL_pdList(wid) == NULL) 
		return(False);


	if (SPELL_deleteWords(wid) != NULL) {
        	_freeList(SPELL_deleteWords(wid));
		SPELL_deleteWords(wid) = NULL;
	}


	/* List out the personal dictionary. */
	error = _SDtSpellListPersonal((Widget) wid, (char *) NULL);
	if (error != NULL) {
		/* handle error */
		free(error);
		return(False);
	}


	/* Set the state of the toggle button. */
	XmToggleButtonSetState(SPELL_personal_toggle(wid),
			       SPELL_personal(wid), False);


	/* Set the state of the current dictionary menu. */
	SPELL_dict_used(wid) = 
		_SDtSpellSetDictMenuItem(SPELL_current_menu(wid),
					 SPELL_current_items(wid), 
					 _getLanguage(SPELL_dictionary(wid)));
	SPELL_dict_to_use(wid) = SPELL_dict_used(wid);



	/* Set the state of the default dictionary menu. */
	if (SPELL_default_dictionary(wid))
		_SDtSpellSetDictMenuItem(SPELL_default_menu(wid),
			SPELL_default_items(wid), 
			_getLanguage(SPELL_default_dictionary(wid)));
	else
		_SDtSpellSetDictMenuItem(SPELL_default_menu(wid),
					 SPELL_default_items(wid), 
					 _getLanguage(SPELL_dictionary(wid)));


	/* If followLocale is True, then set the right toggle. */
	if (SPELL_follow_locale(wid))
		XmToggleButtonGadgetSetState(SPELL_locale_toggle(wid), 
					     True, True);
	else
		XmToggleButtonGadgetSetState(SPELL_default_toggle(wid), 
					     True, True);


	/* Manage the window before returning. */
	XtManageChild(SPELL_option_panel(wid));	
	return(True);
}


/* Function	:	_SDtSpellSetDictMenuItem()
 |
 | Objective	:	Set the main dictionary options menu selection
 |			to the specified dictionary language item.
 |
 | Arguments	:	Widget		- the options menu widget
 |			LangMenuP	- dictionary language menu list
 | 			SPL_LANGUAGE	- dictionary language
 |
 | Return value	:	SPL_LANGUAGE.
 */

SPL_LANGUAGE
_SDtSpellSetDictMenuItem(Widget opt_menu, LangMenuP lang_menu, 
			 SPL_LANGUAGE language)
{
	LangMenuP	p = lang_menu;
	Widget		menu_position = (Widget) NULL;
	Widget		menu_default = (Widget) NULL;
	SPL_LANGUAGE	usedDict = EN_US;

	
	while (p != NULL) {

		/* Search until we find the right menu item for
		 | the specified dictionary language.
		 */
		if (p->lang == language) {
			menu_position = p->langItem;
			usedDict = p->lang;
		}


		/* Just in case the specified dictionary language doesn't
		 | have a menu item, default to the US English dictionary.
		 | This dictionary is guaranteed, since it is part of the
		 | base release.
		 */
		if (p->lang == EN_US)
			menu_default = p->langItem;


		/* Break out if we have got menu items for both the
		 | specified and default language dictionary.  Otherwise,
		 | continue on with the search until the list is exhausted.
		 */
		if (menu_position && menu_default)
			break;
		else
			p = p->next;
	}


	if (menu_position != (Widget) NULL)
		XtVaSetValues(opt_menu, 
			      XmNmenuHistory, menu_position,
			      NULL);
	else
		XtVaSetValues(opt_menu,
			      XmNmenuHistory, menu_default,
			      NULL);


	return(usedDict);
}


/* Function	:	_SDtSpellSelectOptionCategory()
 |
 | Objective	:	Xt callback to display the correct options form
 |			based on the item selected.
 */

void
_SDtSpellSelectOptionCategory(Widget wid, XtPointer client, XtPointer call)
{
	SDtSpellSelectionBoxWidget spell = (SDtSpellSelectionBoxWidget) client;
	XmString		   label;
	char			   *text;


	/* Get the label of the menu selected, and display the appropriate
	 | form based on the string retrieved.
	 */
	XtVaGetValues(wid, XmNlabelString, &label, NULL);
	XmStringGetLtoR(label, XmFONTLIST_DEFAULT_TAG, &text);
	if (strcmp(text, catgets(_spellCat, 2, 3, "Personal Dictionary"))==0) {
		XtUnmanageChild(SPELL_primary_dict_option(spell));
		XtManageChild(SPELL_personal_dict_option(spell));
	} else {
		XtUnmanageChild(SPELL_personal_dict_option(spell));
		XtManageChild(SPELL_primary_dict_option(spell));
	}

	XmStringFree(label);
	XtFree(text);
}


/* Function	:	_SDtSpellSetBehaviorToggles()
 |
 | Objective	:	Xt callback to set a toggle state to True (i.e.,
 |			selected), and unsets the toggle state of the
 |			other toggle to false.  This callback mimics
 |			the one-of-many behavior of normal radio buttons.
 |			This callback is a hack, in order to get the 
 |			proper layout of the primary dictionary options pane.
 |			Wanted an options menu right beside the radio
 |			button.  The bulletin board widget, which manages
 |			radio buttons, would not align the options menu
 |			right beside the radio, hence this hack - made the
 |			toggle buttons part of a form, so that we can
 |			attach the options menu right beside the follow
 |			default toggle.
 */

void
_SDtSpellSetBehaviorToggles(Widget wid, XtPointer client, XtPointer call)
{
	SDtSpellSelectionBoxWidget spell = (SDtSpellSelectionBoxWidget) client;


	if (wid == SPELL_locale_toggle(spell)) {

		XmToggleButtonGadgetSetState(SPELL_locale_toggle(spell), 
					     True, False);
		XmToggleButtonGadgetSetState(SPELL_default_toggle(spell), 
					     False, False);
		XtSetSensitive(SPELL_default_menu(spell), False);

	} else {

		XmToggleButtonGadgetSetState(SPELL_default_toggle(spell), 
					     True, False);
		XmToggleButtonGadgetSetState(SPELL_locale_toggle(spell),
					     False, False);
		XtSetSensitive(SPELL_default_menu(spell), True);
	}
}


/* Function	:	_SDtSpellCreateOptionsAction()
 |
 | Objective	:	Create the buttons for the action area of the
 |			options dialog window.  Set the label of the other
 |			buttons provided by the message dialog, which is
 |			the basis for the options dialog.
 |
 | Arguments	:	SDtSpellSelectionBoxWidget	- the spell widget
 |
 | Return Value	:	Boolean True if action buttons were created, false 
 |			otherwise.
 */

Boolean 
_SDtSpellCreateOptionsAction(SDtSpellSelectionBoxWidget wid)
{
	Arg                    arg_list[20];   /* Argument vector and count */
	int                    arg_num;
	Widget		       applyBtn;       /* Apply and reset buttons. */
	Widget		       resetBtn;
	XmString               xmstr;          /* Label for widgets. */


	/* Create the Apply button. */
        xmstr = XmStringCreateLocalized(catgets(_spellCat, 2, 9, "Apply"));
        arg_num = 0;
        XtSetArg(arg_list[arg_num], XmNlabelString, xmstr); arg_num++;
	applyBtn = XmCreatePushButton(SPELL_option_panel(wid), "applyButton",
				      arg_list, arg_num);
	XmStringFree(xmstr);
	if (applyBtn == (Widget) NULL)
		return(False);
	XtManageChild(applyBtn);
	XtAddCallback(applyBtn, XmNactivateCallback, _SDtSpellOptionApply,
		      (XtPointer) wid);


	/* Add the Reset button. */
        xmstr = XmStringCreateLocalized(catgets(_spellCat, 2, 10, "Reset"));
        arg_num = 0;
        XtSetArg(arg_list[arg_num], XmNlabelString, xmstr); arg_num++;
	resetBtn = XmCreatePushButton(SPELL_option_panel(wid), "resetButton",
				      arg_list, arg_num);
	XmStringFree(xmstr);
	if (resetBtn == (Widget) NULL)
		return(False);
	XtManageChild(resetBtn);
	XtAddCallback(resetBtn, XmNactivateCallback, _SDtSpellOptionReset,
		      (XtPointer) wid);


	/* Set the OK button label. */
        xmstr = XmStringCreateLocalized(catgets(_spellCat, 3, 13, "OK"));
        arg_num = 0;
        XtSetArg(arg_list[arg_num], XmNokLabelString, xmstr); arg_num++;
	XtSetValues(SPELL_option_panel(wid), arg_list, arg_num);
	XmStringFree(xmstr);
	XtAddCallback(SPELL_option_panel(wid), XmNokCallback, 
		      _SDtSpellOptionOk, (XtPointer) wid);


	/* Set the Cancel button label. */
        xmstr = XmStringCreateLocalized(catgets(_spellCat, 2, 11, "Cancel"));
        arg_num = 0;
        XtSetArg(arg_list[arg_num], XmNcancelLabelString, xmstr); arg_num++;
	XtSetValues(SPELL_option_panel(wid), arg_list, arg_num);
	XmStringFree(xmstr);
	XtAddCallback(SPELL_option_panel(wid), XmNcancelCallback, 
		      _SDtSpellOptionClose, 
		      (XtPointer) SPELL_option_panel(wid));


	/* Set the Help button label. */
        xmstr = XmStringCreateLocalized(catgets(_spellCat, 1, 11, "Help"));
        arg_num = 0;
        XtSetArg(arg_list[arg_num], XmNhelpLabelString, xmstr); arg_num++;
	XtSetValues(SPELL_option_panel(wid), arg_list, arg_num);
	XmStringFree(xmstr);

	return(True);
}


/* Function	:	_SDtSpellCreateCategory()
 |
 | Objective	:	Create the category options menu and separator.
 |
 | Arguments	:	SDtSpellSelectionBoxWidget   - the spell widget
 |			Widget			     - parent of category
 |						       form
 |
 | Return Value	:	True if the options menu and separator were created,
 |			False otherwise.
 */

Boolean 
_SDtSpellCreateCategory(SDtSpellSelectionBoxWidget wid, Widget parent)
{
	Arg             arg_list[20];    /* Argument vector and count */
	int             arg_num;
	Widget		categOptionsMenu;
	Widget		menu;
	Widget		button;
	Widget		separator;
	XmString        xmstr;           /* Label for widgets. */


	/* Create the pull-down menu that will be used as the submenu
	 | of the category options menu.
	 */
	menu = XmCreatePulldownMenu(parent, "categOptionMenu", NULL, 0);
	if (menu == (Widget) NULL)
		return(False);


	/* Create the personal dictionary menu button. */
	xmstr = XmStringCreateLocalized(
			catgets(_spellCat, 2, 3, "Personal Dictionary"));
	arg_num = 0;
	XtSetArg(arg_list[arg_num], XmNlabelString, xmstr); arg_num ++;
	button = XmCreatePushButtonGadget(menu, "personal", arg_list, arg_num);
	XmStringFree(xmstr);
	if (button == (Widget) NULL)
		return(False);
	XtAddCallback(button, XmNactivateCallback, 
		      _SDtSpellSelectOptionCategory, (XtPointer) wid);
	XtManageChild(button);


	/* Create the primary, i.e., main dictionary menu button. */
	xmstr = XmStringCreateLocalized(
			catgets(_spellCat, 2, 4, "Main Dictionary"));
	arg_num = 0;
	XtSetArg(arg_list[arg_num], XmNlabelString, xmstr); arg_num ++;
	button = XmCreatePushButtonGadget(menu, "primary", arg_list, arg_num);
	XmStringFree(xmstr);
	if (button == (Widget) NULL)
		return(False);
	XtAddCallback(button, XmNactivateCallback, 
		      _SDtSpellSelectOptionCategory, (XtPointer) wid);
	XtManageChild(button);


	/* Now create the category options menu. */
	xmstr = XmStringCreateLocalized(
			catgets(_spellCat, 2, 2, "Category:"));
	arg_num = 0;
	XtSetArg(arg_list[arg_num], XmNlabelString, xmstr); arg_num ++;
	XtSetArg(arg_list[arg_num], XmNsubMenuId, menu); arg_num ++;
        XtSetArg(arg_list[arg_num], XmNtopAttachment, XmATTACH_FORM);  
	++arg_num;
        XtSetArg(arg_list[arg_num], XmNleftAttachment, XmATTACH_FORM);
	++arg_num;
        XtSetArg(arg_list[arg_num], XmNrightAttachment, XmATTACH_FORM);
	++arg_num;
	categOptionsMenu =
		XmCreateOptionMenu(parent, "categOptionsMenu", 
				   arg_list, arg_num);
	XmStringFree(xmstr);
	if (categOptionsMenu == (Widget) NULL)
		return(False);
	XtManageChild(categOptionsMenu);


	/* Now create the separator that lies beneath the category
	 | options menu.
	 */
        XtSetArg(arg_list[arg_num], XmNtopAttachment, XmATTACH_WIDGET);  
	++arg_num;
        XtSetArg(arg_list[arg_num], XmNtopWidget, categOptionsMenu);  
	++arg_num;
        XtSetArg(arg_list[arg_num], XmNtopOffset, 10);  
	++arg_num;
        XtSetArg(arg_list[arg_num], XmNleftAttachment, XmATTACH_FORM);
	++arg_num;
        XtSetArg(arg_list[arg_num], XmNrightAttachment, XmATTACH_FORM);
	++arg_num;
	separator = XmCreateSeparator(parent, "categorySeparator",
				      arg_list, arg_num);
	if (separator == (Widget) NULL)
		return(False);
	XtManageChild(separator);

	return(True);
}


/* Function	:	_SDtSpellCreatePersonal()
 |
 | Objective	:	Create the form that will display all options
 |			that affect the user's personal dictionary.
 |
 | Arguments	:	SDtSpellSelectionBoxWidget   - the spell widget
 |			Widget			     - parent of the 
 |						       personal form
 |
 | Return Value	:	Boolean True if the personal form and its children
 |			are created, false otherwise.
 */

Boolean 
_SDtSpellCreatePersonal(SDtSpellSelectionBoxWidget wid, Widget parent)
{
	Arg             arg_list[20];    /* Argument vector and count */
	int             arg_num;
	XmString        xmstr;           /* Label for widgets. */
	Widget          personalLabel;
	Widget		removeBtn;
	Widget		importBtn;
	Widget		exportBtn;
	Widget		separator;


	/* Create a form as a child of the work area. */
	separator = XtNameToWidget(parent, "*categorySeparator");
	if (separator == (Widget) NULL)
		return(False);
	arg_num = 0;
	XtSetArg(arg_list[arg_num], XmNtopAttachment, XmATTACH_WIDGET); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNtopWidget, separator); arg_num++;
	XtSetArg(arg_list[arg_num], XmNleftAttachment, XmATTACH_FORM); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNrightAttachment, XmATTACH_FORM); 
	arg_num++;
	SPELL_personal_dict_option(wid) = 
		XmCreateForm(parent, "categOptionsForm", arg_list, arg_num);
	if (SPELL_personal_dict_option(wid) == (Widget) NULL)
		return(False);


	/* Create the label for the scrolling list of words in
	 | the user's personal dictionary.
         */
	arg_num = 0;
	xmstr = XmStringCreateLocalized(
		catgets(_spellCat, 2, 5, "Words in personal dictionary:"));
	XtSetArg(arg_list[arg_num], XmNlabelString, xmstr); arg_num++;
	XtSetArg(arg_list[arg_num], XmNtopAttachment, XmATTACH_FORM); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNtopOffset, 15); arg_num++;
	XtSetArg(arg_list[arg_num], XmNleftAttachment, XmATTACH_FORM); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNleftOffset, 15); arg_num++;
	XtSetArg(arg_list[arg_num], XmNrecomputeSize, False); arg_num++;
	personalLabel =
		XmCreateLabel(SPELL_personal_dict_option(wid), "customLabel", 
			      arg_list, arg_num);
	XmStringFree(xmstr);
	if (personalLabel == (Widget) NULL)
		return(False);
	XtManageChild(personalLabel);


	/* Create the scrolling list that will hold the contents of
	 | the user's personal dictionary.
	 */
	arg_num = 0;
	XtSetArg(arg_list[arg_num], XmNtopAttachment, XmATTACH_WIDGET); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNtopWidget, personalLabel); arg_num++;
	XtSetArg(arg_list[arg_num], XmNleftAttachment, XmATTACH_FORM); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNleftOffset, 15); arg_num++;
	XtSetArg(arg_list[arg_num], XmNrightAttachment, 
		 XmATTACH_OPPOSITE_WIDGET); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNrightWidget, personalLabel); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNrecomputeSize, False); arg_num++;
	XtSetArg(arg_list[arg_num], XmNlistSizePolicy, XmCONSTANT);
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNselectionPolicy, XmEXTENDED_SELECT); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNvisibleItemCount, 8); arg_num++;
	XtSetArg(arg_list[arg_num], XmNscrollBarDisplayPolicy, XmAS_NEEDED); 
	arg_num++;
	SPELL_pdList(wid) =
		XmCreateScrolledList(SPELL_personal_dict_option(wid),
				     "personalDictList",
				     arg_list, arg_num);
	if (SPELL_pdList(wid) == (Widget) NULL)
		return(False);
	XtAddCallback(SPELL_pdList(wid), XmNextendedSelectionCallback,
		      _SDtOptionWordListCB, (XtPointer) wid);
	XtManageChild(SPELL_pdList(wid));


	/* Create the Remove button. */
	xmstr = XmStringCreateLocalized(catgets(_spellCat, 2, 6, "Remove"));
	arg_num = 0;
	XtSetArg(arg_list[arg_num], XmNlabelString, xmstr); arg_num++;
	XtSetArg(arg_list[arg_num], XmNtopAttachment, XmATTACH_WIDGET); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNtopWidget, personalLabel); arg_num++;
	XtSetArg(arg_list[arg_num], XmNleftAttachment, XmATTACH_WIDGET); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNleftWidget, SPELL_pdList(wid)); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNleftOffset, 20);
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNrightAttachment, XmATTACH_FORM); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNrightOffset, 15); arg_num++;
	XtSetArg(arg_list[arg_num], XmNrecomputeSize, False); arg_num++;
	XtSetArg(arg_list[arg_num], XmNalignment, XmALIGNMENT_CENTER);
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNsensitive, False); arg_num++;
	removeBtn = XmCreatePushButton(SPELL_personal_dict_option(wid), 
				       "removeBtn", arg_list, arg_num);
	XmStringFree(xmstr);
	if (removeBtn == (Widget) NULL)
		return(False);
	XtAddCallback(removeBtn, XmNactivateCallback, _SDtRemoveWord, 
		      (XtPointer) wid);
	XtManageChild(removeBtn);


	/* Create the import button. */
	xmstr = XmStringCreateLocalized(catgets(_spellCat, 2, 7, "Import..."));
	arg_num = 0;
	XtSetArg(arg_list[arg_num], XmNlabelString, xmstr); arg_num++;
	XtSetArg(arg_list[arg_num], XmNtopAttachment, XmATTACH_WIDGET); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNtopWidget, removeBtn); arg_num++;
	XtSetArg(arg_list[arg_num], XmNtopOffset, 10); arg_num++;
	XtSetArg(arg_list[arg_num], XmNleftAttachment, XmATTACH_WIDGET); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNleftWidget, SPELL_pdList(wid)); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNleftOffset, 20); arg_num++;
	XtSetArg(arg_list[arg_num], XmNrightAttachment, XmATTACH_FORM); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNrightOffset, 15); arg_num++;
	XtSetArg(arg_list[arg_num], XmNrecomputeSize, False); arg_num++;
	XtSetArg(arg_list[arg_num], XmNalignment, XmALIGNMENT_CENTER);
	arg_num++;
	importBtn = XmCreatePushButton(SPELL_personal_dict_option(wid), 
				       "importBtn", arg_list, arg_num);
	XmStringFree(xmstr);
	if (importBtn == (Widget) NULL)
		return(False);
	XtManageChild(importBtn);
	XtAddCallback(importBtn, XmNactivateCallback, _SDtImportDictionary, 
		      (XtPointer) SPELL_option_panel(wid));


	/* Create the export button. */
	xmstr = XmStringCreateLocalized(catgets(_spellCat, 2, 8, "Export..."));
	arg_num = 0;
	XtSetArg(arg_list[arg_num], XmNlabelString, xmstr); arg_num++;
	XtSetArg(arg_list[arg_num], XmNtopAttachment, XmATTACH_WIDGET);
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNtopWidget, importBtn); arg_num++;
	XtSetArg(arg_list[arg_num], XmNtopOffset, 10); arg_num++;
	XtSetArg(arg_list[arg_num], XmNleftAttachment, XmATTACH_WIDGET);
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNleftWidget, SPELL_pdList(wid));
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNleftOffset, 20); arg_num++;
	XtSetArg(arg_list[arg_num], XmNrightAttachment, XmATTACH_FORM);
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNrightOffset, 15); arg_num++;
	XtSetArg(arg_list[arg_num], XmNrecomputeSize, False); arg_num++;
	XtSetArg(arg_list[arg_num], XmNalignment, XmALIGNMENT_CENTER); 
	arg_num++;
	exportBtn = XmCreatePushButton(SPELL_personal_dict_option(wid), 
				       "exportBtn", arg_list, arg_num);
	XmStringFree(xmstr);
	if (exportBtn == (Widget) NULL)
		return(False);
	XtManageChild(exportBtn);
	XtAddCallback(exportBtn, XmNactivateCallback, _SDtExportDictionary,
		      (XtPointer) wid);


	/* Use personal dictionary toggle widget */
	xmstr = XmStringCreateLocalized(
			catgets(_spellCat, 2, 31, "Use Personal Dictionary"));
	arg_num = 0;
	XtSetArg(arg_list[arg_num], XmNtopAttachment, XmATTACH_WIDGET);
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNtopWidget, SPELL_pdList(wid));
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNtopOffset, 10); arg_num++;
	XtSetArg(arg_list[arg_num], XmNbottomAttachment, XmATTACH_FORM);
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNleftAttachment, XmATTACH_FORM);
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNleftOffset, 15); arg_num++;
	XtSetArg(arg_list[arg_num], XmNrecomputeSize, False); arg_num++;
	XtSetArg(arg_list[arg_num], XmNlabelString, xmstr); arg_num++;
	XtSetArg(arg_list[arg_num], XmNset, SPELL_personal(wid)); arg_num++;
	SPELL_personal_toggle(wid) = 
		XmCreateToggleButton(SPELL_personal_dict_option(wid), 
				     "usePersonalToggle", arg_list, arg_num);
	XmStringFree(xmstr);
	if (SPELL_personal_toggle(wid) == NULL)
		return(False);
	XtManageChild(SPELL_personal_toggle(wid));
	XtManageChild(SPELL_personal_dict_option(wid));


	return(True);
}


/* Function	:	_SDtSpellCreatePrimary()
 |
 | Objective	:	Create the form that will display all options
 |			that affect the user's primary, i.e., main
 |			dictionary.
 |
 | Arguments	:	SDtSpellSelectionBoxWidget   - the spell widget
 |			Widget			     - parent of the 
 |						       personal form
 |
 | Return Value	:	Boolean True if the primary form and its children
 |			are created, false otherwise.
 */

Boolean 
_SDtSpellCreatePrimary(SDtSpellSelectionBoxWidget wid, Widget parent)
{
	Arg             arg_list[20];    /* Argument vector and count */
	int             arg_num;
	XmString        xmstr;           /* Label for widgets. */
	Widget          separator;
	Widget		label;
	LangMenuP	p;               /* Traverse the menu items list. */


	/* Create a form as a child of the work area. */
	separator = XtNameToWidget(parent, "*categorySeparator");
	if (separator == (Widget) NULL)
		return(False);
	arg_num = 0;
	XtSetArg(arg_list[arg_num], XmNtopAttachment, XmATTACH_WIDGET); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNtopWidget, separator); arg_num++;
	XtSetArg(arg_list[arg_num], XmNleftAttachment, XmATTACH_FORM); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNrightAttachment, XmATTACH_FORM); 
	arg_num++;
	SPELL_primary_dict_option(wid) = 
		XmCreateForm(parent, "categOptionsForm", arg_list, arg_num);
	if (SPELL_primary_dict_option(wid) == (Widget) NULL)
		return(False);


	/* Now create the main dictionary options menu, and select the
	 | appropriate language menu item, as defined by the
	 | SPELL_dictionary(wid) variable.
	 */
	SPELL_current_menu(wid) = 
		_SDtSpellCreateDictOptMenu(wid, &SPELL_current_items(wid),
					   SPELL_primary_dict_option(wid));
	if (SPELL_current_menu(wid) == NULL)
		return(False);
	xmstr = XmStringCreateLocalized(
			catgets(_spellCat, 2, 29, "Current Dictionary:"));
	arg_num = 0;
	XtSetArg(arg_list[arg_num], XmNlabelString, xmstr);arg_num++;
	XtSetArg(arg_list[arg_num], XmNtopAttachment, XmATTACH_FORM);
	++arg_num;
	XtSetArg(arg_list[arg_num], XmNtopOffset, 15); arg_num++;
	XtSetArg(arg_list[arg_num], XmNleftAttachment, XmATTACH_FORM);
	++arg_num;
	XtSetArg(arg_list[arg_num], XmNrightAttachment, XmATTACH_FORM);
	++arg_num;
	XtSetValues(SPELL_current_menu(wid), arg_list, arg_num);
	XmStringFree(xmstr);
	SPELL_dict_used(wid) = 
		_SDtSpellSetDictMenuItem(SPELL_current_menu(wid),
					 SPELL_current_items(wid), 
					 _getLanguage(SPELL_dictionary(wid)));
	for (p = SPELL_current_items(wid); p != NULL; p = p->next)
		XtAddCallback(p->langItem, XmNactivateCallback,
			      _SDtSpellSetCurrentLang, (XtPointer) wid);


	/* Create the label. */
	xmstr = XmStringCreateLocalized(
		catgets(_spellCat, 2, 32, "When Spelling Checker Opens:"));
	arg_num = 0;
	XtSetArg(arg_list[arg_num], XmNlabelString, xmstr); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNtopAttachment, XmATTACH_WIDGET); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNtopWidget, SPELL_current_menu(wid)); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNtopOffset, 30); arg_num++;
	XtSetArg(arg_list[arg_num], XmNleftAttachment, XmATTACH_FORM); 
	arg_num++;
	label = XmCreateLabel(SPELL_primary_dict_option(wid), 
			      "spellOpensLabel", arg_list, arg_num);
	XmStringFree(xmstr);
	if (label == (Widget) NULL)
		return(False);
	XtManageChild(label);


	/* Create the follow locale button. */
	xmstr = XmStringCreateLocalized(catgets(_spellCat, 2, 33, "Set Dictionary To Match Locale Language"));
	arg_num = 0;
	XtSetArg(arg_list[arg_num], XmNlabelString, xmstr); arg_num++;
	XtSetArg(arg_list[arg_num], XmNindicatorType, XmONE_OF_MANY); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNtopAttachment, XmATTACH_WIDGET); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNtopWidget, label); arg_num++;
	arg_num++;
	SPELL_locale_toggle(wid) = 
		XmCreateToggleButtonGadget(SPELL_primary_dict_option(wid),
					   "followLocaleToggle", 
					   arg_list, arg_num);

	XmStringFree(xmstr);
	if (SPELL_locale_toggle(wid) == (Widget) NULL)
		return(False);
	XtManageChild(SPELL_locale_toggle(wid));
	XtAddCallback(SPELL_locale_toggle(wid), XmNvalueChangedCallback,
		      _SDtSpellSetBehaviorToggles, (XtPointer) wid);


	/* Create the follow default toggle button. */
	xmstr = XmStringCreateLocalized(catgets(_spellCat, 2, 34, "Set Dictionary To:"));
	arg_num = 0;
	XtSetArg(arg_list[arg_num], XmNlabelString, xmstr); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNindicatorType, XmONE_OF_MANY); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNtopAttachment, XmATTACH_WIDGET); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNtopWidget, SPELL_locale_toggle(wid));
	arg_num++;
	SPELL_default_toggle(wid) = 
		XmCreateToggleButtonGadget(SPELL_primary_dict_option(wid),
					   "followDefaultToggle", 
					   arg_list, arg_num);
	XmStringFree(xmstr);
	if (SPELL_default_toggle(wid) == (Widget) NULL)
		return(False);
	XtManageChild(SPELL_default_toggle(wid));
	XtAddCallback(SPELL_default_toggle(wid), XmNvalueChangedCallback,
		      _SDtSpellSetBehaviorToggles, (XtPointer) wid);


	/* Create the default dictionary options menu. */
	SPELL_default_menu(wid) = 
		 _SDtSpellCreateDictOptMenu(wid, &SPELL_default_items(wid), 
					    SPELL_primary_dict_option(wid));
	if (SPELL_default_menu(wid) == NULL)
		return(False);
	arg_num = 0;
	XtSetArg(arg_list[arg_num], XmNtopAttachment, XmATTACH_WIDGET);
	++arg_num;
	XtSetArg(arg_list[arg_num], XmNtopWidget, SPELL_locale_toggle(wid)); 
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNleftAttachment, XmATTACH_WIDGET);
	++arg_num;
	XtSetArg(arg_list[arg_num], XmNleftWidget, SPELL_default_toggle(wid)); 
	++arg_num;
	XtSetArg(arg_list[arg_num], XmNrightAttachment, XmATTACH_FORM);
	++arg_num;
	XtSetValues(SPELL_default_menu(wid), arg_list, arg_num);
	if (SPELL_default_dictionary(wid))
		_SDtSpellSetDictMenuItem(SPELL_default_menu(wid),
			SPELL_default_items(wid), 
			_getLanguage(SPELL_default_dictionary(wid)));
	else
		_SDtSpellSetDictMenuItem(SPELL_default_menu(wid),
					 SPELL_default_items(wid), 
					 _getLanguage(SPELL_dictionary(wid)));


	/* Set the appropriate behavior toggle. */
	if (SPELL_follow_locale(wid)) {

		XmToggleButtonGadgetSetState(SPELL_locale_toggle(wid),
					     True, False);
		XtSetSensitive(SPELL_default_menu(wid), False);
	} else {

		XmToggleButtonGadgetSetState(SPELL_default_toggle(wid),
					     True, False);
		XtSetSensitive(SPELL_default_menu(wid), True);
	}

	return(True);
}


/* Function	:	_SDtSpellCreateOptionsDlg()
 |
 | Objective	:	Create the options dialog window and display it.
 |
 | Arguments	:	SDtSpellSelectionBoxWidget	- the spell widget
 |
 | Return Value	:	Boolean True if dialog was created, false otherwise.
 */

Boolean 
_SDtSpellCreateOptionsDlg(SDtSpellSelectionBoxWidget wid)
{

	String              title;	     /* Title bar string. */
	Arg                 arg_list[20];    /* Argument vector and count */
	int                 arg_num;
	Widget              tmpwid;	     /* Temporary widget variable. */
	Widget		    workArea;	     /* Options dialog form widget. */
	Widget		    category;	     /* Category form. */
	XmString            xmstr;           /* Label for widgets. */
	static SpellHelpR   helpItem;


	/* Create the options dialog associated with the spell checker
	 | widget.  Use the motif selection dialog as the basis.
	 */
	arg_num = 0;
	title = XtNewString(
			catgets(_spellCat, 2, 1, "Spelling Checker : Options"));
	XtSetArg(arg_list[arg_num], XmNallowShellResize, True);
	++arg_num;
	XtSetArg(arg_list[arg_num], XmNtitle, title);
	++arg_num;
	XtSetArg(arg_list[arg_num], XmNnoResize, True);
	++arg_num;
	XtSetArg(arg_list[arg_num], XmNautoUnmanage, False);
	++arg_num;
	XtSetArg(arg_list[arg_num], XmNdeleteResponse, XmUNMAP);
	++arg_num;
	SPELL_option_panel(wid) = 
		XmCreateMessageDialog((Widget) wid, "sdtspellOptions",
				      arg_list, arg_num);
	XtFree(title);
	if (SPELL_option_panel(wid) == (Widget) NULL)
		return (False);


	/* Hide unnecessary items that are not needed, but are provided
	 | by the message dialog.
	 */
	tmpwid = XmMessageBoxGetChild(SPELL_option_panel(wid), 
				      XmDIALOG_SYMBOL_LABEL);
	XtUnmanageChild(tmpwid);

	tmpwid = XmMessageBoxGetChild(SPELL_option_panel(wid), 
				      XmDIALOG_MESSAGE_LABEL);
	XtUnmanageChild(tmpwid);


	/* Activate the help callback for this dialog. */
	helpItem.help_parent_win = (Widget) wid;
	helpItem.help_location_id = SDTSPELL_HELP_OPTIONS;
	XtAddCallback(SPELL_option_panel(wid), XmNhelpCallback,
		      _SDtSpellHelpCB, (XtPointer) &helpItem);


	/* Create the work area */
	workArea = XmCreateForm(SPELL_option_panel(wid), "optionForm",
				NULL, 0);


	/* Set the action area buttons, i.e., OK, Apply, Reset, etc. */
	if (!_SDtSpellCreateOptionsAction(wid))
		return(False);


	/* Create the category area. */
	if (!_SDtSpellCreateCategory(wid, workArea))
		return(False);


	/* Create the personal dictionary form. */
	if (!_SDtSpellCreatePersonal(wid, workArea))
		return(False);


	/* Create the primary dictionary form. */
	if (!_SDtSpellCreatePrimary(wid, workArea))
		return(False);


	/* Manage the form, and return. */
	XtManageChild(workArea);
	return(True);
}


/* Function	:	_SDtSpellCreateDictOptMenu()
 |
 | Objective	:	Create the the options menu that lists out the
 |			available language dictionaries.
 |
 | Arguments	:	SDtSpellSelectionBoxWidget - spell widget
 |			LangMenuP                  - the menu items list
 |			Widget	                   - parent of the options menu
 |
 | Return value :	Returns the widget created.
 */

Widget
_SDtSpellCreateDictOptMenu(SDtSpellSelectionBoxWidget spell, 
			   LangMenuP *lang_items, Widget parent)
{
	Arg		arg_list[20];	/* Argument vector and count */
	int		arg_num;
	Widget		menu;		/* Menu for the options menu. */
	Widget		dict_opt;	/* The options menu to return. */


	/* Create the pull down menu that will display all the languages
	 | available to the user.
	 */
	menu = XmCreatePulldownMenu(parent, "languagesMenu", NULL, 0);

	
	/* Populate the menu that was just created with the available
	 | language dictionaries.
	 */
	*lang_items = _SDtSpellGetAvailDicts(SPELL_engine(spell), menu);
	if (*lang_items == (LangMenuP) NULL)
		XtSetSensitive(menu, False);


	/* Create the main dictionary options menu and the pulldown
	 | menu and the associated pull down menu.
	 */
	arg_num = 0;
	XtSetArg(arg_list[arg_num], XmNsubMenuId, menu); arg_num++;
	dict_opt = XmCreateOptionMenu(parent, "dictionaryOptionsMenu", 
				      arg_list, arg_num);
	if (dict_opt == NULL)
		return((Widget) NULL);
	XtManageChild(dict_opt);


	return(dict_opt);
}


/*--------------------------------------------------------------*/
/* 			CALLBACKS 				*/
/*--------------------------------------------------------------*/


/* Function	:	_SDtSpellOptionApply()
 |
 | Objective	:	Xt callback to set the properties of the spell
 |			checker, based on the options chosen by the user.
 |			when apply is activated
 */

void 
_SDtSpellOptionApply(Widget wid, XtPointer client, XtPointer call)
{
	SDtSpellSelectionBoxWidget widptr = (SDtSpellSelectionBoxWidget) client;

	_SDtApply(widptr);
}

/* Function	:	_SDtSpellOptionOk()
 |
 | Objective	:	Xt callback to set the properties of the spell
 |			checker, based on the options chosen by the user.
 |			when apply is activated
 */

void 
_SDtSpellOptionOk(Widget wid, XtPointer client, XtPointer call)
{
	SDtSpellSelectionBoxWidget widptr = (SDtSpellSelectionBoxWidget) client;

	/* close the panel unless there was an error applying the change */
	if (_SDtApply(widptr))
		XtUnmanageChild(SPELL_option_panel(widptr));
}


/* Function	:	_SDtSpellOptionReset()
 |
 | Objective    :       Xt callback that resets the Options dialog
 |			to its initial state, i.e., before user started
 |			playing around with settings.
 */
void 
_SDtSpellOptionReset(Widget wid, XtPointer client, XtPointer call)
{
	SDtSpellSelectionBoxWidget spell = (SDtSpellSelectionBoxWidget) client;

	_SDtSpellManageOptions(spell);
}


/* Function	:	_SDtSpellOptionClose()
 |
 | Objective	:	Xt callback to dismiss the Options dialog
 | 			window associated with the spell checker.
 */

void 
_SDtSpellOptionClose(Widget wid, XtPointer client, XtPointer call)
{
	Widget	optionsDlg = (Widget) client;

	XtUnmanageChild(optionsDlg);
}


/* Function	:	_SDtRemoveWord()
 |
 | Objective	:	Xt callback called when remove is 
 |			selected to delete a word.
 */

void 
_SDtRemoveWord(Widget wid, XtPointer client, XtPointer call)
{
	/* Get the structure containg text widget, spell GUI and engine. */
	SDtSpellSelectionBoxWidget spell = (SDtSpellSelectionBoxWidget) client;


	/* The items selected, and the number of items selected,
	 | and the positions of the selected items.
	 */
	XmStringTable	select_items = (XmStringTable) NULL;
	int		select_count;
	int		*select_pos;

	Widget remove = XtNameToWidget(SPELL_option_panel(spell), "*removeBtn");

	char	*word = (char *) NULL;	/* Word to delete. */
	int	i;			/* General counter. */


	/* Get the selected items from the list. */
	XtVaGetValues(SPELL_pdList(spell),
		      XmNselectedItemCount, &select_count,
		      XmNselectedItems, &select_items,
		      NULL);


	/* Get the selected items' positions. */
	XmListGetSelectedPos(SPELL_pdList(spell),
			     &select_pos, &select_count);


	/* This should not happen, but just in case the GUI ends up in
	 | some weird state where the count is 0, just exit.
	 */
	if (select_count == 0)
		return;


	/* For each item in the list, delete it from the personal
	 | dictionary.
	 */
	for (i = 0; i < select_count; i++)
	{
		/* Get the word, and add it to the delete list. */
		XmStringGetLtoR(select_items[i], XmSTRING_DEFAULT_CHARSET, 
				&word);
		SPELL_deleteWords(spell) = _addToList(SPELL_deleteWords(spell),
				 word);
		XtFree(word);
	}


	/* Now, delete the selected items from the list. */
	XmListDeletePositions(SPELL_pdList(spell),
			      select_pos, select_count);
	free(select_pos);


	/* Toggle the sensitivity of the Remove button. */
	XtSetSensitive(remove, False);
}

/* Function	:	_SDtExportDictionary()
 |
 | Objective	:	Xt callback called when export is 
 |			selected to export a personal dictionary
 */
void
_SDtExportDictionary(Widget wid, XtPointer client, XtPointer call)
{
	SDtSpellSelectionBoxWidget spell = (SDtSpellSelectionBoxWidget) client;


	Widget                 fsb = (Widget) NULL; /* File selection box. */
	Widget	               parent;
	Arg                    arg_list[5];    /* Argument vector & count. */
	int                    arg_num = 0;
	char                  *title;          /* Title of FSB. */
	static SpellHelpR      helpItem;


	/* Make the file selection box a child of the options dialog,
	 | optionsPanel, and bring it up.
	 */
	parent = XtParent(XtParent(XtParent(XtParent(XtParent(wid)))));
	title = XtNewString(
			catgets(_spellCat, 4, 3, "Spelling Checker : Export"));
	XtSetArg(arg_list[arg_num], XmNtitle, title); arg_num++;
	fsb = XmCreateFileSelectionDialog(parent, "exportFiles",
					  arg_list, arg_num);
	XtFree(title);
	if (fsb == (Widget) NULL)
		return;


	/* Add callbacks to the OK, Cancel, and Help buttons of
	 | the file selection box.
	 */
	XtAddCallback(fsb, XmNcancelCallback, _SDtSpellDismissFSB, 
		      (XtPointer) NULL);
	XtAddCallback(fsb, XmNokCallback, _SDtSpellCopyPersonal,
		      (XtPointer) spell);


	/* For the help widget. */
	helpItem.help_parent_win = (Widget) spell;
	helpItem.help_location_id = SDTSPELL_HELP_EXPORT;
	XtAddCallback(fsb, XmNhelpCallback,
		      _SDtSpellHelpCB, (XtPointer) &helpItem);


	XtManageChild(fsb);
}


/* Function	:	_SDtImportDictionary()
 |
 | Objective	:	Xt callback called when import is 
 |			selected to import a personal dictionary
 */
void
_SDtImportDictionary(Widget wid, XtPointer client, XtPointer call)
{
	Widget		      fsb = (Widget) NULL;
	Widget		      optionsDlg = (Widget) client;
	Arg		      arg_list[5];
	int		      arg_num = 0;
	char		      *title;
	static SpellHelpR      helpItem;


	/* Make the file selection box a child of the options dialog,
	 | and bring it up.
	 */
	title = XtNewString(catgets(_spellCat, 4, 4, "Spelling Checker : Import"));
	XtSetArg(arg_list[arg_num], XmNtitle, title); arg_num++;
	fsb = XmCreateFileSelectionDialog(optionsDlg, "importFiles",
					  arg_list, arg_num);
	XtFree(title);
	if (fsb == (Widget) NULL)
		return;


	/* Add callbacks to the OK, Cancel, and Help buttons of
	 | the file selection box.
	 */
	XtAddCallback(fsb, XmNcancelCallback, _SDtSpellDismissFSB, 
		      (XtPointer) NULL);
	XtAddCallback(fsb, XmNokCallback, _SDtSpellUpdatePersonal,
		      (XtPointer) optionsDlg);


	/* For the help widget. */
	helpItem.help_parent_win = XtParent(optionsDlg);
	helpItem.help_location_id = SDTSPELL_HELP_IMPORT;
	XtAddCallback(fsb, XmNhelpCallback,
		      _SDtSpellHelpCB, (XtPointer) &helpItem);


	XtManageChild(fsb);
}


/* Function	:	_SDtSpellDismissFSB()
 |
 | Objective	:	Xt callback called when the Cancel button
 |			in the file selection box is selected.
 */

void
_SDtSpellDismissFSB(Widget wid, XtPointer client, XtPointer call)
{
	/* Unmanage and destory the file selection box. */
	XtUnmanageChild(wid);
	XtDestroyWidget(wid);
}


/* Function	:	_SDtSpellOverWrite()
 |
 | Objective	:	Xt callback called when user selects Overwrite
 |			when exporting the personal dictionary to a
 |			file that already exists.
 */

void
_SDtSpellOverWrite(Widget wid, XtPointer client, XtPointer call)
{
	char	*file = (char *) client;
	Widget	 spell = XtParent(wid);

	_SDtSpellListPersonal(spell, file);
	XtRemoveCallback(wid, XmNokCallback, _SDtSpellOverWrite, file);
	XtFree(file);
}


/* Function	:	_SDtSpellCopyPersonal()
 |
 | Objective	:	Xt callback called when the user has selected
 |			OK from the Export file selection box.
 */

void
_SDtSpellCopyPersonal(Widget wid, XtPointer client, XtPointer call)
{
	SDtSpellSelectionBoxWidget spell = (SDtSpellSelectionBoxWidget) client;

	char		*file;		  /* File to save copy. */
	struct stat	buffer;           /* For check if file exists. */
	Widget		parent, err_dlg;
	int		size;             /* Size of strings. */


	/* Error dialog labels and strings, and button mask. */
	char           *err_dlg_title = (char *) NULL;
	char           *err_dlg_msg = (char *) NULL;
	char           *err_dlg_ok = (char *) NULL;
	int		mask = 0;

	/* Get the filename from the callback structure. */
	XmFileSelectionBoxCallbackStruct *cb =
		(XmFileSelectionBoxCallbackStruct *) call;


	XmStringGetLtoR(cb->value, XmFONTLIST_DEFAULT_TAG, &file);
	if ((strcmp(file, "") == 0) || (stat(file, &buffer) == 0)) {

		/* Have an error.  Create the strings required 
		 | by the error dialog.  Start with the title.
		 */
		err_dlg_title = XtNewString(catgets(_spellCat, 4, 1, 
					    "Spelling Checker : Export - Error"));


		/* Depending on the type of error, we will have
		 | different messages and types of error dialogs.
		 */
		if (strcmp(file, "") == 0) {

			/* No filename was retrieved! */
			err_dlg_msg = XtNewString(catgets(_spellCat, 4, 6, "File name was not specified.\nExport of personal dictionary aborted."));
			mask = DLG_BTN_MASK_CANCEL | DLG_BTN_MASK_HELP;

		} else if ((buffer.st_mode & S_IFREG)) {

			/* File is a plain text file.  Is it writable? */
			if (access(file, W_OK) == 0) {

				size = strlen(catgets(_spellCat, 4, 2, "The specified file exists.\nDo you want to over write that file with the\ncurrent contents of your personal dictionary?")) + 
					strlen(file) + 5;
				err_dlg_msg = (char *) calloc(size, 
							      sizeof(char));
				sprintf(err_dlg_msg, "%s\n\n%s",
					file,
					catgets(_spellCat, 4, 2, "The specified file exists.\nDo you want to over write that file with the\ncurrent contents of your personal dictionary?"));


				/* Now create label for the OK button. */
				err_dlg_ok = XtNewString(
						  catgets(_spellCat, 4, 5, 
							  "Overwrite"));


			} else {
				size = strlen(catgets(_spellCat, 4, 7, "You do not have write permissions for the specified file\nExport of personal dictionary aborted.")) + 
				       strlen(file) + 5;
				err_dlg_msg = (char *) calloc(size, 
							      sizeof(char));
				sprintf(err_dlg_msg, "%s\n\n%s",
					file, catgets(_spellCat, 4, 7, "You do not have write permissions for the specified file.\nExport of personal dictionary aborted."));

				mask = DLG_BTN_MASK_CANCEL | DLG_BTN_MASK_HELP;

			}

		} else {

			/* File is a directory or link, invalid file. */
			size = strlen(catgets(_spellCat, 4, 8, "File specified is not a text file.\nExport of personal dictionary aborted.")) + 
				       strlen(file) + 5;
			err_dlg_msg = (char *) calloc(size, sizeof(char));
			sprintf(err_dlg_msg, "%s\n\n%s", file,
				catgets(_spellCat, 4, 8, "File specified is not a text file.\nExport of personal dictionary aborted."));
			mask = DLG_BTN_MASK_CANCEL | DLG_BTN_MASK_HELP;

		}


		/* Create the error dialog, making it a child of the 
		 | options dialog box rather than the file selection
		 | box.  The variable parent refers to the options 
		 | dialog box.
		 */
		parent = XtParent(XtParent(wid));
		if (mask == 0) {
			err_dlg = _SDtSpellShowDialog(parent, err_dlg_title,
						      XmDIALOG_QUESTION, mask, 
						      XmDIALOG_CANCEL_BUTTON,
						      err_dlg_msg, err_dlg_ok, 
						      (char *) NULL,
						      (char *) NULL);
			XtAddCallback(err_dlg, XmNokCallback, 
				      _SDtSpellOverWrite, (XtPointer) file);
		} else
			err_dlg = _SDtSpellShowDialog(parent, err_dlg_title,
						      XmDIALOG_ERROR, mask, 
						      XmDIALOG_CANCEL_BUTTON,
						      err_dlg_msg, err_dlg_ok, 
						      (char *) NULL,
						      (char *) NULL);
		XtVaSetValues(err_dlg, XmNdeleteResponse, XmDESTROY, NULL);
		XtPopup(XtParent(err_dlg), XtGrabNone);
		free(err_dlg_title);
		free(err_dlg_msg);
		if (err_dlg_ok != (char *) NULL)
			free(err_dlg_ok);

	} else if (errno == ENOENT) {

		/* Save a copy of the personal dictionary, in plain 
		 | ASCII format, to the file specified.
		 */
		_SDtSpellListPersonal((Widget) spell, file);
		XtFree(file);
	}


	/* Dismiss the file selection box. */
	XtUnrealizeWidget(wid);
}


/* Function	:	_SDtSpellDoImport()
 |
 | Objective	:	Xt callback called when the user has selected
 |			OK from the Import file selection box.
 */

void
_SDtSpellDoImport(Widget wid, XtPointer client, XtPointer call)
{
	char                     *file = (char *) client;
	XmAnyCallbackStruct      *cbs = (XmAnyCallbackStruct *) call;
	FILE                     *fd;
	Widget                    parent = XtParent(wid);
	Widget                    spellWid;
	char                      buffer[256];
	int                       result;
	int			  i, count = 0;
	XmString                  xms;
	XmStringTable             items;
	char			 *entry;


	if (cbs->reason == XmCR_HELP) {

		/* User wants to cancel the operation. */
		XtPopdown(parent);
		return;
	}


	/* Get the spell widget. */
	spellWid = XtParent(XtParent(XtParent(parent)));


	/* If the user wants to overwrite their personal dictionary,
	 | then do so now.
	 */
	if (cbs->reason == XmCR_CANCEL) {

		XtVaGetValues(SPELL_pdList(spellWid),
			      XmNitemCount, &count,
			      XmNitems, &items,
			      NULL);

		for (i = 0; i < count; i++) {
			XmStringGetLtoR(items[i], XmSTRING_DEFAULT_CHARSET,
					&entry);
			SPELL_deleteWords(spellWid) =
				_addToList(SPELL_deleteWords(spellWid), entry);
			XtFree(entry);
		}


		/* Delete all items in the list. */
		XmListDeleteAllItems(SPELL_pdList(spellWid));
	}



	/* Open the file for reading. */
	fd = fopen(file, "r");
	while (!feof(fd)) {
		result = fscanf(fd, "%s\n", buffer);
		if (result == 1) {

			/* Add the word to the addWords list.
			 | We need to wait for the user to select
			 | OK or Apply before we can make the
			 | change to the personal dictionary
			 | permanently.
			 */
			SPELL_addWords(spellWid) =
				_addToList(SPELL_addWords(spellWid), buffer);
			count++;

			/* Add the word to the end of the scrolling list. */
			xms = XmStringCreateLocalized(buffer);
			XmListAddItem(SPELL_pdList(spellWid), xms, 0);
			XmStringFree(xms);
		}
	}
	fclose(fd);


	/* Make the last item visible, so that the user sees the
	 | words that were added.
	 */
	XmListSetBottomPos(SPELL_pdList(spellWid), 0);
}


/* Function	:	_SDtSpellUpdatePersonal()
 |
 | Objective	:	Xt callback called when the user has selected
 |			OK from the Import file selection box.
 */

void
_SDtSpellUpdatePersonal(Widget wid, XtPointer client, XtPointer call)
{
	char           *file;		  /* File to save copy. */
	struct stat     buffer;           /* For check if file exists. */
	Widget          err_dlg;
	Widget		optionsDlg = (Widget) client;
	int             size;             /* Size of strings. */
	int             acs_res;          /* Result of access() call. */


	/* Error dialog labels and strings, and button mask. */
	char           *err_dlg_title = (char *) NULL;
	char           *err_dlg_msg = (char *) NULL;
	char           *err_dlg_ok = (char *) NULL;
	char           *err_dlg_cancel = (char *) NULL;
	char           *err_dlg_help= (char *) NULL;
	int		mask = 0;


	/* Get the filename from the callback structure. */
	XmFileSelectionBoxCallbackStruct *cb =
		(XmFileSelectionBoxCallbackStruct *) call;


	XmStringGetLtoR(cb->value, XmFONTLIST_DEFAULT_TAG, &file);
	(void) stat(file, &buffer);
	acs_res = (access(file, R_OK | F_OK));
	if ((acs_res == 0) && (buffer.st_mode & S_IFREG)) {

		/* Create a dialog that asks if user wants to 
		 | append to or overwrite their personal dictionary.
		 */
		err_dlg_title = XtNewString(catgets(_spellCat, 4, 4, 
					    "Spelling Checker : Import"));
		err_dlg_msg = XtNewString(catgets(_spellCat, 4, 15,  "Do you want to append the contents of the file\nto your current personal dictionary, or overwrite\nand destroy your current personal dictionary?"));

		/* Normally, the message dialog consists of the buttons
		 |	<Ok>		<Cancel>	<Help>
		 | Override these defaults, so that the dialog has:
		 |	<Append>	<Overwrite>	<Cancel>
		 */
		err_dlg_ok = XtNewString(catgets(_spellCat, 4, 16, "Append"));
		err_dlg_cancel = 
			XtNewString(catgets(_spellCat, 4, 5, "Overwrite"));
		err_dlg_help = 
			XtNewString(catgets(_spellCat, 2, 11, "Cancel"));

	} else {

		/* Have an error.  Create the strings required 
		 | by the error dialog.  Start with the title.
		 */
		err_dlg_title = XtNewString(catgets(_spellCat, 4, 9, 
					    "Spelling Checker : Import - Error"));


		if (strcmp(file, "") == 0) {

			/* No file name was entered. */
			err_dlg_msg = XtNewString(catgets(_spellCat, 4, 10, "File name was not specified.\nImport to personal dictionary aborted."));
			mask = DLG_BTN_MASK_CANCEL | DLG_BTN_MASK_HELP;

		} else if (!(buffer.st_mode & S_IFREG)) {

			/* File is not a regular text file. */
			size = strlen(catgets(_spellCat, 4, 11, "File is not a text file.\nImport to personal dictionary aborted.")) + 
				strlen(file) + 5;
			err_dlg_msg = (char *) calloc(size, sizeof(char));
			sprintf(err_dlg_msg, "%s\n\n%s", file,
				catgets(_spellCat, 4, 11, "File is not a text file.\nImport to personal dictionary aborted."));
			mask = DLG_BTN_MASK_CANCEL | DLG_BTN_MASK_HELP;

		} else if (errno == ENOENT) {

			/* File does not exist. */
			size = strlen(catgets(_spellCat, 4, 12, "The specified file does not exist.\nImport to personal dictionary aborted.")) + 
				strlen(file) + 5;
			err_dlg_msg = (char *) calloc(size, sizeof(char));
			sprintf(err_dlg_msg, "%s\n\n%s", file,
				catgets(_spellCat, 4, 12, "The specified file does not exist.\nImport to personal dictionary aborted."));
			mask = DLG_BTN_MASK_CANCEL | DLG_BTN_MASK_HELP;

		} else if (errno == EACCES) {

			/* No access to file. */
			size = strlen(catgets(_spellCat, 4, 13, "You do not have read permissions for the specified file.\nImport to personal dictionary aborted.")) + 
				strlen(file) + 5;
			err_dlg_msg = (char *) calloc(size, sizeof(char));
			sprintf(err_dlg_msg, "%s\n\n%s", file,
				catgets(_spellCat, 4, 13, "You do not have read permissions for the specified file.\nImport to personal dictionary aborted."));
			mask = DLG_BTN_MASK_CANCEL | DLG_BTN_MASK_HELP;

		} else {

			/* Generic error - just say could not access file. */
			size = strlen(catgets(_spellCat, 4, 14, "The specified file could not be accessed.\nImport to personal dictionary aborted.")) + 
				strlen(file) + 5;
			err_dlg_msg = (char *) calloc(size, sizeof(char));
			sprintf(err_dlg_msg, "%s\n\n%s", file,
				catgets(_spellCat, 4, 14, "The specified file could not be accessed.\nImport to personal dictionary aborted."));
			mask = DLG_BTN_MASK_CANCEL | DLG_BTN_MASK_HELP;
		}
	}


	/* Create the error dialog, making it a child of the options 
	 | dialog box rather than the file selection box.
	 */
	if (mask == 0) {
		err_dlg = _SDtSpellShowDialog(optionsDlg, err_dlg_title,
					      XmDIALOG_QUESTION, mask, 
					      XmDIALOG_OK_BUTTON,
					      err_dlg_msg, err_dlg_ok, 
					      err_dlg_cancel, err_dlg_help);
			XtAddCallback(err_dlg, XmNokCallback, 
				      _SDtSpellDoImport, (XtPointer) file);
			XtAddCallback(err_dlg, XmNcancelCallback, 
				      _SDtSpellDoImport, (XtPointer) file);
			XtAddCallback(err_dlg, XmNhelpCallback, 
				      _SDtSpellDoImport, (XtPointer) file);
	} else
		err_dlg = _SDtSpellShowDialog(optionsDlg, err_dlg_title,
					      XmDIALOG_ERROR, mask, 
					      XmDIALOG_CANCEL_BUTTON,
					      err_dlg_msg, err_dlg_ok, 
					      err_dlg_cancel, err_dlg_help);
	XtVaSetValues(err_dlg, XmNdeleteResponse, XmDESTROY, NULL);
	XtPopup(XtParent(err_dlg), XtGrabNone);
	free(err_dlg_title);
	free(err_dlg_msg);
	if (err_dlg_ok != (char *) NULL)
		free(err_dlg_ok);
	if (err_dlg_cancel != (char *) NULL)
		free(err_dlg_cancel);
	if (err_dlg_help != (char *) NULL)
		free(err_dlg_help);


	/* Dismiss the file selection box. */
	XtUnrealizeWidget(wid);
}


/* Function	:	_SDtSpellSetCurrentLang()
 |
 | Objective	:	Xt callback called when a new language is
 |			selected from the Current Dictionary options menu.
 */

void
_SDtSpellSetCurrentLang(Widget wid, XtPointer client , XtPointer call)
{
	SDtSpellSelectionBoxWidget spell = (SDtSpellSelectionBoxWidget) client;
	LangMenuP		   p = SPELL_current_items(spell);


	while ((p != NULL) && (p->langItem != wid))
		p = p->next;

	if (p)
		SPELL_dict_to_use(spell) = p->lang;
}


/* Function	:	_SDtOptionWordListCB()
 |
 | Objective	:	Xt callback to toggle the sensitivity of
 |			the Remove button.  The button is sensitive if
 |			a word in the custom dictionary is selected.
 |			Otherwise, it remains unavialable to the user.
 */
void 
_SDtOptionWordListCB(Widget wid, XtPointer client , XtPointer call)
{
	SDtSpellSelectionBoxWidget spell = (SDtSpellSelectionBoxWidget) client;
	int count;

	Widget remove = XtNameToWidget(SPELL_option_panel(spell), "*removeBtn");


	/* Get the select count. */
	XtVaGetValues(wid, XmNselectedItemCount, &count,
		      NULL);


	/* If the selected count is greater than 0, then turn on the
	 | Remove button; otherwise, turn it off.
	 */
	if (count > 0)
		XtSetSensitive(remove, True);
	else
		XtSetSensitive(remove, False);
}


/***************************************************************************
 |		LOCAL FUNCTIONS THAT ARE NOT CALLBACKS
 ***************************************************************************/

Boolean
_SDtApply(SDtSpellSelectionBoxWidget spell)
{
	Boolean         dismiss = True;
	char           *error;
	Widget          remove;
	SDtSpellResR	deskset_res;
	Boolean         toggle_state;
	Widget		menu;
	LangMenuP	p;


	/* Toggle the sensitivity of the Remove button. */
	remove = XtNameToWidget(SPELL_option_panel(spell), "*removeBtn");
	XtSetSensitive(remove, False);


	/* Process the personal dictionary for any words that have
	 | been added and deleted, only if the word lists are not
	 | NULL.
	 */
	if (SPELL_addWords(spell) || SPELL_deleteWords(spell)) {
		error = _updateDict(spell);
		if (error != NULL) { /* handle error */
			_SDtSpellError(spell, error);
			free(error);
			/* reset list */
		 	_SDtSpellManageOptions(spell);
			return False;
		}
	}


	/* Set the usage of the personal dictionary based on the 
	 * SPELL_personal_toggle(spell) state.
	 */
	toggle_state = XmToggleButtonGetState(SPELL_personal_toggle(spell));
	if (toggle_state != SPELL_personal(spell))
		XtVaSetValues((Widget) spell, 
			      XmNusePersonal, toggle_state,
			      NULL);


	/* Get the button state of the follow locale toggle, and
	 | set the resource appropriately.
	 */
	toggle_state = XmToggleButtonGadgetGetState(SPELL_locale_toggle(spell));
	XtVaSetValues((Widget) spell,
		       XmNfollowLocale, toggle_state,
		       NULL);


	/* Set the defaultDictionary resource if followLocale is False,
	 | i.e., toggle_state.
	 */
	if (!toggle_state) {

		/* Get the menu item that was selected. */
		XtVaGetValues(SPELL_default_menu(spell),
			      XmNmenuHistory, &menu,
			      NULL);

		for (p = SPELL_default_items(spell); p != NULL; p = p->next)
			if (p ->langItem == menu)
				break;


		if (p)
			XtVaSetValues((Widget) spell, XmNdefaultDictionary, 
				      _SDtSpellLangToLocale(p->lang),
				      NULL);
	}


	/* Switch to the specified language dictionary. */
	if (SPELL_dict_to_use(spell) != SPELL_dict_used(spell)) {

		XtVaSetValues((Widget) spell, XmNdictionary, 
			      _SDtSpellLangToLocale(SPELL_dict_to_use(spell)), 
			      NULL);
		SPELL_dict_used(spell) = SPELL_dict_to_use(spell);
	}


	/* Save to .desksetdefaults. */
	if (SPELL_use_sysdefaults(spell)) {

		deskset_res.usePersonal = SPELL_personal(spell);
		if (SPELL_default_dictionary(spell))
			deskset_res.defaultDictionary = 
				strdup(SPELL_default_dictionary(spell));
		deskset_res.followLocale = SPELL_follow_locale(spell);
		_SDtSpellSaveDefaults(deskset_res);
		if (deskset_res.defaultDictionary)
			free(deskset_res.defaultDictionary);
	}

	return(True);
}

