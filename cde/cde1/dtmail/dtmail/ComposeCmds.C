/*
 *+SNOTICE
 *
 *	$Revision: 1.6 $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement between
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel without
 *	Sun's specific written approval.  This document and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)ComposeCmds.C	1.61 05/07/96"
#endif

#include <EUSCompat.h>
#include <unistd.h>
#include <sys/types.h>

#if defined(NEED_MMAP_WRAPPER)
extern "C" {
#endif
#include <sys/mman.h>
#if defined(NEED_MMAP_WRAPPER)
}
#endif

#include <pwd.h>
#include <Xm/Text.h>
#include <Xm/FileSBP.h>
#include <Xm/ToggleB.h>
#include <Xm/PanedW.h>
#include <Xm/Form.h>
#include <Dt/Dts.h>
#include <Dt/Action.h>
#include <DtMail/IO.hh>
#include "RoamMenuWindow.h"
#include "SendMsgDialog.h"
#include "Undelete.hh"
#include "RoamCmds.h"
#include "ComposeCmds.hh"
#include "Application.h"
#include "RoamApp.h"
#include "DtMailWDM.hh"
#include "FindDialog.h"
#include "MsgScrollingList.hh"
#include "MsgHndArray.hh"
#include "MemUtils.hh"
#include "MailMsg.h"
#include "EUSDebug.hh"
#include "DtMailGenDialog.hh"
#include "DtMailHelp.hh"
#include <DtMail/DtMailError.hh>
#include <DtMail/PropSource.hh>
#include "Help.hh"
#include <Dt/Help.h>
#include "Attachment.h"
#include <Xm/TextF.h>
#include <DtMail/OptCmd.h>

ComposeFamily::ComposeFamily(char *name,
			     char *label,
			     int active,
			     RoamMenuWindow *window)
: RoamCmd(name, label, active, window)
{
    _parent = window;
}

#ifndef CAN_INLINE_VIRTUALS
ComposeFamily::~ComposeFamily( void )
{
}
#endif /* ! CAN_INLINE_VIRTUALS */

// Append a formatted message to Compose's Text area.
// This routine is essentially the same as MsgScollingList::display_message()
// except for two major differences:
// 1. No RoamMenuWindow reference (so that Compose can be standalone).
// 2. Indent string can be used for "include" and "forward".
void
ComposeFamily::Display_entire_msg(DtMailMessageHandle msgno,
				  SendMsgDialog *compose, 
				  char *format 
				  )
{
    DtMailEnv error;
    
    int num_bodyParts;
    DtMail::MailBox *mbox = _menuwindow->mailbox();
    DtMail::Message *msg = mbox->getMessage(error, msgno);
    DtMail::Envelope *env = msg->getEnvelope(error);
    DtMail::BodyPart *tmpBP = NULL;
    DtMailBuffer tmpBuffer;
    void *buffer = NULL;
    unsigned long size = 0;
    
    Editor::InsertFormat ins_format = Editor::IF_NONE;
    Editor::BracketFormat brackets = Editor::BF_NONE;
    
    // Do not need to wrap "include", "forward", and "indent" with
    // catgets().
    if ( strcmp(format, "include") == 0 ) {
	ins_format = Editor::IF_BRACKETED;
	brackets = Editor::BF_INCLUDE;
    } else if ( strncmp(format, "forward", 7) == 0 ) {
	ins_format = Editor::IF_BRACKETED;
	brackets = Editor::BF_FORWARD;
    } else if ( strcmp(format, "indent") == 0 ) {
	ins_format = Editor::IF_INDENTED;
    }
    
    // Get the editor to display the body of message with the appropriate
    // insert/bracket formatting.
    // We only include the first body part of the message. Attachments, 
    // etc. are "FORWARD"-ed but not "INCLUDE"-ed
    
    char * status_string;

    compose->get_editor()->textEditor()->append_to_contents("\n", 2);
    DtMailBoolean firstBPHandled =
	compose->get_editor()->textEditor()->set_message(
					 msg, 
					 &status_string, 
					 Editor::HF_ABBREV,
					 ins_format, 
					 brackets);

    // Now need to handle the unhandled body parts of the message.
    
    num_bodyParts = msg->getBodyCount(error);
    if (error.isSet()) {
	// do something
    }
    
    if (strcmp(format, "forward") == 0) {

	if ((num_bodyParts > 1) || (!firstBPHandled)) {

	// If the message has attachments, then let the attach pane
	// handle attachments but not the first bodyPart (which has
	// already been handled here).
	
	    tmpBP = msg->getFirstBodyPart(error);

	    if (firstBPHandled) {
	    
		//  The first bodyPart has already been handled.
		// The others, beginning from the second, need to be parsed 
		// and put into the attachPane.

		compose->setInclMsgHnd(msg, TRUE);
	    
		tmpBP = msg->getNextBodyPart(error, tmpBP);

	    } else {
		// The first bodyPart was not handled.
		// It may not have been of type text.
		// The attachment pane needs to handle all the bodyParts
		// beginning with the first.
	    
		compose->setInclMsgHnd(msg, FALSE);
	    }
	    
	    char *name;
	    while (tmpBP != NULL) {
		tmpBP->getContents(
				    error, (const void **) &tmpBuffer.buffer,
				    &tmpBuffer.size,
				    NULL,
				    &name,
				    NULL,
				    NULL);
		if (error.isSet()) {
		    // Do something
		}
		// It's possible for an attachment to not have a name.
		if (!name) {
		    name = "NoName";
		}

		compose->add_att(name, tmpBuffer);
		tmpBP = msg->getNextBodyPart(error, tmpBP);
		if (error.isSet()) {
		    // do something
		}

		if (strcmp(name, "NoName") != 0) {
		    free(name);
		}
	    }
	    if (error.isSet()) {
		
		// do something
	    }
	
	    // Need to call this after calling parseAttachments().

	    compose->get_editor()->manageAttachArea();
	
	    // This message has attachment and is being included/forwarded,
	    // so need to fill the Compose Message Handle with attachment 
	    // BodyParts.
	    // See function for further details.

	    // compose->updateMsgHndAtt();
	}
    } 
    
    // Leave it up to check point routine for update or do it now???
    compose->updateMsgHnd();
}

