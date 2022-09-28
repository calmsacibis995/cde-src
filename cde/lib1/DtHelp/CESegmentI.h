/* $XConsortium: CESegmentI.h /main/cde1_maint/1 1995/07/17 17:21:40 drk $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:   CESegmentI.h
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
 *******************************************************************
 *************************************<+>*************************************/
#ifndef _DtHelpSegmentI_h
#define _DtHelpSegmentI_h

#include "SDLI.h"
#include "LinkMgrI.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 *			Private Defines Declarations
 *****************************************************************************/
/*****************************************************************************
 *		CESegment Primary type description
 *
 *	                               |---- Primary Type
 *	                               v
 *	----------------------------------
 *	|                            |...|
 *	----------------------------------
 *
 *****************************************************************************/
		/*------ Mask -------*/
#define	CE_PRIMARY_MASK		(0x07)

		/*------ Values -------*/
#define CE_NOOP		(0x00)
#define CE_STRING	(0x01)
#define CE_LABEL	(0x02)		/* CCDF value */
#define CE_GRAPHIC	(0x03)
#define CE_CAPTION	(0x04)		/* CCDF value */
#define CE_ELEMENT	(0x05)
#define CE_SPECIAL	(0x06)

		/*------ Set -------*/
#define	_CESetCaption(x)	(((x) & ~(CE_PRIMARY_MASK)) | CE_CAPTION)
#define	_CESetGraphic(x)	(((x) & ~(CE_PRIMARY_MASK)) | CE_GRAPHIC)
#define	_CESetLabel(x)		(((x) & ~(CE_PRIMARY_MASK)) | CE_LABEL)
#define	_CESetNoop(x)		(((x) & ~(CE_PRIMARY_MASK)) | CE_NOOP)
#define	_CESetString(x)		(((x) & ~(CE_PRIMARY_MASK)) | CE_STRING)
#define	_CESetSpecial(x)	(((x) & ~(CE_PRIMARY_MASK)) | CE_SPECIAL)

		/*------ Get -------*/
#define	_CEGetPrimaryType(x)	((x) & CE_PRIMARY_MASK)

		/*------ Shift Past-------*/
#define	CE_SHIFT_PAST_PRIMARY	3

		/*------ Is ----------*/
#define _CEIsNoop(x) \
		((((x) & CE_PRIMARY_MASK) == CE_NOOP) ? 1 : 0)
#define _CEIsLabel(x) \
		((((x) & CE_PRIMARY_MASK) == CE_LABEL) ? 1 : 0)
#define _CEIsGraphic(x) \
		((((x) & CE_PRIMARY_MASK) == CE_GRAPHIC) ? 1 : 0)
#define _CEIsCaption(x) \
		((((x) & CE_PRIMARY_MASK) == CE_CAPTION) ? 1 : 0)
#define _CEIsElement(x) \
		((((x) & CE_PRIMARY_MASK) == CE_ELEMENT) ? 1 : 0)
#define _CEIsSpecial(x) \
		((((x) & CE_PRIMARY_MASK) == CE_SPECIAL) ? 1 : 0)
#define _CEIsString(x) \
		((((x) & CE_PRIMARY_MASK) == CE_STRING) ? 1 : 0)

		/*------ Is Not-------*/
#define _CEIsNotCaption(x) \
		((((x) & CE_PRIMARY_MASK) != CE_CAPTION) ? 1 : 0)

#define _CEIsNotElement(x) \
		((((x) & CE_PRIMARY_MASK) != CE_ELEMENT) ? 1 : 0)

#define _CEIsNotGraphic(x) \
		((((x) & CE_PRIMARY_MASK) != CE_GRAPHIC) ? 1 : 0)

#define _CEIsNotLabel(x) \
		((((x) & CE_PRIMARY_MASK) != CE_LABEL) ? 1 : 0)

#define _CEIsNotNoop(x) \
		((((x) & CE_PRIMARY_MASK) != CE_NOOP) ? 1 : 0)

#define _CEIsNotString(x) \
		((((x) & CE_PRIMARY_MASK) != CE_STRING) ? 1 : 0)

#define _CEIsNotSpecial(x) \
		((((x) & CE_PRIMARY_MASK) != CE_SPECIAL) ? 1 : 0)

