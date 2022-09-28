/* $XConsortium: image.c /main/cde1_maint/2 1995/08/29 19:58:44 gtsang $ */
/*
 */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*-
 * image.c - image bouncer for dtscreen, the X Window System lockscreen.
 *
 * Copyright (c) 1991 by Patrick J. Naughton.
 *
 * See dtscreen.c for copying information.
 *
 * Revision History:
 * 29-Jul-90: Written.
 */

#include "dtscreen.h"
#include "xlogo.bit"

static XImage logo = {
    0, 0,			/* width, height */
    0, XYBitmap, 0,		/* xoffset, format, data */
    LSBFirst, 8,		/* byte-order, bitmap-unit */
    LSBFirst, 8, 1		/* bitmap-bit-order, bitmap-pad, depth */
};

#define MAXICONS 256

typedef struct {
    int         x;
    int         y;
}           point;

typedef struct {
    int         width;
    int         height;
    int         nrows;
    int         ncols;
    int         xb;
    int         yb;
    int         iconmode;
    int         iconcount;
    point       icons[MAXICONS];
    long        startTime;
}           imagestruct;

void
drawimage(pwin)
    perwindow *pwin;
{
    imagestruct *ip;
    int         i;

    ip = (imagestruct *)pwin->data;
    XSetForeground(dsp, pwin->gc, BlackPixelOfScreen(pwin->perscreen->screen));
    for (i = 0; i < ip->iconcount; i++) {
	if (!ip->iconmode)
	    XFillRectangle(dsp, pwin->w, pwin->gc,
			   ip->xb + xlogo_width * ip->icons[i].x,
			   ip->yb + xlogo_height * ip->icons[i].y,
			   xlogo_width, xlogo_height);

	ip->icons[i].x = random() % ip->ncols;
	ip->icons[i].y = random() % ip->nrows;
    }
    if (pwin->perscreen->npixels == 2)
      XSetForeground(dsp, pwin->gc, WhitePixelOfScreen(pwin->perscreen->screen));
    for (i = 0; i < ip->iconcount; i++) {
	if (pwin->perscreen->npixels > 2)
	    XSetForeground(dsp, pwin->gc,
		 pwin->perscreen->pixels[random() % pwin->perscreen->npixels]);

	XPutImage(dsp, pwin->w, pwin->gc, &logo,
		  0, 0,
		  ip->xb + xlogo_width * ip->icons[i].x,
		  ip->yb + xlogo_height * ip->icons[i].y,
		  xlogo_width, xlogo_height);
    }
}

void
initimage(pwin)
    perwindow *pwin;
{
    XWindowAttributes xgwa;
    imagestruct *ip;

    if (pwin->data) free(pwin->data);
    pwin->data = (void *)malloc(sizeof(imagestruct));
    memset(pwin->data, '\0', sizeof(imagestruct));
    ip = (imagestruct *)pwin->data;
    ip->startTime = seconds();

    logo.data = (char *) xlogo_bits;
    logo.width = xlogo_width;
    logo.height = xlogo_height;
    logo.bytes_per_line = (xlogo_width + 7) / 8;

    XGetWindowAttributes(dsp, pwin->w, &xgwa);
    ip->width = xgwa.width;
    ip->height = xgwa.height;
    ip->ncols = ip->width / xlogo_width;
    ip->nrows = ip->height / xlogo_height;
    ip->iconmode = (ip->ncols < 2 || ip->nrows < 2);
    if (ip->iconmode) {
	ip->xb = 0;
	ip->yb = 0;
	ip->iconcount = 1;	/* icon mode */
    } else {
	ip->xb = (ip->width - xlogo_width * ip->ncols) / 2;
	ip->yb = (ip->height - xlogo_height * ip->nrows) / 2;
	ip->iconcount = batchcount;
    }
    XSetForeground(dsp, pwin->gc, BlackPixelOfScreen(pwin->perscreen->screen));
    XFillRectangle(dsp, pwin->w, pwin->gc, 0, 0, ip->width, ip->height);
}
