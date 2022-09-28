/*******************************************************************************
**
**  solaris.c 1.28 96/11/15
**
**  RESTRICTED CONFIDENTIAL INFORMATION:
**
**  The information in this document is subject to special
**  restrictions in a confidential disclosure agreement between
**  HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
**  document outside HP, IBM, Sun, USL, SCO, or Univel without
**  Sun's specific written approval.  This document and all copies
**  and derivative works thereof must be returned or destroyed at
**  Sun's request.
**
**  Copyright 1995 Sun Microsystems, Inc.  All rights reserved.
**
*******************************************************************************/
#ifndef lint
static  char sccsid[] = "@(#)solaris.c 1.28 96/11/15 Copyright 1995 Sun Microsystems, Inc.";
#endif

/*******************************************************************************
 *
 *  File:        solaris.c
 *
 *  Description:
 *  -----------
 *  Code for drawing Solaris starburst and related info 
 *  on dthello welcome window.
 *
 ******************************************************************************/

#include <stdio.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <pixrect/rasterfile.h>
#include <X11/Xlib.h>
#include "solaris.h"
#include "Dt/Dt.h"
#include <Xm/MwmUtil.h>
#include <Xm/Xm.h>

static star_data_initialized = 0;
static logo_data_initialized = 0;

static Display* 	 s_dpy;
static Window 		 s_win;
static XWindowAttributes s_win_attr; 
static GC 		 s_gc;
static Colormap		 s_xcmap;
static int		 s_visual_class;
static int		 s_win_depth;
static int		 s_im_depth;

/* Starburst data */

static XImage*	s_star_image;
static int	s_star_x=0;
static int 	s_star_y=0;
static int	s_star_width=0;
static int	s_star_height=0;
static int	s_star_clip_width=0;

/* Logo data */

static XImage*	s_logo_image;
static int	s_logo_x=0;
static int	s_logo_y=0;
static int	s_logo_width=0;
static int	s_logo_height=0;
static int	s_logo_clip_width=0;
static int	s_border_offset=0;

 
#ifdef _LITTLE_ENDIAN
static const unsigned char bytereverse[256] = {
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
	0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
	0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
	0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
	0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
	0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
	0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
	0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
	0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
	0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
	0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
	0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
	0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
	0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
	0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
	0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
	0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
	0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
	0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
	0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
	0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
	0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
	0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
	0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
	0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
	0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
	0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
	0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
	0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
	0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
	0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
	0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};
#endif


/*******************************************************************************
 *
 *  Procedure:        starburst.c
 *
 *  Description:
 *  -----------
 *  Draws Solaris starburst 
 *
 ******************************************************************************/

void
Starburst( Display* dpy, 
	   Window win, 
	   GC *gc, 
	   char* fgColor,
	   char* bgColor,
	   int* ncolors ) {
	FILE			*fp;
	unsigned char		*im_cmap;
	int 			status;
	unsigned char		*r, *g, *b;
	int			i, read_width;
        int			fgPixel, bgPixel;
	XGCValues               gcv; /* Struct for creating GC */

	if (star_data_initialized) {
	    DrawStar();
	    return;
	}

	s_dpy = dpy;
	s_win = win;
	s_gc = *gc;	

	status = XGetWindowAttributes( s_dpy, s_win, &s_win_attr );
	if (status == 0) return;

	s_visual_class = s_win_attr.visual->class;
	s_win_depth = s_win_attr.depth;

/*
 *	Open starburst raster image file
 */
	OpenImage(&fp, STARIMAGE);

	if (!fp) return;

/*
 *	Read image header and color data.
 */
	ReadImageHeader( fp, &read_width, &s_star_width, &s_star_height,
			 &s_star_clip_width, ncolors, &r, &g, &b, &im_cmap);

/*
 *	Create colormap and store colors from image  
 */
	if ( (s_visual_class == PseudoColor && s_win_depth == 8 ) ||
	     (s_visual_class == GrayScale && s_win_depth == 8 ) ||
	     (s_visual_class == DirectColor && s_win_depth == 24) ) {

	    s_xcmap = XCreateColormap(s_dpy, s_win, 
				      s_win_attr.visual, AllocAll);

	    XSetWindowColormap(s_dpy, s_win, s_xcmap);

	    StoreColors(r, g, b, *ncolors, 0);

	    bgPixel = (*ncolors)++;
	    fgPixel = (*ncolors)++; 

	    if ( s_visual_class == DirectColor ) { 
		bgPixel = bgPixel | bgPixel << 8 | bgPixel << 16;
		fgPixel = fgPixel | fgPixel << 8 | fgPixel << 16;
	    }

	    XStoreNamedColor(s_dpy, s_xcmap, bgColor, bgPixel, 
			           DoRed | DoGreen | DoBlue);

	    XStoreNamedColor(s_dpy, s_xcmap, fgColor, fgPixel, 
			          DoRed | DoGreen | DoBlue);

	    XFreeGC(s_dpy, s_gc);
	    gcv.foreground = fgPixel;
	    gcv.background = bgPixel;
	    s_gc = *gc = XCreateGC( s_dpy, s_win, 
				    (GCForeground | GCBackground), &gcv );

	    XSetWindowBackground( s_dpy, s_win, bgPixel );

	    XInstallColormap(s_dpy, s_xcmap);
	}

/*
 *	Read raster image data
 */

	ReadImage( fp, 
		   read_width,
		   r, g, b, 
		   s_star_width,
		   s_star_height,
		   &s_star_image, 0 );

	free(im_cmap);
        fclose (fp);
/*
 *	Draw starburst X image
 */
	s_star_x = s_win_attr.width/2 - s_star_width/2; 
	s_star_y = s_win_attr.height/2 - s_star_height/2; 

	star_data_initialized = 1;

	DrawStar();
}

