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

#include "RoamApp.h"
#include <Xm/List.h>
#include <DtMail/options_util.h>
#include <DtMail/PropUi.hh>
#include <DtMail/ListUiItem.hh>
#include <DtMail/AliasListUiItem.hh>

extern Boolean props_changed;
void alias_stuffing_func(char * key, void * data, void * client_data);
void handleAliasSelection(Widget w, XtPointer clientdata, XtPointer calldata);

// AliasListUiItem::AliasListUiItem
// AliasListUiItem ctor
////////////////////////////////////////////////////////////////////

AliasListUiItem::AliasListUiItem(Widget w, 
		       int source, 
		       char *search_key,
		       Widget key_entry_widget,
		       Widget value_entry_widget):ListUiItem(w, source, search_key, NULL)
{
  source = source; search_key = search_key;

  key_widget = key_entry_widget;
  value_widget = value_entry_widget;

  list_items = NULL;
  deleted_items = NULL;

  XtVaSetValues(w,
	XmNuserData, this,
        XmNautomaticSelection, True,
        XmNselectionPolicy, XmBROWSE_SELECT,
	NULL);

  XtAddCallback(w,
	XmNbrowseSelectionCallback, 
	(XtCallbackProc)handleAliasSelection,
	(XtPointer)this);

}

//-----------------======================-----------------
void handleAliasSelection(Widget w, XtPointer, XtPointer calldata)
{
  AliasListUiItem *item;
  XmListCallbackStruct *list_info = (XmListCallbackStruct *)calldata;
  DtVirtArray<PropStringPair *> *list_items;

  XtVaGetValues(w, XmNuserData, &item, NULL);  

  list_items = item->getItemList();

  if(list_items != NULL) {                    // motif index is 1 based
                                              //virtarry is 0 based
      PropStringPair *pair = (*list_items)[list_info->item_position - 1];

      if(pair != NULL) {
	  XtVaSetValues(item->getKeyWidget(),
			XmNvalue,pair->label,
			NULL);
	  
	  XtVaSetValues(item->getValueWidget(),
			XmNvalue,pair->value,
			NULL);
      }
   }	
}
void AliasListUiItem::update_display()
{
  DtVirtArray<char *> list_str_list(10);
  int i, num_items;
  DtMailEnv error;
  DtMail::Session * d_session = theRoamApp.session()->session();
  DtMail::MailRc * mail_rc = d_session->mailRc(error);
  Widget w = this->getWidget();
  
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
  }

  list_items = new DtVirtArray<PropStringPair *>(10);

  mail_rc->getAliasList(alias_stuffing_func, list_items);
  num_items = list_items->length();

  for(i = 0; i < num_items; i++)
	list_str_list.append((*list_items)[i]->formatPropPair());

  options_list_init(w, &list_str_list);

  for(i = 0; i < num_items; i++)
  	free(list_str_list[i]);
}

// AliasListUiItem::writeFromUiToSource()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void AliasListUiItem::writeFromUiToSource()
{
  int i, num_items;
  DtMailEnv error;
  DtMail::Session * d_session = theRoamApp.session()->session();
  DtMail::MailRc * mail_rc = d_session->mailRc(error);

  if (deleted_items != NULL) {
      	num_items = deleted_items->length();
      	for(i = 0; i < num_items; i++) 
	  	mail_rc->removeAlias(error,(*deleted_items)[i]);
  }

  if (list_items != NULL) {
      	num_items = list_items->length();
      	for(i = 0; i < num_items; i++)
	  	mail_rc->setAlias(error,(*list_items)[i]->label,
			    (*list_items)[i]->value);
   }
   update_display();

   // This is to alert that Alias props have changed so in the 
   // SendMsgDialog we can update the alias list on the File menu.
   // when the propsChanged routine gets called.
   XtVaSetValues(key_widget, XmNuserData, 1, NULL);
}


// AliasListUiItem::writeFromSourceToUi()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void AliasListUiItem::writeFromSourceToUi()
{
  Widget w = this->getWidget();

  XmListDeleteAllItems(w);
  update_display();
}
//
// callback for creating alias list...
void alias_stuffing_func(char * key, void * data, void * client_data)
{
  DtVirtArray<PropStringPair *> *alias_list = (DtVirtArray<PropStringPair *> *)client_data;
  PropStringPair *new_pair;
  int list_len, i;
  
  new_pair = new PropStringPair;
  new_pair->label = strdup(key);
  new_pair->value = strdup((char *)data);
  
  list_len = alias_list->length();
  for (i=0; i < list_len; i++) {
	if (strcasecmp((*alias_list)[i]->label, new_pair->label) >= 0)
		break;
  }
  alias_list->insert(new_pair, i);
}



