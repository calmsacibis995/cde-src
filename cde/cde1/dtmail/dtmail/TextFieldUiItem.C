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
#pragma ident " @(#)TextFieldUiItem.C	1.7 02/05/96 "
#endif

#include <DtMail/options_util.h>
#include <DtMail/PropUi.hh>
#include <DtMail/TextFieldUiItem.hh>
#include "RoamApp.h"
#include <DtMail/IO.hh> //SafeAccess...

// TextFieldUiItem::TextFieldUiItem
// TextFieldUiItem ctor
////////////////////////////////////////////////////////////////////

TextFieldUiItem::TextFieldUiItem(Widget w, int source, char *search_key):PropUiItem(w, source, search_key)
{
#ifdef DEAD_WOOD
  data_source = source;
#endif /* DEAD_WOOD */

  options_field_init(w, &(this->dirty_bit));

}

// TextFieldUiItem::writeFromUiToSource()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void TextFieldUiItem::writeFromUiToSource()
{
  char 	*textfield_value;
  Widget w = this->getWidget();

  textfield_value = options_field_get_value(w);
  prop_source->setValue(textfield_value);

  // If they change the folder directory, make sure it exists.
  if (strcmp(getKey(), "folder") == 0) {
	char *folder = theRoamApp.getFolderDir(TRUE);
	if (SafeAccess(folder, F_OK) != 0)
        	mkdir(folder, 0700);
    	free(folder);
  }
}

// TextFieldUiItem::writeFromSourceToUi()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void TextFieldUiItem::writeFromSourceToUi()
{
  char *value;
  Widget w = this->getWidget();

  value = (char *)prop_source->getValue();

  options_field_set_value(w, value, this->dirty_bit);

}
void TextFieldUiItem::handleSelectButtonPress(char *selection)
{
   Widget w = this->getWidget();
   options_field_set_value(w, selection, this->dirty_bit);
}
