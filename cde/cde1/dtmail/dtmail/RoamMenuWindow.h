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

#include <DtMail/DtMail.hh>

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
#include <DtMail/DtMailError.hh>
#include "MsgScrollingList.hh"
#include "AttachArea.h"
#include "FindDialog.h"
#include "Editor.hh"

#include <Tt/tt_c.h>

//#include "CDEM_CoeEd.hh"

#include "DtMailEditor.hh"

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
    
class RoamMenuWindow : public MenuWindow, public AbstractEditorParent {
  public:	
    RoamMenuWindow ( char *, Tt_message = NULL );
    virtual	~RoamMenuWindow();  
    virtual	void 	initialize();
    virtual void	manage();

    // class name 
    virtual     const char *const className() { return "RoamMenuWindow"; }

    // Popup Menu Event Handler
    static void MenuButtonHandler(Widget, XtPointer, XEvent *, Boolean *); 
    static void map_menu( Widget, XtPointer, XtPointer );
    static void map_open_menu( Widget, XtPointer, XtPointer );
  
    // DND: Drop Site
    static void msgListTransferCallback( Widget, XtPointer, XtPointer );
    void msgListDropRegister();
    void msgListDropEnable();
    void msgListDropDisable();

    void attachmentFeedback(Boolean);

    // DND: Drag Source
    Widget getDragIcon(Widget widget);
    static XtActionProc msgListDragStart(Widget, XEvent*, String*, Cardinal*);
    static void msgListConvertCallback( Widget, XtPointer, XtPointer);
    static void msgListDragFinishCallback(Widget, XtPointer, XtPointer);
    void msgListDragSetup();

    void  propsChanged(void);
    void	setVacationTitle(void);
    void	removeVacationTitle(void);
    void	open(DtMailEnv &, DtMailBoolean, DtMailBoolean);
  
    //void	open_and_load(CMEnv &, DtMailBoolean, DtMailBoolean);
    // Returns ME_NoError, ME_NoMailBox, 
    // ME_ConvertContainerRefused


    void	open_and_load(DtMailEnv &, DtMailBoolean, DtMailBoolean);
    // Returns ME_NoError, ME_NoMailBox, 
    // ME_ConvertContainerRefused

    Boolean	vacation();
    virtual	void	quit();
    void	quit_silently();
    void	Full( Boolean );
    void	mail_file_selection( char * );
    void	create_new_container( char * );
    void	load_mailbox(DtMailEnv & mail_error);
    void	postErrorDialog(DtMailEnv &);

    void	checkForMail(DtMailEnv & error) {
	_we_called_newmail = TRUE;
	_mailbox->checkForMail(error);
    }
	

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
	
    DtMail::MailBox	*mailbox() { return _mailbox; }
    MenuBar	*menuBar(){ return _menuBar; }
    MsgScrollingList *list() { return _list; }
    DtMailGenDialog *genDialog() { return _genDialog; }
    Widget workArea() { return _workArea;} 
    //char *print_script() { return _print_script; }

    Boolean	fullHeader() { return _fullHeader; }

    char        *mailboxName() { return _mailbox_name; }
    Cmd *printCmd()    { return _print; }
    Cmd *openCmd() { return _open; }

    Cmd *previousCmd() { return _previous; }
    Cmd *nextCmd() { return _next; }
    //  Cmd *todayCmd() { return _today; }
#ifdef DEAD_WOOD
    ConvertContainerCmd *convertContainerCmd() { 
	return _convertContainerCmd;
    }
#endif // DEAD_WOOD
    
    OpenContainerCmd *openContainerCmd() { 
	return _openContainerCmd;
    }
    
    Cmd *sortCmd() { return _sort; }
    Cmd *findCmd() { return _find; }
    Cmd *comp_newCmd() { return _comp_new; }
    Cmd *forwardCmd() { return _forward; }
    unsigned int	x() { return _x; }
    unsigned int	y() { return _y; }
    unsigned int	width() { return _width; }
    unsigned int	height() { return _height; }
    char	*mailbox_fullpath() {return _mailbox_fullpath;}
	
//-----------------------------------------------------------------------
	    // Mutators
//-----------------------------------------------------------------------
	    
    void	set_find_dialog( FindDialog *dialog ) 
  { _findDialog = dialog; }
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
    void	showMessageFullHeaders(Boolean);
    void	mailboxName( char *name);
    void	title(const char *);
    Boolean	inbox();

    void	fullHeader( Boolean flag);

    void	registerDialog  ( ViewMsgDialog * );
    void	unregisterDialog( ViewMsgDialog * );  
    
    // SR - Added methods below


    ViewMsgDialog*
    ifViewExists(DtMailMessageHandle msg_num);

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

    CmdList *construct_move_menu_cmdlist(CmdList *, Boolean );
    void  addFoldersFiles(Widget, CmdList*);
    UndelFromListDialog *get_undel_dialog() { 
	return _undeleteFromList->dialog(); }
    DtMail::MailRc *get_mail_rc();
    void clear_message();
    int  inList(char *filename, DtVirtArray<MoveCopyMenuCmd *> *);
    int  inList(char *filename, DtVirtArray<OpenMailboxCmd *> *);
    void  addToRecentList(char *filename);
    void  addToRecentOpenList(char *openfile);
    
