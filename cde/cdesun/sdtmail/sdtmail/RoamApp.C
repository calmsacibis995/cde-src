/*
 *+SNOTICE
 *
 *	$Revision: 1.262 $
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
 *	Copyright 1993,1994,1995,1996 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)RoamApp.C	1.262 07/07/97"
#endif

#include <stream.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>
#include <errno.h>

#ifdef DTMAIL_TOOLTALK
#include <Tt/tt_c.h>
#include <Tt/tttk.h>
#include "SendMsgDialog.h"
#endif

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/MailRc.hh>
#include <SDtMail/String.hh>
#include <SDtMail/Vector.hh>
#include <SDtMail/Utility.hh>
#include <SDtMail/SystemUtility.hh>
#include <SDtMail/MessageStore.hh>
#include <SDtMail/Message.hh>
#include <SDtMail/DeliveryResponse.hh>
#include <SDtMail/MessageEnvelope.hh>
#include <SDtMail/Session.hh>

#include "RoamApp.h"
#include "RoamMenuWindow.h"
#include "RoamCmds.h"
#include "WorkingDialogManager.h"
#include "PasswordDialogManager.h"
#include <DtMail/PropSource.hh>
#include <DtMail/OptCmd.h>
//#include "DtMailGenDialog.hh"

#include "EUSDebug.hh"
#include "MemUtils.hh"
#include "MailSession.hh"
#include "MailMsg.h"
#include "DtMailHelp.hh"
#include "ComposeCmds.hh"
#include "Fonts.h"

#include <Xm/MessageB.h>
//#include <DtMail/DtMailSigChld.h>
#include <signal.h>
#include <ctype.h>
#include <locale.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <stdlib.h>
#include <Dt/Dt.h>
#include <Dt/Dts.h>
#include <Dt/Action.h>
#include <Dt/EnvControl.h>

static const int        READ = 0;
static const int        WRITE = 1;
 
void
force( Widget w)
{
  Widget	shell;
  Display	*dpy;
  XWindowAttributes	xwa;
  Window	window;
  XtAppContext cxt=XtWidgetToApplicationContext( w );
  XEvent		event;

  shell=w;
  dpy=XtDisplay(shell);
  window=XtWindow( shell );

  while ( XGetWindowAttributes(dpy,window,&xwa)) {
    if ( XGetWindowAttributes( dpy, window, &xwa ) &&
	 xwa.map_state != IsViewable )
      break;

    XtAppNextEvent( cxt, &event );
    XtDispatchEvent( &event );
  }
  XmUpdateDisplay(shell);
}

void
forceUpdate( Widget w )
{
  Widget diashell, topshell;
  Window diawindow, topwindow;
  XtAppContext cxt = XtWidgetToApplicationContext( w );
  Display		*dpy;
  XWindowAttributes	xwa;
  XEvent		event;

  for (diashell=w;!XtIsShell(diashell);diashell=XtParent(diashell));
  for ( topshell=diashell;XtIsTopLevelShell( topshell );
	topshell = XtParent( topshell ) );

  dpy=XtDisplay(diashell);
  diawindow=XtWindow( diashell );
  topwindow=XtWindow(topshell);
  while ( XGetWindowAttributes(dpy,diawindow,&xwa)  
          && XEventsQueued( dpy, QueuedAlready) ) {
      
      XtAppNextEvent( cxt, &event );
      XtDispatchEvent( &event );
    }
  XmUpdateDisplay(topshell);
}

XtResource
RoamApp::_resources[] = {
  {
    "printscript",
    "PrintScript",
    XtRString,
    sizeof( XtRString ),
    XtOffset ( RoamApp *, _print_script ),
    XtRString,
    ( XtPointer ) "lp",
  },
  {
    "mailfiles",
    "MailFiles",
    XtRString,
    sizeof( XtRString ),
    XtOffset ( RoamApp *, _mailfiles_folder ),
    XtRString,
    ( XtPointer ) ".",
  },
  {
    "defaultmailbox",
    "DefaultMailBox",
    XtRString,
    sizeof( XtRString ),
    XtOffset ( RoamApp *, _default_mailbox ),
    XtRString,
    ( XtPointer ) ".",
  },

  // Fixed width font
  {
    "userFont",
    "UserFont",
    XtRString,
    sizeof( XtRString ),
    XtOffset ( RoamApp *, _user_font ),
    XtRString,
    ( XtPointer )"-b&h-lucidatypewriter-medium-r-*-sans-*-120-*-*-*-*-*-*",
  },

  // Fixed width fontlist
  {
    "userFont",
    "UserFont",
    XmRFontList,
    sizeof( XmFontList ),
    XtOffset ( RoamApp *, _user_fontlist ),
    XtRString,
    ( XtPointer )"-b&h-lucidatypewriter-medium-r-*-sans-*-120-*-*-*-*-*-*",
  },

  // Variable width font
  {
    "systemFont",
    "SystemFont",
    XtRString,
    sizeof( XtRString ),
    XtOffset ( RoamApp *, _system_font ),
    XtRString,
    ( XtPointer )"-b&h-lucida-medium-r-*-sans-*-120-*-*-*-*-*-*"
  },

  // Variable width fontlist
  {
    "systemFont",
    "SystemFont",
    XmRFontList,
    sizeof( XmFontList ),
    XtOffset ( RoamApp *, _system_fontlist ),
    XtRString,
    ( XtPointer )"-b&h-lucida-medium-r-*-sans-*-120-*-*-*-*-*-*"
  },

  // Font used for attachment glyph
  {
    "glyphFont",
    "GlyphFont",
    XtRString,
    sizeof( XtRString ),
    XtOffset ( RoamApp *, _glyph_font ),
    XtRString,
    ( XtPointer )
    NULL,
  },

};

RoamApp theRoamApp("Dtmail");
int just_compose = 0;

// we need to declare the signal handlers two ways because on sparc OS
// with version > 2.4, when __cplusplus is defined, the signature for 
// the handler takes an int.  On sparc OS <= 2.4, the signature for the 
// handler does not take any arguments.
//
void
#if (SunOS < 55)
SigUsr1()
#else
SigUsr1(int /* theSignal */)
#endif
{
  theRoamApp.writeToSignalFileDesc(1);
}

void
#if (SunOS < 55)
SigUsr2()
#else
SigUsr2(int /* theSignal */)
#endif
{
  theRoamApp.writeToSignalFileDesc(2);
}

//
// This routine goes through the list of current windows,
// finds all the roam menu windows, and quits them.
// If any compose winodows are open, then are not closed.
//
void
RoamApp::closeAllMailboxWindows(void)
{
    const char *cname;
    int win;
    SdmVector <RoamMenuWindow*> windowsToClose;
    RoamMenuWindow *rmw;

    // first, go through all the roam menu windows.
    // we put them in a separate list.
    //
    for (win = 0; win < _numWindows; win++) {
      cname = _windows[win]->className();
      if (strcmp(cname, "RoamMenuWindow") == 0) {
        rmw = (RoamMenuWindow*) _windows[win];
        windowsToClose.AddElementToList(rmw);
      }
    }

    // go through the list of windows we need to close
    // and close the roam menu window itself.
    for (win = 0; win < windowsToClose.ElementCount(); win++) {
      rmw = windowsToClose[win];
      rmw->quit();
    }
}


//
// This routine goes through the list of current windows, finds all
// the roam menu windows, and closes the message stores.
//
void
RoamApp::closeAllMessageStores(void)
{
    int win;

    for (win = 0; win < _numWindows; win++) {
      const char *cname = _windows[win]->className();

      if (strcmp(cname, "RoamMenuWindow") == 0) {
        RoamMenuWindow *rmw = (RoamMenuWindow*) _windows[win];
        SdmMessageStore *mailbox = rmw->mailbox();

        if (mailbox != NULL) {
          SdmError error;
          error.Reset();
          mailbox->Close(error);
        }
      }
    }
}


void
RoamApp::closeLocalNonInboxWindows(void)
{
    const char *cname;
    int win;
    SdmVector <RoamMenuWindow*> windowsToClose;
    RoamMenuWindow *rmw;

    // first, go through all the list of windows and find
    // all the non-local mailboxes (except the inbox).
    // we put them in a separate list.
    //
    for (win = 0; win < _numWindows; win++) {
      cname = _windows[win]->className();
      if (strcmp(cname, "RoamMenuWindow") == 0) {
        rmw = (RoamMenuWindow*) _windows[win];
        if (rmw->isLocal() && !rmw->inbox()) {
          windowsToClose.AddElementToList(rmw);
        }
      }
    }

    if (windowsToClose.ElementCount() > 0) {
      // next, go through list of windows and close the message
      // stores.  we need to do this right away because 
      // we don't want to wait until the user responds to the
      // dialog.
      SdmError mail_error;
      for (win = 0; win < windowsToClose.ElementCount(); win++) {
        mail_error.Reset();
        rmw = windowsToClose[win];
        rmw->mailbox()->Close(mail_error);
      }

      // post dialog to let the user know we are closing all the
      // local mailbox windows.
      char buf[2048];
      char *helpId;
      
      // need tag for help id below.
      helpId = DTMAILHELPLOSTCCLIENTLOCKONMAILBOX;

      sprintf(buf, 
        catgets(DT_catd, 16, 13, 
        "Your mailbox lock no longer exists.  Mailer will close all your\nlocal mailboxes, except your Inbox, to avoid data corruption."));
      _dialog->setToErrorDialog(catgets(DT_catd, 3, 54, "Mailer"), buf);
      _dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), helpId);
          
      // finally, go through the list of windows we need to close
      // and close the roam menu window itself.
      for (win = 0; win < windowsToClose.ElementCount(); win++) {
        rmw = windowsToClose[win];
        rmw->quit();
      }
    }
}

void
RoamApp::handleSignal(int theSignal)
{
    switch (theSignal) 
    {
      case SIGUSR1:
        theRoamApp.setKilledBySignal(Sdm_True);
        theRoamApp.closeAllMailboxWindows();
        break;
         
      case SIGUSR2:
        theRoamApp.setKilledBySignal(Sdm_True);
        theRoamApp.closeLocalNonInboxWindows();
        // we reset the killed_by_signal flag because there
        // may still be some windows opened. 
        theRoamApp.setKilledBySignal(Sdm_False);
        break;
         
      default:
	SdmUtility::LogError(Sdm_False, "dtmail received unexpected signal: %d\n",
			     theSignal);
        break;
    }
}

void 
RoamApp::writeToSignalFileDesc(char buf)
{
  if (SdmSystemUtility::SafeWrite(_signalFileDesc[WRITE], &buf, 1) < 0) {
     SdmUtility::LogError(Sdm_True, 
        "writeToSignalFileDesc: can not write to pipe. errno=%d\n", errno);
     assert (0);
     abort();
  }
}
#ifdef DTMAIL_TOOLTALK

int started_by_tt = 0;
// Move this to constructor of RoamMenuWindow???
int dtmail_mapped = 0;    // For explanation, look in RoamApp.h.
static int roam_tt_fd = 0;
char *roam_tt_procid = NULL;

//  Report ToolTalk error
int dieFromTtError(char *, Tt_status errid)
{
    /* Do not die on warnings or TT_OK */

    if ( tt_is_err(errid) ) {
// 	fprintf(stderr, catgets(DT_catd, 2, 3, "%s returned ToolTalk error: %s\n"), 
// 		procname, tt_status_message(errid));
	ttdt_close(0, 0, 1);
	return 1;   // Returns 1 indicating that error occurred.
    }
	return 0;
}

