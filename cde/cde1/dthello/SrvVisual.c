/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "OSF/Motif: @(#)SrvVisual.c	1.12 95/02/26";
#endif /* lint */
#endif /* REV_INFO */
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1989, 1990, OPEN SOFTWARE FOUNDATION, INC.
*  (c) Copyright 1987, 1988, 1989, 1990, HEWLETT-PACKARD COMPANY
*  ALL RIGHTS RESERVED
*  
*  	THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED
*  AND COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND
*  WITH THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR
*  ANY OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
*  AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
*  SOFTWARE IS HEREBY TRANSFERRED.
*  
*  	THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
*  NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY OPEN SOFTWARE
*  FOUNDATION, INC. OR ITS THIRD PARTY SUPPLIERS  
*  
*  	OPEN SOFTWARE FOUNDATION, INC. AND ITS THIRD PARTY SUPPLIERS,
*  ASSUME NO RESPONSIBILITY FOR THE USE OR INABILITY TO USE ANY OF ITS
*  SOFTWARE .   OSF SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*  KIND, AND OSF EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES, INCLUDING
*  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
*  FITNESS FOR A PARTICULAR PURPOSE.
*  
*  Notice:  Notwithstanding any other lease or license that may pertain to,
*  or accompany the delivery of, this computer software, the rights of the
*  Government regarding its use, reproduction and disclosure are as set
*  forth in Section 52.227-19 of the FARS Computer Software-Restricted
*  Rights clause.
*  
*  (c) Copyright 1989, 1990, Open Software Foundation, Inc.  Unpublished - all
*  rights reserved under the Copyright laws of the United States.
*  
*  RESTRICTED RIGHTS NOTICE:  Use, duplication, or disclosure by the
*  Government is subject to the restrictions as set forth in subparagraph
*  (c)(1)(ii) of the Rights in Technical Data and Computer Software clause
*  at DFARS 52.227-7013.
*  
*  Open Software Foundation, Inc.
*  11 Cambridge Center
*  Cambridge, MA   02142
*  (617)621-8700
*  
*  RESTRICTED RIGHTS LEGEND:  This computer software is submitted with
*  "restricted rights."  Use, duplication or disclosure is subject to the
*  restrictions as set forth in NASA FAR SUP 18-52.227-79 (April 1985)
*  "Commercial Computer Software- Restricted Rights (April 1985)."  Open
*  Software Foundation, Inc., 11 Cambridge Center, Cambridge, MA  02142.  If
*  the contract contains the Clause at 18-52.227-74 "Rights in Data General"
*  then the "Alternate III" clause applies.
*  
*  (c) Copyright 1989, 1990, Open Software Foundation, Inc.
*  ALL RIGHTS RESERVED 
*  
*  
* Open Software Foundation is a trademark of The Open Software Foundation, Inc.
* OSF is a trademark of Open Software Foundation, Inc.
* OSF/Motif is a trademark of Open Software Foundation, Inc.
* Motif is a trademark of Open Software Foundation, Inc.
* DEC is a registered trademark of Digital Equipment Corporation
* DIGITAL is a registered trademark of Digital Equipment Corporation
* X Window System is a trademark of the Massachusetts Institute of Technology
*
*******************************************************************************
******************************************************************************/
#include <stdio.h>
#include <ctype.h>
#include <Xm/XmP.h>
#include <Xm/PrimitiveP.h>
#include <Xm/ManagerP.h>
#include <X11/ShellP.h>

/*  Defines and functions for processing dynamic defaults  */

#define XmMAX_SHORT 65535

#define BoundColor(value)\
	((value < 0) ? 0 : (((value > XmMAX_SHORT) ? XmMAX_SHORT : value)))

/* Contributions of each primary to overall luminosity, sum to 1.0 */

#define XmRED_LUMINOSITY 0.30
#define XmGREEN_LUMINOSITY 0.59
#define XmBLUE_LUMINOSITY 0.11

