/*
 * $XConsortium: TermPrimRenderLineDraw.h /main/cde1_maint/1 1995/07/15 01:32:57 drk $";
 */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
#ifndef	_Dt_TermPrimRenderLineDraw_h
#define	_Dt_TermPrimRenderLineDraw_h

#include "TermPrimRender.h"

extern TermFont _DtTermPrimRenderLineDrawCreate(
    Widget		  w,
    GlyphInfo		  glyphInfo,
    int			  numGlyphs,
    int			  width,
    int			  ascent,
    int			  descent
);

#endif	/* _Dt_TermPrimRenderLineDraw_h */
/* DON'T ADD ANYTHING AFTER THIS #endif... */
