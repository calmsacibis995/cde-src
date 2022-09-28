/************************************<+>*************************************
 ****************************************************************************
 **
 **  File:	Graphics.c
 **
 **  Project:    CDE Help System
 **
 **  Description: This code processes a graphic (X11 bitmap, X11 pixmap,
 **               X11 XWD file, and TIFF). It will degrade graphics to
 **		  match the type of monitor the image is displayed upon.
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 ****************************************************************************
 ************************************<+>*************************************/
/******************************************************************************
 **  The GreyScale and Perform_Dither routine were take and modified from
 **  the g-job 'xgedit' created by Mark F. Cook. The following is the
 **  disclaimer from the dither.c source code
 ******************************************************************************
 **  Program:		xgedit.c
 **
 **  Description:	X11-based graphics editor
 **
 **  File:		dither.c, containing the following
 **                     subroutines/functions:
 **                       Dither_Image()
 **                       Perform_Dither()
 **
 **  Copyright 1988 by Mark F. Cook and Hewlett-Packard, Corvallis
 **  Workstation Operation.  All rights reserved.
 **
 **  Permission to use, copy, and modify this software is granted, provided
 **  that this copyright appears in all copies and that both this copyright
 **  and permission notice appear in all supporting documentation, and that
 **  the name of Mark F. Cook and/or Hewlett-Packard not be used in advertising
 **  without specific, written prior permission.  Neither Mark F. Cook or
 **  Hewlett-Packard make any representations about the suitibility of this
 **  software for any purpose.  It is provided "as is" without express or
 **  implied warranty.
 **
 ******************************************************************************
 **  CHANGE LOG:
 **
 **     1.00.00 - 1/11/89 - Start of Rev. number.
 **
 ** @@@ 4.05.00 - 12/07/89 - Created Dither_Image() and Perform_Dither() to
 ** @@@           allow conversion of color images into a dithered black &
 ** @@@           white image suitable for output on a laser printer.
 **
 ******************************************************************************/

/*
static char rcs_id[]="$XConsortium: Graphics.c /main/cde1_maint/2 1995/10/08 17:19:26 pascale $";
*/

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <X11/Xlib.h>
#include <X11/XWDFile.h>
#ifndef	STUB
#include "il.h"
#include "ilfile.h"
#include "ilX.h"
#endif
#include <Xm/Xm.h>
#include <Dt/xpm.h> /* for Xpm */

#include "Canvas.h"
#include "DisplayAreaP.h"
#include "bufioI.h"
#include "AccessI.h"
#include "GraphicsI.h"
#include "StringFuncsI.h"


/*****************************************************************************/
/*		      Private data					     */
/*****************************************************************************/
#define GR_SUCCESS            0       /* everything okay */
#define GR_DRAWABLE_ERR       1       /* couldn't get drawable attr/geom */
#define GR_FILE_ERR           2       /* file open/read/write problem */
#define GR_REQUEST_ERR        3       /* bad placement or size */
#define GR_ALLOC_ERR          4       /* memory allocation failure */
#define GR_HEADER_ERR         5       /* file header version/size problem */

#define	MAX_GREY_COLORS		8

enum	ColorType	{COLOR_INVALID, BITONAL, GREY_SCALE, COLOR_SCALE};

static	enum ColorType	ForceColor = COLOR_INVALID;
static	short		GreyAllocated = False;
static	Pixel		GreyScalePixels[MAX_GREY_COLORS];
static	char		*GreyScaleColors[MAX_GREY_COLORS] =
			{
				"#212121212121",
				"#424242424242",
				"#636363636363",
				"#737373737373",
				"#949494949494",
				"#adadadadadad",
				"#bdbdbdbdbdbd",
				"#dededededede",
			};

#define Missing_bm_width 75
#define Missing_bm_height 47
static unsigned char Missing_bm_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0x7f, 0x00, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00,
   0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x18, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x18, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x18, 0x04, 0x01, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xc0, 0x00, 0x18, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xc0, 0x00, 0x18, 0x8c, 0x21, 0x00, 0x00, 0x20, 0x00, 0x00, 0xc0, 0x00,
   0x18, 0x54, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x18, 0x24,
   0x31, 0xf0, 0xf0, 0x30, 0xe8, 0xf0, 0xc1, 0x00, 0x18, 0x04, 0x21, 0x08,
   0x08, 0x20, 0x18, 0x09, 0xc1, 0x00, 0x18, 0x04, 0x21, 0xf0, 0xf0, 0x20,
   0x08, 0x09, 0xc1, 0x00, 0x18, 0x04, 0x21, 0x00, 0x01, 0x21, 0x08, 0x09,
   0xc1, 0x00, 0x18, 0x04, 0x21, 0x08, 0x09, 0x21, 0x08, 0x09, 0xc1, 0x00,
   0x18, 0x04, 0x71, 0xf0, 0xf0, 0x70, 0x08, 0xf1, 0xc1, 0x00, 0x18, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc1, 0x00, 0x18, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xc1, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0xf0, 0xc0, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xc0, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00,
   0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x18, 0xf0,
   0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0xc0, 0x00, 0x18, 0x08, 0x01, 0x00,
   0x00, 0x08, 0x00, 0x00, 0xc0, 0x00, 0x18, 0x08, 0x00, 0x00, 0x00, 0x08,
   0x20, 0x00, 0xc0, 0x00, 0x18, 0x08, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00,
   0xc0, 0x00, 0x18, 0x08, 0xd8, 0xf0, 0xe8, 0xf8, 0x30, 0xf0, 0xc0, 0x00,
   0x18, 0xc8, 0x31, 0x01, 0x19, 0x09, 0x21, 0x08, 0xc1, 0x00, 0x18, 0x08,
   0x11, 0xf0, 0x09, 0x09, 0x21, 0x08, 0xc0, 0x00, 0x18, 0x08, 0x11, 0x08,
   0x09, 0x09, 0x21, 0x08, 0xc0, 0x00, 0x18, 0x08, 0x11, 0x08, 0x09, 0x09,
   0x21, 0x08, 0xc1, 0x00, 0x18, 0xf0, 0x10, 0xf0, 0xf9, 0x08, 0x71, 0xf0,
   0xc0, 0x00, 0x18, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0xc0, 0x00,
   0x18, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x18, 0x00,
   0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x18, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xc0, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xc0, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00,
   0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xf8, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00};

/*****************************************************************************/
/*		      Private declarations				     */
/*****************************************************************************/
#ifdef _NO_PROTO
static int		Do_Direct();
static int		Do_Pseudo();
static int		GreyScale ();
static unsigned int	Image_Size();
static void		Perform_Dither();
static	Pixmap		processBitmap() ;
#ifndef	STUB
static	Pixmap		processTiff() ;
#endif
static	Pixmap		processXwd ();
static void		_swaplong ();
static void		_swapshort ();
static	int		XwdFileToPixmap ();
#else
static int		Do_Direct(
				Display *dpy,
				int      screen,
				XWDFileHeader *header,
				Colormap colormap,
				int ncolors,
				XColor *colors,
				enum ColorType	force,
				XImage *in_image,
				XImage *out_image,
				unsigned long **ret_colors,
				int  *ret_number );
