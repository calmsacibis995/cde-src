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
#include <DtMail/PropUi.hh>

#ifndef _LISTUIITEM_HH
#define _LISTUIITEM_HH

// CLASS ListUiItem
// derived class for prop sheet glue items for textfield
///////////////////////////////////////////////////////////
class ListUiItem : public PropUiItem {
  
public:
  ListUiItem(Widget w, int source, char *search_key, DtVirtArray<char *> *alias_list);
  virtual ~ListUiItem(){;}; // we don't alloc any memory
#ifdef DEAD_WOOD
  virtual int getType(){ return _LIST_ITEM; };
  virtual int getSource(){ return data_source; };
#endif /* DEAD_WOOD */
  virtual void writeFromUiToSource();
  virtual void writeFromSourceToUi();

  virtual void handleAddButtonPress(){;};
  virtual void handleChangeButtonPress(){;};
  virtual void handleDeleteButtonPress(){;};

private:
#ifdef DEAD_WOOD
  int data_source;
#endif /* DEAD_WOOD */
};

#endif