void
ComposeFamily::appendSignature(SendMsgDialog * compose, Boolean always_append)
{
    DtMailEnv error;
    DtMail::Session * d_session = theRoamApp.session()->session();
    DtMail::MailRc * mail_rc = d_session->mailRc(error);
    int status;
    char *value = NULL;

    // Sometimes we want to append regardless of the property setting, such
    // as in the case of the Include Signature menu option on Compose Format menu
    if (!always_append) {
    	mail_rc->getValue(error, "dontincludesignature", (const char **)&value);
    	if (error.isNotSet()) {
    		compose->get_editor()->textEditor()->set_to_top();
		return;
    	}
    }

    MailRcSource *prop_source = (MailRcSource*) new MailRcSource (mail_rc, "signature");
    value = (char*)prop_source->getValue();
    delete prop_source;
    char * fullpath = d_session->expandPath(error, value);
    if (value)
	free(value);

    struct stat stat_buf;
    if ( SafeStat ( fullpath, &stat_buf ) < 0 ) {
    	if (!always_append)
    		compose->get_editor()->textEditor()->set_to_top();
	free (fullpath);
	return;
    }

    status = compose->get_editor()->textEditor()->append_at_cursor("\n", 2);
    status = compose->get_editor()->textEditor()->append_to_contents(fullpath);
    free(fullpath);
    if (!always_append)
    	compose->get_editor()->textEditor()->set_to_top();
}

char *
ComposeFamily::valueToAddrString(DtMailValueSeq & value)
{
    int max_len = 0;

    for (int count = 0; count < value.length(); count++) {
	max_len += strlen(*(value[count]));
    }

    // Add (count-1)*2 bytes for extra ", " to separate elements in
    // in the DtMailValueSeq object.
    char * str = new char[max_len + (count-1)*2 + 1];
    str[0] = 0;

    DtMailBoolean need_comma = DTM_FALSE;

    for (int cat = 0; cat < value.length(); cat++) {
	DtMailValue * val = value[cat];
	DtMailAddressSeq * addr_seq = val->toAddress();

	for (int ad = 0; ad < addr_seq->length(); ad++) {
	    DtMailValueAddress * addr = (*addr_seq)[ad];

	    //
	    // This is a hack to prevent dtmail from core dumping
	    // when it reply's to email with several addresses that
	    // each have comments.  Something of the form:
	    // "user1@machine1 (Joe Smith), user2@machine2 (Jane Doe)".
	    // The routine RFCTransport::arpaPhrase() does not
	    // parse out the empty addresses correctly.  The routine arpaPhrase
	    // is called from RFCValue::toAddress() with an address
	    // string that contains a list of comma seperated addresses.
	    // The list happens to contain some null addresses...something
	    // like "foo@bar, ,, ,, ,me@here".  toAddress interprets some
	    // of the commas as addresses.  The bug in toAddress will be
	    // fixed for CDE 1.1.  The following if statement is
	    // a short term fix for this problem.
	    //
	    if (!strcmp(addr->dtm_address, ",")) {
		continue;
	    }

	    if (need_comma) {
		strcat(str, ", ");
	    }

	    need_comma = DTM_TRUE;

	    strcat(str, addr->dtm_address);
	}

	delete addr_seq;
    }

    return(str);
}