static int		Do_Pseudo(
				Display *dpy,
				int	 screen,
				Colormap colormap,
				int ncolors,
				XColor *colors,
				enum ColorType	force,
				XImage *in_image,
				XImage *out_image,
				unsigned long **ret_colors,
				int  *ret_number );
static int		GreyScale (
				Display   *dpy,
				int	   screen,
				Colormap   cmap,
				XImage    *in_image,
				XImage    *out_image,
				XColor    *colors,
				int        ncolors,
				enum ColorType	force,
				int        rshift,
				int        gshift,
				int        bshift,
				Pixel      rmask,
				Pixel      gmask,
				Pixel      bmask );
static unsigned int	Image_Size(
				XImage *image );
static void		Perform_Dither(
				Display *dpy,
				int      screen,
				XImage  *image,
				int     *greyscale );
static	Pixmap		processBitmap(
				Display *dpy,
				int      depth,
				Drawable  drawable,
				GC        gc,
				char    *nameStr,
				unsigned int  *pixmapWidth,
				unsigned int  *pixmapHeight ) ;
#ifndef	STUB
static	Pixmap		processTiff(
				Display *dpy,
				int	 depth,
				Drawable  drawable,
				Colormap  colormap,
				Visual	 *visual,
				GC        gc,
				ilXWC    *tiff_xwc,
				char    *nameStr,
				unsigned int  *pixmapWidth,
				unsigned int  *pixmapHeight ) ;
#endif
static	Pixmap		processXwd(
				Display       *dpy,
				int	       screen,
				int	       depth,
				Drawable       drawable,
				Colormap       colormap,
				Visual	      *visual,
				GC             gc,
        			char   *nameStr,
				unsigned int    *pixmapWidth,
				unsigned int    *pixmapHeight,
				unsigned long **ret_colors,
				int *ret_number );
static void		_swaplong (
				register char *bp,
				register unsigned n );
static void		_swapshort (
				register char *bp,
				register unsigned n );
static	int		XwdFileToPixmap (
				Display  *dpy,
				int	  screen,
				int	  depth,
				Pixmap    pixmap,
				Colormap  colormap,
				Visual	 *visual,
				GC        gc,
				int       src_x,
				int       src_y,
				int       dst_x,
				int       dst_y,
				int       width,
				int       height,
				char     *file_name,
				unsigned long **ret_colors,
				int      *ret_number );
#endif /* _NO_PROTO */

/***************************************************************************** 
 *         Private Routines
 *****************************************************************************/
/***************************************************************************** 
 * taken straight out of xwud and modified.
 *****************************************************************************/
/* Copyright 1985, 1986, 1988 Massachusetts Institute of Technology */

static void
#ifdef _NO_PROTO
_swapshort (bp, n)
    register char *bp;
    register unsigned n;
#else
_swapshort (
    register char *bp,
    register unsigned n )
#endif /* _NO_PROTO */
{
    register char c;
    register char *ep = bp + n;

    while (bp < ep) {
	c = *bp;
	*bp = *(bp + 1);
	bp++;
	*bp++ = c;
    }
}

static void
#ifdef _NO_PROTO
_swaplong (bp, n)
    register char *bp;
    register unsigned n;
#else
_swaplong (
    register char *bp,
    register unsigned n )
#endif /* _NO_PROTO */
{
    register char c;
    register char *ep = bp + n;
    register char *sp;

    while (bp < ep) {
	sp = bp + 3;
	c = *sp;
	*sp = *bp;
	*bp++ = c;
	sp = bp + 1;
	c = *sp;
	*sp = *bp;
	*bp++ = c;
	bp += 2;
    }
}

/*****************************************************************************
 * Function: GreyScale
 *
 * Turn a color image into a 8 grey color image
 * If it can't create a grey scale image, GreyScale will call
 * Perform_Dither to create a bi-tonal image.
 *
 *****************************************************************************
 * The GreyScale code was ported from xgedit and changed to fit our needs
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
GreyScale (dpy, screen, cmap, in_image, out_image, colors, ncolors, force, rshift, gshift, bshift, rmask, gmask, bmask)
    Display   *dpy;
    int	       screen;
    Colormap   cmap;
    XImage    *in_image;
    XImage    *out_image;
    XColor    *colors;
    int        ncolors;
    enum ColorType	force;
    int        rshift;
    int        gshift;
    int        bshift;
    Pixel      rmask;
    Pixel      gmask;
    Pixel      bmask;
#else
GreyScale (
    Display   *dpy,
    int	       screen,
    Colormap   cmap,
    XImage    *in_image,
    XImage    *out_image,
    XColor    *colors,
    int        ncolors,
    enum ColorType	force,
    int        rshift,
    int        gshift,
    int        bshift,
    Pixel      rmask,
    Pixel      gmask,
    Pixel      bmask )
#endif /* _NO_PROTO */
{
  int i, j, x;
  int     inc;
  int     count;
  int	  width = in_image->width;
  int	  height = in_image->height;
  int     value;
  int    *grey_scale;
  Pixel   valueArray[256];
  Pixel   n;
  XColor  ret_color;

/*--- allocate an array big enough to map each pixel in the image  ---*/
/*--- into a corresponding greyscale value (in the range [0-255]). ---*/

  grey_scale = (int *) calloc(width*height, sizeof(int));
  if (grey_scale == NULL) {
     return GR_ALLOC_ERR;
   }

/*---          < ESTABLISH THE GREYSCALE IMAGE >            ---*/
/*--- The NTSC formula for converting an RGB value into the ---*/
/*--- corresponding greyscale value is:                     ---*/
/*---      luminosity = .299 red + .587 green + .114 blue   ---*/

  /*
   * zero the flag array
   */
  for (i = 0; i < 256; i++)
      valueArray [i] = 0;

  for (j=0, x = 0; j<height; j++)
    for (i=0; i<width; i++, x++) {
      n = XGetPixel(in_image, i, j);
      if (rshift)
	{
	  ret_color.red   = (n >> rshift) & rmask;
	  ret_color.green = (n >> gshift) & gmask;
	  ret_color.blue  = (n >> bshift) & bmask;
	  if (ncolors)
	    {
	      ret_color.red   = colors[ret_color.red  ].red;
	      ret_color.green = colors[ret_color.green].green;
	      ret_color.blue  = colors[ret_color.blue ].blue;
	    }
	  else
	    {
	      ret_color.red   = (((Pixel) ret_color.red  ) * 65535) / rmask;
	      ret_color.green = (((Pixel) ret_color.green) * 65535) / gmask;
	      ret_color.blue  = (((Pixel) ret_color.blue ) * 65535) / bmask;
	    }

          value = (((int)(ret_color.red*299) + (int)(ret_color.green*587) +
				(int)(ret_color.blue*114)) / 1000) >> 8;
	}
      else
          value = (((int)(colors[n].red*299) + (int)(colors[n].green*587) +
				(int)(colors[n].blue*114)) / 1000) >> 8;
      grey_scale[x] = value;
      valueArray[value]++;
     } /* for(i...) */

  /*
   * Find out if we can/have allocate the pre-defined grey colors.
   */
  if (!GreyAllocated && force != BITONAL)
    {
      for (i = 0; !GreyAllocated && i < MAX_GREY_COLORS; i++)
	{
	  if (!XParseColor (dpy, cmap, GreyScaleColors[i], &ret_color) ||
		!XAllocColor (dpy, cmap, &ret_color))
	    {
	      /*
	       * Have a problem allocating one of the pre-defined
	       * grey colors. Free the already allocated pixels
	       * and set the flag.
	       */
	      if (i)
	          XFreeColors (dpy, cmap, GreyScalePixels, i, 0);
	      GreyAllocated = -1;
	    }
	  else
	      GreyScalePixels[i] = ret_color.pixel;
	}
      if (!GreyAllocated)
          GreyAllocated = True;
    }

  /*
   * Had a problem allocating the pre-defined grey colors.
   * Try to dither into black and white.
   */
  if (force == BITONAL || GreyAllocated == -1)
    {
      Perform_Dither (dpy, screen, out_image, grey_scale);
      free(grey_scale);
      return GR_SUCCESS;
    }

  /*
   * Find out how many grey scale colors there are.
   */
  for (count = 0, i = 0; i < 256; i++)
      if (valueArray[i])
	  count++;

  /*
   * If we have less than maximum grey colors we want to spread the load
   * between the colors
   */
  if (count < MAX_GREY_COLORS)
      inc = count;
  else
      inc = MAX_GREY_COLORS;

  /*
   * This is rather esoteric code.
   * The line
   *	valueArray[n] = GreyScalePixels[i * MAX_GREY_COLORS / inc]
   *
   * causes the colors to be evenly distributed if the total number
   * of calculated grey shades is less than the number of pre-defined
   * grey colors.
   *
   * The if stmt
   *	if (j >= count / (MAX_GREY_COLORS - i))
   *
   * uniformly reduces the calculated grey shades into the pre-defined
   * grey colors, if the total number of grey shades is greater than
   * the number of pre-defined grey colors.
   *
   * If after reading the following code, the reader still doesn't
   * understand it, pick a couple of numbers between 1-255 for 'count'
   * and walk each of them thru the 'for' loop. Hopefully it will help.
   * (Suggestion - pick a large number for one value and pick
   * a number less than 8 for the other value).
   */
  for (i = 0, n = 0, j = 0; n < 256 && count && i < MAX_GREY_COLORS; n++)
    {
      if (valueArray[n])
	{
	  valueArray[n] = GreyScalePixels[i * MAX_GREY_COLORS / inc];
	  j++;
	  if (j >= count / (MAX_GREY_COLORS - i))
	    {
	      count -= j;
	      i++;
	      j = 0;
	    }
	}
    }

  /*
   * Now replace the image pixels with the grey pixels
   */
  for (j = 0, x = 0; j < height; j++)
    for (i = 0; i < width; i++, x++) {
      XPutPixel (out_image, i, j, valueArray[grey_scale[x]]);
     } /* for(i...) */

  free(grey_scale);

  return GR_SUCCESS;
}

