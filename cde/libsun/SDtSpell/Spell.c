/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

#pragma ident "@(#)Spell.c	1.40 97/03/03 SMI"

/*--------------------------------------------------------------*/
/* 			INCLUDE FILES 				*/
/*--------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <nl_types.h>

#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#endif				/* X_NOT_STDC_ENV */

#include <Xm/ArrowB.h>
#include <Xm/AtomMgr.h>
#include <Xm/DialogS.h>
#include <Xm/DragC.h>
#include <Xm/DropSMgr.h>
#include <Xm/GadgetP.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/Protocols.h>
#include <Xm/RowColumnP.h>
#include <Xm/MessageB.h>
#include <Xm/Scale.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/VendorSEP.h>
#include <Xm/Form.h>
#include <Xm/Separator.h>
#include <Xm/SeparatoG.h>

#include <Dt/EnvControl.h>
#include <Dt/HourGlass.h>

#include <SDt/SpellP.h>

#include "spellLib.h"
#include "globals.h"
#include "misc_spell.h"
#include "engine.h"
#include "defaults.h"
#include "help.h"


#ifdef I18N_MSG
#include "XmMsgI.h"
#endif				/* I18N_MSG */


/*--------------------------------------------------------------*/
/* 		DEFINES/STRUCTURES	 			*/
/*--------------------------------------------------------------*/

/* Minimum size of the scale, in pixels. */
#define XmSPELL_SCALE_MIN_SIZE 150


#define XmSPELL_BOX_BIT  28	/* missing  */


/* Buttons in the main spell GUI. */
#define SKIP_BUTTON		0
#define SKIP_ALL_BUTTON		1
#define CHANGE_BUTTON		2
#define CHANGE_ALL_BUTTON	3
#define LEARN_BUTTON		4
#define OPTION_BUTTON		5


/* Macro to determine if widget is a button. */
#define IsButton(w) ( \
      XmIsPushButton(w)   || XmIsPushButtonGadget(w)   || \
      XmIsToggleButton(w) || XmIsToggleButtonGadget(w) || \
      XmIsArrowButton(w)  || XmIsArrowButtonGadget(w)  || \
      XmIsDrawnButton(w))


/* Blank string. */
#define	NULL_STRING	"\0"


/* Bold font for the misspelled word. */
#define	BOLD_FONT	"-dt-interface user-bold-*-*-*-*-*-*-*-*-*-*-*-*"


/*--------------------------------------------------------------*/
/* 		Global Variables	 			*/
/*--------------------------------------------------------------*/

/* Message catalog. */
nl_catd		_spellCat;


/*--------------------------------------------------------------*/
/* 		Static Function Declarations	 		*/
/*--------------------------------------------------------------*/

/*-----------------------------------------------*/
/* Functions defined as part of the Class Record */
/*-----------------------------------------------*/
static void        ClassPartInitialize(WidgetClass);
static void        Initialize(Widget, Widget, ArgList, Cardinal *);
static void        ChangeManaged(Widget);
static void        Destroy(Widget);
static void        DeleteChild(Widget);
static Boolean     SetValues(Widget, Widget, Widget, ArgList, Cardinal *);
static XmGeoMatrix SpellGeoMatrixCreate(Widget, Widget, XtWidgetGeometry *);
static Boolean     SpellSelectBoxNoGeoRequest(XmGeoMatrix);


/*-----------------------------------------------*/
/* Creation functions used by Initialize         */
/*-----------------------------------------------*/
static void	_SDtSpellCreateList(SDtSpellSelectionBoxWidget);	
static void	_SDtSpellCreateText(SDtSpellSelectionBoxWidget);	
static void	_SDtSpellCreateBottomButtons(SDtSpellSelectionBoxWidget);	
static void     _SDtSpellCreateMispelled(SDtSpellSelectionBoxWidget);
static void     _SDtSpellCreateButtons(SDtSpellSelectionBoxWidget);
static void     _SDtSpellCreateMessage(SDtSpellSelectionBoxWidget);
static void     _SDtSpellCreateScale(SDtSpellSelectionBoxWidget);
static void     _SDtSpellCreateOptions(SDtSpellSelectionBoxWidget);
static void 	_SDtSpellCalcSize(Widget);


/*------------------------------------------------------*/
/* Set functions used to set the values of child widget */
/*------------------------------------------------------*/
static void     _SDtSpellSetMisspelled(Widget, char *);
static void     _SDtSpellSetMsg(Widget, char *);
static void     _SDtSpellSetList(Widget, char **, int);
static void     _SDtSpellSetSensitive(Widget, Boolean);
static void     _SDtSpellSetOkLabel(Widget, char *);
static Boolean  _SDtSpellSetGauge(Widget, int);


/*---------------------------------------*/
/* Callback functions for Child Widgets  */
/*---------------------------------------*/
static void     _SDtSpellSelectPB(Widget, XtPointer, XtPointer);
static void     _SDtSpellOperatePB(Widget, XtPointer, XtPointer);
static void     _SDtSpellOptionButtonPB(Widget, XtPointer, XtPointer);
static void     _SDtSpellTextActivateCB(Widget, XtPointer, XtPointer);
static void     _SDtSpellSuggListCB(Widget, XtPointer, XtPointer);

static void     _SDtSpellPopdown(Widget, XtPointer, XtPointer);


/*--------------------------------------------*/
/* General Functions used during spell check  */
/*--------------------------------------------*/
static void     _SDtSpellClearSearch(Widget);
void     _SDtSpellError(Widget, char *);
static Boolean  _SDtSpellVerifyWord(Widget, char *);
static Boolean  _SDtSpellLoadText(Widget);
static void	_SDtSpellDispatch(Widget);
static Boolean	_SDtSpellCheck(XtPointer);
static char    *_SDtSpellGetNextWord(Widget);
static void     _SDtSpellUpdateList(Widget, char *);


/*------------------------------------------------------------*/
/* Functions used during which are used with approp. buttons  */
/*------------------------------------------------------------*/
static void     _SDtSpellReplaceWord(Widget, char *, Boolean);
static void     _SDtSpellSkipWord(Widget, Boolean);
static void     _SDtSpellLearnWord(Widget, char *);
static void	_SDtSpellDefaultAction(Widget, XtPointer, XtPointer);


/*--------------------------------------------------------------*/
/* 		Spell Widget Resources		 		*/
/*--------------------------------------------------------------*/
static XtResource resources[] =
{
	/* Spell widget-specific resources. */

	{	XmNdictionary,
		XmCDictionary,
		XmRString,
		sizeof(String),
		XtOffsetOf(struct _SDtSpellSelectionBoxRec,
			   spell_select_box.dictionary),
		XmRString,
		(XtPointer) LOCALE_EN_US 
	},
	{	XmNsearchWidget,
		XmCWidget,
		XmRWidget,
		sizeof(Widget),
		XtOffsetOf(struct _SDtSpellSelectionBoxRec,
			   spell_select_box.spellTarget),
		XmRWidget,
		(XtPointer) NULL
	},
	{	XmNusePersonal,
		XmCUsePersonal,
		XmRBoolean,
		sizeof(Boolean),
		XtOffsetOf(struct _SDtSpellSelectionBoxRec,
			   spell_select_box.usePersonal),
		XmRImmediate,
		(XtPointer) True
	},
	{	XmNfollowLocale,
		XmCFollowLocale,
		XmRBoolean,
		sizeof(Boolean),
		XtOffsetOf(struct _SDtSpellSelectionBoxRec,
			   spell_select_box.followLocale),
		XmRImmediate,
		(XtPointer) True
	},
	{	XmNdefaultDictionary,
		XmCDefaultDictionary,
		XmRString,
		sizeof(String),
		XtOffsetOf(struct _SDtSpellSelectionBoxRec,
			   spell_select_box.defaultDictionary),
		XmRString,
		(XtPointer) NULL 
	}
};


/*--------------------------------------------------------------*/
/* 		Class Record Info		 		*/
/*--------------------------------------------------------------*/
externaldef(xmfileselectionboxclassrec) SDtSpellSelectionBoxClassRec
sdtSpellSelectionBoxClassRec =
{
	{
		/* core class record   */
		/* superclass	       */ (WidgetClass) &xmBulletinBoardClassRec,
		/* class_name	       */ "SDtSpellSelectBox",
		/* widget_size	       */ sizeof(_SDtSpellSelectionBoxRec),
		/* class_initialize    */ (XtProc) NULL,
		/* class part init     */ ClassPartInitialize,
		/* class_inited	       */ FALSE,
		/* initialize	       */ Initialize,
		/* initialize hook     */ (XtArgsProc) NULL,
		/* realize	       */ XtInheritRealize,
		/* actions	       */ NULL,	
		/* num_actions	       */ 0,
		/* resources	       */ resources,
		/* num_resources       */ XtNumber(resources),
		/* xrm_class	       */ NULLQUARK,
		/* compress_motion     */ TRUE,
		/* compress_exposure   */ XtExposeCompressMaximal,
		/* compress crossing   */ FALSE,
		/* visible_interest    */ FALSE,
		/* destroy	       */ Destroy,
		/* resize	       */ XtInheritResize,
		/* expose	       */ XtInheritExpose,
		/* set_values	       */ SetValues,
		/* set_values_hook     */ (XtArgsFunc) NULL,
		/* set_values_almost   */ XtInheritSetValuesAlmost,
		/* get_values_hook     */ (XtArgsProc) NULL,
		/* accept_focus	       */ (XtAcceptFocusProc) NULL,
		/* version	       */ XtVersion,
		/* callback_private    */ (XtPointer) NULL,
		/* tm_table            */ XtInheritTranslations,
		/* query_geometry      */ XtInheritQueryGeometry,
		/* display_accelerator */ (XtStringProc) NULL,
		/* extension	       */ (XtPointer) NULL,
	},

	{
		/* composite class record   */
		/* geometry manager         */ XtInheritGeometryManager,
		/* set changed proc	    */ ChangeManaged,
		/* insert_child		    */ XtInheritInsertChild,
		/* delete_child 	    */ DeleteChild,
		/* extension		    */ (XtPointer) NULL,
	},

	{
		/* constraint class record  */
		/* no additional resources  */ (XtResourceList) NULL,
		/* num additional resources */ 0,
		/* size of constraint rec   */ 0,
		/* constraint_initialize    */ (XtInitProc) NULL,
		/* constraint_destroy	    */ (XtWidgetProc) NULL,
		/* constraint_setvalue      */ (XtSetValuesFunc) NULL,
		/* extension                */ (XtPointer) NULL,
	},

	{
		/* manager class record         */
		/* translations                 */ XtInheritTranslations,
		/* get_resources                */ NULL,
		/* num_syn_resources            */ 0,
		/* constraint_syn_resources     */ (XmSyntheticResource *) NULL,
		/* num_constraint_syn_resources */ 0,
		/* parent_process<              */ XmInheritParentProcess,
		/* extension		        */ (XtPointer) NULL,
	},

	{
		/* bulletinBoard class record  */
		/* always_install_accelerators */ TRUE,
		/* geo_matrix_create           */ SpellGeoMatrixCreate,
		/* focus_moved_proc            */ XmInheritFocusMovedProc,
		/* extension                   */ (XtPointer) NULL,
	},
	{
		/* spell class record */
		/* extension		      */ (XtPointer) NULL,
	}
};


externaldef(xmspellselectionboxwidgetclass) WidgetClass
sdtSpellSelectionBoxWidgetClass = (WidgetClass) &sdtSpellSelectionBoxClassRec;


/*-----------------------------------------------------------------*/
/* Functions defined as part of the Class Record 	           */
/*-----------------------------------------------------------------*/

/*********************************************************************
 * Class Initialization.  Sets up accelerators and fast subclassing.
 *********************************************************************/
static void
ClassPartInitialize(WidgetClass fsc)
{
	_XmFastSubclassInit(fsc, XmSPELL_BOX_BIT);
	return;
}


/************************************************************************
 * This routine initializes an instance of the spell widget.  Instance
 * record fields which are shadow resources for child widgets and which
 * are of an allocated type are set to NULL after they are used, since
 * the memory identified by them is not owned by the Spell Box.
 ************************************************************************/