/* Percent effect of intensity, light, and luminosity & on brightness,
   sum to 100 */

#ifdef HP_COLOR
#define XmINTENSITY_FACTOR  75
#define XmLIGHT_FACTOR       0
#define XmLUMINOSITY_FACTOR 25
#else /* HP_COLOR */
#define XmINTENSITY_FACTOR  25
#define XmLIGHT_FACTOR       0
#define XmLUMINOSITY_FACTOR 75
#endif /* HP_COLOR */

/* Thresholds for brightness
   above LITE threshold, LITE color model is used
   below DARK threshold, DARK color model is be used
   use STD color model in between */

#ifdef HP_COLOR
#define XmCOLOR_LITE_THRESHOLD (0.93 * XmMAX_SHORT)
#else /* HP_COLOR */
#define XmCOLOR_LITE_THRESHOLD (0.77 * XmMAX_SHORT)
#endif /* HP_COLOR */

#ifdef HP_COLOR
#define XmFOREGROUND_THRESHOLD (0.70 * XmMAX_SHORT)
#else /* HP_COLOR */
#define XmFOREGROUND_THRESHOLD (0.35 * XmMAX_SHORT)
#endif /* HP_COLOR */

#ifdef HP_COLOR
#define XmCOLOR_DARK_THRESHOLD (0.20 * XmMAX_SHORT)
#else /* HP_COLOR */
#define XmCOLOR_DARK_THRESHOLD (0.15 * XmMAX_SHORT)
#endif /* HP_COLOR */

/* LITE color model
   percent to interpolate RGB towards black for SEL, BS, TS */

#ifdef HP_COLOR
#define XmCOLOR_LITE_SEL_FACTOR   15
#define XmCOLOR_LITE_BS_FACTOR   40
#define XmCOLOR_LITE_TS_FACTOR   20
#else /* HP_COLOR */
#define XmCOLOR_LITE_SEL_FACTOR   15
#define XmCOLOR_LITE_BS_FACTOR   45
#define XmCOLOR_LITE_TS_FACTOR   20
#endif /* HP_COLOR */

/* DARK color model
   percent to interpolate RGB towards white for SEL, BS, TS */

#define XmCOLOR_DARK_SEL_FACTOR   15
#define XmCOLOR_DARK_BS_FACTOR   30
#define XmCOLOR_DARK_TS_FACTOR   50

/* STD color model
   percent to interpolate RGB towards black for SEL, BS
   percent to interpolate RGB towards white for TS
   HI values used for high brightness (within STD)
   LO values used for low brightness (within STD)
   Interpolate factors between HI & LO values based on brightness */

#ifdef HP_COLOR
#define XmCOLOR_HI_SEL_FACTOR   15
#define XmCOLOR_HI_BS_FACTOR   40
#define XmCOLOR_HI_TS_FACTOR   60
#else /* HP_COLOR */
#define XmCOLOR_HI_SEL_FACTOR   15
#define XmCOLOR_HI_BS_FACTOR   35
#define XmCOLOR_HI_TS_FACTOR   70
#endif /* HP_COLOR */

#ifdef HP_COLOR
#define XmCOLOR_LO_SEL_FACTOR   15
#define XmCOLOR_LO_BS_FACTOR   60
#define XmCOLOR_LO_TS_FACTOR   50
#else /* HP_COLOR */
#define XmCOLOR_LO_SEL_FACTOR   15
#define XmCOLOR_LO_BS_FACTOR    60
#define XmCOLOR_LO_TS_FACTOR   40
#endif /* HP_COLOR */

/*
 * GLOBAL VARIABLES
 *
 * These variables define the color cache.
 */
static int Set_Count=0, Set_Size=0;
static XmColorData *Color_Set=NULL;

static String default_background_color_spec = "#729FFF";
static Boolean app_defined = FALSE;