/***************************************************************************
 *                                                                         *
 * Routine:   Perform_Dither                                               *
 *                                                                         *
 * Purpose:   Given a color XImage and a greyscale representation of it,   *
 *            for each pixel in the image, determine whether it should be  *
 *            coverted to Black or White base on the weighted average of   *
 *            the greyscale value of it and the pixels surrounding it.     *
 *            Be sure to do bounds checking for pixels that are at the     *
 *            edge of the image.                                           *
 *                                                                         *
 *            The dithering is done using the Floyd-Steinberg error        *
 *            diffusion algorithm, which incorporates a Stucki error       *
 *            filter (specifics can be found in Chapter 8, "Dithering      *
 *            with Blue Noise", of the book "Digital Halftoning" by        *
 *            Robert Ulichney (MIT Press, 1988).                           *
 *                                                                         *
 *X11***********************************************************************/
static	void
#ifdef _NO_PROTO
Perform_Dither(dpy, screen, image, greyscale)
    Display *dpy;
    int      screen;
    XImage  *image;
    int   *greyscale;
#else
Perform_Dither(
    Display *dpy,
    int      screen,
    XImage  *image,
    int   *greyscale )
#endif /* _NO_PROTO */
{
  int i, j, width, height;
  int result, error, max_lum;
  Pixel    blackPixel = XBlackPixel (dpy, screen);
  Pixel    whitePixel = XWhitePixel (dpy, screen);

  width  = image->width;
  height = image->height;
  max_lum = 256;

  for (j=0; j<height; j++) /* rows */
    for (i=0; i<width; i++) { /* columns */
      if (greyscale[(j*width)+i] < (max_lum/2))
	result = 0;
      else
        result = max_lum - 1;
      error = greyscale[(j*width)+i] - result;
      if (i+1 < width)			/*--- G [j] [i+1] ---*/
        greyscale[(j*width)+i+1] += (error * 8)/42;
      if (i+2 < width)			/*--- G [j] [i+2] ---*/
        greyscale[(j*width)+i+2] += (error * 4)/42;

      if (j+1 < height) {
        if (i-2 >= 0)			/*--- G [j+1] [i-2] ---*/
          greyscale[((j+1)*width)+i-2] += (error * 2)/42;
        if (i-1 >= 0)			/*--- G [j+1] [i-1] ---*/
          greyscale[((j+1)*width)+i-1] += (error * 4)/42;
	/*--- G [j+1] [i] ---*/
        greyscale[((j+1)*width)+i] += (error * 8)/42;
        if (i+1 < width)		/*--- G [j+1] [i+1] ---*/
          greyscale[((j+1)*width)+i+1] += (error * 4)/42;
        if (i+2 < width)		/*--- G [j+1] [i+2] ---*/
          greyscale[((j+1)*width)+i+2] += (error * 2)/42;
       }

      if (j+2 < height) {
        if (i-2 >= 0)			/*--- G [j+2] [i-2] ---*/
          greyscale[((j+2)*width)+i-2] += error/42;
        if (i-1 >= 0)			/*--- G [j+2] [i-1] ---*/
          greyscale[((j+2)*width)+i-1] += (error * 2)/42;
	/*--- G [j+2] [i] ---*/
        greyscale[((j+2)*width)+i] += (error * 4)/42;
        if (i+1 < width)		/*--- G [j+2] [i+1] ---*/
          greyscale[((j+2)*width)+i+1] += (error * 2)/42;
        if (i+2 < width)		/*--- G [j+2] [i+2] ---*/
          greyscale[((j+2)*width)+i+2] += error/42;
       }

      if (result)
	XPutPixel(image, i, j, whitePixel);
      else
	XPutPixel(image, i, j, blackPixel);
     } /* for(i...) */
}


static int
#ifdef _NO_PROTO
Do_Pseudo(dpy, screen, colormap, ncolors, colors, force, in_image, out_image, ret_colors, ret_number)
    Display *dpy;
    int	     screen;
    Colormap colormap;
    int ncolors;
    XColor *colors;
    enum ColorType	force;
    XImage *in_image;
    XImage *out_image;
    unsigned long **ret_colors;
    int  *ret_number;
#else
Do_Pseudo(
    Display *dpy,
    int	     screen,
    Colormap colormap,
    int ncolors,
    XColor *colors,
    enum ColorType	force,
    XImage *in_image,
    XImage *out_image,
    unsigned long **ret_colors,
    int  *ret_number )
