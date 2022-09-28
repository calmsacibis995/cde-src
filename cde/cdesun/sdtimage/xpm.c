#ifndef lint
static char sccsid[] = "@(#)rast.c 1.2 94/11/14";
#endif

/*  Copyright (c) 1987, 1988, Sun Microsystems, Inc.  All Rights Reserved.
 *  Sun considers its source code as an unpublished, proprietary
 *  trade secret, and it is available only under strict license
 *  provisions.  This copyright notice is placed here only to protect
 *  Sun in the event the source is deemed a published work.  Dissassembly,
 *  decompilation, or other means of reducing the object code to human
 *  readable form is prohibited by the license agreement under which
 *  this code is provided to the user or company in possession of this
 *  copy.
 *
 *  RESTRICTED RIGHTS LEGEND: Use, duplication, or disclosure by the
 *  Government is subject to restrictions as set forth in subparagraph
 *  (c)(1)(ii) of the Rights in Technical Data and Computer Software
 *  clause at DFARS 52.227-7013 and in similar clauses in the FAR and
 *  NASA FAR Supplement.
 */

#include <Xm/Xm.h>
#include <Dt/xpm.h>
#include "display.h"
#include "image.h"
#include "imagetool.h"
#include "ui_imagetool.h"

#define UNDEF_PIXEL 0x80000000 /* from xpmP.h */

