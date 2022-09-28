/* $XConsortium: create.c /main/cde1_maint/2 1995/09/06 02:19:15 lehors $ */
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
FUNC(_DtxpmCreateImage, int, (Display * display,
			   xpmInternAttrib * attrib,
			   XImage ** image_return,
			   XImage ** shapeimage_return,
			   XpmAttributes * attributes));
FUNC(_Dtxpm_xynormalizeimagebits, void, (register unsigned char *bp,
				    register XImage * img));
FUNC(_Dtxpm_znormalizeimagebits, void, (register unsigned char *bp,
				    register XImage * img));


_DtxpmCreateImage(display, attrib, image_return, shapeimage_return, attributes)
    Display *display;
    xpmInternAttrib *attrib;
    XImage **image_return;
    XImage **shapeimage_return;
    XpmAttributes *attributes;
{
    return _XmxpmCreateImage(display, attrib, image_return,
			    shapeimage_return, attributes);
}

void
_Dtxpm_xynormalizeimagebits(bp, img)
    register unsigned char *bp;
    register XImage *img;
{
    _Xmxpm_xynormalizeimagebits(bp, img);
}

void
_Dtxpm_znormalizeimagebits(bp, img)
    register unsigned char *bp;
    register XImage *img;
{
    _Xmxpm_znormalizeimagebits(bp, img);
}
