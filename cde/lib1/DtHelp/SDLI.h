/* $XConsortium: SDLI.h /main/cde1_maint/3 1995/10/08 17:20:45 pascale $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:   SDLI.h
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
#ifndef _DtHelpSDLI_h
#define _DtHelpSDLI_h


#ifdef __cplusplus
extern "C" {
#endif

#include "Canvas.h"

/*****************************************************************************
 * SDL DTD versioning defines
 *****************************************************************************/
/*
 * The first version
 */
#define	SDL_DTD_VERSION		1

/*
 * The first minor version
 */
#define SDL_DTD_1_0		0

/*
 * Virpage levels went from 1-n.
 * Any virpage with a level of
 * zero (or less) are hidden.
 */
#define SDL_DTD_1_1		1

/*****************************************************************************
 * Attribute type enumerations
 *****************************************************************************/
enum	SdlAttrDataType {
/* bad data type */
	SdlAttrDataTypeInvalid,
/* valid data types */
	SdlAttrDataTypeId  , SdlAttrDataTypeNumber, SdlAttrDataTypeCdata,
	SdlAttrDataTypeFont, SdlAttrDataTypeEnum };

/*****************************************************************************
 * Option enumerations
 *****************************************************************************/
enum	SdlOption
  {
/* bad option indicator */
	SdlOptionBad    ,

/* Boolean */
	SdlBoolYes      , SdlBoolNo     ,

/* key classes */
	SdlClassAcro    , SdlClassBook    , SdlClassEmph    , SdlClassJargon ,
	SdlClassL10n    , SdlClassName    , SdlClassQuote   , SdlClassProduct,
	SdlClassTermdef , SdlClassTerm    , SdlClassMachIn  , SdlClassMachOut,
	SdlClassMachCont, SdlClassPubLit  , SdlClassUdefkey ,

/* head classes */
	SdlClassLabel     , SdlClassHead , SdlClassCaption ,
	SdlClassAnnotation, SdlClassPHead, SdlClassUdefhead,

/* format classes */
	SdlClassTable   , SdlClassCell    , SdlClassList,
	SdlClassItem    , SdlClassText    , SdlClassUdeffrmt,

/* graphic classes */
	SdlClassFigure, SdlClassInLine  , SdlClassButton  ,
	SdlClassIcon  , SdlClassUdefgraph,

/* phrase styles */
	SdlClassSuper   , SdlClassSub     ,

/* apps types */
	SdlAppAll       , SdlAppHelp      , SdlAppTutorial  ,
	SdlAppRef       , SdlAppSys       ,

/* timing */
	SdlTimingSync   , SdlTimingAsync  ,

/* types */
	SdlTypeLiteral  , SdlTypeLined    ,
	SdlTypeDynamic  , SdlTypeUnlinedLiteral,
	SdlTypeCdata    ,

/* Borders */
	SdlBorderNone   , SdlBorderFull   , SdlBorderHorz   , SdlBorderVert  ,
	SdlBorderTop    , SdlBorderBottom , SdlBorderLeft   , SdlBorderRight ,

/* horizontal and head justification */
	SdlJustifyLeft  , SdlJustifyRight , SdlJustifyCenter, SdlJustifyNum  ,

/* vertical and head justification */
	SdlJustifyTop   , SdlJustifyBottom,

/* head justification */
	SdlJustifyLeftCorner, SdlJustifyRightCorner,
	SdlJustifyLeftMargin, SdlJustifyRightMargin,

/* flow */
	SdlWrap          , SdlNoWrap   , SdlWrapJoin   ,
/* links */
	SdlWindowCurrent , SdlWindowNew, SdlWindowPopup,
/* traversal */
	SdlTraversalReturn, SdlTraversalNoReturn,
/* <id> types */
	SdlIdVirpage , SdlIdBlock   , SdlIdForm    , SdlIdPara    ,
	SdlIdCPara   , SdlIdHead    , SdlIdSubHead , SdlIdAnchor  ,
	SdlIdSwitch  , SdlIdSnRef   , SdlIdGraphic , SdlIdText    ,
	SdlIdAudio   , SdlIdVideo   , SdlIdAnimate ,
	SdlIdCrossDoc, SdlIdManPage , SdlIdTextFile,
	SdlIdSysCmd  , SdlIdScript  , SdlIdCallback,
/* <script> interpreter types */
	SdlInterpKsh, SdlInterpTcl, SdlInterpCsh, SdlInterpSh,
/* <headstyle> placement types */
	SdlPlaceParent, SdlPlaceObject,
/* <headstyle> stack types */
	SdlStackHoriz , SdlStackVert
    };

/*****************************************************************************
 * SDL Element enumerations
 *****************************************************************************/
