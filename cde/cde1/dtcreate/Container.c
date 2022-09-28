/* $XConsortium: Container.c /main/cde1_maint/1 1995/07/17 20:23:45 drk $ */
/*
 * Container.c: The Container widget methods.
 */
#define DEBUG

#include <stdlib.h>
#ifdef  DEBUG
#include <stdio.h>
#endif  /* DEBUG */
#include "ContainerP.h"
#include "IconG.h"
#ifdef  COSEICON
#include <Dt/Icon.h>
#endif  /* COSEICON */
#include <Xm/GadgetP.h>
#include <Xm/DrawP.h>
#include <Xm/PrimitiveP.h>
#include <Xm/PushB.h>
#include <Xm/DragDrop.h>
#include <Xm/RepType.h>

#ifndef MIN
#define MIN(x,y)        ((x) < (y) ? (x) : (y))
#endif /* MIN */
#ifndef MAX
#define MAX(x,y)  ((x) > (y) ? (x) : (y))
#endif /* MAX */

#ifndef __STDC__
#define const   /* nothing */
#define volatile /* nothing */
#endif

#define ZERO_CELL_DIM   0
#define UNSPECIFIED_POSITION_INDEX      -32767
#define NO_CELL -1
#define OBNAME  "_XM_CONTAINER_OUTLINE_BUTTON"

/* Useful macros */
#define _FIRST  "First"
#define _LAST   "Last"
#define _LEFT   "Left"
#define _RIGHT  "Right"
#define _UP     "Up"
#define _DOWN   "Down"
#define _IN     "In"
#define _OUT    "Out"
#define _ENTER  "Enter"
#define _LEAVE  "Leave"
#define _COPY   "Copy"
#define _LINK   "Link"
#define _MOVE   "Move"


/********    Static Function Declarations    ********/
#ifdef _NO_PROTO
        /* Core methods */
static  void                    ClassInitialize();
static  void                    Initialize();
static  void                    Destroy();
static  void                    Resize();
static  void                    Redisplay();
static  Boolean                 SetValues();
static  Boolean                 SVNeedLayout();
static  XtGeometryResult        QueryGeometry();
        /* Composite methods */
static  XtGeometryResult        GeometryManager();
static  void                    ChangeManaged();
        /* Constraint methods */
static  void                    ConstraintInitialize();
static  void                    ConstraintDestroy();
static  Boolean                 ConstraintSetValues();
        /* Manager methods (none) */
        /* Container methods */
static  Boolean                 TestFitItem();
static  Boolean                 PlaceItem();
static  Boolean                 RemoveItem();
static  XtGeometryResult        QuerySpatialGeometry();
        /* Private functions call by Container Methods */
static  void                    PlaceItemNone();
static  void                    PlaceItemGrid();
static  int                     GetCellFromCoord();
static  XPoint *                GetCoordFromCell();
static  void                    PlaceItemCells();
static  void                    PlaceItemReset();
static  void                    PlaceCwid();
static  void                    HideCwid();
        /* Action Procs */
static  void                    ContainerBeginSelect();
static  void                    ContainerButtonMotion();
static  void                    ContainerEndSelect();
static  void                    ContainerBeginToggle();
static  void                    ContainerEndToggle();
static  void                    ContainerBeginExtend();
static  void                    ContainerEndExtend();
static  void                    ContainerCancel();
static  void                    ContainerSelect();
static  void                    ContainerExtend();
static  void                    ContainerMoveCursor();
static  void                    ContainerExtendCursor();
static  void                    ContainerToggleMode();
static  void                    ContainerSelectAll();
static  void                    ContainerDeselectAll();
static  void                    ContainerActivate();
static  void                    ContainerFocus();
        /* Internal Functions */
static  void                    Layout();
static  void                    RequestNewSize();
static  void                    LayoutOutlineDetail();
static  void                    LayoutSpatial();
static  void                    SetCellSize();
static  void                    SizeOutlineButton();
static  void                    UpdateGCs();
static  Dimension               SizeOneSpace();
static  void                    CreateDetailHeading();
static  void                    CreateEntryDetail();
static  Boolean                 ShowColumn();
static  void                    ExposeDetail();
static  void                    ChangeView();
static  CwidNode                NewNode();
static  void                    InsertNode();
static  void                    SeverNode();
static  void                    DeleteNode();
static  CwidNode                GetNextNode();
static  CwidNode                GetNextUpLevelNode();
static  CwidNode                GetPrevNode();
static  void                    StartSelect();
static  Boolean                 ProcessButtonMotion();
static  Widget                  LocatePointer();
static  Boolean                 SelectAllCwids();
static  Boolean                 DeselectAllCwids();
static  Boolean                 MarkCwid();
static  Boolean                 UnmarkCwidVisual();
static  void                    SetMarkedCwids();
static  Boolean                 ResetMarkedCwids();
static  Boolean                 MarkCwidsInRange();
static  Boolean                 MarkCwidsInMarquee();
static  Boolean                 InMarquee();
static  void                    RecalcMarquee();
static  void                    DrawMarquee();
static  void                    KBSelect();
static  void                    SetLocationCursor();
static  void                    BorderHighlight();
static  void                    BorderUnhighlight();
static  Widget                  CalcNextLocationCursor();
static  void                    MakeOutlineButton();
static  void                    ExpandCwid();
static  void                    CollapseCwid();
static  void                    CallActionCB();
static  void                    CallSelectCB();
static  WidgetList              GetSelectedCwids();
static  void                    GetSelectedItems();
static  void                    GetDetailHeading();
static  void                    GetEntryDetail();
static  int                     CompareInts();
static  int                     Isqrt();
        /* Internal Callback Functions */
static  void                    OutlineButtonCallback();
        /* External Function declarations are in XmContainer.h */

#else
        /* Core methods */
static  void                    ClassInitialize(void);
static  void                    Initialize(
                                        Widget          rw,
                                        Widget          nw,
                                        ArgList         args,
                                        Cardinal        *num_args);
static  void                    Destroy(
                                        Widget          wid);
static  void                    Resize(
                                        Widget          wid);
static  void                    Redisplay(
                                        Widget          wid,
                                        XEvent          *event,
                                        Region          region);
static  Boolean                 SetValues(
                                        Widget          cw,
                                        Widget          rw,
                                        Widget          nw,
                                        ArgList         args,
                                        Cardinal        *num_args);
static  Boolean                 SVNeedLayout(
                                        Widget          cw,
                                        Widget          nw);
static  XtGeometryResult        QueryGeometry(
                                        Widget          wid,
                                        XtWidgetGeometry *intended,
                                        XtWidgetGeometry *desired);
        /* Composite methods */
static  XtGeometryResult        GeometryManager(
                                        Widget          instigator,
                                        XtWidgetGeometry *desired,
                                        XtWidgetGeometry *allowed);
static  void                    ChangeManaged(
                                        Widget          wid);
        /* Constraint methods */
static  void                    ConstraintInitialize(
                                        Widget          rcwid,
                                        Widget          ncwid,
                                        ArgList         args,
                                        Cardinal        *num_args);
static  void                    ConstraintDestroy(
                                        Widget          cwid);
static  Boolean                 ConstraintSetValues(
                                        Widget          ccwid,
                                        Widget          rcwid,
                                        Widget          ncwid,
                                        ArgList         args,
                                        Cardinal        *num_args);
        /* Manager methods (none) */
        /* Container methods */
static  Boolean                 TestFitItem(
                                        Widget          wid,
                                        Widget          cwid,
                                        Position        x,
                                        Position        y);
static  Boolean                 PlaceItem(
                                        Widget          wid,
                                        Widget          cwid);
static  Boolean                 RemoveItem(
                                        Widget          wid,
                                        Widget          cwid);
static  XtGeometryResult        QuerySpatialGeometry(
                                        Widget          wid,
                                        XtWidgetGeometry *intended,
                                        XtWidgetGeometry *desired);
        /* Private functions call by Container Methods */
static  void                    PlaceItemNone(
                                        Widget          wid,
                                        Widget          cwid);
static  void                    PlaceItemGrid(
                                        Widget          wid,
                                        Widget          cwid);
static  int                     GetCellFromCoord(
                                        Widget          wid,
                                        Position        x,
                                        Position        y);
static  XPoint *                GetCoordFromCell(
                                        Widget          wid,
                                        int             cell_idx);
static  void                    PlaceItemCells(
                                        Widget          wid,
                                        Widget          cwid);
static  void                    PlaceItemReset(
                                        Widget          wid);
static  void                    PlaceCwid(
                                        Widget          cwid,
                                        Position        x,
                                        Position        y);
static  void                    HideCwid(
                                        Widget          cwid);
        /* Action Procs */
static  void                    ContainerBeginSelect(
                                        Widget          wid,
                                        XEvent          *event,
                                        String          *params,
                                        Cardinal        *num_params);
static  void                    ContainerButtonMotion(
                                        Widget          wid,
                                        XEvent          *event,
                                        String          *params,
                                        Cardinal        *num_params);
static  void                    ContainerEndSelect(
                                        Widget          wid,
                                        XEvent          *event,
                                        String          *params,
                                        Cardinal        *num_params);
static  void                    ContainerBeginToggle(
                                        Widget          wid,
                                        XEvent          *event,
                                        String          *params,
                                        Cardinal        *num_params);
static  void                    ContainerEndToggle(
                                        Widget          wid,
                                        XEvent          *event,
                                        String          *params,
                                        Cardinal        *num_params);
static  void                    ContainerBeginExtend(
                                        Widget          wid,
                                        XEvent          *event,
                                        String          *params,
                                        Cardinal        *num_params);
static  void                    ContainerEndExtend(
                                        Widget          wid,
                                        XEvent          *event,
                                        String          *params,
                                        Cardinal        *num_params);
static  void                    ContainerCancel(
                                        Widget          wid,
                                        XEvent          *event,
                                        String          *params,
                                        Cardinal        *num_params);
static  void                    ContainerSelect(
                                        Widget          wid,
                                        XEvent          *event,
                                        String          *params,
                                        Cardinal        *num_params);
static  void                    ContainerExtend(
                                        Widget          wid,
                                        XEvent          *event,
                                        String          *params,
                                        Cardinal        *num_params);
static  void                    ContainerMoveCursor(
                                        Widget          wid,
                                        XEvent          *event,
                                        String          *params,
                                        Cardinal        *num_params);
static  void                    ContainerExtendCursor(
                                        Widget          wid,
                                        XEvent          *event,
                                        String          *params,
                                        Cardinal        *num_params);
static  void                    ContainerToggleMode(
                                        Widget          wid,
                                        XEvent          *event,
                                        String          *params,
                                        Cardinal        *num_params);
static  void                    ContainerSelectAll(
                                        Widget          wid,
                                        XEvent          *event,
                                        String          *params,
                                        Cardinal        *num_params);
static  void                    ContainerDeselectAll(
                                        Widget          wid,
                                        XEvent          *event,
                                        String          *params,
                                        Cardinal        *num_params);
static  void                    ContainerActivate(
                                        Widget          wid,
                                        XEvent          *event,
                                        String          *params,
                                        Cardinal        *num_params);
static  void                    ContainerFocus(
                                        Widget          wid,
                                        XEvent          *event,
                                        String          *params,
                                        Cardinal        *num_params);
        /* Internal Functions */
static  void                    Layout(
                                        Widget          wid);
static  void                    RequestNewSize(
                                        Widget          wid);
static  void                    LayoutOutlineDetail(
                                        Widget          wid);
static  void                    LayoutSpatial(
                                        Widget          wid);
static  void                    SetCellSize(
                                        Widget          wid);
static  void                    SizeOutlineButton(
                                        Widget          wid);
static  void                    UpdateGCs(
                                        Widget          wid);
static  Dimension               SizeOneSpace(
                                        XmFontList      fl);
static  void                    CreateDetailHeading(
                                        Widget          wid);
static  void                    CreateEntryDetail(
                                        Widget          cwid);
static  Boolean                 ShowColumn(
                                        Widget          wid,
                                        int             column);
static  void                    ExposeDetail(
                                        Widget          cwid);
static  void                    ChangeView(
                                        Widget          wid,
                                        unsigned char   view);
static  CwidNode                NewNode(
                                        Widget          cwid);
static  void                    InsertNode(
                                        CwidNode        node);
static  void                    SeverNode(
                                        CwidNode        node);
static  void                    DeleteNode(
                                        Widget          cwid);
static  CwidNode                GetNextNode(
                                        Widget          wid,
                                        CwidNode        start_item);
static  CwidNode                GetNextUpLevelNode(
                                        CwidNode        start_item);
static  CwidNode                GetPrevNode(
                                        Widget          wid,
                                        CwidNode        start_item);
static  void                    StartSelect(
                                        Widget          wid,
                                        XEvent          *event,
                                        String          *params,
                                        Cardinal        *num_params);
static  Boolean                 ProcessButtonMotion(
                                        Widget          wid,
                                        XEvent          *event,
                                        String          *params,
                                        Cardinal        *num_params);
static  Widget                  LocatePointer(
                                        Widget          wid,
                                        int             x,
                                        int             y);
static  Boolean                 SelectAllCwids(
                                        Widget          wid);
static  Boolean                 DeselectAllCwids(
                                        Widget          wid);
static  Boolean                 MarkCwid(
                                        Widget          cwid,
                                        Boolean         visual_only);
static  Boolean                 UnmarkCwidVisual(
                                        Widget          cwid);
static  void                    SetMarkedCwids(
                                        Widget          wid);
static  Boolean                 ResetMarkedCwids(
                                        Widget          wid);
static  Boolean                 MarkCwidsInRange(
                                        Widget          wid,
                                        Widget          cwid1,
                                        Widget          cwid2,
                                        Boolean         visual_only);
static  Boolean                 MarkCwidsInMarquee(
                                        Widget          wid,
                                        Boolean         find_anchor,
                                        Boolean         visual_only);
static  Boolean                 InMarquee(
                                        Widget          cwid);
static  void                    RecalcMarquee(
                                        Widget          wid,
                                        Widget          cwid,
                                        Position        x,
                                        Position        y);
static  void                    DrawMarquee(
                                        Widget          wid);
static  void                    KBSelect(
                                        Widget          wid,
                                        XEvent          *event,
                                        String          *params,
                                        Cardinal        *num_params);
static  void                    SetLocationCursor(
                                        Widget          cwid);
static  void                    BorderHighlight(
                                        Widget          cwid);
static  void                    BorderUnhighlight(
                                        Widget          cwid);
static  Widget                  CalcNextLocationCursor(
                                        Widget          wid,
                                        String          direction);
static  void                    MakeOutlineButton(
                                        Widget          cwid);
static  void                    ExpandCwid(
                                        Widget          cwid);
static  void                    CollapseCwid(
                                        Widget          cwid);
static  void                    CallActionCB(
                                        Widget          cwid,
                                        XEvent          *event);
static  void                    CallSelectCB(
                                        Widget          wid,
                                        XEvent          *event,
                                        unsigned char   auto_selection_type);
static  WidgetList              GetSelectedCwids(
                                        Widget          wid);
static  void                    GetSelectedItems(
                                        Widget          wid,
                                        int             offset,
                                        XtArgVal        *value);
static  void                    GetDetailHeading(
                                        Widget          wid,
                                        int             offset,
                                        XtArgVal        *value);
static  void                    GetEntryDetail(
                                        Widget          cwid,
                                        int             offset,
                                        XtArgVal        *value);
static  int                     CompareInts(
                                        const void      *p1,
                                        const void      *p2);
static  int                     Isqrt(
                                        int             n);
        /* Internal Callback Functions */
static  void                     OutlineButtonCallback(
                                        Widget          pbwid,
                                        XtPointer       client_data,
                                        XtPointer       call_data);
        /* External Function declarations are in Container.h */

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/

static  char    defaultTranslations[] = "\
~c ~s ~m ~a <Btn1Down>:         ContainerBeginSelect()\n\
<Btn1Motion>:                   ContainerButtonMotion()\n\
~c ~s ~m ~a <Btn1Up>:           ContainerEndSelect()\n\
 c ~s ~m ~a <Btn1Down>:         ContainerBeginToggle()\n\
 c ~s ~m ~a <Btn1Up>:           ContainerEndToggle()\n\
~c  s ~m ~a <Btn1Down>:         ContainerBeginExtend()\n\
~c  s ~m ~a <Btn1Up>:           ContainerEndExtend()\n\
 c  s ~m ~a <Btn1Down>:         ContainerBeginExtend()\n\
 c  s ~m ~a <Btn1Up>:           ContainerEndExtend()\n\
<Key>osfCancel:                 ContainerCancel()\n\
~c ~s <Key>osfSelect:           ContainerSelect()\n\
~c  s <Key>osfSelect:           ContainerExtend()\n\
<Key>osfAddMode:                ContainerToggleMode()\n\
~s  c ~m ~a <Key>slash:         ContainerSelectAll()\n\
~s  c ~m ~a <Key>backslash:     ContainerDeselectAll()\n\
<Key>osfActivate:               ContainerActivate()\n\
";

static  char    traversalTranslations[] = "\
 c ~s <Key>osfBeginLine:        ContainerMoveCursor(First)\n\
 c ~s <Key>osfEndLine:          ContainerMoveCursor(Last)\n\
 c  s <Key>osfBeginLine:        ContainerExtendCursor(First)\n\
 c  s <Key>osfEndLine:          ContainerExtendCursor(Last)\n\
~s <Key>osfUp:                  ContainerMoveCursor(Up)\n\
~s <Key>osfDown:                ContainerMoveCursor(Down)\n\
~s <Key>osfLeft:                ContainerMoveCursor(Left)\n\
~s <Key>osfRight:               ContainerMoveCursor(Right)\n\
 s <Key>osfUp:                  ContainerExtendCursor(Up)\n\
 s <Key>osfDown:                ContainerExtendCursor(Down)\n\
 s <Key>osfLeft:                ContainerExtendCursor(Left)\n\
 s <Key>osfRight:               ContainerExtendCursor(Right)\n\
<FocusIn>:                      ContainerFocus(In)\n\
<FocusOut>:                     ContainerFocus(Out)\n\
 s ~m ~a <Key>Tab:              ManagerGadgetPrevTabGroup()\n\
~s ~m ~a <Key>Tab:              ManagerGadgetNextTabGroup()\n\
";

static  XtActionsRec    actionsList[] = {
{"ContainerBeginSelect",        (XtActionProc)  ContainerBeginSelect},
{"ContainerButtonMotion",       (XtActionProc)  ContainerButtonMotion},
{"ContainerEndSelect",          (XtActionProc)  ContainerEndSelect},
{"ContainerBeginToggle",        (XtActionProc)  ContainerBeginToggle},
{"ContainerEndToggle",          (XtActionProc)  ContainerEndToggle},
{"ContainerBeginExtend",        (XtActionProc)  ContainerBeginExtend},
{"ContainerEndExtend",  (XtActionProc)  ContainerEndExtend},
{"ContainerCancel",             (XtActionProc)  ContainerCancel},
{"ContainerSelect",             (XtActionProc)  ContainerSelect},
{"ContainerExtend",             (XtActionProc)  ContainerExtend},
{"ContainerMoveCursor",         (XtActionProc)  ContainerMoveCursor},
{"ContainerExtendCursor",       (XtActionProc)  ContainerExtendCursor},
{"ContainerToggleMode",         (XtActionProc)  ContainerToggleMode},
{"ContainerSelectAll",          (XtActionProc)  ContainerSelectAll},
{"ContainerDeselectAll",        (XtActionProc)  ContainerDeselectAll},
{"ContainerActivate",           (XtActionProc)  ContainerActivate},
{"ContainerFocus",              (XtActionProc)  ContainerFocus},
};

static  XtResource      resources[] =
        {
                {
                XmNautomaticSelection,XmCAutomaticSelection,
                        XmRAutomaticSelection,sizeof(unsigned char),
                XtOffset(XmContainerWidget,container.automatic),
                        XmRImmediate,(XtPointer)XmAUTO_SELECT},
                {
                XmNcollapsedStatePixmap,XmCCollapsedStatePixmap,XmRPixmap,
                        sizeof(Pixmap),
                XtOffset(XmContainerWidget,container.collapsed_state_pixmap),
                        XmRImmediate,(XtPointer)XmUNSPECIFIED_PIXMAP},
                {
                XmNdefaultActionCallback,XmCCallback,XmRCallback,
                        sizeof(XtCallbackList),
                XtOffset(XmContainerWidget,container.default_action_cb),
                        XmRImmediate,(XtPointer)NULL},
                {
                XmNdetailColumnHeading,XmCDetailColumnHeading,XmRXmStringTable,
                        sizeof(XmStringTable),
                XtOffset(XmContainerWidget,container.detail_heading),
                        XmRImmediate,(XtPointer)NULL},
                {
                XmNdetailCount,XmCDetailCount,XmRCardinal,sizeof(Cardinal),
                XtOffset(XmContainerWidget,container.detail_count),
                        XmRImmediate,(XtPointer)0},
                {
                XmNdetailSeparatorStyle,XmCLineStyle,XmRLineStyle,
                        sizeof(unsigned char),
                XtOffset(XmContainerWidget,container.detail_sep_style),
                        XmRImmediate,(XtPointer)XmSINGLE},
                {
                XmNentryViewType,XmCEntryViewType,XmREntryViewType,
                        sizeof(unsigned char),
                XtOffset(XmContainerWidget,container.entry_viewtype),
                        XmRImmediate,(XtPointer)XmLARGE_ICON},
                {
                XmNexpandedStatePixmap,XmCExpandedStatePixmap,XmRPixmap,
                        sizeof(Pixmap),
                XtOffset(XmContainerWidget,container.expanded_state_pixmap),
                        XmRImmediate,(XtPointer)XmUNSPECIFIED_PIXMAP},
                {
                XmNfontList,XmCFontList,XmRFontList,
                        sizeof(XmFontList),
                XtOffset(XmContainerWidget,container.render_table),
                        XmRImmediate,(XtPointer)NULL},
                {
                XmNincludeModel,XmCIncludeModel,XmRIncludeModel,
                        sizeof(unsigned char),
                XtOffset(XmContainerWidget,container.include_model),
                        XmRImmediate,(XtPointer)XmAPPEND},
                {
                XmNlargeCellHeight,XmCCellHeight,XmRDimension,
                        sizeof(Dimension),
                XtOffset(XmContainerWidget,container.large_cell_height),
                        XmRImmediate,(XtPointer)ZERO_CELL_DIM},
                {
                XmNlargeCellWidth,XmCCellWidth,XmRDimension,
                        sizeof(Dimension),
                XtOffset(XmContainerWidget,container.large_cell_width),
                        XmRImmediate,(XtPointer)ZERO_CELL_DIM},
                {
                XmNlayoutDirection,XmCLayoutDirection,XmRDirection,
                        sizeof(unsigned char),
                XtOffset(XmContainerWidget,container.layout_direction),
                        XmRImmediate,(XtPointer)XmLEFT_TO_RIGHT},
                {
                XmNlayoutType,XmCLayoutType,XmRLayoutType,
                        sizeof(unsigned char),
                XtOffset(XmContainerWidget,container.layout_type),
                        XmRImmediate,(XtPointer)XmSPATIAL},
                {
                XmNmarginHeight,XmCMarginHeight,XmRDimension,
                        sizeof(Dimension),
                XtOffset(XmContainerWidget,container.margin_h),
                        XmRImmediate,(XtPointer)0},
                {
                XmNmarginWidth,XmCMarginWidth,XmRDimension,
                        sizeof(Dimension),
                XtOffset(XmContainerWidget,container.margin_w),
                        XmRImmediate,(XtPointer)0},
                {
                XmNorientation,XmCOrientation,XmROrientation,
                        sizeof(unsigned char),
                XtOffset(XmContainerWidget,container.orientation),
                        XmRImmediate,(XtPointer)XmHORIZONTAL},
                {
                XmNoutlineIndentation,XmCOutlineIndentation,XmRDimension,
                        sizeof(Dimension),
                XtOffset(XmContainerWidget,container.outline_indent),
                        XmRImmediate,(XtPointer)100},
                {
                XmNoutlineLineStyle,XmCLineStyle,XmRLineStyle,
                        sizeof(unsigned char),
                XtOffset(XmContainerWidget,container.outline_sep_style),
                        XmRImmediate,(XtPointer)XmSINGLE},
                {
                XmNplaceStyle,XmCPlaceStyle,XmRPlaceStyle,
                        sizeof(unsigned char),
                XtOffset(XmContainerWidget,container.place_style),
                        XmRImmediate,(XtPointer)XmNONE},
                {
                XmNresizeHeight,XmCResizeHeight,XmRBoolean,sizeof(Boolean),
                XtOffset(XmContainerWidget,container.resize_height),
                        XmRImmediate,(XtPointer)True},
                {
                XmNresizeWidth,XmCResizeWidth,XmRBoolean,sizeof(Boolean),
                XtOffset(XmContainerWidget,container.resize_width),
                        XmRImmediate,(XtPointer)True},
                {
                XmNselectedObjects,XmCSelectedObjects,XmRWidgetList,
                        sizeof(WidgetList),
                XtOffset(XmContainerWidget,container.selected_items),
                        XmRImmediate,(XtPointer)NULL},
                {
                XmNselectedObjectCount,XmCSelectedObjectCount,XmRInt,
                        sizeof(int),
                XtOffset(XmContainerWidget,container.selected_item_count),
                        XmRImmediate,(XtPointer)0},
                {
                DtNselectionCallback,XmCCallback,XmRCallback,
                        sizeof(XtCallbackList),
                XtOffset(XmContainerWidget,container.selection_cb),
                        XmRImmediate,(XtPointer)NULL},
                {
                XmNselectionPolicy,XmCSelectionPolicy,XmRSelectionPolicy,
                        sizeof(unsigned char),
                XtOffset(XmContainerWidget,container.selection_policy),
                        XmRImmediate,(XtPointer)XmEXTENDED_SELECT},
                {
                XmNselectionTechnique,XmCSelectionTechnique,
                        XmRSelectionTechnique,sizeof(unsigned char),
                XtOffset(XmContainerWidget,container.selection_technique),
                        XmRImmediate,(XtPointer)XmTOUCH_OVER},
                {
                XmNsmallCellHeight,XmCCellHeight,XmRDimension,
                        sizeof(Dimension),
                XtOffset(XmContainerWidget,container.small_cell_height),
                        XmRImmediate,(XtPointer)ZERO_CELL_DIM},
                {
                XmNsmallCellWidth,XmCCellWidth,XmRDimension,
                        sizeof(Dimension),
                XtOffset(XmContainerWidget,container.small_cell_width),
                        XmRImmediate,(XtPointer)ZERO_CELL_DIM},
                {
                XmNvalueChangedCallback,XmCCallback,XmRCallback,
                        sizeof(XtCallbackList),
                XtOffset(XmContainerWidget,container.value_changed_cb),
                        XmRImmediate,(XtPointer)NULL},
        };

