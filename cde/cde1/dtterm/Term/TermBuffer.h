/* 
** $Header: TermBuffer.h,v 1.10 94/12/21 10:46:36 tom_hp_cv Exp $
*/

/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef   _Dt_TermBuffer_h
#define   _Dt_TermBuffer_h
#include <TermPrimBuffer.h>

/*
** Defines for video enhancements
*/
#define	BOLD	     (1 << 5)
#define SECURE       (1 << 4)
#define HALF_BRIGHT  (1 << 3)
#define UNDERLINE    (1 << 2)
#define INVERSE      (1 << 1)
#define BLINK        (1 << 0)
#define VIDEO_MASK  (BOLD | SECURE | HALF_BRIGHT | UNDERLINE | INVERSE | BLINK)

#define IS_BOLD(flags)        ((flags) & BOLD)
#define IS_SECURE(flags)      ((flags) & SECURE)
#define IS_HALF_BRIGHT(flags) ((flags) & HALF_BRIGHT)
#define IS_UNDERLINE(flags)   ((flags) & UNDERLINE)
#define IS_INVERSE(flags)     ((flags) & INVERSE)
#define IS_BLINK(flags)       ((flags) & BLINK)

/*
** Defines for field types
*/
#define FIELD_UNPROTECT 0
#define FIELD_PROTECT   1
#define FIELD_TRANSMIT  2
#define FIELD_END       3
#define FIELD_MASK      3

#define COLOR_MASK      0x0F

/* 
** Defines for font ID
*/
#define FONT_NORMAL   0
#define FONT_LINEDRAW 1
#define	FONT_GLYPH    3
#define FONT_MASK     3

typedef enum _dtEnhID
{
  enhVideo   = 0,
  enhField   = 1,
  enhUnused  = 2,
  enhFont    = 3,
  enhFgColor = 4,
  enhBgColor = 5,
  NUM_ENHANCEMENT_FIELDS
} dtEnhID;


/*
** enumerate the different erase modes...
*/
typedef enum _DtEraseMode
{
    eraseFromCol0,      /* from col 0 to cursor                     */
    eraseCharCount,     /* "count" characters from cursor           */
    eraseLineCount,     /* "count" lines from cursor                */
    eraseToEOL,         /* from cursor to end-of-line               */
    eraseLine,          /* the entire line the cursor is on         */
    eraseBuffer,        /* the entire buffer                        */
    eraseFromRow0Col0,  /* from row 0, col 0 to the cursor          */
    eraseToEOB          /* from the cursor to the end of the buffer */
} DtEraseMode;


/* 
** This may be useful for debugging purposes.
*/
#if (defined(DEBUG) | defined(__CODECENTER__))
typedef enum _videoFlags
{
    b_i_h_u_s = 0x00, b_i_h_u_S = 0x01, b_i_h_U_s = 0x02, b_i_h_U_S = 0x03,
    b_i_H_u_s = 0x04, b_i_H_u_S = 0x05, b_i_H_U_s = 0x06, b_i_H_U_S = 0x07,
    b_I_h_u_s = 0x08, b_I_h_u_S = 0x09, b_I_h_U_s = 0x0A, b_I_h_U_S = 0x0B,
    b_I_H_u_s = 0x0C, b_I_H_u_S = 0x0D, b_I_H_U_s = 0x0E, b_I_H_U_S = 0x0F,
    B_i_h_u_s = 0x10, B_i_h_u_S = 0x11, B_i_h_U_s = 0x12, B_i_h_U_S = 0x13,
    B_i_H_u_s = 0x14, B_i_H_u_S = 0x15, B_i_H_U_s = 0x16, B_i_H_U_S = 0x17,
    B_I_h_u_s = 0x18, B_I_h_u_S = 0x19, B_I_h_U_s = 0x1A, B_I_h_U_S = 0x1B,
    B_I_H_u_s = 0x1C, B_I_H_u_S = 0x1D, B_I_H_U_s = 0x1E, B_I_H_U_S = 0x1F,
} videoFlags;
#endif /* (defined(DEBUG) | defined(__CODECENTER__)) */


#ifdef	   __cplusplus
extern "C" {
#endif	/* __cplusplus */

extern TermBuffer
_DtTermBufferCreateBuffer
(
    Widget  w,
    long    rows,
    short   cols,
    short   sizeOfBuffer,
    short   sizeOfLine,
    short   sizeOfEnh
);

extern void
_DtTermBufferFreeBuffer
(
    TermBuffer tb
);

/*
** Delete enough characters from the specified line such that the 
** width of the characters deleted equal or exceeds the desired
** width.  The actual width deleted is returned in "width".
*/
void
_DtTermBufferDelete
(
    TermBuffer  tb,
    short      *row,
    short      *col,
    short      *width
);

extern void
_DtTermBufferErase
(
    TermBuffer  tb,
    short       row,
    short       col,
    short       count,
    DtEraseMode eraseSwitch
);

#ifdef	__cplusplus
} /* close scope of 'extern "C"'... */
#endif	/* __cplusplus */

#endif /* _Dt_TermBuffer_h */
/* DON'T ADD ANYTHING AFTER THIS #endif... */