enum	SdlElement
    {
	SdlElementNone,
	SdlElementComment,
	SdlElementSdlDoc,
	SdlElementVStruct,
	SdlElementVirpage,
	SdlElementHead,
	SdlElementSubHead,
	SdlElementSnb,
	SdlElementBlock,
	SdlElementForm,
	SdlElementPara,
	SdlElementCPara,
	SdlElementFdata,
	SdlElementFstyle,
	SdlElementFrowvec,
	SdlElementKey,
	SdlElementCdata,
/* System notations */
	SdlElementGraphic,
	SdlElementText,
	SdlElementAudio,
	SdlElementVideo,
	SdlElementAnimate,
	SdlElementCrossDoc,
	SdlElementManPage,
	SdlElementTextFile,
	SdlElementSysCmd,
	SdlElementCallback,
	SdlElementScript,
/* System notations (end) */
	SdlElementAnchor,
	SdlElementLink,
	SdlElementSwitch,
	SdlElementSnRef,
	SdlElementRefItem,
	SdlElementAltText,
	SdlElementSphrase,
	SdlElementRev,
	SdlElementSpc,
/* Conditionals */
	SdlElementIf,
	SdlElementCond,
	SdlElementThen,
	SdlElementElse,
/* Conditionals (end) */
/* The sgml header to the document */
	SdlElementDocument,
	SdlElementSgml,
	SdlElementDocType,
/* Generated elements in the document */
	SdlElementLoids,
	SdlElementToss,
	SdlElementLoPhrases,
	SdlElementPhrase,
	SdlElementIndex,
	SdlElementEntry,
	SdlElementRelDocs,
	SdlElementRelFile,
	SdlElementNotes,
/* Generated elements (end) */
/* Table of styles and sematics. */
	SdlElementKeyStyle,
	SdlElementHeadStyle,
	SdlElementFormStyle,
	SdlElementFrmtStyle,
	SdlElementGrphStyle,
/* Table of styles and sematics.(end) */
	SdlElementId,
/* Support asyncronous blocks */
	SdlElementBlockAsync,
/* Support getting the title of a virpage */
	SdlElementTitle,
/* Support content that has optional markup followed by PCDATA */
	SdlPcDataFollows
    };

/********    Private Defines Declarations    ********/
/* sort of enumerated types */
#define	ENUM_VALUE	0x01
#define	NUMBER_VALUE	0x02
#define	STRING1_VALUE	0x03
#define	STRING2_VALUE	0x04

#define	VALUE_MASK	0x07

#define	VALUE_TYPE_SHIFT	3
#define	MINOR_MASK	(0x07 << VALUE_TYPE_SHIFT)

#define	SET_ENUM_ATTR(x)	((x) << VALUE_TYPE_SHIFT | ENUM_VALUE)
#define	SET_NUMBER_ATTR(x)	((x) << VALUE_TYPE_SHIFT | NUMBER_VALUE)
#define	SET_STRING_ATTR(x)	((x) << VALUE_TYPE_SHIFT | STRING1_VALUE)
#define	SET_STRING2_ATTR(x)	((x) << VALUE_TYPE_SHIFT | STRING2_VALUE)

#define	ATTRS_EQUAL(type, x, y)	\
		(((type) != SdlAttrDataTypeEnum) ? \
			((x) == (y) ? 1 : 0) : \
		(((((x) & VALUE_MASK) == ((y) & VALUE_MASK)) && \
		((x) & (y) & ~(VALUE_MASK))) ? 1 : 0))

#define	_CEAttrFlagType(x)	((x) & VALUE_MASK)
#define	_CEIsString1(x)		((((x) & VALUE_MASK) == STRING1_VALUE) ? 1 : 0)
#define	_CEIsString2(x)		((((x) & VALUE_MASK) == STRING2_VALUE) ? 1 : 0)
#define	_CEIsAttrAString(x)	(_CEIsString1(x) || _CEIsString2(x))

#define	_CEIsStringAttrSet(pa, attr) \
	(((((attr) & VALUE_MASK)==STRING1_VALUE) ? ((pa).str1_values & attr) : \
		((((attr) & VALUE_MASK)==STRING2_VALUE) ? \
			((pa).str2_values & attr) : 0)) & (~VALUE_MASK))

#define	_CEIsAttrSet(pa, attr) \
	(((((attr) & VALUE_MASK)==ENUM_VALUE) ? ((pa).enum_values & attr) : \
	 ((((attr) & VALUE_MASK)==NUMBER_VALUE) ? ((pa).num_values & attr) : \
	 ((((attr) & VALUE_MASK)==STRING1_VALUE) ? ((pa).str1_values & attr) : \
				((pa).str2_values & attr)))) & (~VALUE_MASK))

/************************************************************************
 * Enum types
 ************************************************************************/
/*
 * this set has the same attribute 'class',
 * but each has a different set of values.
 */
#define	SDL_ATTR_CLASS		SET_ENUM_ATTR(1 << 3)
#define	SDL_ATTR_CLASSALL	(SDL_ATTR_CLASS)
#define	SDL_ATTR_CLASSH		SET_ENUM_ATTR((1 << 3) | 0x01)
#define	SDL_ATTR_CLASSF		SET_ENUM_ATTR((1 << 3) | 0x02)
#define	SDL_ATTR_CLASSK		SET_ENUM_ATTR((1 << 3) | 0x03)
#define	SDL_ATTR_CLASSG		SET_ENUM_ATTR((1 << 3) | 0x04)
#define	SDL_ATTR_CLASSFG	SET_ENUM_ATTR((1 << 3) | 0x05)
#define	SDL_ATTR_CLASSP		SET_ENUM_ATTR((1 << 3) | 0x06)

