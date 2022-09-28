/* $XConsortium: HourGlass.h /main/cde1_maint/1 1995/07/17 18:12:44 drk $ */
/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:       HourGlass.h
 **
 **   Project:     DT
 **
 **   Description: Public include file for HourGlass Library.
 **
 **
 **   (c) Copyright 1987, 1988, 1989 by Hewlett-Packard Company
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _hourglass_h
#define _hourglass_h

/* _DtGetHourGlassCursor -
 *
 *   Builds and returns the appropriate HourGlass cursor.
 */

#ifdef _NO_PROTO
extern Cursor _DtGetHourGlassCursor();
#else
extern Cursor _DtGetHourGlassCursor( 
                        Display *dpy) ;
#endif

/* _DtTurnOnHourGlass -
 *
 *   Gets and displays an hourglass cursor in the window of the widget
 *   which is passed in to the funciton.
 */

#ifdef _NO_PROTO
extern void _DtTurnOnHourGlass();
#else
extern void _DtTurnOnHourGlass( 
                        Widget w) ;
#endif
 
 /*  Widget widget;
  *
  * widget is the toplevel shell of the window you want
  * the hourglass cursor to appear in.
  */


/* _DtTurnOffHourGlass -
 *
 *   Removes the hourglass cursor from the window of the widget
 *   which is passed in to the funciton.
 */

#ifdef _NO_PROTO
extern void _DtTurnOffHourGlass();
#else
extern void _DtTurnOffHourGlass( 
                        Widget w) ;
#endif
  
 /* Widget widget;
  *
  *  widget is the toplevel shell of the window you want
  *  to remove hourglass cursor from.
  */


#endif /* _hourglass_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */



