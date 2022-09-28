/*
 *+SNOTICE
 *
 *	$Revision: 1.25 $
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
 *	Copyright 1993, 1994, 1995, 1996 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)PasswordDialogManager.h	1.25 06/04/97"
#endif

///////////////////////////////////////////////////////////
// PasswordDialogManager.h
//////////////////////////////////////////////////////////

#ifndef PASSWORDDIALOGMANAGER_H
#define PASSWORDDIALOGMANAGER_H
#include "PromptDialogManager.h"

void passwordDialogOKCallback(int *data);
void passwordDialogCancelCallback(int *data);

class PasswordDialogManager : public PromptDialogManager {
private:

  Widget _imap_server;
  char *_servername;
  Widget _user;
  char *_username;
  Widget _password;
  char *_pwd; // Big enough for most reasonable passwords.
  Widget _choose_bandwidth;
  SdmBoolean _high_bandwidth;
  struct OpenStruct *_os;
  Widget _dialog;
  SdmStrStrL _serverDelim;
  SdmError _error;

  static void traverse(Widget, XEvent*, String*, Cardinal*);
  static void modifyVerifyCallback(Widget, XtPointer, XmTextVerifyCallbackStruct *);
  static void bandwidthCallback(Widget, XtPointer, XtPointer);
  static void anotherOkcb(Widget, XtPointer, XtPointer);

protected:
    
  Widget createDialog ( Widget );
  static void okCallback ( Widget, XtPointer, XtPointer );
  static void cancelCallback( Widget, XtPointer, XtPointer );

    
public:
  
  PasswordDialogManager ( char * );

  virtual Widget post(char *,
			char *, 
			void *clientData = NULL,
			DialogCallback ok = NULL,
			DialogCallback apply = NULL,
			DialogCallback cancel = NULL,
			DialogCallback help = NULL);

  virtual Widget post(char *,
			char *, 
			Widget w,
			void *clientData = NULL,
			DialogCallback ok = NULL,
			DialogCallback apply = NULL,
			DialogCallback cancel = NULL,
			DialogCallback help = NULL);

    virtual Widget post (
			 char *,
			 char *, 
			 Widget w,
			 void *clientData      = NULL,
			 DialogCallback ok     = NULL,
			 DialogCallback cancel = NULL,
			 DialogCallback help   = NULL );

    virtual Widget post (
			 char *,
			 char *, 
			 void *clientData      = NULL,
			 DialogCallback ok     = NULL,
			 DialogCallback cancel = NULL,
			 DialogCallback help   = NULL );

  virtual int post_and_return(char *, char *,
			DialogCallback ok = NULL,
			DialogCallback apply = NULL,
			DialogCallback cancel = NULL,
			DialogCallback help = NULL);
			
  virtual int	   post_and_return(
      char *, 
      char *,
      Widget);
  virtual int	   post_and_return(
      char *, 
      char *, 
      char *,
      Widget);
  virtual int	   post_and_return(
      char *, 
      char *, 
      char *, 
      char *,
      Widget);


  void modifyVerify( Widget,XmTextVerifyCallbackStruct * );
  void setHigh(SdmBoolean high) {_high_bandwidth = high;};

  char *password() {return _pwd;};
  char *user() {return _username;};
  char *imapserver() {return _servername;};
  Widget imapServerWidget() {return _imap_server;};
  Widget dialog() {return _dialog;};

  void setPassword(char *passwd) {if (_pwd) free(_pwd); _pwd = passwd;};
  void setUser(char *user) {if (_username) free(_username); _username = user;};
  void setImapServer(char *imap_server) {if (_servername) free(_servername); _servername = imap_server;};

  // returns text data from the dialog widgets
  char *User();
  char *ImapServer();
  SdmBoolean loginDialog();
  SdmBoolean postDialog();
  OpenStruct* os() { return _os; }; 
  SdmBoolean IsHighbandwidth() { return _high_bandwidth; }

  const char *getDelimiter(const char *server);
  const char *delimiter(const char *server);
  SdmError setDelimiter(const char *server, SdmMessageStore *ms);
  SdmError loginError() { return _error; };
  void resetLoginError() { _error = NULL; };

  static void okcb(Widget, XtPointer, XtPointer);
  static void OkCallback(Widget, XtPointer, XtPointer);
#ifdef DEAD_WOOD
  char *userName();
  void resetPassword();
#endif /* DEAD_WOOD */

};

extern PasswordDialogManager *thePasswordDialogManager;
#endif