static void
Initialize(Widget rw, Widget nw, ArgList args_in, Cardinal *num_args)
{
	SDtSpellSelectionBoxWidget new_w = (SDtSpellSelectionBoxWidget) nw;
	Arg             args[16];
	int             numArgs;
	XmString        xmstr;
	Widget          search, form;


	/* Set to some default values.  For the public resources, the
	 | following are defaults:
	 |	SPELL_dictionary(new_w) = LOCALE_EN_US
	 |	SPELL_personal(new_w) = True
	 |	SPELL_target(new_w) = NULL
	 | See the resources section of Spell.c for the default assignments.
	 */


	/* Set up data that will be used to spell check. */
	search = SPELL_target(new_w);
	if (search != NULL) {

		if ((XmIsText(search) || XmIsTextField(search))) {

			SPELL_isText(new_w) = True;
			SPELL_TxtWid(new_w) = (Widget) search;

		} else {

			/* DtEditor */
			SPELL_isText(new_w) = False;
			SPELL_TxtWid(new_w) = XtNameToWidget(search, "*text");

		}
	}


	/* First load the SPELL_engine */
	SPELL_engine(new_w) =
		_SDtSpellInitSpellEnginePtr(&SPELL_personal(new_w),
					    &SPELL_dictionary(new_w));
	if (SPELL_engine(new_w) == NULL) {
		/* ERROR */
		return;
	}


	/* Default values for some of the widget's members. */
	SPELL_data(new_w) = NULL;
	SPELL_listPosition(new_w) = -1;
	SPELL_word(new_w) = NULL;
	SPELL_suggestions(new_w) = NULL;
	SPELL_sug_count(new_w) = 0;
	SPELL_done(new_w) = False;
	SPELL_size(new_w) = 0;
	SPELL_current(new_w) = 0;
	SPELL_cursor(new_w) = 0;
	SPELL_start(new_w) = 0;
	SPELL_end(new_w) = 0;
	SPELL_adjust(new_w) = 0;
	SPELL_deleteWords(new_w) = NULL;
	SPELL_addWords(new_w) = NULL;
	SPELL_wrap_around(new_w) = False;
	SPELL_use_sysdefaults(new_w) = False;
	SPELL_error(new_w) = NULL;
	SPELL_workID(new_w) = 0;


	/* Create the widgets that make up the spell GUI. */ 
	SPELL_form(new_w) = XmCreateForm((Widget)new_w, "spellform", NULL, 0);
	XtManageChild(SPELL_form(new_w));


	_SDtSpellCreateList(new_w);          /* Create the suggestion list.   */
	_SDtSpellCreateText(new_w);          /* Create the suggestion text.   */
	_SDtSpellCreateBottomButtons(new_w); /* Create Close, Help buttons.   */
	_SDtSpellCreateMispelled(new_w);     /* Create mispelled words area.  */
	_SDtSpellCreateButtons(new_w);	     /* Create operator buttons.      */
	_SDtSpellCreateMessage(new_w);	     /* Create message text.          */
	_SDtSpellCreateScale(new_w);	     /* Create status scale.          */
	_SDtSpellCreateOptions(new_w);	     /* Create options button.        */

	/* During initialization, these should be the same. */
	SPELL_dict_to_use(new_w) = SPELL_dict_used(new_w);


	/* Set up our scale size. */
	_SDtSpellCalcSize((Widget) new_w);


	/* Fix the layout of the widgets by specifying the proper
	 | attachment values. Start with the message area.
	 */
	numArgs = 0;
	XtSetArg(args[numArgs], XmNtopAttachment, XmATTACH_FORM);
	numArgs++;
	XtSetArg(args[numArgs], XmNrightAttachment, XmATTACH_FORM);
	numArgs++;
	XtSetValues(SPELL_message_bar(new_w), args, numArgs);


	/* Fix the layout of the scale. */
	numArgs = 0;
	XtSetArg(args[numArgs], XmNtopAttachment, XmATTACH_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNtopWidget, SPELL_message_bar(new_w));
	numArgs++;
	XtSetArg(args[numArgs], XmNtopOffset, 2);
	numArgs++;
	XtSetArg(args[numArgs], XmNrightAttachment, XmATTACH_FORM);
	numArgs++;
	XtSetValues(SPELL_search_scale(new_w), args, numArgs);


	/* Now fix the layout of the misspelled word label. */
	numArgs = 0;
	XtSetArg(args[numArgs], XmNtopAttachment, XmATTACH_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNtopWidget, SPELL_message_bar(new_w));
	numArgs++;
	XtSetArg(args[numArgs], XmNtopOffset, 2);
	numArgs++;
/*
	XtSetArg(args[numArgs], XmNleftAttachment, XmATTACH_FORM);
	numArgs++;
*/
	XtSetValues(SPELL_misspelledLabel(new_w), args, numArgs);


	/* Now fix the layout of the misspelled word itself. */
	numArgs = 0;
	XtSetArg(args[numArgs], XmNtopAttachment, XmATTACH_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNtopWidget, SPELL_message_bar(new_w));
	numArgs++;
	XtSetArg(args[numArgs], XmNtopOffset, 2);
	numArgs++;
	XtSetArg(args[numArgs], XmNleftAttachment, XmATTACH_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNleftWidget, SPELL_misspelledLabel(new_w));
	numArgs++;
	XtSetArg(args[numArgs], XmNrightAttachment, XmATTACH_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNrightWidget, SPELL_search_scale(new_w));
	numArgs++;
	XtSetArg(args[numArgs], XmNrightOffset, 20);
	numArgs++;
/*
	XtSetArg(args[numArgs], XmNlabelString, XmStringCreateSimple("LINDA"));
	numArgs++;
*/
	XtSetValues(SPELL_misspelledWid(new_w), args, numArgs);


	/* Fix the layout of the suggestion label. */
	numArgs = 0;
	XtSetArg(args[numArgs], XmNtopAttachment, XmATTACH_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNtopWidget, SPELL_misspelledLabel(new_w));
	numArgs++;
	XtSetArg(args[numArgs], XmNtopOffset, 9);
	numArgs++;
/*
	XtSetArg(args[numArgs], XmNleftAttachment, XmATTACH_FORM);
	numArgs++;
*/
	XtSetArg(args[numArgs], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNrightWidget, SPELL_misspelledLabel(new_w));
	numArgs++;
	XtSetValues(SPELL_Textlabel(new_w), args, numArgs);


	/* Fix the layout of the suggestion text field. */
	numArgs = 0;
	XtSetArg(args[numArgs], XmNtopAttachment, XmATTACH_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNtopWidget, SPELL_misspelledLabel(new_w));
	numArgs++;
	XtSetArg(args[numArgs], XmNtopOffset, 7);
	numArgs++;
	XtSetArg(args[numArgs], XmNleftAttachment, XmATTACH_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNleftWidget, SPELL_Textlabel(new_w));
	numArgs++;
	XtSetArg(args[numArgs], XmNrightAttachment, XmATTACH_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNrightWidget, SPELL_search_scale(new_w));
	numArgs++;
	XtSetArg(args[numArgs], XmNrightOffset, 20);
	numArgs++;
	XtSetValues(SPELL_Text(new_w), args, numArgs);


	/* Fix the layout of the change button. */
	numArgs = 0;
	XtSetArg(args[numArgs], XmNtopAttachment, XmATTACH_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNtopWidget, SPELL_search_scale(new_w));
	numArgs++;
	XtSetArg(args[numArgs], XmNtopOffset, 9);
	numArgs++;
	XtSetArg(args[numArgs], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNleftWidget, SPELL_search_scale(new_w));
	numArgs++;
	XtSetValues(SPELL_update(new_w), args, numArgs);


	/* Fix the layout of the change all button. */
	numArgs = 0;
	XtSetArg(args[numArgs], XmNtopAttachment, XmATTACH_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNtopWidget, SPELL_search_scale(new_w));
	numArgs++;
	XtSetArg(args[numArgs], XmNtopOffset, 9);
	numArgs++;
	XtSetArg(args[numArgs], XmNrightAttachment, XmATTACH_FORM);
	numArgs++;
	XtSetValues(SPELL_update_all(new_w), args, numArgs);


	/* Fix the layout of the scrolling list. */
	numArgs = 0;
	XtSetArg(args[numArgs], XmNtopAttachment, XmATTACH_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNtopWidget, SPELL_Text(new_w));
	numArgs++;
	XtSetArg(args[numArgs], XmNtopOffset, 10);
	numArgs++;
	XtSetArg(args[numArgs], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNleftWidget, SPELL_Text(new_w));
	numArgs++;
	XtSetArg(args[numArgs], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNrightWidget, SPELL_Text(new_w));
	numArgs++;
	XtSetArg(args[numArgs], XmNbottomAttachment, XmATTACH_FORM);
	numArgs++;
	XtSetArg(args[numArgs], XmNbottomOffset, 10);
	numArgs++;
	XtSetValues(XtParent(SPELL_List(new_w)), args, numArgs);


	/* Fix the layout of the skip button. */
	numArgs = 0;
	XtSetArg(args[numArgs], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNtopWidget, XtParent(SPELL_List(new_w)));
	numArgs++;
	XtSetArg(args[numArgs], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNleftWidget, SPELL_update(new_w));
	numArgs++;
	XtSetValues(SPELL_skip(new_w), args, numArgs);


	/* Fix the layout of the skip all button. */
	numArgs = 0;
	XtSetArg(args[numArgs], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNtopWidget, XtParent(SPELL_List(new_w)));
	numArgs++;
	XtSetArg(args[numArgs], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNrightWidget, SPELL_update_all(new_w));
	numArgs++;
	XtSetValues(SPELL_skip_all(new_w), args, numArgs);


	/* Fix the layout of the learn button. */
	numArgs = 0;
	XtSetArg(args[numArgs], XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNbottomWidget, XtParent(SPELL_List(new_w)));
	numArgs++;
	XtSetArg(args[numArgs], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNleftWidget, SPELL_update(new_w));
	numArgs++;
	XtSetValues(SPELL_learn(new_w), args, numArgs);


	/* Fix the layout of the options button. */
	numArgs = 0;
	XtSetArg(args[numArgs], XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNbottomWidget, XtParent(SPELL_List(new_w)));
	numArgs++;
	XtSetArg(args[numArgs], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET);
	numArgs++;
	XtSetArg(args[numArgs], XmNrightWidget, SPELL_update_all(new_w));
	numArgs++;
	XtSetValues(SPELL_options(new_w), args, numArgs);


	/* Add callback when spell dialog is dismissed. */
        XtAddCallback((Widget) new_w,
                      XmNunmapCallback, _SDtSpellPopdown,
                      (XtPointer) new_w);
	

	if (XtClass(new_w) == sdtSpellSelectionBoxWidgetClass) {
		XtManageChildren(new_w->composite.children,
				 new_w->composite.num_children);
	}


	/* Clear out the spell gui of any junk. */
	_SDtSpellClearSearch((Widget) new_w);


	/* Set the sensitivity of the learn button, depending if 
	 | the usePersonal is set to True.
	 */
	if (!SPELL_personal(new_w)) {
		XtSetSensitive(SPELL_learn(new_w), False);
	}


	/* Assign the application context. */
	SPELL_appcontext(new_w) =
		XtDisplayToApplicationContext(XtDisplay(new_w));


	return;
}


/* Widget redrawing when the widget is managed/unmanged. */
void
ChangeManaged(Widget wid)
{

	(*(xmBulletinBoardClassRec.composite_class.change_managed)) (wid);
	_SDtSpellClearSearch(wid);

	_SDtSpellSetSensitive(wid, False);
}


/* Process widget arguments when XtSetValues is called on the spell widget. */
Boolean
SetValues(Widget ow, Widget rw, Widget nw, ArgList args, Cardinal * num_args)
{
	SDtSpellSelectionBoxWidget old_w = (SDtSpellSelectionBoxWidget) ow;
	SDtSpellSelectionBoxWidget new_w = (SDtSpellSelectionBoxWidget) nw;
	SpellEngP                  speller = SPELL_engine(new_w);
	Boolean			   sensitive;

	/* Was the searchWidget changed? */
	if (SPELL_target(old_w) != SPELL_target(new_w)) {

		/* clear out info */
		Widget          search = SPELL_target(new_w);

		_SDtSpellClearSearch((Widget) new_w);
		if (search != NULL) {

			if ((XmIsText(search) || XmIsTextField(search))) {

				SPELL_isText(new_w) = True;
				SPELL_TxtWid(new_w) = (Widget) search;

			} else {

				/* DtEditor */
				SPELL_isText(new_w) = False;
				SPELL_TxtWid(new_w) = 
					XtNameToWidget(search, "*text");

			}
		}
	}


	/* Was the usage of personal dictionary changed? */
	if (SPELL_personal(old_w) != SPELL_personal(new_w)) {
		if (SPELL_personal(new_w)) {

			/* User wants to use personal dictionary during
			 | spelling correction.
			 */
			speller->userDict =
				_SDtSpellCreateUserDict(speller->mainDict,
						SPELL_dictionary(new_w));
			speller->usePD = True;

			/* Toggle the Learn button to be sensitive, only
			 * if the Skip/Skip All buttons are sensitive.
			 */
			XtVaGetValues(SPELL_skip(new_w),
				      XtNsensitive, &sensitive,
				      NULL);
			XtSetSensitive(SPELL_learn(new_w), sensitive);

		} else {

			/* User *DOESN'T* want to use personal dictionary
			 | during spelling correction.
			 */
			_terminatePersonalDict(speller);
			speller->userDict = False;

			/* Toggle the learn button to be insensitive. */
			XtSetSensitive(SPELL_learn(new_w), False);
		}
	}


	/* Was the dictionary changed? */
	if (SPELL_dictionary(old_w) != SPELL_dictionary(new_w)) {


		/* Close the spell engine temporarily. */
		if (_SDtSpellCloseSpell(SPELL_engine(old_w))) {
			fprintf(stderr, "Set Values: close failed.");
			return(False);
		}


		if (!_SDtSpellReInit(SPELL_engine(new_w), 
				     &SPELL_dictionary(new_w))) {
			fprintf(stderr, 
				"Set Values: reinitialization failed\n");
			return(False);
		}
	}


	/* Was the behavior changed? */
	if (SPELL_follow_locale(old_w) != SPELL_follow_locale(new_w)) {


		/* If true, set the locale toggle button. */
		if (SPELL_follow_locale(new_w)) {

		   XmToggleButtonGadgetSetState(SPELL_locale_toggle(new_w),
						True, True);
		} else {

		   XmToggleButtonGadgetSetState(SPELL_default_toggle(new_w),
						True, True);
		}
	}


	(*(xmBulletinBoardClassRec.core_class.set_values))
		(ow, rw, nw, args, num_args);

	return True;
}


