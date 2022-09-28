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
#include <DtMail/MoveMenuListUiItem.hh>

extern void handleIgnoreSelection(Widget, XtPointer, XtPointer );
extern Boolean props_changed;

// MoveMenuListUiItem::MoveMenuListUiItem
// MoveMenuListUiItem ctor
////////////////////////////////////////////////////////////////////

MoveMenuListUiItem::MoveMenuListUiItem(Widget w, 
		       int source, 
		       char *search_key,
		       Widget text_entry_widget):ListUiItem(w, source, search_key, NULL)
{
  source = source; search_key = search_key;

  entry_field_widget = text_entry_widget;

  list_items = NULL;

  XtVaSetValues(w,
	XmNuserData, this,
        XmNautomaticSelection, True,
        XmNselectionPolicy, XmBROWSE_SELECT,
	NULL);

    XtAddCallback(w,
		XmNbrowseSelectionCallback, 
		(XtCallbackProc)handleMMSelection,
		  (XtPointer)this);


}
//---------------------------------------------------------------
void handleMMSelection(Widget w, XtPointer, XtPointer calldata)
{
  MoveMenuListUiItem *item;
  XmListCallbackStruct *list_info = (XmListCallbackStruct *)calldata;
  char *selection_string = NULL;
  DtVirtArray<char *> *list_items;

  XtVaGetValues(w, 
	XmNuserData, &item,
	NULL);  

  list_items = item->getItemList();

  if (list_items != NULL) {
      // motif index is 1 based
      //virtarry is 0 based
      char *ptr = (*list_items)[list_info->item_position - 1];

      if(ptr != NULL)
          XtVaSetValues(item->getEntryFieldWidget(),
                        XmNvalue, ptr,
                        NULL);
    }
}

// MoveMenuListUiItem::writeFromUiToSource()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void MoveMenuListUiItem::writeFromUiToSource()
{
  Widget w = this->getWidget();
  DtMailEnv error;
  DtMail::Session * d_session = theRoamApp.session()->session();
  DtMail::MailRc * mail_rc = d_session->mailRc(error);
  int i, num_items, total_len = 0;
  char *mm_str = NULL;

  if (list_items && list_items->length() > 0) {
	  num_items = list_items->length();

	  // calc mailrc strlen...
	  total_len = 1; // add space for the terminator...
	  for(i = 0; i < num_items; i++) {
	      // strlen + space
	      total_len = total_len + strlen((*list_items)[i]) + 1;
	  }

	  mm_str = (char *)malloc(total_len);
	  mm_str[0] = '\0';

	  for(i = 0; i < num_items; i++) {
	      strcat(mm_str, (*list_items)[i]);
	      strcat(mm_str, " ");
	  }
	
	  mail_rc->setValue(error, "filemenu2", mm_str);
	  free(mm_str);
  }
  else
      mail_rc->removeValue(error, "filemenu2");

}
int MoveMenuListUiItem::myStrCmp(const char **str1, const char **str2)
{
    return(strcoll(*str1, *str2));
}

// MoveMenuListUiItem::writeFromSourceToUi()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void MoveMenuListUiItem::writeFromSourceToUi()
{
  DtMailEnv error;
  DtMail::Session * d_session = theRoamApp.session()->session();
  DtMail::MailRc * mail_rc = d_session->mailRc(error);
  Widget w = this->getWidget();
  char *list_str = NULL, *token;
  int num_items, i;

  XmListDeleteAllItems(w);

  if (list_items != NULL) {
  	num_items = list_items->length();
	for (i=0; i < num_items; i++) 
		free((*list_items)[i]);
  	delete list_items;
	list_items = NULL;
  }

  mail_rc->getValue(error, "filemenu2", &list_str);
  if (list_str == NULL) return;
  if (*list_str == NULL) {
	free(list_str);
	return;
  }

  char * expanded_str = d_session->expandPath(error, list_str);
  free(list_str);

  if ((token = (char *) strtok(expanded_str, " "))) {

      list_items = new DtVirtArray<char *>(10);

      list_items->append(strdup(token));
      
      while(token = (char *)strtok(NULL, " ")) 
	  	list_items->append(strdup(token));

      options_list_init(w, list_items);
  }
  free(expanded_str);
}

