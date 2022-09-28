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
#pragma ident "@(#)ComposeCmds.C	1.117 06/04/97"
#endif

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <ctype.h>

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
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/MessageStore.hh>
#include <SDtMail/Message.hh>
#include <SDtMail/MessageEnvelope.hh>
#include <SDtMail/MessageBody.hh>
#include <SDtMail/ContentBuffer.hh>
#include <SDtMail/Session.hh>
#include <SDtMail/SystemUtility.hh>
#include <SDtMail/DataTypeUtility.hh>
#include <SDtMail/MailRc.hh>
#include <SDtMail/Utility.hh>
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
#include <DtMail/PropSource.hh>
#include "Help.hh"
#include <Dt/Help.h>
#include "Attachment.h"
#include <Xm/TextF.h>
#include <locale.h>
#include <SDtMail/Connection.hh>
#include <lcl/lcl.h>

ComposeFamily::ComposeFamily(char *name,
			     char *label,
			     int active,
			     RoamMenuWindow *window)
  : RoamCmd(name, label, active, window)
{
  _parent = window;
}

ComposeFamily::~ComposeFamily( void )
{
}

// Append a formatted message to Compose's Text area.
// This routine is essentially the same as MsgScollingList::display_message()
// except for two major differences:
// 1. No RoamMenuWindow reference (so that Compose can be standalone).
// 2. Indent string can be used for "include" and "forward".
void
ComposeFamily::Display_entire_msg(
          SdmError &error,
          SdmMessageNumber msgno,
				  SendMsgDialog *compose, 
				  char *format 
				  )
{
  SdmMessage *msg;
  SdmMessageEnvelope *env;
      
  SdmMessageStore *mbox = _menuwindow->mailbox();

  error.Reset();

  mbox->SdmMessageFactory(error, msg, msgno);
  if (error) return;

  msg->SdmMessageEnvelopeFactory(error, env);
  if (error) return;

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

  compose->get_editor()->textEditor()->insert_at_cursor("\n", 2);
  SdmBoolean firstBPHandled =
    compose->get_editor()->textEditor()->set_message(
                 error,
						     msg, 
						     &status_string, 
						     Editor::HF_ABBREV,
						     ins_format, 
						     brackets);

  if (error) return;

  // Now need to handle the unhandled body parts of the message.
  // we need to handle adding the attachments individually.  we
  // can't call AttachArea::parseAttachments because this method
  // does not hook up the body structure for a multipart SdmMessage.
  // This is done in SendMsgDialog::add_att; therefore, we need to
  // call this method to add each attachment.  AddAttachmentsForMessage
  // takes care of all this.
  if (strcmp(format, "forwardBody") == 0) {
    AddAttachmentsForMessage(error, msg, firstBPHandled, compose);
  } 
}


//
// AddAttachmentsForMessage is similar to the processMessage method in
// AttachArea.C.  The difference is that we call SendMsgDialog::add_att
// instead of AttachAread::addAttachment to add the attachment to the
// message.
int
ComposeFamily::AddAttachmentsForMessage(SdmError &mail_error, SdmMessage *msg, 
					SdmBoolean skipFirstText, SendMsgDialog *composeDialog)
{
  SdmMessageBodyStructure bpStruct;
  SdmMessage *nestedMsg;
  SdmMessageBody *tmpBP, *nestedBody;
  int nestedBodyCount;
  int numAttachmentsAdded = 0;
  int bodyCount;
  SdmString strContents;
  SdmContentBuffer contentBuf;
  
  msg->GetBodyCount(mail_error, bodyCount);
  if (mail_error) return 0;

  for (int i = 1; i <= bodyCount; i++) {

    msg->SdmMessageBodyFactory(mail_error, tmpBP, i);
    if (mail_error) 
      return numAttachmentsAdded;

    tmpBP->GetStructure(mail_error, bpStruct);
    if (mail_error) 
      return numAttachmentsAdded;

    switch ( bpStruct.mbs_type ) {

    case Sdm_MSTYPE_multipart:

      tmpBP->SdmMessageFactory(mail_error, nestedMsg);
      // if we can't get the nested message from a multipart mixed,
      // just continue.  we can't get the contents of the bodies in
      // the multipart.
      if (!mail_error) {
        nestedMsg->SdmMessageBodyFactory(mail_error, nestedBody, 1);
        if (mail_error) 
          return numAttachmentsAdded;
         
        nestedMsg->GetBodyCount(mail_error, nestedBodyCount);
        if (mail_error) 
          return numAttachmentsAdded;

        numAttachmentsAdded += AddAttachmentsForMessage(mail_error,
						      nestedMsg,
						      skipFirstText,
						      composeDialog);
						      
        if (mail_error) 
          return numAttachmentsAdded;
        
      } else {
        mail_error = Sdm_EC_Success;  // reset error
      }
      break;

    case Sdm_MSTYPE_text:
      // skip the first text if it was already placed in message area.
      if ( skipFirstText ) {
        break;
      }
    default:
      const char *name = (const char*)bpStruct.mbs_attachment_name;
      if ( !name || name[0] == '\0') {
        name = "NoName";
      }
      tmpBP->GetContents(mail_error, contentBuf);
      if (mail_error) 
          return numAttachmentsAdded;
          
      contentBuf.GetContents(mail_error, strContents);
      if (mail_error) 
        return numAttachmentsAdded;
      
      composeDialog->add_att((const char*)name, strContents);
      // Reset this since this is called from the Forward command, so no edits!
      composeDialog->setEditsMade(Sdm_False); 
      numAttachmentsAdded++;
      break;
    }

    // Do it after the first iteration. (It should be done only once)
    skipFirstText = Sdm_False;
      
  } // end for
  
  return numAttachmentsAdded;
}