// Container menu "Compose==>New Message"
ComposeCmd::ComposeCmd( 
			char *name,
			char *label,
			int active,
			RoamMenuWindow *window
			) : ComposeFamily( name, label, active, window )
{
}

// Put up a blank compose window.
void
ComposeCmd::doit()
{
    SendMsgDialog * newsend = theCompose.getWin();
    if (newsend == NULL) {
	DtMailGenDialog * dialog = _parent->genDialog();
	
	dialog->setToErrorDialog(catgets(DT_catd, 1, 203, "Mailer"),
				 catgets(DT_catd, 1, 204, "Unable to create a compose window."));
	char * helpId = DTMAILHELPNOCOMPOSE;
	int answer = dialog->post_and_return(helpId);
	return;
    }
    XtRemoveCallback(newsend->getTextW(), XmNmodifyVerifyCallback,
		&SendMsgDialog::editorContentsChangedCB, newsend);

    appendSignature(newsend, 0);

    XtAddCallback(newsend->getTextW(), XmNmodifyVerifyCallback,
		&SendMsgDialog::editorContentsChangedCB, newsend);
}
// Container menu "Compose==>New Message"
ComposeSigCmd::ComposeSigCmd( 
			char *name,
			char *label,
			int active,
			RoamMenuWindow *window,
			SendMsgDialog *smd
			) : ComposeFamily( name, label, active, window )
{
	_smd = smd;
}
void
ComposeSigCmd::doit()
{
    XtRemoveCallback(_smd->getTextW(), XmNmodifyVerifyCallback,
                        &SendMsgDialog::editorContentsChangedCB, _smd);

    appendSignature(_smd, 1);

    XtAddCallback(_smd->getTextW(), XmNmodifyVerifyCallback,
                        &SendMsgDialog::editorContentsChangedCB, _smd);
}

// Container menu "Compose==>New, Include All" and "Compose==>Forward Message"
// The last parameter is a switch for "include" or "forward" format.
ForwardCmd::ForwardCmd( 
			char *name,
			char *label,
			int active,
			RoamMenuWindow *window, 
			int forward
			) : ComposeFamily(name, label, active, window)
{
    _forward = forward;
}

// Forward or Include selected messages.
// For Include message(s), all Compose window header fields are left blank.
// For Forward message(s), the Compose window "Subject" header field is filled
// with the subject of the last selected message.
void
ForwardCmd::doit()
{
    FORCE_SEGV_DECL(MsgHndArray, msgList);
    FORCE_SEGV_DECL(MsgStruct, tmpMS);
    DtMailMessageHandle msgno;
    
    // Get a Compose window.
    SendMsgDialog *newsend = theCompose.getWin();
    if ( newsend == NULL ) {
	DtMailGenDialog * dialog = _parent->genDialog();
	
	dialog->setToErrorDialog(catgets(DT_catd, 1, 205, "Mailer"),
			 catgets(DT_catd, 1, 206, "Unable to create a compose window."));
	char * helpId = DTMAILHELPNOCOMPOSE;
	int answer = dialog->post_and_return(helpId);
	return;
    }
    XtRemoveCallback(newsend->getTextW(), XmNmodifyVerifyCallback,
                        &SendMsgDialog::editorContentsChangedCB, newsend);

    // Put the signature above the message.
    //
    appendSignature(newsend, 0);

    // For Forwarding subject
    DtMail::MailBox * mbox = _menuwindow->mailbox();
    DtMail::Message * msg;
    DtMail::Envelope * env;
    DtMailValueSeq	value;
    DtMailEnv error;
    
    // For each selected message, put it in the Compose window.
    if ( msgList = _menuwindow->list()->selected() ) {
	for ( int k = 0;  k < msgList->length();  k++ ) {
	    tmpMS = msgList->at(k);
	    msgno = tmpMS->message_handle;
	    msg = mbox->getMessage(error, msgno);
	    env = msg->getEnvelope(error);
	    value.clear();
	    env->getHeader(error, DtMailMessageSubject, DTM_TRUE, value);
	    if (!error.isSet()) 
	    	newsend->setHeader("Subject", *(value[0]), DTM_FALSE);
	    if ( _forward ) 
		Display_entire_msg(msgno, newsend, "forward");
	    else 
		Display_entire_msg(msgno, newsend, "forwardBody");
	}
    }
    newsend->get_editor()->textEditor()->set_to_top();

    XtAddCallback(newsend->getTextW(), XmNmodifyVerifyCallback,
                        &SendMsgDialog::editorContentsChangedCB, newsend);
}