/*****************************************************************************
 *		Generic Flags
 *
 *	                    ---------------- Newline Flag
 *	                    | -------------- Ghostlink Flag (for the TOC)
 *	                    | | ------------ Hypertext Flag
 *	                    | | | ---------- DuplicateFlag
 *	                    | | | | -------- Initialized Flag
 *	                    | | | | |  |---- Primary Type
 *	                    v v v v v  v
 *	----------------------------------
 *	|                |.|.|.|.|.|.|---|
 *	----------------------------------
 *                        ^
 *                        |----------------- Non Breaking Character (SDL only)
 *
 *****************************************************************************/
		/*------ Flag -------*/
#define	CE_INIT_FLAG		(0x01 << CE_SHIFT_PAST_PRIMARY)
#define	CE_DUPLICATE_FLAG	(0x01 << (CE_SHIFT_PAST_PRIMARY + 1))
#define	CE_HYPERTEXT_FLAG	(0x01 << (CE_SHIFT_PAST_PRIMARY + 2))
#define	CE_GHOSTLINK_FLAG	(0x01 << (CE_SHIFT_PAST_PRIMARY + 3))
#define	CE_NEWLINE_FLAG		(0x01 << (CE_SHIFT_PAST_PRIMARY + 4))
#define	CE_NON_BREAK_FLAG	(0x01 << (CE_SHIFT_PAST_PRIMARY + 5))

		/*------ Shift Past -------*/
#define	CE_SHIFT_PAST_GENERIC_FLAGS	(CE_SHIFT_PAST_PRIMARY + 6)

		/*------ Clear -------*/
#define	_CEClearInitialized(x)		((x) & ~(CE_INIT_FLAG))
#define	_CEClearHyperFlag(x) \
			((x) & ~(CE_HYPERTEXT_FLAG | CE_GHOSTLINK_FLAG))
#define	_CEClearNewline(x)		((x) & ~(CE_NEWLINE_FLAG))
#define	_CEClearBreakFlag(x)		((x) & ~(CE_NON_BREAK_FLAG))

		/*------ Set -------*/
#define	_CESetInitialized(x)		((x) | CE_INIT_FLAG)
#define	_CESetDuplicate(x)		((x) | CE_DUPLICATE_FLAG)
#define	_CESetHyperFlag(x)		((x) | CE_HYPERTEXT_FLAG)
#define	_CESetGhostLink(x)		((x) | CE_GHOSTLINK_FLAG)
#define	_CESetNewLineFlag(x)		((x) | CE_NEWLINE_FLAG)
#define	_CESetNonBreakFlag(x)		((x) | CE_NON_BREAK_FLAG)

		/*------ Is ----------*/
#define _CEIsInitialized(x)		(((x) & CE_INIT_FLAG) ? 1 : 0)
#define _CEIsDuplicate(x)		(((x) & CE_DUPLICATE_FLAG) ? 1 : 0)
#define _CEIsVisibleLink(x)		(((x) & CE_HYPERTEXT_FLAG) ? 1 : 0)
#define _CEIsHyperLink(x) \
		(((x) & (CE_HYPERTEXT_FLAG | CE_GHOSTLINK_FLAG)) ? 1 : 0)
#define _CEIsNewLine(x)			(((x) & CE_NEWLINE_FLAG) ? 1 : 0)
#define _CEIsNonBreakingChar(x)		(((x) & CE_NON_BREAK_FLAG) ? 1 : 0)

		/*------ Is Not ----------*/
#define	_CEIsNotInitialized(x)	(((x) & CE_INIT_FLAG) ? 0 : 1)
#define	_CEIsNotDuplicate(x)	(((x) & CE_DUPLICATE_FLAG) ? 0 : 1)
#define	_CEIsNotNewLine(x)	(((x) & CE_NEWLINE_FLAG) ? 0 : 1)
#define _CEIsNotBreakingChar(x)	(((x) & CE_NON_BREAK_FLAG) ? 0 : 1)

/*---------------------------------------------------------------------------*
 *			Spc Valid Flag
 *
 *	                    ---------------- Newline Flag
 *	                    | -------------- Ghostlink Flag (for the TOC)
 *	                    | | ------------ Hypertext Flag
 *	                    | | | ---------- DuplicateFlag
 *	                    | | | | -------- Initialized Flag
 *	                    | | | | |  |---- Primary Type (all but graphic)
 *	                    v v v v v  v
 *	----------------------------------
 *	|              |.|-|-|-|-|-|-|0x6|
 *	----------------------------------
 *                      ^ ^
 *                      | |----------------- Non Breaking Character (SDL only)
 *	                |----------- Valid Spc Flag
 *
 *---------------------------------------------------------------------------*/
		/*------ Flag -------*/
