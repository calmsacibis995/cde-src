/*
 *+SNOTICE
 *
 *	$Revision: 1.3 $
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

#ifndef ROAMAPP_H
#define ROAMAPP_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)RoamApp.h	1.63 05/08/96"
#else
static char *sccs__ROAMAPP_H = "@(#)RoamApp.h	1.63 05/08/96";
#endif


#include <stdio.h>

#include "Application.h"
#include "MailSession.hh"

enum resourceType {
	DraftDir = 0, 
	TemplateDir = 1, 
	DeadletterDir = 2, 
	SentMessageLogFile = 3 
};


class Cmd;
class VacationCmd;
class DtMailGenDialog;
class RoamMenuWindow;

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
    DtMailGenDialog *_dialog;
    int _busy_count;
    Cmd *_options;
    char *_optionsHandle;
    char *_templateList;

    DtbModuleDialogInfo _tempDialog;

    FILE *session_fp;

    Boolean _killedBySignal;
    
  protected:
    char	*_print_script;
    char	*_mailfiles_folder;
    char	*_default_mailbox;
    char	*_user_font;	// Fixed width font
    XmFontList	_user_fontlist;
    char	*_system_font;	// Variable width font
    XmFontList	_system_fontlist;
    char	*_glyph_font;	// Font for attchment glyph
    MailSession *_mail_session;
    DtMail::Transport * _mail_transport;
    int	_tt_fd;
    
    static void applicationTimeout ( XtPointer, XtIntervalId * );
    void timeout(XtIntervalId *);
    static void showBusyState(DtMailEnv &, DtMailBusyState, void *);
    static void setBusyState(DtMailEnv &, DtMailBusyState, void *);
    static long raLastInteractiveEventTime(void *);
    static void raDisableGroupPrivileges(void *);
    static void raEnableGroupPrivileges(void *);

  public:
    RoamApp ( char * );
    virtual ~RoamApp();     
    virtual const char *const className() { return "RoamApp"; }
    char	*mail_folder() { return _mailfiles_folder; }
    char	*print_script(){ return _print_script; }
    char	*default_mailbox(){ return _default_mailbox; }
    virtual void shutdown();
    MailSession *session(void) { return _mail_session; }
    DtMail::Transport * default_transport(void) { return _mail_transport; }
    static void statusCallback(DtMailOperationId, DtMailEnv &, void *);
    void setKilledBySignal(void) { _killedBySignal = TRUE; }
    Boolean killedBySignal(void) { return _killedBySignal; }
    void closeAllWindows(void);
    void busyAllWindows(const char * msg = NULL);
    void unbusyAllWindows(void);
    void globalPropChange(void);
    void startVacation();
    void updateTemplates();
    int numOpenRMW();
    void stopVacation();
    DtMailGenDialog *genDialog();
    void setSession(MailSession *);
    RoamMenuWindow* mailView(int index){ return (*_mailview)[index]; }
    RoamMenuWindow* inboxWindow();
    int registerMailView(RoamMenuWindow *);
    void unregisterMailView(int);
    Cmd *mailOptions(void) { return _options; };
    char* optionsDialog(void) { return _optionsHandle; }
    void setOptionsDialog(char *oHandle) { _optionsHandle = oHandle; }
    
    char*       getFolderDir(Boolean expand);
    char*       getResource(resourceType, Boolean expand);

    void        setTemplateDialog( DtbModuleDialogInfo tempDialog) {_tempDialog = tempDialog; }
    DtbModuleDialogInfo        getTemplateDialog() {return _tempDialog; }

    void        addDrafts(CmdList*, SendMsgDialog*, char *);
    
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
