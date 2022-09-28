/* $XConsortium: ComboBox.h /main/cde1_maint/2 1995/10/05 09:21:03 lehors $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */
/***********************************************************
Copyright 1993 Interleaf, Inc.

Permission to use, copy, modify, and distribute this software
and its documentation for any purpose without fee is granted,
provided that the above copyright notice appear in all copies
and that both copyright notice and this permission notice appear
in supporting documentation, and that the name of Interleaf not
be used in advertising or publicly pertaining to distribution of
the software without specific written prior permission.

Interleaf makes no representation about the suitability of this
software for any purpose. It is provided "AS IS" without any
express or implied warranty.
******************************************************************/

#ifndef _Dt_ComboBox_h
#define _Dt_ComboBox_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Constants
 */

/* Resources */

/***********************************************************************
 *
 * NOTE:
 *	Because of future development of the ComboBox, not all of the 
 *	following resources should be used.  The only ones which are
 *	supported are the ones found in the Common Desktop Environment:
 *	Programmer's Guide.  The supported resources include:
 *	DtNmarginHeight, DtNmarginWidth, DtNselectedItem, 
 *	DtNselectedPosition, DtNselectionCallback, and DtNcomboBoxType.
 *
 *	The use of the unsupported resources found in this header
 *	file may cause dangerous and unexpected behavior.
 *
 **********************************************************************/

