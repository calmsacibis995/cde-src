#if 0

// This code was removed on 4/4/97 because the desktop editor is the
// only editor that should be used by dtmail and because improvements
// to DtEditor have rendered this code broken.

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

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)XmTextEditor.C	1.68 04/08/97"
#else
static char *sccs__FILE__ = "@(#)XmTextEditor.C	1.68 04/08/97";
#endif

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dlfcn.h>

#if defined(NEED_MMAP_WRAPPER)
extern "C" {
#endif

#include <sys/types.h>
#include <sys/mman.h>

#if defined(NEED_MMAP_WRAPPER)
}
#endif

#include <Xm/Form.h>
#include <Xm/Text.h>
#include <Xm/MessageB.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Session.hh>
#include <SDtMail/MailRc.hh>
#include <SDtMail/SystemUtility.hh>
#include "XmTextEditor.h"
#include <RoamApp.h>
#include <MailMsg.h>
#include <SDt/Spell.h>

// Handle returned by dlopen() when searching for the spell checking GUI library.
void *spellgLibHandle2 = NULL;

//[Error Reporter primarily for Memory Errors
//[In the case of this file, this check is just a bogus error check,
//[because XmTextEditor is an unpublished feature. Check is done here
//[to be consistent with DtEditor.C. So who will catch errors here
//[it will be the XtSetErrorHandler() function.

void disp_xm_editor_err(const char* file, const int line);

XmTextEditor::XmTextEditor(
    Widget parent,
    DtMailEditor *owner_of_editor
)
{
    my_parent   = parent;
    my_owner	= owner_of_editor;
    _w    = NULL;
	my_text = NULL;

    _buffer = NULL;
    _buf_len = 0;
    _modified_text = NULL;
    _modified_text_buflen = 0;
    begin_ins_bracket = NULL;
    indent_str = NULL;
    end_ins_bracket = NULL;
    _auto_show_cursor = Sdm_False;
    text_already_selected = Sdm_False;
}

XmTextEditor::~XmTextEditor() 
{
    if (_buffer) {
	this->my_owner->deleteBuf(&_buffer);
    }

    if(_modified_text )  {
	if(_modified_text->ptr)  {
		free(_modified_text->ptr);
		_modified_text->ptr = NULL;
	}
	free(_modified_text);
	_modified_text = NULL;
    }

    /* Don't forget to close the library opened via dlopen(). */
    if (spellgLibHandle2 != (void *) NULL)
      {
	  dlclose(spellgLibHandle2);
      }

    if (indent_str) {
	free (indent_str);
    }
}

void
XmTextEditor::initialize()
{
    Arg args[10];
    int n = 0;
    short rows, cols;
    SdmError  error;
    SdmSession * d_session = theRoamApp.isession()->isession();
    SdmConnection * con = theRoamApp.connection()->connection();
    SdmMailRc * mailrc;
    
    con->SdmMailRcFactory(error, mailrc);
    assert(!error);

    char * value;
    mailrc->GetValue(error, "popuplines", &value);
    if (error != Sdm_EC_Success) {
    	rows = (short)24;
    }
    else {
    	rows = (short) strtol(value, NULL, 10);
	free(value);
    }

    // If toolcols is set, overwrite the column width with "toolcols" value.
    // Otherwise, default resource value will be used.
    mailrc->GetValue(error, "toolcols", &value);
    if (error == Sdm_EC_Success){
        cols = (short) strtol(value, NULL, 10);
        XtSetArg(args[n], XmNcolumns, cols); n++;
	free(value);
    }

    XtSetArg(args[n], XmNeditable, Sdm_False); n++;
    XtSetArg(args[n], XmNrows, rows); n++;

    _w = XmCreateScrolledText(my_parent, "Text",
			      args, n );

    XtManageChild(_w);

    XtAddEventHandler(XtParent(_w), ButtonPressMask,
                        Sdm_False, MenuButtonHandler, 
			(XtPointer) this);

}