/*
 * this set has the same attribute 'type',
 * but each has a different set of values.
 */
#define	SDL_ATTR_TYPE		SET_ENUM_ATTR((1 << 4) | 0x00)
#define	SDL_ATTR_TYPEFRMT	SET_ENUM_ATTR((1 << 4) | 0x01)
#define	SDL_ATTR_TYPEID		SET_ENUM_ATTR((1 << 4) | 0x02)

#define	SDL_ATTR_APP		SET_ENUM_ATTR(1 << 5)
#define	SDL_ATTR_TIMING		SET_ENUM_ATTR(1 << 6)
#define	SDL_ATTR_HDR		SET_ENUM_ATTR(1 << 7)

/* Format Style Attributes */
#define	SDL_ATTR_BORDER		SET_ENUM_ATTR(1 << 8)
#define	SDL_ATTR_JUSTIFY	SET_ENUM_ATTR((1 << 9) | 0x00)
#define	SDL_ATTR_JUSTIFY1	SET_ENUM_ATTR((1 << 9) | 0x01)
#define	SDL_ATTR_JUSTIFY2	SET_ENUM_ATTR((1 << 9) | 0x02)
#define	SDL_ATTR_VJUST		SET_ENUM_ATTR(1 << 10)

/* Head Style Attributes */
#define	SDL_ATTR_ORIENT		SET_ENUM_ATTR(1 << 11)
#define	SDL_ATTR_VORIENT	SET_ENUM_ATTR(1 << 12)
#define	SDL_ATTR_FLOW		SET_ENUM_ATTR(1 << 13)
#define	SDL_ATTR_WINDOW		SET_ENUM_ATTR(1 << 14)
#define	SDL_ATTR_PLACEMENT	SET_ENUM_ATTR(1 << 15)
#define	SDL_ATTR_STACK		SET_ENUM_ATTR(1 << 16)

/* Link Attribute */
#define	SDL_ATTR_TRAVERSAL	SET_ENUM_ATTR(1 << 17)

/* Font Style Attributes */
#define	SDL_ATTR_STYLE		SET_ENUM_ATTR(1 << 18)
#define	SDL_ATTR_SPACING	SET_ENUM_ATTR(1 << 19)
#define	SDL_ATTR_WEIGHT		SET_ENUM_ATTR(1 << 20)
#define	SDL_ATTR_SLANT		SET_ENUM_ATTR(1 << 21)
#define	SDL_ATTR_SPECIAL	SET_ENUM_ATTR(1 << 22)
#define	SDL_ATTR_INTERP 	SET_ENUM_ATTR(1 << 23)

/************************************************************************
 * number types
 ************************************************************************/
/*
 * LEVEL and RLEVEL share the same data element in the structure
 */
#define	SDL_ATTR_LEVEL		 SET_NUMBER_ATTR(1 << 3)
#define	SDL_ATTR_RLEVEL		 SET_NUMBER_ATTR(1 << 4)

#define	SDL_ATTR_LENGTH		 SET_NUMBER_ATTR(1 << 5)
#define	SDL_ATTR_NCOLS		 SET_NUMBER_ATTR(1 << 6)
#define	SDL_ATTR_COUNT		 SET_NUMBER_ATTR(1 << 7)

/* Font Style Attributes */
#define	SDL_ATTR_POINTSZ	 SET_NUMBER_ATTR(1 << 8)
#define	SDL_ATTR_SETWIDTH	 SET_NUMBER_ATTR(1 << 9)

/* Format Style Attributes */
#define	SDL_ATTR_FMARGIN	 SET_NUMBER_ATTR(1 << 10)
#define	SDL_ATTR_LMARGIN	 SET_NUMBER_ATTR(1 << 11)
#define	SDL_ATTR_RMARGIN	 SET_NUMBER_ATTR(1 << 12)
#define	SDL_ATTR_TMARGIN	 SET_NUMBER_ATTR(1 << 13)
#define	SDL_ATTR_BMARGIN	 SET_NUMBER_ATTR(1 << 14)
#define	SDL_ATTR_SRCHWT		 SET_NUMBER_ATTR(1 << 15)
#define	SDL_ATTR_OFFSET		 SET_NUMBER_ATTR(1 << 16)

/************************************************************************
 * string1 types
 ************************************************************************/
#define	SDL_ATTR_VERSION	SET_STRING_ATTR(1 << 0)
#define	SDL_ATTR_LANGUAGE	SET_STRING_ATTR(1 << 1)
#define	SDL_ATTR_DOCID		SET_STRING_ATTR(1 << 2)

/* style attr */
#define	SDL_ATTR_PHRASE		SET_STRING_ATTR(1 << 3)

