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

#include <ctype.h>
#include <sys/param.h>
#include "RoamApp.h"
#include <Xm/List.h>
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Session.hh>
#include <SDtMail/MailRc.hh>
#include <SDtMail/Utility.hh>
#include <DtMail/options_util.h>
#include <DtMail/PropUi.hh>
#include <DtMail/ListUiItem.hh>
#include <DtMail/MoveMenuListUiItem.hh>
#include <DtMail/OptCmd.h>
#include <nl_types.h>

extern nl_catd DT_catd;

extern SdmBoolean props_changed;

// MoveMenuListUiItem::MoveMenuListUiItem
// MoveMenuListUiItem ctor
////////////////////////////////////////////////////////////////////

MoveMenuListUiItem::MoveMenuListUiItem(Widget w, 
		       int source, 
		       char *search_key,
		       Widget text_entry_widget,
		       Widget server_text_field,
   		       Widget local_imap_rb)
:ListUiItem(w, source, search_key, (DtVirtArray<char *> *)NULL)
{
  source = source; search_key = search_key;

  _mailbox_folder_widget = text_entry_widget;
  _server_widget = server_text_field;
  _local_imap_radiob = local_imap_rb;

  WidgetList items;
  int values[2] = { 0, 1 };

  XtVaGetValues(_local_imap_radiob, XmNchildren, &items, NULL);

  options_radiobox_init(_local_imap_radiob, 2, items, values, &dirty_bit);

  _list_items = NULL;

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
  DtVirtArray<char *> *_list_items;

  XtVaGetValues(w, XmNuserData, &item, NULL);  

  _list_items = item->getItemList();

  if (_list_items != NULL) {
      // motif index is 1 based
      char *ptr = (*_list_items)[list_info->item_position - 1];

      if (ptr != NULL) {

   	 char *folder, *server;
	 theRoamApp.fromLabelToServerMb(ptr, &server, &folder);

	 XtVaSetValues(item->getServerWidget(),
		XmNvalue, server ? server : "",
		NULL);

	 options_radiobox_set_value(item->getRadioWidget(), 
			server ? 1 : 0, TRUE);

	 XtSetSensitive(item->getServerWidget(), server ? True : False);
	 if (server)
		free(server);

         XtVaSetValues(item->getEntryFieldWidget(),
                        XmNvalue, folder ? folder : "",
                        NULL);
	 if (folder) 
	 	free(folder);
      }
      OptCmd *oc = theRoamApp.mailOptions();
      DtbOptionsDialogInfo od = oc->optionsDialog();
      XtSetSensitive(od->mm_delete_butt, True);
      XtSetSensitive(od->mm_change_butt, True);
      XtSetSensitive(od->alphalist_but, True);
   }
}

