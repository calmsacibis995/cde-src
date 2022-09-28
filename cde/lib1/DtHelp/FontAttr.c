/* $XConsortium: FontAttr.c /main/cde1_maint/1 1995/07/17 17:28:57 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:	Font.c
 **
 **   Project:     Text Graphic Display Library
 **
 **   Description: Semi private format utility functions that do not
 **		   require the Display Area, Motif, Xt or X11.
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

/*
 * private includes
 */
#include "Canvas.h"
#include "FontAttrI.h"

#ifdef NLS16
#endif

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO
#else
#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

/******************************************************************************
 *
 * Private variables and defines.
 *
 *****************************************************************************/
static	CEFontSpec	DefaultFontAttrs =
  {
	"C",
	"ISO-8859-1",
	{ 10, 0,				/* pointsz, setsize */
	  NULL,					/* color            */
	  NULL, NULL, NULL, NULL,		/* xlfd strings     */
	  NULL, NULL, NULL, NULL,		/* MS-Win strings   */
	  CEFontStyleSanSerif, CEFontSpacingProp, CEFontWeightMedium,
	  CEFontSlantRoman   , CEFontSpecialNone, NULL
	}
  };

/******************************************************************************
 *
 * Semi Public variables.
 *
 *****************************************************************************/
/******************************************************************************
 *
 * Private Functions
 *
 *****************************************************************************/
/******************************************************************************
 *
 * Semi Public Functions
 *
 *****************************************************************************/
/******************************************************************************
 * Function:	void _DtHelpCeCopyDefFontAttrList (char **font_attr)
 *
 * Parameters:	font_attr	Specifies the font attribute list
 *
 * Return Value: void
 *
 * Purpose:	Initialize a font attribute list to the default.
 *		Sets '_DtMB_LEN_MAX' to the default character size.
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeCopyDefFontAttrList ( font_attr )
    CEFontSpec    *font_attr;
#else
_DtHelpCeCopyDefFontAttrList (CEFontSpec *font_attr )
#endif /* _NO_PROTO */
{
    *font_attr = DefaultFontAttrs;
}
