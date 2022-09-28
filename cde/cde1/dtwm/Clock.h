/* $XConsortium: Clock.h /main/cde1_maint/1 1995/07/18 01:51:49 drk $ */
/**---------------------------------------------------------------------
***	
***	file:		Clock.h
***
***	project:	MotifPlus Widgets
***
***	description:	Public include file for DtClock class.
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

#ifndef _DtClock_h
#define _DtClock_h

#include <Xm/Xm.h>

#ifndef DtIsClock
#define DtIsClock(w) XtIsSubclass(w, dtClockGadgetClass)
#endif /* DtIsClock */

/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget DtCreateClock() ;

#else

extern Widget DtCreateClock( 
                        Widget parent,
                        String name,
                        ArgList arglist,
                        Cardinal argcount) ;
#endif

extern WidgetClass dtClockGadgetClass;

typedef struct _DtClockClassRec * DtClockClass;
typedef struct _DtClockRec      * DtClockGadget;

#endif /* _DtClock_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
