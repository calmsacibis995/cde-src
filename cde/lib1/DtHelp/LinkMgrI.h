/* $XConsortium: LinkMgrI.h /main/cde1_maint/1 1995/07/17 17:47:32 drk $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:   LinkMgrI.h
 **
 **  Project:
 **
 **  Description:  Public Header file for Canvas.c
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 *******************************************************************
 *************************************<+>*************************************/
#ifndef _DtHelpLinkMgrI_h
#define _DtHelpLinkMgrI_h


#ifdef __cplusplus
extern "C" {
#endif

/********    Semi-Public Structures Declarations    ********/

typedef	struct	{
	int	 type;
	int	 hint;
	char	*link;
	char	*descrip;
} CELinkEntry;

typedef	struct	{
	int		 max;
	CELinkEntry	*list;
} CELinkData;

/********    Semi-Public Function Declarations    ********/

#ifdef _NO_PROTO
extern	int		 _DtHelpCeAddLinkToList ();
extern	int		 _DtHelpCeGetLinkHint();
extern	int		 _DtHelpCeGetLinkInfo();
extern	char		*_DtHelpCeGetLinkSpec();
extern	int		 _DtHelpCeGetLinkType();
extern	CEBoolean	_DtHelpCeLinkPosition();
extern	void		 _DtHelpCeRmLinkFromList();
#else
extern	int		 _DtHelpCeAddLinkToList (
				CELinkData	*link_data,
				char		*link,
				int		 type,
				int		 hint,
				char		*description);
extern	int		 _DtHelpCeGetLinkHint(
				CELinkData	 link_data,
				int		 link_index);
extern	int		 _DtHelpCeGetLinkInfo(
				CanvasHandle	 canvas_handle,
				CELinkData	 link_data,
				int		 link_index,
				CEHyperTextCallbackStruct *ret_info);
extern	char		*_DtHelpCeGetLinkSpec(
				CELinkData	 link_data,
				int		 link_index);
extern	int		 _DtHelpCeGetLinkType(
				CELinkData	 link_data,
				int		 link_index);
extern	CEBoolean	_DtHelpCeLinkPosition(
				CanvasHandle	canvas_handle,
				int		hyper_indx,
				Unit		*ret_x,
				Unit		*ret_y,
				Unit		*ret_width,
				Unit		*ret_height);
extern	void		 _DtHelpCeRmLinkFromList(
				CELinkData	 link_data,
				int		 link_index);
#endif /* _NO_PROTO */

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _DtHelpLinkMgrI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
