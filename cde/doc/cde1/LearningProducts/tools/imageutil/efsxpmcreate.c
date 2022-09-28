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
* create.c:                                                                   *
*                                                                             *
*  XPM library                                                                *
*  Create an X image and possibly its related shape mask                     *
*  from the given xpmInternAttrib.                                            *
*                                                                             *
*  Developed by Arnaud Le Hors                                                *
\*****************************************************************************/

#include "ilxpmP.h"
#ifdef VMS
#include "sys$library:ctype.h"
#else
#include <ctype.h>
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif



LFUNC(SetColor, int, (xpmInternAttrib *attrib, char *colorname,
		      unsigned int color_index, unsigned char * image_pixel,
		      unsigned char * mask_pixel, unsigned int *mask_pixel_index));


#ifdef NEED_STRCASECMP

LFUNC(strcasecmp, int, (char *s1, char *s2));

/*
 * in case strcasecmp is not provided by the system here is one
 * which does the trick
 */
static int
strcasecmp(s1, s2)
    register char *s1, *s2;
{
    register int c1, c2;

    while (*s1 && *s2) {
	c1 = isupper(*s1) ? tolower(*s1) : *s1;
	c2 = isupper(*s2) ? tolower(*s2) : *s2;
	if (c1 != c2)
	    return (1);
	s1++;
	s2++;
    }
    if (*s1 || *s2)
	return (1);
    return (0);
}

#endif


typedef struct  {
        char    *name;
        short   r,g,b;
        } NamedColor;

extern NamedColor NameLUT[];
extern NumNames;

/*
 * set the color pixel related to the given colorname,
 * return 0 if success, 1 otherwise.
 */

static int
SetColor( attrib, colorname, color_index,
	 image_pixel, mask_pixel, mask_pixel_index)
    xpmInternAttrib *attrib;
    char *colorname;
    unsigned int color_index;
    unsigned char *image_pixel, *mask_pixel;
    unsigned int *mask_pixel_index;
{
    short index;

    if (strcasecmp(colorname, TRANSPARENT_COLOR)) {
	if (_ilefsilxpmParseColor(attrib->pPalette, &attrib->nPaletteColors, colorname, &index))
	    return (1);
	*image_pixel = index;
	*mask_pixel = 1;
    } else {
	*image_pixel = 0;
	*mask_pixel = 0;
	*mask_pixel_index = color_index;/* store the color table index */
    }
    return (0);
}

/* function call in case of error, frees only localy allocated variables */
#undef RETURN
#define RETURN(status) \
  { if (image) ilDestroyObject((ilObject)image); \
    if (shapeimage) ilDestroyObject((ilObject)shapeimage); \
    if (image_pixels) free(image_pixels); \
    if (mask_pixels) free(mask_pixels); \
    return(status); }


