/* 
 * Copyright (c) 1996 Sun Microsystems, Inc. 
 */

#ifndef _MISC_SPELL_H_
#define _MISC_SPELL_H_

#pragma ident "@(#)misc_spell.h	1.7 96/06/19 SMI"

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------- DEFINES ----------------------------------*/

/* Bit masks to define buttons in the message dialog. */
#define DLG_BTN_MASK_OK		0x4
#define DLG_BTN_MASK_CANCEL	0x2
#define DLG_BTN_MASK_HELP	0x1


/* Size of error message buffer. */
#define MSG_SIZE	1024


/*--------------------------- FUNCTION DEFITIONS -----------------------------*/


extern char	 *_SDtSpellStripCopyWord(SpellEngP, char *, int *, int *);
extern char	 *_SDtSpellErrorTypes(int);
extern Widget	  _SDtSpellShowDialog(Widget, char *, int, int, int, char *, 
				    char *, char *, char*);
extern LangMenuP  _SDtSpellGetAvailDicts(SpellEngP, Widget);
extern char      *_SDtSpellLangToLocale(SPL_LANGUAGE);


#ifdef __cplusplus
}
#endif		/* __cplusplus */


#endif		/* _MISC_SPELL_H_ */
