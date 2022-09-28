/* $XConsortium: AccessSDLI.h /main/cde1_maint/1 1995/07/17 17:19:21 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        AccessSDLI.h
 **
 **   Project:     Run Time Project File Access
 **
 **   Description: Header file for Access.h
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _DtHelpAccessSDLI_h
#define _DtHelpAccessSDLI_h

/****************************************************************************
 *			Semi-Private Routines
 ****************************************************************************/
#ifdef _NO_PROTO
extern	void		 _DtHelpCeCleanSdlVolume();
extern	void		 _DtHelpCeCloseSdlVolume();
extern	CEElement	*_DtHelpCeFindSdlElement();
extern	CEBoolean	 _DtHelpCeFindSdlId ();
extern	int		 _DtHelpCeGetSdlDocStamp ();
extern	char		*_DtHelpCeGetSdlHomeTopicId ();
extern	int		 _DtHelpCeGetSdlIdPath ();
extern	int		 _DtHelpCeGetSdlKeywordList ();
extern	short		 _DtHelpCeGetSdlMinorNumber ();
extern	int		 _DtHelpCeGetSdlTopicChildren();
extern	const char	*_DtHelpCeGetSdlVolCharSet ();
extern	char		*_DtHelpCeGetSdlVolLanguage ();
extern	char		*_DtHelpCeGetSdlVolumeAsciiAbstract ();
extern	char		*_DtHelpCeGetSdlVolumeLocale ();
extern	CESDLVolume	*_DtHelpCeGetSdlVolumePtr();
extern	void		 _DtHelpCeInitSdlVolume();
extern	CEElement	*_DtHelpCeMapSdlIdToElement();
extern	CESegment	*_DtHelpCeMapSdlIdToSegment();
extern	int		 _DtHelpCeMapIdToSdlTopicId();
extern	int		 _DtHelpCeOpenSdlVolume();
extern	int		 _DtHelpCeRereadSdlVolume();
#else
extern	void		 _DtHelpCeCleanSdlVolume(
				CanvasHandle	canvas,
				VolumeHandle	volume );
extern	void		 _DtHelpCeCloseSdlVolume (
				CanvasHandle	canvas_handle,
				VolumeHandle	volume);
extern	CEElement	*_DtHelpCeFindSdlElement(
				CESegment	  *seg_list,
				enum SdlElement	   target,
				CEBoolean	   depth);
extern	CEBoolean	 _DtHelpCeFindSdlId (
				VolumeHandle	  vol,
				char		 *target_id,
				int		  fd,
				char		**ret_name,
				int		 *ret_offset );
extern	int		 _DtHelpCeGetSdlDocStamp (
				VolumeHandle	  volume,
				char		**ret_doc,
				char		**ret_time);
extern	char		*_DtHelpCeGetSdlHomeTopicId (VolumeHandle vol);
extern	int		 _DtHelpCeGetSdlIdPath (
				VolumeHandle	   volume,
				char		  *target_id,
				char		***ret_ids);
extern	CEElement	*_DtHelpCeMapSdlIdToElement(
				VolumeHandle	  vol,
				char		 *target_id,
				int		  fd);
extern	int		 _DtHelpCeGetSdlKeywordList (
				VolumeHandle	volume);
extern	short		 _DtHelpCeGetSdlMinorNumber (
				VolumeHandle	volume);
extern	int		 _DtHelpCeGetSdlTopicChildren(
				VolumeHandle	  volume,
				char		 *target_id,
				char		***ret_ids);
extern	const char	*_DtHelpCeGetSdlVolCharSet (
				VolumeHandle	volume);
extern	char		*_DtHelpCeGetSdlVolLanguage (
				VolumeHandle	volume);
extern	char		*_DtHelpCeGetSdlVolumeAsciiAbstract (
				CanvasHandle	canvas_handle,
				VolumeHandle	volume);
extern	char		*_DtHelpCeGetSdlVolumeLocale (
				VolumeHandle	volume);
extern	CESDLVolume	*_DtHelpCeGetSdlVolumePtr(
				VolumeHandle	volume );
extern	CESegment	*_DtHelpCeMapSdlIdToSegment(
				VolumeHandle	  vol,
				const char	 *target_id,
				int		  fd);
extern	void		 _DtHelpCeInitSdlVolume(
				VolumeHandle	volume );
extern	int		 _DtHelpCeMapIdToSdlTopicId(
				VolumeHandle	  vol,
				const char	 *target_id,
				char		**ret_id);
extern	int		 _DtHelpCeOpenSdlVolume(
				CanvasHandle	canvas,
				VolumeHandle	volume );
extern	int		 _DtHelpCeRereadSdlVolume(
				CanvasHandle	canvas,
				VolumeHandle	volume );
#endif
#endif /* _DtHelpAccessSDLI_h */
