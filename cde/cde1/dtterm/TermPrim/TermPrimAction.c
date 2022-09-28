#ifndef lint
#ifdef  VERBOSE_REV_INFO
static char rcs_id[] = "$XConsortium: TermPrimAction.c /main/cde1_maint/1 1995/07/15 01:22:45 drk $";
#endif  /* VERBOSE_REV_INFO */
#endif  /* lint */

/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#include "TermHeader.h"
#include "TermPrimDebug.h"
#include "TermPrimP.h"
#include "TermPrimI.h"
#include "TermPrimData.h"
#include "TermPrimAction.h"
#include "TermPrimFunction.h"

static void
invokeAction(Widget w, char *transmitString, TermFunction function, int count)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    struct termData *tpd = tw->term.tpd;

    if (KEYBOARD_LOCKED(tpd->keyboardLocked)) {
	/* keyboard locked -- ring the bell...
	 */
	(void) _DtTermPrimBell(w);
    } else if (tpd->transmitFunctions && transmitString) {
	/* transmit functions mode -- transmit them...
	 */
	(void) _DtTermPrimSendInput(w, (unsigned char *) transmitString,
		strlen(transmitString));
    } else {
	/* perform the function...
	 */
	(*function)(w, count, fromAction);

#ifdef	NOTDEF
	/* we need to wait for the scroll to complete before turning
	 * on the cursor...
	 */
	(void) _DtTermPrimScrollComplete(w, True);
#endif	/* NOTDEF */
	(void) _DtTermPrimCursorOn(w);
    }
    return;
}

typedef struct {
    const char *string;
    char value;
} EnumType;

static int
stringToEnum(char *c, EnumType *enumTypes, int numEnumTypes)
{
    int i;

    for (i = 0; i < numEnumTypes; i++) {
	if (!strcmp(enumTypes[i].string, c))
	    return(i);
    }

    return(-1);
}


/*** INSERT CHAR/LINE *********************************************************
 * 
 *     #    #    #   ####   ######  #####    #####
 *     #    ##   #  #       #       #    #     #
 *     #    # #  #   ####   #####   #    #     #
 *     #    #  # #       #  #       #####      #
 *     #    #   ##  #    #  #       #   #      #
 *     #    #    #   ####   ######  #    #     #
 * 
 *                                       #
 *   ####   #    #    ##    #####       #   #          #    #    #  ######
 *  #    #  #    #   #  #   #    #     #    #          #    ##   #  #
 *  #       ######  #    #  #    #    #     #          #    # #  #  #####
 *  #       #    #  ######  #####    #      #          #    #  # #  #
 *  #    #  #    #  #    #  #   #   #       #          #    #   ##  #
 *   ####   #    #  #    #  #    # #        ######     #    #    #  ######
 */

void
_DtTermPrimActionInsert(Widget w, XEvent *event,
	String *params, Cardinal *num_params)
{
    Debug('i', fprintf(stderr,
	    ">>_DtTermPrimActionInsert: not yet implemented\n"));
}



/*** CURSOR MOTION ************************************************************
 * 
 *   ####   #    #  #####    ####    ####   #####
 *  #    #  #    #  #    #  #       #    #  #    #
 *  #       #    #  #    #   ####   #    #  #    #
 *  #       #    #  #####        #  #    #  #####
 *  #    #  #    #  #   #   #    #  #    #  #   #
 *   ####    ####   #    #   ####    ####   #    #
 * 
 * 
 *  #    #   ####    #####     #     ####   #    #
 *  ##  ##  #    #     #       #    #    #  ##   #
 *  # ## #  #    #     #       #    #    #  # #  #
 *  #    #  #    #     #       #    #    #  #  # #
 *  #    #  #    #     #       #    #    #  #   ##
 *  #    #   ####      #       #     ####   #    #
 */

void
_DtTermPrimActionRedrawDisplay(Widget w, XEvent *event,
	String *params, Cardinal *num_params)
{
    (void) invokeAction(w, NULL, _DtTermPrimFuncRedrawDisplay, 1);
    return;
}


void
_DtTermPrimActionReturn(Widget w, XEvent *event,
	String *params, Cardinal *num_params)
{
    Debug('i', fprintf(stderr,
	    ">>_DtTermPrimActionReturn: not yet implemented\n"));
    return;
}

void
_DtTermPrimActionTab(Widget w, XEvent *event,
	String *params, Cardinal *num_params)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    struct termData *tpd = tw->term.tpd;
    Boolean shiftedTab = False;
    Boolean numericTab = False;
    int i;

    (void) _DtTermPrimSendInput(w, (unsigned char *) "\t", 1);
    return;
}


/*** STRING *******************************************************************
 *  
 *   ####    #####  #####      #    #    #   ####
 *  #          #    #    #     #    ##   #  #    #
 *   ####      #    #    #     #    # #  #  #
 *       #     #    #####      #    #  # #  #  ###
 *  #    #     #    #   #      #    #   ##  #    #
 *   ####      #    #    #     #    #    #   ####
 */

void
_DtTermPrimActionString(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    unsigned char *c;
    unsigned char hexVal;

    /* this is based on the functionality offered by xterm...
     */

    if (*num_params != 1)
	return;

    /* process hex values... */
    if (((*params)[0] == '0') &&
	    (((*params)[1] == 'x') || ((*params)[1] == 'X')) &&
	    ((*params)[2] != '\0')) {
	for (hexVal = 0, c = (unsigned char *) (*params + 2); *c; c++) {
	    hexVal *= 16;
	    *c = tolower(*c);
	    if ((*c >= '0') && (*c <= '9')) {
		hexVal += *c - '0';
	    } else if ((*c >= 'a') && (*c <= 'f')) {
		hexVal += *c - 'a' + 10;
	    } else {
		break;
	    }
	}
	/* if we hit the end of the string, send the hex value... */
	if (*c == '\0') {
	    (void) _DtTermPrimSendInput(w, &hexVal, 1);
	}
    } else {
	(void) _DtTermPrimSendInput(w, (unsigned char *) *params, strlen(*params));
    }
    return;
}
	    

/*** KEYMAP *******************************************************************
 * 
 *  #    #  ######   #   #  #    #    ##    #####
 *  #   #   #         # #   ##  ##   #  #   #    #
 *  ####    #####      #    # ## #  #    #  #    #
 *  #  #    #          #    #    #  ######  #####
 *  #   #   #          #    #    #  #    #  #
 *  #    #  ######     #    #    #  #    #  #
 */

void
_DtTermPrimActionKeymap(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    XtTranslations keymap;
    char buffer[BUFSIZ];
    static XtTranslations original = (XtTranslations) 0;
    static XtResource key_resources[] = {
	{XtNtranslations, XtCTranslations, XtRTranslationTable,
		sizeof(XtTranslations), 0, XtRTranslationTable, (caddr_t) NULL}
    };
    char mapName[BUFSIZ];
    char mapClass[BUFSIZ];

    if (*num_params != 1) {
	return;
    }

    if (original == (XtTranslations) 0) {
	original = w->core.tm.translations;
    }

    if (!strcmp(params[0], "None")) {
	(void) XtOverrideTranslations(w, original);
	return;
    }

    (void) sprintf(mapName, "%sKeymap", params[0]);
    (void) strcpy(mapClass, mapName);
    mapClass[0] = toupper(mapClass[0]);
    (void) XtGetSubresources(w, (XtPointer) &keymap, mapName, mapClass,
	    key_resources, XtNumber(key_resources), NULL, (Cardinal) 0);

    if (keymap != NULL) {
	(void) XtOverrideTranslations(w, keymap);
    }
    return;
}
