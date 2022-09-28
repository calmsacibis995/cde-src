/* $XConsortium: UtilSDLI.h /main/cde1_maint/1 1995/07/17 17:53:31 drk $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:        UtilSDLI.h
 **
 **  Project:     Cde Help System
 **
 **  Description: Internal header file for UtilSDL.c
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 *****************************************************************************
 *************************************<+>*************************************/
#ifndef _DtHelpUtilSDLI_h
#define _DtHelpUtilSDLI_h


#ifdef __cplusplus
extern "C" {
#endif

/********     Private Define Declarations    ********/
#define	DTD_NAMELEN	64
/******** End Private Define Declarations    ********/

/********    Private Function Declarations    ********/

#ifdef _NO_PROTO
extern	CEString *_DtHelpCeAllocStrStruct();
extern	void	_DtHelpCeAddSegToList();
extern	int	_DtHelpCeAllocElement ();
extern	int	_DtHelpCeAllocSegment ();
extern	int	_DtHelpCeFindSkipSdlElementEnd();
extern	int	_DtHelpCeGetSdlAttribute ();
extern	int	_DtHelpCeGetSdlAttributeCdata ();
extern	int	_DtHelpCeGetSdlCdata ();
extern	int	_DtHelpCeGetSdlId ();
extern	int	_DtHelpCeGetSdlNumber ();
extern	int	_DtHelpCeMatchSdlElement ();
extern	CEElement *_DtHelpCeMatchSemanticStyle ();
extern	void	_DtHelpCeMergeSdlAttribInfo();
extern	int	_DtHelpCeReturnSdlElement ();
extern	int	_DtHelpCeRunInterp ();
extern	int	_DtHelpCeSaveString ();
extern	int	_DtHelpCeSkipCdata ();
#else
extern	CEString *_DtHelpCeAllocStrStruct(void);
extern	void	_DtHelpCeAddSegToList(
			CESegment	   *seg_ptr,
			CESegment	  **seg_list,
			CESegment	  **last_seg);
extern	int	_DtHelpCeAllocElement (
			enum SdlElement     el_value,
			CESDLAttrStruct    *old_attrs,
			CEElement	  **new_el);
extern	int	_DtHelpCeAllocSegment (
			int		    malloc_size,
			int		   *alloc_size,
			CESegment	  **next_seg,
			CESegment	  **ret_seg);
extern	int	_DtHelpCeFindSkipSdlElementEnd(
			BufFilePtr	  f);
extern	int	_DtHelpCeGetSdlAttribute (
			BufFilePtr	 f,
			int		 max_len,
			char		*attribute_name);
extern	int	_DtHelpCeGetSdlAttributeCdata (
			BufFilePtr	  f,
			CEBoolean	  limit,
			char		**string);
extern	int	_DtHelpCeGetSdlCdata (
			BufFilePtr	  f,
                        SDLCdata          abbrev,
			enum SdlOption	  type,
			int		  char_len,
			CEBoolean	  nl_to_space,
			CEBoolean	 *cur_space,
			CEBoolean	 *cur_nl,
			CEBoolean	 *cur_mb,
			char		**string,
			int		  str_max);
extern	int	_DtHelpCeGetSdlId (
			BufFilePtr	  f,
			char		**id_value);
extern	int	_DtHelpCeGetSdlNumber (
			BufFilePtr	 f,
			char		*number_string);
extern	int	_DtHelpCeMatchSdlElement (
			BufFilePtr	 f,
			const char	*element_str,
			int		 sig_chars);
extern	CEElement *_DtHelpCeMatchSemanticStyle (
			CESegment	*toss,
			enum SdlOption	 clan,
			int		 level,
			char		*ssi);
extern	void	_DtHelpCeMergeSdlAttribInfo(
			CEElement           *src_el,
			CEElement           *dst_el);
extern	int	_DtHelpCeReturnSdlElement (
			BufFilePtr	  f,
			const SDLElement *el_list,
			CEBoolean	  cdata_flag,
			enum SdlElement	*ret_el_define,
			char		**ret_data,
			CEBoolean	 *ret_end_flag);
extern	int	_DtHelpCeRunInterp (
			VStatus		 (*filter_exec)(),
			ClientData	  client_data,
			enum SdlOption	  interp_type,
			char		 *data,
			char		**ret_data);
extern	int	_DtHelpCeSaveString (
			CESegment	  **seg_list,
			CESegment	  **last_seg,
			CESegment	  **prev_seg,
			char		   *string,
			CanvasFontSpec	   *font_specs,
			int		    link_index,
			int		    multi_len,
			int		    flags,
			CEBoolean	    newline);
extern	int	_DtHelpCeSkipCdata (
			BufFilePtr	f,
			CEBoolean	flag);
#endif /* _NO_PROTO */

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _DtHelpUtilSDLI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
