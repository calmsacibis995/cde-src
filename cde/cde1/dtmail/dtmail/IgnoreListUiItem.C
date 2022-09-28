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
#include <DtMail/IgnoreListUiItem.hh>

void handleIgnoreSelection(Widget w, XtPointer clientdata, XtPointer calldata);
extern Boolean props_changed;

// IgnoreListUiItem::IgnoreListUiItem
// IgnoreListUiItem ctor
////////////////////////////////////////////////////////////////////

IgnoreListUiItem::IgnoreListUiItem(Widget w, 
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
		(XtCallbackProc)handleIgnoreSelection,
		  (XtPointer)this);


}
//---------------------------------------------------------------
void handleIgnoreSelection(Widget w, XtPointer, XtPointer calldata)
{
  IgnoreListUiItem *item;
  XmListCallbackStruct *list_info = (XmListCallbackStruct *)calldata;
  char *selection_string = NULL;
  DtVirtArray<PropStringPair *> *list_items;

  XtVaGetValues(w, 
	XmNuserData, &item,
	NULL);  

  list_items = item->getItemList();

  if(list_items != NULL)
    {
      // motif index is 1 based
      //virtarry is 0 based
      PropStringPair *pair = (*list_items)[list_info->item_position - 1];

      if(pair != NULL)
        {
          XtVaSetValues(item->getEntryFieldWidget(),
                        XmNvalue,pair->label,
                        NULL);

	}	

    }
}

// IgnoreListUiItem::writeFromUiToSource()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void IgnoreListUiItem::writeFromUiToSource()
{
  Widget w = this->getWidget();
  DtMailEnv error;
  DtMail::Session * d_session = theRoamApp.session()->session();
  DtMail::MailRc * mail_rc = d_session->mailRc(error);
  int i, num_items;

  if(deleted_items != NULL)
    {
      num_items = deleted_items->length();

      for(i = 0; i < num_items; i++)
	{
	  mail_rc->removeIgnore(error,(*deleted_items)[i]);
	}
    }

  if(list_items != NULL)
    {
      num_items = list_items->length();
      for(i = 0; i < num_items; i++)
	{
	  mail_rc->addIgnore(error,(*list_items)[i]->label);
	}
    }
  
  if(deleted_items != NULL)
    {
      delete deleted_items;
      deleted_items = NULL;
    }
}

// IgnoreListUiItem::writeFromSourceToUi()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void IgnoreListUiItem::writeFromSourceToUi()
{
  DtMailEnv error;
  DtMail::Session * d_session = theRoamApp.session()->session();
  DtMail::MailRc * mail_rc = d_session->mailRc(error);
  DtVirtArray<char *> *char_list = NULL;
  int list_len, i;
  PropStringPair *new_pair;

  Widget w = this->getWidget();

  XmListDeleteAllItems(w);

  if(deleted_items != NULL)
    {
      delete deleted_items;
      deleted_items = NULL;
    }

  if(list_items != NULL)
    delete list_items;

  // ignore list code here...
  char_list = mail_rc->getIgnoreList();
  
  if(char_list != NULL)
    {
      list_len = char_list->length();
      list_items = new DtVirtArray<PropStringPair *>(10);

      for(i = 0; i < list_len; i++)
	{
	  new_pair = new PropStringPair;
	  new_pair->label = strdup((*char_list)[i]);
	  new_pair->value = NULL;

	  list_items->append(new_pair);
	}
    }
  
  options_list_init(w, char_list);
  
}
///////////////////////////////////////////////////////////
void IgnoreListUiItem::handleAddButtonPress()
{
  Widget entry_field = this->getEntryFieldWidget();
  char *test_str = NULL;
  PropStringPair *new_pair = NULL;

  XtVaGetValues(entry_field,
		XmNvalue, &test_str,
		NULL);

  if (test_str && *test_str) {
	new_pair = new PropStringPair;
	int *pos_list, num_pos;

	new_pair->label = strdup(test_str);
	new_pair->value = NULL;
      
	if (XmListGetSelectedPos(this->getWidget(), &pos_list, &num_pos)) {
 		num_pos = pos_list[0];
  		free(pos_list);
	}
	else
		num_pos = 1;

        if (list_items == NULL)
              list_items = new DtVirtArray<PropStringPair *>(10);

        list_items->insert(new_pair, num_pos - 1); 
	XmString lstr = XmStringCreateLocalized(new_pair->label);
        XmListAddItem(this->getWidget(), lstr, num_pos);
        XmStringFree(lstr);
	XmListSelectPos(this->getWidget(), num_pos, TRUE);

        XmListSetPos(this->getWidget(), num_pos);
	    
 	props_changed = TRUE;
      }
}
///////////////////////////////////////////////////////////
void IgnoreListUiItem::handleChangeButtonPress()
{
  Widget entry_field = this->getEntryFieldWidget();
  char *test_str = NULL;
  PropStringPair *new_pair = NULL;
  XmString replace_string;
  int *pos_list, num_pos;

  // if nothing selected nothing to change...
  if(XmListGetSelectedPos(this->getWidget(),
			  &pos_list,
			  &num_pos))
    {
      XtVaGetValues(entry_field,
		    XmNvalue, &test_str,
		    NULL);

      if(test_str != NULL)
	if(strlen(test_str) > 1)
	  {

	    new_pair = (*list_items)[pos_list[0] - 1];

	    if(deleted_items == NULL)
		deleted_items = new DtVirtArray<char *>(10);

	    deleted_items->append(strdup((*list_items)[pos_list[0] -1]->label));

	    free(new_pair->label);
	    new_pair->label = strdup(test_str);
      
	    list_items->insert(new_pair,pos_list[0] - 1); 
	    replace_string = XmStringCreateLocalized(new_pair->label);

	    XmListReplaceItemsPos(this->getWidget(),
				  &replace_string,
				  1,
				  pos_list[0]);

	    XmListSelectPos(this->getWidget(),
			    pos_list[0],
			    TRUE);
	    XmStringFree(replace_string);
	  }
 	props_changed = TRUE;
	free(pos_list);
    }
}
///////////////////////////////////////////////////////////
void IgnoreListUiItem::handleDeleteButtonPress()
{
  Widget list_widget = this->getWidget();
  int *p_list, p_count;

  // get the selected position
  if(XmListGetSelectedPos(list_widget,
			  &p_list,
			  &p_count))
    {
      
      if(deleted_items == NULL)
	{
	  deleted_items = new DtVirtArray<char *>(10);
	}	

      deleted_items->append(strdup((*list_items)[p_list[0] -1]->label));

      // delete the item from our list 
      this->list_items->remove(p_list[0] - 1); // remove only first

      // delete the item from the widget
      XmListDeletePos(list_widget, p_list[0]);

      XtVaSetValues(this->getEntryFieldWidget(),
                        XmNvalue,"",
                        NULL);
      XmListSelectPos(list_widget,
		      p_list[0],
		      TRUE);
      props_changed = TRUE;
      
    }



}