#endif /* _NO_PROTO */
{
    register int i, x, y, colorCount = 0;
    register XColor *color;
    int      result = 0;
    Pixel    pixel;

    for (i = 0; i < ncolors; i++)
	colors[i].flags = 0;

    /*
     * beware 'result'.
     * It is set to one upon entering this routine.
     * The only way it can be modified is by the call to XAllocColor.
     */
    if (force == COLOR_SCALE)
	result = 1;

    for (y = 0; result && y < in_image->height; y++)
      {
        for (x = 0; result && x < in_image->width; x++)
	  {
	    pixel = XGetPixel(in_image, x, y);
	    color = &colors[pixel];
	    if (!color->flags)
	      {
		color->flags = DoRed | DoGreen | DoBlue;
		result = XAllocColor(dpy, colormap, color);
		if (!result)
		    color->flags = 0;
		else
		    colorCount++;
	      }
	    if (result)
	        XPutPixel(out_image, x, y, color->pixel);
	  }
      }

    /*
     * If result == 0, a call to XAllocColor failed
     * Try to grey scale the image.
     */
    if (!result)
      {
	if (colorCount)
	  {
	    for (i = 0; i < ncolors; i++)
	      {
	        if (colors[i].flags)
	          {
	            XFreeColors (dpy, colormap, &(colors[i].pixel), 1, 0);
		    colors[i].flags = 0;
	          }
	      }
	  }
	result = GreyScale (dpy, screen, colormap, in_image, out_image, colors,
			ncolors, force, 0, 0, 0, 0, 0, 0);
      }
    else if (colorCount)
      {
	result = GR_SUCCESS;
        *ret_colors = (unsigned long *) malloc (
					sizeof (unsigned long) * colorCount);
	if (*ret_colors == NULL)
	  {
	    colorCount = 0;
	    result = GR_ALLOC_ERR;
	  }

        for (i = 0, x = 0; i < ncolors && x < colorCount; i++)
	    if (colors[i].flags)
	        (*ret_colors)[x++] = colors[i].pixel;

	*ret_number = colorCount;
      }

    /*
     * result was set to a XHPIF value via the 'else' stmt or
     * returned from GreyScale routine.
     */
    return result;
}

static int
#ifdef _NO_PROTO
Do_Direct(dpy, screen, header, colormap, ncolors, colors, force, in_image, out_image, ret_colors, ret_number)
    Display *dpy;
    int      screen;
    XWDFileHeader *header;
    Colormap colormap;
    int ncolors;
    XColor *colors;
    enum ColorType	force;
    XImage *in_image;
    XImage *out_image;
    unsigned long **ret_colors;
    int  *ret_number;
#else
Do_Direct(
    Display *dpy,
    int      screen,
    XWDFileHeader *header,
    Colormap colormap,
    int ncolors,
    XColor *colors,
    enum ColorType	force,
    XImage *in_image,
    XImage *out_image,
    unsigned long **ret_colors,
    int  *ret_number )
#endif /* _NO_PROTO */
{
    register int x, y;
    XColor color;
    unsigned long rmask, gmask, bmask;
    int   rshift = 0, gshift = 0, bshift = 0;
    int   i;
    int   result;
    int   pixMax = 256;
    int   pixI   = 0;
    Pixel pix;
    Pixel *oldPixels;
    Pixel *newPixels;

    oldPixels = (Pixel *) malloc (sizeof (Pixel) * pixMax);
    newPixels = (Pixel *) malloc (sizeof (Pixel) * pixMax);

    if (oldPixels == NULL || newPixels == NULL)
      {
	if (oldPixels)
	    free (oldPixels);
	if (newPixels)
	    free (newPixels);

	return GR_ALLOC_ERR;
      }

    rmask = header->red_mask;
    while (!(rmask & 1)) {
	rmask >>= 1;
	rshift++;
    }
    gmask = header->green_mask;
    while (!(gmask & 1)) {
	gmask >>= 1;
	gshift++;
    }
    bmask = header->blue_mask;
    while (!(bmask & 1)) {
	bmask >>= 1;
	bshift++;
    }
    if (in_image->depth <= 12)
	pix = 1 << in_image->depth;

    if (force == COLOR_SCALE)
        color.flags = DoRed | DoGreen | DoBlue;
    else
	color.flags = 0;

    for (y = 0; color.flags && y < in_image->height; y++)
      {
	for (x = 0; color.flags && x < in_image->width; x++)
	  {
	    pix = XGetPixel(in_image, x, y);

	    i = 0;
	    while (i < pixI && oldPixels[i] != pix)
		i++;

	    if (i == pixI)
	      {
		color.red = (pix >> rshift) & rmask;
		color.green = (pix >> gshift) & gmask;
		color.blue = (pix >> bshift) & bmask;
		if (ncolors) {
		    color.red = colors[color.red].red;
		    color.green = colors[color.green].green;
		    color.blue = colors[color.blue].blue;
		} else {
		    color.red = ((unsigned long)color.red * 65535) / rmask;
		    color.green = ((unsigned long)color.green * 65535) / gmask;
		    color.blue = ((unsigned long)color.blue * 65535) / bmask;
		}
		if (!XAllocColor(dpy, colormap, &color))
		    color.flags = 0;
		else 
		  {
		    if (pixI >= pixMax)
		      {
			pixMax += 128;
			oldPixels = (Pixel *) realloc ((void *) oldPixels,
					(sizeof (Pixel) * pixMax));
			newPixels = (Pixel *) realloc ((void *) newPixels,
					(sizeof (Pixel) * pixMax));

			/*
			 * check the realloc
			 */
			if (oldPixels == NULL || newPixels == NULL)
			  {
			    if (oldPixels)
				free (oldPixels);

			    if (newPixels)
			      {
				XFreeColors(dpy, colormap, newPixels, pixI, 0);
				free (newPixels);
			      }

			    return GR_ALLOC_ERR;
			  }
		      }
		    oldPixels[pixI]   = pix;
		    newPixels[pixI++] = color.pixel;
		  }
	      }
	    if (color.flags)
	        XPutPixel(out_image, x, y, newPixels[i]);
	  }
      }
    if (color.flags)
      {
	result = GR_SUCCESS;
        if (pixI < pixMax)
	  {
	    newPixels = (Pixel *) realloc ((void *) newPixels,
					(sizeof (Pixel) * pixI));
	    if (newPixels == NULL)
	        result = GR_ALLOC_ERR;
	  }

	free (oldPixels);

        *ret_colors = newPixels;
        *ret_number = pixI;
      }
    else
      {
	if (pixI)
	    XFreeColors (dpy, colormap, newPixels, pixI, 0);

        free (oldPixels);
        free (newPixels);

	result = GreyScale(dpy, screen, colormap, in_image, out_image, colors,
			ncolors, force, rshift, gshift, bshift, rmask, gmask, bmask);
      }

    return result;
}

static unsigned int
#ifdef _NO_PROTO
Image_Size(image)
     XImage *image;
#else
Image_Size(
     XImage *image )
#endif /* _NO_PROTO */
{
    if (image->format != ZPixmap)
      return(image->bytes_per_line * image->height * image->depth);

    return((unsigned)image->bytes_per_line * image->height);
}

