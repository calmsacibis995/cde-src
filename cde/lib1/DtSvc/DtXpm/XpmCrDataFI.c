/* $XConsortium: XpmCrDataFI.c /main/cde1_maint/2 1995/09/06 02:18:15 lehors $ */
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

/* #include "xpmP.h" */
FUNC(_DtxpmCreateData, int, (char ***data_return,
	    xpmInternAttrib * attrib, XpmAttributes * attributes));

/* #include "xpm.h" */
FUNC(_DtXpmCreateDataFromImage, int, (Display * display,
				    char ***data_return,
				    XImage * image,
				    XImage * shapeimage,
				    XpmAttributes * attributes));

int
_DtXpmCreateDataFromImage(display, data_return, image, shapeimage, attributes)
    Display *display;
    char ***data_return;
    XImage *image;
    XImage *shapeimage;
    XpmAttributes *attributes;
{
    return _XmXpmCreateDataFromImage(display, data_return, image, shapeimage, attributes);
}

int
_DtxpmCreateData(data_return, attrib, attributes)
    char ***data_return;
    xpmInternAttrib *attrib;
    XpmAttributes *attributes;
{
    return _XmxpmCreateData(data_return, attrib, attributes);
}
