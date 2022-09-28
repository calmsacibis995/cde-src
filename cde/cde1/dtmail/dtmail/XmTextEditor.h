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

#ifndef XMTEXT_EDITOR_H
#define XMTEXT_EDITOR_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)XmTextEditor.h	1.36 04/05/96"
#endif

#include "DtMailEditor.hh"
#include "Editor.hh"

class XmTextEditor : public Editor
{

  public:
    XmTextEditor(Widget parent, DtMailEditor *owner);
    ~XmTextEditor();

    virtual void	initialize();

    virtual int	set_contents(
				     const char *contents,
				     const unsigned long len
				     );

    virtual int	set_contents(const char * path);

    virtual char *	get_contents();

    virtual int	append_to_contents(
					   const char *new_contents,
					   const unsigned long len
					   );
    virtual int	append_to_contents_and_cache(
					   const char *new_contents,
					   const unsigned long len
					   );
    virtual int	append_to_contents(const char * path);
    virtual int	update_contents(void);

    virtual int	append_at_cursor(const char *path);
    virtual int	append_at_cursor(
				     const char *contents, 
				     const unsigned long len
				     );

    virtual void	clear_contents();

#ifdef DEAD_WOOD
    virtual Widget	get_text_widget();
#endif /* DEAD_WOOD */

    virtual Pixel	get_text_foreground();
    virtual Pixel	get_text_background();
#ifdef DEAD_WOOD
    virtual XmFontList	get_text_fontList();
#endif /* DEAD_WOOD */
    virtual Dimension   get_text_width();

    virtual Widget	get_editor();

    virtual void	set_editable(Boolean bool);

    virtual void	auto_show_cursor_off();
    virtual void	auto_show_cursor_restore();
    virtual void	set_to_top();
    virtual void	set_to_bottom();

    virtual void	obtained_focus();
    virtual void	text_selected();
    virtual void	text_unselected();

    virtual int		no_text();
    
    virtual void	find_change();
    virtual void	spell();
    virtual void	format();
    virtual void	set_word_wrap(Boolean bool);
    virtual void	undo_edit();
    virtual void	cut_selection();
    virtual void	copy_selection();
    virtual void	paste_from_clipboard();
    virtual int	paste_special_from_clipboard(Editor::InsertFormat);
    virtual void	clear_selection();
    virtual void	delete_selection();
    virtual void	select_all();
    virtual void	disable_redisplay();
    virtual void	enable_redisplay();

    static void MenuButtonHandler(Widget, XtPointer, XEvent *, Boolean *); 

  protected:
    // modify verify callback used during Paste Special
    static void modify_verify_callback(Widget, XtPointer, XtPointer);
    static void text_selected_callback(
				       Widget, 
				       void *, 
				       void *
				       );
    static void text_unselected_callback(
					 Widget, 
					 void *, 
					 void *
					 );
#ifdef DEAD_WOOD
    static void focus_callback(
			       Widget, 
			       void *, 
			       void *
			      );
#endif /* DEAD_WOOD */

  private:
    struct PSClientData {
	XmTextEditor *obj;
	Editor::InsertFormat insert_format;
    };
    XmTextBlockRec	*_modified_text; 
    size_t		_modified_text_buflen;
    enum PasteSpecBuf { MAXBUFSZ = 2048, BUFINC = 512};
    void modifyPasteData(XmTextVerifyCallbackStruct *, Editor::InsertFormat);
    void reallocPasteBuf(size_t size_req) {
		_modified_text->ptr = 
			(char *)realloc((void *)_modified_text->ptr, size_req);
		_modified_text_buflen = size_req; 
    }
    const char *indent_str;
    const char *begin_ins_bracket;
    const char *end_ins_bracket;
    Widget my_parent;
    Widget my_text;
    DtMailEditor *my_owner;
    Boolean text_already_selected;

    int loadFile(const char * path, const int pos);

    char *		_buffer;
    unsigned long	_buf_len;
    Boolean		_auto_show_cursor;
};

#endif // XMTEXT_EDITOR_HH