// Container menu "Compose==>Reply to Semder" and
// "Compose==>Reply to Sender, Include"
// The last parameter is a switch for including the selected message or not.
ReplyCmd::ReplyCmd ( 
		     char *name, 
		     char *label,
		     int active, 
		     RoamMenuWindow *window, 
		     int include 
		     ) : ComposeFamily ( name, label, active, window )
{
    _include = include;
}

// For each message selected, reply to sender.
void
ReplyCmd::doit()
{
    FORCE_SEGV_DECL(MsgHndArray, msgList);
    FORCE_SEGV_DECL(MsgStruct, tmpMS);
    DtMailMessageHandle msgno;
    FORCE_SEGV_DECL(char, from);
    FORCE_SEGV_DECL(char, subject);
    FORCE_SEGV_DECL(char, cc);
    DtMailEnv error;
    DtMail::MailBox * mbox = _menuwindow->mailbox();
    
    // Initialize the error.
    error.clear();
    
    if ( msgList=_menuwindow->list()->selected() )
	for ( int i=0; i < msgList->length(); i++ ) {
	    tmpMS = msgList->at(i);
	    msgno = tmpMS->message_handle;
	    SendMsgDialog *newsend = theCompose.getWin();
	    if ( newsend == NULL ) {
		DtMailGenDialog * dialog = _parent->genDialog();
		
		dialog->setToErrorDialog(catgets(DT_catd, 1, 207, "Mailer"),
			 catgets(DT_catd, 1, 208, 
			"Unable to create a compose window."));
		char * helpId = DTMAILHELPNOCOMPOSE;
		int answer = dialog->post_and_return(helpId);
		return;
	    }
	    XtRemoveCallback(newsend->getTextW(), XmNmodifyVerifyCallback,
                        &SendMsgDialog::editorContentsChangedCB, newsend);

	    XmUpdateDisplay( newsend->baseWidget() );
	    
	    DtMail::Message * msg = mbox->getMessage(error, msgno);
	    DtMail::Envelope * env = msg->getEnvelope(error);
	    
	    DtMailValueSeq	value;
	    
	    env->getHeader(error, DtMailMessageSender, DTM_TRUE, value);
	    if (error.isSet()) {
		newsend->setHeader("To", "nobody@nowhere", DTM_FALSE);
	    }
	    else {
		char * addr_str = valueToAddrString(value);
		newsend->setHeader("To", addr_str, DTM_FALSE);
		delete addr_str;
	    }
	    
	    value.clear();
	    env->getHeader(error, DtMailMessageSubject, DTM_TRUE, value);
	    if (error.isSet()) {
		subject = new char[200];
		strcpy(subject, "Re: ");
		DtMailValueSeq sent;
		env->getHeader(error,
			       DtMailMessageSentTime,
			       DTM_TRUE,
			       sent);
		if (error.isSet()) {
		    strcat(subject, "Your Message");
		}
		else {
		    strcat(subject, "Your Message Sent on ");
		    strcat(subject, *(sent[0]));
		}
		newsend->setHeader("Subject", subject, DTM_FALSE);
	    }
	    else {
		// Get the BE store of header.  It may contain newlines or 
		// tab chars which can munge the scrolling list's display!

		const char * orig = *(value[0]);

		int fc;
		int orig_length;
		char *tmp_subj;

		// Check if BE store contains the funky chars.

		for (fc = 0, orig_length = strlen(orig), 
		      tmp_subj = (char *) orig; 
		    fc < orig_length; 
		    fc++, tmp_subj++) {

		    char c = *tmp_subj;
		    if ((c == '\n') 
		     || (c == '\t') 
		     || (c == '\r')) {

			break;
		    }
		}

		subject = new char[fc+6];
		
		if (strncasecmp(orig, "Re:", 3)) {
			strcpy(subject, "Re: ");
		}
		else {
		    *subject = 0;
		}
		
		strncat((char *)subject, orig, fc);

		newsend->setHeader("Subject", subject, DTM_FALSE);
	    }
	    
	    newsend->title( subject );
	    delete subject;
	    if ( _include ) {
		Display_entire_msg(msgno, newsend, "indent");
	    }
	    appendSignature(newsend, 0);
	    newsend->setInputFocus(1);
	    XtAddCallback(newsend->getTextW(), XmNmodifyVerifyCallback,
                        &SendMsgDialog::editorContentsChangedCB, newsend);
	}
}

