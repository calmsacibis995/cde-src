/*
 *+SNOTICE
 *
 *	$Revision: 1.1 $
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
#pragma ident "@(#)MailSession.C	1.107 02/07/97"
#endif

#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>

#include <X11/Intrinsic.h>
#include <Dt/Wsm.h>
#include <DtMail/DtMailXtProc.h>

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Session.hh>
#include <SDtMail/SessionLink.hh>
#include <SDtMail/MessageStore.hh>
#include <SDtMail/Message.hh>
#include <SDtMail/SystemUtility.hh>
#include <MailSession.hh>
#include <RoamApp.h>
#include "MemUtils.hh"
#include "RoamMenuWindow.h"
#include "PasswordDialogManager.h"
#include "RoamCmds.h"
#include "MailMsg.h"

extern forceUpdate (Widget);
SdmSession * MailSession::_app_isession = NULL;
SdmSession * MailSession::_app_osession = NULL;

MailSession::MailSession(SdmError & error,
			 XtAppContext context,
			 SdmSessionType sessionType)
: _open_mailboxes(128)
{
	SdmSession* ses;

	error.Reset();

	SdmConnection* mcon = theRoamApp.connection()->connection();
	assert (mcon != NULL);

	if ( sessionType == Sdm_ST_InSession ) {
	    mcon->SdmSessionFactory(error, ses, Sdm_ST_InSession);
	    if (error) return;
	    ses->StartUp(error);
	    if (error) return;
	    _app_isession = (SdmSession*) ses;
	}
	else if (sessionType == Sdm_ST_OutSession) {
	    mcon->SdmSessionFactory(error, ses, Sdm_ST_OutSession);
	    if (error) return;
	    ses->StartUp(error);
	    if (error) return;
	    _app_osession = (SdmSession*) ses;
	}

	if ( sessionType == Sdm_ST_InSession ) {
	    SdmXtAttachSession(error, context, *_app_isession);
	    if (error) return;
	} else {
	    SdmXtAttachSession(error, context, *_app_osession);
	    if (error) return;
	}

// Pop - looks like nobody is using it    DtMailDamageContext = context;
}

MailSession::~MailSession(void)
{
	for (int mb = 0; mb < _open_mailboxes.length(); mb++) {
		delete _open_mailboxes[mb]->handle;
		free (_open_mailboxes[mb]->path);
		free (_open_mailboxes[mb]->hostname);
	}

	delete _app_isession;
	delete _app_osession;
}

SdmBoolean
MailSession::isMboxOpen( const char *path, const char *server )
{
    int pos = locate(path, server);

    if (pos >= 0)
	return (Sdm_True);
    else
	return (Sdm_False);
}

RoamMenuWindow *
MailSession::getRMW( SdmMessageStore *mbox)
{
	int slot = locate(mbox);

	return(_open_mailboxes[slot]->rmw);
}

RoamMenuWindow *
MailSession::getRMW( const char *path, const char *host)
{
    int index = locate(path, host);
    
    return(_open_mailboxes[index]->rmw);
}

SdmBoolean
MailSession::getMailboxName( RoamMenuWindow *rmw, SdmString& path, SdmString& server)
{
	int slot = locate(rmw);
	if (slot < 0) {
    return Sdm_False;
  } else {
    path = _open_mailboxes[slot]->path;
    if (_open_mailboxes[slot]->hostname) {
      server = _open_mailboxes[slot]->hostname;
    }
    return Sdm_True;
  }
}

void
MailSession::updateMailboxCB(SdmError & error,
			     void *clientdata,
			     SdmServiceFunctionType type,
			     SdmEvent* event)
{
    RoamMenuWindow *rmw = (RoamMenuWindow *)clientdata;
    if (rmw != NULL) {
      switch(type) {
        case Sdm_SVF_CheckNewMail:
          if (event->sdmCheckNewMail.isStart) {
              theRoamApp.setBusyState(error, DtMailBusyState_NewMail,
                    &theRoamApp);
          }
          else {
            if (event->sdmCheckNewMail.numMessages > 0) {
              rmw->list()->load_new_headers(error, event->sdmCheckNewMail.numMessages);
              rmw->newMailIndicators();
            }
              theRoamApp.setBusyState(error, DtMailBusyState_NotBusy,
                    &theRoamApp);
              if (error != Sdm_EC_Success && rmw) {
                rmw->get_editor()->attachArea()->bottomStatusMessage(
                    catgets(DT_catd, 16, 5, "Check for new mail failed."));
                // Force the display of the status message
                XmUpdateDisplay(rmw->baseWidget());
              }
          }
          break;

        case Sdm_SVF_AutoSave:
          if (event->sdmAutoSave.isStart) {
              theRoamApp.setBusyState(error, DtMailBusyState_AutoSave,
                    &theRoamApp);
          } else {
              theRoamApp.setBusyState(error, DtMailBusyState_NotBusy,
                    &theRoamApp);
              if (error != Sdm_EC_Success && rmw) {
                rmw->get_editor()->attachArea()->bottomStatusMessage(
                    catgets(DT_catd, 16, 4, "Auto-save failed."));
                // Force the display of the status message
                XmUpdateDisplay(rmw->baseWidget());
              }
          }
          break;
      }
    }
}


void MailSession::CloseMailboxCallback ( void* clientData )
{
  RoamMenuWindow* rmw = (RoamMenuWindow*) clientData;
  
  // first, make sure roam menu window is still there.
  int slot = theRoamApp.isession()->locate(rmw);
  if (slot < 0) { 
    slot = theRoamApp.osession()->locate(rmw);
  }
  
  if (slot >= 0) {
    rmw->quit();
  }
}

void MailSession::SetReadOnlyMailboxCallback ( void* clientData )
{
  // Pressed ok
  RoamMenuWindow* rmw = (RoamMenuWindow*) clientData;
  MailSession *ses = theRoamApp.isession();
  SdmString path, server;
  SdmBoolean found;
  SdmError error;

  // first, let's get the name of the mailbox for this roam menu window.
  found = theRoamApp.isession()->getMailboxName(rmw, path, server);
  if (found == Sdm_False) { 
    found = theRoamApp.osession()->getMailboxName(rmw, path, server);
  }
  
  if (found) {
    rmw->setupReadOnlyMailbox(Sdm_False);
  }
}



void MailSession::CloseAndReopenMailboxCallback ( void* clientData )
{
  // Pressed ok
  RoamMenuWindow* rmw = (RoamMenuWindow*) clientData;
  MailSession *ses = theRoamApp.isession();
  SdmString path, server;
  SdmBoolean found;
  SdmError error;

  // first, let's get the name of the mailbox for this roam menu window.
  found = theRoamApp.isession()->getMailboxName(rmw, path, server);
  if (found == Sdm_False) { 
    found = theRoamApp.osession()->getMailboxName(rmw, path, server);
  }
  
  if (found) {
    // we fist unmanage the original window.  DO NOT QUIT HERE because this
    // could cause dtmail to exit if it's the only window displayed.
    rmw->unmanage();

    Atom *workspaces = NULL;
    unsigned long numWorkspaces = 0;

    DtWsmGetWorkspacesOccupied(XtDisplay(rmw->baseWidget()),
                               XtWindow(rmw->baseWidget()),
                               &workspaces,
                               &numWorkspaces);

    SdmToken *token = new SdmToken;
    token->SetValue("servicechannel", "cclient");
    token->SetValue("serviceclass", "messagestore");
    token->SetValue("serviceoption", "messagestorename", (const char*)path);

    // if this is a remote mailbox, bring up the login dialog.
    if (server.Length() > 0) {
      thePasswordDialogManager->setImapServer(strdup((const char*)server));

      // if cancelled out of login dialog, just close the original roam
      // menu window and return.
      if (!thePasswordDialogManager->loginDialog()) {
        rmw->quit();
        return;
      }

      // set up token for remote login.
      token->SetValue("servicetype", "imap");
      token->SetValue("serviceoption", "hostname", thePasswordDialogManager->imapserver());
      token->SetValue("serviceoption", "username", thePasswordDialogManager->user());
      token->SetValue("serviceoption", "password", thePasswordDialogManager->password());

    } else {
      // set up token for local login.
      token->SetValue("servicetype", "local");
    } 

    SdmMailRc *mailRc;        
    theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailRc);
    assert(!error);
    if (mailRc->IsValueDefined("dontsavemimemessageformat"))
      token->SetValue("serviceoption", "preservev3messages", "x");  

    // Don't need to close mailbox.  It will close when the rmw->quit
    // call is done.  The new mailbox will not be opened until 
    // we have returned from this function.  Even though we
    // have created the new roam menu window and initialized it 
    // below, a separate event is used to handle the open of the
    // mailbox.  This event will not be handled until after we return.
    // By that time, the quit would have completed, the mailbox would
    // be closed, and the entry for the original mailbox would have
    // been removed from the mail session list of opened mailboxes.

    RoamMenuWindow *mailview = new RoamMenuWindow(path, NULL, token, Sdm_False);
    mailview->initialize(0, NULL);

    if (numWorkspaces > 0 && workspaces != NULL) {
      DtWsmSetWorkspacesOccupied(XtDisplay(mailview->baseWidget()),
                               XtWindow(mailview->baseWidget()),
                               workspaces,
                               numWorkspaces);
    }

    if (workspaces) {
      XtFree((char*)workspaces);
    }

    // it's ok to quit here now that the new wsindow is up.
    rmw->quit();
  }
}


void
MailSession::lockActivityCB(SdmError & error,
			     void *clientdata,
			     SdmServiceFunctionType type,
			     SdmEvent* event)
{
  RoamMenuWindow *rmw = (RoamMenuWindow *)clientdata;
  MailSession *ses = theRoamApp.isession();
  error = Sdm_EC_Success;

  if (rmw && type == Sdm_SVF_DataPortLockActivity && 
      event->sdmDataPortLockActivity.type == Sdm_Event_DataPortLockActivity) 
  {
    DtMailGenDialog *genDialog = rmw->genDialog();
    SdmString path, server;
    SdmBoolean found;

    // first, let's get the name of the mailbox for this roam menu window.
    found = theRoamApp.isession()->getMailboxName(rmw, path, server);
    if (found == Sdm_False) { 
      found = theRoamApp.osession()->getMailboxName(rmw, path, server);
    }
    if (found == Sdm_True) {

      char buf[2048];
      char *helpId;

      switch(event->sdmDataPortLockActivity.lockActivityEvent) 
      {
        case SdmDataPortLockActivity::Sdm_DPLA_ServerConnectionBroken:
        {
          helpId = DTMAILHELPLOSTIMAPSERVERCONNECTION;
          sprintf(buf, catgets(DT_catd, 16, 10, "The IMAP server connection does not exist for %s on %s."), 
            (const char*) path, (const char*) server);
          break;
        }
        case SdmDataPortLockActivity::Sdm_DPLA_SessionLockGoingAway:
        case SdmDataPortLockActivity::Sdm_DPLA_SessionLockTakenAway:
        {
          helpId = DTMAILHELPLOSTEXCLUSIVEACCESSTOMAILBOX;
          sprintf(buf,
            catgets(DT_catd, 16, 9, "You have lost exclusive access to this mailbox."));
          break;
        }
        
        case SdmDataPortLockActivity::Sdm_DPLA_MailboxConnectionReadOnly:
        {
          // we must set the writable flag for the roam menu window to false
          // right here.  if user decides to close and reopen the mailbox,
          // we don't want them to be prompted to save their changes.
          //
          rmw->setWritable(Sdm_False);
          genDialog->setToWarningDialog(catgets(DT_catd, 3, 48, "Mailer"),
				 catgets(DT_catd, 3, 148, "This mailbox is now open in another Mailer session.\nYour mailbox is now read-only."));
          genDialog->post(rmw,
                  catgets(DT_catd, 3, 149, "Keep Read-Only"),
                  catgets(DT_catd, 16, 11, "Close and Reopen"),
                  DTMAILHELPMAILBOXCONNECTIONREADONLY, 
                  &SetReadOnlyMailboxCallback,
                  &CloseAndReopenMailboxCallback);        
          return;
        }

        case SdmDataPortLockActivity::Sdm_DPLA_MailboxChangedByOtherUser:
        {
          helpId = DTMAILHELPERROR;
          sprintf(buf,
            catgets(DT_catd, 16, 14, "This mailbox has been changed by another instance of Mailer or another\nmailer, or by another program or process. \n\nSome changes made to this mailbox in this Mailer session may be lost.\nFor instance, if you marked some messages for deletion and did not\nclick Destroy Deleted Messages, and if auto-save did not run on this\nMailer session before the mailbox was changed outside of this Mailer\nsession, those messages will not be deleted.\n\nClick Close to close the mailbox or Close and Reopen to close the\nmailbox and reopen it."));
          break;
        }
         
        default:
          return;
      }  // end switch

      rmw->setServerDisconnect(Sdm_True);

      genDialog->setToQuestionDialog(catgets(DT_catd, 3, 48, "Mailer"), buf);
      genDialog->post(rmw,
                  catgets(DT_catd, 16, 11, "Close and Reopen"),
                  catgets(DT_catd, 16, 12, "Close"),
                  helpId, 
                  &CloseAndReopenMailboxCallback,
                  &CloseMailboxCallback);
    }
  }  
}

// IMPORTANT NOTE.  Right now the following "logging" service functions are
// called directly by libSDtMail which also ensures that any given service
// function is called serially.  These functions will only be called while
// the front-end "event-thread" is processing a synchronous call such as
// "Open", so there's still only a single thread that enters Xt/Motif.
// However, when the front-end switches to use the asynchronous Opens, the
// mid-end MUST be modified to use the original pipe/XtAppAddInput mechanism
// in order to avoid multiple threads entering non MT-safe X libraries.
void
MailSession::logMessageServiceCB(
		SdmError & /*error*/,
		void *clientdata,
		SdmServiceFunctionType type,
		SdmEvent* event)
{
  RoamMenuWindow *rmw = (RoamMenuWindow*)clientdata;
  SdmMailRc *mailRc;        
  SdmError localError;
  const char *p;

  theRoamApp.connection()->connection()->SdmMailRcFactory(localError, mailRc);

  switch (type) {
    // Error log messages are now only generated by the mm_fatal c-client
    // callback.
  case Sdm_SVF_ErrorLog:
    p = event->sdmErrorLog.errorMessage ?
      (const char *)*event->sdmErrorLog.errorMessage : NULL;

    if (p && *p) {
      if (mailRc->IsValueDefined("logerror"))
        printf("logerror: %s\n", p);

      if (rmw) {
        rmw->genDialog()->setToErrorDialog(catgets(DT_catd, 2, 21, "Mailer"),
                                           (char *)p);
        rmw->genDialog()->post_and_return(catgets(DT_catd, 3, 76, "OK"),
                                          DTMAILHELPERROR);
      }
    }
    if (event->sdmErrorLog.errorMessage)
      delete event->sdmErrorLog.errorMessage;

    break;
  case Sdm_SVF_DebugLog:
    p = event->sdmDebugLog.debugMessage ?
      (const char *)*event->sdmDebugLog.debugMessage : NULL;

    if (p && mailRc->IsValueDefined("logdebug"))
      printf("logdebug: %s\n", p);

    if (event->sdmDebugLog.debugMessage)
      delete event->sdmDebugLog.debugMessage;

    break;
  case Sdm_SVF_NotifyLog:
    p = event->sdmNotifyLog.notifyMessage ?
      (const char *)*event->sdmNotifyLog.notifyMessage : NULL;

    if (p && *p) {
      if (mailRc->IsValueDefined("lognotify"))
        printf("lognotify: %s\n", p);

      if (rmw) {
        rmw->get_editor()->attachArea()->bottomStatusMessage((char*)p);
        // Force the display of the status message
        XmUpdateDisplay(rmw->baseWidget());
      }
    }
    if (event->sdmNotifyLog.notifyMessage)
      delete event->sdmNotifyLog.notifyMessage;

    break;
  }
}


