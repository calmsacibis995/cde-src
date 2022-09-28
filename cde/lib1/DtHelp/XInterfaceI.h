/* $XConsortium: XInterfaceI.h /main/cde1_maint/1 1995/07/17 17:55:11 drk $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:   XInterface.h
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
#ifndef _XInterface_h
#define _XInterface_h


#ifdef __cplusplus
extern "C" {
#endif

/********    Public Defines Declarations    ********/

/********    Public Typedef Declarations    ********/

/********    Public Structures Declarations    ********/

/********    Public Macro Declarations    ********/

/********    Semi-Private Function Declarations    ********/
#ifdef _NO_PROTO
extern	void		_DtHelpClearTocMarker();
extern	const char	*_DtHelpDAGetSpcString();
#else
extern	void		_DtHelpClearTocMarker(
				DtHelpDispAreaStruct	*pDAS);
extern	const char	*_DtHelpDAGetSpcString(int idx);
#endif /* _NO_PROTO */

/********    Public Function Declarations    ********/

#ifdef _NO_PROTO
extern	VStatus	_DtHelpDABuildSelectedSpc();
extern	VStatus	_DtHelpDABuildSelectedString();
extern	void	_DtHelpDADestroyGraphic();
extern	void	_DtHelpDADestroySpc();
extern	void	_DtHelpDADrawGraphic();
extern	void	_DtHelpDADrawLine();
extern	void	_DtHelpDADrawSpc();
extern	void	_DtHelpDADrawString();
extern	void	_DtHelpDAGetCvsMetrics();
extern	void	_DtHelpDAGetFontMetrics();
extern	Unit	_DtHelpDAGetStrWidth();
extern	VStatus	_DtHelpDALoadGraphic();
extern	void	_DtHelpDAResolveFont();
extern	VStatus	_DtHelpDAResolveSpc();
#else
extern	VStatus	_DtHelpDABuildSelectedSpc (
			ClientData	 client_data,
			SelectionInfo	*prev_info,
			SpecialPtr	 spc_handle,
			Unit		 space,
			Flags		 flags );
extern	VStatus	_DtHelpDABuildSelectedString (
			ClientData	 client_data,
			SelectionInfo	*prev_info,
			char		*string,
			int		 byte_len,
			int		 char_len,
			FontPtr		 font_ptr,
			Unit		 space,
			Flags		 flags );
extern	void	_DtHelpDADestroyGraphic (
			ClientData	 client_data,
			GraphicPtr	 graphic_ptr);
extern	void	_DtHelpDADestroySpc (
			ClientData	 client_data,
			SpecialPtr	 spc_handle);
extern	void	_DtHelpDADrawLine (
			ClientData	 client_data,
			Unit		 x1,
			Unit		 y1,
			Unit		 x2,
			Unit		 y2);
extern	void	_DtHelpDADrawGraphic (
			ClientData	 client_data,
			GraphicPtr	 graphic_ptr,
			Unit		 x,
			Unit		 y,
			Flags		 old_flag,
			Flags		 new_flag);
extern	void	_DtHelpDADrawSpc (
			ClientData	 client_data,
			SpecialPtr	 spc_handle,
			Unit		 x,
			Unit		 y,
			Unit		 box_x,
			Unit		 box_y,
			Unit		 box_height,
			int		 link_type,
			Flags		 old_flags,
			Flags		 new_flags);
extern	void	_DtHelpDADrawString (
			ClientData	 client_data,
			Unit		 x,
			Unit		 y,
			const char	*string,
			int		 byte_len,
			int		 char_len,
			FontPtr		 font_ptr,
			Unit		 box_x,
			Unit		 box_y,
			Unit		 box_height,
			int		 link_type,
			Flags		 old_flags,
			Flags		 new_flags);
extern	void	_DtHelpDAGetCvsMetrics (
			ClientData		 client_data,
			CanvasMetrics		*ret_canvas,
			CanvasFontMetrics	*ret_font,
			CanvasLinkMetrics	*ret_link,
			CanvasTraversalMetrics	*ret_traversal);
extern	void	_DtHelpDAGetFontMetrics (
			ClientData	 client_data,
			FontPtr		 font_ptr,
			Unit		*ret_ascent,
			Unit		*ret_descent,
			Unit		*char_width,
			Unit		*ret_super,
			Unit		*ret_sub);
extern	Unit	_DtHelpDAGetStrWidth (
			ClientData	 client_data,
			const char	*string,
			int		 byte_len,
			int		 char_len,
			FontPtr		 font_ptr);
extern	VStatus	_DtHelpDALoadGraphic (
			ClientData	 client_data,
			char		*vol_xid,
			char		*topic_xid,
			char		*file_xid,
			char		*format,
			char		*method,
			Unit		*ret_width,
			Unit		*ret_height,
			GraphicPtr	*ret_graphic);
extern	void	_DtHelpDAResolveFont (
			ClientData	 client_data,
			char		*lang,
			const char	*char_set,
			CanvasFontSpec	 font_attr,
			char		**mod_string,
			FontPtr		*ret_font);
extern	VStatus	_DtHelpDAResolveSpc (
			ClientData	 client_data,
			char		*lang,
			const char	*char_set,
			CanvasFontSpec	 font_attr,
			const char	*spc_string,
			SpecialPtr	*ret_handle,
			Unit		*ret_width,
			Unit		*ret_height,
			Unit		*ret_ascent);
#endif /* _NO_PROTO */

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif


#endif /* _XInterface_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