///////////////////////////////////////////////////////////
void AliasListUiItem::handleAddButtonPress()
{
  char *key_str = NULL;
  char *value_str = NULL;
  PropStringPair *new_pair = NULL;

  XtVaGetValues(key_widget,
		XmNvalue, &key_str,
		NULL);

  XtVaGetValues(value_widget,
		XmNvalue, &value_str,
		NULL);

  if (key_str && *key_str) {
	new_pair = new PropStringPair;
	int *pos_list, num_pos;

	new_pair->label = strdup(key_str);
	
	if(value_str != NULL)
	  new_pair->value = strdup(value_str);
	else
	  new_pair->value = NULL;
	
	if(XmListGetSelectedPos(this->getWidget(), &pos_list, &num_pos))
		num_pos = pos_list[0];
	else 
		num_pos = 1;

        if(list_items == NULL)
              list_items = new DtVirtArray<PropStringPair *>(10);

	list_items->insert(new_pair, num_pos-1); 
	
    	char *str = new_pair->formatPropPair();
    	XmString str2 = XmStringCreateLocalized(str);
    	free(str);
    	XmListAddItem(this->getWidget(), str2, num_pos);
    	XmStringFree(str2);
	    
    	XmListSelectPos(this->getWidget(), num_pos, TRUE);

    	XmListSetPos(this->getWidget(), num_pos); 
    	props_changed = TRUE;
   }
}
///////////////////////////////////////////////////////////
void AliasListUiItem::handleChangeButtonPress()
{
  char *key_str = NULL;
  char *value_str = NULL;  
  PropStringPair *new_pair = NULL;
  XmString replace_string;
  int *pos_list, num_pos;

  // if nothing selected nothing to change...
  if(XmListGetSelectedPos(this->getWidget(), &pos_list, &num_pos)) {

      XtVaGetValues(key_widget, XmNvalue, &key_str, NULL);
      XtVaGetValues(value_widget, XmNvalue, &value_str, NULL);

      if(key_str != NULL)
	if(strlen(key_str) > 0) {

	    new_pair = (*list_items)[pos_list[0] - 1];
	    if(deleted_items == NULL)
		deleted_items = new DtVirtArray<char *>(10);

	    deleted_items->append(strdup((*list_items)[pos_list[0] -1]->label));

	    free(new_pair->label);
	    new_pair->label = strdup(key_str);

	    if (new_pair->value != NULL) {
		free(new_pair->value);
		if(value_str != NULL)
		  new_pair->value = strdup(value_str);
		else
		  new_pair->value = NULL;
	    }

            char *str = new_pair->formatPropPair();
	    replace_string = XmStringCreateLocalized(str);
	    free(str);

	    XmListReplaceItemsPos(this->getWidget(), &replace_string, 1, pos_list[0]);
	    XmStringFree(replace_string);

	    XmListSelectPos(this->getWidget(), pos_list[0], TRUE);
        }
      props_changed = TRUE;
      free(pos_list);
  }
}
///////////////////////////////////////////////////////////
void AliasListUiItem::handleDeleteButtonPress()
{
  Widget list_widget = this->getWidget();
  int *p_list, p_count;

  // get the selected position
  if(XmListGetSelectedPos(list_widget, &p_list, &p_count)) {
      
      if (deleted_items == NULL)
	  deleted_items = new DtVirtArray<char *>(10);

      deleted_items->append(strdup((*list_items)[p_list[0] -1]->label));

      // delete the item from our list 
      this->list_items->remove(p_list[0] - 1); // remove only first

      // delete the item from the widget
      XmListDeletePos(list_widget, p_list[0]);

      XtVaSetValues(this->getKeyWidget(), XmNvalue,"", NULL);

      XtVaSetValues(this->getValueWidget(), XmNvalue,"", NULL);

      XmListSelectPos(list_widget, p_list[0], TRUE);

      props_changed = TRUE;
      free(p_list);      
    }
}