/*  Name set by background pixmap converter, used and cleared  */
/*  Primitive and Manager Initialize procedure.                */

static char * background_pixmap_name = NULL;


/*  Foreward declarations. */

#ifdef _NO_PROTO
static void _XmCvtStringToBackgroundPixmap() ;
static void CvtStringToPixmap() ;
static void _XmCvtStringToGadgetPixmap() ;
static void _XmCvtStringToPrimForegroundPixmap() ;
static void _XmCvtStringToPrimHighlightPixmap() ;
static void _XmCvtStringToPrimTopShadowPixmap() ;
static void _XmCvtStringToPrimBottomShadowPixmap() ;
static void _XmCvtStringToManForegroundPixmap() ;
static void _XmCvtStringToManHighlightPixmap() ;
static void _XmCvtStringToManTopShadowPixmap() ;
static void _XmCvtStringToManBottomShadowPixmap() ;
static void _XmGetDynamicDefault() ;
static void SetMonochromeColors() ;
static int _XmBrightness() ;
static void CalculateColorsForLightBackground() ;
static void CalculateColorsForDarkBackground() ;
static void CalculateColorsForMediumBackground() ;
static void _XmCalculateColorsRGB() ;
#else
static void _XmCvtStringToBackgroundPixmap( 
                        XrmValue *args,
                        Cardinal numArgs,
                        XrmValue *fromVal,
                        XrmValue *toVal) ;
static void CvtStringToPixmap( 
                        XrmValue *args,
                        Cardinal numArgs,
                        XrmValue *fromVal,
                        XrmValue *toVal) ;
static void _XmCvtStringToGadgetPixmap( 
                        XrmValue *args,
                        Cardinal numArgs,
                        XrmValue *fromVal,
                        XrmValue *toVal) ;
static void _XmCvtStringToPrimForegroundPixmap( 
                        XrmValue *args,
                        Cardinal numArgs,
                        XrmValue *fromVal,
                        XrmValue *toVal) ;
static void _XmCvtStringToPrimHighlightPixmap( 
                        XrmValue *args,
                        Cardinal numArgs,
                        XrmValue *fromVal,
                        XrmValue *toVal) ;
static void _XmCvtStringToPrimTopShadowPixmap( 
                        XrmValue *args,
                        Cardinal numArgs,
                        XrmValue *fromVal,
                        XrmValue *toVal) ;
static void _XmCvtStringToPrimBottomShadowPixmap( 
                        XrmValue *args,
                        Cardinal numArgs,
                        XrmValue *fromVal,
                        XrmValue *toVal) ;
static void _XmCvtStringToManForegroundPixmap( 
                        XrmValue *args,
                        Cardinal numArgs,
                        XrmValue *fromVal,
                        XrmValue *toVal) ;
static void _XmCvtStringToManHighlightPixmap( 
                        XrmValue *args,
                        Cardinal numArgs,
                        XrmValue *fromVal,
                        XrmValue *toVal) ;
static void _XmCvtStringToManTopShadowPixmap( 
                        XrmValue *args,
                        Cardinal numArgs,
                        XrmValue *fromVal,
                        XrmValue *toVal) ;
static void _XmCvtStringToManBottomShadowPixmap( 
                        XrmValue *args,
                        Cardinal numArgs,
                        XrmValue *fromVal,
                        XrmValue *toVal) ;
static void _XmGetDynamicDefault( 
                        Widget widget,
                        int type,
                        int offset,
                        XrmValue *value) ;
static void SetMonochromeColors( 
                        XmColorData *colors) ;
static int _XmBrightness( 
                        XColor *color) ;
static void CalculateColorsForLightBackground( 
                        XColor *bg_color,
                        XColor *fg_color,
                        XColor *sel_color,
                        XColor *ts_color,
                        XColor *bs_color) ;
static void CalculateColorsForDarkBackground( 
                        XColor *bg_color,
                        XColor *fg_color,
                        XColor *sel_color,
                        XColor *ts_color,
                        XColor *bs_color) ;