/* WARNING: DtNactivateCallback is an unsupported resource in CDE 1.0 */
#ifndef DtNactivateCallback
#define DtNactivateCallback	XmNactivateCallback
#endif
/* WARNING: DtNalignment is an unsupported resource in CDE 1.0 */
#ifndef DtNalignment
#define DtNalignment		XmNalignment
#endif
/* WARNING: DtNarrowSize is an unsupported resource in CDE 1.0 */
#ifndef DtNarrowSize
#define DtNarrowSize		"arrowSize"
#endif
/* WARNING: DtNarrowSpacing is an unsupported resource in CDE 1.0 */
#ifndef DtNarrowSpacing
#define DtNarrowSpacing		"arrowSpacing"
#endif
/* WARNING: DtNarrowType is an unsupported resource in CDE 1.0 */
#ifndef DtNarrowType
#define DtNarrowType		"arrowType"
#endif
/* WARNING: DtNcolumns is an unsupported resource in CDE 1.0 */
#ifndef DtNcolumns
#define DtNcolumns		XmNcolumns
#endif
#ifndef DtNcomboBoxType
#define DtNcomboBoxType		"comboBoxType"
#endif
/* WARNING: DtNfocusCallback is an unsupported resource in CDE 1.0 */
#ifndef DtNfocusCallback
#define DtNfocusCallback	XmNfocusCallback
#endif
/* WARNING: DtNhorizontalSpacing is an unsupported resource in CDE 1.0 */
#ifndef DtNhorizontalSpacing
#define DtNhorizontalSpacing	XmNhorizontalSpacing
#endif
/* WARNING: DtNitemCount is an unsupported resource in CDE 1.0 */
#ifndef DtNitemCount
#define DtNitemCount		XmNitemCount
#endif
/* WARNING: DtNitems is an unsupported resource in CDE 1.0 */
#ifndef DtNitems
#define DtNitems		XmNitems
#endif
/* WARNING: DtNlabelString is an unsupported resource in CDE 1.0 */
#ifndef DtNlabelString
#define DtNlabelString		XmNlabelString
#endif
/* WARNING: DtNlist is an unsupported resource in CDE 1.0 */
#ifndef DtNlist
#define DtNlist			"list"
#endif
/* WARNING: DtNlistFontList is an unsupported resource in CDE 1.0 */
#ifndef DtNlistFontList
#define DtNlistFontList		"listFontList"
#endif
/* WARNING: DtNlistMarginHeight is an unsupported resource in CDE 1.0 */
#ifndef DtNlistMarginHeight
#define DtNlistMarginHeight	XmNlistMarginHeight
#endif
/* WARNING: DtNlistMarginWidth is an unsupported resource in CDE 1.0 */
#ifndef DtNlistMarginWidth
#define DtNlistMarginWidth	XmNlistMarginWidth
#endif
/* WARNING: DtNlistSpacing is an unsupported resource in CDE 1.0 */
#ifndef DtNlistSpacing
#define DtNlistSpacing		XmNlistSpacing
#endif
/* WARNING: DtNlosingFocusCallback is an unsupported resource in CDE 1.0 */
#ifndef DtNlosingFocusCallback
#define DtNlosingFocusCallback	XmNlosingFocusCallback
#endif
#ifndef DtNmarginHeight
#define DtNmarginHeight		XmNmarginHeight
#endif
#ifndef DtNmarginWidth
#define DtNmarginWidth		XmNmarginWidth
#endif
/* WARNING: DtNmaxLength is an unsupported resource in CDE 1.0 */
#ifndef DtNmaxLength
#define DtNmaxLength		XmNmaxLength
#endif
/* WARNING: DtNmenuPostCallback is an unsupported resource in CDE 1.0 */
#ifndef DtNmenuPostCallback
#define DtNmenuPostCallback	"menuPostCallback"
#endif
/* WARNING: DtNorientation is an unsupported resource in CDE 1.0 */
#ifndef DtNorientation
#define DtNorientation		XmNorientation
#endif
/* WARNING: DtNpoppedUp is an unsupported resource in CDE 1.0 */
#ifndef DtNpoppedUp
#define DtNpoppedUp		"poppedUp"
#endif
/* WARNING: DtNrecomputeSize is an unsupported resource in CDE 1.0 */
#ifndef DtNrecomputeSize
#define DtNrecomputeSize	XmNrecomputeSize
#endif
#ifndef DtNselectedItem
#define DtNselectedItem		"selectedItem"
#endif
#ifndef DtNselectedPosition
#define DtNselectedPosition	"selectedPosition"
#endif
#ifndef DtNselectionCallback
#define DtNselectionCallback	"selectionCallback"
#endif
/* WARNING: DtNtextField is an unsupported resource in CDE 1.0 */
#ifndef DtNtextField
#define DtNtextField		"textField"
#endif
/* WARNING: DtNtopItemPosition is an unsupported resource in CDE 1.0 */
#ifndef DtNtopItemPosition
#define DtNtopItemPosition	XmNtopItemPosition
#endif
/* WARNING: DtNupdateLabel is an unsupported resource in CDE 1.0 */
#ifndef DtNupdateLabel
#define DtNupdateLabel		"updateLabel"
#endif
/* WARNING: DtNverticalSpacing is an unsupported resource in CDE 1.0 */
#ifndef DtNverticalSpacing
#define DtNverticalSpacing	XmNverticalSpacing
#endif
/* WARNING: DtNvisibleItemCount is an unsupported resource in CDE 1.0 */
#ifndef DtNvisibleItemCount
#define DtNvisibleItemCount	XmNvisibleItemCount
#endif

