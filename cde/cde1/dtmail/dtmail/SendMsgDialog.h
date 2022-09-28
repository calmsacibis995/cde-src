/*
 *+SNOTICE
 *
 * $Revision: 1.6 $
 *
 * RESTRICTED CONFIDENTIAL INFORMATION:
 * 
 * The information in this document is subject to special
 * restrictions in a confidential disclosure agreement between
 * HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 * document outside HP, IBM, Sun, USL, SCO, or Univel without
 * Sun's specific written approval.  This document and all copies
 * and derivative works thereof must be returned or destroyed at
 * Sun's request.
 *
 * Copyright 1993, 1994, 1995 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef SENDMSGDIALOG_H
#define SENDMSGDIALOG_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)SendMsgDialog.h	1.109 04/29/96"
#else
static char *sccs__SENDMSGDIALOG_H = "@(#)SendMsgDialog.h	1.109 04/29/96";
#endif

//#include "Editor.hh"
//#include "CDEM_CoeEd.hh"
//#include "XmTextEditor.h"

#include <Tt/tt_c.h>
#include "DtMailEditor.hh"
#include "MenuWindow.h"
#include "NoUndoCmd.h"
#include "RoamCmds.h"
#include "DtMailGenDialog.hh"
#include "ComposeCmds.hh"
#include "MenuButton.h"
#include "RoamApp.h"

class MailBox;
class Cmd;

// This is the Compose window.
class SendMsgDialog : public MenuWindow, public AbstractEditorParent {
  public:
    enum ShowState {
	SMD_ALWAYS,
	SMD_SHOWN,
	SMD_HIDDEN,
	SMD_NEVER
	};
    
    struct HeaderList {
	char  		*label;
	char  		*header;
	char		*value;
	ShowState	show;
	Widget		form_widget;
	Widget		label_widget;
	MenuButton	*mb_widget;
	Widget		field_widget;
	Widget		display_widget;
	HeaderList(void);
	HeaderList(const HeaderList &);
	~HeaderList(void);
	};
    
  private:
    
    Widget	_main_form;
    DtMailEditor  *_my_editor;
    Widget  _textW;
    DtMailGenDialog *_genDialog; 

    Boolean _takeDown;
    Boolean _first_time;
    Boolean _already_sending;  // Flag to make sure we don't send more than once.

    Boolean _show_attach_area;
    Boolean _edits_made;
    
    Cmd  *_send_button;
    Cmd  *_close_button;
    Cmd  *_separator;
    
    // File Menu items
    
    SelectFileCmd *_file_include;
    SaveAsDraftCmd *_save_as_draft;
    SaveAsTemplateCmd *_save_as_template;
    SaveAsTextCmd *_save_as_text;
    CmdList *_savecmdList;
    MenuButton *_saveas_button;
    Widget _saveas_menubutton;
    Cmd *_file_log;
    Cmd *_file_send;
    Cmd   *_file_sendAs[10];   // Assume 10 transports.
    Cmd *_file_close;
    Widget _file_cascade;
    
    // Edit Menu items
    
    Cmd *_edit_undo;
    Cmd *_edit_cut;
    Cmd *_edit_copy;
    Cmd *_edit_paste;
    Cmd *_edit_paste_special[2];
    Cmd *_edit_clear;
    Cmd *_edit_delete;
    Cmd *_edit_select_all;
    
    // Attachment
    Widget   _attachmentMenu;
    CmdList *_attachmentMenuList;
    CmdList *_attachmentPopupMenuList;
    CmdList *_textPopupMenuList;
    CmdList *_attachmentActionsList;
    Cmd *_attachment_popup_title;
    Cmd *_compose_popup_title;

    SaveAttachCmd  *_att_save;
    SelectFileCmd *_att_add;
    Cmd *_att_delete;
    Cmd *_att_undelete;
    Cmd *_att_rename;
    Cmd  *_att_select_all;
    Cmd   *_att_show_pane;

    void  construct_attachment_popup(void);
    void  construct_text_popup(void);
    
    // Format
    
    Cmd *_format_word_wrap;
    Cmd  *_format_settings;
    Cmd *_format_find_change;
    Cmd   *_format_spell;
    Cmd *_format_bcc;    // Depends on Properties setting.
    Widget _format_menu;
    CmdList *_format_cmds;
    CmdList *_templates;
    CmdList *_drafts;
    ComposeSigCmd *_signature;
    char *_templateFile;
    char *_templateFullPath;
    char *_draftFile;
    Widget _format_cascade;

    Cmd *_aliasCmd;
    Cmd *_sigCmd;
    Cmd *_vacationCmd;

    CmdList *_aliasCmdList;
    DtVirtArray<PropStringPair *> *_aliases;

    Widget _status_form;
    Widget _status_text;

    int _num_sendAs;    // Number of Transports available through Back End.
    
    // The header list keeps track of the current headers on the Format
    // menu, as well as their current state in the compose header region.
    DtVirtArray<HeaderList *>	_header_list;
    Widget			_header_form;
    char			*_additionalfields;
    
    // Help
    Cmd *_overview;
    Cmd *_tasks;
    Cmd *_reference;
    Cmd *_messages;
    Cmd *_on_item;
    Cmd *_using_help;
    Cmd *_about_mailer;

    // This is the internal data structure used for submission.
    // As user adds/deletes attachment, this data structure is update immediately.
    // This data structure is also updated at periodic checkpoints to store the
    // latest Compose window data.
    // Before sending, this is also updated.
    DtMail::Message *_msgHandle;   
    
    // Pointer to the message that is being included as an attachment.
    // When updateMsgHndAtt() is called, data in this handle is copied to _msgHandle.
    DtMail::Message *_inclMsgHandle;
    
    // Pointer to the last Attachment BodyPart so that newBodyPart(error, _lastAttBP)
    // returns a message body part after the last body part.  This is so that
    // attachments will be ordered properly.
    DtMail::BodyPart *_lastAttBP;
    
    // When makeMessage() is called, it creates a message handle with its
    // first body part empty.  If set to TRUE, then this boolean variable
    // indicates that the first body part of _msgHandle has already been used
    // (probably for text).  Otherwise, the first body part can be used for attachment.
    Boolean _firstBPHandled;
    
    // updateMsgHndAtt() uses this Boolean variable to determine whether the
    // current message handle to be copied contains a text body part or not.
    Boolean _inclMsgHasText;
    
    // The auto save interval id is used to stop the interval timer for auto
    // saving.
    //
    XtIntervalId _auto_save_interval;
    char * _auto_save_path;
    char * _auto_save_file;
    
    char * _dead_letter_buf;
    
    DtMailBoolean _log_msg;
    
    void mkAutoSavePath();
    static void autoSaveCallback(XtPointer, XtIntervalId *id);
    void doAutoSave(void);
    int getAutoSaveInterval(void);
    
    void   createMenuPanes();   // Create all menu
    void addTemplates(CmdList *); // Create the initial template menu.
    void addAliases(); // Create the initial alias menu.
    void createAliasMenuButton(MenuButton **button, Widget form, char* label); 
    Widget createWorkArea( Widget );   // Create all widget, button
    void createHeaders(Widget header);
    void doDynamicHeaderMenus(void);
    Boolean reservedHeader(const char *label);

    void valueToAddrText(Widget, DtMailValueSeq &);
    
    // File selection Box callbacks 
#ifdef DEAD_WOOD
    static void open_att_cb( void *, char * );
#endif /* DEAD_WOOD */
    static void include_file_cb( void *, char * );
    static void add_att_cb( void *, char * );
    static void save_att_cb( void *, char * );
    
    // Return a message handle -- internal data structure of a mail message.
    DtMail::Message * makeMessage(void);
    // Clear Compose window text fields to prepare for re-use.
    void reset();

    // Find a header in the list by name.
    //
    int lookupHeader(const char * name);
    void reattachHeaders(void);
    void justifyHeaders(void);
    void forceFormResize(Widget);
    int _header_list_index;

    // Called when the sendmail child process completes
    static void sendmailErrorProc (int pid, int status, void *data);
    void popupMemoryError(DtMailEnv &error);

  public:
    SendMsgDialog(Tt_message = NULL);
    virtual ~SendMsgDialog();
    void createFormatMenu(); // Create the format menu
    void createFileMenu(); // Create the file menu
    virtual const char *const className () { return "SendMsgDialog"; }
    void initialize();
    void send_message( const char *, int );

    void include_file(char *);
    void add_att(char *);
    void add_att(char *, DtMailBuffer);
    void add_att(DtMailBuffer);
    void setMsgHnd();    // initialize _msgHandle.
    DtMail::Message * getMsgHnd() { return _msgHandle; }
    void startAutoSave(void);
    void stopAutoSave(void);
    void setLogState(DtMailBoolean state) { _log_msg = state; }
    void setEditsMade(Boolean em) { _edits_made = em; }
    void propsChanged(void);
    Boolean isMsgValid(void);	// checks to see whether _msgHandle is NULL
    
    void setLastAttBP();
    void setLastAttBP(DtMail::BodyPart *);
    void setFirstBPHandled(Boolean);

    void attachmentFeedback(Boolean);

    // The set/get Header methods manage the dynamic header structures.
    //
    void setHeader(const char * name, const char * value, 
		DtMailBoolean append_header);
    void setHeader(const char * name, DtMailValueSeq & value, 
		DtMailBoolean append_header);
    void resetHeaders(void);
    void resetAliasMenus(void);
    void setInputFocus(const int mode);
    HeaderList *getFirstHeader();
    HeaderList *getNextHeader();
    void nextTextField(Widget);

    char *getTemplateFile() { return _templateFile; }
    char *getTemplateFullPath() { return _templateFullPath; }
    char *getDraftFile() { return _draftFile; }
    void setTemplateFile(char *file) { _templateFile = file; }
    void setTemplateFullPath(char *file) { _templateFullPath = file; }
    void setDraftFile(char *file) { _draftFile = file; }

    // The load/store Headers methods will reload the pane from a message,
    // or store the header pane values to the message. Specifying NULL will
    // load the header pane from the current value for _msgHandle.
    // If load_all is true, then all headers are loaded, and fields are
    // added as necessary to the header pane. Otherwise, the list of shown
    // headers are loaded and all others are ignored.
    //
    // storeHeaders transfers the currently stored headers to the specified
    // message object, or the internal message if none is specified.
    //
    void loadHeaders(DtMail::Message * msg = NULL,
	     DtMailBoolean load_all = DTM_FALSE,
		 DtMailBoolean append_header = DTM_FALSE);
    void storeHeaders(DtMail::Message * msg = NULL);

    // Check if a message has addressees.  If a message is Sent and
    // it has no addressees, we need to barf.

    Boolean hasAddressee();

    // The changeHeaderState method toggles the dynamic headers between
    // shown an hidden. It will also update the menu label accordingly.
    //
    void changeHeaderState(const char * name);

    // The set/clear Status methods will set and clear the status line.
    //
    void setStatus(const char * str);
    void clearStatus(void);

    // a function for saving your self for WM_SAVE_YOURSELF protocol
    void saveYourSelf(void);
    
    // These are public so that the check point routine can call
    // updateMsgHnd, and include/forward routines can call updateMsgHndAtt.
    void updateMsgHnd();
    void updateMsgHndAtt();
    
    char *text();
    
    DtMailEditor* get_editor() {return _my_editor; }
    
    // Mutators
    
    void setInclMsgHnd(DtMail::Message *, Boolean);
    void text( const char * );
    void append( const char * );
    void quit();

    // Method to check if self has content in it.
    Boolean checkDirty();
    Boolean handleQuitDialog();
    void    goAway(Boolean);

    // Add the specified file (first parameter) as attachment.
    void inclAsAttmt( char *, char *);
    // Add the content of the buffer as attachment.
    void inclAsAttmt( unsigned char *, int, char *);
    // Parse the buffer and fill the Compose window with data.
    void parseNplace( char *, int );
    // Parse the file and fill the Compose window with data.
    void parseNplace(const char * path);
    
    // Load the dead letter file.
    void loadDeadLetter(const char * path);
    
    // SR - Text-selection callbacks.
    
    virtual void text_selected();
    virtual void text_unselected();
    void attachment_selected();
    void all_attachments_deselected();
    void all_attachments_selected();
    void selectAllAttachments(); 
    
    
    DtMailGenDialog *genDialog() { return _genDialog; }  
    
    void showAttachArea();
    void hideAttachArea();
    
    void activate_default_attach_menu();
    void deactivate_default_attach_menu();
    
    // Activate and deactivate paste stuff
    void activate_edit_paste() { _edit_paste->activate(); }
    void activate_edit_paste_indented() {_edit_paste_special[0]->activate();}
    void activate_edit_paste_bracketed() {_edit_paste_special[1]->activate();}
    
    void delete_selected_attachments();
    
    void undelete_last_deleted_attachment();
    
    void save_selected_attachment(char *);
    void save_selected_msg_text(char *);
    
    Boolean renameAttachmentOK();
    
    void addAttachmentActions(char **, int);
    void removeAttachmentActions();
    void invokeAttachmentAction(int);   

    virtual void manage();
    virtual void unmanage();

    Widget getTextW() { return _textW; }
    static void editorContentsChangedCB(Widget, XtPointer, XtPointer);

};


// Manager of all compose windows.
class Compose {
  friend void SendMsgDialog::quit();
    
  public:
    virtual const char *const className () { return "Compose"; }
    virtual ~Compose();
    Compose();
    SendMsgDialog *getWin(Tt_message = NULL);
    SendMsgDialog *SessiongetWin();
    void Self_destruct( XtPointer, XtIntervalId * );
    void fitIntoList(SendMsgDialog *, Boolean);
    SendMsgDialog* getAnUnusedSMD();
  
// Number of Compose windows on the recycle list.

    int numNotInUse() { return _not_in_use; }
    XtIntervalId getTimeOutId() { return _timeout_id; }
    
  private:
    XtIntervalId _timeout_id;   // Self destruct id.
    struct Compose_Win {
	    SendMsgDialog *win;
	    struct Compose_Win *next;
	    Boolean in_use;
	};

// Recycle list pointers.
    Compose_Win *_compose_head;
  
    int _not_in_use;

// Number of Compose windows created.
    int _num_created;
};

extern Compose theCompose;

#ifdef DTMAIL_TOOLTALK
extern int started_by_tt;
#endif

#endif


