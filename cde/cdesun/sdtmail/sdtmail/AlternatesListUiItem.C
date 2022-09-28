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

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/MailRc.hh>
#include <SDtMail/Error.hh>
#include <SDtMail/Connection.hh>
#include "RoamApp.h"
#include <Xm/List.h>
#include <DtMail/options_util.h>
#include <DtMail/PropUi.hh>
#include <DtMail/ListUiItem.hh>
#include <DtMail/AlternatesListUiItem.hh>
#include <DtMail/OptCmd.h>

extern Boolean props_changed;
void handleAlternateSelection(Widget w, XtPointer clientdata, XtPointer calldata);

// AlternatesListUiItem::AlternatesListUiItem
// AlternatesListUiItem ctor
////////////////////////////////////////////////////////////////////

AlternatesListUiItem::AlternatesListUiItem(Widget w, 
		       int source, 
		       char *search_key,
		       Widget text_entry_widget):ListUiItem(w, source, search_key, NULL)
{
  source = source; search_key = search_key;

  entry_field_widget = text_entry_widget;

  list_items = NULL;
  deleted_items = NULL;

  XtVaSetValues(w,
        XmNuserData, this,
        XmNautomaticSelection, True,
        XmNselectionPolicy, XmBROWSE_SELECT,
        NULL);

  XtAddCallback(w,
	XmNbrowseSelectionCallback, 
	(XtCallbackProc)handleAlternateSelection,
	(XtPointer)this);
}


AlternatesListUiItem::~AlternatesListUiItem()
{
  int i, num_items;

  if (list_items != NULL) {
    num_items = list_items->length();
    for (i=0; i < num_items; i++) {
        PropStringPair *psp = (*list_items)[i];
        delete psp;
    }
    delete list_items;
    list_items = NULL;
  }
  
  if (deleted_items != NULL) {
    num_items = deleted_items->length();
    for (i=0; i < num_items; i++)
            delete (*deleted_items)[i];
    delete deleted_items;
    deleted_items = NULL;
  }
}
//---------------------------------------------------------------
void handleAlternateSelection(Widget w, XtPointer clientdata, XtPointer calldata)
{
  AlternatesListUiItem *item;
  XmListCallbackStruct *list_info = (XmListCallbackStruct *)calldata;
  DtVirtArray<PropStringPair *> *list_items;
  clientdata = clientdata;

  XtVaGetValues(w, XmNuserData, &item, NULL);  
  list_items = item->getItemList();

  if (list_items != NULL) {
    // motif index is 1 based
    //virtarry is 0 based
    PropStringPair *pair = (*list_items)[list_info->item_position - 1];
    if (pair != NULL)
      XtVaSetValues(item->getEntryFieldWidget(), XmNvalue,pair->label, NULL);
      OptCmd *oc = theRoamApp.mailOptions();
      DtbOptionsDialogInfo od = oc->optionsDialog();
      XtSetSensitive(od->local_name_del_but, True);
      XtSetSensitive(od->local_name_chg_but, True);
  }
}


// AlternatesListUiItem::writeFromUiToSource()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void AlternatesListUiItem::writeFromUiToSource()
{
  Widget w = this->getWidget();
  SdmError error;
  SdmMailRc *mail_rc;
  SdmConnection *con = theRoamApp.connection()->connection();
  assert (con != NULL);
  
  con->SdmMailRcFactory(error, mail_rc);
  assert ( !error );
  int i, num_items;

  if(deleted_items != NULL)
  {
      num_items = deleted_items->length();

      for(i = 0; i < num_items; i++)
      {
        mail_rc->RemoveAlternate(error,(*deleted_items)[i]);
        assert ( !error );
      }
  }

  if(list_items != NULL)
  {
      num_items = list_items->length();
      for(i = 0; i < num_items; i++)
      {
        mail_rc->SetAlternate(error,(*list_items)[i]->label);
        assert ( !error );
      }
  }
  
  if(deleted_items != NULL)
  {
    delete deleted_items;
    deleted_items = NULL;
  }
}

// AlternatesListUiItem::writeFromSourceToUi()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void AlternatesListUiItem::writeFromSourceToUi()
{
  SdmError error;
  SdmMailRc *mail_rc;
  SdmConnection *con = theRoamApp.connection()->connection();
  if(con->SdmMailRcFactory(error, mail_rc))  {
    DtMailGenDialog *dialog = theRoamApp.genDialog();
    dialog->post_error(error);
  }  
  Widget w = this->getWidget();
  const char *list_str = NULL;
  DtVirtArray<char *> *char_list = NULL;
  PropStringPair *new_pair;
  char *token, *buf = NULL;
  int i, num_items;

  XmListDeleteAllItems(w);

  if (deleted_items != NULL) {
    num_items = deleted_items->length();
    for (i=0; i < num_items; i++)
            delete (*deleted_items)[i];
    delete deleted_items;
    deleted_items = NULL;
  }

  if (list_items != NULL) {
    num_items = list_items->length();
    for (i=0; i < num_items; i++)  {
        PropStringPair *psp = (*list_items)[i];
        delete psp;
    }
    delete list_items;
    list_items = NULL;
  }

  if (mail_rc) {
    if ((list_str = mail_rc->GetAlternates(error)) == NULL) 
      list_str = strdup("");
  }
  
  if ((buf = (char *) malloc(strlen(list_str) + 1)) == NULL)
    return;
    
  strcpy(buf, (char *)list_str);
  
  if ((token = (char *) strtok(buf, " "))) {
      list_items = new DtVirtArray<PropStringPair *>(10);
      char_list = new DtVirtArray<char *>(10);

      new_pair = new PropStringPair;
      new_pair->label = strdup(token);
      new_pair->value = NULL;
      list_items->append(new_pair);

      char_list->append(strdup(token));
 
      while(token = (char *)strtok(NULL, " ")) {
        new_pair = new PropStringPair;
        new_pair->label = strdup(token);
        new_pair->value = NULL;
        list_items->append(new_pair);
        char_list->append(strdup(token)); 
      }
  }

  options_list_init(w, char_list);

  if (char_list != NULL) {
    num_items = char_list->length();
    for (i=0; i < num_items; i++)  {
        char *string = (*char_list)[i];
        delete string;
    }
    delete char_list;
    char_list = NULL;
  }  

  if (list_str != NULL)
    free((void *)list_str);

  if (buf != NULL)
    free((void *)buf);

  if (list_items && list_items->length() > 0)
        XmListSelectPos(w, 1, TRUE);
  else {
        OptCmd *oc = theRoamApp.mailOptions();
        DtbOptionsDialogInfo od = oc->optionsDialog();
        XtSetSensitive(od->local_name_del_but, False);
        XtSetSensitive(od->local_name_chg_but, False);
  }
}

