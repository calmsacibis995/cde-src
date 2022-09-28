/* 
 * Copyright (c) 1996 Sun Microsystems, Inc. 
 */


#pragma ident "@(#)defaults.h	1.3 96/06/19 SMI"


/* This file contains defitions regarding default directories, filenames,
 | and the like that affect the user's personal preferences and personal
 | dictionaries.
 */


#ifndef _DEFAULTS_H
#define _DEFAULTS_H


#ifdef __cplusplus
extern "C" {
#endif


/*-------------------------------- DEFINITIONS -------------------------------*/

/* Default path and name for the personal dictionary. */
#define PD_PATH         "/.sdtspell_dict/"
#define	PD_BASENAME     "sdtspell_personal"


/*--------------------------------- DATA TYPES -------------------------------*/

/* Structure for the resources that are public.  This structure is
 | used when retrieving things from the .desksetdefaults file.
 */
typedef struct _SDtSpellRes {
        Boolean          usePersonal;
	Boolean		 followLocale;
        char            *defaultDictionary;
} SDtSpellResR, *SDtSpellResP;
 
 
/*------------------------- FUNCTION DECLARATIONS ----------------------------*/

extern void		_SDtSpellGetDefaults(SDtSpellResP);
extern void		_SDtSpellSaveDefaults(SDtSpellResR);
extern char *		_SDtSpellGetDefaultsDir(void);
extern void		_SDtSpellGetDefaultPD(char *, char **, char **);


#ifdef __cplusplus
}
#endif


#endif /* _DEFAULTS_H */