static int
#ifdef _NO_PROTO
XwdFileToPixmap (dpy, screen, depth, pixmap, colormap, visual, gc, src_x, src_y, dst_x, dst_y, width, height, file_name, ret_colors, ret_number)
    Display  *dpy;
    int	      screen;
    int       depth;
    Pixmap    pixmap;
    Colormap  colormap;
    Visual   *visual;
    GC        gc;
    int       src_x;
    int       src_y;
    int       dst_x;
    int       dst_y;
    int       width;
    int       height;
    char     *file_name;
    unsigned long **ret_colors;
    int      *ret_number;
#else
XwdFileToPixmap (
    Display  *dpy,
    int	      screen,
    int       depth,
    Pixmap    pixmap,
    Colormap  colormap,
    Visual   *visual,
    GC        gc,
    int       src_x,
    int       src_y,
    int       dst_x,
    int       dst_y,
    int       width,
    int       height,
    char     *file_name,
    unsigned long **ret_colors,
    int      *ret_number )
#endif /* _NO_PROTO */
{
    int result;
    register int i;
    XImage in_image, *out_image;
    register char *buffer;
    unsigned long swaptest = 1;
    int count;
    unsigned buffer_size;
    int ncolors;
    Bool rawbits = False;
    XColor *colors;
#ifdef __alpha
/* Use a different structure for compatibility with 32-bit platform */
    XWDColor   xwd_color;
#endif /* __alpha */
     XWDFileHeader header;
    FILE *in_file;

    in_file = fopen(file_name, "r");
    if (in_file == NULL)
	return GR_FILE_ERR;
    
    /*
     * Read in header information.
     */
    if(fread((char *)&header, sizeof(header), 1, in_file) != 1)
      {
         /*
	  * close the input file
	  */
        (void) fclose(in_file);
        return GR_HEADER_ERR;
      }

    if (*(char *) &swaptest)
	_swaplong((char *) &header, sizeof(header));

    /* check to see if the dump file is in the proper format */
    if (header.file_version != XWD_FILE_VERSION)
      {
         /*
	  * close the input file
	  */
        (void) fclose(in_file);
	return GR_HEADER_ERR;
      }

    if (header.header_size < sizeof(header))
      {
         /*
	  * close the input file
	  */
        (void) fclose(in_file);
	return GR_HEADER_ERR;
      }

     /*
      * skip the window name
      */
    if (fseek(in_file, (header.header_size - sizeof(header)), 1))
      {
         /*
	  * close the input file
	  */
        (void) fclose(in_file);
	return GR_FILE_ERR; 
      }

    /*
     * initialize the input image
     */
    in_image.width = (int) header.pixmap_width;
    in_image.height = (int) header.pixmap_height;
    in_image.xoffset = (int) header.xoffset;
    in_image.format = (int) header.pixmap_format;
    in_image.byte_order = (int) header.byte_order;
    in_image.bitmap_unit = (int) header.bitmap_unit;
    in_image.bitmap_bit_order = (int) header.bitmap_bit_order;
    in_image.bitmap_pad = (int) header.bitmap_pad;
    in_image.depth = (int) header.pixmap_depth;
    in_image.bits_per_pixel = (int) header.bits_per_pixel;
    in_image.bytes_per_line = (int) header.bytes_per_line;
    in_image.red_mask = header.red_mask;
    in_image.green_mask = header.green_mask;
    in_image.blue_mask = header.blue_mask;
    in_image.obdata = NULL;
    _XInitImageFuncPtrs(&in_image);

    /* read in the color map buffer */
    ncolors = header.ncolors;
    if (ncolors) {
	colors = (XColor *)malloc((unsigned) ncolors * sizeof(XColor));
	if (!colors)
          {
             /*
	      * close the input file
	      */
            (void) fclose(in_file);
	    return GR_ALLOC_ERR;
	  }

#ifdef __alpha
/* Use XWDColor instead of XColor. Byte-swapping if it is necessary.
 * Move values back into Xcolor structure.
 */
        for (i = 0; i < ncolors; i++) {
            if ( fread( (char *) &xwd_color, sizeof(XWDColor), 1 , in_file )  != 1 )
            {
                 /*
                  * close the input file
                  */
                (void) fclose(in_file);
                XFree ((char *) colors);
                return GR_FILE_ERR;
            }

            if (*(char *) &swaptest) {
                _swaplong((char *) &xwd_color.pixel, sizeof(xwd_color.pixel));
                _swapshort((char *) &xwd_color.red, 3 * sizeof(xwd_color.red));
            }

            colors[i].pixel = xwd_color.pixel;
            colors[i].red   = xwd_color.red;
            colors[i].green = xwd_color.green;
            colors[i].blue  = xwd_color.blue;
            colors[i].flags = xwd_color.flags;
        }
#else
	if(fread((char *) colors, sizeof(XColor), ncolors, in_file) != ncolors)
	  {
             /*
	      * close the input file
	      */
            (void) fclose(in_file);
	    XFree ((char *) colors);
	    return GR_FILE_ERR;
	  }

	if (*(char *) &swaptest) {
	    for (i = 0; i < ncolors; i++) {
		_swaplong((char *) &colors[i].pixel, sizeof(long));
		_swapshort((char *) &colors[i].red, 3 * sizeof(short));
	    }
	}
#endif /* __alpha */
    }

    /*
     * alloc the pixel buffer
     */
    buffer_size = Image_Size(&in_image);
    buffer = (char *) malloc(buffer_size);
    if (buffer == NULL)
      {
         /*
	  * close the input file
	  */
        (void) fclose(in_file);
	XFree ((char *) colors);
        return GR_ALLOC_ERR;
      }

    /* read in the image data */
    count = fread(buffer, sizeof(char), (int)buffer_size, in_file);

     /*
      * close the input file
      */
    (void) fclose(in_file);

    if (count != buffer_size)
      {
	XFree ((char *) colors);
	XFree (buffer);
        return GR_FILE_ERR;
      }

    if (in_image.depth == 1) {
	in_image.format = XYBitmap;
	rawbits = True;
    }
    in_image.data = buffer;

    /* create the output image */
    result = GR_SUCCESS;

    if (rawbits) {
	out_image = &in_image;
    } else {
	out_image = XCreateImage(dpy, visual, depth,
			 (depth == 1) ? XYBitmap : in_image.format,
				 in_image.xoffset, NULL,
				 in_image.width, in_image.height,
				 XBitmapPad(dpy), 0);

	out_image->data = (char *) malloc(Image_Size(out_image));

	if ((header.visual_class == TrueColor) ||
		   (header.visual_class == DirectColor))
	    result = Do_Direct(dpy, screen, &header, colormap, ncolors, colors,
			ForceColor,
		      &in_image, out_image, ret_colors, ret_number);
	else
	    result = Do_Pseudo(dpy, screen, colormap, ncolors, colors, ForceColor,
			&in_image, out_image, ret_colors, ret_number);
    }

    if (result != GR_ALLOC_ERR)
        XPutImage(dpy, pixmap, gc, out_image,
			  src_x, src_y, dst_x, dst_y,
			  width, height);
    /*
     * free the buffers
     */
    XFree ((char *) colors);
    XFree (buffer);

    /*
     * Destroy the image if it got created
     */
    if (!rawbits)
      XDestroyImage(out_image);

    return result;
}

