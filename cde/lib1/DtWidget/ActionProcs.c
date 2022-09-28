/**********************************<+>*************************************
***************************************************************************
**
**  File:        ActionProcs.c
**
**  Project:     Editor widget 
**
**  Description: Contains the code implementing the action procedures
**               and translation tables specific to Editor editing.
**  -----------
**
*******************************************************************
**  (c) Copyright Hewlett-Packard Company, 1991, 1992, 1993.  All rights are
**  reserved.  Copying or other reproduction of this program
**  except for archival purposes is prohibited without prior
**  written consent of Hewlett-Packard Company.
********************************************************************
**
**
**************************************************************************
**********************************<+>*************************************/
#include "EditorP.h"
#include <X11/keysym.h> 


/*******************************************************
 * 
 *  Procedure definitions
 *
 *******************************************************/


/* XXX Obsolete 
void
_DtEditorAddActionProcs(
	DtEditorWidget pPriv)
{
    XtAppAddActionHook(M_app_context(pPriv), 
		       (XtActionHookProc)GlobalActionHook, 
		       (XtPointer)&_DtEditorActionName);
    XtAppAddActions(M_app_context(pPriv), mediaEditActions, 
		    XtNumber(mediaEditActions));
}
XXX Obsolete */




/************************************************************************
 *  _GetModeSwitchModifier
 *      Find if any Mod<n> modifier is acting as the Group modifier - you
 *      can't assume Mod1Mask is always it.
 *      Returns the mask of the modifier key to which the Mode_switch
 *      keysym is attached.
 *
 ************************************************************************/
/* ARGSUSED */
static unsigned int
_GetModeSwitchModifier(
                Display *dpy)
{
    register XModifierKeymap *pMap;
    register int mapIndex, keyCol, mapSize;
    KeySym keySym;
    unsigned int modeSwitchModMask = 0;
    pMap = XGetModifierMapping(dpy);
    mapSize = 8*pMap->max_keypermod;

    for (mapIndex = 3*pMap->max_keypermod; mapIndex < mapSize; mapIndex++) {
        /* look only at the first 4 columns of key map */
        for (keyCol = 0; keyCol < 4; keyCol++) {
            keySym = XKeycodeToKeysym(dpy, pMap->modifiermap[mapIndex], keyCol);
            if (keySym == XK_Mode_switch)
                modeSwitchModMask |= 1 << (mapIndex / pMap->max_keypermod);
        }
    }

    XFreeModifiermap(pMap);
    return modeSwitchModMask;
}

/************************************************************************
 *
 *  _DtEditorQuoteNextChar
 *      This action routine circumvents the normal XtDispatchEvent
 *      mechanism, inserting the next control or extended character
 *      directly into text without processed by event handlers or the
 *      translation manager.  This means, for
 *      example, that the next control or extended character will be
 *      inserted into text without being intrepreted as a Motif menu
 *      or text widget accelerator.
 *
 *       NOTE:  ExtendcharMask should be a global set via a MappingNotify
 *              event handler rather than each time this routine is called
 *
 ************************************************************************/
/* ARGSUSED */
void
_DtEditorQuoteNextChar(
                Widget w,
                XEvent *event)
{
    XtAppContext app = XtWidgetToApplicationContext(w);
    static unsigned int ExtendcharMask;
/* XXX if (! ExtendcharMask)  */
        ExtendcharMask = _GetModeSwitchModifier(XtDisplay(w));
    for (;;) {
        XEvent nextEvent;
        XtAppNextEvent(app, &nextEvent);
        if ((nextEvent.type == KeyPress) &&
          (! IsModifierKey(XLookupKeysym((XKeyEvent *) &nextEvent,0)))) {
            if (nextEvent.xkey.state & (ControlMask|ExtendcharMask)) {
                XtCallActionProc(w, "self-insert", &nextEvent, NULL, 0);
            } else {
                XtDispatchEvent(&nextEvent);
            }
            break;
        } else {
            XtDispatchEvent(&nextEvent);
        }
    }
} /* end _DtEditorQuoteNextChar */

