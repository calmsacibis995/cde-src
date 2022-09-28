/* $XConsortium: ContainerP.h /main/cde1_maint/1 1995/07/17 20:24:27 drk $ */
/*
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * Motif top of 2.0 tree as of Tue Jun  1 05:18:29 EDT 1993
 */
#ifndef	_XmContainerP_h
#define _XmContainerP_h
 
#include <Xm/XmP.h>
#include <Xm/ManagerP.h>
#include <Xm/DragCP.h>
#include "Container.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * One _XmCwidNodeRec structure is allocated by Container for each of 
 * it's children (except for OutlineButtons).  Information about the 
 * relationship of the child to other Container children (parentage and
 * order) is maintained here by Container.
 *
 * _XmCwidNodeRec structures are XtCalloc'd by Container in the 
 * ConstraintInitialize method and XtFree'd in the ConstraintDestroy method.
 * They are linked/unlinked to other _XmCwidNodeRec structures in the
 * ChangeManaged method.
 */
typedef struct _XmCwidNodeRec
        {
	struct	_XmCwidNodeRec *	next_ptr;
	struct	_XmCwidNodeRec *	prev_ptr;
	struct	_XmCwidNodeRec *	child_ptr;
	struct	_XmCwidNodeRec *	parent_ptr;
	Widget			widget_ptr;
	}	XmCwidNodeRec, *CwidNode;

/*
 * Container allocates a _XmContainerXfrActionRec structure to store
 * the data from a ContainerStartTransfer action until it can determine
 * whether the action should start a primary transfer or begin a drag.
 */
typedef	struct	_XmContainerXfrActionRec
	{
	Widget		wid;
	XEvent		*event;
	String		*params;
	Cardinal	*num_params;
	Atom		operation;
	}	XmContainerXfrActionRec, *ContainerXfrAction;
	
/* Container constraint class part record */
typedef	struct	_XmContainerConstraintPart
	{
	_XmString *	entry_detail;		/* XmNentryDetail */
	Widget		entry_parent;		/* XmNentryParent */
	unsigned char	outline_state;		/* XmNoutlineState */
	Position	large_icon_x;		/* XmNlargeIconX */
	Position	large_icon_y;		/* XmNlargeIconY */
	int		position_index;		/* XmNpositionIndex */
	Position	small_icon_x;		/* XmNsmallIconX */
	Position	small_icon_y;		/* XmNsmallIconY */
	/* Private variables */
	CwidNode 	node_ptr;
	int		depth;
	unsigned char	selection_visual;
	unsigned char	selection_state;
	Boolean		container_created;
	Widget		outline_button;
	Boolean		visible_in_outline;
	Boolean		was_managed;
	Position	row_y;
	Dimension	row_height;
	Dimension	detail_height;
	Cardinal	entry_detail_count;
	/* Private variables used by Container SpatialLayout methods */
	int		cell_idx;
	}	XmContainerConstraintPart, * XmContainerConstraint;

typedef	struct	_XmContainerConstraintRec
	{
	XmManagerConstraintPart		manager;
	XmContainerConstraintPart	container;
	}	XmContainerConstraintRec, * XmContainerConstraintPtr;

#ifdef _NO_PROTO
typedef Boolean (*XmSpatialPlacementProc)();
typedef Boolean (*XmSpatialRemoveProc)();
typedef Boolean (*XmSpatialTestFitProc)();
#else
typedef Boolean (*XmSpatialPlacementProc)(Widget, Widget);
typedef Boolean (*XmSpatialRemoveProc)(Widget, Widget);
typedef Boolean (*XmSpatialTestFitProc)(Widget, Widget,
                                                Position, Position);
#endif	/* _NO_PROTO */

/* Container widget class record  */
typedef	struct	_XmContainerClassPart
	{
	XmSpatialTestFitProc		test_fit_item;
	XmSpatialPlacementProc		place_item;
	XmSpatialRemoveProc		remove_item;
	XtGeometryHandler		query_spatial_geometry;
	}	XmContainerClassPart;

/* Full class record declaration */
typedef	struct	_XmContainerClassRec
	{
	CoreClassPart		core_class;
	CompositeClassPart  	composite_class;
	ConstraintClassPart	constraint_class;
	XmManagerClassPart	manager_class;
	XmContainerClassPart		container_class;
	}	XmContainerClassRec;

extern	XmContainerClassRec	xmContainerClassRec;