int
MailSession::getOpenCount()
{
    return(_open_mailboxes.length());
}

#ifdef OPEN_ASYNC

SdmMessageStore *
MailSession::openAsync(SdmError &error,
                       const SdmToken &token,
                       void * client_data
)
{
    SdmString path, hostname;
    int slot;
    OpenContext *ctx = new OpenContext;
    SdmMessageStore *mailbox;

    token.FetchValue(path, "serviceoption", "messagestorename");
    token.FetchValue(hostname, "serviceoption", "hostname");
    
    slot = locate(path, hostname);
    if (slot >= 0) {
      _open_mailboxes[slot]->open_ref_count += 1;
      return(_open_mailboxes[slot]->handle);
    }

    _app_isession->SdmMessageStoreFactory(error, mailbox);
    if (error) {
      return NULL;
    }

    mailbox->StartUp(error);
    if (error) {
      delete mailbox;
      return NULL;
    }

    ctx->session = this;
    ctx->mbox = mailbox;
    ctx->rmw = (RoamMenuWindow *)client_data;
    ctx->path = strdup((const char *)path);
    ctx->hostname = strdup((const char *)hostname);

    SdmServiceFunction open_async_svf((const SdmCallback)
				     &MailSession::openAsyncCB, 
				     (void *)ctx, Sdm_SVF_SolicitedEvent);

    mailbox->Open_Async(error, open_async_svf, (void *)ctx, token);
    if (error) {
      delete mailbox;
      return NULL;
    }
    return(mailbox);
}

