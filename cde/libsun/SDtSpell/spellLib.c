/* 
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */ 

#pragma ident "@(#)spellLib.c	1.22 96/08/28 SMI"


/* This file contains all functionality related to the specific spell
 | implementation.
 */


/*------------------------------- INCLUDE FILES ------------------------------*/

#include "spelldefs.h"
#include "spelltypes.h"
#include "spellLib.h"
#include "globals.h"
#include "engine.h"
#include "list.h"
#include "misc_spell.h"
#include "defaults.h"


/*--------------------------------- FUNCTIONS --------------------------------*/

/* Function	:	_SDtSpellCreateMainDict()
 | 
 | Objective    :       Allocate memory required by the main dictionary
 |			and initialize the spell engine.
 | 
 | Arguments	:	char *	- dictionary string, to determine 
 |				  the language dictionary to use.
 |
 | Return value :       ICBuffP, which is a pointer to the main
 |                      dictionary part of the spell engine.  If memory
 |                      cannot be allocated, or if the engine could not
 |                      be initialized, then this value is NULL.
 */

ICBuffP
_SDtSpellCreateMainDict(char **language)
{
	ICBuffP new;			/* Return value. */


	/* Are we able to access the spell engine library? */
	if (_initEngineFuncs()) {
		/* Allocate memory for the main dictionary. */
		new = (ICBuffP) calloc(1, sizeof(ICBuffR));
		if (new == (ICBuffP) NULL)
			return (NULL);


		/* Initialize the spell engine using the newly-
		 | allocated main dictionary variable.
		 */
		new->icBuff = _initMainDict(language);
		if (new->icBuff == (ICBUFF *) NULL) {
			free(new);
			return((ICBuffP) NULL);
		}
		return(new);
	} else
		return((ICBuffP) NULL);
}



/* Function	:	_SDtSpellCreateUserDict()
 | 
 | Objective    :       Allocate memory required by the user dictionary
 |                      and initialize the spell engine so that
 |			personal dictionaries can be used.
 | 
 | Arguments    :       PDBuffP, a pointer to the main dictionary that
 |			this user dictionary will be associated with.
 |
 | Return value :       PDBuffP, which is a pointer to the main
 |                      dictionary part of the spell engine.  If memory
 |                      cannot be allocated, or if the engine could not
 |			be initialized, then this value is NULL.
 */

PDBuffP
_SDtSpellCreateUserDict(ICBuffP main, char *language)
{
	PDBuffP     new;                      /* Return value. */
        char       *pd_path = (char *) NULL;  /* Personal dictionary path */
        char       *pd_file = (char *) NULL;  /* and filename.            */


	/* Allocate space for the user dictionary. */
	new = (PDBuffP) calloc(1, sizeof(PDBuffR));
	if (new != (PDBuffP) NULL) {

		/* Get the path and filename of the personal dictionary. */
		_SDtSpellGetDefaultPD(language, &pd_path, &pd_file);


		/* Initialize the user dictionary. */
		new->pdBuff = 
			_initPersonalDict(&(main->icBuff), pd_path, pd_file);
		free(pd_path);
		free(pd_file);


		if (new->pdBuff == (PD_IC_IO *) NULL) {
			free(new);
			return ((PDBuffP) NULL);
		}
		return(new);
	}
	else
		return((PDBuffP) NULL);
}


/* Function	:	_SDtSpellReInit()
 |
 | Description  :       Function to check if main and user dictionary
 |                      variables need to be reinitialized.  This can
 |                      occur when the main and user dictionaries are
 |                      shut down when the user closes the spell
 |                      window, and then brings it back later.
 |
 | Arguments    :       Void *  - pointer to the spell engine
 |				  associated with the spell widget
 |
 |			char *	- dictionary string to determine what
 |				  language dictionary to use
 | Return value	:	None.
 */