static void CalculateColorsForMediumBackground( 
                        XColor *bg_color,
                        XColor *fg_color,
                        XColor *sel_color,
                        XColor *ts_color,
                        XColor *bs_color) ;
static void _XmCalculateColorsRGB( 
                        XColor *bg_color,
                        XColor *fg_color,
                        XColor *sel_color,
                        XColor *ts_color,
                        XColor *bs_color) ;
#endif /* _NO_PROTO */


static XmColorProc ColorRGBCalcProc = _XmCalculateColorsRGB;


/*  Argument lists sent down to the pixmap converter functions  */

static XtConvertArgRec backgroundArgs[] =
{
   { XtBaseOffset, (caddr_t) 0, sizeof (int) },
};

static XtConvertArgRec primForegroundArgs[] =
{
   { XtBaseOffset, (caddr_t) XtOffset(Widget, core.screen), sizeof (Screen*) },
   { XtBaseOffset, (caddr_t) XtOffset(XmPrimitiveWidget,
primitive.foreground), sizeof (Pixel) },
   { XtBaseOffset, (caddr_t) XtOffset(Widget, core.background_pixel),
sizeof (Pixel) }
};

static XtConvertArgRec primHighlightArgs[] =
{
   { XtBaseOffset, (caddr_t) XtOffset(Widget, core.screen), sizeof (Screen*) },
   { XtBaseOffset, (caddr_t) XtOffset(XmPrimitiveWidget,
primitive.highlight_color), sizeof (Pixel) },
   { XtBaseOffset, (caddr_t) XtOffset(Widget, core.background_pixel),
sizeof (Pixel) }
};

static XtConvertArgRec primTopShadowArgs[] =
{
   { XtBaseOffset, (caddr_t) XtOffset(Widget, core.screen), sizeof (Screen*) },
   { XtBaseOffset, (caddr_t) XtOffset (XmPrimitiveWidget,
primitive.top_shadow_color), sizeof (Pixel) },
   { XtBaseOffset, (caddr_t) XtOffset (Widget, core.background_pixel),
sizeof (Pixel) }
};

static XtConvertArgRec primBottomShadowArgs[] =
{
   { XtBaseOffset, (caddr_t) XtOffset(Widget, core.screen), sizeof (Screen*) },
   { XtBaseOffset, (caddr_t) XtOffset (XmPrimitiveWidget,
primitive.bottom_shadow_color), sizeof (Pixel) },
   { XtBaseOffset, (caddr_t) XtOffset (Widget, core.background_pixel),
sizeof (Pixel) }
};

static XtConvertArgRec manForegroundArgs[] =
{
   { XtWidgetBaseOffset, (caddr_t) XtOffset(Widget, core.screen),
sizeof (Screen*) },
   { XtWidgetBaseOffset, (caddr_t) XtOffset(XmManagerWidget,
manager.foreground), sizeof (Pixel) },
   { XtWidgetBaseOffset, (caddr_t) XtOffset(Widget,
core.background_pixel), sizeof (Pixel) }
};

static XtConvertArgRec manHighlightArgs[] =
{
   { XtWidgetBaseOffset, (caddr_t) XtOffset(Widget, core.screen),
sizeof (Screen*) },
   { XtWidgetBaseOffset, (caddr_t) XtOffset(XmManagerWidget,
manager.highlight_color), sizeof (Pixel) },
   { XtWidgetBaseOffset, (caddr_t) XtOffset(Widget,
core.background_pixel), sizeof (Pixel) }
};

static XtConvertArgRec manTopShadowArgs[] =
{
   { XtWidgetBaseOffset, (caddr_t)XtOffset(Widget, core.screen), sizeof (Screen*) },
   { XtWidgetBaseOffset, (caddr_t) XtOffset (XmManagerWidget,
manager.top_shadow_color), sizeof (Pixel) },
   { XtWidgetBaseOffset, (caddr_t) XtOffset (Widget,
core.background_pixel), sizeof (Pixel) }
};