Tt_message
attachmt_msg_handler(
    Tt_message msg,
    void *client_data,
    Tttk_op op,
    Tt_status diag,
    unsigned char *contents,
    int len,
    char *file,
    char *docname 
)
{
   static const char *thisFcn = "attachmt_msg_handler()";
   Tt_status status = TT_OK;
   SendMsgDialog *compose;

   // For security reasons, we will fail the request if either the effective
   // user id or effective group id of the tt message doesn't match the euid/egid
   // of the dtmail process.
   if ((tt_message_uid(msg) != geteuid()) || (tt_message_gid(msg) != getegid())) {
     tt_message_status_set(msg, TT_DESKTOP_EACCES);
     tt_message_fail(msg);
     return msg;
   }

   if ( diag != TT_OK ) {
      // Toolkit detected an error
      // Let toolkit handle error
      return msg;
   }

   ttdt_message_accept(msg, NULL, (Widget)0, client_data, 1, 1);

   if ( op == TTME_MAIL_EDIT ) { 
//       dieFromTtError("tt_message_reply", status);
       // Put the data that is coming in (via buffer or file) as
       // the attachment of a message.
       if ( len > 0 ) {    // Pass by content
	   compose = theCompose.getWin(msg);
	   compose->inclAsAttmt(contents, len, docname);
       } else if ( file != NULL ) {    // Pass by filename
	   compose = theCompose.getWin(msg);
	   compose->inclAsAttmt(file, docname);
       } else {   // DO NOTHING.
	   // This is not an entry point to bring up a blank compose window
	   // because ttMediaLoadPatCB() returns with diagnosis =
	   // TT_DESKTOP_ENODATA. For INOUT mode, it expects file or buffer.
       }
   } else {
       tttk_message_fail( msg, TT_DESKTOP_ENODATA, 0, 1 );
   }
//
// Not sure if these should be freed.
//
//    tt_free( (caddr_t)contents );
//    tt_free( file );
//    tt_free( docname );
   return 0;
}

int X_error_handler(Display *dpy, XErrorEvent *err)
{
  char msg[80];

  XGetErrorText(dpy, err->error_code, msg, 80);
  SdmUtility::LogError(Sdm_True, "X Error: %s\n", msg);

  return 1;
}


// X IO Error.  Usually the effect session exit and the resulting
// X connection disconnect.  Treat the same as a "Retain and Close"
// dtmail exit by saving the Mailbox message store states.

int X_IO_error_handler(Display*)
{
  // First, save the state of all the local message stores that are 
  // currently opened.
  theRoamApp.closeAllMessageStores();


  // Shut down the application.  
  //
  // Note #1: Calling the shutdown method in RoamApp is a cleaner 
  // way to shutdown.  It will close the SdmSession and clean up 
  // the outgoing store before exiting the program.
  //
  // Note #2: It is ok to to call shutdown here to exit the 
  // application.  The man page for XSetIOErrorHandler specifies
  // that this handler should not return.
  //
  if (theRoamApp.baseWidget() != NULL) {
    XtRemoveAllCallbacks(theRoamApp.baseWidget(), XmNdestroyCallback);
    theRoamApp.setExitCode(1);
    theRoamApp.shutdown();  // shutdown will call _exit.
  }

  return(1);  // never returns, but statifies function's header prototype
}


// We expect this to be called only in case of memory allocation errors
// GL 2/11/97 - This routine is called when *ANY* Xt fatal error is encountered.
// I have no idea why someone thought that this would only be called because
// of memory allocation errors.
void dtmail_xterr_handler(String msg)
{
  SdmUtility::LogError(Sdm_True, "Xt Error: %s\n", msg);

  if (theRoamApp.baseWidget() != NULL) {
    XtRemoveAllCallbacks(theRoamApp.baseWidget(), XmNdestroyCallback);
    theRoamApp.shutdown();
  }
  exit(3);
}

void dtmail_xtwrn_handler(String msg)
{
  SdmUtility::LogError(Sdm_False, "Xt Warning: %s\n", msg);
  return;
}

void
roam_app_signal_handler(XtPointer calldata, int *pipe, XtInputId* id)
{
  RoamApp *theApp = (RoamApp*)calldata;
  assert (theApp != NULL);
  assert (pipe != NULL);
    
  char bogus_buf;
  ssize_t rc;
  if ((rc = SdmSystemUtility::SafeRead(*pipe, &bogus_buf, 1)) <= 0) {
    // if the error is NOT because the pipe got close, we want to log the error.
    if (!(rc == 0 && errno == 0)) {
      SdmUtility::LogError(Sdm_False, 
        "roam_app_signal_handler: can not read from pipe. errno=%d\n", errno);
    }
    // if an error occurred, we want to close the read file descriptor in the pipe
    // and remove the input source. 
    close (*pipe);
    XtRemoveInput(*id);
    return;
  }
  
  if (bogus_buf == 1) {
    theApp->handleSignal(SIGUSR1);
  } else if (bogus_buf == 2) {
    theApp->handleSignal(SIGUSR2);
  } else {
      SdmUtility::LogError(Sdm_False, 
        "roam_app_signal_handler: received invalid data from pipe: %d\n", bogus_buf);
  }
}

void
RoamApp::checkForUnixFromLine(const char *fileName)
{
    SdmString fromLine;
    int fd, size;
    char buf[128], *fbuf;
    struct stat stat_buf;

    if ( (fd = SdmSystemUtility::SafeOpen(fileName, O_RDWR)) == -1 ) {
	return;
    }
    if (SdmSystemUtility::SafeRead(fd, buf, 5) != 5) {
	return;
    }
    buf[5] = '\0';

    // If the first 5 characters aren't "From " then we need to insert a
    // valid unix From line.
    if (strcmp(buf, "From ") != 0) {
	SdmSystemUtility::SafeFStat(fd, &stat_buf);
	fbuf = (char *)malloc((sizeof(char)*stat_buf.st_size));

	// Reset the file pointer to the beginning of the file.
	SdmSystemUtility::SafeLseek(fd, 0, SEEK_SET);
	if (SdmSystemUtility::SafeRead(fd, fbuf, stat_buf.st_size) != stat_buf.st_size) {
	    return;
	}

	// Create a unix from line
	SdmMessageUtility::GenerateUnixFromLine(fromLine);

	// Reset the file pointer to the beginning of the file
	SdmSystemUtility::SafeLseek(fd, 0, SEEK_SET);

	// Write out our new unix from line
	size = SdmSystemUtility::SafeWrite(fd, (void *)((const char *)fromLine),
					   (size_t)fromLine.Length());
	// Make sure all of our data made it out to the file
	if (size != fromLine.Length()) {
	    SdmSystemUtility::SafeClose(fd);
	    return;
	}
	// Write out the original file contents
	size = SdmSystemUtility::SafeWrite(fd, (void *)fbuf, (size_t)stat_buf.st_size);
	if (size != stat_buf.st_size) {
	    SdmSystemUtility::SafeClose(fd);
	    return;
	}
    }
    SdmSystemUtility::SafeClose(fd);
}


