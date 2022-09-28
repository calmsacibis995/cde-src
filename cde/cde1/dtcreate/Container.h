/* $XConsortium: Container.h /main/cde1_maint/1 1995/07/17 20:24:07 drk $ */
/*
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * Motif top of 2.0 tree as of Tue Jun  1 05:18:29 EDT 1993
 */
#ifndef	_XmContainer_h
#define _XmContainer_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************
 * Container Widget defines                                             *
 ************************************************************************/
#define		XmUNSPECIFIED_POSITION		-1

#define 	XmRDirection			"Direction"

        /* XmRAutomaticSelection */
enum {  XmAUTO_SELECT,
        XmNO_AUTO_SELECT
        };
        /* XmRLineStyle */
enum {  /* XmNO_LINE */
        XmSINGLE = 1
        };
        /* XmREntryViewType */
enum {  /* XmLARGE_ICON */
        /* XmSMALL_ICON */
        XmDETAIL = 2
        };
        /* XmRIncludeModel */
enum {  XmAPPEND,
        XmCLOSEST,
        XmFIRST_FIT
        };
        /* XmRLayoutType */
enum {  XmOUTLINE,
        XmSPATIAL
        };
        /* XmRPlaceStyle */
enum {  /* XmNONE */
	XmGRID = 1,
	XmCELLS	
        };
        /* XmRSelectionTechnique */
enum {  XmMARQUEE,
        XmMARQUEE_EXTEND_START,
        XmMARQUEE_EXTEND_BOTH,
        XmTOUCH_ONLY,
        XmTOUCH_OVER
        };
        /* XmRExpandState */
enum {  XmCOLLAPSED,
        XmEXPANDED
        };
	/* XmRDirection */
enum{ 	XmLEFT_TO_RIGHT,
	XmRIGHT_TO_LEFT
    };

typedef struct
{
  int     reason;
  XEvent  *event;
  Widget  item;
} XmContainerOutlineCallbackStruct;

typedef struct
{
  int             reason;
  XEvent          *event;
  WidgetList      selected_items;
  int             selected_item_count;
  unsigned char   auto_selection_type;
} XmContainerSelectCallbackStruct;

enum{   XmAUTO_UNSET,			XmAUTO_BEGIN,
	XmAUTO_MOTION,			XmAUTO_CANCEL,
	XmAUTO_NO_CHANGE,		XmAUTO_CHANGE
        };
enum{	XmCR_COLLAPSED,			XmCR_EXPANDED
	};


/* Class record constants */
extern	WidgetClass	xmContainerWidgetClass;

typedef	struct	_XmContainerClassRec	*XmContainerWidgetClass;
typedef struct  _XmContainerRec	*XmContainerWidget;

#ifndef XmIsContainer
#define XmIsContainer(w) XtIsSubclass(w, xmContainerWidgetClass)
#endif /* XmIsContainer */

#define XmNcollapsedStatePixmap "collapsedStatePixmap"
#define XmNdetailColumnHeading  "detailColumnHeading"
#define XmNdetailCount          "detailCount"
#define XmNdetailMask           "detailMask"
#define XmNdetailSeparatorStyle "detailSeparatorStyle"
#define XmNdetailTabList        "detailTabList"
#define XmNentryViewType        "entryViewType"
#define XmNexpandedStatePixmap  "expandedStatePixmap"
#define XmNincludeModel         "includeModel"
#define XmNlargeCellHeight      "largeCellHeight"
#define XmNlargeCellWidth       "largeCellWidth"
#define XmNlayoutDirection      "layoutDirection"
#define XmNlayoutType           "layoutType"
#define XmNoutlineIndentation   "outlineIndentation"
#define XmNoutlineLineStyle     "outlineLineStyle"
#define XmNplaceStyle           "placeStyle"
#define	XmNprimaryOwnership	"primaryOwnership"
#define	XmNselectedObjects	"selectedObjects"
#define	XmNselectedObjectCount	"selectedObjectCount"
#define DtNselectionCallback    "selectionCallback"
#define XmNselectionTechnique   "selectionTechnique"
#define XmNsmallCellHeight      "smallCellHeight"
#define XmNsmallCellWidth       "smallCellWidth"

#define XmNentryDetail          "entryDetail"
#define XmNentryParent          "entryParent"
#define XmNoutlineState         "outlineState"
#define XmNlargeIconX           "largeIconX"
#define XmNlargeIconY           "largeIconY"
#define XmNsmallIconX           "smallIconX"
#define XmNsmallIconY           "smallIconY"

#define XmCCollapsedStatePixmap "CollapsedStatePixmap"
#define XmCDetailColumnHeading  "DetailColumnHeading"
#define XmCDetailCount          "DetailCount"
#define XmCDetailMask           "DetailMask"
#define XmCEntryViewType        "EntryViewType"
#define XmCLineStyle            "LineStyle"
#define XmCDetailTabList        "DetailTabList"
#define XmCExpandedStatePixmap  "ExpandedStatePixmap"
#define XmCIncludeModel         "IncludeModel"
#define XmCCellHeight           "CellHeight"
#define XmCCellWidth            "CellWidth"
#define XmCLayoutDirection      "LayoutDirection"
#define XmCLayoutType           "LayoutType"
#define XmCOutlineIndentation   "OutlineIndentation"
#define XmCPlaceStyle           "PlaceStyle"
#define	XmCPrimaryOwnership	"PrimaryOwnership"
#define	XmCSelectedObjects	"SelectedObjects"
#define	XmCSelectedObjectCount	"SelectedObjectCount"
#define XmCSelectionTechnique   "SelectionTechnique"

#define XmCEntryDetail          "EntryDetail"
#define XmCOutlineState         "OutlineState"

#define XmRAutomaticSelection   "AutomaticSelection"
#define XmRBitArray             "BitArray"
#define XmRLineStyle            "LineStyle"
#define XmREntryViewType        "EntryViewType"
#define XmRIncludeModel         "IncludeModel"
#define XmRDirection		"Direction"
#define XmRLayoutType           "LayoutType"
#define XmRPlaceStyle           "PlaceStyle"
#define XmRPrimaryOwnership	"PrimaryOwnership"
#define XmRSelectionTechnique   "SelectionTechnique"

#define XmROutlineState         "OutlineState"

/********    Public Function Declarations    ********/
#ifdef _NO_PROTO
extern	Widget		XmCreateContainer();
extern	int		XmContainerGetItemChildren();
extern	void		XmContainerRelayout();
extern	void		XmContainerReorder();
extern	Widget		XmContainerGetItemByCoordinates();
#else
extern	Widget	XmCreateContainer(
			Widget parent,
			char *name,
			ArgList arglist,
			Cardinal argcount);
extern	int	XmContainerGetItemChildren(
			Widget		wid,
			Widget		item,
			WidgetList	*item_children);
extern	void	XmContainerRelayout(
			Widget	wid);
extern	void	XmContainerReorder(
			Widget		wid,
			WidgetList	cwid_list,
			int		cwid_count);	
extern	Widget	XmContainerGetItemByCoordinates(
			Widget		wid,
			Position        x,
			Position        y);
#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmContainer_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