#endif

SdmMessageStore *
MailSession::open(SdmError &error, 
                  SdmMessageNumber &nmsgs,
                  SdmBoolean &readOnly,
                  const SdmToken &token,
                  void *client_data
)
{
  SdmString path, hostname;
  char *str;
  DtMailGenDialog *genDialog = theRoamApp.genDialog();
  
  // First, see if we have this open in this process.
  token.FetchValue(path, "serviceoption", "messagestorename");
  token.FetchValue(hostname, "serviceoption", "hostname");
  int slot = locate(path, hostname);
  if (slot >= 0) {
    _open_mailboxes[slot]->open_ref_count += 1;
    return(_open_mailboxes[slot]->handle);
  }

  // Create a handle for determining what to do next. This will add us to
  // the file session so we will start getting call back requests on this
  // file.
  SdmMessageStore * mailbox;
  _app_isession->SdmMessageStoreFactory(error, mailbox);
  if (error) {
    return NULL;
  }

  mailbox->StartUp(error);
  if (error) {
    delete mailbox;
    return NULL;
  }

  registerServiceFunctions(error, mailbox, client_data);
  mailbox->Open(error, nmsgs, readOnly, token);
    
  checkForTooltalkLockingError(error, nmsgs, readOnly, token, mailbox);
  if (mailbox == NULL) {
    return mailbox;
  }    

  // Don't assert if the open fails.  the caller of this
  // open should be checking the error returned by the
  // method and handling it appropriately.
  //
  // if an error occurred, delete the mailbox.  this
  // will cause the mailbox to shutdown.  
  // note: don't check for the file if we couldn't open the
  // mailbox because we couldn't login to the server or we
  // couldn't connect to the server.
  //
  if (error && 
      !error.IsErrorContained(Sdm_EC_CCL_CantLoginToIMAPServer) &&
      !error.IsErrorContained(Sdm_EC_CannotConnectToIMAPServer)) 
  {
    SdmError localError;
    if (mailbox->Attach(localError, token) != Sdm_EC_Success) {
      delete mailbox;
      return NULL;
    }
    
    if (token.CheckValue("servicetype", "imap")) {
      // See if the file exists.
      SdmString reference(path);
      SdmString pattern("");
      SdmSortIntStrL dirInfo;
      mailbox->ScanNamespace(localError, dirInfo, reference, pattern);
      if (localError == Sdm_EC_Success && (dirInfo.ElementCount() == 0 ||
	(dirInfo.ElementCount() == 1 && *dirInfo[0].GetString() == NULL))) {
        localError = Sdm_EC_Fail;
      }     
    } else {
      if (SdmSystemUtility::SafeAccess(path, F_OK) != 0) {
        localError = Sdm_EC_Fail;
      }
    }

    if (localError != Sdm_EC_Success) {
      char buf[512];
      str = catgets(DT_catd, 3, 3,
                    "The mailbox %s does not exist.\nCreate a mailbox with this name?");
      sprintf(buf, str, (const char*)path);
      genDialog->setToQuestionDialog(catgets(DT_catd, 3, 48, "Mailer"), buf);
      if (genDialog->post_and_return(catgets(DT_catd,3,5,"OK"),
                                     catgets(DT_catd, 1, 5, "Cancel"), DTMAILHELPERROR) == 1) {
        if (mailbox->Create(error, path) != Sdm_EC_Success) {
          delete mailbox;
          return NULL;
        }
        if (mailbox->Open(error, nmsgs, readOnly, token) != Sdm_EC_Success) {
          checkForTooltalkLockingError(error, nmsgs, readOnly, token, mailbox);
          if (mailbox == NULL) {
            return mailbox;
          }    
        }
      }
      else {
        error.Reset();
        delete mailbox;
        return NULL;
      }

      // found the file; that means that the open failed not because mail file did not exist.
      // clean up local variables before returning.
    } else {  
      if (mailbox) delete mailbox;
      mailbox = NULL;
    }
  }

  if (error == Sdm_EC_Success) {
    // No error; add service functions and add store to message store list.    

    addToList(mailbox, path, hostname, (RoamMenuWindow *)client_data);
  }
    
  return(mailbox);
}