static void
DrawStar() {
/*
 *	Clear window and draw the Solaris starburst image
 */
	XClearWindow(s_dpy, s_win);

	if (star_data_initialized) {
	    XPutImage ( s_dpy, s_win , s_gc, s_star_image , 0, 0, 
		        s_star_x, s_star_y, s_star_clip_width, s_star_height );
	}
}


/*******************************************************************************
 *
 *  Procedure:        Logo
 *
 *  Description:
 *  -----------
 *  Draws Adobe logo
 *
 ******************************************************************************/

void
Logo( int border_offset, int cindex_offset ) {
	FILE			*fp;
	struct rasterfile	im_header;
	unsigned char		*im_cmap;
	int			i, read_width, ncolors, status;
	unsigned char		*r, *g, *b;

	if (logo_data_initialized) {
	    DrawLogo();
	    return;
	}

	s_border_offset = border_offset;

/*
 *	Open logo's raster image file
 */
	OpenImage(&fp, ADOBEIMAGE);

	if (!fp) return;

/*
 *	Read image header and color data.
 */

	ReadImageHeader( fp, &read_width, &s_logo_width, &s_logo_height, 
			 &s_logo_clip_width, &ncolors, &r, &g, &b, &im_cmap );


/*
 *	Store colors from image, overlays color on starburst image's color map
 */
	if ( star_data_initialized &&
	     (s_visual_class == PseudoColor && s_win_depth == 8 ) ||
	     (s_visual_class == GrayScale && s_win_depth == 8 ) ||
	     (s_visual_class == DirectColor && s_win_depth == 24) ) {

	    StoreColors(r, g, b, ncolors, cindex_offset );
	}

/*
 *	Read raster image data
 */

	ReadImage( fp, 
		   read_width,
		   r, g, b, 
		   s_logo_width,
		   s_logo_height,
		   &s_logo_image,
		   cindex_offset );

	free(im_cmap);
        fclose (fp);

/*
 *  	Draw logo
 */

	s_logo_x = s_win_attr.width - s_logo_width - s_border_offset ; 
	s_logo_y = s_win_attr.height - s_logo_height - s_border_offset ; 

	logo_data_initialized = 1;
	
	DrawLogo();
}

static void
DrawLogo() {
/*
 *	Display logo X image
 */
	if (logo_data_initialized) {
	    XPutImage ( s_dpy, s_win , s_gc, s_logo_image, 0, 0, 
		        s_logo_x, s_logo_y, 
		        s_logo_clip_width, s_logo_height);
	}
}


/*******************************************************************************
 *
 *  Procedure:        Version.c
 *
 *  Description:
 *  -----------
 *  Draws dt version string 
 *
 ******************************************************************************/

void
Version ( XFontSet fontset )
{
	int x, y;
	XFontSetExtents *extents;

	/* adjust origin by font metric */

	x = VERSION_XOFFSET + s_win_attr.width/2 - XmbTextEscapement( fontset, 
		  SDtVERSION_STRING, strlen(SDtVERSION_STRING) )/2;
	y = VERSION_YOFFSET + s_win_attr.height/2 + s_star_height/2;

	extents = XExtentsOfFontSet(fontset);
	y += -(extents->max_logical_extent.y);

	if (y > s_win_attr.height - s_border_offset) 
	    y = s_win_attr.height - s_border_offset; 

	/* draw the version string */

	XmbDrawImageString ( s_dpy, s_win, fontset, s_gc, x, y, 
		             SDtVERSION_STRING, strlen(SDtVERSION_STRING) );
}


