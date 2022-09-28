#ifndef lint
#ifdef  VERBOSE_REV_INFO
static char rcs_id[] = "$XConsortium: TermPrimWMProtocols.c /main/cde1_maint/1 1995/07/15 01:37:39 drk $";
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
#include <Xm/Xm.h>
#include <Xm/Protocols.h>

static Atom xa_WM_DELETE_WINDOW;
static Atom xa_WM_SAVE_YOURSELF;
static Boolean initialized = False;

static void
protocolsInitialize(Widget topLevel)
{
    if (!initialized) {
	xa_WM_DELETE_WINDOW = XInternAtom(XtDisplay(topLevel),
		"WM_DELETE_WINDOW", False);
	xa_WM_SAVE_YOURSELF = XInternAtom(XtDisplay(topLevel),
		"WM_SAVE_YOURSELF", False);
	initialized = True;
    }
}

void
_DtTermPrimAddDeleteWindowCallback(Widget topLevel, XtCallbackProc callback,
	XtPointer client_data)
{
    /* initialize things... */
    if (!initialized)
	(void) protocolsInitialize(topLevel);
    (void) XmAddWMProtocols(topLevel, &xa_WM_DELETE_WINDOW, 1);
    (void) XmAddWMProtocolCallback(topLevel, xa_WM_DELETE_WINDOW, callback,
	    client_data);
}

void
_DtTermPrimAddSaveYourselfCallback(Widget topLevel, XtCallbackProc callback,
	XtPointer client_data)
{
    /* initialize things... */
    if (!initialized)
	(void) protocolsInitialize(topLevel);
    (void) XmAddWMProtocols(topLevel, &xa_WM_SAVE_YOURSELF, 1);
    (void) XmAddWMProtocolCallback(topLevel, xa_WM_SAVE_YOURSELF, callback,
	    client_data);
}
