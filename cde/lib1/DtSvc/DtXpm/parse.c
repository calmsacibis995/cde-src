/* $XConsortium: parse.c /main/cde1_maint/5 1995/10/08 22:23:03 pascale $ */
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
FUNC(_DtxpmParseData, int, (xpmData * data,
			     xpmInternAttrib * attrib_return,
			     XpmAttributes * attributes));

char **_DtxpmColorKeys = _XmxpmColorKeys;

int
_DtxpmParseData(data, attrib_return, attributes)
    xpmData *data;
    xpmInternAttrib *attrib_return;
    XpmAttributes *attributes;
{
    return _XmxpmParseData(data, attrib_return, attributes);
}
