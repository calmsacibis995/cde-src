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
#pragma ident " %W %G "
#endif

#include "MailMsg.h"
#include "RoamApp.h"
#include <Xm/List.h>
#include <DtMail/options_util.h>
#include <DtMail/FilterListUiItem.hh>

extern Boolean props_changed;
void handleSelection(Widget, XtPointer, XtPointer);

// FilterListUiItem::FilterListUiItem
// FilterListUiItem ctor
////////////////////////////////////////////////////////////////////
FilterListUiItem::FilterListUiItem(Widget w, 
		       int source, 
		       char *search_key,
                       RuleCmd *rule)
  :_rule(rule), ListUiItem(w, source, search_key, NULL)
{
  source = source; search_key = search_key;
  list_items = NULL;
  deleted_items = NULL;

  XtVaSetValues(w,
	XmNuserData, this,
        XmNautomaticSelection, True,
        XmNselectionPolicy, XmBROWSE_SELECT,
	NULL);

  XtAddCallback(w,
      XmNbrowseSelectionCallback, 
      (XtCallbackProc)handleSelection,
      (XtPointer)this);
}

//-----------------======================-----------------
void handleSelection(Widget w, XtPointer, XtPointer calldata)
{
  FilterListUiItem *item;
  XmListCallbackStruct *list_info = (XmListCallbackStruct *)calldata;
  DtVirtArray<filter *> *list_items;

  XtVaGetValues(w, XmNuserData, &item, NULL);  

  list_items = item->getItemList();

  if(list_items != NULL) {                    // motif index is 1 based
                                              //virtarry is 0 based
      filter *f = (*list_items)[list_info->item_position - 1];
      if (f != NULL) {
	  item->update_rule_dialog(f);
      }
  }

}

void FilterListUiItem::update_rule_dialog(filter *f)
{
	_rule->updateUi(f);
}

void FilterListUiItem::update_display()
{
  DtVirtArray<char *> list_str_list(10);
  int i, num_items;
  DtMailEnv error;
  DtMail::Session * d_session = theRoamApp.session()->session();
  DtMail::MailRc * mail_rc = d_session->mailRc(error);
  Widget w = this->getWidget();
  
  if (list_items != NULL) 
        delete list_items;

  list_items = new DtVirtArray<filter *>(10);
  mail_rc->getFilterList(list_items);
  num_items = list_items->length();

  for(i = 0; i < num_items; i++) {
      struct filter *f = (*list_items)[i];
      char * str = format_string((*list_items)[i]);
      list_str_list.append(str);
  }

  options_list_init(w, &list_str_list);

  for(i = 0; i < num_items; i++)
  	free(list_str_list[i]);
}

// FilterListUiItem::writeFromUiToSource()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void FilterListUiItem::writeFromUiToSource()
{
  int i, num_items;
  DtMailEnv error;
  DtMail::Session * d_session = theRoamApp.session()->session();
  DtMail::MailRc * mail_rc = d_session->mailRc(error);
  
  int total_len = 1;
  int num_rule_on = 0;

  if (deleted_items != NULL) {
      num_items = deleted_items->length();
      for (i=0; i < num_items; i++) {
	  struct filter *f = mail_rc->getFilter((*deleted_items)[i]);
	  mail_rc->removeFilter(error, f);
      }
      delete deleted_items;
      deleted_items = NULL;
  }

  if (list_items != NULL) {
      	num_items = list_items->length();
	
	// calc mailrc strlen...
	for(i = 0; i < num_items; i++) {
	    // strlen + space
	    if ((*list_items)[i]->f_act->a_string != NULL &&
		(*list_items)[i]->f_act->a_type == SAVE)
		total_len = total_len + strlen((*list_items)[i]->f_act->a_string) + 1;

	}

	char *mailboxlist = NULL;
	if (total_len <=1)
	    mail_rc->removeValue(error, "shownewmailist");
	else {
	    mailboxlist = new char[total_len];
	    mailboxlist[0] = '\0';

	    mail_rc->setFilter(error, list_items);

	    for(i = 0; i < num_items; i++) {
		// Mark down any rule that is truned on
		if ((*list_items)[i]->f_state)
		    num_rule_on++;

		// Construct the mailbox list for Show New Mail dialog
		if ((*list_items)[i]->f_act->a_string != NULL &&
		    (*list_items)[i]->f_act->a_type == SAVE) {
		    strcat(mailboxlist, (*list_items)[i]->f_act->a_string);
		    strcat(mailboxlist, " ");
		}
	    }

	    if (mailboxlist) {
		mail_rc->setValue(error, "shownewmailist", mailboxlist);
		delete []mailboxlist;
	    }
	}
    }

  update_display();
   
  // If there is any rule that is turned on, we activate mailfilter program
  // in ~/.forward file
  if (num_rule_on > 0)
      mail_rc->update_forward(1);
  else
      mail_rc->update_forward(0);

}


// FilterListUiItem::writeFromSourceToUi()
// Takes values from source DB and update the UI
///////////////////////////////////////////////////////////////////
void FilterListUiItem::writeFromSourceToUi()
{
  Widget w = this->getWidget();

  XmListDeleteAllItems(w);
  update_display();
}