/* Head Style Attributes */
#define	SDL_ATTR_HEADWDTH	SET_STRING_ATTR(1 << 4)

/* refitem */
#define	SDL_ATTR_BUTTON		SET_STRING_ATTR(1 << 5)

/* link */
#define	SDL_ATTR_DESCRIPT	SET_STRING_ATTR(1 << 6)
#define	SDL_ATTR_LINKINFO	SET_STRING_ATTR(1 << 7)

/* Font Style Attributes */
#define	SDL_ATTR_COLOR		SET_STRING_ATTR(1 <<  8)
#define	SDL_ATTR_XLFD		SET_STRING_ATTR(1 <<  9)
#define	SDL_ATTR_XLFDI		SET_STRING_ATTR(1 << 10)
#define	SDL_ATTR_XLFDB		SET_STRING_ATTR(1 << 11)
#define	SDL_ATTR_XLFDIB		SET_STRING_ATTR(1 << 12)
#define	SDL_ATTR_TYPENAM	SET_STRING_ATTR(1 << 13)
#define	SDL_ATTR_TYPENAMI	SET_STRING_ATTR(1 << 14)
#define	SDL_ATTR_TYPENAMB	SET_STRING_ATTR(1 << 15)
#define	SDL_ATTR_TYPENAMIB	SET_STRING_ATTR(1 << 16)

/*
 * start share of element 'id'.
 */
#define	SDL_ATTR_ID		SET_STRING_ATTR(1 << 17)
#define	SDL_ATTR_NAME  		SET_STRING_ATTR(1 << 18)
#define	SDL_ATTR_RID		SET_STRING_ATTR(1 << 19)
#define	SDL_ATTR_CELLS		SET_STRING_ATTR(1 << 20)
#define	SDL_ATTR_FRST_PG	SET_STRING_ATTR(1 << 21)

/*
 * end shar of element 'id'
 */

/*
 * SSI and RSSI share the same element 'ssi'.
 */
#define	SDL_ATTR_SSI		SET_STRING_ATTR(1 << 22)
#define	SDL_ATTR_RSSI		SET_STRING_ATTR(1 << 23)

/************************************************************************
 * string2 types
 ************************************************************************/
/*
 * Group A, B, C, and D all share the same data space
 *
 * Group A.
 */
#define	SDL_ATTR_CHARSET	SET_STRING2_ATTR(1 <<  0)
#define	SDL_ATTR_COLW		SET_STRING2_ATTR(1 <<  1)
#define	SDL_ATTR_COLJ		SET_STRING2_ATTR(1 <<  2)
#define	SDL_ATTR_TEXT		SET_STRING2_ATTR(1 <<  3)
#define	SDL_ATTR_BRANCHES	SET_STRING2_ATTR(1 <<  4)
#define	SDL_ATTR_ABBREV		SET_STRING2_ATTR(1 <<  5)
#define	SDL_ATTR_ENTER		SET_STRING2_ATTR(1 <<  6)
#define	SDL_ATTR_EXIT		SET_STRING2_ATTR(1 <<  7)

/*
 * Group B.
 */
#define	SDL_ATTR_PRODUCT	SET_STRING2_ATTR(1 <<  8)
#define	SDL_ATTR_LICENSE	SET_STRING2_ATTR(1 <<  9)
#define	SDL_ATTR_TIMESTAMP	SET_STRING2_ATTR(1 << 10)
#define	SDL_ATTR_PRODPN		SET_STRING2_ATTR(1 << 11)
#define	SDL_ATTR_PRODVER	SET_STRING2_ATTR(1 << 12)
#define	SDL_ATTR_AUTHOR 	SET_STRING2_ATTR(1 << 13)
#define	SDL_ATTR_SDLDTD 	SET_STRING2_ATTR(1 << 14)
#define	SDL_ATTR_SRCDTD 	SET_STRING2_ATTR(1 << 15)
#define	SDL_ATTR_PUBID		SET_STRING2_ATTR(1 << 16)

/*
 * Group C.
 * the group uses the SDL_ATTR_PRODUCT & SDL_ATTR_LICENSE
 */
#define	SDL_ATTR_FORMAT		SET_STRING2_ATTR(1 << 17)
#define	SDL_ATTR_METHOD		SET_STRING2_ATTR(1 << 18)
#define	SDL_ATTR_XID		SET_STRING2_ATTR(1 << 19)
#define	SDL_ATTR_COMMAND	SET_STRING2_ATTR(1 << 20)
#define	SDL_ATTR_DATA		SET_STRING2_ATTR(1 << 21)

/*
 * Group D.
 * Used for <entry>
 */
#define	SDL_ATTR_MAIN		SET_STRING2_ATTR(1 << 22)
#define	SDL_ATTR_LOCS		SET_STRING2_ATTR(1 << 23)
#define	SDL_ATTR_SYNS		SET_STRING2_ATTR(1 << 24)
#define	SDL_ATTR_SORT		SET_STRING2_ATTR(1 << 25)