int
XmTextEditor::set_contents(
			   const char *contents,
			   const unsigned long len
			   )
{
	int status;

    if (contents[len - 1] == 0) {
	XmTextSetString(_w, (char *)contents);
    }
    else {
	this->my_owner->needBuf(&_buffer, &_buf_len, len + 1);
	this->my_owner->stripCRLF(&_buffer, contents, len);
	XmTextSetString(_w, _buffer);
    }

	status = 0;
	if (status != 0) {
		disp_xm_editor_err(__FILE__, __LINE__);
		return -1;
	}
	else return 0;
}

int
XmTextEditor::set_contents(const char * path)
{
	int status;

    loadFile(path, 0);
	status = 0;
	if (status != 0) {
		disp_xm_editor_err(__FILE__, __LINE__);
		return -1;
	}
	else return 0;
}

char*
XmTextEditor::get_contents(Boolean)
{
    return(XmTextGetString(_w));
}

int
XmTextEditor::update_contents(void)
{
    return 0;
};

int
XmTextEditor::preallocate_contents_cache(const unsigned long)
{
  return 0;
}

// Add function to cache the text to be inserted.
// Better performance if text is added to widget
// in a single call.
int
XmTextEditor::append_to_contents_and_cache( const char *,
					    const unsigned long)
{
    return 0;
}

int
XmTextEditor::append_to_contents(
    const char *contents,
    const unsigned long len
)
{
	int status;

    if (contents[len - 1] == 0) {
  	XmTextInsert( _w, XmTextGetLastPosition( _w ), 
		      (char *)contents);
    }
    else {
	// Not null terminated, and most likely has crlf instead of lf.
	//
	this->my_owner->needBuf(&_buffer, &_buf_len, len + 1);
	this->my_owner->stripCRLF(&_buffer, contents, len);
	XmTextInsert(_w, XmTextGetLastPosition(_w), _buffer);
    }
	
	status = 0;
	if (status != 0) {
		disp_xm_editor_err(__FILE__, __LINE__);
		return -1;
	}
	else return 0;
}

int
XmTextEditor::append_to_contents(const char * path)
{
	int status;

    loadFile(path, (const int) XmTextGetLastPosition(_w));
	status = 0;
	if (status != 0) {
		disp_xm_editor_err(__FILE__, __LINE__);
		return -1;
	}
	else return 0;
}

int
XmTextEditor::append_at_cursor(
				 const char *contents,
				 const unsigned long len
				 )
{
	int status;

    if (contents[len - 1] == 0) {
  	XmTextInsert(
		_w, 
		XmTextGetInsertionPosition(_w), 
		(char *)contents
		);
    }
    else {
	// Not null terminated, and most likely has crlf instead of lf.
	//
	this->my_owner->needBuf(&_buffer, &_buf_len, len + 1);
	this->my_owner->stripCRLF(&_buffer, contents, len);
	XmTextInsert(
		_w, 
		XmTextGetInsertionPosition(_w), 
		_buffer);
    }
	status = 0;
	if (status != 0) {
		disp_xm_editor_err(__FILE__, __LINE__);
		return -1;
	}
	else return 0;
}

int
XmTextEditor::append_at_cursor(const char * path)
{
	int status;

    loadFile(path, (const int)XmTextGetInsertionPosition(_w));
	status = 0;
	if (status != 0) {
		disp_xm_editor_err(__FILE__, __LINE__);
		return -1;
	}
	else return 0;
}

void    
XmTextEditor::clear_contents()
{

    XmTextSetString(_w, "");

}

#ifdef DEAD_WOOD
Widget
XmTextEditor::get_text_widget()
{
    return _w;
}
#endif /* DEAD_WOOD */

Pixel
XmTextEditor::get_text_foreground()
{
    Pixel fg;
    
    XtVaGetValues(_w,
	XmNforeground, &fg,
	NULL);
    return(fg);
}


Pixel
XmTextEditor::get_text_background()
{
    Pixel bg;
    
    XtVaGetValues(_w,
	XmNbackground, &bg,
	NULL);
    return(bg);
}

#ifdef DEAD_WOOD
XmFontList
XmTextEditor::get_text_fontList()
{
    XmFontList fl;
    
    XtVaGetValues(_w,
	XmNfontList, &fl,
	NULL);
    return(fl);
}
#endif /* DEAD_WOOD */

