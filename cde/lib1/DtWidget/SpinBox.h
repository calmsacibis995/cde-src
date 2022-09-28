/* $XConsortium: SpinBox.h /main/cde1_maint/1 1995/07/17 18:32:38 drk $ */
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

#ifndef _Dt_SpinBox_h
#define _Dt_SpinBox_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Constants
 */

/* Resources */

#ifndef DtNarrowLayout
#define DtNarrowLayout		"arrowLayout"
#endif
#ifndef DtNarrowSensitivity
#define DtNarrowSensitivity	"arrowSensitivity"
#endif
#ifndef DtNarrowSize
#define DtNarrowSize		"arrowSize"
#endif
#ifndef DtNspinBoxChildType
#define DtNspinBoxChildType	"spinBoxChildType"
#endif
#ifndef DtNposition
#define DtNposition		"position"
#endif
#ifndef DtNtextField
#define DtNtextField		"textField"
#endif
#ifndef DtNwrap
#define DtNwrap			"wrap"
#endif
#ifndef DtNincrementValue
#define DtNincrementValue	"incrementValue"
#endif
#ifndef DtNmaximumValue
#define DtNmaximumValue		"maximumValue"
#endif
#ifndef DtNminimumValue
#define DtNminimumValue		"minimumValue"
#endif
#ifndef DtNnumValues
#define DtNnumValues		"numValues"
#endif
#ifndef DtNvalues
#define DtNvalues		"values"
#endif
#ifndef DtNactivateCallback
#define DtNactivateCallback	XmNactivateCallback
#endif
#ifndef DtNalignment
#define DtNalignment		XmNalignment
#endif
#ifndef DtNcolumns
#define DtNcolumns		XmNcolumns
#endif
#ifndef DtNdecimalPoints
#define DtNdecimalPoints	XmNdecimalPoints
#endif
#ifndef DtNeditable
#define DtNeditable		XmNeditable
#endif
#ifndef DtNfocusCallback
#define DtNfocusCallback	XmNfocusCallback
#endif
#ifndef DtNinitialDelay
#define DtNinitialDelay		XmNinitialDelay
#endif
#ifndef DtNlosingFocusCallback
#define DtNlosingFocusCallback	XmNlosingFocusCallback
#endif
#ifndef DtNmarginHeight
#define DtNmarginHeight		XmNmarginHeight
#endif
#ifndef DtNmarginWidth
#define DtNmarginWidth		XmNmarginWidth
#endif
#ifndef DtNmaxLength
#define DtNmaxLength		XmNmaxLength
#endif
#ifndef DtNmodifyVerifyCallback
#define DtNmodifyVerifyCallback	XmNmodifyVerifyCallback
#endif
#ifndef DtNrecomputeSize
#define DtNrecomputeSize	XmNrecomputeSize
#endif
#ifndef DtNrepeatDelay
#define DtNrepeatDelay		XmNrepeatDelay
#endif
#ifndef DtNvalueChangedCallback
#define DtNvalueChangedCallback	XmNvalueChangedCallback
#endif

#ifndef DtCArrowLayout
#define DtCArrowLayout		"ArrowLayout"
#endif
#ifndef DtCArrowSensitivity
#define DtCArrowSensitivity	"ArrowSensitivity"
#endif
#ifndef DtCSpinBoxChildType
#define DtCSpinBoxChildType	"SpinBoxChildType"
#endif
#ifndef DtCTextField
#define DtCTextField		"TextField"
#endif
#ifndef DtCWrap
#define DtCWrap			"Wrap"
#endif
#ifndef DtCIncrementValue
#define DtCIncrementValue	"incrementValue"
#endif
#ifndef DtCMaximumValue
#define DtCMaximumValue		"maximumValue"
#endif
#ifndef DtCMinimumValue
#define DtCMinimumValue		"minimumValue"
#endif
#ifndef DtCNumValues
#define DtCNumValues		"numValues"
#endif
#ifndef DtCValues
#define DtCValues		"values"
#endif
#ifndef DtCAlignment
#define DtCAlignment		XmCAlignment
#endif
#ifndef DtCCallback
#define DtCCallback		XmCCallback
#endif
#ifndef DtCColumns
#define DtCColumns		XmCColumns
#endif
#ifndef DtCDecimalPoints
#define DtCDecimalPoints	XmCDecimalPoints
#endif
#ifndef DtCEditable
#define DtCEditable		XmCEditable
#endif
#ifndef DtCInitialDelay
#define DtCInitialDelay		XmCInitialDelay
#endif
#ifndef DtCItems
#define DtCItems		XmCItems
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
#ifndef DtCPosition
#define DtCPosition		XmCPosition
#endif
#ifndef DtCRecomputeSize
#define DtCRecomputeSize	XmCRecomputeSize
#endif
#ifndef DtCRepeatDelay
#define DtCRepeatDelay		XmCRepeatDelay
#endif

