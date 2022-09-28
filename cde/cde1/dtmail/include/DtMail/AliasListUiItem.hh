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

#ifndef _ALIASLISTUIITEM_HH
#define _ALIASLISTUIITEM_HH

#ifdef DEAD_WOOD
extern void handleDoubleSelection(Widget w, XtPointer clientdata, XtPointer calldata);
#endif /* DEAD_WOOD */

// CLASS AliasListUiItem
// derived class for prop sheet glue items for textfield
///////////////////////////////////////////////////////////
class AliasListUiItem : public ListUiItem {
  
public:
  AliasListUiItem(Widget w, int source, char *search_key, Widget w_k, Widget w_v);
  virtual ~AliasListUiItem(){;}; // we don't alloc any memory
  virtual void writeFromUiToSource();
  virtual void writeFromSourceToUi();
  Widget getKeyWidget(){ return key_widget;};
  Widget getValueWidget(){ return value_widget;};
  DtVirtArray<PropStringPair *> *getItemList(){ return list_items; };

  virtual void handleAddButtonPress();
  virtual void handleChangeButtonPress();
  virtual void handleDeleteButtonPress();

private:

  void update_display();
  DtVirtArray<PropStringPair *> *list_items;
  DtVirtArray<char *> *deleted_items;

  Widget key_widget;
  Widget value_widget;
};

#endif
