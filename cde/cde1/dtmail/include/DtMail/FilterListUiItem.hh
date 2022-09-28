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
 *	Copyright 1993, 1994, 1995, 1996 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "%W %G"
#endif

#include <DtMail/ListUiItem.hh>
#include <DtMail/RuleCmd.h>
#include <DtMail/filter.h>

#ifndef _FILTERLISTUIITEM_HH
#define _FILTERLISTUIITEM_HH


// CLASS FilterListUiItem
///////////////////////////////////////////////////////////
class FilterListUiItem : public ListUiItem {
  
public:
  FilterListUiItem(Widget, int, char *, RuleCmd *);
  virtual ~FilterListUiItem(){;}; // we don't alloc any memory
  virtual void writeFromUiToSource();
  virtual void writeFromSourceToUi();
  DtVirtArray<filter *>*getItemList(){ return list_items; };
  virtual void handleAddButtonPress(filter *);
  virtual void handleChangeButtonPress(filter *);
  virtual void handleDeleteButtonPress();
  void handleMoveDownButtonPress();
  void handleMoveUpButtonPress();
  void update_rule_dialog(filter *);

private:
  RuleCmd *_rule;
  void update_display();
  char *format_string(struct filter *);
  DtVirtArray<filter *> *list_items; 
  DtVirtArray<char *> *deleted_items;
};

#endif