/* Attribute Value defines */
enum	SdlAttrValue
  {
	SdlAttrValueBad,
	SdlAttrValueImplied,
	SdlAttrValueRequired,
	SdlAttrValueImpliedDef
  };

/* Segment types */
#define	SDL_ELEMENT	CE_ELEMENT
#define	SDL_STRING	CE_STRING
#define	SDL_GRAPHIC	CE_GRAPHIC

#define	SDL_ELEMENT_SHIFT	CE_SHIFT_PAST_CHAR_SIZE
#define	SDL_ELEMENT_MASK	(0xfffff  << SDL_ELEMENT_SHIFT)

/*
 * this is the maximum number of bits in an unsigned long value.
 * This is for the inclusion and exclustion masks.
 */
/* On Hewlett-Packard Machines! */
#define	SDL_MASK_SIZE	32
#define	SDL_MASK_LEN	 2

/********    Private Macro Declarations    ********/

#define	CESetTypeOfSegment(x,y)	(((x) & ~(CE_PRIMARY_MASK)) | (y))
#define	CEGetTypeOfSegment(x)	_CEGetPrimaryType(x)

#define	CESetTypeToElement(x)	CESetTypeOfSegment((x), SDL_ELEMENT)
#define	CESetTypeToString(x)	CESetTypeOfSegment((x), SDL_STRING)
#define	CESetTypeToGraphic(x)	CESetTypeOfSegment((x), SDL_GRAPHIC)
#define	CESetTypeToSpecial(x)	CESetTypeOfSegment((x), CE_SPECIAL)

#ifndef	CEOffset
#define	CEOffset(p_type, field) \
	((unsigned int)(((char *)(&(((p_type)NULL)->field)))-((char *)NULL)))
#endif

#define	CESearchSDLMask(mask, value) \
	((mask[(value)/SDL_MASK_SIZE] & \
		(((unsigned)1L) << ((value) % SDL_MASK_SIZE))) ? True : False)

#define	CEMaskValue1(value) \
	(((value) < SDL_MASK_SIZE ? ((unsigned)1L) : 0) \
						<< ((value) % SDL_MASK_SIZE))

#define	CEMaskValue2(value) \
	(((value) < SDL_MASK_SIZE ? 0: ((unsigned)1L)) \
						<< ((value) % SDL_MASK_SIZE))

#define	CEClearAllBits	{ 0, 0 }

#define	CESetAllBits	\
	{ ~(CEMaskValue1(SdlElementNone)), ~(CEMaskValue2(SdlElementNone)) }

#define	CECheckMask(mask) \
	(((mask)[0] != 0 || (mask)[1] != 0) ? 1 : 0)

#define	CEStripElementFromMask(mask, value) \
	(mask)[(value)/SDL_MASK_SIZE] = \
		((mask)[(value)/SDL_MASK_SIZE] & \
			(~(((unsigned)1L) << ((value) % SDL_MASK_SIZE))))

#define	CEInitializeMask(value)	\
	{ \
	  CEMaskValue1(value), \
	  CEMaskValue2(value), \
	}

#define	CEInitializeMaskTwo(value1, value2) \
	{ \
	  CEMaskValue1(value1) | CEMaskValue1(value2), \
	  CEMaskValue2(value1) | CEMaskValue2(value2), \
	}
#define	CEInitializeMaskThree(value1, value2, value3) \
	{ \
	  CEMaskValue1(value1) | CEMaskValue1(value2) | CEMaskValue1(value3), \
	  CEMaskValue2(value1) | CEMaskValue2(value2) | CEMaskValue2(value3), \
	}
#define	CEInitializeMaskFour(value1, value2, value3, value4) \
	{ \
	  CEMaskValue1(value1)   | CEMaskValue1(value2) | \
	    CEMaskValue1(value3) | CEMaskValue1(value4),  \
	  CEMaskValue2(value1)   | CEMaskValue2(value2) | \
	    CEMaskValue2(value3) | CEMaskValue2(value4),  \
	}
#define	CEInitializeMaskFive(value1, value2, value3, value4, value5) \
	{ \
	  CEMaskValue1(value1) | CEMaskValue1(value2) | CEMaskValue1(value3) | \
	    CEMaskValue1(value4) | CEMaskValue1(value5),\
	  CEMaskValue2(value1) | CEMaskValue2(value2) | CEMaskValue2(value3) | \
	    CEMaskValue2(value4) | CEMaskValue2(value5),\
	}
#define	CEInitializeMaskSix(value1, value2, value3, value4, value5, value6) \
	{ \
	  CEMaskValue1(value1)   | CEMaskValue1(value2) | \
	    CEMaskValue1(value3) | CEMaskValue1(value4) | \
	    CEMaskValue1(value5) | CEMaskValue1(value6),  \
	  CEMaskValue2(value1)   | CEMaskValue2(value2) | \
	    CEMaskValue2(value3) | CEMaskValue2(value4) | \
	    CEMaskValue2(value5) | CEMaskValue2(value6),  \
	}
