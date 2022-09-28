/* $XConsortium: FormatCCDFI.h /main/cde1_maint/1 1995/07/17 17:30:45 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        FormatCCDFI.h
 **
 **   Project:     TextGraphic Display routines
 **
 **   Description: Header file for Format.c
 **
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
#ifndef _DtHelpFormatCCDFI_h
#define _DtHelpFormatCCDFI_h

typedef	void*	VarHandle;

#ifndef	__CEGetParagraphList
#define	__CEGetParagraphList(var_handle, ret_handle) \
	__DtHelpCeGetParagraphList(var_handle, ret_handle)
#endif

#ifdef	_NO_PROTO
extern	int		_DtHelpCeFrmtCcdfPathAndChildren();
extern	int		_DtHelpCeGetCcdfTitleChunks();
extern	int		_DtHelpCeGetCcdfVolTitleChunks();
extern	int		_DtHelpCeFrmtCcdfTopic ();
extern	int		__DtHelpCeGetParagraphList ();
extern	int		__DtHelpCeProcessString();
extern	VarHandle	__DtHelpCeSetUpVars ();
#else
extern	int		_DtHelpCeGetCcdfTitleChunks(
				CanvasHandle	canvas_handle,
				_DtHelpVolume     volume,
				char		*loc_id,
				void		***ret_chunks);
extern	int		_DtHelpCeFrmtCcdfPathAndChildren(
				CanvasHandle         canvas_handle,
				_DtHelpVolume         volume,
				char                 *loc_id,
				TopicHandle          *ret_handle );
extern	int		_DtHelpCeGetCcdfVolTitleChunks(
				CanvasHandle	canvas_handle,
				_DtHelpVolume     volume,
				void		***ret_chunks);
extern	int		_DtHelpCeFrmtCcdfTopic (
				CanvasHandle	canvas_handle,
				_DtHelpVolume     volume,
				char          *filename,
				int            offset,
				char          *id_string,
				TopicHandle	 *ret_topic);
extern	int		__DtHelpCeGetParagraphList (
				VarHandle	 var_handle,
				TopicHandle	*ret_handle);
extern	int		__DtHelpCeProcessString(
				CanvasHandle	  canvas_handle,
				VarHandle	  var_handle,
				BufFilePtr	  my_file,
				char		 *scan_string,
				char		 *in_string,
				int		  in_size,
				int		  seg_type,
				CEFontSpec	*font_attr);
extern	VarHandle	__DtHelpCeSetUpVars (
				char		*lang,
				char		*code_set);
#endif

#endif /* _DtHelpFormatCCDFI_h */