/* Container instance record */
typedef	struct	_XmContainerPart
	{
	unsigned char	automatic;		/* XmNautomaticSelection */
	Pixmap		collapsed_state_pixmap;	/* XmNcollapsedStatePixmap */
	XtCallbackList	convert_cb;		/* XmNconvertCallback */
	XtCallbackList	default_action_cb;	/* XmNdefaultActionCallback */
	XtCallbackList	destination_cb;		/* XmNdestinationCallback */
	_XmString *	detail_heading;		/* XmNdetailColumnHeading */
	Cardinal	detail_count;		/* XmNdetailCount */
	unsigned char	detail_sep_style;	/* XmNdetailSeparatorStyle */
	unsigned char	entry_viewtype;		/* XmNentryViewType */
	Pixmap		expanded_state_pixmap;	/* XmNexpandedStatePixmap */
	XmFontList	render_table;		/* XmNfontList */
	unsigned char	include_model;		/* XmNincludeModel */
	Dimension	large_cell_height;	/* XmNlargeCellHeight */
	Dimension	large_cell_width;	/* XmNlargeCellWidth */
	unsigned char	layout_direction;	/* XmNlayoutDirection */
	unsigned char	layout_type;		/* XmNlayoutType */
	Dimension	margin_h;		/* XmNmarginHeight */
	Dimension	margin_w;		/* XmNmarginWidth */
	unsigned char	orientation;		/* XmNorientation */
	Dimension	outline_indent;		/* XmNoutlineIndentation */
	unsigned char	outline_sep_style;	/* XmNoutlineLineStyle */
	unsigned char	place_style;		/* XmNplaceStyle */
	unsigned char	primary_ownership;	/* XmNprimaryOwnership */
	Boolean		resize_height;		/* XmNresizeHeight */
	Boolean		resize_width;		/* XmNresizeWidth */
	unsigned char	resize_model;		/* XmNresizeModel */
	WidgetList	selected_items;		/* DtNselectedItems */
	unsigned int	selected_item_count;	/* DtNselectedItemCount */
	XtCallbackList	selection_cb;		/* DtNselectionCallback */
	unsigned char	selection_policy;	/* XmNselectionPolicy */
	unsigned char	selection_technique;	/* XmNselectionTechnique */
	Dimension	small_cell_height;	/* XmNsmallCellHeight */
	Dimension	small_cell_width;	/* XmNsmallCellWidth */
	unsigned char	snap_model;		/* XmNsnapModel */
	XtCallbackList	value_changed_cb;	/* XmNvalueChangedCallback */
	/* Private variables */
	GC		normalGC;
	GC		inverseGC;

	Boolean		extending_mode;
	Boolean		marquee_mode;
	CwidNode 	first_node;
	CwidNode 	last_node;
	Boolean		self;
	Boolean		toggle_pressed;
	Boolean		extend_pressed;
	Boolean		cancel_pressed;
	Boolean		kaddmode;

	unsigned char	selection_state;
	Boolean		no_auto_sel_changes;
	Widget		anchor_cwid;
	Boolean		started_in_anchor;
	XPoint		anchor_point;
	Boolean		marquee_drawn;
	XPoint		marquee_start;
	XPoint		marquee_end;
	XPoint		marquee_smallest;
	XPoint		marquee_largest;
	GC		marqueeGC;

	Widget		loc_cursor_cwid;
	Boolean		have_focus;
	Boolean		have_primary;

	Time		last_click_time;

	Dimension	hspacing;
	Dimension	ob_width;
	Dimension	ob_height;

	XRectangle	heading_rect;
	int *		tabs;
	Dimension	heading_col1_width;
	Cardinal	detail_count_at_create;

	int		max_depth;
	XSegment	*outline_segs;
	int		outline_seg_count;
	XSegment	*detail_segs;
	int		detail_seg_count;

	Dimension	prev_width;
	Dimension	ideal_width;
	Dimension	ideal_height;
	Boolean		width_frozen;
	Boolean		height_frozen;

	Widget		*cells;
	int		cell_count;
	int		next_free_cell;
	int		current_width_in_cells;
	int		current_height_in_cells;

	ContainerXfrAction transfer_action;
	XtIntervalId	transfer_timer_id;
	Boolean		dragging;
	int		drag_offset_x;
	int		drag_offset_y;
	Widget		druggee;
	}	XmContainerPart;

/* Full instance record declaration */
typedef	struct	_XmContainerRec
	{
	CorePart	core;
	CompositePart   composite;
	ConstraintPart	constraint;
	XmManagerPart	manager;
	XmContainerPart	container;
	}	XmContainerRec;

