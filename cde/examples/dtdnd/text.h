/* $XConsortium: text.h /main/cde1_maint/1 1995/07/17 16:43:59 drk $ */
/*****************************************************************************
 *****************************************************************************
 **
 **   File:         text.h
 **
 **   Description:  Header for text transfer portion of CDE Drag & Drop Demo.
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
 * Public Text Transfer Function Declarations
 */

void		textDragSetup(Widget);
void		textDropSetup(Widget);
void		textDragStart(Widget, XEvent*);
Widget		textCreateDragSource(Widget);
Widget		textCreateDropSite(Widget);