// In most cases we don't reply to the tooltalk message in this function.  That
// is taken care of in the destructor for the MenuWindow object.  It is done this
// way because otherwise ttsession will delete the file or wipe-out the buffer
// they passed us.
Tt_message
tooltalk_msg_handler(Tt_message msg,
		     void *client_data,
		     Tttk_op op,
		     Tt_status diag,
		     unsigned char *contents,
		     int len,
		     char *file,
		     char *docname)
{
    static const char *thisFcn = "tooltalk_msg_handler()";
    Tt_status status = TT_OK;
    SendMsgDialog *compose;

    // For security reasons, we will fail the request if either the effective
    // user id or effective group id of the tt message doesn't match the euid/egid
    // of the dtmail process.
    if ((tt_message_uid(msg) != geteuid()) || (tt_message_gid(msg) != getegid())) {
      tt_message_status_set(msg, TT_DESKTOP_EACCES);
      tt_message_fail(msg);
      return msg;
    }

    if ( diag != TT_OK ) {
	// toolkit detected an error
	// Let toolkit handle error
	return msg;
    }

    XtAppSetErrorHandler(theApplication->appContext(), dtmail_xterr_handler);
    XtAppSetWarningHandler(theApplication->appContext(), dtmail_xtwrn_handler);

    // Need to check the return value of this call.
    //
    ttdt_message_accept(msg, NULL, (Widget)0, client_data, 1, 1);
    if ( op == TTME_MAIL ) {
	char *tmpFile;
	// Send without GUI.
	if ( len > 0 ) {
	    // write the buffer out to a temp file.
	    int fd;
	    tmpFile = strdup("/tmp/mbox.XXXXXX");
 
	    // Create the temporary mailbox file - since this is a
	    // behind the scenes kind of deal and it is e-mail, we use
	    // mode 0600 as opposed to mode 0666 for security reasons.

	    mktemp(tmpFile);
	    if ((fd = SdmSystemUtility::SafeOpen(tmpFile, O_CREAT|O_WRONLY, 0600)) == -1) {
		SdmUtility::LogError(Sdm_False, "tooltalk_msg_handler: Open failed\n");
		tt_message_fail(msg);
		return msg;
	    }
	    SdmSystemUtility::SafeWrite(fd, contents, len);
	    SdmSystemUtility::SafeClose(fd);
	} else if (file) {
	    int fd;
	    char *tmpbuf;
	    tmpFile = strdup("/tmp/mbox.XXXXXX");
 	    mktemp(tmpFile);

	    // We need to create a temp file to open with the c-client because
	    // the c-client modifies the file and the user might not want their
	    // file modified.
	    if ((fd = SdmSystemUtility::SafeOpen(file, O_RDONLY)) == -1) {
		SdmUtility::LogError(Sdm_False, "tooltalk_msg_handler: Open failed\n");
		tt_message_fail(msg);
		return msg;
	    }

            struct stat buf;
            if (SdmSystemUtility::SafeFStat(fd, &buf) < 0) {
		SdmUtility::LogError(Sdm_False, "tooltalk_msg_handler: Stat failed\n");
                close(fd);
		tt_message_fail(msg);
                return msg;
            }

	    tmpbuf = (char *) malloc(buf.st_size);
            if (SdmSystemUtility::SafeRead(fd, tmpbuf, buf.st_size) != buf.st_size) {
		SdmUtility::LogError(Sdm_False, "tooltalk_msg_handler: Read failed\n");
                close(fd);
		tt_message_fail(msg);
                return msg;
            }
	    SdmSystemUtility::SafeClose(fd);

	    // Open the tmp file and write the contents of tmpbuf into it.
 
	    // Create the temporary mailbox file - since this is a
	    // behind the scenes kind of deal and it is e-mail, we use
	    // mode 0600 as opposed to mode 0666 for security reasons.

	    if ((fd = SdmSystemUtility::SafeOpen(tmpFile, O_CREAT|O_WRONLY, 0600)) == -1) {
		SdmUtility::LogError(Sdm_False, "tooltalk_msg_handler: Open failed\n");
		tt_message_fail(msg);
		return msg;
	    }

	    if (SdmSystemUtility::SafeWrite(fd, tmpbuf, buf.st_size) != buf.st_size) {
		SdmUtility::LogError(Sdm_False, "tooltalk_msg_handler: Write failed\n");
		tt_message_fail(msg);
		return msg;
	    }
	    SdmSystemUtility::SafeClose(fd);
	}	    

	theRoamApp.checkForUnixFromLine(tmpFile);

	SdmError err;
	SdmToken token;
	SdmMessageStore *msgStore;
	SdmMessage *msgHandle;
	SdmSession *o_sess = theRoamApp.osession()->osession();

	o_sess->StartUp(err);
	if ((err != Sdm_EC_Success) && (err != Sdm_EC_SessionAlreadyStarted)) {
	    SdmUtility::LogError(Sdm_False, "tooltalk_msg_handler: Session StartUp failed. %s\n",
				 err.ErrorMessage());
	    tt_message_fail(msg);
	    return msg;
	}
	err.Reset();
	if (o_sess->SdmMessageStoreFactory(err, msgStore) != Sdm_EC_Success) {
	    SdmUtility::LogError(Sdm_False, "tooltalk_msg_handler: Message Store Factory failed. %s\n",
				 err.ErrorMessage());
	    tt_message_fail(msg);
	    return msg;
	}
	if (msgStore->StartUp(err) != Sdm_EC_Success) {
	    SdmUtility::LogError(Sdm_False, "tooltalk_msg_handler: Message Store StartUp failed. %s\n",
				 err.ErrorMessage());
	    tt_message_fail(msg);
	    return msg;
	}
	
	token.SetValue("servicechannel", "cclient");
	token.SetValue("serviceclass", "messagestore");
	token.SetValue("servicetype", "local");
	token.SetValue("serviceoption", "readonly", "x");
	token.SetValue("serviceoption", "messagestorename", tmpFile);
	token.SetValue("serviceoption", "nosessionlocking", "true");
	token.SetValue("serviceoption", "ignoresessionlock", "true");
	SdmBoolean ro;
        SdmMessageNumber nmsgs = 0;
	if (msgStore->Open(err, nmsgs, ro, token) != Sdm_EC_Success) {
	    SdmUtility::LogError(Sdm_False, "tooltalk_msg_handler: Message Store Open failed. %s\n",
				 err.ErrorMessage());
	    tt_message_fail(msg);
	    return msg;
	}

	// Get the message handle from the message store
	if (msgStore->SdmMessageFactory(err, msgHandle, 1) != Sdm_EC_Success) {
	    SdmUtility::LogError(Sdm_False, "tooltalk_msg_handler: Message Factory failed. %s\n",
				 err.ErrorMessage());
	    tt_message_fail(msg);
	    return msg;
	}

	// Get the Envelope for the msg
	SdmMessageEnvelope *env;
	if (msgHandle->SdmMessageEnvelopeFactory(err, env) != Sdm_EC_Success) {
	    SdmUtility::LogError(Sdm_False,
			 "tooltalk_msg_handler: MessageEnvelope Factory failed. %s\n",
			 err.ErrorMessage());
	    tt_message_fail(msg);
	    return msg;
	}
	// Get the first bodypart from our message
	SdmMessageBody *bp;
	if (msgHandle->SdmMessageBodyFactory(err, bp, 1) != Sdm_EC_Success) {
	    SdmUtility::LogError(Sdm_False,
			 "tooltalk_msg_handler: MessageBody Factory failed. %s\n",
			 err.ErrorMessage());
	    tt_message_fail(msg);
	    return msg;
	}
	// Get the msg struct for the bodypart
	SdmMessageBodyStructure mbstruct;
	if (bp->GetStructure(err, mbstruct) != Sdm_EC_Success) {
	    SdmUtility::LogError(Sdm_False,
			 "tooltalk_msg_handler: MessageBody GetStructure failed. %s\n",
			 err.ErrorMessage());
	    tt_message_fail(msg);
	    return msg;
	}
	// CHARSET
	SdmStringL charset_list;
	SdmContentBuffer content_buf;
	// If the msg is multipart/mixed, use the first body part in the
	// nested multipart body.
	if ( mbstruct.mbs_type == Sdm_MSTYPE_multipart ) {
	    SdmMessage *newMsg;
	    bp->SdmMessageFactory(err, newMsg);
	    if (!err) {
		newMsg->SdmMessageBodyFactory(err, bp, 1);
		if (!err) {
		    bp->GetContents(err, content_buf);
		    if (!err) {
			content_buf.SetCharacterSet(err, "");
			content_buf.SetDataTypeCharSet(err, "");
			if (!err) {
			    bp->SetContents(err, content_buf);
			}
		    }
		}
	    }
	} else if ((mbstruct.mbs_mime_content_type == "text/plain") &&
		   (mbstruct.mbs_disposition != Sdm_MSDISP_attachment)) {
	    bp->GetContents(err, content_buf);
	    if (!err) {
		// Some explanation for the following 2 lines of code.
		// It turns out that if we don't set these to empty strings
		// it will default to US-ASCII...this is bad.
		content_buf.SetCharacterSet(err, "");
		content_buf.SetDataTypeCharSet(err, "");
		if (!err) {
		    bp->SetContents(err, content_buf);
		}
	    } else {
		printf("can't get content buffer\n");
	    }
	}
	// CHARSET

	SdmDeliveryResponse delivery_response;
	// Should we save this message in the Sent Mail log?
	msgHandle->Submit(err, delivery_response, Sdm_MSFMT_Mime, Sdm_False, NULL);
	if (err) {
	    SdmUtility::LogError(Sdm_False, "tooltalk_msg_handler: Submit failed. %s\n",
				 err.ErrorMessage());
	    tt_message_fail(msg);
	    return msg;
	}
	status = tt_message_reply(msg);
	    
	if (msgStore->Close(err) != Sdm_EC_Success) {
	    SdmUtility::LogError(Sdm_False, "tooltalk_msg_handler: Message Store Close failed. %s\n",
				 err.ErrorMessage());
	    tt_message_fail(msg);
	    return msg;
	}
	SdmSystemUtility::SafeUnlink(tmpFile);
    } else if ( op == TTME_MAIL_COMPOSE ) {
	// Bring up blank compose window.
	status = tt_message_reply(msg);
// 	dieFromTtError("tt_message_reply", status);
	compose = theCompose.getWin(msg);
	if ( docname ) {
	    compose->title(docname);
	}
    } else if ( op == TTME_MAIL_EDIT ) { 
	// Bring up compose window with given data filled in.
// 	status = tt_message_reply(msg);
// 	dieFromTtError("tt_message_reply", status);
	// Parse data (coming in as buffer or file)
	if ( len > 0 ) {    // Pass by content
	    compose = theCompose.getWin(msg);
	    compose->parseNplace((char *)contents, len);
	    if ( docname ) {
		compose->title(docname);
	    }
	} else if ( file != NULL ) {    // Pass by filename
	    compose = theCompose.getWin(msg);
	    compose->parseNplace(file);
	    if ( docname ) {
		compose->title(docname);
	    }
	}
    } else if ( op == TTME_DISPLAY ) {
	// It is the Display message.  
	// Since the compose window can be started independent of RoamMenuWindow,
	// a DtMail process may be around (because of a compose window).  Then if
	// a DtMail process exists (ToolTalk will not start another DtMail process),
	// that process needs to respond to this DISPLAY message. 
	// If for some reason, the view of the specified mail folder is unmapped,
	// then need to map it.  Otherwise, create the view for the specified mail folder.
	// Need to remove self destruct when using an existing DtMail process if that
	// process was started by compose.

// This is the wrong place to reply to this message.  Replying here causes
// the action layer to think that it is ok to remove the file it passed in, but we
// are not done with it yet.  Calling tt_message_reply at the end of this function
// doesn't work either.  We are just not going to reply to this tooltalk message
// (tooltalk folks say this is ok) for the time being.
// 	status = tt_message_reply(msg);
// 	dieFromTtError("tt_message_reply", status);

	if ( theApplication == NULL ) {
#ifdef WM_TT
	    fprintf(stdout, "%s: theApplication is NULL\n", thisFcn);
#endif
	    return 0;
	}

#ifdef WM_TT
	fprintf(stdout, "%s: TTME_DISPLAY\n", thisFcn);
#endif

	if ( theCompose.getTimeOutId() != 0 ) { 
	    XtRemoveTimeOut(theCompose.getTimeOutId());
	}
	// We should probably fill in the token here rather than passing in a NULL,
	// so that we can open an IMAP inbox in addition to a local one.
	// On the other hand, do we ever want to open a mailbox via IMAP if we
	// get a TTME_DISPLAY message?  If so, how do we figure out when to do
	// IMAP vs. non-IMAP?  For now at least, we will always have dtmail open
	// mailboxes locally when getting TTME_DISPLAY.

	SdmToken *token;
	token = new SdmToken;
	token->SetValue("servicechannel", "cclient");
	token->SetValue("serviceclass", "messagestore");
	token->SetValue("servicetype", "local");

	SdmBoolean tmp_file = Sdm_False;
	if (!file) {
	    static const char *temp = "mbox.XXXXXX";
	    char fileName[MAXPATHLEN];
	    fileName[0] = '\0';
	    int fd;

	    strcpy(fileName, "/tmp/");
	    strcat(fileName, temp);
	    mktemp(fileName);
 
	    // Create the temporary mailbox file - since this is a
	    // behind the scenes kind of deal and it is e-mail, we use
	    // mode 0600 as opposed to mode 0666 for security reasons.

	    if ( (fd = SdmSystemUtility::SafeOpen(fileName, O_CREAT | O_WRONLY, 0600)) == -1 ) {
	      tttk_message_fail( msg, TT_DESKTOP_ENFILE, 0, 1 );
	      return 0;
	    }

	    if (SdmSystemUtility::SafeWrite(fd, (void *)contents, len) != len) {
	      tttk_message_fail( msg, TT_DESKTOP_EACCES, 0, 1 );
	      return 0;
	    }
	    SdmSystemUtility::SafeClose(fd);
	    file = strdup(fileName);
	    tmp_file = Sdm_True;

  // 
  // The following is a HACK!!  We'd like to open up attachments readonly and
  // not getting the session lock.  However, there's no way to determine whether
  // this function is being called by the launching of an attachment or by 
  // something else (eg. dtfile).  Therefore, we will just check to see if the
  // the file that's being opened is located in the .dt/tmp directory (which
  // is where mailbox attachments are temporarily created).
  //	    
	} else if (strstr(file, "/.dt/tmp/") != NULL) {
    token->SetValue("serviceoption", "nosessionlocking", "true");
    token->SetValue("serviceoption", "ignoresessionlock", "true");
    if (SdmSystemUtility::SafeAccess(file, W_OK) < 0) {
      token->SetValue("serviceoption", "readonly", "true");
    }
  }
	RoamMenuWindow *roamwin = new RoamMenuWindow(file, msg, token, tmp_file);
	roamwin->initialize(0, NULL);
	roamwin->mailboxName(file);
//	roamwin->manage();
	// Set this to True so Self_destruct() would not be started by Compose.
	dtmail_mapped = 1;
    } else {
	tttk_message_fail( msg, TT_DESKTOP_ENODATA, 0, 1 );
    }
//     tt_free( (caddr_t)contents );
//     tt_free( file );
//     tt_free( docname );
    return 0;
}


static Tt_callback_action
tooltalk_event_handler(Tt_message msg, Tt_pattern )
{
   char *op;
   Tt_status status;


   op = tt_message_op( msg );
   status = tt_ptr_error( op );
   if ((status != TT_OK) || (op == 0)) {
      /* Let tttk_Xt_input_handler() Do The Right Thing */
      return TT_CALLBACK_CONTINUE;
   }

   if (strcmp( op, "XSession_Ending" ) == 0) {
      // WARNING:  DO NOT exit or shutdown the application at
      // this point.  dtsession sends the XSession_Ending
      // event BEFORE the wm_save_yourself callback is called
      // (see RoamApp::saveSessionCB).  Therefore, we should
      // only save the state of mailboxes here.
      theRoamApp.closeAllMessageStores();
   } else {
      tt_free( op );
      return TT_CALLBACK_CONTINUE;
   }
   tt_free( op );
   tttk_message_destroy( msg );
   return TT_CALLBACK_PROCESSED;
}


Tt_message
quit_message_cb(Tt_message m,
	void *client_data,
	Tt_message)
{
    static const char *thisFcn = "quit_message_cb()";
    int silent, force;
    Tt_status status;

    // For security reasons, we will fail the request if either the effective
    // user id or effective group id of the tt message doesn't match the euid/egid
    // of the dtmail process.
    if ((tt_message_uid(m) != geteuid()) || (tt_message_gid(m) != getegid())) {
      tt_message_status_set(m, TT_DESKTOP_EACCES);
      tt_message_fail(m);
      return m;
    }

    char *op = tt_message_op(m);
    if ( tttk_string_op(op) == TTDT_QUIT ) {

	/* silent? */
	status = tt_message_arg_ival(m, 0, &silent);
	if ( status != TT_OK ) {
	    SdmUtility::LogError(Sdm_True, "DEBUG: %s: tt_message_arg_ival(0): %s\n",
		    thisFcn, tt_status_message(status));
	    return m;
	}
	if ( silent ) {
	}

	/* force? */
	status = tt_message_arg_ival(m, 1, &force);
	if ( status != TT_OK ) {
	    SdmUtility::LogError(Sdm_True, "DEBUG: %s: tt_message_arg_ival(1): %s\n",
		    thisFcn, tt_status_message(status));
	    return m;
	}
	if ( force ) {
	    tt_message_reply(m);
	    free(op);
	    // tt_release(mark);
	    RoamApp *app = (RoamApp *)client_data;
	    // Need to make the following call; otherwise, will 
	    // run into Phase2Destroy
	    XtRemoveAllCallbacks(app->baseWidget(), 
				 XmNdestroyCallback);
	    app->shutdown();
	}
	// tt_release(mark);
	return m; 
    } else {
	// tt_release(mark);
	free(op);
	return m;
    }
}
#endif   /* DTMAIL_TOOLTALK */

void
RoamApp::lastInteractiveCB(SdmError&, void *, SdmServiceFunctionType type, SdmEvent* event)
{
        switch(type) {
        case Sdm_SVF_LastInteractiveEvent:
                        *(event->sdmGetLastInteractiveEventTime.lastInteractiveTime)
				 = theApplication->lastInteractiveEventTime();
                        break;
        }

}

void
Usage(char *progname)
{
   printf("Usage:  %s...\n", progname);
   printf
     (
      "  -a file1 ... fileN\n"
      "      Starts a compose window with files \"file1\" through \"fileN\" as attachments.\n\n"
      "  -c\n"
      "      Starts a compose window.\n\n"
      "  -f [server:][mailfile]\n"
      "      The specified mail folder is opened instead of INBOX.  The optional\n"
      "      server argument specifies an IMAP server to use.  If only \"server:\"\n"
      "      is specified, the INBOX on this IMAP server is opened.\n\n"
      "  -h\n"
      "      Displays this help message.\n\n"
      "  -l deadletterfile\n"
      "      Starts a compose window with a \"dead letter\" file.\n\n"
      "  -T address\n"
      "      Open a Compose window with \"address\" in the To field.\n\n"
      );
}

nl_catd DT_catd = (nl_catd) -1;    // catgets file descriptor

// This routine figures out where to get the inbox from.  It first
// looks at environment or .mailrc variables, and finally just
// constructs the name using the /var/mail/<username> convention.
// Should the front end be doing this, or is this a job for the
// mid-level library?  But if it's an IMAP inbox, we should just
// return "INBOX".
// The parameter local is used to force GetInbox to return the
// local path; otherwise, it relies on the "imapinboxserver"
// variable to figure out which path to return.
char *
RoamApp::GetInbox(SdmBoolean localinbox)
{
    SdmError error;
    SdmMailRc * mailRc;
    char *value = NULL;
    char *inbox_path = NULL;

    _mail_connection->connection()->SdmMailRcFactory(error, mailRc);
    assert(!error);

    // Figure out if the inbox is over IMAP or local.
    if ((!mailRc->IsValueDefined("imapinboxserver")) || localinbox) { // Handle local case

	// Get the value of the "DT_MAIL" variable
	mailRc->GetValue(error, "DT_MAIL", &inbox_path);

	if (error) {
	    // If it isn't there, get the value of "MAIL"
	    mailRc->GetValue(error, "MAIL", &inbox_path);
	    if (error) {
		// If it isn't there, use /var/mail/<username>
		passwd pw;
		const char *inbox_dir = "/var/mail/%s";
		char pathname[MAXPATHLEN];

		SdmSystemUtility::GetPasswordEntry(pw);
		sprintf(pathname, inbox_dir, pw.pw_name);
		inbox_path = strdup(pathname);
	    }
	}
    } else { // Handle IMAP case
	inbox_path = strdup("INBOX");
    }
    return (inbox_path);
}

// Private function that allows the mid end library to request enabling
// or disabling of the process group privileges

void RoamApp::GroupPrivilegeActionCallback(void* client_data, SdmBoolean enableFlag)
{
  RoamApp * self = (RoamApp *)client_data;
  if (enableFlag) {
    theApplication->enableGroupPrivileges();
  }
  else {
    theApplication->disableGroupPrivileges();
  }
}

SdmBoolean
RoamApp::FirstInboxOpen()
{
    return _first_inbox_open;
}

// This routine is used to find out if this is the first time that the
// inbox was opened.  We rely on the calling functions to set the state
// of the variable through this routine.
void
RoamApp::FirstInboxOpen(SdmBoolean first_open)
{
    _first_inbox_open = first_open;
}

void RoamApp::initialize ( int *argcp, char **argv )
{
  // ignore sigchld signals so defunct processes not created as a result of fork/exec calls
  signal(SIGCHLD, SIG_IGN);

    // Check to see if this is the vacation start/stop invocation
    if (argcp && *argcp >= 3) {
	if (argv && strcmp(argv[1], "-v") == 0) {

    		_vacation = new VacationCmd("Vacation", catgets(DT_catd, 1, 3, "Vacation"));
		if (strcmp (argv[2], "start") == 0) {
			if (argcp && *argcp == 4)
				_vacation->addVacationToForwardFile(argv[3]);
			else
				_vacation->addVacationToForwardFile(NULL);
		}
		else if (strcmp(argv[2], "stop") == 0)
			_vacation->removeVacationFromForwardFile();
		exit (1);
	  }
    }

    _argc = *argcp;
    _argv = (char **)malloc(*argcp * sizeof(char *));
    for (int i = 0; i < _argc; i++) {
	_argv[i] = (char *)malloc(strlen(argv[i]) + 1);
	strcpy(_argv[i], argv[i]);
    }

    Tt_status status;
    Tt_pattern *tt_pat;
    char **av = argv;
    int	opt;

    _busy_count = 0;
    _killedBySignal = Sdm_False;
    _passwd_dialog = NULL;
    _options = NULL;
    _tempDialog = NULL;
    _vacation = NULL;
    _first_inbox_open = Sdm_True;
    _first_login = Sdm_True;
    _exit_code = 0;       // normal exit status
    _inboxPath = NULL;
    _inboxServer = NULL;
    _outgoing_store = NULL;
#ifdef AUTOFILING
    _autofiling = NULL;
#endif

    int n = 1;
    char * mail_file = NULL;
    char * mail_server = NULL;
    char * dead_letter = NULL;
    char * cmdline_to  = NULL;
    char *session_file = NULL;
    char *helpId;
    SdmToken *token = NULL;

    // create the pipe used for handling signals.
    // do this before registering the signal handlers.
    if (pipe(_signalFileDesc) < 0) {
      SdmUtility::LogError(Sdm_True, "RoamApp::initialize : error creating pipe\n");
      exit (1);
    }

    // set up signal handlers.
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = SigUsr1;
    if (sigaction(SIGUSR1, &sa, NULL)) {
      SdmUtility::LogError(Sdm_True,
		"RoamApp::initialize : error setting up signal handler. errno=%d\n", errno);
      exit(1);
    }
    sa.sa_handler = SigUsr2;
    if (sigaction(SIGUSR2, &sa, NULL) < 0) {
      SdmUtility::LogError(Sdm_True,
	 "RoamApp::initialize : error setting up signal handler. errno=%d\n", errno);
      exit(1);
    }

    _mailview = new DtVirtArray<RoamMenuWindow *>(2);

    // Need to handle the errors generated by Xlib and the server.  These
    // errors are not handled by XtAppSetErrorHandler(). (the Xt routine only
    // handles Xt generated errors)
    XSetErrorHandler(X_error_handler);

    XSetIOErrorHandler(X_IO_error_handler);

    // Must be called before XtAppInitialize.
    XtSetLanguageProc(NULL, NULL, NULL);
    XtAppSetErrorHandler(theApplication->appContext(), dtmail_xterr_handler);
    XtAppSetWarningHandler(theApplication->appContext(), dtmail_xtwrn_handler);

    // Set up environment variable (including NLSPATH) before calling 
    // XtAppInitialize() (cmvc 6576). 
    _DtEnvControl (DT_ENV_SET);

    // This will take care of standard arguments such as -display.
    Application::initialize(argcp,argv);

    // If -session arg is present remove it and return session file name. 
    session_file = parseSessionArg(argcp, argv);

    // export display

    static char displayenv[128];

    sprintf(displayenv, "DISPLAY=%s", XDisplayString(_display));
    putenv(displayenv);

    // Process dtmail opt args.

    int num_legit_args = 0;

    while((opt = getopt(*argcp, argv, "a:cf:l:htT:")) != EOF) {
	switch (opt) {
	  case 'a':
	    	just_compose = 1;
		num_legit_args++;
		break;
	  case 'T':
	    	just_compose = 1;
		cmdline_to = optarg;
		num_legit_args += 2;
		break;
	  case 'c':
		just_compose = 1;
		num_legit_args++;
		break;

	  case 't':
		// Started by ToolTalk
		started_by_tt++;
		num_legit_args++;
		n = 2;
		break;

	  case 'f':
		mail_file = optarg;
		num_legit_args++;
		break;

	  case 'l':
		just_compose = 1;
		num_legit_args++;
		break;
	  case 'h':
	  default:
		Usage(argv[0]);
		exit (1);
	}
    }

    // Disallow mutually exclusive arguments
    if (just_compose && mail_file) {
      Usage(argv[0]);
      exit(1);
    }

    initDebug();

    if(!just_compose && !started_by_tt && !mail_file && session_file)
    	openSessionFile(session_file); // Open the session file

    MdbgChainCheck();
    getResources( _resources, XtNumber ( _resources ) );

    DtInitialize(XtDisplay(baseWidget()), baseWidget(), argv[0], "Dtmail");

    // Must be called after XtSetLanguageProc and DtInitialize because DtInitialize 
    // sets the environment variable NLSPATH which is used in catopen(). That's why
    // we have to take out catopen from Application::initialize and put a new mathod
    // open->catalog for both Application and RoamApp
    this->open_catalog();

    // Initialize the mail_error. This also has to be done after DtInitialize 
    // because the DtMailEnv constructor calls catopen as well.
    SdmError mail_error;

    DtDbLoad();


    //
    // Some of the scrolling lists contain formatted text and thus
    // require a fixed width font to display the text correctly.
    // We want to use the fixed width font defined by dt.userFont as
    // it will be internationalized correctly.  If it is not there
    // we fall back to a fixed width lucida font.  If that fails
    // we let the widget fallback to what ever it thinks is best.
    //

    // Default font in case we can't find anything else.
    XrmDatabase db = XtScreenDatabase(XtScreen(baseWidget()));

    if (db) {
	FontType	userfont;
	char		*glyphname = NULL;
        char		buf[1024];


	// Need to check fixed width font mailrc variable.  Assume
	// True for now
#if 0
	if (True) {
#endif

		XrmPutStringResource(&db, "*Work_Area*Text*fontList",
							 _user_font);
		XrmPutStringResource(&db, "*Work_Area*iconGadget*fontList",
							 _user_font);
		XrmPutStringResource(&db, "*Work_Area*Text*textFontList",
							 _user_font);
		
		XrmPutStringResource(&db, "*XmDialogShell*XmList*FontList",
							 _user_font);

		// Convert the user fontlist to a font.
                if (!fontlist_to_font(_user_fontlist, &userfont)) {
                    /* Couldn't convert the user fontlist to a font */
                    if (!(userfont.f.cf_font =
                             XLoadQueryFont(XtDisplay(baseWidget()), "fixed")))
			fprintf (stderr, "RoamApp::initialize : error loading fixed font\n");
                    else
                        userfont.cf_type = XmFONT_IS_FONT;
                }

#if 0
	} else {
		FontType	systemfont;

		XrmPutStringResource(&db, "*Work_Area*Text*fontList",
							 _system_font);
		XrmPutStringResource(&db, "*Work_Area*iconGadget*fontList",
							 _system_font);
		XrmPutStringResource(&db, "*Work_Area*Text*textFontList",
							 _system_font);
		XrmPutStringResource(&db, "*XmDialogShell*XmList*FontList",
							 _system_font);

                if (!fontlist_to_font(_system_fontlist, &systemfont)) {
                    /* Couldn't convert the system fontlist to a font */
                    if (!(systemfont.f.cf_font =
                             XLoadQueryFont(XtDisplay(baseWidget()),
				"variable")))
			fprintf (stderr, "RoamApp::initialize : error loading fixed font\n");
                    else
                        systemfont.cf_type = XmFONT_IS_FONT;
                }
                
	}
#endif

	// If the glyph font is specified in a resource file, use it.
        if (_glyph_font) {
	    glyphname = XtNewString(_glyph_font);
	}
	
	// If the glyph font hasn't been specified, try to match it with
	// the user font.
        if (!glyphname) {
	    // Get the font name that matches the user font pixel size.
            load_app_font(baseWidget(), &userfont, &glyphname);
        }

	// Create a fontlist that contains the glyph and user fonts
	strcpy(buf, _user_font);
	if (strchr(_user_font, '=') == NULL) {
	    // No tag.  Add one
	    strcat(buf, "=plain, ");
	}

	// If the symbol font can't be found, use user font above
	if (glyphname) {
	    strcat(buf, glyphname);
	    strcat(buf, "=attach");
	}

	// Loosely bind font to message list.  This lets users override
 	// with a more strongly bound name
	// (ie "Dtmail*Message_List*FontList");
	XrmPutStringResource(&db, "*Message_List*FontList", buf);

	XtFree (glyphname);

    }

    // If we don't have a mail file yet, then we need to retrieve the
    // Initialize Tooltalk
    // NOTE:  For now, must make the FIRST ttdt_open call to be the proc_id 
    //        that will respond to the start message.  Therefore, call gui's 
    //        ttdt_open before libDtMail calls its.
    char *sess = NULL;

    sess = (char *)getenv("TT_SESSION");
    if (!sess || (*sess == '\0')) {
      sess = getenv("_SUN_TT_SESSION");
    }
    if (!sess || (*sess == '\0')) {
      tt_default_session_set(tt_X_session(XDisplayString(_display)));
    }

    roam_tt_procid = ttdt_open( &roam_tt_fd, "DTMAIL", "SunSoft", "%I", 1 );
    if (dieFromTtError("ttdt_open", tt_ptr_error(roam_tt_procid))) {
	// Serious error here -- Tooltalk did not initialize -- 
	// we're exiting for now.
	DtMailGenDialog *exit_dialog = new DtMailGenDialog(
					   "ExitDialog", 
					   theApplication->baseWidget());
	exit_dialog->setToErrorDialog( catgets(DT_catd, 2, 1, "Mailer"),
				       catgets(DT_catd, 2, 2, "ToolTalk is not initialized.  Mailer cannot run without ToolTalk.\nTry starting /usr/dt/bin/ttsession, or contact your System Administrator.") );
	helpId = DTMAILHELPCANTINITTOOLTALK;
	exit_dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), helpId);
	XtRemoveAllCallbacks(theApplication->baseWidget(), XmNdestroyCallback);
	exit(1);
    }

    // This is for supporting old ptype where RFC_822_Message is
    // in lower case.
    status = ttmedia_ptype_declare( "RFC_822_Message",
				    0,
				    tooltalk_msg_handler,
				    this,
				    1);
    dieFromTtError("ttmedia_ptype_declare", tt_ptr_error(status));

    status = ttmedia_ptype_declare( "RFC_822_MESSAGE",
				    0,
				    tooltalk_msg_handler,
				    this,
				    1);
    dieFromTtError("ttmedia_ptype_declare", tt_ptr_error(status));

    status = ttmedia_ptype_declare( "MAIL_TYPE",
				    0,
				    attachmt_msg_handler,
				    this,
				    1);
    dieFromTtError("ttmedia_ptype_declare", tt_ptr_error(status));

    /* Join the default session -- This should have been done by default */
    tt_pat = ttdt_session_join((const char *)0,
			       (Ttdt_contract_cb)quit_message_cb,
			       baseWidget(),
			       this,
			       1);
    dieFromTtError("ttdt_session_join", tt_ptr_error(*tt_pat));


    // register to get the tooltalk event when the session ends.
    // when this happens, we want to save the state of any mailboxes
    // that are opened before the session ends.
    //
    Tt_pattern events2Watch = tt_pattern_create();
    tt_pattern_category_set( events2Watch, TT_OBSERVE );
    tt_pattern_class_add( events2Watch, TT_NOTICE );
    tt_pattern_scope_add( events2Watch, TT_SESSION );
    char* sessId = tt_default_session();
    tt_pattern_session_add( events2Watch, sessId );
    tt_free( sessId );
    tt_pattern_op_add( events2Watch, "XSession_Ending" );
    tt_pattern_callback_add( events2Watch, tooltalk_event_handler );
    tt_pattern_register( events2Watch );


    // Here is a/the place to put MailConnection creation
    _mail_connection = new MailConnection(mail_error);
    if (mail_error != Sdm_EC_Success) {
      // what do we do here?  there are no windows for dialogs
      // should we just register a syslog() and exit? ugggh
      // SR
      SdmUtility::LogError(Sdm_True,
		"RoamApp::initialize : error connecting to back end.\n");
      exit(1);
    }

    // Got a connection with the mid end library established.
    // Hook in our private function to allow the mid end library to request enabling
    // or disabling of the process group privileges

    (void) _mail_connection->connection()->RegisterGroupPrivilegeActionCallback(mail_error,
								  GroupPrivilegeActionCallback,
								  this);
    if (mail_error != Sdm_EC_Success) {
      SdmUtility::LogError(Sdm_True,
		"RoamApp::initialize : error registering service function.\n");
      exit(1);
    }

    SdmServiceFunction sf((const SdmCallback)&RoamApp::lastInteractiveCB, NULL,
                                Sdm_SVF_LastInteractiveEvent);
    _mail_connection->connection()->RegisterServiceFunction(mail_error, sf);
    if (mail_error != Sdm_EC_Success) {
      // what do we do here?  there are no windows for dialogs
      // should we just register a syslog() and exit? ugggh
      // SR
      SdmUtility::LogError(Sdm_True,
		"RoamApp::initialize : error registering service function.\n");
      exit(1);
    }

    _mail_isession = new MailSession(mail_error, Application::appContext(), Sdm_ST_InSession);
    if (mail_error != Sdm_EC_Success) {
      // what do we do here?  there are no windows for dialogs
      // should we just register a syslog() and exit? ugggh
      // SR
      SdmUtility::LogError(Sdm_True,
		"RoamApp::initialize : error creating incoming MailSession.\n");
      exit (1);
    }

    _mail_osession = new MailSession(mail_error, Application::appContext(), Sdm_ST_OutSession);
    if (mail_error != Sdm_EC_Success) {
      // what do we do here?  there are no windows for dialogs
      // should we just register a syslog() and exit? ugggh
      // SR
      SdmUtility::LogError(Sdm_True,
		"RoamApp::initialize : error creating outgoing MailSession\n");
      exit (1);
    }
    
    // .mailrc parsing error checking
    SdmSession * d_session = _mail_isession->isession();
    SdmMailRc * mailRc;
    _mail_connection->connection()->SdmMailRcFactory(mail_error, mailRc);
    mail_error = mailRc->GetParseError();
    if (mail_error != Sdm_EC_Success) {
	int answer = 0;
	DtMailGenDialog *mailrc_dialog = new DtMailGenDialog("MailRcDialog",
							     theApplication->baseWidget());
	mailrc_dialog->setToQuestionDialog(catgets(DT_catd, 2, 1, "Mailer"),
					   catgets(DT_catd, 2, 22, "There were unrecoverable syntax errors found in the .mailrc file.\nCheck for more messages in the ~/.dt/errorlog file.\nFix the errors and restart Mailer. If you choose to continue,\nyou will not be able to save to the .mailrc file any changes made\nin the Options dialog box.") );
	helpId = DTMAILHELPERROR;
	Widget hpBut = XmMessageBoxGetChild(mailrc_dialog->baseWidget(),
					    XmDIALOG_HELP_BUTTON);
	XtSetSensitive(hpBut, False);
	answer = mailrc_dialog->post_and_return(catgets(DT_catd, 2, 23,
			"Continue"), catgets(DT_catd, 2, 24, "Exit"), helpId);
	if (answer == 2) {
	    XtRemoveAllCallbacks(theApplication->baseWidget(),
				 XmNdestroyCallback);
	    exit(1);
	}
    }

    mail_error.Reset();
    // Store the template list so we know if the list has changed via props.
    if ( !just_compose ) {
	if ( mailRc->IsValueDefined("templates") )
	    mailRc->GetValue(mail_error, "templates", &_templateList);
#ifdef AUTOFILING
    	    _autofiling = new AutoFilingCmd(
                        "Auto-filing Mailboxes",
                        catgets(DT_catd, 1, 256, "Auto-filing Mailboxes..."),
                        catgets(DT_catd, 1, 258, "Mailer - Auto-filing Mailboxes"),
			Sdm_True,
                        this);
#endif // AUTOFILING
	else
	    _templateList = NULL;

#ifdef AUTOFILING
	_autofiling = new AutoFilingCmd("Auto-filing Mailboxes",
		catgets(DT_catd, 1, 256, "Auto-filing Mailboxes..."),
		catgets(DT_catd, 1, 258, "Mailer - Auto-filing Mailboxes"),
		Sdm_True,
		this);
#endif
    } else {
	_templateList = NULL;
    }

    _options = new OptCmd("Mail Options...", catgets(DT_catd, 1, 2,
			     "Mail Options..."), Sdm_True, _w);


    if ( just_compose ) {
        // Need to install self destruct
	SendMsgDialog *compose = theCompose.getWin();

	if ( dead_letter ) {
	    compose->loadDeadLetter(dead_letter);
	}
	else {
	    XtRemoveCallback(compose->getTextW(), XmNmodifyVerifyCallback,
		&SendMsgDialog::editorContentsChangedCB, compose);

	    compose->appendSignature(Sdm_False);

	    XtAddCallback(compose->getTextW(), XmNmodifyVerifyCallback,
                &SendMsgDialog::editorContentsChangedCB, compose);

	    if ( *argcp > num_legit_args + 1) {    // Have attachment(s)
		for ( int k = num_legit_args + 1;  k < *argcp;  k++ ) {
		    compose->inclAsAttmt(argv[k], NULL);
		}
	    }
            if (cmdline_to) {
                    compose->setHeader( "To", cmdline_to, Sdm_True );
	    }
	}
    }

    if (mail_file) {
      char *p;

      if (p = strchr(mail_file, ':')) {
        mail_server = mail_file;
        *p++ = '\0';
        mail_file = *p ? p : NULL;
      }
    }

    // If a mailbox location has been specified with the -f option, then
    // explicitly initialize an SdmToken object to reflect this location.
    if (mail_server || mail_file) {
      token = new SdmToken;
      token->SetValue("servicechannel", "cclient");
      token->SetValue("serviceclass", "messagestore");
      if (mail_server) {
        token->SetValue("servicetype", "imap");
        token->SetValue("serviceoption", "hostname", mail_server);
      }
      else {
        token->SetValue("servicetype", "local");
      }
    }

    if (!mail_file) {
      // The catches the case `dtmail -f hostname:' which is interpreted to
      // mean the INBOX on the IMAP server `hostname'.
      mail_file = mail_server ? "INBOX" : GetInbox();
    }
    _inboxPath = GetInbox();

    mailRc->GetValue(mail_error, "imapinboxserver", &_inboxServer);


    // Process the message that started us, if any.  Should I pass in
    // roam_tt_procid or NULL.
    tttk_Xt_input_handler( 0, 0, 0 );
    XtAppAddInput( Application::appContext(), roam_tt_fd,
		   (XtPointer)(XtInputReadMask | XtInputExceptMask),
		   tttk_Xt_input_handler, roam_tt_procid );
		   
    // register input source used for signal handling.
    XtAppAddInput(Application::appContext(), _signalFileDesc[READ], (XtPointer)XtInputReadMask,
                   roam_app_signal_handler, (XtPointer) this);


    // Get the vacation handle before the new RoamMenuWindow
    // This is for setting the Vacation title stripe on the window
    _vacation = new VacationCmd("Vacation", catgets(DT_catd, 1, 3, "Vacation"));

    // DtMail only supports the "Mail" message.  
    // If DtMail is started by ToolTalk, then we assume that the 
    // client wants a Compose window.  Therefore, we do not
    // bring up a RoamMenuWindow.
    //
    if ( !session_fp && !started_by_tt && !just_compose) {
	// We should probably fill in the token before we pass it in so that
	// we can open IMAP as well as local mailboxes.
	RoamMenuWindow *rmw = new RoamMenuWindow(mail_file, NULL, token);
	dtmail_mapped = 1;
	rmw->initialize(_argc, _argv);
	// rmw->mailboxName(mail_file);
	// rmw->manage();
    } else {
	if(session_fp ) 
	    restoreSession();
    }

    // We update the at job id resources used to determine if vacation is on 
    // or off.
    _vacation->resetVacationStatus();

    // This is for setting the Vacation title stripe on the window and must be
    // done after resetVacationStatus().
    if (_vacation->vacationIsOn())
        setTitles();

    initSession();

    // Make sure the default folder and exists. Its safer to do 
    // this here as there are so many places to check that we might as well 
    // get it out of the way.
    char *dir = getFolderDir(local, Sdm_True);
    if (SdmSystemUtility::SafeAccess(dir, F_OK) != 0)
	mkdir(dir, 0700);
    free(dir);
 
    _dialog = new DtMailGenDialog("Dialog", _w);
} 

RoamApp::RoamApp(char *name) : Application (name)
{
    DebugPrintf(2, "RoamApp::RoamApp(0x%x \"%s\")\n", name);
}

// Let the destructor of parent Application class handle the
// exit().

RoamApp::~RoamApp()
{
#ifdef DTMAIL_TOOLTALK
    if ( roam_tt_procid != NULL ) {
	ttdt_close(0, 0, 1);
    }
#endif

    if (_w) 
      XtRemoveAllCallbacks(_w, XmNdestroyCallback);
      
    // Hack!! for now, null out the widget here because in some cases  
    // when the widget is destroyed (in BasicComponent::~BasicComponent)
    // a bus error occurs.  Since there is only one RoamApp object and it
    // is destroyed during static destruction, there isn't a problem with
    // not destroying the widget in this case except that it will show
    // up as a memory leak.
    _w = NULL;
    catclose(DT_catd);
    
}

void
RoamApp::shutdown()
{
    // note: we must destroy the outgoing store before we delete
    // ANY MailSession object  Destroying ANY MailSession object 
    // will result in destroying both the incoming and outgoing 
    // SdmSession objects because they are static objects that 
    // are destroyed in the MailSession destructor.
    if (_outgoing_store) {
      //
      // remove the outgoing store file because it's used temporarily
      // right now anyway.  eventually, with disconnect support,
      // we will be using a user specified file for the outgoing store.
      // In this case, we should only close the store and not remove the file.
      // 
      delete _outgoing_store;    // destroying store will also close it.
      _outgoing_store = NULL;
      SdmSystemUtility::SafeUnlink((const char*)_outgoing_store_name);
    }
    
    delete _mail_isession;
    _exit(_exit_code);
}

void
RoamApp::applicationTimeout( XtPointer client_data,
				   XtIntervalId *id )
{
  RoamApp *app = ( RoamApp * ) client_data;
  app->timeout( id );
}

void
RoamApp::timeout( XtIntervalId * )
{
	SdmSession * d_session = _mail_isession->isession();
	SdmError error;

#ifdef DOLATER
    d_session->poll(error);
#endif // DOLATER

    XtAppAddTimeOut( XtWidgetToApplicationContext( _w ),
		     15000,
		     &RoamApp::applicationTimeout,
		     (XtPointer) this );
}

void
RoamApp::showBusyState(SdmError &, DtMailBusyState busy_state, void * client_data)
{
    RoamApp * self = (RoamApp *)client_data;

    switch (busy_state)
    {
    case DtMailBusyState_AutoSave:
      self->busyAllWindows(catgets(DT_catd, 3, 1, "Auto-saving..."));
      break;

    case DtMailBusyState_NewMail:
      self->busyAllWindows(catgets(DT_catd, 3, 86, "Checking for new mail..."));
      break;

    case DtMailBusyState_NotBusy:
    default:
      self->unbusyAllWindows();
      break;
    }
}

long
RoamApp::raLastInteractiveEventTime(void * client_data)
{
    RoamApp * self = (RoamApp *)client_data;
    return (theApplication->lastInteractiveEventTime());
}

void
RoamApp::setBusyState(SdmError & error, DtMailBusyState busy_state, void * client_data)
{
    RoamApp * self = (RoamApp *)client_data;

    static const MAXBUSYSTATES=20;
    static DtMailBusyState busyStateStack[MAXBUSYSTATES] = {DtMailBusyState_NotBusy};
    static busyStateStackTop = 0;

    SdmError localError;	// used to hold local copy of passed in error state

    localError = error;

    switch (busy_state)
    {
    case DtMailBusyState_AutoSave:
    case DtMailBusyState_NewMail:
    default:
      showBusyState(localError, busy_state, client_data);
      if (busyStateStackTop)  		// Already busy from previous message ?
	self->_busy_count--;		// Yes, backoff busy count added for this message
      busyStateStack[++busyStateStackTop] = busy_state;
      assert(busyStateStackTop < MAXBUSYSTATES);
      break;

    case DtMailBusyState_NotBusy:
      if (busyStateStackTop) {
	if (localError != Sdm_EC_Success) {
	    // There should always be a valid RoamMenuWindow pointer in
	    // the zero'th element of the _mailview array
	    self->mailView(0)->postErrorDialog(localError);
	}
	busy_state = busyStateStack[--busyStateStackTop];
        showBusyState(localError, busy_state, client_data);
	if (busyStateStackTop)
	  self->_busy_count--;		// backoff busy count for redisplaying this message
      }
      else {
	showBusyState(localError, busy_state, client_data);
      }
      break;
    }
    assert(busyStateStack[0] == DtMailBusyState_NotBusy);
}

void
RoamApp::busyAllWindows(const char * msg)
{
  for (int win = 0; win < _numWindows; win++) {
    if (_busy_count == 0) {
      _windows[win]->busyCursor();
    }
    if (msg) {
      _windows[win]->setStatus(msg);
    }
  }
  // if a message was specified, them call XmUpdateDisplay to force the
  // display of the status message on the windows.
  if (msg != NULL) {
    XmUpdateDisplay(baseWidget());
  }

  _busy_count++;
}

void
RoamApp::unbusyAllWindows(void)
{
    _busy_count--;
    if (_busy_count == 0) {
      RoamMenuWindow *rmw;
      for (int win = 0; win < _numWindows; win++) {
        _windows[win]->normalCursor();
        _windows[win]->clearStatus();
		if (strcmp(_windows[win]->className(), "RoamMenuWindow") == 0) {
			rmw = (RoamMenuWindow *)_windows[win];
			rmw->get_editor()->attachArea()->bottomStatusMessage(" ");
		}
      }
    }
}

void
RoamApp::globalPropChange(void)
{
	busyAllWindows(catgets(DT_catd, 1, 4, "Updating properties..."));

	for (int win = 0; win < _numWindows; win++) {
		_windows[win]->propsChanged();
	}
	updateTemplates();

	unbusyAllWindows();
}

DtMailGenDialog *
RoamApp::genDialog()
{
    return(_dialog);
}

int
RoamApp::numOpenRMW(void)
{
        const char *cname;
        int win;
        int numOpen = 0;

        for (win = 0; win < _numWindows; win++) {
                cname = _windows[win]->className();
                if (strcmp(cname, "RoamMenuWindow") == 0) {
                        if (!_windows[win]->isIconified())
                                numOpen++;
                }
        }
        return numOpen;
}
// This routine updates the templates menus in the RoamMenuWindow and
// the SendMsgDialog when the template list changes.
void
RoamApp::updateTemplates(void)
{
    SdmSession * d_session = _mail_isession->isession();
    SdmError error;
    SdmMailRc *mailRc;
    char *value = NULL;
    int win = -1;

    _mail_connection->connection()->SdmMailRcFactory(error, mailRc);
    assert(!error);
    // Only update if templates have changed.
    mailRc->GetValue(error, "templates", &value);
    if ( (value == NULL && _templateList != NULL) ||
        (value != NULL &&
	 (_templateList == NULL || strcmp(value, _templateList)) != 0) ) {

    	const char *cname;

     	for (win = 0; win < _numWindows; win++) {
		cname = _windows[win]->className();
		if (strcmp(cname, "RoamMenuWindow") == 0) {
    			RoamMenuWindow *rmw;
			rmw = (RoamMenuWindow *)_windows[win];
			rmw->construct_compose_menu();
		}
		else if (strcmp(cname, "SendMsgDialog") == 0) {
    			SendMsgDialog *smd;
			smd = (SendMsgDialog *)_windows[win];
			smd->createFormatMenu();
		}
        }

    	if (_templateList != NULL) 
                free(_templateList); 
    	if (value && *value) 
                _templateList = value; 
    	else 
                _templateList = NULL; 
    }
    // This means the templates haven't changed, so free value 
    if (win == -1 && value)
	free(value);
}
// If the user restarts the inbox view by changing the imapinboxserver
// setting in props, we have to reset the "Inbox" menu items for open,
// move and copy. we have to reset both the rmw pointers and the inbox path.
// Note: Move and Copy determine path on the fly.

void 
RoamApp::resetInboxCmd()
{
    const char *cname;
    RoamMenuWindow *rmw;

    char *mail_file = GetInbox();
    for (int win = 0; win < _numWindows; win++) {
	cname = _windows[win]->className(); 
	if (strcmp(cname, "RoamMenuWindow") == 0) { 
    		rmw = (RoamMenuWindow *)_windows[win];
		rmw->openInbox()->setMailboxName(mail_file);
	}
    }
    
}
void 
RoamApp::resetTitles()
{
    const char *cname;
    RoamMenuWindow *rmw;

    for (int win = 0; win < _numWindows; win++) {
	cname = _windows[win]->className(); 
	if (strcmp(cname, "RoamMenuWindow") == 0) { 
    		rmw = (RoamMenuWindow *)_windows[win];
	    	rmw->removeVacationTitle();
	}
    }
    
}
void 
RoamApp::setTitles()
{
    RoamMenuWindow *rmw;
    const char *cname;

    for (int win = 0; win < _numWindows; win++) {
	cname = _windows[win]->className();
	if (strcmp(cname, "RoamMenuWindow") == 0) {
		rmw = (RoamMenuWindow *)_windows[win];
		rmw->setVacationTitle();
	}
    }
}

