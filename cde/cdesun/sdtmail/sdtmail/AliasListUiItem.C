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
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Error.hh>
#include <SDtMail/MailRc.hh>
#include <SDtMail/Connection.hh>
#include "MailMsg.h"
#include <DtMail/OptCmd.h>

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

AliasListUiItem::~AliasListUiItem()
{
  int i, num_items;

  if (list_items != NULL) {
    num_items = list_items->length();
    for (i=0; i < num_items; i++)  {
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

      OptCmd *oc = theRoamApp.mailOptions();
      DtbOptionsDialogInfo od = oc->optionsDialog();
      XtSetSensitive(od->alias_delete_but, True);
      XtSetSensitive(od->al_change_but, True);
    }
  }	
}
void AliasListUiItem::update_display()
{
  DtVirtArray<char *> list_str_list(10);
  int i, num_items;
  SdmError error;
  SdmMailRc *mail_rc;
  SdmConnection *con = theRoamApp.connection()->connection();
  assert (con != NULL);
  
  if(con->SdmMailRcFactory(error, mail_rc))  {
    DtMailGenDialog *dialog = theRoamApp.genDialog();
    dialog->post_error(error);
  }  
  
  Widget w = this->getWidget();
  assert (w != NULL);
  
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

  list_items = new DtVirtArray<PropStringPair *>(10);

  if (mail_rc) {
    mail_rc->GetAliasList(alias_stuffing_func, list_items);
  }
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
  SdmError error;
  SdmMailRc *mail_rc;
  SdmConnection *con = theRoamApp.connection()->connection();
  if(con->SdmMailRcFactory(error, mail_rc))  {
    DtMailGenDialog *dialog = theRoamApp.genDialog();
    dialog->post_error(error);
  }  

  if (mail_rc) {
    if (deleted_items != NULL) {
          num_items = deleted_items->length();
          for(i = 0; i < num_items; i++) 
        mail_rc->RemoveAlias(error,(*deleted_items)[i]);
    }

    if (list_items != NULL) {
      num_items = list_items->length();
      for(i = 0; i < num_items; i++)
        mail_rc->SetAlias(error,(*list_items)[i]->label,
            (*list_items)[i]->value);
     }
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
  if (list_items && list_items->length() > 0)
  	XmListSelectPos(w, 1, TRUE);
  else {
	OptCmd *oc = theRoamApp.mailOptions();
        DtbOptionsDialogInfo od = oc->optionsDialog();
	XtSetSensitive(od->alias_delete_but, False);
        XtSetSensitive(od->al_change_but, False);
  }
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
  int i;
  DtMailGenDialog *dialog = theRoamApp.genDialog();
  SdmError err;

  XtVaGetValues(key_widget,
		XmNvalue, &key_str,
		NULL);

  XtVaGetValues(value_widget,
		XmNvalue, &value_str,
		NULL);

  if (key_str && !theRoamApp.validateAliases(err, key_str)) {
    dialog->post_error(err, DTMAILHELPALIASESINVALIDALIAS,
		       catgets(DT_catd, 15, 23, 
			       "You have typed in an invalid alias name. Alias\nnames must not contain spaces, commas, or quotes."));
    free(key_str);
    if (value_str) free(value_str);
    return;
  }

  if (value_str && !theRoamApp.validateAliases(err, value_str)) {
    dialog->post_error(err, DTMAILHELPALIASESINVALIDADDRESS,
		       catgets(DT_catd, 15, 22, 
			       "You have typed in an invalid address list.\nBe sure to separate multiple addresses\nor aliases with commas or commas and spaces."));
    free(key_str);
    if (value_str) free(value_str);
    return;
  }

  if (key_str && (i = strlen(key_str)) > 0) {
      for (i--; i >= 0; i--) {
		if (key_str[i] == ' ') {
                        dialog->setToErrorDialog(catgets(DT_catd, 3, 48, "Mailer"),
                                catgets(DT_catd, 15, 19, 
				"An alias name cannot contain spaces."));
                        dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), 
				DTMAILHELPALIASESNOSPACES);
                        free(key_str);
                        if (value_str) free(value_str);
                        return;
                }
      } 
      // Cant add alias  with no address
      if (value_str && *value_str == '\0') {
		free(key_str);
		free(value_str);
		return;
      }
      for (i=0; list_items != NULL && i < list_items->length(); i++) {
		char *label = (*list_items)[i]->label;
		if (strcmp(label, key_str) == 0) {
                        dialog->setToErrorDialog(catgets(DT_catd, 3, 48, "Mailer"),
                                catgets(DT_catd, 15, 21, 
				"This alias name is already on the Alias list."));
                        dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), 
				DTMAILHELPALIASESDUPLICATE);
                        free(key_str);
                        free(value_str);
                        return;
		}
      }
      new_pair = new PropStringPair;
      int *pos_list=NULL, num_pos;

      new_pair->label = key_str;
      new_pair->value = value_str;

      if (XmListGetSelectedPos(this->getWidget(), &pos_list, &num_pos))
        num_pos = pos_list[0];
      else 
        num_pos = 1;

      if (list_items == NULL)
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
      if (pos_list) free(pos_list);
   }
}
///////////////////////////////////////////////////////////
void AliasListUiItem::handleChangeButtonPress()
{
  char *key_str = NULL;
  char *value_str = NULL;  
  PropStringPair *new_pair = NULL;
  XmString replace_string;
  int *pos_list, num_pos, i;
  SdmError err;

  // if nothing selected nothing to change...
  if(XmListGetSelectedPos(this->getWidget(), &pos_list, &num_pos)) {

    XtVaGetValues(key_widget, XmNvalue, &key_str, NULL);
    XtVaGetValues(value_widget, XmNvalue, &value_str, NULL);

    if (key_str && !theRoamApp.validateAliases(err, key_str)) {
        DtMailGenDialog *dialog = theRoamApp.genDialog();
	dialog->post_error(err, DTMAILHELPALIASESINVALIDALIAS,
			   catgets(DT_catd, 15, 23,
				   "You have typed in an invalid alias name. Alias names\nmust not contain spaces, commas, or quotes."));
        free(key_str);
        if (value_str) free(value_str);
        return;  
    }

    if (value_str && !theRoamApp.validateAliases(err, value_str)) {
        DtMailGenDialog *dialog = theRoamApp.genDialog();
	dialog->post_error(err, DTMAILHELPALIASESINVALIDADDRESS, 
			   catgets(DT_catd, 15, 22,
				   "You have typed in an invalid address list.\nBe sure to separate multiple addresses\nor aliases with commas or commas and spaces."));
        free(key_str);
        if (value_str) free(value_str);
        return; 
    }

    if (key_str != NULL && (i = strlen(key_str)) > 0) {
	for (i--; i >= 0; i--) {
		if (key_str[i] == ' ') {
                        DtMailGenDialog *dialog = theRoamApp.genDialog();
                        dialog->setToErrorDialog(catgets(DT_catd, 3, 48, "Mailer"),
                                catgets(DT_catd, 15, 19, 
				"An alias name cannot contain spaces."));
                        dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), 
				DTMAILHELPALIASESNOSPACES);
                        free(key_str);
                        if (value_str) free(value_str);
                        return;
                }
	} 
	// Cant change address to nothing
 	if (value_str && *value_str == '\0') {
                free(key_str);
                free(value_str);
                return;
        }
	for (i=0; list_items != NULL && i < list_items->length(); i++) {
                char *label = (*list_items)[i]->label; 
                if ((pos_list[0]-1) != i && strcmp(label, key_str) == 0) { 
                        DtMailGenDialog *dialog = theRoamApp.genDialog();
                        dialog->setToErrorDialog(catgets(DT_catd, 3, 48, "Mailer"),                                catgets(DT_catd, 15, 21, 
				"This alias name is already on the Alias list."));
                        dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), 
				DTMAILHELPALIASESDUPLICATE);
                        free(key_str);
                        free(value_str);
                        return;
                } 
        }

  	new_pair = (*list_items)[pos_list[0] - 1];

  	if(deleted_items == NULL)
            deleted_items = new DtVirtArray<char *>(10);

  	deleted_items->append(strdup((*list_items)[pos_list[0] -1]->label));

  	free(new_pair->label);
  	new_pair->label = key_str;
  	if (new_pair->value != NULL) 
	  	free(new_pair->value);
  	new_pair->value = value_str;
  	char *str = new_pair->formatPropPair();
  	replace_string = XmStringCreateLocalized(str);
  	free(str);

  	XmListReplaceItemsPos(this->getWidget(), &replace_string, 1, pos_list[0]);
  	XmStringFree(replace_string);

  	XmListSelectPos(this->getWidget(), pos_list[0], TRUE);
      	props_changed = TRUE;
    	free(pos_list);
    }
  }
}
///////////////////////////////////////////////////////////
void AliasListUiItem::handleDeleteButtonPress()
{
  Widget list_widget = this->getWidget();
  int *p_list, p_count, num;

  // get the selected position
  if (XmListGetSelectedPos(list_widget, &p_list, &p_count)) {
      
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

    XtVaSetValues(this->getKeyWidget(), XmNvalue,"", NULL);

    XtVaSetValues(this->getValueWidget(), XmNvalue,"", NULL);

    XtVaGetValues(list_widget, XmNitemCount, &num, NULL); 
    if (num == 0) {
        OptCmd *oc = theRoamApp.mailOptions();
        DtbOptionsDialogInfo od = oc->optionsDialog();
        XtSetSensitive(od->alias_delete_but, False);
        XtSetSensitive(od->al_change_but, False);
    }
    else {
    	if (p_list[0] > num)
    		XmListSelectPos(list_widget, p_list[0]-1, TRUE);
    	else
    		XmListSelectPos(list_widget, p_list[0], TRUE);
    }

    props_changed = TRUE;
    free(p_list);      
  }
}
