/* $XConsortium: XpmCrDataFP.c /main/cde1_maint/2 1995/09/06 02:18:30 lehors $ */
/*
 * (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.                *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

/*
 * Duplicate code in Motif library, this is a compatibility layer
 */

#include "_xpmI.h"

/* #include "xpm.h" */
FUNC(_DtXpmCreateDataFromPixmap, int, (Display * display,
				    char ***data_return,
				    Pixmap pixmap,
				    Pixmap shapemask,
				    XpmAttributes * attributes));


int
_DtXpmCreateDataFromPixmap(display, data_return, pixmap, shapemask, attributes)
    Display *display;
    char ***data_return;
    Pixmap pixmap;
    Pixmap shapemask;
    XpmAttributes *attributes;
{
    return _XmXpmCreateDataFromPixmap(display, data_return, pixmap,
					shapemask, attributes);
}