static XtConvertArgRec manBottomShadowArgs[] =
{
   { XtWidgetBaseOffset, (caddr_t) XtOffset(Widget, core.screen),
sizeof (Screen*) },
   { XtWidgetBaseOffset, (caddr_t) XtOffset (XmManagerWidget,
manager.bottom_shadow_color), sizeof (Pixel) },
   { XtWidgetBaseOffset, (caddr_t) XtOffset (Widget,
core.background_pixel), sizeof (Pixel) }
};

static XtConvertArgRec gadgetPixmapArgs[] =
{
   { XtWidgetBaseOffset, (caddr_t) XtOffset(Widget, core.parent),
sizeof (Widget ) }
};

/***** hhhhhhhhhhhhhhhhh      *************/
XmColorProc 
#ifdef _NO_PROTO
XmGetColorCalculation()
#else
XmGetColorCalculation( void )
#endif /* _NO_PROTO */
{
	return(ColorRGBCalcProc);
}



XmColorProc 
#ifdef _NO_PROTO
XmSetColorCalculation( proc )
        XmColorProc proc ;
#else
XmSetColorCalculation(
        XmColorProc proc )
#endif /* _NO_PROTO */
{
	XmColorProc a = ColorRGBCalcProc;

	if (proc != NULL)
		ColorRGBCalcProc = proc;
	else
		ColorRGBCalcProc = _XmCalculateColorsRGB;
	
	return(a);
}

static void 
#ifdef _NO_PROTO
_XmCalculateColorsRGB( bg_color, fg_color, sel_color, ts_color, bs_color )
        XColor *bg_color ;
        XColor *fg_color ;
        XColor *sel_color ;
        XColor *ts_color ;
        XColor *bs_color ;
#else
_XmCalculateColorsRGB(
        XColor *bg_color,
        XColor *fg_color,
        XColor *sel_color,
        XColor *ts_color,
        XColor *bs_color )
#endif /* _NO_PROTO */
{
	int brightness = _XmBrightness(bg_color);

	if (brightness < XmCOLOR_DARK_THRESHOLD)
		CalculateColorsForDarkBackground(bg_color, fg_color,
			sel_color, ts_color, bs_color);
	else if (brightness > XmCOLOR_LITE_THRESHOLD)
		CalculateColorsForLightBackground(bg_color, fg_color,
			sel_color, ts_color, bs_color);
	else
		CalculateColorsForMediumBackground(bg_color, fg_color,
			sel_color, ts_color, bs_color);
}
	
static void 
#ifdef _NO_PROTO
CalculateColorsForDarkBackground( bg_color, fg_color, sel_color, ts_color, bs_color )
        XColor *bg_color ;
        XColor *fg_color ;
        XColor *sel_color ;
        XColor *ts_color ;
        XColor *bs_color ;
#else
CalculateColorsForDarkBackground(
        XColor *bg_color,
        XColor *fg_color,
        XColor *sel_color,
        XColor *ts_color,
        XColor *bs_color )