// MoveMenuListUiItem::writeFromUiToSource()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void MoveMenuListUiItem::writeFromUiToSource()
{
  SdmError error;
  SdmSession * d_session = theRoamApp.isession()->isession();
  SdmMailRc * mail_rc;
  theRoamApp.connection()->connection()->SdmMailRcFactory(error, mail_rc);

  // Right now we don't store the server type or login name with the list items
  // and we dont store the logins in .mailrc. If there is a server name
  // we infer that it is imap, if no server name, than local. And we assume
  // no login is stored. Eventually we need to store this info when we support
  // server types other then local and imap and when we support the storing
  // of different logins per mailbox/server pairs. Ideally we want to store in 
  // the form mailbox,serverName,serverType,loginName. For now, we are storing 
  // just  the mailbox name for local mailboxes, and mailbox,serverName,imap 
  // for imap mailboxes. For Sputnik we create a list called movemenu, 
  // but maintain the old filemenu2 list as well for backward compatibility

    if (_list_items) {
  	int num_items;
	if ((num_items = _list_items->length()) < 1)  {
      	    mail_rc->RemoveValue(error, "movemenu");
      	    mail_rc->RemoveValue(error, "filemenu2");
	    return;
	}

  	char *old_mm_str, *mm_str, *p, *ptmp;
  	char string[2048];

	int i, total_len = 1, j = 0;
	for(i = 0; i < num_items; i++) {
	    // Need length to store the string "imap" for imap mailboxes 
	    for (p = (*_list_items)[i]; *p != '\0'; p++)
		if ( isspace(*p) || *p == ',' )
		    j++;
	    total_len = total_len + strlen((*_list_items)[i]) + 5;
	}

	mm_str = (char *)malloc(total_len + j); // j counts the number of spaces
  	mm_str[0] = '\0';
	// This is overkill but...
	old_mm_str = (char *)malloc(total_len + j);
  	old_mm_str[0] = '\0';

	for(i = 0; i < num_items; i++) {
		char *ptr;
		// Store the mailbox and server name and make a comma
		// separated string with 'imap' appended.
		strcpy(string, (*_list_items)[i]);

		// Prepend backlash in front of every space or comma (',')
		for (p = string; *p != '\0'; p++) {
			if ( isspace(*p) || *p == ',' ) {
				for (ptmp = p + strlen(p); p <= ptmp; ptmp--)
					*(ptmp + 1) = *ptmp;
				*p = '\\';
				++p;
			}
		}
		if ((ptr = strrchr(string, '(')) != NULL) {
			*ptr++ = ',';
			if ((ptr = strrchr(ptr, ')')) != NULL) 
				strcpy(ptr, ",imap");
		}
		else {
			strcat(old_mm_str, string);
			strcat(old_mm_str, " ");
		}
		strcat(mm_str, string);
		strcat(mm_str, " ");
	}
	
	mail_rc->SetValue(error, "movemenu", mm_str);
	mail_rc->SetValue(error, "filemenu2", old_mm_str);
	free(mm_str);
	free(old_mm_str);
    }
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
  	SdmError error;
  	SdmSession * d_session = theRoamApp.isession()->isession();
  	SdmMailRc * mail_rc;
	theRoamApp.connection()->connection()->SdmMailRcFactory(error, mail_rc);
  	Widget w = this->getWidget();
  	char *filemenu = NULL, *list_str = NULL;
  	int num_items, i;
	char buf[MAXPATHLEN + MAXNAMELEN];
	char *p, *ptmp, *start_ptr, *serverTypePtr;

  	XmListDeleteAllItems(w);

  	if (_list_items != NULL) {
  		num_items = _list_items->length();
		for (i=0; i < num_items; i++) 
			free((*_list_items)[i]);
  		delete _list_items;
		_list_items = NULL;
  	}

  	mail_rc->GetValue(error, "movemenu", &list_str);
	if (list_str != NULL && *list_str != NULL) {
  		// The items are specified in the following syntax:
  		// mailboxname,serverName,serverType,loginName
  		// where serverType so far is imap and loginName doesn't exist.
 		// Single specifed Mailbox Names are assumed to be local.

  		char *expanded_token;
		_list_items = new DtVirtArray<char *>(10);

		// Clean up the backslashes
		start_ptr = list_str;
		for (p = list_str; p && *p; p++) {
          // Search for the end of mailbox name
          if ( isspace(*p) && !isspace(*(p-1)) && *(p-1) != '\\' ) {
			strncpy(buf, start_ptr, p - start_ptr);
			buf[p - start_ptr] = '\0';
			// squeeze out the backslashes from mailbox name
			for (ptmp = strchr(buf, '\\'); ptmp; ptmp = strchr(buf, '\\')) {
			    for (; ptmp && *ptmp; ptmp++)
				*ptmp = *(ptmp + 1);
			}

			SdmUtility::ExpandPath(error, expanded_token,
					buf, *mail_rc,
					SdmUtility::kFolderResourceName);
			//theRoamApp.fromTokenToLabel(expanded_token);
			if ( (serverTypePtr = strstr(expanded_token, ",imap")) != NULL ) {
			    *serverTypePtr = ')';
			    *(serverTypePtr + 1) = '\0';
			    if ( (serverTypePtr = strrchr(expanded_token, ',')) != NULL ) {
				*serverTypePtr = '(';
			    }
			}
			_list_items->append(expanded_token);

			start_ptr = p+1;
          }
		}
	}
	if (list_str) {
		free(list_str);
		list_str = NULL;
	}
	

	// Create list for old move menu for easy comparison, add any old 
	// mailboxes that arent in new list, to it.
  	mail_rc->GetValue(error, "filemenu2", &list_str);
	if (list_str != NULL && *list_str != NULL) {
  		char *expanded_token;
		start_ptr = list_str;
		for (p = list_str; *p; p++) {
          if ( isspace(*p) && !isspace(*(p-1)) && *(p-1) != '\\' ) {
            strncpy(buf, start_ptr, p - start_ptr);
            buf[p - start_ptr] = '\0';
            for (ptmp = strchr(buf, '\\'); ptmp; ptmp = strchr(buf, '\\')) {
                for (; *ptmp; ptmp++)
                *ptmp = *(ptmp + 1);
            }

            SdmUtility::ExpandPath(error, expanded_token,
                    buf, *mail_rc,
                    SdmUtility::kFolderResourceName);
                    
            //theRoamApp.fromTokenToLabel(expanded_token);
            if ( (serverTypePtr = strstr(expanded_token, ",imap")) != NULL ) {
                *serverTypePtr = ')';
                *(serverTypePtr + 1) = '\0';
                if ( (serverTypePtr = strrchr(expanded_token, ',')) != NULL ) {
                *serverTypePtr = '(';
                }
            }

            if (_list_items == NULL) {
              _list_items = new DtVirtArray<char *>(10);
            }

            if ( !inList(expanded_token) )
                _list_items->append(expanded_token);

            start_ptr = p+1;
          }
		}
	}
	if (list_str) 
		free(list_str);

	if (_list_items)
		options_list_init(w, _list_items);
	
	// create an empty list in case we reference the _list_items.
    if (_list_items == NULL) {
      _list_items = new DtVirtArray<char *>(10);
    }

  	// Clear the server and mailbox text fields.
  	XtVaSetValues(_mailbox_folder_widget, XmNvalue,"", NULL);
  	XtVaSetValues(_server_widget, XmNvalue,"", NULL);
	
  	// Init the local/imap radio button
  	options_radiobox_set_value(_local_imap_radiob, 0, True);

    if (_list_items && _list_items->length() > 0)
        	XmListSelectPos(w, 1, TRUE);
	else {
        	OptCmd *oc = theRoamApp.mailOptions();
        	DtbOptionsDialogInfo od = oc->optionsDialog();
        	XtSetSensitive(od->mm_delete_butt, False);
        	XtSetSensitive(od->mm_change_butt, False);
        	XtSetSensitive(od->alphalist_but, False);
	}
}