/* Geometry management for the spell widget. */
static XmGeoMatrix
SpellGeoMatrixCreate(Widget wid, Widget instigator, XtWidgetGeometry * desired)
{
	SDtSpellSelectionBoxWidget spell = (SDtSpellSelectionBoxWidget) wid;
	XmGeoMatrix     geoSpec;
	register XmGeoRowLayout layoutPtr;
	register XmKidGeometry boxPtr;
	XmKidGeometry   firstButtonBox;
	Boolean         dirListLabelBox;
	Boolean         listLabelBox;
	Boolean         dirListBox;
	Boolean         listBox;
	Boolean         selLabelBox;
	Boolean         filterLabelBox;
	Dimension       vspace = BB_MarginHeight(spell);
	int             i;


	geoSpec = _XmGeoMatrixAlloc(
				    XmSPELL_MAX_WIDGETS_VERT,
				    spell->composite.num_children,
				    0);

	geoSpec->composite = (Widget) spell;
	geoSpec->instigator = (Widget) instigator;

	if (desired) {
		geoSpec->instig_request = *desired;
	}

	geoSpec->margin_w = BB_MarginWidth(spell) +
		spell->manager.shadow_thickness;
	geoSpec->margin_h = BB_MarginHeight(spell) + 
		spell->manager.shadow_thickness;
	geoSpec->no_geo_request = SpellSelectBoxNoGeoRequest;

	layoutPtr = &(geoSpec->layouts->row);
	boxPtr = geoSpec->boxes;


	/* Now position the form */

	/* position the Spell bar to the right end of the 
	   parent widget - and increment the line */

	layoutPtr->sticky_end = True;
	if (_XmGeoSetupKid(boxPtr, SPELL_form(spell))) {
		boxPtr += 2;
		++layoutPtr;
	}

	/* separator */
	vspace = BB_MarginHeight(spell);
	layoutPtr->space_above = vspace;
	layoutPtr->fix_up = _XmSeparatorFix;
	if (_XmGeoSetupKid(boxPtr, SPELL_Separator(spell))) {
		vspace = BB_MarginHeight(spell);
		boxPtr += 2;
		++layoutPtr;
	}


	/* Now Set up buttons */
	/* button row */
	layoutPtr->space_above = vspace;
	layoutPtr->even_width = True;
	layoutPtr->even_height = True;
	layoutPtr->fill_mode = XmGEO_CENTER;
	layoutPtr->fit_mode = XmGEO_WRAP;
	layoutPtr->uniform_border = True;
	layoutPtr->border = 0;
	layoutPtr->sticky_end = False;
	layoutPtr->space_between = 0;


	if (_XmGeoSetupKid(boxPtr, SPELL_startButton(spell))) {
		++boxPtr;
	}
	if (_XmGeoSetupKid(boxPtr, SPELL_CloseButton(spell))) {
		++boxPtr;
	}
	if (_XmGeoSetupKid(boxPtr, SPELL_HelpButton(spell))) {
		++boxPtr;
	}
	++layoutPtr;
	layoutPtr->space_above = vspace;
	layoutPtr->end = TRUE;
	return (geoSpec);
}


static Boolean
SpellSelectBoxNoGeoRequest(XmGeoMatrix geoSpec)
{
	/****************/

	if (BB_InSetValues(geoSpec->composite) && 
	    (XtClass(geoSpec->composite) == sdtSpellSelectionBoxWidgetClass)) {
		return (TRUE);
	}
	return (FALSE);
}


/* Widget destroyed. */
static void
Destroy(Widget fsb)
{
	/****************/
	/* CALL CLOSE IF MAMANGED */
	/* free text data if used from getString */
	/* LKW */

	return;
}


/*********************************************************************
 * This procedure is called to remove the child from the child list,
 * and to allow the parent to do any neccessary clean up.
 *********************************************************************/
static void
DeleteChild(Widget w)
{
	SDtSpellSelectionBoxWidget fs;


	if (XtIsRectObj(w)) {
		fs = (SDtSpellSelectionBoxWidget) XtParent(w);

		if (w == SPELL_misspelledLabel(fs)) {
			SPELL_misspelledLabel(fs) = NULL;
		} else if (w == SPELL_misspelledWid(fs)) {
			SPELL_misspelledWid(fs) = NULL;
		} else if (w == SPELL_update(fs)) {
			SPELL_update(fs) = NULL;
		} else if (w == SPELL_update_all(fs)) {
			SPELL_update_all(fs) = NULL;
		} else if (w == SPELL_skip(fs)) {
			SPELL_skip(fs) = NULL;
		} else if (w == SPELL_skip_all(fs)) {
			SPELL_skip_all(fs) = NULL;
		} else if (w == SPELL_learn(fs)) {
			SPELL_learn(fs) = NULL;
		} else if (w == SPELL_message_bar(fs)) {
			SPELL_message_bar(fs) = NULL;
		} else if (w == SPELL_search_scale(fs)) {
			SPELL_search_scale(fs) = NULL;
		} else if (w == SPELL_options(fs)) {
			SPELL_options(fs) = NULL;
		} else if (w == SPELL_List(fs)) {
			SPELL_List(fs) = NULL;
		} else if (w == SPELL_Text(fs)) {
			SPELL_Text(fs) = NULL;
		} else if (w == SPELL_Textlabel(fs)) {
			SPELL_Textlabel(fs) = NULL;
		} else if (w == SPELL_Separator(fs)) {
			SPELL_Separator(fs) = NULL;
		} else if (w == SPELL_startButton(fs)) {
			SPELL_startButton(fs) = NULL;
		} else if (w == SPELL_CloseButton(fs)) {
			SPELL_CloseButton(fs) = NULL;
		} else if (w == SPELL_HelpButton(fs)) {
			SPELL_HelpButton(fs) = NULL;
		}


	}
	(*((XmBulletinBoardWidgetClass) xmBulletinBoardWidgetClass)
                    ->composite_class.delete_child)( w) ;

	return;
}




/*-----------------------------------------------------------------*/
/* Creation functions used by Initialize        		   */
/*-----------------------------------------------------------------*/


/* Function	:	_SDtSpellCreateList()
 | 
 | Objective	:	Create the scrolling list that will be used to
 |			display word suggestions for misspelled words.
 |
 | Arguments	:	SDtSpellSelectionBoxWidget	- the spell widget
 |
 | Return value :	None.
 */

static void
_SDtSpellCreateList(SDtSpellSelectionBoxWidget sel)
{
	Arg             al[20];       /* Argument list and count. */
	register int    ac = 0;


	XtSetArg(al[ac], XmNselectionPolicy, XmBROWSE_SELECT);  ac++;
	XtSetArg(al[ac], XmNlistSizePolicy, XmRESIZE_IF_POSSIBLE);  ac++;
	XtSetArg(al[ac], XmNscrollBarDisplayPolicy, XmAS_NEEDED);  ac++;
	XtSetArg(al[ac], XmNnavigationType, XmSTICKY_TAB_GROUP); ++ac;
	XtSetArg(al[ac], XmNvisibleItemCount, 5); ++ac;

	SPELL_List(sel) = XmCreateScrolledList((Widget) SPELL_form(sel), 
					       "suggestionList", al, ac) ;

	XtAddCallback(SPELL_List(sel), XmNbrowseSelectionCallback,
                      _SDtSpellSuggListCB, (XtPointer) sel);
	XtAddCallback(SPELL_List(sel), XmNdefaultActionCallback,
		      _SDtSpellDefaultAction, (XtPointer) sel);


	XtManageChild(SPELL_List(sel)) ;
}


/* Function	:	_SDtSpellCreateText()
 |
 | Objective	:	Create the suggestion label and text field of
 |			the main spell GUI.
 |
 | Arguments	:	SDtSpellSelectionBoxWidget - the spell widget
 |
 | Return value	:	None.
 */

static void
_SDtSpellCreateText(SDtSpellSelectionBoxWidget sel)
{
	Arg             al[20];
	register int    ac = 0;
	XmString        xmstr;


	/* Create the suggestion label. */
	xmstr = XmStringCreateLocalized(
			catgets(_spellCat, 1, 3, "Suggestion:"));
	XtSetArg (al[ac], XmNlabelString, xmstr);  ac++;
	SPELL_Textlabel(sel) = 
		XmCreateLabelGadget((Widget) SPELL_form(sel),
				     "suggestionLabel", al, ac);
	XmStringFree(xmstr);
	XtManageChild(SPELL_Textlabel(sel));


	/* Create the suggestion text field. */
	ac = 0;	
	XtSetArg(al[ac], XmNcolumns, 32);  ac++;
	XtSetArg(al[ac], XmNresizeWidth, False);  ac++;
	XtSetArg(al[ac], XmNeditMode, XmSINGLE_LINE_EDIT);  ac++;
        XtSetArg( al[ac], XmNnavigationType, XmSTICKY_TAB_GROUP) ; ++ac;
	SPELL_Text(sel) =
		XmCreateTextField((Widget) SPELL_form(sel), "suggestionField",
				  al, ac);
	XtAddCallback(SPELL_Text(sel), XmNactivateCallback, 
		      _SDtSpellDefaultAction, (XtPointer) sel);
	XtAddCallback(SPELL_Text(sel), XmNvalueChangedCallback, 
		      _SDtSpellTextActivateCB, (XtPointer) sel);



#ifdef ACCEL
	/* Install text accelerators. */
        temp_accelerators = sel->core.accelerators;
	sel->core.accelerators = sel->selection_box.text_accelerators;
	XtInstallAccelerators(SPELL_Text(sel), (Widget) sel);
	sel->core.accelerators = temp_accelerators;
#endif

	XtManageChild(SPELL_Text(sel));
        return ;
}


/* Function	:	_SDtSpellCreateBottomButtons()
 |
 | Objective	;	Create the Start/Stop, Close, and Help buttons
 |			associated with the main spell widget.
 |
 | Argument	:	SDtSpellSelectionBoxWidget - the spell widget
 |
 | Return value	:	None.
 */

static void	
_SDtSpellCreateBottomButtons(SDtSpellSelectionBoxWidget sel)
{
	Arg                    al[20];
	register int           ac = 0;
	XmString               xmstr;
	static SpellHelpR      helpItem;


	/* Create the separator that separates the buttons from the
	 | rest of the spell GUI.
	 */
	XtSetArg(al[ac], XmNhighlightThickness, 0); ac++;
	SPELL_Separator(sel) =
		XmCreateSeparatorGadget((Widget) sel, "separator", al, ac);


	/* Create the Start/Stop button. */
	xmstr = XmStringCreateLocalized(
			catgets(_spellCat, 1, 12, "Start Check"));
    	SPELL_startButton(sel) =
		_XmBB_CreateButtonG((Widget) sel, xmstr, "startButton");
	XmStringFree(xmstr);


	/* Create the Close button. */
	xmstr = XmStringCreateLocalized(
			catgets(_spellCat, 1, 10, "Close"));
	SPELL_CloseButton(sel) =
		_XmBB_CreateButtonG((Widget) sel, xmstr, "closeButton");
	XmStringFree(xmstr);


	/* Create the Help button. */
	xmstr = XmStringCreateLocalized(
			catgets(_spellCat, 1, 11, "Help"));
	SPELL_HelpButton(sel) =
		_XmBB_CreateButtonG((Widget) sel, xmstr, "helpButton");
	XmStringFree(xmstr);


	/* Remove the default callbacks for the buttons. */
	XtRemoveAllCallbacks(SPELL_startButton(sel), XmNactivateCallback);
	XtRemoveAllCallbacks(SPELL_CloseButton(sel), XmNactivateCallback);
	XtRemoveAllCallbacks(SPELL_HelpButton(sel), XmNactivateCallback);


	/* Add callbacks for the buttons. */
	XtAddCallback(SPELL_startButton(sel), XmNactivateCallback,
		      _SDtSpellSelectPB, (XtPointer) XmDIALOG_OK_BUTTON);
	XtAddCallback(SPELL_CloseButton(sel), XmNactivateCallback,
		      _SDtSpellSelectPB, (XtPointer) XmDIALOG_CANCEL_BUTTON);


	/* For the help widget. */
	helpItem.help_parent_win = (Widget) sel;
	helpItem.help_location_id = SDTSPELL_HELP_TOP_LEVEL;
	XtAddCallback(SPELL_HelpButton(sel), XmNactivateCallback,
		      _SDtSpellHelpCB, (XtPointer) &helpItem);
}