void
MailSession::checkForTooltalkLockingError(SdmError &error, 
                  SdmMessageNumber &nmsgs,
                  SdmBoolean &readOnly,
                  const SdmToken &token,
                  SdmMessageStore *&mailbox)
{
  char *helpId, *str;
  DtMailGenDialog *genDialog = theRoamApp.genDialog();
  SdmToken local_token(token);
  SdmBoolean promptOpenWithoutExclusiveAccess = Sdm_False;

  // Check for tooltalk locking errors first, since these are errors that we
  // can recover from and require input from the user.
  if (error.IsErrorContained(Sdm_EC_CannotObtainMailboxLock)) 
  {
    if (error.IsErrorContained(Sdm_EC_MBL_OtherMailerOwnsWriteAccess)) 
    {
      str = catgets(DT_catd, 3, 82,
                    "Someone else is using this mailbox.\nWould you like to demand exclusive access?");
      genDialog->setToQuestionDialog(catgets(DT_catd, 3, 48, "Mailer"), str);

      // If the user asks to take the lock away from the other Mailer, then
      // try to take the lock away.
      if (genDialog->post_and_return(catgets(DT_catd, 3, 5, "OK"),
                                     catgets(DT_catd, 3, 73, "Cancel"), DTMAILHELPTAKELOCK) == 1) 
      {

        error.Reset();
        local_token.SetValue("serviceoption", "grabsessionlock", "true");
        mailbox->Open(error, nmsgs, readOnly, local_token);

        // If there is still an error, handle it.
        if (error.IsErrorContained(Sdm_EC_CannotObtainMailboxLock)) 
        {
          if (error.IsErrorContained(Sdm_EC_MBL_OtherMailerOwnsWriteAccess)) 
          {
            promptOpenWithoutExclusiveAccess = Sdm_True;
            str = catgets(DT_catd, 3, 83,
			  "Mailer cannot obtain exclusive access to this mailbox.  You can choose to\nto open this mailbox read-only or open it read-write without exclusive access.\n\nCaution: Click Read-Write only if you are sure that neither you nor\nanyone else has this mailbox open in another mail application.\nIf this mailbox is open in another mail application, clicking\nRead-Write can corrupt or destroy your mailbox contents.");
            helpId = DTMAILHELPOPENREADONLY;
          }
        }
      } 
      else   // user canceled out of grabbing lock.
      {
          promptOpenWithoutExclusiveAccess = Sdm_True;
          str = catgets(DT_catd, 3, 83,
			  "Mailer cannot obtain exclusive access to this mailbox.  You can choose to\nto open this mailbox read-only or open it read-write without exclusive access.\n\nCaution: Click Read-Write only if you are sure that neither you nor\nanyone else has this mailbox open in another mail application.\nIf this mailbox is open in another mail application, clicking\nRead-Write can corrupt or destroy your mailbox contents.");
          helpId = DTMAILHELPOPENREADONLY;
      }
    } 
    else if (error.IsErrorContained(Sdm_EC_MBL_TooltalkNotResponding)) 
    {
      promptOpenWithoutExclusiveAccess = Sdm_True;
      str = catgets(DT_catd, 3, 94,
		    "Mailer is unable to obtain exclusive access to this\nmailbox because the system is not responding.\n(The file $HOME/.dt/errorlog may contain additional information).\n\nFor this time only, you can choose to open this mailbox\nread-only, or to open it read-write without exclusive access.\n\nCaution: Click Read-Write only if you are sure that neither you nor \nanyone else has this mailbox open in another mail application.\nIf this mailbox is open in another mail application, clicking \nRead-Write can corrupt or destroy your mailbox contents.");
      helpId = DTMAILHELPOPENREADWRITEOVERRIDE;
    }

    if (promptOpenWithoutExclusiveAccess) 
    {
      // Note: str should be set to the appropriate message.

      genDialog->setToQuestionDialog(catgets(DT_catd, 3, 48, "Mailer"), str);

      // We weren't able to obtain the lock, so give the user the option of
      // opening the mailbox read-only, opening it read-write and ignoring the
      // lock, or cancelling.
      int answer = genDialog->post_and_return(catgets(DT_catd, 3, 95, "Read-only"),
                                              catgets(DT_catd, 3, 73, "Cancel"), 
                                              catgets(DT_catd, 3, 96, "Read-Write"),
                                              helpId);

      if (answer == 1) { // Read only
        error.Reset();
        local_token.ClearValue("serviceoption", "grabsessionlock");
        local_token.SetValue("serviceoption", "readonly", "true");
        local_token.SetValue("serviceoption", "ignoresessionlock", "true");
        mailbox->Open(error, nmsgs, readOnly, local_token);
      } else if (answer == 3) { // Read/write
        error.Reset();
        local_token.SetValue("serviceoption", "ignoresessionlock", "true");
        local_token.ClearValue("serviceoption", "grabsessionlock");
        mailbox->Open(error, nmsgs, readOnly, local_token);
      } else { // Cancel
        error.Reset();
        delete mailbox;
        mailbox = NULL;
      }  
    }
  }
}

