/* $XConsortium: hopalong.c /main/cde1_maint/2 1995/08/29 19:58:07 gtsang $ */
/*
 */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*-
 * hopalong.c - Real Plane Fractals for dtscreen, the X Window System lockscreen.
 *
 * Copyright (c) 1991 by Patrick J. Naughton.
 *
 * See dtscreen.c for copying information.
 *
 * Revision History:
 * 29-Oct-90: fix bad (int) cast.
 * 29-Jul-90: support for multiple screens.
 * 08-Jul-90: new timing and colors and new algorithm for fractals.
 * 15-Dec-89: Fix for proper skipping of {White,Black}Pixel() in colors.
 * 08-Oct-89: Fixed long standing typo bug in RandomInitHop();
 *	      Fixed bug in memory allocation in inithop();
 *	      Moved seconds() to an extern.
 *	      Got rid of the % mod since .mod is slow on a sparc.
 * 20-Sep-89: Lint.
 * 31-Aug-88: Forked from dtscreen.c for modularity.
 * 23-Mar-88: Coded HOPALONG routines from Scientific American Sept. 86 p. 14.
 */

#include "dtscreen.h"
#include <math.h>

typedef struct {
    int         centerx;
    int         centery;	/* center of the screen */
    double      a;
    double      b;
    double      c;
    double      i;
    double      j;		/* hopalong parameters */
    int         inc;
    int         pix;
    long        startTime;
}           hopstruct;

static XPoint *pointBuffer = 0;	/* pointer for XDrawPoints */

#define TIMEOUT 30

void
inithop(pwin)
    perwindow *pwin;
{
    double      range;
    XWindowAttributes xgwa;
    hopstruct  *hp;

    if (pwin->data) free(pwin->data);
    pwin->data = (void *)malloc(sizeof(hopstruct));
    memset(pwin->data, '\0', sizeof(hopstruct));
    hp = (hopstruct *)pwin->data;
    XGetWindowAttributes(dsp, pwin->w, &xgwa);
    hp->centerx = xgwa.width / 2;
    hp->centery = xgwa.height / 2;
    range = sqrt((double) hp->centerx * hp->centerx +
		 (double) hp->centery * hp->centery) /
	(10.0 + random() % 10);

    hp->pix = 0;
    hp->inc = (int) ((random() / MAXRAND) * 200) - 100;
    hp->a = (random() / MAXRAND) * range - range / 2.0;
    hp->b = (random() / MAXRAND) * range - range / 2.0;
    hp->c = (random() / MAXRAND) * range - range / 2.0;
    if (!(random() % 2))
	hp->c = 0.0;

    hp->i = hp->j = 0.0;

    if (!pointBuffer)
	pointBuffer = (XPoint *) malloc(batchcount * sizeof(XPoint));

    XSetForeground(dsp, pwin->gc, BlackPixelOfScreen(pwin->perscreen->screen));
    XFillRectangle(dsp, pwin->w, pwin->gc, 0, 0,
		   hp->centerx * 2, hp->centery * 2);
    XSetForeground(dsp, pwin->gc, WhitePixelOfScreen(pwin->perscreen->screen));
    hp->startTime = seconds();
}


void
drawhop(pwin)
    perwindow *pwin;
{
    double      oldj;
    int         k = batchcount;
    XPoint     *xp = pointBuffer;
    hopstruct  *hp = (hopstruct *)pwin->data;

    hp->inc++;
    if (!mono && pwin->perscreen->npixels > 2) {
	XSetForeground(dsp, pwin->gc, pwin->perscreen->pixels[hp->pix]);
	if (++hp->pix >= pwin->perscreen->npixels)
	    hp->pix = 0;
    }
    while (k--) {
	oldj = hp->j;
	hp->j = hp->a - hp->i;
	hp->i = oldj + (hp->i < 0
			? sqrt(fabs(hp->b * (hp->i + hp->inc) - hp->c))
			: -sqrt(fabs(hp->b * (hp->i + hp->inc) - hp->c)));
	xp->x = hp->centerx + (int) (hp->i + hp->j);
	xp->y = hp->centery - (int) (hp->i - hp->j);
	xp++;
    }
    XDrawPoints(dsp, pwin->w, pwin->gc,
		pointBuffer, batchcount, CoordModeOrigin);
    if (seconds() - hp->startTime > TIMEOUT)
	inithop(pwin);
}