static  XmSyntheticResource     syn_resources[] =
        {
                {
                XmNdetailColumnHeading,sizeof(XmStringTable),
                XtOffset(XmContainerWidget,container.detail_heading),
                GetDetailHeading,NULL},
                {
                XmNlargeCellHeight,sizeof(Dimension),
                XtOffset(XmContainerWidget,container.large_cell_height),
                _XmFromVerticalPixels,_XmToVerticalPixels},
                {
                XmNlargeCellWidth,sizeof(Dimension),
                XtOffset(XmContainerWidget,container.large_cell_width),
                _XmFromHorizontalPixels,_XmToHorizontalPixels},
                {
                XmNmarginHeight,sizeof(Dimension),
                XtOffset(XmContainerWidget,container.margin_h),
                _XmFromVerticalPixels,_XmToVerticalPixels},
                {
                XmNmarginWidth,sizeof(Dimension),
                XtOffset(XmContainerWidget,container.margin_w),
                _XmFromHorizontalPixels,_XmToHorizontalPixels},
                {
                XmNoutlineIndentation,sizeof(Dimension),
                XtOffset(XmContainerWidget,container.outline_indent),
                _XmFromHorizontalPixels,_XmToHorizontalPixels},
                {
                XmNselectedObjects,sizeof(WidgetList),
                XtOffset(XmContainerWidget,container.selected_items),
                GetSelectedItems,NULL},
                {
                XmNsmallCellHeight,sizeof(Dimension),
                XtOffset(XmContainerWidget,container.small_cell_height),
                _XmFromVerticalPixels,_XmToVerticalPixels},
                {
                XmNsmallCellWidth,sizeof(Dimension),
                XtOffset(XmContainerWidget,container.small_cell_width),
                _XmFromHorizontalPixels,_XmToHorizontalPixels},
        };

/*  The constraint resource list  */

static  XtResource      constraints[] =
        {
                {
                XmNentryDetail,XmCEntryDetail,XmRXmStringTable,
                        sizeof(XmStringTable),
                XtOffset(XmContainerConstraintPtr,container.entry_detail),
                        XmRImmediate,(XtPointer)NULL},
                {
                XmNentryParent,XmCWidget,XmRWidget,sizeof(Widget),
                XtOffset(XmContainerConstraintPtr,container.entry_parent),
                        XmRImmediate,(XtPointer)NULL},
                {
                XmNoutlineState,XmCOutlineState,XmROutlineState,
                        sizeof(unsigned char),
                XtOffset(XmContainerConstraintPtr,container.outline_state),
                        XmRImmediate,(XtPointer)XmCOLLAPSED},
                {
                XmNlargeIconX,XmCIconX,XmRPosition,sizeof(Position),
                XtOffset(XmContainerConstraintPtr,container.large_icon_x),
                        XmRImmediate,(XtPointer)XmUNSPECIFIED_POSITION},
                {
                XmNlargeIconY,XmCIconY,XmRPosition,sizeof(Position),
                XtOffset(XmContainerConstraintPtr,container.large_icon_y),
                        XmRImmediate,(XtPointer)XmUNSPECIFIED_POSITION},
                {
                XmNpositionIndex,XmCPositionIndex,XmRInt,sizeof(int),
                XtOffset(XmContainerConstraintPtr,container.position_index),
                        XmRImmediate,(XtPointer)UNSPECIFIED_POSITION_INDEX},
                {
                XmNsmallIconX,XmCIconX,XmRPosition,sizeof(Position),
                XtOffset(XmContainerConstraintPtr,container.small_icon_x),
                        XmRImmediate,(XtPointer)XmUNSPECIFIED_POSITION},
                {
                XmNsmallIconY,XmCIconY,XmRPosition,sizeof(Position),
                XtOffset(XmContainerConstraintPtr,container.small_icon_y),
                        XmRImmediate,(XtPointer)XmUNSPECIFIED_POSITION},
        };

static  XmSyntheticResource     syn_constraint_resources[] =
        {
                {
                XmNentryDetail,sizeof(XmStringTable),
                XtOffset(XmContainerConstraintPtr,container.entry_detail),
                GetEntryDetail,NULL},
                {
                XmNlargeIconX,sizeof(Position),
                XtOffset(XmContainerConstraintPtr,container.large_icon_x),
                _XmFromHorizontalPixels,_XmToHorizontalPixels},
                {
                XmNlargeIconY,sizeof(Position),
                XtOffset(XmContainerConstraintPtr,container.large_icon_y),
                _XmFromVerticalPixels,_XmToVerticalPixels},
                {
                XmNsmallIconX,sizeof(Position),
                XtOffset(XmContainerConstraintPtr,container.small_icon_x),
                _XmFromHorizontalPixels,_XmToHorizontalPixels},
                {
                XmNsmallIconY,sizeof(Position),
                XtOffset(XmContainerConstraintPtr,container.small_icon_y),
                _XmFromVerticalPixels,_XmToVerticalPixels},
        };

static  String  AutomaticSelectionNames[] =
{       "auto_select", "no_auto_select"
        };
XmRepTypeId     XmRID_AUTOMATIC_SELECTION;

static  String  LineStyleNames[] =
{       "no_line", "single"
        };
XmRepTypeId     XmRID_LINE_STYLE;

static  String  EntryViewTypeNames[] =
{       "large_icon", "small_icon", "detail"
        };
XmRepTypeId     XmRID_ENTRY_VIEW_TYPE;

static  String  IncludeModelNames[] =
{       "append", "closest", "first_fit"
        };
XmRepTypeId     XmRID_INCLUDE_MODEL;

static  String  DirectionNames[] =
{       "left_to_right_top_to_bottom", "right_to_left_top_to_bottom"
        };
XmRepTypeId     XmRID_DIRECTION;

static  String  LayoutTypeNames[] =
{       "outline", "spatial"
        };
XmRepTypeId     XmRID_LAYOUT_TYPE;

static  String  OutlineStateNames[] =
{       "collapsed", "expanded"
        };
XmRepTypeId     XmRID_OUTLINE_STATE;

static  String  PlaceStyleNames[] =
{       "none", "grid"
        };
XmRepTypeId     XmRID_PLACE_STYLE;

static  String  SelectionTechniqueNames[] =
{       "marquee", "marquee_extend_start", "marquee_extend_both",
        "touch_only", "touch_over"
        };
XmRepTypeId     XmRID_SELECTION_TECHNIQUE;

/***************************/
/* Default collapsedPixmap */
/***************************/

#define xm_collapsed16_width 16
#define xm_collapsed16_height 16
static char xm_collapsed16_bits[] = {
   0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xfe, 0x7f, 0xfc, 0x3f, 0xfc, 0x3f,
   0xf8, 0x1f, 0xf8, 0x1f, 0xf0, 0x0f, 0xf0, 0x0f, 0xe0, 0x07, 0xe0, 0x07,
   0xc0, 0x03, 0xc0, 0x03, 0x80, 0x01, 0x80, 0x01};

/**************************/
/* Default expandedPixmap */
/**************************/

#define xm_expanded16_width 16
#define xm_expanded16_height 16
static char xm_expanded16_bits[] = {
   0x80, 0x01, 0x80, 0x01, 0xc0, 0x03, 0xc0, 0x03, 0xe0, 0x07, 0xe0, 0x07,
   0xf0, 0x0f, 0xf0, 0x0f, 0xf8, 0x1f, 0xf8, 0x1f, 0xfc, 0x3f, 0xfc, 0x3f,
   0xfe, 0x7f, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff};


externaldef( xmcontainerclassrec) XmContainerClassRec   xmContainerClassRec =
{       /* CoreClassPart */
        {
        (WidgetClass) &xmManagerClassRec, /* superclass */
        "XmContainer",                  /* class_name           */
        sizeof(XmContainerRec),         /* widget_size          */
        ClassInitialize,                /* class_initialize     */
        NULL,                           /* class_initialize     */
        FALSE,                          /* class_inited         */
        Initialize,                     /* initialize           */
        NULL,                           /* initialize_hook      */
        XtInheritRealize,               /* realize              */
        actionsList,                    /* actions              */
        XtNumber(actionsList),          /* num_actions          */
        resources,                      /* resources            */
        XtNumber(resources),            /* num_resources        */
        NULLQUARK,                      /* xrm_class            */
        TRUE,                           /* compress_motion      */
        XtExposeCompressMultiple,       /* compress_exposure    */
        TRUE,                           /* compress_enterleave  */
        TRUE,                           /* visible_interest     */
        Destroy,                        /* destroy              */
        Resize,                         /* resize               */
        Redisplay,                      /* expose               */
        SetValues,                      /* set_values           */
        NULL,                           /* set_values_hook      */
        XtInheritSetValuesAlmost,       /* set_values_almost    */
        NULL,                           /* get_values_hook      */
        NULL,                           /* accept_focus         */
        XtVersion,                      /* version              */
        NULL,                           /* callback private     */
        defaultTranslations,            /* tm_table             */
        QueryGeometry,                  /* query_geometry       */
        NULL,                           /* display_accelerator  */
        NULL,                           /* extension            */
        },
        /* Composite class fields                               */
        {
        GeometryManager,                /* geometry_manager     */
        ChangeManaged,                  /* change_managed       */
        XtInheritInsertChild,           /* insert_child         */
        XtInheritDeleteChild,           /* delete_child         */
        NULL,                           /* extension            */
        },
        /* Constraint class fields                              */
        {
        constraints,                    /* resource list        */
        XtNumber(constraints),          /* num resources        */
        sizeof(XmContainerConstraintRec), /* constraint size    */
        ConstraintInitialize,           /* init proc            */
        ConstraintDestroy,              /* destroy proc         */
        ConstraintSetValues,            /* set values proc      */
        NULL,                           /* extension            */
        },
        /*  XmManager class fields                              */
        {
        traversalTranslations,          /* translations         */
        syn_resources,                  /* get resources        */
        XtNumber(syn_resources),        /* num get_resources    */
        syn_constraint_resources,       /* get_cont_resources   */
        XtNumber(syn_constraint_resources), /* num_get_cont_resources */
        XmInheritParentProcess,         /* parent_process       */
        NULL,                           /* extension            */
        },
        /* Container class fields */
        {
        (XmSpatialTestFitProc)TestFitItem,      /* test_fit_item        */
        (XmSpatialPlacementProc)PlaceItem,      /* place_item           */
        (XmSpatialRemoveProc)RemoveItem,        /* remove_item          */
        (XtGeometryHandler)QuerySpatialGeometry, /* query_spatial_geometry */
        }
};

externaldef( xmcontainerwidgetclass) WidgetClass xmContainerWidgetClass
                = (WidgetClass) &xmContainerClassRec;

/*-------------
| Core methods |
-------------*/
/************************************************************************
 * ClassInitialize
 ************************************************************************/
static void
#ifdef _NO_PROTO
ClassInitialize()
#else
ClassInitialize(void)
#endif /* _NO_PROTO */
{
        XmRID_AUTOMATIC_SELECTION = XmRepTypeRegister(XmRAutomaticSelection,
                AutomaticSelectionNames,NULL,XtNumber(AutomaticSelectionNames));
        XmRID_LINE_STYLE  = XmRepTypeRegister(XmRLineStyle,
                LineStyleNames,NULL,XtNumber(LineStyleNames));
        XmRID_ENTRY_VIEW_TYPE = XmRepTypeRegister(XmREntryViewType,
                EntryViewTypeNames,NULL,XtNumber(EntryViewTypeNames));
        XmRID_INCLUDE_MODEL = XmRepTypeRegister(XmRIncludeModel,
                IncludeModelNames,NULL,XtNumber(IncludeModelNames));
        XmRID_DIRECTION = XmRepTypeRegister(XmRDirection,
                DirectionNames,NULL,XtNumber(DirectionNames));
        XmRID_LAYOUT_TYPE = XmRepTypeRegister(XmRLayoutType,
                LayoutTypeNames,NULL,XtNumber(LayoutTypeNames));
        XmRID_LINE_STYLE = XmRepTypeRegister(XmRLineStyle,
                LineStyleNames,NULL,XtNumber(LineStyleNames));
        XmRID_OUTLINE_STATE = XmRepTypeRegister(XmROutlineState,
                OutlineStateNames,NULL,XtNumber(OutlineStateNames));
        XmRID_PLACE_STYLE = XmRepTypeRegister(XmRPlaceStyle,
                PlaceStyleNames,NULL,XtNumber(PlaceStyleNames));
        XmRID_SELECTION_TECHNIQUE = XmRepTypeRegister(XmRSelectionTechnique,
                SelectionTechniqueNames,NULL,XtNumber(SelectionTechniqueNames));
}


/************************************************************************
 * Initialize
 ************************************************************************/
static  void
#ifdef _NO_PROTO
Initialize(rw,nw,args,num_args)
        Widget          rw;
        Widget          nw;
        ArgList         args;           /* unused */
        Cardinal        *num_args;      /* unused */
#else
Initialize(
        Widget          rw,
        Widget          nw,
        ArgList         args,           /* unused */
        Cardinal        *num_args)      /* unused */
#endif /* _NO_PROTO */
{
        XmContainerWidget       rcw = (XmContainerWidget)rw;
        XmContainerWidget       ncw = (XmContainerWidget)nw;

        /*
         * Verify enumerated resources.
         */
        if (!XmRepTypeValidValue(XmRID_AUTOMATIC_SELECTION,
                        ncw->container.automatic,nw))
                ncw->container.automatic = XmAUTO_SELECT;
        if (!XmRepTypeValidValue(XmRID_LINE_STYLE,
                        ncw->container.detail_sep_style,nw))
                ncw->container.detail_sep_style = XmSINGLE;
        if (!XmRepTypeValidValue(XmRID_ENTRY_VIEW_TYPE,
                        ncw->container.entry_viewtype,nw))
                ncw->container.entry_viewtype = XmLARGE_ICON;
        if (!XmRepTypeValidValue(XmRID_INCLUDE_MODEL,
                        ncw->container.include_model,nw))
                ncw->container.include_model = XmAPPEND;
        if (!XmRepTypeValidValue(XmRID_DIRECTION,
                        ncw->container.layout_direction,nw))
                ncw->container.layout_direction = XmLEFT_TO_RIGHT;
        if (!XmRepTypeValidValue(XmRID_LAYOUT_TYPE,
                        ncw->container.layout_type,nw))
                ncw->container.layout_type = XmSPATIAL;
        if (!XmRepTypeValidValue(XmRepTypeGetId("Orientation"),
                        ncw->container.orientation,nw))
                ncw->container.orientation = XmHORIZONTAL;
        if (!XmRepTypeValidValue(XmRID_LINE_STYLE,
                        ncw->container.outline_sep_style,nw))
                ncw->container.outline_sep_style = XmSINGLE;
        if (!XmRepTypeValidValue(XmRID_PLACE_STYLE,
                        ncw->container.place_style,nw))
                ncw->container.place_style = XmNONE;
        if (!XmRepTypeValidValue(XmRepTypeGetId(XmRSelectionPolicy),
                        ncw->container.selection_policy,nw))
                ncw->container.selection_policy = XmEXTENDED_SELECT;
        if (!XmRepTypeValidValue(XmRID_SELECTION_TECHNIQUE,
                        ncw->container.selection_technique,nw))
                ncw->container.selection_technique = XmTOUCH_OVER;


        /*
         * Initialize internal variables.
         */
        ncw->container.first_node = NULL;
        ncw->container.last_node = NULL;
        ncw->container.self = False;

        ncw->container.toggle_pressed = False;
        ncw->container.extend_pressed = False;
        ncw->container.cancel_pressed = False;
        ncw->container.kaddmode =
                (CtrPolicyIsMULTIPLE(ncw) || CtrPolicyIsSINGLE(ncw));
        ncw->container.marquee_mode = !(CtrTechIsTOUCH_ONLY(ncw));
        ncw->container.marquee_drawn = False;

        ncw->container.selection_state = XmSELECTED;
        ncw->container.anchor_cwid = NULL;
        ncw->container.loc_cursor_cwid = NULL;
        ncw->container.have_focus = True;
        ncw->container.have_primary = False;

        ncw->container.last_click_time = 0;


        /*
         * Get default Font.
         */
        if (ncw->container.render_table == NULL)
                {
                XmFontList      defaultFont = NULL;

                defaultFont = _XmGetDefaultFontList(nw,XmLABEL_FONTLIST);
                ncw->container.render_table = XmFontListCopy(defaultFont);
                }
        else
                ncw->container.render_table =
                                XmFontListCopy(ncw->container.render_table);

        /*
         * Initialize GCs
         */
        ncw->container.normalGC = NULL;
        ncw->container.inverseGC = NULL;
        ncw->container.marqueeGC = NULL;
        UpdateGCs(nw);

        /*
         * Get XmNexpandedStatePixmap & XmNcollapsedStatePixmap
         */
        if (ncw->container.expanded_state_pixmap == XmUNSPECIFIED_PIXMAP)
                {
                Pixmap  bitmap;

                bitmap = XCreateBitmapFromData(XtDisplay(ncw),
                        RootWindowOfScreen(XtScreen(ncw)),
                        (char *)xm_expanded16_bits,
                        xm_expanded16_width,xm_expanded16_height);
                ncw->container.expanded_state_pixmap =
                        XCreatePixmap(XtDisplay(ncw),
                                RootWindowOfScreen(XtScreen(ncw)),
                                xm_expanded16_width,xm_expanded16_height,
                                DefaultDepthOfScreen(XtScreen(ncw)));
                XCopyPlane(XtDisplay(ncw),bitmap,
                        ncw->container.expanded_state_pixmap,
                        ncw->container.normalGC,0,0,
                        xm_expanded16_width,xm_expanded16_height,0,0,1);
                XFreePixmap(XtDisplay(ncw),bitmap);
                }

        if (ncw->container.collapsed_state_pixmap == XmUNSPECIFIED_PIXMAP)
                {
                Pixmap  bitmap;

                bitmap = XCreateBitmapFromData(XtDisplay(ncw),
                        RootWindowOfScreen(XtScreen(ncw)),
                        (char *)xm_collapsed16_bits,
                        xm_collapsed16_width,xm_collapsed16_height);
                ncw->container.collapsed_state_pixmap =
                        XCreatePixmap(XtDisplay(ncw),
                                RootWindowOfScreen(XtScreen(ncw)),
                                xm_collapsed16_width,xm_collapsed16_height,
                                DefaultDepthOfScreen(XtScreen(ncw)));
                XCopyPlane(XtDisplay(ncw),bitmap,
                        ncw->container.collapsed_state_pixmap,
                        ncw->container.normalGC,0,0,
                        xm_collapsed16_width,xm_collapsed16_height,0,0,1);
                XFreePixmap(XtDisplay(ncw),bitmap);
                }

        SizeOutlineButton(nw);

        ncw->container.hspacing = SizeOneSpace(ncw->container.render_table);

        ncw->container.heading_rect.x = ncw->container.margin_w;
        ncw->container.heading_rect.y = ncw->container.margin_h;
        ncw->container.heading_rect.width = 0;
        ncw->container.detail_count_at_create = ncw->container.detail_count;
        if (ncw->container.detail_count != 0)
                {
                ncw->container.tabs = (int *)XtCalloc(
                        (ncw->container.detail_count + 1),sizeof(int));
                ncw->container.tabs[0] = ncw->container.margin_w;
                }
        else
                ncw->container.tabs = NULL;
        CreateDetailHeading(nw);

        ncw->container.max_depth = 0;
        ncw->container.outline_segs = NULL;
        ncw->container.outline_seg_count = 0;
        ncw->container.detail_segs = NULL;
        ncw->container.detail_seg_count = 0;


    /*
     * Initialize GRID/CELLS
     */
    ncw->container.cells = NULL;
    ncw->container.cell_count = 0;
    ncw->container.next_free_cell = 0;
    ncw->container.current_width_in_cells = 0;
    ncw->container.current_height_in_cells = 0;

    /*
     * Freeze width & height, if requested and if specified.
     */
    if (rcw->core.width == 0)
        ncw->container.width_frozen = False;
    else
        ncw->container.width_frozen =
                        (ncw->container.resize_width) ? False : True;
    if (rcw->core.height == 0)
        ncw->container.height_frozen = False;
    else
        ncw->container.height_frozen =
                        (ncw->container.resize_height) ? False : True;
}

/************************************************************************
 * Destroy
 ************************************************************************/
static  void
#ifdef _NO_PROTO
Destroy(wid)
        Widget  wid;
#else
Destroy(
        Widget  wid)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        int                     i;
        XmContainerConstraint   c;

        XmFontListFree(cw->container.render_table);
        XtReleaseGC(wid,cw->container.normalGC);
        XtReleaseGC(wid,cw->container.inverseGC);
        XtReleaseGC(wid,cw->container.marqueeGC);
        for (i = 0; i < cw->composite.num_children; i++)
                {
                c = GetContainerConstraint(cw->composite.children[i]);
                if (c->container_created)
                        XtDestroyWidget(cw->composite.children[i]);
                }
        if (cw->container.detail_heading != NULL)
                {
                for (i = 0; i < cw->container.detail_count; i++)
                        _XmStringFree(cw->container.detail_heading[i]);
                XtFree((XtPointer)cw->container.detail_heading);
                }
        if (cw->container.outline_segs != NULL)
                XtFree((XtPointer)cw->container.outline_segs);
        if (cw->container.detail_segs != NULL)
                XtFree((XtPointer)cw->container.detail_segs);
        if (cw->container.tabs != NULL)
                XtFree((XtPointer)cw->container.tabs);
}

/************************************************************************
 * Resize
 ************************************************************************/
static void
#ifdef _NO_PROTO
Resize(wid)
    Widget      wid;
#else
Resize(
    Widget      wid)
#endif /* _NO_PROTO */
{
    XmContainerWidget   cw = (XmContainerWidget)wid;

#ifdef  DEBUG
printf("Resize\n");
#endif
    /*
     * The one-dimensional layouts are oblivious to size constraints
     * in the Left-to-Right layout direction.
     * So, just return. Otherwise, call Layout() to adjust to new size.
     */
    if ((CtrViewIsDETAIL(cw) || CtrLayoutIsOUTLINE(cw)) &&
         (CtrLayoutIsLtoR(cw)))
        return;
    /*
     * One-dimensional Right-to_Left layouts are oblivious to
     * height changes; so, just return if the width hasn't changed.
     */
    if ((CtrViewIsDETAIL(cw) || CtrLayoutIsOUTLINE(cw)) &&
        (cw->core.width == cw->container.prev_width))
        return;
    cw->container.prev_width = cw->core.width;
    Layout(wid);
}

/************************************************************************
 * Redisplay
 ************************************************************************/
static  void
#ifdef _NO_PROTO
Redisplay(wid,event,region)
        Widget  wid;
        XEvent  *event;
        Region  region;
#else
Redisplay(
        Widget  wid,
        XEvent  *event,
        Region  region)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;

#ifdef DEBUG
printf("Redisplay\n");
#endif

        if CtrViewIsDETAIL(cw)
        /*
         * If we're in detail view, we have to render all the XmStrings in
         * the heading and the entries.
         */
         {
         int            i;
         Dimension      check_width;
         Position       check_x = 0;
         Position       ltor_x;
         CwidNode       node;
         XmContainerConstraint  c;
         GC             rect_gc,string_gc;
         Dimension      *baselines;
         int            baseline_count;
         Position       string_y;

        /*
         * Check for exposures in each detail-heading column and redraw.
         */
         if (cw->container.detail_heading != NULL)
                {
                if (cw->container.detail_count > 0)
                        check_x = cw->container.tabs[0];
                for (i = 0; i < cw->container.detail_count; i++)
                 if (ShowColumn(wid,i))
                        {
                        check_width = cw->container.tabs[i+1];
                        ltor_x = check_x;
                        if CtrLayoutIsRtoL(cw)
                                check_x = cw->core.width
                                                - check_x - check_width;
                        if (XRectInRegion(region,check_x,
                                        cw->container.heading_rect.y,
                                        check_width,
                                        cw->container.heading_rect.height)
                                                        != RectangleOut)
                                {
                                _XmStringDraw(XtDisplay(wid),XtWindow(wid),
                                        cw->container.render_table,
                                        cw->container.detail_heading[i],
                                        cw->container.normalGC,
                                        check_x,cw->container.heading_rect.y,
                                        check_width,
                                        XmALIGNMENT_BEGINNING,
                                        cw->manager.string_direction,NULL);
                                }
                        if CtrLayoutIsRtoL(cw)
                                check_x = ltor_x;
                        check_x += cw->container.tabs[i+1]
                                        + cw->container.hspacing;
                        }
                }
        /*
         * Check for exposures in each entry-detail column of each node
         * and redraw the exposed strings.
         */
         node = cw->container.first_node;
         while (node != NULL)
          {
          c = GetContainerConstraint(node->widget_ptr);
          if (c->entry_detail != NULL)
                {
                XmGadgetClassExt        *gcePtr;

                if (c->selection_visual == XmSELECTED)
                        {
                        string_gc = cw->container.inverseGC;
                        rect_gc = cw->container.normalGC;
                        }
                else
                        {
                        string_gc = cw->container.normalGC;
                        rect_gc = cw->container.inverseGC;
                        }
                string_y = c->row_y;
                /*
                 * If the first column is a Gadget, see if it has a
                 * widget_baseline method so we can line up the
                 * other columns with it.
                 */
                gcePtr = _XmGetGadgetClassExtPtr(
                            (XmGadgetClass)XtClass(node->widget_ptr),
                            NULLQUARK);
                if (*gcePtr && (*gcePtr)->widget_baseline)
                        if ((*((*gcePtr)->widget_baseline))
                                (node->widget_ptr,&baselines,
                                                &baseline_count))
                                {
                                string_y = MAX(c->row_y,
                                        baselines[baseline_count-1]
                                                - c->detail_height);
                                XtFree((XtPointer)baselines);
                                }
                check_x = cw->container.tabs[0] + cw->container.tabs[1]
                                + cw->container.hspacing;
                for (i = 1; i < cw->container.detail_count; i++)
                /*
                 * Check each column of the entry detail.
                 */
                 if (ShowColumn(wid,i))
                        {
                        check_width = cw->container.tabs[i+1];
                        ltor_x = check_x;
                        if CtrLayoutIsRtoL(cw)
                                check_x = cw->core.width
                                                - check_x - check_width;
                        if (XRectInRegion(region,check_x,c->row_y,check_width,
                                                c->row_height) != RectangleOut)
                                {
                                XFillRectangle(XtDisplay(wid),XtWindow(wid),
                                        rect_gc,check_x,c->row_y,
                                        check_width,c->row_height);
                                _XmStringDraw(XtDisplay(wid),XtWindow(wid),
                                        cw->container.render_table,
                                        c->entry_detail[i],string_gc,
                                        check_x,string_y,check_width,
                                        XmALIGNMENT_BEGINNING,
                                        cw->manager.string_direction,NULL);
                                }
                        if CtrLayoutIsRtoL(cw)
                                check_x = ltor_x;
                        check_x += cw->container.tabs[i+1]
                                        + cw->container.hspacing;
                        }
                }
                /*
                 * No sense checking for exposures if we've hit the bottom
                 * of the Window.  Otherwise, get the next node.
                 */
                if (c->row_y + c->row_height > cw->core.height)
                        break;
                node = GetNextNode(wid,node);
          }
         }

        /*
         * If lines are present & we're in detail view, draw the lines.
         */
        if (CtrDrawLinesDETAIL(cw) && (cw->container.detail_seg_count > 0))
                XDrawSegments(XtDisplay(wid),XtWindow(wid),
                        cw->container.normalGC,
                        cw->container.detail_segs,
                        cw->container.detail_seg_count);

        /*
         * If lines are present & we're in outline layout, draw the lines.
         */
        if (CtrDrawLinesOUTLINE(cw) && (cw->container.outline_seg_count > 0))
                XDrawSegments(XtDisplay(wid),XtWindow(wid),
                        cw->container.normalGC,
                        cw->container.outline_segs,
                        cw->container.outline_seg_count);

        /*
         * Redisplay all affected gadgets & Highlight the item at the
         *      location cursor.
         */
        _XmRedisplayGadgets(wid,event,region);
        BorderHighlight(cw->container.loc_cursor_cwid);
}