void
ComposeFamily::appendSignature(SendMsgDialog * compose, SdmBoolean always_append)
{
  SdmError error;
  SdmSession * d_session = theRoamApp.osession()->osession();
  SdmMailRc *mail_rc;
  theRoamApp.connection()->connection()->SdmMailRcFactory(error, mail_rc);
  assert(!error);
  int status;
  char *value = NULL;

  // Sometimes we want to append regardless of the property setting, such
  // as in the case of the Include Signature menu option on Compose Format menu
  if (!always_append) {
    if (mail_rc->IsValueDefined("dontincludesignature")) {
      compose->get_editor()->textEditor()->set_to_top();
      return;
    }
  }

  MailRcSource *prop_source = (MailRcSource*) new MailRcSource (mail_rc, "signature");
  value = (char*)prop_source->getValue();
  delete prop_source;
  char * fullpath;
  SdmUtility::ExpandPath(error, fullpath, value, *mail_rc, SdmUtility::kFolderResourceName);

  if (value)
    free(value);

  FILE *fp;
  fp = fopen(fullpath, "r");
  if (fp != NULL) {
    status = compose->get_editor()->textEditor()->append_to_contents("\n", 2);
    if (status == 0)
      status = compose->get_editor()->textEditor()->append_to_contents(fileno(fp));
    fclose(fp);
  }

  free(fullpath);
  if (!always_append)
    compose->get_editor()->textEditor()->set_to_top();
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

  appendSignature(newsend, Sdm_False);

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

  appendSignature(_smd, Sdm_True);

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
  SdmMessageNumber	msgno;

  // For Forwarding subject
  SdmMessageStore * mbox = _menuwindow->mailbox();
  SdmMessage * msg;
  SdmMessageEnvelope * env;
  SdmIntStrL	headerValues;
  SdmError	error;
  SendMsgDialog *newsend;
   
  // For each selected message, put it in the Compose window.
  if ( msgList = _menuwindow->list()->selected() ) {

    // Get a Compose window.
    newsend = theCompose.getWin();
    if ( newsend == NULL ) {
      DtMailGenDialog * dialog = _parent->genDialog();

      dialog->setToErrorDialog(catgets(DT_catd, 1, 205, "Mailer"),
             catgets(DT_catd, 1, 206, "Unable to create a compose window."));
      char * helpId = DTMAILHELPNOCOMPOSE;
      int answer = dialog->post_and_return(helpId);
      msgList->remove_all_elements();
      delete msgList;
      return;
    }

    for ( int k = 0;  k < msgList->length();  k++ ) {

      // Initialize the error.
      error.Reset();

      tmpMS = msgList->at(k);
      msgno = tmpMS->message_handle;

      XtRemoveCallback(newsend->getTextW(), XmNmodifyVerifyCallback,
           &SendMsgDialog::editorContentsChangedCB, newsend);

      // Put the signature above the message.
      //
      appendSignature(newsend, Sdm_False);
      
      mbox->SdmMessageFactory(error, msg, msgno);
      if (error) {
        DtMailGenDialog * dialog = _parent->genDialog();
	dialog->post_error(error, DTMAILHELPCANNOTACCESSMESSAGE,
			   catgets(DT_catd, 3,139, "Mailer cannot access the message you selected."));
        newsend->goAway(Sdm_False);
        msgList->remove_all_elements();
        delete msgList;
        return;
      }

      msg->SdmMessageEnvelopeFactory(error, env);
      if (error) {
        DtMailGenDialog * dialog = _parent->genDialog();
	dialog->post_error(error, DTMAILHELPCANNOTACCESSMESSAGE,
			   catgets(DT_catd, 3,139, "Mailer cannot access the message you selected."));
        newsend->goAway(Sdm_False);
        msgList->remove_all_elements();
        delete msgList;
        return;
      }

      headerValues.ClearAllElements();
      env->GetHeaders(error, headerValues, Sdm_MHA_Subject);
      if (error != Sdm_EC_Success && error != Sdm_EC_RequestedDataNotFound) {
        DtMailGenDialog * dialog = _parent->genDialog();
	dialog->post_error(error, DTMAILHELPCANNOTACCESSMESSAGE,
			   catgets(DT_catd, 3,139, "Mailer cannot access the message you selected."));
        newsend->goAway(Sdm_False);
        msgList->remove_all_elements();
        delete msgList;
        return;
      }
      error.Reset();

      // I18N Decode the subject header
      SdmString charset;
      SdmString subject;
      env->GetCharacterSet(error, charset);
      assert(!error);

      for (int j=0; j< headerValues.ElementCount(); j++) {
        env->DecodeHeader(error, subject, headerValues[j].GetString(), charset);
        }
        // I18N end
          
        newsend->setHeader("Subject", (const char*) subject, Sdm_False);
       
      if ( _forward ) 
        Display_entire_msg(error, msgno, newsend, "forwardBody");
      else 
        Display_entire_msg(error, msgno, newsend, "forward");

      if (error != Sdm_EC_Success && error != Sdm_EC_RequestedDataNotFound) {
        DtMailGenDialog * dialog = _parent->genDialog();
        char buf[2048];
        sprintf(buf, catgets(DT_catd, 3,139, "Unable to get data for message: %d"), msgno);
	dialog->post_error(error, DTMAILHELPERROR, buf);
        newsend->goAway(Sdm_False);
        msgList->remove_all_elements();
        delete msgList;
        return;
      } else {        
        newsend->get_editor()->textEditor()->set_to_top();
        XtAddCallback(newsend->getTextW(), XmNmodifyVerifyCallback,
          &SendMsgDialog::editorContentsChangedCB, newsend);
        error.Reset();
      }
      
      _menuwindow->delete_message(msgno);
    }
    msgList->remove_all_elements();
    delete msgList;
  }
}