#define	CE_VALID_SPC		(0x01 << CE_SHIFT_PAST_GENERIC_FLAGS)

		/*------ Shift Past -------*/
#define	CE_SHIFT_PAST_VALID_SPC	(CE_SHIFT_PAST_GENERIC_FLAGS + 1)

		/*------ Set -------*/
#define _CESetValidSpc(x)	((x) | CE_VALID_SPC)

		/*------ Clear -------*/
#define _CEClearValidSpc(x)	((x) & ~(CE_VALID_SPC))

		/*------ Is ----------*/
#define _CEIsValidSpc(x)	(((x) & CE_VALID_SPC) ? 1 : 0)

		/*------ Is Not --------*/
#define	_CEIsNotValidSpc(x)	(((x) & CE_VALID_SPC) ? 0 : 1)

/*---------------------------------------------------------------------------*
 *			Static Flag
 *
 *	                    ---------------- Newline Flag
 *	                    | -------------- Ghostlink Flag (for the TOC)
 *	                    | | ------------ Hypertext Flag
 *	                    | | | ---------- DuplicateFlag
 *	                    | | | | -------- Initialized Flag
 *	                    | | | | |  |---- Primary Type (all but graphic)
 *	                    v v v v v  v
 *	----------------------------------
 *	|            |.|-|-|-|-|-|-|-|0x?|
 *	----------------------------------
 *                    ^ ^ ^
 *                    | | |----------------- Non Breaking Character (SDL only)
 *                    | |----------- Valid Spc Flag
 *	              --------- Static String Flag
 *
 *
 *---------------------------------------------------------------------------*/
		/*------ Flag -------*/
#define	CE_STATIC_FLAG		(0x01 << CE_SHIFT_PAST_VALID_SPC)

		/*------ Shift Past -------*/
#define	CE_SHIFT_PAST_STATIC	(CE_SHIFT_PAST_VALID_SPC + 1)

		/*------ Set -------*/
#define _CESetStaticFlag(x)	((x) | CE_STATIC_FLAG)

		/*------ Clear -------*/
#define _CEClearStaticFlag(x)	((x) & ~(CE_STATIC_FLAG))

		/*------ Is ----------*/
#define _CEIsStaticString(x)	(((x) & CE_STATIC_FLAG) ? 1 : 0)

		/*------ Is Not --------*/
#define	_CEIsNotStaticString(x)	(((x) & CE_STATIC_FLAG) ? 0 : 1)

/*---------------------------------------------------------------------------*
 *			Char Info
 *
 *	                    ---------------- Newline Flag
 *	                    | -------------- Ghostlink Flag (for the TOC)
 *	                    | | ------------ Hypertext Flag
 *	                    | | | ---------- DuplicateFlag
 *	                    | | | | -------- Initialized Flag
 *	                    | | | | |  |---- Primary Type (all but graphic)
 *	                    v v v v v  v
 *	----------------------------------
 *	|         |..|-|-|-|-|-|-|-|-|0x?|
 *	----------------------------------
 *                 ^  ^ ^ ^
 *                 |  | | |----------------- Non Breaking Character (SDL only)
 *                 |  | |----------- Valid Spc Flag
 *	           |  |-------- Static String Flag
 *	           ------------ Char Size (n - 1)
 *
 *---------------------------------------------------------------------------*/
		/*------ Shift to -------*/
#define	CE_SHIFT_TO_CHAR_SIZE	(CE_SHIFT_PAST_STATIC)

		/*------ Shift Past -------*/
#define	CE_SHIFT_PAST_CHAR_SIZE	(CE_SHIFT_TO_CHAR_SIZE + 2)

		/*------ Mask -------*/
#define	CE_CHAR_SIZE_MASK	(0x03 << CE_SHIFT_TO_CHAR_SIZE)

		/*------ Set Info -------*/
#define	_CESetCharLen(x,y) \
	(((x) & ~(CE_CHAR_SIZE_MASK)) | (((y) - 1) << CE_SHIFT_TO_CHAR_SIZE))

		/*------ Get Info -------*/
#define	_CEGetCharLen(x) \
	((((x) & (CE_CHAR_SIZE_MASK)) >> CE_SHIFT_TO_CHAR_SIZE) + 1)

