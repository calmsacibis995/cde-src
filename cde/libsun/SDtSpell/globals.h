/* 
 * Copyright (c) 1996 Sun Microsystems, Inc. 
 */

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#pragma ident "@(#)globals.h	1.6 96/05/02 SMI"

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------ INCLUDE FILES -------------------------------*/

#include <nl_types.h>

#include "spelltypes.h"


/*-------------------------------- DEFITIONS ---------------------------------*/

/* Bit masks for the error dialog, in order to determine which buttons
 | to unmanage.
 */
#define	ERR_MASK_OK	0x4
#define ERR_MASK_CANCEL	0x2
#define ERR_MASK_HELP	0x1


/*---------------------------- GLOBAL VARIABLES ------------------------------*/

/* Handle to the message catalog. */
extern nl_catd		_spellCat;


#ifdef __cplusplus
}
#endif		/* __cplusplus */


#endif		/* _GLOBALS_H_ */
