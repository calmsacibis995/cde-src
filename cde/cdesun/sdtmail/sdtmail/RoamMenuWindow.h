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

#ifndef ROAMMENUWINDOW_H
#define ROAMMENUWINDOW_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)RoamMenuWindow.h	1.120 05 Apr 1995"
#endif

//-----------------------------------------------------------------------------
    // Roam window for viewing messages
//-----------------------------------------------------------------------------

//#include <DtMail/DtMail.hh>
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/MailRc.hh>
#include <SDtMail/Error.hh>
#include <SDtMail/MessageStore.hh>
#include <SDtMail/Message.hh>
#include <SDtMail/MessageEnvelope.hh>

#include "MenuWindow.h"
#include "MenuBar.h"
#include "DtMailGenDialog.hh"
#include "NoOpCmd.h"
#include "QuitCmd.h"
#include "UndoCmd.h"
#include "IconifyCmd.h"
#include "CmdList.h"
#include "RoamCmds.h"
#include "ViewMsgDialog.h"
#include "MsgScrollingList.hh"
#include "AttachArea.h"
#include "FindDialog.h"
#include "Views.h"
#include "Editor.hh"

#include <Tt/tt_c.h>

//#include "CDEM_CoeEd.hh"

#include "DtMailEditor.hh"
#include "DtMailTypes.hh"

#include "Undelete.hh"

/*  ROMENU_*       msgid 400 - 499
 */
#define ROMENU_NOINIT    400
#define ROMENU_NEWM      401
#define ROMENU_STAT      402
#define ROMENU_SUM       403
#define ROMENU_SEL       404
#define ROMENU_NOOPEN    405
#define ROMENU_NOCREAT   406
#define ROMENU_NOCRT     407
#define ROMENU_CREAT     408
#define ROMENU_LOAD      409
#define ROMENU_INFO      410
#define ROMENU_EMPTYC    411
#define ROMENU_NEXT      412
#define ROMENU_PREV      413
#define ROMENU_PRT       414
#define ROMENU_MVCP      415
#define ROMENU_SENDER    416
#define ROMENU_SUB       417
#define ROMENU_DATE      418
#define ROMENU_SIZE      419
#define ROMENU_OPENBOX   420
#define ROMENU_NEW       421
#define ROMENU_OPEN      422
#define ROMENU_EMPTY     423
#define ROMENU_SAVEAS    424
#define ROMENU_CLOSE     425
#define ROMENU_OPMSG     426
#define ROMENU_SAVEMSG   427
#define ROMENU_PRINT     428
#define ROMENU_DEL       429
#define ROMENU_LAST      430
#define ROMENU_LIST      431
#define ROMENU_UNDEL     432
#define ROMENU_MSG       433
#define ROMENU_UNDMSG    434
#define ROMENU_CPTO      435
#define ROMENU_COPY      436
#define ROMENU_SELALL    437
#define ROMENU_PROP      438
#define ROMENU_EDIT      439
#define ROMENU_FDMSG     440
#define ROMENU_FULL      441
#define ROMENU_ABB       442
#define ROMENU_BYDATE    443
#define ROMENU_BYSEND    444
#define ROMENU_BYSUB     445
#define ROMENU_BYSIZE    446
#define ROMENU_BYSTAT    447
#define ROMENU_VIEW      448
#define ROMENU_HDR       449
#define ROMENU_NEWMSG    450
#define ROMENU_NEWINCL   451
#define ROMENU_FORWARD   452
#define ROMENU_REPLY     453
#define ROMENU_RPYALL    454
#define ROMENU_RPYINCL   455
#define ROMENU_RPYAINCL  456
#define ROMENU_COMP      457
#define ROMENU_VAC       458
#define ROMENU_ONITEM    459
#define ROMENU_ONAPP     460
#define ROMENU_ONVER     461
#define ROMENU_HELP      462
#define ROMENU_LOADING   463
#define ROMENU_CON       464
#define ROMENU_DELMSG    465
#define ROMENU_NOEMPTY   466
#define ROMENU_EMPTY_CONT     467
#define ROMENU_EMPTY_NOFILE   468
#define ROMENU_MOVE	469
#define ROMENU_RELNOTES	470
    
