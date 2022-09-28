/*********************************************************************
 *
 *	File:		DndIconP.h
 *
 *	Description:	Private include file for Drag Icon 
 *			routines for the DND Convenience API.
 *
 *********************************************************************
 *
 *+SNOTICE
 *
 *	$Revision: 1.4 $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement bertween
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel wihtout
 *	Sun's specific written approval.  This documment and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 *
 *+ENOTICE
 */

#ifndef _Dt_DndIconP_h
#define _Dt_DndIconP_h

#ifdef __cplusplus
extern "C" {
#endif

extern void	
_DtDndSelectDragSource(
	Widget			anyWidget,
	DtDndDragSource		sourceType,
	Widget			sourceIcon);
extern void
_DtDndGetIconOffset(
	Widget			dragContext, 
	DtDndDragSource		sourceType,
	int *			offsetXReturn,
	int *			offsetYReturn);

#ifdef __cplusplus
}
#endif
 
#endif /* _Dt_DndIconP_h */
