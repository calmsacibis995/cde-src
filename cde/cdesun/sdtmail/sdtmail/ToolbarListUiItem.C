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
#include <SDtMail/Error.hh>
#include <SDtMail/MailRc.hh>
#include <string.h>
#include <stdlib.h>
#include "RoamApp.h"
#include <DtMail/options_util.h>
#include <Xm/PushBG.h>
#include <Xm/List.h>
#include <DtMail/PropUi.hh>
#include <DtMail/ListUiItem.hh>
#include <DtMail/ToolbarListUiItem.hh>
#include <DtMail/ToolbarConfig.hh>

extern Boolean props_changed;

////////////////////////////////////////////////////////////////////
ToolbarListUiItem::ToolbarListUiItem(Widget w,
				     ToolbarConfig *toolbar,
				     int source, 
				     char *search_key,
				     Widget cmd_list)
    :ListUiItem(w, source, search_key, NULL)
{
    int len, i, j;
    CmdList *toolbar_cmds;
    RoamMenuWindow *rmw = theRoamApp.mailView(0);
    DtVirtArray<long> *toolbar_indices = rmw->getToolbarIndices();
    DtVirtArray<char *> toolbar_list(10);
    DtVirtArray<char *> command_list(20);
    Boolean found = False;
    char **sort_list;

    if (!rmw) {
	fprintf(stderr, "Unable to create Toolbar options pane, no main window.\n");
	return;
    }
    toolbar_cmds = rmw->getToolbarCmds();
    _toolbar = toolbar;
    _toolbar_list_save = new DtVirtArray<XmString>(10);
    _command_list_save = new DtVirtArray<XmString>(20);
    _cmd_list_widget = cmd_list;

    XtVaSetValues(w,
		  XmNuserData, this,
		  XmNselectionPolicy, XmSINGLE_SELECT,
		  XmNscrollBarDisplayPolicy, XmAS_NEEDED,
		  NULL);
    XtVaSetValues(_cmd_list_widget,
		  XmNselectionPolicy, XmMULTIPLE_SELECT,
		  XmNscrollBarDisplayPolicy, XmAS_NEEDED,
		  NULL);

    for (i=0; i<toolbar_indices->length(); i++) {
	toolbar_list.append((*toolbar_cmds)[(*toolbar_indices)[i]]->getLabel());
    }

    for (i=0; i<toolbar_cmds->size(); i++) {
	for (j=0; j<toolbar_indices->length(); j++) {
	    if (i == (*toolbar_indices)[j]) {
		found = True;
		break;
	    }
	}
	if (!found) {
	    command_list.append((*toolbar_cmds)[i]->getLabel());
	} else {
	    found = False;
	}
    }

    // Save the toolbar XmStrings.
    len = toolbar_list.length();
    for (i=0; i<len; i++) {
	_toolbar_list_save->append(XmStringCreateLocalized(toolbar_list[i]));
    }

    len = command_list.length();
    sort_list = (char **)malloc(len * sizeof(char *));
    for (i=0; i<len; i++) {
	sort_list[i] = command_list[i];
    }

    qsort((void *)sort_list,len,sizeof(char *),&ToolbarListUiItem::myStrCmp);

    // Save the command XmStrings.
    for (i=0; i<len; i++) {
	_command_list_save->append(XmStringCreateLocalized(sort_list[i]));
    }

    // The handleListItemSelection callback will select the corresponding
    // button in the toolbar preview pane.
    XtAddCallback(w,
		  XmNsingleSelectionCallback, 
		  (XtCallbackProc)&ToolbarListUiItem::handleListItemSelection,
		  (XtPointer)this);
}

///////////////////////////////////////////////////////////////////
int ToolbarListUiItem::myStrCmp(const void *str1, const void *str2)
{
    return(strcasecmp(*(char **)str1, *(char **)str2));
}

