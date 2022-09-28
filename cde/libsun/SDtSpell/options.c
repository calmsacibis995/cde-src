/* 
 |  (c) Copyright 1993, 1994 Sun Microsystems, Inc. 
 */

#pragma ident "@(#)options.c	1.11 96/06/19 SMI"


/*------------------------------ INCLUDE FILES -------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/param.h>
#include <nl_types.h>
#include <errno.h>

#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/Separator.h>

#include <SDt/Spell.h>
#include <SDt/SpellP.h>

#include "spelltypes.h"
#include "spellLib.h"
#include "engine.h"
#include "globals.h"
#include "list.h"


/*------------------------------ DEFINITIONS  --------------------------------*/

#define SEPARATOR "separator"


/*--------------------------- FUNCTION DEFINITIONS ---------------------------*/

/* Function	:	_SDtSpellAddDictMenuItem()
 |
 | Objective    :       Create a menu item or a separator widget
 |                      for the specified menu item, i.e., dictionary
 |			name.
 |
 | Argument     :       char   *menuItem        - the label of the menu
 |                                                item; if NULL, then
 |						  the function creates
 |						  a separator
 |                        Widget menu           - the menu where the
 |                                                item will be added
 |
 | Return value :       Widget of the menu item containing the
 |		 	specified dictionary label or separator.
 */

Widget
_SDtSpellAddDictMenuItem(char *menuItem, Widget menu)
{
	Widget	  item;		/* Widget returned to user. */
	XmString  label;	/* Menu label of item to add. */


	if (menuItem == (char *) NULL)
	{
		/* Create a separator. */
		item = XtVaCreateManagedWidget(SEPARATOR,
	                                       xmSeparatorWidgetClass, menu,
	                                       NULL);
	}
	else
	{
		/* Create the label and menu item. */
		label = XmStringCreateLocalized(menuItem);
		item = XtVaCreateManagedWidget(menuItem,
	                                       xmPushButtonWidgetClass, menu,
	                                       XmNlabelString, label,
	                                       NULL);

		/* Free memory. */
		XmStringFree(label);
	}


	return(item);
}


/****************************************************************************
 ****************************************************************************

	Functions Related to the language Menu

 ****************************************************************************
 ****************************************************************************/

/* Function	:	_freeMenuList()
 |
 | Objective	:	Free the menu list recursively.
 |
 | Arguments	:	LangMenuP	- menu list containing the available
 |					  dictionaries
 | Return Value	:	None.
 */

void
_freeMenuList(LangMenuP menu)
{
	if (menu -> next != NULL)
		_freeMenuList(menu -> next);

	free(menu);
	menu = (LangMenuP) NULL;
}


/* Function _SDtSpellCreateMenuList()
 |
 | Objective    :       To recursively add an item to the menu list.
 |
 | Arguments    :       LangMenuP       - menu list where item will be
 |					  added
 |                      Widget          - menu that will contain the
 |					  menu item to be added
 |                      SPL_LANGUAGE    - language that menu item
 |					  refers to
 |			char *		- the item to be added
 |
 | Return value	:	None.
 |
 | Side Effects	:	None.
 */

void 
_SDtSpellCreateMenuList(LangMenuP *mList, Widget menu, 
			SPL_LANGUAGE lang, char *item)
{
	/* Recursively build the menu list. */
	if (*mList != (LangMenuP) NULL)
		_SDtSpellCreateMenuList(&((*mList)->next), menu, lang, item);
	else
	{
		/* Create the new menu item. */
		(*mList) = (LangMenuP) calloc(1, sizeof(LangMenuR));
		(*mList)->langItem = _SDtSpellAddDictMenuItem(item, menu);
		(*mList)->lang = lang;
		(*mList)->next = (LangMenuP) NULL;
	}
}


