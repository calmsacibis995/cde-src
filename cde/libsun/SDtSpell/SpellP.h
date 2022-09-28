/* 
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */ 

#pragma ident "@(#)SpellP.h	1.19 96/07/26 SMI"


#ifndef _SDtSpellP_h
#define _SDtSpellP_h


#include <Xm/BulletinBP.h>
#include <SDt/Spell.h>
#include <spelltypes.h>

#ifdef __cplusplus
extern "C" {
#endif


#define XmSPELL_MAX_WIDGETS_VERT 25 


/* Constraint part record for SpellSelectBox widget */
typedef struct _SDtSpellSelectBoxConstraintPart
{
	char		unused;
} SDtSpellSelectBoxConstraintPart, *SDtSpellSelectBoxConstraint;



/*  New fields for the SpellSelectBox widget class record  */
typedef struct
{
	XtPointer	extension;      /* Pointer to extension record */
} SDtSpellSelectBoxClassPart;


/* Full class record declaration */
typedef struct _SDtSpellSelectionBoxClassRec
{
	CoreClassPart			core_class;
	CompositeClassPart		composite_class;
	ConstraintClassPart		constraint_class;
	XmManagerClassPart		manager_class;
	XmBulletinBoardClassPart	bulletin_board_class;
	SDtSpellSelectBoxClassPart	spell_select_box_class;
} SDtSpellSelectionBoxClassRec;


externalref SDtSpellSelectionBoxClassRec sdtSpellSelectionBoxClassRec;


/* New fields for the SpellSelectBox widget record */
typedef struct
{
    char           *dictionary;        	/* dictionary currently in use */
    Widget          spellTarget;      	/* widget to search */
    Boolean         usePersonal;      	/* use a personal dictionary also */
    Boolean         followLocale;	/* follow locale or default setting? */
    char	   *defaultDictionary;	/* Default dictionary. */
    Boolean	    is_textWidget;    	/* is source a XmText or DtEditor? */
    Widget 	    txtwidget;		/* pointer to text field in text or
					   editor widget */
    int             workID;		/* Work proc id. */
    XtAppContext    appcontext;		/* Application context */
    void	   *spellEngine;	/* pointer to spell informations str */

    char	   *textpointer;	/* pointer to the spot in memory which
					   contains the spell text */

    int		    currentListItem;	/* currently selected item in the list
					   or -1 if no items are selected */

    char	   *misspelledWord;	/*pointer to misspelled wd */
    char	  **suggestions;	/*pointer to malternatives */
    int		    sugcnt;		/*pointer to cnt of alternatives*/
    Boolean	    done;
    int		    totalSize;	     	/* size of the text in characters */
    int 	    currentPos;		/* current position of text pointer */
    int             cursorPos;		/* position of insert cursor */
    int 	    selectLeft;		/* left side of selection */
    int		    selectRight; 	/* right side of selection */
    int		    adjust;		/* adjustment between internal
					   and external position */
    WordListP       deleteWords;	/* list of words currently set to be
					   deleted */
    WordListP       addWords;           /* list of words currently set to be
                                           added */
    SPL_LANGUAGE    dict_used;          /* Dictionary in current use. */
    SPL_LANGUAGE    dict_to_use;        /* Dictionary to change to. */
    Boolean         wrap_around;	/* Wrap around text. */
    Boolean         use_sysdefaults;	/* Need to know if we're going to
					   save preferences system-wide. */


    /* Widgets for the main spell GUI. */
    Widget	    startBtn;
    Widget	    close;
    Widget	    help;
    Widget	    separator;
    Widget	    text;
    Widget	    list;
    Widget	    textLabel;

    Widget	    form;
    Widget	    misspelledLabel;
    Widget          misspelledWid; 	/* label with current word */
    Widget 	    update;
    Widget 	    update_all;
    Widget 	    skip;
    Widget 	    skip_all;
    Widget 	    learn;
    Widget	    message_bar;
    Widget	    search_scale;
    Widget	    options;
    Widget	    errdiag;


    /* Widgets related to the options panel */
    Widget	    option_panel;
    Widget          primary_form;       /* Primary dictionary options. */
    Widget	    current_menu;       /* Options menu for current dict used */
    LangMenuP       current_items;      /* Menu items for current menu. */
    Widget          default_menu;       /* Options menu for default dict. */
    LangMenuP       default_items;	/* Menu items for default menu. */
    Widget          locale_toggle;	/* Follow locale toggle. */
    Widget          default_toggle;	/* Follow default toggle. */
    Widget          personal_form;      /* Personal dictionary options. */
    Widget	    pd_list;            /* Contents of personal dictionary */
    Widget	    personal_toggle;

} SDtSpellSelectBoxPart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _SDtSpellSelectionBoxRec
{
    CorePart			core;
    CompositePart		composite;
    ConstraintPart		constraint;
    XmManagerPart		manager;
    XmBulletinBoardPart		bulletin_board;
    SDtSpellSelectBoxPart	spell_select_box;
} _SDtSpellSelectionBoxRec;


/* Access macros */

#define SPELL_dictionary(w) \
                (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.dictionary)

#define SPELL_personal(w) \
             (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.usePersonal)

#define SPELL_follow_locale(w) \
             (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.followLocale)

#define SPELL_default_dictionary(w) \
             (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.defaultDictionary)

#define SPELL_target(w) \
		(((SDtSpellSelectionBoxWidget)(w))->spell_select_box.spellTarget)

#define SPELL_isText(w) \
              (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.is_textWidget)

#define SPELL_TxtWid(w) \
               (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.txtwidget)

#define SPELL_workID(w) \
               (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.workID)

#define SPELL_appcontext(w) \
               (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.appcontext)

#define SPELL_engine(w) \
             (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.spellEngine)

#define SPELL_data(w) \
            (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.textpointer)

#define SPELL_listPosition(w) \
            (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.currentListItem)

#define SPELL_word(w) \
             (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.misspelledWord)

#define SPELL_suggestions(w) \
             (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.suggestions)

#define SPELL_sug_count(w) \
             (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.sugcnt)

#define SPELL_done(w) \
             (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.done)

#define SPELL_size(w) \
            (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.totalSize)

#define SPELL_current(w) \
             (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.currentPos)

#define SPELL_cursor(w) \
             (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.cursorPos)

#define SPELL_start(w) \
             (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.selectLeft)

#define SPELL_end(w) \
             (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.selectRight)

#define SPELL_adjust(w) \
             (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.adjust)

#define SPELL_deleteWords(w) \
               (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.deleteWords)

#define SPELL_addWords(w) \
               (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.addWords)

#define SPELL_dict_used(w) \
               (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.dict_used)

#define SPELL_dict_to_use(w) \
               (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.dict_to_use)

#define SPELL_wrap_around(w) \
               (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.wrap_around)

#define SPELL_use_sysdefaults(w) \
		(((SDtSpellSelectionBoxWidget)(w))->spell_select_box.use_sysdefaults)

#define SPELL_startButton(w) \
                (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.startBtn)

#define SPELL_CloseButton(w) \
                (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.close)

#define SPELL_HelpButton(w) \
                (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.help)

#define SPELL_Separator(w) \
                (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.separator)

#define SPELL_Text(w) \
                (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.text)

#define SPELL_List(w) \
                (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.list)

#define SPELL_Textlabel(w) \
                (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.textLabel)

#define SPELL_form(w) \
                (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.form)

#define SPELL_misspelledLabel(w) \
                (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.misspelledLabel)

#define SPELL_misspelledWid(w) \
                (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.misspelledWid)

#define SPELL_update(w) \
                (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.update)

#define SPELL_update_all(w) \
                (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.update_all)

#define SPELL_skip(w) \
                (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.skip)

#define SPELL_skip_all(w) \
                (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.skip_all)

#define SPELL_learn(w) \
                (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.learn)

#define SPELL_message_bar(w) \
                (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.message_bar)

#define SPELL_search_scale(w) \
                (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.search_scale)

#define SPELL_options(w) \
                (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.options)

#define SPELL_error(w) \
               (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.errdiag)

#define SPELL_option_panel(w) \
               (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.option_panel)

#define SPELL_primary_dict_option(w) \
               (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.primary_form)

#define SPELL_current_menu(w) \
               (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.current_menu)

#define SPELL_current_items(w) \
               (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.current_items)

#define SPELL_default_menu(w) \
               (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.default_menu)

#define SPELL_default_items(w) \
               (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.default_items)

#define SPELL_locale_toggle(w) \
               (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.locale_toggle)

#define SPELL_default_toggle(w) \
               (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.default_toggle)

#define SPELL_personal_dict_option(w) \
               (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.personal_form)

#define SPELL_pdList(w) \
               (((SDtSpellSelectionBoxWidget)(w))->spell_select_box.pd_list)

#define SPELL_personal_toggle(w) \
		(((SDtSpellSelectionBoxWidget)(w))->spell_select_box.personal_toggle)


/* Text Macros */

#define SPELL_setHighlight(w, s, e)   { \
		XmTextSetSelection(SPELL_TxtWid(w), s, e, \
			XtLastTimestampProcessed(XtDisplay(w))); \
		SPELL_start(w) = s; \
		SPELL_end(w) = e; }


#define SPELL_getInsert(w) \
	XmTextGetInsertionPosition(SPELL_TxtWid(w))


#define SPELL_setEditState(w, state) \
	XtSetSensitive(w, state)


#define SPELL_getSelected(w) \
	 XmTextGetSelection(SPELL_TxtWid(w))


#define SPELL_clearSelection(w)  {\
		XmTextClearSelection(SPELL_TxtWid(w), \
			XtLastTimestampProcessed(XtDisplay(w))); \
		SPELL_start(w) = -1; \
		SPELL_end(w) = -1; \
}


/********    Private Function Declarations    ********/

Boolean      _SDtSpellCreateOptionsDlg(SDtSpellSelectionBoxWidget);
Boolean      _SDtSpellManageOptions(SDtSpellSelectionBoxWidget);
char        *_SDtSpellListPersonal(Widget, char *);

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif


#endif /* _SDtSpellP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
