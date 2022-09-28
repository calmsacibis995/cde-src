/* $XConsortium: GraphicsI.h /main/cde1_maint/1 1995/07/17 17:34:29 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        GraphicsI.h
 **
 **   Project:     Cde Help System
 **
 **   Description: Internal header file for Graphics.c
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

#ifndef _DtGraphicsI_h
#define _DtGraphicsI_h

/*
 * Semi-Public Routines
 */
#ifdef _NO_PROTO
extern	Pixmap	 _DtHelpProcessGraphic() ;
#else
extern	Pixmap	 _DtHelpProcessGraphic(
			Display         *dpy,
			Drawable         drawable,
			int		 screen,
			int		 depth,
			GC               gc,
			Pixmap          *def_pix,
			Dimension       *def_pix_width,
			Dimension       *def_pix_height,
			ilXWC           *tiff_xwc,
			Colormap	 colormap,
			Visual		*visual,
			Pixel		 fore_ground,
			Pixel		 back_ground,
			char            *filename,
			Dimension       *pixmapWidth,
			Dimension       *pixmapHeight,
			Pixmap		*ret_mask,
			Pixel          **ret_colors,
			int             *ret_number );
#endif /* _NO_PROTO */

#endif /* _DtGraphisI.h */