#define	CEInitializeMaskSeven(value1, value2, value3,\
				value4, value5, value6, value7) \
	{ \
	  CEMaskValue1(value1)   | CEMaskValue1(value2) | \
	    CEMaskValue1(value3) | CEMaskValue1(value4) | \
	    CEMaskValue1(value5) | CEMaskValue1(value6) | \
	    CEMaskValue1(value7),  \
	  CEMaskValue2(value1)   | CEMaskValue2(value2) | \
	    CEMaskValue2(value3) | CEMaskValue2(value4) | \
	    CEMaskValue2(value5) | CEMaskValue2(value6) | \
	    CEMaskValue2(value7),  \
	}

#define	CEInitializeMaskEight(value1, value2, value3, value4, value5, \
				value6, value7, value8) \
	{ \
	  CEMaskValue1(value1)   | CEMaskValue1(value2) | \
	    CEMaskValue1(value3) | CEMaskValue1(value4) | \
	    CEMaskValue1(value5) | CEMaskValue1(value6) | \
	    CEMaskValue1(value7) | CEMaskValue1(value8),  \
	  CEMaskValue2(value1)   | CEMaskValue2(value2) | \
	    CEMaskValue2(value3) | CEMaskValue2(value4) | \
	    CEMaskValue2(value5) | CEMaskValue2(value6) | \
	    CEMaskValue2(value7) | CEMaskValue2(value8),  \
	}

#define	CEInitializeMaskNine(value1, value2, value3, value4, value5, \
				value6, value7, value8, value9) \
	{ \
	  CEMaskValue1(value1)   | CEMaskValue1(value2) | \
	    CEMaskValue1(value3) | CEMaskValue1(value4) | \
	    CEMaskValue1(value5) | CEMaskValue1(value6) | \
	    CEMaskValue1(value7) | CEMaskValue1(value8) | \
	    CEMaskValue1(value9),                         \
	  CEMaskValue2(value1)   | CEMaskValue2(value2) | \
	    CEMaskValue2(value3) | CEMaskValue2(value4) | \
	    CEMaskValue2(value5) | CEMaskValue2(value6) | \
	    CEMaskValue2(value7) | CEMaskValue2(value8) | \
	    CEMaskValue2(value9),                         \
	}

#define	CEInitializeMaskEleven(value1, value2, value3, value4, value5, \
				value6, value7, value8, value9, value10, \
				value11) \
	{ \
	  CEMaskValue1(value1)    | CEMaskValue1(value2)  | \
	    CEMaskValue1(value3)  | CEMaskValue1(value4)  | \
	    CEMaskValue1(value5)  | CEMaskValue1(value6)  | \
	    CEMaskValue1(value7)  | CEMaskValue1(value8)  | \
	    CEMaskValue1(value9)  | CEMaskValue1(value10) | \
	    CEMaskValue1(value11),                          \
	  CEMaskValue2(value1)    | CEMaskValue2(value2)  | \
	    CEMaskValue2(value3)  | CEMaskValue2(value4)  | \
	    CEMaskValue2(value5)  | CEMaskValue2(value6)  | \
	    CEMaskValue2(value7)  | CEMaskValue2(value8)  | \
	    CEMaskValue2(value9)  | CEMaskValue2(value10) | \
	    CEMaskValue2(value11),                          \
	}

#define	CEInitializeMaskTwelve(value1, value2, value3, value4, value5, \
				value6, value7, value8, value9, value10, \
				value11, value12) \
	{ \
	  CEMaskValue1(value1)    | CEMaskValue1(value2)  | \
	    CEMaskValue1(value3)  | CEMaskValue1(value4)  | \
	    CEMaskValue1(value5)  | CEMaskValue1(value6)  | \
	    CEMaskValue1(value7)  | CEMaskValue1(value8)  | \
	    CEMaskValue1(value9)  | CEMaskValue1(value10) | \
	    CEMaskValue1(value11) | CEMaskValue1(value12),  \
	  CEMaskValue2(value1)    | CEMaskValue2(value2)  | \
	    CEMaskValue2(value3)  | CEMaskValue2(value4)  | \
	    CEMaskValue2(value5)  | CEMaskValue2(value6)  | \
	    CEMaskValue2(value7)  | CEMaskValue2(value8)  | \
	    CEMaskValue2(value9)  | CEMaskValue2(value10) | \
	    CEMaskValue2(value11) | CEMaskValue2(value12),  \
	}

#define	CEInitializeMaskThirteen(value1, value2, value3, value4, value5, \
				value6 , value7 , value8 , value9, value10, \
				value11, value12, value13) \
	{ \
	  CEMaskValue1(value1)    | CEMaskValue1(value2)  | \
	    CEMaskValue1(value3)  | CEMaskValue1(value4)  | \
	    CEMaskValue1(value5)  | CEMaskValue1(value6)  | \
	    CEMaskValue1(value7)  | CEMaskValue1(value8)  | \
	    CEMaskValue1(value9)  | CEMaskValue1(value10) | \
	    CEMaskValue1(value11) | CEMaskValue1(value12) | \
	    CEMaskValue1(value13),                          \
	  CEMaskValue2(value1)    | CEMaskValue2(value2)  | \
	    CEMaskValue2(value3)  | CEMaskValue2(value4)  | \
	    CEMaskValue2(value5)  | CEMaskValue2(value6)  | \
	    CEMaskValue2(value7)  | CEMaskValue2(value8)  | \
	    CEMaskValue2(value9)  | CEMaskValue2(value10) | \
	    CEMaskValue2(value11) | CEMaskValue2(value12) | \
	    CEMaskValue2(value13),                          \
	}