struct OpenStruct {
    RoamMenuWindow *rmw;
    Cmd *command;
};

struct MsgNumberStruct {
    SdmMessageNumber msgno;
    MsgNumberStruct *next;
};

struct RenameStruct {
    RoamMenuWindow *rmw;
    char *server;
    char *filename;
    Widget dialog;
};

typedef struct {
    Boolean  new_mail;
} MailStatus;

class RoamMenuWindow : public MenuWindow, public AbstractEditorParent {
  public:	
    RoamMenuWindow ( const char *,
		     Tt_message = NULL,
		     SdmToken * = NULL,
		     SdmBoolean = Sdm_False);
    virtual	~RoamMenuWindow();  
    virtual	void 	initialize(int, char **);
    virtual void	manage();

    // class name 
    virtual     const char *const className() { return "RoamMenuWindow"; }

    // Popup Menu Event Handler
    static void MenuButtonHandler(Widget, XtPointer, XEvent *, char *); 
    static void map_menu( Widget, XtPointer, XtPointer );
    static void map_open_menu( Widget, XtPointer, XtPointer );
    static void setup_open_menu(void *);
    static void setup_open_other_menu(void *);
    static void setup_copy_menu(void *);
    static void setup_template_menu(void *);
    static void setup_draft_menu(void *);
    static void setup_move_menu(void *);
    static void setup_move_other_menu(void *);
    static void setup_move_popup_menu(void *);
    static void setup_move_popup_other_menu(void *);
    static void setup_text_popup_menu(void *);
  
    // DND: Drop Site
    static void msgListTransferCallback( Widget, XtPointer, XtPointer );
    void msgListDropRegister();
    void msgListDropEnable();
    void msgListDropDisable();

    void attachmentFeedback(SdmBoolean);

    // DND: Drag Source
    Widget getDragIcon(Widget widget);
    static XtActionProc msgListDragStart(Widget, XEvent*, String*, Cardinal*);
    static void msgListConvertCallback( Widget, XtPointer, XtPointer);
    static void msgListDragFinishCallback(Widget, XtPointer, XtPointer);
    void msgListDragSetup();

    void  propsChanged(void);
    void	setVacationTitle(void);
    void	removeVacationTitle(void);
    void	open(SdmError &);
    void	open_internal(SdmError &);
#ifdef OPEN_ASYNC
    void	finishOpen(SdmMessageStore *);
#endif
    SdmBoolean  isLocal() { return _is_local; }
    SdmBoolean  GoAway() { return _go_away; }  // Should this rmw go away?
    SdmBoolean  mailBoxWritable() { return _is_writable; }
    void	setLocal(SdmBoolean local) { _is_local = local; }
    void	setGoAway(SdmBoolean goaway) { _go_away = goaway; }
    void	setWritable(SdmBoolean writable) {_is_writable = writable; }
    void	setServerDisconnect(SdmBoolean disconnected) 
      { _server_disconnected = disconnected; }
    void 	restart (SdmBoolean r) { _restart = r; };

    void	open_and_load(SdmError &);
    void    setupReadOnlyMailbox(SdmBoolean postWarning);
    
    SdmBoolean	vacation();
    virtual	void	quit();
    void	Full( SdmBoolean );
    SdmBoolean	mail_file_selection( const char *, const char * = NULL );
    void	create_new_container(char* server, char * filename);
    void	rename_mailbox(char * server, char * filename);
    void	rename_mailbox_cleanup(Widget, struct RenameStruct *);
    void	delete_mailbox(char * server, char * filename);
    void	load_mailbox(SdmError & mail_error);
    void	postErrorDialog(SdmError &);

    void	checkForMail(SdmError & error);
	

// Commented out because we no longer need to check for new mail.
// The BE does it now.    
//	void	startTimer();
//	void	timeout( XtIntervalId *);

//-------------------------------------------------------------------------
//		Accessors
//-------------------------------------------------------------------------

    DtMailEditor* get_editor() { return(my_editor); }

