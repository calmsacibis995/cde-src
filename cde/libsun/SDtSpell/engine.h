/* 
 * Copyright (c) 1996 Sun Microsystems, Inc. 
 */

#ifndef _ENGINE_H
#define _ENGINE_H

#pragma ident "@(#)engine.h	1.8 96/06/11 SMI"

#ifdef __cplusplus
extern "C" {
#endif

#include "spelltypes.h"


/* Locale names.  These are the locales supported by the spell checker. */
#define	LOCALE_C		"C"			/* English */
#define	LOCALE_CA		"ca"			/* Catalan */
#define LOCALE_EN_AU		"en_AU"			/* English */
#define LOCALE_EN_UK		"en_UK"
#define LOCALE_EN_US		"en_US"
#define LOCALE_DE		"de"			/* German */
#define LOCALE_DE_CH		"de_CH"
#define LOCALE_ES		"es"			/* Spanish */
#define LOCALE_FR		"fr"			/* French */
#define LOCALE_FR_BE		"fr_BE"
#define LOCALE_FR_CA		"fr_CA"
#define LOCALE_FR_CH		"fr_CH"
#define LOCALE_IT		"it"			/* Italian */
#define LOCALE_SV		"sv"			/* Swedish */


/* Global error message place holder. */
extern char _sdtSpellCheckMsg[256];


/* Spell engine functions. */

extern SPL_LANGUAGE	_getLanguage(char *);
extern void		_setDictLanguage(ICBUFF *, char **);
extern Boolean   	_initEngineFuncs(void);
extern ICBUFF    	*_initMainDict(char **);
extern PD_IC_IO  	*_initPersonalDict(ICBUFF **, char *, char *);
extern Boolean   	_terminateMainDict(SpellEngP);
extern Boolean   	_terminatePersonalDict(SpellEngP);
extern Boolean   	_addPersonalDict(char *, SpellEngP);
extern Boolean   	_delPersonalDict(char *, SpellEngP *);
extern Boolean   	_getPersonalDictEntry(SpellEngP *);
extern Boolean   	_lockPersonalDict(SpellEngP *, Boolean);
extern Boolean	 	_savePersonalDict(SpellEngP); 
extern int		_convertWord(unsigned char *, int, ICBUFF *,
				     unsigned char **);
extern Boolean   	_correctWord(ICBUFF **, int, Boolean *);
extern Boolean   	_verifyWord(char *, ICBUFF **, Boolean *);


#ifdef __cplusplus
}
#endif

#endif /* _ENGINE_H */
