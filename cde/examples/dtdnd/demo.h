/* $XConsortium: demo.h /main/cde1_maint/1 1995/07/17 16:42:21 drk $ */
/*****************************************************************************
 *****************************************************************************
 **
 **   File:         demo.h
 **
 **   Description:  Header for foundation of CDE Drag & Drop Demo.
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of
 **      Novell, Inc.
 **
 ****************************************************************************
 ************************************<+>*************************************/

/*
 * Public General-Purpose Demo Function Declarations
 */

void		demoDragFinishCallback(Widget, XtPointer, XtPointer);
void		demoDragMotionHandler(Widget, XtPointer, XtPointer, XEvent*);
void		demoDrawExposeCallback(Widget, XtPointer, XtPointer);