#ifndef DtCAlignment
#define DtCAlignment		XmCAlignment
#endif
#ifndef DtCArrowSize
#define DtCArrowSize		"ArrowSize"
#endif
#ifndef DtCArrowSpacing
#define DtCArrowSpacing		"ArrowSpacing"
#endif
#ifndef DtCArrowType
#define DtCArrowType		"ArrowType"
#endif
#ifndef DtCCallback
#define DtCCallback		XmCCallback
#endif
#ifndef DtCColumns
#define DtCColumns		XmCColumns
#endif
#ifndef DtCComboBoxType
#define DtCComboBoxType		"ComboBoxType"
#endif
#ifndef DtCHorizontalSpacing
#define DtCHorizontalSpacing	"HorizontalSpacing"
#endif
#ifndef DtCItemCount
#define DtCItemCount		XmCItemCount
#endif
#ifndef DtCItems
#define DtCItems		XmCItems
#endif
#ifndef DtCList
#define DtCList			"List"
#endif
#ifndef DtCListFontList
#define DtCListFontList		"ListFontList"
#endif
#ifndef DtCListMarginHeight
#define DtCListMarginHeight	XmCListMarginHeight
#endif
#ifndef DtCListMarginWidth
#define DtCListMarginWidth	XmCListMarginWidth
#endif
#ifndef DtCListSpacing
#define DtCListSpacing		XmCListSpacing
#endif
#ifndef DtCMarginHeight
#define DtCMarginHeight		XmCMarginHeight
#endif
#ifndef DtCMarginWidth
#define DtCMarginWidth		XmCMarginWidth
#endif
#ifndef DtCMaxLength
#define DtCMaxLength		XmCMaxLength
#endif
#ifndef DtCOrientation
#define DtCOrientation		XmCOrientation
#endif
#ifndef DtCPoppedUp
#define DtCPoppedUp		"PoppedUp"
#endif
#ifndef DtCRecomputeSize
#define DtCRecomputeSize	XmCRecomputeSize
#endif
#ifndef DtCSelectedItem
#define DtCSelectedItem		"SelectedItem"
#endif
#ifndef DtCSelectedPosition
#define DtCSelectedPosition	"SelectedPosition"
#endif
#ifndef DtCTextField
#define DtCTextField		"TextField"
#endif
#ifndef DtCTopItemPosition
#define DtCTopItemPosition	XmCTopItemPosition
#endif
#ifndef DtCUpdateLabel
#define DtCUpdateLabel		"UpdateLabel"
#endif
#ifndef DtCVerticalSpacing
#define DtCVerticalSpacing	"VerticalSpacing"
#endif
#ifndef DtCVisibleItemCount
#define DtCVisibleItemCount	XmCVisibleItemCount
#endif
#ifndef DtCXmString
#define DtCXmString		XmCXmString
#endif

/* Representation types */

#ifndef DtRArrowType
#define DtRArrowType		"ArrowType"
#endif
#ifndef DtRComboBoxType
#define DtRComboBoxType		"ComboBoxType"
#endif

/* DtNorientation values */

#ifndef DtLEFT
#define DtLEFT		1
#endif
#ifndef DtRIGHT
#define DtRIGHT		2
#endif

/* DtNarrowType values */

#ifndef DtMOTIF
#define DtMOTIF		0
#endif
#ifndef DtWINDOWS
#define DtWINDOWS	1
#endif

/* DtNcomboBoxType values */

#ifndef DtDROP_DOWN_LIST
#define DtDROP_DOWN_LIST	2
#endif
#ifndef DtDROP_DOWN_COMBO_BOX
#define DtDROP_DOWN_COMBO_BOX	1
#endif

/* DtNalignment values */

#ifndef DtALIGNMENT_BEGINNING
#define DtALIGNMENT_BEGINNING	XmALIGNMENT_BEGINNING
#endif
#ifndef DtALIGNMENT_CENTER
#define DtALIGNMENT_CENTER	XmALIGNMENT_CENTER
#endif
#ifndef DtALIGNMENT_END
#define DtALIGNMENT_END		XmALIGNMENT_END
#endif

/* Callback reasons */

#ifndef DtCR_SELECT
#define DtCR_SELECT	    57 /* Large #, so no collisions with XM */
#endif
#ifndef DtCR_MENU_POST
#define DtCR_MENU_POST      129 /* Large #, so no collisions with XM */
#endif


/*
 * Types
 */

typedef struct {
	int		reason;
	XEvent		*event;
	XmString	item_or_text;
	int		item_position;
} DtComboBoxCallbackStruct;


/* Widget class and instance */

typedef struct _DtComboBoxClassRec *DtComboBoxWidgetClass;
typedef struct _DtComboBoxRec      *DtComboBoxWidget;


/*
 * Data
 */

/* Widget class record */

externalref WidgetClass dtComboBoxWidgetClass;


/*
 * Functions
 */

extern Widget DtCreateComboBox(
		Widget		parent,
		char		*name,
		ArgList		arglist,
		Cardinal	argcount);

extern void DtComboBoxAddItem(
		Widget 		combo,
		XmString	item,
		int		pos,
		Boolean		unique);

extern void DtComboBoxDeletePos(
		Widget		combo,
		int		pos);

extern void DtComboBoxSetItem(
		Widget		combo,
		XmString	item);

extern void DtComboBoxSelectItem(
		Widget		combo,
		XmString	item);

#ifdef __cplusplus
}
#endif

#endif	/* _Dt_ComboBox_h */
