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
#pragma ident " @(#)ScaleUiItem.C	1.4 12 Aug 1994 "
#endif

#include <DtMail/options_util.h>
#include <DtMail/PropUi.hh>
#include <DtMail/ScaleUiItem.hh>

// ScaleUiItem::ScaleUiItem
// ScaleUiItem ctor
////////////////////////////////////////////////////////////////////

ScaleUiItem::ScaleUiItem(Widget w, int source, char *search_key):PropUiItem(w, source, search_key)
{
#ifdef DEAD_WOOD
  data_source = source;
#endif /* DEAD_WOOD */

  options_field_init(w, &(this->dirty_bit));

}

// ScaleUiItem::writeFromUiToSource()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void ScaleUiItem::writeFromUiToSource()
{
  int 	scale_value;
  Widget w = this->getWidget();
  char val_str[24];

  scale_value = options_scale_get_value(w);

  sprintf(val_str, "%d", scale_value);	
   
  prop_source->setValue(val_str);
}

// ScaleUiItem::writeFromSourceToUi()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void ScaleUiItem::writeFromSourceToUi()
{
  char *value;
  Widget w = this->getWidget();

  value = (char*)prop_source->getValue();

  options_scale_set_value(w, value, this->dirty_bit);

}
