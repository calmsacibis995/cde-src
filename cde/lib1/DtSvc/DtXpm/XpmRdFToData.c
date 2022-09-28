/* $XConsortium: XpmRdFToData.c /main/cde1_maint/1 1995/07/17 18:20:30 drk $ */
/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/* Copyright 1990,91 GROUPE BULL -- See license conditions in file COPYRIGHT */
/*****************************************************************************\
* XpmRdFToData.c:                                                             *
*                                                                             *
*  XPM library                                                                *
*  Parse an XPM file and create an array of strings corresponding to it.      *
*                                                                             *
*  Developed by Dan Greening dgreen@cs.ucla.edu / dgreen@sti.com              *
\*****************************************************************************/

#include "xpmP.h"

int
XpmReadFileToData(filename, data_return)
    char *filename;
    char ***data_return;
{
    xpmData mdata;
    char buf[BUFSIZ];
    int l, n = 0;
    XpmAttributes attributes;
    xpmInternAttrib attrib;
    int ErrorStatus;
    XGCValues gcv;
    GC gc;

    attributes.valuemask = XpmReturnPixels|XpmReturnInfos|XpmReturnExtensions;
    /*
     * initialize return values 
     */
    if (data_return) {
	*data_return = NULL;
    }

    if ((ErrorStatus = _DtxpmReadFile(filename, &mdata)) != XpmSuccess)
	return (ErrorStatus);
    _DtxpmInitInternAttrib(&attrib);
    /*
     * parse the header file 
     */
    mdata.Bos = '\0';
    mdata.Eos = '\n';
    mdata.Bcmt = mdata.Ecmt = NULL;
    _DtxpmNextWord(&mdata, buf, BUFSIZ);	/* skip the first word */
    l = _DtxpmNextWord(&mdata, buf, BUFSIZ);	/* then get the second word */
    if ((l == 3 && !strncmp("XPM", buf, 3)) ||
	(l == 4 && !strncmp("XPM2", buf, 4))) {
	if (l == 3)
	    n = 1;			/* handle XPM as XPM2 C */
	else {
	    l = _DtxpmNextWord(&mdata, buf, BUFSIZ);/* get the type key word */

	    /*
	     * get infos about this type 
	     */
	    while (_DtxpmDataTypes[n].type
		   && strncmp(_DtxpmDataTypes[n].type, buf, l))
		n++;
	}
	if (_DtxpmDataTypes[n].type) {
	    if (n == 0) {		/* natural type */
		mdata.Bcmt = _DtxpmDataTypes[n].Bcmt;
		mdata.Ecmt = _DtxpmDataTypes[n].Ecmt;
		_DtxpmNextString(&mdata);	/* skip the end of headerline */
		mdata.Bos = _DtxpmDataTypes[n].Bos;
	    } else {
		_DtxpmNextString(&mdata);	/* skip the end of headerline */
		mdata.Bcmt = _DtxpmDataTypes[n].Bcmt;
		mdata.Ecmt = _DtxpmDataTypes[n].Ecmt;
		mdata.Bos = _DtxpmDataTypes[n].Bos;
		mdata.Eos = '\0';
		_DtxpmNextString(&mdata);	/* skip the assignment line */
	    }
	    mdata.Eos = _DtxpmDataTypes[n].Eos;

	    ErrorStatus = _DtxpmParseData(&mdata, &attrib, &attributes);
	} else
	    ErrorStatus = XpmFileInvalid;
    } else
	ErrorStatus = XpmFileInvalid;

    if (ErrorStatus == XpmSuccess) {
	int i;
	
	/* maximum of allocated pixels will be the number of colors */
	attributes.pixels = (Pixel *) malloc(sizeof(Pixel) * attrib.ncolors);
	attrib.xcolors = (XColor*) malloc(sizeof(XColor) * attrib.ncolors);

	if (!attributes.pixels || !attrib.xcolors)
	    ErrorStatus = XpmNoMemory;
	else {
	    for (i = 0; i < attrib.ncolors; i++) {
		/* Fake colors */
		attrib.xcolors[i].pixel = attributes.pixels[i] = i + 1;
	    }
	    _DtxpmSetAttributes(&attrib, &attributes);
	    if (!(attrib.colorStrings =
		  (char**) malloc(attributes.ncolors * sizeof(char*))))
		ErrorStatus = XpmNoMemory;
	    else {
		attrib.ncolors = attributes.ncolors;
		attributes.mask_pixel = attrib.mask_pixel;
		for (i = 0; i < attributes.ncolors; i++)
		    attrib.colorStrings[i] = attributes.colorTable[i][0];
	    }
	}
    }
    if (ErrorStatus == XpmSuccess)
	ErrorStatus = _DtxpmCreateData(data_return, &attrib, &attributes);
    _DtXpmFreeAttributes(&attributes);
    _DtxpmFreeInternAttrib(&attrib);
    _DtXpmDataClose(&mdata);

    return (ErrorStatus);
}