Dimension
XmTextEditor::get_text_width()
{
    Dimension wid;
    
    XtVaGetValues(_w,
        XmNwidth, &wid,
	NULL);
    return (wid);
}

Widget
XmTextEditor::get_editor()
{
    return XtParent(_w);
}

void
XmTextEditor::set_editable(
	SdmBoolean bool
)
{
  XmTextSetEditable( _w, bool);

  // If not editable, turn off the cursor?

  XtVaSetValues(_w,
		XmNcursorPositionVisible, bool,
		NULL);
}


// TOGO void
// TOGO XmTextEditor::set_container(
// TOGO    CMContainer
// TOGO )
// TOGO {
// TOGO    // Extract text and display?
// TOGO }

// TOGO CMContainer
// TOGO XmTextEditor::get_container()
// TOGO {
// TOGO    return(NULL);    
// TOGO }

void
XmTextEditor::auto_show_cursor_off()
{
    // Get the original value of XmNautoShowCursorPosition
    XtVaGetValues(_w,
		  XmNautoShowCursorPosition, &_auto_show_cursor,
		  NULL);

    // Set it to false so we don't scroll with the cursor.
    XtVaSetValues(_w,
		  XmNautoShowCursorPosition, Sdm_False,
		  NULL);
}

void
XmTextEditor::auto_show_cursor_restore()
{
    // Restore the original value of XmNautoShowCursorPosition.
    XtVaSetValues(_w,
		  XmNautoShowCursorPosition, _auto_show_cursor,
		  NULL);
}

void
XmTextEditor::set_to_top()
{

    XmTextShowPosition(_w, 0);
    XmTextSetInsertionPosition(_w, 0);

}

void
XmTextEditor::set_to_bottom()
{
	XmTextSetInsertionPosition( _w, XmTextGetLastPosition(_w) );
}

#ifdef DEAD_WOOD
void
XmTextEditor::focus_callback(
    Widget,
    void *clientData,
    void *
)
{
 
    XmTextEditor  *obj=(XmTextEditor *) clientData;

    obj->obtained_focus();

}
#endif /* DEAD_WOOD */

void
XmTextEditor::obtained_focus()
{
    // If there is text already selected, then the highlighted
    // text is unhighlighted when the widget gets the focus
    // next.  Need to disable the parent's menu items now.

    if (XmTextGetSelection(_w) != NULL) {
	text_already_selected = Sdm_True;
    }

    if (text_already_selected)
	this->text_unselected();
}

void
XmTextEditor::text_selected_callback(
    Widget,
    void *clientData,
    void *
)
{

    XmTextEditor  *obj=(XmTextEditor *) clientData;

    obj->text_selected();

}

void
XmTextEditor::text_unselected_callback(
    Widget,
    void *,
    void *
)
{

//    XmTextEditor  *obj=(XmTextEditor *) clientData;

//    obj->text_unselected();

}

void
XmTextEditor::text_selected()
{
    if (!text_already_selected) {
	text_already_selected = Sdm_True;
	my_owner->owner()->text_selected();
    }
}

void
XmTextEditor::text_unselected()
{
    text_already_selected = Sdm_False;
    my_owner->owner()->text_unselected();
}

void
XmTextEditor::undo_edit()
{
	// This is to be consistent with DtEditor.
    // Do nothing since Motif XmText does not support this.
}

void
XmTextEditor::set_word_wrap(SdmBoolean)
{
	// This is to be consistent with DtEditor.
    // Do nothing since Motif XmText does not support this.
}

void
XmTextEditor::find_change()
{
	// This is to be consistent with DtEditor.
    // Do nothing since Motif XmText does not support this.
}

void
XmTextEditor::spell()
{
    // Function pointer to the SdtInvokeSpell() call, returned by dlsym().
    SdmBoolean (*SdtInvokeSpell_FP)(Widget);

    /* See if we can access the Sun Spell Checking GUI library. */
    spellgLibHandle2 = dlopen("libSDtSpell.so", RTLD_LAZY);

    if (spellgLibHandle2 != (void *) NULL)
      {
	  // We have access to the Sun Spell Checking library.
	  // Get the handle to the spell checking function.
	  SdtInvokeSpell_FP = (SdmBoolean (*)(Widget)) dlsym(spellgLibHandle2, "SdtInvokeSpell");
	  
	  if (SdtInvokeSpell_FP != NULL)
	      // Run the Sun Spell checker. 
	      if ((*SdtInvokeSpell_FP)(_w) == True) {
		return;
              }
      }

    // Do nothing since Motif XmText does not support this.
}

