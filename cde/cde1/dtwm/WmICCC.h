/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993, 1994 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.4
*/ 
/*   $XConsortium: WmICCC.h /main/cde1_maint/1 1995/07/15 01:50:35 drk $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include <X11/Xatom.h>
#include <X11/Xmd.h>


/*
 * Value definitions and macros:
 */



/*************************************<->*************************************
 *
 *  ICCC property data structures ...
 *
 *
 *  Description:
 *  -----------
 *  These data structures are mwm versions of the window manager data
 *  stuctures that are specified in the ICCCM and the Xlib specification
 *  for client/window manager communication.  In general these data
 *  structures correspond to client window property values.
 *
 *************************************<->***********************************/


/* mwm version of the xPropSizeHints structure: */

typedef struct _PropSizeHints
{
    unsigned long	flags;
    long		x;			/* R2 conventions */
    long		y;			/* R2 conventions */
    long		width;			/* R2 conventions */
    long		height;			/* R2 conventions */
    long		minWidth;
    long		minHeight;
    long		maxWidth;
    long		maxHeight;
    long		widthInc;
    long		heightInc;
    long		minAspectX;
    long		minAspectY;
    long		maxAspectX;
    long		maxAspectY;
    long		baseWidth;		/* current conventions */
    long		baseHeight;		/* current conventions */
    long		winGravity;		/* current conventions */

} PropSizeHints;

#define PROP_SIZE_HINTS_ELEMENTS	18


/* mwm version of the XSizeHints structure: */

typedef struct _SizeHints
{
    int		icccVersion;
    long	flags;
    int		x;				/* R2 conventions */
    int		y;				/* R2 conventions */
    int		width;				/* R2 conventions */
    int		height;				/* R2 conventions */
    int		min_width;
    int		min_height;
    int		max_width;
    int		max_height;
    int		width_inc;
    int		height_inc;
    AspectRatio min_aspect;
    AspectRatio max_aspect;
    int		base_width;			/* current conventions */
    int		base_height;			/* current conventions */
    int		win_gravity;			/* current conventions */

} SizeHints;

/* mwm version of the xPropWMState structure: */

typedef struct _PropWMState
{
    unsigned long	state;
    unsigned long	icon;
} PropWMState;

#define PROP_WM_STATE_ELEMENTS		2


/* ICCC versions (icccVersion): */
#define ICCC_R2		0
#define ICCC_CURRENT	1
#define ICCC_UNKNOWN	ICCC_CURRENT


/* SizeHints flags field values: */
#define US_POSITION		(1L << 0)
#define US_SIZE			(1L << 1)
#define P_POSITION		(1L << 2)
#define P_SIZE			(1L << 3)
#define P_MIN_SIZE		(1L << 4)
#define P_MAX_SIZE		(1L << 5)
#define P_RESIZE_INC		(1L << 6)
#define P_ASPECT		(1L << 7)
#define P_BASE_SIZE		(1L << 8)
#define P_WIN_GRAVITY		(1L << 9)

/* PropWMState state field value: */
#define WithdrawnSTATE		0