void
MailSession::registerServiceFunctions(
    SdmError error,
    SdmMessageStore *mailbox,
    void *client_data
)
{
  SdmServiceFunction sf1((const SdmCallback)&MailSession::updateMailboxCB,
                         client_data, Sdm_SVF_CheckNewMail);
  SdmServiceFunction sf2((const SdmCallback)&MailSession::updateMailboxCB,
                         client_data, Sdm_SVF_AutoSave);
  SdmServiceFunction sf3((const SdmCallback)&MailSession::lockActivityCB,
                         client_data, Sdm_SVF_DataPortLockActivity);
  SdmServiceFunction sf4((const SdmCallback)&MailSession::logMessageServiceCB,
                         client_data, Sdm_SVF_ErrorLog);
  SdmServiceFunction sf5((const SdmCallback)&MailSession::logMessageServiceCB,
                         client_data, Sdm_SVF_DebugLog);
  SdmServiceFunction sf6((const SdmCallback)&MailSession::logMessageServiceCB,
                         client_data, Sdm_SVF_NotifyLog);

  mailbox->RegisterServiceFunction(error, sf1);
  mailbox->RegisterServiceFunction(error, sf2);
  mailbox->RegisterServiceFunction(error, sf3);
  mailbox->RegisterServiceFunction(error, sf4);
  mailbox->RegisterServiceFunction(error, sf5);
  mailbox->RegisterServiceFunction(error, sf6);
}