void
XmTextEditor::format()
{
	// This is to be consistent with DtEditor.
    // Do nothing since Motif XmText does not support this.
}

void
XmTextEditor::cut_selection()
{
	// Shouldn't really use CurrentTime
	XmTextCut( _w, CurrentTime );
}

void
XmTextEditor::copy_selection()
{
	// Shouldn't really use CurrentTime
	XmTextCopy( _w, CurrentTime );
}

void
XmTextEditor::paste_from_clipboard()
{
	XmTextPaste( _w );
}

int
XmTextEditor::paste_special_from_clipboard(Editor::InsertFormat format)
{
	int status;
	PSClientData cd;

	cd.obj = this;
	cd.insert_format = format;

	XtAddCallback(_w, XmNmodifyVerifyCallback, 
			modify_verify_callback, (XtPointer)&cd);

	XmTextPaste( _w );

	XtRemoveCallback(_w, XmNmodifyVerifyCallback, 
			modify_verify_callback, (XtPointer)&cd);

	status = 0;
	if (status != 0) {
		disp_xm_editor_err(__FILE__, __LINE__);
		return -1;
	}
	else return 0;
}


// Removes the selection and leaves the resulting white space.
void
XmTextEditor::clear_selection()
{
	// Shouldn't really use CurrentTime
	// XmTextClearSelection( _w, CurrentTime );

	// BUG in XmTextClearSelection.  Selection is not cleared.  Only the cursor
	// is advanced to the last selected position.  Therefore need the following
	// workaround.

	XmTextPosition left, right;

	if ( XmTextGetSelectionPosition( _w, &left, &right ) ) {
	   char *sel = XmTextGetSelection( _w );
	   // NOTE:  Modifying buffer returned by XmTextGetSelection.
	   // Future Motif implementation might return read only buffer.
	   if (sel != NULL) {
	   	memset( sel, ' ', strlen(sel) );
	   	// XmTextInsert( _w, left, sel );
	   	XmTextReplace( _w, left, right, sel );
	   	XtFree(sel);
	   }
	}
}

// Removes the selection and compresses the resulting white space.
void
XmTextEditor::delete_selection()
{
	XmTextRemove( _w );
}

void
XmTextEditor::select_all()
{
	XmTextSetSelection( _w, 0, XmTextGetLastPosition(_w), CurrentTime );
}

void
XmTextEditor::disable_redisplay(void)
{
    XmTextDisableRedisplay(_w);
}

void
XmTextEditor::enable_redisplay(void)
{
    XmTextEnableRedisplay(_w);
}

int
XmTextEditor::loadFile(const char * path, const int pos)
{
    int fd = SdmSystemUtility::SafeOpen(path, O_RDONLY);
    if (fd < 0) {
	return 0;
    }

    struct stat info;
    if (fstat(fd, &info) < 0) {
	close(fd);
	return 0;
    }

    int page_size = SdmSystemUtility::HardwarePageSize();
    size_t map_size = (size_t)(info.st_size + 
				(page_size - (info.st_size % page_size)));
    char * map;

    map = mmap(0, map_size, PROT_READ, MAP_PRIVATE|MAP_NORESERVE, fd, 0);
    if (map == (char *)-1) {
	// We could not map it for some reason. Let's just read it into
	// _buffer and pass it to XmText.
	//
	this->my_owner->needBuf(&_buffer, &_buf_len, info.st_size + 1);
	if (read(fd, _buffer, (unsigned int) info.st_size) < 0) {
	    close(fd);
	    return 0;
	}
	_buffer[info.st_size] = 0;
	XmTextInsert(_w, pos, _buffer);
    }
    else {
	// We now have a mapped file. XmText wants a zero terminated
	// buffer. We get luck with mmap because unless the file is
	// an even page size, we will have some zero fill bytes that
	// are legal to access.
	//
	// Of course in the case of an even page size file we must
	// copy the buffer, terminate it and then give it to XmText.
	//
	if (info.st_size < map_size) {
	    XmTextInsert(_w, pos, map);
	}
	else {
	    this->my_owner->needBuf(&_buffer, &_buf_len, info.st_size + 1);
	    this->my_owner->stripCRLF(&_buffer, map, info.st_size);
	    XmTextInsert(_w, pos, _buffer);
	}
	munmap(map, map_size);
    }

    close(fd);

	return 0;
}

