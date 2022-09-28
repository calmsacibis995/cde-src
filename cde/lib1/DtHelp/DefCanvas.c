/* $XConsortium: DefCanvas.c /main/cde1_maint/1 1995/07/17 17:25:14 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        DefCanvas.c
 **
 **   Project:     Cde DtHelp
 **
 **   Description:
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

/*
 * system includes
 */
#include <stdlib.h>
#include <string.h>

/*
 * private includes
 */
#include "Canvas.h"
#include "AsciiSpcI.h"
#include "DefCanvasI.h"

#ifdef NLS16
#endif

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO
static	void		DefCvsMetrics();
#ifdef	not_used
static	Unit		DefCvsStrWidth();
static	void		DefCvsGetFont();
static	void		DefCvsFontMetrics();
static	VStatus		DefCvsResolveSpc();
#endif /* not_used */
#else
static	void		DefCvsMetrics(
				ClientData		 client_data,
				CanvasMetrics		*ret_canvas,
				CanvasFontMetrics	*ret_font,
				CanvasLinkMetrics	*ret_link,
				CanvasTraversalMetrics	*ret_traversal);
#ifdef	not_used
static	Unit		DefCvsStrWidth(
				ClientData	 client_data,
				char		*string,
				int		 byte_len,
				int		 char_len,
				FontPtr		 font_ptr );
static	void		DefCvsGetFont(
				ClientData	 client_data,
				char		*lang,
				char		*charset,
				CanvasFontSpec	 font_attr,
				char		**mod_string,
				FontPtr		*ret_font );
static	void		DefCvsFontMetrics(
				ClientData	 client_data,
				FontPtr		 font_ptr,
				Unit		*ret_ascent,
				Unit		*ret_descent,
				Unit		*char_width,
				Unit		*ret_super,
				Unit		*ret_sub);
static	VStatus		DefCvsResolveSpc(
				ClientData	 client_data,
				char		*lang,
				char		*charset,
				CanvasFontSpec	 font_attr,
				char		*spc_symbol,
				SpecialPtr	*ret_handle,
				Unit		*ret_width,
				Unit		*ret_height,
				Unit		*ret_ascent);
#endif /* not_used */
#endif /* _NO_PROTO */

/********    End Public Function Declarations    ********/

static VirtualInfo	MyVirtInfo =
  {
	DefCvsMetrics,	/* void            (*_CEGetCanvasMetrics)(); */
	NULL, /* DefCvsStrWidth, Unit            (*_CEGetStringWidth)(); */
	NULL,		/* void            (*_CEDrawString)(); */
	NULL,		/* void            (*_CEDrawLine)(); */
	NULL, /* DefCvsGetFont,	 void            (*_CEGetFont)(); */
	NULL, /* DefCvsFontMetrics, void            (*_CEGetFontMetrics)(); */
	NULL,		/* VStatus         (*_CELoadGraphic)(); */
	NULL,		/* void            (*_CEDrawGraphic)(); */
	NULL,		/* void            (*_CEDestroyGraphic)(); */
	NULL, /* DefCvsResolveSpc, VStatus         (*_CEResolveSpecial)(); */
	NULL,		/* void            (*_CEDrawSpecial)(); */
	NULL,		/* void            (*_CEDestroySpecial)(); */
	NULL,		/* VStatus         (*_CEBuildSelectionString)(); */
	NULL,		/* VStatus         (*_CEBuildSelectionGraphic)(); */
	NULL,		/* VStatus         (*_CEBuildSelectionSpc)(); */
  };

/******************************************************************************
 *
 * Private functions
 *
 ******************************************************************************/
/******************************************************************************
 * Canvas functions
 ******************************************************************************/
/*****************************************************************************
 * Function:	void DefCvsMetrics ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
static	CanvasFontMetrics	defFontMetrics = { 1, 0, 1 };
static	CanvasLinkMetrics	defLinkMetrics = { 0, 0, 0, 0 };

static	void
#ifdef _NO_PROTO
DefCvsMetrics (client_data, ret_canvas, ret_font, ret_link, ret_traversal)
    ClientData			 client_data;
    CanvasMetrics		*ret_canvas;
    CanvasFontMetrics		*ret_font;
    CanvasLinkMetrics		*ret_link;
    CanvasTraversalMetrics	*ret_traversal;
#else
DefCvsMetrics (
    ClientData			 client_data,
    CanvasMetrics		*ret_canvas,
    CanvasFontMetrics		*ret_font,
    CanvasLinkMetrics		*ret_link,
    CanvasTraversalMetrics	*ret_traversal)
#endif /* _NO_PROTO */
{
    if (ret_canvas)
      {
        ret_canvas->width          = 1;
        ret_canvas->height         = 1;
	ret_canvas->leading        = 0;
	ret_canvas->top_margin     = 0;
	ret_canvas->line_thickness = 0;
      }
    if (ret_font)
	*ret_font = defFontMetrics;

    if (ret_link)
	*ret_link = defLinkMetrics;

    if (ret_traversal)
	*ret_traversal = defLinkMetrics;
}


