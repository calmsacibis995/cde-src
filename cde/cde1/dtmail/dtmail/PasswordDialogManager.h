/*
 *+SNOTICE
 *
 *	$Revision: 1.2 $
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
#pragma ident "@(#)PasswordDialogManager.h	1.10 02/22/95"
#endif

///////////////////////////////////////////////////////////
// PasswordDialogManager.h
//////////////////////////////////////////////////////////

#ifndef PASSWORDDIALOGMANAGER_H
#define PASSWORDDIALOGMANAGER_H
#include "PromptDialogManager.h"

class PasswordDialogManager : public PromptDialogManager {
private:

  Widget _user;
  Widget _password;
  static void modifyVerifyCallback(Widget, XtPointer, XmTextVerifyCallbackStruct *);
  char _pwd[100]; // Big enough for most reasonable passwords.
protected:
    
  Widget createDialog ( Widget );
    
public:
  
  PasswordDialogManager ( char * );
  void modifyVerify( Widget,XmTextVerifyCallbackStruct * );
#ifdef DEAD_WOOD
  char *userName();
  char *password();
  void resetPassword();
#endif /* DEAD_WOOD */

};

extern PasswordDialogManager *thePasswordDialogManager;

#endif