    void  newMailIndicators(void);

    void expunge(void);

    Boolean requiredConversion() {return _requiredConversion;}
    
    // SR - Text-selection callbacks.
    
    virtual void text_selected();
    virtual void text_unselected();

    void	attachment_selected();
    void	all_attachments_deselected();
    void	all_attachments_selected();
    void	selectAllAttachments();
    void	add_att(char *) { ; }
    void	add_att(char *, DtMailBuffer) { ; }
    void	add_att(DtMailBuffer) { ; }

    void	activate_default_attach_menu();
    void	deactivate_default_attach_menu();
    
    void	activate_default_message_menu();
    void	deactivate_default_message_menu();	

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

    // syncViewAndStore() does the sync-ing of the view of a mail
    // container and the storage of that container.
    // E.g., this callback and therefore the method gets invoked every 
    // time a message gets expunged by the back end based on "timed delete".
    // 
    // The method needs to then remove the expunged message from the
    // deleted messages list, thereby syncing the view to be always
    // current with the storage.
    // Similarly, the method also gets invoked when the container store
    // has received new mail.  The view then needs to be updated....

    DtMailBoolean	syncViewAndStore(DtMailCallbackOp, va_list );

	// Static public methods now.

    static DtMailBoolean syncViewAndStoreCallback(
				DtMailCallbackOp op,
				const char * path,
				const char * prompt_hint,
				void * client_data,
				...);
				    
    static int ConvertStatusCB(
				int current, 
				int total, 
				void *);

	// save your session
    virtual void saveYourSelf(void);

	// Post the messasge popup
    virtual void postMsgsPopup(XEvent *event);

    inline Boolean IsLoaded () {return _is_loaded;};

 protected:
	
    Boolean	_fullHeader;
    char	*_mailbox_name;
    char	*_mailbox_fullpath;
    char	*_mail_files;
    Boolean 	_requiredConversion;
    Boolean	_we_called_newmail;
    
    int		_numDialogs;
    ViewMsgDialog	**_dialogs;
    FindDialog	*_findDialog;
    int	_check_mail_interval;
    
    Cmd *_manage;
    Cmd *_separator;
    
    // Mailbox Menu
    
    CheckForNewMailCmd *_check_new_mail;
    Cmd *_msg_select_all;
    SelectFileCmd *_new_container;
    OpenMailboxCmd *_open_inbox;
    OpenMailboxCmd *_logfile;
    SelectFileCmd *_open;
    Cmd *_destroyDeletedMessages;
    Cmd *_quit;
    Cmd *_print;

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
    char 	*_filemenu2;
    char 	*_folder;
    Widget _message_cascade;
    Widget _file_cascade;
    
    // View Menu  
    
    Cmd *_next;
    Cmd *_previous;
    
    Cmd *_find;
    Cmd *_abbrevHeaders;
    
    Cmd *_sort;  
    
    
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
    
#ifdef DEAD_WOOD
    ConvertContainerCmd *_convertContainerCmd;
#endif // DEAD_WOOD
    OpenContainerCmd    *_openContainerCmd;
    
    // Roam junk.  Can be thrown away? - SR
    
    Cmd *_undeleted;
    

    Cmd *_hide_show_button;
    Widget _hide_show;
    Cmd *_move_copy_button;
    Cmd *_copy_move_button;
    CmdList *_open_other_mboxes;
    CmdList *_move_other_mboxes;
    CmdList *_move_popup_other_mboxes;
    CmdList *_copy_other_mboxes;
    Cmd *_mailbox_list;
    
    Widget createWorkArea ( Widget );
    void   createMenuPanes();
    void   createCopyList(CmdList *);
    DtMail::MailBox	*_mailbox;
    Boolean _inbox;
    MsgScrollingList *_list;
    DtMailGenDialog *_genDialog;
    
    void  configurenotify( 
		unsigned int, 
		unsigned int,
		unsigned int, 
		unsigned int,
		unsigned int);

    void  mapnotify();

    
  private:

    int _mailbox_index;
    CmdList *_toolbar_cmds;
    DtVirtArray<char *> *_toolbar_icons;
    DtVirtArray<char *> *_toolbar_inactive_icons;
    char *_toolbar_mailrc_string;
    Boolean _toolbar_use_icons;
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
    Boolean clear_message_p;
    Boolean _is_loaded;
    
    Boolean _open_create_flag;
    Boolean _open_lock_flag;

    static void toolbarEH( Widget, XtPointer, XEvent *, Boolean *);
    static void ownselectionCallback( Widget, XtPointer, XtPointer );
    static void structurenotify( Widget, XtPointer, XEvent *, Boolean *);
    static void file_selection_callback( void *, char * );
    static void create_container_callback( void *, char * );
    static void move_callback( void *, char *);
    static void copy_callback( void *, char *);

    static void save_attachment_callback( void *, char *);    
    static void delete_attachment_callback( void *, char *);    

    static void conversionFinishedCallback(
			RoamInterruptibleCmd *,
			Boolean,
			void *);

// Ripping this out since we no longer do checking and retrieval of 
// new mail.  The BE does it now.
//	static void applicationTimeout ( XtPointer, XtIntervalId * );
    
	
    };

#endif