int
xpm_load(image)
ImageInfo	*image;
{
    XpmAttributes  xpm_attrs;
    XImage        *ximage, *xshape = NULL;
    int 	   status, i, j;
    Visual         visual;
    XpmColorSymbol symbols[5];
	
    XilDataType       datatype;
    Xil_boolean       success;
    XilMemoryStorage  storage;
    int               x_index, xil_index, row, col;
    unsigned int      pixel_stride;
    unsigned char    *startlineptr, *xstartlineptr;
    unsigned char    *lineptr, *xlineptr;
    int 	      xpixel_stride, canvas_depth;
    Display          *dpy = image_display->xdisplay;
    Pixel             pixels[256];
    XVisualInfo       vinfo;
    unsigned long     pmask[1];
    Boolean           free_colormap = False;
    int               screen_depth;
    char           ***colorTable=(char ***)NULL;

/* 
 * This also writes compressed data to tempfile.
 */
    if (openfile (image->file, image->realfile, image->compression, 
		  image->data, image->file_size) != 0)
       return (-1);
/*
 * Don't need it open.
 */
    closefile ();

/*
 * Create a temp colormap for the xpm read function to use since
 * it needs one to alloc the colors into.
 */
    xpm_attrs.depth = DefaultDepth (dpy, DefaultScreen (dpy));
    screen_depth=xpm_attrs.depth;

    if (xpm_attrs.depth == 8 || xpm_attrs.depth == 4) {
      if ((XMatchVisualInfo (dpy, DefaultScreen (dpy), xpm_attrs.depth,
                             image_display->visual->class,
                             &vinfo)) == 0)
        return -1;
      xpm_attrs.colormap = XCreateColormap (dpy,
                                        RootWindow (dpy, DefaultScreen (dpy)),
                                        vinfo.visual, AllocNone);
      xpm_attrs.visual = vinfo.visual;
      free_colormap = True;
    }
    else if (xpm_attrs.depth == 24 &&
              (XMatchVisualInfo (dpy, DefaultScreen (dpy), xpm_attrs.depth,
                             TrueColor, &vinfo)) != 0
            ) {
        /*
         * If it's 24-bit, then use TrueColor Visual for best results
         */
      xpm_attrs.colormap = XCreateColormap (dpy,
                                        RootWindow (dpy, DefaultScreen (dpy)),
                                        vinfo.visual, AllocNone);
      xpm_attrs.visual = vinfo.visual;
      free_colormap = True;
    } else {
      xpm_attrs.colormap = DefaultColormap (dpy, DefaultScreen (dpy));
      xpm_attrs.visual = DefaultVisual(XtDisplay (base->mainw),
                                       DefaultScreen (XtDisplay (base->mainw)));
    }
 
    symbols[0].name  = "background";
    symbols[0].value = NULL;
    XtVaGetValues (image_display->new_canvas, XmNdepth, &canvas_depth, NULL);
    if (canvas_depth == 24)
      symbols[0].pixel = WhitePixelOfScreen (XtScreen (base->mainw));
    else
      symbols[0].pixel = BlackPixelOfScreen (XtScreen (base->mainw));

   /*
    * Specify the visual and set the XpmAttributes mask.
    */
 
    xpm_attrs.colorsymbols = symbols;
    xpm_attrs.numsymbols   = 1;

    xpm_attrs.valuemask =  XpmColorSymbols | XpmDepth | XpmColormap | XpmVisual;

    status = _DtXpmReadFileToImage (XtDisplay (base->top_level), 
	        image->realfile,
		&ximage, &xshape, &xpm_attrs);
		
    if (status != XpmSuccess) {
      if (free_colormap == True)
        XFreeColormap (dpy, xpm_attrs.colormap);
      return -1;
    }

    image->width   = ximage->width;
    image->height  = ximage->height;
    image->depth   = ximage->depth;

    xpixel_stride = ximage->bytes_per_line / ximage->width;

    if (xpm_attrs.depth == 8) {
      XColor xcolors [256];

      image->colors = 256;
      image->red = (unsigned char *) malloc (sizeof (unsigned char) * 256);
      image->green = (unsigned char *) malloc (sizeof (unsigned char) * 256);
      image->blue = (unsigned char *) malloc (sizeof (unsigned char) * 256);

      for (i = 0; i < 256; i++)
        xcolors[i].pixel = i;

      XQueryColors (image_display->xdisplay, xpm_attrs.colormap, xcolors, 256);

      for (i = 0; i < 256; i++) {
	image->red [i] = xcolors[i].red >> 8;
	image->green [i] = xcolors[i].green >> 8;
	image->blue [i] = xcolors[i].blue >> 8;
      }
    }

    datatype = ximage->depth == 1 ? XIL_BIT : XIL_BYTE;
    image->orig_image = (*xil->create) (image_display->state, xpm_attrs.width,
	xpm_attrs.height, ximage->depth == 24 ? 3 : 1, datatype);

    (*xil->export_ptr) (image->orig_image);
    success = (*xil->get_memory_storage) (image->orig_image, &storage);
    if (success == FALSE) {
      XDestroyImage (ximage);
      if (free_colormap == True)
        XFreeColormap (dpy, xpm_attrs.colormap);
      return (-1);
    }

    if (datatype == XIL_BYTE) {
	image->bytes_per_line = storage.byte.scanline_stride;
	pixel_stride = storage.byte.pixel_stride;
    } else {
	image->bytes_per_line = storage.bit.scanline_stride;
    }

    /*
     * Copy data to new image. Now, image->bytes_per_line should be
     * correct for all depths, so we should be able to copy the correct
     * number of bytes from the ximage to the xil image in a standard way.
     */
    xil_index = 0;
    x_index = 0;
    if (ximage->depth != 24) {
	for (row = 0; row < ximage->height; row++) {
	    memcpy (&storage.byte.data[xil_index], &ximage->data[x_index], 
		image->bytes_per_line);
	    xil_index += image->bytes_per_line;
	    x_index += ximage->bytes_per_line;
	}
    } else {
        unsigned char Red, Green, Blue;

        /*
         * (DKenny - 12 Jun, 1997)
         * 
         * Compensate for different Byte Ordering in the data due to different
         * Server Byte Ordering.
         */
        if ( ximage->byte_order == LSBFirst ) { /* X86 Server */
            Red   = 1;
            Green = 0;
            Blue  = 3;
        } else {                                /* SPARC Server */
            Red   = 2;
            Green = 1;
            Blue  = 0;
        } 
	startlineptr = storage.byte.data;
	xstartlineptr = (unsigned char *) ximage->data;

	lineptr = startlineptr;
	xlineptr = xstartlineptr + 1;

	for (row = 0; row < ximage->height; row++) {
	    for (col = 0; col < ximage->width; col++) {
                lineptr[0] = xlineptr[Red];
                lineptr[1] = xlineptr[Green];
                lineptr[2] = xlineptr[Blue];

		lineptr += pixel_stride;
		xlineptr += xpixel_stride;
	    }
	    startlineptr += image->bytes_per_line;
	    lineptr = startlineptr;
	    xstartlineptr += ximage->bytes_per_line;
	    xlineptr = xstartlineptr + 1;
	}
	image->rgborder = 1;

    }


    (*xil->import_ptr) (image->orig_image, TRUE);
/*
 * Free Colormap that was created
 */
    if (free_colormap == True)
      XFreeColormap (dpy, xpm_attrs.colormap);

    return(0);

}