#ifdef OPEN_ASYNC

void
MailSession::finishOpen( OpenContext *ctx )
{
  SdmError error;

  registerServiceFunctions(error, ctx->mbox, ctx->rmw);

  ctx->rmw->finishOpen(ctx->mbox);

  // Free the memory associated with path and hostname since we won't
  // be using it any more.
  delete ctx->path;
  delete ctx->hostname;
  delete ctx;
}


void
MailSession::openAsyncCB(SdmError &,
			 void *client_data,
			 SdmServiceFunctionType type,
			 SdmEvent* event)
{
  OpenContext *ctx = (OpenContext *)client_data;
  DtMailGenDialog *genDialog = theRoamApp.genDialog();

  switch(type) {
  case Sdm_SVF_SolicitedEvent:
    if (*(event->sdmOpenMessageStore.error) != Sdm_EC_Success) {
      // More error reporting needs to be done here.
      //
      genDialog->post_error(*(event->sdmOpenMessageStore.error), DTMAILHELPCANNOTOPENMAILBOX, 
			                  catgets(DT_catd, 15, 26, "Mailer cannot open the mailbox."));

      // The open failed.  Free the memory allocated for path
      // and hostname then delete the messagestore and rmw.
      delete ctx->path;
      delete ctx->hostname;
      delete ctx->mbox;
      delete ctx->rmw;
      delete ctx;
    } else {
	  if (theRoamApp.isRegistered(ctx->rmw))
        ctx->session->finishOpen(ctx);
	  else {
        return;
	  }
    }
    break;
  }
}