///////////////////////////////////////////////////////////////////
ToolbarListUiItem::~ToolbarListUiItem()
{
    int i;

    if (_toolbar_list_save) {
	for (i=0; i<_toolbar_list_save->length(); i++)
	    XmStringFree((*_toolbar_list_save)[i]);
    }
    if (_command_list_save) {
	for (i=0; i<_command_list_save->length(); i++)
	    XmStringFree((*_command_list_save)[i]);
    }
}

///////////////////////////////////////////////////////////////////
char * ToolbarListUiItem::convertToMailrcFormat(char *str)
{
    int i, len;
    char *result;

    len = strlen(str);
    result = (char *)malloc((len + 1) * sizeof(char));
    for (i=0; i<len+1; i++) {
	if (str[i] == ' ') {
	    result[i] = '_';
	} else {
	    result[i] = str[i];
	}
    }
    return (result);
}

///////////////////////////////////////////////////////////////////
void ToolbarListUiItem::writeFromUiToSource()
{
    SdmError error;
    SdmMailRc *mailrc;
    SdmConnection *con = theRoamApp.connection()->connection();
    DtMailGenDialog *dialog = theRoamApp.genDialog();
    if(con->SdmMailRcFactory(error, mailrc))  {
      dialog->post_error(error);
    }
    CmdList *toolbar_cmds = theRoamApp.mailView(0)->getToolbarCmds();
    DtVirtArray<long> *indices;	// Needs to be long to avoid a bug in 
    				// DtVirtArray class.
    char *mailrc_str, *str;
    XmStringTable items;
    Widget w = this->getWidget();
    int i, j, count;

    error.Reset();

    XtVaGetValues(w,
		  XmNitemCount, &count,
		  XmNitems, &items,
		  NULL);
		  
    if (count < 1 && mailrc) {
      mailrc->SetValue(error, "toolbarcommands", " ");
      if (error) {
          printf("Clearing 'toolbarcommands' in mailrc failed.\n");
          dialog->post_error(error);
      }
    }

    if (_toolbar_list_save) {
	for (i=0; i<_toolbar_list_save->length(); i++) {
	    XmStringFree((*_toolbar_list_save)[i]);
	}
	delete _toolbar_list_save;
	_toolbar_list_save = new DtVirtArray<XmString>(10);
    }
    if (_command_list_save) {
	for (i=0; i<_command_list_save->length(); i++) {
	    XmStringFree((*_command_list_save)[i]);
	}
	delete _command_list_save;
	_command_list_save = new DtVirtArray<XmString>(20);
    }

    mailrc_str = (char *)malloc(1024);
    mailrc_str[0] = '\0';
    indices = new DtVirtArray<long>(10);

    // Save a snapshot of the toolbar list.  This is needed so we can
    // easily reset it if the user does not save changes.
    for (i=0; i<count; i++) {
	_toolbar_list_save->append(XmStringCopy(items[i]));
	XmStringGetLtoR(items[i], XmSTRING_DEFAULT_CHARSET, &str);
	for (j=0; j<toolbar_cmds->size(); j++) {
	    if (strcmp(str, (*toolbar_cmds)[j]->getLabel()) == 0) {
		indices->append((long) j);
		str = convertToMailrcFormat((char*)(*toolbar_cmds)[j]->name());
		if (i != 0)
		    strcat(mailrc_str, " ");
		strcat(mailrc_str, str);
		free(str);
		break;
	    }
	}
    }
    theRoamApp.mailView(0)->setToolbarIndices(indices);

    // Take a snapshot of the Command list widget and save the XmStrings
    XtVaGetValues(_cmd_list_widget,
		  XmNitemCount, &count,
		  XmNitems, &items,
		  NULL);
    for (i=0; i<count; i++) {
	_command_list_save->append(XmStringCopy(items[i]));
    }

    mailrc->SetValue(error, "toolbarcommands", mailrc_str);
    if(error) {
	// we should display a dialog here informing the user
	// of our problem.  ERROR
	printf("[DEBUG] Unable to set toolbarcommands mailrc value.\n");
    }
    free(mailrc_str);
}