/************************************************************************
 * SetValues
 ************************************************************************/
static  Boolean
#ifdef _NO_PROTO
SetValues(cw,rw,nw,args,num_args)
        Widget          cw;
        Widget          rw;             /* unused */
        Widget          nw;
        ArgList         args;           /* unused */
        Cardinal        *num_args;      /* unused */
#else
SetValues(
        Widget          cw,
        Widget          rw,             /* unused */
        Widget          nw,
        ArgList         args,           /* unused */
        Cardinal        *num_args)      /* unused */
#endif /* _NO_PROTO */
{
        XmContainerWidget       ccw = (XmContainerWidget)cw;
        XmContainerWidget       ncw = (XmContainerWidget)nw;
        Boolean                 need_layout = False;
        Boolean                 need_expose = False;
        CwidNode                node;
        XmContainerConstraint   c;
        int                     i;

        /*
         * Verify enumerated resources.
         */
        if (!XmRepTypeValidValue(XmRID_AUTOMATIC_SELECTION,
                        ncw->container.automatic,nw))
                ncw->container.automatic =
                                ccw->container.automatic;
        if (!XmRepTypeValidValue(XmRID_LINE_STYLE,
                        ncw->container.detail_sep_style,nw))
                ncw->container.detail_sep_style =
                                ccw->container.detail_sep_style;
        if (!XmRepTypeValidValue(XmRID_ENTRY_VIEW_TYPE,
                        ncw->container.entry_viewtype,nw))
                ncw->container.entry_viewtype =
                                ccw->container.entry_viewtype;
        if (!XmRepTypeValidValue(XmRID_INCLUDE_MODEL,
                        ncw->container.include_model,nw))
                ncw->container.include_model =
                                ccw->container.include_model;
        if (!XmRepTypeValidValue(XmRID_DIRECTION,
                        ncw->container.layout_direction,nw))
                ncw->container.layout_direction =
                                ccw->container.layout_direction;
        if (!XmRepTypeValidValue(XmRID_LAYOUT_TYPE,
                        ncw->container.layout_type,nw))
                ncw->container.layout_type =
                                ccw->container.layout_type;
        if (!XmRepTypeValidValue(XmRepTypeGetId("XmROrientation"),
                        ncw->container.orientation,nw))
                ncw->container.orientation =
                                ccw->container.orientation;
        if (!XmRepTypeValidValue(XmRID_LINE_STYLE,
                        ncw->container.outline_sep_style,nw))
                ncw->container.outline_sep_style =
                                ccw->container.outline_sep_style;
        if (!XmRepTypeValidValue(XmRID_PLACE_STYLE,
                        ncw->container.place_style,nw))
                ncw->container.place_style =
                                ccw->container.place_style;
        if (!XmRepTypeValidValue(XmRepTypeGetId(XmRSelectionPolicy),
                        ncw->container.selection_policy,nw))
                ncw->container.selection_policy =
                                ccw->container.selection_policy;
        if (!XmRepTypeValidValue(XmRID_SELECTION_TECHNIQUE,
                        ncw->container.selection_technique,nw))
                ncw->container.selection_technique =
                        ccw->container.selection_technique;

    /*
     * Check if we'll need a new layout.
     */
    need_layout = SVNeedLayout(cw,nw);

    /*
     * Change all IconGadget children's XmNentryType to match
     * XmNentryViewType, unless we're change from/to XmDETAIL.
     */
    if ((ncw->container.entry_viewtype != ccw->container.entry_viewtype) &&
        (!CtrViewIsDETAIL(ncw)) && (!CtrViewIsDETAIL(ccw)))
        ChangeView(nw,ncw->container.entry_viewtype);

    /*
     * SetCellSizes if we need to.
     */
    if (CtrPlaceStyleIsGRID(ncw) || CtrPlaceStyleIsCELLS(ncw))
        {
        if (CtrViewIsLARGEICON(ncw) &&
            ((ncw->container.large_cell_width == ZERO_CELL_DIM) ||
             (ncw->container.large_cell_height == ZERO_CELL_DIM)))
            SetCellSize(nw);
        if (CtrViewIsSMALLICON(ncw) &&
            ((ncw->container.small_cell_width == ZERO_CELL_DIM) ||
             (ncw->container.small_cell_height == ZERO_CELL_DIM)))
            SetCellSize(nw);
        }

    /*
     * Hide all non-level-0 children if we've switched from CtrLayoutIsOUTLINE
     */
    if ((ncw->container.layout_type != ccw->container.layout_type) &&
        (CtrLayoutIsOUTLINE(ccw)))
        {
        node = ccw->container.first_node;
        while (node != NULL)
            {
            c = GetContainerConstraint(node->widget_ptr);
            if (((c->depth == 0) && (c->outline_button != NULL)) ||
                (c->depth != 0))
                HideCwid(node->widget_ptr);
            node = GetNextNode(cw,node);
            }
        }

    if ((ncw->container.collapsed_state_pixmap !=
                                ccw->container.collapsed_state_pixmap) ||
        (ncw->container.expanded_state_pixmap !=
                                ccw->container.expanded_state_pixmap))
        {
        SizeOutlineButton(nw);
        if (CtrLayoutIsOUTLINE(ncw))
            {
            need_expose = True;
            if ((ncw->container.ob_width != ccw->container.ob_width) ||
                (ncw->container.ob_height != ccw->container.ob_height))
                need_layout = True;
            }
        }

        /*
         * Invalidate "placed" constraints if Spatial Layout options
         *      have changed.
         */
        if (CtrLayoutIsSPATIAL(ncw) &&
            (ncw->container.place_style != ccw->container.place_style))
                {
                node = ncw->container.first_node;
                while (node != NULL)
                        {
                        c = GetContainerConstraint(node->widget_ptr);
                        c->cell_idx = NO_CELL;
                        if (node->child_ptr != NULL)
                                node = node->child_ptr;
                        else
                                if (node->next_ptr != NULL)
                                        node = node->next_ptr;
                                else
                                        node = GetNextUpLevelNode(node);
                        }
                }

        /*
         * Set selected items, if necessary.
         */
        if (ncw->container.selected_items !=
                                        ccw->container.selected_items)
                {
                int     save_item_count;

                DeselectAllCwids(cw);
                if (ncw->container.selected_items == NULL)
                        ncw->container.selected_item_count = 0;
                else
                        {
                        save_item_count = ncw->container.selected_item_count;
                        ncw->container.selected_item_count = 0;
                        for (i = 0; i < save_item_count; i++)
                            MarkCwid(ncw->container.selected_items[i],False);
                        }
                }

        /*
         * Change GC's and Fontlist, if necessary.
         */
        if (ncw->container.render_table != ccw->container.render_table)
                {
                XmFontListFree(ccw->container.render_table);
                ncw->container.render_table =
                                XmFontListCopy(ncw->container.render_table);
                ncw->container.hspacing = SizeOneSpace(ncw->container.render_table);
                UpdateGCs(nw);
                }
        else
                if ((ncw->manager.foreground != ccw->manager.foreground) ||
                    (ncw->core.background_pixel != ccw->core.background_pixel))
                        UpdateGCs(nw);

        /*
         * Build new tab array
         */
        if (ncw->container.detail_count != ccw->container.detail_count)
                {
                if (ncw->container.detail_count >
                                        ccw->container.detail_count_at_create)
                        {
                        ncw->container.tabs = (int *)XtRealloc
                                        ((XtPointer)ccw->container.tabs,
                                         sizeof(int) *
                                          (ncw->container.detail_count + 1));
                        for (i = ccw->container.detail_count_at_create;
                             i < ncw->container.detail_count; i++)
                                ncw->container.tabs[i+1] = 0;
                        if (ccw->container.detail_count == 0)
                                ncw->container.tabs[0] =
                                                ncw->container.margin_w;
                        }
                }

        /*
         * Rebuild heading, if necessary.
         */
        if (ncw->container.detail_heading != ccw->container.detail_heading)
                {
                if (ccw->container.detail_heading != NULL)
                        {
                        for (i = 0; i < ccw->container.detail_count; i++)
                                _XmStringFree(ccw->container.detail_heading[i]);
                        XtFree((XtPointer)ccw->container.detail_heading);
                        }
                CreateDetailHeading(nw);
                }

        /*
         * Set internal variables that are based on resource values.
         */
        if (ncw->container.margin_h != ccw->container.margin_h)
                ncw->container.heading_rect.x = ncw->container.margin_h;
        if (ncw->container.margin_w != ccw->container.margin_w)
                ncw->container.heading_rect.y = ncw->container.margin_w;

        if (ncw->container.resize_width != ccw->container.resize_width)
                ncw->container.width_frozen = !ncw->container.resize_width;
        if (ncw->container.resize_height != ccw->container.resize_height)
                ncw->container.height_frozen = !ncw->container.resize_height;

        if (ncw->container.selection_policy != ccw->container.selection_policy)
                ncw->container.kaddmode =
                        (CtrPolicyIsMULTIPLE(ncw) || CtrPolicyIsSINGLE(ncw));
        if (ncw->container.selection_technique !=
                                        ccw->container.selection_technique)
                ncw->container.marquee_mode =
                        (CtrTechIsTOUCH_ONLY(ncw) ? False : True);

        if (need_layout)
                {
                Layout(nw);
                RequestNewSize(nw);
                need_expose = True;
                }
        return(need_expose);
}

/************************************************************************
 * SVNeedLayout
 ************************************************************************/
static  Boolean
#ifdef _NO_PROTO
SVNeedLayout(cw,nw)
        Widget          cw;
        Widget          nw;
#else
SVNeedLayout(
        Widget          cw,
        Widget          nw)
#endif /* _NO_PROTO */
{
    XmContainerWidget       ccw = (XmContainerWidget)cw;
    XmContainerWidget       ncw = (XmContainerWidget)nw;

    if (ncw->container.entry_viewtype != ccw->container.entry_viewtype)
        return(True);

    if (CtrViewIsDETAIL(ncw) &&
       ((ncw->container.detail_heading != ccw->container.detail_heading) ||
        (ncw->container.detail_count != ccw->container.detail_count) ||
        (ncw->container.detail_sep_style != ccw->container.detail_sep_style) ||
        (ncw->container.render_table != ccw->container.render_table)))
        return(True);

    if (ncw->container.layout_type != ccw->container.layout_type)
        return(True);

    if (CtrLayoutIsOUTLINE(ncw) &&
       ((ncw->container.outline_indent != ccw->container.outline_indent) ||
        (ncw->container.outline_sep_style != ccw->container.outline_sep_style)))
        return(True);

    if ((ncw->container.margin_h != ccw->container.margin_h) ||
        (ncw->container.margin_w != ccw->container.margin_w) ||
        (ncw->container.layout_direction != ccw->container.layout_direction))
        return(True);

    if (CtrLayoutIsSPATIAL(ncw) &&
       ((ncw->container.place_style != ccw->container.place_style) ||
        (ncw->container.orientation != ccw->container.orientation)))
        return(True);

    if (!CtrPlaceStyleIsNONE(ncw) && CtrViewIsLARGEICON(ncw) &&
        ((ncw->container.large_cell_height !=
                                        ccw->container.large_cell_height) ||
         (ncw->container.large_cell_width !=
                                        ccw->container.large_cell_width)))
        return(True);

    if (!CtrPlaceStyleIsNONE(ncw) && CtrViewIsSMALLICON(ncw) &&
        ((ncw->container.small_cell_height !=
                                        ccw->container.small_cell_height) ||
         (ncw->container.small_cell_width !=
                                        ccw->container.small_cell_width)))
        return(True);

    return(False);
}

/************************************************************************
 * QueryGeometry
 ************************************************************************/
static  XtGeometryResult
#ifdef _NO_PROTO
QueryGeometry(wid,intended,desired)
        Widget                  wid;
        XtWidgetGeometry        *intended;
        XtWidgetGeometry        *desired;
#else
QueryGeometry(
        Widget                  wid,
        XtWidgetGeometry        *intended,
        XtWidgetGeometry        *desired)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        XtWidgetGeometry        geo_spatial;

#ifdef  DEBUG
printf("QueryGeometry\n");
#endif

        /*
         * Anything but Width & Height are fine.
         */
        if (!(intended->request_mode & CWWidth) &&
            !(intended->request_mode & CWHeight))
                return(XtGeometryYes);
        /*
         * Are width & height frozen?  Then let's just return them and
         * avoid all the calculation.
         */
        if ((cw->container.width_frozen) && (cw->container.height_frozen))
                {
                desired->width = cw->core.width;
                desired->height = cw->core.height;
                return(_XmGMReplyToQueryGeometry(wid,intended,desired));
                }
        if (CtrViewIsDETAIL(cw) || CtrLayoutIsOUTLINE(cw))
                {
                /*
                 * Ideal width & height have already been calculated
                 * by Layout().
                 */
                desired->width = cw->container.ideal_width;
                desired->height = cw->container.ideal_height;
                }
        else
                {
                /*
                 * Get width & height from class method.
                 */
                if ((*(xmContainerClassRec.container_class.
                       query_spatial_geometry))(wid,intended,&geo_spatial)
                                                        == XtGeometryYes)
                        return(XtGeometryYes);
                desired->width = geo_spatial.width;
                desired->height = geo_spatial.height;
                }
	desired->width = MAX(1,desired->width);
	desired->height = MAX(1,desired->height);
        if (cw->container.width_frozen)
                desired->width = cw->core.width;
        if (cw->container.height_frozen)
                desired->height = cw->core.height;
        return(_XmGMReplyToQueryGeometry(wid,intended,desired));
}


/*------------------
| Composite methods |
------------------*/
/************************************************************************
 * GeometryManager
 ************************************************************************/
static XtGeometryResult
#ifdef _NO_PROTO
GeometryManager(instigator,desired,allowed)
        Widget                  instigator;
        XtWidgetGeometry        *desired;
        XtWidgetGeometry        *allowed;
#else
GeometryManager(
        Widget                  instigator,
        XtWidgetGeometry        *desired,
        XtWidgetGeometry        *allowed)
#endif /* _NO_PROTO */
{
        XmGadget        g = (XmGadget)instigator;

        /*
         * pre-load x,y, width & height (same offsets for Widgets & Gadgets)
         */
        allowed->request_mode = 0;
        allowed->x = g->rectangle.x;
        allowed->y = g->rectangle.y;
        allowed->width = g->rectangle.width;
        allowed->height = g->rectangle.height;
        if (desired->request_mode && CWWidth)
                {
                allowed->request_mode |= CWWidth;
                allowed->width = desired->width;
                }
        if (desired->request_mode && CWHeight)
                {
                allowed->request_mode |= CWHeight;
                allowed->height = desired->height;
                }
        if (allowed->request_mode != 0)
                {
                _XmConfigureObject(instigator,allowed->x,allowed->y,
                                allowed->width,allowed->height,0);
                return(XtGeometryDone);
                }
        else
                return(XtGeometryNo);
}

/************************************************************************
 * ChangeManaged
 ************************************************************************/
static void
#ifdef _NO_PROTO
ChangeManaged(wid)
        Widget  wid;
#else
ChangeManaged(
        Widget  wid)
#endif /* _NO_PROTO */
{
    XmContainerWidget   cw = (XmContainerWidget)wid;
    Boolean             changes = False;
    int                 i;
    Widget              cwid;
    XmContainerConstraint c;
    CwidNode            node;
    XtWidgetGeometry    geo_current;
    XtWidgetGeometry    geo_desired;
    XtGeometryResult    geo_response = XtGeometryYes;
    XtGeometryResult    pwid_geo_response = XtGeometryYes;

#ifdef  DEBUG
printf("ChangeManaged\n");
#endif
    for (i = 0; i < cw->composite.num_children; i++)
        {
        cwid = cw->composite.children[i];
        c = GetContainerConstraint(cwid);
        if (c->was_managed != XtIsManaged(cwid))
            {
            c->was_managed = XtIsManaged(cwid);
            /*
             * A cwid's been changed, does it affect Layout?
             */
            if (c->container_created)
                {
                /*
                 * Is it a new OutlineButton attached to
                 * a visible item?
                 */
                node = c->node_ptr;
                if ((node) && (node->widget_ptr))
                        {
                        c = GetContainerConstraint(node->widget_ptr);
                        if ((c) && (c->visible_in_outline))
                                changes = True;
                        }
                }
                else
                    /* not an OutlineButton, must be a cwid */
                    {
                    if (XtIsManaged(cwid) &&
                        CtrLayoutIsOUTLINE(wid) &&
                        (!c->visible_in_outline))
                        /*
                         * Don't let newly managed children
                         * pop up in Outline if they're
                         * supposed to be hidden.
                         */
                        HideCwid(cwid);
                if ((c->entry_parent == NULL) ||
                    ((c->visible_in_outline) && CtrLayoutIsOUTLINE(wid)))
                    /*
                     * It's a child that should be shown
                     * or removed; Layout is affected.
                     */
                    changes = True;
                if XtIsManaged(cwid)
                    /*
                     * Put it into the linked
                     * list of nodes.  Layout()
                     * will call the "place-item"
                     * method to position it later.
                     */
                    InsertNode(c->node_ptr);
                else
                    /*
                     * Remove it from the linked
                     * list of nodes.  And remove
                     * it from any Spatial layout
                     * representation.
                     */
                    {
                    SeverNode(c->node_ptr);
                    (*(xmContainerClassRec.container_class.remove_item))
                        (wid,cwid);
                    }
                }
            }
        }
    if (changes && (CtrLayoutIsOUTLINE(cw) || CtrViewIsDETAIL(cw)))
        /*
         * Call Layout() for one-dimensional
         * layout types so ideal_width/height are updated before
         * the call to QueryGeometry() routine.
         */
        Layout(wid);

    if (!XtIsRealized(cw) &&
        (CtrPlaceStyleIsGRID(cw) || CtrPlaceStyleIsCELLS(cw)))
        /*
         * First time through, let's set the Cell sizes if the user hasn't.
         */
        SetCellSize(wid);
    if (changes)
        {
        geo_current.request_mode = CWWidth | CWHeight;
        geo_current.width = cw->core.width;
        geo_current.height = cw->core.height;
        geo_response = (*(xmContainerClassRec.core_class.query_geometry))
                        (wid,&geo_current,&geo_desired);
#ifdef  DEBUG
                printf("returned width  = %d\n",geo_desired.width);
                printf("returned height = %d\n",geo_desired.height);
#endif
        if (geo_response != XtGeometryYes)
            pwid_geo_response = _XmMakeGeometryRequest(wid,&geo_desired);
#ifdef  DEBUG
        if (geo_response != XtGeometryYes)
            {
            if (pwid_geo_response == XtGeometryYes)
                  printf("_XmMakeGeometryRequest returned XtGeometryYes\n");
            else
                  printf("_XmMakeGeometryRequest returned XtGeometryNo\n");
            }
#endif

        }

    /*
     * If it's the first time through ChangeManaged,
     * and XmNresizeWidth/Height is set to False,
     * and the width/height wasn't specified and frozen
     *  at Initialize() time, then freeze it now.
     */
    if (!XtIsRealized(cw))
        {
        if ((!cw->container.width_frozen) && (!cw->container.resize_width))
            cw->container.width_frozen = True;
        if ((!cw->container.height_frozen) && (!cw->container.resize_height))
            cw->container.height_frozen = True;
        /*
         * Set the first item as the "keyboard focus".
         */
        cw->container.loc_cursor_cwid = cw->container.first_node->widget_ptr;
        }

    /*
     * No changes, no new Layout() required.
     */
    if (!changes)
        return;

    /*
     * The one-dimensional layouts are oblivious to any size constraints
     * in the Left-to-Right layout direction.
     *  One-dimensional Right-to_Left layouts are sensitive to width changes.
     */
    if (CtrViewIsDETAIL(cw) || CtrLayoutIsOUTLINE(cw))
        {
        if (CtrLayoutIsLtoR(cw) || (cw->core.width == cw->container.prev_width))
            return;
        cw->container.prev_width = cw->core.width;
        }

    /*
     * There are changes that affect layout, so do it.  But, don't request
     * a resize - _XmMakeGeometryRequest said no above and we've been
     * synchronous since then, so it will just say no again.
     */
    Layout(wid);
}

/*-------------------
| Constraint methods |
-------------------*/
/************************************************************************
 * ConstraintInitialize
 ************************************************************************/
static void
#ifdef _NO_PROTO
ConstraintInitialize(rcwid,ncwid,args,num_args)
        Widget          rcwid;          /* unused */
        Widget          ncwid;
        ArgList         args;           /* unused */
        Cardinal        *num_args;      /* unused */
#else
ConstraintInitialize(
        Widget          rcwid,          /* unused */
        Widget          ncwid,
        ArgList         args,           /* unused */
        Cardinal        *num_args)      /* unused */
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)XtParent(ncwid);
        XmContainerConstraint   nc = GetContainerConstraint(ncwid);
        XmContainerConstraint   pc;     /* parent's constraints */
        nc->was_managed = False;
        nc->node_ptr = NULL;
        nc->container_created = cw->container.self;
        if (nc->container_created) return;

        /*
         * validate resource values
         */
        if (!XmRepTypeValidValue(XmRID_OUTLINE_STATE,nc->outline_state,ncwid))
                nc->outline_state = XmCOLLAPSED;
        if (nc->position_index != UNSPECIFIED_POSITION_INDEX)
                nc->position_index = MAX(0,nc->position_index);

        /*
         * Create new node for child - ChangeManaged will insert into list.
         */
        (void) NewNode(ncwid);

#ifdef  COSEICON
        nc->selection_visual = (_DtIconGetState(ncwid)) ?
                        XmSELECTED : XmNOT_SELECTED;
#else
        nc->selection_visual = XmNOT_SELECTED;
        XtVaGetValues(ncwid,XmNvisualEmphasis,&nc->selection_visual,NULL);
#endif  /* COSEICON */
        if (nc->selection_visual == XmSELECTED)
                {
                cw->container.selected_item_count++;
                if (cw->container.anchor_cwid == NULL)
                        cw->container.anchor_cwid = ncwid;
                }
        nc->selection_state = nc->selection_visual;
        nc->outline_button = NULL;
        if (nc->entry_parent != NULL)
                {
                pc = GetContainerConstraint(nc->entry_parent);
                nc->depth = pc->depth +1;
                cw->container.max_depth = MAX(cw->container.max_depth,
                                                nc->depth);
                if (pc->outline_state == XmEXPANDED)
                        nc->visible_in_outline = pc->visible_in_outline;
                else
                        nc->visible_in_outline = False;
                if ((!nc->visible_in_outline) || (!CtrLayoutIsOUTLINE(cw)))
                        HideCwid(ncwid);
                /*
                 * Check to see if we've created a new parent
                 */
                if (pc->outline_button == NULL)
                        {
                        MakeOutlineButton(nc->entry_parent);
                        if ((!pc->visible_in_outline) || (!CtrLayoutIsOUTLINE(cw)))
                                HideCwid(pc->outline_button);
                        }
                }
        else
                {
                nc->visible_in_outline = True;
                nc->depth = 0;
                }
        nc->outline_button = NULL;
        nc->was_managed = False;

        CreateEntryDetail(ncwid);

        /*
         * Initialize variables used by
         *      Container SpatialLayout methods.
         */
        nc->cell_idx = NO_CELL;
}

/************************************************************************
 * ConstraintDestroy
 ************************************************************************/
static  void
#ifdef _NO_PROTO
ConstraintDestroy(cwid)
        Widget  cwid;
#else
ConstraintDestroy(
        Widget  cwid)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)XtParent(cwid);
        XmContainerConstraint   c = GetContainerConstraint(cwid);

        if (cwid == cw->container.loc_cursor_cwid)
                cw->container.loc_cursor_cwid = NULL;
        if (cwid == cw->container.anchor_cwid)
                cw->container.anchor_cwid = NULL;
        if (c->container_created) return;

        DeleteNode(cwid);
        if (c->entry_detail != NULL)
                {
                int     i;

                for (i = 0; i < c->entry_detail_count; i++)
                        _XmStringFree(c->entry_detail[i]);
                XtFree((XtPointer)c->entry_detail);
                }
        if (c->selection_state == XmSELECTED)
                {
                unsigned char   save_state = cw->container.selection_state;

                cw->container.selection_state = XmNOT_SELECTED;
                MarkCwid(cwid,False);
                cw->container.selection_state = save_state;
                }
}