/*****************************************************************************/
/*  General functions for changing a file into a pixmap                      */
/*                                                                           */
/*****************************************************************************/
/***********
 *
 * Function processBitmap
 *
 * takes a string.
 *	Make a bitmap into the pixmap for the graphic.
 *
 ***********/
static	Pixmap 
#ifdef _NO_PROTO
processBitmap(dpy, depth, drawable, gc, nameStr, pixmapWidth, pixmapHeight )
	Display       *dpy;
	int	       depth;
	Drawable       drawable;
	GC             gc;
        char          *nameStr ;
	unsigned int  *pixmapWidth;
	unsigned int  *pixmapHeight;
#else
processBitmap(
	Display       *dpy,
	int	       depth,
	Drawable       drawable,
	GC             gc,
        char          *nameStr,
	unsigned int  *pixmapWidth,
	unsigned int  *pixmapHeight )
#endif /* _NO_PROTO */
{
    int           result;
    int           junk;
    Pixmap        bitmap=NULL;
    Pixmap        pixmap=NULL;

   /*
    * Read the bitmap file
    */
    result = XReadBitmapFile (dpy, drawable, nameStr, pixmapWidth, pixmapHeight,
			&bitmap, &junk, &junk);

    /*  Be sure to implement XvpCopyPlane later         */

    if (result == BitmapSuccess)
      {
	if (depth == 1)
	    pixmap = bitmap;
	else
	  {

	    pixmap = XCreatePixmap (dpy, drawable, (*pixmapWidth),
		(*pixmapHeight), depth);

	    if (pixmap == NULL) {
	        XFreePixmap(dpy, bitmap);
	        return(NULL);
	    }

	    XCopyPlane (dpy, bitmap, pixmap, gc,
			0, 0, (*pixmapWidth), (*pixmapHeight), 0, 0, 1L);
          }
      }
    if (result != BitmapSuccess)
        return(NULL);

    return(pixmap);
}

#ifndef	STUB
/***********
 *
 * Function processTiff
 *
 * takes a string.
 *	Make a tiff into the pixmap for the graphic.
 *
 ***********/
static	Pixmap 
#ifdef _NO_PROTO
processTiff(dpy, depth, drawable, colormap, visual, gc, tiff_xwc,
		nameStr, pixmapWidth, pixmapHeight )
	Display       *dpy;
	int	       depth;
	Drawable       drawable;
	Colormap       colormap;
	Visual	      *visual;
	GC             gc;
        ilXWC	      *tiff_xwc;
        char   *nameStr ;
	unsigned int    *pixmapWidth;
	unsigned int    *pixmapHeight;
#else
processTiff(
	Display       *dpy,
	int	       depth,
	Drawable       drawable,
	Colormap       colormap,
	Visual	      *visual,
	GC             gc,
        ilXWC	      *tiff_xwc,
        char   *nameStr,
	unsigned int    *pixmapWidth,
	unsigned int    *pixmapHeight )
#endif /* _NO_PROTO */
{
    int		 result = -1;
    FILE	*inStream;
    Pixmap       pixmap = NULL;
    ilFile	 inFile;
    ilPipe	 inPipe;
    ilFileImage	 inImage;
    const ilImageDes   *inDes;
    static ilContext	IlContext = NULL;

    if (IlContext == NULL)
      {
	if (IL_CREATE_CONTEXT (&IlContext, 0))
	  {
	    IlContext = NULL;
	    return NULL;
	  }
      }

    if (*tiff_xwc == NULL)
      {
        *tiff_xwc = ilCreateXWC (IlContext, dpy, visual, colormap, gc, 0, 0);
        if (*tiff_xwc == NULL)
	    return NULL;
      }

    inStream = fopen (nameStr, "r");
    if (!inStream)
	return NULL;

    inFile = ilConnectFile (IlContext, inStream, 0, 0);
    if (inFile)
      {
	inImage = ilListFileImages (inFile, 0);
	if (inImage)
	  {
	    *pixmapWidth  = (int) inImage->width;
	    *pixmapHeight = (int) inImage->height;
	    pixmap = XCreatePixmap (dpy, drawable, (*pixmapWidth),
				(*pixmapHeight), depth);
	    if (pixmap)
	      {
	        inPipe = ilCreatePipe (IlContext, 0);
	        if (inPipe)
	          {
		    if (ForceColor == GREY_SCALE)
			inDes = IL_DES_GRAY;
		    else if (ForceColor == BITONAL)
			inDes = IL_DES_BITONAL;

		    result = 0;
		    if (!ilReadFileImage (inPipe, inImage, (ilRect *) NULL, 0))
			result = -1;

		    if (result == 0 && ForceColor != COLOR_SCALE &&
			ilConvert (inPipe, inDes, ((ilImageFormat *) NULL), 0, NULL) != True)
			result  = -1;

		    if (result == 0 &&
			ilWriteXDrawable (inPipe, pixmap, *tiff_xwc,
						(ilRect *) NULL, 0, 0, 0))
		      {
			ilExecutePipe (inPipe, 0);
			if (IlContext->error != 0)
			    result = -1;
		      }
		    ilDestroyObject (inPipe);
	          }
		if (result == -1)
		  {
		    XFreePixmap (dpy, pixmap);
		    pixmap = NULL;
		  }
	      }
	  }
	ilDestroyObject (inFile);
      }

    fclose (inStream);

    return pixmap;
}
#endif

/***********
 *
 * Function processXwd
 *
 * takes a string.
 *	Make an XWD file into the pixmap for the graphic.
 *
 ***********/
static	Pixmap 
#ifdef _NO_PROTO
processXwd(dpy, screen, depth, drawable, colormap, visual, gc, nameStr, pixmapWidth, pixmapHeight, ret_colors, ret_number)
	Display       *dpy;
	int	       screen;
	int	       depth;
	Drawable       drawable;
	Colormap       colormap;
	Visual	      *visual;
	GC             gc;
        char   *nameStr ;
	unsigned int    *pixmapWidth;
	unsigned int    *pixmapHeight;
	unsigned long **ret_colors;
	int *ret_number;
#else
processXwd(
	Display       *dpy,
	int	       screen,
	int	       depth,
	Drawable       drawable,
	Colormap       colormap,
	Visual	      *visual,
	GC             gc,
        char   *nameStr,
	unsigned int    *pixmapWidth,
	unsigned int    *pixmapHeight,
	unsigned long **ret_colors,
	int *ret_number )
#endif /* _NO_PROTO */
{
    Pixmap         pixmap;
    XWDFileHeader  header;
    FILE	  *in_file;
    int            result;
    unsigned long swaptest = TRUE;

   /*
    * Read the header information.
    */

    in_file = fopen(nameStr, "r");
    if (in_file == NULL)
    	return NULL;

    /* Read in XWDFileHeader structure */
    result = fread((char *)&header, sizeof(header), 1, in_file);
    (void) fclose(in_file);

    if (result == 1 && *(char *) &swaptest)
	_swaplong((char *) &header, sizeof(header));

    if (result != 1 || header.file_version != XWD_FILE_VERSION)
	return NULL;
    
    *pixmapWidth = header.pixmap_width;
    *pixmapHeight = header.pixmap_height;
    pixmap = XCreatePixmap (dpy, drawable, (*pixmapWidth), (*pixmapHeight),
		depth);

    if (pixmap) {
        if (XwdFileToPixmap (dpy, screen, depth, pixmap, colormap, visual,
		gc, 0, 0, 0, 0,
		(*pixmapWidth), (*pixmapHeight), nameStr,
		ret_colors, ret_number)
			== GR_SUCCESS) {
	    return pixmap;
        }
    }
    return NULL;
}

