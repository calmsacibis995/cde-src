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
 * This is the GUI item for implementing a scolled window and an
 * associated textfield that stores the files name. The named file
 * is read into the scrolled window and the text is written out to
 * the named file.
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident " @(#)FileBoxUiItem.C	1.4 22 Feb 1995 "
#endif

#include <Xm/Xm.h>
#include <DtMail/PropUi.hh>
#include <DtMail/FileBoxUiItem.hh>
#include <DtMail/IO.hh>
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Utility.hh>
#include <SDtMail/SystemUtility.hh>
#include "RoamApp.h"
#include "DtMailGenDialog.hh"
#include <DtMail/options_util.h>
#include <nl_types.h>
#include <DtMail/OptCmd.h>

extern nl_catd DT_catd;

// FileBoxUiItem::FileBoxUiItem
////////////////////////////////////////////////////////////////////

FileBoxUiItem::FileBoxUiItem(Widget w, 
		       int source, 
		       char *search_key
		       ):PropUiItem(w, source, search_key)
{
}

// FileBoxUiItem::writeFromUiToSource()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void FileBoxUiItem::writeFromUiToSource()
{
  char *tmp_buf, *pathFile = NULL, *full_dirname = NULL;
  Widget w = this->getWidget();
  char buf[2048];
  DtMailGenDialog *genDialog = theRoamApp.genDialog();
  SdmError error;
  SdmMailRc * mail_rc;
  theRoamApp.connection()->connection()->SdmMailRcFactory(error, mail_rc);
  int fd;

  pathFile = (char*)prop_source->getValue();
  if ( !error && pathFile != NULL) {
	if (*pathFile == NULL) {
		// No path/filename specified
                genDialog->setToErrorDialog(catgets(DT_catd, 1, 82, 
			"Mailer"), catgets(DT_catd, 3, 97, "Please specify a filename"));
                genDialog->post_and_return(DTMAILHELPNOOPEN);
                return;
	}
	SdmUtility::ExpandPath(error, full_dirname, pathFile, *mail_rc, SdmUtility::kFolderResourceName);

	if (strcmp(getKey(), "signature") == 0) {
		OptCmd *oc= (OptCmd*)theRoamApp.mailOptions();
		// Signature file already exists.
		if (SdmSystemUtility::SafeAccess(full_dirname, F_OK) == 0) {
			if (oc->signatureFileChanged()) {
				genDialog->setToQuestionDialog(catgets(DT_catd, 3, 4, "Mailer"),
			     		catgets(DT_catd, 15, 25, "The signature file that you specified already exists.\nDo you want to overwrite its contents with this signature text?"));
                		if (genDialog->post_and_return(catgets(DT_catd, 3, 5,"OK"), 
					catgets(DT_catd, 1, 5, "Cancel"), DTMAILHELPSIGNATUREFILE) == 2) {
					free(pathFile);
					free(full_dirname);
					return;
				}
			}
		}
 		oc->resetSignature();
	}

	// Create the property file - since this is a behind the
	// scenes kind of deal, we use mode 0600 as opposed to mode
	// 0666 for security reasons.

	fd = SdmSystemUtility::SafeOpen(full_dirname, O_RDWR | O_CREAT | O_TRUNC, 0600);
	free(full_dirname);
	if (fd < 0) {
		sprintf(buf, catgets(DT_catd, 3, 35, "Unable to open %s."), 
			pathFile);
        	genDialog->setToErrorDialog(catgets(DT_catd, 1, 82, "Mailer"), 
			buf);
		genDialog->post_and_return(DTMAILHELPNOOPEN);
		free(pathFile);
        	return;
	}
  	XtVaGetValues(w, XmNvalue, &tmp_buf, NULL);
	if (SdmSystemUtility::SafeWrite(fd, tmp_buf, strlen(tmp_buf)) < 0 ||
		SdmSystemUtility::SafeWrite(fd, "\n", 1) < 0) {
        	sprintf(buf, catgets(DT_catd, 3, 53, "Unable to write to %s."), 
			pathFile);
        	genDialog->setToErrorDialog(catgets(DT_catd, 3, 54, "Mailer"), buf);
        	genDialog->post_and_return(DTMAILHELPNOWRITE);
        	SdmSystemUtility::SafeClose(fd);
		free(pathFile);
        	return;
    	}
	SdmSystemUtility::SafeClose(fd);
	free(pathFile);
   }
}

// FileBoxUiItem::writeFromSourceToUi()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void FileBoxUiItem::writeFromSourceToUi()
{
  char *tmp_buf, *p_value = NULL, *full_dirname = NULL;
  Widget w = this->getWidget();
  SdmError error;
  SdmMailRc * mail_rc;
  struct stat sbuf;

  theRoamApp.connection()->connection()->SdmMailRcFactory(error, mail_rc);

  p_value = (char*)prop_source->getValue();
  if ( !error && p_value != NULL) {
        SdmUtility::ExpandPath(error, full_dirname, p_value, *mail_rc, SdmUtility::kFolderResourceName);
	free(p_value);
        int fd = SdmSystemUtility::SafeOpen(full_dirname, O_RDONLY);
        free(full_dirname);
        if (fd < 0)
                return;
    	if (SdmSystemUtility::SafeFStat(fd, &sbuf) < 0) {
       		SdmSystemUtility::SafeClose(fd);
        	return;
	}
	tmp_buf = (char*)malloc((unsigned int)sbuf.st_size);
	if (SdmSystemUtility::SafeRead(fd, (void*)tmp_buf, (unsigned int)sbuf.st_size) < 0) {
        	free(tmp_buf);
        	SdmSystemUtility::SafeClose(fd);
        	return;
        }
	SdmSystemUtility::SafeClose(fd);
  	tmp_buf[sbuf.st_size-1] = '\0';
  	XtVaSetValues(w, XmNvalue, tmp_buf, NULL);
  	free(tmp_buf);
  }
}
