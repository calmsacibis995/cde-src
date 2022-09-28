/* 
 * Copyright (c) 1996 Sun Microsystems, Inc. 
 */

#pragma ident "@(#)list.c	1.9 96/05/20 SMI"

/*------------------------------ INCLUDE FILES -------------------------------*/

#include <stdio.h>
#include <malloc.h>
#include <nl_types.h>

#include "spelltypes.h"


/*--------------------------- FUNCTION DEFINITIONS ---------------------------*/

/* Function	:	_freeList()
 |
 | Objective    :       This function frees the memory occupied by all the
 |			words in the word list, and the memory of the word
 |			list itself.  This is done by recursively calling
 |			this function.
 |
 | Arguments    :       WordListP	list	- the specified word list
 |
 | Return Value	:	None.
 |
 | Side Effects	:	None.
 */

#ifdef _NO_PROTO
void _freeList(WordListP list)
	WordListP	list;
#else
void _freeList(WordListP list)
#endif
{
	/* Make sure that list isn't null. */
	if (list == (WordListP) NULL)
		return;
	else
		_freeList(list -> next);


	/* Free the space occupied by the word, then the list item itself. */
	free(list -> word);
	free(list);
}


/* Function	:	_isInList()
 |
 | Objective    :       This function checks to see if the given word
 |			is in the specified word list.
 | 
 | Arguments    :       WordListP	list	- the specified word list
 |			char		*word	- the word to check if it is
 |						  already in the word list
 |
 | Return Value	:	True if the word is in the list, False otherwise.
 |
 | Side Effects	:	None.
 */

#ifdef _NO_PROTO
Boolean _isInList(list, word)
	WordListP	list;
	char		*word;
#else
Boolean _isInList(WordListP list, char *word)
#endif
{
	/* Temporary variable to traverse the list. */
	WordListP tmp = list;


	while (tmp != (WordListP) NULL)
	{
		if (strcmp(tmp -> word, word) == 0)
			return(True);
		else
			tmp = tmp -> next;
	};


	return(False);
}


/* Function	:	_addListSorted()
 |
 | Objective	:	Add a word into the given word list, while preserving
 |			the list's sorted order.
 |
 | Arguments	:	WordListP - sorted word list
 |			char *    - word to add in list
 |
 | Return value :	None.
 |
 | Side effects	:	None.
 |
 */

#ifdef _NO_PROTO
void _addListSorted(list, word)
	WordListP	*list;
	char		*word;
#else
void _addListSorted(WordListP *passlist, char *word)
#endif
{
	/* New item to add in the list. */
	WordListP new = (WordListP) NULL;
	WordListP list = *passlist;

	/* In case we need to add at the end of the list. */
	if (list == (WordListP) NULL)
	{
		list = (WordListP) malloc(sizeof(WordListR));
		list -> word = (char *) malloc(sizeof(char) * 
					       (strlen(word) + 5));
		strcpy(list -> word, word);
		list -> next = (WordListP) NULL;
		*passlist = list;
		return;
	}


	/* In case we need to add in front of the list. */
	if (strcmp(list -> word, word) > 0)
	{
		new = (WordListP) malloc(sizeof(WordListR));
		new  -> word = (char *) malloc(sizeof(char) * 
					       (strlen(word) + 5));
		strcpy(new  -> word, word);
		new -> next = list;
		*passlist = new;
		return;
	}


	/* If current word in list is less than the word we want to
	 | add, then recursivley add the word in the sorted list by
	 | proceeding to the next word list item.
	 */
	if (strcmp(list -> word, word) < 0)
		_addListSorted( &(list -> next), word);
}


/* _addToList()
 |
 | Objective    :       This function adds a word to the specified word list.
 | 			The word is added at the beginning of a word list.
 |
 | Arguments    :       WordListP	- the specified word list
 |			char *		- the word to add in the list
 |
 | Return Value	:	The updated word list that contains the new word.
 |
 | Side Effects	:	None.
 */

WordListP _addToList(WordListP list, char *word)
{
	WordListP	new;	/* If we need to add the word. */


	/* If the word isn't in the list, then create a new entry
	 | for it, add it to the beginning, and return the list.
	 */
	if (!(_isInList(list, word)))
	{
		new = (WordListP) malloc(sizeof(WordListR));

		new -> word = strdup(word);
		new -> next = list;
		list = new;
	}

	return(list);
}