///////////////////////////////////////////////////////////////////
void ToolbarListUiItem::writeFromSourceToUi()
{
    SdmError error;
    SdmMailRc *mailrc;
    SdmConnection *con = theRoamApp.connection()->connection();
    if(con->SdmMailRcFactory(error, mailrc))  {
      DtMailGenDialog *dialog = theRoamApp.genDialog();
      dialog->post_error(error);
    }
    Widget w = this->getWidget();
    int i, count;
    Boolean use_icons;
    XmStringTable items;

    // OptCmd::unmanage seems to think that it needs to update the pane
    // (by calling OptCmd::update_pane) even when props_changed is false.
    // The only way the toolbar lists could get out of sync with the mailrc
    // is if props_changed were true.  It is unnecessary for us to update
    // the UI if props_changed is false.

// For some reason, props_changed is not set to Sdm_True the first time
// the options dialog is being brought up.  This is not how EVIAN behaves.
// Probably some code that is ifdef'ed out.
//     if (!props_changed)
// 	return;

    // 
    // Restore the "Toolbar" command list
    //
    count = _toolbar_list_save->length();
    items = (XmStringTable)XtMalloc(count * sizeof(XmString));
    for (i=0; i<count; i++) {
	items[i] = XmStringCopy((*_toolbar_list_save)[i]);
    }
    XmListDeleteAllItems(w);
    XtVaSetValues(w,
		  XmNitems, items,
		  XmNitemCount, count,
		  NULL);

    error.Reset();
    use_icons = True;
    if (mailrc) {
      if (mailrc->IsValueDefined("toolbarusetext") == Sdm_True)
        use_icons = False;
    }

    _toolbar->update(items, count, use_icons);
    for (i=0; i<count; i++) {
	XmStringFree(items[i]);
    }
    XtFree((char *)items);

    //
    // Restore the "Commands" command list
    //
    count = _command_list_save->length();
    items = (XmStringTable)XtMalloc(count * sizeof(XmString));
    for (i=0; i<count; i++) {
	items[i] = XmStringCopy((*_command_list_save)[i]);
    }
    XmListDeleteAllItems(_cmd_list_widget);
    XtVaSetValues(_cmd_list_widget,
		  XmNitems, items,
		  XmNitemCount, count,
		  NULL);
    for (i=0; i<count; i++) {
	XmStringFree(items[i]);
    }
    XtFree((char *)items);
}

///////////////////////////////////////////////////////////////////
void ToolbarListUiItem::handleListItemSelection(Widget w,
						XtPointer client_data,
						XtPointer calldata)
{
    XmListCallbackStruct *list_info = (XmListCallbackStruct *)calldata;
    ToolbarListUiItem *obj = (ToolbarListUiItem *)client_data;

    //
    // If the item selected is not visible, we must be here due to a selection by the
    // user in the mock toolbar (ToolbarConfig::buttonCB calls XmListSelectPos() with
    // the third argument, notify, set to True).  This causes this function to be called.
    // Since this is so, we don't need to call ToolbarConfig's selectItem method.
    // If the item selected is actually visible, we must call ToolbarConfig's selectItem()
    // method because there is no way of knowing whether the user selected the list item
    // or the toolbar button.
    //
    if (!XmListPosToBounds(w, list_info->item_position, NULL, NULL, NULL, NULL)) {
	XmListSetBottomPos(w, list_info->item_position);
    } else {
	obj->_toolbar->selectItem(list_info->item_position - 1);
    }
}