SdmBoolean
MoveMenuListUiItem::inList(char *name)
{
    if (_list_items != NULL) {
      int list_len = _list_items->length();
      for(int i = 0; i < list_len; i++) {
          char *li = (*_list_items)[i];
          if (li && strcmp(li, name) == 0)
          return Sdm_True;
      }    
    }
    return Sdm_False;
}

///////////////////////////////////////////////////////////
void MoveMenuListUiItem::handleAddButtonPress()
{
  char *mailbox, *server;
  char fullname[2048];
  Boolean isActive;

  XtVaGetValues(_mailbox_folder_widget, XmNvalue, &mailbox, NULL);
  XtVaGetValues(_server_widget, XmNsensitive, &isActive,
  			XmNvalue, &server, NULL);

  if (mailbox && *mailbox) {
	int *pos_list, num_pos;
	if(XmListGetSelectedPos(this->getWidget(), &pos_list, &num_pos)) {
		num_pos = pos_list[0];
  		free(pos_list);
	}
	else
		num_pos = 1;

    	if (_list_items == NULL)
	      	_list_items = new DtVirtArray<char *>(10);

	if (isActive && server && *server)
		sprintf(fullname, "%s(%s)", mailbox, server);
	else
		strcpy(fullname, mailbox);
	if (server) free(server);
 	if (inList(fullname)) {
		DtMailGenDialog *genDialog = theRoamApp.genDialog();
		genDialog->setToErrorDialog(catgets(DT_catd, 3, 48, "Mailer"),
			catgets(DT_catd, 15, 27, "The mailbox name already exists."));
		genDialog->post_and_return(catgets(DT_catd, 1, 92, "OK"),
				DTMAILHELPMOVEMENUNAME);
   		free(mailbox);
		return;
	}
    	_list_items->insert(strdup(fullname), num_pos - 1); 
	XmString lstr = XmStringCreateLocalized(fullname);
    	XmListAddItem(this->getWidget(), lstr, num_pos);
	XmStringFree(lstr);
    	XmListSelectPos(this->getWidget(), num_pos, TRUE);
    	XmListSetPos(this->getWidget(), num_pos); 
	props_changed = Sdm_True;
   }
   if (mailbox) free(mailbox);
}

///////////////////////////////////////////////////////////
void MoveMenuListUiItem::handleChangeButtonPress()
{
  char *server, *mailbox;
  XmString replace_string;
  int *pos_list, num_pos;
  Boolean isActive;
  char fullname[2048];

  // if nothing selected nothing to change...
  if (XmListGetSelectedPos(this->getWidget(), &pos_list, &num_pos)) {

      XtVaGetValues(_mailbox_folder_widget, XmNvalue, &mailbox, NULL);
      XtVaGetValues(_server_widget, XmNsensitive, &isActive,
                        XmNvalue, &server, NULL);

      if (mailbox != NULL && *mailbox != NULL) {

	    free((*_list_items)[pos_list[0] - 1]);
	    _list_items->remove(pos_list[0] - 1);
	    if (server && *server && isActive) 
		sprintf(fullname, "%s(%s)", mailbox, server);
	    else
		strcpy(fullname, mailbox);
	    if (inList(fullname)) {
                DtMailGenDialog *genDialog = theRoamApp.genDialog(); 
                genDialog->setToErrorDialog(catgets(DT_catd, 3, 48, "Mailer"),
			catgets(DT_catd, 15, 27, "The mailbox name already exists."));
                genDialog->post_and_return(catgets(DT_catd, 1, 92, "OK"),
				DTMAILHELPMOVEMENUNAME); 
                free(mailbox);
                return;
	    }
   	    _list_items->insert(strdup(fullname), pos_list[0] - 1);
  
	    replace_string = XmStringCreateLocalized(fullname);
	    XmListReplaceItemsPos(this->getWidget(), &replace_string, 1,
				  pos_list[0]);
	    XmStringFree(replace_string);

	    XmListSelectPos(this->getWidget(), pos_list[0], TRUE);
	    props_changed = Sdm_True;
	    if (server) free(server);
      }
      if (mailbox) free(mailbox);
      free(pos_list);
   }
}

