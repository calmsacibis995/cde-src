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
#pragma ident " @(#)TextFieldUiItem.C	1.18 01/13/97 "
#endif

#include <DtMail/options_util.h>
#include <DtMail/PropUi.hh>
#include <DtMail/TextFieldUiItem.hh>
#include "RoamApp.h"
#include "MailMsg.h"
//#include <DtMail/IO.hh> //SafeAccess...
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/SystemUtility.hh>

// TextFieldUiItem::TextFieldUiItem
// TextFieldUiItem ctor
////////////////////////////////////////////////////////////////////

TextFieldUiItem::TextFieldUiItem(Widget w, int source, char *search_key):PropUiItem(w, source, search_key)
{
#ifdef DEAD_WOOD
  data_source = source;
#endif /* DEAD_WOOD */

  options_field_init(w, &(this->dirty_bit));

}

// TextFieldUiItem::writeFromUiToSource()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void TextFieldUiItem::writeFromUiToSource()
{
  char 	*textfield_value;
  Widget w = this->getWidget();
  const char *value;

  textfield_value = options_field_get_value(w);
  if (textfield_value == NULL || *textfield_value == NULL) {
	value = prop_source->getDefaultValue();
  	prop_source->setValue(value);
  }
  else { // make sure there are no blanks
	while(*textfield_value == ' ') textfield_value++;
	if (*textfield_value != '\0') {
		char *ptr = textfield_value + strlen(textfield_value)-1;
		while(*ptr == ' ') *ptr-- = '\0';
  		prop_source->setValue(textfield_value);
	}
	else {
		value = prop_source->getDefaultValue();
  		prop_source->setValue(value);
	}
  }

  // If they change the folder directory, make sure it exists.
  if (strcmp(getKey(), "folder") == 0) {
	char *folder = theRoamApp.getFolderDir(local, Sdm_True);
	if (SdmSystemUtility::SafeAccess(folder, F_OK) != 0) {
        	if (mkdir(folder, 0700) != 0) {
		    char buf[2048];
		    DtMailGenDialog *genDialog = theRoamApp.genDialog();
		    sprintf(buf, catgets(DT_catd, 3, 51, "Unable to create %s."),
              		folder);
      		    genDialog->setToErrorDialog(catgets(DT_catd, 3, 54, "Mailer"), buf);
      	 	    genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOCREATE);
		}
	}
    	free(folder);
  }
  else {
  	// Make sure the record directory exists.
  	if (strcmp(getKey(), "record") == 0) {
		char *fullpath;
		SdmError error;
		SdmMailRc * mailRc;

		theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailRc);
		SdmUtility::ExpandPath(error, fullpath, textfield_value,
			 *mailRc, SdmUtility::kFolderResourceName);
		char *ptr = strrchr(fullpath, '/');
		if (ptr != NULL) *ptr = NULL;
		// If its not a / that means that it is relative to folder dir
		// and we know that already exists.
		if (fullpath && *fullpath == '/') 
			if (SdmSystemUtility::SafeAccess(fullpath, F_OK) != 0)
        			mkdir(fullpath, 0700);
		free(fullpath);
	}
  }
}

// TextFieldUiItem::writeFromSourceToUi()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void TextFieldUiItem::writeFromSourceToUi()
{
  char *value;
  Widget w = this->getWidget();

  value = (char *)prop_source->getValue();

  options_field_set_value(w, value, this->dirty_bit);

}
void TextFieldUiItem::handleSelectButtonPress(char *selection)
{
   Widget w = this->getWidget();
   options_field_set_value(w, selection, this->dirty_bit);
}