/************************************************************************
 * ConstraintSetValues
 ************************************************************************/
static Boolean
#ifdef _NO_PROTO
ConstraintSetValues(ccwid,rcwid,ncwid,args,num_args)
        Widget          ccwid;
        Widget          rcwid;          /* unused */
        Widget          ncwid;
        ArgList         args;           /* unused */
        Cardinal        *num_args;      /* unused */
#else
ConstraintSetValues(
        Widget          ccwid,
        Widget          rcwid,          /* unused */
        Widget          ncwid,
        ArgList         args,           /* unused */
        Cardinal        *num_args)      /* unused */
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)XtParent(ncwid);
        XmContainerConstraint   cc = GetContainerConstraint(ccwid);
        XmContainerConstraint   nc = GetContainerConstraint(ncwid);
        XmContainerConstraint   pc;     /* Parent's Contraints */
        Boolean                 need_layout = False;
        Boolean                 need_expose = False;

        if (cw->container.self) return(False);
        if (nc->container_created) return(False);

#ifdef  DEBUG
printf("ConstraintSetValues\n");
#endif

        /*
         * Validate resource values
         */
        if (!XmRepTypeValidValue(XmRID_OUTLINE_STATE,nc->outline_state,ncwid))
                nc->outline_state = cc->outline_state;
        if (nc->position_index < 0)
                nc->position_index = cc->position_index;

        /*
         * Check if layout required.
         */
        if (nc->position_index != cc->position_index)
                need_layout = True;
        if (CtrViewIsDETAIL(cw) && (nc->entry_detail != cc->entry_detail))
                need_layout = True;
        if (CtrLayoutIsOUTLINE(cw) &&
            ((nc->entry_parent != cc->entry_parent) ||
             (nc->outline_state != cc->outline_state)))
                need_layout = True;

        /*
         * Check for change in selection status
         */
#ifdef  COSEICON
        nc->selection_visual = (_DtIconGetState(ncwid)) ?
                XmSELECTED : XmNOT_SELECTED;
#else
        XtVaGetValues(ncwid,XmNvisualEmphasis,&nc->selection_visual,NULL);
#endif  /* COSEICON */
        if (nc->selection_visual != cc->selection_visual)
                {
                if (nc->selection_visual == XmSELECTED)
                        {
                        cw->container.selected_item_count++;
                        if (cw->container.anchor_cwid == NULL)
                                cw->container.anchor_cwid = ncwid;
                        }
                else
                        cw->container.selected_item_count--;
                nc->selection_state = nc->selection_visual;
                }

        /*
         * Changes in parentage or position.
         * If XmNpositionIndex has changed, but the "node" has no siblings,
         *      then there's no need to change the node's position in the
         *      linked list.
         */
        if ((nc->entry_parent != cc->entry_parent) ||
            ((nc->position_index != cc->position_index) &&
             ((nc->node_ptr->prev_ptr != NULL) ||
              (nc->node_ptr->next_ptr != NULL))))
                {
                SeverNode(nc->node_ptr);
                InsertNode(nc->node_ptr);
                }
        if (nc->entry_parent != cc->entry_parent)
                if (nc->entry_parent == NULL)
                        {
                        nc->depth = 0;
                        nc->visible_in_outline = True;
                        }
                else
                        {
                        pc = GetContainerConstraint(nc->entry_parent);
                        nc->depth = pc->depth +1;
                        cw->container.max_depth = MAX(cw->container.max_depth,
                                                nc->depth);
                        if (pc->outline_state == XmEXPANDED)
                                nc->visible_in_outline = pc->visible_in_outline;
                        else
                                nc->visible_in_outline = False;
                        if ((!nc->visible_in_outline) || (!CtrLayoutIsOUTLINE(cw)))
                                HideCwid(ncwid);
                        /*
                         * Check to see if we've created a new parent
                         */
                        if (pc->outline_button == NULL)
                                {
                                MakeOutlineButton(nc->entry_parent);
                                if ((!pc->visible_in_outline) ||
                                    (!CtrLayoutIsOUTLINE(cw)))
                                        HideCwid(pc->outline_button);
                                }
                        }

        /*
         * Change in entry_detail
         */
        if (nc->entry_detail != cc->entry_detail)
                {
                if (cc->entry_detail != NULL)
                        {
                        int     i;

                        for (i = 0; i < cc->entry_detail_count; i++)
                                _XmStringFree(cc->entry_detail[i]);
                        XtFree((XtPointer)cc->entry_detail);
                        }
                CreateEntryDetail(ncwid);
                }

        /*
         * Change Spatial locations.  May need a new Layout if
         *      items can't be placed.
         */
        if ((CtrViewIsLARGEICON(cw) &&
             ((nc->large_icon_x != cc->large_icon_x) ||
              (nc->large_icon_y != cc->large_icon_y)))
                ||
            (CtrViewIsSMALLICON(cw) &&
             ((nc->small_icon_x != cc->small_icon_x) ||
              (nc->small_icon_y != cc->small_icon_y))))
                (*(xmContainerClassRec.container_class.remove_item))
                        ((Widget)cw,ncwid);
        if (CtrLayoutIsSPATIAL(cw) &&
            ((nc->large_icon_x != cc->large_icon_x) ||
             (nc->large_icon_y != cc->large_icon_y) ||
             (nc->small_icon_x != cc->small_icon_x) ||
             (nc->small_icon_y != cc->small_icon_y)))
                need_layout &=
                  !(*(xmContainerClassRec.container_class.place_item))
                        ((Widget)cw,ncwid);

        if (need_layout)
                {
                Layout((Widget)cw);
                RequestNewSize((Widget)cw);
                need_expose = True;
                }
        return(need_expose);
}

/*----------------
| Manager methods |
----------------*/
        /* None */
/*------------------
| Container methods |
------------------*/
/************************************************************************
 * TestFitItem
 ************************************************************************/
static  Boolean
#ifdef _NO_PROTO
TestFitItem(wid,cwid,x,y)
    Widget      wid;
    Widget      cwid;
    Position    x;
    Position    y;
#else
TestFitItem(
    Widget      wid,
    Widget      cwid,
    Position    x,
    Position    y)
#endif  /* _NO_PROTO */
{
    XmContainerWidget   cw = (XmContainerWidget)wid;
    int trial_cell;

#ifdef  DEBUG
printf("TestFitItem\n");
#endif  /* DEBUG */
    /*
     * Must be a 2-D layout.
     */
    if (CtrViewIsDETAIL(cw) || CtrLayoutIsOUTLINE(cw))
        return(True);
    if ((x < (Position)cw->container.margin_w) ||
        (y < (Position)cw->container.margin_h) ||
        (x > (Position)(cw->core.width - cw->container.margin_w)) ||
        (y > (Position)(cw->core.height - cw->container.margin_h)))
        return(False);

    if (CtrPlaceStyleIsGRID(cw) || CtrPlaceStyleIsCELLS(cw))
        {
        trial_cell = GetCellFromCoord(wid,x - cw->container.margin_w,
                                        y - cw->container.margin_h);
#ifdef  DEBUG
printf("trial_cell is %d\n",trial_cell);
#endif  /* DEBUG */
        if (cw->container.cells[trial_cell] != NULL)
            return(False);
        }
    return(True);
}

/************************************************************************
 * PlaceItem
 ************************************************************************/
static  Boolean
#ifdef _NO_PROTO
PlaceItem(wid,cwid)
    Widget      wid;
    Widget      cwid;
#else
PlaceItem(
    Widget      wid,
    Widget      cwid)
#endif /* _NO_PROTO */
{
    XmContainerWidget   cw = (XmContainerWidget)wid;

#ifdef DEBUG
printf("PlaceItem\n");
#endif
    /*
     * No-op if not Spatial layout.
     */
    if (CtrViewIsDETAIL(cw) || CtrLayoutIsOUTLINE(cw))
        return(True);

    if (cwid == NULL)
        {
        PlaceItemReset(wid);
        return(True);
        }

    if (CtrItemIsPlaced(cwid))
        return(True);

    switch(cw->container.place_style)
    {
    case XmNONE:
        PlaceItemNone(wid,cwid);
        break;
    case XmGRID:
        PlaceItemGrid(wid,cwid);
        break;
    case XmCELLS:
        PlaceItemCells(wid,cwid);
    }
    if (!CtrItemIsPlaced(cwid))
        HideCwid(cwid);
    return(CtrItemIsPlaced(cwid));
}

/************************************************************************
 * RemoveItem
 ************************************************************************/
static  Boolean
#ifdef _NO_PROTO
RemoveItem(wid,cwid)
    Widget          wid;
    Widget          cwid;
#else
RemoveItem(
    Widget          wid,
    Widget          cwid)
#endif /* _NO_PROTO */
{
    XmContainerWidget   cw = (XmContainerWidget)wid;
    XmContainerConstraint c = GetContainerConstraint(cwid);

    if (!CtrItemIsPlaced(cwid))
        return(True);
    switch(cw->container.place_style)
    {
    case XmCELLS:
    case XmGRID:
        cw->container.cells[c->cell_idx] = NULL;
    case XmNONE:
        c->cell_idx = NO_CELL;
    }
    return(True);
}

/************************************************************************
 * QuerySpatialGeometry
 ************************************************************************/
static  XtGeometryResult
#ifdef _NO_PROTO
QuerySpatialGeometry(wid,intended,desired)
    Widget              wid;
    XtWidgetGeometry    *intended;      /* unused */
    XtWidgetGeometry    *desired;
#else
QuerySpatialGeometry(
    Widget              wid,
    XtWidgetGeometry    *intended,      /* unused */
    XtWidgetGeometry    *desired)
#endif /* _NO_PROTO */
{
    XmContainerWidget   cw = (XmContainerWidget)wid;
    CwidNode            node;
    XmContainerConstraint c;
    XPoint              spot;
    Dimension           trial_width = 0, trial_height = 0;
    Dimension           cell_width = 0,cell_height = 0;
    int                 width_in_cells = 1,height_in_cells = 1;
    int                 min_width_in_cells = 1,min_height_in_cells = 1;
    int                 cwid_width_in_cells,cwid_height_in_cells;
    int                 cell_count = 0;
    Dimension           marginless_width,marginless_height;
    XtGeometryResult    answer;

#ifdef  DEBUG
printf("QuerySpatialGeometry\n");
#endif  /* DEBUG */

    desired->request_mode = CWWidth & CWHeight;
    desired->width = desired->height = 0;
    if (CtrPlaceStyleIsGRID(cw) || CtrPlaceStyleIsCELLS(cw))
        {
        if (CtrViewIsLARGEICON(cw))
            {
            cell_width = cw->container.large_cell_width;
            cell_height = cw->container.large_cell_height;
            }
        else
            {
            cell_width = cw->container.small_cell_width;
            cell_height = cw->container.small_cell_height;
            }
        }
    /*
     * Go through the linked list of cwids ands let determine
     * their geometry needs.
     */
    node = cw->container.first_node;
    while (node != NULL)
        {
        if (CtrPlaceStyleIsNONE(cw) || CtrIncludeIsCLOSEST(cw))
            {
            /*
             * In this case, we care about the desired x,y coordinates of
             * each child, so let's compute the smallest rectangle that
             * would contain all of them and still fulfill their desires:
             *     (trial_width X trial_height)
             */
            if (CtrItemIsPlaced(node->widget_ptr))
                {
                spot.x = ((Widget)node->widget_ptr)->core.x;
                spot.y = ((Widget)node->widget_ptr)->core.y;
                }
            else
                {
                c = GetContainerConstraint(node->widget_ptr);
                if (CtrViewIsLARGEICON(cw))
                    {
                    spot.x = (c->large_icon_x == XmUNSPECIFIED_POSITION) ?
                                0 : c->large_icon_x;
                    spot.y = (c->large_icon_y == XmUNSPECIFIED_POSITION) ?
                                0 : c->large_icon_y;
                    }
                else
                    {
                    spot.x = (c->small_icon_x == XmUNSPECIFIED_POSITION) ?
                                0 : c->small_icon_x;
                    spot.y = (c->small_icon_y == XmUNSPECIFIED_POSITION) ?
                                0 : c->small_icon_y;
                    }
                spot.x += cw->container.margin_w;
                spot.y += cw->container.margin_h;
                }
            trial_width = MAX(trial_width,
                        spot.x + ((Widget)node->widget_ptr)->core.width);
            trial_height = MAX(trial_height,
                        spot.y + ((Widget)node->widget_ptr)->core.height);
            }
            node = GetNextNode(wid,node);
            if (CtrPlaceStyleIsGRID(cw))
                /*
                 * CtrPlaceStyleIsGRID: One cell per cwid
                 */
                cell_count++;
            if (CtrPlaceStyleIsCELLS(cw))
                /*
                 * CtrPlaceStyleIsCELLS: Need enough cells to completely
                 *                              contain all cwids.
                 */
                {
                cwid_width_in_cells = ((Widget)node->widget_ptr)->core.width
                                        / cell_width;
                if (((Widget)node->widget_ptr)->core.width % cell_width)
                        cwid_width_in_cells++;
                min_width_in_cells = MAX(min_width_in_cells,
                                                cwid_width_in_cells);
                cwid_height_in_cells = ((Widget)node->widget_ptr)->core.height
                                        / cell_height;
                if (((Widget)node->widget_ptr)->core.height % cell_height)
                        cwid_height_in_cells++;
                min_height_in_cells = MAX(min_height_in_cells,
                                                cwid_height_in_cells);
                cell_count += cwid_width_in_cells + cwid_height_in_cells;
                }
        }
    if (CtrPlaceStyleIsNONE(cw) || CtrIncludeIsCLOSEST(cw))
        {
        trial_width += cw->container.margin_w;
        trial_height += cw->container.margin_h;
        }
    if (CtrPlaceStyleIsNONE(cw))
        {
        desired->width = trial_width;
        desired->height = trial_height;
        }
    else        /* CtrPlaceStyleIsGRID || CtrPlaceStyleIsCELLS */
        {
        /*
         * Note: Either Width or Height might be fixed by the user.  If both
         * were fixed, then QueryGeometry would not have invoked us.
         */
        if (cw->container.width_frozen)
            {
            /*
             * Width is fixed.  So, how much height do we need?
             */
            marginless_width = cw->core.width - 2 * cw->container.margin_w;
            width_in_cells = marginless_width / cell_width;
            if (marginless_width % cell_width)
                width_in_cells++;
            height_in_cells = cell_count / width_in_cells;
            if (cell_count % width_in_cells)
                height_in_cells++;
            }
        if (cw->container.height_frozen)
            {
            /*
             * Height is fixed.  So, how much width do we need?
             */
            marginless_height = cw->core.height - 2 * cw->container.margin_h;
            height_in_cells = marginless_height / cell_height;
            if (marginless_height % cell_height)
                height_in_cells++;
            width_in_cells = cell_count / height_in_cells;
            if (cell_count % height_in_cells)
                width_in_cells++;
            }
        if (!cw->container.width_frozen && !cw->container.height_frozen)
            /*
             * Nothing frozen.  Simple square will do.
             */
            {
            width_in_cells = Isqrt(cell_count);
            height_in_cells = width_in_cells;
            }
        if (CtrPlaceStyleIsCELLS(cw))
            /*
             * Make adjustments to accomodate very wide or tall cwids.
             */
            {
            width_in_cells = MAX(width_in_cells,min_width_in_cells);
            height_in_cells = MAX(height_in_cells,min_height_in_cells);
            }
        /*
         * Add in the margins and calculate the pixels needed.
         */
        desired->width = width_in_cells * cell_width
                                + 2 * cw->container.margin_w;
        desired->height = height_in_cells * cell_height
                                + 2 * cw->container.margin_h;
        if (CtrIncludeIsCLOSEST(cw))
            /*
             * Make adjustments to accomodate cwids' desired x,y placement.
             */
            {
            desired->width = MAX(desired->width,trial_width);
            desired->height = MAX(desired->height,trial_height);
            }
        }
    desired->width = MAX(1, desired->width);
    desired->height = MAX(1, desired->height);
    if (cw->container.width_frozen)
        desired->width = cw->core.width;
    if (cw->container.height_frozen)
        desired->height = cw->core.height;
    answer = XtGeometryYes;
    if ((intended->request_mode && CWWidth) &&
        (intended->width < desired->width))
        answer = XtGeometryNo;
    if ((intended->request_mode && CWHeight) &&
        (intended->height < desired->height))
        answer = XtGeometryNo;
    return(answer);
}

/*---------------------------------------------
| private functions used by Container methods |
---------------------------------------------*/
/************************************************************************
 * PlaceItemNone
 ************************************************************************/
static  void
#ifdef _NO_PROTO
PlaceItemNone(wid,cwid)
    Widget      wid;
    Widget      cwid;
#else
PlaceItemNone(
    Widget      wid,
    Widget      cwid)
#endif /* _NO_PROTO */
{
    XmContainerWidget           cw = (XmContainerWidget)wid;
    XmContainerConstraint       c = GetContainerConstraint(cwid);
    XPoint                      spot;

    if (CtrViewIsLARGEICON(cw))
        {
        spot.x = (c->large_icon_x == XmUNSPECIFIED_POSITION) ?
                        0 : c->large_icon_x;
        spot.y = (c->large_icon_y == XmUNSPECIFIED_POSITION) ?
                        0 : c->large_icon_y;
        }
    else
        {
        spot.x = (c->small_icon_x == XmUNSPECIFIED_POSITION) ?
                        0 : c->small_icon_x;
        spot.y = (c->small_icon_y == XmUNSPECIFIED_POSITION) ?
                        0 : c->small_icon_y;
        }
    PlaceCwid(cwid,spot.x,spot.y);

    /*
     * Mark the cwid as placed (any value except NO_CELL)
     */
    c->cell_idx = 1;
}

/************************************************************************
 * PlaceItemGrid
 ************************************************************************/
static  void
#ifdef _NO_PROTO
PlaceItemGrid(wid,cwid)
    Widget          wid;
    Widget          cwid;
#else
PlaceItemGrid(
    Widget          wid,
    Widget          cwid)
#endif /* _NO_PROTO */
{
    XmContainerWidget   cw = (XmContainerWidget)wid;
    XmContainerConstraint c = GetContainerConstraint(cwid);
    int trial_cell = 0;
    Position    trial_x,trial_y;
    XPoint *    cell_coord;

    if (CtrIncludeIsAPPEND(cw))
        trial_cell = cw->container.next_free_cell;
    if (CtrIncludeIsCLOSEST(cw))
        {
        trial_x = (CtrViewIsLARGEICON(cw)) ? c->large_icon_x : c->small_icon_x;
        trial_y = (CtrViewIsLARGEICON(cw)) ? c->large_icon_y : c->small_icon_y;
        trial_cell = GetCellFromCoord(wid,trial_x,trial_y);
        if (trial_cell >= cw->container.cell_count)
            {
            if (cw->container.cells[cw->container.cell_count - 1] == NULL)
                trial_cell = cw->container.cell_count - 1;
            else
                trial_cell = cw->container.next_free_cell;
            }
        }
    while ((trial_cell < cw->container.cell_count) && !CtrItemIsPlaced(cwid))
        {
        if (cw->container.cells[trial_cell] == NULL)
                {
                cw->container.cells[trial_cell] = cwid;
                c->cell_idx = trial_cell;
                }
        trial_cell++;
        }
    if (!CtrItemIsPlaced(cwid))
        return;
    if (CtrIncludeIsAPPEND(cw))
        cw->container.next_free_cell = c->cell_idx;
    cell_coord = GetCoordFromCell(wid,c->cell_idx);
    if (CtrViewIsLARGEICON(cw))
        {
        c->large_icon_x = cell_coord->x;
        c->large_icon_y = cell_coord->y;
        }
    else
        {
        c->small_icon_x = cell_coord->x;
        c->small_icon_y = cell_coord->y;
        }
    PlaceCwid(cwid,cell_coord->x,cell_coord->y);
#ifdef  DEBUG
printf("cell_idx is %d\n",c->cell_idx);
#endif  /* DEBUG */
}

/************************************************************************
 * GetCellFromCoord
 ************************************************************************/
static  int
#ifdef _NO_PROTO
GetCellFromCoord(wid,x,y)
    Widget      wid;
    Position    x;
    Position    y;
#else
GetCellFromCoord(
    Widget      wid,
    Position    x,
    Position    y)
#endif /* _NO_PROTO */
{
    XmContainerWidget   cw = (XmContainerWidget)wid;
    int cell_width,cell_height;
    int row,col;

    cell_width = (CtrViewIsLARGEICON(cw)) ?
        cw->container.large_cell_width : cw->container.small_cell_width;
    cell_height = (CtrViewIsLARGEICON(cw)) ?
        cw->container.large_cell_height : cw->container.small_cell_height;
    /*
     * Note: x & y parameters are assumed not to include Container margins.
     */
    row = y / cell_height;
    col = x / cell_width;
    if (CtrLayoutIsRtoL(cw))
        col = cw->container.current_width_in_cells - col - 1;
    if (CtrIsHORIZONTAL(cw))
        return(cw->container.current_width_in_cells * row + col);
    else
        return(cw->container.current_height_in_cells * col + row);
}

/************************************************************************
 * GetCoordFromCell
 ************************************************************************/
static  XPoint *
#ifdef _NO_PROTO
GetCoordFromCell(wid,cell_idx)
    Widget      wid;
    int         cell_idx;
#else
GetCoordFromCell(
    Widget      wid,
    int         cell_idx)
#endif /* _NO_PROTO */
{
    XmContainerWidget   cw = (XmContainerWidget)wid;
    int cell_width,cell_height;
    int row,col;
    static XPoint       point;

    if (CtrIsHORIZONTAL(cw))
        {
        row = cell_idx / cw->container.current_width_in_cells;
        col = cell_idx - (row * cw->container.current_width_in_cells);
        }
    else
        {
        col = cell_idx / cw->container.current_height_in_cells;
        row = cell_idx - (col * cw->container.current_height_in_cells);
        }
    if (CtrLayoutIsRtoL(cw))
        col = cw->container.current_width_in_cells - col - 1;
    cell_width = (CtrViewIsLARGEICON(cw)) ?
        cw->container.large_cell_width : cw->container.small_cell_width;
    cell_height = (CtrViewIsLARGEICON(cw)) ?
        cw->container.large_cell_height : cw->container.small_cell_height;
    /*
     * Note: returned point does not include Container Margins
     */
    point.x = col * cell_width;
    point.y = row * cell_height;
    return(&point);
}

/************************************************************************
 * PlaceItemCells
 ************************************************************************/
static  void
#ifdef _NO_PROTO
PlaceItemCells(wid,cwid)
    Widget          wid;
    Widget          cwid;
#else
PlaceItemCells(
    Widget          wid,
    Widget          cwid)
#endif /* _NO_PROTO */
{
    PlaceItemGrid(wid,cwid);
}

/************************************************************************
 * PlaceItemReset
 ************************************************************************/
static  void
#ifdef _NO_PROTO
PlaceItemReset(wid)
    Widget          wid;
#else
PlaceItemReset(
    Widget          wid)
#endif /* _NO_PROTO */
{
    XmContainerWidget   cw = (XmContainerWidget)wid;
    int i;
    int cell_width = 0,cell_height = 0;
    int width_in_cells,height_in_cells;

    /*
     * Deallocate old cell structure.
     */
    if (cw->container.cells != NULL)
        {
        for (i = 0; i < cw->container.cell_count; i++)
            if (cw->container.cells[i] != NULL)
                (*(xmContainerClassRec.container_class.remove_item))
                        (wid,cw->container.cells[i]);
        XtFree((XtPointer)cw->container.cells);
        }
    if (CtrPlaceStyleIsNONE(cw))
        return;
    /*
     * Create new cell structure.
     */
    cell_width = (CtrViewIsLARGEICON(cw)) ?
        cw->container.large_cell_width : cw->container.small_cell_width;
    cell_height = (CtrViewIsLARGEICON(cw)) ?
        cw->container.large_cell_height : cw->container.small_cell_height;
    width_in_cells = (cw->core.width - 2 * cw->container.margin_w)
                        / cell_width;
    height_in_cells = (cw->core.height - 2 * cw->container.margin_h)
                        / cell_height;
    cw->container.cell_count = width_in_cells * height_in_cells;
    /*
     * Create the "phantom" row/column that holds clipped items
     */
    if (CtrIsHORIZONTAL(cw))
        cw->container.cell_count += height_in_cells;
    else
        cw->container.cell_count += width_in_cells;
    cw->container.cells = (Widget *)XtCalloc(cw->container.cell_count,
                                sizeof(Widget));
    cw->container.next_free_cell = 0;
}

/************************************************************************
 * PlaceCwid
 ************************************************************************/
static  void
#ifdef _NO_PROTO
PlaceCwid(cwid,x,y)
    Widget      cwid;
    Position    x;
    Position    y;
#else
PlaceCwid(
    Widget      cwid,
    Position    x,
    Position    y)
#endif /* _NO_PROTO */
{
    XmContainerWidget           cw;
    XmContainerConstraint       c;

    if (cwid == NULL) return;
    cw = (XmContainerWidget)XtParent(cwid);
    c = GetContainerConstraint(cwid);
    if (CtrViewIsLARGEICON(cw))
        {
        c->large_icon_x = x;
        c->large_icon_y = y;
        }
    else
        {
        c->small_icon_x = x;
        c->small_icon_y = y;
        }
    x += cw->container.margin_w;
    y += cw->container.margin_h;
    if ((x != ((Widget)cwid)->core.x) || (y != ((Widget)cwid)->core.y))
        _XmConfigureObject(
            cwid,x,y,((Widget)cwid)->core.width,((Widget)cwid)->core.height,0);
}

/************************************************************************
 * HideCwid
 ************************************************************************/
static  void
#ifdef _NO_PROTO
HideCwid(cwid)
    Widget  cwid;
#else
HideCwid(
    Widget  cwid)