///////////////////////////////////////////////////////////
void ToolbarListUiItem::handleAddButtonPress()
{
    Widget w = this->getWidget();
    int *p_list, p_count, cmd_count=0, pos=0;
    XmStringTable items, items2;

    XtVaGetValues(_cmd_list_widget,
		  XmNselectedItemCount, &cmd_count,
		  XmNselectedItems, &items,
		  NULL);
    if(cmd_count > 0) {
	if (XmListGetSelectedPos(w, &p_list, &p_count)) {
	    pos = *p_list;
	    free (p_list);
	}
	XtVaGetValues(w,
		      XmNitemCount, &p_count,
		      NULL);

	if (pos == 0)
	    pos = p_count;

	// Add the new commands after the one selected.
	XmListAddItems(w, items, cmd_count, pos+1);

	XtVaGetValues(w,
		      XmNitems, &items2,
		      XmNitemCount, &p_count,
		      NULL);

	_toolbar->insertItems(items2, p_count, cmd_count, pos);

	// Must call XmListSelectPos after insertItems because the former
	// causes the handleListItemSelection callback to be invoked which
	// twiddles the appearance of the button in the toolbar preview.
	// That button won't be there if insertItems has not been called.
	XmListSelectPos(w, pos + cmd_count, True);

	// Cust call XmListDeleteItems last because it frees all of
	// the XmStrings that you pass in.
	//
	XmListDeleteItems(_cmd_list_widget, items, cmd_count);

	props_changed = TRUE;
    }
}

///////////////////////////////////////////////////////////
void ToolbarListUiItem::handleDeleteButtonPress()
{
    Widget w = this->getWidget();
    int *p_list, p_count, count, pos=1;
    XmString selected_item;
    XmStringTable items;
    
    if(XmListGetSelectedPos(w, &p_list, &p_count)) {
	XtVaGetValues(w,
		      XmNitemCount, &count,
		      XmNselectedItems, &items,
		      NULL);

	selected_item = XmStringCopy(items[0]);

	XmListDeletePos(w, p_list[0]);
	if (*p_list < count)
	    XmListSelectPos(w, (*p_list), True);
	else
	    XmListSelectPos(w, (*p_list)-1, True);

	//
	// Find the correct position in the command list to re-insert
	// the deleted command into.
	XtVaGetValues(_cmd_list_widget,
		      XmNitems, &items,
		      XmNitemCount, &count,
		      NULL);
	char *str1, *str2;
	XmStringGetLtoR(selected_item, XmSTRING_DEFAULT_CHARSET, &str1);
	for (int i=0; i<count; i++) {
	    XmStringGetLtoR(items[i], XmSTRING_DEFAULT_CHARSET, &str2);
	    if (strcasecmp(str1, str2) < 0) {
		pos = i + 1;
		break;
	    }
	}
	// Add this item to the end of the list
	if (i==count)
	    pos = count +1;

	XmListAddItem(_cmd_list_widget, selected_item, pos);

	_toolbar->removeItem(p_list[0] - 1);

	props_changed = TRUE;
	free(p_list);
    }
}


///////////////////////////////////////////////////////////
void ToolbarListUiItem::handleMoveUpButtonPress()
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

    XtVaGetValues(w,
		  XmNitems, &items,
                  XmNselectedItems, &selected_items,
                  NULL);

    _toolbar->swapLabels(items, *position-1, *position-2);

    selected_item = XmStringCopy(selected_items[0]);
    XmListDeletePos(w, *position);
    XmListAddItem(w, selected_item, (*position)-1);
    XmListSelectPos(w, (*position)-1, True);
    XmStringFree(selected_item);

    XtFree((char *)position);
}

///////////////////////////////////////////////////////////
void ToolbarListUiItem::handleMoveDownButtonPress()
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

    _toolbar->swapLabels(items, *position, *position-1);

    selected_item = XmStringCopy(selected_items[0]);
    XmListDeletePos(w, *position);
    XmListAddItem(w, selected_item, (*position)+1);
    XmListSelectPos(w, (*position)+1, True);
    XmStringFree(selected_item);

    XtFree((char *)position);
}