/* Function	:	_SDtSpellCreateMispelled()
 |
 | Objective	:	Create the misspelled label and misspelled word
 |			associated with the main spell GUI.
 |
 | Argument	:	SDtSpellSelectionBoxWidget - spell widget
 |
 | Return value	:	None.
 */

void
_SDtSpellCreateMispelled(SDtSpellSelectionBoxWidget wid)
{
	Display         *display;
        XFontStruct     *font;
	XmFontListEntry entry;
        XmFontList      fontlist;
	XmString        xmstr;
	Arg             args[10];
	int             n;


	/* Create a font for the misspelled word. */
	display = XtDisplay(wid);
	font = XLoadQueryFont(display, BOLD_FONT);
	entry = XmFontListEntryCreate("tag1", XmFONT_IS_FONT, font);
	fontlist = XmFontListAppendEntry(NULL, entry);
	XtFree(entry);


	/* Create the misspelled label. */
	xmstr = XmStringCreateLocalized(catgets(_spellCat, 1, 2, "Word:"));
	SPELL_misspelledLabel(wid) = 
		_XmBB_CreateLabelG((Widget) SPELL_form(wid),
				   xmstr, "misspelledLabel");
	XmStringFree(xmstr);


	/* Create the misspelled word, and assign the bold font to it. */
	xmstr = XmStringCreateLocalized("                  ");
	SPELL_misspelledWid(wid) = _XmBB_CreateLabelG((Widget) SPELL_form(wid),
						      xmstr, "misspelled");
	XmStringFree(xmstr);
	XtVaSetValues(SPELL_misspelledWid(wid),
                      XmNfontList, fontlist,
		      NULL);


	XtManageChild(SPELL_misspelledLabel(wid));
	XtManageChild(SPELL_misspelledWid(wid));
}


/* Function	:	_SDtSpellCreateButtons()
 |
 | Objective	:	Create the spell widgets operator buttons.
 |
 | Argument	:	SDtSpellSelectionBoxWidget - the spell widget
 |
 | Return value	:	None.
 */

void
_SDtSpellCreateButtons(SDtSpellSelectionBoxWidget wid)
{
	XmString        xmstr;
	Widget          rc;
	Arg             args[10];
	int             n = 0;


	/* Create the Skip button. */
	xmstr = XmStringCreateLocalized(catgets(_spellCat, 1, 4, "Skip"));
	SPELL_skip(wid) = _XmBB_CreateButtonG((Widget) SPELL_form(wid),
					      xmstr, "skipButton");
	XmStringFree(xmstr);
	XtManageChild(SPELL_skip(wid));
	XtAddCallback(SPELL_skip(wid), XmNactivateCallback,
		      _SDtSpellOperatePB, (XtPointer) SKIP_BUTTON);


	/* Create the Skip All button. */
	xmstr = XmStringCreateLocalized(catgets(_spellCat, 1, 5, "Skip All"));
	SPELL_skip_all(wid) = _XmBB_CreateButtonG((Widget) SPELL_form(wid),
						  xmstr, "skipAllButton");
	XmStringFree(xmstr);
	XtManageChild(SPELL_skip_all(wid));
	XtAddCallback(SPELL_skip_all(wid), XmNactivateCallback, 
		      _SDtSpellOperatePB, (XtPointer) SKIP_ALL_BUTTON);


	/* Create the Change button. */
	xmstr = XmStringCreateLocalized(catgets(_spellCat, 1, 6, "Change"));
	SPELL_update(wid) = _XmBB_CreateButtonG((Widget) SPELL_form(wid),
						xmstr, "changeButton");
	XmStringFree(xmstr);
	XtManageChild(SPELL_update(wid));
	XtAddCallback(SPELL_update(wid), XmNactivateCallback, 
		      _SDtSpellOperatePB, (XtPointer) CHANGE_BUTTON);


	/* Create the Change All button. */
	xmstr = XmStringCreateLocalized(catgets(_spellCat, 1, 7, "Change All"));
	SPELL_update_all(wid) = _XmBB_CreateButtonG((Widget) SPELL_form(wid),
						    xmstr, "changeAllButton");
	XmStringFree(xmstr);
	XtManageChild(SPELL_update_all(wid));
	XtAddCallback(SPELL_update_all(wid), XmNactivateCallback, 
		      _SDtSpellOperatePB, (XtPointer) CHANGE_ALL_BUTTON);


	/* Create the Learn button. */
	xmstr = XmStringCreateLocalized(catgets(_spellCat, 1, 8, "Learn Word"));
	SPELL_learn(wid) = _XmBB_CreateButtonG((Widget) SPELL_form(wid),
					       xmstr, "learnButton");
	XmStringFree(xmstr);
	XtManageChild(SPELL_learn(wid));
	XtAddCallback(SPELL_learn(wid), XmNactivateCallback,
		      _SDtSpellOperatePB, (XtPointer) LEARN_BUTTON);
}


/* Function	:	_SDtSpellCreateMessage()
 |
 | Objective	:	Create the message area where spell messages will
 |			be displayed.
 |
 | Argument	:	SDtSpellSelectionBoxWidget - the spell widget
 |
 | Return value	:	None.
 */

void
_SDtSpellCreateMessage(SDtSpellSelectionBoxWidget wid)
{
	XmString        xmstr;

	xmstr = XmStringCreateLocalized(catgets(_spellCat, 3, 14, "Ready"));
	SPELL_message_bar(wid) = _XmBB_CreateLabelG((Widget) SPELL_form(wid),
						    xmstr, "messageBar");
	XtVaSetValues(SPELL_message_bar(wid), XmNalignment, 
		      XmALIGNMENT_BEGINNING, NULL);
	XmStringFree(xmstr);

	XtManageChild(SPELL_message_bar(wid));
	return;
}


/* Function	:	_SDtSpellCreateScale()
 |
 | Objective	:	Create the scale that displays the spell checker's
 |			progress.
 |
 | Argument	:	SDtSpellSelectionBoxWidget - the spell widget
 |
 | Return value	:	None.
 */

void
_SDtSpellCreateScale(SDtSpellSelectionBoxWidget wid)
{
	Arg             al[10];
	Cardinal        ac = 0;
	Widget          button;


	XtSetArg(al[ac], XmNorientation, XmHORIZONTAL); ac++;
	XtSetArg(al[ac], XmNscaleMultiple, 1); ac++;
	XtSetArg(al[ac], XmNdecimalPoints, 0); ac++;
	XtSetArg(al[ac], XmNminimum, 0); ac++;
	XtSetArg(al[ac], XmNmaximum, 100); ac++;
	XtSetArg(al[ac], XmNheight, 15); ac++;
	XtSetArg(al[ac], XmNwidth, XmSPELL_SCALE_MIN_SIZE); ac++;
	XtSetArg(al[ac], "slidingMode", True); ac++;

	SPELL_search_scale(wid) = 
		XmCreateScale((Widget) SPELL_form(wid), "scale", al, ac);
	XtManageChild( SPELL_search_scale(wid));

	return;
}


/* Function	:	_SDtSpellCreateOptions()
 |
 | Objective	:	Create the options button and the options dialog
 |			goes with it.
 |
 | Argument	:	SDtSpellSelectionBoxWidget - the spell widget
 |
 | Return value	:	None.
 */

void
_SDtSpellCreateOptions(SDtSpellSelectionBoxWidget wid)
{
	XmString        xmstr;


	/* Create the Options button. */
	xmstr = XmStringCreateLocalized(
			catgets(_spellCat, 1, 9, "Options..."));
	SPELL_options(wid) =
		 _XmBB_CreateButtonG((Widget) SPELL_form(wid),
				     xmstr, "optionsButton");
	XmStringFree(xmstr);
	XtManageChild(SPELL_options(wid));


	/* Create the options dialog. */
	 _SDtSpellCreateOptionsDlg(wid);


	/* Add the callback for the options button. */
	XtAddCallback(SPELL_options(wid), XmNactivateCallback, 
                      _SDtSpellOptionButtonPB, (XtPointer) wid);
}


/* Function	:	_SDtSpellCalcSize()
 |
 | Objective	:	Calculate the minimum size required by the
 |			spell widget in order to display properly.
 |
 | Arguments	:	Widget	- the spell widget.
 |
 | Return value	:	None.
 */

void 	
_SDtSpellCalcSize(Widget w) 
{
	Dimension       msg_w;
	Dimension       btn_w = 0;
	Dimension       tmp_w = 0;


	/* Scale size is the largest of :
	 |	minimum scale size
	 |	2 * size of largest button + 20
	 |	error message
	 */
	XtVaGetValues(SPELL_message_bar(w), XmNwidth, &msg_w, NULL);
	XtVaGetValues(SPELL_update(w), XmNwidth, &btn_w, NULL);
	XtVaGetValues(SPELL_update_all(w), XmNwidth, &tmp_w, NULL);
	if (tmp_w > btn_w)
		btn_w = tmp_w;

	XtVaGetValues(SPELL_skip(w), XmNwidth, &tmp_w, NULL);
	if (tmp_w > btn_w)
		btn_w = tmp_w;

	XtVaGetValues(SPELL_skip_all(w), XmNwidth, &tmp_w, NULL);
	if (tmp_w > btn_w)
		btn_w = tmp_w;

	XtVaGetValues(SPELL_learn(w), XmNwidth, &tmp_w, NULL);
	if (tmp_w > btn_w)
		btn_w = tmp_w;

	XtVaGetValues(SPELL_options(w), XmNwidth, &tmp_w, NULL);
	if (tmp_w > btn_w)
		btn_w = tmp_w;

	/* now set all the buttons to the max size */
	XtVaSetValues(SPELL_update(w),
		      XmNwidth, btn_w,
		      XmNresize, 
		      False, NULL);
	XtVaSetValues(SPELL_update_all(w),
		      XmNwidth, btn_w,
		      XmNresize, False,
		      NULL);
	XtVaSetValues(SPELL_skip(w),
		      XmNwidth, btn_w,
		      XmNresize, False,
		      NULL);
	XtVaSetValues(SPELL_skip_all(w),
		      XmNwidth, btn_w,
		      XmNresize, False,
		      NULL);
	XtVaSetValues(SPELL_learn(w),
		      XmNwidth, btn_w,
		      XmNresize, False,
		      NULL);
	XtVaSetValues(SPELL_options(w),
		      XmNwidth, btn_w,
		      XmNresize, False,
		      NULL);

	/* calculate scale size */
	tmp_w = 2*btn_w + 20;
	if (tmp_w < msg_w) tmp_w = msg_w;
	if (tmp_w < XmSPELL_SCALE_MIN_SIZE) tmp_w = XmSPELL_SCALE_MIN_SIZE;
	XtVaSetValues(SPELL_search_scale(w), XmNwidth, tmp_w, NULL);	


	/* labels should be the same size */
	XtVaGetValues(SPELL_Textlabel( w), XmNwidth, &btn_w, NULL);
	XtVaGetValues(SPELL_misspelledLabel( w), XmNwidth, &tmp_w, NULL);
	if (btn_w < tmp_w)
		btn_w = tmp_w;
	XtVaSetValues(SPELL_Textlabel(w),
		      XmNwidth, btn_w,
		      XmNresize, False,
		      NULL);
	XtVaSetValues(SPELL_misspelledLabel(w),
		      XmNwidth, btn_w,
		      XmNresize, False,
		      NULL);
}


/*-----------------------------------------------------------------*/
/* set functions used to set the values of child widget            */
/*-----------------------------------------------------------------*/

/* Function	:	_SDtSpellSetMisspelled()
 |
 | Objective	:	Display the misspelled word in the spell GUI.
 |
 | Argument	:	Widget		- the spell widget
 |			char *		- the misspelled word
 |
 | Return value	:	None.
 */

void
_SDtSpellSetMisspelled(Widget wid, char *word)
{
	XmString        xmstr;


	/* If word is NULL, then blank the field out. */
	if (word == NULL)
		word = NULL_STRING;
	xmstr = XmStringCreateLocalized(word);
	XtVaSetValues(SPELL_misspelledWid(wid), XmNlabelString,
		      xmstr, NULL);
	XmStringFree(xmstr);
}


/* Function	:	_SDtSpellSetMsg()
 |
 | Objective	:	Set the spell widget's message area with the
 |			specified message, in order to inform user about
 |			particular events and/or conditions.
 |
 | Argument	:	Widget		- the spell widget
 |			char *		- the message to display
 |
 | Return value	:	None.
 */