///////////////////////////////////////////////////////////
void MoveMenuListUiItem::handleDeleteButtonPress()
{
  Widget list_widget = this->getWidget();
  int *p_list, p_count;

  if (XmListGetSelectedPos(list_widget, &p_list, &p_count)) {
      // delete the item from our list 
      this->_list_items->remove(p_list[0] - 1); // remove only first

      // delete the item from the widget
      XmListDeletePos(list_widget, p_list[0]);

      XtVaSetValues(_mailbox_folder_widget, XmNvalue,"", NULL);

      int num;
      XtVaGetValues(list_widget, XmNitemCount, &num, NULL);
      if (num == 0) {
	   OptCmd *oc = theRoamApp.mailOptions();
	   DtbOptionsDialogInfo od = oc->optionsDialog();
	   XtSetSensitive(od->mm_delete_butt, False);
	   XtSetSensitive(od->mm_change_butt, False);
	   XtSetSensitive(od->alphalist_but, False);
      }
      else {
     	   if (p_list[0] > num)
                XmListSelectPos(list_widget, p_list[0]-1, TRUE);
           else
                XmListSelectPos(list_widget, p_list[0], TRUE);
      }
      props_changed = Sdm_True;
      free(p_list);
   }
}

///////////////////////////////////////////////////////////
void MoveMenuListUiItem::handleBrowseButtonPress(char *selection, char *server)
{
	XmString lstr;

	if (_list_items == NULL)
		_list_items = new DtVirtArray<char *>(10);

	if (server != NULL) {
		char *mailbox;
		mailbox = (char*)malloc(strlen(selection) + strlen(server) + 3); 
		sprintf(mailbox, "%s(%s)", selection, server);
		lstr = XmStringCreateLocalized(mailbox);
		_list_items->insert(mailbox, 0);
	}
	else {
		lstr = XmStringCreateLocalized(selection);
		_list_items->insert(strdup(selection), 0);
	}
	Widget w = this->getWidget();
	XmListAddItem(w, lstr, 1);
	XmListSelectPos(w, 1, TRUE);
	XmListSetPos(w, 1);
	XmStringFree(lstr);
}

void MoveMenuListUiItem::handleMoveUpButtonPress()
{

    int *position, count, top;
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
	XmNtopItemPosition, &top,
	NULL);

    char *save_item = (*_list_items)[*position-1];
    _list_items->remove(*position-1);
    _list_items->insert(save_item, *position-2);
 
    selected_item = XmStringCopy(selected_items[0]);
    XmListDeletePos(w, *position);
    XmListAddItem(w, selected_item, (*position)-1);
    XmListSelectPos(w, (*position)-1, True);

    // Make sure item is visible
    if (((*position)-1) < top)
	XmListSetPos(w, (*position)-1);

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
 
    if (count == 0 || !status || (*position) == count) 
        return;
    
    char *save_item = (*_list_items)[*position-1];
    _list_items->remove(*position-1);
    _list_items->insert(save_item, *position);

    selected_item = XmStringCopy(selected_items[0]);
    XmListDeletePos(w, *position);
    XmListAddItem(w, selected_item, (*position)+1);
    XmListSelectPos(w, (*position)+1, True);

    // Make sure item is visible
    XmListSetBottomPos(w, (*position)+1);

    XmStringFree(selected_item);
    XtFree((char *)position);
}

void MoveMenuListUiItem::handleAlphabetizeButtonPress()
{
    int num_items = _list_items->length();
    if (num_items < 1) return;

    Widget w = this->getWidget();

    char **sort = (char**)malloc(num_items * sizeof (char*));
    int i;
    for (i = 0; i < num_items; i++)
	sort[i] = (*_list_items)[i];
    qsort(sort, _list_items->length(), sizeof(char*),
	 (int (*)(const void *, const void *))myStrCmp);
    // Remove and readd to list...
    while(_list_items->length())
	_list_items->remove(0);
    for (i = 0; i < num_items; i++)
	_list_items->append(sort[i]);
    free(sort);
    options_list_init(w, _list_items);
}
