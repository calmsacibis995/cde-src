/*********************************************************************
 *
 *	File:		DndFileP.h
 *
 *	Description:	Private include file for File Transfer
 *			routines for the DND Convenience API.
 *
 *********************************************************************
 *
 *+SNOTICE
 *
 *	$Revision: 1.5 $
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

#ifndef _Dt_DndFile_h
#define _Dt_DndFile_h

#ifdef __cplusplus
extern "C" {
#endif
 
void		_DtDndFileConvertTargets(Display*, Atom**, Cardinal*);

Boolean		_DtDndFileConvertProc(Widget, Atom*, Atom*, Atom*, XtPointer*,
			unsigned long*, int*, DtDragInfo*,
			XSelectionRequestEvent*);

void		_DtDndFileTransferEntries(Widget, XmDropTransferEntryRec**,
			Cardinal*, DtDropInfo*);

void		_DtDndFileTransferProc(Widget, DtDropInfo*, Atom*, Atom*,
			XtPointer, unsigned long*, int*);

#ifdef __cplusplus
}
#endif
 
#endif /* _Dt_DndFile_h */
