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
#pragma ident "@(#)SendMsgDialog.h	1.134 06/11/97"
#else
static char *sccs__SENDMSGDIALOG_H = "@(#)SendMsgDialog.h	1.134 06/11/97";
#endif

#include <Tt/tt_c.h>
#include "DtMailEditor.hh"
#include "MenuWindow.h"
#include "NoUndoCmd.h"
#include "RoamCmds.h"
#include "DtMailGenDialog.hh"
#include "ComposeCmds.hh"
#include "MenuButton.h"
#include "RoamApp.h"
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Error.hh>
#include <SDtMail/Message.hh>
#include <SDtMail/MessageBody.hh>
#include <SDtMail/String.hh>


class MailBox;
class Cmd;

// This is the Compose window.
class SendMsgDialog : public MenuWindow, public AbstractEditorParent {
public:
  enum ShowState {SMD_ALWAYS, SMD_SHOWN, SMD_HIDDEN, SMD_NEVER};

  struct HeaderList {
    char  		*label;
    char  		*header;
    char		*value;
    ShowState	        show;
    Widget		form_widget;
    Widget		label_widget;
    MenuButton	        *mb_widget;
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

  SdmBoolean _show_attach_area;
  SdmBoolean _edits_made;
    
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
  Cmd  *_change_charset;
  Cmd *_format_find_change;
  Cmd   *_format_spell;
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
  SdmBoolean _updateAliasMenu;
  DtVirtArray<PropStringPair *> *_aliases;

  Widget _status_form;
  Widget _status_text;

  int _num_sendAs;    // Number of Transports available through Back End.

  SdmBoolean _isQuitDialogUp;

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

  // This is the internal data structure used for submission.  As user
  // adds/deletes attachment, this data structure is update immediately.
  // This data structure is also updated at periodic checkpoints to store
  // the latest Compose window data.  Before sending, this is also updated.
  SdmMessage *_msgHandle;      
        
  // When makeMessage() is called, it creates a message handle with its
  // first body part empty.  If set to TRUE, then this boolean variable
  // indicates that the first body part of _msgHandle has already been used
  // (probably for text).  Otherwise, the first body part can be used for attachment.
  SdmBoolean _firstBPHandled;
    
        
  // The auto save interval id is used to stop the interval timer for auto
  // saving.
  //
  XtIntervalId _auto_save_interval;
  char *_auto_save_path;
  char *_auto_save_file;
   
  SdmString _user_charset;

  SdmBoolean _log_msg;
  SdmBoolean _iconic_session;  // If this SMD is started from session and
                                 // initial state is iconic

  void mkAutoSavePath();
  static void autoSaveCallback(XtPointer, XtIntervalId *id);

  void doAutoSave(void);
  int getAutoSaveInterval(void);
  void writeAutoSave(void);
    
  void   createMenuPanes();   // Create all menu
  void addTemplates(CmdList *); // Create the initial template menu.
  void addAliases(); // Create the initial alias menu.
  void addAliasOption(); 
  void createAliasMenuButton(MenuButton **button, Widget form, char* label); 
  Widget createWorkArea( Widget );   // Create all widget, button
  void createHeaders(Widget header);
  void doDynamicHeaderMenus(void);
  void dumpHeaders(void);
  SdmBoolean reservedHeader(const char *label);

  // File selection Box callbacks 
  static void include_file_cb( void *, char * );
  static void add_att_cb( void *, char * );
  static void save_att_cb( void *, char * );

  // Callback for lazy mapping of the alias menu
  static void mapAliasMenu(Widget, XtPointer, XtPointer);

  static void setTearOffTitle(Widget, XtPointer, XtPointer);

  // Return a message handle -- internal data structure of a mail message.
  SdmMessage * makeMessage(SdmError &error);
  // Clear Compose window text fields to prepare for re-use.
  void reset();

  // Find a header in the list by name.
  int lookupHeader(const char * name);
  void reattachHeaders(void);
  void justifyHeaders(void);
  int _header_list_index;

  // Called when Submit returns an error
  static void submitErrorProc (DtMailGenDialog *genDialog, SdmError& error);
  void parseIt(const char *, int, SdmStrStrL *, SdmString *);

  struct SubmitContext {
      SendMsgDialog *smd;
      SdmBoolean deleteDraft;
      char *draft;
  };

public:
  SendMsgDialog(Tt_message = NULL);
  virtual ~SendMsgDialog();
  void createFormatMenu(); // Create the format menu
  void createFileMenu(); // Create the file menu
  virtual const char *const className () { return "SendMsgDialog"; }
  void initialize(int, char **);
  void appendSignature(SdmBoolean);
  void send_message(int);
  static void submitAsyncCB(SdmError&, void *, SdmServiceFunctionType, SdmEvent*);
  SdmBoolean parseFile(const char *, SdmStrStrL *, SdmString *);

