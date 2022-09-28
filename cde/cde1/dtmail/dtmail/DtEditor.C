/*
 *+SNOTICE
 *
 *	$Revision: 1.7 $
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

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)DtEditor.C	1.27 25 Jan 1994"
#else
static char *sccs__FILE__ = "@(#)DtEditor.C	1.27 25 Jan 1994";
#endif

#ifdef DTEDITOR

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Text.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <X11/IntrinsicP.h>
#include <Xm/Text.h>
#include <Xm/CutPaste.h>
#include <Xm/AtomMgr.h>
#include <Xm/MessageB.h>
#include <SDt/Spell.h>

#include <dlfcn.h>

#include "RoamApp.h"
#include "DtEditor.hh"
#include "MailMsg.h"               // DT_catd defined here
#include "SpellCheckerHelp.hh"     // defines ids for spell checker help

//[Added because error handling has moved here from DtEditor Wigdet
#include <Xm/TextP.h>

//[Error Reporter primarily for Memory Errors
void disp_wd_editor_err(const int type, const char* , const int );

// Handle returned by dlopen() when searching for the spell checking GUI library.
void *spellgLibHandle = NULL;

CDEM_DtWidgetEditor::CDEM_DtWidgetEditor(
    Widget parent,
    DtMailEditor *owner_of_editor
)
{
    my_parent   = parent;
    my_owner	= owner_of_editor;
    my_text     = NULL;
    my_text_core = NULL;
    _modified_text = NULL;
    _modified_text_buflen = 0;

    begin_ins_bracket = NULL;
    indent_str = NULL;
    end_ins_bracket = NULL;
    _auto_show_cursor = FALSE;

    _buffer     = NULL;
    _buf_len    = (unsigned long) 0;
    text_already_selected = FALSE;

    _insertion_cache = NULL;
}

CDEM_DtWidgetEditor::~CDEM_DtWidgetEditor() 
{
    if (my_text) {
	// No DtEditor API equivalent
	// Remove the callbacks first.

	XtRemoveCallback(my_text, DtNtextSelectCallback,
	    &CDEM_DtWidgetEditor::text_selected_callback, this);
	XtRemoveCallback(my_text, DtNtextDeselectCallback,
	    &CDEM_DtWidgetEditor::text_unselected_callback, this);

	XtDestroyWidget(my_text);
    }
    if (_buffer) {
	delete _buffer;
	_buffer = NULL;
    }

    if(_modified_text )  {
	if(_modified_text->ptr)  {
		free(_modified_text->ptr);
		_modified_text->ptr = NULL;
	}
	free(_modified_text);
	_modified_text = NULL;
    }

    if (_insertion_cache)
        delete _insertion_cache;

    /* Don't forget to close the library opened via dlopen(). */
    if (spellgLibHandle != (void *) NULL)
      {
	  dlclose(spellgLibHandle);
      }

}

void
CDEM_DtWidgetEditor::initialize()
{
    
    int i = 0;

    Arg args[10];
    int n = 0;
    short rows;
    DtMailEnv  error;
    DtMail::Session * d_session = theRoamApp.session()->session();
    DtMail::MailRc * mailrc = d_session->mailRc(error);

    const char * value;
    mailrc->getValue(error, "popuplines", &value);
    if (error.isSet()) {
	value = "24";
    }
    rows = strtol(value, NULL, 10);

    XtSetArg(args[i], DtNeditable, FALSE); i++;
    XtSetArg(args[i], DtNrows, rows); i++;
    XtSetArg(args[i], DtNcursorPositionVisible, FALSE); i++;

    my_text = DtCreateEditor(my_parent, "Text", args, i);

  
    XtAddCallback(my_text, DtNtextSelectCallback,
 	   &CDEM_DtWidgetEditor::text_selected_callback, this);
    XtAddCallback(my_text, DtNtextDeselectCallback,
 	   &CDEM_DtWidgetEditor::text_unselected_callback, this);
    XtAddCallback(my_text, XmNhelpCallback,
  	   &CDEM_DtWidgetEditor::help_editor_callback, this);


    XtAddEventHandler(my_text, ButtonPressMask,
                        FALSE, MenuButtonHandler, 
			(XtPointer) this);

    XtManageChild(my_text);

}

int
CDEM_DtWidgetEditor::set_contents(
    const char *contents,
    const unsigned long len
)
{
    DtEditorContentRec content;
    DtEditorErrorCode status;

    this->my_owner->needBuf(&_buffer, &_buf_len, len + 1);
    this->my_owner->stripCRLF(&_buffer, contents, len);

    content.type = DtEDITOR_TEXT;
    content.value.string = _buffer;
    status = DtEditorSetContents(my_text, &content);
	if (status != 0) {
		disp_wd_editor_err(2, __FILE__, __LINE__);
		return -1;
	}
	return 0;
}

int
CDEM_DtWidgetEditor::set_contents(
    const char *path
)
{
   DtEditorErrorCode status;
   struct stat statbuf;
   int flen = 0;

   //[stat success should be checked here, but is not because
   //[it will be caught later in DtEditorSetContentsFromFile
   if (stat(path, &statbuf) == 0)
       flen = statbuf.st_size;
   else
       flen = 0;
   
   status = DtEditorSetContentsFromFile(my_text, (char *)path);
	if (status != 0) {
		disp_wd_editor_err(2, __FILE__, __LINE__);
		return -1;
	}
	return 0;
}

char*
CDEM_DtWidgetEditor::get_contents()
{
    DtEditorErrorCode status;
    static DtEditorContentRec content;
	int len;
 
    content.type = DtEDITOR_TEXT;
    // Get the contents with hardCarriageReturns = TRUE and
    // markContentsAsSaved = TRUE.

   len = DtEditorGetLastPosition(my_text);

    status = DtEditorGetContents(my_text, &content, TRUE, TRUE);
	if (status != 0) {
		disp_wd_editor_err(2, __FILE__, __LINE__);
		return NULL;
	}
    else return(content.value.string);
}

//
// Once all text has been collected in the _insertion_cache,
// update the DtEditor widget using DtEditorInsert.  Then
// delete the cache.
//
int
CDEM_DtWidgetEditor::update_contents(void)
{
    DtEditorContentRec rec;
    DtEditorErrorCode status;
    int rc;

    if (_insertion_cache) {
        rec.type = DtEDITOR_TEXT;
        rec.value.string = (char *)_insertion_cache;
 
        status = DtEditorInsert(my_text, &rec);
        if (status != 0) {
            disp_wd_editor_err(2, __FILE__, __LINE__);
            rc = -1;
        }
 
        delete _insertion_cache;
	_insertion_cache = NULL;
    }
    else {
        rc = -1;
    }
 
    return (rc);
}

int
CDEM_DtWidgetEditor::append_to_contents_and_cache(
    const char *contents,
    const unsigned long len
)
{
    char *temp_contents;

    if ( contents[len - 1] == 0 ) {
        temp_contents = (char *)contents;
    } else {
        this->my_owner->needBuf(&_buffer, &_buf_len, len + 1);
        this->my_owner->stripCRLF(&_buffer, contents, len);
        temp_contents = _buffer;
    }

    //
    // Don't insert the contents into the DtEditor widget yet,
    // concatenate all the contents together and then set
    // them once using update_contents
    //
    if (_insertion_cache)
    {
	char *temp_cache;

        temp_cache = new char [strlen (_insertion_cache)
            + strlen (temp_contents) + 1];
	if (!temp_cache) {
		disp_wd_editor_err(1, __FILE__, __LINE__);
		return -1;
	}
        sprintf (temp_cache, "%s%s", _insertion_cache, temp_contents);
	delete _insertion_cache;
	_insertion_cache = temp_cache;
    }
    else
    {
        _insertion_cache = new char [strlen (temp_contents) + 1];
	if (!_insertion_cache) {
		disp_wd_editor_err(1, __FILE__, __LINE__);
		return -1;
	}
        sprintf (_insertion_cache, "%s", temp_contents);
    }
 
    return 0;
}

int
CDEM_DtWidgetEditor::append_to_contents(
    const char *contents,
    const unsigned long len
)
{
    DtEditorContentRec rec;
    DtEditorErrorCode status;

    rec.type = DtEDITOR_TEXT;

    if ( contents[len - 1] == 0 ) {
	rec.value.string = (char *)contents;
    } else {
	this->my_owner->needBuf(&_buffer, &_buf_len, len + 1);
	this->my_owner->stripCRLF(&_buffer, contents, len);
	rec.value.string = _buffer;
    }

    status = DtEditorInsert(my_text, &rec);
	if (status != 0) {
		disp_wd_editor_err(2, __FILE__, __LINE__);
		return -1;
	}
	return 0;
}

int
CDEM_DtWidgetEditor::append_to_contents(
    const char *path
)
{
   DtEditorErrorCode status;
   struct stat statbuf;
   int flen = 0;

   //[stat success should be checked here, but is not because
   //[it will be caught later in DtEditorSetContentsFromFile
   if (stat(path, &statbuf) == 0)
       flen = statbuf.st_size;
   else
       flen = 0;
   
    status = DtEditorAppendFromFile(my_text, (char *)path);
	if (status != 0) {
		disp_wd_editor_err(2, __FILE__, __LINE__);
		return -1;
	}
	return 0;
    
}

int
CDEM_DtWidgetEditor::append_at_cursor(
    const char *path
)
{
   DtEditorErrorCode status;
   struct stat statbuf;
   int flen = 0;

   //[stat success should be checked here, but is not because
   //[it will be caught later in DtEditorSetContentsFromFile
   stat(path, &statbuf );
   if (stat(path, &statbuf) == 0)
       flen = statbuf.st_size;
   else
       flen = 0;
   
    status = DtEditorInsertFromFile(my_text, (char *) path);
	if (status != 0) {
		disp_wd_editor_err(2, __FILE__, __LINE__);
		return -1;
	}
	return 0;
}

int
CDEM_DtWidgetEditor::append_at_cursor(
    const char *contents,
    const unsigned long len
)
{
    DtEditorErrorCode status;
    DtEditorContentRec rec;

    rec.type = DtEDITOR_TEXT;

    if ( contents[len - 1] == 0 ) {
	rec.value.string = (char *)contents;
    } else {
	this->my_owner->needBuf(&_buffer, &_buf_len, len + 1);
	this->my_owner->stripCRLF(&_buffer, contents, len);
	rec.value.string = _buffer;
    }

    status = DtEditorAppend(my_text, &rec);
	if (status != 0) {
		disp_wd_editor_err(2, __FILE__, __LINE__);
		return -1;
	}
	return 0;
}

void
CDEM_DtWidgetEditor::clear_contents()
{
 
// Doesn't work yet.  Work around with setting an empty string...
//    DtEditorReset(my_text);

    DtEditorContentRec content;
    DtEditorErrorCode status;

    content.type = DtEDITOR_TEXT;
    content.value.string = NULL;
    status = DtEditorSetContents(my_text, &content);
}

#ifdef DEAD_WOOD
Widget
CDEM_DtWidgetEditor::get_text_widget()
{
    // We actually need to return the text widget contained
    // within DtEditor.  For now, just return the DtEditor.

    return(my_text);
}
#endif /* DEAD_WOOD */

Pixel
CDEM_DtWidgetEditor::get_text_foreground()
{
    Pixel fg;
    
    XtVaGetValues(my_text,
	DtNtextForeground, &fg,
	NULL);

    return(fg);
}


// DtEditor returns the bg color of the Form widget, not the 
// text widget that the Form contains.
// This explains why the attachment pane color is that of the scroll bar...
// DtEditor needs to return the color of its text widget.
// OBTW, DtNtextBackground and DtNtextForeground don't work.  They
// return uninitialized values

Pixel
CDEM_DtWidgetEditor::get_text_background()
{
    Pixel bg;
    
    XtVaGetValues(my_text,
	DtNtextBackground, &bg,
	NULL);
    
    return(bg);
}

#ifdef DEAD_WOOD
XmFontList
CDEM_DtWidgetEditor::get_text_fontList()
{
    XmFontList fl;
    
    XtVaGetValues(my_text,
	DtNtextFontList, &fl,
	NULL);

    return(fl);
}
#endif /* DEAD_WOOD */

Dimension
CDEM_DtWidgetEditor::get_text_width()
{
    Dimension wid;
    
    XtVaGetValues(my_text,
        XmNwidth, &wid,
	NULL);
    return (wid);
}


Widget
CDEM_DtWidgetEditor::get_editor()
{
    return(my_text);
}

void
CDEM_DtWidgetEditor::set_editable(
	Boolean bool
)
{
   XtVaSetValues(my_text,
		DtNeditable, bool,
		DtNcursorPositionVisible, bool,
		NULL);
}

void
CDEM_DtWidgetEditor::undo_edit()
{

    DtEditorUndoEdit(my_text);

}

void
CDEM_DtWidgetEditor::cut_selection()
{

    DtEditorCutToClipboard(my_text);

}

void
CDEM_DtWidgetEditor::copy_selection()
{

    DtEditorCopyToClipboard(my_text);

}

void
CDEM_DtWidgetEditor::paste_from_clipboard()
{

    DtEditorPasteFromClipboard(my_text);

}

int
CDEM_DtWidgetEditor::paste_special_from_clipboard(
    Editor::InsertFormat format
)
{
    int i, status;
    unsigned long length, recvd = 0L;
    char *clipboard_data;
    Display *dpy = XtDisplayOfObject(my_text);
    Window window = XtWindowOfObject(my_text);
    Boolean get_ct = False;
    int malloc_size = 0;
    char **tmp_value;

    // Much of this code mimics what was done in Xm/TextF.c in the function
    // XmTextFieldPaste().  Dealing with data from the clipboard doesn't
    // seem to be spelled out very well in any documentation.

    // Find out how long the clipboard string is.
    do {
	status = XmClipboardInquireLength(dpy, window, "STRING", &length);
    } while (status == ClipboardLocked);

    if (status == ClipboardNoData || length == 0) {
	// If length is 0, it might be COMPOUND_TEXT, so we need to check
	// for that too.
	do {
	    status = XmClipboardInquireLength(dpy, window, "COMPOUND_TEXT",
						&length);
	} while (status == ClipboardLocked);

	// If it's still 0, there's nothing in the clipboard.
	if (status == ClipboardNoData || length == 0)
	    return 0;
	get_ct = True;  // We're dealing with COMPOUND_TEXT.
    }
    
    clipboard_data = XtMalloc((unsigned)length);
    
    // Get the string from the clipboard.
    if (!get_ct) {
	do {
	    status = XmClipboardRetrieve(
			    dpy, window, "STRING", clipboard_data, 
			    length,  &recvd, NULL
			    );
	} while (status == ClipboardLocked);
    } else {
	do {
	    status = XmClipboardRetrieve(
			    dpy, window, "COMPOUND_TEXT", clipboard_data, 
			    length,  &recvd, NULL
			    );
	} while (status == ClipboardLocked);
    }
    
    if (status != ClipboardSuccess || recvd != length) {
	XmClipboardEndRetrieve(dpy, window);
	XtFree(clipboard_data);
	return 0;
    }

    XTextProperty tmp_prop;
    tmp_prop.value = (unsigned char *) clipboard_data;

    if (!get_ct)
	tmp_prop.encoding = XA_STRING;
    else
	tmp_prop.encoding = XmInternAtom(dpy, "COMPOUND_TEXT", False);
    
    tmp_prop.format = 8;
    tmp_prop.nitems = recvd;
    int num_vals = 0;

    status = XmbTextPropertyToTextList(dpy, &tmp_prop, &tmp_value, &num_vals);
    XtFree(clipboard_data);

    if (num_vals && (status == Success || status > 0)) {
	char * total_tmp_value;

	// This should work for both multibyte and 7 bit.
	for (i = 0, malloc_size = 1; i < num_vals; i++)
	    malloc_size += strlen(tmp_value[i]);
	total_tmp_value = XtMalloc ((unsigned) malloc_size);
	total_tmp_value[0] = '\0';
	for (i = 0; i < num_vals; i++)
	    strcat(total_tmp_value, tmp_value[i]);
	XFreeStringList(tmp_value);

	// Compute the length of the string to pass to modifyData().
	length = strlen(total_tmp_value);
	// Now modify the data such that the necessary formatting occurs
	// within it.  Bracketting will cause a line at the beginning and
	// end of the data.  Indenting will prepend the indent string before 
	// each line, realigning the lines if necessary.
	// The results are stored in _modified_text so total_tmp_value can 
	// be freed immediately after this call.
	this->modifyData(total_tmp_value, (unsigned) length, format);
	XtFree(total_tmp_value);
    } else {
	return 0;
    }
    

    // Now copy the modified data stripped of CRLFs to a buffer.
    // Put that buffer into the structure appropriate for DtEditor.

    DtEditorContentRec rec;

    rec.type = DtEDITOR_TEXT;
    
    // Length needs to be reset since the text now contains
    // new characters that do the necessary formatting.

    length = _modified_text->length;

    if ( _modified_text->ptr[(unsigned) length - 1] == 0 ) {
	rec.value.string = (char *)_modified_text->ptr;
    } else {
	this->my_owner->needBuf(
			&_buffer, &_buf_len, 
			(unsigned) length + 1
			);
	this->my_owner->stripCRLF(
			&_buffer, _modified_text->ptr, 
			(unsigned) length);
	rec.value.string = _buffer;
    }
    
    DtEditorErrorCode istatus;

    istatus = DtEditorInsert(my_text, &rec);
	if (istatus != 0) {
		disp_wd_editor_err(2, __FILE__, __LINE__);
		return -1;
	}
	return 0;
}


void
CDEM_DtWidgetEditor::clear_selection()
{

    DtEditorClearSelection(my_text);

}

void
CDEM_DtWidgetEditor::delete_selection()
{
    DtEditorDeleteSelection(my_text);
}

void
CDEM_DtWidgetEditor::set_word_wrap(
    Boolean bool
)
{
   XtVaSetValues(my_text, DtNwordWrap, bool, NULL);
}

void
CDEM_DtWidgetEditor::set_to_top()
{
    XtVaSetValues(my_text, 
	DtNtopCharacter, 0, 
	DtNcursorPosition, 0,
	NULL);
}


void
CDEM_DtWidgetEditor::text_selected_callback(
    Widget,
    void * clientData,
    void *
)
{

    CDEM_DtWidgetEditor  *obj=(CDEM_DtWidgetEditor *) clientData;

    obj->text_selected();

}

void
CDEM_DtWidgetEditor::text_unselected_callback(
    Widget,
    void * clientData,
    void *
)
{

    CDEM_DtWidgetEditor  *obj=(CDEM_DtWidgetEditor *) clientData;

    obj->text_unselected();

}


void
CDEM_DtWidgetEditor::help_editor_callback(
    Widget w,
    void * ,
    void * callData
)
{
    DtEditorHelpCallbackStruct *editorHelp =
                        (DtEditorHelpCallbackStruct *) callData;

    char* locationId = NULL;

    switch (editorHelp->reason) {
 
        /* -----> Check Spelling dialog and dialog fields */
        case DtEDITOR_HELP_SPELL_DIALOG:
            locationId = SPELL_HELP;
            break;
        case DtEDITOR_HELP_SPELL_MISSPELLED_WORDS:
            locationId = SPELL_MISSPELLED_WORDS_HELP;
            break;
        case DtEDITOR_HELP_SPELL_CHANGE:
            locationId = SPELL_CHANGETO_HELP;
            break;
 
        default:
	    break;
 
    } /* switch (editorHelp->reason) */

    if (locationId) 
    	DisplayMain (w, "Textedit", locationId);
 
}


void
CDEM_DtWidgetEditor::text_selected()
{

    if (!text_already_selected) {
	text_already_selected = TRUE;
	my_owner->owner()->text_selected();
    }
}

void
CDEM_DtWidgetEditor::text_unselected()
{

    my_owner->owner()->text_unselected();
    text_already_selected = FALSE;

}

void
CDEM_DtWidgetEditor::find_change()
{
   DtEditorInvokeFindChangeDialog(my_text);
}

void
CDEM_DtWidgetEditor::spell()
{
    // Function pointer to the SdtInvokeSpell() call, returned by dlsym().
    Boolean (*SdtInvokeSpell_FP)(Widget);

    /* See if we can access the Sun Spell Checking GUI library. */
    spellgLibHandle = dlopen("libSDtSpell.so", RTLD_LAZY);

    if (spellgLibHandle != (void *) NULL)
      {
	  // We have access to the Sun Spell Checking library.
	  // Get the handle to the spell checking function.
	  SdtInvokeSpell_FP = (Boolean (*)(Widget)) dlsym(spellgLibHandle, "SDtInvokeSpell");
	  
	  if (SdtInvokeSpell_FP != NULL) {
	      // Run the Sun Spell checker. 
	      if ((*SdtInvokeSpell_FP)(my_text) == True) {
	         return;  // successfully ran spell checker; then return.
              }
          }
      }

    // Use the default CDE spell checker. 
    DtEditorInvokeSpellDialog(my_text);
}

void
CDEM_DtWidgetEditor::format()
{
   DtEditorInvokeFormatDialog(my_text);
}

void
CDEM_DtWidgetEditor::auto_show_cursor_off()
{
}

void
CDEM_DtWidgetEditor::auto_show_cursor_restore()
{
}

void
CDEM_DtWidgetEditor::select_all()
{
   DtEditorSelectAll(my_text);
}

void
CDEM_DtWidgetEditor::set_to_bottom()
{
}

int
CDEM_DtWidgetEditor::no_text()
{
   char *text = get_contents();   
   if (text == NULL)
		return 1;
   int text_len = strlen(text);
   int result = 1;

   for ( int k = 0;  k < text_len;  k++ ) {
	  if ( isgraph(text[k]) ) {
		 result = 0;
		 break;
	  }
   }

   XtFree(text);
   return result;
}

void
CDEM_DtWidgetEditor::disable_redisplay()
{
    DtEditorDisableRedisplay(my_text);
}

void
CDEM_DtWidgetEditor::enable_redisplay()
{

    DtEditorEnableRedisplay(my_text);

}



/*
 * This function modifies the pasted data
 * with an indent prefix before each new line or brackets it.
 */

void
CDEM_DtWidgetEditor::modifyData(
    char *sp,  // source pointer to the insert string 
    int length, // length does not include '\0' char
    Editor::InsertFormat insert_format
)
{
    if(_modified_text == NULL)
	_modified_text = (XmTextBlockRec *)calloc(1,sizeof(XmTextBlockRec));

    char *maxsp = sp  + length; // maxmimum source ptr

    // Allocate memory rounded off to the nearest BUFINC
    size_t size_req = (size_t)(((length/BUFINC)+1)*BUFINC);
    if((_modified_text_buflen < size_req)	||
       ((_modified_text_buflen > CDEM_DtWidgetEditor::MAXBUFSZ) && 
	(size_req <  CDEM_DtWidgetEditor::MAXBUFSZ)) 	)
	reallocPasteBuf(size_req);
    
    if(_modified_text->ptr == NULL)
	    return; // No memory available

    switch( insert_format) {
      case IF_INDENTED:	
      {
	  DtMailEnv error;
	  int ip = 0;
	  
		// Get the indent prefix string
          DtMail::Session *m_session = 
		  theRoamApp.session()->session();
	  m_session->mailRc(error)->getValue(error, 
		  "indentprefix", &indent_str);
	  if ( error.isSet() || !indent_str) 
		  indent_str = "> ";

	  size_t indlen = strlen(indent_str);

	  // Copy the src buf into dest, inserting indent before '\n'
	  while(sp < maxsp) {

	      // Make sure there is enough space
	      // for an indent prefix, one char and a terminating '\0'
	      if(!((ip+indlen+2) < _modified_text_buflen) ) {
		  size_req = (size_t)((((_modified_text_buflen + 
		      indlen+2)/BUFINC)+1)*BUFINC);
		  reallocPasteBuf(size_req);
		  if(_modified_text->ptr == NULL)
		      return; // No memory available
	      }

	      // Copy the indent string at the beginning 
	      if(!ip) { 
		  memcpy(_modified_text->ptr, indent_str, indlen);
		  ip += indlen;
	      }

	      // Copy the next byte and check for new line
	      _modified_text->ptr[ip++] = *sp++; 
	      if(*(sp-1) == '\n' && (sp < maxsp)) {
		  memcpy(&_modified_text->ptr[ip], indent_str, indlen);
		  ip += indlen;
	      }
	      
	  }
	  if (*(sp-1) != '\n') {
	      _modified_text->ptr[ip++] = '\n'; // add a newline if none there.
	  }
	  _modified_text->ptr[ip] = '\0'; // terminate with a null char
	  _modified_text->length = ip; // Do not include '\0' char in len
      }
      break;
      case IF_BRACKETED:
    {
	
	if( !begin_ins_bracket)
	    begin_ins_bracket = catgets(DT_catd, 1, 201,
		"\n------------- Begin Included Message -------------\n\n"); 
	if(!end_ins_bracket) 
	    end_ins_bracket = catgets(DT_catd, 1, 202,
		"\n------------- End Included Message -------------\n"); 
	
	size_t begin_len = strlen(begin_ins_bracket); 
	size_t end_len = strlen(end_ins_bracket); 

	// Make sure there is enough space
	if((size_req = length + begin_len + end_len + 1) > 
	   _modified_text_buflen) {
	    size_req = (size_t) ((((size_req)/BUFINC)+1)*BUFINC);
	    reallocPasteBuf(size_req);
	}

	if(_modified_text->ptr == NULL)
	    return;

	strcpy(_modified_text->ptr, begin_ins_bracket);
	strncat(_modified_text->ptr,sp,length);
	strcat(_modified_text->ptr, end_ins_bracket); 
	_modified_text->length = end_len + begin_len + length;
    }
      break;
    default:
      break;
  }
}

void
CDEM_DtWidgetEditor::MenuButtonHandler(
    Widget ,
    XtPointer cd,
    XEvent *event,
    Boolean *)
{
    CDEM_DtWidgetEditor *obj = (CDEM_DtWidgetEditor *)cd;

    if(event->xany.type != ButtonPress)
	return;

    XButtonEvent *be = (XButtonEvent *)event;

    if(be->button == Button3)
	obj->my_owner->owner()->postTextPopup(event);
}

void disp_wd_editor_err(const int type, const char* , const int )
{
    if (type == 1) {
	char* helpId;

	DtMailGenDialog *nomem_dialog = new DtMailGenDialog("NoMemDialog",
	    theApplication->baseWidget());
	nomem_dialog->setToErrorDialog(catgets(DT_catd, 2, 1, "Mailer"),
	    catgets(DT_catd, 3, 36, "Unable to allocate memory."));
	helpId = DTMAILHELPERROR;
	Widget hpBut = XmMessageBoxGetChild(nomem_dialog->baseWidget(),
	    XmDIALOG_HELP_BUTTON);
	XtSetSensitive(hpBut, False);
	nomem_dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), helpId);
    }
    else {
	//[Due to lack of time for FCS1.0.1 - ignored
    }
}

#endif
