/* $XConsortium: LayoutSDLI.h /main/cde1_maint/1 1995/07/17 17:44:59 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 ** File: LayoutSDLI.h
 ** Project: Common Desktop Environment
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _DtHelpCeLayoutSDLI_h
#define _DtHelpCeLayoutSDLI_h

/********    Semi-Public Function Declarations    ********/
#ifdef _NO_PROTO
extern	CEBoolean	_DtHelpCeResizeSDLCanvas();
extern	void		_DtHelpCeSetSdlTopic();
#else
extern	CEBoolean	_DtHelpCeResizeSDLCanvas(
				CanvasHandle	canvas_handle,
				Unit		*ret_width,
				Unit		*ret_height);
extern	void		_DtHelpCeSetSdlTopic(
				CanvasHandle	 canvas_handle,
				TopicHandle	 topic_handle,
				Unit		 scroll_percent,
				Unit		*ret_width,
				Unit		*ret_height,
				Unit		*ret_y);
#endif /* _NO_PROTO */

#endif /* _DtHelpCeLayoutSDLI_h */
