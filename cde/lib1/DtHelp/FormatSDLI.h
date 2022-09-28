/* $XConsortium: FormatSDLI.h /main/cde1_maint/1 1995/07/17 17:32:46 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **  File:  FormatSDLI.h
 **  Project:  Common Desktop Environment
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _DtHelpFormatSDLI_h
#define _DtHelpFormatSDLI_h

#include <sys/stat.h>

#ifndef	__CEGetSDLElementAttrs
#define	__CEGetSDLElementAttrs(element) \
		_DtHelpCeGetSdlElementAttrs(element)
#endif
#ifndef	__CEGetSDLAttribEntry
#define	__CEGetSDLAttribEntry(attr_define) \
		_DtHelpCeGetSdlAttribEntry(attr_define)
#endif

/********    Semi-Private Function Declarations    ********/
#ifdef _NO_PROTO
extern	int			 _DtHelpCeFrmtSDLTitleToAscii();
extern	char			*_DtHelpCeFrmtSdlVolumeAbstractToAscii();
extern	int			 _DtHelpCeFrmtSdlPathAndChildren();
extern	int			 _DtHelpCeGetSdlTitleChunks();
extern	int			 _DtHelpCeGetSdlVolIds();
extern	int			 _DtHelpCeGetSdlVolIndex();
extern	int			 _DtHelpCeGetSdlVolTitleChunks();
extern	CESegment		*_DtHelpCeGetSdlVolToss();
extern  CEElement		*_DtHelpCeMapIdToLoidEntry();
extern	int			 _DtHelpCeFrmtSdlVolumeInfo();
extern	char			*_DtHelpCeGetInterpCmd();
extern	const SDLAttribute	*_DtHelpCeGetSdlAttribEntry();
extern	const SDLAttribute	*_DtHelpCeGetSdlAttributeList();
extern	const SDLElementAttrList *_DtHelpCeGetSdlElementAttrs();
extern	int			 _DtHelpCeParseSdlTopic();
#else
extern	int			 _DtHelpCeFrmtSDLTitleToAscii(
					CanvasHandle	 canvas_handle,
					VolumeHandle	 volume_handle,
					int		 offset,
					char		**ret_title,
					char		**ret_abbrev);
extern	char			*_DtHelpCeFrmtSdlVolumeAbstractToAscii(
					CanvasHandle	 canvas_handle,
					VolumeHandle	 volume);
extern	int			 _DtHelpCeFrmtSdlPathAndChildren(
					CanvasHandle	 canvas_handle,
					VolumeHandle	 volume,
					int		 fd,
					char		*target_id,
					TopicHandle	*ret_handle);
extern	int			 _DtHelpCeGetSdlTitleChunks(
					CanvasHandle	 canvas_handle,
					VolumeHandle	 volume,
					char		*loc_id,
					void		***ret_chunks);
extern	int			 _DtHelpCeGetSdlVolIds(
					VolumeHandle	 volume,
					int		 fd,
					CESegment	**ret_loids);
extern	int			 _DtHelpCeGetSdlVolIndex(
					VolumeHandle	 volume);
extern	int			 _DtHelpCeGetSdlVolTitleChunks(
					CanvasHandle	 canvas_handle,
					VolumeHandle	 volume,
					void		***ret_chunks);
extern	CESegment		*_DtHelpCeGetSdlVolToss(
					VolumeHandle	 volume,
					int		 fd);
extern  CEElement		*_DtHelpCeMapIdToLoidEntry(
					CESegment       *id_segs,
					char            *target_id);
extern	int			 _DtHelpCeFrmtSdlVolumeInfo(
					CanvasHandle	 canvas_handle,
					char		*filename,
					VolumeHandle	 volume,
					time_t		*ret_time);
extern	char			*_DtHelpCeGetInterpCmd(
					enum SdlOption	interp_type);
extern	const SDLAttribute	*_DtHelpCeGetSdlAttribEntry(
					int		attr_define);
extern	const SDLAttribute	*_DtHelpCeGetSdlAttributeList(void);
extern	const SDLElementAttrList *_DtHelpCeGetSdlElementAttrs(
					enum SdlElement	element);
extern	int			 _DtHelpCeParseSdlTopic(
					CanvasHandle	 canvas_handle,
					VolumeHandle 	 volume,
					int		 fd,
					int		 offset,
					char		*id_string,
					CEBoolean	 rich_text,
					TopicHandle	*ret_handle);
#endif /* _NO_PROTO */

/********    End Public Function Declarations    ********/

#endif /* _DtHelpFormatSDLI_h */
