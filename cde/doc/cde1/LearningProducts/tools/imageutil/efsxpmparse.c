/**---------------------------------------------------------------------
***	
***    (c)Copyright 1991 Hewlett-Packard Co.
***    
***                             RESTRICTED RIGHTS LEGEND
***    Use, duplication, or disclosure by the U.S. Government is subject to
***    restrictions as set forth in sub-paragraph (c)(1)(ii) of the Rights in
***    Technical Data and Computer Software clause in DFARS 252.227-7013.
***                             Hewlett-Packard Company
***                             3000 Hanover Street
***                             Palo Alto, CA 94304 U.S.A.
***    Rights for non-DOD U.S. Government Departments and Agencies are as set
***    forth in FAR 52.227-19(c)(1,2).
***
***-------------------------------------------------------------------*/

/* Copyright 1990,91 GROUPE BULL -- See licence conditions in file COPYRIGHT */
/*****************************************************************************\
* parse.c:                                                                    *
*                                                                             *
*  XPM library                                                                *
*  Parse an XPM file or array and store the found informations                *
*  in an an xpmInternAttrib structure which is returned.                      *
*                                                                             *
*  Developed by Arnaud Le Hors                                                *
\*****************************************************************************/


#include "ilxpmP.h"

char *_ilefsxpmColorKeys[] =
{
 "s",					/* key #1: symbol */
 "m",					/* key #2: mono visual */
 "g4",					/* key #3: 4 grays visual */
 "g",					/* key #4: gray visual */
 "c",					/* key #5: color visual */
};


/* function call in case of error, frees only localy allocated variables */
#undef RETURN
#define RETURN(status) \
  { if (colorTable) _ilefsxpmFreeColorTable(colorTable, ncolors); \
    if (chars) free(chars); \
    if (pixelindex) free(pixelindex); \
    if (hints_cmt)  free(hints_cmt); \
    if (colors_cmt) free(colors_cmt); \
    if (pixels_cmt) free(pixels_cmt); \
    return(status); }

/*
 * This function parses an Xpm file or data and store the found informations
 * in an an xpmInternAttrib structure which is returned.
 */
int
_ilefsxpmParseData(data, attrib_return, attributes)
    xpmData *data;
    xpmInternAttrib *attrib_return;
    XpmAttributes *attributes;
{
    /* variables to return */
    unsigned int width, height;
    unsigned int ncolors = 0;
    unsigned int cpp;
    unsigned int x_hotspot, y_hotspot, hotspot = 0;
    char ***colorTable = NULL;
    unsigned int *pixelindex = NULL;
    char *hints_cmt = NULL;
    char *colors_cmt = NULL;
    char *pixels_cmt = NULL;

    /* calculation variables */
    unsigned int rncolors = 0;		/* read number of colors, it is
					 * different to ncolors to avoid
					 * problem when freeing the
					 * colorTable in case an error
					 * occurs while reading the hints
					 * line */
    unsigned int key;			/* color key */
    char *chars = NULL, buf[BUFSIZ];
    unsigned int *iptr;
    unsigned int a, b, x, y, l;

    /*
     * read hints: width, height, ncolors, chars_per_pixel 
     */
    if (!(_ilefsxpmNextUI(data, &width) && _ilefsxpmNextUI(data, &height)
	  && _ilefsxpmNextUI(data, &rncolors) && _ilefsxpmNextUI(data, &cpp)))
	RETURN(XpmFileInvalid);

    ncolors = rncolors;

    /*
     * read hotspot coordinates if any 
     */
    hotspot = _ilefsxpmNextUI(data, &x_hotspot) && _ilefsxpmNextUI(data, &y_hotspot);

    /*
     * store the hints comment line 
     */
    if (attributes && (attributes->valuemask & XpmReturnInfos))
	_ilefsxpmGetCmt(data, &hints_cmt);

    /*
     * read colors 
     */
    colorTable = (char ***) calloc(ncolors, sizeof(char **));
    if (!colorTable)
	RETURN(XpmNoMemory);

    for (a = 0; a < ncolors; a++) {
	_ilefsxpmNextString(data);		/* skip the line */
	colorTable[a] = (char **) calloc((NKEYS + 1), sizeof(char *));
	if (!colorTable[a])
	    RETURN(XpmNoMemory);

	/*
	 * read pixel value 
	 */
	*colorTable[a] = (char *) malloc(cpp);
	if (!*colorTable[a])
	    RETURN(XpmNoMemory);
	for (b = 0; b < cpp; b++)
	    colorTable[a][0][b] = _ilefsxpmGetC(data);

	/*
	 * read color keys and values 
	 */
	while (l = _ilefsxpmNextWord(data, buf)) {
	    for (key = 1; key < NKEYS + 1; key++)
		if (!strncmp(_ilefsxpmColorKeys[key - 1], buf, l))
		    break;
	    if (key <= NKEYS && (l = _ilefsxpmNextWord(data, buf))) {
		colorTable[a][key] = (char *) malloc(l + 1);
		if (!colorTable[a][key])
		    RETURN(XpmNoMemory);
		strncpy(colorTable[a][key], buf, l);
		colorTable[a][key][l] = '\0';
	    } else
		RETURN(XpmFileInvalid);	/* key without value */
	}
    }

    /*
     * store the colors comment line 
     */
    if (attributes && (attributes->valuemask & XpmReturnInfos))
	_ilefsxpmGetCmt(data, &colors_cmt);

    /*
     * read pixels and index them on color number 
     */
    pixelindex =
	(unsigned int *) malloc(sizeof(unsigned int) * width * height);
    if (!pixelindex)
	RETURN(XpmNoMemory);

    iptr = pixelindex;

    chars = (char *) malloc(cpp);
    if (!chars)
	RETURN(XpmNoMemory);

    for (y = 0; y < height; y++) {
	_ilefsxpmNextString(data);
	for (x = 0; x < width; x++, iptr++) {
	    for (a = 0; a < cpp; a++)
		chars[a] = _ilefsxpmGetC(data);
	    for (a = 0; a < ncolors; a++)
		if (!strncmp(colorTable[a][0], chars, cpp))
		    break;
	    if (a == ncolors)
		RETURN(XpmFileInvalid);	/* no color matches */
	    *iptr = a;
	}
    }

    /*
     * store the pixels comment line 
     */
    if (attributes && (attributes->valuemask & XpmReturnInfos))
	_ilefsxpmGetCmt(data, &pixels_cmt);

    free(chars);

    /*
     * store found informations in the xpmInternAttrib structure 
     */
    attrib_return->width = width;
    attrib_return->height = height;
    attrib_return->cpp = cpp;
    attrib_return->ncolors = ncolors;
    attrib_return->colorTable = colorTable;
    attrib_return->pixelindex = pixelindex;

    if (attributes) {
	if (attributes->valuemask & XpmReturnInfos) {
	    attributes->hints_cmt = hints_cmt;
	    attributes->colors_cmt = colors_cmt;
	    attributes->pixels_cmt = pixels_cmt;
	}
	if (hotspot) {
	    attributes->x_hotspot = x_hotspot;
	    attributes->y_hotspot = y_hotspot;
	    attributes->valuemask |= XpmHotspot;
	}
    }
    return (XpmSuccess);
}