///////////////////////////////////////////////////////////
void MoveMenuListUiItem::handleAddButtonPress()
{
  Widget entry_field = this->getEntryFieldWidget();
  char *test_str = NULL;
  char *ptr = NULL;

  XtVaGetValues(entry_field,
		XmNvalue, &test_str,
		NULL);

  if (test_str && *test_str) {
	int *pos_list, num_pos;
	if(XmListGetSelectedPos(this->getWidget(), &pos_list, &num_pos)) {
		num_pos = pos_list[0];
  		free(pos_list);
	}
	else
		num_pos = 1;

    	if (list_items == NULL)
	      	list_items = new DtVirtArray<char *>(10);

    	list_items->insert(strdup(test_str), num_pos - 1); 
	XmString lstr = XmStringCreateLocalized(test_str);
    	XmListAddItem(this->getWidget(), lstr, num_pos);
	XmStringFree(lstr);
    	XmListSelectPos(this->getWidget(), num_pos, TRUE);
    	XmListSetPos(this->getWidget(), num_pos); 
	props_changed = TRUE;
      }
}
///////////////////////////////////////////////////////////
void MoveMenuListUiItem::handleChangeButtonPress()
{
  Widget entry_field = this->getEntryFieldWidget();
  char *test_str = NULL;
  XmString replace_string;
  int *pos_list, num_pos;

  // if nothing selected nothing to change...
  if(XmListGetSelectedPos(this->getWidget(), &pos_list, &num_pos)) {

      XtVaGetValues(entry_field, XmNvalue, &test_str, NULL);
      if (test_str && *test_str) {

	    free((*list_items)[pos_list[0] - 1]);
	    list_items->remove(pos_list[0] - 1);
   	    list_items->insert(strdup(test_str), pos_list[0] - 1);
  
	    replace_string = XmStringCreateLocalized(test_str);
	    XmListReplaceItemsPos(this->getWidget(),
				  &replace_string,
				  1,
				  pos_list[0]);
	    XmStringFree(replace_string);

	    XmListSelectPos(this->getWidget(),
			    pos_list[0],
			    TRUE);
	    props_changed = TRUE;
  	    free(pos_list);
      }
   }
}
///////////////////////////////////////////////////////////
void MoveMenuListUiItem::handleDeleteButtonPress()
{
  Widget list_widget = this->getWidget();
  int *p_list, p_count;

  // get the selected position 
    if(XmListGetSelectedPos(list_widget, &p_list, &p_count)) {
      
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


///////////////////////////////////////////////////////////
void MoveMenuListUiItem::handleBrowseButtonPress(char *selection)
{
	if (list_items == NULL)
		list_items = new DtVirtArray<char *>(10);

	XmString lstr = XmStringCreateLocalized(selection);
	Widget w = this->getWidget();
	list_items->insert(strdup(selection), 0);
	XmListAddItem(w, lstr, 1);
	XmListSelectPos(w, 1, TRUE);
	XmListSetPos(w, 1);
	
	XmStringFree(lstr);
}
void MoveMenuListUiItem::handleMoveUpButtonPress()
{
    int *position, count;
    XmStringTable selected_items;
    XmString selected_item;
    Boolean status;
    Widget w = this->getWidget();
 
    status = XmListGetSelectedPos(w, &position, &count);
    if (count == 0 || !status || *position == 1) {
        return;
    }

    XtRemoveCallback(w,
                      XmNbrowseSelectionCallback,
                      (XtCallbackProc)handleMMSelection,
                      (XtPointer) this );
 
    XtVaGetValues(w,
                  XmNselectedItems, &selected_items,
                  NULL);

    char *save_item = (*list_items)[*position-1];
    list_items->remove(*position-1);
    list_items->insert(save_item, *position-2);
 
    selected_item = XmStringCopy(selected_items[0]);
    XmListDeletePos(w, *position);
    XmListAddItem(w, selected_item, (*position)-1);
    XmListSelectPos(w, (*position)-1, True);
    XmStringFree(selected_item);
 
    XtFree((char *)position);

    XtAddCallback(w,
                XmNbrowseSelectionCallback,
                (XtCallbackProc)handleMMSelection,
                  (XtPointer)this);
}
 
///////////////////////////////////////////////////////////
void MoveMenuListUiItem::handleMoveDownButtonPress()
{
    int *position, count, p_count;
    XmStringTable selected_items;
    XmString selected_item;
    Boolean status;
    Widget w = this->getWidget();
 

    status = XmListGetSelectedPos(w, &position, &p_count);
    XtVaGetValues(w,
                  XmNitemCount, &count,
                  XmNselectedItems, &selected_items,
                  NULL);
 
    if (count == 0 || !status || (*position) == count) {
        return;
    }
    char *save_item = (*list_items)[*position-1];
    list_items->remove(*position-1);
    list_items->insert(save_item, *position);

    selected_item = XmStringCopy(selected_items[0]);
    XmListDeletePos(w, *position);
    XmListAddItem(w, selected_item, (*position)+1);
    XmListSelectPos(w, (*position)+1, True);
    XmStringFree(selected_item);
 
    XtFree((char *)position);
}
void MoveMenuListUiItem::handleAlphabetizeButtonPress()
{
    int num_items = list_items->length();
    if (num_items < 1) return;

    Widget w = this->getWidget();

    char **sort = (char**)malloc(num_items * sizeof (char*));
    int i;
    for (i = 0; i < num_items; i++)
	sort[i] = (*list_items)[i];
    qsort(sort, list_items->length(), sizeof(char*),
	 (int (*)(const void *, const void *))myStrCmp);
    // Remove and readd to list...
    while(list_items->length())
	list_items->remove(0);
    for (i = 0; i < num_items; i++)
	list_items->append(sort[i]);
    free(sort);
    options_list_init(w, list_items);
}
