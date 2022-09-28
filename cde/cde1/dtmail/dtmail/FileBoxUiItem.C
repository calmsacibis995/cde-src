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

#include <DtMail/PropUi.hh>
#include <DtMail/FileBoxUiItem.hh>
#include <DtMail/IO.hh>
#include <Xm/Xm.h>
#include "RoamApp.h"
#include "DtMailGenDialog.hh"
#include <DtMail/options_util.h>
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
  DtMail::Session * d_session = theRoamApp.session()->session();
  DtMailEnv error;
  DtMail::MailRc * mail_rc = d_session->mailRc(error);
  int fd;

  pathFile = (char*)prop_source->getValue();
  if (error.isNotSet() && pathFile != NULL) {
	if (*pathFile == NULL) {
		// No path/filename specified
                genDialog->setToErrorDialog(catgets(DT_catd, 1, 82, 
			"Mailer"), catgets(DT_catd, 3, 97, "Please specify a filename"));
                genDialog->post_and_return(DTMAILHELPNOOPEN);
                return;
	}
	full_dirname = d_session->expandPath(error, pathFile);
	fd = SafeOpen(full_dirname, O_RDWR | O_CREAT | O_TRUNC, 0600);
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
	if (SafeWrite(fd, tmp_buf, strlen(tmp_buf)) < 0 ||
		SafeWrite(fd, "\n", 1) < 0) {
        	sprintf(buf, catgets(DT_catd, 3, 53, "Unable to write to %s."), 
			pathFile);
        	genDialog->setToErrorDialog(catgets(DT_catd, 3, 54, "Mailer"), buf);
        	genDialog->post_and_return(DTMAILHELPNOWRITE);
        	SafeClose(fd);
		free(pathFile);
        	return;
    	}
	SafeClose(fd);
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
  DtMail::Session * d_session = theRoamApp.session()->session();
  DtMailEnv error;
  DtMail::MailRc * mail_rc = d_session->mailRc(error);
  struct stat sbuf;

  p_value = (char*)prop_source->getValue();
  if (error.isNotSet() && p_value != NULL) {
        full_dirname = d_session->expandPath(error, p_value);
	free(p_value);
        int fd = SafeOpen(full_dirname, O_RDONLY);
        free(full_dirname);
        if (fd < 0)
                return;
    	if (SafeFStat(fd, &sbuf) < 0) {
       		SafeClose(fd);
        	return;
	}
	tmp_buf = (char*)malloc((unsigned int)sbuf.st_size);
	if (SafeRead(fd, (void*)tmp_buf, (unsigned int)sbuf.st_size) < 0) {
        	free(tmp_buf);
        	SafeClose(fd);
        	return;
        }
	SafeClose(fd);
  	tmp_buf[sbuf.st_size-1] = '\0';
  	XtVaSetValues(w, XmNvalue, tmp_buf, NULL);
  	free(tmp_buf);
  }
}
