/*
 *+SNOTICE
 *
 *	$Revision: 1.58 $
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

#ifndef VIEWMSGDIALOG_H
#define VIEWMSGDIALOG_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)ViewMsgDialog.h	1.58 08/06/96"
#endif

#include "DialogShell.h"
#include "Editor.hh"
#include "Dialog.h"
#include "AttachArea.h"
#include "DtMailEditor.hh"
#include "MenuWindow.h"
#include "MenuBar.h"
#include "DtMailGenDialog.hh"
#include "NoOpCmd.h"
#include "QuitCmd.h"
#include "UndoCmd.h"
#include "IconifyCmd.h"
#include "CmdList.h"
#include "RoamCmds.h"
#include <SDtMail/Error.hh>

// Forward declarations

class Cmd;
class RoamMenuWindow;


class ViewMsgDialog : public DialogShell, public AbstractEditorParent {

  public:
    
    ViewMsgDialog( RoamMenuWindow * );
    virtual ~ViewMsgDialog();
    
    virtual void initialize();
    virtual void quit();

    virtual const char *const className () { return "ViewMsg"; }
    void propsChanged(void);

// Accessors
    

//  AttachArea	*attachArea(){ return _attach; }
    SdmMessageNumber msgno() { return _msgno; }
    RoamMenuWindow *parent() { return _parent; };	 
    void attachmentFeedback(SdmBoolean);   

// Mutators
    void msgno( SdmMessageNumber msg_num ) { _msgno=msg_num; }
    void append(const char *, const unsigned long);
    void text(const char *, const unsigned long);
    
    void popped_down();
    void popped_up();
    
// SR - Added methods below

    void raise();
    DtMailEditor* get_editor();

    DtMailGenDialog *genDialog() { return _genDialog; }

    void    auto_show_cursor_off();
    void    auto_show_cursor_restore();
    void    set_to_top();

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

    void	save_selected_attachment(char *);	

    void	showAttachArea();
    void	hideAttachArea();

    void	activate_default_attach_menu();
    void	deactivate_default_attach_menu();

    void	addAttachmentActions(
				    char **,
				    int
		);
    void	removeAttachmentActions();
    void	invokeAttachmentAction(int);	  	

    virtual void manage();

    static void save_attachment_callback( void *, char *);    

  protected:

    void createMenuPanes();
    Widget createWorkArea( Widget );  
  
    void	construct_edit_menu();
    void	construct_attachment_menu();
    void	construct_attachment_popup(void);
    void	construct_text_popup(void);
    void  construct_help_menu();

  private:

    RoamMenuWindow *_parent;	
    Widget _workArea;
    Widget _main;

    MenuBar *_menuBar;
    DtMailGenDialog *_genDialog;

    Cmd*  _edit_copy;
    Cmd*  _edit_select_all;

    // Attachment
    Widget   _attachmentMenu;
    CmdList *_attachmentActionsList;
    CmdList *_attachmentMenuList;
    CmdList *_attachmentPopupMenuList;
    CmdList *_textPopupMenuList;
    Cmd*  _attachment_popup_title;
    Cmd*  _text_popup_title;
    Cmd*  _attach_save_as;
    Cmd*  _attach_select_all;

    // Help Menu
    Cmd *_overview;
    Cmd *_tasks;
    Cmd *_reference;
    Cmd *_messages;
    Cmd *_on_item;
    Cmd *_using_help;
    Cmd *_about_mailer;

    Widget _text;
    DtMailEditor *my_editor;
    SdmMessageNumber	_msgno;
    Cmd *_separator;
};

#endif