/*******************************************************************************
 *
 *  Procedure:        OpenImage
 *
 *  Description:
 *  -----------
 *  Find and open image file.
 *
 ******************************************************************************/

static FILE*
OpenImage(FILE **fp, char* base_name)
{
	char im_name[MAXPATHLEN];
	char im_pipe[MAXPATHLEN];
	struct stat sbuf;

/*
 *	Make raster image file name
 */
	strcpy(im_name, base_name);
	s_im_depth = s_win_depth;

	if (s_im_depth > 1) {
	    switch (s_visual_class) {
	    case PseudoColor:
	        if (s_win_depth == 8) 
		    strcat(im_name, "8");
		else
		    s_im_depth = 1;
		break;

	    case DirectColor:
	    case TrueColor:
		if (s_win_depth == 24)
		    strcat(im_name, "8");
		else 
		    s_im_depth = 1;
		break;

	    case GrayScale:
	    case StaticGray:
		if (s_win_depth == 8)
		    strcat(im_name, "8g");
		else
		    s_im_depth = 1;

		break;

	    default:
		s_im_depth = 1;
		break;
	    }
	}

	/* 
         * The 1-bit image is used for B/W monitors and also as a fallback 
	 * image for any rare visual/depth combinations not specifically
	 * accounted for in this code.
	 */

	if (s_im_depth == 1) strcat(im_name, "1");

/*
 *	Check if uncompressed file exists, if not, try compressed version
 */
	if (stat(im_name, &sbuf) == 0) {
	    *fp = fopen(im_name, "r");
	} else {
	    strcat(im_name, ".Z");
	    if (stat(im_name, &sbuf) == 0) {
	        sprintf(im_pipe, "/usr/bin/uncompress -c %s", im_name);
	        *fp = popen(im_pipe, "r");
	    } else *fp=0;
	}
}


/*******************************************************************************
 *
 *  Procedure:        ReadImageHeader
 *
 *  Description:
 *  -----------
 *  Read image header and color map data
 *
 ******************************************************************************/

static void
ReadImageHeader( FILE *fp,
		 int *read_width,
	    	 int *ras_width, 
	    	 int *ras_height, 
		 int *clip_width,
		 int *ncolors,
	    	 unsigned char **r,
	    	 unsigned char **g,
	    	 unsigned char **b,
	    	 unsigned char **im_cmap )
{
	struct rasterfile im_header;

	fread(&im_header, sizeof(im_header), 1, fp);

	im_header.ras_magic     = htonl(im_header.ras_magic);
    	im_header.ras_width     = htonl(im_header.ras_width);
    	im_header.ras_height    = htonl(im_header.ras_height);
    	im_header.ras_depth     = htonl(im_header.ras_depth);
    	im_header.ras_length    = htonl(im_header.ras_length);
    	im_header.ras_type      = htonl(im_header.ras_type);
    	im_header.ras_maptype   = htonl(im_header.ras_maptype);
    	im_header.ras_maplength = htonl(im_header.ras_maplength);

	if ( s_im_depth == 1 ) {
	    *read_width = (im_header.ras_width + 7)/8;
	    *ras_width = ((*read_width + 1) & 0xfffffffe) * 8; 
	    *clip_width = im_header.ras_width;
   	}
	else {
	    *read_width = (im_header.ras_width + 1 ) & 0xfffffffe; 
	    *ras_width = *clip_width = *read_width;
	}

	*ras_height = im_header.ras_height;

	*im_cmap = (unsigned char *) 
		   malloc ((unsigned) im_header.ras_maplength);

	fread ((char *) *im_cmap, im_header.ras_maplength, 1, fp);

	*ncolors = im_header.ras_maplength/3;

	*r = *im_cmap;
	*g = *r + *ncolors;
	*b = *g + *ncolors;

	/*
	 * Account for 24-bit reversed order plane group (BGR) devices.
	 */

	if ( (s_visual_class == DirectColor && s_win_depth == 24) ||
	     (s_visual_class == TrueColor && s_win_depth == 24) ) { 

	    if ( s_win_attr.visual->blue_mask  == 0xff &&
	         s_win_attr.visual->green_mask == 0xff00 && 
	         s_win_attr.visual->red_mask   == 0xff0000 ) {

	        unsigned char *tmp;

		tmp = *r;
		*r = *b;
		*b = tmp;
	    }
 	}
}