///////////////////////////////////////////////////////////
void AlternatesListUiItem::handleAddButtonPress()
{
  Widget entry_field = this->getEntryFieldWidget();
  char *test_str = NULL;
  PropStringPair *new_pair = NULL;

  XtVaGetValues(entry_field,
		XmNvalue, &test_str,
		NULL);

  if (test_str != NULL)
    if (strlen(test_str) > 0) {
      new_pair = new PropStringPair;
      int *pos_list, num_pos;

      new_pair->label = test_str;
      new_pair->value = NULL;

      if (XmListGetSelectedPos(this->getWidget(), &pos_list, &num_pos)) {
          if (list_items == NULL)
              list_items = new DtVirtArray<PropStringPair *>(10);

          list_items->insert(new_pair,pos_list[0] - 1); 
	  XmString lstr = XmStringCreateLocalized(new_pair->label);
          XmListAddItem(this->getWidget(), lstr, pos_list[0]);
	  XmStringFree(lstr);

          XmListSelectPos(this->getWidget(), pos_list[0], TRUE);

          XmListSetPos(this->getWidget(), pos_list[0]);
           
          free(pos_list);
      }	
      else {
          if (list_items == NULL)
              list_items = new DtVirtArray<PropStringPair *>(10);

          list_items->insert(new_pair,0); 
	  XmString lstr = XmStringCreateLocalized(new_pair->label);
          XmListAddItem(this->getWidget(), lstr, 1);
	  XmStringFree(lstr);
          XmListSelectPos(this->getWidget(), 1, TRUE);
          XmListSetPos(this->getWidget(), 1);
      }
      props_changed = TRUE;
    }
}
///////////////////////////////////////////////////////////
void AlternatesListUiItem::handleChangeButtonPress()
{
  Widget entry_field = this->getEntryFieldWidget();
  char *test_str = NULL;
  PropStringPair *new_pair = NULL;
  XmString replace_string;
  int *pos_list, num_pos;

  // if nothing selected nothing to change...
  if(XmListGetSelectedPos(this->getWidget(), &pos_list, &num_pos)) {
    XtVaGetValues(entry_field, XmNvalue, &test_str, NULL);

    if (test_str != NULL) {
      if (strlen(test_str) > 0) {

        new_pair = (*list_items)[pos_list[0] - 1];

        if (deleted_items == NULL) 
          deleted_items = new DtVirtArray<char *>(10);

        deleted_items->append(strdup((*list_items)[pos_list[0] -1]->label));

        free(new_pair->label);
        new_pair->label = test_str;

        replace_string = XmStringCreateLocalized(new_pair->label);

        XmListReplaceItemsPos(this->getWidget(), &replace_string, 1,
		 pos_list[0]);

        XmListSelectPos(this->getWidget(), pos_list[0], TRUE);
      }
    }
    props_changed = TRUE;
    free(pos_list);
  }
}
///////////////////////////////////////////////////////////
void AlternatesListUiItem::handleDeleteButtonPress()
{
  Widget list_widget = this->getWidget();
  int *p_list, num;

  // get the selected position
  if (XmListGetSelectedPos(list_widget, &p_list, &num)) {
    if (deleted_items == NULL)
      deleted_items = new DtVirtArray<char *>(10);

    deleted_items->append(strdup((*list_items)[p_list[0] -1]->label));

    // delete the item from our list.  we need to first save a pointer
    // to the item we are going to remove because the remove call
    // does not delete the item.  we'll need to do it ourself.
    PropStringPair *item = (*this->list_items)[p_list[0] - 1];
    this->list_items->remove(p_list[0] - 1); // remove only first
    delete item;

    // delete the item from the widget
    XmListDeletePos(list_widget, p_list[0]);

    XtVaSetValues(this->getEntryFieldWidget(), XmNvalue,"", NULL);
 
    XtVaGetValues(list_widget, XmNitemCount, &num, NULL);
    if (num == 0) {
        OptCmd *oc = theRoamApp.mailOptions();
        DtbOptionsDialogInfo od = oc->optionsDialog();
        XtSetSensitive(od->local_name_del_but, False);
        XtSetSensitive(od->local_name_chg_but, False);
    }
    else {
        if (p_list[0] > num)
                XmListSelectPos(list_widget, p_list[0]-1, TRUE);
        else
                XmListSelectPos(list_widget, p_list[0], TRUE);
    }
                      
    free(p_list);
    props_changed = TRUE;
  }
}