/* Representation types */

#ifndef DtRIncrementValue
#define DtRIncrementValue	"IncrementValue"
#endif
#ifndef DtRMaximumValue
#define DtRMaximumValue		"MaximumValue"
#endif
#ifndef DtRMinimumValue
#define DtRMinimumValue		"MinimumValue"
#endif
#ifndef DtRNumValues
#define DtRNumValues		"NumValues"
#endif
#ifndef DtRValues
#define DtRValues		"Values"
#endif
#ifndef DtRArrowSensitivity
#define DtRArrowSensitivity	"ArrowSensitivity"
#endif
#ifndef DtRArrowLayout
#define DtRArrowLayout		"ArrowLayout"
#endif
#ifndef DtRSpinBoxChildType
#define DtRSpinBoxChildType	"SpinBoxChildType"
#endif

/* DtNarrowLayout values */

#ifndef DtARROWS_FLAT_BEGINNING
#define DtARROWS_FLAT_BEGINNING		4
#endif
#ifndef DtARROWS_FLAT_END
#define DtARROWS_FLAT_END		3
#endif
#ifndef DtARROWS_SPLIT
#define DtARROWS_SPLIT			2
#endif
#ifndef DtARROWS_BEGINNING
#define DtARROWS_BEGINNING		1
#endif
#ifndef DtARROWS_END
#define DtARROWS_END			0
#endif

/* DtNarrowSensitivity values */

#ifndef DtARROWS_SENSITIVE
#define DtARROWS_SENSITIVE		3
#endif
#ifndef DtARROWS_DECREMENT_SENSITIVE
#define DtARROWS_DECREMENT_SENSITIVE	2
#endif
#ifndef DtARROWS_INCREMENT_SENSITIVE
#define DtARROWS_INCREMENT_SENSITIVE	1
#endif
#ifndef DtARROWS_INSENSITIVE
#define DtARROWS_INSENSITIVE		0
#endif

/* DtNspinBoxChildType values */

#ifndef DtNUMERIC
#define DtNUMERIC	3
#endif

#ifndef DtSTRING
#define DtSTRING	XmSTRING
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

#ifndef DtCR_OK
#define DtCR_OK			XmCR_OK
#endif
#ifndef DtCR_SPIN_NEXT
#define DtCR_SPIN_NEXT		62
#endif
#ifndef DtCR_SPIN_PRIOR
#define DtCR_SPIN_PRIOR		63
#endif


/*
 * Types
 */

typedef struct {
	int		reason;
	XEvent		*event;
	Widget		widget;
	Boolean		doit;
	int		position;
	XmString	value;
	Boolean		crossed_boundary;
} DtSpinBoxCallbackStruct;

/* Widget class and instance */

typedef struct _DtSpinBoxClassRec *DtSpinBoxWidgetClass;
typedef struct _DtSpinBoxRec      *DtSpinBoxWidget;

/*
 * Data
 */

/* Widget class record */

externalref WidgetClass dtSpinBoxWidgetClass;


/*
 * Functions
 */

extern Widget DtCreateSpinBox(
		Widget		parent,
		char		*name,
		ArgList		arglist,
		Cardinal	argcount);

extern void DtSpinBoxAddItem(
		Widget		widget,
		XmString	item,
		int		pos);

extern void DtSpinBoxDeletePos(
		Widget		widget,
		int		pos);

extern void DtSpinBoxSetItem(
		Widget		widget,
		XmString	item);

#ifdef __cplusplus
}
#endif

#endif	/* _Dt_SpinBox_h */
