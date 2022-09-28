#ifndef lint
#ifdef  VERBOSE_REV_INFO
/* this file contains rendering functions for the "glyph font" */
/* hacked up from: */
static char rcs_id[] = "$Header: TermPrimRenderFont.c,v 1.6 94/12/21 10:57:06 tom_hp_cv Exp $";
#endif  /* VERBOSE_REV_INFO */
#endif  /* lint */

/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#include "TermHeader.h"
#include "TermPrimP.h"
#include "TermPrimDebug.h"
#include "TermPrimRenderP.h"
#include "TermPrimRenderFont.h"
#include "TermGlyphFont.h"

static void
GlyphFontRenderFunction(
    Widget		  w,
    TermFont		  font,
    Pixel		  fg,
    Pixel		  bg,
    unsigned long	  flags,
    int			  x,
    int			  y,
    unsigned char	 *string,
    int			  len
)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    struct termData *tpd = tw->term.tpd;
    XGCValues values;
    unsigned long valueMask;
    int i;
    int xx;
    int yy;

    /* XFontStruct *fontStruct = (XFontStruct *) font->fontInfo; */
    GlyphFont *gf = *(GlyphFont **)font->fontInfo;

    /* set the renderGC... */
    valueMask = (unsigned long) 0;

    /* set the foreground... */
    if (TermIS_SECURE(flags)) {
	if (tpd->renderGC.foreground != bg) {
	    tpd->renderGC.foreground = bg;
	    values.foreground = bg;
	    valueMask |= GCForeground;
	}
    } else {
	if (tpd->renderGC.foreground != fg) {
	    tpd->renderGC.foreground = fg;
	    values.foreground = fg;
	    valueMask |= GCForeground;
	}
    }

    /* set background... */
    if (tpd->renderGC.background != bg) {
	tpd->renderGC.background = bg;
	values.background = bg;
	valueMask |= GCBackground;
    }

    if (valueMask) {
	(void) XChangeGC(XtDisplay(w), tpd->renderGC.gc, valueMask,
		&values);
    }

    /* draw image string a line of text... */
    if (isDebugFSet('t', 1)) {
#ifdef	BBA
#pragma BBA_IGNORE
#endif	/*BBA*/
	/* Fill in the text area so we can see what is going to
	 * be displayed...
	 */
	(void) XFillRectangle(XtDisplay(w),
		XtWindow(w),
		tpd->renderGC.gc,
		x,
		y,
		gf->maxWidth * len,
		tpd->cellHeight);
	(void) XSync(XtDisplay(w), False);
	(void) shortSleep(100000);
    }
			
    if ( gf ) {
	if ( tw->term.glyphGutter > 0 ) {
	    /* x = tpd->offsetX; / * force first glyph to be in gutter */
	    /* clear gutter */
	    if ( x < tw->term.glyphGutter )
	      (void) XFillRectangle(XtDisplay(w),
		XtWindow(w),
		tpd->clearGC.gc,
		x,
		y,
		tw->term.glyphGutter,
		gf->maxHeight);
	}
	xx = x;
        yy = y + (tpd->cellHeight - gf->maxHeight) / 2;
	for ( i = 0; i < len; i++ ) {
	    int c = string[i] - gf->baseChar;
	    Glyph *g = 0;
	    if ( c >= 0 && c < gf->numGlyphs )
		    g = &gf->data[c];
	    if ( g && g->pixmap != NULL ) {
		(void) XCopyArea(XtDisplay(w),	/* Display		*/
		     g->pixmap,  		/* src			*/
		     XtWindow(w),		/* dest			*/
		     tpd->renderGC.gc,		/* GC			*/
		     0,				/* src_x		*/
		     0,				/* src_y		*/
		     g->width,			/* width		*/
		     g->height,			/* height		*/
		     xx + g->xoff,		/* dest_x		*/
		     yy + g->yoff);		/* dest_y		*/
	    }
	    xx += tpd->cellWidth;
	}
    }
		     
    /* ignore overstrike for glyphs */

    /* handle the underline enhancement... */
    /* draw the underline, unless in the gutter... */
    if (TermIS_UNDERLINE(flags) && tw->term.glyphGutter == 0) {
	XDrawLine(XtDisplay(w),			/* Display		*/
		XtWindow(w),			/* Window		*/
		tpd->renderGC.gc,		/* GC			*/
		x,				/* X1			*/
		y + tpd->cellHeight - 1,		/* Y1			*/
		x + len * tpd->cellWidth,	/* X2			*/
		y + tpd->cellHeight - 1);	/* Y2			*/
    }
}

static void
GlyphFontDestroyFunction(
    Widget		  w,
    TermFont		  font
)
{
    (void) XtFree((char *) font);
}

static void
GlyphFontExtentsFunction(
    Widget		  w,
    TermFont		  font,
    unsigned char	 *string,
    int			  len,
    int			 *widthReturn,
    int			 *heightReturn,
    int			 *ascentReturn
)
{
    /* XFontStruct *fontStruct = (XFontStruct *) font->fontInfo; */
    GlyphFont *gf = *(GlyphFont **)font->fontInfo;

    if (widthReturn) {
        /* *widthReturn = len * fontStruct->max_bounds.width; */
	*widthReturn = len * gf->maxWidth;
    }
    if (heightReturn) {
	/* *heightReturn = fontStruct->ascent + fontStruct->descent; */
	*heightReturn = gf->maxHeight;
    }
    if (ascentReturn) {
	/* *ascentReturn = fontStruct->ascent; */
	*ascentReturn = gf->maxHeight;
    }
    return;
}

extern TermFont glyphFont;

TermFont
_DtTermPrimRenderGlyphFontCreate(
    Widget		  w,
    GlyphFont		 **font
)
{
    TermFont termFont;
    GlyphFont *gf;

    termFont = (TermFont) XtMalloc(sizeof(TermFontRec));
    termFont->renderFunction = GlyphFontRenderFunction;
    termFont->destroyFunction = GlyphFontDestroyFunction;
    termFont->extentsFunction = GlyphFontExtentsFunction;
    termFont->fontInfo = (XtPointer) font;

    glyphFont = termFont;
    return(termFont);
}