#endif /* _NO_PROTO */
{
    XmContainerConstraint   c;
    CwidNode                node;
    CwidNode                child_node;

    if (cwid == NULL) return;
    c = GetContainerConstraint(cwid);

    _XmConfigureObject(
        cwid,(Position)(0 - ((Widget)cwid)->core.width),
             (Position)(0 - ((Widget)cwid)->core.height),
             ((Widget)cwid)->core.width,((Widget)cwid)->core.height,0);

    if (c->container_created) return;

    /*
     * Hide our button!
     */
    if (c->outline_button != NULL)
        HideCwid(c->outline_button);

    /*
     * If we're XmEXPANDED, then let's hide our children too.
     */
    if (c->outline_state != XmEXPANDED)
        return;
    node = c->node_ptr;
    child_node = node->child_ptr;
    while (child_node != NULL)
        {
        HideCwid(child_node->widget_ptr);
        c = GetContainerConstraint(child_node->widget_ptr);
        if (c->outline_button != NULL)
            HideCwid(c->outline_button);
        child_node = child_node->next_ptr;
        }
}


/*------------
| ActionProcs |
------------*/

/************************************************************************
 * ContainerBeginSelect
 ************************************************************************/
static  void
#ifdef _NO_PROTO
ContainerBeginSelect(wid,event,params,num_params)
        Widget          wid;
        XEvent          *event;
        String          *params;
        Cardinal        *num_params;
#else
ContainerBeginSelect(
        Widget          wid,
        XEvent          *event,
        String          *params,
        Cardinal        *num_params)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;

        cw->container.extending_mode = (CtrPolicyIsMULTIPLE(cw));
        cw->container.cancel_pressed = False;
        StartSelect(wid,event,params,num_params);
}

/************************************************************************
 * ContainerButtonMotion
 ************************************************************************/
static  void
#ifdef _NO_PROTO
ContainerButtonMotion(wid,event,params,num_params)
        Widget          wid;
        XEvent          *event;
        String          *params;
        Cardinal        *num_params;
#else
ContainerButtonMotion(
        Widget          wid,
        XEvent          *event,
        String          *params,
        Cardinal        *num_params)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        Boolean                 selection_changes;

        if (cw->container.cancel_pressed)
                return;
        if (CtrPolicyIsSINGLE(wid))
                return;
        selection_changes = ProcessButtonMotion(wid,event,params,num_params);
        cw->container.no_auto_sel_changes |= selection_changes;
        if (CtrIsAUTO_SELECT(cw) && selection_changes)
                CallSelectCB(wid,event,XmAUTO_MOTION);
}

/************************************************************************
 * ContainerEndSelect
 ************************************************************************/
static  void
#ifdef _NO_PROTO
ContainerEndSelect(wid,event,params,num_params)
        Widget          wid;
        XEvent          *event;
        String          *params;
        Cardinal        *num_params;
#else
ContainerEndSelect(
        Widget          wid,
        XEvent          *event,
        String          *params,
        Cardinal        *num_params)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        Boolean                 selection_changes;

        if (cw->container.cancel_pressed)
                return;
        if (cw->container.toggle_pressed)
                {
                ContainerEndToggle(wid,event,params,num_params);
                return;
                }
        if (cw->container.extend_pressed)
                {
                ContainerEndExtend(wid,event,params,num_params);
                return;
                }
        if (CtrPolicyIsSINGLE(cw))
                {
                if (cw->container.no_auto_sel_changes)
                        CallSelectCB(wid,event,XmAUTO_UNSET);
                return;
                }

        selection_changes = ProcessButtonMotion(wid,event,params,num_params);
        cw->container.no_auto_sel_changes |= selection_changes;
        if (cw->container.marquee_drawn)
                {
                DrawMarquee(wid);
                cw->container.marquee_drawn = False;
                if XtIsRealized(wid)
                        XClearArea(XtDisplay(wid),XtWindow(wid),
                                cw->container.marquee_smallest.x,
                                cw->container.marquee_smallest.y,
                                cw->container.marquee_largest.x,
                                cw->container.marquee_largest.y,True);
                }
        if (cw->container.anchor_cwid != NULL)
                {
                if (!cw->container.kaddmode)
                        SetLocationCursor(cw->container.anchor_cwid);
                if (CtrPolicyIsBROWSE(cw))
                        cw->container.no_auto_sel_changes |=
                                MarkCwid(cw->container.anchor_cwid,False);
                else
                        /* CtrPolicyIsMULTIPLE || CtrPolicyIsEXTENDED */
                        SetMarkedCwids(wid);
                }
        if (CtrIsAUTO_SELECT(cw))
                if (selection_changes)
                        CallSelectCB(wid,event,XmAUTO_CHANGE);
                else
                        CallSelectCB(wid,event,XmAUTO_NO_CHANGE);
        else
                if (cw->container.no_auto_sel_changes)
                        CallSelectCB(wid,event,XmAUTO_UNSET);
}

/************************************************************************
 * ContainerBeginToggle
 ************************************************************************/
static  void
#ifdef _NO_PROTO
ContainerBeginToggle(wid,event,params,num_params)
        Widget          wid;
        XEvent          *event;
        String          *params;
        Cardinal        *num_params;
#else
ContainerBeginToggle(
        Widget          wid,
        XEvent          *event,
        String          *params,
        Cardinal        *num_params)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;

#ifdef DEBUG
printf("ContainerBeginToggle\n");
#endif

        cw->container.toggle_pressed = True;
        cw->container.cancel_pressed = False;
        if (CtrPolicyIsSINGLE(cw) || CtrPolicyIsBROWSE(cw))
                return;
        cw->container.extending_mode = True;
        StartSelect(wid,event,params,num_params);
}

/************************************************************************
 * ContainerEndToggle
 ************************************************************************/
static  void
#ifdef _NO_PROTO
ContainerEndToggle(wid,event,params,num_params)
        Widget          wid;
        XEvent          *event;
        String          *params;
        Cardinal        *num_params;
#else
ContainerEndToggle(
        Widget          wid,
        XEvent          *event,
        String          *params,
        Cardinal        *num_params)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;

#ifdef DEBUG
printf("ContainerEndToggle\n");
#endif
        cw->container.toggle_pressed = False;
        if (cw->container.cancel_pressed)
                return;
        if (CtrPolicyIsSINGLE(cw) || CtrPolicyIsBROWSE(cw))
                return;
        ContainerEndSelect(wid,event,params,num_params);
}

/************************************************************************
 * ContainerBeginExtend
 ************************************************************************/
static  void
#ifdef _NO_PROTO
ContainerBeginExtend(wid,event,params,num_params)
        Widget          wid;
        XEvent          *event;
        String          *params;        /* unused */
        Cardinal        *num_params;    /* unused */
#else
ContainerBeginExtend(
        Widget          wid,
        XEvent          *event,
        String          *params,        /* unused */
        Cardinal        *num_params)    /* unused */
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        Widget                  current_cwid;

#ifdef DEBUG
printf("ContainerBeginExtend\n");
#endif

        cw->container.extend_pressed = True;
        cw->container.cancel_pressed = False;
        if (CtrPolicyIsSINGLE(cw) || CtrPolicyIsBROWSE(cw))
                return;
        if (!(CtrViewIsDETAIL(cw) || CtrLayoutIsOUTLINE(cw)))
                return;
        if ((current_cwid
                = LocatePointer(wid,event->xbutton.x,event->xbutton.y)) == NULL)
                return;
        if (cw->container.anchor_cwid == NULL)
                return;
        (void) MarkCwidsInRange(wid,cw->container.anchor_cwid,
                                current_cwid,(Boolean)True);
        if (CtrIsAUTO_SELECT(cw))
                CallSelectCB(wid,event,XmAUTO_BEGIN);
}

/************************************************************************
 * ContainerEndExtend
 ************************************************************************/
static  void
#ifdef _NO_PROTO
ContainerEndExtend(wid,event,params,num_params)
        Widget          wid;
        XEvent          *event;
        String          *params;        /* unused */
        Cardinal        *num_params;    /* unused */
#else
ContainerEndExtend(
        Widget          wid,
        XEvent          *event,
        String          *params,        /* unused */
        Cardinal        *num_params)    /* unused */
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;

#ifdef DEBUG
printf("ContainerEndExtend\n");
#endif
        cw->container.extend_pressed = False;
        if (cw->container.cancel_pressed)
                return;
        if (CtrPolicyIsSINGLE(cw) || CtrPolicyIsBROWSE(cw))
                return;
        if (!(CtrViewIsDETAIL(cw) || CtrLayoutIsOUTLINE(cw)))
                return;
        SetMarkedCwids(wid);
        if (CtrIsAUTO_SELECT(cw))
                CallSelectCB(wid,event,XmAUTO_NO_CHANGE);
        else
                CallSelectCB(wid,event,XmAUTO_UNSET);
}

/************************************************************************
 * ContainerCancel
 ************************************************************************/
static  void
#ifdef _NO_PROTO
ContainerCancel(wid,event,params,num_params)
        Widget          wid;
        XEvent          *event;
        String          *params;        /* unused */
        Cardinal        *num_params;    /* unused */
#else
ContainerCancel(
        Widget          wid,
        XEvent          *event,
        String          *params,        /* unused */
        Cardinal        *num_params)    /* unused */
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        Boolean                 selection_changes = False;

#ifdef DEBUG
printf("ContainerCancel\n");
#endif

        cw->container.toggle_pressed = False;
        cw->container.extend_pressed = False;
        if (CtrPolicyIsSINGLE(cw))
                return;
        cw->container.cancel_pressed = True;
        if (CtrPolicyIsBROWSE(cw))
                {
                if (cw->container.anchor_cwid != NULL)
                        {
                        selection_changes = UnmarkCwidVisual(
                                                cw->container.anchor_cwid);
                        if (CtrIsAUTO_SELECT(cw) && selection_changes)
                                {
                                CallSelectCB(wid,event,XmAUTO_CANCEL);
                                }
                        }
                return;
                }
        /* CtrPolicyIsMULTIPLE || CtrPolicyIsEXTENDED */
        selection_changes = ResetMarkedCwids(wid);
        if (cw->container.marquee_drawn)
                {
                DrawMarquee(wid);
                cw->container.marquee_drawn = False;
                if XtIsRealized(wid)
                        XClearArea(XtDisplay(wid),XtWindow(wid),
                                cw->container.marquee_smallest.x,
                                cw->container.marquee_smallest.y,
                                cw->container.marquee_largest.x,
                                cw->container.marquee_largest.y,True);
                }
        if (CtrIsAUTO_SELECT(cw) && selection_changes)
                {
                CallSelectCB(wid,event,XmAUTO_CANCEL);
                }
}

/************************************************************************
 * ContainerSelect
 ************************************************************************/
static  void
#ifdef _NO_PROTO
ContainerSelect(wid,event,params,num_params)
        Widget          wid;
        XEvent          *event;
        String          *params;
        Cardinal        *num_params;
#else
ContainerSelect(
        Widget          wid,
        XEvent          *event,
        String          *params,
        Cardinal        *num_params)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        XmContainerConstraint   c;

#ifdef DEBUG
printf("ContainerSelect\n");
#endif

        if (cw->container.loc_cursor_cwid != NULL)
                {
                c = GetContainerConstraint(cw->container.loc_cursor_cwid);
                if (c->container_created)
                        {
                        XtCallActionProc(cw->container.loc_cursor_cwid,
                                "ArmAndActivate",event,params,*num_params);
                        BorderHighlight(cw->container.loc_cursor_cwid);
                        return;
                        }
                }
        cw->container.extending_mode = cw->container.kaddmode;
        KBSelect(wid,event,params,num_params);
}

/************************************************************************
 * ContainerExtend
 ************************************************************************/
static  void
#ifdef _NO_PROTO
ContainerExtend(wid,event,params,num_params)
        Widget          wid;
        XEvent          *event;
        String          *params;        /* unused */
        Cardinal        *num_params;    /* unused */
#else
ContainerExtend(
        Widget          wid,
        XEvent          *event,
        String          *params,        /* unused */
        Cardinal        *num_params)    /* unused */
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        XmContainerConstraint   c;

#ifdef DEBUG
printf("ContainerExtend\n");
#endif
        if (cw->container.loc_cursor_cwid != NULL)
                {
                c = GetContainerConstraint(cw->container.loc_cursor_cwid);
                if (c->container_created)
                        return;
                }
        if (!(cw->container.kaddmode))
                return;
        if (!(CtrViewIsDETAIL(cw) || CtrLayoutIsOUTLINE(cw)))
                return;
        cw->container.no_auto_sel_changes |=
                        MarkCwidsInRange(wid,cw->container.anchor_cwid,
                                cw->container.loc_cursor_cwid,(Boolean)False);
        if (CtrIsAUTO_SELECT(cw))
                {
                CallSelectCB(wid,event,XmAUTO_BEGIN);
                CallSelectCB(wid,event,XmAUTO_NO_CHANGE);
                }
        else
                if (cw->container.no_auto_sel_changes)
                        CallSelectCB(wid,event,XmAUTO_UNSET);
}

/************************************************************************
 * ContainerMoveCursor
 ************************************************************************/
static  void
#ifdef _NO_PROTO
ContainerMoveCursor(wid,event,params,num_params)
        Widget          wid;
        XEvent          *event;
        String          *params;
        Cardinal        *num_params;
#else
ContainerMoveCursor(
        Widget          wid,
        XEvent          *event,
        String          *params,
        Cardinal        *num_params)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        XmContainerConstraint   c;

        if (*num_params == 0) return;
        SetLocationCursor(CalcNextLocationCursor(wid,params[0]));
        if (cw->container.loc_cursor_cwid != NULL)
                {
                c = GetContainerConstraint(cw->container.loc_cursor_cwid);
                if (c->container_created)
                        return;
                }
        if (cw->container.kaddmode)
                return;
        cw->container.extending_mode = False;
        KBSelect(wid,event,params,num_params);
}

/************************************************************************
 * ContainerExtendCursor
 ************************************************************************/
static  void
#ifdef _NO_PROTO
ContainerExtendCursor(wid,event,params,num_params)
        Widget          wid;
        XEvent          *event;
        String          *params;
        Cardinal        *num_params;
#else
ContainerExtendCursor(
        Widget          wid,
        XEvent          *event,
        String          *params,
        Cardinal        *num_params)
#endif /* _NO_PROTO */
{
#ifdef DEBUG
printf("ContainerExtendCursor\n");
#endif

        if (*num_params == 0) return;
        SetLocationCursor(CalcNextLocationCursor(wid,params[0]));
        ContainerExtend(wid,event,params,num_params);
}

/************************************************************************
 * ContainerToggleMode
 ************************************************************************/
static  void
#ifdef _NO_PROTO
ContainerToggleMode(wid,event,params,num_params)
        Widget          wid;
        XEvent          *event;         /* unused */
        String          *params;        /* unused */
        Cardinal        *num_params;    /* unused */
#else
ContainerToggleMode(
        Widget          wid,
        XEvent          *event,         /* unused */
        String          *params,        /* unused */
        Cardinal        *num_params)    /* unused */
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        XmContainerConstraint   c;

#ifdef DEBUG
printf("ContainerToggleMode\n");
#endif
        if CtrPolicyIsEXTENDED(cw)
                {
                c = GetContainerConstraint(cw->container.loc_cursor_cwid);
                if (c->container_created)
                        cw->container.kaddmode = !cw->container.kaddmode;
                else
                        {
                        BorderUnhighlight(cw->container.loc_cursor_cwid);
                        cw->container.kaddmode = !cw->container.kaddmode;
                        BorderHighlight(cw->container.loc_cursor_cwid);
                        }
                }
}

/************************************************************************
 * ContainerSelectAll
 ************************************************************************/
static  void
#ifdef _NO_PROTO
ContainerSelectAll(wid,event,params,num_params)
        Widget          wid;
        XEvent          *event;
        String          *params;
        Cardinal        *num_params;
#else
ContainerSelectAll(
        Widget          wid,
        XEvent          *event,
        String          *params,
        Cardinal        *num_params)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;

#ifdef DEBUG
printf("ContainerSelectAll\n");
#endif

        if (CtrPolicyIsSINGLE(cw) || CtrPolicyIsBROWSE(cw))
                {
                ContainerSelect(wid,event,params,num_params);
                return;
                }
        cw->container.no_auto_sel_changes |= SelectAllCwids(wid);
        if (CtrIsAUTO_SELECT(cw))
                {
                CallSelectCB(wid,event,XmAUTO_BEGIN);
                CallSelectCB(wid,event,XmAUTO_NO_CHANGE);
                }
        else
                if (cw->container.no_auto_sel_changes)
                        CallSelectCB(wid,event,XmAUTO_UNSET);
}

/************************************************************************
 * ContainerDeselectAll
 ************************************************************************/
static  void
#ifdef _NO_PROTO
ContainerDeselectAll(wid,event,params,num_params)
        Widget          wid;
        XEvent          *event;
        String          *params;        /* unused */
        Cardinal        *num_params;    /* unused */
#else
ContainerDeselectAll(
        Widget          wid,
        XEvent          *event,
        String          *params,        /* unused */
        Cardinal        *num_params)    /* unused */
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;

#ifdef DEBUG
printf("ContainerDeselectAll\n");
#endif

        cw->container.no_auto_sel_changes |= DeselectAllCwids(wid);
        if (CtrIsAUTO_SELECT(cw) && (!CtrPolicyIsSINGLE(cw)))
                {
                CallSelectCB(wid,event,XmAUTO_BEGIN);
                CallSelectCB(wid,event,XmAUTO_NO_CHANGE);
                }
        else
                if (cw->container.no_auto_sel_changes)
                        CallSelectCB(wid,event,XmAUTO_UNSET);
}

/************************************************************************
 * ContainerActivate
 ************************************************************************/
static  void
#ifdef _NO_PROTO
ContainerActivate(wid,event,params,num_params)
        Widget          wid;
        XEvent          *event;
        String          *params;
        Cardinal        *num_params;
#else
ContainerActivate(
        Widget          wid,
        XEvent          *event,
        String          *params,
        Cardinal        *num_params)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        XmContainerConstraint   c;

#ifdef DEBUG
printf("ContainerActivate\n");
#endif

        if (cw->container.loc_cursor_cwid != NULL)
                {
                c = GetContainerConstraint(cw->container.loc_cursor_cwid);
                if (c->container_created)
                        {
                        XtCallActionProc(cw->container.loc_cursor_cwid,
                                "ArmAndActivate",event,params,*num_params);
                        BorderHighlight(cw->container.loc_cursor_cwid);
                        return;
                        }
                }
        if (cw->container.anchor_cwid == NULL)
                return;
        CallActionCB(cw->container.anchor_cwid,event);
}

/************************************************************************
 * ContainerFocus
 ************************************************************************/
static  void
#ifdef _NO_PROTO
ContainerFocus(wid,event,params,num_params)
        Widget          wid;
        XEvent          *event;         /* unused */
        String          *params;
        Cardinal        *num_params;    /* unused */
#else
ContainerFocus(
        Widget          wid,
        XEvent          *event,         /* unused */
        String          *params,
        Cardinal        *num_params)    /* unused */
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;

        if (cw->container.loc_cursor_cwid == NULL)
                return;

        /*
         * Highlight on a FocusIn,
         * Unhighlight on a FocusOut.
         */
        if (strcmp(params[0],_IN) == 0)
                {
                cw->container.have_focus = True;
                BorderHighlight(cw->container.loc_cursor_cwid);
                }
        else
                {
                cw->container.have_focus = False;
                BorderUnhighlight(cw->container.loc_cursor_cwid);
                }
}



/*-------------------
| Internal functions |
-------------------*/
/************************************************************************
 * Layout
 ************************************************************************/
static  void
#ifdef _NO_PROTO
Layout(wid)
        Widget  wid;
#else
Layout(
        Widget  wid)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;

#ifdef  DEBUG
printf("Layout\n");
#endif

        if (CtrLayoutIsOUTLINE(cw) || CtrViewIsDETAIL(cw))
                LayoutOutlineDetail(wid);
        else
                LayoutSpatial(wid);
}

/************************************************************************
 * RequestNewSize
 ************************************************************************/
static  void
#ifdef _NO_PROTO
RequestNewSize(wid)
        Widget  wid;
#else
RequestNewSize(
        Widget  wid)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        Dimension               ideal_width,ideal_height;
        XtWidgetGeometry        geo_current,geo_spatial,geo_desired;

#ifdef  DEBUG
printf("RequestNewSize\n");
#endif


    if (CtrLayoutIsOUTLINE(cw) || CtrViewIsDETAIL(cw))
        {
        /*
         * Layout() routine keeps track of the
         * ideal width/height for "one-dimensional" layouts.
         */
        ideal_width = cw->container.ideal_width;
        ideal_height = cw->container.ideal_height;
        }
    else
        {
        /*
         * Get the desired width/height for a two-dimensional
         * Spatial layout from the class method.
         */
        geo_current.request_mode = CWWidth | CWHeight;
        geo_current.width = cw->core.width;
        geo_current.height = cw->core.height;
        (*(xmContainerClassRec.core_class.query_geometry))
                (wid,&geo_current,&geo_spatial);
        ideal_width = geo_spatial.width;
        ideal_height = geo_spatial.height;
        }
    geo_desired.request_mode = 0;
    if ((ideal_width != cw->core.width) && (!cw->container.width_frozen))
        {
        geo_desired.request_mode |= CWWidth;
        geo_desired.width = ideal_width;
        }
    if ((ideal_height != cw->core.height) && (!cw->container.height_frozen))
        {
        geo_desired.request_mode |= CWHeight;
        geo_desired.height = ideal_height;
        }
    if (geo_desired.request_mode != 0)
        if (_XmMakeGeometryRequest(wid,&geo_desired)
                                        == XtGeometryYes)
                (*(xmContainerClassRec.core_class.resize))(wid);
}

/************************************************************************
 * LayoutOutlineDetail
 ************************************************************************/
static  void
#ifdef _NO_PROTO
LayoutOutlineDetail(wid)
        Widget  wid;
#else
LayoutOutlineDetail(
        Widget  wid)