/************************************
 * myXpmReadFileToPixmap
 ***********************************/
static int
#ifdef _NO_PROTO
myXpmReadFileToPixmap(display, screen, d, filename, pixmap_return,
                    shapemask_return, attributes,
		    gc, bg, fg, depth)
    Display        *display;
    int		    screen;
    Drawable        d;
    char           *filename;
    Pixmap         *pixmap_return;
    Pixmap         *shapemask_return;
    XpmAttributes  *attributes;
    GC              gc;
    Pixel	    bg;
    Pixel	    fg;
    int             depth;
#else
myXpmReadFileToPixmap(
    Display        *display,
    int		    screen,
    Drawable        d,
    char           *filename,
    Pixmap         *pixmap_return,
    Pixmap         *shapemask_return,
    XpmAttributes  *attributes,
    GC              gc,
    Pixel	    bg,
    Pixel	    fg,
    int             depth )
#endif /* _NO_PROTO */
{
    XImage *image, **imageptr = NULL;
    XImage *shapeimage, **shapeimageptr = NULL;
    int ErrorStatus;
    int switchFlag = 0;

    /*
     * initialize return values
     */
    if (pixmap_return) {
        *pixmap_return = NULL;
        imageptr = &image;
    }
    if (shapemask_return) {
        *shapemask_return = NULL;
        shapeimageptr = &shapeimage;
    }

    /*
     * create the images
     */
    ErrorStatus = _XmXpmReadFileToImage(display, filename, imageptr,
                                     shapeimageptr, attributes);
    if (ErrorStatus < 0)
        return (ErrorStatus);

    /*
     * Check to see if we will need to switch the foreground and
     * background colors.  When forced to a depth of 1, the Xpm call
     * returns a ZPixmap ready to place in pixmap of depth 1.
     * Unfortunately, usually the pixmap is of a different depth.  This
     * causes a depth mismatch for the XPutImage.  Therefore, we change
     * the format type to XYBitmap and XPutImage is happy.  But on
     * servers where BlackPixel is not at pixel 1 in the colormap, this
     * causes the image to be inverted.  So by switching the foreground
     * and background in the gc, the image is corrected for these
     * systems.
     *
     * The other way of doing this, is to create a pixmap of depth 1 and
     * do an XPutImage to it.  Then do a XCopyPlane to a pixmap of the
     * correct depth.  But this is a major performance hit compared to
     * switching the colors.
     *
     * The downside of switching colors is that it works only when
     * BlackPixel and WhitePixel fill colormap entries 0 and 1.  But
     * since this is the 99.9% case, we'll go with it.
     */
    if (image->depth == 1 && XBlackPixel(display, screen) != 1L)
	switchFlag = 1;

    /*
     * create the pixmaps
     */
    if (imageptr && image) {
        *pixmap_return = XCreatePixmap(display, d, image->width,
                                       image->height, depth);

	if (image->depth == 1)
	    image->format = XYBitmap;

	if (switchFlag)
	  {
            XSetBackground (display, gc, fg);
            XSetForeground (display, gc, bg);
	  }

        XPutImage(display, *pixmap_return, gc, image, 0, 0, 0, 0,
                  image->width, image->height);

	if (switchFlag)
	  {
	    XSetBackground (display, gc, bg);
	    XSetForeground (display, gc, fg);
	  }

        XDestroyImage(image);
    }

    if (shapeimageptr && shapeimage)
        XDestroyImage(shapeimage);

    return (ErrorStatus);
}

static XpmColorSymbol colorSymbol = {"none", NULL, 0};

/***********
 *
 * Function processXpm
 *
 * takes a string.
 *	Make an XPM file into the pixmap for the graphic.
 *
 ***********/
static	Pixmap 
#ifdef _NO_PROTO
processXpm(dpy, screen, depth, drawable, force_color,
	   colormap, visual, gc, nameStr,
	   pixmapWidth, pixmapHeight, fore_ground, back_ground,
	   ret_mask, ret_colors, ret_number)
	Display       *dpy;
	int	       screen;
	int	       depth;
	Drawable       drawable;
	enum ColorType force_color;
	Colormap       colormap;
	Visual	      *visual;
	GC		gc;
        char   *nameStr ;
	unsigned int    *pixmapWidth;
	unsigned int    *pixmapHeight;
	Pixel   fore_ground;
	Pixel   back_ground;
	Pixmap *ret_mask;
	Pixel **ret_colors;
	int *ret_number;
#else
processXpm(
	Display       *dpy,
	int	       screen,
	int	       depth,
	Drawable       drawable,
	enum ColorType force_color,
	Colormap       colormap,
	Visual	      *visual,
	GC		gc,
        char   *nameStr,
	unsigned int    *pixmapWidth,
	unsigned int    *pixmapHeight,
	Pixel   fore_ground,
	Pixel   back_ground,
	Pixmap *ret_mask,
	Pixel **ret_colors,
	int *ret_number )
#endif /* _NO_PROTO */
{
    int		    i, j;
    int             result;
    short           done;
    Pixmap          pixmap  = NULL;
    XpmAttributes   xpmAttr;
    Visual          vis2;

    colorSymbol.pixel = back_ground;

    xpmAttr.valuemask    = XpmVisual       | XpmReturnPixels |
			   XpmColorSymbols | XpmColormap     | XpmDepth;
    xpmAttr.visual       = visual;
    xpmAttr.colorsymbols = &colorSymbol;
    xpmAttr.colormap     = colormap;
    xpmAttr.numsymbols   = 1;
    xpmAttr.depth        = depth;
    xpmAttr.pixels       = NULL;

    do
      {
        /*
         * If not color, force to black and white.
         */
        if (force_color != COLOR_SCALE)
          {
            memcpy (&vis2, xpmAttr.visual, sizeof(Visual));

            vis2.class       = StaticGray;
            vis2.map_entries = 2;

            xpmAttr.depth  = 1;
            xpmAttr.visual = &vis2;
          }

        result = myXpmReadFileToPixmap (dpy, screen, drawable, nameStr, &pixmap,
					ret_mask, &xpmAttr, gc,
					back_ground, fore_ground, depth);
	done = True;

	/*
	 * if we did not successfully get our icon, force the color
	 * to black and white.
	 */
	if (result == XpmColorFailed && force_color == COLOR_SCALE)
	  {
	    force_color = BITONAL;
	    done       = False;
          }

      } while (done == False);

    if (result == XpmSuccess || result == XpmColorError)
      {
	*pixmapWidth  = xpmAttr.width;
	*pixmapHeight = xpmAttr.height;
	*ret_colors   = xpmAttr.pixels;
	*ret_number   = xpmAttr.npixels;

	/*
	 * squeeze out the pixel used for the transparent color since we
	 * don't want to free it when we free the other colors
	 */
	i = 0;
	while (i < xpmAttr.npixels && xpmAttr.pixels[i] != colorSymbol.pixel)
	    i++;

	if (i < xpmAttr.npixels)
	  {
	    for (j = i, i = i + 1; i < xpmAttr.npixels; i++, j++)
		xpmAttr.pixels[j] = xpmAttr.pixels[i];

	    *ret_number = j;
	  }
      }

    return pixmap;
}

