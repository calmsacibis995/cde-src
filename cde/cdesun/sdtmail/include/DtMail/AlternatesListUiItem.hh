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
#pragma ident "%W %G"
#endif

#include <stdlib.h>
#include <X11/Intrinsic.h>

#ifndef _ALTERNATESLISTUIITEM_HH
#define _ALTERNATESLISTUIITEM_HH

// CLASS AlternatesListUiItem
// derived class for prop sheet glue items for textfield
///////////////////////////////////////////////////////////
class AlternatesListUiItem : public ListUiItem {
  
public:
  AlternatesListUiItem(Widget w, int source, char *search_key, Widget w_list);
  virtual ~AlternatesListUiItem(); 
  virtual void writeFromUiToSource();
  virtual void writeFromSourceToUi();
  Widget getEntryFieldWidget(){ return entry_field_widget;};
  DtVirtArray<PropStringPair *> *getItemList(){ return list_items; };
  
  virtual void handleAddButtonPress();
  virtual void handleChangeButtonPress();
  virtual void handleDeleteButtonPress();

private:

  DtVirtArray<PropStringPair *> *list_items;
  DtVirtArray<char *> *deleted_items;
  Widget entry_field_widget;
};

#endif
