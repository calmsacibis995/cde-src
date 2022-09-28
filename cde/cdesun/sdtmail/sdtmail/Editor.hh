/*
 *+SNOTICE
 *
 *	$Revision: 1.57 $
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

#ifndef EDITOR_H
#define EDITOR_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)Editor.hh	1.57 05/08/97"
#endif

#include <Xm/Xm.h>
#include "MenuBar.h"

// Get all the Bento-related API and types (CMContainer, etc.)

#include "UIComponent.h"
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Message.hh>
#include <SDtMail/Error.hh>

class DtMailEditor;

class Editor : public UIComponent {
    
  public:	
    Editor();
    virtual ~Editor();

    virtual void	initialize() = 0;

    virtual int	set_contents(
				     const char *contents,
				     const unsigned long len
				     ) = 0;

    virtual int	set_contents(const char * path) = 0;

    virtual char *	get_contents(Boolean hcr) = 0;

    virtual int	insert_at_cursor(
					   const char *new_contents,
					   const unsigned long len,
					   const int useBufferAsIs = 0
					   ) = 0;
    virtual int preallocate_contents_cache(const unsigned long len) = 0;
    virtual int	append_to_contents_cache(
					   const char *new_contents,
					   const unsigned long len,
					   const int useBufferAsIs = 0
					   ) = 0;
    virtual int	append_to_contents(int fd) = 0;
    virtual int	insert_contents_cache_at_cursor(void) = 0;

    virtual int	insert_at_cursor(int fd) = 0;
    virtual int	append_to_contents(
				     const char *contents, 
				     const unsigned long len
				     ) = 0;

    enum InsertFormat {
	IF_NONE,	// No special format
	IF_INDENTED,	// Indented with indent_prefix
	IF_BRACKETED	// Bracketed
    };

    enum BracketFormat {
	BF_NONE,	// No bracketing
	BF_INCLUDE,	// As included message,
	BF_FORWARD	// As forwarded message
	};

    enum HeaderFormat {
	HF_NONE,	// Do not insert headers in message body.
	HF_FULL,	// Insert all headers.
	HF_ABBREV	// Do not insert ignored headers.
	};

    virtual SdmBoolean	set_message(
              SdmError &error,
              SdmMessage * msg,
					    char ** status_string,
					    HeaderFormat header_format = HF_ABBREV,
					    InsertFormat format = IF_NONE,
					    BracketFormat brackets = BF_NONE);

    virtual void	clear_contents() = 0;

    virtual Widget	get_editor() = 0;
    
    virtual Pixel	get_text_foreground() = 0;
    virtual Pixel	get_text_background() = 0;
    virtual Dimension   get_text_width() = 0;    

    virtual void	set_editable(SdmBoolean bool) = 0;

    virtual void        auto_show_cursor_off() = 0;
    virtual void        auto_show_cursor_restore() = 0;

    virtual void	set_to_top() = 0;
    virtual void	set_to_bottom() = 0;

	virtual void    find_change() = 0;
	virtual void    spell() = 0;
	virtual void    format() = 0;
	virtual void    set_word_wrap(SdmBoolean bool) = 0;
	virtual void    undo_edit() = 0;
    virtual void	cut_selection() = 0;
    virtual void	copy_selection() = 0;
    virtual void	paste_from_clipboard() = 0;
    virtual int	paste_special_from_clipboard(InsertFormat) = 0;
    virtual void	clear_selection() = 0;
    virtual void	delete_selection() = 0;
    virtual void	select_all() = 0;

    virtual void	disable_redisplay() = 0;
    virtual void	enable_redisplay() = 0;

};

class AbstractEditorParent {
  protected:
	Widget _attachmentPopupMenu;
	Widget _textPopupMenu;
	MenuBar *_menuPopupAtt;
	MenuBar *_menuPopupText;
  public:

    AbstractEditorParent();
    virtual ~AbstractEditorParent();

    virtual     const char *const className() 
				{ return "AbstractEditorParent"; }

    virtual DtMailEditor *  get_editor() = 0;

    // Text/attachment (de)selection methods

    virtual void text_selected()   = 0;
    virtual void text_unselected() = 0;
    virtual void attachment_selected() = 0;
    virtual void all_attachments_deselected() = 0;
    virtual void all_attachments_selected() = 0;
    virtual void add_att(const char *) = 0;
    virtual void add_att(const char *, SdmString) = 0;
    virtual void add_att(SdmString) = 0;

    virtual void postAttachmentPopup(XEvent *event);
    virtual void postTextPopup(XEvent *event);
    
    virtual void addAttachmentActions(
			char **,
			int
		) = 0;
    virtual void invokeAttachmentAction(int) = 0;
    virtual void removeAttachmentActions() = 0;

    virtual void selectAllAttachments() = 0;

    virtual void	showAttachArea() = 0;
    virtual void	hideAttachArea() = 0;

    virtual void	attachmentFeedback(SdmBoolean) = 0;

};

    
#endif // EDITOR_HH
