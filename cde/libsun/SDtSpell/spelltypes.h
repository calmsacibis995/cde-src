/* 
 * Copyright (c) 1996 Sun Microsystems, Inc. 
 */

#ifndef _SPELLTYPES_H_
#define _SPELLTYPES_H_

#pragma ident "@(#)spelltypes.h	1.8 96/08/28 SMI"

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------ INCLUDE FILES -------------------------------*/

#include <stdio.h>
#include <nl_types.h>
#include <X11/Intrinsic.h>


/* Required Inso Spell Engine include files. */
#include <SDtSpellE/sllibhdr.h>
#include <SDtSpellE/icsapi.h>
#include <SDtSpellE/icsext.h>


/* Definitions and enumerated types unique to the spell checker. */
#include "spelldefs.h"


/*----------------------------- TYPE DEFINITIONS -----------------------------*/

/* NOTE: Type definitions ending in R refer to a single record, while those
 | ending in P refer to a pointer.
 */


/* Word list needed to keep track of either menu labels (used for language
 | dictionaries that are loaded onto the system; menu is built on the
 | fly), or words that have been designated as "ignore all" or "change all."
 */
typedef struct _WordListItem
{
	char			*word;
	struct _WordListItem	*next;

} WordListR, *WordListP;


/* By default, the only the English dictionaries are installed.  If
 | other languages are installed, then we'll have to add menu items
 | on the fly.
 */
typedef struct _LangMenuItem
{
	Widget			langItem;   /* Menu item. */
	SPL_LANGUAGE		lang;	    /* Language associated with item. */
	struct _LangMenuItem	*next;

} LangMenuR, *LangMenuP;


/* The ICBUFF structure required by the Inso spell engine to access
 | the language dictionary(s).  The engine supports the use of more
 | than one language dictionary.
 */
typedef struct _ICBuffItem
{
	ICBUFF			*icBuff;
	struct _ICBuffItem	*next;

} ICBuffR, *ICBuffP;

 
/* The PD_IO_IC structure required by the Inso spell engine to access
 | the user's personal dictionaries.  The engine supports the use of
 | more than one personal dictionary.
 */
typedef struct _PDBuffItem
{
	PD_IC_IO		*pdBuff;
	struct _PDBuffItem	*next;

} PDBuffR, *PDBuffP;


/* Structure of buffers required by the spell engine. */
typedef struct _SpellEngItem
{
	ICBuffP			mainDict;     /* Main dictionary(s). */
	PDBuffP			userDict;     /* Personal dictionary(s). */
	Boolean			usePD;        /* Use personal dictionary(s)? */
	struct _spellEngItem	*next;
	WordListP 		ignoreWords;
	WordListP		globChanged;

} SpellEngR, *SpellEngP;



#ifdef __cplusplus
}
#endif		/* __cplusplus */


#endif		/* _SPELLTYPES_H_ */