#endif /* _NO_PROTO */
{
    XmContainerWidget           cw = (XmContainerWidget)wid;
    XmContainerConstraint       c;
    XmContainerConstraint       pc;

    Position                    x,y,cwid_x,cwid_y,ob_x,ob_y;
    CwidNode                    node;
    Dimension                   cwid_width,cwid_height;
    int                         i;
    int                         n_outline_segs,n_detail_segs,seg_idx;
    int                         vspacing;
    XPoint *                    point_at_depth = NULL;

    n_outline_segs = n_detail_segs = seg_idx = 0;
    if (CtrDrawLinesOUTLINE(cw))
        {
        node = cw->container.first_node;
        while (node != NULL)
            {
            c = GetContainerConstraint(node->widget_ptr);
            if (c->depth != 0)
                n_outline_segs += 2;
            node = GetNextNode(wid,node);
            }
        if (cw->container.outline_seg_count != n_outline_segs)
            {
            if (cw->container.outline_segs != NULL)
                XtFree((XtPointer)cw->container.outline_segs);
            cw->container.outline_segs = (XSegment *)XtCalloc
                                (n_outline_segs,sizeof(XSegment));
            cw->container.outline_seg_count = n_outline_segs;
            }
        point_at_depth = (XPoint *)XtCalloc((cw->container.max_depth + 1),
                                        sizeof(XPoint));
        }
    else
        {
        if (cw->container.outline_segs != NULL)
            XtFree((XtPointer)cw->container.outline_segs);
        cw->container.outline_segs = NULL;
        cw->container.outline_seg_count = 0;
        }
    vspacing = CtrDrawLinesDETAIL(cw) ? 3 : 0;
    if (CtrLayoutIsOUTLINE(cw) || CtrViewIsDETAIL(cw))
        {
        y = cw->container.margin_h;
        if (CtrViewIsDETAIL(cw) && (cw->container.detail_heading != NULL))
            {
            y += cw->container.heading_rect.height + vspacing;
            if (cw->container.detail_count > 0)
                cw->container.tabs[1] = cw->container.heading_col1_width;
            n_detail_segs++;
            }
        else
            if (CtrViewIsDETAIL(cw) &&
                (cw->container.detail_count > 0))
                cw->container.tabs[1] = 0;
        cw->container.ideal_width = cw->container.margin_w;
        cw->container.ideal_height = y;

        node = cw->container.first_node;
        while (node != NULL)
            {
            cw->container.last_node = node;
            c = GetContainerConstraint(node->widget_ptr);
            c->row_y = y;
            x = cw->container.margin_w +
                        cw->container.outline_indent * c->depth;
            cwid_width = ((XmGadget)node->widget_ptr)->rectangle.width;
            cwid_height = ((XmGadget)node->widget_ptr)->rectangle.height;
            c->row_height = cwid_height;
            if CtrViewIsDETAIL(cw)
                c->row_height = MAX(c->row_height,c->detail_height);
            c->row_height = MAX(c->row_height,cw->container.ob_height);
            ob_y = y + (c->row_height - cw->container.ob_height)/2;
            if (CtrLayoutIsOUTLINE(cw) && (c->outline_button != NULL))
                {
                ob_x = CtrLayoutIsLtoR(cw) ? x : MAX(cw->container.margin_w,
                                cw->core.width - cw->container.ob_width - x);
                _XmConfigureObject(c->outline_button,
                    ob_x,ob_y,cw->container.ob_width,cw->container.ob_height,0);
                }
            if CtrLayoutIsOUTLINE(cw)
                x += cw->container.ob_width;
            cwid_x = CtrLayoutIsLtoR(cw) ? x : MAX(cw->container.margin_w,
                                cw->core.width - cwid_width - x);
            cwid_y = y + (c->row_height - cwid_height)/2;
            PlaceCwid(node->widget_ptr,cwid_x - cw->container.margin_w,
                                        cwid_y - cw->container.margin_h);
            cw->container.ideal_width = MAX(cw->container.ideal_width,
                        (Dimension)x + cwid_width + cw->container.margin_w);
            if (CtrViewIsDETAIL(cw) &&
                (cw->container.detail_count > 0))
                cw->container.tabs[1] = MAX(cw->container.tabs[1],
                        (Dimension)x + cwid_width - cw->container.margin_w);
            if (CtrDrawLinesOUTLINE(cw))
                {
                if (point_at_depth[c->depth].x == 0)
                    point_at_depth[c->depth].x = cw->container.margin_w
                                + cw->container.outline_indent * c->depth
                                + cw->container.ob_width / 2;
                if (c->entry_parent != NULL)
                    {
                    pc = GetContainerConstraint(c->entry_parent);
                    cw->container.outline_segs[seg_idx].x1 =
                                        point_at_depth[pc->depth].x;
                    cw->container.outline_segs[seg_idx].x2 =
                                        cw->container.outline_segs[seg_idx].x1;
                    cw->container.outline_segs[seg_idx].y1 =
                                        point_at_depth[pc->depth].y;
                    cw->container.outline_segs[seg_idx].y2 =
                                        c->row_y + c->row_height/2;
                    cw->container.outline_segs[seg_idx + 1].x1 =
                                        cw->container.outline_segs[seg_idx].x2;
                    cw->container.outline_segs[seg_idx + 1].y1 =
                                        cw->container.outline_segs[seg_idx].y2;
                    cw->container.outline_segs[seg_idx + 1].x2 =
                                        cw->container.margin_w
                                        + (cw->container.outline_indent
                                                * c->depth);
                    if (c->outline_button == NULL)
                        cw->container.outline_segs[seg_idx + 1].x2
                                                += cw->container.ob_width;
                    cw->container.outline_segs[seg_idx + 1].y2 =
                                cw->container.outline_segs[seg_idx].y2;
                    seg_idx += 2;
                    }
                 point_at_depth[c->depth].y = ob_y + cw->container.ob_height;
                }
            y += c->row_height + vspacing;
            n_detail_segs++;
            node = GetNextNode(wid,node);
            }
        if CtrViewIsDETAIL(cw)
            {
            if (cw->container.detail_count > 0)
                cw->container.ideal_width = cw->container.tabs[0];
            else
                cw->container.ideal_width = cw->container.margin_w;
            for (i = 0; i < cw->container.detail_count; i++)
                if (ShowColumn(wid,i))
                    {
                    cw->container.ideal_width += cw->container.tabs[i+1]
                                                + cw->container.hspacing;
                    n_detail_segs++;
                    }
            }
        /*
         * No space at the ends; take one away & add the margins
         */
        cw->container.ideal_width += cw->container.margin_w;
        cw->container.ideal_height = y - vspacing;
        cw->container.ideal_height += cw->container.margin_h;
        n_detail_segs = MAX(0,n_detail_segs - 2);
        }

    if (CtrDrawLinesDETAIL(cw) && (n_detail_segs > 0))
        {
        int     vgroove = vspacing - vspacing/2;
        int     hgroove = cw->container.hspacing - (cw->container.hspacing/2);
        int     x,x1,x2,y1,y2;

        if (cw->container.detail_seg_count != n_detail_segs)
            {
            if (cw->container.detail_segs != NULL)
                XtFree((XtPointer)cw->container.detail_segs);
            cw->container.detail_segs = (XSegment *)XtCalloc
                (n_detail_segs,sizeof(XSegment));
            cw->container.detail_seg_count = n_detail_segs;
            }

        seg_idx = 0;
        /*
         * Calculate the horizontal lines.
         */
        x1 = cw->container.margin_w;
        x2 = cw->container.ideal_width - cw->container.margin_w;
        if (cw->container.detail_heading != NULL)
            {
            cw->container.detail_segs[seg_idx].x1 = x1;
            cw->container.detail_segs[seg_idx].x2 = x2;
            cw->container.detail_segs[seg_idx].y1 = cw->container.margin_h
                                + cw->container.heading_rect.height + vgroove;
            cw->container.detail_segs[seg_idx].y2
                                = cw->container.detail_segs[seg_idx].y1;
            seg_idx++;
            }
        node = cw->container.first_node;
        while (node != NULL)
            {
            if (node != cw->container.last_node)
                {
                c = GetContainerConstraint(node->widget_ptr);
                cw->container.detail_segs[seg_idx].x1 = x1;
                cw->container.detail_segs[seg_idx].x2 = x2;
                cw->container.detail_segs[seg_idx].y1
                                = c->row_y + c->row_height + vgroove;
                cw->container.detail_segs[seg_idx].y2
                                = cw->container.detail_segs[seg_idx].y1;
                seg_idx++;
                }
            node = GetNextNode(wid,node);
            }
        /*
         * Calculate the vertical lines.
         */
        y1 = cw->container.margin_h;
        y2 = cw->container.ideal_height - cw->container.margin_h;
        x = cw->container.tabs[0];
        for (i = 0; i < ((int)cw->container.detail_count - 1); i++)
            if (ShowColumn(wid,i))
                {
                x += cw->container.tabs[i+1];
                cw->container.detail_segs[seg_idx].x1 = x + hgroove;
                cw->container.detail_segs[seg_idx].x2 = x + hgroove;
                cw->container.detail_segs[seg_idx].y1 = y1;
                cw->container.detail_segs[seg_idx].y2 = y2;
                x += cw->container.hspacing;
                seg_idx++;
                }
        /*
         * If Left-to-Right, reverse all the x1's & x2's.
         * And also adjust them to fit our current size, since
         * x position changes with width in Right-to-Left layout.
         */
        if CtrLayoutIsRtoL(cw)
            {
            int adjust;

            adjust = cw->core.width - cw->container.ideal_width;
            for (i = 0; i < cw->container.detail_seg_count; i++)
                {
                cw->container.detail_segs[i].x1 = MAX(0,
                                cw->container.ideal_width
                                - cw->container.detail_segs[i].x1 + adjust);
                cw->container.detail_segs[i].x2 = MAX(0,
                                cw->container.ideal_width
                                - cw->container.detail_segs[i].x2 + adjust);
                }
            }
        }
    else
        {
        if (cw->container.detail_segs != NULL)
            XtFree((XtPointer)cw->container.detail_segs);
        cw->container.detail_segs = NULL;
        cw->container.detail_seg_count = 0;
        }

    /*
     * Free the Malloc'd x_at_depth, y_at_depth.
     * If Left-to-Right, reverse all the x1's & x2's
     * and adjust, same as for Detail Lines.
     */
    if (CtrDrawLinesOUTLINE(cw))
        {
        if (point_at_depth != NULL)
            XtFree((XtPointer)point_at_depth);
        if CtrLayoutIsRtoL(cw)
            {
            int adjust;

            adjust = cw->core.width - cw->container.ideal_width;
            for (i = 0; i < cw->container.outline_seg_count; i++)
                {
                cw->container.outline_segs[i].x1 = MAX(0,
                                cw->container.ideal_width
                                - cw->container.outline_segs[i].x1 + adjust);
                cw->container.outline_segs[i].x2 = MAX(0,
                                cw->container.ideal_width
                                - cw->container.outline_segs[i].x2 + adjust);
                }
            }
        }
}

/************************************************************************
 * LayoutSpatial
 ************************************************************************/
static  void
#ifdef _NO_PROTO
LayoutSpatial(wid)
        Widget  wid;
#else
LayoutSpatial(
        Widget  wid)
#endif /* _NO_PROTO */
{
    XmContainerWidget   cw = (XmContainerWidget)wid;
    int         cell_width,cell_height;
    int         width_in_cells,height_in_cells;
    XmContainerConstraint c;
    CwidNode    node;
    XPoint      spot;

    if (CtrPlaceStyleIsGRID(cw) || CtrPlaceStyleIsCELLS(cw))
        /*
         * Let's see if things have changed enough so that we have to reset.
         */
        {
        cell_width = (CtrViewIsLARGEICON(cw)) ?
            cw->container.large_cell_width : cw->container.small_cell_width;
        cell_height = (CtrViewIsLARGEICON(cw)) ?
            cw->container.large_cell_height : cw->container.small_cell_height;
        width_in_cells = (cw->core.width - 2 * cw->container.margin_w)
                                / cell_width;
        height_in_cells = (cw->core.height - 2 * cw->container.margin_h)
                                / cell_height;

        if ((width_in_cells != cw->container.current_width_in_cells) ||
            (height_in_cells != cw->container.current_height_in_cells))
            {
            (*(xmContainerClassRec.container_class.place_item))
                        (wid,NULL);
            cw->container.current_width_in_cells = width_in_cells;
            cw->container.current_height_in_cells = height_in_cells;
            }
        }
    node = cw->container.first_node;
    while (node != NULL)
        {
        cw->container.last_node = node;
        c = GetContainerConstraint(node->widget_ptr);
        if (!CtrItemIsPlaced(node->widget_ptr))
            (*(xmContainerClassRec.container_class.place_item))
                  (wid,node->widget_ptr);
        else
            {
            if CtrViewIsLARGEICON(cw)
                {
                spot.x = c->large_icon_x;
                spot.y = c->large_icon_y;
                }
            else
                {
                spot.x = c->small_icon_x;
                spot.y = c->small_icon_y;
                }
            PlaceCwid(node->widget_ptr,spot.x,spot.y);
            }
        node = GetNextNode(wid,node);
        }
}

/************************************************************************
 * SetCellSize
 ************************************************************************/
static  void
#ifdef _NO_PROTO
SetCellSize(wid)
    Widget      wid;
#else
SetCellSize(
    Widget      wid)
#endif /* _NO_PROTO */
{
    XmContainerWidget   cw = (XmContainerWidget)wid;
    CwidNode            node;
    Dimension           cell_width = ZERO_CELL_DIM;
    Dimension           cell_height = ZERO_CELL_DIM;

    node = cw->container.first_node;
    while (node != NULL)
        {
        if (node == cw->container.first_node)
            /*
             * Initialize to first_node width/height.
             */
            {
            cell_width = ((Widget)node->widget_ptr)->core.width;
            cell_height = ((Widget)node->widget_ptr)->core.height;
            }
        if (CtrPlaceStyleIsGRID(cw))
            /*
             * GRID: cell should be size of largest cwid.
             */
            {
            cell_width = MAX(cell_width,
                                ((Widget)node->widget_ptr)->core.width);
            cell_height = MAX(cell_height,
                                ((Widget)node->widget_ptr)->core.height);
            }
        else
            {
            /*
             * CELLS: cell should be size of smallest cwid.
             */
            cell_width = MIN(cell_width,
                                ((Widget)node->widget_ptr)->core.width);
            cell_height = MIN(cell_height,
                                ((Widget)node->widget_ptr)->core.height);
            }
        node = GetNextNode(wid,node);
        }
    if (CtrViewIsLARGEICON(cw))
        {
        if (cw->container.large_cell_width == ZERO_CELL_DIM)
            cw->container.large_cell_width = cell_width;
        if (cw->container.large_cell_height == ZERO_CELL_DIM)
            cw->container.large_cell_height = cell_height;
        }
    else
        {
        if (cw->container.small_cell_width == ZERO_CELL_DIM)
            cw->container.small_cell_width = cell_width;
        if (cw->container.small_cell_height == ZERO_CELL_DIM)
            cw->container.small_cell_height = cell_height;
        }
}

/************************************************************************
 * SizeOutlineButton
 ************************************************************************/
static  void
#ifdef _NO_PROTO
SizeOutlineButton(wid)
        Widget  wid;
#else
SizeOutlineButton(
        Widget  wid)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        Widget                  ob;
        Dimension               width,height;

        /*
         * Create & Delete dummy outline button so we can get size.
         */
        cw->container.self = True;
        ob = XtVaCreateWidget(OBNAME,
                xmPushButtonWidgetClass,(Widget)cw,
                XmNtraversalOn,False,XmNlabelType,XmPIXMAP,
                XmNlabelPixmap,cw->container.expanded_state_pixmap,NULL);
        XtVaGetValues(ob,XmNwidth,&cw->container.ob_width,
                        XmNheight,&cw->container.ob_height,NULL);
        XtVaSetValues(ob,XmNlabelPixmap,
                        cw->container.collapsed_state_pixmap,NULL);
        XtVaGetValues(ob,XmNwidth,&width,XmNheight,&height,NULL);
        XtDestroyWidget(ob);
        cw->container.self = False;
        cw->container.ob_width = MAX(cw->container.ob_width,width);
        cw->container.ob_height = MAX(cw->container.ob_height,height);
}

/************************************************************************
 * UpdateGCs
 ************************************************************************/
static  void
#ifdef _NO_PROTO
UpdateGCs(wid)
        Widget  wid;
#else
UpdateGCs(
        Widget  wid)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        XFontStruct *           fs = (XFontStruct *)NULL;
        XGCValues               values;
        XtGCMask                valueMask;

        /*
         * Free any previous GC's
         */
        if (cw->container.normalGC != NULL)
                XtReleaseGC(wid,cw->container.normalGC);
        if (cw->container.inverseGC != NULL)
                XtReleaseGC(wid,cw->container.inverseGC);
        if (cw->container.marqueeGC != NULL)
                XtReleaseGC(wid,cw->container.marqueeGC);
        /*
         * Get GC's
         */
        _XmFontListGetDefaultFont(cw->container.render_table,&fs);
        valueMask = GCForeground | GCBackground;
        values.foreground = cw->manager.foreground;
        values.background = cw->core.background_pixel;
        if (fs != NULL)
                {
                values.font = fs->fid;
                valueMask |= GCFont;
                }
        cw->container.normalGC = XtGetGC(wid,valueMask,&values);

        valueMask = GCForeground | GCBackground;
        values.foreground = cw->core.background_pixel;
        values.background = cw->manager.foreground;
        if (fs != NULL)
                {
                values.font = fs->fid;
                valueMask |= GCFont;
                }
        cw->container.inverseGC = XtGetGC(wid,valueMask,&values);

        valueMask = GCForeground | GCFunction;
        values.foreground = cw->core.background_pixel ^
                                cw->manager.foreground;
        values.function = GXxor;
        cw->container.marqueeGC = XtGetGC(wid,valueMask,&values);
}

/************************************************************************
 * SizeOneSpace
 ************************************************************************/
static  Dimension
#ifdef _NO_PROTO
SizeOneSpace(fl)
        XmFontList      fl;
#else
SizeOneSpace(
        XmFontList      fl)
#endif /* _NO_PROTO */
{
        XmString        aspace;
        Dimension       width,height;

        aspace = XmStringCreateSimple(" ");
        XmStringExtent(fl,aspace,&width,&height);
        XmStringFree(aspace);
        return(width);
}


/************************************************************************
 * CreateDetailHeading
 ************************************************************************/
static  void
#ifdef _NO_PROTO
CreateDetailHeading(wid)
        Widget  wid;
#else
CreateDetailHeading(
        Widget  wid)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        _XmString *             xmstr_table;
        int                     i;
        Dimension               w,h;

        cw->container.heading_rect.height = 0;
        cw->container.heading_col1_width = 0;
        if ((cw->container.detail_count == 0) ||
            (cw->container.detail_heading == NULL))
                return;
        /*
         * Create internal form of Detail Column Header.
         */
        xmstr_table = (_XmString *)XtMalloc
                        (cw->container.detail_count * sizeof(_XmString));
        for (i = 0; i < cw->container.detail_count; i++)
                {
                xmstr_table[i] = _XmStringCreate
                         ((XmString)cw->container.detail_heading[i]);
                _XmStringExtent(cw->container.render_table,xmstr_table[i],&w,&h);
                cw->container.heading_rect.height =
                        MAX(h,cw->container.heading_rect.height);
                        cw->container.tabs[i+1] = MAX(w,
                                                cw->container.tabs[i+1]);
                }
        if (cw->container.detail_count > 1)
                cw->container.heading_col1_width = cw->container.tabs[1];
        cw->container.detail_heading = xmstr_table;
}

/************************************************************************
 * CreateEntryDetail
 ************************************************************************/
static  void
#ifdef _NO_PROTO
CreateEntryDetail(cwid)
        Widget  cwid;
#else
CreateEntryDetail(
        Widget  cwid)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)XtParent(cwid);
        XmContainerConstraint   c = GetContainerConstraint(cwid);
        _XmString *             xmstr_table;
        int                     i;
        Dimension               w,h;

        c->detail_height = 0;
        c->entry_detail_count = cw->container.detail_count;
        if ((cw->container.detail_count == 0) || (c->entry_detail == NULL))
                return;
        xmstr_table = (_XmString *)XtMalloc
                        (cw->container.detail_count * sizeof(_XmString));
        for (i = 0; i < cw->container.detail_count; i++)
                {
                xmstr_table[i] = _XmStringCreate((XmString)c->entry_detail[i]);
                _XmStringExtent(cw->container.render_table,xmstr_table[i],&w,&h);
                c->detail_height = MAX(c->detail_height,h);
                if ((c->depth == 0) || CtrLayoutIsOUTLINE(cw))
                        cw->container.tabs[i+1] =
                                        MAX(cw->container.tabs[i+1],w);
                }
        c->entry_detail = xmstr_table;
}

/************************************************************************
 * ShowColumn
 ************************************************************************/
static  Boolean
#ifdef _NO_PROTO
ShowColumn(wid,column)
        Widget  wid;
        int     column;
#else
ShowColumn(
        Widget  wid,
        int     column)
#endif /* _NO_PROTO */
{
        return(True);
}

/************************************************************************
 * ExposeDetail
 ************************************************************************/
static  void
#ifdef _NO_PROTO
ExposeDetail(cwid)
        Widget  cwid;
#else
ExposeDetail(
        Widget  cwid)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)XtParent(cwid);
        XmContainerConstraint   c = GetContainerConstraint(cwid);
        Position                x;
        Dimension               width;

        if (!XtIsRealized(XtParent(cwid)))
                return;
        if (c->entry_detail == NULL)
                return;
        x = cw->container.tabs[0] + cw->container.tabs[1]
                + cw->container.hspacing;
        width = MAX(0,cw->core.width - x);
        if (CtrLayoutIsRtoL(cw))
            x = 0;
        XClearArea(XtDisplay(XtParent(cwid)),XtWindow(XtParent(cwid)),
                                        x,c->row_y,width,c->row_height,True);
}

/************************************************************************
 * ChangeView
 ************************************************************************/
static  void
#ifdef _NO_PROTO
ChangeView(wid,view)
        Widget          wid;
        unsigned char   view;
#else
ChangeView(
        Widget          wid,
        unsigned char   view)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        CwidNode                node;

        node = cw->container.first_node;
        while (node != NULL)
                {
                cw->container.self = True;
                XtVaSetValues(node->widget_ptr,XmNviewType,view,NULL);
                cw->container.self = False;

                if (node->child_ptr != NULL)
                        node = node->child_ptr;
                else
                        if (node->next_ptr != NULL)
                                node = node->next_ptr;
                        else
                                node = GetNextUpLevelNode(node);
                }
}

/************************************************************************
 * NewNode
 ************************************************************************/
static  CwidNode
#ifdef _NO_PROTO
NewNode(cwid)
        Widget  cwid;
#else
NewNode(
        Widget  cwid)
#endif /* _NO_PROTO */
{
        XmContainerConstraint   c = GetContainerConstraint(cwid);
        CwidNode                new_node;

        /*
         * Create the new CwidNode structure.
         */
        new_node = (CwidNode)XtCalloc(1,sizeof(XmCwidNodeRec));
        new_node->widget_ptr = cwid;    /* node ----> widget link */
        c->node_ptr = new_node;         /* widget --> node link */
        return(new_node);
}

/************************************************************************
 * InsertNode
 ************************************************************************/
static  void
#ifdef _NO_PROTO
InsertNode(node)
        CwidNode        node;
#else
InsertNode(
        CwidNode        node)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw;
        XmContainerConstraint   c;
        XmContainerConstraint   pc;     /* parent's constraints */
        XmContainerConstraint   sc;     /* sibling's constraints */
        Widget          cwid;
        CwidNode        prev_node;
        CwidNode        next_node;
        CwidNode        parent_node;

        cwid = node->widget_ptr;
        cw = (XmContainerWidget)XtParent(cwid);
        c = GetContainerConstraint(cwid);

        /*
         * Find the first cwid within XmNentryParent, NULL if none.
         */
        if (c->entry_parent == NULL)
                {
                parent_node = NULL;
                prev_node = cw->container.first_node;
                }
        else
                {
                pc = GetContainerConstraint(c->entry_parent);
                parent_node = pc->node_ptr;
                prev_node = parent_node->child_ptr;
                }

        if (prev_node == NULL)
                {
                /*
                 * Assume this is the first and only cwid within XmNentryParent.
                 */
                next_node = NULL;
                if (c->position_index == UNSPECIFIED_POSITION_INDEX)
                        c->position_index = 0;
                }
        else
                {
                /*
                 * Chain through this level until we find the correct position.
                 */
                next_node = prev_node;
                prev_node = NULL;
                while (next_node != NULL)
                        {
                        sc = GetContainerConstraint(next_node->widget_ptr);
                        if ((c->position_index != UNSPECIFIED_POSITION_INDEX) &&
                            (c->position_index <= sc->position_index))
                                break;
                        prev_node = next_node;
                        next_node = next_node->next_ptr;
                        }
                /*
                 * Set XmNpositionIndex, if required.
                 */
                if (c->position_index == UNSPECIFIED_POSITION_INDEX)
                        {
                        sc = GetContainerConstraint(prev_node->widget_ptr);
                        c->position_index = sc->position_index + 1;
                        }
                }

        /*
         * Insert the node into the linked list.
         */
        node->child_ptr = NULL;
        node->parent_ptr = parent_node;
        if ((node->prev_ptr = prev_node) != NULL)
                prev_node->next_ptr = node;
        if ((node->next_ptr = next_node) != NULL)
                next_node->prev_ptr = node;
        if ((parent_node != NULL) && (node->prev_ptr == NULL))
                parent_node->child_ptr = node;

        /*
         * Update cw->container.first_node, if we're now the first node.
         */
        if (node->next_ptr == cw->container.first_node)
                cw->container.first_node = node;
}

/************************************************************************
 * SeverNode
 ************************************************************************/
static  void
#ifdef _NO_PROTO
SeverNode(node)
        CwidNode        node;
#else
SeverNode(
        CwidNode        node)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw;
        CwidNode                parent_node;
        XmContainerConstraint   pc;
        CwidNode                prev_node;
        CwidNode                next_node;

        if (node == NULL) return;

        cw = (XmContainerWidget)XtParent(node->widget_ptr);

        /*
         * Find new cw->container.first_node, if we're now the first node.
         */
        if (node == cw->container.first_node)
                {
                if (node->next_ptr != NULL)
                        cw->container.first_node = node->next_ptr;
                else
                        cw->container.first_node = GetNextUpLevelNode(node);
                }
        /*
         * If we're child #1, fix up parent-child pointers.
         */
        if ((node->prev_ptr == NULL) &&
            (node->parent_ptr != NULL))
                {
                parent_node = node->parent_ptr;
                parent_node->child_ptr = node->next_ptr;
                /*
                 * Check if former parent is now childless
                 */
                pc = GetContainerConstraint(parent_node->widget_ptr);
                if ((parent_node->child_ptr == NULL) &&
                    (pc->outline_button != NULL))
                        {
                        XtDestroyWidget(pc->outline_button);
                        pc->outline_button = NULL;
                        }
                }

        /*
         * Unlink node from the linked list.
         */
        if (node->prev_ptr != NULL)
                {
                prev_node = node->prev_ptr;
                prev_node->next_ptr = node->next_ptr;
                }
        if (node->next_ptr != NULL)
                {
                next_node = node->next_ptr;
                next_node->prev_ptr = node->prev_ptr;
                }
}

/************************************************************************
 * DeleteNode
 ************************************************************************/
static  void
#ifdef  _NO_PROTO
DeleteNode(cwid)
        Widget  cwid;
#else
DeleteNode(
        Widget  cwid)
#endif /* _NO_PROTO */
{
        XmContainerConstraint   c = GetContainerConstraint(cwid);
        CwidNode        target_node;
        CwidNode        child_node;
        CwidNode        target_child;

#ifdef DEBUG
printf("DeleteNode\n");
#endif

        if ((target_node = c->node_ptr) == NULL) return;

        /*
         * Delete any children first.
         */
        child_node = target_node->child_ptr;
        while (child_node != NULL)
                {
                target_child = child_node;
                child_node = child_node->next_ptr;
                DeleteNode(target_child->widget_ptr);
                }
        SeverNode(target_node);
        XtFree((XtPointer)target_node);
        c->node_ptr = NULL;
        c->visible_in_outline = False;
}

/************************************************************************
 * GetNextNode
 ************************************************************************/
static  CwidNode
#ifdef _NO_PROTO
GetNextNode(wid,start_node)
        Widget          wid;
        CwidNode        start_node;
#else
GetNextNode(
        Widget          wid,
        CwidNode        start_node)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        XmContainerConstraint   c;
        CwidNode                node;

        node = start_node;
        if (!CtrLayoutIsOUTLINE(cw))
                {
                while (node != NULL)
                        {
                        if (node->next_ptr == NULL)
                                return(NULL);
                        node = node->next_ptr;
                        if XtIsManaged(node->widget_ptr)
                                return(node);
                        }
                return(NULL);
                }

        while (node != NULL)            /* depth-first search of tree */
                {
                if (node->child_ptr != NULL)
                        node = node->child_ptr;
                else
                        if (node->next_ptr != NULL)
                                node = node->next_ptr;
                        else
                                node = GetNextUpLevelNode(node);
                if (node != NULL)
                        {
                        c = GetContainerConstraint(node->widget_ptr);
                        if ((c->visible_in_outline) &&
                            XtIsManaged(node->widget_ptr))
                                return(node);
                        else
                                return(GetNextUpLevelNode(node));
                        }
                }
        return(NULL);
}

/************************************************************************
 * GetNextUpLevelNode
 ************************************************************************/
static  CwidNode
#ifdef _NO_PROTO
GetNextUpLevelNode(start_node)
        CwidNode        start_node;
#else
GetNextUpLevelNode(
        CwidNode        start_node)
#endif /* _NO_PROTO */
{
        CwidNode        node;

        node = start_node;
        while (node != NULL)
                {
                node = node->parent_ptr;
                if (node != NULL)
                        if (node->next_ptr != NULL)
                                return(node->next_ptr);
                }
        return(NULL);
}

/************************************************************************
 * GetPrevNode
 ************************************************************************/
static  CwidNode
#ifdef _NO_PROTO
GetPrevNode(wid,start_node)
        Widget          wid;
        CwidNode        start_node;
#else
GetPrevNode(
        Widget          wid,
        CwidNode        start_node)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        CwidNode        node;
        CwidNode        next_node;

        if (CtrLayoutIsSPATIAL(cw))
                return(start_node->prev_ptr);

        node = start_node;
        while (node != NULL)
                {
                if (node->prev_ptr != NULL)
                        node = node->prev_ptr;
                else
                        if (node->parent_ptr != NULL)
                                node = node->parent_ptr;
                        else
                                return(NULL);
                next_node = node;
                while (node != NULL)
                        {
                        next_node = GetNextNode((Widget)cw,node);
                        if (next_node == start_node)
                                return(node);
                        else
                                node = next_node;
                        }
                }
        return(NULL);
}