/*---------------------------------------------------------------------------*
 *			End On Newline Flag
 *
 *	                    ---------------- Newline Flag
 *	                    | -------------- Ghostlink Flag (for the TOC)
 *	                    | | ------------ Hypertext Flag
 *	                    | | | ---------- DuplicateFlag
 *	                    | | | | -------- Initialized Flag
 *	                    | | | | |  |---- Primary Type (all but graphic)
 *	                    v v v v v  v
 *	----------------------------------
 *	|       | |--|-|-|-|-|-|-|-|-|0x?|
 *	----------------------------------
 *               ^ ^  ^ ^ ^
 *               | |  | | |----------------- Non Breaking Character (SDL only)
 *               | |  | |----------- Valid Spc Flag
 *	         | |  |-------- Static String Flag
 *	         | ------------ Char Size (n - 1)
 *               -------------- File & String manipulation flag
 *
 *---------------------------------------------------------------------------*/
		/*------ Flag -------*/
#define	CE_EOL_FLAG		(0x01 << CE_SHIFT_PAST_CHAR_SIZE)

		/*------ Shift Past -------*/
#define	CE_SHIFT_PAST_EOL_FLAG	(CE_SHIFT_PAST_CHAR_SIZE + 1)

		/*------ Set ------------*/
#define	_CESetEolFlag(x)	((x) | CE_EOL_FLAG)

		/*------ Is -------------*/
#define	_CEEndOnNewLine(x)	(((x) & CE_EOL_FLAG) ? 1 : 0)

/*---------------------------------------------------------------------------*
 *			Caption attributes
 *			CCDF only
 *
 *	                    ---------------- Newline Flag
 *	                    | -------------- Ghostlink Flag (for the TOC)
 *	                    | | ------------ Hypertext Flag
 *	                    | | | ---------- DuplicateFlag
 *	                    | | | | -------- Initialized Flag
 *	                    | | | | |  |---- Primary Type (captions)
 *	                    v v v v v  v
 *	----------------------------------
 *	|  |..|.|-|-|--|0|-|-|-|-|-|-|0x4|
 *	----------------------------------
 *           ^ ^ ^ ^  ^ ^ ^
 *           | | | |  | | |----------------- Non Breaking Character (SDL only)
 *           | | | |  | |----------- Valid Spc Flag
 *	     | | | |  |-------- Static String Flag
 *	     | | | ------------ Char Size (n - 1)
 *           | | -------------- File & String manipulation flag
 *	     | ---------------- Caption Top/Bottom positioning
 *	     ------------------ Caption positions between margins
 *
 *---------------------------------------------------------------------------*/
		/*------ Flag -------*/
#define	CE_CAPTION_TOP_FLAG	(0x01 << CE_SHIFT_PAST_EOL_FLAG)

		/*------ Shift to -------*/
#define	CE_SHIFT_TO_CAPTION_POSITION	(CE_SHIFT_PAST_EOL_FLAG + 1)

		/*------ Shift Past -------*/
#define	CE_SHIFT_PAST_CAPTION_POSITION	(CE_SHIFT_TO_CAPTION_POSITION + 2)

		/*------ Set -------*/
#define	_CESetCaptionAtTop(x)		((x) | CE_CAPTION_TOP_FLAG)
#define	_CESetCaptionAtBottom(x)	((x) & ~(CE_CAPTION_TOP_FLAG))

		/*------ Is -------*/
#define	_CEIsCaptionAtTop(x)	(((x) & CE_CAPTION_TOP_FLAG) ? 1 : 0)
#define	_CEIsCaptionNotAtTop(x)	(((x) & CE_CAPTION_TOP_FLAG) ? 0 : 1)

		/*------ Mask -------*/
#define	CE_CAPTION_POSITION_MASK	(0x03 << CE_SHIFT_TO_CAPTION_POSITION)

		/*------ Values -------*/
#define CE_CAPTION_CENTER      (0x00)
#define CE_CAPTION_LEFT        (0x01)
#define CE_CAPTION_RIGHT       (0x02)

		/*------ Set -------*/
#define	_CESetCaptionPosition(x,y) \
		(((x) & ~(CE_CAPTION_POSITION_MASK)) | \
					((y) << CE_SHIFT_TO_CAPTION_POSITION))

#define _CESetCaptionInfo(x,y) \
		(_CESetCaptionPosition(_CESetCaption(x), y))

		/*------ Get -------*/
#define	_CEGetCaptionPosition(x) \
	(((x) & CE_CAPTION_POSITION_MASK) >> CE_SHIFT_TO_CAPTION_POSITION)