#endif

void
MailSession::close(SdmError&, const SdmMessageStore * mb)
{
	// Find the mail box slot.
	int slot = locate(mb);
	if (slot < 0) {
    // couldn't find the store in this session, just return.
		return;
	}

	MailBoxItem * mbi = _open_mailboxes[slot];
	mbi->open_ref_count -= 1;
	if (mbi->open_ref_count <= 0) {
		delete mbi->handle;
		free (mbi->path);
		free (mbi->hostname);
		_open_mailboxes.remove(slot);
		delete mbi;
	}
}

void
MailSession::copy(
    SdmError & error,
    SdmMessage &,		// msg
    const char *		// path
)
{
	error.Reset();
}

int
MailSession::locate(const char * path, const char *hostname)
{
    if (!hostname)
	hostname = "";

    for (int slot = 0; slot < _open_mailboxes.length(); slot++) {
	if (strcmp(_open_mailboxes[slot]->path, path) == 0 &&
	    strcmp(_open_mailboxes[slot]->hostname, hostname) == 0) {
	    return(slot);
	}
    }

    return(-1);
}

int
MailSession::locate(const SdmMessageStore * mb)
{
	for (int slot = 0; slot < _open_mailboxes.length(); slot++) {
		if (mb == _open_mailboxes[slot]->handle) {
			return(slot);
		}
	}

	return(-1);
}

int
MailSession::locate(const RoamMenuWindow * rmw)
{
	for (int slot = 0; slot < _open_mailboxes.length(); slot++) {
		if (rmw == _open_mailboxes[slot]->rmw) {
			return(slot);
		}
	}

	return(-1);
}
void
MailSession::addToList(
    SdmMessageStore * mb,
    const char * path,
    const char * hostname,
    RoamMenuWindow *obj
)
{
	MailBoxItem * mbi = new MailBoxItem;

	mbi->handle = mb;
	mbi->path = strdup(path);
	mbi->hostname = strdup(hostname);
	mbi->open_ref_count = 1;
	mbi->rmw = obj;

	_open_mailboxes.append(mbi);
}

Tt_status
MailSession::lockCB(
    Tt_message,
    Tttk_op,
    const char *,
    uid_t,
    int,
    void *
)
{
	// We never give up the lock during a copy!
	return(TT_ERR_INVALID);
}