/*******************************************************************************
 *
 *  Procedure:        StoreColors
 *
 *  Description:
 *  -----------
 *  Store supplied colors in color map
 *
 ******************************************************************************/

static void
StoreColors( unsigned char* r, 
	     unsigned char* g, 
	     unsigned char* b, 
	     int ncolors,
	     int cindex_offset )
{
	XColor *xcolors;
	int i, pix;

	xcolors = (XColor*) malloc(ncolors*sizeof(XColor));

	if (s_win_attr.visual->class == DirectColor) {
	    for (i=0; i<ncolors; i++) {
	        pix = i + cindex_offset; 
		xcolors[i].pixel = pix | pix << 8 | pix << 16;
	        xcolors[i].red =   (*(r) << 8) + *r; r++;
	        xcolors[i].green = (*(g) << 8) + *g; g++;
	        xcolors[i].blue =  (*(b) << 8) + *b; b++;
		xcolors[i].flags = DoRed | DoGreen | DoBlue;
	    } 
	} else {
	    for (i=0; i<ncolors; i++) {
		xcolors[i].pixel = i + cindex_offset;
	        xcolors[i].red =   *(r++) << 8;
	        xcolors[i].green = *(g++) << 8;
	        xcolors[i].blue =  *(b++) << 8;
		xcolors[i].flags = DoRed | DoGreen | DoBlue;
	    } 
	}

	XStoreColors(s_dpy, s_xcmap, xcolors, ncolors);

	free(xcolors);
}


/*******************************************************************************
 *
 *  Procedure:        ReadImage
 *
 *  Description:
 *  -----------
 *  Read image data from file
 *
 ******************************************************************************/

static void
ReadImage( FILE *fp, 
	   int read_width,
	   unsigned char *r,
	   unsigned char *g,
	   unsigned char *b,
	   int im_width, 
	   int im_height, 
	   XImage **x_image,
	   int cindex_offset )
{
	unsigned char *im_data, *im_temp;
	int i, im_size, im_format;
        unsigned int pix;

	im_size = im_width * im_height;

	if ( (s_visual_class == TrueColor && s_win_depth == 24 ) ||
	     (s_visual_class == DirectColor && s_win_depth == 24) ) {
	    int *p;

	    /* convert 8-bit image data into 24-bit image data */

	    im_data = (unsigned char *) malloc ((unsigned) im_size * 4);
            p = (int*) im_data;

	    im_temp = (unsigned char *) malloc ((unsigned) im_size);
    	    fread ((char *) im_temp, im_size, 1, fp);

	    for (i=0; i<im_size; i++) {
		if (s_visual_class == DirectColor) {
                    pix = (unsigned int) im_temp[i] + cindex_offset;
		    *p++ = pix | pix << 8 | pix << 16;
		} else {
		    *p++ = r[im_temp[i]] |
		  	  (g[im_temp[i]] << 8) |
			  (b[im_temp[i]] << 16);
		}
	    }
	    free(im_temp);

	} else if ( s_visual_class == StaticGray && s_win_depth == 8) {
	    unsigned char *p;
	    p = im_data = (unsigned char *) malloc ((unsigned) im_size);

	    im_temp = (unsigned char *) malloc ((unsigned) im_size);
    	    fread ((char *) im_temp, im_size, 1, fp);

	    for (i=0; i<im_size; i++) {
	   	*p++ = r[im_temp[i]];
	    }

	    free(im_temp);

	} else if ( s_im_depth == 1 ) {
	    int j;
	    unsigned char *p;

	    p = im_data = (unsigned char *) malloc ((unsigned) im_size);

	    for (i=0; i < im_height; i++) {
		fread(p, read_width, 1, fp);
#ifdef _LITTLE_ENDIAN
		for (j=0; j < read_width; j++) {
		    p[j] = bytereverse[p[j]];
		}
#endif
		p += read_width;
	    }

	} else {
	    unsigned char *p;

	    p = im_data = (unsigned char *) malloc ((unsigned) im_size);

    	    fread ( (char *) im_data, im_size, 1, fp);

	    if (cindex_offset > 0) { 
		for (i=0; i<im_size; i++) {
		    *p++ = cindex_offset + *p;
	        }
 	    }
	}

	/*
	 *	Create starburst X image 
 	 */

	if ( s_im_depth == 1 ) {
	   im_format = XYBitmap;
	}
	else {
	   im_format = ZPixmap;
	}

	*x_image = XCreateImage ( s_dpy, s_win_attr.visual, s_im_depth, 
				  im_format, 0, im_data, 
		        	  im_width, im_height,
				  8, 0 );
}
