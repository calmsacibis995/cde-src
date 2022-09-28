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

#ifndef _MOVEMENULISTUIITEM_HH
#define _MOVEMENULISTUIITEM_HH

void handleMMSelection(Widget, XtPointer, XtPointer);

// CLASS MoveMenuListUiItem
// derived class for prop sheet glue items for textfield
///////////////////////////////////////////////////////////
class MoveMenuListUiItem : public ListUiItem {
  
public:
  MoveMenuListUiItem(Widget w, int source, char *search_key, Widget w_list, Widget server, Widget rb);
  virtual ~MoveMenuListUiItem(){;}; // we don't alloc any memory
  virtual void writeFromUiToSource();
  virtual void writeFromSourceToUi();

  Widget getEntryFieldWidget(){ return _mailbox_folder_widget;};
  Widget getServerWidget(){ return _server_widget;};
  Widget getRadioWidget(){ return _local_imap_radiob;};
  void   setClientData(void *cd){ _clientData = cd;};
  void*  getClientData(){ return _clientData; };

  DtVirtArray<char *> *getItemList(){ return _list_items; };

  void handleBrowseButtonPress(char*, char*);
  void handleAddButtonPress();
  void handleChangeButtonPress();
  void handleDeleteButtonPress();
  void handleMoveDownButtonPress();
  void handleMoveUpButtonPress();
  void handleAlphabetizeButtonPress();

private:

  DtVirtArray<char *> *_list_items;
  Widget _mailbox_folder_widget;
  Widget _server_widget;
  Widget _local_imap_radiob;
  static int myStrCmp(const char **,const char **);
  void *_clientData;
  SdmBoolean inList(char*);
};

#endif