// Container menu "Compose==>Reply to All" and "Compose==>Reply to All, Include"
// The last parameter is a switch for including the selected message or not.
ReplyAllCmd::ReplyAllCmd( 
			  char *name,
			  char *label,
			  int active,
			  RoamMenuWindow *window, 
			  int include 
			  ) : ComposeFamily( name, label, active, window )
{
    _include = include;
}

// For each message selected, reply to everybody.
void
ReplyAllCmd::doit()
{
    FORCE_SEGV_DECL(MsgHndArray, msgList);
    FORCE_SEGV_DECL(MsgStruct, tmpMS);
    FORCE_SEGV_DECL(char, subject);
    FORCE_SEGV_DECL(char, to);
    FORCE_SEGV_DECL(char, buffer);
    DtMailMessageHandle msgno;
    DtMail::MailBox *mbox = _menuwindow->mailbox();
    DtMailEnv error;
    char *currentCcValue;
    DtMailValueSeq value;
    SendMsgDialog *newsend;
    DtMailGenDialog * dialog;
    DtMail::Message *msg;
    DtMail::Envelope *env;


    // Initialize the mail_error.
    error.clear();
    
    
    if ( msgList = _menuwindow->list()->selected() )
	for ( int k = 0;  k < msgList->length();  k++ ) {
	    tmpMS = msgList->at(k);
	    msgno = tmpMS->message_handle;
	    newsend = theCompose.getWin();
	    if ( newsend == NULL ) {
		dialog = _parent->genDialog();
		
		dialog->setToErrorDialog(catgets(DT_catd, 1, 209, "Mailer"),
			 catgets(DT_catd, 1, 210, 
			"Unable to create a compose window."));
		char * helpId = DTMAILHELPNOCOMPOSE;
		int answer = dialog->post_and_return(helpId);
		return;
	    }
	    XtRemoveCallback(newsend->getTextW(), XmNmodifyVerifyCallback,
                        &SendMsgDialog::editorContentsChangedCB, newsend);
	    msg = mbox->getMessage(error, msgno);
	    env = msg->getEnvelope(error);
	    
	    env->getHeader(
			   error, 
			   DtMailMessageToReply,
			   DTM_TRUE, 
			   value);
	    
	    env->getHeader(
			   error, 
			   DtMailMessageSender, 
			   DTM_TRUE, 
			   value);

	    char * addr_str = valueToAddrString(value);
	    newsend->setHeader("To", addr_str, DTM_FALSE);
	    delete addr_str;
	    value.clear();
	    env->getHeader(
			   error, 
			   DtMailMessageSubject, 
			   DTM_TRUE, 
			   value);
	    if ( error.isSet() ) {
		subject = new char[200];
		strcpy(subject, "Re: ");
		DtMailValueSeq sent;
		env->getHeader(error,
			       DtMailMessageSentTime,
			       DTM_TRUE,
			       sent);
		if (error.isSet()) {
		    strcat(subject, "Your Message");
		}
		else {
		    strcat(subject, "Your Message Sent on ");
		    strcat(subject, *(sent[0]));
		}
		newsend->setHeader("Subject", subject, DTM_FALSE);
	    } else {
		// Get the BE store of header.  It may contain newlines or 
		// tab chars which can munge the scrolling list's display!

		const char * orig = *(value[0]);


		int fc = 0;
		int orig_length;
		char *tmp_subj;

		// Check if BE store contains the funky chars.

		for (fc = 0, orig_length = strlen(orig), 
		      tmp_subj = (char *)orig; 
		     fc < orig_length; 
		     fc++, tmp_subj++) {

		    char c = *tmp_subj;
		    if ((c == '\n') 
		     || (c == '\t') 
		     || (c == '\r')) {

			break;
		    }
		}

		subject = new char[fc+6];
		
		if (strncasecmp(orig, "Re:", 3)) {
		    strcpy(subject, "Re: ");
		}
		else {
		    *subject = 0;
		}
		
		strncat((char *)subject, orig, fc);

		newsend->setHeader("Subject", subject, DTM_FALSE);
	    }
	    value.clear();
	    env->getHeader(
			   error, 
			   DtMailMessageCcReply, 
			   DTM_TRUE, 
			   value);
	    if (!error.isSet()) {
		// Strip out newlines from the cc line.  They *may* be 
		// present.
		currentCcValue = valueToAddrString(value);

		newsend->setHeader("Cc", currentCcValue, DTM_FALSE);
	    }
	    value.clear();
	    newsend->title(subject);
	    delete subject;
	    if ( _include )
		Display_entire_msg(msgno, newsend, "indent");
	    appendSignature(newsend, 0);
	    newsend->setInputFocus(1);
   	    XtAddCallback(newsend->getTextW(), XmNmodifyVerifyCallback,
                        &SendMsgDialog::editorContentsChangedCB, newsend);
	}
}
AliasCmd::AliasCmd(char *name,
			 char *label,
			 int active,
			 SendMsgDialog * compose,
			 const char * alias)
