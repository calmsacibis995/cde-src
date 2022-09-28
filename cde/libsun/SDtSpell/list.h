/* 
 * Copyright (c) 1996 Sun Microsystems, Inc. 
 */

#ifndef _LIST_H_
#define _LIST_H_

#pragma ident "@(#)list.h	1.6 96/05/08 SMI"

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------- FUNCTION DEFITIONS -----------------------------*/

#ifdef _NO_PROTO

extern void		_freeList();
extern Boolean		_isInList();
extern void		_addListSorted();
extern WordListP	_addToList();

#else

extern void		_freeList(WordListP);
extern Boolean		_isInList(WordListP, char *);
extern void		_addListSorted(WordListP *, char *);
extern WordListP	_addToList(WordListP, char *);

#endif		/* _NO_PROTO */


#ifdef __cplusplus
}
#endif		/* __cplusplus */


#endif		/* _LIST_H_ */