#ifdef	not_used
/*****************************************************************************
 * Function:	void DefCvsGetFont ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
static	void
#ifdef _NO_PROTO
DefCvsGetFont (client_data, lang, charset, font_attr, mod_string, ret_font)
    ClientData		 client_data;
    char		*lang;
    char		*charset;
    CanvasFontSpec	 font_attr;
    char		**mod_string;
    FontPtr		*ret_font;
#else
DefCvsGetFont (
    ClientData		 client_data,
    char		*lang,
    char		*charset,
    CanvasFontSpec	 font_attr,
    char		**mod_string,
    FontPtr		*ret_font )
#endif /* _NO_PROTO */
{
    *ret_font = (FontPtr) 0;
}

/*****************************************************************************
 * Function:	VStatus DefCvsResolveSpc ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
static	VStatus
#ifdef _NO_PROTO
DefCvsResolveSpc (client_data, lang, charset, font_attr, spc_symbol,
		   ret_handle, ret_width, ret_height, ret_ascent)
    ClientData		 client_data;
    char		*lang;
    char		*charset;
    CanvasFontSpec	 font_attr;
    char		*spc_symbol;
    SpecialPtr		*ret_handle;
    Unit		*ret_width;
    Unit		*ret_height;
    Unit		*ret_ascent;
#else
DefCvsResolveSpc (
    ClientData		 client_data,
    char		*lang,
    char		*charset,
    CanvasFontSpec	 font_attr,
    char		*spc_symbol,
    SpecialPtr		*ret_handle,
    Unit		*ret_width,
    Unit		*ret_height,
    Unit		*ret_ascent)
#endif /* _NO_PROTO */
{
    const char	*spcStr;

    spcStr = _DtHelpCeResolveSpcToAscii (spc_symbol);

    if (spcStr != NULL)
      {
	*ret_handle = ((SpecialPtr)(spcStr));
	*ret_height = 1;
	*ret_ascent = 1;
	*ret_width  = strlen(spcStr);

	return 0;
      }

    return -1;
}

/*****************************************************************************
 * Function:	void DefCvsFontMetrics ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
static	void
#ifdef _NO_PROTO
DefCvsFontMetrics (client_data, font_ptr,
		ret_ascent, ret_descent, char_width, ret_super, ret_sub)
    ClientData	 client_data;
    FontPtr	 font_ptr;
    Unit	*ret_ascent;
    Unit	*ret_descent;
    Unit	*char_width;
    Unit	*ret_super;
    Unit	*ret_sub;
#else
DefCvsFontMetrics (
    ClientData	 client_data,
    FontPtr	 font_ptr,
    Unit	*ret_ascent,
    Unit	*ret_descent,
    Unit	*char_width,
    Unit	*ret_super,
    Unit	*ret_sub)
#endif /* _NO_PROTO */
{
    if (ret_ascent)
	*ret_ascent = 1;
    if (ret_descent)
	*ret_descent = 0;
    if (char_width)
	*char_width = 0;
    if (ret_super)
	*ret_super = 0;
    if (ret_sub)
	*ret_sub = 0;

    return;
}

/*****************************************************************************
 * Function:	Unit DefCvsStrWidth ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
static	Unit
#ifdef _NO_PROTO
DefCvsStrWidth (client_data, string, byte_len, char_len, font_ptr)
    ClientData	 client_data;
    char	*string;
    int		 byte_len;
    int		 char_len;
    FontPtr	 font_ptr;
#else
DefCvsStrWidth (
    ClientData	 client_data,
    char	*string,
    int		 byte_len,
    int		 char_len,
    FontPtr	 font_ptr )
#endif /* _NO_PROTO */
{
   return ((Unit)(byte_len/char_len));
}
#endif /* not_used */

/*****************************************************************************
 *		Public Functions
 *****************************************************************************/
/*****************************************************************************
 * Function:	CanvasHandle _DtHelpCeCreateDefCanvas (void);
 *
 * Parameters:
 *
 * Returns:	A handle to the canvas or NULL if an error occurs.
 *
 * Purpose:	Create a canvas and attach the appropriate virtual functions
 *		to the canvas.
 *****************************************************************************/
CanvasHandle
#ifdef _NO_PROTO
_DtHelpCeCreateDefCanvas ()
#else
_DtHelpCeCreateDefCanvas (void)
#endif /* _NO_PROTO */
{
    return (_DtHelpCeCreateCanvas(MyVirtInfo, NULL));

} /* End _DtHelpCeCreateDefCanvas */
