/* $XConsortium: FontAttrI.h /main/cde1_maint/1 1995/07/17 17:29:20 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **  File:  FontAttrI.h
 **  Project:  Common Desktop Environment
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _CEFontAttrI_h
#define _CEFontAttrI_h

/*
 * font attributes
 */
#define _CEFONT_CHAR_SET       6
#define _CEFONT_LANG_TER       5
#define _CEFONT_TYPE           4
#define _CEFONT_WEIGHT         3
#define _CEFONT_ANGLE          2
#define _CEFONT_SIZE           1
#define _CEFONT_SPACING        0

#define _CEFONT_END            7

#define _CEFontAttrNumber     8

typedef struct {
        char    *lang;
        char    *char_set;
        CanvasFontSpec font_specs;
} CEFontSpec;

/********    Public Function Declarations    ********/
#ifdef _NO_PROTO
extern	void	_DtHelpCeCopyDefFontAttrList();
#else
extern	void	_DtHelpCeCopyDefFontAttrList(
				CEFontSpec *font_attr );
#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

#endif /* _CEFontAttrI_h */