: NoUndoCmd(name, label, active)
{
    _compose = compose;
    if (alias)
    	_alias = strdup(alias);
}

AliasCmd::~AliasCmd(void)
{
    if (_alias)
	free(_alias);
}

void
AliasCmd::doit()
{
    Widget fw = XmGetFocusWidget(_compose->baseWidget());
    String label;
    XmString str;
    char buf[128];

    XtVaGetValues(fw, XmNlabelString, &str, NULL);
    XmStringGetLtoR(str, XmSTRING_DEFAULT_CHARSET, &label);
    XmStringFree(str);

    // Find the text field associated with the menu button that 
    // invoked this alias command.
    for (SendMsgDialog::HeaderList *hl = _compose->getFirstHeader();
                hl; hl = _compose->getNextHeader()) {
	// Get rid of the ":"	
	strcpy(buf, label);
	buf[strlen(label)-1] = '\0';
        if (strcmp(buf, hl->label) == 0 && 
		(hl->show == SendMsgDialog::SMD_ALWAYS ||
                hl->show == SendMsgDialog::SMD_SHOWN)) {

	  	XtRemoveCallback(hl->field_widget,
          		XmNvalueChangedCallback, 
			&SendMsgDialog::editorContentsChangedCB,
          		_compose);

		XmTextPosition left=-1, right=-1;

		XmTextFieldGetSelectionPosition(hl->field_widget, 
					&left, &right);
		// If left is -1 that means nothing was selected so
		// nothing to replace, so just insert the text at the
		// insertion point.
		if (left != -1)
			XmTextFieldReplace(hl->field_widget, left, right, _alias);
		else
			XmTextFieldInsert(hl->field_widget, 
				XmTextGetInsertionPosition(hl->field_widget), _alias);
		// Just append a space at the end of the textfield for 
		// convenience in case they want to insert more aliases.
		XmTextFieldInsert(hl->field_widget, 
			XmTextGetInsertionPosition(hl->field_widget), " ");

		XmProcessTraversal(hl->field_widget, XmTRAVERSE_CURRENT);

	  	XtAddCallback(hl->field_widget,
          		XmNvalueChangedCallback, 
			&SendMsgDialog::editorContentsChangedCB,
          		_compose);
		break;
   	}
   }
   XtFree(label);
}

LoadFileCmd::LoadFileCmd(char *name,
			 char *label,
			 int active,
			 SendMsgDialog * compose,
			 const char * file,
			 FileType type)
: NoUndoCmd(name, label, active)
{
    _compose = compose;
    _type = type;

    if (file && *file != '/' && *file != '~' && *file != '$') {
	char *dir;
	// This is for backward compatibility. In theory the only way a template has
        // a relative path is if it was set up in mailtool, so default dir goes to home
  	// directory vs. MailTemplates. If they hand edited .mailrc then too bad.
	if (_type == TYPE_TEMPLATE)
  		dir = "$HOME";
	else if (_type == TYPE_DRAFT)
  		dir = (char*)theRoamApp.getResource(DraftDir, FALSE);
        // Relative path.  Should be relative to home directory
        _file = (char *)malloc(strlen(file) + strlen(dir) + 2);
	sprintf(_file, "%s/%s", dir, file);
	if (type == TYPE_DRAFT)
		free(dir);
    } else 
	if (file)
        	_file = strdup(file);
	else
		_file = NULL;
}

LoadFileCmd::~LoadFileCmd(void)
{
    if (_file)
    	free(_file);
}