// Container menu "Compose==>Reply to Sender" and
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
  SdmMessageNumber msgno;
  SdmError error;
  SdmMessageStore * mbox = _menuwindow->mailbox();
  SdmIntStrL	headerValues;
  SdmIntStrL	sent;
    
  if ( msgList=_menuwindow->list()->selected() ) {
    for ( int i=0; i < msgList->length(); i++ ) {
      // Initialize the error.
      error.Reset();
      
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
        msgList->remove_all_elements();
        delete msgList;
        return;
      }
      XtRemoveCallback(newsend->getTextW(), XmNmodifyVerifyCallback,
		       &SendMsgDialog::editorContentsChangedCB, newsend);

      XmUpdateDisplay( newsend->baseWidget() );

      SdmMessage * msg;
      mbox->SdmMessageFactory(error, msg, msgno);
      if (error) {
        DtMailGenDialog * dialog = _parent->genDialog();
	dialog->post_error(error, DTMAILHELPCANNOTACCESSMESSAGE,
			   catgets(DT_catd, 3,139, "Mailer cannot access the message you selected."));
        newsend->goAway(Sdm_False);
        msgList->remove_all_elements();
        delete msgList;
        return;
      }
      SdmMessageEnvelope * env;
      msg->SdmMessageEnvelopeFactory(error, env);
      if (error) {
        DtMailGenDialog * dialog = _parent->genDialog();
	dialog->post_error(error, DTMAILHELPCANNOTACCESSMESSAGE,
			   catgets(DT_catd, 3,139, "Mailer cannot access the message you selected."));
        newsend->goAway(Sdm_False);
        msgList->remove_all_elements();
        delete msgList;
        return;
      }

      SdmString sender;
      SdmString subject;

      // How about Sdm_MHA_From?
      headerValues.ClearAllElements();
      env->GetHeaders(error, headerValues, Sdm_MHA_P_Subject);
      if (error == Sdm_EC_RequestedDataNotFound) error.Reset();

      if (!error) {
        SdmMessageUtility::MakeReplyHeader(error, sender, Sdm_MHA_P_Sender, *env, Sdm_False);
        if (error == Sdm_EC_RequestedDataNotFound) error.Reset();
      }

      if (error != Sdm_EC_Success) {
        DtMailGenDialog * dialog = _parent->genDialog();
	dialog->post_error(error, DTMAILHELPCANNOTACCESSMESSAGE,
			   catgets(DT_catd, 3,139, "Mailer cannot access the message you selected."));
        newsend->goAway(Sdm_False);
        msgList->remove_all_elements();
        delete msgList;
        return;
      }
              
      for (int j=0; j< headerValues.ElementCount(); j++) {
	// I18N Decode the subject header
	SdmString charset;
	env->GetCharacterSet(error, charset);
	assert(!error);
	env->DecodeHeader(error, subject, headerValues[j].GetString(), charset);
	// I18N end
      }

      // note: assume that if sender exists, there's only one.
      // if we can't find a sender, try the unix "From " line.
      if (sender.Length() == 0) {
        SdmStrStrL xUnixFrom;

        mbox->GetHeader(error, xUnixFrom, "x-unix-from", msgno);
        error.Reset();
        if (xUnixFrom.ElementCount() != 0) {
          const char *unixFrom = (const char*)xUnixFrom[0].GetSecondString();
          char *p;
          // IMPORTANT NOTE: To avoid unnecessary duplication, the following
          // code writes DIRECTLY into the internal SdmString object.  BE VERY
          // CAREFUL about applying other SdmString operations to this object,
          // since the SdmString believes its length to still be the original
          // length.
          if (p = strchr(unixFrom, ' '))
            *p = '\0';
          newsend->setHeader("To", unixFrom, Sdm_False);
        }
        else {
          newsend->setHeader("To", "nobody@nowhere", Sdm_False);
        }
      }
      else {
        newsend->setHeader("To", (const char *)sender, Sdm_False);
      }

      // if we can't get subject header.  we create our own.
      // note: assume that if subject exists, there's only only.
      if ( subject.Length() == 0 ) {
        subject = "Re: ";
        sent.ClearAllElements();
        env->GetHeaders(error, sent, Sdm_MHA_Date);
        if ( error ) {
          error = Sdm_EC_Success;   // reset error.
          subject +="Your Message";
        }
        else {
          subject +=  "Your Message Sent on ";
          subject += sent[0].GetString();
        }
        newsend->setHeader("Subject", (const char*) subject, Sdm_False);
      }
      else {
        // Get the BE store of header.  It may contain newlines or 
        // tab chars which can munge the scrolling list's display!
        SdmString str = subject;    // save reference to original subject.

        int fc;
        const char *tmp_subj;
        char c;

        // Check if BE store contains the funky chars.

        for (fc = 0, tmp_subj = (const char*)str; 
             fc < str.Length(); 
             fc++, tmp_subj++) 
	  {
            c = *tmp_subj;
            if ((c == '\n') || (c == '\t') || (c == '\r')) {
              break;
            }
	  }

        if (strncasecmp((const char*)str, "Re:", 3)) {
          subject = "Re: ";
        } else {
          subject = "";
        }

        // concatenate string to subject.
        subject += str(0, fc);

        newsend->setHeader("Subject", (const char*) subject, Sdm_False);
      }

      newsend->title( (const char*) subject );
      if ( _include ) {
        Display_entire_msg(error, msgno, newsend, "indent");
      }
      
      if (error != Sdm_EC_Success && error != Sdm_EC_RequestedDataNotFound) {
        DtMailGenDialog * dialog = _parent->genDialog();
	dialog->post_error(error, DTMAILHELPCANNOTACCESSMESSAGE,
			   catgets(DT_catd, 3,139, "Mailer cannot access the message you selected."));
        newsend->goAway(Sdm_False);
        msgList->remove_all_elements();
        delete msgList;
        return;
      } else {
        appendSignature(newsend, Sdm_False);
        newsend->setInputFocus(1);
        XtAddCallback(newsend->getTextW(), XmNmodifyVerifyCallback,
		      &SendMsgDialog::editorContentsChangedCB, newsend);
        error.Reset();
      }
      
      _menuwindow->delete_message(msgno);

    } // end for each message
    
    msgList->remove_all_elements();
    delete msgList;
  } // end if selected
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
  SdmMessageNumber msgno;
  SdmMessageStore *mbox = _menuwindow->mailbox();
  SdmError error;
  SdmIntStrL	sent;
  SdmIntStrL  headerValues;
  SendMsgDialog *newsend;
  DtMailGenDialog * dialog;
  SdmMessage *msg;
  SdmMessageEnvelope *env;

  if ( msgList = _menuwindow->list()->selected() ) {
    for ( int k = 0;  k < msgList->length();  k++ ) {
 
      // Initialize the mail_error.
      error.Reset();

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
        msgList->remove_all_elements();
        delete msgList;
        return;
      }
        
      XtRemoveCallback(newsend->getTextW(), XmNmodifyVerifyCallback,
		       &SendMsgDialog::editorContentsChangedCB, newsend);

      mbox->SdmMessageFactory(error, msg, msgno);
      if (error) {
        newsend->goAway(Sdm_False);
        dialog = _parent->genDialog();
	dialog->post_error(error, DTMAILHELPCANNOTACCESSMESSAGE,
			   catgets(DT_catd, 3,139, "Mailer cannot access the message you selected."));
        msgList->remove_all_elements();
        delete msgList;
        return;
      }
      msg->SdmMessageEnvelopeFactory(error, env);
      if (error) {
        dialog = _parent->genDialog();
	dialog->post_error(error, DTMAILHELPCANNOTACCESSMESSAGE,
			   catgets(DT_catd, 3,139, "Mailer cannot access the message you selected."));
        newsend->goAway(Sdm_False);
        msgList->remove_all_elements();
        delete msgList;
        return;
      }

      headerValues.ClearAllElements();

      SdmString	fromStr;
      SdmString	toStr;
      SdmString ccStr;
      SdmString subject;
      
      // get the subject and the address headers.  
      // call SdmMessageUtility::MakeReplyHeader so addresses will be removed
      // if metoo, alternates, and/or allnet variables are set in .mailrc.
      //
      env->GetHeaders(error, headerValues, Sdm_MHA_P_Subject);
      if (error == Sdm_EC_RequestedDataNotFound) error.Reset();
      
      if (!error) {
        SdmMessageUtility::MakeReplyHeader(error, ccStr, Sdm_MHA_P_Cc, *env, Sdm_True);
        if (error == Sdm_EC_RequestedDataNotFound) error.Reset();
      }
      if (!error) {
        SdmMessageUtility::MakeReplyHeader(error, fromStr, Sdm_MHA_P_Sender, *env, Sdm_True);
        // If the sender cannot be found, try the unix "From " line
        if (error == Sdm_EC_RequestedDataNotFound) {
          SdmStrStrL xUnixFrom;

          mbox->GetHeader(error, xUnixFrom, "x-unix-from", msgno);
          error.Reset();
          if (xUnixFrom.ElementCount() != 0) {
            fromStr = xUnixFrom[0].GetSecondString();
            char *p;
            // IMPORTANT NOTE: To avoid unnecessary duplication, the following
            // code writes DIRECTLY into the internal SdmString object.  BE VERY
            // CAREFUL about applying other SdmString operations to this object,
            // since the SdmString believes its length to still be the original
            // length.
            if (p = strchr(fromStr, ' '))
              *p = '\0';
          }
          error.Reset();
        }
      }
      if (!error) {
        SdmMessageUtility::MakeReplyHeader(error, toStr, Sdm_MHA_P_To, *env, Sdm_True);
        if (error == Sdm_EC_RequestedDataNotFound) error.Reset();
      }
      if (toStr.Length() != 0) {
        toStr += ", ";
        toStr += fromStr;
      }
      else
        toStr = fromStr;

      if (error != Sdm_EC_Success) {
        dialog = _parent->genDialog();
	dialog->post_error(error, DTMAILHELPCANNOTACCESSMESSAGE,
			   catgets(DT_catd, 3,139, "Mailer cannot access the message you selected."));
        newsend->goAway(Sdm_False);
        msgList->remove_all_elements();
        delete msgList;
        return;
      }
      error.Reset();

      // create single string with decoded subject header value
      //.      
      for (int j=0; j< headerValues.ElementCount(); j++) {
        // I18N Decode the subject header
        SdmString charset;
        env->GetCharacterSet(error, charset);
        assert(!error);
        env->DecodeHeader(error, subject, headerValues[j].GetString(), charset);
        // I18N end
      }
      error.Reset();
      
      newsend->setHeader("To", (const char *)toStr, Sdm_False);

      newsend->setHeader("Cc", (const char *)ccStr, Sdm_False);

      if ( subject.Length() == 0 ) {
        subject = "Re: ";
        sent.ClearAllElements();
        env->GetHeaders(error, sent, Sdm_MHA_Date);
        if (error != Sdm_EC_Success && error != Sdm_EC_RequestedDataNotFound) {
          dialog = _parent->genDialog();
	  dialog->post_error(error, DTMAILHELPCANNOTACCESSMESSAGE,
			     catgets(DT_catd, 3,139, "Mailer cannot access the message you selected."));
          newsend->goAway(Sdm_False);
          msgList->remove_all_elements();
          delete msgList;
          return;
        } else if ( error == Sdm_EC_RequestedDataNotFound) {
          error = Sdm_EC_Success;   // reset error.
          subject +="Your Message";
          error.Reset();
        }
        else {
          subject +=  "Your Message Sent on ";
          subject += sent[0].GetString();
        }
        newsend->setHeader("Subject", (const char*) subject, Sdm_False);
      }
      else {
        // Get the BE store of header.  It may contain newlines or 
        // tab chars which can munge the scrolling list's display!
        SdmString str = subject;    // save reference to orig subject.

        int fc;
        const char *tmp_subj;
        char c;

        // Check if BE store contains the funky chars.

        for (fc = 0, tmp_subj = (const char*)str; 
             fc < str.Length(); 
             fc++, tmp_subj++) 
        {
            c = *tmp_subj;
            if ((c == '\n') || (c == '\t') || (c == '\r')) {
              break;
            }
        }

        if (strncasecmp((const char*)str, "Re:", 3)) {
          subject = "Re: ";
        } else {
          subject = "";
        }
        
        // concatenate string to subject.
        subject += str(0, fc);

        newsend->setHeader("Subject", (const char*) subject, Sdm_False);
      }

      newsend->title((const char*)subject);

      if ( _include )
          Display_entire_msg(error, msgno, newsend, "indent");

      if (error != Sdm_EC_Success && error != Sdm_EC_RequestedDataNotFound) {
        dialog = _parent->genDialog();
	dialog->post_error(error, DTMAILHELPCANNOTACCESSMESSAGE,
			   catgets(DT_catd, 3,139, "Mailer cannot access the message you selected."));
        newsend->goAway(Sdm_False);
        msgList->remove_all_elements();
        delete msgList;
        return;
      } else {
        appendSignature(newsend, Sdm_False);
        newsend->setInputFocus(1);
        XtAddCallback(newsend->getTextW(), XmNmodifyVerifyCallback,
          &SendMsgDialog::editorContentsChangedCB, newsend);
		    error.Reset();
      }
      
      _menuwindow->delete_message(msgno);
                          
    } // end for

    msgList->remove_all_elements();
    delete msgList;
  } // end if
}

