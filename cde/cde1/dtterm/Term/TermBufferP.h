/* 
** $Header: TermBufferP.h,v 1.11 94/12/21 10:46:38 tom_hp_cv Exp $
*/

/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef   _Dt_TermBufferP_h
#define   _Dt_TermBufferP_h
#include  "TermBuffer.h"
#include  "TermPrimBufferP.h"

#ifdef	   __cplusplus
extern "C" {
#endif	/* __cplusplus */

/* 
** This is the enhancement structure, one per character.
*/
typedef struct _DtEnh
{
    /* 
    ** The video enhancements.
    */
    unsigned char   video;

    /*  
    ** FIELD_UNPROTECT  0
    ** FIELD_PROTECT    1
    ** FIELD_TRANSMIT   2
    ** FIELD_END        3
    */
    unsigned char   field;

    /* 
    ** index into color[0..15], on for foreground, on for background
    */
    unsigned char   fgColor;
    unsigned char   bgColor;

    /* 
    ** FONT_NORMAL   0
    ** FONT_LINEDRAW 1
    ** FONT_GLYPH    3
    */
    unsigned char   font;
} DtTermEnhPart, *DtEnh;

/* 
** This a line, the character buffer, and the enhancements.
*/
typedef struct _DtLinePart
{
    DtEnh      enh;
} DtLinePart;

typedef struct _DtTermLineRec
{
    TermLinePart    term_line;
    DtLinePart      dt_line;
} DtTermLineRec, *DtLine;

typedef struct _DtTermBufferPart
{
    DtTermEnhPart   enhState;
    Boolean         enhDirty;   /* true if the enhState is non default */
} DtTermBufferPart;

typedef struct _DtTermBufferRec
{
    TermBufferPart      term_buffer;
    DtTermBufferPart    dt_buffer;
} DtTermBufferRec, *DtTermBuffer;

/* 
** Define some useful DT counter parts for the TermPrimBufferP.h macros
*/
#define DT_LINE_OF_TBUF(tb, row) ((DtLine)(LINE_OF_TBUF(tb, row)))
#define DT_LINES(tb)             ((DtLine*)(LINES(tb)))
#define DT_ENH_STATE(tb)         (((DtTermBuffer)(tb))->dt_buffer.enhState)
#define DT_ENH_DIRTY(tb)         (((DtTermBuffer)(tb))->dt_buffer.enhDirty)
#define DT_ENH(line)             ((line)->dt_line.enh)
#ifdef	__cplusplus
} /* close scope of 'extern "C"'... */
#endif	/* __cplusplus */

#endif /* _Dt_TermBufferP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif... */