void
LoadFileCmd::doit()
{
    DtMailEnv error;
    DtMail::Session * d_session = theRoamApp.session()->session();
    DtMailBuffer mbuf;
    int answer, status, reset_compose = FALSE;
    char *helpId;
    DtMail::BodyPart *tmpBP = NULL;

    // We only want to get a new compose if this command is called from the
    // Compose menu-include Template from RMW or from the Edit button 
    // on the Templates pane of the mail options, not from the Format menu 
    // of the compose win. Thats because we dont want to permanently couple 
    // a compose window to a template cmd.. we'll just take whatever compose
    // window happens to be available at the time (as they are recycled)

    if (!_file)
	return;

    if (_compose == NULL) {
    	_compose = theCompose.getWin();
	reset_compose = TRUE;
    }

    if (_type == TYPE_TEMPLATE) {
    	char *buf = _compose->getTemplateFile();
    	if (buf) free(buf);
    	buf = _compose->getTemplateFullPath();
    	if (buf) free(buf);
    	_compose->setTemplateFile(strdup(this->getLabel()));
  	buf = strdup(_file);
	char *last_slash = strrchr(buf, '/');
	if (last_slash)
		*last_slash = '\0';
    	_compose->setTemplateFullPath(buf);
    }
    else if (_type == TYPE_DRAFT) {
    	char *buf = _compose->getDraftFile();
    	if (buf) free(buf);
    	_compose->setDraftFile(strdup(this->getLabel()));
    }

    DtMailGenDialog * dialog = _compose->genDialog();
    char *fullpath = d_session->expandPath(error, _file);

    // Map the file and try to parse it as a message. If it is a message,
    // then load it with headers. Otherwise, throw everything into the
    // editor.
    //
    int fd = SafeOpen(fullpath, O_RDONLY);
    free(fullpath);
    if (fd < 0) {
	if (_type == TYPE_TEMPLATE) {
		dialog->setToErrorDialog(catgets(DT_catd, 1, 211, "Mailer"),
			 catgets(DT_catd, 1, 212, "The template does not exist."));
		helpId = DTMAILHELPNOTEMPLATE;
    	}
	else if (_type == TYPE_DRAFT) {
		dialog->setToErrorDialog(catgets(DT_catd, 1, 287, "Mailer"),
			 catgets(DT_catd, 1, 287, "The draft does not exist."));
		helpId = DTMAILHELPNODRAFT;
        }
	answer = dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), helpId);
	if (reset_compose) 
		_compose = NULL;
	return;
    }

    struct stat sbuf;
    if (SafeFStat(fd, &sbuf) < 0) {
	if (_type == TYPE_TEMPLATE) {
		dialog->setToErrorDialog(catgets(DT_catd, 1, 213, "Mailer"),
			 catgets(DT_catd, 1, 214, "The template appears to be corrupt."));
		helpId = DTMAILHELPCORRUPTTEMPLATE;
   	}
	else if (_type == TYPE_DRAFT) {
		dialog->setToErrorDialog(catgets(DT_catd, 1, 213, "Mailer"),
			 catgets(DT_catd, 1, 288, "The draft appears to be corrupt."));
		helpId = DTMAILHELPCORRUPTDRAFT;
	}
	answer = dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), helpId);
	SafeClose(fd);
	if (reset_compose) 
		_compose = NULL;
	return;
    }

    int page_size = (int)sysconf(_SC_PAGESIZE);
    size_t map_size = (size_t) (sbuf.st_size + 
				(page_size - (sbuf.st_size % page_size)));

    int free_buf = 0;
    mbuf.size = sbuf.st_size;
    mbuf.buffer = mmap(0, map_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mbuf.buffer == (char *)-1) {
	free_buf = 1;
	mbuf.buffer = new char[mbuf.size];
	if (mbuf.buffer == NULL) {
	     if (_type == TYPE_TEMPLATE) {
		    dialog->setToErrorDialog(catgets(DT_catd, 1, 215, "Mailer"),
			     catgets(DT_catd, 1, 216, 
				"There is not enough memory to load the template."));
		    helpId = DTMAILHELPNOMEMTEMPLATE;
	     }
	     else if (_type == TYPE_DRAFT) {
		    dialog->setToErrorDialog(catgets(DT_catd, 1, 215, "Mailer"),
			     catgets(DT_catd, 1, 289, 
				"There is not enough memory to load the draft."));
		    helpId = DTMAILHELPNOMEMDRAFT;
	    }
	    answer = dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), helpId);
	    SafeClose(fd);
	    if (reset_compose) 
		_compose = NULL;
	    return;
	}

	if (SafeRead(fd, mbuf.buffer, (unsigned int)mbuf.size) < mbuf.size) {
	    if (_type == TYPE_TEMPLATE) 
	    	dialog->setToErrorDialog(catgets(DT_catd, 1, 217, "Mailer"),
		     catgets(DT_catd, 1, 218, "The template appears to be corrupt."));
	
	     else if (_type == TYPE_DRAFT) 
	    	dialog->setToErrorDialog(catgets(DT_catd, 1, 217, "Mailer"),
		     catgets(DT_catd, 1, 288, "The draft appears to be corrupt."));
	     helpId = DTMAILHELPERROR;
	     answer = dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), helpId);
	     SafeClose(fd);
	     delete mbuf.buffer;
	     if (reset_compose) 
		_compose = NULL;
	     return;
	}
    }

    // Now we ask the library to parse it. If this fails for any reason, this
    // is not a message, so we give up.
    //
    DtMail::Message * msg = d_session->messageConstruct(error,
							DtMailBufferObject,
							&mbuf,
							NULL,
							NULL,
							NULL);
    if (error.isSet()) {
        XtRemoveCallback(_compose->getTextW(), XmNmodifyVerifyCallback,
                        &SendMsgDialog::editorContentsChangedCB, _compose);
	status = _compose->get_editor()->textEditor()->append_to_contents(
			(char *) mbuf.buffer, mbuf.size);
        _compose->get_editor()->textEditor()->set_to_top();
        XtAddCallback(_compose->getTextW(), XmNmodifyVerifyCallback,
                        &SendMsgDialog::editorContentsChangedCB, _compose);
	if (status != 0) {
    		if (free_buf) {
			free(mbuf.buffer);
    		}
    		else {
			munmap((char *)mbuf.buffer, map_size);
    		}
		SafeClose(fd);
	    	if (reset_compose) 
			_compose = NULL;
		return;
	}
    }
    else {
	DtMail::BodyPart * bp = msg->getFirstBodyPart(error);
	if (bp == NULL || error.isSet()) {
		dialog = _compose->genDialog();
	        if (_type == TYPE_TEMPLATE) {
			dialog->setToWarningDialog(catgets(DT_catd, 1, 203, "Mailer"),
                                 catgets(DT_catd, 1, 249, "The template is empty."));
        		dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPTEMPLATEEMPTY);
		}
		else if (_type == TYPE_DRAFT) {
			dialog->setToWarningDialog(catgets(DT_catd, 1, 203, "Mailer"),
                                 catgets(DT_catd, 1, 290, "The draft is empty."));
        		dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPDRAFTEMPTY);
		}
	}
	else {
             XtRemoveCallback(_compose->getTextW(), XmNmodifyVerifyCallback,
                        &SendMsgDialog::editorContentsChangedCB, _compose);
	     char * status_string;
	     if (reset_compose)
	     	_compose->loadHeaders(msg, DTM_TRUE, DTM_FALSE);
	     else
	     	_compose->loadHeaders(msg, DTM_TRUE, DTM_TRUE);
	     DtMailBoolean firstBPHandled =
   	    	 _compose->get_editor()->textEditor()->set_message(
                                         msg,
                                         &status_string,
                                         Editor::HF_NONE,
                                         Editor::IF_NONE,
                                         Editor::BF_NONE);
 
	     int num_bodyParts = msg->getBodyCount(error);
	     DtMailBuffer tmpBuffer;
	     if ((num_bodyParts > 1) || (!firstBPHandled)) {
            	tmpBP = msg->getFirstBodyPart(error);
	        if (firstBPHandled) {
                	_compose->setInclMsgHnd(msg, TRUE);
                	tmpBP = msg->getNextBodyPart(error, tmpBP);
            	} else 
                	_compose->setInclMsgHnd(msg, FALSE);
               
            	char *name;
            	while (tmpBP != NULL) {
               		 tmpBP->getContents(
                                    error, (const void **) &tmpBuffer.buffer,
                                    &tmpBuffer.size,
                                    NULL,
                                    &name,
                                    NULL,
                                    NULL);
                	if (!name) 
                    		name = "NoName";
 
                	_compose->add_att(name, tmpBuffer);
                	tmpBP = msg->getNextBodyPart(error, tmpBP);
                	if (strcmp(name, "NoName") != 0) 
                    		free(name);
            	}
	        _compose->get_editor()->manageAttachArea();
            }
	    XtAddCallback(_compose->getTextW(), XmNmodifyVerifyCallback,
                        &SendMsgDialog::editorContentsChangedCB, _compose);
	}
    }
    if (free_buf)
	free(mbuf.buffer);
    
    else 
	munmap((char *)mbuf.buffer, map_size);
    if (reset_compose) 
	_compose = NULL;

    SafeClose(fd);
}

HideShowCmd::HideShowCmd(char *name,
			 char *widgetlabel,
			 int active,
			 SendMsgDialog * compose,
			 const char * label)
: NoUndoCmd(name, (char *)widgetlabel, active)
{
    _compose = compose;
    _header = strdup(label);
}

HideShowCmd::~HideShowCmd(void)
{
    if (_header) {
	free(_header);
    }
}

void
HideShowCmd::doit(void)
{
    _compose->changeHeaderState(_header);
}
