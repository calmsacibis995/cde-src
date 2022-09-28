/*******************************************************************************
**
**  "@(#)solaris.h 1.14 97/06/18 
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

/*******************************************************************************
 *
 *  File:        solaris.h
 *
 *  Description:
 *  -----------
 *  Header file for code drawng the Solaris starburst and related information
 *  on the dthello window. 
 *
 ******************************************************************************/

#define STARIMAGE "/usr/dt/config/images/Solaris.im"
#define ADOBEIMAGE "/usr/dt/config/images/Adobe.im"

#define VERSION_XOFFSET 90
#define VERSION_YOFFSET  5

#ifdef _NO_PROTO

extern void Starburst() ;
extern void Logo() ;
extern void Version() ;

static FILE* OpenImage() ;
static void ReadImageHeader() ;
static void StoreColors() ;
static void ReadImage() ;
static void DrawStar() ;
static void DrawLogo() ;

#else

extern void Starburst( Display* dpy, 
		       Window win, 
		       GC *gc, 
		       char* fgColor,
		       char* bgColor,
		       int* ncolors );

extern void Logo( int border_offset, int cindex_offset );
extern void Version( XFontSet fontset ); 

static FILE* OpenImage( FILE **fp,
			char* base_name ); 

static void ReadImageHeader( FILE *fp,
			     int *read_width,
	    	 	     int *im_width, 
	    	 	     int *im_height, 
	    	 	     int *clip_width, 
			     int *ncolors,
	    	 	     unsigned char **r,
	    	 	     unsigned char **g,
	    	 	     unsigned char **b,
	    	 	     unsigned char **im_cmap );

static void StoreColors( unsigned char* r, 
			 unsigned char* g, 
			 unsigned char* b, 
			 int ncolors,
			 int cindex_offset );

static void ReadImage( FILE *fp, 
	 	       int read_width,	
		       unsigned char* r, 
		       unsigned char* g, 
		       unsigned char* b, 
		       int im_width,
		       int im_height,
		       XImage **x_image,
		       int cindex_offset );

static void DrawStar() ;
static void DrawLogo() ;

#endif /* _NO_PROTO */
