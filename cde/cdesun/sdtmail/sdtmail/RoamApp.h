/*
 *+SNOTICE
 *
 *	$Revision: 1.93 $
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

#ifndef ROAMAPP_H
#define ROAMAPP_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)RoamApp.h	1.93 07/07/97"
#else
static char *sccs__ROAMAPP_H = "@(#)RoamApp.h	1.93 07/07/97";
#endif


#include <stdio.h>

#include "Application.h"
#include "MailSession.hh"
#include "MailConnection.hh"
#include "DtMailTypes.hh"

enum resourceType {
	DraftDir = 0, 
	TemplateDir = 1, 
	DeadletterDir = 2, 
	SentMessageLogFile = 3 
};
enum folderType { imap = 0, local = 1 };


class Cmd;
class VacationCmd;
class DtMailGenDialog;
class RoamMenuWindow;
class OptCmd;
#ifdef AUTOFILING
class AutoFilingCmd;
#endif // AUTOFILING

class RoamApp : public Application
{
  private:
    static XtResource _resources[];
    DtVirtArray<RoamMenuWindow *> *_mailview;
    static void saveSessionCB(Widget w, XtPointer client, XtPointer call);
    char *parseSessionArg(int *argc, char **argv);
    void openSessionFile(char *filename);
    void initSession(void);
    void restoreSession(void);
    VacationCmd	 *_vacation;
#ifdef AUTOFILING
    AutoFilingCmd *_autofiling;
#endif // AUTOFILING
    DtMailGenDialog *_dialog;
    int _busy_count;
    Cmd *_passwd_dialog;
    OptCmd *_options;
    char *_templateList;
    SdmBoolean _first_inbox_open;
    SdmBoolean _first_login;
    int _exit_code;
    int _argc;
    char **_argv;

    DtbModuleDialogInfo _tempDialog;

    FILE *session_fp;
    
    int _signalFileDesc[2];

    char * _inboxPath;
    char * _inboxServer;

    SdmBoolean _killedBySignal;
    static void lastInteractiveCB(SdmError & error, void *clientdata, SdmServiceFunctionType type, SdmEvent* event);

    void closeAllMailboxWindows(void);
    void closeLocalNonInboxWindows(void);
    void setKilledBySignal(SdmBoolean isKilled) { _killedBySignal = isKilled; }
    
  protected:
    char	*_print_script;
    char	*_mailfiles_folder;
    char	*_default_mailbox;
    char	*_user_font;	// Fixed width font
    XmFontList	_user_fontlist;
    char	*_system_font;	// Variable width font
    XmFontList	_system_fontlist;
    char	*_glyph_font;	// Font for attchment glyph
    MailConnection *_mail_connection;
    MailSession *_mail_isession;
    MailSession *_mail_osession;
    SdmMessageStore *_outgoing_store;
    SdmString _outgoing_store_name;
    
    int	_tt_fd;
    
    static void applicationTimeout ( XtPointer, XtIntervalId * );
    void timeout(XtIntervalId *);
    static long raLastInteractiveEventTime(void *);
    static void GroupPrivilegeActionCallback(void* clientData, SdmBoolean enableFlag);

  public:
    RoamApp ( char * );
    virtual ~RoamApp();     
    virtual const char *const className() { return "RoamApp"; }
    char	*mail_folder() { return _mailfiles_folder; }
    char	*print_script(){ return _print_script; }
    char	*default_mailbox(){ return _default_mailbox; }
    virtual void shutdown();
    MailConnection *connection(void) { return _mail_connection; }
    MailSession *isession(void) { return _mail_isession; }
    MailSession *osession(void) { return _mail_osession; }
    
    SdmMessageStore *GetOutgoingStore(SdmError &error);
    
    char *GetInbox(SdmBoolean local = Sdm_False);
    SdmBoolean FirstInboxOpen();
    void FirstInboxOpen(SdmBoolean);

    void checkForUnixFromLine(const char *filename);

    SdmBoolean killedBySignal(void) { return _killedBySignal; }
    void writeToSignalFileDesc(char buf);
    void handleSignal(int theSignal);
    void busyAllWindows(const char * msg = NULL);
    void unbusyAllWindows(void);
    void closeAllMessageStores(void);
    void globalPropChange(void);
    void startVacation();
    void updateTemplates();
    int numOpenRMW();
    void stopVacation();
    DtMailGenDialog *genDialog();
    void setSession(MailSession *);
    RoamMenuWindow* mailView(int index){ return (*_mailview)[index]; }
    RoamMenuWindow* inboxWindow();
    SdmBoolean isRegistered(RoamMenuWindow *);
    int registerMailView(RoamMenuWindow *);
    void unregisterMailView(int);
    OptCmd *mailOptions(void) { return _options; };
#ifdef AUTOFILING
    AutoFilingCmd *autoFiling(void) { return _autofiling; };
#endif // AUTOFILING
    
    char*       getFolderDir(folderType type, SdmBoolean expand);
    char*       getResource(resourceType, SdmBoolean expand);

    void        setTemplateDialog( DtbModuleDialogInfo tempDialog) {_tempDialog = tempDialog; }
    DtbModuleDialogInfo        getTemplateDialog() {return _tempDialog; }

    void        addDrafts(CmdList*, SendMsgDialog*, char *, char *);
    
#if (XlibSpecificationRelease>=5)   
    virtual void initialize ( int *, char ** );  
#else
    virtual void initialize ( unsigned int *, char ** );  
#endif

    virtual void open_catalog();  

	// save your session
    virtual void saveYourSelf(void);
    FILE *sessionFile(void) { return session_fp;}
    VacationCmd* vacation();
    static void map_menu( Widget, XtPointer, XtPointer );
    void resetTitles();
    void setTitles();
    void configure_menu(Widget menu);
    SdmBoolean fromLabelToServerMb(char *label, char** server, char** folder);
    void fromTokenToLabel(char *token);
    void showBusyState(SdmError &, DtMailBusyState, void *);
    void setBusyState(SdmError &, DtMailBusyState, void *);
    SdmBoolean validateAliases(SdmError &err, char *alist);
    void resetInboxCmd();
    void resetInboxPath(char *path) {  _inboxPath = path; };
    void resetInboxServer(char *server) {  if (_inboxServer) free(_inboxServer); _inboxServer = server; };
    char* getInboxPath() { return _inboxPath; };
    char* getInboxServer() { return _inboxServer; };

    void firstLogin(SdmBoolean fl) { _first_login = fl; };
    SdmBoolean firstLogin() { return _first_login; };
    SdmBoolean resolvePath(char **filen, const char *server);
    void setExitCode(int e) { _exit_code = e; }
};

// Used for mapping dynamic menus
static struct RAclientdata {
        time_t timestamp;
        char *filename;
        SendMsgDialog *smd;
};
typedef struct RAclientdata RAMenuClientData;

// This method will parse a colon/space tuples that are used in
// the mail properties.
//
struct PropStringPair {
    char *	label;
    char *	value;

    PropStringPair(void);
    PropStringPair(const PropStringPair &);
    ~PropStringPair(void);
    char *formatPropPair();
};

void parsePropString(const char * input, DtVirtArray<PropStringPair *> & result);
char* getPropStringValue(DtVirtArray<PropStringPair *> & result, const char *value);

extern RoamApp theRoamApp;

// This variable indicates whether RoamMenuWindow is mapped or not.
// If a RMW is mapped, then Self_destruct will not be called by Compose
// if Compose was started by ToolTalk.
extern int dtmail_mapped;

#endif // ROAMAPP_H