Boolean
_SDtSpellReInit(void *engine, char **language)
{
	SpellEngP   spell = (SpellEngP) engine;


	/* The spell engine should not be null.  It should have
	 | been allocated during the widget creation routine.  If
	 | it is, then return FALSE.
	 */
	if (spell == (SpellEngP) NULL)
		return(False);


	/* Reinitialize the main dictionary if it is NULL. */
	if (spell->mainDict == (ICBuffP) NULL)
		spell->mainDict = _SDtSpellCreateMainDict(language);
	if (spell -> mainDict == (ICBuffP) NULL)
		return(False);


	/* If flag is true, then only reinitialize the user
	 | dictionary if it is NULL.
	 */
	if (spell->usePD) {

		if (spell->userDict == (PDBuffP) NULL)
			spell->userDict = 
				_SDtSpellCreateUserDict(spell->mainDict,
							*language);
		if (spell->userDict == (PDBuffP) NULL)
			spell->usePD = False;
	}

	/* Return True. */
	return(True);
}


/* Function	:	_SDtSpellInitSpellEnginePtr()
 | 
 | Objective    :       Initialize the spell engine (backend) that will
 |			be used by the CDE spell checker.
 |
 | Arguments    :       Boolean         - flag to determine if the
 |					  personal dictionary will 
 |					  be used
 |			char *		- dictionary string that
 |					  will determine what language
 |					  dictionary to use
 | Return value :       Generic void * pointer to the spell engine
 +			variable.
 */

void *
_SDtSpellInitSpellEnginePtr(Boolean *usePersonal, char **language)
{
	SpellEngP    spellEngine;	       /* Return value. */


	/* Allocate memory for the spell engine structure. */
	spellEngine = (SpellEngP) calloc(1, sizeof(SpellEngR));
	if (spellEngine == (SpellEngP) NULL)
		return (NULL);
	else {
		/* Create and initialize the main dictionary. */
		spellEngine->mainDict = _SDtSpellCreateMainDict(language);
		if (spellEngine->mainDict == NULL) {
			free(spellEngine);
			return((void *) NULL);
		}


		/* Create and initialize the user dictionary only
		 | if the usePersonal flag is true.
		 */
		if (*usePersonal) {
			spellEngine->userDict = 
				_SDtSpellCreateUserDict(spellEngine->mainDict,
							*language);


			/* If creation was not successful, then don't
			 | use personal dictionary.  However, spell
			 | checking can still continue using only
			 | the main dictionary.  Need to inform user
			 | that personal dictionaries won't be used.
			 */
			if (spellEngine->userDict == (PDBuffP) NULL)
				spellEngine->usePD = False;
			else
				spellEngine->usePD = True;

			*usePersonal = spellEngine->usePD;
		} else {
			spellEngine->userDict = (PDBuffP) NULL;
			*usePersonal = spellEngine->usePD = False;
		}
	}


	return ((void *) spellEngine);
}


/* Function	:	_SDtSpellCloseSpell()
 |
 | Objective	:	Perform clean up when the spell widget is
 |			unmapped after the user selects the "Close"
 |			button.
 |
 | Arugments    :       void *          - pointer to the spell engine
 |					  structure
 | 
 | Return value :       char *          - error message, if close
 |					  unsuccesful
 */

