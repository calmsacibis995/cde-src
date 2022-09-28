/* $XConsortium: hsbramp.c /main/cde1_maint/2 1995/08/29 19:58:25 gtsang $ */
/*
 */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*-
 * hsbramp.c - Create an HSB ramp.
 *
 * Copyright (c) 1991 by Patrick J. Naughton.
 *
 * See dtscreen.c for copying information.
 *
 * Revision History:
 * 29-Jul-90: renamed hsbramp.c from HSBmap.c
 *	      minor optimizations.
 * 01-Sep-88: Written.
 */

#include <sys/types.h>
#include <math.h>

void
hsb2rgb(H, S, B, r, g, b)
    double      H,
                S,
                B;
    u_char     *r,
               *g,
               *b;
{
    int         i;
    double      f;
    double      bb;
    u_char      p;
    u_char      q;
    u_char      t;

    H -= floor(H);		/* remove anything over 1 */
    H *= 6.0;
    i = floor(H);		/* 0..5 */
    f = H - (float) i;		/* f = fractional part of H */
    bb = 255.0 * B;
    p = (u_char) (bb * (1.0 - S));
    q = (u_char) (bb * (1.0 - (S * f)));
    t = (u_char) (bb * (1.0 - (S * (1.0 - f))));
    switch (i) {
    case 0:
	*r = (u_char) bb;
	*g = t;
	*b = p;
	break;
    case 1:
	*r = q;
	*g = (u_char) bb;
	*b = p;
	break;
    case 2:
	*r = p;
	*g = (u_char) bb;
	*b = t;
	break;
    case 3:
	*r = p;
	*g = q;
	*b = (u_char) bb;
	break;
    case 4:
	*r = t;
	*g = p;
	*b = (u_char) bb;
	break;
    case 5:
	*r = (u_char) bb;
	*g = p;
	*b = q;
	break;
    }
}


/*
 * Input is two points in HSB color space and a count
 * of how many discreet rgb space values the caller wants.
 *
 * Output is that many rgb triples which describe a linear
 * interpolate ramp between the two input colors.
 */

void
hsbramp(h1, s1, b1, h2, s2, b2, count, red, green, blue)
    double      h1,
                s1,
                b1,
                h2,
                s2,
                b2;
    int         count;

    u_char     *red,
               *green,
               *blue;
{
    double      dh;
    double      ds;
    double      db;

    dh = (h2 - h1) / count;
    ds = (s2 - s1) / count;
    db = (b2 - b1) / count;
    while (count--) {
	hsb2rgb(h1, s1, b1, red++, green++, blue++);
	h1 += dh;
	s1 += ds;
	b1 += db;
    }
}