/*---------------------------------------------------------------------------*
 *			Super/Sub script attributes
 *			SDL only.
 *
 *	                    ---------------- Newline Flag
 *	                    | -------------- Ghostlink Flag (for the TOC)
 *	                    | | ------------ Hypertext Flag
 *	                    | | | ---------- DuplicateFlag
 *	                    | | | | -------- Initialized Flag
 *	                    | | | | |  |---- Primary Type (string)
 *	                    v v v v v  v
 *	----------------------------------
 *	|   |.|.|-|--|-|-|-|-|-|-|-|-|0x?|
 *	----------------------------------
 *           ^ ^ ^ ^  ^ ^ ^
 *           | | | |  | | |----------------- Non Breaking Character (SDL only)
 *           | | | |  | |----------- Valid Spc Flag
 *	     | | | |  |-------- Static String Flag
 *	     | | | ------------ Char Size (n - 1)
 *           | | -------------- File & String manipulation flag
 *	     | ---------------- Superscript flag.
 *	     ------------------ Subscript flag.
 *
 *---------------------------------------------------------------------------*/
		/*------ Flag -------*/
#define	CE_SUPER_FLAG	(0x01 << CE_SHIFT_PAST_EOL_FLAG)
#define	CE_SUB_FLAG	(0x01 << (CE_SHIFT_PAST_EOL_FLAG + 1))

		/*------ Set -------*/
#define	_CESetSuperFlag(x)	((x) & CE_SUPER_FLAG)
#define	_CESetSubFlag(x)	((x) & CE_SUB_FLAG)

		/*------ Is -------*/
#define	_CEIsSuperScript(x)	(((x) & CE_SUPER_FLAG) ? 1 : 0)
#define	_CEIsSubScript(x)	(((x) & CE_SUB_FLAG) ? 1 : 0)


/*---------------------------------------------------------------------------*
 *			CE_GRAPHIC type attribute
 *
 *	                    ---------------- Newline Flag
 *	                    | -------------- Ghostlink Flag (for the TOC)
 *	                    | | ------------ Hypertext Flag
 *	                    | | | ---------- DuplicateFlag
 *	                    | | | | -------- Initialized Flag
 *	                    | | | | |  |---- Primary Type (graphic)
 *	                    v v v v v  v
 *	----------------------------------
 *	|             |..|-|-|-|-|-|-|0x3|
 *	----------------------------------
 *                     ^  ^
 *                     |  |----------------- Non Breaking Character (SDL only)
 *	               |--------- Graphic Type
 *
 *---------------------------------------------------------------------------*/
		/*------ Shift to -------*/
#define	CE_SHIFT_TO_GRAPHIC_TYPE	(CE_SHIFT_PAST_GENERIC_FLAGS)

		/*------ Shift Past -------*/
#define	CE_SHIFT_PAST_GRAPHIC_TYPE	(CE_SHIFT_TO_GRAPHIC_TYPE + 2)

		/*------ Mask -------*/
#define	CE_GRAPHIC_MASK			(0x03 << CE_SHIFT_TO_GRAPHIC_TYPE)

		/*------ Values -------*/
#define CE_ALONE_CENTERED      (0x00)
#define CE_IN_LINE             (0x01)
#define CE_LEFT_JUSTIFIED      (0x02)
#define CE_RIGHT_JUSTIFIED     (0x03)

		/*------ Set -------*/
#define	_CESetGraphicType(x,y) \
		(((x) & ~(CE_GRAPHIC_MASK)) | ((y) << CE_SHIFT_TO_GRAPHIC_TYPE))

#define	_CESetGraphicInfo(x,y)	(_CESetGraphicType(_CESetGraphic(x), y))

		/*------ Get -------*/
#define	_CEGetGraphicType(x) \
		(((x) & CE_GRAPHIC_MASK) >> CE_SHIFT_TO_GRAPHIC_TYPE)

/*---------------------------------------------------------------------------*
 *			CE_GRAPHIC postion attribute
 *
 *	                    ---------------- Newline Flag
 *	                    | -------------- Ghostlink Flag (for the TOC)
 *	                    | | ------------ Hypertext Flag
 *	                    | | | ---------- DuplicateFlag
 *	                    | | | | -------- Initialized Flag
 *	                    | | | | |  |---- Primary Type (graphic)
 *	                    v v v v v  v
 *	----------------------------------
 *	|          |..| 0|-|-|-|-|-|-|0x3|
 *	----------------------------------
 *                  ^  ^  ^
 *                  |  |  |----------------- Non Breaking Character (SDL only)
 *	            |  |--------- Graphic Type (Standalone)
 *	            ------------- Standalone Graphic Position
 *
 *---------------------------------------------------------------------------*/
		/*------ Shift to -------*/