    ViewMsgDialog * msgView()
      { if ( _numDialogs==0 )
	    return 0;
	  else
	      return _dialogs[_numDialogs-1];
	}
	
    FindDialog	*get_find_dialog();
    Views	*get_views_dialog(SdmBoolean, SdmBoolean create = Sdm_True);
	
    SdmMessageStore	*mailbox() { return _mailbox; }
    MenuBar	*menuBar(){ return _menuBar; }
    MsgScrollingList *list() { return _list; }
    DtMailGenDialog *genDialog() { return _genDialog; }
    Widget workArea() { return _workArea;} 
    //char *print_script() { return _print_script; }

    SdmBoolean	fullHeader() { return _fullHeader; }

    char        *mailboxName() { return _mailbox_name; }
    Cmd *printCmd()    { return _print; }
    Cmd *openCmd() { return _open; }

    Cmd *previousCmd() { return _previous; }
    Cmd *nextCmd() { return _next; }
    //  Cmd *todayCmd() { return _today; }
    
    OpenContainerCmd *openContainerCmd() { 
	return _openContainerCmd;
    }
    
    Cmd *lastSortCmd() { return _lastSortCmd; }
    void setLastSortCmd(Cmd* cmd) { _lastSortCmd = cmd; }
    Cmd *findCmd() { return _find; }
    Cmd *comp_newCmd() { return _comp_new; }
    Cmd *forwardCmd() { return _forward; }
    unsigned int	x() { return _x; }
    unsigned int	y() { return _y; }
    unsigned int	width() { return _width; }
    unsigned int	height() { return _height; }
    char	*mailbox_fullpath() {return _mailbox_fullpath; }
    SdmToken *token() { return _token; }
    
	
//-----------------------------------------------------------------------
// Mutators
//-----------------------------------------------------------------------
	    
    void	set_find_dialog(FindDialog *dialog) { _findDialog = dialog; }
    void	setStatus(const char *);
    void	showToolbarHelp(int);
    void	clearStatus(void);
    void	message( char * );
    void	message_summary();
    void	message_summary(
				int msg_num,
				int num_msgs, 
				int num_new, 
				int num_deleted
				);
    void	message_selected(
				 int msg_num, 
				 int num_msgs, 
				 int num_new, 
				 int num_deleted
				 );
    void	showMessageFullHeaders(SdmBoolean);
    void	mailboxName( char *name);
    void	title(const char *);
    SdmBoolean	inbox();

    void	fullHeader( SdmBoolean flag);

    void	registerDialog  ( ViewMsgDialog * );
    void	unregisterDialog( ViewMsgDialog * );  
    
    // SR - Added methods below

    ViewMsgDialog* ifViewExists(SdmMessageNumber msg_num);
    void delete_message(SdmMessageNumber msgno);


    void	buildToolbar();
    CmdList*	getToolbarCmds() { return _toolbar_cmds;}
    DtVirtArray<char *>* getToolbarIcons() { return _toolbar_icons; }
    DtVirtArray<long>* getToolbarIndices() { return _toolbar_indices; }
    void	setToolbarIndices(DtVirtArray<long> *);
    void	setMailboxIndex(int i) { _mailbox_index = i; }

    void  	addToRowOfLabels(MsgScrollingList *msglist, int);
    void  	addToRowOfMessageStatus();
    
    void  construct_file_menu();
    void  construct_message_menu();
    void  construct_edit_menu();
    void  construct_view_menu();
    void  construct_compose_menu();
    void  construct_options_menu();
    void  construct_move_menu();
    void  construct_attachment_menu();
    void  construct_attachment_popup(void);
    void  construct_text_popup(void);
    void  construct_message_popup();
    void  construct_help_menu();

    void  create_toolbar_cmdlist();

    CmdList *construct_move_menu_cmdlist(CmdList *, SdmBoolean );
    void  addFoldersFiles(Widget, CmdList*);
    UndelFromListDialog *get_undel_dialog() { 
	return _undeleteFromList->dialog(); }
    SdmMailRc *get_mail_rc();
    int  inList(char *filename, DtVirtArray<MoveCopyMenuCmd *> *);
    int  inList(char *filename, DtVirtArray<OpenMailboxCmd *> *);
    void  addToRecentList(char *filename);
    void  addToRecentOpenList(char *openfile);
    