AliasCmd::AliasCmd(char *name,
		   char *label,
		   int active,
		   SendMsgDialog * compose,
		   const char * alias)
  : NoUndoCmd(name, label, active)
{
  _compose = compose;
  _alias = alias ? strdup(alias) : NULL;
}

AliasCmd::~AliasCmd(void)
{
  if (_alias)
    free(_alias);
}

void
AliasCmd::doit()
{
  String label;
  XmString str;
  char buf[128];
  Widget menubutton = XmGetPostedFromWidget(XtParent(_invoker));

  XtVaGetValues(menubutton, XmNlabelString, &str, NULL);
  XmStringGetLtoR(str, XmSTRING_DEFAULT_CHARSET, &label);
  XmStringFree(str);

  // Find the text field associated with the menu button that invoked this
  // alias command.
  for (SendMsgDialog::HeaderList *hl = _compose->getFirstHeader();
       hl; hl = _compose->getNextHeader()) {
    // Get rid of the ":"	
    strcpy(buf, label);
    buf[strlen(label)-1] = '\0';
    if (strcmp(buf, hl->label) == 0 && 
      (hl->show == SendMsgDialog::SMD_ALWAYS ||
       hl->show == SendMsgDialog::SMD_SHOWN)) 
    {
      XtRemoveCallback(hl->field_widget,
		       XmNvalueChangedCallback, 
		       &SendMsgDialog::editorContentsChangedCB,
		       _compose);

      XmTextPosition left=-1, right=-1;

      XmTextFieldGetSelectionPosition(hl->field_widget, 
				      &left, &right);
      // If left is -1 that means nothing was selected so nothing to
      // replace, so just insert the text at the insertion point.
      if (left != -1)
          XmTextFieldReplace(hl->field_widget, left, right, _alias);
      else {
	  char* value = XmTextGetString(hl->field_widget);
          // Separate the aliases with a comma and space - cclient requires
          // addresses to be comma separated.  Only append comma if one does
          // not already exist.
	  if (value) {
		if (*value) {
	  		SdmBoolean dont_append_comma = Sdm_False;
			char *ptr = value+strlen(value)-1;
			while (ptr && *ptr) 
		 		if (*ptr != ' ' && *ptr != ',') 
					break;
				else if (*ptr-- == ',') {
					dont_append_comma = Sdm_True;
					break;
				}
			if (!dont_append_comma)
          			XmTextFieldInsert(hl->field_widget, 
					XmTextGetInsertionPosition(hl->field_widget),
				 	", ");
		}
		XtFree(value);
	  }
          XmTextFieldInsert(hl->field_widget, 
              XmTextGetInsertionPosition(hl->field_widget), _alias);
      }

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
      dir = (char*)theRoamApp.getResource(DraftDir, Sdm_False);
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
  SdmError error;
  SdmSession *d_session = theRoamApp.osession()->osession();
  SdmSession *i_session = theRoamApp.isession()->isession();
  int i, reset_compose = FALSE;
  SdmMessageBody *tmpBP = NULL;
  SdmMailRc *mail_rc;
  SdmToken token;

  error.Reset();

  theRoamApp.connection()->connection()->SdmMailRcFactory(error, mail_rc);
  assert(!error);

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
    reset_compose = Sdm_True;
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
  char *fullpath;
  SdmUtility::ExpandPath(error, fullpath, _file, *mail_rc,
			 SdmUtility::kFolderResourceName);
  if (error) {
    fullpath = strdup(_file);
  }

  token.SetValue("servicechannel", "cclient");
  token.SetValue("serviceclass", "messagestore");
  token.SetValue("servicetype", "local");
  token.SetValue("serviceoption", "readonly", "x");
  token.SetValue("serviceoption", "ignoresessionlock", "true");
  token.SetValue("serviceoption", "messagestorename", fullpath);
  if (mail_rc->IsValueDefined("dontsavemimemessageformat"))
    token.SetValue("serviceoption", "preservev3messages", "x");
  if (mail_rc->IsValueDefined("logdebug"))
    token.SetValue("serviceoption", "debug", "x");

  SdmMessageStore *tmpmbox = NULL;
  SdmStrStrL headers;
  SdmString bodyStr;
  SdmMessage *msg;
  SdmMessageEnvelope *env;
  SdmMessageBody *body;

  i_session->SdmMessageStoreFactory(error, tmpmbox);
  if ( !tmpmbox || error ) {
    free(fullpath);
    return;
  }

  tmpmbox->StartUp(error);
  if ( error ) {
    free(fullpath);
    return;
  }

  int fd;
  static const char   *temp = "mbox.XXXXXX";
  char                fileName[MAXPATHLEN];

  fileName[0] = '\0';

  SdmBoolean readOnly;
  SdmMessageNumber nmsgs = 0;
  tmpmbox->Open(error, nmsgs, readOnly, token);
  if ( error ) {
    if ( _compose->parseFile(fullpath, &headers, &bodyStr) != Sdm_True ) {
      free(fullpath);
      return;
    }

    // Create a temporary file to serve as an outgoing mailbox
    strcpy(fileName, "/tmp/");
    strcat(fileName, temp);
    mktemp(fileName);

    // Create the temporary mailbox file - since this is a
    // behind the scenes kind of deal and it is e-mail, we use
    // mode 0600 as opposed to mode 0666 for security reasons.

    if ( (fd = SdmSystemUtility::SafeOpen(fileName, O_CREAT | O_WRONLY, 0600)) == -1 ) {
      return;
    }
 
    SdmSystemUtility::SafeClose(fd);

    // Change the token with appropriate values
    token.ClearValue("serviceoption", "messagestorename", fullpath);
    token.SetValue("serviceoption", "messagestorename", fileName);
    token.ClearValue("serviceoption", "readonly");
    token.SetValue("serviceoption", "readwrite", "x");

    free(fullpath);

    if ( tmpmbox )
      delete tmpmbox;
 
    d_session->SdmMessageStoreFactory(error, tmpmbox);
    if ( error ) {
      delete tmpmbox;
      SdmSystemUtility::SafeUnlink(fileName);
      return;
    }

    tmpmbox->StartUp(error);
    if ( error ) {
      delete tmpmbox;
      SdmSystemUtility::SafeUnlink(fileName);
      return;
    }

    tmpmbox->Open(error, nmsgs, readOnly, token);
    if ( error ) {
      delete tmpmbox;
      SdmSystemUtility::SafeUnlink(fileName);
      return;
    }

    // Create a message in this auxiliary outgoing message store
    tmpmbox->SdmMessageFactory(error, msg);
    if ( error ) {
      delete tmpmbox;
      SdmSystemUtility::SafeUnlink(fileName);
      return;
    }

    msg->SdmMessageEnvelopeFactory(error, env);
    if ( error ) {
      SdmSystemUtility::SafeUnlink(fileName);
      return;
    }

    if (headers.ElementCount() > 0) {
      env->SetHeaders(error, headers);
      if ( error ) {
        delete tmpmbox;
        SdmSystemUtility::SafeUnlink(fileName);
        return;
      }
    }

    msg->SdmMessageBodyFactory(error, body,
			       (SdmMsgStrType)Sdm_MSTYPE_text, "plain");
    if ( error ) {
      delete tmpmbox;
      SdmSystemUtility::SafeUnlink(fileName);
      return;
    }

    SdmContentBuffer content_buf(bodyStr);
    // I18N set the user selected charset on the content buffer
    content_buf.SetCharacterSet(error, _compose->getCharacterSet());
    if ( error ) {
      delete tmpmbox;
      SdmSystemUtility::SafeUnlink(fileName);
      return;
    }

    body->SetContents(error, content_buf);
    if ( error ) {
      delete tmpmbox;
      SdmSystemUtility::SafeUnlink(fileName);
      return;
    }

    SdmDataTypeUtility::SetContentTypesForMessage(error, *body);
    if (error) {
      error.Reset();  // just continue.
    }
  }
  else { // File successfully opened.
    free(fullpath);

    tmpmbox->SdmMessageFactory(error, msg, 1);
    if ( error ) {
      delete tmpmbox;
      return;
    }
  }

  SdmMessageBody *bp = NULL;
  msg->SdmMessageBodyFactory(error, bp, 1); // Get the first message body
  if (bp == NULL || error) {
#ifdef DOLATER
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
#endif
    if ( *fileName )
      SdmSystemUtility::SafeUnlink(fileName);
      
    if (tmpmbox)
      delete tmpmbox;
      
    return;
  }

  XtRemoveCallback(_compose->getTextW(), XmNmodifyVerifyCallback,
		   &SendMsgDialog::editorContentsChangedCB, _compose);

  if (reset_compose)
    _compose->loadHeaders(error, msg, Sdm_True, Sdm_False);
  else
    _compose->loadHeaders(error, msg, Sdm_True, Sdm_True);

  // Set the message body in compose area, we don't need the header
   char *status_string;
  _compose->get_editor()->textEditor()->set_message(error,
						      msg,
						      &status_string,
						    Editor::HF_NONE);        _compose->get_editor()->textEditor()->set_to_top();

  SdmContentBuffer contentBuf;
  SdmString contents;
  SdmMessageBodyStructure bodyStruct;

  if (!error) {
    bp->GetStructure(error, bodyStruct);
  }

  if ( !error && (bodyStruct.mbs_type == Sdm_MSTYPE_multipart ) ) {
      SdmMessage *nestedMsg;
      int bodyCount;
      const char *name;

      bp->SdmMessageFactory(error, nestedMsg);
      if (!error) {
        nestedMsg->GetBodyCount(error, bodyCount);
      }

      if (!error) {
        for (i = 2; i <= bodyCount; i++) {
          nestedMsg->SdmMessageBodyFactory(error, bp, i);
          if (error) break;

          bp->GetContents(error, contentBuf);
          if (error) break;        
          contentBuf.GetContents(error, contents);
          if (error) break;

          // Load the draft or template back - so is decoded
          if ((SdmSystemUtility::InCLocale() == Sdm_False) &&
               ((_type == TYPE_TEMPLATE) || (_type == TYPE_DRAFT))) {
            char *outputBp;
            int outputLen;

            DecodeBodyBuf(error, outputBp, outputLen, contents, contents.Length());
            if (error) break;
            contents = SdmString(outputBp, outputLen);
          }

          bp->GetStructure(error, bodyStruct);
          if (error) break;
          name = bodyStruct.mbs_attachment_name;
          _compose->add_att((char *)name, contents);
        }
      }
  }

  if (reset_compose)
    _compose = NULL;

  if (tmpmbox) 
    delete tmpmbox;

  if ( *fileName )
    SdmSystemUtility::SafeUnlink(fileName);
}

HideShowCmd::HideShowCmd(char *name, char *widgetlabel, int active,
			 SendMsgDialog * compose, const char * label)
  : NoUndoCmd(name, widgetlabel, active)
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
void
LoadFileCmd::DecodeBodyBuf(SdmError &err, char *&r_outputBp, int &r_outputLen,
              const char * inputBp, const unsigned long inputLen)
{ 
  SdmConnection	*conn;
  LCLd	lcld = (LCLd)NULL;
  LCTd	lclt = (LCTd)NULL;
  LclError	*ret = (LclError *)NULL;
  LclCharsetSegmentSet	*segs = (LclCharsetSegmentSet *)NULL;
  char	*body_buffer = (char *)NULL;
  char    *locale;

  err = Sdm_EC_Success;

  conn = SdmConnection::GetConnection();
  if (!conn) {
    err = Sdm_EC_ConnectionNotStarted;
    return;
  }
  if ( (locale = setlocale(LC_CTYPE, NULL)) == NULL ) {
    SdmUtility::LogError(Sdm_True,
       "sdtmailpr: localization failure - locale is not set correctly.\n");
    exit(1);
  }

  lcld = lcl_open(locale);

  if (!lcld) {
    err = Sdm_EC_LclOpenFailed;
    return;
  }

  // If the original input string is not null terminated, then we need to make a copy
  // as the lcl library requires a null terminated string for input.

  if (inputBp[inputLen] != '\0') {
    body_buffer = (char *)SdmMemoryRepository::_ONew(inputLen+1, Sdm_MM_AnonymousAllocation);
    if(body_buffer == (char *)NULL){
      err = Sdm_EC_Fail;
      return;
    }
    memcpy(body_buffer, inputBp, inputLen);
    body_buffer[inputLen] = (char)NULL;
  }

  // create lclt
  lclt = lct_create(lcld, LctNSourceType, LctNMsgText, LctNNone,
      body_buffer ? body_buffer : inputBp,
      LctNSourceForm, LctNInComingStreamForm,
      LctNKeepReference, LctNKeepByReference, NULL);

  // get converted body
  ret = lct_getvalues(lclt, LctNDisplayForm, LctNBodySegment, &segs, NULL);
  if (ret && ((ret->error_code != LctErrorIconvNonIdenticalConversion) 
      && (ret->error_code != LctErrorIconvHalfDone))){
    lcl_destroy_error(ret);
    lct_destroy(lclt);
    if (body_buffer)
      SdmMemoryRepository::_ODel(body_buffer, Sdm_MM_AnonymousAllocation);
    err = Sdm_EC_Fail;
    return;
  }

  r_outputBp = segs->seg[0].segment;
  r_outputLen = segs->seg[0].size;
  segs->seg[0].segment = (char *)NULL;
  lcl_destroy_segment_set(segs);

  /* destroy lclt */
  lct_destroy(lclt);
  if (body_buffer)
      SdmMemoryRepository::_ODel(body_buffer, Sdm_MM_AnonymousAllocation);

  return;
}