#define	CE_SHIFT_TO_GRAPHIC_POSITION	(CE_SHIFT_PAST_GRAPHIC_TYPE)

		/*------ Shift Past -------*/
#define	CE_SHIFT_PAST_GRAPHIC_POSITION	(CE_SHIFT_TO_GRAPHIC_POSITION + 2)

		/*------ Mask -------*/
#define	CE_GR_POSITION_MASK		(0x03 << CE_SHIFT_TO_GRAPHIC_POSITION)

		/*------ Values -------*/
#define CE_AL_CENTER           (0x00)
#define CE_AL_LEFT             (0x01)
#define CE_AL_RIGHT            (0x02)

		/*------ Set -------*/
#define _CESetGraphicPosition(x,y) \
	(((x) & ~(CE_GR_POSITION_MASK)) | ((y) << CE_SHIFT_TO_GRAPHIC_POSITION))

		/*------ Get -------*/
#define	_CEGetGraphicPosition(x) \
	(((x) & CE_GR_POSITION_MASK) >> CE_SHIFT_TO_GRAPHIC_POSITION)

/*****************************************************************************
 *			Private Define Declarations
 *****************************************************************************/
/*****************************************************************************
 *			Private Structures Declarations
 *****************************************************************************/

		/* ------ Individual Segments ------- */

typedef	struct _ceString {
	char		*string;
	FontPtr		 font_handle;
	CanvasFontSpec	*font_specs;
} CEString;

typedef	struct {
	SpecialPtr	spc_handle;
	Unit		width;
	Unit		height;
	Unit		ascent;
} CESpecial;

typedef struct _ceGraphics {
	int	 type;
	int	 lnk_idx;
	Unit     pos_x;
	Unit     pos_y;
	Unit     width;
	Unit     height;
	void    *handle;
} CEGraphics;

typedef	struct	_ceSegment	CESegment;

typedef struct _ceElement{
	enum SdlElement  el_type;
	CEBoolean	 format_break;
	CESDLAttrStruct  attributes;
	CESegment	*seg_list;
} CEElement;

		/* ------ Generic Segment ------- */

typedef	union	seg_handles {
	CEElement	*el_handle;
	CEString	*str_handle;
	CESpecial	*spc_handle;
	char		*file_handle;
	void		*generic;
	int		 grph_idx;
} CESegHandles;

struct	_ceSegment {
	CEBoolean	   top;
	unsigned int	   seg_type;
	int		   link_index;
	CESegHandles	   seg_handle;
	struct _ceSegment *next_seg;
	struct _ceSegment *next_disp;
};

		/* ------ Backwards Compatability ------- */

typedef	struct {
	int		 first_indent;
	int		 left_margin;
	int		 right_margin;
	int		 spacing;
	int		 lines_before;
	int		 lines_after;
	CESegment	*seg_list;
} CEParagraph;

typedef	struct {
	int		 para_num;
	char		*name;
	char		*id_str;
	CEParagraph	*para_list;
	CESegment	*id_seg;
	CESegment	*toss;
	CELinkData	 link_data;
	void		*element_list;
} CETopicStruct;

/*****************************************************************************
 *			Private Macro Declarations
 *****************************************************************************/
#ifndef _CEStringOfSegment
#define _CEStringOfSegment(x)   ((x)->seg_handle.str_handle->string)
#endif

#ifndef _CEFontOfSegment
#define _CEFontOfSegment(x)  ((x)->seg_handle.str_handle->font_handle)
#endif

#ifndef _CEElementOfSegment
#define _CEElementOfSegment(x)  ((x)->seg_handle.el_handle)
#endif

#ifndef _CEElementTypeOfSegment
#define _CEElementTypeOfSegment(x)  (((x)->seg_handle.el_handle)->el_type)
#endif

#ifndef	_CESpcOfSegment
#define _CESpcOfSegment(x)  ((x)->seg_handle.spc_handle)
#endif

/*****************************************************************************
 *			Private Function Declarations
 *****************************************************************************/

#ifdef _NO_PROTO
#else
#endif /* _NO_PROTO */

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _DtHelpSegmentI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