    void setToNormalIcon();
    void setToEmptyIcon();
    void  newMailIndicators(void);

    void expunge(void);

    SdmBoolean requiredConversion() {return _requiredConversion;}
    
    // SR - Text-selection callbacks.
    
    virtual void text_selected();
    virtual void text_unselected();

    void	attachment_selected();
    void	all_attachments_deselected();
    void	all_attachments_selected();
    void	selectAllAttachments();
    void	add_att(const char *) { ; }
    void	add_att(const char *, SdmString) { ; }
    void	add_att(SdmString) { ; }

    void	activate_default_attach_menu();
    void	deactivate_default_attach_menu();
    void	activate_default_message_menu();
    void	deactivate_default_message_menu();
    void	activate_multi_select_menu();
    void	deactivate_multi_select_menu();
    
    void	showAttachArea();
    void	hideAttachArea();
       
    void	addAttachmentActions(
				    char **,
				    int
		);
    void	removeAttachmentActions();
    void	invokeAttachmentAction(int);

    void	save_selected_attachment(char *);
    
    int		showConversionStatus(int, int);
    
    void	conversionFinished();
    void	convert(char *, char *);
    
    void	resetCacheList(int new_size);

				    
#ifdef DOLATER
    static int ConvertStatusCB(
				int current, 
				int total, 
				void *);
#endif // DOLATER

	// save your session
    virtual void saveYourSelf(void);

	// Post the message popup
    virtual void postMsgsPopup(XEvent *event);

    inline SdmBoolean IsLoaded () {return _is_loaded;};
    void passwd_dialog_up(SdmBoolean bool) { _passwd_dialog_up = bool; } ;
    OpenMailboxCmd* openInbox() { return _open_inbox; } ;
    DtVirtArray<MoveCopyMenuCmd *>*  moveContainerList () {return _move_containerlist;};
    DtVirtArray<MoveCopyMenuCmd *>*  cachedmoveContainerList () {return _cached_move_containerlist;};
    int startedIconic() { return _startIconic; };
    void setIconic(int i) { _startIconic = i; };
    void setServer( char *s) { _server = s; };
    static void cancelcb(struct OpenStruct *);
    char *ImapFolder() { return _imapfolder; }; 

 protected:
	
    SdmBoolean	_fullHeader;
    char	*_mailbox_name;
    char	*_mailbox_fullpath;
    char	*_mail_files;
    SdmBoolean 	_requiredConversion;
    SdmToken    *_token;
    
    int		_numDialogs;
    ViewMsgDialog	**_dialogs;
    FindDialog	*_findDialog;
    Views *_viewsDialog;
    int	_check_mail_interval;
    
    Cmd *_manage;
    Cmd *_separator;
    
    // Mailbox Menu
    
    CheckForNewMailCmd *_check_new_mail;
    Cmd *_msg_select_all;
    CustomSelectFileCmd *_new_container;
    CustomSelectFileCmd *_rename_mailbox;
    CustomSelectFileCmd *_delete_mailbox;
    OpenMailboxCmd *_open_inbox;
    OpenMailboxCmd *_logfile;
    SelectFileCmd *_open;
    Cmd *_destroyDeletedMessages;
    Cmd *_quit;
    Cmd *_print;
    CmdList *_openCmdlist;

    ChooseCmd *_choose;
    DeleteCmd *_delete;

    // Message Menu
    
    Cmd *_open_msg;
    Cmd *_edit_copy;
    Cmd *_select_all;
    Cmd *_save_msg;
    Cmd *_save_msg_as;
    Cmd *_print_msg;
    Cmd *_delete_msg;
    Cmd *_undeleteLast;
    UndeleteCmd *_undeleteFromList;
    
