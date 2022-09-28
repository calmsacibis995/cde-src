/* $XConsortium: XpmCrIFData.c /main/cde1_maint/2 1995/09/06 02:18:44 lehors $ */
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
FUNC(_DtXpmCreateImageFromData, int, (Display * display,
				   char **data,
				   XImage ** image_return,
				   XImage ** shapemask_return,
				   XpmAttributes * attributes));


int
_DtXpmCreateImageFromData(display, data, image_return,
		       shapeimage_return, attributes)
    Display *display;
    char **data;
    XImage **image_return;
    XImage **shapeimage_return;
    XpmAttributes *attributes;
{
    return _XmXpmCreateImageFromData(display, data, image_return,
			   shapeimage_return, attributes);
}