void
RoamApp::startVacation()
{

    if (_vacation->startVacation() == 0)
	setTitles();
    else
	resetTitles();
}

void
RoamApp::stopVacation()
{
    _vacation->stopVacation();
    resetTitles();
}

char*
getPropStringValue(DtVirtArray<PropStringPair *> & results, const char *label)
{
        for (int mrc = 0; mrc < results.length(); mrc++) {
            PropStringPair * psp = results[mrc];
            if (strcmp(label, psp->label) == 0) {
		if (psp->value != NULL)
                	return strdup(psp->value);
        	return (char*)NULL;
	    } 
        }
        return (char*)NULL;
}

void
parsePropString(const char * input_string, DtVirtArray<PropStringPair *> & results)
{
  if (input_string == NULL)
	return;

    // If it's not multibyte, use the regular string function
  if (MB_CUR_MAX <= 1) {
     const char * start = input_string;
     const char * end;
     char *ptr;

    do {
	while (isspace(*start)) {
	    start += 1;
	}
	
	for (end = start; end && *end; end++) 
	    if (isspace(*end) && *(end-1) != '\\')
		break;

	PropStringPair * new_pair = new PropStringPair;

	int len = end - start;
	char * label = new char[len + 5];
	memset(label, 0, len+5);
	strncpy(label, start, len);
	label[len] = 0;
	for (ptr = label; (ptr = strstr(ptr, "\\ ")); ptr++) 
		strcpy(ptr, ptr+1); 
	
	char * file = strchr(label, ':');
	if (!file) {
	    new_pair->label = strdup(label);
	    new_pair->value = NULL;
	}
	else {
	    *file = 0;
	    new_pair->label = strdup(label);

	    file += 1;
	    if (strlen(file) == 0) {
		new_pair->value = NULL;
	    }
	    else {
		new_pair->value = strdup(file);
	    }
	}
 	int i, list_len = results.length();
	for (i=0; i < list_len; i++) {
	    PropStringPair *psp = results[i];
	    if (strcmp(psp->label, new_pair->label) >= 0)
		 break;
        }
	results.insert(new_pair, i);

	delete [] label;

	start = end;
	
	while (*start && isspace(*start)) {
	    start += 1;
	}
	
    } while(*start && *end);
  }

  else {
    // The string can contain multibyte characters and it must be converted to 
    // wide character string before parsing
    int len = strlen(input_string);
    wchar_t *wc_input= new wchar_t[len+1];
    mbstowcs(wc_input, input_string, len+1);
    const wchar_t *start = wc_input;
    const wchar_t *end;
    wchar_t *ptr;

	do {
	    while (iswspace(*start)) {
		start += 1;             
	    }

	    for (end = start; end && *end; end++)
		if (iswspace(*end) && *(end-1) != (wint_t)'\\')
		    break;
	    PropStringPair * new_pair = new PropStringPair;

	    int wclen = end - start;
	    wchar_t *wc_label = new wchar_t[wclen+1];
	    wsncpy(wc_label, start, wclen);
	    wc_label[wclen] = (wint_t)'\0';

	    // Search the string "\ " and take out the back slash
	    wchar_t esc_space[3];
	    mbstowcs(esc_space, "\\ ", 3);
	    for (ptr = wc_label; (ptr = wcswcs(ptr, esc_space)); ptr++)
		    wcscpy(ptr, ptr+1);

	    wchar_t *file = wcschr(wc_label, (wint_t)':');
	    if (!file) {
		new_pair->label = new char[(wclen+1)*MB_CUR_MAX];
		wcstombs(new_pair->label, wc_label, wclen+1);
		new_pair->value = NULL;

	    }
	    else {
		*file = (wint_t)'\0';
		new_pair->label = new char[(wclen+1)*MB_CUR_MAX];
		wcstombs(new_pair->label, wc_label, (wclen+1)*MB_CUR_MAX);

		file += 1;
		int filelen = wcslen(file);
		if (filelen == 0) {
		    new_pair->value = NULL;
		}
		else {
		    new_pair->value = new char[(filelen+1)*MB_CUR_MAX];
		    wcstombs(new_pair->value, file, (filelen+1)*MB_CUR_MAX);

		}
	    }
	    int i, list_len = results.length();
	    for (i=0; i < list_len; i++) {
		PropStringPair *psp = results[i];
		if (strcmp(psp->label, new_pair->label) >= 0)
		     break;
	    }
	    results.insert(new_pair, i);

	    delete []wc_label;
	    start = end;

	    while (*start && iswspace(*start)) {
		start += 1;
	    }

	} while(*start && *end);

	delete []wc_input;
    }
}

PropStringPair::PropStringPair(void)
{
    label = NULL;
    value = NULL;
}

PropStringPair::PropStringPair(const PropStringPair & other)
{
    label = NULL;
    value = NULL;

    if (other.label) {
	label = strdup(other.label);
    }

    if (other.value) {
	value = strdup(other.value);
    }
}

PropStringPair::~PropStringPair(void)
{
    if (label) {
	free(label);
    }

    if (value) {
	free(value);
    }
}

void
RoamApp::setSession(MailSession *session)
{
   _mail_isession = session; 
}


VacationCmd*
RoamApp::vacation()
{
    return (_vacation);
}

char *
PropStringPair::formatPropPair()
{

    // figure out whitespace for formatting
    // assume 13 for normal sized alias name

    if (value != NULL) {
    	char *formatted_str = NULL;
    	int i, num_spaces = 0;
	int key_len = strlen(label);

    	if (key_len < 13)
		num_spaces = 13 - key_len;
	formatted_str = (char*)malloc(key_len + strlen((char *)value) + num_spaces + 5);
	strcpy(formatted_str, label);
	for (i=key_len; i < (key_len+num_spaces); i++)
		formatted_str[i] = ' ';	
	formatted_str[i] = '\0';	
	strcat(formatted_str, " = ");
	strcat(formatted_str, (char*)value);
        return formatted_str;
    }
    return strdup(label);

}

SdmBoolean
RoamApp::isRegistered(RoamMenuWindow *rmw)
{
    int i;

    for (i=0; i<_mailview->length(); i++) {
        if ((*_mailview)[i] == rmw)
	    return(Sdm_True);
    }
    return(Sdm_False);
}

int RoamApp::registerMailView(RoamMenuWindow *rmw)
{
    _mailview->append(rmw);
    return (_mailview->length() - 1);
}

void RoamApp::unregisterMailView(int index)
{
    int i;

    _mailview->remove(index);
    // reset the mailbox number for each of the RMW's created after the
    // one being removed.
    for (i=index; i<_mailview->length(); i++) {
	(*_mailview)[i]->setMailboxIndex(i);
    }
}

RoamMenuWindow*
RoamApp::inboxWindow()
{
    RoamMenuWindow *rmw;
    const char *cname;

    for (int win = 0; win < _numWindows; win++) {
	cname = _windows[win]->className();
	if (strcmp(cname, "RoamMenuWindow") == 0) {
	    rmw = (RoamMenuWindow *)_windows[win];
	    if (rmw->inbox()) {
		return(rmw);
	    }
	}
    }
    return(NULL);
}

void
RoamApp::open_catalog()
{
    Application::open_catalog();      // Open Motif Application message catalog file
    DT_catd = catopen(DTMAIL_CAT, NL_CAT_LOCALE); // Open DtMail message catalog file
}

SdmMessageStore*
RoamApp::GetOutgoingStore(SdmError &error)
{
  static const char   *temp = "outmbox.XXXXXX";
  error = Sdm_EC_Success;
  
  if (_outgoing_store == NULL) {
    assert (_mail_osession != NULL);
    SdmSession *d_session = _mail_osession->osession();
    assert (d_session != NULL);

    SdmToken token;
    char  fileName[MAXPATHLEN];
    SdmMailRc *mailRc;
    SdmMessageStore *mstore;

    // Create a temporary file to serve as an outgoing mailbox
    strcpy(fileName, "/tmp/");
    strcat(fileName, temp);
    mktemp(fileName);

    if (d_session->SdmMessageStoreFactory(error, mstore)) {
      SdmSystemUtility::SafeUnlink(fileName);
      return(NULL);
    }

    if (mstore->StartUp(error)) {
      delete mstore;
      return(NULL);
    }

    token.SetValue("servicechannel", "cclient");
    token.SetValue("serviceclass", "messagestore");
    token.SetValue("serviceoption", "messagestorename", fileName);
    token.SetValue("servicetype", "local");
    token.SetValue("serviceoption", "readwrite", "x");
    token.SetValue("serviceoption", "ignoresessionlock", "true");
    token.SetValue("serviceoption", "nosessionlocking", "true");
    theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailRc);
    assert (!error);
    if (mailRc->IsValueDefined("dontsavemimemessageformat"))
      token.SetValue("serviceoption", "preservev3messages", "x");

    if (mstore->Attach(error, token)) {
      delete mstore;
      return(NULL);
    }

    mstore->Create(error, fileName);
    if ( error ) {
      delete mstore;
      return(NULL);
    }

    SdmBoolean readOnly;
    SdmMessageNumber nmsgs = 0;
    mstore->Open(error, nmsgs, readOnly, token);
    if ( error ) {
      delete mstore;
      return(NULL);
    }
    
    _outgoing_store = mstore;
    _outgoing_store_name = fileName;
  }
  
  return _outgoing_store;
}

  
char *
RoamApp::getFolderDir(folderType type, SdmBoolean expand)
{
	FORCE_SEGV_DECL(SdmSession, m_session);
	char *bufptr, *expanded_dir;
	char folderdir[1024];
	SdmMailRc *mailRc;
	MailRcSource *mrs;
	SdmError error;

	error.Reset();

	// Get the folder directory...
	_mail_connection->connection()->SdmMailRcFactory(error, mailRc);
  	assert(!error);
 	if (type == imap)
		mrs = (MailRcSource*)new MailRcSource(mailRc, "imapfolder");
	else if (type == local)
		mrs = (MailRcSource*)new MailRcSource(mailRc, "folder");
	else 
		return NULL;
	bufptr = (char*)mrs->getValue();
	int i = strlen(bufptr);
	if (i > 1 && bufptr[i-1] == '/')
		bufptr[i-1] = '\0';
	delete mrs;
	SdmUtility::ExpandPath(error, expanded_dir, bufptr, *mailRc, 
			type == imap ? SdmUtility::kImapFolderResourceName : 
					SdmUtility::kFolderResourceName);

	// Is the folder a relative path? Prepend the home directory
	// But only if local. remote directory could be on non-unix platform
	if (type == local && expanded_dir[0] != '/') {
		if (expand)
			sprintf(folderdir, "%s/%s", getenv("HOME"), expanded_dir);
		else
			sprintf(folderdir, "$HOME/%s", bufptr);
		free(expanded_dir);
		free(bufptr);
		return strdup(folderdir);
	}

	if (expand) {
		free(bufptr);
		return expanded_dir;
	}

	free(expanded_dir);
	return bufptr;
}

