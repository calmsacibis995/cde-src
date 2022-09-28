/*******************************************************************************
**
**  datefield.c
**
**  $XConsortium: datefield.c /main/cde1_maint/1 1995/07/14 23:06:39 drk $
**
**  RESTRICTED CONFIDENTIAL INFORMATION:
**
**  The information in this document is subject to special
**  restrictions in a confidential disclosure agreement between
**  HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
**  document outside HP, IBM, Sun, USL, SCO, or Univel without
**  Sun's specific written approval.  This document and all copies
**  and derivative works thereof must be returned or destroyed at
**  Sun's request.
**
**  Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
**
*******************************************************************************/

/*									*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc. 				*
 */

#pragma ident "@(#)datefield.c	1.10 96/11/12 Sun Microsystems, Inc."

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/LabelG.h>
#include <Xm/Label.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include "util.h"
#include "datefield.h"
#include "misc.h"

/*
**  Returns a date string the parser can handle
*/
extern char*
get_date_from_widget(Tick t, Widget w, OrderingType order,
		     SeparatorType separator) {
        char		*date = NULL;
	WidgetClass	wc;
        static char	buf[80];
 
	memset(buf, '\0', 80);

	if (!w)
		format_tick(t, order, separator, buf);
	else {
		wc = XtClass(w);
		if (wc == xmTextWidgetClass)
			date = XmTextGetString(w);
		else if (wc == xmTextFieldWidgetClass)
			date = XmTextFieldGetString(w);
		else
			return NULL;

		if (!date || *date == '\0')
			format_tick(t, order, separator, buf);
		else if (!datestr2mdy(date, order, separator, buf)) {
			XtFree(date);
			return NULL;
		}
		if (date)
			XtFree(date);
	}

	return buf;
}

extern void
set_date_in_widget(Tick t, Widget w, OrderingType order,
		   SeparatorType separator) {
        char		buf[80];
	XmString	xmstr;
	WidgetClass	wc = XtClass(w);

        format_tick(t, order, separator, buf);
	if (wc == xmLabelGadgetClass || wc == xmLabelWidgetClass) {
		xmstr = XmStringCreateLocalized(buf);
		XtVaSetValues(w, XmNlabelString, xmstr,
			NULL);
		XmStringFree(xmstr);
	} else if (wc == xmTextWidgetClass)
		XmTextSetString(w, buf);
	else if (wc == xmTextFieldWidgetClass)
		XmTextFieldSetString(w, buf);
}