/* Function	:	_SDtSpellGetAvailDicts()
 |
 | Objective    :	Find out what dictionaries are available on the 
 |			system, and generate a list that can be used
 |			for dictionary options menu.
 | 
 | Arguments    :       ICBUFF *	- the spell engine's icbuff member,
 |					  to get location of dictionaries
 |			SDtSpellOptWinP	- handle to all options window widgets
 |
 | Return Value :       Boolean True, if the query to the system was
 |			succesful, False otherwise.
 |
 | Side Effects	:	None.
 */

LangMenuP 
_SDtSpellGetAvailDicts(SpellEngP spellEngine, Widget menu)
{
	ICBUFF         *icBuff = spellEngine->mainDict->icBuff;


	/* Word list of labels for the menu item, and one to
	 | traverse the sorted word list.
	 */
	WordListP       menuLabels = (WordListP) NULL;
	WordListP       p = (WordListP) NULL;
	LangMenuP       langMenu = (LangMenuP) NULL;


	/* Directory pointer where the dicationaries are located, and
	 | the entries (files, links, subdirs, etc.) in that directory.
	 */
	DIR            *dir;
	struct dirent  *dir_entry;


	/* Keep track of the language and dialect. */
	SPL_LANGUAGE    language;


	/* Open the directory, and read all the entries. */
	dir = opendir((char *) icBuff->mstr_path.path);
	while ((dir_entry = readdir(dir)) != NULL) {

		/* Look for specific *.dat files, and create the menu label
		 | word list.  This list will be used later to construct
		 | the menu items list.
		 */
		if (strcmp(dir_entry->d_name, "C.dat") == 0) {

			/* C.dat implies Australian, UK and US english
			 | dictionaries are installed, since these are part
			 | of the base release.
			 */
			_addListSorted(&menuLabels, catgets(_spellCat, 2, 15,
							    "English (US)"));
			_addListSorted(&menuLabels, catgets(_spellCat, 2, 14,
							    "English (UK)"));
			_addListSorted(&menuLabels, catgets(_spellCat, 2, 13,
						   "English (Australian)"));

		} else if (strcmp(dir_entry->d_name, "fr.dat") == 0)

			/* One French dictionary used, whether locale is
			 | Canada, France, Switzerland, etc.
			 */
			_addListSorted(&menuLabels, catgets(_spellCat, 2, 16,
							    "French"));

		else if (strcmp(dir_entry->d_name, "de.dat") == 0) {

			/* Two types of German are supported -
			 | Germany/Austria which uses the Scharfes S, and
			 | Swiss which uses Doppel S.
			 */
			_addListSorted(&menuLabels, catgets(_spellCat, 2, 18,
					       "German (Germany/Austria)"));
			_addListSorted(&menuLabels, catgets(_spellCat, 3, 3,
						   "German (Switzerland)"));

		} else if (strcmp(dir_entry->d_name, "it.dat") == 0)

			_addListSorted(&menuLabels, catgets(_spellCat, 2, 20,
							    "Italian"));

		else if (strcmp(dir_entry->d_name, "es.dat") == 0)

			_addListSorted(&menuLabels, catgets(_spellCat, 2, 21,
							    "Spanish"));

		else if (strcmp(dir_entry->d_name, "sv.dat") == 0)

			_addListSorted(&menuLabels, catgets(_spellCat, 2, 22,
							    "Swedish"));
	}


	/* Construct the sorted-order menu list. */
	for (p = menuLabels; p != (WordListP) NULL; p = p->next) {

		if (strcmp(p->word,
		    catgets(_spellCat, 2, 15, "English (US)")) == 0)
				language = EN_US;
		else if (strcmp(p->word,
			 catgets(_spellCat, 2, 14, "English (UK)")) == 0)
				language = EN_UK;
		else if (strcmp(p->word,
			 catgets(_spellCat, 2, 13, "English (Australian)")) == 0)
				language = EN_AU;
		else if (strcmp(p->word,
			 catgets(_spellCat, 2, 16, "French")) == 0)
				language = FR;
		else if (strcmp(p->word,
			 catgets(_spellCat, 2, 18, "German (Germany/Austria)")) == 0)
				language = DE;
		else if (strcmp(p->word,
			 catgets(_spellCat, 2, 19, "German (Switzerland)")) == 0)
				language = DE_CH;
		else if (strcmp(p->word,
			 catgets(_spellCat, 2, 20, "Italian")) == 0)
				language = IT;
		else if (strcmp(p->word,
			 catgets(_spellCat, 2, 21, "Spanish")) == 0)
				language = ES;
		else if (strcmp(p->word,
			 catgets(_spellCat, 2, 22, "Swedish")) == 0)
				language = SV;

		_SDtSpellCreateMenuList(&langMenu, menu, language, p->word);
	}


	/* Free the space occupied by menu labels word list. */
	_freeList(menuLabels);


	/* Close the directory. */
	closedir(dir);

	return(langMenu);
}