/************************************************************************
 * StartSelect
 ************************************************************************/
static  void
#ifdef _NO_PROTO
StartSelect(wid,event,params,num_params)
        Widget          wid;
        XEvent          *event;
        String          *params;        /* unused */
        Cardinal        *num_params;    /* unused */
#else
StartSelect(
        Widget          wid,
        XEvent          *event,
        String          *params,        /* unused */
        Cardinal        *num_params)    /* unused */
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        Time                    click_time;
        int                     multi_click_time;
        Widget                  current_cwid;
        XmContainerConstraint   c;

        multi_click_time = XtGetMultiClickTime(XtDisplay(wid));
        click_time = event->xbutton.time;
        if ((click_time - cw->container.last_click_time) < multi_click_time)
                {
                cw->container.last_click_time = click_time;
                if (cw->container.anchor_cwid != NULL)
                        CallActionCB(cw->container.anchor_cwid,event);
                return;
                }
        cw->container.last_click_time = click_time;

        cw->container.no_auto_sel_changes = False;
        current_cwid = LocatePointer(wid,event->xbutton.x,event->xbutton.y);
        if (CtrPolicyIsSINGLE(cw))
                {
                if (current_cwid == cw->container.anchor_cwid)
                        return;
                cw->container.no_auto_sel_changes |= DeselectAllCwids(wid);
                cw->container.anchor_cwid = current_cwid;
                if (cw->container.anchor_cwid == NULL)
                        return;
                cw->container.no_auto_sel_changes |=
                                MarkCwid(cw->container.anchor_cwid,False);
                SetLocationCursor(cw->container.anchor_cwid);
                return;
                }
        if (CtrPolicyIsBROWSE(cw))
                {
                if (current_cwid != cw->container.anchor_cwid)
                        {
                        cw->container.no_auto_sel_changes |=
                                        DeselectAllCwids(wid);
                        cw->container.anchor_cwid = current_cwid;
                        if (cw->container.anchor_cwid != NULL)
                                {
                                cw->container.no_auto_sel_changes |=
                                        MarkCwid(cw->container.anchor_cwid,
                                                        True);
                                SetLocationCursor(cw->container.anchor_cwid);
                                }
                        }
                if (CtrIsAUTO_SELECT(cw))
                        CallSelectCB(wid,event,XmAUTO_BEGIN);
                return;
                }
        if (!cw->container.extending_mode)
                cw->container.no_auto_sel_changes |= DeselectAllCwids(wid);
        cw->container.anchor_cwid = current_cwid;
        if (cw->container.anchor_cwid != NULL)
                SetLocationCursor(cw->container.anchor_cwid);
        cw->container.anchor_point.x = event->xbutton.x;
        cw->container.marquee_smallest.x = event->xbutton.x;
        cw->container.marquee_largest.x = event->xbutton.x;
        cw->container.anchor_point.y = event->xbutton.y;
        cw->container.marquee_smallest.y = event->xbutton.y;
        cw->container.marquee_largest.y = event->xbutton.y;
        if (CtrTechIsTOUCH_OVER(cw))
                if (cw->container.anchor_cwid == NULL)
                        cw->container.marquee_mode = True;
                else
                        cw->container.marquee_mode = False;
        if (cw->container.anchor_cwid == NULL)
                {
                if (CtrIsAUTO_SELECT(cw))
                        CallSelectCB(wid,event,XmAUTO_BEGIN);
                cw->container.started_in_anchor = False;
                return;
                }
        else
                cw->container.started_in_anchor = True;
        if (cw->container.extending_mode)
                {
                c = GetContainerConstraint(cw->container.anchor_cwid);
                if (c->selection_state == XmSELECTED)
                        cw->container.selection_state = XmNOT_SELECTED;
                else
                        cw->container.selection_state = XmSELECTED;
                }
        cw->container.no_auto_sel_changes |=
                        MarkCwid(cw->container.anchor_cwid,True);
        if (CtrIsAUTO_SELECT(cw))
                CallSelectCB(wid,event,XmAUTO_BEGIN);
        if ((CtrTechIsMARQUEE_ES(cw) || CtrTechIsMARQUEE_EB(cw)) &&
            (!CtrViewIsDETAIL(cw)))
                {
                RecalcMarquee(wid,cw->container.anchor_cwid,
                                event->xbutton.x,event->xbutton.y);
                DrawMarquee(wid);
                cw->container.marquee_drawn = True;
                }
}

/************************************************************************
 * ProcessButtonMotion
 ************************************************************************/
static  Boolean
#ifdef _NO_PROTO
ProcessButtonMotion(wid,event,params,num_params)
        Widget          wid;
        XEvent          *event;
        String          *params;        /* unused */
        Cardinal        *num_params;    /* unused */
#else
ProcessButtonMotion(
        Widget          wid,
        XEvent          *event,
        String          *params,        /* unused */
        Cardinal        *num_params)    /* unused */
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        Widget                  current_cwid;
        Boolean                 selection_changes = False;
        Boolean                 find_anchor = False;
        XmContainerConstraint   c;

        current_cwid = LocatePointer(wid,event->xbutton.x,event->xbutton.y);
        if (CtrPolicyIsBROWSE(cw))
                {
                if (current_cwid == cw->container.anchor_cwid)
                        return(False);
                if (cw->container.anchor_cwid != NULL)
                        {
                        cw->container.selection_state = XmNOT_SELECTED;
                        selection_changes = MarkCwid(cw->container.anchor_cwid,
                                                        False);
                        cw->container.selection_state = XmSELECTED;
                        }
                if (current_cwid != NULL)
                        selection_changes |= MarkCwid(current_cwid,True);
                cw->container.anchor_cwid = current_cwid;
                return(selection_changes);
                }
        /* CtrPolicyIsMULTIPLE || CtrPolicyIsEXTENDED */
        /* CUA Random Selection Technique */
        if (!CtrViewIsDETAIL(cw) && CtrLayoutIsSPATIAL(cw) &&
            (!cw->container.marquee_mode))
                {
                if (current_cwid== NULL)
                        return(False);
                if (cw->container.anchor_cwid == NULL)
                        {
                        /* Gotta have a reference point for toggling */
                        c = GetContainerConstraint(current_cwid);
                        if (c->selection_visual == XmSELECTED)
                                cw->container.selection_state = XmNOT_SELECTED;
                        else
                                cw->container.selection_state = XmSELECTED;
                        cw->container.anchor_cwid = current_cwid;
                        }
                return(MarkCwid(current_cwid,True));
                }
        /* CUA Range Selection Technique */
        if (CtrViewIsDETAIL(cw) ||
           (CtrLayoutIsOUTLINE(cw) && !cw->container.marquee_mode))
                return(MarkCwidsInRange(wid,cw->container.anchor_cwid,
                                        current_cwid,(Boolean)True));
        /* CUA Marquee Selection Technique */
        find_anchor = (cw->container.anchor_cwid == NULL);
        RecalcMarquee(wid,current_cwid,event->xbutton.x,event->xbutton.y);
        selection_changes = MarkCwidsInMarquee(wid,find_anchor,True);
        DrawMarquee(wid);
        cw->container.marquee_drawn = True;
        return(selection_changes);
}

/************************************************************************
 * LocatePointer
 ************************************************************************/
static  Widget
#ifdef  _NO_PROTO
LocatePointer(wid,x,y)
        Widget  wid;
        int     x;
        int     y;
#else
LocatePointer(
        Widget  wid,
        int     x,
        int     y)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        CwidNode                node;
        XmContainerConstraint   c;
        Widget                  prev_cwid;
        XmGadget                g;

        if (CtrViewIsDETAIL(cw) && (cw->container.detail_count != 0))
                {
                int                     x1,x2;

                x1 = cw->container.margin_w;
                x2 = cw->container.ideal_width - cw->container.margin_w;
                if CtrLayoutIsLtoR(cw)
                        x2 = MIN(x2,cw->core.width);
                if CtrLayoutIsRtoL(cw)
                        {
                        x1 = MAX(0,cw->core.width - x2);
                        x2 = cw->core.width - cw->container.margin_w;
                        }
                if ((x < x1) || (x2 < x))
                        return(NULL);
                node = cw->container.first_node;
                prev_cwid = NULL;
                while (node != NULL)
                        {
                        c = GetContainerConstraint(node->widget_ptr);
                        if (y < c->row_y)
                                return(prev_cwid);
                        if (y <= c->row_y + c->row_height)
                                return(node->widget_ptr);
                        if (c->row_y + c->row_height > cw->core.height)
                                return(NULL);
                        prev_cwid = node->widget_ptr;
                        node = GetNextNode(wid,node);
                        }
                return(NULL);
                }
        node = cw->container.first_node;
        prev_cwid = NULL;
        while (node != NULL)
                {
                c = GetContainerConstraint(node->widget_ptr);
                g = (XmGadget)node->widget_ptr;
                if ((y < g->rectangle.y) && CtrLayoutIsOUTLINE(cw))
                        return(prev_cwid);
                if ((g->rectangle.x <= x) &&
                    (x <= g->rectangle.x + g->rectangle.width) &&
                    (g->rectangle.y <= y) &&
                    (y <= g->rectangle.y + g->rectangle.height))
                        return(node->widget_ptr);
                if ((g->rectangle.y + g->rectangle.height > cw->core.height) &&
                    CtrLayoutIsOUTLINE(cw))
                        return(NULL);
                if ((g->rectangle.x <= x) &&
                    (x <= g->rectangle.x + g->rectangle.width))
                        prev_cwid = node->widget_ptr;
                else
                        prev_cwid = NULL;
                node = GetNextNode(wid,node);
                }
        return(NULL);
}

/************************************************************************
 * SelectAllCwids
 ************************************************************************/
static  Boolean
#ifdef  _NO_PROTO
SelectAllCwids(wid)
        Widget  wid;
#else
SelectAllCwids(
        Widget  wid)
#endif  /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        CwidNode                node;
        Boolean                 selection_changes = False;

        /*
         * Mark all visible cwids as XmSELECTED
         */
        cw->container.selection_state = XmSELECTED;
        node = cw->container.first_node;
        while (node != NULL)
                {
                selection_changes |= MarkCwid(node->widget_ptr,False);
                node = GetNextNode(wid,node);
                }
        return(selection_changes);
}

/************************************************************************
 * DeselectAllCwids
 ************************************************************************/
static  Boolean
#ifdef  _NO_PROTO
DeselectAllCwids(wid)
        Widget  wid;
#else
DeselectAllCwids(
        Widget  wid)
#endif  /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        CwidNode                node;
        Boolean                 selection_changes = False;

        if (cw->container.selected_item_count == 0)
                {
                cw->container.selection_state = XmSELECTED;
                return(False);
                }

        /*
         * Mark all visible cwids as XmNOT_SELECTED
         */
        cw->container.selection_state = XmNOT_SELECTED;
        node = cw->container.first_node;
        while (node != NULL)
                {
                selection_changes |= MarkCwid(node->widget_ptr,False);
                node = GetNextNode(wid,node);
                if (cw->container.selected_item_count == 0)
                        {
                        cw->container.selection_state = XmSELECTED;
                        return(selection_changes);
                        }
                }
#ifdef  DEBUG
printf("DeselectAllCwids searching for hidden XmSELECTED cwids\n");
#endif  /* DEBUG */
        /*
         * Didn't work.  There must be some hidden child cwids that are
         * XmSELECTED.  We'll have to traverse the entire list.
         */
        node = cw->container.first_node;
        while (node != NULL)
                {
                selection_changes |= MarkCwid(node->widget_ptr,False);
                if (cw->container.selected_item_count == 0)
                        {
                        cw->container.selection_state = XmSELECTED;
                        return(selection_changes);
                        }
                /*
                 * depth-first search of tree, whether the cwids are
                 * visible or not.
                 */
                if (node->child_ptr != NULL)
                        node = node->child_ptr;
                else
                        if (node->next_ptr != NULL)
                                node = node->next_ptr;
                        else
                                node = GetNextUpLevelNode(node);
                }
        cw->container.selection_state = XmSELECTED;
        return(selection_changes);
}

/************************************************************************
 * MarkCwid
 ************************************************************************/
static  Boolean
#ifdef  _NO_PROTO
MarkCwid(cwid,visual_only)
        Widget          cwid;
        Boolean         visual_only;
#else
MarkCwid(
        Widget          cwid,
        Boolean         visual_only)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)XtParent(cwid);
        XmContainerConstraint   c = GetContainerConstraint(cwid);
        Boolean                 selection_changes = False;

        if (cw->container.selection_state != c->selection_visual)
                {
                c->selection_visual = cw->container.selection_state;
                cw->container.self = True;
#ifdef  COSEICON
                _DtIconSetState(cwid,(c->selection_visual == XmSELECTED),True);
#else
                XtVaSetValues(cwid,XmNvisualEmphasis,c->selection_visual,NULL);
#endif  /* COSEICON */
                cw->container.self = False;
                if CtrViewIsDETAIL(cw)
                        ExposeDetail(cwid);
                if (c->selection_visual == XmSELECTED)
                        cw->container.selected_item_count++;
                else
                        cw->container.selected_item_count--;
                selection_changes = True;
                }
        if (!visual_only)
                c->selection_state = c->selection_visual;
        return(selection_changes);
}

/************************************************************************
 * UnmarkCwidVisual
 ************************************************************************/
static  Boolean
#ifdef  _NO_PROTO
UnmarkCwidVisual(cwid)
        Widget          cwid;
#else
UnmarkCwidVisual(
        Widget          cwid)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)XtParent(cwid);
        XmContainerConstraint   c = GetContainerConstraint(cwid);

        /*
         * If the item visual matches its select state, or the item
         * visual doesn't match our current select/unselect action,
         * then return.
         * Otherwise, restore the item's visual to its select state
         * and add to/remove from the selected items list.
         */
        if ((c->selection_visual == c->selection_state) ||
            (c->selection_visual != cw->container.selection_state))
                return(False);
        c->selection_visual = c->selection_state;
        cw->container.self = True;
#ifdef  COSEICON
        _DtIconSetState(cwid,(c->selection_visual == XmSELECTED),True);
#else
        XtVaSetValues(cwid,XmNvisualEmphasis,c->selection_visual,NULL);
#endif  /* COSEICON */
        cw->container.self = False;
        if CtrViewIsDETAIL(cw)
                ExposeDetail(cwid);
        if (c->selection_visual == XmSELECTED)
                        cw->container.selected_item_count++;
                else
                        cw->container.selected_item_count--;
        return(True);
}

/************************************************************************
 * SetMarkedCwids
 ************************************************************************/
static  void
#ifdef  _NO_PROTO
SetMarkedCwids(wid)
        Widget  wid;
#else
SetMarkedCwids(
        Widget  wid)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        CwidNode                node;
        XmContainerConstraint   c;

        node = cw->container.first_node;
        while (node != NULL)
                {
                c = GetContainerConstraint(node->widget_ptr);
                c->selection_state = c->selection_visual;
                node = GetNextNode(wid,node);
                }
}

/************************************************************************
 * ResetMarkedCwids
 ************************************************************************/
static  Boolean
#ifdef  _NO_PROTO
ResetMarkedCwids(wid)
        Widget  wid;
#else
ResetMarkedCwids(
        Widget  wid)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        CwidNode                node;
        Boolean                 selection_changes = False;

        node = cw->container.first_node;
        while (node != NULL)
                {
                selection_changes |= UnmarkCwidVisual(node->widget_ptr);
                node = GetNextNode(wid,node);
                }
        return(selection_changes);
}

/************************************************************************
 * MarkCwidsInRange
 ************************************************************************/
static  Boolean
#ifdef  _NO_PROTO
MarkCwidsInRange(wid,cwid1,cwid2,visual_only)
        Widget  wid;
        Widget  cwid1;
        Widget  cwid2;
        Boolean visual_only;
#else
MarkCwidsInRange(
        Widget  wid,
        Widget  cwid1,
        Widget  cwid2,
        Boolean visual_only)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        CwidNode                node;
        Boolean                 cwid1_found = False;
        Boolean                 cwid2_found = False;
        Boolean                 marking_started = False;
        Boolean                 marking_ended = False;
        Boolean                 selection_changes = False;

        node = cw->container.first_node;
        if (cwid1 == NULL)
                if ((cwid1 = cwid2) == NULL) return(False);
        if (cwid2 == NULL)
                if ((cwid2 = cwid1) == NULL) return(False);
        while (node != NULL)
                {
                if (node->widget_ptr == cwid1)
                        cwid1_found = True;
                if (node->widget_ptr == cwid2)
                        cwid2_found = True;
                if ((cwid1_found || cwid2_found) && (!marking_started))
                        marking_started = True;
                if (marking_started && (!marking_ended))
                        selection_changes |= MarkCwid(node->widget_ptr,
                                                visual_only);
                else
                        selection_changes |= UnmarkCwidVisual(node->widget_ptr);
                if (cwid1_found && cwid2_found && marking_started)
                        marking_ended = True;
                node = GetNextNode((Widget)cw,node);
                }
        return(selection_changes);
}

/************************************************************************
 * MarkCwidsInMarquee
 ************************************************************************/
static  Boolean
#ifdef  _NO_PROTO
MarkCwidsInMarquee(wid,find_anchor,visual_only)
        Widget  wid;
        Boolean find_anchor;
        Boolean visual_only;
#else
MarkCwidsInMarquee(
        Widget  wid,
        Boolean find_anchor,
        Boolean visual_only)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        CwidNode                node;
        Boolean                 selection_changes = False;
        XmContainerConstraint   c;

        node = cw->container.first_node;
        while (node != NULL)
                {
                if (InMarquee(node->widget_ptr))
                        {
                        if (find_anchor)
                                {
                                cw->container.anchor_cwid = node->widget_ptr;
                                c = GetContainerConstraint(node->widget_ptr);
                                if (c->selection_state == XmSELECTED)
                                        cw->container.selection_state
                                                        = XmNOT_SELECTED;
                                else
                                        cw->container.selection_state
                                                        = XmSELECTED;
                                find_anchor = False;
                                }
                        selection_changes |= MarkCwid(node->widget_ptr,
                                                visual_only);
                        }
                else
                        selection_changes |= UnmarkCwidVisual(node->widget_ptr);
                node = GetNextNode(wid,node);
                }
        return(selection_changes);
}

/************************************************************************
 * InMarquee
 ************************************************************************/
static  Boolean
#ifdef _NO_PROTO
InMarquee(cwid)
        Widget  cwid;
#else
InMarquee(
        Widget  cwid)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)XtParent(cwid);
        Position                cwid_x,cwid_y,cwid_xw,cwid_yh;
        Dimension               width,height;

        XtVaGetValues(cwid,XmNx,&cwid_x,XmNy,&cwid_y,
                XmNwidth,&width,XmNheight,&height,NULL);
        cwid_xw = cwid_x + (Position)width;
        cwid_yh = cwid_y + (Position)height;
        return( (cwid_x >= cw->container.marquee_start.x) &&
                (cwid_y >=  cw->container.marquee_start.y) &&
                (cwid_xw <= cw->container.marquee_end.x) &&
                (cwid_yh <= cw->container.marquee_end.y));
}

/************************************************************************
 * RecalcMarquee
 ************************************************************************/
static  void
#ifdef _NO_PROTO
RecalcMarquee(wid,cwid,x,y)
        Widget          wid;
        Widget          cwid;
        Position        x;
        Position        y;
#else
RecalcMarquee(
        Widget          wid,
        Widget          cwid,
        Position        x,
        Position        y)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        Dimension               width,height;
        Position                anchor_x,anchor_y,anchor_xw,anchor_yh;
        Position                cwid_x,cwid_y,cwid_xw,cwid_yh;

        /*
         * Erase any marquee that is currently drawn.
         */
        if (cw->container.marquee_drawn)
                {
                DrawMarquee(wid);
                cw->container.marquee_drawn = False;
                }
        /*
         * Set the new marquee start and end points depending upon
         * the XmNselectionTechnique resource.
         * If XmMARQUEE_EXTEND_BOTH, use the anchor_cwid and current_cwid
         *      corners as endpoints (if they are given).
         * If XmMARQUEE_EXTEND_START, use the anchor_cwid corners (if given)
         *      and the current pointer position as endpoints.
         * If XmMARQUEE, use the original anchor points and the current
         *      pointer position, given by the x & y parameters.
         * container.marquee_start.x & marquee_start.y always marks upper left.
         * container.marquee_end.x & marquee_end.y always marks lower right.
         */
        switch(cw->container.selection_technique)
        {
        case    XmMARQUEE_EXTEND_BOTH:
                        if ((cw->container.started_in_anchor) && (cwid != NULL))
                                {
                                XtVaGetValues(cw->container.anchor_cwid,
                                        XmNx,&anchor_x,XmNy,&anchor_y,
                                        XmNwidth,&width,
                                        XmNheight,&height,NULL);
                                anchor_xw = anchor_x + (Position)width;
                                anchor_yh = anchor_y + (Position)height;
                                XtVaGetValues(cwid,XmNx,&cwid_x,XmNy,&cwid_y,
                                        XmNwidth,&width,
                                        XmNheight,&height,NULL);
                                cwid_xw = cwid_x + (Position)width;
                                cwid_yh = cwid_y + (Position)height;
                                cw->container.marquee_start.x =
                                        MIN(anchor_x,cwid_x);
                                cw->container.marquee_start.y =
                                        MIN(anchor_y,cwid_y);
                                cw->container.marquee_end.x =
                                        MAX(anchor_xw,cwid_xw);
                                cw->container.marquee_end.y =
                                        MAX(anchor_yh,cwid_yh);
                                break;
                                }
                        if (cw->container.started_in_anchor)
                                cwid = cw->container.anchor_cwid;
                        if ((!cw->container.started_in_anchor) &&
                            (cwid != NULL))
                                {
                                x = cw->container.anchor_point.x;
                                y = cw->container.anchor_point.y;
                                }
        case    XmMARQUEE_EXTEND_START:
                        if CtrTechIsMARQUEE_ES(cw)
                                if (cw->container.started_in_anchor)
                                        cwid = cw->container.anchor_cwid;
                                else
                                        cwid = NULL;
                        if (cwid != NULL)
                                {
                                XtVaGetValues(cwid,XmNx,&cwid_x,
                                        XmNy,&cwid_y,XmNwidth,&width,
                                        XmNheight,&height,NULL);
                                cwid_xw = cwid_x + (Position)width;
                                cwid_yh = cwid_y + (Position)height;
                                if (x <= cwid_xw)
                                        {
                                        cw->container.marquee_start.x =
                                                MIN(x,cwid_x);
                                        cw->container.marquee_end.x = cwid_xw;
                                        }
                                else
                                        {
                                        cw->container.marquee_start.x = cwid_x;
                                        cw->container.marquee_end.x =
                                                MAX(x,cwid_xw);
                                        }
                                if (y <= cwid_yh)
                                        {
                                        cw->container.marquee_start.y =
                                                MIN(y,cwid_y);
                                        cw->container.marquee_end.y = cwid_yh;
                                        }
                                else
                                        {
                                        cw->container.marquee_start.y = cwid_y;
                                        cw->container.marquee_end.y =
                                                MAX(y,cwid_yh);
                                        }
                                break;
                                }
        case    XmMARQUEE:
        case    XmTOUCH_OVER:
                        cw->container.marquee_start.x =
                                MIN(x,cw->container.anchor_point.x);
                        cw->container.marquee_start.y =
                                MIN(y,cw->container.anchor_point.y);
                        cw->container.marquee_end.x =
                                MAX(x,cw->container.anchor_point.x);
                        cw->container.marquee_end.y =
                                MAX(y,cw->container.anchor_point.y);
                        break;
        default:
                        return;
        }
        /*
         * Keep track of the largest area covered by the marquee.
         */
        cw->container.marquee_smallest.x = MIN(cw->container.marquee_start.x,
                                cw->container.marquee_smallest.x);
        cw->container.marquee_smallest.y = MIN(cw->container.marquee_start.y,
                                cw->container.marquee_smallest.y);
        cw->container.marquee_largest.x = MAX(cw->container.marquee_end.x,
                                cw->container.marquee_largest.x);
        cw->container.marquee_largest.y = MAX(cw->container.marquee_end.y,
                                cw->container.marquee_largest.y);
}

/************************************************************************
 * DrawMarquee
 ************************************************************************/
static  void
#ifdef _NO_PROTO
DrawMarquee(wid)
        Widget          wid;
#else
DrawMarquee(
        Widget          wid)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        Dimension               width,height;

        /*
         * If the container widget is realized, draw the marquee rectangle
         * using the gc parameter.
         */
        if (!XtIsRealized(wid))
                return;
        width = (Dimension)(cw->container.marquee_end.x
                                - cw->container.marquee_start.x);
        height = (Dimension)(cw->container.marquee_end.y
                                - cw->container.marquee_start.y);
        XDrawRectangle(XtDisplay(wid),XtWindow(wid),cw->container.marqueeGC,
                cw->container.marquee_start.x,cw->container.marquee_start.y,
                width,height);
}

/************************************************************************
 * KBSelect
 ************************************************************************/
static  void
#ifdef _NO_PROTO
KBSelect(wid,event,params,num_params)
        Widget          wid;
        XEvent          *event;
        String          *params;        /* unused */
        Cardinal        *num_params;    /* unused */
#else
KBSelect(
        Widget          wid,
        XEvent          *event,
        String          *params,        /* unused */
        Cardinal        *num_params)    /* unused */
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        XmContainerConstraint   c;

        if (cw->container.loc_cursor_cwid == NULL)
                return;
        cw->container.no_auto_sel_changes = False;
        c = GetContainerConstraint(cw->container.loc_cursor_cwid);
        cw->container.anchor_cwid = cw->container.loc_cursor_cwid;
        if ((!cw->container.extending_mode || CtrPolicyIsSINGLE(cw)) &&
            ((cw->container.selected_item_count > 1) ||
             (c->selection_state != XmSELECTED)))
                cw->container.no_auto_sel_changes |= DeselectAllCwids(wid);
        if (cw->container.extending_mode)
                {
                if (c->selection_state == XmSELECTED)
                        cw->container.selection_state = XmNOT_SELECTED;
                else
                        cw->container.selection_state = XmSELECTED;
                }
        cw->container.no_auto_sel_changes |=
                        MarkCwid(cw->container.loc_cursor_cwid,False);
        if (CtrIsAUTO_SELECT(cw))
                {
                CallSelectCB(wid,event,XmAUTO_BEGIN);
                CallSelectCB(wid,event,XmAUTO_NO_CHANGE);
                }
        else
                if (cw->container.no_auto_sel_changes)
                        CallSelectCB(wid,event,XmAUTO_UNSET);
}