  void include_file(char *);
  void add_att(const char *);
  void add_att(const char *, SdmString);
  void add_att(SdmString);
  void setMsgHnd();    // initialize _msgHandle.
  SdmMessage * getMsgHnd() { return _msgHandle; }
  void startAutoSave(void);
  void stopAutoSave(void);
  void setLogState(SdmBoolean state) { _log_msg = state; }
  void setEditsMade(SdmBoolean em) { _edits_made = em; }
  void propsChanged(void);
  SdmBoolean isMsgValid(void) const {return _msgHandle ? Sdm_True : Sdm_False;}

  void setFirstBPHandled(SdmBoolean);

  void attachmentFeedback(SdmBoolean);

  // The set/get Header methods manage the dynamic header structures.
  //
  void setHeader(const char * name, const char * value, 
                 SdmBoolean append_header);
  void setHeader(const char * name, SdmStringL & value, 
                 SdmBoolean append_header);
  void resetHeaders(void);
  void resetAliasMenus(void);
  void setInputFocus(const int mode);
  HeaderList *getFirstHeader();
  HeaderList *getNextHeader();
  void updateHeaders(void);
  void nextTextField(Widget);

  char *getTemplateFile() const { return _templateFile; }
  char *getTemplateFullPath() const { return _templateFullPath; }
  char *getDraftFile() const { return _draftFile; }
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
  void loadHeaders(SdmError &error, SdmMessage * msg = NULL,
                   SdmBoolean load_all = Sdm_False,
                   SdmBoolean append_header = Sdm_False);
  void storeHeaders(SdmError &error, SdmMessage * msg = NULL);

  // Check if a message has addressees.  If a message is Sent and
  // it has no addressees, we need to barf.
  SdmBoolean hasAddressee();

  // The changeHeaderState method toggles the dynamic headers between
  // shown an hidden. It will also update the menu label accordingly.
  void changeHeaderState(const char * name);

  // The set/clear Status methods will set and clear the status line.
  void setStatus(const char * str);
  void clearStatus(void);

  // a function for saving your self for WM_SAVE_YOURSELF protocol
  void saveYourSelf(void);
    
  char *text() const { return _my_editor->textEditor()->get_contents(True); }
    
  DtMailEditor* get_editor() {return _my_editor; }
  SdmBoolean checkDirty() const { return _edits_made ? Sdm_True : Sdm_False; }
    
  // Mutators
  void text( const char * );
  void append( const char * );

  // Method to check if self has content in it.
  SdmBoolean handleQuitDialog();
  void    goAway(SdmBoolean);
  void    quit() { goAway(Sdm_True); }

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
    
  // Text-selection callbacks.
  virtual void text_selected();
  virtual void text_unselected();
  void attachment_selected();
  void all_attachments_deselected();
  void all_attachments_selected();
  void selectAllAttachments(); 

  DtMailGenDialog *genDialog() const { return _genDialog; }  

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
    
  SdmBoolean renameAttachmentOK();
    
  void addAttachmentActions(char **, int);
  void removeAttachmentActions();
  void invokeAttachmentAction(int);   

  virtual void manage();
  virtual void unmanage();

  Widget getTextW() const { return _textW; }
  static void editorContentsChangedCB(Widget, XtPointer, XtPointer);

  void set_iconic_session() { _iconic_session = Sdm_True; }
  SdmBoolean is_iconic_session() { return _iconic_session; }

  void setCharacterSet(SdmString &charset) { _user_charset = charset; }
  SdmString &getCharacterSet() { return (_user_charset); }
};

// Manager of all compose windows.
class Compose {
  friend void SendMsgDialog::goAway(SdmBoolean);
  friend void SendMsgDialog::send_message(int);

public:
  virtual const char *const className () { return "Compose"; }
  virtual ~Compose();
  Compose();
  SendMsgDialog *getWin(Tt_message = NULL);
  SendMsgDialog *SessiongetWin(int, char**);
  void Self_destruct( XtPointer, XtIntervalId * );
  void fitIntoList(SendMsgDialog *, SdmBoolean);
  SendMsgDialog* getAnUnusedSMD();
  
  // Number of Compose windows on the recycle list.

  int numNotInUse() { return _not_in_use; }
  XtIntervalId getTimeOutId() { return _timeout_id; }
    
private:
  XtIntervalId _timeout_id;   // Self destruct id.
  struct Compose_Win {
    SendMsgDialog *win;
    struct Compose_Win *next;
    SdmBoolean in_use;
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