#endif /* _NO_PROTO */
{
	int color_value;

	if (fg_color)
	{
		fg_color->red = XmMAX_SHORT;
		fg_color->green = XmMAX_SHORT;
		fg_color->blue = XmMAX_SHORT;
	}

	if (sel_color)
	{
		color_value = bg_color->red;
		color_value += XmCOLOR_DARK_SEL_FACTOR *
			(XmMAX_SHORT - color_value) / 100;
		sel_color->red = color_value;

		color_value = bg_color->green;
		color_value += XmCOLOR_DARK_SEL_FACTOR *
			(XmMAX_SHORT - color_value) / 100;
		sel_color->green = color_value;

		color_value = bg_color->blue;
		color_value += XmCOLOR_DARK_SEL_FACTOR *
			(XmMAX_SHORT - color_value) / 100;
		sel_color->blue = color_value;
	}

	if (bs_color)
	{
		color_value = bg_color->red;
		color_value += XmCOLOR_DARK_BS_FACTOR *
			(XmMAX_SHORT - color_value) / 100;
		bs_color->red = color_value;

		color_value = bg_color->green;
		color_value += XmCOLOR_DARK_BS_FACTOR *
			(XmMAX_SHORT - color_value) / 100;
		bs_color->green = color_value;

		color_value = bg_color->blue;
		color_value += XmCOLOR_DARK_BS_FACTOR *
			(XmMAX_SHORT - color_value) / 100;
		bs_color->blue = color_value;
	}

	if (ts_color)
	{
		color_value = bg_color->red;
		color_value += XmCOLOR_DARK_TS_FACTOR *
			(XmMAX_SHORT - color_value) / 100;
		ts_color->red = color_value;

		color_value = bg_color->green;
		color_value += XmCOLOR_DARK_TS_FACTOR *
			(XmMAX_SHORT - color_value) / 100;
		ts_color->green = color_value;

		color_value = bg_color->blue;
		color_value += XmCOLOR_DARK_TS_FACTOR *
			(XmMAX_SHORT - color_value) / 100;
		ts_color->blue = color_value;
	}
}


static void 
#ifdef _NO_PROTO
CalculateColorsForLightBackground( bg_color, fg_color, sel_color, ts_color, bs_color )
        XColor *bg_color ;
        XColor *fg_color ;
        XColor *sel_color ;
        XColor *ts_color ;
        XColor *bs_color ;
#else
CalculateColorsForLightBackground(
        XColor *bg_color,
        XColor *fg_color,
        XColor *sel_color,
        XColor *ts_color,
        XColor *bs_color )
#endif /* _NO_PROTO */
{
	int color_value;

	if (fg_color)
	{
		fg_color->red = 0;
		fg_color->green = 0;
		fg_color->blue = 0;
	}

	if (sel_color)
	{
		color_value = bg_color->red;
		color_value -= (color_value * XmCOLOR_LITE_SEL_FACTOR) / 100;
		sel_color->red = color_value;

		color_value = bg_color->green;
		color_value -= (color_value * XmCOLOR_LITE_SEL_FACTOR) / 100;
		sel_color->green = color_value;

		color_value = bg_color->blue;
		color_value -= (color_value * XmCOLOR_LITE_SEL_FACTOR) / 100;
		sel_color->blue = color_value;
	}

	if (bs_color)
	{
		color_value = bg_color->red;
		color_value -= (color_value * XmCOLOR_LITE_BS_FACTOR) / 100;
		bs_color->red = color_value;

		color_value = bg_color->green;
		color_value -= (color_value * XmCOLOR_LITE_BS_FACTOR) / 100;
		bs_color->green = color_value;

		color_value = bg_color->blue;
		color_value -= (color_value * XmCOLOR_LITE_BS_FACTOR) / 100;
		bs_color->blue = color_value;
	}

	if (ts_color)
	{
		color_value = bg_color->red;
		color_value -= (color_value * XmCOLOR_LITE_TS_FACTOR) / 100;
		ts_color->red = color_value;

		color_value = bg_color->green;
		color_value -= (color_value * XmCOLOR_LITE_TS_FACTOR) / 100;
		ts_color->green = color_value;

		color_value = bg_color->blue;
		color_value -= (color_value * XmCOLOR_LITE_TS_FACTOR) / 100;
		ts_color->blue = color_value;
	}
}

static void 
#ifdef _NO_PROTO
CalculateColorsForMediumBackground( bg_color, fg_color, sel_color, ts_color, bs_color )
        XColor *bg_color ;
        XColor *fg_color ;
        XColor *sel_color ;
        XColor *ts_color ;
        XColor *bs_color ;