char *
_SDtSpellCloseSpell(void *spellptr)
{
	SpellEngP       spellEngine = (SpellEngP) spellptr;


	/* Just in case we have a GUI, but for one reason or another,
	 | the spell engine is null.
	 */
	if (spellEngine == NULL)
		return NULL;


	/* First terminate the use of the personal/custom dictionary. */
	if (spellEngine->usePD && spellEngine->userDict) {
		if (!_savePersonalDict(spellEngine)) {
			char *failMsg;
                        /* Display an error message. */
                        failMsg = (char *) calloc(
                                strlen(catgets(_spellCat, 1, 16, 
					"Spelling Checker - ")) +
                                strlen(catgets(_spellCat, 2, 27, 
					"could not save dictionary.")) +
                                strlen(catgets(_spellCat, 1, 36, 
					"Modifications will be lost.")) + 5,
                                sizeof(char));
                        sprintf(failMsg, "%s%s\n%s",
                                catgets(_spellCat, 1, 16,"Spelling Checker - "),
                                catgets(_spellCat, 2, 27,
					"could not save dictionary."),
                                catgets(_spellCat, 1, 36, 
					"Modifications will be lost."));

			return failMsg;
		}
		if (!_terminatePersonalDict(spellEngine)) {
			char *failMsg;
                        /* Display an error message. */
                        failMsg = (char *) calloc(
                                strlen(catgets(_spellCat, 1, 16, 
					"Spelling Checker - ")) +
                                strlen(catgets(_spellCat, 1, 17, 
					"failed to terminate personal dictionary.")) + 5,
                                sizeof(char));
                        sprintf(failMsg, "%s%s",
                                catgets(_spellCat, 1, 16,"Spelling Checker - "),
                                catgets(_spellCat, 1, 17, 
				  "failed to terminate personal dictionary."));

			return failMsg;
		}
	}


	/* Terminate the use of the main dictionary. */
	if (!_terminateMainDict(spellEngine)) {
		char *failMsg;
                /* Display an error message. */
                failMsg = (char *) calloc(
                        strlen(catgets(_spellCat, 1, 16, "Spelling Checker - ")) +
                        strlen(catgets(_spellCat, 1, 17, 
				"failed to terminate main dictionary.")) + 5,
                        sizeof(char));
                sprintf(failMsg, "%s%s",
                        catgets(_spellCat, 1, 16, "Spelling Checker - "),
                        catgets(_spellCat, 1, 17, 
				"failed to terminate main dictionary."));

		return failMsg;
	}


	/* After terminating, free memory occupied by the globally
	 | ignored or changed word list, and reset them to NULL.
	 */
	_freeList(spellEngine->ignoreWords);
	_freeList(spellEngine->globChanged);
	spellEngine->ignoreWords = (WordListP) NULL;
	spellEngine->globChanged = (WordListP) NULL;
	return NULL;
}


/* Function	:	_SDtSpellGetSuggestion()
 |
 | Objective	:	Get the alternative from the spell engine and
 | 			convert it to the machine's format (as opposed to
 |			spell engine format).
 | 
 | Arguments	:	SpellEngP		- spell engine pointer
 |			unsigned char **	- alternative in machine
 |						  format
 |			int			- location of alternative in
 |						  array of alternatives
 |
 | Return value	:	Result of conversion from spell engine format to
 |			machine format.
 */

int
_SDtSpellGetSuggestion(SpellEngP engine, unsigned char **dest, int array_loc)
{
	unsigned char *sugg;		/* Suggestion. */

	/* Get the alternative. */
	sugg = &(engine->mainDict->icBuff->list[
			engine->mainDict->icBuff->corptr[array_loc]]);


	/* Convert the suggestion from the spell engine format
	 | to a format the platform can understand and display.
	 */
	return(_convertWord(sugg, DEF_TO_NAT, engine->mainDict->icBuff, dest));
}


void
_SDtSpellBuildSuggestions(SpellEngP spell, wordReturn **wordinfo)
{
	int             convRes = OKRET;        /* Conversion result. */
	int             altnum;
	int		num;


	/* Get the list of possible correction suggestions,
	 | convert it to char ** and return it in the list
	 | argument.
	 */
	altnum = spell->mainDict->icBuff->num_alt;
	(*wordinfo)->alt_num = altnum;
	if (altnum > 0)	{
		(*wordinfo)->alternatives = 
			(char **) calloc(altnum, sizeof(char *));
		for (num = 0; num < altnum; num++) {
			convRes = _SDtSpellGetSuggestion(spell,
					(unsigned char **) 
					&((*wordinfo)->alternatives[num]), 
					num);
			if (convRes != OKRET) {
				/* NEED TO DISPLAY ERROR THAT
				 | NOT ALL ALTERNATIVES WERE
				 | CONVERTED CORRECTLY.
				 */
			}
		}
	} else
		(*wordinfo)->alternatives = (char **) NULL;
}