int
xbm_load(image)
ImageInfo	*image;
{
    Pixmap            pixmap = NULL;
    XImage	     *ximage = NULL;
    int 	      status, i, xhot, yhot;
    unsigned int      width, height;
    XilDataType       datatype;
    Xil_boolean       success;
    XilMemoryStorage  storage;
    int               x_index, xil_index, row, col;
    unsigned int      pixel_stride;
    unsigned char    *startlineptr, *xstartlineptr;
    unsigned char    *lineptr, *xlineptr;
    int 	      xpixel_stride;

/* 
 * This also writes compressed data to tempfile.
 */
    if (openfile (image->file, image->realfile, image->compression, 
		  image->data, image->file_size) != 0)
       return (-1);
/*
 * Don't need it open.
 */
    closefile ();

    status = XReadBitmapFile (XtDisplay (base->top_level), 
	        XtWindow (base->mainw), image->realfile,
	 	&width, &height, 
		&pixmap, &xhot, &yhot);
		
    if (status != BitmapSuccess) {
      if (pixmap != NULL)
	XFreePixmap (XtDisplay(base->top_level), pixmap);
      return (-1);
    }
    ximage = XGetImage (image_display->xdisplay, pixmap, 0, 0, width, height,
			AllPlanes, ZPixmap);

    image->depth  = ximage->depth;
    xpixel_stride = ximage->bytes_per_line / ximage->width;

    if (ximage->depth == 8) {
      XColor xcolors [256];

      image->colors = 2;
      image->red = (unsigned char *) malloc (sizeof (unsigned char) * 2);
      image->green = (unsigned char *) malloc (sizeof (unsigned char) * 2);
      image->blue = (unsigned char *) malloc (sizeof (unsigned char) * 2);

      for (i = 0; i < 2; i++)
        xcolors[i].pixel = i;

      XQueryColors (image_display->xdisplay, 
	DefaultColormap (image_display->xdisplay, 
			 DefaultScreen (image_display->xdisplay)),
	xcolors, 256);

      for (i = 0; i < 2; i++) {
	image->red [i] = xcolors[i].red >> 8;
	image->green [i] = xcolors[i].green >> 8;
	image->blue [i] = xcolors[i].blue >> 8;
      }
    }

    datatype = ximage->depth == 1 ? XIL_BIT : XIL_BYTE;
    image->orig_image = (*xil->create) (image_display->state, ximage->width,
	ximage->height, ximage->depth == 24 ? 3 : 1, datatype);

    (*xil->export_ptr) (image->orig_image);
    success = (*xil->get_memory_storage) (image->orig_image, &storage);
    if (success == FALSE) {
       XDestroyImage (ximage);
       return (-1);
    }

    if (datatype == XIL_BYTE) {
	image->bytes_per_line = storage.byte.scanline_stride;
	pixel_stride = storage.byte.pixel_stride;
    } else {
	image->bytes_per_line = storage.bit.scanline_stride;
    }

    /*
     * Copy data to new image. Now, image->bytes_per_line should be
     * correct for all depths, so we should be able to copy the correct
     * number of bytes from the ximage to the xil image in a standard way.
     */
    xil_index = 0;
    x_index = 0;
    if (ximage->depth != 24) {
	for (row = 0; row < ximage->height; row++) {
	    memcpy (&storage.byte.data[xil_index], &ximage->data[x_index], 
		image->bytes_per_line);
	    xil_index += image->bytes_per_line;
	    x_index += ximage->bytes_per_line;
	}
    } else {
	startlineptr = storage.byte.data;
	xstartlineptr = (unsigned char *) ximage->data;

	lineptr = startlineptr;
	xlineptr = xstartlineptr + 1;

	for (row = 0; row < ximage->height; row++) {
	    for (col = 0; col < ximage->width; col++) {
		memcpy (lineptr, xlineptr, 3);
		lineptr += pixel_stride;
		xlineptr += xpixel_stride;
	    }
	    startlineptr += image->bytes_per_line;
	    lineptr = startlineptr;
	    xstartlineptr += ximage->bytes_per_line;
	    xlineptr = xstartlineptr + 1;
	}
	image->rgborder = 0;
/*
	swap_red_blue (image, storage.byte.data, pixel_stride, 
 	               image->bytes_per_line);
*/
    }


    (*xil->import_ptr) (image->orig_image, TRUE);

    return(0);

}

