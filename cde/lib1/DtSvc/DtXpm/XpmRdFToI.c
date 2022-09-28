/* $XConsortium: XpmRdFToI.c /main/cde1_maint/3 1995/10/08 22:22:39 pascale $ */
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
extern xpmDataType *_DtxpmDataTypes;
/* #include "xpm.h" */
FUNC(_DtXpmReadFileToImage, int, (Display * display,
			       char *filename,
			       XImage ** image_return,
			       XImage ** shapeimage_return,
			       XpmAttributes * attributes));



xpmDataType *_DtxpmDataTypes = _XmxpmDataTypes;

int
_DtXpmReadFileToImage(display, filename, image_return,
		   shapeimage_return, attributes)
    Display *display;
    char *filename;
    XImage **image_return;
    XImage **shapeimage_return;
    XpmAttributes *attributes;
{
    return _XmXpmReadFileToImage(display, filename, image_return,
		       shapeimage_return, attributes);
}
