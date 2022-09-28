/* $XConsortium: ClockP.h /main/cde1_maint/1 1995/07/18 01:52:02 drk $ */
/**---------------------------------------------------------------------
***	
***	file:		ClockP.h
***
***	project:	MotifPlus Widgets
***
***	description:	Private include file for DtClock class.
***			Portions adapted from the Xaw Clock widget.
***	
***	
***			(c) Copyright 1990 by Hewlett-Packard Company.
***
***
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
***
***
***-------------------------------------------------------------------*/


#ifndef _DtClockP_h
#define _DtClockP_h

#include <Xm/XmP.h>
#include <Dt/ControlP.h>
#include "Clock.h"

#define SEG_BUFF_SIZE		128
#define ASCII_TIME_BUFLEN	32	/* big enough for 26 plus slop */



/*-------------------------------------------------------------
**	Class Structure
*/

/*	Class Part
*/
#ifndef XtSpecificationRelease

typedef struct _DtClockClassPart
{
	caddr_t			extension;
} DtClockClassPart;

#else

typedef struct _DtClockClassPart
{
	XmCacheClassPartPtr	cache_part;
	caddr_t			extension;
} DtClockClassPart;

#endif /* XtSpecificationRelease */

/*	Full Class Record
*/
typedef struct _DtClockClassRec
{
	RectObjClassPart	rect_class;
	XmGadgetClassPart	gadget_class;
	DtIconClassPart	icon_class;
	DtControlClassPart	control_class;
	DtClockClassPart	clock_class;
} DtClockClassRec;

/*	Actual Class
*/
extern DtClockClassRec dtClockClassRec;



/*-------------------------------------------------------------
**	Instance Structure
*/

/*	Instance Part
*/
typedef struct _DtClockPart
{
	int		update;		/* update frequence */
	Dimension 	radius;		/* radius factor */
	int		backing_store;	/* backing store type */
	Boolean 	reverse_video;
	Boolean 	chime;
	Boolean 	beeped;
	Dimension 	tick_spacing;
	Dimension 	minute_hand_length;
	Dimension 	hour_hand_length;
	Dimension 	hand_width;
	Position 	centerX;
	Position 	centerY;
	int		numseg;
	XPoint		segbuff[SEG_BUFF_SIZE];
	XPoint		*segbuffptr;
	XPoint		*hour, *sec;
	struct 		tm  otm ;

	XtIntervalId 	interval_id;
	char 		prev_time_string[ASCII_TIME_BUFLEN];
	int		clock_interval;

	Boolean		_do_update;
	GC		clock_hand_gc;
	GC		clock_background_gc;

	Dimension	left_inset;
	Dimension	right_inset;
	Dimension	top_inset;
	Dimension	bottom_inset;
	Dimension	clock_width;
	Dimension	clock_height;
} DtClockPart;

/*	Full Instance Record
*/
typedef struct _DtClockRec
{
	ObjectPart	object;
	RectObjPart	rectangle;
	XmGadgetPart	gadget;
	DtIconPart	icon;
	DtControlPart	control;
	DtClockPart	clock;
} DtClockRec;



/*-------------------------------------------------------------
**	Class and Instance Macros
*/

/*	DtClock Class Macros
*/

/*	DtClock Instance Macros
*/
#define G_ClockInterval(g)	(g -> clock.clock_interval)
#define G_ClockHandGC(g)	(g -> clock.clock_hand_gc)
#define G_ClockBackgroundGC(g)	(g -> clock.clock_background_gc)
#define G_ClockHandPixel(g)	(g -> icon.pixmap_foreground)
#define G_ClockBackground(g)	(g -> icon.pixmap_background)
#define G_ClockWidth(g)		(g -> clock.clock_width)
#define G_ClockHeight(g)	(g -> clock.clock_height)
#define G_LeftInset(g)		(g -> clock.left_inset)
#define G_RightInset(g)		(g -> clock.right_inset)
#define G_TopInset(g)		(g -> clock.top_inset)
#define G_BottomInset(g)	(g -> clock.bottom_inset)
#define G_ClockDoUpdate(g)	(g -> clock._do_update)


#endif /* _DtClockP_h */


/* DON'T ADD ANYTHING AFTER THIS #endif */
