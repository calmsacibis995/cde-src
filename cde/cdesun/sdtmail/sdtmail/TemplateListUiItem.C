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
#pragma ident "@(#)TemplateListUiItem.C	1.49 12/05/96"
#endif

#include "RoamApp.h"
#include "TemplateDialog.h"
#include <dirent.h>
#include <Xm/List.h>
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Connection.hh>
#include <SDtMail/Error.hh>
#include <SDtMail/MailRc.hh>
#include <SDtMail/Utility.hh>
#include <SDtMail/SystemUtility.hh>
#include <DtMail/options_util.h>
#include <DtMail/OptCmd.h>
#include "ComposeCmds.hh"
#include <DtMail/PropUi.hh>
#include <DtMail/ListUiItem.hh>
#include <DtMail/TemplateListUiItem.hh>
#include <DtMail/PropSource.hh>
#include <DtMail/IO.hh>
#include <nl_types.h>

extern nl_catd DT_catd;

extern SdmBoolean props_changed;
void handleTemplateSelection(Widget w, XtPointer clientdata, XtPointer calldata);

// TemplateListUiItem::TemplateListUiItem
// TemplateListUiItem ctor
////////////////////////////////////////////////////////////////////

TemplateListUiItem::TemplateListUiItem(Widget w, 
		       int source, 
		       char *search_key):ListUiItem(w, source, search_key, NULL)
{
  source = source; search_key = search_key;

  _list_items = NULL;
  _files_to_delete = NULL;
  _files_to_rename = NULL;
  
  XtVaSetValues(w,
	XmNuserData, this,
        XmNautomaticSelection, True,
        XmNselectionPolicy, XmBROWSE_SELECT,
	NULL);
  XtAddCallback(w,
        XmNbrowseSelectionCallback,
        (XtCallbackProc)handleTemplateSelection,
        (XtPointer)this);
}
void handleTemplateSelection(Widget , XtPointer, XtPointer )
{
      OptCmd *oc = theRoamApp.mailOptions();
      DtbOptionsDialogInfo od = oc->optionsDialog();
      XtSetSensitive(od->edit_template_button, True);
      XtSetSensitive(od->delete_template_button, True);
      XtSetSensitive(od->rename_template_button, True);
}

void TemplateListUiItem::replaceListItem(char *oldvalue, char *newvalue)
{
	int i, list_len;

	list_len = _list_items->length();
	for (i=0; i < list_len; i++) {
		PropStringPair *psp = (*_list_items)[i];
		if (psp && psp->value && strcmp(psp->value, newvalue) == 0) {
			free(psp->value);
			psp->value = strdup(oldvalue);
			break;
		}
	}
}

