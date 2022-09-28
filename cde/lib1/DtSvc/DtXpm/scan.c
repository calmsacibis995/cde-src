/* $XConsortium: scan.c /main/cde1_maint/3 1995/09/06 02:20:43 lehors $ */
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
FUNC(_DtxpmScanImage, int, (Display * display,
			 XImage * image,
			 XImage * shapeimage,
			 XpmAttributes * attributes,
			 xpmInternAttrib * attrib));

int
_DtxpmScanImage(display, image, shapeimage, attributes, attrib)
    Display *display;
    XImage *image;
    XImage *shapeimage;
    XpmAttributes *attributes;
    xpmInternAttrib *attrib;
{
    return _XmxpmScanImage(display, image, shapeimage, attributes, attrib);
}