#define	CEInitializeMaskFourteen(value1, value2, value3, value4, value5, \
				value6 , value7 , value8 , value9, value10, \
				value11, value12, value13, value14) \
	{ \
	  CEMaskValue1(value1)    | CEMaskValue1(value2)  | \
	    CEMaskValue1(value3)  | CEMaskValue1(value4)  | \
	    CEMaskValue1(value5)  | CEMaskValue1(value6)  | \
	    CEMaskValue1(value7)  | CEMaskValue1(value8)  | \
	    CEMaskValue1(value9)  | CEMaskValue1(value10) | \
	    CEMaskValue1(value11) | CEMaskValue1(value12) | \
	    CEMaskValue1(value13) | CEMaskValue1(value14),  \
	  CEMaskValue2(value1)    | CEMaskValue2(value2)  | \
	    CEMaskValue2(value3)  | CEMaskValue2(value4)  | \
	    CEMaskValue2(value5)  | CEMaskValue2(value6)  | \
	    CEMaskValue2(value7)  | CEMaskValue2(value8)  | \
	    CEMaskValue2(value9)  | CEMaskValue2(value10) | \
	    CEMaskValue2(value11) | CEMaskValue2(value12) | \
	    CEMaskValue2(value13) | CEMaskValue1(value14),  \
	}

/********    Private Typedef Declarations    ********/
typedef	char*	SDLId;
typedef	char*	SDLCdata;

typedef	int	SDLNumber;
typedef	unsigned long	SDLMask;	/* must be a 32 bit value */

typedef	CanvasFontSpec*	CanvasFontSpecPtr;

/********    Private Structures Declarations    ********/

typedef struct  {
	const char	*string;
	enum SdlOption	 option_value;
} OptionList;

typedef struct  {
	unsigned long		attr_value;
	const OptionList	*options;
} SDLAttributeOptions;

typedef	struct {
	const char	*string;
	enum CEFontSpecValue	 option_value;
} FontSpecOption;

typedef	struct {
	unsigned long		 attr_value;
	const FontSpecOption	*options;
} SDLFontSpecList;

typedef	struct {
	SDLMask	mask[SDL_MASK_LEN];
	int	(*model)();
} SDLContent;

typedef	struct	{
	enum SdlOption	border;
	enum SdlOption	justify;
	enum SdlOption	vjust;
	SDLNumber	fmargin;
	SDLNumber	lmargin;
	SDLNumber	rmargin;
	SDLNumber	tmargin;
	SDLNumber	bmargin;
} CEFrmtSpecs;

typedef	union sdl_id_val {
	SDLId		id;
	SDLId		name;
	SDLId		rid;
	SDLId		cells;
	SDLId		frst_pg;
} SDLIdVals;

typedef	struct	{
	SDLCdata	colw;
	SDLCdata	colj;
	SDLCdata	text;
	SDLCdata	branches;
	SDLCdata	abbrev;
	SDLCdata	enter;
	SDLCdata	exit;
	SDLCdata	junk1;
	SDLCdata	junk2;
} el_attrs;

typedef	struct	{
	SDLCdata	main;
	SDLCdata	locs;
	SDLCdata	syns;
	SDLCdata	sort;
} entry_attrs;

typedef	struct	{
	SDLCdata	product;    /* these 2 elements must match the same*/
	SDLCdata	license;    /* position as in sdlnotation_attrs */
	SDLCdata	timestamp;
	SDLCdata	prodpn;
	SDLCdata	prodver;
	SDLCdata	author;
	SDLCdata	sdldtd;
	SDLCdata	srcdtd;
	SDLCdata	pub_id;
} sdldoc_attrs;

typedef	struct	{
	SDLCdata	product;    /* these 2 elements must match the same */
	SDLCdata	license;    /* position as in sdldoc_attrs          */
	SDLCdata	format;
	SDLCdata	method;
	SDLCdata	xid;
	SDLCdata	cmd;
	SDLCdata	data;
	SDLCdata	junk1;
	SDLCdata	junk2;
} sdlnotation_attrs;

typedef	union merged_string {
	el_attrs	el_strs;
	entry_attrs	entry_strs;
	sdlnotation_attrs  not_strs;
	sdldoc_attrs	doc_strs;
} SDLAttrStrings;

typedef	struct	_frmt_info {
	Unit	y_units;
	Unit	x_units;
	int	beg_txt;
	int	end_txt;
	int	beg_gr;
	int	end_gr;
} SDLFrmtInfo;

typedef	union sdl_num_val {
	SDLNumber	length;
	SDLNumber	ncols;
	SDLNumber	offset;
	SDLNumber	count;
} SDLNumVals;