// TemplateListUiItem::writeFromUiToSource()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void TemplateListUiItem::writeFromUiToSource()
{
  SdmError error;
  SdmConnection *con = theRoamApp.connection()->connection();
  SdmMailRc *mail_rc;
  con->SdmMailRcFactory(error, mail_rc);
  assert(!error);
  char *tstr, *token, buf[512];
  int i, num_items, buf_size=1024;
  DtMailGenDialog *genDialog = theRoamApp.genDialog();

  if (_files_to_rename) {
	num_items = _files_to_rename->length();
	for (i=0; i < num_items; i++) {
		PropStringPair *psp = (*_files_to_rename)[i];
		char *oldname;
		SdmUtility::ExpandPath(error, oldname, psp->label, *mail_rc, SdmUtility::kFolderResourceName);
		if (SdmSystemUtility::SafeAccess(oldname, F_OK) != 0) {
			sprintf(buf, catgets(DT_catd, 3, 110,
                                "The template %s\ndoes not exist. Cannot rename template."),
                                oldname);
                        genDialog->setToErrorDialog(
                                catgets(DT_catd, 3, 24, "Mailer"), buf);
                        genDialog->post_and_return(catgets(DT_catd, 1, 92, "OK"),
				 DTMAILHELPTEMPLATEERRORRENAME);
			replaceListItem(psp->label, psp->value); 
			free(oldname);
			continue;
		}
		char *newname;
		SdmUtility::ExpandPath(error, newname, psp->value, *mail_rc, SdmUtility::kFolderResourceName);
		if (SdmSystemUtility::SafeAccess(newname, F_OK) == 0) {
                        sprintf(buf, catgets(DT_catd, 3, 112,
                                "The template %s\nalready exists. Overwrite?"),
                                newname);
                        genDialog->setToQuestionDialog(
                                catgets(DT_catd, 3, 24, "Mailer"), buf);
                        if (genDialog->post_and_return(DTMAILHELPTEMPLATEERRORRENAME) == 2) {
                        	free(oldname); free(newname);
				replaceListItem(psp->label, psp->value); 
                        	continue;
			}
                }
		if (rename(oldname, newname) != 0) {
      			sprintf(buf, catgets(DT_catd, 3, 107,
				"Unable to rename the template file:\n%s"),
				oldname);
			genDialog->setToErrorDialog(
				catgets(DT_catd, 3, 24, "Mailer"), buf);
			genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPTEMPLATEERRORRENAME);
			replaceListItem(psp->label, psp->value); 
		}
		delete psp;
		free(oldname); free(newname);
	}
	delete _files_to_rename;
	_files_to_rename = NULL;
  }
  if (_files_to_delete) {
	num_items = _files_to_delete->length();
	for (i=0; i < num_items; i++) {
		PropStringPair *psp = (*_files_to_delete)[i];
		char *expname;
		SdmUtility::ExpandPath(error, expname, psp->value, *mail_rc, SdmUtility::kFolderResourceName);
		if (SdmSystemUtility::SafeUnlink(expname) < 0) {
      			sprintf(buf, catgets(DT_catd, 3, 106,
				"Unable to delete the template file:\n%s"),
				expname);
			genDialog->setToErrorDialog(
				catgets(DT_catd, 3, 24, "Mailer"), buf);
			genDialog->post_and_return(DTMAILHELPTEMPLATEERRORREMOVE);
		}
		delete(psp);
		free(expname);
	}
	delete _files_to_delete;
	_files_to_delete = NULL;
  }

  if (_list_items != NULL && _list_items->length() > 0) {
	tstr = (char *)malloc(1024);
	tstr[0] = '\0';
  	num_items = _list_items->length();
  	for(i = 0; i < num_items; i++) {
		if (i != 0) strcat(tstr, " ");
		// Allow 512 characters for each name. If not enough space
		// then realloc the buffer to allow for space for the next name
		if ((strlen(tstr) + 512) > 1024) {
			buf_size += 1024;
			tstr = (char*)realloc(tstr, buf_size);
		}
      		if ((*_list_items)[i]->label) {
			if (strchr((*_list_items)[i]->label, ' ')) {
				strcpy(buf, (*_list_items)[i]->label);
				strcat(tstr, (char *)strtok(buf, " "));
				while (token = (char *)strtok(NULL, " ")) {
					strcat(tstr, "\\ ");
					strcat(tstr, token);
				}
			}
			else 
				strcat(tstr, (*_list_items)[i]->label); 
	      		strcat(tstr, ":");
      			if ((*_list_items)[i]->value) {
				if (strchr((*_list_items)[i]->value, ' ')) {
					strcpy(buf, (*_list_items)[i]->value);
					strcat(tstr, (char *)strtok(buf, " "));
					while (token = (char *)strtok(NULL, " ")) {
						strcat(tstr, "\\ ");
						strcat(tstr, token);
					}
				}
				else 
					strcat(tstr, (*_list_items)[i]->value); 
			}
		}
	}
	mail_rc->SetValue(error, "templates", tstr);
  	assert(!error);
	free(tstr);
  }
  else {
  	mail_rc->RemoveValue(error, "templates");
  	assert(!error);
  }
  update_display();
}

