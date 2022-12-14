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
#pragma ident " @(#)CheckBoxUiItem.C	1.5 02/22/95 "
#endif

#include <DtMail/options_util.h>
#include <DtMail/PropUi.hh>
#include <DtMail/CheckBoxUiItem.hh>

// CheckBoxUiItem::CheckBoxUiItem
// CheckBoxUiItem ctor
////////////////////////////////////////////////////////////////////

CheckBoxUiItem::CheckBoxUiItem(Widget w, int source, char *search_key):PropUiItem(w, source, search_key)
{
#ifdef DEAD_WOOD
  data_source = source;
#endif /* DEAD_WOOD */

  options_checkbox_init(w, &(this->dirty_bit));

}

// CheckBoxUiItem::writeFromUiToSource()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void CheckBoxUiItem::writeFromUiToSource()
{
  Boolean	checkbox_value;
  Widget w = this->getWidget();

  checkbox_value = options_checkbox_get_value(w);
   
  if(checkbox_value == TRUE) // make sure the value is in the table
    {
      prop_source->setValue("");
    }	
  else
    {
      prop_source->setValue("f");
    }
}

// CheckBoxUiItem::writeFromSourceToUi()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void CheckBoxUiItem::writeFromSourceToUi()
{
  char *value;
  Widget w = this->getWidget();

  value = (char *)prop_source->getValue();

  // 
  // this will have to be made more robust... 
  //
  // This assumes that a non-null value means that the 
  // value is set and that a non-null means turn on the CB
  if(strcmp(value, "") == 0)
    options_checkbox_set_value(w, TRUE, this->dirty_bit);
  else if(strcmp(value, "f") == 0)
    options_checkbox_set_value(w, FALSE, this->dirty_bit);
  
}



