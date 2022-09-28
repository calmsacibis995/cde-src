/* $XConsortium: CleanUpI.h /main/cde1_maint/1 1995/07/17 17:24:53 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **  File:   CleanUpI.h
 **  Project: Common Desktop Environment
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _DtHelpCleanUpI_h
#define _DtHelpCleanUpI_h

/********    Public Function Declarations    ********/
#ifdef _NO_PROTO
extern	void	_DtHelpCeFreeSdlAttribs();
extern	void	_DtHelpCeFreeSdlStructs();
#else
extern	void	_DtHelpCeFreeSdlAttribs(
				enum SdlElement      element,
				CESDLAttrStruct     *attributes);
extern	void	_DtHelpCeFreeSdlStructs(
				CECanvasStruct	 *canvas,
				CESegment	**seg_list,
				CELinkData	 *link_data);
#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

#endif /* _DtHelpCleanUpI_h */