// TemplateListUiItem::writeFromSourceToUi()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void TemplateListUiItem::writeFromSourceToUi()
{
  int list_len, i;
  Widget w = this->getWidget();

  XmListDeleteAllItems(w);

  update_display();

  if (_files_to_rename) {
        list_len = _files_to_rename->length();
        for (i=0; i < list_len; i++) {
                PropStringPair *psp = (*_files_to_rename)[i];
		delete psp;
	}
	delete _files_to_rename;
	_files_to_rename = NULL;
  }

  if (_files_to_delete) {
        list_len = _files_to_delete->length();
        for (i=0; i < list_len; i++) {
                PropStringPair *psp = (*_files_to_delete)[i];
		delete psp;
	}
	delete _files_to_delete;
	_files_to_delete = NULL;
  }
  if (_list_items && _list_items->length() > 0)
	XmListSelectPos(w, 1, TRUE);
  else {
	OptCmd *oc = theRoamApp.mailOptions();
        DtbOptionsDialogInfo od = oc->optionsDialog();
	XtSetSensitive(od->edit_template_button, False);
      	XtSetSensitive(od->delete_template_button, False);
      	XtSetSensitive(od->rename_template_button, False);
  }
}
void TemplateListUiItem::update_display()
{
  SdmError error;
  SdmConnection *con = theRoamApp.connection()->connection();
  SdmMailRc *mail_rc;
  con->SdmMailRcFactory(error, mail_rc);
  assert(!error);
  DtVirtArray<char *> list_str_list(10);
  Widget w = this->getWidget();
  int list_len, i;
  char *value = NULL;

  mail_rc->GetValue(error, "templates", &value);

  if (_list_items != NULL) {
	list_len = _list_items->length();
	for (i=0; i < list_len; i++)  {
		PropStringPair *psp = (*_list_items)[i];
		delete psp;
	}
  	delete _list_items; 
  }

  _list_items = new DtVirtArray<PropStringPair *>(10);

  parsePropString(value, *_list_items);

  if (value != NULL)
    free((char *)value);

  list_len = _list_items->length();
  for(i = 0; i < list_len; i++)
    list_str_list.append((*_list_items)[i]->formatPropPair());
 
  options_list_init(w, &list_str_list);

  for(i = 0; i < list_len; i++)
    free(list_str_list[i]);


}
///////////////////////////////////////////////////////////
void TemplateListUiItem::handleEditButtonPress()
{
  int *pos_list, num_pos;

  // if nothing selected nothing to change...
  if(XmListGetSelectedPos(this->getWidget(), &pos_list, &num_pos)) {
  	SdmError error;
  	PropStringPair *psp = (*_list_items)[pos_list[0] - 1];
	LoadFileCmd *tc;

  	free(pos_list);

	tc = new LoadFileCmd(psp->label, psp->label,
				FALSE, NULL, psp->value, TYPE_TEMPLATE);
	tc->doit();
	delete tc;
    }
}

static void
rename_CB( Widget widget, XtPointer , XtPointer)
{
    TemplateListUiItem *cmd_ptr = NULL;

    XtVaGetValues(widget,
                XmNuserData, &cmd_ptr,
                NULL);

    if (cmd_ptr != NULL)
                cmd_ptr->getNameAndRename();
}