/* Function	: _SDtSpellGetPDContents()
 | 
 | Objective	: Partially get the contents the personal dictionary.
 |		  In order to retrieve the entire personal dictionary,
 |		  call this function in a loop until the list_num
 |		  returned is 0.
 |
 | Argument	: SpellEngP		- spell engine
 |		  Boolean *		- flag to see if retrieval was
 |					  successful
 |		  int *			- flag to see if conversion was
 |					  successful
 |		  int *			- number of entries retrieved
 | Return Value : Unsigned char ** list of entries in the personal
 |		  dictionary.
 |
 | Notes	: The list must be freed explicitly by the calling function.
 */

unsigned char **
_SDtSpellGetPDContents(SpellEngP engine, Boolean *getOK, int *result, 
		       int *list_num)
{
	unsigned char   pdEntry[2 * ICMAX + 1];    /* Entry in dictionary. */
	unsigned char **entries;                   /* List of entries. */
	int		convRes;
	int             i;


	*getOK = _getPersonalDictEntry(&engine);
	if (*getOK) {

		*list_num = engine->userDict->pdBuff->byNumRets;
		if (*list_num > 0) {

			/* Allocate space for the list of words. */
			entries = (unsigned char **) calloc(*list_num, 
						sizeof(unsigned char *));
			for (i = 0; i < *list_num; i++) {

				strcpy((char *) pdEntry, (char *) 
				       &(engine->userDict->pdBuff->
				       pReturns[i]->szRetWord));

				/* Convert the word from the spell
				 | engine format to a format the
				 | platform can understand and
				 | display, and put it in the list
				 */
				convRes = _convertWord(pdEntry, DEF_TO_NAT,
						       engine->mainDict->icBuff,
						       &(entries[i]));
				if (convRes != OKRET)
					*result = convRes;
			}
		}
		return(entries);
	} else
		return((unsigned char **) NULL);
}



/* Function	: _SDtSpellListPersonal()
 | 
 | Objective    : List out the contents of the personal dictionary
 |                either to a file, or to the scrolling list in the
 |		  Options window.  The contents are saved to a file
 |		  if the second argument, file, is not NULL.
 |
 | Arguments	: Widget	- widget that invoked this function;
 |                                either the Options window when
 |				  displaying contents of the personal
 |				  dictionary, or the file selection
 |				  box when exporting the personal
 |				  dictionary.
 |		  char *	- filename, if the widget argument is
 |				  a file selection box widget.
 |
 | Return value	: char *	- error message if listing fails
 */