typedef	union sdl_lev {
	SDLNumber	level;
	SDLNumber	rlevel;
} SDLLevel;

typedef	union sdl_ssi_val {
	SDLCdata	ssi;
	SDLCdata	rssi;
} SDLssiVals;

typedef	struct	{
	unsigned long		enum_values;
	unsigned long		num_values;
	unsigned long		str1_values;
	unsigned long		str2_values;
	enum SdlOption	clan;			/* the class of the element
						   but class is C++ reserved */
	enum SdlOption	hdr;
	enum SdlOption	app;
	enum SdlOption	timing;
	enum SdlOption	type;
	enum SdlOption	orient;
	enum SdlOption	vorient;
	enum SdlOption	placement;
	enum SdlOption	stacking;
	enum SdlOption	flow;
	enum SdlOption	window;
	enum SdlOption	traversal;
	enum SdlOption	interp;
	SDLLevel	lev;
	SDLNumber	srch_wt;
	SDLNumVals	num_vals;
	SDLIdVals	id_vals;
	SDLssiVals	ssi_vals;
	SDLCdata	version;
	SDLCdata	language;
	SDLCdata	charset;
	SDLCdata	doc_id;
	SDLAttrStrings	strings;
	SDLCdata	phrase;
	SDLCdata	headw;
	SDLCdata	button;
	SDLCdata	descript;
	SDLCdata	linkinfo;
	CanvasFontSpec	font_specs;
	CEFrmtSpecs     frmt_specs;
	SDLFrmtInfo	frmt_info;
} CESDLAttrStruct, *CESDLAttrPtr;

typedef	struct {
	const char		*name;
	enum SdlAttrDataType	 data_type;
	int			 sdl_attr_define;
	int			 field_ptr;
} SDLAttribute;

typedef	struct {
	int			 sdl_attr_define;
	enum SdlAttrValue	 sdl_value;
	const char		*def_string;
} SDLElementAttrList;

typedef	struct {
	const char		*element_str;     /* the element string     */
	const char		*element_end_str; /* the ending string      */
	int			 str_sig_chars;   /* # of significant chars */
	int			 end_sig_chars;   /* # of significant chars */
	enum	SdlElement	 sdl_element;     /* The element's value    */
	SDLMask			 exceptions[SDL_MASK_LEN];
						  /* what elements cannot
						     appear in this element */
	const SDLElementAttrList *attrib_list;    /* attribute list         */
	const SDLContent	*content;         /* The element's content  */
	CEBoolean		 elim_flag;	  /* If this element's
						     segment list is null,
						     eleminate the element  */
	CEBoolean		 format_break;    /* does this cause a
						     formatting break       */
	CEBoolean		 cdata_flag;	  /* If no attributes, is
						     there cdata before the
						     end                    */
} SDLElement;

/********    Private Macros Declarations    ********/

#ifndef	_DtHelpCeSdlIdString
#define	_DtHelpCeSdlIdString(x)		((x)->attributes.id_vals.id)
#endif

#ifndef	_DtHelpCeSdlRidString
#define	_DtHelpCeSdlRidString(x)	((x)->attributes.id_vals.rid)
#endif

#ifndef	_DtHelpCeSdlSsiString
#define	_DtHelpCeSdlSsiString(x)	((x)->attributes.ssi_vals.ssi)
#endif

#ifndef	_DtHelpCeSdlRssiString
#define	_DtHelpCeSdlRssiString(x)	((x)->attributes.ssi_vals.rssi)
#endif

#ifndef	_DtHelpCeSdlLevel
#define	_DtHelpCeSdlLevel(x)		((x)->attributes.lev.level)
#endif

#ifndef	_DtHelpCeSdlRlevel
#define	_DtHelpCeSdlRlevel(x)		((x)->attributes.lev.rlevel)
#endif

#ifndef	_DtHelpCeSdlClass
#define	_DtHelpCeSdlClass(x)		((x)->attributes.clan)
#endif

#ifndef	_DtHelpCeSdlLangAttr
#define	_DtHelpCeSdlLangAttr(x)		((x)->attributes.language)
#endif

#ifndef	_DtHelpCeSdlCharSetAttr
#define	_DtHelpCeSdlCharSetAttr(x)	((x)->attributes.charset)
#endif

#ifndef	_DtHelpCeSdlDocIdAttr
#define	_DtHelpCeSdlDocIdAttr(x)	((x)->attributes.doc_id)
#endif

#ifndef	_DtHelpCeSdlTimeStampAttr
#define	_DtHelpCeSdlTimeStampAttr(x) \
				((x)->attributes.strings.doc_strs.timestamp)
#endif

#ifndef	_DtHelpCeSdlFirstPgAttr
#define	_DtHelpCeSdlFirstPgAttr(x)	((x)->attributes.id_vals.frst_pg)
#endif

/********    Private Function Declarations    ********/

#ifdef _NO_PROTO
#else
#endif /* _NO_PROTO */

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _DtHelpSDLI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