SdmBoolean
TemplateListUiItem::inList(char *label)
{
    int list_len = _list_items->length();
    for(int i = 0; i < list_len; i++) {
      	PropStringPair *psp = (*_list_items)[i];
      	if (psp && psp->label) {
        	if (strcmp(psp->label, label) == 0) 
          		return Sdm_True;
      	}
    }
    return Sdm_False;
}
void
TemplateListUiItem::getNameAndRename()
{
    DtbModuleDialogInfo td = theRoamApp.getTemplateDialog();
    char *label = NULL;
    int *pos_list, num_pos;
    char buf[1024];
    DtMailGenDialog *genDialog = theRoamApp.genDialog();

    if (XmListGetSelectedPos(this->getWidget(), &pos_list, &num_pos)) {
      	XtVaGetValues(td->templ_name, XmNvalue, &label, NULL);
	if (label != NULL && *label != NULL) {
  		PropStringPair *psp = (*_list_items)[pos_list[0] - 1];
		if (psp->value != NULL && inList(label)) {
			sprintf(buf, catgets(DT_catd, 3, 123,
     				"The menu label %s already exists.\nRename failed."), label);

                	genDialog->setToErrorDialog(catgets(DT_catd, 3, 48, "Mailer"), buf);
                	genDialog->post_and_return(catgets(DT_catd, 1, 92, "OK"),
				 DTMAILHELPTEMPLATERENAMEFAILED);

			free(label);
   			free(pos_list);
			return;
		}
  		XmString replace_string;
		if (psp->value) {
                	genDialog->setToQuestionDialog(
				catgets(DT_catd, 3, 48, "Mailer"), 
				catgets(DT_catd, 3, 138, "Do you also want to rename the template file?"));
                        int answer;
                        if ((answer = genDialog->post_and_return(
                                catgets(DT_catd, 3, 137, "Do Not Rename File"),
				catgets(DT_catd, 3, 19, "Cancel"),
                                catgets(DT_catd, 3, 136, "Rename File"),
                                DTMAILHELPTEMPLATERENAME)) == 3) {
				// Rename the file also
				char *last_slash = strrchr(psp->value, '/');
				if (last_slash) {
					PropStringPair *pair_str = new PropStringPair;
					// Store the old file name
					pair_str->label = strdup(psp->value); 
					// Create and store the new file name
					strcpy(buf, psp->value);
					buf[last_slash-psp->value+1] = '\0';
					strcat(buf, label); 
					pair_str->value = strdup(buf); 
					if (_files_to_rename == NULL)
						_files_to_rename = 
							new DtVirtArray<PropStringPair *>(3);
					_files_to_rename->append(pair_str);
					free(psp->value);
					psp->value = strdup(buf);
				}
			}
			else if (answer == 2) { // Cancel 
				free(label);
   				free(pos_list);
				return;
			}
		}
	    	free(psp->label);
	    	psp->label = label;

		char *str = psp->formatPropPair();
	    	replace_string = XmStringCreateLocalized(str);
		free(str);

	    	XmListReplaceItemsPos(this->getWidget(), &replace_string, 1, pos_list[0]);
		XmStringFree(replace_string);

		XmListSelectPos(this->getWidget(), pos_list[0], TRUE);
	    	props_changed = Sdm_True;
        }
        else if (label)
		free(label);
   	free(pos_list);
        quit();
    }
}
static void
cancel_CB(
    Widget widget,
    XtPointer,
    XtPointer
)
{
    TemplateListUiItem *cmd_ptr = NULL;

    XtVaGetValues(widget,
                XmNuserData, &cmd_ptr,
                NULL);

    if (cmd_ptr != NULL)
                cmd_ptr->quit();
}
void
TemplateListUiItem::quit()
{
    DtbModuleDialogInfo td = theRoamApp.getTemplateDialog();
    if (td)
    	XtPopdown(td->templateDialog);
}

///////////////////////////////////////////////////////////
void TemplateListUiItem::handleRenameButtonPress()
{
    int *pos_list, num_pos;
    if (XmListGetSelectedPos(this->getWidget(), &pos_list, &num_pos)) {
	free(pos_list);
   	DtbModuleDialogInfo td = theRoamApp.getTemplateDialog();
    	if (td == NULL) {
		td = (DtbModuleDialogInfo)malloc(sizeof(
                               DtbModuleDialogInfoRec));
        	dtbTemplTemplateDialogInfo_clear(td);
		OptCmd *oc = theRoamApp.mailOptions(); 
	 	DtbOptionsDialogInfo oHandle = oc->optionsDialog();
        	dtb_template_dialog_initialize(td, oHandle->templates_pane);
        	dtb_save_toplevel_widget(oHandle->templates_pane);
        	XtSetMappedWhenManaged(td->templateDialog, False);
		theRoamApp.setTemplateDialog(td);
	}
        XmString label_str = XmStringCreateLocalized(catgets(DT_catd, 1, 97, "Rename"));
	XtVaSetValues(td->save_button, XmNuserData, this, 
			XmNlabelString, label_str,
			NULL);	
	XmStringFree(label_str);
	XtVaSetValues(td->cancel_button, XmNuserData, this, NULL);	
        XtVaSetValues(td->templ_name, XmNvalue, "", NULL);
	XtVaSetValues(td->templateDialog,
                XmNtitle, catgets(DT_catd, 1, 283, "Mailer - Rename Template"),
                NULL);
    	XtManageChild(td->dialog_shellform);
        XtPopup(td->templateDialog, XtGrabNone);
 	XtRemoveAllCallbacks (td->save_button, XmNactivateCallback);
 	XtRemoveAllCallbacks (td->cancel_button, XmNactivateCallback);
    	XtAddCallback(td->save_button,
                XmNactivateCallback, rename_CB, this);
    	XtAddCallback(td->cancel_button,
                XmNactivateCallback, cancel_CB, this);
    }
        
}

