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
#include <DtMail/RadioBoxUiItem.hh>
#include <Xm/ToggleB.h>
//#include <X11/Intrinsic.h>

extern Boolean props_changed;

////////////////////////////////////////////////////////////////////
RadioBoxUiItem::RadioBoxUiItem(Widget w, 
			       ToolbarConfig *toolbar,
			       int source, 
			       char *search_key,
			       Widget toolbar_list_w) :
    PropUiItem(w, source, search_key)
{
    WidgetList items;
    int values[2] = {0, 1};

    _toolbar = toolbar;
    _toolbar_list_w = toolbar_list_w;
    
    XtVaGetValues(w,
		  XmNchildren, &items,
		  NULL);
    options_radiobox_init(w, 2, items, values, &dirty_bit);

    XtAddCallback(items[0],
		  XmNvalueChangedCallback, &RadioBoxUiItem::handleSelection,
		  this);
//     XtAddCallback(items[1],
// 		  XmNvalueChangedCallback, &RadioBoxUiItem::handleSelection,
// 		  this);
}

///////////////////////////////////////////////////////////////////
RadioBoxUiItem::~RadioBoxUiItem()
{
    delete _toolbar;
}

///////////////////////////////////////////////////////////////////
void RadioBoxUiItem::writeFromUiToSource()
{
    Widget w = this->getWidget();
    int value;

    value = options_radiobox_get_value(w);
    if (!value) {
	prop_source->setValue("f");
    } else {
	prop_source->setValue("");
    }
}

// RadioBox::writeFromSourceToUi() does not need to update the mock
// toolbar.  This is done by the writeFromSourceToUi function in 
// the ToolbarListUiItem class.
///////////////////////////////////////////////////////////////////
void RadioBoxUiItem::writeFromSourceToUi()
{
    Widget w = this->getWidget();
    char *value;
  
    value = (char *)prop_source->getValue();
    if (strcmp(value, "") == 0) {
	options_radiobox_set_value(w, 1, True);
    } else if (strcmp(value, "f") == 0) {
	options_radiobox_set_value(w, 0, True);
    }
}

///////////////////////////////////////////////////////////////////
void RadioBoxUiItem::handleSelection(Widget w,
				     XtPointer client_data,
				     XtPointer)
{
    RadioBoxUiItem *obj = (RadioBoxUiItem *)client_data;
    XmStringTable items;
    Widget rb = obj->getWidget();
    
    XtVaGetValues(obj->_toolbar_list_w,
		  XmNitems, &items,
		  NULL);

    if (XmToggleButtonGetState(w)) {
	options_radiobox_set_value(rb, 0, True);
	obj->_toolbar->useIcons(items, True);
    } else {
	options_radiobox_set_value(rb, 1, True);
	obj->_toolbar->useIcons(items, False);
    }
}