char *
_SDtSpellListPersonal(Widget wid, char *file)
{
	SpellEngP       spellEngine;          /* Spell engine. */
	Widget          errWid;		     /* ID of error dialog widget. */
	Boolean         done;		     /* Was error dialog dismissed? */
	Boolean         finished = False;    /* Loop termination. */
	char           *failMsg;	     /* Buffer for error message. */
	int             conv_result;	     /* Conversion result. */
	int             i;


	/* Variables used in retrieving contents of the personal
	 | dictionary.
	 */
	unsigned char  **pdEntries = (unsigned char **) NULL;
	int              pdEntriesNum;
	Boolean		 getOK;


	/* Variables used when saving to a file. */
	FILE		*fd;
	char		 word[2 * ICMAX + 1];


	/* Variable used when displaying to the scrolling list. */
	XmString         entry;


	/* Get the spell engine.  If the usePersonal flag is set to
	 | False, then we don't have a personal dictionary buffer.
	 | Temporarily create one so that we can list out the contents
	 | of the spell engine.
	 */
	spellEngine = (SpellEngP) SPELL_engine(wid);
	if (!SPELL_personal(wid)) {
		spellEngine->userDict = 
			_SDtSpellCreateUserDict(spellEngine->mainDict,
						SPELL_dictionary(wid));
	}


	/* If file is NULL, then we are displaying to a list widget.
	 | Otherwise, we are listing the contents out to a file.
	 */
	if (file == (char *) NULL) {
		/* Delete the contents of the list first. */
		XmListDeleteAllItems(SPELL_pdList(wid));
	} else {
		/* Open the file for writing only. */
		if ((fd = fopen(file, "w")) == (FILE *) NULL) {
			/* Return an error message. */
			return("FAIL");
		}
	}


	/* Before we can get the contents of the personal dictionary,
	 | we must lock the personal dictionary first.
	 */
	if (!(_lockPersonalDict(&spellEngine, True)))
	{
		/* Display an error message about obtaining the lock. */
		failMsg = (char *) calloc(strlen(catgets(_spellCat, 2, 28,
			"Failed to display dictionary - ")) + 
			strlen(catgets(_spellCat, 2, 25, 
			"could not obtain lock.")) +5, sizeof(char));
		sprintf(failMsg, "%s%s", catgets(_spellCat, 2, 28,
			"Failed to display dictionary - "), catgets(_spellCat,
			2, 25, "could not obtain lock."));
		if (file != (char *) NULL)
			fclose(fd);
		return(failMsg);
	}


	/* Retrieve the contents of the personal dictionary. */
		conv_result = OKRET;
	while (!finished) {
		pdEntries = _SDtSpellGetPDContents(spellEngine, &getOK,
						   &conv_result,
						   &pdEntriesNum);

		if (!getOK) {
			/* DISPLAY ERROR MESSAGE - Retrieval. */
			finished = True;
		} else {
			if (conv_result != OKRET) {
				/* DISPLAY ERROR MESSAGE - conversion. */
				finished = True;
			}


			if (pdEntriesNum == 0)
				finished = True;
			else {
				/* Go through each item in the list. */
				for (i = 0; i < pdEntriesNum; i++) {
					if (file == (char *) NULL) {
						/* Display scrolling list. */
						entry = XmStringCreateLocalized(
							(char *) pdEntries[i]);
						XmListAddItemUnselected(
							SPELL_pdList(wid),
							entry, 0);
						XmStringFree(entry);
					} else {
						/* Save to file. */
						sprintf(word, "%s\n", 
							pdEntries[i]);
						fputs(word, fd);
					}
					free(pdEntries[i]);
				}
				free(pdEntries);
			}
		}
	}


	/* Finished writing out the file. */
	if (file != (char *) NULL)
		fclose(fd);


	/* After listing the contents, we need to unlock the personal
	 | dictionary, or else spell check will not work.
	 */
	if (_lockPersonalDict(&spellEngine, False) != True)
	{
		/* Display an error message about releasing the lock. */
		failMsg = (char *) calloc(strlen(catgets(_spellCat, 1, 15,
			"Warning - ")) + strlen(catgets(_spellCat, 2, 26, 
			"could not release lock.")) +5, sizeof(char));
		sprintf(failMsg, "%s%s", catgets(_spellCat, 1, 15,
			"Warning - "), catgets(_spellCat, 2, 26, 
			"could not release lock."));
		return(failMsg);
	}


	/* Terminate the personal dictionary, since the user wants it
	 | off during spell checking.  We just temporarily turned it
	 | on to list out the contents of the personal dictionary.
	 */
	if (!SPELL_personal(wid)) {
		_terminatePersonalDict(spellEngine);
	}


	/* All is okay. */
	return(NULL);
}