/************************************************************************
 * SetLocationCursor
 ************************************************************************/
static  void
#ifdef  _NO_PROTO
SetLocationCursor(cwid)
        Widget  cwid;
#else
SetLocationCursor(
        Widget  cwid)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw;

        if (cwid == NULL)
                return;
        cw = (XmContainerWidget)XtParent(cwid);
        if (cwid == cw->container.loc_cursor_cwid)
                return;
        BorderUnhighlight(cw->container.loc_cursor_cwid);
        cw->container.loc_cursor_cwid = cwid;
        BorderHighlight(cw->container.loc_cursor_cwid);
}

/************************************************************************
 * BorderHighlight
 ************************************************************************/
static  void
#ifdef  _NO_PROTO
BorderHighlight(cwid)
        Widget  cwid;
#else
BorderHighlight(
        Widget  cwid)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw;
        XmContainerConstraint   c;
        XmGadget                g = (XmGadget)cwid;
        XRectangle              hrect;

        if (cwid == NULL)
                return;
        cw = (XmContainerWidget)XtParent(cwid);
        if (!cw->container.have_focus)
                return;
        c = GetContainerConstraint(cwid);
        if (c->container_created)
                {
                (*(((XmPrimitiveWidgetClass) XtClass(cwid))
                        ->primitive_class.border_highlight))(cwid);
                return;
                }
        if CtrViewIsDETAIL(cw)
                {
                hrect.x = cw->container.margin_w;
                hrect.width = cw->container.ideal_width
                                - 2 * cw->container.margin_w;
                if (cw->core.width < hrect.x + hrect.width)
                        hrect.width = cw->core.width - hrect.x;
                if CtrLayoutIsRtoL(cw)
                        hrect.x = MAX(0,cw->core.width - hrect.width
                                        - cw->container.margin_w);
                hrect.y = c->row_y;
                hrect.height = c->row_height;
                }
        else
                {
                hrect.x = g->rectangle.x;
                hrect.y = g->rectangle.y;
                hrect.width = g->rectangle.width;
                hrect.height = g->rectangle.height;
                }
        g = (XmGadget)cwid;
        _XmDrawHighlight(XtDisplay(cwid),XtWindow(cwid),
                ((XmManagerWidget)(g->object.parent))->manager.highlight_GC,
                hrect.x,hrect.y,hrect.width,hrect.height,
                g->gadget.highlight_thickness,
                (cw->container.kaddmode) ? LineOnOffDash : LineSolid);
}

/************************************************************************
 * BorderUnhighlight
 ************************************************************************/
static  void
#ifdef  _NO_PROTO
BorderUnhighlight(cwid)
        Widget  cwid;
#else
BorderUnhighlight(
        Widget  cwid)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw;
        XmContainerConstraint   c;
        XmGadget                g = (XmGadget)cwid;
        XRectangle              hrect;

        if (cwid == NULL)
                return;
        cw = (XmContainerWidget)XtParent(cwid);
        c = GetContainerConstraint(cwid);
        if (c->container_created)
                {
                (*(((XmPrimitiveWidgetClass) XtClass(cwid))
                        ->primitive_class.border_unhighlight))(cwid);
                /*
                (*(((XmGadgetClass) XtClass(cwid))
                        ->gadget_class.border_highlight))(cwid);
                 */
                return;
                }
        if CtrViewIsDETAIL(cw)
                {
                hrect.x = cw->container.margin_w;
                hrect.width = cw->container.ideal_width
                                - 2 * cw->container.margin_w;
                if (cw->core.width < hrect.x + hrect.width)
                        hrect.width = cw->core.width - hrect.x;
                if CtrLayoutIsRtoL(cw)
                        hrect.x = MAX(0,cw->core.width - hrect.width
                                        - cw->container.margin_w);
                hrect.y = c->row_y;
                hrect.height = c->row_height;
                }
        else
                {
                hrect.x = g->rectangle.x;
                hrect.y = g->rectangle.y;
                hrect.width = g->rectangle.width;
                hrect.height = g->rectangle.height;
                }
        _XmDrawHighlight(XtDisplay(cwid),XtWindow(cwid),
                ((XmManagerWidget)(g->object.parent))->manager.background_GC,
                hrect.x,hrect.y,hrect.width,hrect.height,
                g->gadget.highlight_thickness,
                (cw->container.kaddmode) ? LineOnOffDash : LineSolid);
}

/************************************************************************
 * CalcNextLocationCursor
 ************************************************************************/
static  Widget
#ifdef  _NO_PROTO
CalcNextLocationCursor(wid,direction)
        Widget  wid;
        String  direction;
#else
CalcNextLocationCursor(
        Widget  wid,
        String  direction)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        XmContainerConstraint   c;
        CwidNode                newloc_node = NULL;

        if (cw->container.loc_cursor_cwid == NULL)
                cw->container.loc_cursor_cwid =
                        cw->container.first_node->widget_ptr;

        if (strcmp(direction,_FIRST) == 0)
                return(cw->container.first_node->widget_ptr);
        if (strcmp(direction,_LAST) == 0)
                return(cw->container.last_node->widget_ptr);

        c = GetContainerConstraint(cw->container.loc_cursor_cwid);
        if (c->container_created)
                {
                if (CtrLayoutIsOUTLINE(cw) && CtrLayoutIsRtoL(cw) &&
                    (strcmp(direction,_LEFT) == 0))
                        return(c->node_ptr->widget_ptr);
                if (CtrLayoutIsOUTLINE(cw) && CtrLayoutIsLtoR(cw) &&
                    (strcmp(direction,_RIGHT) == 0))
                        return(c->node_ptr->widget_ptr);
                SetLocationCursor(c->node_ptr->widget_ptr);
                c = GetContainerConstraint(cw->container.loc_cursor_cwid);
                }

        if (strcmp(direction,_RIGHT) == 0)
                {
                if (CtrLayoutIsOUTLINE(cw) && CtrLayoutIsRtoL(cw) &&
                    (c->outline_button != NULL))
                        return(c->outline_button);
                if ((CtrViewIsDETAIL(cw)) || (CtrLayoutIsOUTLINE(cw)))
                        return(cw->container.loc_cursor_cwid);
                if (CtrLayoutIsLtoR(cw))
                        newloc_node = GetNextNode((Widget)cw,c->node_ptr);
                else
                        newloc_node = GetPrevNode((Widget)cw,c->node_ptr);
                }
        if (strcmp(direction,_LEFT) == 0)
                {
                if (CtrLayoutIsOUTLINE(cw) && CtrLayoutIsLtoR(cw) &&
                   (c->outline_button != NULL))
                        return(c->outline_button);
                if ((CtrViewIsDETAIL(cw)) || (CtrLayoutIsOUTLINE(cw)))
                        return(cw->container.loc_cursor_cwid);
                if (CtrLayoutIsLtoR(cw))
                        newloc_node = GetPrevNode((Widget)cw,c->node_ptr);
                else
                        newloc_node = GetNextNode((Widget)cw,c->node_ptr);
                }
        if (strcmp(direction,_DOWN) == 0)
                {
                newloc_node = GetNextNode((Widget)cw,c->node_ptr);
                if (newloc_node == NULL)
                        newloc_node = cw->container.first_node;
                }
        if (strcmp(direction,_UP) == 0)
                {
                newloc_node = GetPrevNode((Widget)cw,c->node_ptr);
                if (newloc_node == NULL)
                        newloc_node = cw->container.last_node;
                }
        if (newloc_node != NULL)
                return(newloc_node->widget_ptr);
        return(cw->container.loc_cursor_cwid);
}

/************************************************************************
 * MakeOutlineButton
 ************************************************************************/
static  void
#ifdef _NO_PROTO
MakeOutlineButton(cwid)
        Widget  cwid;
#else
MakeOutlineButton(
        Widget  cwid)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)XtParent(cwid);
        XmContainerConstraint   c = GetContainerConstraint(cwid);
        XmContainerConstraint   obc;
        Pixmap                  pm;

        if (c->outline_state == XmEXPANDED)
                pm = cw->container.expanded_state_pixmap;
        else
                pm = cw->container.collapsed_state_pixmap;
        cw->container.self = True;
        c->outline_button = XtVaCreateWidget(OBNAME,
                                xmPushButtonWidgetClass,(Widget)cw,
                                XmNtraversalOn,False,
                                XmNlabelType,XmPIXMAP,
                                XmNlabelPixmap,pm,NULL);
        XtAddCallback(c->outline_button,XmNactivateCallback,
                        (XtCallbackProc)OutlineButtonCallback,
                        (XtPointer)cwid);
        cw->container.self = False;
        obc = GetContainerConstraint(c->outline_button);
        obc->node_ptr = c->node_ptr;
        XtManageChild(c->outline_button);
}

/************************************************************************
 * ExpandCwid
 ************************************************************************/
static  void
#ifdef _NO_PROTO
ExpandCwid(cwid)
        Widget  cwid;
#else
ExpandCwid(
        Widget  cwid)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)XtParent(cwid);
        XmContainerConstraint   c = GetContainerConstraint(cwid);
        CwidNode                node;
        CwidNode                child_node;

        if (c->outline_button == NULL)
                return;
        cw->container.self = True;
        XtVaSetValues(c->outline_button,
                XmNlabelPixmap,cw->container.expanded_state_pixmap,NULL);
        cw->container.self = False;

        c->outline_state = XmEXPANDED;
        node = c->node_ptr;
        child_node = node->child_ptr;
        if (child_node == NULL)
                return;
        while (child_node != NULL)
                {
                c = GetContainerConstraint(child_node->widget_ptr);
                c->visible_in_outline = True;
                child_node = child_node->next_ptr;
                }
        Layout((Widget)cw);
        RequestNewSize((Widget)cw);
        if (XtIsRealized((Widget)cw))
            XClearArea(XtDisplay((Widget)cw),XtWindow((Widget)cw),0,0,0,0,True);
}

/************************************************************************
 * CollapseCwid
 ************************************************************************/
static  void
#ifdef _NO_PROTO
CollapseCwid(cwid)
        Widget  cwid;
#else
CollapseCwid(
        Widget  cwid)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)XtParent(cwid);
        XmContainerConstraint   c = GetContainerConstraint(cwid);
        CwidNode                node;
        CwidNode                child_node;

        if (c->outline_button == NULL)
                return;
        cw->container.self = True;
        XtVaSetValues(c->outline_button,
                XmNlabelPixmap,cw->container.collapsed_state_pixmap,NULL);
        cw->container.self = False;

        c->outline_state = XmCOLLAPSED;
        node = c->node_ptr;
        child_node = node->child_ptr;
        if (child_node == NULL)
                return;
        while (child_node != NULL)
                {
                c = GetContainerConstraint(child_node->widget_ptr);
                c->visible_in_outline = False;
                HideCwid(child_node->widget_ptr);
                if (c->outline_button != NULL)
                        HideCwid(c->outline_button);
                child_node = child_node->next_ptr;
                }
        Layout((Widget)cw);
        RequestNewSize((Widget)cw);
        if (XtIsRealized((Widget)cw))
            XClearArea(XtDisplay((Widget)cw),XtWindow((Widget)cw),0,0,0,0,True);
}

/************************************************************************
 * CallActionCB
 ************************************************************************/
static  void
#ifdef _NO_PROTO
CallActionCB(cwid,event)
        Widget  cwid;
        XEvent  *event;
#else
CallActionCB(
        Widget  cwid,
        XEvent  *event)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)XtParent(cwid);
        XmContainerConstraint   c = GetContainerConstraint(cwid);
        XmContainerSelectCallbackStruct cbs;

        if (!(XtHasCallbacks((Widget)cw,XmNdefaultActionCallback)
                                == XtCallbackHasSome))
                return;
        cbs.reason = XmCR_DEFAULT_ACTION;
        cbs.event = event;
        if (c->selection_state == XmSELECTED)
                {
                cbs.selected_items = GetSelectedCwids((Widget)cw);
                cbs.selected_item_count = cw->container.selected_item_count;
                }
        else
                {
                cbs.selected_items = (WidgetList)XtMalloc(sizeof(Widget));
                cbs.selected_items[0] = cwid;
                cbs.selected_item_count = 1;
                }
        cbs.auto_selection_type = XmAUTO_UNSET;
        XtCallCallbackList((Widget)cw,cw->container.default_action_cb,&cbs);
        XtFree((XtPointer)cbs.selected_items);
}

/************************************************************************
 * CallSelectCB
 ************************************************************************/
static  void
#ifdef _NO_PROTO
CallSelectCB(wid,event,auto_selection_type)
        Widget          wid;
        XEvent          *event;
        unsigned char   auto_selection_type;
#else
CallSelectCB(
        Widget          wid,
        XEvent          *event,
        unsigned char   auto_selection_type)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        XmContainerSelectCallbackStruct cbs;

        if (!(XtHasCallbacks(wid,DtNselectionCallback)
                                == XtCallbackHasSome))
                return;
        cbs.selected_items = NULL;
        cbs.selected_item_count = 0;

        switch(cw->container.selection_policy)
        {
        case    XmSINGLE_SELECT:
                        cbs.reason = XmCR_SINGLE_SELECT;
        case    XmBROWSE_SELECT:
                        if (CtrPolicyIsBROWSE(cw))
                                cbs.reason = XmCR_BROWSE_SELECT;
                        if (cw->container.anchor_cwid != NULL)
                                {
                                cbs.selected_items = (WidgetList)
                                                XtMalloc(sizeof(Widget));
                                cbs.selected_items[0] =
                                        cw->container.anchor_cwid;
                                cbs.selected_item_count = 1;
                                }
                        break;
        case    XmMULTIPLE_SELECT:
                        cbs.reason = XmCR_MULTIPLE_SELECT;
        case    XmEXTENDED_SELECT:
                        if (CtrPolicyIsEXTENDED(cw))
                                cbs.reason = XmCR_EXTENDED_SELECT;
                        cbs.selected_items = GetSelectedCwids(wid);
                        cbs.selected_item_count =
                                cw->container.selected_item_count;
                        break;
        }
        cbs.event = event;
        cbs.auto_selection_type = auto_selection_type;
        XtCallCallbackList(wid,cw->container.selection_cb,&cbs);
        if (cbs.selected_items != NULL)
                XtFree((XtPointer)cbs.selected_items);
}

/************************************************************************
 * GetSelectedCwids
 ************************************************************************/
static  WidgetList
#ifdef _NO_PROTO
GetSelectedCwids(wid)
        Widget          wid;
#else
GetSelectedCwids(
        Widget          wid)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        WidgetList              selected_items;
        CwidNode                node;
        unsigned int            tally = 0;
        XmContainerConstraint   c;

        if (cw->container.selected_item_count == 0)
                return(NULL);

        selected_items = (WidgetList)XtMalloc
                        (cw->container.selected_item_count * sizeof(Widget));
        /*
         * Search through all the visible items first - it'll work 99% of
         * the time and it's faster than searching through all the items.
         */
        node = cw->container.first_node;
        while (node != NULL)
                {
                c = GetContainerConstraint(node->widget_ptr);
                if (c->selection_visual == XmSELECTED)
                        {
                        selected_items[tally] = node->widget_ptr;
                        tally++;
                        if (tally == cw->container.selected_item_count)
                                return(selected_items);
                        }
                node = GetNextNode(wid,node);
                }
#ifdef  DEBUG
printf("GetSelectedCwids searching for hidden selected items\n");
#endif  /* DEBUG */
        /*
         * Didn't work.  There must be some hidden child items that are
         * XmSELECTED.  We'll have to traverse the entire list.
         */
        tally = 0;
        node = cw->container.first_node;
        while (node != NULL)
                {
                c = GetContainerConstraint(node->widget_ptr);
                if (c->selection_visual == XmSELECTED)
                        {
                        selected_items[tally] = node->widget_ptr;
                        tally++;
                        if (tally == cw->container.selected_item_count)
                                return(selected_items);
                        }
                /*
                 * depth-first search of tree, whether the items are
                 * visible or not.
                 */
                if (node->child_ptr != NULL)
                        node = node->child_ptr;
                else
                        if (node->next_ptr != NULL)
                                node = node->next_ptr;
                        else
                                node = GetNextUpLevelNode(node);
                }
        return(NULL);
}

/************************************************************************
 * GetSelectedItems
 ************************************************************************/
static  void
#ifdef _NO_PROTO
GetSelectedItems(wid,offset,value)
        Widget          wid;
        int             offset; /* unused */
        XtArgVal        *value;
#else
GetSelectedItems(
        Widget          wid,
        int             offset, /* unused */
        XtArgVal        *value)
#endif /* _NO_PROTO */
{
        WidgetList      selected_items;

        selected_items = GetSelectedCwids(wid);
        *value = (XtArgVal)selected_items;
}

/************************************************************************
 * GetDetailHeading
 ************************************************************************/
static  void
#ifdef _NO_PROTO
GetDetailHeading(wid,offset,value)
        Widget          wid;
        int             offset; /* unused */
        XtArgVal        *value;
#else
GetDetailHeading(
        Widget          wid,
        int             offset, /* unused */
        XtArgVal        *value)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        XmStringTable           xmstr_table;
        int                     i;

        xmstr_table = (XmStringTable)XtMalloc
                        (cw->container.detail_count * sizeof(XmString));
        for (i = 0; i < cw->container.detail_count; i++)
                xmstr_table[i] = _XmStringCreateExternal
                                        (cw->container.render_table,
                                         cw->container.detail_heading[i]);
        *value = (XtArgVal)xmstr_table;
}

/************************************************************************
 * GetEntryDetail
 ************************************************************************/
static  void
#ifdef _NO_PROTO
GetEntryDetail(cwid,offset,value)
        Widget          cwid;
        int             offset; /* unused */
        XtArgVal        *value;
#else
GetEntryDetail(
        Widget          cwid,
        int             offset, /* unused */
        XtArgVal        *value)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)XtParent(cwid);
        XmContainerConstraint   c = GetContainerConstraint(cwid);
        XmStringTable           xmstr_table;
        int                     i;

        xmstr_table = (XmStringTable)XtMalloc
                        (cw->container.detail_count * sizeof(XmString));
        for (i = 0; i < cw->container.detail_count; i++)
                xmstr_table[i] = _XmStringCreateExternal
                                (cw->container.render_table,c->entry_detail[i]);
        *value = (XtArgVal)xmstr_table;
}

/************************************************************************
 * CompareInts
 ************************************************************************/
static  int
#ifdef _NO_PROTO
CompareInts(p1,p2)
        const void    *p1;
        const void    *p2;
#else
CompareInts(
        const void    *p1,
        const void    *p2)
#endif /* _NO_PROTO */
{
        int     i1 = * (int *)p1;
        int     i2 = * (int *)p2;

        /*
         * A "qsort" function that simply compares two integers
         */
        return(i1 - i2);
}

/************************************************************************
 * Isqrt - Integer Square Root (using Newton's Method) - rounded up
 ************************************************************************/
static  int
#ifdef _NO_PROTO
Isqrt(n)
    int n;
#else
Isqrt(
    int n)
#endif /* _NO_PROTO */
{
    int current_answer,next_trial;

    if (n <= 1)
        return(n);
    current_answer = n;
    next_trial = n/2;
    while(current_answer > next_trial)
        {
        current_answer = next_trial;
        next_trial = (next_trial + n/next_trial)/2;
        }
    if (current_answer * current_answer < n)
        current_answer++;
    return(current_answer);
}


/*---------------------------
| Internal Callback Routines |
---------------------------*/
/************************************************************************
 * OutlineButtonCallback
 ************************************************************************/
static  void
#ifdef  _NO_PROTO
OutlineButtonCallback(pbwid,client_data,call_data)
        Widget          pbwid;
        XtPointer       client_data;
        XtPointer       call_data;
#else
OutlineButtonCallback(
        Widget          pbwid,
        XtPointer       client_data,
        XtPointer       call_data)
#endif /* _NO_PROTO */
{
        Widget                  cwid = (Widget)client_data;
        XmContainerWidget       cw = (XmContainerWidget)XtParent(cwid);
        XmContainerConstraint   c = GetContainerConstraint(cwid);
        XmGadget                g = (XmGadget)cwid;
        XmAnyCallbackStruct *   pbcbs = (XmAnyCallbackStruct *)call_data;
        XmContainerOutlineCallbackStruct        cbs;
        unsigned char           state_before_callback;

        SetLocationCursor(pbwid);
        /*
         * PushButton's Arm() action keeps grabbing keyboard focus - ugh!
         */
        XmProcessTraversal((Widget)cw,XmTRAVERSE_CURRENT);

        if (c->outline_state == XmEXPANDED)
                {
                c->outline_state = XmCOLLAPSED;
                cbs.reason = XmCR_COLLAPSED;
                }
        else
                {
                c->outline_state = XmEXPANDED;
                cbs.reason = XmCR_EXPANDED;
                }
        cbs.event = pbcbs->event;
        cbs.item = cwid;
        state_before_callback = c->outline_state;
        XtCallCallbackList((Widget)cw,cw->container.value_changed_cb,&cbs);
        /*
         * In case user destroys child.
         */
        if (g->object.being_destroyed)
                return;
        /*
         * In case user has called SetValues in XmNvalueChangedCallback
         * to prevent action from occurring.
         */
        if (c->outline_state != state_before_callback)
                return;
        if (c->outline_state == XmCOLLAPSED)
                CollapseCwid(cwid);
        else
                ExpandCwid(cwid);
}


/*-------------------
| External functions |
-------------------*/

/************************************************************************
 *  XmContainerGetItemByCoordinates
 ************************************************************************/
Widget
#ifdef  _NO_PROTO
XmContainerGetItemByCoordinates( wid,x,y)
        Widget          wid;
        Position        x;
        Position        y;
#else
XmContainerGetItemByCoordinates(
        Widget          wid,
        Position        x,
        Position        y)
#endif /* _NO_PROTO */
{
        /*
         * Return the item at the specified coordinates.
         */
        return(LocatePointer(wid,(int)x,(int)y));
}
/************************************************************************
 * XmContainerGetItemChildren
 ************************************************************************/
int
#ifdef  _NO_PROTO
XmContainerGetItemChildren(wid,item,item_children)
        Widget          wid;
        Widget          item;
        WidgetList      *item_children;
#else
XmContainerGetItemChildren(
        Widget          wid,
        Widget          item,
        WidgetList      *item_children)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        XmContainerConstraint   c;
        CwidNode        node;
        CwidNode        first_child_node;
        WidgetList      clist;
        int             i,clist_count;

        if (item == NULL)
                {
                if (cw->container.first_node == NULL)
                        return(0);
                first_child_node = cw->container.first_node;
                }
        else
                {
                if (XtParent(item) != wid)
                        return(0);
                c = GetContainerConstraint(item);
                node = c->node_ptr;
                if (node->child_ptr == NULL)
                        return(0);
                first_child_node = node->child_ptr;
                }
        clist_count = 1;
        node = first_child_node;
        while (node->next_ptr != NULL)
                {
                clist_count++;
                node = node->next_ptr;
                }
        clist = (WidgetList)XtMalloc(clist_count * sizeof(Widget));
        node = first_child_node;
        for (i = 0; i < clist_count; i++)
                {
                clist[i] = node->widget_ptr;
                node = node->next_ptr;
                }
        *item_children = clist;
        return(clist_count);
}

/************************************************************************
 *  XmContainerRelayout
 ************************************************************************/
void
#ifdef _NO_PROTO
XmContainerRelayout(wid)
        Widget  wid;
#else
XmContainerRelayout(
        Widget  wid)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;

        if (CtrLayoutIsOUTLINE(cw) || CtrViewIsDETAIL(cw))
                return;
        Layout(wid);
        if (XtIsRealized(wid))
            XClearArea(XtDisplay(wid),XtWindow(wid),0,0,0,0,True);
}

/************************************************************************
 *  XmContainerReorder
 ************************************************************************/
void
#ifdef _NO_PROTO
XmContainerReorder(wid,cwid_list,cwid_count)
        Widget          wid;
        WidgetList      cwid_list;
        int             cwid_count;
#else
XmContainerReorder(
        Widget          wid,
        WidgetList      cwid_list,
        int             cwid_count)
#endif /* _NO_PROTO */
{
        XmContainerWidget       cw = (XmContainerWidget)wid;
        XmContainerConstraint   c;
        Widget                  pcwid;
        int *                   pi_list;
        int                     i, pi_count;

        if (cwid_count <= 1)
                return;
        c = GetContainerConstraint(cwid_list[0]);
        pcwid = c->entry_parent;
        pi_list = (int *)XtMalloc(cwid_count * sizeof(int));
        pi_count = 0;
        for (i=0; i < cwid_count; i++)
                {
                c = GetContainerConstraint(cwid_list[i]);
                if (c->entry_parent == pcwid)
                        {
                        pi_list[pi_count] = c->position_index;
                        pi_count++;
                        }
                }
        qsort(pi_list,pi_count,sizeof(int),CompareInts);
        pi_count = 0;
        for (i=0; i < cwid_count; i++)
                {
                c = GetContainerConstraint(cwid_list[i]);
                if (c->entry_parent == pcwid)
                        {
                        c->position_index = pi_list[pi_count];
                        pi_count++;
                        SeverNode(c->node_ptr);
                        InsertNode(c->node_ptr);
                        }
                }
        XtFree((XtPointer)pi_list);

        if (CtrLayoutIsOUTLINE(cw) || CtrViewIsDETAIL(cw))
                Layout(wid);
}

/************************************************************************
 *  XmCreateContainer
 ************************************************************************/
Widget
#ifdef _NO_PROTO
XmCreateContainer( parent,name,arglist,argcount)
        Widget          parent;
        char            *name;
        ArgList         arglist;
        Cardinal        argcount;
#else
XmCreateContainer(
        Widget          parent,
        char            *name,
        ArgList         arglist,
        Cardinal        argcount)
#endif /* _NO_PROTO */
{
        /*
         * Create an instance of a container and return the widget id.
         */
   return(XtCreateWidget(name,xmContainerWidgetClass,parent,arglist,argcount));
}
