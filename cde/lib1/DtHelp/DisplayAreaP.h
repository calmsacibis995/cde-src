/* $XConsortium: DisplayAreaP.h /main/cde1_maint/2 1995/10/08 17:17:29 pascale $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        DisplayAreaP.h
 **
 **   Project:     Cde Help System
 **
 **   Description: Defines the Display Area structures and defines.
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef	_DtHelpDisplayAreaP_h
#define	_DtHelpDisplayAreaP_h

#include <X11/X.h>
#include <Xm/XmP.h>
#include "Canvas.h"
#ifndef	STUB
#include "ilX.h"
#endif

/*
 * Traversal flags
 */
#define	_DT_HELP_SHADOW_TRAVERSAL	(1 << 0)
#define	_DT_HELP_TRAVERSAL_DRAWN	(1 << 1)
#define	_DT_HELP_TRAVERSAL_MOVED	(1 << 2)
#define	_DT_HELP_DRAW_TOC_IND		(1 << 3)
#define	_DT_HELP_NOT_INITIALIZED	(1 << 4)

/*
 * enum states for selection
 */
enum	_DtHelpSelectState
    {
	_DtHelpNothingDoing,
	_DtHelpCopyOrLink,
	_DtHelpSelectingText
    };

/*
 * Whether the display area has the focus or not
 */
#define _DT_HELP_FOCUS_FLAG           0x04

typedef struct {
	int		 used;
	int              num_pixels;
	Pixmap           pix;
	Pixmap           mask;
	Dimension        width;
	Dimension        height;
	Pixel           *pixels;
} DtHelpGraphicStruct;

typedef	struct {
	FontPtr	 font_ptr;
	int	 spc_idx;
} DtHelpSpecialChars;

typedef	struct {
	short   inited;
	CanvasFontMetrics fm;
} DtHelpDAFSMetrics;

typedef	struct	_DtHelpDAfontInfo {
	char		**exact_fonts;	/* the list of fonts specified by the
					   toss element rather than hints.   */
	XrmDatabase	def_font_db;	/* The default font resource db      */
	XrmDatabase	font_idx_db;	/* Which font index goes with which
					   set of font resources	     */
	XFontStruct	**font_structs;	/* The font structures opened        */
	XFontSet	*font_sets;	/* The font sets opened              */
	DtHelpDAFSMetrics *fs_metrics;

	XrmQuark	lang_charset;	/* the char set for current lang     */
	int		*exact_idx;	/* The indexes for the exact fonts   */
	int		max_structs;	/* the max number of font_structs    */
	int		max_sets;	/* The max number of font_sets	     */
	int		struct_cnt;	/* the cur number of font_structs    */
	int		set_cnt;	/* The cur number of font_sets	     */
	int		def_idx;	/* The default index                 */
} DtHelpDAFontInfo;

/*
 * DisplayArea structure
 */
typedef	struct _dtHelpDispAreaStruct {
	Widget	dispWid;		/* The text and graphic area. */
	Widget	vertScrollWid;		/* The vertical scroll bar    */
	Widget	horzScrollWid;		/* The horizontal scroll bar  */
	Boolean vertIsMapped;
	Boolean horzIsMapped;
	short	neededFlags;		/* _DtHelpAS_NEEDED flags        */

	Dimension	formWidth;	/* Pixel width of the parent  area  */
	Dimension	formHeight;	/* Pixel height of the parent area  */
	Dimension	dispWidth;	/* Pixel width of the display area  */
	Dimension	dispHeight;	/* Pixel height of the display area */
	Dimension	dispUseHeight;	/* Pixel height of the display area
					   minus the decor margin.          */
	Dimension	dispUseWidth;	/* Pixel width of the display area
					   minus the decor margin.          */
	Dimension	marginWidth;	/* Pixel padding at the left and
					   right of the display area.       */
	Dimension	marginHeight;	/* Pixel padding at the top and
					   bottom of the display area.      */

	short	decorThickness;		/* the shadow thickness plus highlight
					   thickness of the display area    */

	void	(*hyperCall)();		/* The hypertext callback */
	void	(*resizeCall)();	/* The resize callback    */
	caddr_t	clientData;		/* The client's data for the callback */

	Pixel	traversalColor;		/* The client's traversal color */
	Pixel	foregroundColor;	/* The client's foreground color     */
	Pixel	backgroundColor;	/* The client's foreground color     */
	GC	pixmapGC;
	GC	normalGC;
	GC	invertGC;
	Pixmap  def_pix;		/* the default 'missing pixmap'      */
	Dimension def_pix_width;	/* the width of the default pixmap   */
	Dimension def_pix_height;	/* the height of the default pixmap  */
	ilXWC	tiff_xwc;		/* the tiff XWC			     */

	Colormap  colormap;		/* The colormap to use		*/
	Visual	 *visual;		/* The visual to use		*/

	DtHelpDAFontInfo font_info;	/* The font information		*/

	int	depth;			/* The depth of the window	*/

	int	fontAscent;
	int	lineHeight;
	int	leading;
	long	charWidth;		/* The average size of a character   */
	int	moveThreshold;		/* The number of pixels that must
					   be moved before a copy-paste
					   action occurs.                    */
	int	underLine;
	int	lineThickness;		/* For traversal box and underline   */
	int	firstVisible;		/* The absolute number of the first
					   line visible in the window.      */
	int	visibleCount;		/* The number of lines viewable     */
	int	maxYpos;		/* Maximum Y positioning	    */

	int	virtualX;		/* The virtual x of the window      */
	int	maxX;			/* The max virtual x of a line      */

	int	max_spc;		/* The maximum special characters   */
	int	cur_spc;		/* The current unused structure     */
	int	timerX;			/* Used for button clicks/selections */
	int	timerY;			/* Used for button clicks/selections */
	int	scr_timer_x;		/* Used for button clicks/selections */
	int	scr_timer_y;		/* Used for button clicks/selections */
	int	toc_width;		/* */
	int	toc_height;		/* */
	int	toc_x;			/* */
	int	toc_y;			/* */

	Time	 anchor_time;		/* Indicates the primary selection
					   time.                              */
	Boolean	 primary;		/* Indicates if this widget has the
					   primary selection                  */
	Boolean	 text_selected;		/* Indicates if the selection has
					   occurred     		      */
	enum _DtHelpSelectState select_state;
					/* Indicates the state of the current
					   selection.                         */
	short	 traversal;		/* Indicates if the traversal indicator
					   is always on.		      */
	SpecialPtr  toc_indicator;	/* The indicator used in the toc */

	XtIntervalId scr_timer_id;
	DtHelpSpecialChars *spc_chars;	/* Structure containing the spc chars */
	CanvasHandle	  canvas;

} DtHelpDispAreaStruct;

typedef	struct {
	int	 reason;
	XEvent	*event;
	Window	 window;
	char	*specification;
	int	 hyper_type;
	int	 window_hint;
} DtHelpHyperTextStruct;

#endif /* _DtHelpDisplayAreaP_h */