///////////////////////////////////////////////////////////
void FilterListUiItem::handleAddButtonPress(filter *f)
{
  if(f != NULL) {
      int *pos_list, num_pos;

      if(XmListGetSelectedPos(this->getWidget(), &pos_list, &num_pos)) 
	  num_pos = pos_list[0];
      else
	  num_pos = 1;

      if(list_items == NULL)
	  list_items = new DtVirtArray<filter *>(10);

      list_items->insert(f, num_pos-1);
      char *str = format_string(f);
      
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
void FilterListUiItem::handleChangeButtonPress(filter *f)
{
    if (!f) return;

    XmString replace_string;
    int *pos_list, num_pos;

    // if nothing selected nothing to change...
    if(XmListGetSelectedPos(this->getWidget(), &pos_list, &num_pos)) {
	if (*f->f_name != NULL) {

	    if(deleted_items == NULL)
                deleted_items = new DtVirtArray<char *>(10);

            deleted_items->append(strdup((*list_items)[pos_list[0] -1]->f_name));

	    // Remove the old rule, and insert the new rule
	    list_items->remove(pos_list[0] -1);
	    list_items->insert(f, pos_list[0]-1);

	    char *str = format_string(f);
	    replace_string = XmStringCreateLocalized(str);
	    free(str);
	
	    XmListReplaceItemsPos(this->getWidget(), &replace_string, 1, pos_list[0]);
	    XmStringFree(replace_string);
	    XmListSelectPos(this->getWidget(), pos_list[0], TRUE);
	    props_changed = TRUE;
	}
	free(pos_list);
    }
}

///////////////////////////////////////////////////////////
void FilterListUiItem::handleDeleteButtonPress()
{
  Widget list_widget = this->getWidget();
  int *p_list, p_count;

  // get the selected position
  if(XmListGetSelectedPos(list_widget, &p_list, &p_count)) {
      if (deleted_items == NULL)
          deleted_items = new DtVirtArray<char *>(10);

      deleted_items->append(strdup((*list_items)[p_list[0] -1]->f_name));

      // delete the item from our list 
      list_items->remove(p_list[0] - 1); // remove only first

      // delete the item from the widget
      XmListDeletePos(list_widget, p_list[0]);
      XmListSelectPos(list_widget, p_list[0], TRUE);

      props_changed = TRUE;
      free(p_list);
  }

}

///////////////////////////////////////////////////////////
void FilterListUiItem::handleMoveUpButtonPress()
{
    int *position, count;
    XmStringTable items, selected_items;
    XmString selected_item;
    Boolean status;
    Widget w = this->getWidget();

    status = XmListGetSelectedPos(w, &position, &count);
    if (count == 0 || !status || *position == 1) {
        return;
    }

    if (deleted_items == NULL)
          deleted_items = new DtVirtArray<char *>(10);

    deleted_items->append(strdup((*list_items)[position[0]-1]->f_name));

    XtVaGetValues(w,
		  XmNitems, &items,
                  XmNselectedItems, &selected_items,
                  NULL);
    
    struct filter *f = copy_filter((*list_items)[position[0]-1]);
    list_items->insert(f, position[0]-2);
    list_items->remove(position[0]);

    selected_item = XmStringCopy(selected_items[0]);
    XmListDeletePos(w, position[0]);
    XmListAddItem(w, selected_item, position[0]-1);
    XmListSelectPos(w, position[0]-1, True);
    XmStringFree(selected_item);
    free(position);

    props_changed = TRUE;
}

///////////////////////////////////////////////////////////
void FilterListUiItem::handleMoveDownButtonPress()
{
    int *position, count, p_count;
    XmStringTable selected_items, items;
    XmString selected_item;
    Boolean status;
    Widget w = this->getWidget();

    status = XmListGetSelectedPos(w, &position, &p_count);
    XtVaGetValues(w,
		  XmNitems, &items,
		  XmNitemCount, &count,
                  XmNselectedItems, &selected_items,
                  NULL);

    if (count == 0 || !status || (*position) == count) {
        return;
    }


    if (deleted_items == NULL)
          deleted_items = new DtVirtArray<char *>(10);

    deleted_items->append(strdup((*list_items)[position[0]-1]->f_name));

    struct filter *f = copy_filter((*list_items)[position[0]-1]);
    list_items->insert(f, position[0]+1);
    list_items->remove(position[0]-1);

    selected_item = XmStringCopy(selected_items[0]);
    XmListDeletePos(w, position[0]);
    XmListAddItem(w, selected_item, position[0]+1);
    XmListSelectPos(w, position[0]+1, True);
    XmStringFree(selected_item);
    free(position);

    props_changed = TRUE;
}

// Format the string that will be displayed in the Auto-Filing scrolling list.
// We only display the rule name and the ON/OFF state
char *
FilterListUiItem::format_string(struct filter *f)
{

    char *str;
    if (f->f_state) 
	str = strdup(catgets(DT_catd, 4, 6, "ON"));
    else 
	str = strdup(catgets(DT_catd, 4, 7, "OFF"));
    
    char *formatted_str = NULL;
    int i, num_spaces = 0;
    int key_len = strlen(str);

    if (key_len < 13)
	num_spaces = 13 - key_len;
    formatted_str = (char*)malloc(key_len + strlen(f->f_name) + num_spaces + 5);
    strcpy(formatted_str, str);
    free(str);

    for (i=key_len; i < (key_len+num_spaces); i++)
	formatted_str[i] = ' ';	
    formatted_str[i] = '\0';	
    strcat(formatted_str, f->f_name);
    return formatted_str;
}