/******************************************************************************
 *
 * Public Functions
 *
 *****************************************************************************/
/******************************************************************************
 *
 * Function _DtHelpProcessGraphic
 *
 * takes a string.
 *	Get the pixmap for the graphic.
 *
 *****************************************************************************/
Pixmap 
#ifdef _NO_PROTO
_DtHelpProcessGraphic(dpy, drawable, screen, depth, gc,
			def_pix, def_pix_width, def_pix_height, tiff_xwc,
			colormap, visual, fore_ground, back_ground, filename,
			width, height, ret_mask, ret_colors, ret_number )
	Display    *dpy;
	Drawable    drawable;
	int	    screen;
	int	    depth;
	GC          gc;
        Pixmap	   *def_pix;
	Dimension  *def_pix_width;
	Dimension  *def_pix_height;
        ilXWC	   *tiff_xwc;
	Colormap    colormap;
	Visual	   *visual;
	Pixel	    fore_ground;
	Pixel	    back_ground;
        char       *filename ;
	Dimension  *width;
	Dimension  *height;
	Pixmap     *ret_mask;
	Pixel     **ret_colors;
	int	   *ret_number;
#else
_DtHelpProcessGraphic(
	Display    *dpy,
	Drawable    drawable,
	int	    screen,
	int	    depth,
	GC          gc,
        Pixmap	   *def_pix,
	Dimension  *def_pix_width,
	Dimension  *def_pix_height,
        ilXWC	   *tiff_xwc,
	Colormap    colormap,
	Visual	   *visual,
	Pixel	    fore_ground,
	Pixel	    back_ground,
        char       *filename,
	Dimension  *width,
	Dimension  *height,
	Pixmap     *ret_mask,
	Pixel     **ret_colors,
	int	   *ret_number )
#endif /* _NO_PROTO */
{

    unsigned int    pixWidth  = 0;
    unsigned int    pixHeight = 0; 
    char  *ptr;
    Pixmap pix = NULL;

    /*
     * initialize the return values
     */
    *ret_colors = NULL;
    *ret_number = 0;
    *ret_mask   = None;

    /*
     * Initialize the Force Variable if this is the first time
     */
    if (ForceColor == COLOR_INVALID)
      {
	char    *ptr;
	char    *ptr2;
	char    *xrmName  = NULL;
	char    *xrmClass = NULL;
	char	*retStrType;
	XrmValue retValue;

	ForceColor = COLOR_SCALE;

	XtGetApplicationNameAndClass (dpy, &ptr, &ptr2);
	xrmName  = malloc (strlen (ptr) + 14);
	xrmClass = malloc (strlen (ptr2) + 14);
	if (xrmName && xrmClass)
	  {
	    strcpy (xrmName , ptr);
	    strcat (xrmName , ".helpColorUse");

	    strcpy (xrmClass, ptr2);
	    strcat (xrmClass, ".HelpColorUse");

	    if (XrmGetResource (XtDatabase (dpy), xrmName, xrmClass,
					&retStrType, &retValue) == True)
	      {
		ptr = (char *) retValue.addr;
		/*
		 * check for GreyScale
		 */
		if (*ptr == 'G' || *ptr == 'g')
		    ForceColor = GREY_SCALE;
		/*
		 * check for BlackWhite
		 */
		else if (*ptr == 'B' || *ptr == 'b')
		    ForceColor = BITONAL;
	      }

	    free (xrmName);
	    free (xrmClass);
	  }

	/*
	 * choose the correct visual type to use
	 */
	if (ForceColor == COLOR_SCALE &&
		(visual->class == GrayScale || visual->class == StaticGray))
	    ForceColor = GREY_SCALE;

	if (ForceColor != BITONAL && depth == 1)
	    ForceColor = BITONAL;
      }

    if (filename != NULL &&
		_DtHelpCeStrrchr(filename, ".", MB_CUR_MAX, &ptr) != -1)
      {
	int   result;
	char *inFile = NULL;

	result = _DtHelpCeGetUncompressedFileName (filename, &inFile);
	if (result == -1)
	    pix = NULL;
        else if (strcmp (ptr, ".xwd") == 0)
	    pix = processXwd (dpy, screen, depth, drawable, colormap, visual,
					gc, inFile, &pixWidth,
					&pixHeight, ret_colors, ret_number);
        else if (strncmp (ptr, ".tif", 4) == 0)
#ifndef	STUB
	    pix = processTiff (dpy, depth, drawable, colormap, visual,
				gc, tiff_xwc, inFile, &pixWidth, &pixHeight);
#else
	    pix = NULL;
#endif
        else if (strcmp (ptr, ".xpm") == 0 || strcmp (ptr, ".pm") == 0)
	    pix = processXpm (dpy, screen, depth, drawable,
					ForceColor, colormap, visual,
					gc, inFile, &pixWidth, &pixHeight,
					fore_ground, back_ground,
					ret_mask, ret_colors, ret_number);
        else
	    pix = processBitmap(dpy, depth, drawable, gc, inFile,
						&pixWidth, &pixHeight);

	if (result == 0)
	  {
	    unlink (inFile);
	    XtFree (inFile); /* purify said to use XtFree, XtMalloc was used */
          }
      }

    if (pix == NULL)
      {
	/*
	 * Try to get a localized pixmap
         * NOTE....
         * This is a cached pixmap.....
         * Make sure XFreePixmaps does not free this one.
	 */
	if (*def_pix == NULL)
	  {
	    *def_pix = XmGetPixmap(XDefaultScreenOfDisplay(dpy),
					"Dthgraphic", fore_ground, back_ground);
	    if (*def_pix != XmUNSPECIFIED_PIXMAP)
	      {
	        Window        root_id;
	        int           x, y;
	        unsigned int  border;
	        unsigned int  depth;

	        if (XGetGeometry(dpy, *def_pix, &root_id, &x, &y,
				&pixWidth, &pixHeight, &border, &depth) == 0)
	          {
		    XmDestroyPixmap(XDefaultScreenOfDisplay(dpy), *def_pix);
		    *def_pix = NULL;
	          }
	      }
	    else
	        *def_pix = NULL;

	    /*
	     * couldn't get a localized pixmap, go with a build in default
	     */
	    if (*def_pix == NULL)
	      {
	        pixWidth  = Missing_bm_width;
	        pixHeight = Missing_bm_height;
	        *def_pix  = XCreatePixmapFromBitmapData ( dpy, drawable,
					(char *) Missing_bm_bits,
					pixWidth, pixHeight,
					fore_ground, back_ground, depth);
	      }

	    *def_pix_width  = (Dimension) pixWidth;
	    *def_pix_height = (Dimension) pixHeight;
	  }

	pix       = *def_pix;
        pixWidth  = *def_pix_width;
        pixHeight = *def_pix_height;
      }

    *width  = (Dimension) pixWidth;
    *height = (Dimension) pixHeight;

    return pix;
}