char *
RoamApp::getResource(resourceType rtype, SdmBoolean expand)
{
    FORCE_SEGV_DECL(SdmSession, m_session);
    SdmError error;
    char *bufptr=NULL, *expanded_res;
    char resource[1024];
    SdmMailRc *mailRc;
    MailRcSource *mrs;

    _mail_connection->connection()->SdmMailRcFactory(error, mailRc);
    assert(!error);
    m_session = theRoamApp.isession()->isession();
    if (rtype == DraftDir)
        mrs = (MailRcSource*)new MailRcSource(mailRc, "draftdir");
    else if (rtype == TemplateDir)
        mrs = (MailRcSource*)new MailRcSource(mailRc, "templatedir");
    else if (rtype == DeadletterDir)
        mrs = (MailRcSource*)new MailRcSource(mailRc, "deaddir");
    else if (rtype == SentMessageLogFile)
        mrs = (MailRcSource*)new MailRcSource(mailRc, "record");
    bufptr = (char*)mrs->getValue();
    delete mrs;

    SdmUtility::ExpandPath(error, expanded_res, bufptr, *mailRc, SdmUtility::kFolderResourceName);

    // Is it a relative path? Prepend the folder resource
    if (expanded_res[0] != '/') {
    	char *folder = NULL;

        // If message log, make it relative to folder directory
    	if (rtype == SentMessageLogFile)
		folder = getFolderDir(local, expand);
   	 else { // For all other directorys, relative to home directory
		if (expand) {
		    SdmUtility::ExpandPath(error, folder, "~", *mailRc, SdmUtility::kFolderResourceName);
	    	} else {
		    folder = "$HOME";
		}
    	 }

     	// Prepend the appropriate directory.
    	if (expand) 
	    	sprintf(resource, "%s/%s", folder, expanded_res);
     	else
	    	sprintf(resource, "%s/%s", folder, bufptr);
    	free(expanded_res);
    	if (rtype == SentMessageLogFile || expand)
	    free(folder);
     	free(bufptr);
    	return strdup(resource);
    }
    if (expand) {
    	free(bufptr);
    	return expanded_res;
    }
    free(expanded_res);
    return bufptr;
}

void
RoamApp::configure_menu(Widget menu)
{
  WidgetList children;
  Dimension button_width, button_height, menu_width;
  short maxcols, newcols, menucols;
  int num_buttons=0, buttons_that_fit;
  Cardinal num_children=0;
  Position menu_x, menu_y;
  int screenheight = HeightOfScreen(XtScreen(menu));

  XtVaGetValues(menu,
                XmNx, &menu_x,
                XmNy, &menu_y,
                XmNwidth, &menu_width,
                XmNnumColumns, &menucols,
                XmNnumChildren, &num_children,
                XmNchildren, &children,
                NULL);

  // Only count managed children
  for (int i = 0; i < num_children; i++)
    if (XtIsManaged(children[i]))
      num_buttons++;

  if (num_children == 0 || num_buttons == 0) {
	newcols = 0;
  } else {
  	XtVaGetValues(children[0],
                XmNheight, &button_height,
                XmNwidth, &button_width,
                NULL);

  	// The number of buttons that will fit on half the height of screen
  	buttons_that_fit = screenheight / (int)button_height / 2;

  	// The total number of buttons on the menu divided by the number of
  	// buttons on half a screen gives us the number of columns for the menu
  	newcols = (num_buttons / buttons_that_fit) +
    	((num_buttons % buttons_that_fit) > 0 ? 1 : 0);
	
  	maxcols = WidthOfScreen(XtScreen(menu)) / ((int)menu_width / (int)menucols);
  	if (newcols > maxcols)
    	   newcols = maxcols;
  }

  XtVaSetValues(menu, XmNnumColumns, newcols, NULL);
}

void
RoamApp::map_menu( Widget menu, XtPointer clientData, XtPointer)
{
  if (clientData) {
    CmdList *cl = (CmdList *) clientData;
    RAMenuClientData *mcd = (RAMenuClientData *) cl->getClientData();
    if (mcd) {
      // mcd[0] - MailDrafts; mcd[1] - DeadLetters
      // (set in RoamMenuWindow::construct_compose_menu)
      struct stat fstat, fstatDead;
      stat(mcd[0].filename, &fstat);
      stat(mcd[1].filename, &fstatDead);
      if (fstat.st_mtime != mcd[0].timestamp ||
          fstatDead.st_mtime != mcd[1].timestamp) {
        MenuBar *mb = new MenuBar("menuPR", menu);
        // If menu was mapped with commands already, remove them and add new
        // ones
        if (mcd[0].timestamp != 0 || mcd[1].timestamp != 0) {
          mb->removeCommands(menu, cl);
          while (cl->size()) cl->remove(0);
        }
        if (fstat.st_mtime != mcd[0].timestamp)
          mcd[0].timestamp = fstat.st_mtime;
        if (fstatDead.st_mtime != mcd[1].timestamp)
          mcd[1].timestamp = fstatDead.st_mtime;
        theRoamApp.addDrafts(cl, mcd[0].smd, mcd[0].filename, mcd[1].filename);
        mb->addCommands(cl, FALSE, XmMENU_PULLDOWN);
      }
    }
  }
  theRoamApp.configure_menu(menu);
}

#define DOTS(a) (a[0] == '.' && \
        (a[1] == 0 || (a[1] == '.' && a[2] == 0)))

void
RoamApp::addDrafts(CmdList *subCmd, SendMsgDialog *smd,
                   char *draftDir, char *deadletterDir)
{
  Cmd *button = NULL;
  DIR *dirp; 
  struct dirent *direntp;
  struct stat fstat;
  char filename[1024];

  if (dirp = opendir(draftDir)) {
    while ((direntp = readdir(dirp)) != NULL) {
      if (direntp->d_ino == 0 || DOTS(direntp->d_name)) continue;
      sprintf(filename, "%s/%s", draftDir, direntp->d_name);
      SdmSystemUtility::SafeStat(filename, &fstat);
      if ((fstat.st_mode & S_IFMT) == S_IFREG) {
        button = new LoadFileCmd(direntp->d_name, direntp->d_name,
                                 1, smd, filename, TYPE_DRAFT);
        subCmd->insert(button);
      }
    }
  }
  if (dirp = opendir(deadletterDir)) {
    while ((direntp = readdir(dirp)) != NULL) {
      if (direntp->d_ino == 0 || DOTS(direntp->d_name)) continue;
      sprintf(filename, "%s/%s", deadletterDir, direntp->d_name);
      SdmSystemUtility::SafeStat(filename, &fstat);
      if ((fstat.st_mode & S_IFMT) == S_IFREG) {
        button = new LoadFileCmd(direntp->d_name, direntp->d_name,
                                 1, smd, filename, TYPE_DRAFT);
        subCmd->add(button);
      }
    }
  }
  if (button == NULL) {
    button = new LoadFileCmd("No Drafts",
                             catgets(DT_catd, 3, 127, "No Drafts"),
                             1, smd,
                             NULL, TYPE_DRAFT);
    subCmd->add(button);
  }
}

//Convert from Label to server and mailbox
SdmBoolean
RoamApp::fromLabelToServerMb(char *label, char **server, char **mailbox)
{
        if (server == NULL || mailbox == NULL || label == NULL)
                return Sdm_False;
 
        char *sptr, *eptr;
        SdmError error;
        SdmMailRc *mailRc;
        SdmBoolean ret_val =  Sdm_False;
 
        *server=NULL;
        *mailbox=NULL;
 
        theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailRc);
        assert(!error);
 
        if ((sptr = strrchr(label, '(')) != NULL &&
	    (eptr = strrchr(label, ')')) != NULL &&
	    (sptr < eptr)) {
                // Get the server specifed.
                *server = (char*)malloc(strlen(sptr));
                strcpy(*server, sptr+1);
                (*server)[strlen(sptr)-2] = '\0';
   		*mailbox = (char*)malloc(sptr-label+1);
		strncpy(*mailbox, label, sptr-label); 
		(*mailbox)[sptr-label] = '\0';
		ret_val = Sdm_True;
	}
       	else 
		*mailbox = strdup(label);

        return (ret_val);
}
// Token must be of the form: "MailboxName,serverName,serverType" or
// else just "MailboxName". This routine will return MailboxName(serverName)
// or else just MailboxName if no serverName is supplied.
// This is for converting from how the info is stored in .mailrc to menu label
void 
RoamApp::fromTokenToLabel(char *token)
{
        char *tptr;
        if ((tptr = strchr(token, ',')) != NULL) {
                *tptr = '(';
                if ((tptr = strchr(tptr, ',')) != NULL) {
                        *tptr++ = ')';
                        *tptr = '\0';
                }
        }

}
SdmBoolean 
RoamApp::validateAliases(SdmError &err, char *alist)
{
        int err_ind; 
        SdmString *alias = new SdmString(alist);
 
	SdmMessageUtility::ParseRFC822Headers(err, err_ind, *alias);
	delete alias;
	if (err)
		return Sdm_False;
	return Sdm_True;
}
SdmBoolean
RoamApp::resolvePath(char **filen, const char *server)
{
    char fullpath[2048];
    SdmError error;
    SdmMailRc *mailRc;
    SdmConnection *con = theRoamApp.connection()->connection();
    con->SdmMailRcFactory(error, mailRc);
    assert(!error);
    char *fn, *dir;
 
    fullpath[0] = '\0';
    SdmUtility::ExpandPath(error, fn, *filen, *mailRc,
                server == NULL ? SdmUtility::kFolderResourceName :
                SdmUtility::kImapFolderResourceName);
 
    if (server != NULL) { // Imap mailbox
                const char *delim = thePasswordDialogManager->delimiter(server);
                thePasswordDialogManager->setImapServer(strdup(server));
                SdmBoolean firstTime = Sdm_True;
 
                // If the delimiter is NULL, that means we have never connected
                // to this server. Since we have to put up a login eventually for
                // opening/moving the dialog, we do it here and get the delimiter.
                // We keep putting up a login dialog if failure occurs on connection.
                // assuming the password, user name, was incorrect. We bail if they
		// cancel the dialog.
 
                while (delim == NULL) {
                        thePasswordDialogManager->resetLoginError();
                        SdmBoolean success;
                        if (firstTime) {
                                // Only posts if necessary...
                                success = thePasswordDialogManager->loginDialog();
                                firstTime = Sdm_False;
                        }
                        else
                                // Always posts login dialog... Will do this if there was
				// an error connecting with previous information given
                                success = thePasswordDialogManager->postDialog();
                        if (!success) {
                                // Canceled out of dialog...
                                free(fn);
                                XtUnmanageChild(thePasswordDialogManager->baseWidget());
                                return Sdm_False;
                        }
                        if (thePasswordDialogManager->loginError() == Sdm_EC_Success) {
                                delim = thePasswordDialogManager->delimiter(server);
				break;
			}
                }
 
                // This means the filename is a relative path...
                if (delim && delim[0] != fn[0]) {
                        dir = theRoamApp.getFolderDir(imap, Sdm_True);
                        // If delimiters match, use the default imap folder.
                        if (delim[0] == dir[0])
				// If the dir IS the delimiter or if the 
				// dir has a delim appended to it...
				if (dir[1] == '\0' || dir[strlen(dir)-1] == delim[0])
                                	sprintf(fullpath, "%s%s", dir, fn);
				else
                                	sprintf(fullpath, "%s%s%s", dir, delim, fn);
                        free(dir);
                }
        }
        // Local mailbox
        else if (*fn != '/' && *fn != '$' && *fn != '~') { // relative path
		dir = theRoamApp.getFolderDir(local, Sdm_True);
		if (strcmp(dir, "/") == 0)
                	sprintf(fullpath, "/%s", fn);
		else
                	sprintf(fullpath, "%s/%s", dir, fn);
                free(dir);
        }
        free(*filen);

        if (fullpath[0] != '\0') {
                *filen = strdup(fullpath);
                free(fn);
        }
        else
                *filen = fn;

        return Sdm_True;
}
