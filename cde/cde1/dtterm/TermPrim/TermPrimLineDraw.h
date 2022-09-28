/*
 * $XConsortium: TermPrimLineDraw.h /main/cde1_maint/1 1995/07/15 01:27:19 drk $";
 */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
#ifndef	_Dt_TermPrimLineDraw_h
#define	_Dt_TermPrimLineDraw_h

typedef struct _LineDrawFontRec *LineDrawFont;

LineDrawFont _DtTermPrimLineDrawCreateFont(Widget w, GlyphInfo glyphInfo,
	int numGlyphs,  int width, int ascent, int descent);
void _DtTermPrimLineDrawFreeFont(LineDrawFont lineDrawFont);
void _DtTermPrimLineDrawImageString(Display *display, Drawable d,
	LineDrawFont lineDrawFont,
	GC gc, GC clearGC, int x, int y, unsigned char *string, int length);
#endif	/* _Dt_TermPrimLineDraw_h */