/* Function	:	_SDtSpellCheckWord()
 | 
 | Description	:	This function verifies the given word to see
 |			if it is correctly spelled.  If the word fails
 |			the verification, then the spell engine tries
 |                      to come up with a list of spell suggestions, and
 |			returns this list to the user.  In addition, the
 |			starting and ending locations of the word in the
 |			text source widget will be updated so that only 
 |			the word will be highlighted.  Leading and trailing 
 |			punctuation will not be selected when the word is 
 |			highlighted.
 |
 | Arguments	:	void *		- pointer to spell engine
 |			char *		- word to verify
 |			wordReturn *	- list of suggestions in case
 |					  verification fails
 |			char *		- error message from spell engine
 |                      int *           - starting position of word in
 |					  text source widget
 |                      int *           - ending position of word in
 |					  text source widget
 |
 | Return Value	:	True if the word is correctly verified, false
 | 			otherwise.
 */

Boolean 
_SDtSpellCheckWord(void *spellptr, char *word, wordReturn * wordinfo,
		   char **error, int *start, int *end)
{
	SpellEngP       spellEngine = (SpellEngP) spellptr;
	int             num, diff;
	unsigned char  *convWord;	/* The input word in dictionary,  */
	int             convRes;        /* format, and conversion result. */
	int             altnum;
	Boolean         alternates = False;	/* Alternates found? */
	Boolean         verified;


	/* This should never happen, but just in case... */
	if (word == NULL)
		return (True);


	/* Verify the word for its spelling correctness. */
	verified = _verifyWord(word, &(spellEngine->mainDict->icBuff),
			       &alternates);
	if (verified)
		return (True);		/* Word is valid. */


	/* Word is misspelled.  Strip the word of its leading and
	 | trailing punctuation, and copy to the word entry in
	 | wordReturn.
	 */
	wordinfo->word = _SDtSpellStripCopyWord(spellptr, word, start, end);
	wordinfo->errortype = spellEngine->mainDict->icBuff->error_flags;
	wordinfo->alt_num = 0;


	/* Was the word previously verified and then used in a 
	 | global change or global ignore operation?
	 */
	if ((spellEngine->ignoreWords != NULL &&
	     _isInList(spellEngine->ignoreWords, wordinfo->word)) ||
	     (spellEngine->globChanged != NULL &&
	     _isInList(spellEngine->globChanged, wordinfo->word))) {

		/* We don't need to check the word, since it was part
		 | of a global operation.  However, we need to fool the
		 | spell engine into thinking that this word was the
		 | word previously checked, because the engine looks at
		 | the previously checked word for certain type of
		 | errors like capitalization, double word errors, etc.
		 | Convert the word first to spell engine format, and put
		 | it in the prev member of the ICBUFF variable.
		 */
		convRes = _convertWord((unsigned char *) wordinfo->word,
				       NAT_TO_DEF,
				       spellEngine->mainDict->icBuff,
				       &convWord);
		if (convRes != OKRET) {
			/* SHOW AN ERROR HERE. */
			/* set error */
		}


		/* Copy the converted word to the prev_word member of the
		 | ICBUFF structure.
		 */
		strcpy((char *) spellEngine->mainDict->icBuff->prev_word,
		       (char *) convWord);
		free(convWord);


		/* Return True to let UI know the word is OK.  */
		return (True);
	}


	/* If no alternatives are found after verification, then invoke
	 | the correct function of the spell engine, in order to get a
	 | list of possible corrections.
	 */
	if (!alternates) {
		if (_correctWord(&(spellEngine->mainDict->icBuff), ICCORRECT,
				 &alternates)) {

			/* Get more one-word corrections. */
			while (alternates)
				_correctWord(&(spellEngine->mainDict->icBuff),
					     ICCORMORE, &alternates);
		}


		_SDtSpellBuildSuggestions(spellEngine, &wordinfo);


		/* Try space insertion correction, and append to
		 | the suggestion list.
		 */
		if ((spellEngine->mainDict->icBuff->len > 5) &&
		    (spellEngine->mainDict->icBuff->lang == SLENGLISH ||
		     spellEngine->mainDict->icBuff->lang == SLFRENCH || 
		     spellEngine->mainDict->icBuff->lang == SLITALIAN || 
		     spellEngine->mainDict->icBuff->lang == SLSPANISH ||
		     spellEngine->mainDict->icBuff->lang == SLCATALAN || 
		     spellEngine->mainDict->icBuff->lang == SLPORTUGUESE ||
		     spellEngine->mainDict->icBuff->lang == SLCZECH ||
		     spellEngine->mainDict->icBuff->lang == SLRUSSIAN)) {

				_correctWord(&(spellEngine->mainDict->icBuff),
					     ICSPACEINSERT, &alternates);


			/* If space insertion correction was performed, then
			 | get the list of possible correction suggestions,
			 | convert it to char ** and append it to the list
			 | of alternatives.
			 */
			if (spellEngine->mainDict->icBuff->num_alt > 0) {

				altnum = spellEngine->mainDict->icBuff->num_alt;
				diff = wordinfo->alt_num;
				wordinfo->alt_num += altnum;


				/* Resize the list, and append to the end. */
				wordinfo->alternatives = (char **)
					realloc(wordinfo->alternatives,
					wordinfo->alt_num * sizeof(char *));
				for (num = 0; num < altnum; num++) {
					convRes = _SDtSpellGetSuggestion(spellEngine, (unsigned char **) &(wordinfo->alternatives[num + diff]), num);

					if (convRes != OKRET) {
						/* NEED TO DISPLAY ERROR THAT
						 | NOT ALL ALTERNATIVES WERE
						 | CONVERTED CORRECTLY.
						 */
					}
				}
			}
		}
	} else {

		_SDtSpellBuildSuggestions(spellEngine, &wordinfo);

	}

	return False;
}


