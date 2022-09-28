/* 
 * Copyright (c) 1996 Sun Microsystems, Inc. 
 */

#ifndef _SPELLDEFS_H_
#define _SPELLDEFS_H_

#pragma ident "@(#)spelldefs.h	1.6 97/03/03 SMI"

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------- CONSTANT DEFINTIONS ----------------------------*/

#define	SDTSPELL_MSG_CATALOG	"SDtSpell"


/*----------------------------- ENUMERATED TYPES -----------------------------*/

/* To determine what type of widget we are working with. */
typedef enum
{
	SPL_XMTEXT,
	SPL_DTEDITOR
} SPL_WIDGET_TYPE;


/* Locales that the CDE spell checker supports. */
typedef enum
{
	CA,		/* Catalan 		*/
	EN_AU,		/* English (Australia)	*/
	EN_UK,		/* English (UK)		*/
	EN_US,		/* English (US)		*/
	DE,		/* German (Germany)	*/
	DE_CH,		/* German (Switzerland)	*/
	ES,		/* Spanish		*/
	FR,		/* French 		*/
	IT,		/* Italian		*/
	SV,		/* Swedish		*/
	UNSUPPORTED	/* Unsupported locale	*/
} SPL_LANGUAGE;


#ifdef __cplusplus
}
#endif		/* __cplusplus */


#endif		/* _SPELLDEFS_H_ */
