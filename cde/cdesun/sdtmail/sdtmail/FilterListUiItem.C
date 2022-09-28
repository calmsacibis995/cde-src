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
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Vector.hh>
#include "RoamApp.h"
#include <Xm/List.h>
#include <DtMail/options_util.h>
#include <DtMail/FilterListUiItem.hh>
#include <parse.h>
#include <filter.h>
#include <DtVirtArray.hh>

extern SdmBoolean props_changed;
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
  _list_items = NULL;
  _deleted_items = NULL;

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

//
///////////////////////////////////////////////////////////////////
void handleSelection(Widget w, XtPointer, XtPointer calldata)
{
  FilterListUiItem *item;
  XmListCallbackStruct *list_info = (XmListCallbackStruct *)calldata;
  DtVirtArray<filter *> *filter_items;

  XtVaGetValues(w, XmNuserData, &item, NULL);  

  filter_items = item->getItemList();

  if(filter_items != NULL) {                    // motif index is 1 based
                                              //virtarry is 0 based
      filter *f = (*filter_items)[list_info->item_position - 1];
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
    SdmStringL list_str_list;
    int i, num_items;
    struct filter *f;
    SdmString str;
    Widget w = this->getWidget();
  
    if (_list_items != NULL) 
	delete _list_items;

    _list_items = new DtVirtArray<filter *>(10);
    getFilterList(_list_items);
    
    num_items = _list_items->length();

    for(i = 0; i < num_items; i++) {
	f = (*_list_items)[i];
	str = format_string((*_list_items)[i]);
	list_str_list.AddElementToList(str);
    }
    options_list_init(w, &list_str_list);
}

void FilterListUiItem::getFilterList(DtVirtArray<filter *> *)
{
}

void FilterListUiItem::removeFilter(char *)
{
}

void FilterListUiItem::setFilter(DtVirtArray<filter *> *)
{
}

void FilterListUiItem::updateForward(int)
{
}




// FilterListUiItem::writeFromUiToSource()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void FilterListUiItem::writeFromUiToSource()
{
  int i, num_items;
  SdmError error;
  SdmMailRc * mail_rc;
  int num_rule_on = 0;

  theRoamApp.connection()->connection()->SdmMailRcFactory(error, mail_rc);

  if (_deleted_items != NULL) {
      num_items = _deleted_items->length();
      for (i=0; i < num_items; i++) {
	  removeFilter((*_deleted_items)[i]);
      }
      delete _deleted_items;
      _deleted_items = NULL;
  }

  if (_list_items != NULL) {
      SdmString mbox_list;

      num_items = _list_items->length();
      setFilter(_list_items);
      for(i = 0; i < num_items; i++) {
	  if (i != 0)
	      mbox_list += " ";
	  if ((*_list_items)[i]->f_act->a_string != NULL &&
	      (*_list_items)[i]->f_act->a_type == SAVE) {
	      mbox_list += (*_list_items)[i]->f_act->a_string;
	  }
	  if ((*_list_items)[i]->f_state)
	      num_rule_on++;
      }

      // DEBUG
      mbox_list.Print();

      if (mbox_list.Length() != 0)
	  mail_rc->SetValue(error, "shownewmailist", (const char *)mbox_list);
  }
  update_display();
   
  // If there is any rule that is turned on, we activate mailfilter program
  // in ~/.forward file
  if (num_rule_on > 0)
      updateForward(1);
  else
      updateForward(0);
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

      if(_list_items == NULL)
	  _list_items = new DtVirtArray<filter *>(10);

      _list_items->insert(f, num_pos-1);
      char *str = format_string(f);
      
      XmString str2 = XmStringCreateLocalized(str);
      free(str);
      XmListAddItem(this->getWidget(), str2, num_pos);
      XmStringFree(str2);
      
      XmListSelectPos(this->getWidget(), num_pos, Sdm_True);
      XmListSetPos(this->getWidget(), num_pos);

      props_changed = Sdm_True;
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

	    if(_deleted_items == NULL)
                _deleted_items = new DtVirtArray<char *>(10);

            _deleted_items->append(strdup((*_list_items)[pos_list[0]-1]->f_name));

	    // Remove the old rule, and insert the new rule
	    _list_items->remove(pos_list[0] -1);
	    _list_items->insert(f, pos_list[0]-1);

	    char *str = format_string(f);
	    replace_string = XmStringCreateLocalized(str);
	    free(str);
	
	    XmListReplaceItemsPos(this->getWidget(), &replace_string, 1,
				  pos_list[0]);
	    XmStringFree(replace_string);
	    XmListSelectPos(this->getWidget(), pos_list[0], Sdm_True);
	    props_changed = Sdm_True;
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
	if (_deleted_items == NULL)
	    _deleted_items = new DtVirtArray<char *>(10);

	_deleted_items->append(strdup((*_list_items)[p_list[0] -1]->f_name));

	// delete the item from our list 
	_list_items->remove(p_list[0] - 1); // remove only first

	// delete the item from the widget
	XmListDeletePos(list_widget, p_list[0]);
	XmListSelectPos(list_widget, p_list[0], Sdm_True);

	props_changed = Sdm_True;
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

    if (_deleted_items == NULL)
          _deleted_items = new DtVirtArray<char *>(10);

    _deleted_items->append(strdup((*_list_items)[position[0]-1]->f_name));

    XtVaGetValues(w,
		  XmNitems, &items,
                  XmNselectedItems, &selected_items,
                  NULL);
    
    struct filter *f = copy_filter((*_list_items)[position[0]-1]);
    _list_items->insert(f, position[0]-2);
    _list_items->remove(position[0]);

    selected_item = XmStringCopy(selected_items[0]);
    XmListDeletePos(w, position[0]);
    XmListAddItem(w, selected_item, position[0]-1);
    XmListSelectPos(w, position[0]-1, True);
    XmStringFree(selected_item);
    free(position);

    props_changed = Sdm_True;
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


    if (_deleted_items == NULL)
          _deleted_items = new DtVirtArray<char *>(10);

    _deleted_items->append(strdup((*_list_items)[position[0]-1]->f_name));

    struct filter *f = copy_filter((*_list_items)[position[0]-1]);
    _list_items->insert(f, position[0]+1);
    _list_items->remove(position[0]-1);

    selected_item = XmStringCopy(selected_items[0]);
    XmListDeletePos(w, position[0]);
    XmListAddItem(w, selected_item, position[0]+1);
    XmListSelectPos(w, position[0]+1, True);
    XmStringFree(selected_item);
    free(position);

    props_changed = Sdm_True;
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