    CmdList *_copy_to;
    Widget _copyMenu;
    CopyToInboxCmd *_copy_inbox;
    int _first_cached_copy_item;
    DtVirtArray<MoveCopyMenuCmd *> *_copy_containerlist;
    DtVirtArray<MoveCopyMenuCmd *> *_cached_copy_containerlist;
    DtVirtArray<MoveCopyMenuCmd *> *_movecopymenucmd_list;
    char 	*_filemenu2;
    char 	*_folder;
    char 	*_imapfolder;
    char 	*_logfilename;
    Widget _message_cascade;
    Widget _file_cascade;
    Widget _openMenu;

    DtVirtArray<OpenMailboxCmd *> *_cached_open_containerlist;
    DtVirtArray<OpenMailboxCmd *> *_openmailboxcmd_list;
    int _first_cached_open_item;
    
    // View Menu  
    
    Cmd *_next;
    Cmd *_previous;
    
    Cmd *_find;
    Cmd *_show_views;
    Cmd *_abbrevHeaders;
    // Add Change Charset Command
    Cmd *_charset;
    
    Cmd *_lastSortCmd;  
    
    
    // Compose Menu
    
    Cmd *_comp_new;
    Cmd *_preply;
    Cmd *_forward;
    Cmd *_forwardBody;
    
    Cmd *_sortTD;
    Cmd *_sortSender;	
    Cmd *_sortSubject;	
    Cmd *_sortSize;	
    Cmd *_sortStatus;
    // Cmd *_sortMsgno;
    
    Cmd *_replySender;
    Cmd *_replyAll;
    Cmd *_replySinclude;
    Cmd *_replyAinclude;
    CmdList *_templates;
    CmdList *_drafts;
    Widget _compose_cascade;
    
    // Move Menu
    Widget _moveMenu;
    CmdList *_move_menu_cmdList;
    CmdList *_move_popup_menu_cmdList;
    Widget _move_cascade;
    MoveToInboxCmd *_move_inbox;
    DtVirtArray<MoveCopyMenuCmd *> *_move_containerlist;
    DtVirtArray<MoveCopyMenuCmd *> *_cached_move_containerlist;
    int _cached_list_size;
    int _max_cached_list_size;
    int _user_list_size;
    int _first_cached_move_item;

    // Options Menu
    Cmd *_opt_basic;
    Cmd *_opt_tb;
    Cmd *_opt_mv;
    Cmd *_opt_mf;
    Cmd *_opt_no;
    Cmd *_opt_cw;
    Cmd *_opt_sig;
    Cmd *_opt_vac;
    Cmd *_opt_td;
    Cmd *_opt_al;
    Cmd *_opt_ad;

    // Attachment Pullright
    
    MenuBar   *_attachmentMenuPR;

    CmdList *_attachmentMenuList;
    CmdList *_attachmentPopupMenuList;
    Cmd *_attachment_popup_title;
    CmdList *_textPopupMenuList;
    Cmd *_text_popup_title;

    // Message Popup

    CmdList *_attachmentActionsList;
    Widget _msgsPopupMoveMenu;
    CmdList * _msgsPopupMenuList;
    Cmd *_msgs_popup_title;

    MenuBar *_menuPopupMsgs;
    Widget   _msgsPopupMenu;

    Cmd *_save_attachment;
    Cmd *_delete_attachment;
    Cmd *_undelete_attachment;
    Cmd *_rename_attachment;
    Cmd *_select_all_attachments;

    // Help Menu
    Cmd *_overview;
    Cmd *_tasks;
    Cmd *_reference;
    Cmd *_messages;
    Cmd *_on_item;
    Cmd *_using_help;
    Cmd *_about_mailer;
    
    // Message filing interface
    
    OpenContainerCmd    *_openContainerCmd;
    
    // Roam junk.  Can be thrown away? - SR
    
    Cmd *_undeleted;
    

    Cmd *_hide_show_button;
    Widget _hide_show;
    MoveCopyCmd *_move_copy_button;
    MoveCopyCmd *_copy_move_button;
    CmdList *_open_other_mboxes;
    CmdList *_move_other_mboxes;
    CmdList *_move_popup_other_mboxes;
    CmdList *_copy_other_mboxes;

    CmdList *_mailbox_cmdList;
    CmdList *_message_cmdList;
    CmdList *_compose_cmdList;
    CmdList *_view_cmdList;
    CmdList *_option_cmdList;
    CmdList *_help_cmdList;
    