void 
XmTextEditor::modify_verify_callback(
	Widget , XtPointer client, XtPointer call_data)
{
	PSClientData *cd = (PSClientData *)client;
	XmTextVerifyCallbackStruct *modify_info = 
			(XmTextVerifyCallbackStruct *)call_data;

	// Make sure we are being called programmatically
	if(modify_info->event != (XEvent *)NULL)
		return;

	cd->obj->modifyPasteData(modify_info, cd->insert_format);
}

/*
 * This fucntion modifies the pasted data
 * with an indent prefix before each new line or brackets it.
 */

void
XmTextEditor::modifyPasteData(
	XmTextVerifyCallbackStruct *modify_info,
	Editor::InsertFormat insert_format)
{
	// The toolkit does not use this any more, this must be weird stuff.
	if(modify_info->text->format == XmFMT_16_BIT)
		return;

	if(_modified_text == NULL)
		_modified_text = (XmTextBlockRec *)calloc(1,sizeof(XmTextBlockRec));

	char *sp = modify_info->text->ptr; // source pointer to the insert string 
	int length = modify_info->text->length; // length does not include '\0' char
	char *maxsp = sp  + length; // maxmimum source ptr

	// Allocate memory rounded off to the nearest BUFINC
	size_t size_req = (size_t)(((length/BUFINC)+1)*BUFINC);
	if((_modified_text_buflen < size_req)	||
		  ((_modified_text_buflen > XmTextEditor::MAXBUFSZ) && 
			(size_req <  XmTextEditor::MAXBUFSZ)) 	)
		reallocPasteBuf(size_req);

	if(_modified_text->ptr == NULL)
		return; // No memory available

	switch( insert_format) {
	case IF_INDENTED:	
		{
		SdmError error;
		int ip = 0;

		// Get the indent prefix string
		SdmSession *m_session = 
				theRoamApp.isession()->isession();
		SdmConnection *con = theRoamApp.connection()->connection();
		SdmMailRc *mailrc;
		con->SdmMailRcFactory(error, mailrc);
    		assert(!error);
		mailrc->GetValue(error, 
				"indentprefix", &indent_str);
		if ((error != Sdm_EC_Success) || !indent_str) 
			indent_str = strdup("> ");

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
			if(*(sp-1) == '\n') {
				memcpy(&_modified_text->ptr[ip], indent_str, indlen);
				ip += indlen;
			}

		}
		_modified_text->ptr[ip] = '\0'; // terminate with a null char
		_modified_text->length = ip; // Do not include '\0' char in len
		}
		break;
	case IF_BRACKETED:
		{

		if( !begin_ins_bracket)
			begin_ins_bracket = catgets(DT_catd, 1, 199,
				"\n------------- Begin Included Message -------------\n\n"); 
		if(!end_ins_bracket) 
			end_ins_bracket = catgets(DT_catd, 1, 200,
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
			
	_modified_text->format = modify_info->text->format;

	// Stuff the modified text block ptr in the return call data
	modify_info->text = _modified_text;
}

void
XmTextEditor::MenuButtonHandler(
    Widget ,
    XtPointer cd,
    XEvent *event,
    char *)
{
	XmTextEditor *obj = (XmTextEditor *)cd;

	if(event->xany.type != ButtonPress)
		return;

	XButtonEvent *be = (XButtonEvent *)event;

	if(be->button == Button3)
		obj->my_owner->owner()->postTextPopup(event);
}


void disp_xm_editor_err(const char*, const int)
{
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

#endif