int
xpm_save (image)
ImageInfo	*image;
{
    unsigned char *data;
    unsigned int   pixel_stride, scanline_stride;
    XpmAttributes  xpm_attrs;
    XImage        *ximage;
    XilImage       new_image = NULL;
    int            status, i, j = 0;
    Pixel          pixels[256];
    XColor         xcolors[256];
    Colormap       colormap;
    XVisualInfo    vinfo;
    unsigned long  pmask[1];
    Display       *dpy = image_display->xdisplay;
    int            depth, nbytes, actual_depth, class;
    unsigned int   width, height, nbands;
    XilDataType    datatype;
    Boolean        free_colormap = False;
    char        ***colorTable = (char ***)NULL;

    memset(&xpm_attrs, 0, sizeof(XpmAttributes));

    xpm_attrs.width = image->width;
    xpm_attrs.height = image->height;
    xpm_attrs.cpp = 1;
    xpm_attrs.valuemask = XpmSize | XpmCharsPerPixel;
    if (image->colors == 0)
      image->colors = 2;
    xpm_attrs.ncolors = image->colors;
     
/*
 * Create a colormap for the xpm write function to use since our
 * colormap puts the colors at the end.
 */
    XtVaGetValues (image_display->new_canvas, XmNdepth, &actual_depth, NULL );

    depth = actual_depth;
    class = image_display->visual->class;

    if (depth == 4 || depth == 8) {
      if ((XMatchVisualInfo (dpy, DefaultScreen (dpy), depth,
			     class, &vinfo)) == 0) 
	return -1;
      colormap = XCreateColormap (dpy, RootWindow (dpy, DefaultScreen (dpy)),
				  vinfo.visual, AllocNone);
      free_colormap = True;
      if (XAllocColorCells (dpy, colormap, 1, pmask, 0, pixels, depth == 4 ? 16 : 256) == 0) {
          if (free_colormap == True) 
 	    XFreeColormap (dpy, colormap);    
	  return -1;
      }
    } 
    else 
      colormap = DefaultColormap (dpy, DefaultScreen (dpy));

    if (image->colors == 2) {  /* dithered to mono */
      xcolors[0].pixel = 1;
      xcolors[0].red = xcolors[0].green = xcolors[0].blue = 0;
      xcolors[0].flags = DoRed | DoGreen | DoBlue;
      xcolors[1].pixel = 0;
      xcolors[1].red = xcolors[1].green = xcolors[1].blue = 65535;
      xcolors[1].flags = DoRed | DoGreen | DoBlue;
    }
    else  {
      for (i = 0; i < image->colors; i++) {
	xcolors[i].pixel = i;
	xcolors[i].red   = (image->red[i] << 8) + image->red[i];
	xcolors[i].green = (image->green[i]  << 8) + image->green[i];
	xcolors[i].blue  = (image->blue[i] << 8) + image->blue[i];
	xcolors[i].flags = DoRed | DoGreen | DoBlue;
      }
    }
/*
 * Store the images colors in this new colormap.
 */
    if (depth == 4) {
      XStoreColors (dpy, colormap, xcolors, 16);
      xpm_attrs.colormap = colormap;
      xpm_attrs.valuemask |= XpmColormap;
    }
    else if (depth == 8) {
      XStoreColors (dpy, colormap, xcolors, image->colors);
      xpm_attrs.colormap = colormap;
      xpm_attrs.valuemask |= XpmColormap;
    }
    else if (depth == 1) {
      xpm_attrs.colormap = colormap;
      xpm_attrs.valuemask |= XpmColormap;
      xpm_attrs.depth = 1;
      xpm_attrs.valuemask |= XpmDepth;
    } 
    else if ( depth == 24 ) {
      /*
       * (DKenny - 04 Jun, 1997)
       * Using the colorTable attribute to provide Xpm function with
       * colormap, as we cannot rely on the XColormap to be right for our
       * needs.
       */

      colorTable = (char ***)calloc( image->colors+1, sizeof(char**) );
      if ( ! colorTable )
        return(-1);

      for (i = 0; i < image->colors; i++) {
	char tmp_color[20];

        sprintf(tmp_color, "#%04X%04X%04X", xcolors[i].red,
				      xcolors[i].green, xcolors[i].blue);

        colorTable[i]   =(char **)calloc( 6, sizeof(char*) );
        if ( ! colorTable[i] ) {
          free(colorTable);
          return(-1);
        }

        pixels[i]=i;                                /* List of color pix */
        colorTable[i][0]=NULL;                      /* String for Color  */
        colorTable[i][1]=NULL;                      /* Symbolic name     */
        colorTable[i][2]=NULL;                      /* default mono      */
        colorTable[i][3]=NULL;                      /* default 4-level   */
        colorTable[i][4]=NULL;                      /* default grayscale */
        colorTable[i][5]=(char *)strdup(tmp_color); /* default color     */
      }    
      xpm_attrs.mask_pixel = UNDEF_PIXEL;           /* Transparent Pixel */
      xpm_attrs.colorTable = colorTable;
      xpm_attrs.ncolors    = image->colors;
      xpm_attrs.pixels     = pixels;
      xpm_attrs.npixels    = image->colors;
      xpm_attrs.valuemask |= XpmInfos;

    }


    data = retrieve_data (image->orig_image, &pixel_stride, &scanline_stride);
    (*xil->get_info) (image->orig_image, &width, &height, &nbands, &datatype);
    depth = retrieve_depth (nbands, datatype);
/*
 * Again, if 24, force data to 8.
 */
    if ( depth == 24 ) { 
      new_image = (XilImage) image_24to8 (image->orig_image);
      (*xil->get_info) (new_image, &width, &height, &nbands, &datatype);
      data = retrieve_data (new_image, &pixel_stride, &scanline_stride);
      depth = retrieve_depth (nbands, datatype);
    }
/*
 * If monochrome system and data is 8, then force to 1 or
 * XError occurs in XQueryColors()
 */
    else if (actual_depth == 1 && depth == 8) {
      new_image = (XilImage) image_8to1 (image->orig_image);
      (*xil->get_info) (new_image, &width, &height, &nbands, &datatype);
      data = retrieve_data (new_image, &pixel_stride, &scanline_stride);
      depth = retrieve_depth (nbands, datatype);
    }
/*
 * Create an Ximage from the image data.
 */
    if (depth == 1)
      nbytes = (image->width / 8) + ((image->width % 8 == 0) ? 0 : 1);
    else 
      nbytes = pixel_stride * image->width;

    ximage = XCreateImage (dpy, DefaultVisual ((dpy), DefaultScreen (dpy)),
               depth, (depth == 1 ? XYPixmap : ZPixmap), 0, (char *)data, 
	       image->width, image->height, 32, nbytes);
/*
 * Write out the xpm file.
 */
    status = _DtXpmWriteFileFromImage (dpy, image->file, ximage, NULL, 
				       &xpm_attrs);
/*
 * Free the ximage
   if (ximage)
     XDestroyImage (ximage);
 */
/*
 * copy new_image to orig_image
 * Free old xilimage
 */
   if (new_image) {
     if (image->orig_image)
	(*xil->destroy) (image->orig_image);
     image->orig_image = new_image;
   }
/*
 * Free the colormap just created before returning.
 */
    if (free_colormap == True)
      XFreeColormap (dpy, colormap);

    /*
     * Free the colorTable if it was allocated
     */
    if ( colorTable ) {
      for (i=0; i< image->colors; i++ ) {
	if ( colorTable[i][5])
	  free(colorTable[i][5]);
      }
      free(colorTable);
    }

    if (status != XpmSuccess)
      return (-1);
               
    return (0);

}

