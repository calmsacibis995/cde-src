/*
 *+SNOTICE
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement bertween
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel wihtout
 *	Sun's specific written approval.  This documment and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident " %W %G "
#endif

#include <DtMail/options_util.h>
#include <DtMail/PropUi.hh>
#include <DtMail/RadioBoxUiItem2.hh>
#include <Xm/Xm.h>

extern Boolean props_changed;

////////////////////////////////////////////////////////////////////
RadioBoxUiItem2::RadioBoxUiItem2(Widget w, 
			       int source, 
			       char *search_key) :
    PropUiItem(w, source, search_key)
{
    WidgetList items;
    _values[0] = 1;
    _values[1] = 0;

    XtVaGetValues(w, XmNchildren, &items, NULL);
    options_radiobox_init(w, 2, items, _values, &dirty_bit);
}

///////////////////////////////////////////////////////////////////
void RadioBoxUiItem2::writeFromUiToSource()
{
    Widget w = this->getWidget();
    int value;

    value = options_radiobox_get_value(w);
    if (value) {
	prop_source->setValue("f");
    } else {
	prop_source->setValue("");
    }
}

// RadioBox2::writeFromSourceToUi() does not need to update the mock
// toolbar.  This is done by the writeFromSourceToUi function in 
// the ToolbarListUiItem class.
///////////////////////////////////////////////////////////////////
void RadioBoxUiItem2::writeFromSourceToUi()
{
    Widget w = this->getWidget();
    char *value;
  
    value = (char *)prop_source->getValue();
    if (strcmp(value, "") == 0) {
	options_radiobox_set_value(w, 0, True);
    } else if (strcmp(value, "f") == 0) {
	options_radiobox_set_value(w, 1, True);
    }
}