void
_SDtSpellSetMsg(Widget wid, char *msg)
{
	XmString        xmstr;


	/* If message is null, then blank out the message area. */
	if (msg != NULL)
		xmstr = XmStringCreateLocalized(msg);
	else
		xmstr = XmStringCreateLocalized(NULL_STRING);
	XtVaSetValues(SPELL_message_bar(wid), XmNlabelString, xmstr, NULL);
	XmStringFree(xmstr);
}


/* Function	:	_SDtSpellSetList()
 | 
 | Objective    :       Populate the suggestion list with list of 
 |			possible spell suggestions retrieved by the
 |			spell engine.
 | 
 | Arguments	:	Widget		- the spell widget
 |			char **		- list of spell suggestions
 |			int		- number of spell suggestions
 |
 | Return value :       None.
 */

void
_SDtSpellSetList(Widget wid, char **suggestions, int cnt)
{
	int             i;
	XmString       *xmstrlist;


	/* Clear the list's contents. */
	XmListDeleteAllItems(SPELL_List(wid));


	/* No suggestions. */
	if (cnt == 0 || suggestions == NULL) {

		/* Inform the user that there are no suggestions found. */
		_SDtSpellSetMsg(wid, catgets(_spellCat, 3, 16,
					     "No suggestions found."));
		SPELL_listPosition(wid) = -1;
		return;
	}


	/* Create the list contents from the suggestion list input.
	 | Put the first list item in the suggestion text field;
	 | select the first item in the list, and adjust the viewer
	 | so that the list is displayed from the start.
	 */
	xmstrlist = (XmString *) XtMalloc(sizeof(XmString) * cnt + 1);
	for (i = 0; i < cnt; i++)
		xmstrlist[i] = XmStringCreateLocalized(suggestions[i]);
	XmListAddItems(SPELL_List(wid), xmstrlist, cnt, 0);
	XmTextSetString(SPELL_Text(wid), suggestions[0]);
	XmListSelectPos(SPELL_List(wid), 1, False);
	SPELL_listPosition(wid) = 1;


	/* Free memory. */
	for (i = 0; i < cnt; i++) {
		XmStringFree(xmstrlist[i]);
	}
	XtFree((char *) xmstrlist);


	/* Set up the suggestion data of the spell widget. */
	if (SPELL_suggestions(wid) != NULL) {
		for (i = 0; i < SPELL_sug_count(wid); i++)
			free(SPELL_suggestions(wid)[i]);
		free((char *) SPELL_suggestions(wid));
		SPELL_suggestions(wid) = NULL;
	}
	SPELL_suggestions(wid) = suggestions;
	SPELL_sug_count(wid) = cnt;
}


/* Function	:	_SDtSpellSetSensitive()
 |
 | Objective	:	Set the sensitivity of the spell widget's operator
 |			buttons to the desired sensitivity.
 |
 | Argument	:	Widget		- the spell widget
 |			Boolean		- flag if true, makes the operator
 |					  buttons sensitive.
 |
 | Return value	:	None.
 */

void 
_SDtSpellSetSensitive(Widget wid, Boolean set)
{
	/* Only set the sensitivity of the Skip, Skip All,
	 | Change, Change All, and Learn buttons.  The
	 | Options button should be active at all times.
	 */
	XtSetSensitive(SPELL_update(wid), set);
	XtSetSensitive(SPELL_update_all(wid), set);
	XtSetSensitive(SPELL_skip(wid), set);
	XtSetSensitive(SPELL_skip_all(wid), set);
	if (SPELL_personal(wid))
		XtSetSensitive(SPELL_learn(wid), set);


	/* Set the sensitivity of the suggestion text
	 | field and list.
	 */
	XtSetSensitive(SPELL_Text(wid), set);
	XtSetSensitive(SPELL_List(wid), set);
}


/* Function	:	_SDtSpellSetOkLabel()
 |
 | Objective	:	Set the Ok button of the selection widget (which
 |			the spell widget is based on) to either "Start Check"
 |			or "Stop Check"
 |
 | Argument	:	Widget		- the spell widget
 |			char *		- the label to assign to the button;
 |					  the label should be localized
 |
 | Return value	:	None.
 */

void
_SDtSpellSetOkLabel(Widget wid, char *label)
{
	XmString        xmstr;

	if (label == NULL)
		return;
	xmstr = XmStringCreateLocalized(label);

	XtVaSetValues(SPELL_startButton(wid), XmNlabelString, xmstr, NULL);

	XmStringFree(xmstr);
}



/* Function	:	_SDtSpellSetGauge()
 |
 | Objective	:	Set the gauge to some calculated value
 |
 | Argument	:	Widget		- the spell widget
 |			int		- number of bytes of word that was
 |					  just recently processed
 | Return value	:	None.
 */
Boolean
_SDtSpellSetGauge(Widget wid, int processed)
{
	int             pos = 0;
	static int      value = 0;


	/* Calculate the progress indicator by keeping a running
	 | total of the number of bytes processed, and dividing
	 | this total by the size.
	 */
	value += processed;
	pos = (value * 100) / SPELL_size(wid);
	if (pos < 0) {
		pos = 0;
		value = 0;	/* Reset to 0 if negative; occurs when 
				 * spelling check has been interrupted.
				 */
	}

	if (pos > 100) {
		pos = 100;
		value = 0;	/* Spell check completed normally, so
				 * start from 0 again.
				 */
	}

	XtVaSetValues(SPELL_search_scale(wid), XmNvalue,
		      pos, NULL);
}


/*-----------------------------------------------------------------*/
/* Callback functions for Child Widgets                            */
/*-----------------------------------------------------------------*/


/* Function	:	_SDtSpellSelectPB()
 | 
 | Objetive	:	Callback that is invoked when either the 
 |			Start/Stop, Close, or Help button is pressed
 |			(from within the main spell GUI).
 */
void
_SDtSpellSelectPB(Widget wid, XtPointer which_button, XtPointer call)
{
	SDtSpellSelectionBoxWidget spell =
		(SDtSpellSelectionBoxWidget) XtParent(wid);
	XmAnyCallbackStruct *callback = (XmAnyCallbackStruct *) call;

	char           *emsg = NULL;
	XmString        xmstr;
	char           *label;
	int             comparison = ~0;   /* Make sure its not 0. */


	switch ((long) which_button) {

	case XmDIALOG_OK_BUTTON:

		/* Start/Stop selected - Do we have a data widget that
		 | will be the source of our data?
		 */
		if (SPELL_target(spell) == NULL) {
			/* ERROR */
			_XmWarning(wid, "Unable to search because no searchWidget is set\n");
			return;
		}


		/* If Stop Check is active, stop the spelling checker,
		 | otherwise, start the spelling checker.
		 */
		XtVaGetValues(wid, XmNlabelString, &xmstr, NULL);
		XmStringGetLtoR(xmstr, XmFONTLIST_DEFAULT_TAG, &label);
		XmStringFree(xmstr);
		if (label != (char *) NULL) {

			comparison = strcmp(label, catgets(_spellCat, 1, 13,
							   "Stop Check"));
			XtFree(label);
		}
		if (comparison == 0) {

			SpellEngP	engine = SPELL_engine(spell);


			/* Inform the user that the spelling checker
			 | has stopped, and reset the button label to
			 | Start Check.  Reset the other GUI contents,
			 | and set done to True.
			 */
			_SDtSpellSetMsg((Widget) spell, 
					catgets(_spellCat, 3, 3, 
						"Check Stopped"));
			_SDtSpellSetOkLabel((Widget) spell, 
					    catgets(_spellCat, 1, 12, 
						    "Start Check"));
			XmTextSetString(SPELL_Text(spell), NULL_STRING);
			_SDtSpellClearSearch((Widget) spell);
			_SDtSpellSetSensitive((Widget) spell, False);
			SPELL_done(spell) = True;
			SPELL_wrap_around(spell) = False;


			/* Remove any highlighted words within the text,
			 | and reset the insertion cursor.
			 */
			SPELL_clearSelection(spell);
			XmTextSetInsertionPosition(SPELL_TxtWid(spell), 
				(XmTextPosition) (SPELL_cursor(spell) + 
				                  SPELL_adjust(spell)));


			/* Reset the prev_word structure to the NULL
			 | string, so that when spelling check is restarted,
			 | the engine doesn't check to previous word.  If it
			 | does, we can get incorrect capitalization or
			 | double word errors.
			 */
			strcpy((char *) engine->mainDict->icBuff->prev_word,
			       NULLSTR);


			/* Remove the work proc before returning. */
			XtRemoveWorkProc(SPELL_workID((Widget) spell));
			return;
		}


		/* Start the spelling checker. */
		if (_SDtSpellLoadText((Widget) spell)) {

			/* Reset the gauge 0. */
			_SDtSpellSetGauge((Widget) spell, -SPELL_size(spell));


			/* Start the spelling checker work proc. */
			_SDtSpellSetMsg((Widget) spell, 
					catgets(_spellCat, 3, 1, 
						"Checking..."));
			_SDtSpellSetSensitive((Widget) spell, False);
			_SDtSpellSetOkLabel((Widget) spell, 
					    catgets(_spellCat, 1, 13, 
						    "Stop Check"));
			SPELL_workID(spell) = 
				XtAppAddWorkProc(SPELL_appcontext(spell),
					 (XtWorkProc) _SDtSpellCheck,
					 (XtPointer) ((Widget) spell));
		} else {
			_SDtSpellSetMsg((Widget) spell, catgets(_spellCat, 3, 11, "No text to check for spelling."));
		}
		break;


	case XmDIALOG_CANCEL_BUTTON:

		/* Close selected. */
		if (SPELL_engine(spell) == NULL)
			break;

		if (spell->bulletin_board.shell && 
		    spell->bulletin_board.auto_unmanage) {

			XtUnmanageChild((Widget) spell);

		} else {

			emsg = _SDtSpellCloseSpell(SPELL_engine(spell));
			if (emsg != NULL) {
				/* ERROR */
				_SDtSpellError((Widget) spell, emsg);
				XtFree(emsg);
			}
			SPELL_clearSelection((Widget) spell)
			_SDtSpellClearSearch((Widget) spell);
		}
		break;


	case XmDIALOG_HELP_BUTTON:

		/* Help selected. */
		if (spell->manager.help_callback) {
			XtCallCallbackList(((Widget) wid),
					   spell->manager.help_callback,
					   call);
		} else {
			_XmManagerHelp((Widget) wid,
				       callback->event,
				       NULL, NULL);
		}
		break;

	}
}


/* Function	:	_SDtSpellOperatePB()
 |
 | Objective	:	Xt callback that operates on the data, based on 
 |			what button operation was selected.
 */

void
_SDtSpellOperatePB(Widget wid, XtPointer which_button, XtPointer call)
{
	SDtSpellSelectionBoxWidget spell = 
		(SDtSpellSelectionBoxWidget) XtParent(XtParent(wid));


	char           *txt;


	switch ((long) which_button) {

	case SKIP_BUTTON:
	case SKIP_ALL_BUTTON:
		_SDtSpellSkipWord((Widget) spell,
				  ((long) which_button == SKIP_ALL_BUTTON));
		break;


	case CHANGE_BUTTON:
	case CHANGE_ALL_BUTTON:
		txt = XmTextGetString(SPELL_Text(spell));
		_SDtSpellReplaceWord((Widget) spell, txt,
				    ((long) which_button == CHANGE_ALL_BUTTON));
		XtFree(txt);
		break;


	case LEARN_BUTTON:
		_SDtSpellLearnWord((Widget) spell, SPELL_word(spell));
		break;
	}


	/* Start the spelling checker work proc. */
	XmTextSetString(SPELL_Text(spell), NULL_STRING);
	_SDtSpellClearSearch((Widget) spell);
	_SDtSpellSetMsg((Widget) spell, 
			catgets(_spellCat, 3, 1, "Checking..."));
	_SDtSpellSetSensitive((Widget) spell, False);
	_SDtSpellSetOkLabel((Widget) spell, 
			    catgets(_spellCat, 1, 13, "Stop Check"));
	SPELL_workID((Widget) spell) = 
		XtAppAddWorkProc(SPELL_appcontext((Widget) spell),
				 (XtWorkProc) _SDtSpellCheck,
				 (XtPointer) ((Widget) spell));
}


/* Function	:	_SDtSpellOptionButtonPB()
 |
 | Objective	:	Xt callback that brings up the options dialog
 |			when the Options button is pressed.
 */

void
_SDtSpellOptionButtonPB(Widget wid, XtPointer client, XtPointer call)
{
	SDtSpellSelectionBoxWidget spell = (SDtSpellSelectionBoxWidget) client;


	if (SPELL_option_panel(spell))
		_SDtSpellManageOptions(spell);
}


/* Function	:	_SDtSpellTextActivateCB()
 |
 | Objective	:	Xt callback that is invoked when the user types
 |			in text in the suggestion text field.
 */

