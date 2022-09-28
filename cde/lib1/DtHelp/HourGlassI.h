/* $XConsortium: HourGlassI.h /main/cde1_maint/2 1995/08/26 22:42:55 barstow $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:       HourGlassI.h
 **
 **   Project:     Cache Creek
 **
 **   Description: Internal include file for HourGlass Library.
 **
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _hourglassI_h
#define _hourglassI_h

/* DtHelpGetHourGlassCursor -
 *
 *   Builds and returns the appropriate HourGlass cursor.
 */

#ifdef _NO_PROTO
extern Cursor DtHelpGetHourGlassCursor();
#else
extern Cursor DtHelpGetHourGlassCursor( 
                        Display *dpy) ;
#endif

/* DtHelpTurnOnHourGlass -
 *
 *   Gets and displays an hourglass cursor in the window of the widget
 *   which is passed in to the funciton.
 */

#ifdef _NO_PROTO
extern void _DtHelpTurnOnHourGlass();
#else
extern void _DtHelpTurnOnHourGlass( 
                        Widget w) ;
#endif
 
 /*  Widget widget;
  *
  * widget is the toplevel shell of the window you want
  * the hourglass cursor to appear in.
  */


/* DtHelpTurnOffHourGlass -
 *
 *   Removes the hourglass cursor from the window of the widget
 *   which is passed in to the funciton.
 */

#ifdef _NO_PROTO
extern void _DtHelpTurnOffHourGlass();
#else
extern void _DtHelpTurnOffHourGlass( 
                        Widget w) ;
#endif
  
 /* Widget widget;
  *
  *  widget is the toplevel shell of the window you want
  *  to remove hourglass cursor from.
  */

/* DtHelpTurnOnNoEnter -
 *
 *   Removes the hourglass cursor from the window of the widget
 *   which is passed in to the funciton.
 */

#ifdef _NO_PROTO
extern void _DtHelpTurnOnNoEnter();
#else
extern void _DtHelpTurnOnNoEnter( 
                        Widget w) ;
#endif
  
 /* Widget widget;
  *
  *  widget is the toplevel shell of the window you want
  *  to remove hourglass cursor from.
  */

/* DtHelpTurnOffNoEnter -
 *
 *   Removes the hourglass cursor from the window of the widget
 *   which is passed in to the funciton.
 */

#ifdef _NO_PROTO
extern void _DtHelpTurnOffNoEnter();
#else
extern void _DtHelpTurnOffNoEnter( 
                        Widget w) ;
#endif
  
 /* Widget widget;
  *
  *  widget is the toplevel shell of the window you want
  *  to remove hourglass cursor from.
  */


#endif /* _hourglassI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