///////////////////////////////////////////////////////////
void TemplateListUiItem::handleDeleteButtonPress()
{
  	Widget w = this->getWidget();
  	int *p_list, p_count;
  	PropStringPair *psp = NULL;

  	// get the selected position
  	if (XmListGetSelectedPos(w, &p_list, &p_count)) {
	      	DtMailGenDialog *genDialog = theRoamApp.genDialog();
		char buf[512];

      		psp = (*_list_items)[p_list[0] - 1];
		if (psp->value) {
	      		sprintf(buf, catgets(DT_catd, 3, 102,
			"This will delete the template menu label.\nDo you also want to delete the template file:\n%s?"), psp->value);
			genDialog->setToQuestionDialog(catgets(DT_catd, 3, 48, "Mailer"), buf);
			int answer;
      			if ((answer = genDialog->post_and_return(
				catgets(DT_catd, 3, 122, "Do Not Delete File"),
				catgets(DT_catd, 3, 19, "Cancel"),
			 	catgets(DT_catd, 3, 121, "Delete File"), 
				DTMAILHELPTEMPLATEDELETE)) == 3) {
				if (_files_to_delete == NULL)
					_files_to_delete = new DtVirtArray<PropStringPair *>(3);
		 		PropStringPair *psp2 = new PropStringPair(*psp);
				_files_to_delete->append(psp2);
			}
			else if (answer == 2) { // cancel
				free(p_list);
				return;
			}
		}
      		// delete the item from our list 
		psp = (*_list_items)[p_list[0] - 1];
		delete psp;
      		_list_items->remove(p_list[0] - 1);

      		// delete the item from the widget
      		XmListDeletePos(w, p_list[0]);

		int num;
		XtVaGetValues(w, XmNitemCount, &num, NULL);
    		if (num == 0) {
			OptCmd *oc = theRoamApp.mailOptions();
        		DtbOptionsDialogInfo od = oc->optionsDialog();
			XtSetSensitive(od->edit_template_button, False);
        		XtSetSensitive(od->delete_template_button, False);
        		XtSetSensitive(od->rename_template_button, False);
    		}
    		else {
        		if (p_list[0] > num)
                		XmListSelectPos(w, p_list[0]-1, TRUE);
        		else
                		XmListSelectPos(w, p_list[0], TRUE);
    		}

		free(p_list);
      		props_changed = Sdm_True;
      	}

}
///////////////////////////////////////////////////////////
void TemplateListUiItem::handleIncludeButtonPress(char *selection)
{
	if (!selection || !*selection) return;

	char *last_slash;
	PropStringPair *psp;

	if (_list_items == NULL)
		_list_items = new DtVirtArray<PropStringPair *>(10);

	psp = new PropStringPair;
	psp->value = strdup(selection);
	last_slash = strrchr(selection, '/');
	if (last_slash && (last_slash+1) && *(last_slash+1))
		psp->label = strdup(last_slash+1);
	else
		psp->label = strdup(selection);
	char *str = psp->formatPropPair();
	XmString lstr = XmStringCreateLocalized(str);
	free(str);

	Widget w = this->getWidget();
	_list_items->insert(psp, 0);
	XmListAddItem(w, lstr, 1);
	XmListSelectPos(w, 1, TRUE);
	XmListSetPos(w, 1);
	
	XmStringFree(lstr);
}
