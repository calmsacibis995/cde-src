/* $XConsortium: buff.h /main/cde1_maint/1 1995/07/17 16:41:43 drk $ */
/*****************************************************************************
 *****************************************************************************
 **
 **   File:         buff.h
 **
 **   Description:  Header for buffer transfer portion of CDE Drag & Drop Demo.
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
 * Public Appointment Buffer Transfer Function Declarations
 */

Widget		apptCreateDragSource(Widget);
void		apptDragSetup(Widget);
void		apptDragStart(Widget, XEvent*);
void		apptTransferCallback(Widget, XtPointer, XtPointer);

