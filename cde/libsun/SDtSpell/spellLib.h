/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

#ifndef _SPELLLIB_H_
#define _SPELLLIB_H_

#pragma ident "@(#)spellLib.h	1.13 96/06/11 SMI"

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------- TYPE DEFITIONS -------------------------------*/

/*
 * This structure will contain the spelling correction suggestions | returned
 * by the spell engine.
 */
typedef struct {
	char           *word;
	char          **alternatives;
	int             alt_num;
	int             errortype;
} wordReturn, *wordRetPtr;


/*---------------------------- FUNCTION DEFITIONS ----------------------------*/

extern void    *_SDtSpellInitSpellEnginePtr(Boolean *, char **);
extern char    *_SDtSpellConvertErrorTypes(int);
extern char    *_SDtSpellCloseSpell(void *);
extern Boolean  _SDtSpellCheckWord(void *, char *, wordReturn *, char **,
				   int *, int *);
extern Boolean  _SDtSpellChangePersonal(void *, Boolean);
extern Boolean  _SDtSpellAddWord(void *, char *);
extern Boolean  _SDtSpellIgnoreWord(void *, char *);
extern void     _SDtSpellGlobalChange(void *, char *);
extern Boolean	_SDtSpellReInit(void *, char **);
extern PDBuffP	_SDtSpellCreateUserDict(ICBuffP, char *);


#ifdef __cplusplus
}
#endif


#endif				/* _SPELLLIB_H_ */