#define CtrIsAUTO_SELECT(w) \
        ((((XmContainerWidget)(w))->container.automatic == XmAUTO_SELECT) && \
         (((XmContainerWidget)(w))->container.selection_policy \
                                                        == XmSINGLE_SELECT))
#define	CtrViewIsLARGEICON(w) \
	(((XmContainerWidget)(w))->container.entry_viewtype == XmLARGE_ICON)
#define CtrViewIsSMALLICON(w) \
	(((XmContainerWidget)(w))->container.entry_viewtype == XmSMALL_ICON)
#define CtrViewIsDETAIL(w) \
	(((XmContainerWidget)(w))->container.entry_viewtype == XmDETAIL)
#define	CtrDrawLinesDETAIL(w) \
	(CtrViewIsDETAIL(w) && \
	 (((XmContainerWidget)(w))->container.detail_count > 0) && \
	 (((XmContainerWidget)(w))->container.detail_sep_style == XmSINGLE))
#define	CtrLayoutIsLtoR(w) \
	(((XmContainerWidget)(w))->container.layout_direction \
					== XmLEFT_TO_RIGHT)
#define	CtrLayoutIsRtoL(w) \
	(((XmContainerWidget)(w))->container.layout_direction \
					== XmRIGHT_TO_LEFT)
#define	CtrIsHORIZONTAL(w) \
	(((XmContainerWidget)(w))->container.orientation \
					== XmHORIZONTAL)
#define	CtrIsVERTICAL(w) \
	(((XmContainerWidget)(w))->container.orientation \
					== XmVERTICAL)
#define CtrLayoutIsOUTLINE(w) \
	(((XmContainerWidget)(w))->container.layout_type == XmOUTLINE)
#define CtrDrawLinesOUTLINE(w) \
	(CtrLayoutIsOUTLINE(w) && \
	 (((XmContainerWidget)(w))->container.outline_sep_style \
					== XmSINGLE))
#define CtrLayoutIsSPATIAL(w) \
	(((XmContainerWidget)(w))->container.layout_type == XmSPATIAL)
#define	CtrPlaceStyleIsNONE(w) \
	(((XmContainerWidget)(w))->container.place_style == XmNONE)
#define CtrPlaceStyleIsGRID(w) \
	(((XmContainerWidget)(w))->container.place_style == XmGRID)
#define CtrPlaceStyleIsCELLS(w) \
	(((XmContainerWidget)(w))->container.place_style == XmCELLS)
#define	CtrIncludeIsAPPEND(w) \
	(((XmContainerWidget)(w))->container.include_model == XmAPPEND)
#define	CtrIncludeIsCLOSEST(w) \
	(((XmContainerWidget)(w))->container.include_model == XmCLOSEST)
#define	CtrIncludeIsFIRST_FIT(w) \
	(((XmContainerWidget)(w))->container.include_model == XmFIRST_FIT)
#define	CtrPolicyIsSINGLE(w) \
	(((XmContainerWidget)(w))->container.selection_policy \
					== XmSINGLE_SELECT)
#define	CtrPolicyIsBROWSE(w) \
	(((XmContainerWidget)(w))->container.selection_policy \
					== XmBROWSE_SELECT)
#define	CtrPolicyIsMULTIPLE(w) \
	(((XmContainerWidget)(w))->container.selection_policy \
					== XmMULTIPLE_SELECT)
#define	CtrPolicyIsEXTENDED(w) \
	(((XmContainerWidget)(w))->container.selection_policy \
					== XmEXTENDED_SELECT)
#define	CtrTechIsTOUCH_OVER(w) \
	(((XmContainerWidget)(w))->container.selection_technique \
					== XmTOUCH_OVER)
#define	CtrTechIsTOUCH_ONLY(w) \
	(((XmContainerWidget)(w))->container.selection_technique \
					== XmTOUCH_ONLY)
#define	CtrTechIsMARQUEE(w) \
	(((XmContainerWidget)(w))->container.selection_technique \
					== XmMARQUEE)
#define CtrTechIsMARQUEE_ES(w) \
	(((XmContainerWidget)(w))->container.selection_technique \
					== XmMARQUEE_EXTEND_START)
#define	CtrTechIsMARQUEE_EB(w) \
	(((XmContainerWidget)(w))->container.selection_technique \
					== XmMARQUEE_EXTEND_BOTH)
#define GetContainerConstraint(w) \
	(&((XmContainerConstraintPtr) (w)->core.constraints)->container)
#define	CtrItemIsPlaced(w) \
	(((XmContainerConstraintPtr)(w)->core.constraints)->container.cell_idx \
					!= NO_CELL)

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmContainerP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
