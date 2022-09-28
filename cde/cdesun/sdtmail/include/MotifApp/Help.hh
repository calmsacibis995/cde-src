/*
 *+SNOTICE
 *
 *      $Revision: 1.2 $
 *
 *      RESTRICTED CONFIDENTIAL INFORMATION:
 *
 *      The information in this document is subject to special
 *      restrictions in a confidential disclosure agreement between
 *      HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *      document outside HP, IBM, Sun, USL, SCO, or Univel without
 *      Sun's specific written approval.  This document and all copies
 *      and derivative works thereof must be returned or destroyed at
 *      Sun's request.
 *
 *      Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */
 
#ifndef HELP_HH
#define HELP_HH

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)Help.hh	1.16 03/31/97"
#endif

#include <Dt/Help.h>

// Option defines for menubar help access
#define HELP_ON_ITEM     1
#define HELP_ON_TOPIC    2
#define HELP_ON_VERSION  3

#define DTMAILCONTAINERMENUID "DTMAILVIEWMAINWINDOWMENUBARFILE"
//#define DTMAILEDITMENUID "DTMAILVIEWMAINWINDOWMENUBAREDIT"
#define DTMAILMESSAGEMENUID "DTMAILVIEWMAINWINDOWMENUBARMESSAGE"
//#define DTMAILATTACHMENUID "DTMAILVIEWMAINWINDOWMENUBARATTACH"
#define DTMAILVIEWMENUID "DTMAILVIEWMAINWINDOWMENUBARVIEW"
#define DTMAILCOMPOSEMENUID "DTMAILVIEWMAINWINDOWMENUBARCOMPOSE"
#define DTMAILOPTIONSMENUID "DTMAILVIEWMAINWINDOWMENUBAROPTIONS"
#define DTMAILMOVEMENUID "DTMAILVIEWMAINWINDOWMENUBARMOVE"
#define DTMAILHELPFSB "DTMAILHELPFILESELECTIONBOX"

#define APP_MENU_ID "onApplicationMenu"
#define VER_MENU_ID "_copyright"

char *getHelpId(Widget);
void HelpCB(Widget, XtPointer, XtPointer);
extern void DisplayMain(Widget, char *, char *);
Widget getErrorHelpWidget(void);
void clearErrorHelpWidget(void);
extern void DisplayErrorHelp(Widget, char *, char *);
void HelpErrorCB(Widget, XtPointer, XtPointer);
extern void DisplayVersion(Widget, char *, char *);
static void CloseMainCB(Widget, XtPointer, XtPointer);

#endif