void
_SDtSpellTextActivateCB(Widget wid, XtPointer client, XtPointer call)
{
	SDtSpellSelectionBoxWidget  spell = (SDtSpellSelectionBoxWidget) client;
	char       		    *txt;


 	/* Activate change and change all buttons. */
	XtSetSensitive(SPELL_update(spell), True);
	XtSetSensitive(SPELL_update_all(spell), True);


	/* Get the text in the suggestion field, and see if 
	 | the list is affected, i.e., if user types in something
	 | that is in the list, then let the list select and
	 | highlight it.
	 */
	XtVaGetValues(wid, XmNvalue, &txt, NULL);
	if (strlen(txt) <= 0)
		return;
	_SDtSpellUpdateList((Widget) spell, txt);
}


/* Function	:	_SDtSpellSuggListCB()
 |
 | Objective	:	Xt callback that is called when a suggested correction
 |			in the list is selected.
 */

void
_SDtSpellSuggListCB(Widget wid, XtPointer client, XtPointer call)
{
	SDtSpellSelectionBoxWidget  spell = (SDtSpellSelectionBoxWidget) client;
	XmListCallbackStruct       *cb = (XmListCallbackStruct *) call;
	int                         pos;

	pos = cb->item_position;

	if (!XmListPosSelected(wid, pos )) {

		/* unselecting */
		XmTextSetString(SPELL_Text(spell), NULL_STRING);
		SPELL_listPosition(spell) = -1;

	} else {

		XmTextSetString(SPELL_Text(spell), 
				SPELL_suggestions(spell)[pos - 1]);
		SPELL_listPosition(spell) = pos;
	}

}


void
_SDtSpellPopdown(Widget wid, XtPointer client, XtPointer call)
{
	SDtSpellSelectionBoxWidget spell = 
		(SDtSpellSelectionBoxWidget) client;
	char *error;


	error = _SDtSpellCloseSpell(SPELL_engine(spell));
	if (error != NULL) {

		/* ERROR */
		_SDtSpellError((Widget) spell, error);
		XtFree(error);
	}
	SPELL_clearSelection((Widget) spell);
	_SDtSpellClearSearch((Widget) spell);
}


/*-----------------------------------------------------------------*/
/* General Functions used during spell check  */
/*-----------------------------------------------------------------*/

/* Function	:	_SDtSpellInitEnv()
 |
 | Objective	:	Initialize environment variables and catalog.
 |
 | Arguments	:	None.
 |
 | Return value	:	None.
 */

void 
_SDtSpellInitEnv(void)
{
	static Boolean initialized = False;

	/* Only want to initialize once. */
	if (initialized)
		return;
	initialized = True;

	/* Initialized DT environment, so that right paths are set. */
        _DtEnvControl(DT_ENV_SET);

	/* Initialize catalog. */
        if (_spellCat == (nl_catd) 0) {

                /* Just need to open the catalog file once. */
                _spellCat = catopen(SDTSPELL_MSG_CATALOG, NL_CAT_LOCALE);
	        if (_spellCat == (nl_catd) - 1)
 	               fprintf(stderr, "WARNING: could not open message catalog: %s\nDefault standard messages will be used.\n", SDTSPELL_MSG_CATALOG);
	}
}

/* Function	:	_SDtSpellClearSearch()
 |
 | Objective	:	Delete the contents of the suggestion list,
 |			and blank out the misspelled word.  Set the
 |			spell command buttons to insensitive, since
 |			there is no misspelled word to correct.
 |
 | Arguments	:	Widget		- the spell widget
 |
 | Return value	:	None.
 */

void
_SDtSpellClearSearch(Widget wid)
{
	XmListDeleteAllItems(SPELL_List(wid));
	_SDtSpellSetMisspelled(wid, (char *) NULL);
	XmTextSetString(SPELL_Text(wid), NULL_STRING);
	_SDtSpellSetSensitive(wid, False);
}


/* Function	:	_SDtSpellError()
 |
 | Objective	:	Create a generic error dialog with only an OK
 |			button.
 |
 | Arguments	:	Widget		- spell widget
 |			char *		- message to be displayed in the
 |					  dialog
 | Return Value	:	None.
 */

void
_SDtSpellError(Widget wid, char *msg)
{
	XmString xmstr;
	Arg args[4];
	int n = 0;


	/* Create the spell dialog. */
	if (SPELL_error(wid) == NULL) {

	        xmstr = XmStringCreateLocalized(
			catgets(_spellCat, 3, 12, "Spelling Checker : Error"));
		XtSetArg(args[n], XmNdialogTitle, xmstr); n++;
		XtSetArg(args[n], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL);
		n++;
		XtSetArg(args[n], XmNmessageAlignment, XmALIGNMENT_CENTER);
		n++;
		SPELL_error(wid) = XmCreateErrorDialog(wid, "error",
						       args, n);
		XmStringFree(xmstr);

		XtUnmanageChild(XmMessageBoxGetChild(SPELL_error(wid),
						     XmDIALOG_CANCEL_BUTTON));
		XtUnmanageChild(XmMessageBoxGetChild(SPELL_error(wid),
						     XmDIALOG_HELP_BUTTON));
	}


	/* Attach the message to the dialog and pop it up. */
	xmstr = XmStringCreateLocalized(msg);
	n = 0;
	XtSetArg(args[n], XmNmessageString, xmstr); n++;
	XtSetValues(SPELL_error(wid), args, n);
	XtManageChild(SPELL_error(wid));
}


/* Function	:	_SDtSpellVerifyWord()
 |
 | Objective	:	Verify the spelling of the given word.
 |
 | Argument	:	Widget		- spell widget
 |			char *		- word to verify
 |
 | Return Value	:	Boolean True if the word is correctly spelled,
 |			False otherwise.
 */

Boolean
_SDtSpellVerifyWord(Widget wid, char *word)
{
	wordReturn      ret;		/* Spell suggestions. */
	char           *err = NULL;


	/* Initialize so that there are no spell suggestions. */
	ret.alt_num = 0;
	ret.alternatives = NULL;


	/* No spell engine - return True. */
	if (SPELL_engine(wid) == NULL)
		return True;


	/* Check the word and get any suggestions if incorrect. */
	if (_SDtSpellCheckWord(SPELL_engine(wid), word, &ret,
			       &err, &(SPELL_start(wid)), &(SPELL_end(wid))))
		return True;


	/* A system error occurred during correction. */
	if (err != NULL) {
		_SDtSpellError(wid, err);
		XtFree(err);
	}


	/* Display the type of spelling error, and display alternatives,
	 | if there are any.
	 */
	SPELL_word(wid) = strdup(ret.word);
	_SDtSpellSetMsg(wid, _SDtSpellErrorTypes(ret.errortype));
	_SDtSpellSetList(wid, ret.alternatives, ret.alt_num);
	free(ret.word);
	return False;
}


/* Function	:	_SDtSpellLoadText()
 |
 | Objective	:
 | Arguments	:	Widget		- the spell widget
 | Return Value	:	Boolean True
 |			False otherwise.
 */

Boolean
_SDtSpellLoadText(Widget wid)
{
	XmTextPosition	start, end;  /* Start and end of primary selection. */
	int		i;
	char           *text;


	/* This function needs to be changed to use XmTextGetSource.
	 | For now load everything into a buffer.  First, discard the
	 | old data stream...
	 */
	if (SPELL_data(wid) != NULL) {
		XtFree(SPELL_data(wid));
		SPELL_data(wid) = NULL;
	}


	/* ...and any misspelled words from the old data stream... */
	if (SPELL_word(wid) != NULL) {
		free(SPELL_word(wid));
		SPELL_word(wid) = NULL;
	}


	/* ...as well as any old suggestions from the previous data stream. */
	if (SPELL_suggestions(wid) != NULL) {
		for (i = 0; i < SPELL_sug_count(wid); i++)
			free(SPELL_suggestions(wid)[i]);
		free((char *) SPELL_suggestions(wid));
		SPELL_suggestions(wid) = NULL;
		SPELL_sug_count(wid) = 0;
	}


	/* Now get the data that we want to check. */
	if (SPELL_TxtWid(wid)) {
		/* Does the user want to correct a selection rather
		 | than the entire document?  Check if primary selection
		 | had data.
		 */
		SPELL_data(wid) = XmTextGetSelection(SPELL_TxtWid(wid));
		if (SPELL_data(wid)) {
			/* Get the size of the selection, and then
			 | remove the highlight.  Also set the start
			 | and current position to the start of the
			 | selection.
			 */
			SPELL_size(wid) = strlen(SPELL_data(wid));
			XmTextGetSelectionPosition(SPELL_TxtWid(wid),
						   &start, &end);
			XmTextClearSelection(SPELL_TxtWid(wid),
				XtLastTimestampProcessed(XtDisplay(wid)));
			SPELL_adjust(wid) = (int) start;
		} else {

			/* Check the entire document. */
			SPELL_data(wid) = XmTextGetString(SPELL_TxtWid(wid));
			SPELL_size(wid) = strlen(SPELL_data(wid));
			text = SPELL_data(wid);


			/* Need to adjust, in case user sets insertion
			 * cursor in the middle of the word.  Make sure
			 * start doesn't end up less than 0.
			 */
			SPELL_adjust(wid) = 0;
			start = XmTextGetCursorPosition(SPELL_TxtWid(wid));
			while (text[start] != ' ' && text[start] != '\n' &&
			       text[start] != '\t' && start != 0) {

				start--;
			}
			SPELL_cursor(wid) = start;
			SPELL_current(wid) = SPELL_cursor(wid);
		}
	} else {
		SPELL_data(wid) = NULL;
		SPELL_size(wid) = 0;
	}


	/* Reset the starting and ending pointers. */
	SPELL_start(wid) = -1;
	SPELL_end(wid) = -1;
	SPELL_done(wid) = False;


	/* Couldn't get the data from the widget. */
	if (SPELL_data(wid) == NULL)
		return False;


	/* Grab new data. */
	if (SPELL_size(wid) <= 0) {
		XtFree(SPELL_data(wid));
		SPELL_data(wid) = NULL;
		return False;
	}
	return True;
}


/* Functon	:	_SDtSpellDispatch()
 | 
 | Objective	:	Force an update of the spell widget while it's
 |			processing.
 |
 | Argument	:	Widget		- spell widget
 |
 | Return Value	:	None.
 */

void
_SDtSpellDispatch(Widget wid)
{
	/* If we dont make SpellCheck into work proc paradim this should be
	 | expanded to dispatch more messages otherwise this will no longer
	 | be applicable
	 */

	XmUpdateDisplay(wid);
}


/* Function	:	_SDtSpellCheck()
 |
 | Objective	:	Start correcting for spelling errors.
 |
 | Argument	:	Widget		- spell widget
 |
 | Return value	:	Boolean True if spell checking works, false
 |			otherwise.
 */

Boolean
_SDtSpellCheck(XtPointer client)
{
	Widget          wid = (Widget) client;
	char           *word;
	char          **suggestions;


	/* Start spell checking by breaking down the data stream into
	 | single words, and then feeding each one to the spell engine.
	 */
	word = _SDtSpellGetNextWord(wid);
	if (word != NULL) {

		_SDtSpellSetGauge(wid, strlen(word));
		_SDtSpellDispatch(wid);


		/* Verify the spelling of the word. */
		if (!_SDtSpellVerifyWord(wid, word)) {


			/* Incorrectly spelled word.  Highlight the word
			 | in its source, as well as display it prominently
			 | in the spell GUI.
			 */
			SPELL_setHighlight(wid,  
					   SPELL_start(wid) + SPELL_adjust(wid),
					   SPELL_end(wid) + SPELL_adjust(wid));
			_SDtSpellSetMisspelled(wid, SPELL_word(wid));
			_SDtSpellSetSensitive(wid, True);


			/* Inactivate change and change all buttons if
			 * there are no suggestions (can be determined if
			 * list position is -1
			 */
			if (SPELL_listPosition(wid) == -1) {
				XtSetSensitive(SPELL_update(wid), False);
				XtSetSensitive(SPELL_update_all(wid), False);
			}

			return(True);
		} else {
			/* Continue with correction. */
			return(False);
		}
	} else {

		SpellEngP       spellEngine = (SpellEngP) SPELL_engine(wid);


		/* Spell check finished processing the document. */
		SPELL_done(wid) = True;
		_SDtSpellSetGauge(wid, SPELL_size(wid));
		_SDtSpellSetMsg(wid, 
				catgets(_spellCat, 3, 2, "Check completed."));
		_SDtSpellSetSensitive(wid, False);
		_SDtSpellClearSearch(wid);


		/* Reset the global ignore and change word lists to
		 | NULL.  The user might type in new data, or might
		 | load in a new file.  We don't know, so err on the
		 | safe side that after a check has completed, start
		 | from scratch again.
		 */
		_freeList(spellEngine->ignoreWords);
		_freeList(spellEngine->globChanged);
		spellEngine->ignoreWords = (WordListP) NULL;
		spellEngine->globChanged = (WordListP) NULL;
		_SDtSpellSetSensitive(wid, False);


		/* Reset the prev_word structure to the NULL
		 | string, so that when spelling check is restarted,
		 | the engine doesn't check to previous word.  If it
		 | does, we can get incorrect capitalization or
		 | double word errors.
		 */
		strcpy((char *) spellEngine->mainDict->icBuff->prev_word,
		       NULLSTR);


		/* Change the button from Stop to Start. */
		_SDtSpellSetOkLabel(wid, catgets(_spellCat, 1, 12, 
						 "Start Check"));


		/* Set the insertion cursor to the original location,
		 | adding for adjustments to changes.
		 */
		XmTextSetInsertionPosition(SPELL_TxtWid(wid), 
			(XmTextPosition) (SPELL_cursor(wid) + 
				          SPELL_adjust(wid)));
		return(True);
	}
}