_ilefsxpmCreateImage(
    ilObject      object,
    xpmInternAttrib *attrib,
    ilClientImage *image_return,
    ilClientImage *shapeimage_return,
    XpmAttributes *attributes)
{
    /* variables stored in the XpmAttributes structure */

    ilContext     context;
    XpmColorSymbol *colorsymbols;
    unsigned int numsymbols;

    /* variables to return */
    ilClientImage image = NULL;
    ilClientImage shapeimage = NULL;
    unsigned int mask_pixel;
    unsigned int ErrorStatus, ErrorStatus2;

    /* calculation variables */
    unsigned char *image_pixels = NULL;
    unsigned char *mask_pixels = NULL;
    char *colorname;
    unsigned int a, b, l;
    ilBool  pixel_defined;
    unsigned int key;

    context = object->context;

    /*
     * retrieve information from the XpmAttributes 
     */
    if (attributes && attributes->valuemask & XpmColorSymbols) {
	colorsymbols = attributes->colorsymbols;
	numsymbols = attributes->numsymbols;
    } else
	numsymbols = 0;

/*
    if (attributes && attributes->valuemask & XpmVisual)
	visual = attributes->visual;
    else
	visual = DefaultVisual(display, DefaultScreen(display));

    if (attributes && attributes->valuemask & XpmColormap)
	colormap = attributes->colormap;
    else
	colormap = DefaultColormap(display, DefaultScreen(display));

    if (attributes && attributes->valuemask & XpmDepth)
	depth = attributes->depth;
    else
	depth = DefaultDepth(display, DefaultScreen(display));
*/


    ErrorStatus = XpmSuccess;

    /*
     * alloc pixels index tables 
     */

/*    key = xpmVisualType(visual); */
    key = COLOR;

    image_pixels = (unsigned char *) malloc(attrib->ncolors);
    if (!image_pixels)
	RETURN(XpmNoMemory);

    mask_pixels = (unsigned char *) malloc(attrib->ncolors);
    if (!mask_pixels)
	RETURN(XpmNoMemory);

    mask_pixel = UNDEF_PIXEL;

    /*
     * get pixel colors, store them in index tables 
     */
    for (a = 0; a < attrib->ncolors; a++) {
	colorname = NULL;
	pixel_defined = FALSE;

	/*
	 * look for a defined symbol 
	 */
	if (numsymbols && attrib->colorTable[a][1]) {
	    for (l = 0; l < numsymbols; l++)
		if (!strcmp(colorsymbols[l].name, attrib->colorTable[a][1]))
		    break;
	    if (l != numsymbols) {
		if (colorsymbols[l].value)
		    colorname = colorsymbols[l].value;
		else
		    pixel_defined = TRUE;
	    }
	}
	if (!pixel_defined) {		/* pixel not given as symbol value */

	    if (colorname) {		/* colorname given as symbol value */
		if (!SetColor(attrib, colorname, a,
			   &image_pixels[a], &mask_pixels[a], &mask_pixel))
		    pixel_defined = TRUE;
		else
		    ErrorStatus = XpmColorError;
	    }
	    b = key;
	    while (!pixel_defined && b > 1) {
		if (attrib->colorTable[a][b]) {
		    if (!SetColor(attrib, attrib->colorTable[a][b],
				  a, &image_pixels[a], &mask_pixels[a],
				  &mask_pixel)) {
			pixel_defined = TRUE;
			break;
		    } else
			ErrorStatus = XpmColorError;
		}
		b--;
	    }

	    b = key + 1;
	    while (!pixel_defined && b < NKEYS + 1) {
		if (attrib->colorTable[a][b]) {
		    if (!SetColor(attrib, attrib->colorTable[a][b],
				  a, &image_pixels[a], &mask_pixels[a],
				  &mask_pixel)) {
			pixel_defined = TRUE;
			break;
		    } else
			ErrorStatus = XpmColorError;
		}
		b++;
	    }

	    if (!pixel_defined)
		RETURN(XpmColorFailed);

	}
/*
     else {
	    image_pixels[a] = colorsymbols[l].pixel;
	    mask_pixels[a] = 1;
	}
*/
    }

    /*
     * create the image 
     */
    if (image_return) {
        ilImageInfo    info;
        ilPtr   pixel_buffer;
        ilImageFormat  format;

        bzero ((char *) &info, sizeof (info));
        info.pDes = IL_DES_PALETTE;
        format    = *IL_FORMAT_BYTE;
        info.pFormat = &format;
        info.width   = attrib->width;
        info.height  = attrib->height;
        info.pPalette= attrib->pPalette;
        info.clientPalette = TRUE;
        info.clientPixels = TRUE;
        pixel_buffer = (unsigned char *)malloc (info.width * info.height);
        if (!pixel_buffer) RETURN(XpmNoMemory);
        info.plane[0].pPixels = pixel_buffer;
        info.plane[0].nBytesPerRow = info.width;

	/*
	 * set the image data 
	 *
	 */
        {
            register unsigned char *optr;
            register unsigned int *iptr;
            register int xy;
            optr = pixel_buffer;
            iptr = attrib->pixelindex;
            for (xy = 0; xy < info.width * info.height ; xy++)
                *optr++ = image_pixels[*iptr++];
        }
        image = ilCreateClientImage (context, &info, 0);
    }

    /*
     * create the shape mask image 
     */
    if (mask_pixel != UNDEF_PIXEL && shapeimage_return) {
        ilImageInfo    info;
        ilPtr   pixel_buffer;
        ilImageFormat  format;

        bzero ((char *) &info, sizeof (info));
        info.pDes = IL_DES_BITONAL;
        format    = *IL_FORMAT_BIT;
        info.pFormat = &format;
        info.width   = attrib->width;
        info.height  = attrib->height;
        info.pPalette= NULL;
        info.clientPixels = TRUE;
        info.plane[0].nBytesPerRow = ((( ( info.width + 7 ) >> 3) + 3) >>2) << 2 ;

        pixel_buffer = (unsigned char *) malloc (info.plane[0].nBytesPerRow * info.height);
        if (!pixel_buffer) RETURN(XpmNoMemory);
        info.plane[0].pPixels = pixel_buffer;

	/*
	 * set the shape image data 
	 *
	 */
        {
            register unsigned char *optr, byte;
            register unsigned int *iptr;
            register int bit;
            register int x, y, z;
            int      leftover;
            leftover = (info.plane[0].nBytesPerRow << 3) - info.width; 
            optr = pixel_buffer;
            iptr = attrib->pixelindex; 

	    for (y = 0; y < info.height; y++) {
		for (x = 0; x < info.width >> 3 ; x++) {
                    byte = 0;
                    for (bit = 0; bit < 8; bit++) 
                        byte = (byte << 1) + mask_pixels[*iptr++];
                    *optr++ = byte;
                   }
                if (leftover % 8) {
                    byte = 0;
                    for (bit = 0; bit < 8 - (leftover % 8); bit++)
                        byte = (byte << 1) + mask_pixels[*iptr++];
                    byte <<= (leftover % 8);
                    *optr++ = byte;
                }
                while (leftover > 7) {
                    *optr++ = 0;
                    leftover -= 8;
                }
            }
        }
        shapeimage = ilCreateClientImage (context, &info, 0);
    }
    free(mask_pixels);
    free(image_pixels);


    /*
     * return created images 
     */
    if (image_return)
	*image_return = image;

    if (shapeimage_return)
	*shapeimage_return = shapeimage;

    return (ErrorStatus);
}