/* Function	:	_SDtSpellAddWord()
 | 
 | Objective	:	Add a word to the user's personal dictionary.
 |
 | Arguments	:	void *		- pointer to the spell engine
 |			char *		- word to delete from the 
 |					  user's personal dictionary.
 |
 | Return value	:	Boolean		- True if deletion succeeded,
 |					  False otherwise.
 */

Boolean
_SDtSpellAddWord(void *spellptr, char *word)
{
	return _addPersonalDict(word, (SpellEngP) spellptr);
}


/* Function	:	_SDtSpellIgnoreWord()
 |
 | Objective	:	Add the specified word to the list of words
 |			that are to be ignored globally in the source.
 |
 | Arguments	:	void *		- pointer to the spell engine
 |			char *		- word to globally ignore
 |
 | Return value	:	Boolean		- True if word was added to the
 |                                        ignore word list, False
 |					  otherwise.
 */

Boolean
_SDtSpellIgnoreWord(void *spellptr, char *word)
{
	SpellEngP       spellEngine = (SpellEngP) spellptr;


	spellEngine->ignoreWords =
		_addToList(spellEngine->ignoreWords, word);
}


/* Function	:	_SDtSpellIgnoreWord()
 |
 | Objective	:	Add the specified word to the list of words
 |			that have been changed globally in the source.
 |
 | Arguments	:	void *		- pointer to the spell engine
 |			char *		- word that was changed in the
 |					  entire document
 |
 | Return value	:	None.
 */

void
_SDtSpellGlobalChange(void *spellptr, char *oldword)
{
	SpellEngP       spellEngine = (SpellEngP) spellptr;


	spellEngine->globChanged =
		_addToList(spellEngine->globChanged, oldword);
}

/*------------------------------ STUBS FOR NOW -------------------------------*/


