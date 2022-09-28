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

#ifndef _TEMPLATESLISTUIITEM_HH
#define _TEMPLATESLISTUIITEM_HH

// CLASS TemplateListUiItem
// derived class for prop sheet glue items for textfield
///////////////////////////////////////////////////////////
class TemplateListUiItem : public ListUiItem {
  
public:

  TemplateListUiItem(Widget w, int source, char *search_key);
  virtual ~TemplateListUiItem(){;}; // we don't alloc any memory

  virtual void writeFromUiToSource();
  virtual void writeFromSourceToUi();

  void handleEditButtonPress();
  void handleDeleteButtonPress();
  void handleRenameButtonPress();
  void handleIncludeButtonPress(char*);
  void getNameAndRename();
  Boolean inList( PropStringPair *, char *);
  void quit();

  DtVirtArray<PropStringPair *> *getItemList(){ return _list_items; };
  void update_display();
  void replaceListItem(char *, char *);

private:

  DtVirtArray<PropStringPair *> *_list_items;
  DtVirtArray<PropStringPair *> *_files_to_rename;
  DtVirtArray<PropStringPair *> *_files_to_delete;


};
#endif
