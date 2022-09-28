/* $XConsortium: HourGlassI.h /main/cde1_maint/1 1995/07/17 21:00:32 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:       HourGlassI.h
 **
 **   Project:     CDE dthelpdemo sample program.
 **
 **   Description: Internal include file for HourGlass Library.
 **
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992, 1993, 1994
 **      Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of
 **      Novell, Inc.
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _hourglassI_h
#define _hourglassI_h

/* TurnOnHourGlass -
 *
 *   Gets and displays an hourglass cursor in the window of the widget
 *   which is passed in to the funciton.
 */

extern void _DtHelpTurnOnHourGlass(
    Widget widget);

 /*
  * widget is the toplevel shell of the window you want
  * the hourglass cursor to appear in.
  */


/* TurnOffHourGlass -
 *
 *   Removes the hourglass cursor from the window of the widget
 *   which is passed in to the funciton.
 */

extern void _DtHelpTurnOffHourGlass(
    Widget widget);

 /*
  *  widget is the toplevel shell of the window you want
  *  to remove hourglass cursor from.
  */

#endif /* _hourglassI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */








