/*
 *+SNOTICE
 *
 *	$Revision: 1.6 $
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

#ifndef DTEDITOR_HH
#define DTEDITOR_HH

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)DtEditor.hh	1.18 05/08/97"
#else
static char *sccs__FILE__ = "@(#)DtEditor.hh	1.18 05/08/97";
#endif

// Include the DtWidgetEditor .h file given by HP.

#include <Dt/Editor.h>

#include "Editor.hh"
#include "DtMailEditor.hh"

class CDEM_DtWidgetEditor : public Editor
{

  public:
    CDEM_DtWidgetEditor(
		Widget parent, 
		DtMailEditor *owner_of_editor);
    ~CDEM_DtWidgetEditor();
    
	// Pure virtual functions of class Editor
    //
    virtual void	initialize();

    virtual int	set_contents(
				const char *contents,
				const unsigned long len
				);

    virtual int	set_contents(const char *path);
				
    virtual char*	get_contents(Boolean hardCR);

    virtual int	insert_at_cursor(
				const char *new_contents,
				const unsigned long len,
				const int useBufferAsIs = 0
				);
    virtual int	append_to_contents_cache(
				const char *new_contents,
				const unsigned long len,
				const int useBufferAsIs = 0
				);
    virtual int preallocate_contents_cache(const unsigned long len);
    virtual int	append_to_contents(int fd);
    virtual int	insert_contents_cache_at_cursor(void);

    virtual int	insert_at_cursor(int fd);

    virtual int	append_to_contents(
				const char *contents,
				const unsigned long len
				);

    virtual void	clear_contents();

#ifdef DEAD_WOOD
    virtual Widget	get_text_widget();
#endif /* DEAD_WOOD */
    virtual Pixel	get_text_foreground();
    virtual Pixel	get_text_background();
    virtual Dimension   get_text_width();
#ifdef DEAD_WOOD
    virtual XmFontList	get_text_fontList();
#endif /* DEAD_WOOD */

    virtual Widget	get_editor();
    
    virtual void	set_editable(SdmBoolean bool);

    virtual void	auto_show_cursor_off();
    virtual void	auto_show_cursor_restore();

    virtual void	set_to_top();
    virtual void	set_to_bottom();

    virtual void	cut_selection();
    virtual void	copy_selection();
    virtual void	paste_from_clipboard();
    virtual int	paste_special_from_clipboard(Editor::InsertFormat);
    virtual void	clear_selection();
    virtual void	delete_selection();
    virtual void	select_all();

    virtual void	disable_redisplay();
    virtual void	enable_redisplay();

    //
	// End of Pure virtual functions for class Editor.

	// Pure virtual functions of class AbstractEditorParent
    virtual void	text_selected();
    virtual void	text_unselected();

	// Functions specific to DtEditor
    virtual void	set_word_wrap(SdmBoolean bool);
    virtual void	undo_edit();
    virtual void	find_change();
    virtual void    spell();
    virtual void    format();

    static void MenuButtonHandler(Widget, XtPointer, XEvent *, char *); 
			
  protected:

    // modify verify callback used during Paste Special
    static void modify_verify_callback(Widget, XtPointer, XtPointer);

    static void text_selected_callback(Widget, void *, void *);
    static void text_unselected_callback(Widget, void *, void *);
    static void help_editor_callback(Widget, void *, void *);
	

  private:
    struct PSClientData {
	CDEM_DtWidgetEditor *obj;
	Editor::InsertFormat insert_format;
    };
    XmTextBlockRec	*_modified_text; 
    size_t		_modified_text_buflen;
    enum PasteSpecBuf { MAXBUFSZ = 2048, BUFINC = 512};
    void modifyData(char *, int, Editor::InsertFormat);
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
    Widget my_text_core;

    DtMailEditor *my_owner;
    SdmBoolean text_already_selected;
    SdmBoolean		_auto_show_cursor;

    char * _buffer;
    unsigned long _buf_len;

    // Cache all appends to the Editor widget
    // Call DtEditorInsert one time after all appends are complete
    char *_insertion_cache;
    int _insertion_cache_length;
    unsigned long _insertion_cache_allocation;
};

#endif // DTEDITOR_HH