/* Function	:	_updateDict()
 |
 | Objective    :	Update the personal dictionary contents after
 |			adding or deleting word(s) from the options
 |			dialog window.
 | 
 */

char *
_updateDict(SDtSpellSelectionBoxWidget spell)
{
	WordListP	p;	/* Pointer to traverse word list. */
	SpellEngP spellEngine = (SpellEngP)  SPELL_engine(spell);
	char *error;
	


	Widget	errWid;		/* ID of error dialog widget. */
	Boolean	done;		/* Was error dailog dismissed? */
	char	*failMsg;	/* Buffer for error message. */
	Boolean delOK = True;	/* Did deletion succeed? */


	/* The user might have turned off usage of personal dictionary
	 | when checking, but may have added/deleted words via the
	 | options.  We need to create handles for the personal
	 | dictionary structures in order to correctly add and delete
	 | words. */
	if (!SPELL_personal(spell)) {
		spellEngine->userDict = 
			_SDtSpellCreateUserDict(spellEngine->mainDict,
						SPELL_dictionary(spell));
	}




	/* Start with the addWords list.  Add words that have been
	 | imported into the personal dictionary.
	 */
	p = SPELL_addWords(spell);
	while(p) {
		if (!(_addPersonalDict(p->word, spellEngine))) {
			/* Bring up a dialog that tells the user what word 
			 | we could not delete.
			 */
			failMsg = (char *) calloc(strlen(catgets(_spellCat, 2, 
				30, "Failed to add word: ")) + 
				strlen(p -> word) + 5, sizeof(char));
			sprintf(failMsg, "%s%s", catgets(_spellCat, 2, 30,
				"Failed to add word: "), p -> word);
			return(failMsg);
		}

		/* Continue on with other words in list, even if another
		 | word failed.
		 */
		p = p -> next;
	}


	/* Free the addWords list. */
	_freeList(SPELL_addWords(spell));
	SPELL_addWords(spell) = (WordListP) NULL;


	/* Now proceed with the deleteWordsList.  Remove the words
	 | that the user has marked for deletion from the personal
	 | dictionary.
	 */
	p = SPELL_deleteWords(spell);
	while(p)
	{
		/* Try and delete the word. */
		if (!(_delPersonalDict(p -> word, &spellEngine)))
		{
			/* Bring up a dialog that tells the user what word 
			 | we could not delete.
			 */
			failMsg = (char *) calloc(strlen(catgets(_spellCat, 2, 
				23, "Failed to delete word: ")) + 
				strlen(p -> word) + 5, sizeof(char));
			sprintf(failMsg, "%s%s", catgets(_spellCat, 2, 23,
				"Failed to delete word: "), p -> word);
			return(failMsg);
		}


		/* Continue on with other words in list, even if another
		 | word failed.
		 */
		p = p -> next;
	}


	/* Free the deleteWords list. */
	_freeList(SPELL_deleteWords(spell));
	SPELL_deleteWords(spell) = (WordListP) NULL;


	/* Now save the dictionary. */
	if (!(_savePersonalDict(spellEngine)))
	{
		/* Display an error message about releasing the lock. */
		failMsg = (char *) calloc(strlen(catgets(_spellCat, 2, 24,
			"Deletion failed - ")) + strlen(catgets(_spellCat, 2, 
			27, "could not save dictionary.")) + 5, sizeof(char));
		sprintf(failMsg, "%s%s", catgets(_spellCat, 2, 24,
			"Deletion failed - "), catgets(_spellCat, 2, 27,
			"could not save dictionary."));
		return(failMsg);
	}


	/* Terminate the personal dictionary, since the user wants it
	 | off during spell checking.  We just temporarily turned it
	 | on to list out the contents of the personal dictionary.
	 */
	if (!SPELL_personal(spell)) {
		_terminatePersonalDict(spellEngine);
	}


	return(NULL);
}
