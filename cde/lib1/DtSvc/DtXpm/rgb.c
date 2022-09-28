/* $XConsortium: rgb.c /main/cde1_maint/2 1995/09/06 02:20:28 lehors $ */
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
FUNC(_DtxpmReadRgbNames, int, (char *rgb_fname, xpmRgbName * rgbn));
FUNC(_DtxpmGetRgbName, char *, (xpmRgbName * rgbn, int rgbn_max,
			     int red, int green, int blue));
FUNC(_DtxpmFreeRgbNames, void, (xpmRgbName * rgbn, int rgbn_max));

int
_DtxpmReadRgbNames(rgb_fname, rgbn)
    char *rgb_fname;
    xpmRgbName rgbn[];

{
    return _XmxpmReadRgbNames(rgb_fname, rgbn);
}

char *
_DtxpmGetRgbName(rgbn, rgbn_max, red, green, blue)
    xpmRgbName rgbn[];
    int rgbn_max;
    int red, green, blue;
{
    return _XmxpmGetRgbName(rgbn, rgbn_max, red, green, blue);
}

void
_DtxpmFreeRgbNames(rgbn, rgbn_max)
    xpmRgbName rgbn[];
    int rgbn_max;
{
    _XmxpmFreeRgbNames(rgbn, rgbn_max);
}