#else
CalculateColorsForMediumBackground(
        XColor *bg_color,
        XColor *fg_color,
        XColor *sel_color,
        XColor *ts_color,
        XColor *bs_color )
#endif /* _NO_PROTO */
{
	int brightness = _XmBrightness(bg_color);
	int color_value, f;

	if (brightness > XmFOREGROUND_THRESHOLD)
	{
		fg_color->red = 0;
		fg_color->green = 0;
		fg_color->blue = 0;
	}
	else
	{
		fg_color->red = XmMAX_SHORT;
		fg_color->green = XmMAX_SHORT;
		fg_color->blue = XmMAX_SHORT;
	}

	if (sel_color)
	{
		f = XmCOLOR_LO_SEL_FACTOR + (brightness
			* ( XmCOLOR_HI_SEL_FACTOR - XmCOLOR_LO_SEL_FACTOR )
			/ XmMAX_SHORT );

		color_value = bg_color->red;
		color_value -= (color_value * f) / 100;
		sel_color->red = color_value;

		color_value = bg_color->green;
		color_value -= (color_value * f) / 100;
		sel_color->green = color_value;

		color_value = bg_color->blue;
		color_value -= (color_value * f) / 100;
		sel_color->blue = color_value;
	}

	if (bs_color)
	{
		f = XmCOLOR_LO_BS_FACTOR + (brightness 
			* ( XmCOLOR_HI_BS_FACTOR - XmCOLOR_LO_BS_FACTOR )
			/ XmMAX_SHORT);

		color_value = bg_color->red;
		color_value -= (color_value * f) / 100;
		bs_color->red = color_value;

		color_value = bg_color->green;
		color_value -= (color_value * f) / 100;
		bs_color->green = color_value;

		color_value = bg_color->blue;
		color_value -= (color_value * f) / 100;
		bs_color->blue = color_value;
	}

	if (ts_color)
	{
		f = XmCOLOR_LO_TS_FACTOR + (brightness
			* ( XmCOLOR_HI_TS_FACTOR - XmCOLOR_LO_TS_FACTOR )
			/ XmMAX_SHORT);

		color_value = bg_color->red;
		color_value += f * ( XmMAX_SHORT - color_value ) / 100;
		ts_color->red = color_value;

		color_value = bg_color->green;
		color_value += f * ( XmMAX_SHORT - color_value ) / 100;
		ts_color->green = color_value;

		color_value = bg_color->blue;
		color_value += f * ( XmMAX_SHORT - color_value ) / 100;
		ts_color->blue = color_value;
	}
}

static int 
#ifdef _NO_PROTO
_XmBrightness( color )
        XColor *color ;
#else
_XmBrightness(
        XColor *color )
#endif /* _NO_PROTO */
{
	int brightness;
	int intensity;
	int light;
	int luminosity, maxprimary, minprimary;
	int red = color->red;
	int green = color->green;
	int blue = color->blue;

	intensity = (red + green + blue) / 3;

	/* 
	 * The casting nonsense below is to try to control the point at
	 * the truncation occurs.
	 */

	luminosity = (int) ((XmRED_LUMINOSITY * (float) red)
		+ (XmGREEN_LUMINOSITY * (float) green)
		+ (XmBLUE_LUMINOSITY * (float) blue));

	maxprimary = ( (red > green) ?
					( (red > blue) ? red : blue ) :
					( (green > blue) ? green : blue ) );

	minprimary = ( (red < green) ?
					( (red < blue) ? red : blue ) :
					( (green < blue) ? green : blue ) );

	light = (minprimary + maxprimary) / 2;

	brightness = ( (intensity * XmINTENSITY_FACTOR) +
				   (light * XmLIGHT_FACTOR) +
				   (luminosity * XmLUMINOSITY_FACTOR) ) / 100;
	return(brightness);
}

/***** eeeeeeeeeeeeeeeeee     *************/