    Widget createWorkArea ( Widget );
    void   createMenuPanes();
    void   createCopyList(CmdList *);
    SdmMessageStore	*_mailbox;
    SdmBoolean _inbox;
    MsgScrollingList *_list;
    DtMailGenDialog *_genDialog;
    char *getServer() { return _server; };
    
    void  configurenotify( 
		unsigned int, 
		unsigned int,
		unsigned int, 
		unsigned int,
		unsigned int);

    void  mapnotify();

    
  private:

    SdmBoolean _tmp_file;
    SdmBoolean _async;	// Indicates how the message store is being opened
    SdmBoolean _is_opening_and_loading; // indicates we are in the process 
                                        // of opening and loading the mailbox.
    SdmBoolean _isstartup;   // Indicates we are at the window startup so we
			     // can delay menu creation for startup performance 
    SdmMessageNumber _initial_nmsgs;
    int _mailbox_index;
    CmdList *_toolbar_cmds;
    DtVirtArray<char *> *_toolbar_icons;
    DtVirtArray<char *> *_toolbar_inactive_icons;
    char *_toolbar_mailrc_string;
    SdmBoolean _toolbar_use_icons;
    static DtVirtArray<long> *_toolbar_indices;  //indices of _toolbar_cmds
				// that actually show up in the toolbar.
    
    static XtResource _resources[];
    static char *_supported[];

    Pixmap _mbox_image;
    Pixmap _mbox_mask;

    unsigned int _x;
    unsigned int _y;
    unsigned int _width;
    unsigned int _height;
    unsigned int _border_width;
    // Widget _filing;
    // Widget _text;
    
    DtMailEditor *my_editor;
    Widget _message, _message_summary;
    Widget rowOfLabels, rowOfButtons;
    Widget rowOfMessageStatus;
    XtIntervalId _timer_id;
    SdmBoolean _is_loaded;
    SdmBoolean _is_local;
    SdmBoolean _go_away;            // Setting this to true means get rid of this rmw.
    SdmBoolean _is_writable;        // True if the mailbox is writable, false if readonly.
    SdmBoolean _passwd_dialog_up;
    SdmBoolean _server_disconnected;
    SdmBoolean _restart;
    Atom       _SUN_MAIL_STATUS;    // This is the atom used to notify the front panel
				    // of new mail.
    int        _lock_status;        // This tells us if we have a lock on the new mail
				    // atom so that we can tell the front panel that we
				    // have new mail.
    
    SdmBoolean _open_create_flag;
    SdmBoolean _open_lock_flag;

    static void okcb(void *);
    static void toolbarEH( Widget, XtPointer, XEvent *, char *);
    static void ownselectionCallback( Widget, XtPointer, XtPointer );
    static void structurenotify( Widget, XtPointer, XEvent *, char *);
    static void file_selection_callback( void *, char * );
    static void open_callback( void *, char * );
    static void create_container_callback( void *, char * );
    static void rename_mailbox_callback( void *, char * );
    static void delete_mailbox_callback( void *, char * );
    static void	rename_mailbox_cancel(Widget, XtPointer, XtPointer);
    static void	rename_mailbox_ok(Widget, XtPointer, XtPointer);
    static void move_callback( void *, char *);
    static void copy_callback( void *, char *);

    static void save_attachment_callback( void *, char *);    

    static void conversionFinishedCallback(
			RoamInterruptibleCmd *,
			SdmBoolean,
			void *);
    
    SdmMessageNumber get_msgno_after_expunge(SdmMessageNumber msgnum,
			SdmMessageNumberL& expungedMsgs);
    void 	     add_to_recent_msgs(SdmMessageNumber msgno);

    SdmBoolean notInList(char *str, CmdList *list);
    int _islocal; // used for FSB search

    char *_server;
    int _startIconic;

    MsgNumberStruct *_recent_msgs_head;
    MsgNumberStruct *_recent_msgs_tail;
    long             _recent_msgs_cnt;
    SdmBoolean       _expunging;

    void resetToken();
};

#endif

