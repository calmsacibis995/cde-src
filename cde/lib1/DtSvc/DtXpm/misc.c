/* $XConsortium: misc.c /main/cde1_maint/2 1995/09/06 02:19:58 lehors $ */
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
FUNC(_DtXpmAttributesSize, int, ());
FUNC(_DtXpmFreeAttributes, int, (XpmAttributes * attributes));
FUNC(_DtXpmFreeExtensions, int, (XpmExtension * extensions, int nextensions));

/* #include "xpmP.h" */
FUNC(_DtxpmSetAttributes, int, (xpmInternAttrib * attrib,
			     XpmAttributes * attributes));
FUNC(_DtxpmInitInternAttrib, int, (xpmInternAttrib * dtdata));
FUNC(_DtxpmFreeInternAttrib, int, (xpmInternAttrib * dtdata));
FUNC(_DtxpmFreeColorTable, int, (char ***colorTable, int ncolors));


int
_DtxpmFreeColorTable(colorTable, ncolors)
    char ***colorTable;
    int ncolors;
{
    return _XmxpmFreeColorTable(colorTable, ncolors);
}

int
_DtxpmInitInternAttrib(attrib)
    xpmInternAttrib *attrib;
{
    return _XmxpmInitInternAttrib(attrib);
}

int
_DtxpmFreeInternAttrib(attrib)
    xpmInternAttrib *attrib;
{
    return _XmxpmFreeInternAttrib(attrib);
}

int
_DtXpmFreeExtensions(extensions, nextensions)
    XpmExtension *extensions;
    int          nextensions;
{
    return _XmXpmFreeExtensions(extensions, nextensions);
}

int
_DtXpmFreeAttributes(attributes)
    XpmAttributes *attributes;
{
    return _XmXpmFreeAttributes(attributes);
}


int
_DtxpmSetAttributes(attrib, attributes)
    xpmInternAttrib *attrib;
    XpmAttributes *attributes;
{
    return _XmxpmSetAttributes(attrib, attributes);
}

/*
 * !!! Motif has this function as internal to the library, so if the
 * size changes there, we're not going to know about it and we are
 * going to be wrong with the return value.
 */
int
_DtXpmAttributesSize(void)
{
    return sizeof(XpmAttributes);
}

