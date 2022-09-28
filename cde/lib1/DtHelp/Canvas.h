/* $XConsortium: Canvas.h /main/cde1_maint/1 1995/07/17 17:23:27 drk $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:         Canvas.h
 **
 **  Project:      Cde Help System
 **
 **  Description:  Public Header file for the UI independent core engine
 **                of the help system.
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 *****************************************************************************
 *************************************<+>*************************************/
#ifndef _DtHelpCanvas_h
#define _DtHelpCanvas_h


#ifdef __cplusplus
extern "C" {
#endif

/********    Public Defines Declarations    ********/
#ifndef	NULL
#define	NULL	0
#endif
#ifndef	False
#define	False	0
#endif
#ifndef	True
#define	True	1
#endif
#ifndef	FALSE
#define	FALSE	0
#endif
#ifndef	TRUE
#define	TRUE	1
#endif

/**************************
 * VStatus Defines
 **************************/
#define	VStatusSuccess	 0
#define	VStatusBad	-1

/**************************
 * Link Types
 **************************/

#define	CELinkType_Execute	3
#define	CELinkType_ManPage	4
#define	CELinkType_AppDefine	5
#define	CELinkType_SameVolume	6
#define	CELinkType_CrossLink	7
#define	CELinkType_TextFile	8

/**************************
 * Window Hint Kinds
 **************************/

#define	CEWindowHint_PopupWindow	1
#define	CEWindowHint_CurrentWindow	2
#define	CEWindowHint_NewWindow  	3

/**************************
 * Rendering flags
 **************************/

#define	CELinkFlag		(0x01 <<  0)
#define	CELinkBegin		(0x01 <<  1)
#define	CELinkEnd		(0x01 <<  2)

#define	CELinkPopUp		(0x01 <<  3)
#define	CELinkNewWindow		(0x01 <<  4)

#define	CETraversalFlag		(0x01 <<  5)
#define	CETraversalBegin	(0x01 <<  6)
#define	CETraversalEnd		(0x01 <<  7)

#define	CETocTopicFlag		(0x01 <<  8)
#define	CETocTopicBegin		(0x01 <<  9)
#define	CETocTopicEnd		(0x01 << 10)

#define	CESelectedFlag		(0x01 << 11)
#define	CEEndOfLine		(0x01 << 12)

/******************************
 * Set Traversal Declarations
 ******************************/
#define	CETraversalTop		0
#define	CETraversalNext		1
#define	CETraversalPrev		2
#define	CETraversalBottom	3

#define	CETraversalOn		4
#define	CETraversalOff		5

/******************************
 * Selection Declarations
 ******************************/
#define	CESelectionStart	0
#define	CESelectionEnd		1
#define	CESelectionUpdate	2
#define	CESelectionClear	3

/**********************************
 * Get Selection Type Declarations
 **********************************/
#define	CESelectedText			0
#define	CESelectedGraphics		1
#define	CESelectedTextAndGraphics	2

/**********************************
 * TOC flags
 **********************************/
#define	CEDimensions			(0x01 << 0)
#define	CETocPoints			(0x01 << 1)

/*********************************
 * Title Chunk types
 *********************************/
#define DT_HELP_CE_END		0
#define DT_HELP_CE_STRING	(1 << 0)
#define DT_HELP_CE_CHARSET	(1 << 1)
#define DT_HELP_CE_FONT_PTR	(1 << 2)
#define DT_HELP_CE_SPC		(1 << 3)
#define DT_HELP_CE_NEWLINE	(1 << 4)

/********    Public Typedef Declarations    ********/
typedef	void*	CanvasHandle;
typedef	void*	ClientData;
typedef	void*	FontPtr;
typedef	void*	SpecialPtr;
typedef	void*	GraphicPtr;
typedef	void*	TopicHandle;
typedef	void*	SelectionInfo;
typedef	void*	VolumeHandle;

typedef	unsigned long	Flags;

typedef	int	VStatus;
typedef	int	Unit;

typedef	char	CEBoolean;

/********    Public Enum Declarations    ********/
enum	CEFontSpecValue
    {
	      CEFontSpecBad,
/* styles  */ CEFontStyleSerif      , CEFontStyleSanSerif   , CEFontStyleSymbol,
/* spacing */ CEFontSpacingMono     , CEFontSpacingProp     ,
/* weights */ CEFontWeightMedium    , CEFontWeightBold      ,
/* slant   */ CEFontSlantRevItalic  , CEFontSlantItalic     , CEFontSlantRoman,
/* special */ CEFontSpecialUnderLine, CEFontSpecialStrikeOut, CEFontSpecialNone 
    };

/********    Public Structures Declarations    ********/
typedef	struct {
	void		(*_CEGetCanvasMetrics)();
	Unit		(*_CEGetStringWidth)();
	void		(*_CEDrawString)();
	void		(*_CEDrawLine)();
	void		(*_CEGetFont)();
	void		(*_CEGetFontMetrics)();
	VStatus		(*_CELoadGraphic)();
	void		(*_CEDrawGraphic)();
	void		(*_CEDestroyGraphic)();
	VStatus		(*_CEResolveSpecial)();
	void		(*_CEDrawSpecial)();
	void		(*_CEDestroySpecial)();
	VStatus		(*_CEBuildSelectionString)();
	VStatus		(*_CEBuildSelectionGraphic)();
	VStatus		(*_CEBuildSelectionSpc)();
	VStatus		(*_CEFilterExecCmd)();
} VirtualInfo;

typedef	struct	{
	char	*specification;
	char	*description;
	int	 hyper_type;
	int	 win_hint;
} CEHyperTextCallbackStruct;

typedef	struct	{
	Unit	width;			/* max width of canvas area  */
	Unit	height;			/* max height of canvas area */
	Unit	top_margin;		/* top margin to allow for   */
	Unit	side_margin;		/* left margin to allow for  */
	Unit	leading;		/* leading between lines     */
	Unit	line_thickness;		/* line thickness for tables */
} CanvasMetrics;

typedef	struct	{
	Unit	ascent;		/* Maximum ascent               */
	Unit	descent;	/* Maximum descent              */
	Unit	average_width;	/* Average width of a character */
	Unit	super;		/* Offset from baseline for super scripts */
	Unit	sub;		/* Offset from baseline for sub scripts   */
} CanvasFontMetrics;

struct	_canvasIndMetrics {
	Unit	space_before;
	Unit	space_after;
	Unit	space_above;
	Unit	space_below;
};

typedef	struct _canvasIndMetrics	CanvasLinkMetrics;
typedef	struct _canvasIndMetrics	CanvasTraversalMetrics;
typedef	struct _canvasIndMetrics	CanvasTocTopicMetrics;
typedef	struct _canvasIndMetrics	CanvasTocMarkerMetrics;

typedef	struct	{
	int	 pointsz;	/* height of font in points */
	int	 set_width;	/* width  of font in points */
	char	*color;		/* 1 of 42 UDT colors or 'RGB:rrrr/gggg/bbbb */
	char	*xlfd;		/* xlfd name for use on X Window System */
	char	*xlfdb;		/* xlfd name for use on X Window System */
	char	*xlfdi;		/* xlfd name for use on X Window System */
	char	*xlfdib;	/* xlfd name for use on X Window System */
	char	*typenam;	/* typeface spec for MS-Windows interface */
	char	*typenamb;	/* typeface spec for MS-Windows interface */
	char	*typenami;	/* typeface spec for MS-Windows interface */
	char	*typenamib;	/* typeface spec for MS-Windows interface */
	enum CEFontSpecValue  style;	/* generic typeface spec */
	enum CEFontSpecValue  spacing;	/* generic typeface spec */
	enum CEFontSpecValue  weight;	/* generic typeface spec */
	enum CEFontSpecValue  slant;	/* generic typeface spec */
	enum CEFontSpecValue  special;	/* generic typeface spec */
	void	*expand;	/* reserved pointer for later expansion */
} CanvasFontSpec;

/********    Public Macro Declarations    ********/
#define	CEIsHyperLink(x)	((x) & CELinkFlag)
#define	CEIsHyperBegin(x)	((x) & CELinkBegin)
#define	CEIsHyperEnd(x)		((x) & CELinkEnd)
#define	CEIsPopUp(x)		((x) & CELinkPopUp)
#define	CEIsNewWindow(x)	((x) & CELinkNewWindow)

#define	CEHasTraversal(x)	((x) & CETraversalFlag)
#define	CEIsTraversalBegin(x)	((x) & CETraversalBegin)
#define	CEIsTraversalEnd(x)	((x) & CETraversalEnd)

#define	CEIsTocTopicOn(x)	((x) & CETocTopicFlag)
#define	CEIsTocTopicBegin(x)	((x) & CETocTopicBegin)
#define	CEIsTocTopicEnd(x)	((x) & CETocTopicEnd)

#define	CEIsSelected(x)		((x) & CESelectedFlag)
#define	CEIsEndOfLine(x)	((x) & CEEndOfLine)

/********    Public Function Declarations    ********/
#ifdef _NO_PROTO
extern	void		_DtHelpCeCleanCanvas();
extern	void		_DtHelpCeDestroyCanvas();
extern	CEBoolean	_DtHelpCeGetTraversalSelection();
extern	CEBoolean	_DtHelpCeResizeCanvas();
extern	void		_DtHelpCeScrollCanvas();
extern	int		_DtHelpCeSetTocPath();
extern	void		_DtHelpCeSetTopic();
extern	CanvasHandle	_DtHelpCeCreateCanvas();
extern	CanvasHandle	_DtHelpCeCreateDefCanvas ();
extern	VStatus		_DtHelpCeGetSelectionData();
extern	CEBoolean	_DtHelpCeProcessLink();
extern	CEBoolean	_DtHelpCeMoveTraversal();
extern	void		_DtHelpCeProcessSelection();
extern	void		_DtHelpCeTurnTocMarkOff();
extern	int		_DtHelpCeTurnTocMarkOn();
#else
extern	CanvasHandle	_DtHelpCeCreateCanvas (
				VirtualInfo	 virt_info,
				ClientData	 client_data);
extern	CanvasHandle	_DtHelpCeCreateDefCanvas (void);
extern	VStatus		_DtHelpCeGetSelectionData (
				CanvasHandle	 canvas_handle,
				int		 type,
				SelectionInfo	*ret_select);
extern	CEBoolean	_DtHelpCeProcessLink(
				CanvasHandle	 canvas_handle,
				Unit		 x1,
				Unit		 y1,
				Unit		 x2,
				Unit		 y2,
				CEHyperTextCallbackStruct *ret_info);
extern	CEBoolean	_DtHelpCeMoveTraversal(
				CanvasHandle	 canvas_handle,
				int		 direction,
				CEBoolean	 wrap,
				CEBoolean	 render,
				char		*rid,
				Unit		*ret_x,
				Unit		*ret_y,
				Unit		*ret_height);
extern	void		_DtHelpCeProcessSelection (
				CanvasHandle	 canvas_handle,
				Unit		 x,
				Unit		 y,
				CEBoolean	 mode);
extern	void		_DtHelpCeTurnTocMarkOff(
				CanvasHandle	 canvas_handle,
				CEBoolean	 render);
extern	int		_DtHelpCeTurnTocMarkOn(
				CanvasHandle	 canvas_handle,
				CEBoolean	 render,
				Unit		 *ret_toc_y,
				Unit		 *ret_toc_height);
extern	void		_DtHelpCeCleanCanvas(
				CanvasHandle	 canvas_handle);
extern	void		_DtHelpCeDestroyCanvas(
				CanvasHandle	 canvas_handle);
extern	CEBoolean	_DtHelpCeGetTraversalSelection(
				CanvasHandle	 canvas_handle,
				CEHyperTextCallbackStruct *ret_info);
extern	CEBoolean	_DtHelpCeResizeCanvas(
				CanvasHandle	 canvas_handle,
				Unit		*ret_width,
				Unit		*ret_height);
extern	void		_DtHelpCeScrollCanvas(
				CanvasHandle	 canvas_handle,
				Unit		 x1,
				Unit		 y1,
				Unit		 x2,
				Unit		 y2);
extern	int		_DtHelpCeSetTocPath(
				CanvasHandle	 canvas_handle,
				VolumeHandle	 volume_handle,
				char		*id,
				CEBoolean	 render,
				Unit		*ret_width,
				Unit		*ret_height,
				Unit		*ret_toc_y,
				Unit		*ret_toc_height,
				Unit		*ret_valid);
extern	void		_DtHelpCeSetTopic(
				CanvasHandle	 canvas_handle,
				TopicHandle	 topic_handle,
				int		 scroll_percent,
				Unit		*ret_width,
				Unit		*ret_height,
				Unit		*ret_y);
#endif /* _NO_PROTO */

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif


#endif /* _DtHelpCanvas_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