/* Function	:	_SDtSpellGetNextWord()
 | 
 | Objective	:	Break down the data stream into single words,
 |			so that the words can be fed into the engine
 |			for spell verification.
 |
 | Argument	:	Widget		- the spell widget
 |
 | Return value	:	None.
 */

char           *
_SDtSpellGetNextWord(Widget wid)
{
	/* We may be able to simpllity this by using the text search
	 | function ... otherwise we need to check for more cases for
	 | special characters
	 */

	int             startpos;
	int             endpos = 0;
	char           *txt = SPELL_data(wid);
	static char     str[255];


	/* We've reached the end of the document.  Do we need to
	 | wrap-around, or end the spelling check?
	 */
	if (SPELL_current(wid) >= SPELL_size(wid)) {

		/* If the spelling checker was started from the very
		 * beggining of the document, or if we've already
		 * wrapped around, then terminate the spelling check
		 * by returning NULL.  Otherwise, wrap around to the
		 * beginning of the document and continue checking.
		 */
		if (SPELL_cursor(wid) == 0 || SPELL_wrap_around(wid)) {

			/* Reset wrap around for next spelling check. */
			SPELL_wrap_around(wid) = False;
			return((char *) NULL);
		} else {
			SPELL_wrap_around(wid) = True;
			SPELL_current(wid) = 0;
			SPELL_adjust(wid) = 0;
		}
	} else if (SPELL_wrap_around(wid) && 
		 (SPELL_current(wid) >= SPELL_cursor(wid)+SPELL_adjust(wid)-1)) {

		/* We wrapped around already, and ended up at the position
		 * where the spelling check was started.  End the spelling
		 * check by returning NULL.  Reset wrap around for next
		 * spelling check before returning.
		 */
		SPELL_wrap_around(wid) = False;
		return((char *) NULL);
	}


	/* First skip white space. */
	startpos = SPELL_current(wid);
	while (startpos < SPELL_size(wid) &&
	       (txt[startpos] == ' ' || txt[startpos] == '\n') ||
		txt[startpos] == '\t')
		startpos++;


	/* Now find end of the word. */
	endpos = startpos;
	while (endpos < SPELL_size(wid) && txt[endpos] != ' ' &&
	       txt[endpos] != '\n' && txt[endpos] != '\t')
		endpos++;


	/* If start and end position are the same, then we have reached
	 | the end of the datastream before reaching the eof mark.  This
	 | could mean that there was whitespace before the EOF mark.
	 | Do we need to wrap around and check from the beginning, or do
	 | we end spelling check by returning NULL?
	 */
	if (startpos == endpos) {


		/* If we've already wrapped around, or if we've started
		 * spelling check from the very beginning of the document,
		 * then return NULL since we don't need to continue checking.
		if (SPELL_wrap_around(wid) || SPELL_cursor(wid) == 0) {
			SPELL_wrap_around(wid) = False;
			return((char *) NULL);
		}


		/* Need to wrap around. */
		SPELL_wrap_around(wid) = True;
		SPELL_current(wid) = 0;
		SPELL_adjust(wid) = 0;
		startpos = SPELL_current(wid);


		/* First skip white space. */
		while (startpos < SPELL_size(wid) &&
		       (txt[startpos] == ' ' || txt[startpos] == '\n') ||
			txt[startpos] == '\t')
			startpos++;


		/* Now find end of the word. */
		endpos = startpos;
		while (endpos < SPELL_size(wid) && txt[endpos] != ' ' &&
		       txt[endpos] != '\n' && txt[endpos] != '\t')
			endpos++;
	}


	/* Set globals. */
	SPELL_current(wid) = endpos;
	SPELL_end(wid) = endpos;
	SPELL_start(wid) = startpos;


	/* Prepare the text for use by the spell checker. */
	strncpy(str, txt + startpos, endpos - startpos);
	str[endpos - startpos] = '\0';


	/* Return the word. */
	return(str);
}


/* Function	:	_SDtSpellUpdateList()
 | 
 | Objective	:	If the user types in a suggestion in the suggest
 |			text field, and the suggestion is in the suggestion
 |			list, then update the list's view port so that the
 |			user can see the suggestion, as well as select the
 |			item in the list.
 |
 | Argument	:	Widget		- spell widget
 |			char *		- 
 |
 | Return value	:	None.
 */

void
_SDtSpellUpdateList(Widget wid, char *txt)
{
	int		top;		/* Top of list. */
	int		visible;	/* Viewport size. */
	int             i;
	char          **list = SPELL_suggestions(wid);


	for (i = 0; i < SPELL_sug_count(wid); i++) {
		if (!strcmp(txt, list[i])) {

			/* Found the item in position i.  Select it. */
			XmListSelectPos(SPELL_List(wid), i + 1, False);
			SPELL_listPosition(wid) = i;


			/* Do we need to adjust the viewport, so
			 | that the user sees the item selected?
			 */
			XtVaGetValues(SPELL_List(wid),
				      XmNtopItemPosition, &top,
				      XmNvisibleItemCount, &visible,
				      NULL);
			if (i + 1 < top)
				XmListSetPos(SPELL_List(wid), i + 1);
			else if (i + 1 >= top + visible)
				XmListSetBottomPos(SPELL_List(wid), i + 1);


			return;
		}
	}


	/* If we are here, unselect position */
	if (SPELL_listPosition(wid) != -1) {
		XmListDeselectAllItems(SPELL_List(wid));
		SPELL_listPosition(wid) = -1;
	}
}


/*-----------------------------------------------------------------*/
/*  Functions used during which are used with appropriate buttons  */
/*-----------------------------------------------------------------*/


/* Function	:	_SDtSpellReplaceWord()
 |
 | Objective	:	Replace the misspelled word with the suggestion
 |			that the user has chosen or typed in.
 |
 | Arguments	:	Widget		- the spell GUI widget
 |			char *		- the replacement word
 |			Boolean		- flag if a global change is requested
 | 
 | Return value	:	None.
 |
 | Side effects	:	The user should see the changes take place in the
 |			document they are changing.
 */ 
void
_SDtSpellReplaceWord(Widget wid, char *replace, Boolean all)
{
	XmTextPosition  left, right;
	XmTextPosition  previous, current;	/* For global changes. */
	Boolean		found = False;		/* Word found globally. */
	char           *oldword = SPELL_word(wid);
	char	       *newword;		/* Copy of replace. */
	char           *selected;
	int		cursorPos;		/* Track original values     */
	int		currentPos;		/* when doing global change. */
	int		adjust;


	/* Just in case... */
	if (oldword == NULL || wid == NULL)
		return;


	/* Return if replacing word with itself. */
	if (!strcmp(oldword, replace))
		return;


	/* Replace by wiping out the old word. */
	if (replace == NULL)
		newword = NULL_STRING;
	else
		newword = strdup(replace);


	/* Reselect text - start to end - in case selection was
	 | moved elsewhere by a user action, such as selecting the
	 | suggestion in the suggestion field.
	 */
	XmTextSetSelection(SPELL_TxtWid(wid),
			   SPELL_start(wid), SPELL_end(wid),
			   XtLastTimestampProcessed(XtDisplay(wid)));
	selected = (char *) SPELL_getSelected(wid);


	if (selected == NULL || strcmp(oldword, selected)) {
		/* We have a problem - we've lost the position of
		 | the text that we're trying to replace!  The one
		 | that we highlighted is not the one that we're 
		 | expecting!  Remove the selection and reload the
		 | data.
		 */
		SPELL_clearSelection(wid);
		_SDtSpellLoadText(wid);
		XBell(XtDisplay(wid), 0);
		return;
	}


	XmTextGetSelectionPosition(SPELL_TxtWid(wid),
				   &left, &right);
	XmTextReplace(SPELL_TxtWid(wid), left, right, newword);


	/* Keep track of the difference between the number of
	 | characters of the corrected and misspelled word.
	 */
	SPELL_adjust(wid) += strlen(newword) - strlen(oldword);


	/* Did user specify global change? */
	if (all) {

		_SDtSpellSetMsg(wid, 
			catgets(_spellCat, 3, 15, "Replacing globally..."));
		_SDtSpellDispatch(wid);
		_DtTurnOnHourGlass(wid);


		/* Add to the list of globally changed words. */
		_SDtSpellGlobalChange(SPELL_engine(wid), oldword);


		/* Replace from the current point forward, only if
		 | there is still data after the right position. 
		 | Don't want to keep this process from looping on
		 | itself for special replace cases, e.g., laz replaced
		 | with lazy globally, since laz is still part of lazy,
		 | we'll end up replacing the laz in lazy with lazy
		 | forever.
		 */
		previous = (XmTextPosition) (right + strlen(newword) - 
			   strlen(oldword));
		found = XmTextFindString(SPELL_TxtWid(wid), previous,
					 oldword, XmTEXT_FORWARD, 
					 &current);
		while ((found) && 
		       (current > right + strlen(newword) - strlen(oldword))) {

			XmTextReplace(SPELL_TxtWid(wid), current,
				      current + (right - left), newword);
			_SDtSpellSetGauge(wid, strlen(oldword));
			previous = current + (right - left);
			found = XmTextFindString(SPELL_TxtWid(wid),
						 previous, oldword, 
						 XmTEXT_FORWARD,
						 &current);
		}


		/* Replace from the current point backward, only if
		 | we still have data before the left position.  See
		 | reason above for replacing current point forward.
		 */
		previous = (XmTextPosition) (left - 1);
		found = XmTextFindString(SPELL_TxtWid(wid), previous,
					 oldword, XmTEXT_BACKWARD, 
					 &current);
		while (found && current < left) {

			/* Keep track of the difference between the number of
			 | characters of the corrected and misspelled word.
			 */
			SPELL_adjust(wid) += strlen(newword) - strlen(oldword);


			XmTextReplace(SPELL_TxtWid(wid), current,
				      current + (right - left), newword);
			_SDtSpellSetGauge(wid, strlen(oldword));
			previous = current - 1;
			found = XmTextFindString(SPELL_TxtWid(wid),
						 previous, oldword, 
						 XmTEXT_BACKWARD,
						 &current);
		}


		/* Reload the text again due to the global change.
		 | Need to do this, in order to keep the buffer up to
		 | date, so that we know what we are correcting.  Also,
		 | since the _SDtSpellLoadText() function reseets the
		 | cursor and current position, we want to keep track of 
		 | the old values, and then recalculate the new positions
		 | based on the old values.
		 */
		cursorPos = SPELL_cursor(wid);
		currentPos = SPELL_current(wid);
		adjust = SPELL_adjust(wid);
		_SDtSpellLoadText(wid);
		SPELL_cursor(wid) = cursorPos + adjust;
		SPELL_current(wid) = currentPos + adjust;
		_DtTurnOffHourGlass(wid);
	}


	/* Free memory. */
	if (strcmp(newword, NULL_STRING) != 0)
		free(newword);
	if (SPELL_word(wid) != NULL) {
		free(SPELL_word(wid));
		SPELL_word(wid) = NULL;
	}
}


/* Function	:	_SDtSpellSkipWord()
 | 
 | Objective	:	Do not correct the flagged word and proceed on with
 |			the next word to verify for spelling.
 |
 | Arguments	:	Widget		- spell widget
 |			Boolean		- global skip flag; if True, the
 |					  spell checker will skip the word
 |					  if it is encountered again.
 | Return value	:	None.
 */

void
_SDtSpellSkipWord(Widget wid, Boolean all)
{
	/* In case we don't have the spell engine, exit out of function.
	 | There should be more going on here, since this is an error
	 | condition.
	 */
	if (SPELL_engine(wid) == NULL)
		return;


	/* Add the word to the list of words to ignore globally. */
	if (all) {
		_SDtSpellIgnoreWord(SPELL_engine(wid), SPELL_word(wid));
	}


	/* Reset GUI */
	SPELL_clearSelection(wid);
	if (SPELL_word(wid) != NULL) {
		free(SPELL_word(wid));
		SPELL_word(wid) = NULL;
	}
}


/* Function	:	_SDtSpellLearnWord()
 |
 | Objective	:	Flag a particular word as correct so that in
 |			the future, the spell checker won't report
 |			the word as misspelled.  This is accomplished
 |			by adding the word to the user's personal
 |			dictionary.
 |
 | Arguments	:	Widget		- the spell checker widget
 |			char *		- word to be learned as correct
 |
 | Return value	:	None.
 |
 | Side effects	:	The user's personal dictionary will be updated.
 |			The update will be saved once the user terminates
 |			the spell process, i.e., dismiss the spell window.
 */

void
_SDtSpellLearnWord(Widget wid, char *word)
{
	if (SPELL_engine(wid) == NULL)
		return;


	/* Add the word to the personal dictionary.  If it fails, 
	 | pop-up an error dialog and inform the user that the
	 | addition failed, and inactivate the learn button.
	 */
	if (!_SDtSpellAddWord(SPELL_engine(wid), SPELL_word(wid))) {
		char *errmsg;
                errmsg = (char *) calloc(
                                strlen(catgets(_spellCat, 1, 16,
                                        "Spelling Checker - ")) +
                                strlen(catgets(_spellCat, 2, 30,
                                        "Failed to add word: ")) +
                                strlen(SPELL_word(wid)) + 5,
                                sizeof(char));
		sprintf(errmsg, "%s%s\n%s",
                                catgets(_spellCat, 1, 16,"Spelling Checker - "),
                                catgets(_spellCat, 2, 30,
                                        "Failed to add word: ") ,
				SPELL_word(wid));

		_SDtSpellError(wid, errmsg);
		free(errmsg);
		XtSetSensitive(SPELL_learn(wid), False);
	}


	/* Move the data pointer position so that we can continue
	 | checking the next word (if any).  Remove the highlight
	 | from the word in the text source, and free memory occupied
	 | by the copy of the supposedly misspelled word.
	 */
	SPELL_current(wid) = SPELL_end(wid);
	SPELL_clearSelection(wid);
	if (SPELL_word(wid) != NULL) {
		free(SPELL_word(wid));
		SPELL_word(wid) = NULL;
	}
}


/*-----------------------------------------------------------------*/
/*			DEFAULT ACTION FUNCTIONS		   */
/*-----------------------------------------------------------------*/

/* Function	:	_SDtSpellDefaultAction()
 |
 | Objective	:	Replace the misspelled word with the suggestion
 |			after the user has either double clicked on the
 |			suggestion in the list, or entered carriage
 |			return in the suggestion text field.
 |
 | Arguments	:	Widget wid		- suggestion list widget
 |			XtPointer client	- client data
 |			XtPointer call		- call data
 |
 | Return value	:	None.
 */
void _SDtSpellDefaultAction(Widget wid, XtPointer client, XtPointer call)
{
	Widget	 spell;
	char	*txt;


	if (XmIsList(wid))
		spell = XtParent(XtParent(XtParent(wid)));
	else
		spell = XtParent(XtParent(wid));


	/* Get the text in the suggestion field and use it to replace
	 | the misspelled word in the user's document.
	 */
	txt = XmTextGetString(SPELL_Text(spell));
	_SDtSpellReplaceWord((Widget) spell, txt, False);
	XtFree(txt);


	/* Restart the spelling checker process. */
	XmTextSetString(SPELL_Text(spell), NULL_STRING);
	_SDtSpellClearSearch((Widget) spell);
	_SDtSpellSetMsg(spell, catgets(_spellCat, 3, 1, "Checking..."));
	_SDtSpellSetSensitive(spell, False);
	_SDtSpellSetOkLabel(spell, catgets(_spellCat, 1, 13, "Stop Check"));
	SPELL_workID(spell) =
		XtAppAddWorkProc(SPELL_appcontext(spell),
				 (XtWorkProc) _SDtSpellCheck,
				 (XtPointer) spell);
}


/*-----------------------------------------------------------------*/
/*			PUBLIC FUNCTIONS			   */
/*-----------------------------------------------------------------*/


/* Function	:	SDtCreateSpellSelectionBox()
 |
 | Objective	:	Create an instance of the spell widget that
 |			can be used in a Motif program.
 |
 | Arguments	:	Widget		- parent of the spell widget
 |			String		- name to give to the spell widget
 |			ArgList		- Motif arguments to give to the
 |					  widget during creation.
 |			Cardinal	- number of Motif arguments
 |
 | Return value	:	Widget, the spell widget.
 */

Widget
SDtCreateSpellSelectionBox(Widget p, String name, ArgList args, Cardinal n)
{
	Widget	spellw;

	/* Open the catalog file. */
	_SDtSpellInitEnv();

	/* Create the widget using the standard XtCreate function. */
	spellw = XtCreateWidget(name, sdtSpellSelectionBoxWidgetClass, p,
			        args, n);
	if (spellw != (Widget) NULL) 
		DtAppInitialize(SPELL_appcontext(spellw), XtDisplay(spellw),
				spellw, "SDtSpell");
	return(spellw);
				
}


/* Function	:	SDtCreateSpellSelectionDialog()
 |
 | Objective	:	Create a spell widget dialog that can be used in
 |			a Motif program.
 |
 | Arguments	:	Widget		- parent of the spell widget
 |			String		- name to the spell widget
 |			ArgList		- Motif arguments to give to the
 |					  widget during creation.
 |			Cardinal	- number of Motif arguments
 |
 | Return value	:	Widget, the spell widget.
 */

Widget
SDtCreateSpellSelectionDialog(Widget ds_p, String name, ArgList spell_args,
			      Cardinal spell_n)
{
	Widget          spell;	   /* New spell widget */
	Widget          ds;	   /* Dialog shell for the spell widget  */
	ArgList         ds_args;   /* Arglist for shell  */
	char           *ds_name;   /* Name of the dialog shell. */


	/* Open the catalog file. */
	_SDtSpellInitEnv();


	/* Create Dialog shell parent of the spell widget. */
	ds_name = XtMalloc((strlen(name) + XmDIALOG_SUFFIX_SIZE + 1) *
			   sizeof(char));
	strcpy(ds_name, name);
	strcat(ds_name, XmDIALOG_SUFFIX);
	ds_args = (ArgList) XtMalloc(sizeof(Arg) * (spell_n + 1));
	memcpy(ds_args, spell_args, (sizeof(Arg) * spell_n));
	XtSetArg(ds_args[spell_n], XmNallowShellResize, True);
	ds = XmCreateDialogShell(ds_p, ds_name, ds_args, spell_n + 1);
	XtFree((char *) ds_args);
	XtFree(ds_name);


	/* Create spell widget that will be inside the dialog that was
	 | just created.
	 */
	spell = XtCreateWidget(name, sdtSpellSelectionBoxWidgetClass, ds,
			       spell_args, spell_n);
	if (spell != (Widget) NULL) {
		XtAddCallback(spell, XmNdestroyCallback,
			      _XmDestroyParentCallback, NULL);
		if (spell != (Widget) NULL) 
			DtAppInitialize(SPELL_appcontext(spell), 
					XtDisplay(spell),
					spell, "SDtSpell");
	}
	return (spell);
}


/* Function	:	SDtInvokeSpell()
 |
 | Objective	:	Convenience function that creates a spell dialog 
 |			that assumes certain defaults.
 |
 | Arguments	:	Widget		- the text source that will be
 |					  checked
 |
 | Return value	:	Boolean True if the solaris spell checker and
 |			engine can be run, false otherwise.
 */

Boolean
SDtInvokeSpell(Widget textW)
{
	Cursor		hg;		 /* Hour glass cursor. */
	Widget          current, up;	 /* Determine parent of text source. */
	static Widget   spellDlg = NULL; /* Spell widget. */
	Arg             arg_list[20];	 /* Argument list. */
	int             arg_num = 0;
	String		title;		 /* Window title of spell widget. */
	String		dict;		 /* Dictionary language string. */
	SDtSpellResR	dflt_res;        /* Resources from .desksetdefaults. */
        char           *locale;


	/* Initialize engine library function routines. */
	if (!_initEngineFuncs())
		return(False);


	/* Get the defaults from the spell defaults file. */
	_SDtSpellGetDefaults(&dflt_res);


	/* Get the hour glass cursor, and display it. */
	_DtGetHourGlassCursor(XtDisplay(textW));
	_DtTurnOnHourGlass(textW);


	/* Determine the parent of the text source widget by going up the
	 | application's widget hierarchy.
	 */
	current = textW;
	up = XtParent(current);
	while (up != NULL) {
		current = up;
		up = XtParent(current);
	}


	/* Create the title string of the spell dialog. */
	_SDtSpellInitEnv();
	title = XtNewString(catgets(_spellCat, 1, 1, "Spelling Checker"));


	/* What dictionary will the spell checker use?  The dictionary
	 | will be based on the following logic:
	 | 	If followLocale is False and defaultDictionary is defined,
	 |		then use defaultDictionary value
	 |	else if locale is defined
	 |		then use dictionary for locale
	 |	else
	 |		default to US English.
	 */
	locale = getenv("LANG");
	if (!dflt_res.followLocale && dflt_res.defaultDictionary)
		dict = XtNewString(dflt_res.defaultDictionary);
	else if (locale)
		dict = XtNewString(locale);
	else
		dict = XtNewString(LOCALE_EN_US);


	/* Set the arguments to the spelling checker widget. */
	XtSetArg(arg_list[arg_num], XmNsearchWidget, textW);
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNtitle, title); arg_num++;
	XtSetArg(arg_list[arg_num], XmNdictionary, dict); arg_num++;
	XtSetArg(arg_list[arg_num], XmNusePersonal, 
		 dflt_res.usePersonal);
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNfollowLocale, 
		 dflt_res.followLocale);
	arg_num++;
	XtSetArg(arg_list[arg_num], XmNdefaultDictionary, 
		 dflt_res.defaultDictionary);
	arg_num++;


	/* Create the spell dialog, if it hasn't been created yet. */
	if (spellDlg == NULL) {

		/* Create a spell dialog. */
		spellDlg = SDtCreateSpellSelectionDialog(current, "sdtSpell",
							 arg_list, arg_num);
		if (spellDlg == (Widget) NULL)
			return (False);


		/* Check if spell engine is available. */
                 if (!SDtSpellIsEngineAvail(spellDlg)) {
                 	XtDestroyWidget(spellDlg);
                 	spellDlg = NULL;
                 	return (False);
                 }
	} else {

		/* The spell engine's components - mainDict and userDict,
		 | may be null if user closed the spell widget.  Need to
		 | check if so, and reinitialize if necessary.
		 */
		if (!(_SDtSpellReInit(SPELL_engine(spellDlg), 
				      &SPELL_dictionary(spellDlg)))) {
			XtDestroyWidget(spellDlg);
			spellDlg = NULL;
			_DtTurnOffHourGlass(textW);
			return(False);
		}

		/* Spell dialog has been created.  Reset the
		 | widget so that it makes use of the new text
		 | widget source, as well as the other values
		 | found in the .desksetdefaults file (in case
		 | the file was updated by some other process or
		 | method).
		 */
		XtSetValues(spellDlg, arg_list, arg_num);
	}


	/* Save any changes to the $HOME/.desksetdefaults file. */
	SPELL_use_sysdefaults(spellDlg) = True;


	/* Display the spell dialog, and then start spelling,
	 | before returning true, if and only if data is present.
	 */
	XtManageChild(spellDlg);
	_DtTurnOffHourGlass(textW);
	if (_SDtSpellLoadText(spellDlg)) {

		/* Start the spelling checker work proc. */
		_SDtSpellSetMsg(spellDlg, catgets(_spellCat, 3, 1, 
						  "Checking..."));
		_SDtSpellSetSensitive(spellDlg, False);
		_SDtSpellSetOkLabel(spellDlg, catgets(_spellCat, 1, 13, 
						      "Stop Check"));
		SPELL_workID(spellDlg) =
			XtAppAddWorkProc(SPELL_appcontext(spellDlg),
					 (XtWorkProc) _SDtSpellCheck,
					 (XtPointer) spellDlg);
	} else {
		_SDtSpellSetMsg(spellDlg, catgets(_spellCat, 3, 11, 
				"No text to check for spelling."));
	}


	/* Free space occupied by variables. */
	XtFree(title);
	if (dflt_res.defaultDictionary)
		free(dflt_res.defaultDictionary);
	return (True);
}


	
/* Function	:	SDtSpellIsEngineAvail()
 |
 | Objective	:	Determine if the Solaris Spell engine is present
 |			in the spell widget that was created.
 |
 | Arguments	:	Widget		- the spell widget
 | 
 | Return Value	:	Boolean true if the solaris spell engine is available,
 |			false otherwise.
 */

Boolean
SDtSpellIsEngineAvail(Widget wid)
{
	if (wid == NULL || !SDtIsSpellSelectionBox(wid))
		return(False);

	return(SPELL_engine(wid) != NULL);
}
	
