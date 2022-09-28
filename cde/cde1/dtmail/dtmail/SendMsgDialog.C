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

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)SendMsgDialog.C	1.235 05/07/96"
#else
static char *sccs__FILE__ = "@(#)SendMsgDialog.C	1.235 05/07/96";
#endif

#include <EUSCompat.h>
#include "AttachArea.h"
#include "Attachment.h"
#include "RoamApp.h"
#include "SendMsgDialog.h"
#include "NoOpCmd.h"
#include "RoamCmds.h"
#include "ComposeCmds.hh"
#include "CmdList.h"
#include "MenuBar.h"
#include "MenuButton.h"
#include "ButtonInterface.h"
#include "MemUtils.hh"
#include "Help.hh"
#include "DtMailHelp.hh"
#include "SelectFileCmd.h"
#include "DtMailGenDialog.hh"
#include "Application.h"
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <Xm/RepType.h>
#include <Xm/ScrolledW.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/SeparatoG.h>
#include <DtMail/IO.hh>
#include <DtMail/DtMailP.hh>
#include <DtMail/PropSource.hh>
#include <DtMail/OptCmd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/param.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include "MailMsg.h"

#define OFFSET 10    // Number of spaces from margin

#ifdef DTMAIL_TOOLTALK
// Time to self destruct
#define DESTRUCT_TIMEOUT   60000    // 1 minutes
#endif

// Pipe used between RFCTransport::childHandler and XtAppAddInput
static int _transfds[2];

static const char *ComposeIcon = "IcMcomp";

struct DefaultHeaders {
    const char *		label;
    const char *		header;
    SendMsgDialog::ShowState	show;
};

static DefaultHeaders DefaultHeaderList[] = {
{ "To",		DtMailMessageTo,	SendMsgDialog::SMD_ALWAYS }, 
{ "Subject",	DtMailMessageSubject,	SendMsgDialog::SMD_ALWAYS },
{ "Cc",		DtMailMessageCc,	SendMsgDialog::SMD_ALWAYS },
{ "Bcc",	DtMailMessageBcc,	SendMsgDialog::SMD_HIDDEN },
{ NULL,		NULL,			SendMsgDialog::SMD_NEVER  }
};

// These headers can never be controlled by the user. They are generated
// by dtmail and the user is not allowed to override the values generated
// by the software. Besides, most users would not have a clue as to what
// a correct value would be.
//
static const char * BlockedHeaders[] = {
    "Content-Length",
    "Content-Type",
    "Content-MD5",
    "Content-Transfer-Encoding",
    "Mime-Version",
    "X-Sun-Charset",
    NULL
};


static DtMailBoolean
block(const char * header)
{
    for (const char ** test = BlockedHeaders; *test; test++) {
	if (strcasecmp(header, *test) == 0) {
	    return(DTM_TRUE);
	}
    }

    return(DTM_FALSE);
}

class CmdList;

Compose theCompose;   // Manages all compose windows.

SendMsgDialog::HeaderList::HeaderList(void)
{
    label = NULL;
    header = NULL;
    value = NULL;
    show = SMD_NEVER;
    form_widget = NULL;
    label_widget = NULL;
    mb_widget = NULL;
    field_widget = NULL;
    display_widget = NULL;
}

SendMsgDialog::HeaderList::HeaderList(const HeaderList & other)
{
    label = NULL;
    header = NULL;
    show = other.show;
    form_widget = other.form_widget;
    label_widget = other.label_widget;
    field_widget = other.field_widget;
    display_widget = other.display_widget;
    mb_widget = other.mb_widget;

    if (other.label) {
	label = strdup(other.label);
    }

    if (other.header) {
	header = strdup(other.header);
    }

    if (other.value) {
	value = strdup(other.value);
    }
}

SendMsgDialog::HeaderList::~HeaderList(void)
{
    if (label) {
	free(label);
    }

    if (header) {
	free(header);
    }

    if (value) {
	free(value);
    }
}
Boolean SendMsgDialog::reservedHeader(const char *label)
{
	for (DefaultHeaders * hl = DefaultHeaderList; hl->label; hl++)
        	if (strcmp(label, hl->label) == 0)
                	return TRUE;
        return FALSE;
}


SendMsgDialog::SendMsgDialog(Tt_message tt_msg) :
    MenuWindow ( "ComposeDialog", tt_msg ), _header_list(16)
{
    _main_form = NULL;
    _num_sendAs = 0;
    _genDialog = NULL;

    _show_attach_area = FALSE;
    _takeDown = FALSE;
    _first_time = TRUE;
    _already_sending = FALSE;
    _edits_made = FALSE;

    // Attachment menu
    _attachmentActionsList = NULL;
    _attachmentMenu = NULL;
    _attachmentMenuList = NULL;
    _attachmentPopupMenuList = NULL;
    _textPopupMenuList = NULL;

    _attachment_popup_title = NULL;
    _compose_popup_title = NULL;
    
    _att_show_pane = NULL;
    _att_select_all = NULL;
    _att_save = NULL;
    _att_add = NULL;
    _att_delete = NULL;
    _att_undelete = NULL;
    _att_rename = NULL;
    _att_select_all = NULL;
    _auto_save_interval = NULL;
    _auto_save_path = NULL;
    _auto_save_file = NULL;
    _dead_letter_buf = NULL;
    
    _file_include = NULL;
    _save_as_draft = NULL;
    _save_as_template = NULL; 
    _save_as_text = NULL; 
    _savecmdList = NULL; 
    _saveas_menubutton = NULL; 
    _saveas_button = NULL; 
    _file_send = NULL;
    _file_log = NULL;
    _file_sendAs[0] = NULL;
    _file_close = NULL;
    _file_cascade = NULL;

    _format_word_wrap = NULL;
    _format_settings = NULL;
    _format_find_change = NULL;
    _format_spell = NULL;
    _format_menu = NULL;
    _format_cmds = NULL;
    _format_cascade = NULL;
    _templates = NULL;
    _drafts = NULL;
    _signature = NULL;
    _templateFile = NULL;
    _templateFullPath = NULL;
    _draftFile = NULL;
    _additionalfields = NULL;

    _aliasCmd = NULL;
    _sigCmd = NULL;
    _vacationCmd = NULL;
    _aliasCmdList = NULL;

    _separator = new SeparatorCmd( "Separator", TRUE );

    // Help Menu
    _overview = NULL;
    _tasks = NULL;
    _reference = NULL;
    _messages = NULL;
    _on_item = NULL;
    _using_help = NULL;
    _about_mailer = NULL;


    // Now we need to get the additional headers from the property system.
    //   
    DtMailEnv error;
    DtMail::Session * d_session = theRoamApp.session()->session();
    DtMail::MailRc * mail_rc = d_session->mailRc(error);
 
    char * value=NULL;  
    mail_rc->getValue(error, "additionalfields", &value);
 
    DtVirtArray<PropStringPair *> results(8);
    if (error.isNotSet()) {
        _additionalfields = value;
        parsePropString(value, results);
    }
 
    // Load the header list with the predefined/fixed headers.
    //
    for (DefaultHeaders * hl = DefaultHeaderList; hl->label; hl++) {
	HeaderList * copy_hl = new HeaderList;
	copy_hl->label = strdup(hl->label);
	copy_hl->value = getPropStringValue(results, hl->label);
	copy_hl->header = strdup(hl->header);
	copy_hl->show = hl->show;
	_header_list.append(copy_hl);
    }

    if (error.isNotSet()) {

	for (int mrc = 0; mrc < results.length(); mrc++) {
	    PropStringPair * psp = results[mrc];

	    if (!reservedHeader(psp->label)) {
	    	HeaderList * copy_hl = new HeaderList;
	    	copy_hl->label = strdup(psp->label);
	    	copy_hl->header = strdup(psp->label);
	    	if (psp->value) 
			copy_hl->value = strdup(psp->value);
	    	copy_hl->show = SMD_HIDDEN;
	    	_header_list.append(copy_hl);
	    }
	}

	while(results.length()) {
	    PropStringPair * psp = results[0];
	    delete psp;
	    results.remove(0);
	}
    }
    else 
	_additionalfields = NULL;

    extern void alias_stuffing_func(char *, void *, void *);
    _aliases = new DtVirtArray<PropStringPair *>(10);
    mail_rc->getAliasList(alias_stuffing_func, _aliases);
    if (_aliases->length()) {
    	_aliasCmdList = new CmdList ("aliases", "aliases");
    	addAliases();
    }
    else 
	_aliasCmdList = NULL;
}

SendMsgDialog::~SendMsgDialog()
{
    
    delete _genDialog;
    
    // File
    
    delete _file_include;
    delete _file_send;
    delete _file_log;
    delete _file_close;
    
    // Edit
    
    delete _edit_undo;
    delete _edit_cut;
    delete _edit_copy;
    delete _edit_paste;
    delete _edit_clear;
    delete _edit_delete;
    delete _edit_select_all;
    
    // Attachment
    
    delete _att_save;
    delete _att_add;
    delete _att_delete;
    delete _att_undelete;
    delete _att_rename;
    delete _att_select_all;
    
    delete _attachmentActionsList;
    
    // Format
    
    delete _format_word_wrap;
    delete _format_settings;
    delete _format_find_change;
    delete _format_spell;
    delete _format_cmds;
    delete _aliasCmd;
    delete _sigCmd;
    delete _vacationCmd;
    delete _aliasCmdList;
    
    // Help
    delete      _overview;
    delete      _tasks;
    delete      _reference;
    delete      _messages;
    delete      _on_item;
    delete      _using_help;
    delete      _about_mailer;
 

    // Things created by createWorkArea()
	//[Commented out for bugid 1214276, for FCS1.0.1
    //delete _my_editor;
    delete _send_button;
    delete _close_button;
    delete _savecmdList;
    
    delete  _auto_save_path;
    if (_auto_save_file) {
        delete _auto_save_file;
    }
    if (_dead_letter_buf) {
        delete _dead_letter_buf;
    }

    // Loop through the array of header lists and 
    // clean up the memory used for each HeaderList.
    //
    while(_header_list.length()) {
        HeaderList * header = _header_list[0];
        _header_list.remove(0);
        delete header;
    }

    delete _attachmentMenuList;
    delete _attachmentPopupMenuList;
    delete _textPopupMenuList;
    delete _attachment_popup_title;
    delete _compose_popup_title;
    delete _templates;
    delete _drafts;
    if (_additionalfields != NULL)
	free(_additionalfields);
    delete _separator;
}
void
SendMsgDialog::nextTextField(Widget w)
{
    HeaderList *hl=NULL;
    int i, len = _header_list.length();
    Boolean found_it = False;

    for (i=0; i < len; i++) {
	hl = _header_list[i];
	// Find the next managed text field to set input focus
        if (hl->field_widget == w) {
    		for (int j=i+1; j < len; j++) {
			hl = _header_list[j];
			if (hl && (hl->show == SMD_ALWAYS || 
				hl->show == SMD_SHOWN)) {
    				XmProcessTraversal(hl->field_widget, 
					XmTRAVERSE_CURRENT);
				found_it = True;
				break;
			}
		}
	}
    }
    if (!found_it)
    	(void) XmProcessTraversal(w, XmTRAVERSE_NEXT_TAB_GROUP);
}

// Callback for each header row
void
header_form_traverse(Widget w, XtPointer cd, XtPointer)
{
    SendMsgDialog * self = (SendMsgDialog *)cd;

    self->nextTextField(w);
}

// Create Message Handle with empty first body part.
DtMail::Message *
SendMsgDialog::makeMessage(void)
{
    DtMailEnv error;
    DtMail::Session * d_session = theRoamApp.session()->session();
    DtMailBuffer mbuf;
    
    mbuf.buffer = NULL;
    mbuf.size = 0;
    
    DtMail::Message * msg = d_session->messageConstruct(error,
							DtMailBufferObject,
							&mbuf,
							NULL,
							NULL,
							NULL);
    if (error.isSet() || !msg) {
	return(NULL);
    }
    
    DtMail::BodyPart * bp = msg->newBodyPart(error, NULL);
    
    // For now, reserve the first body part for text.
    setFirstBPHandled(TRUE);
    
    return(msg);
}

// Before Submitting, call this routine.
// This routine does not save attachment(s).
// Check point routine should also call this routine.
void
SendMsgDialog::updateMsgHnd()
{
    DtMailEnv error;
    DtMail::Envelope * env = _msgHandle->getEnvelope(error);
    int textLen;

    storeHeaders();

    char * widget_text = this->text();
	if (widget_text == NULL) {
		textLen = 0;
	}
    else if (widget_text && *widget_text == NULL) {
	textLen = 0;
	widget_text = NULL;
    }
    else
    	textLen = strlen(widget_text);
    
    // Even if textlen is 0 because user has cleared all previous text,
    // need to setContents again to clear first BP.  Otherwise, deleted
    // text will show up. 

    // Get FirstBodyPart and fill it up.
    DtMail::BodyPart *bp = _msgHandle->getFirstBodyPart(error);
    bp->setContents(error, widget_text, textLen, NULL, NULL, 0, NULL);
    setFirstBPHandled(TRUE);
    
    XtFree(widget_text);
}

// Before Submitting, also call this routine.
// This routine fills the message handle with attachment(s).
// The reason why we get attachment(s) from the Back End before submission
// is in case the attachment(s) are updated during the compose session.
// If changes are saved back, then the AttachArea class would register the
// update with Back End.  So BE always has the latest attachments.
void
SendMsgDialog::updateMsgHndAtt()
{
    if ( _inclMsgHandle == NULL )
	return;    // No attachments
    
    DtMailEnv error;
    DtMail::BodyPart *msgBP;
    DtMail::BodyPart *inclBP = _inclMsgHandle->getFirstBodyPart(error);
    const void *contents;
    unsigned long len;
    char *type;
    char *name;
    int mode;
    char *desc;
    
    // If message handle to be copied, _inclMsgHandle, does not have its first
    // body part as text, then get its first body part content and copy it.
    // If it contains text, then skip it.
    if ( !_inclMsgHasText ) {
	inclBP->getContents(error, &contents, &len, &type, &name, &mode, &desc);
	if ( _firstBPHandled ) {
	    msgBP = _msgHandle->newBodyPart(error, _lastAttBP); 
	} else {
	    msgBP = _msgHandle->getFirstBodyPart(error);
	}
	msgBP->setContents(error, contents, len, type, name, mode, desc);
	setLastAttBP(msgBP);
	free(name);
	free(type);
    }
    
    // Continue to get the next body part and start copy.
    while ((inclBP = _inclMsgHandle->getNextBodyPart(error, inclBP)) != NULL) {
	inclBP->getContents(error, &contents, &len, &type, &name, &mode, &desc);
	if ( _firstBPHandled ) {
	    msgBP = _msgHandle->newBodyPart(error, _lastAttBP); 
	} else {
	    msgBP = _msgHandle->getFirstBodyPart(error);
	}
	msgBP->setContents(error, contents, len, type, name, mode, desc);
	setLastAttBP(msgBP);
	free(name);
	free(type);
    }  // end of while loop
}

// Update the _lastAttBP pointer so that subsequent newBodyPart() calls can
// return a message body part following the last body part.
void
SendMsgDialog::setLastAttBP()
{
    DtMailEnv error;
    _lastAttBP = _msgHandle->getNextBodyPart(error, _lastAttBP);
}

// This sister routine is needed when _lastAttBP has not been initialized.
void
SendMsgDialog::setLastAttBP(DtMail::BodyPart *bp)
{
    _lastAttBP = bp;
}

// Initialize _msgHandle
void
SendMsgDialog::setMsgHnd()
{
    _msgHandle = makeMessage();
    _inclMsgHandle = NULL;
    _inclMsgHasText = FALSE;
    _lastAttBP = NULL;
}

// Set timeout to ten minutes (as milliseconds)
//
void
SendMsgDialog::startAutoSave(void)
{
    mkAutoSavePath();
    _auto_save_interval = XtAppAddTimeOut(theApplication->appContext(),
					  getAutoSaveInterval(),
					  autoSaveCallback,
					  this);
}

void
SendMsgDialog::stopAutoSave(void)
{
    if (!_auto_save_interval) {
	return;
    }
    
    XtRemoveTimeOut(_auto_save_interval);
    _auto_save_interval = NULL;
    
    unlink(_auto_save_file);
    free(_auto_save_file);
    _auto_save_file = NULL;
}

static const char * BACKUP_DEAD_LETTER = "./DeadLetters";
static const char * PREFIX = "mail.dead.letter";

void
SendMsgDialog::mkAutoSavePath(void)
{
    // First, see if we need to set up the path.
    //

    if (!_auto_save_path) {
	_auto_save_path = theRoamApp.getResource(DeadletterDir, TRUE);

	if (!_auto_save_path) {
    		DtMailEnv error;
    		DtMail::Session * d_session = theRoamApp.session()->session();
	    	_auto_save_path = d_session->expandPath(error, BACKUP_DEAD_LETTER);
	}
    }
    
    // If we still have a path, punt.
    // 
    if (!_auto_save_path) {
	return;
    }
    
    if (SafeAccess(_auto_save_path, W_OK) != 0) {
	if (errno != ENOENT) {
	    // Not an error we can overcome here.
	    //
	    free(_auto_save_path);
	    _auto_save_path = NULL;
	    return;
	}
	
	if (mkdir(_auto_save_path, 0700) < 0) {
	    free(_auto_save_path);
	    _auto_save_path = NULL;
	    return;
	}
    }
    
    // Now we run through the possible file names until we hit pay dirt.
    //
    _auto_save_file = new char[strlen(_auto_save_path) + 100];
    for (int suffix = 1; ; suffix++) {
	sprintf(_auto_save_file, "%s/%s.%d", _auto_save_path, PREFIX, suffix);
	if (SafeAccess(_auto_save_file, F_OK) != 0) {
	    break;
	}
    }
}

void
SendMsgDialog::loadDeadLetter(const char * path)
{
    _auto_save_file = strdup(path);
    parseNplace(path);
    
    _auto_save_interval = XtAppAddTimeOut(theApplication->appContext(),
					  getAutoSaveInterval(),
					  autoSaveCallback,
					  this);
}

void
SendMsgDialog::autoSaveCallback(XtPointer client_data, XtIntervalId * id)
{
    SendMsgDialog * self = (SendMsgDialog *)client_data;
    
    if (self->_auto_save_interval != *id) {
	// Random noise. Ignore it.
	return;
    }
    
    self->doAutoSave();
    
    self->_auto_save_interval = XtAppAddTimeOut(theApplication->appContext(),
						self->getAutoSaveInterval(),
						autoSaveCallback,
						self);
}

void
SendMsgDialog::doAutoSave(void)
{
    DtMail::Session * d_session = theRoamApp.session()->session();
    DtMailEnv error;

    busyCursor();
    setStatus(catgets(DT_catd, 3, 70, "Writing dead letter..."));

    updateMsgHnd();
    
    RFCWriteMessage(error, d_session, _auto_save_file, _msgHandle, FALSE);
    normalCursor();
    clearStatus();
}

int
SendMsgDialog::getAutoSaveInterval(void)
{
    DtMailEnv error;
    // Initialize the mail_error.
    
    error.clear();
    
    DtMail::Session * d_session = theRoamApp.session()->session();
    DtMail::MailRc * mail_rc = d_session->mailRc(error);
    
    int save_interval;
    char * value=NULL;
    mail_rc->getValue(error, "composeinterval", &value);
    if (error.isSet() || value == NULL) {
	save_interval = 10 * 60 * 1000; // 10 minutes
    }
    else {
	save_interval = (int) strtol(value, NULL, 10) * 60 * 1000;
	save_interval = (save_interval <= 0) ? 10 * 60 * 1000 : save_interval;
    }
    if (value) free(value);
    
    return(save_interval);
}

void
SendMsgDialog::setInclMsgHnd(DtMail::Message *msg, Boolean text)
{
    _inclMsgHandle = msg;
    _inclMsgHasText = text;
}

void
SendMsgDialog::setFirstBPHandled(Boolean handle)
{
    _firstBPHandled = handle;
}

void
SendMsgDialog::setHeader(const char * name, const char * value, DtMailBoolean append_header)
{
    // See if this header is in the list. If so, set the widget for
    // it.
    //
    int slot = lookupHeader(name);
    if (slot < 0) {
	return;
    }

    HeaderList * hl = _header_list[slot];
    if (hl->show == SMD_NEVER) {
	// The user removed this header via props.
	//
	return;
    }


    char *field_value = NULL;
    if (hl->field_widget) {
        XtRemoveCallback(hl->field_widget,
	  XmNvalueChangedCallback, editorContentsChangedCB,
	  this);
	if (append_header) {
		if ((field_value = XmTextGetString(hl->field_widget)) &&
			*field_value) {
			char *new_value = (char*)malloc(strlen(field_value) 
					+ strlen(value) + 2);
			sprintf(new_value, "%s %s", field_value, value); 
			XmTextSetString(hl->field_widget, new_value);
			hl->value = new_value;
		}
			
	}
	if (!field_value || !(*field_value)) {
		XmTextSetString(hl->field_widget, (char*)value);
		hl->value = strdup(value);
 	}
	if (field_value)
		free(field_value);

	if (hl->show == SMD_HIDDEN)
		changeHeaderState(name);
        XtAddCallback(hl->field_widget,
	  XmNvalueChangedCallback, editorContentsChangedCB,
	  this);
    }

}

void
SendMsgDialog::setHeader(const char * name, DtMailValueSeq & value, DtMailBoolean append_header)
{
    if (value.length() == 0) {
	return;
    }

    if (value.length() == 1) {
	setHeader(name, *(value[0]), append_header);
    }
    else {
	int max_len = 0;
	for (int slen = 0; slen < value.length(); slen++) {
	    max_len += strlen(*(value[slen]));
	}

	char * new_str = new char[max_len + (value.length() * 3)];

	strcpy(new_str, "");
	for (int copy = 0; copy < value.length(); copy++) {
	    if (copy != 0) {
		strcat(new_str, " ");
	    }

	    strcat(new_str, *(value[copy]));
	}

	setHeader(name, new_str, append_header);
	delete new_str;
    }
}

void
SendMsgDialog::loadHeaders(DtMail::Message * input,
		   DtMailBoolean load_all, DtMailBoolean append_header)
{
    // We are going to go through every header in the message.
    // If it is not one of the headers we block, then we will
    // load it into the header pane, depending on the setting of
    // load_all. If true, then we load every header we allow and
    // create new dynamic headers as necessary. If load_all is false,
    // then we only load headers that already are available in the
    // memory pane.
    //
    DtMailEnv error;
    DtMail::Message * msg = (input ? input : _msgHandle);
    DtMail::Envelope * env = msg->getEnvelope(error);
    DtMailHeaderHandle hnd;

    int hcount = 0;
    char * name;
    DtMailValueSeq value;

    for (hnd = env->getFirstHeader(error, &name, value);
	 error.isNotSet() && hnd;
	 hnd = env->getNextHeader(error, hnd, &name, value)) {

	// Always ignore the Unix from line.
	//
	if (hcount == 0 &&
	    strcmp(name, "From") == 0) {
	    free(name);
	    value.clear();
	    hcount += 1;
	    continue;
	}

	hcount += 1;

	// See if the name is one we always block.
	//
	if (block(name)) {
	    free(name);
	    value.clear();
	    continue;
	}

	int slot = lookupHeader(name);
	if (slot < 0) {
	    // We dont have a place for this information. We may need
	    // to create a new header.
	    //
	    if (load_all) {
		HeaderList *hl = new HeaderList;
		hl->label = strdup(name);
		hl->header = strdup(name);
		hl->show = SMD_HIDDEN;
		_header_list.append(hl);
		createHeaders(_header_form);
		doDynamicHeaderMenus();
	    }
	    else {
		free(name);
		value.clear();
		continue;
	    }
	}

	setHeader(name, value, append_header);
	free(name);
	value.clear();
    }
}

void
SendMsgDialog::storeHeaders(DtMail::Message * input)
{
    DtMail::Message * msg = (input ? input : _msgHandle);
    DtMailEnv error;
    DtMail::Envelope * env = msg->getEnvelope(error);

    // Walk through the headers. Fetch the strings from the ones
    // that are visible to the user and stuff them into the
    // message.
    //
    for (int scan = 0; scan < _header_list.length(); scan++) {
	HeaderList * hl = _header_list[scan];
	if (hl->show != SMD_ALWAYS && hl->show != SMD_SHOWN) {
	    continue;
	}

	char * value = XmTextGetString(hl->field_widget);

	// If the header has a value, we want to set the value
	// in the back end.  Otherwise, we don't want to send
	// out a blank header, so we remove it.
	if (value && *value) {
	    env->setHeader(error, hl->header, DTM_TRUE, value);
	    if (value) XtFree(value);
	}
	else {
	    env->removeHeader(error, hl->header);
	}
    }
}

void
SendMsgDialog::changeHeaderState(const char * name)
{

    int slot = lookupHeader(name);
    if (slot < 0) {
	return;
    }

    HeaderList * hl = _header_list[slot];
    if (hl->show == SMD_ALWAYS || hl->show == SMD_NEVER) {
	return;
    }

    // If the user is trying to remove a header with a value other than
    // the default, we should at least ask.
    //
    if (hl->show == SMD_SHOWN) {
	char * value=NULL;
	XtVaGetValues(hl->field_widget,
		      XmNvalue, &value,
		      NULL);
	if (value && *value) {
	    if (!hl->value || strcmp(value, hl->value) != 0) {
		char buf[200];
		sprintf(buf, 
			catgets(DT_catd, 2, 17, 
				"You have edited \"%s\". Delete anyway?"), 
			hl->label);
		_genDialog->setToWarningDialog(catgets(DT_catd, 3, 71,
						       "Mailer"),
					       buf);
		char * helpId = DTMAILHELPERROR;
		int answer = _genDialog->post_and_return(
					    catgets(DT_catd, 3, 72, "OK"), 
					    catgets(DT_catd, 3, 73, "Cancel"), 
					    helpId);
		if (answer == 2) {
		    XtFree(value);
		    return;
		}
	    }
	}
        if (value)
		XtFree(value);
    }

    // Now we need to toggle the current state of the header.
    //
    char label[100];
    char button_name[100];

    if (hl->show == SMD_SHOWN) {
	XtUnmanageChild(hl->form_widget);
	hl->show = SMD_HIDDEN;
 	sprintf(label, "%s ", catgets(DT_catd, 1, 228, "Add"));
	// Create old label for changing
    	sprintf(button_name, "%s ", catgets(DT_catd, 1, 228, "Delete"));
    }
    else {
	XtManageChild(hl->form_widget);
	hl->show = SMD_SHOWN;
	sprintf(label, "%s ", catgets(DT_catd, 1, 229, "Delete"));
	// Create old label for changing
    	sprintf(button_name, "%s ", catgets(DT_catd, 1, 228, "Add"));
    }
    strcat(label, hl->label);
    strcat(label, ":");

    strcat(button_name, hl->label);
    strcat(button_name, ":");

    justifyHeaders();
    reattachHeaders();

    _menuBar->changeLabel(_format_menu, button_name, label);
}
void
SendMsgDialog::setStatus(const char * str)
{
    char *tmpstr = strdup(str);
    XmString label = XmStringCreateLocalized(tmpstr);

    XtVaSetValues(_status_text,
		  XmNlabelString, label,
		  NULL);

    XmUpdateDisplay(baseWidget());
    XmStringFree(label);
    free(tmpstr);
}

void
SendMsgDialog::clearStatus(void)
{
    setStatus(" ");
}

Boolean
SendMsgDialog::isMsgValid(void)
{
    if (!_msgHandle)
        return FALSE;
    else
        return TRUE;
}


// Sendmail is exed'd and this parent process returns immediately.  When
// the sendmail child exits, this function is called with the pid of the 
// child and its status.
void
SendMsgDialog::sendmailErrorProc (int, int status, void *data)
{
    SendMsgDialog *smd = (SendMsgDialog *)data;
    char *helpId = NULL;
    char buf[2048], *file;

    smd->_first_time = TRUE;
    smd->_takeDown = FALSE;

 
    file = smd->getDraftFile();
    // We must strdup it because if gets freed when the compose popup goes away.
    // We have to do this because if theres an error, the compose gets remapped
    // and then we reset it at the bottom of this routine, so the draft file
    // name doesnt get lost.
    if (status != DTME_NoError && file) file = strdup(file);

    // pid is the child process (sendmail) id
    // status is the exit status of the child process
    // data is any extra data associated with the child process
    switch (status) {
        case DTME_NoError:
	    // The mail was successfully sent so return the compose
	    // window to the cache and then return.

	    // Only delete the draft is there was no error.
	    if (file) {
                char *dir = (char*)theRoamApp.getResource(DraftDir, TRUE);
                sprintf(buf, "%s/%s", dir, file);
                free(dir);
                // Unlink Draft file
                if (unlink(buf) < 0) {
                        sprintf(buf, catgets(DT_catd, 3, 120,
                        "Unable to destroy the draft file:\n%s"), buf);
                        smd->_genDialog->setToErrorDialog(
                                catgets(DT_catd, 3, 24, "Mailer"), buf);
                        smd->_genDialog->post_and_return(
                                catgets(DT_catd, 3, 74, "OK"), DTMAILHELPDRAFTREMOVE );
                }
		free(file);
		smd->setDraftFile(NULL);
            }

	    smd->_send_button->activate();
	    smd->_close_button->activate();
	    smd->goAway(FALSE);
	    return;

        case DTME_BadMailAddress:
	    /* NL_COMMENT
	     * There was an error in one or more of the email addresses.  
	     * Ask the user to type in a valid address and try again.
	     */
	    sprintf(buf, catgets(DT_catd, 5, 5, 
"Some of the addresses in the message are incorrect,\n\
and do not refer to any known users in the system.\n\
Please make sure all of the addresses are valid and try again."));
  	    helpId = DTMAILHELPBADADDRESS;

	    break;
 
        case DTME_NoMemory:
	    /* NL_COMMENT
	     * Mailer ran out of memory.  Ask the user to quit some other
	     * applications so there will be more memory available.
	     */
 
            sprintf(buf, catgets(DT_catd, 5, 6,
"Mailer does not have enough memory\n\
available to send this message.\n\
Try quitting other applications and\n\
resend this message."));
            helpId = DTMAILHELPNOMEMORY;
		
	    break;

	case DTME_TransportFailed:
	default:
	    /* NL_COMMENT
	     * There was an error from the mail transport (sendmail).
	     */

	    sprintf(buf, catgets(DT_catd, 5, 7,
"An error occurred while trying to send your message.\n\
Check to make sure the message was received.  If not,\n\
you may have to resend this message."));
	    helpId = DTMAILHELPTRANSPORTFAILED;
    }

    // popup the compose window
    // Set the window state to Normal before manage the window
    XtVaSetValues(smd->baseWidget(), XtNinitialState, NormalState, NULL);
    smd->manage();
    smd->_send_button->activate();
    smd->_close_button->activate();

    // popup the error dialog
    smd->_genDialog->setToErrorDialog(catgets(DT_catd, 2, 21, "Mailer"),
			     (char *) buf);
    smd->_genDialog->post_and_return(catgets(DT_catd, 3, 76, "OK"), helpId);
    if (file)
	smd->setDraftFile(file);
	
}


void
SendMsgDialog::send_message(const char * trans_impl, int trans_type)
{
    DtMailEnv error;
    DtMailOperationId id;
    DtMail::Transport * mail_transport;
    DtMail::Session * d_session = theRoamApp.session()->session();
    DtMailEditor *editor = this->get_editor();
    AttachArea *attachArea = editor->attachArea();
    int numPendingActions, answer;
    char *helpId = NULL;
    char buf[2048], buf2[1024];
    static int first_time = 1;

    //
    // Check to see if we are already trying to send from this
    // SendMsgDialog.  If we are, we don't want to do it again.
    if (_already_sending) {
	return;
    } else {
	_already_sending = TRUE;
    }

    // Unmap the window.
    // If you do it in the reverse order, users get confused coz
    // the window remains behind for a couple seconds after hitting 
    // "Send" and it ...
    this->unmanage();
    _send_button->deactivate();
    _close_button->deactivate();

    // Check if message has addressees.  If it doesn't, what sense does
    // it make to Send it? 

    if (!this->hasAddressee()) {
	// Message has no valid addressee.  Pop up error dialog.

 	sprintf(buf, catgets(DT_catd, 5, 8,
		 "Try Send after specifying  recipient(s) of the message in \nthe To:, Cc:, or Bcc: fields."));

	helpId = DTMAILHELPNEEDADDRESSEE;

        // Need help tag for above HelpID.
	
	// popup the compose window
        // Set the window state to Normal before manage the window
        XtVaSetValues(this->baseWidget(), XtNinitialState, NormalState, NULL);
	this->manage();
	_send_button->activate();
	_close_button->activate();

	_genDialog->setToErrorDialog(catgets(DT_catd, 2, 21, "Mailer"),
				     (char *) buf);
	_genDialog->post_and_return(catgets(DT_catd, 3, 76, "OK"), helpId);

	// Reset the flag before we return.
	_already_sending = FALSE;

	return;
    }

    // Since we are Send-ing, the SMD can be taken down later without
    // checking for dirty...

    _first_time = TRUE;
    _takeDown = TRUE;

    stopAutoSave();
    
    // Just get text from text widget; attachment BPs are filled
    // already when they are included/forwarded/added.
    
    updateMsgHnd();
    char *rvalue=NULL;
    error.clear();
    d_session->mailRc(error)->getValue(error, "displaymsgsizewarning", &rvalue);
    if (rvalue) {
    	free(rvalue); rvalue = NULL;
    }
    if (error.isNotSet()) {
	//  display the warning popup 
    	d_session->mailRc(error)->getValue(error, "msgsizelimit", &rvalue);
    	if (error.isNotSet() && rvalue && *rvalue) {
    		char *type, *name, *desc;
    		const void *contents;
    		int mode, len_total=0; 
    		unsigned long len;
		DtMail::BodyPart *inclBP = _msgHandle->getFirstBodyPart(error);
		while ( error.isNotSet() && inclBP != NULL ) {
			if (inclBP->flagIsSet(error, DtMailBodyPartDeletePending)) {
				inclBP = _msgHandle->getNextBodyPart(error, inclBP);
                		continue;
			}
			inclBP->getContents(error, &contents, &len, &type, &name, 
					&mode, &desc);
			len_total += len;
			free(name); free(type);
			inclBP = _msgHandle->getNextBodyPart(error, inclBP);
     		}  
		long lrvalue = strtol(rvalue, NULL, 10);
		if (len_total > (lrvalue*1000)) {
			char len_total_str [50];
			int bytes;
			if (len_total < 1000) 
        			sprintf(len_total_str, "%d", len_total);
    			else if (len_total < 1000000) {
				bytes = (int)((double)len_total/(double)1000);
        			sprintf(len_total_str, "%dKbytes", bytes);
			}
    			else {
				bytes = (int)((double)len_total/(double)1000000);
        			sprintf(len_total_str, "%dMbytes", bytes);
			}
    			sprintf(buf, catgets( DT_catd, 5, 9, 
                        "This size of this message ( %s ) exceeds the\nmessage size limit ( %dKbytes ) you have set\nin your mail options. You can:" ), len_total_str, lrvalue);
			_genDialog->setToQuestionDialog(
                        	catgets(DT_catd, 5, 9, "Mailer"),
                        	buf);
			helpId = DTMAILHELPMSGSIZELIMIT;
        		answer = _genDialog->post_and_return(catgets(DT_catd, 1, 230, "Send"),
				 catgets(DT_catd, 3, 19, "Cancel"), helpId);

        		if (answer == 2) {
            			// Cancel selected
            			// Reset the flag before we return.
				_already_sending = FALSE;
				// popup the compose window
				this->manage();
				_send_button->activate();
				_close_button->activate();
            			return;
			}
		}
	}
	if (rvalue != NULL) free (rvalue);
    }
    error.clear();
		
    // Check if there are any pending attachments (open, print....)
    // If there are, pop up the dialog.
    // If the user wants to Send the message as is, continue with  the
    // submission process. 
    // If the user opted to Cancel, then return.
    
    
    numPendingActions = attachArea->getNumPendingActions();
    sprintf(buf, catgets(
	                DT_catd, 
	                3, 
	                77, 
                        "You have an attachment open that may have unsaved changes.\nSending this message will break the connection to the open\n attachment. Any unsaved changes will not be part of the\n message. You can use Save As to save Changes after the\n connection is broken, but the changes will not be part of\n the attachment." ));
    
    while (numPendingActions != 0) {
	// popup the compose window
	this->manage();
	_send_button->activate();
	_close_button->activate();

	/* NL_COMMENT
	 * The user tried to send a messages without saving changes in
	 * some open attachments.  This warning makes sure that is what
	 * the user intended.
	 */
	
	_genDialog->setToQuestionDialog(
			catgets(DT_catd, 5, 1, "Mailer"),
			buf);
	helpId = DTMAILHELPPENDINGACTIONS;
	
	answer = _genDialog->post_and_return(helpId);
	
	if (answer == 1) {
	    // OK selected
	    numPendingActions = 0;
	    this->unmanage();
	    _send_button->deactivate();
	    _close_button->deactivate();
	}
	else if (answer == 2) {
	    // Cancel selected
	    // Reset the flag before we return.
	    _already_sending = FALSE;
	    return;
	}
    }

    rvalue = NULL;
    d_session->mailRc(error)->getValue(error, "expert", &rvalue);
    if (rvalue) free(rvalue);
    if (error.isSet()) {
    	char *file = getDraftFile();
    	if (file) {
		char *dir = (char*)theRoamApp.getResource(DraftDir, TRUE);
		sprintf(buf2, "%s/%s", dir, file);
		free(dir);
		sprintf(buf, catgets( DT_catd, 3, 118, 
		"Sending the message will destroy the Draft:\n%s"), buf2);
		_genDialog->setToQuestionDialog( catgets(DT_catd, 5, 9, "Mailer"),
			buf);
		answer = _genDialog->post_and_return(
			catgets(DT_catd, 3, 119, "Send and Destroy"),
			catgets(DT_catd, 3, 19, "Cancel"), DTMAILHELPSENDDELETEDRAFT);
		if (answer == 2) { 
			// Cancel selected
			// Reset the flag before we return.
			_already_sending = FALSE;
			// popup the compose window
			this->manage();
			_send_button->activate();
			_close_button->activate();
			return;
		}
	}
        error.clear();
    }
   
    // Determine which transport mechanism will be used.
    if ( trans_type ) {   // Default
	// Only register XtAppAddInput once
        if (first_time)
        {
	    // Create the pipe between the RFCTransport::childHandler
	    // and XtAppAddInput
	    if (pipe(_transfds) < 0) {
	        // If this failed, make sure we try to initialize again later.
		error.setError(DTME_NoMemory); 	
    		popupMemoryError (error);

		// Reset the flag before we return.
		_already_sending = FALSE;
		return;
	    }

	    // Call ourproc when input is available on _transfds[0]
	    XtAppAddInput(XtWidgetToApplicationContext(this->_main_form),
		_transfds[0], (XtPointer)XtInputReadMask,
	        (XtInputCallbackProc)
		    (theRoamApp.default_transport()->getSendmailReturnProc()),
		NULL);
	    first_time = 0;
	}

	// Tell the transport where the callback is
	theRoamApp.default_transport()->initTransportData( _transfds,
		&(SendMsgDialog::sendmailErrorProc), this);
	id = theRoamApp.default_transport()->submit(error,
		_msgHandle, _log_msg);

    } else {
	// Construct transport
	mail_transport = d_session->transportConstruct(error,
		trans_impl, RoamApp::statusCallback, this);

	// Only register XtAppAddInput once
        if (first_time)
        {
	    // Create the pipe between the RFCTransport::childHandler
	    // and XtAppAddInput
	    if (pipe(_transfds) < 0) {
	        // If this failed, make sure we try to initialize again later.
		error.setError(DTME_NoMemory); 	
    		popupMemoryError (error);

		// Reset the flag before we return.
		_already_sending = FALSE;
		return;
	    }

	    // Call ourproc when input is available on _transfds[0]
	    XtAppAddInput(XtWidgetToApplicationContext(this->_main_form),
		_transfds[0], (XtPointer)XtInputReadMask,
		(XtInputCallbackProc)(mail_transport->getSendmailReturnProc()),
		NULL);
	    first_time = 0;
	}

	// Tell the transport where the callback is
	mail_transport->initTransportData(_transfds,
		&(SendMsgDialog::sendmailErrorProc), this);
	id = mail_transport->submit(error, _msgHandle, _log_msg);
    }
  
    popupMemoryError (error);

    // Reset the flag before we return.
    _already_sending = FALSE;
}

void
SendMsgDialog::popupMemoryError(DtMailEnv &error)
{
    char *helpId = NULL;
    char buf[2048];

    _takeDown = FALSE;

    // Popup an error dialog if necessary. 
    if (error.isSet()) {
	if ((DTMailError_t)error == DTME_NoMemory) {

	    /* NL_COMMENT
	     * Mailer ran out of memory.  Ask the user to quit some other
	     * applications so there will be more memory available.
	     */
	    sprintf(buf, catgets(DT_catd, 5, 6,
"Mailer does not have enough memory\n\
available to send this message.\n\
Try quitting other applications and\n\
resend this message."));
	    helpId = DTMAILHELPNOMEMORY;
         }
	 else {
		/* NL_COMMENT
             * An unidentifiable error happened during mail transport
             * Pop it up *as is* (need to update this function if so)
             */
            sprintf(buf, "%s", (const char *)error);
            helpId = DTMAILHELPERROR;
        }

	// popup the compose window
	this->manage();
	_send_button->activate();
	_close_button->activate();

	// popup the error dialog
	this->_genDialog->setToErrorDialog(catgets(DT_catd, 2, 21, "Mailer"),
		(char *) buf);
	this->_genDialog->post_and_return(catgets(DT_catd, 3, 76, "OK"),
		helpId);
    }	
}

void
SendMsgDialog::editorContentsChangedCB(Widget, XtPointer cd, XtPointer)
{
     SendMsgDialog * self = (SendMsgDialog *)cd;
     self->_edits_made = TRUE;
}

Widget 
SendMsgDialog::createWorkArea ( Widget parent )
{
    FORCE_SEGV_DECL(CmdInterface, ci);
    Widget send_form;
    DtMail::Session *m_session = theRoamApp.session()->session();
    char *value = NULL;
    DtMailEnv error;

    
    // Create the parent form
    
    _main_form = XmCreateForm( parent, "Work_Area", NULL, 0 );
    
    printHelpId("form", _main_form);
    /* add help callback */
    XtAddCallback(_main_form, XmNhelpCallback, HelpCB, DTMAILCOMPOSEWINDOW);
    
    
    // Create the area for status messages.
    //
    _status_form = XtVaCreateManagedWidget("StatusForm",
					   xmFormWidgetClass, _main_form,
					   XmNtopAttachment, XmATTACH_FORM,
					   XmNrightAttachment, XmATTACH_FORM,
					   XmNrightOffset, 2,
					   XmNleftAttachment, XmATTACH_FORM,
					   XmNleftOffset, 2,
					   NULL);

    _status_text = XtVaCreateManagedWidget("StatusLabel",
					   xmLabelWidgetClass, _status_form,
					   XmNtopAttachment, XmATTACH_FORM,
					   XmNbottomAttachment, XmATTACH_FORM,
					   XmNrightAttachment, XmATTACH_FORM,
					   XmNleftAttachment, XmATTACH_FORM,
					   XmNalignment, XmALIGNMENT_BEGINNING,
					   NULL);

    clearStatus();

    Widget s_sep = XtVaCreateManagedWidget("StatusSep",
					   xmSeparatorGadgetClass,
					   _main_form,
					   XmNtopAttachment, XmATTACH_WIDGET,
					   XmNtopWidget, _status_form,
					   XmNleftAttachment, XmATTACH_FORM,
					   XmNrightAttachment, XmATTACH_FORM,
					   NULL);
    
    _header_form = XtVaCreateManagedWidget("HeaderArea",
					   xmFormWidgetClass, _main_form,
					   XmNtopAttachment, XmATTACH_WIDGET,
					   XmNtopWidget, s_sep,
					   XmNleftAttachment, XmATTACH_FORM,
					   XmNrightAttachment, XmATTACH_FORM,
					   NULL);
    printHelpId("header_form", _header_form);

    createHeaders(_header_form);
    
    Widget sep1 = XtVaCreateManagedWidget("Sep1",
					  xmSeparatorGadgetClass,
					  _main_form,
					  XmNtopAttachment, XmATTACH_WIDGET,
					  XmNtopWidget, _header_form,
					  XmNtopOffset, 1,
					  XmNrightAttachment, XmATTACH_FORM,
					  XmNleftAttachment, XmATTACH_FORM,
					  NULL);

    // Create the editor and attach it to the header_form
    
    _my_editor = new DtMailEditor(_main_form, this);
    
    _my_editor->initialize();

    Widget twid = _my_editor->textEditor()->get_editor();
    m_session->mailRc(error)->getValue(error, "composewincols", &value);
    if (error.isNotSet() && value && *value) {
        long cols = strtol(value, NULL, 10);
        XtVaSetValues(twid, DtNcolumns, cols, NULL);
    }
    if (value != NULL)
	free(value);
	
    _my_editor->attachArea()->setOwnerShell(this);
    _my_editor->setEditable(TRUE);
    _my_editor->manageAttachArea();

    _textW = XtNameToWidget(twid, "*text");
    
    XtAddCallback(_textW, XmNmodifyVerifyCallback,
                        editorContentsChangedCB, this);

    // Create a RowCol widget that contains buttons
    
    send_form =  XtCreateManagedWidget("SendForm", 
				       xmFormWidgetClass, _main_form, NULL, 0);
    
    
    // Create the Send and Close buttons as children of rowCol
    
    _send_button = new SendCmd ( "Send", 
	                         catgets(DT_catd, 1, 230, "Send"), 
	                         TRUE, 
	                         this, 
	                         1);
    ci  = new ButtonInterface (send_form, _send_button);
    
    XtVaSetValues(ci->baseWidget(),
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNleftOffset, OFFSET,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNbottomOffset, 5,
		  NULL);
    
    Widget send_bw = ci->baseWidget();
    XtManageChild(send_bw);
    ci->manage();


    // Create Save As button

    _savecmdList = new CmdList ( "Save As", catgets(DT_catd, 1, 243 , "Save As") );

    _save_as_draft = new SaveAsDraftCmd ("Draft Message...",
                      catgets(DT_catd, 1, 241, "Draft Message..."),
                      TRUE,
                      this,
		      baseWidget());

    _savecmdList->add(_save_as_draft);

    _save_as_template = new SaveAsTemplateCmd ("Template...",
                      catgets(DT_catd, 1, 240, "Template..."),
                      TRUE,
                      get_editor()->textEditor(),
                      this,
                      baseWidget());

    _savecmdList->add(_save_as_template);

    _save_as_text = new SaveAsTextCmd ("Text...",
                      catgets(DT_catd, 1, 242, "Text..."),
                      catgets(DT_catd, 1, 126, "Mailer - Save As Text"),
                      TRUE,
                      get_editor()->textEditor(),
                      this,
                      baseWidget());

    _savecmdList->add(_save_as_text);

    _saveas_button  = new MenuButton (send_form, catgets(DT_catd, 1, 243 ,
"Save As"));
    _saveas_menubutton = _saveas_button->addCommands(_savecmdList);

    XtVaSetValues(_saveas_menubutton,
		  XmNleftOffset, 20,
		  XmNleftAttachment, XmATTACH_WIDGET,
		  XmNleftWidget, send_bw,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNbottomOffset, 4,
		  NULL);
    
    // Create Close Button
    _close_button = new CloseCmd (
				  "Close",
	                          catgets(DT_catd, 1, 118, "Close"),
				  TRUE, 
				  this->baseWidget(), 
				  this );

    ci = new ButtonInterface (send_form, _close_button);
    XtVaSetValues(ci->baseWidget(),
		  XmNleftOffset, 20,
		  XmNleftAttachment, XmATTACH_WIDGET,
		  XmNleftWidget, _saveas_menubutton,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNbottomOffset, 5,
		  NULL);
    
    XtManageChild(ci->baseWidget());
    ci->manage();
    
    // Now attach the editor to the form and to the rowCol
    // And the rowCol to the bottom of the form.
    // We need this attachment ordering so that resizes always
    // get transferred to the editor.
    
    Widget wid = _my_editor->container();
    XtVaSetValues(wid,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNtopAttachment, XmATTACH_WIDGET,
		  XmNtopWidget, sep1,
		  XmNtopOffset, 1,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, send_form,
		  NULL);
    
    XtVaSetValues(send_form,
		  XmNbottomAttachment, XmATTACH_FORM,
		  NULL);
    
    
    // Set focus 
    HeaderList * hl = _header_list[0];
    (void) XmProcessTraversal(hl->field_widget, XmTRAVERSE_CURRENT);
    
    // Set the title to be New Message
    this->title(catgets(DT_catd, 1, 119, "New Message"));
    
    XtManageChild(_main_form);

    return _main_form;
}     
void
SendMsgDialog::createAliasMenuButton(MenuButton **button, Widget form, char* label)
{
	if (!(*button)) {
		*button = new MenuButton (form, label);
		XtVaSetValues((*button)->baseWidget(),
			XmNtopAttachment, XmATTACH_FORM,
			XmNbottomAttachment, XmATTACH_FORM,
			XmNleftAttachment, XmATTACH_FORM,
			NULL);
	}
	if (_aliasCmdList) {
		(*button)->addCommands (_aliasCmdList);
		Widget menu = (*button)->getPaneWidget();
		XtVaSetValues(menu,
                	XmNpacking, XmPACK_COLUMN,
                  	XmNorientation, XmVERTICAL,
                  	NULL);
    		XtVaSetValues(menu, XmNuserData, menu, NULL);
    		XtAddCallback(menu, XmNmapCallback,
               	        &RoamMenuWindow::map_menu, NULL);
	}
}
void
SendMsgDialog::createHeaders(Widget header_form)
{
    Widget	previous_form = NULL;
    char	field_name[50];

    for (int header = 0; header < _header_list.length(); header++) {
	HeaderList * hl = _header_list[header];

	// We use SMD_NEVER to indicate the header has disappeared from
	// the list.
	//
	if (hl->show == SMD_NEVER) {
	    continue;
	}

	// If the widgets already exist, then simply manage them.
	if (hl->form_widget) {
	    previous_form = hl->form_widget;
            XtRemoveCallback(hl->field_widget,
                  XmNvalueChangedCallback, editorContentsChangedCB,
                  this);
	    if (hl->value)
	    	XtVaSetValues(hl->field_widget,
                          XmNvalue, hl->value,
                          NULL);
 	    else
	    	XtVaSetValues(hl->field_widget,
                          XmNvalue, "",
                          NULL);
            XtAddCallback(hl->field_widget,
                  XmNvalueChangedCallback, editorContentsChangedCB,
                  this);
	    continue;
	}

        strcpy(field_name, "form_");
        strncat(field_name, hl->label, 45);
        field_name[strlen(hl->label) + 5] = 0;
	if (previous_form == NULL) {
	    // Create a form, attaching it to the top. This is a special
	    // case. Other lines are created attached to the form above
	    // them.
	    hl->form_widget =
		XtVaCreateWidget(field_name,
				 xmFormWidgetClass,
				 header_form,
				 XmNtopAttachment, XmATTACH_FORM,
				 XmNtopOffset, 2,
				 XmNleftAttachment, XmATTACH_FORM,
				 XmNleftOffset, 3,
				 XmNrightAttachment, XmATTACH_FORM,
				 XmNrightOffset, 3,
				 NULL);
	}
	else {
	    hl->form_widget =
		XtVaCreateWidget(field_name,
				 xmFormWidgetClass,
				 header_form,
				 XmNtopAttachment, XmATTACH_WIDGET,
				 XmNtopWidget, previous_form,
				 XmNleftAttachment, XmATTACH_FORM,
				 XmNleftOffset, 3,
				 XmNrightAttachment, XmATTACH_FORM,
				 XmNrightOffset, 3,
				 NULL);
	}

	// The label will be to the left of the form.
	//
	strcpy(field_name, hl->label);
	strcat(field_name, ":");

	// Create a menubutton for the alias list
	if (reservedHeader(hl->label) && 
		strcasecmp("subject", hl->label) != 0)
		createAliasMenuButton(&(hl->mb_widget), hl->form_widget, 
				(char*)field_name);

	XmString label = XmStringCreateLocalized(field_name);
	hl->label_widget = XtVaCreateManagedWidget(hl->label,
		    xmLabelWidgetClass,
		    hl->form_widget,
		    XmNtopAttachment, XmATTACH_FORM,
		    XmNbottomAttachment, XmATTACH_FORM,
		    XmNleftAttachment, XmATTACH_FORM,
		    XmNlabelString, label,
		    NULL);
	XmStringFree(label);
	
	strcpy(field_name, "field_");
	strncat(field_name, hl->label, 43);
	field_name[strlen(hl->label) + 6] = 0;
	
	hl->field_widget =
	    XtVaCreateManagedWidget(field_name,
				    xmTextFieldWidgetClass,
				    hl->form_widget,
				    XmNtraversalOn,	True,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_FORM,
				    NULL);
        XtAddCallback(hl->field_widget,
                  XmNvalueChangedCallback, editorContentsChangedCB,
                  this);

	if (hl->show != SMD_HIDDEN) {
	    XtManageChild(hl->form_widget);
	}
	else {
	    XtVaSetValues(hl->form_widget,
			  XmNtopAttachment, XmATTACH_NONE,
			  NULL);
	}

	XtAddCallback(hl->field_widget,
		      XmNactivateCallback,
		      header_form_traverse,
		      this);

	if (hl->value) {
	    XtRemoveCallback(hl->field_widget,
                  XmNvalueChangedCallback, editorContentsChangedCB,
                  this);

	    XtVaSetValues(hl->field_widget,
			  XmNvalue, hl->value,
			  NULL);
	    XtAddCallback(hl->field_widget,
                  XmNvalueChangedCallback, editorContentsChangedCB,
                  this);
        }

	previous_form = hl->form_widget;

	// Manage the appropriate widget depending on header and alias list
        if (reservedHeader(hl->label) &&
                strcasecmp("subject", hl->label) != 0
                && _aliases->length()) {
                hl->display_widget = (hl->mb_widget)->baseWidget();
                XtUnmanageChild(hl->label_widget);
		XtVaSetValues(hl->field_widget,
	    		XmNleftAttachment, XmATTACH_WIDGET,
	    		XmNleftWidget, (hl->mb_widget)->baseWidget(),
			NULL);
        }
        else {
                hl->display_widget = hl->label_widget;
                if (hl->mb_widget)
                        XtUnmanageChild((hl->mb_widget)->baseWidget());
		XtVaSetValues(hl->field_widget,
	    		XmNleftAttachment, XmATTACH_WIDGET,
	    		XmNleftWidget, hl->label_widget,
			NULL);
        }
    }

    justifyHeaders();
}

void
SendMsgDialog::doDynamicHeaderMenus(void)
{
    // This is really a pain, but we have to blow away the list to
    // build another one. This could probably be done more efficiently,
    // but we wont try to figure out how right now.
    //
    if (_format_cmds) {
	_menuBar->removeOnlyCommands(_format_menu, _format_cmds);
    	delete _format_cmds;
	_format_cmds = NULL;
    }

    _format_cmds = new CmdList("DynamicFormatCommands", "DynamicFormatCommands");

    // Only put on commands that are shown or hidden. The items that
    // are always are never should not be presented to the user as
    // an option to change.
    //
    char label[100];

    for (int h = 0; h < _header_list.length(); h++) {
	HeaderList * hl = _header_list[h];

	switch(hl->show) {
	  case SMD_SHOWN:
      	    sprintf(label, "%s ", catgets(DT_catd, 1, 229, "Delete"));
	    break;

	  case SMD_HIDDEN:
            sprintf(label, "%s ", catgets(DT_catd, 1, 228, "Add"));
	    break;

	  default:
	    continue;
	}

	strcat(label, hl->label);
	strcat(label, ":");

	char * priv_label = strdup(label);

	Cmd * new_cmd = new HideShowCmd(priv_label, priv_label, 1, this, hl->label);
	// Add the commands one at a time with addCommand() vs. all
	// at once with addCommands(). That way new commands will
	// be created instead of reusing old ones.
    	_menuBar->addCommand(_format_menu, new_cmd);
	_format_cmds->add(new_cmd);
    }
}

void
bogus_cb(void *)
{
// Should theInfoDialogManager be destroyed here ???
}

void
SendMsgDialog::include_file_cb( void *client_data, char *selection )
{
    SendMsgDialog *obj = (SendMsgDialog *)client_data;
    obj->include_file(selection);
    
}

void
SendMsgDialog::include_file(
			    char *selection
			    )
{
    FILE *fp;
    int status;
    
    // I don't need to open the file to see if it's readable if loadFile()
    // returns error status.
    if ( (fp = fopen(selection, "r")) == NULL ) {
    	char *buf = new char[MAXPATHLEN];
	sprintf(buf, catgets(DT_catd, 2, 18, "Error: Cannot include file %s"), 
		selection);
	theInfoDialogManager->post(
				   "Mailer", 
				   buf, 
				   (void *)this->_file_include, 
				   bogus_cb);
	delete buf;
    } else {
	fclose(fp);
	status = this->_my_editor->textEditor()->append_to_contents("\n", 2);
	if (status != 0) 
		return;
	status = this->_my_editor->textEditor()->append_at_cursor(selection);
	if (status != 0) 
		return;
	status = this->_my_editor->textEditor()->append_to_contents("\n", 2);
	if (status != 0) 
		return;
	this->_my_editor->textEditor()->set_to_top();
    }
}

void
SendMsgDialog::add_att_cb( void *client_data, char *selection )
{
    SendMsgDialog *obj = (SendMsgDialog *)client_data;
    obj->add_att(selection);
}

void
SendMsgDialog::add_att(char *file)
{
    // Activate Attachment menu???

    this->get_editor()->attachArea()->addAttachment(_msgHandle, _lastAttBP,
                                                    file, NULL);
    
    this->setLastAttBP();
    this->activate_default_attach_menu();

    // This will manage the attach pane too.
    ((ToggleButtonCmd *)_att_show_pane)->setButtonState(TRUE, TRUE);
}

void
SendMsgDialog::add_att(char *name, DtMailBuffer buf)
{
    this->get_editor()->attachArea()->addAttachment(_msgHandle,
						    _lastAttBP,
						    name,
						    buf);
    this->setLastAttBP();
    this->activate_default_attach_menu();
    
    // This will manage the attach pane too.
    ((ToggleButtonCmd *)_att_show_pane)->setButtonState(TRUE, TRUE);
}

void
SendMsgDialog::add_att(DtMailBuffer buf)
{
    char *name = NULL;
    add_att(name, buf);
}

void
SendMsgDialog::save_att_cb( void *client_data, char *selection )
{
    SendMsgDialog *obj = (SendMsgDialog *)client_data;
    
    obj->save_selected_attachment(selection);
    
}

void
SendMsgDialog::save_selected_attachment(
					char *selection
					)
{
    DtMailEnv mail_error;
    
    mail_error.clear();
    
    AttachArea *attarea = this->get_editor()->attachArea();
    Attachment *attachment = attarea->getSelectedAttachment(); 
    
    // Get selected attachment, if none selected, then return.
    if ( attachment == NULL ) {
	// Let User know that no attachment has been selected???
	int answer = 0;
	char *helpId = NULL;
	
	
	_genDialog->setToErrorDialog(
				     catgets(DT_catd, 1, 120, "Mailer"),    
				     catgets(DT_catd, 2, 19, "An attachment needs to be selected before issuing the\n\"Save As\" command to save to a file.") );
	helpId = DTMAILHELPSELECTATTACH;
	answer = _genDialog->post_and_return(
					     catgets(DT_catd, 3, 74, "OK"), helpId );
	return;
    }
    
    // Save selected attachments.
    attachment->saveToFile(mail_error, selection);
    if ( mail_error.isSet() ) {
	// Let User know error condition???
	return;
    }
}
void
SendMsgDialog::resetAliasMenus(void)
{
    for (int i = 0; i < _header_list.length(); i++) {
	HeaderList *hl = _header_list[i];
	XtUnmanageChild(hl->display_widget);
	hl->display_widget = hl->label_widget;
	if (reservedHeader(hl->label) 
		&& strcasecmp("subject", hl->label) != 0
		&& _aliases->length()) {
		hl->display_widget = (hl->mb_widget)->baseWidget();
		// Recreate the aliases pulldown menu
		createAliasMenuButton(&(hl->mb_widget), hl->form_widget, NULL);
		XtVaSetValues(hl->field_widget,
	    		XmNleftAttachment, XmATTACH_WIDGET,
	    		XmNleftWidget, (hl->mb_widget)->baseWidget(),
			NULL);
	}
	else
		XtVaSetValues(hl->field_widget,
	    		XmNleftAttachment, XmATTACH_WIDGET,
	    		XmNleftWidget, hl->label_widget,
			NULL);
	XtManageChild(hl->display_widget);
    }
    justifyHeaders();
    reattachHeaders();
}

void
SendMsgDialog::propsChanged(void)
{
    DtMail::Session *m_session = theRoamApp.session()->session();
    char *value = NULL;
    DtMailEnv error;

    m_session->mailRc(error)->getValue(error, "hideattachments",
                                &value);

    if (value) {
	if (_show_attach_area) {
        	_show_attach_area = FALSE;
        	this->hideAttachArea();
	}
	free(value);
    }
    else if (!_show_attach_area) {
	_show_attach_area = TRUE;
	this->showAttachArea();
    }
    m_session->mailRc(error)->getValue(error, "dontlogmessages", &value);
    if (error.isNotSet()) {
        setLogState(DTM_FALSE);
        ((ToggleButtonCmd *)_file_log)->setButtonState(FALSE, TRUE);
    }
    else {
        setLogState(DTM_TRUE);
        ((ToggleButtonCmd *)_file_log)->setButtonState(TRUE, TRUE);
    }
    DtbOptionsDialogInfo od = (DtbOptionsDialogInfo)theRoamApp.optionsDialog();
    if (!od) return;
    int alias_changed=0;
    XtVaGetValues(od->alias_tf, XmNuserData, &alias_changed, NULL);
    if (alias_changed) {
	extern void alias_stuffing_func(char *, void *, void *);
	while (_aliases->length()) {
        	delete (*_aliases)[0];
        	_aliases->remove(0);
    	}
	m_session->mailRc(error)->getAliasList(alias_stuffing_func, _aliases);
    	if (_aliasCmdList)
		delete _aliasCmdList;
	if (_aliases->length()) {
		_aliasCmdList = new CmdList ("aliases", "aliases");
		addAliases();
	}
	else
		_aliasCmdList = NULL;
	resetAliasMenus();
    }
}
void
SendMsgDialog::createFileMenu()
{
    CmdList *cmdList;
    DtMailEnv error;

    // File
    cmdList = new CmdList( "File", catgets(DT_catd, 1, 121, "File") );
    
    // Default directory is set below at the same time as the default
    // directory for att_add.
    if (!_file_include)
    	_file_include   = new SelectFileCmd (
				 "Include...",
				 catgets(DT_catd, 1, 122, "Include..."),
				 catgets(DT_catd, 1, 123, "Mailer - Include"),
				 catgets(DT_catd, 1, 124, "Include"),
				 TRUE, 
				 SendMsgDialog::include_file_cb, 
				 this,
				 this->baseWidget());
    

    CmdList *subcmdList = new CmdList ( "Save As", catgets(DT_catd, 1, 243 , "Save As") );

    subcmdList->add(_save_as_draft);
    subcmdList->add(_save_as_template);
    subcmdList->add(_save_as_text);
    
    if (!_file_log)
    	_file_log = new LogMsgCmd ( 
			"Save Outgoing Mail",
			catgets(DT_catd, 1, 127, "Save Outgoing Mail"), TRUE, this);
    
    // 1 for default transport.
    
    if (!_file_send)
    	_file_send = new SendCmd (
		 "Send",
                 catgets(DT_catd, 1, 117, "Send"), 
                 TRUE, 
                 this, 
                 1 );
    
    CmdList *subcmdList1 = new CmdList ( "Send As", catgets(DT_catd, 1, 128, "Send As") );
    
    // Find out how many transports there are and build sub menu dynamically.
    DtMail::Session *d_session;

    if ( theRoamApp.session() == NULL ) {
	 MailSession *new_session = new MailSession(
						error, 
						theApplication->appContext());
	 theRoamApp.setSession(new_session);
     }

     d_session = theRoamApp.session()->session();
     const char **impls = d_session->enumerateImpls(error);
    
     if (!_num_sendAs) {
     	for ( int impl = 0;  impls[impl];  impl++ ) {
		DtMailBoolean trans;
		d_session->queryImpl(error, impls[impl],
		     DtMailCapabilityTransport, &trans);
		if (!error.isSet() && trans == DTM_TRUE ) {
			_file_sendAs[_num_sendAs] = new SendCmd( 
			     strdup(impls[impl]),
			     (char *)impls[impl],
			     TRUE, 
			     this, 
			     0 );
			subcmdList1->add( _file_sendAs[_num_sendAs++] );
		}
	}
		// Assume an error means this query failed.  
		// But keep going and get the next transport.
    }

    if (!_file_close)
    	_file_close = new CloseCmd ( 
				 "Close",
				 catgets(DT_catd, 1, 129, "Close"), 
				 TRUE, 
				 _menuBar->baseWidget(), 
				 this );
    
    // Now build the menu
    
    cmdList->add( _file_include );
    cmdList->add( subcmdList );
    cmdList->add( _file_log );
    cmdList->add( _separator );
    
    cmdList->add( _file_send );
    cmdList->add( subcmdList1 );

    cmdList->add( _separator );

    cmdList->add( _file_close );
    
    _menuBar->addCommands ( &_file_cascade, cmdList, FALSE, XmMENU_BAR);

    delete subcmdList;
    delete subcmdList1;
    delete cmdList;
}

void
SendMsgDialog::createMenuPanes()
{
    CmdList *cmdList;
    Cardinal n = 0;
    FORCE_SEGV_DECL(DtMail::Session, m_session);

    // File

    createFileMenu();
    
    // Edit
    
    cmdList = new CmdList( "Edit", catgets(DT_catd, 1, 130, "Edit") );
    
    _edit_undo = new EditUndoCmd ( "Undo", catgets(DT_catd, 1, 131, "Undo"), TRUE, this );
    _edit_cut = new EditCutCmd ( "Cut", catgets(DT_catd, 1, 132, "Cut"), TRUE, this );
    _edit_copy = new EditCopyCmd ( "Copy", catgets(DT_catd, 1, 133, "Copy"), TRUE, this );
    _edit_paste = new EditPasteCmd ( "Paste", catgets(DT_catd, 1, 134 , "Paste"), 
				     TRUE, 
				     this );
    
    // Begin Paste Special submenu
    CmdList *subcmdList1 = new CmdList ( "Paste Special", catgets(DT_catd, 1, 135 , "Paste Special") );
    _edit_paste_special[0] = new EditPasteSpecialCmd (
						      "Bracketed",
						      catgets(DT_catd, 1, 136 , "Bracketed"),
						      TRUE, this, Editor::IF_BRACKETED
						      );
    subcmdList1->add(_edit_paste_special[0]);
    _edit_paste_special[1] = new EditPasteSpecialCmd (
						      "Indented",
						      catgets(DT_catd, 1, 137 , "Indented"), 
						      TRUE, this, Editor::IF_INDENTED );
    subcmdList1->add(_edit_paste_special[1]);
    // End Paste Special submenu
    
    _edit_clear = new EditClearCmd ( "Clear", catgets(DT_catd, 1, 138, "Clear"), 
				     TRUE, 
				     this );
    _edit_delete = new EditDeleteCmd ( "Delete", catgets(DT_catd, 1, 139, "Delete"), 
				       TRUE, 
				       this );
    _edit_select_all = new EditSelectAllCmd ( 
					      "Select All",
					      catgets(DT_catd, 1, 140, "Select All"), 
					      TRUE, 
					      this );
    _format_find_change = new FindChangeCmd ( 
					      "Find/Change...",
					      catgets(DT_catd, 1, 155, "Find/Change..."), 
					      TRUE, 
					      this );
    _format_spell = new SpellCmd (
				  "Check Spelling...",
				  catgets(DT_catd, 1, 156, "Check Spelling..."), 
				  TRUE, 
				  this );

    cmdList->add( _edit_undo );
    cmdList->add( _separator );
    cmdList->add( _edit_cut );
    cmdList->add( _edit_copy );
    cmdList->add( _edit_paste );
    cmdList->add( subcmdList1 ); // Add Paste Special submenu
    cmdList->add( _separator );
    cmdList->add( _edit_clear );
    cmdList->add( _edit_delete );
    cmdList->add( _separator );
    cmdList->add( _edit_select_all );
    cmdList->add( _separator );
    cmdList->add( _format_find_change );
    cmdList->add( _format_spell );

    _menuBar->addCommands ( cmdList );
    delete cmdList;
    delete subcmdList1;


    // Compose Popup CmdList
    construct_text_popup();
    
    // Attachment
    
    cmdList = new CmdList( "Attachments", catgets(DT_catd, 1, 141, "Attachments") );
    
    _att_add   = new SelectFileCmd (
				    "Add File...",
				    catgets(DT_catd, 1, 142, "Add File..."),
				    catgets(DT_catd, 1, 143, "Mailer - Attachments - Add"), 
				    catgets(DT_catd, 1, 144, "Add"),
				    TRUE, 
				    SendMsgDialog::add_att_cb,
				    this,
				    this->baseWidget());

    _att_save  = new SaveAttachCmd (
				    "Save As...",
				    catgets(DT_catd, 1, 145, "Save As..."),
				    catgets(DT_catd, 1, 146, "Mailer - Attachments - Save As"),
				    FALSE, 
				    SendMsgDialog::save_att_cb,
				    this,
				    this->baseWidget());
    _att_delete = new DeleteAttachCmd (
				       "Delete",
				       catgets(DT_catd, 1, 147, "Delete"),
				       FALSE, 
				       this);
    _att_undelete = new UndeleteAttachCmd (
					   "Undelete",
					   catgets(DT_catd, 1, 148, "Undelete"),
					   FALSE, 
					   this);
    _att_rename = new RenameAttachCmd(
				      "Rename",
				      catgets(DT_catd, 1, 149, "Rename"),
				      FALSE,
				      this);
    
    _att_select_all = new SelectAllAttachsCmd( 
					       "Select All",
					       catgets(DT_catd, 1, 150, "Select All"), 
					       this
					       );
    
#ifdef NL_OBSOLETE
	/* NL_COMMENT
	 * This is an obsolete message. It is replaced by message 226 in
	 * set 1.
	 */
    _att_show_pane = new ShowAttachPaneCmd(
					   "Show Pane",
					   catgets(DT_catd, 1, 151, "Show Pane"),
					   this
					   );
#endif
	/* NL_COMMENT
	 * This is the label for a toggle item in a menu.  When the item
	 * is set to "Show List", the Attachment List is mapped in the
	 * Compose Window.  This message replaces message 151 in set 1.
	 */
    _att_show_pane = new ShowAttachPaneCmd(
					   "Show List",
					   catgets(DT_catd, 1, 226, "Show List"),
					   this
					   );
    cmdList->add( _att_add );
    cmdList->add( _att_save );
    cmdList->add( _separator );
    
//   subcmdList1 = new CmdList ( "Create", "Create" );
//   // subcmdList1->add( att_audio );
//   // subcmdList1->add( att_appt );
//   cmdList->add( subcmdList1 );
//   cmdList->add( _separator );
    
    cmdList->add( _att_delete );
    cmdList->add( _att_undelete );
    cmdList->add( _att_rename );
    cmdList->add( _att_select_all );
    cmdList->add(_att_show_pane);
    
    // Create a pulldown from the items in the list.  Retain a handle
    // to that pulldown since we need to dynamically add/delete entries 
    // to this menu based on the selection of attachments.
    
    _attachmentMenu = _menuBar->addCommands ( cmdList );
    construct_attachment_popup();
 
//  delete subcmdList1;
    delete cmdList;
    
    createFormatMenu();
    
    _overview = new OnAppCmd("Overview", catgets(DT_catd, 1, 71, "Overview"),
                                TRUE, this);
    _tasks = new TasksCmd("Tasks", catgets(DT_catd, 1, 72, "Tasks"), 
				TRUE, this);
    _reference = new ReferenceCmd("Reference", catgets(DT_catd, 1, 73, "Reference"), 
				TRUE, this);
    _messages = new MessagesCmd("Messages", catgets(DT_catd, 1, 301, "Messages"), 
				TRUE, this);
    _on_item = new OnItemCmd("On Item", catgets(DT_catd, 1, 74, "On Item"),
                                TRUE, this);
    _using_help = new UsingHelpCmd("Using Help", catgets(DT_catd, 1, 75, "Using Help"), 
				TRUE, this);
    _about_mailer = new RelNoteCmd("About Mailer...", catgets(DT_catd, 1, 77, "About Mailer..."),
                                    TRUE, this);
    cmdList = new CmdList("Help", catgets(DT_catd, 1, 76, "Help"));
    cmdList->add(_overview);
    cmdList->add(_separator);
    cmdList->add(_tasks);
    cmdList->add(_reference);
    cmdList->add(_messages);
    cmdList->add(_separator);
    cmdList->add(_on_item);
    cmdList->add(_separator);
    cmdList->add(_using_help);
    cmdList->add(_separator);
    cmdList->add(_about_mailer);
    _menuBar->addCommands(cmdList, TRUE);
    delete cmdList;
}
void
SendMsgDialog::addAliases()
{
    int list_len = _aliases->length();
    for (int i = 0; i < list_len; i++) {
	PropStringPair *psp = (*_aliases)[i];
	if (psp->label && psp->value) {
		Cmd * subCmd = new AliasCmd(strdup(psp->label), 
			   psp->label, 
			   1, 
			   this, 
			   psp->label);
		_aliasCmdList->insert(subCmd);
	}
    }
}
void
SendMsgDialog::construct_attachment_popup(void)
{
    _attachmentPopupMenuList = new CmdList( "AttachmentsPopup", "AttachmentsPopup");

    _attachment_popup_title = new LabelCmd (
			"Mailer - Attachments",
			catgets(DT_catd, 1, 158, "Mailer - Attachments"), TRUE);

    _attachmentPopupMenuList->add( _attachment_popup_title);
    _attachmentPopupMenuList->add( _separator);
    _attachmentPopupMenuList->add( _att_add );
    _attachmentPopupMenuList->add( _att_save );
    _attachmentPopupMenuList->add( _att_delete );
    _attachmentPopupMenuList->add( _att_undelete );
    _attachmentPopupMenuList->add( _att_select_all );

    _menuPopupAtt = new MenuBar(_my_editor->attachArea()->getClipWindow(), 
					"RoamAttachmentPopup", XmMENU_POPUP);
    _attachmentPopupMenu = _menuPopupAtt->addCommands(_attachmentPopupMenuList, 
				FALSE, XmMENU_POPUP);
}

void
SendMsgDialog::construct_text_popup(void)
{
   if (theApplication->bMenuButton() != Button3)
        return;

    _textPopupMenuList = new CmdList( "TextPopup", "TextPopup");

    _compose_popup_title     = new LabelCmd (
			"Mailer - Compose",
			catgets(DT_catd, 1, 159, "Mailer - Compose"), TRUE);

    _textPopupMenuList->add( _compose_popup_title);
    _textPopupMenuList->add( _separator);
    _textPopupMenuList->add( _file_send);
    _textPopupMenuList->add( _edit_undo );
    _textPopupMenuList->add( _edit_cut );
    _textPopupMenuList->add( _edit_copy );
    _textPopupMenuList->add( _edit_paste );
    _textPopupMenuList->add( _edit_delete );
    _textPopupMenuList->add( _edit_select_all );

    Widget parent = _my_editor->textEditor()->get_editor();
    _menuPopupText = new MenuBar(parent, "SendMsgTextPopup", XmMENU_POPUP);
    _textPopupMenu = _menuPopupText->addCommands(_textPopupMenuList, 
				FALSE, XmMENU_POPUP);
}

void
SendMsgDialog::createFormatMenu()
{
    CmdList *cmdList;

    cmdList = new CmdList( "Format", catgets(DT_catd, 1, 152,"Format") );
    
    if (!_format_word_wrap)
    	_format_word_wrap = new WordWrapCmd ( 
					  "Word Wrap",
					  catgets(DT_catd, 1, 153, "Word Wrap"), 
					  TRUE, 
					  this );
   if (!_format_settings)
    	_format_settings = new FormatCmd ( "Settings...",
				       catgets(DT_catd, 1, 154, "Settings..."), 
				       TRUE, 
				       this );
    
    cmdList->add( _format_word_wrap );
    cmdList->add( _format_settings );
    cmdList->add( _separator);
    
    if (_templates)
	delete _templates;
    _templates = new CmdList ( "Use Template", catgets(DT_catd, 1, 157, "Use Template") );

    addTemplates(_templates);
    cmdList->add(_templates);

    if (_drafts)
	delete _drafts;
    _drafts = new CmdList ( "Use Draft", catgets(DT_catd, 3, 108, "Use Draft") );
    cmdList->add(_drafts);

    if (!_signature)
    	_signature = new ComposeSigCmd ( "Include Signature", 
			catgets(DT_catd, 1, 299, "Include Signature"),
			TRUE,
			NULL,
			this );
    cmdList->add(_signature);

    cmdList->add( _separator );


    if (!_aliasCmd)
 	_aliasCmd = new OptionsCmd(
                        "Aliases",
                        catgets(DT_catd, 1, 269, "Aliases..."),
                        1,
                        OPT_AL);
    if (!_sigCmd)
    	_sigCmd = new OptionsCmd(
                        "Signature",
                        catgets(DT_catd, 1, 266, "Signature..."),
                        1,
                        OPT_SIG);

    if (!_vacationCmd)
        _vacationCmd = new OptionsCmd(
                        "Vacation Message",
                        catgets(DT_catd, 1, 267, "Vacation Message..."),
                        1,
                        OPT_VAC);

    cmdList->add( _sigCmd );
    cmdList->add( _vacationCmd );
    cmdList->add( _aliasCmd );
    cmdList->add( _separator );

    _format_menu = _menuBar->addCommands ( &_format_cascade, cmdList, 
			FALSE, XmMENU_BAR);

    Widget menu = _templates->getPaneWidget();
    Widget cascade = _templates->getCascadeWidget();
    XtVaSetValues(menu,
                  XmNpacking, XmPACK_COLUMN,
                  XmNorientation, XmVERTICAL,
                  NULL);
    XtVaSetValues(cascade, XmNuserData, menu, NULL);
    XtAddCallback(cascade, XmNcascadingCallback, &RoamMenuWindow::map_menu, NULL);

    menu = _drafts->getPaneWidget();
    XtVaSetValues(menu,
                  XmNpacking, XmPACK_COLUMN,
                  XmNorientation, XmVERTICAL,
		  NULL);
    RAMenuClientData *mcd = (RAMenuClientData*) malloc (sizeof(RAMenuClientData));
    mcd->timestamp = 0;
    mcd->filename = theRoamApp.getResource(DraftDir, TRUE);
    mcd->smd = this;
    _drafts->setClientData((void*)mcd);
    XtAddCallback(menu, XmNmapCallback, &RoamApp::map_menu, _drafts);

    delete cmdList;

    doDynamicHeaderMenus();
}
void
SendMsgDialog::addTemplates(CmdList * subCmd)
{
    DtMailEnv error;
    int count;
    char *value=NULL;
    Cmd *button;

    DtMail::Session *m_session = theRoamApp.session()->session();
    m_session->mailRc(error)->getValue(error, "templates", &value);
    if (!value || !(*value)) {
	button = new LoadFileCmd("No Templates",
                                 catgets(DT_catd, 3, 114, "No Templates"),
                                 1, this,
                                 NULL, TYPE_TEMPLATE);

        subCmd->add(button);
	if (value)
		free(value);
	return;
    }

    DtVirtArray<PropStringPair *> templates(8);
    parsePropString(value, templates);
    if (value)
    	free(value);

    int template_count = templates.length();
    for (count = 0; count < template_count; count++) {
	PropStringPair * psp = templates[count];
	if (psp->label && psp->value) {
	    button = new LoadFileCmd(strdup(psp->label), 
					   psp->label, 
					   1, 
					   this, 
					   psp->value, TYPE_TEMPLATE);
	    subCmd->insert(button);
	}
    }

    for (count=0; count < template_count; count++) {
	PropStringPair * psp = templates[0];
	templates.remove(0);
        delete psp;
    }
}

void
SendMsgDialog::initialize()
{
    Cardinal n = 0;
    Arg args[1];
    const char * hideAttachPane = NULL;
    DtMailEnv error;
    
    // Without the TearOffModelConverter call, there will be warning messages:
    // Warning: No type converter registered for 'String' to 'TearOffModel' 
    // conversion.
    
    XmRepTypeInstallTearOffModelConverter();
    MenuWindow::initialize();
    XtSetArg(args[n], XmNdeleteResponse, XmDO_NOTHING); n++;
    XtSetValues( _w, args, n);
    
    _genDialog = new DtMailGenDialog("Dialog", _main);
    
    // See if the .mailrc specifies if attachPane is to be shown or hid 
    // at SMD startup time.
    
    DtMail::Session *m_session = theRoamApp.session()->session();
    m_session->mailRc(error)->getValue(error, "hideattachments", 
				&hideAttachPane);
    
    if (!hideAttachPane) {
	_show_attach_area = TRUE;
    }
    else {
	_show_attach_area = FALSE;
	// The user wants to hide attachments

	this->hideAttachArea();
    }
    
    // Log Message Toggle button.  A LogMsgCmd is a ToggleButtonCmd....
    char * value = NULL;
    m_session->mailRc(error)->getValue(error, "dontlogmessages", &value);
    if (value) free (value);
    if (error.isNotSet()) {
	// "dontlogmessages" is TRUE
	setLogState(DTM_FALSE);
	((ToggleButtonCmd *)_file_log)->setButtonState(FALSE, TRUE);
    } else {
	// "dontlogmessages" is FALSE
	setLogState(DTM_TRUE);
	((ToggleButtonCmd *)_file_log)->setButtonState(TRUE, TRUE);
    }
    
    // Word Wrap Toggle button.  A WordWrapCmd is a ToggleButtonCmd...
    ((ToggleButtonCmd *)_format_word_wrap)->setButtonState(
			   ((WordWrapCmd *)_format_word_wrap)->wordWrap(), 
			   FALSE
			   );

    // Initialize the Edit menu

    this->text_unselected();
	
    setIconName(ComposeIcon);
    this->setDraftFile(NULL);
    this->setTemplateFile(NULL);
    this->setTemplateFullPath(NULL);
}

static void
Self_destruct(XtPointer, XtIntervalId *)
{
#ifdef WM_TT
    fprintf(stderr, "DEBUG: Self_destruct(): invoked!\n");
#endif
    
    
    XtRemoveAllCallbacks(
			 theApplication->baseWidget(), 
			 XmNdestroyCallback);
    delete theApplication;
}

// Clears Compose window title, header fields, text, and attachment areas.
void
SendMsgDialog::reset()
{
    _my_editor->textEditor()->clear_contents();
    _my_editor->attachArea()->resetPendingAction();

    _takeDown = FALSE;
    // This will deselect any Attachment action, if any available now.
    // Also deselect text menu items....

    this->deactivate_default_attach_menu();
    this->text_unselected();
    this->all_attachments_deselected();
    _att_undelete->deactivate();  // This needs to be done in addition

    this->get_editor()->attachArea()->removeCurrentAttachments();

    // Unmanage the dialog
    this->unmanage();

    if (_show_attach_area) { // .mailrc wants default attach area invisible

    // Unmanage the attach Area.  Set the show_pane button.
    // This is done because if we are caching this window (after 
    // unmanaging), we don't want the window to pop back up, on uncaching,
    // with the attachment pane visible, etc..

	this->showAttachArea();
    }
    else {
	this->hideAttachArea();
    }

    // Need to destroy current Message handle.
    delete _msgHandle;    // All its body parts are deleted.
    _msgHandle = NULL;
    _lastAttBP = NULL;    // So just set this to NULL.
    // Delete or set to NULL ???
    _inclMsgHandle = NULL; 
    _inclMsgHasText = NULL;

    for (int clear = 0; clear < _header_list.length(); clear++) {
	HeaderList * hl = _header_list[clear];

        XtRemoveCallback(hl->field_widget,
                  XmNvalueChangedCallback, editorContentsChangedCB,
                  this);
	if (hl->value) 
	    XtVaSetValues(hl->field_widget,
			  XmNvalue, hl->value,
			  NULL);
	else 
	    XtVaSetValues(hl->field_widget,
			  XmNvalue, "",
			  NULL);
        XtAddCallback(hl->field_widget,
                  XmNvalueChangedCallback, editorContentsChangedCB,
                  this);
    }
    // Reset the Log state in case the user happened to change it.
    DtMail::Session *m_session = theRoamApp.session()->session();
    char * value = NULL;
    DtMailEnv error;

    m_session->mailRc(error)->getValue(error, "dontlogmessages", &value);
    if (value) free(value);
    if (error.isNotSet()) {
        // "dontlogmessages" is TRUE
        setLogState(DTM_FALSE);
        ((ToggleButtonCmd *)_file_log)->setButtonState(FALSE, TRUE);
    } else {
        // "dontlogmessages" is FALSE
        setLogState(DTM_TRUE);
        ((ToggleButtonCmd *)_file_log)->setButtonState(TRUE, TRUE);
    }

}

// Recycles Compose window.
void
SendMsgDialog::quit()
{

    // There are several ways we could have reached here.
    // 1) From the user choosing Send.
    // 2) From the user clicking on the Close button or Close menu item
    // 3) The user choosing Close from window manager menu.
    // For (1), we just forge ahead.  For that, the _takeDown boolean
    // is set in send_message() method.
    // For (2), the boolean is set in goAway().
    // For (3), we call goAway() which sets the _takeDown depending on
    // a dialog negotiation if SMD has contents.

     if (_file_include->fileBrowser() != NULL) 
         XtPopdown(XtParent(_file_include->fileBrowser())); 
     if (_att_add->fileBrowser() != NULL) 
         XtPopdown(XtParent(_att_add->fileBrowser())); 

     if (_save_as_text->fileBrowser() != NULL) 
         XtPopdown(XtParent(_save_as_text->fileBrowser())); 
     if (_att_save->fileBrowser() != NULL) 
         XtPopdown(XtParent(_att_save->fileBrowser())); 

    if (!_takeDown) {
        // Check to see if it's the first time through the quit()
        // method.  Set _first_time to FALSE so that we don't come
        // down this path again until we're done quitting or bad
        // things will happen.
        if (_first_time == TRUE) {
            _first_time = FALSE;
            this->goAway(TRUE);
            // We're done quitting, so we can set _first_time to TRUE again.
            _first_time = TRUE;
        }
        return;
    }  

    stopAutoSave();
    
    theCompose._not_in_use++;
    this->reset();
    theCompose.fitIntoList(this, FALSE);
    
#ifdef DTMAIL_TOOLTALK
    if ( started_by_tt && (theCompose._timeout_id == 0) &&
	 (theCompose._not_in_use == theCompose._num_created) &&
	 !dtmail_mapped ) {    //  For explanation of dtmail_mapped, look at RoamApp.h.
	theCompose._timeout_id = XtAppAddTimeOut( 
						  theApplication->appContext(), 
						  (unsigned long)DESTRUCT_TIMEOUT, 
						  Self_destruct, 
						  NULL);
#ifdef WM_TT
	fprintf(stderr, 
		"DEBUG: SendMsgDialog::quit(): Not in use = %d, Created = %d\n",
		theCompose._not_in_use, 
		theCompose._num_created);
#endif
    }
#endif
    // If user has quit the last Container window and he just quit 
    // (send or close) the last Compose window, then terminate the 
    // application.
    // 
    if ( (theApplication->num_windows() == theCompose._not_in_use) && 
	 (theCompose._timeout_id == 0) ) {
	theCompose.~Compose();
    }
    char *buf_ptr = this->getTemplateFile();
    if (buf_ptr) {
	 free(buf_ptr);
    	 this->setTemplateFile(NULL);
    }

    buf_ptr = this->getTemplateFullPath();
    if (buf_ptr) {
	 free(buf_ptr);
     	 this->setTemplateFullPath(NULL);
    }

    buf_ptr = this->getDraftFile();
    if (buf_ptr) {
	 free(buf_ptr);
    	 this->setDraftFile(NULL);
    }
    clearStatus();
}

// Given a file name, include the file as attachment.
void
SendMsgDialog::inclAsAttmt(char *file, char *name)
{
    this->get_editor()->attachArea()->addAttachment(_msgHandle, _lastAttBP,
                                                    file, name);
    this->setLastAttBP();
    
}

// Given a buffer, include its content as an attachment.
void
SendMsgDialog::inclAsAttmt(unsigned char *contents, int len, char *name)
{
    DtMailBuffer mbuf;

    mbuf.buffer = (void *)contents;
    mbuf.size = (unsigned long)len;
    this->get_editor()->attachArea()->addAttachment(_msgHandle, _lastAttBP,
                                                    (String)name, mbuf);
    this->setLastAttBP();
}

// Given a RFC_822_Message formatted buffer, parse it and fill the Compose Window.
void
SendMsgDialog::parseNplace(char *contents, int len)
{
    // 1. Create message handle for contents

    DtMailEnv error;
    DtMail::Session * d_session = theRoamApp.session()->session();
    DtMailBuffer mbuf;
    
    mbuf.buffer = (void *)contents;
    mbuf.size = (unsigned long)len;
    
    DtMail::Message * msg = d_session->messageConstruct(error,
							DtMailBufferObject,
							&mbuf,
							NULL,
							NULL,
							NULL);
    if ( !msg ) {
	return;
    } else if ( error.isSet() ) {
	if ( error == DTME_UnknownFormat ) {
	    // The content does not have header info.  Therefore, store
	    // everything as text.
	    _my_editor->textEditor()->set_contents((const char *) mbuf.buffer,
											   mbuf.size);
	    return;
	}
    }

    char * status_string;
    DtMailBoolean first_bp_handled;
    first_bp_handled = _my_editor->textEditor()->set_message(
							     msg,
							     &status_string,
							     Editor::HF_NONE
							     );
    
    int num_bodyParts = msg->getBodyCount(error);

    // Don't use setInclMsgHnd() because it causes the SMD's attachments
    // to get out of sink with the BE.  Just assign the newly created message
    // to _msgHandle.
    //
    if ((num_bodyParts > 1) || (!first_bp_handled)) {
	int start;
	if (first_bp_handled) {
	    start = 2;
//	    setInclMsgHnd(msg, TRUE);
	}
	else {
	    start = 1;
//	    setInclMsgHnd(msg, FALSE);
	}

	if (_msgHandle) {
	    delete _msgHandle;
	}
	_msgHandle = msg;
	_my_editor->attachArea()->parseAttachments(error,
						   msg,
						   TRUE,
//						   FALSE,
						   start);
	// Need to call this after calling parseAttachments() so attachments
	// will be displayed in the attachment pane.
	_my_editor->manageAttachArea();

	// Need to update this compose window's internal message handle.
	//
	// GL - calling updateMsgHndAtt is no longer necessary because we
	// just assigning the newly created msg to _msgHandle.
//	updateMsgHndAtt();
    }

    loadHeaders(msg, DTM_TRUE, DTM_FALSE);
}

// Given a RFC_822_Message formatted file, parse it and fill the Compose Window.
void
SendMsgDialog::parseNplace(const char * path)
{
    // 1. Get file content into buffer.
    int fd = SafeOpen(path, O_RDONLY);
    if (fd < 0) {
	return;
    }
    
    struct stat buf;
    if (SafeFStat(fd, &buf) < 0) {
	close(fd);
	return;
    }
    
    _dead_letter_buf = new char[buf.st_size];
    if (!_dead_letter_buf) {
	close(fd);
	return;
    }
    
    if (SafeRead(fd, _dead_letter_buf, 
		 (unsigned int) buf.st_size) != buf.st_size) {
	delete _dead_letter_buf;
	close(fd);
	return;
    }
    
    parseNplace(_dead_letter_buf, (int) buf.st_size);
}

void
SendMsgDialog::text( const char *text )
{
    _my_editor->textEditor()->set_contents( text, strlen(text) );
}

void
SendMsgDialog::append( const char *text )
{
    _my_editor->textEditor()->append_to_contents( text, strlen(text) );
}

char *
SendMsgDialog::text()
{
    // Potential memory leak here.  Because XmTextGetString returns 
    // pointer to space containing all the text in the widget.  Need 
    // to call XtFree after we use this space
    // Also DtEditor widget requires application to free data.
    
    return (_my_editor->textEditor()->get_contents());
    
}


void
SendMsgDialog::text_selected()
{
    // turn on sensitivity for Cut/Clear/Copy/Delete
    _edit_cut->activate();
    _edit_copy->activate();
    _edit_clear->activate();
    _edit_delete->activate();
    _edit_select_all->activate();
}

void
SendMsgDialog::text_unselected()
{
    // turn off sensitivity for those items
    _edit_cut->deactivate();
    _edit_copy->deactivate();
    _edit_clear->deactivate();
    _edit_delete->deactivate();
}

Compose::Compose()
{
    _compose_head = NULL;
    _not_in_use = 0;
    _num_created = 0;
    _timeout_id = 0;
}

Compose::~Compose()
{
    Compose::Compose_Win *a_node;
    Compose::Compose_Win *next_node;

    a_node = _compose_head;
    while (a_node) {
	next_node = a_node->next;
	if (!a_node->in_use) {
	    delete a_node->win;
	    delete a_node;
	}
	a_node = next_node;
    }
}

// Get a compose window either by creating a new SendMsgDialog or
// from the recycle list.
SendMsgDialog *
Compose::getWin(Tt_message tt_msg)
{
    SendMsgDialog *newsend = NULL;
    
#ifdef DTMAIL_TOOLTALK
    if ( _timeout_id ) {
	XtRemoveTimeOut( _timeout_id );
	_timeout_id = 0;
    }
#endif
    
    newsend = theCompose.getAnUnusedSMD();

    if (!newsend) {
	// We have no unused SMDs around; therefore, create new window.
	theRoamApp.busyAllWindows();
	newsend = new SendMsgDialog(tt_msg);
	newsend->initialize();
	theCompose.fitIntoList(newsend, TRUE);
	theRoamApp.unbusyAllWindows();
	_num_created++;
    } else {
	newsend->resetHeaders();
	_not_in_use--;
    }

    newsend->text_unselected();
    newsend->manage();
    newsend->startAutoSave();
    
    // Get new Message Handle
    newsend->setMsgHnd();
    newsend->title(catgets(DT_catd, 1, 160, "New Message"));
    return newsend;
}

// Create a new SendMsgDialog, used in session restore
SendMsgDialog *
Compose::SessiongetWin()
{
    SendMsgDialog *newsend = NULL;
    
#ifdef DTMAIL_TOOLTALK
    if ( _timeout_id ) {
	XtRemoveTimeOut( _timeout_id );
	_timeout_id = 0;
    }
#endif

    newsend = new SendMsgDialog();
    newsend->initialize();
    theCompose.fitIntoList(newsend, TRUE);
    _num_created++;

    newsend->text_unselected();
    newsend->startAutoSave();
    
    // Get new Message Handle
    newsend->setMsgHnd();
    newsend->title(catgets(DT_catd, 1, 160, "New Message"));
    return newsend;
}


// Attachments

void
SendMsgDialog::attachment_selected()
{
    _att_save->activate();
    _att_delete->activate();
    _att_rename->activate();
    
}

void
SendMsgDialog::all_attachments_selected()
{
    _att_delete->activate();
    _att_save->deactivate();
    _att_rename->deactivate();

    if (_attachmentActionsList != NULL) {
	_menuBar->removeCommands(_attachmentMenu, _attachmentActionsList);
	_menuPopupAtt->removeCommands(_attachmentPopupMenu, 
				_attachmentActionsList);
	delete _attachmentActionsList;
	_attachmentActionsList = NULL;
    }

}


void
SendMsgDialog::all_attachments_deselected()
{
    _att_save->deactivate();
    _att_delete->deactivate();
    _att_rename->deactivate();
    
    if (_attachmentActionsList != NULL) {
	_menuBar->removeCommands(_attachmentMenu, _attachmentActionsList);
	_menuPopupAtt->removeCommands(_attachmentPopupMenu, 
				_attachmentActionsList);
	delete _attachmentActionsList;
	_attachmentActionsList = NULL;
    }
    
}


void
SendMsgDialog::addAttachmentActions(
				    char **actions,
				    int indx
				    )
{
    int i;
    char *anAction;
    AttachmentActionCmd *attachActionCmd;
    
    if (_attachmentActionsList == NULL) { 
	_attachmentActionsList = new CmdList("AttachmentActions", "AttachmentActions");
    }
    else {
	_menuBar->removeCommands(_attachmentMenu, _attachmentActionsList);
	_menuPopupAtt->removeCommands(_attachmentPopupMenu, 
					_attachmentActionsList);
	delete _attachmentActionsList;
	_attachmentActionsList = new CmdList("AttachmentActions", "AttachmentActions");
    }
    
    char *actionLabel;         
    for (i = 0; i < indx; i++) {
	anAction = actions[i];
	actionLabel = DtActionLabel(anAction);   // get the localized action label
	attachActionCmd = new AttachmentActionCmd(
						  anAction,
					          actionLabel,
						  this,
						  i);
	_attachmentActionsList->add(attachActionCmd);
	
    }
    _attachmentMenu = _menuBar->addCommands(
					    _attachmentMenu, 
					    _attachmentActionsList
					    );
    _attachmentPopupMenu = _menuPopupAtt->addCommands(
					    _attachmentPopupMenu, 
					    _attachmentActionsList
					    );
}

void
SendMsgDialog::removeAttachmentActions()
{
    
    // Stubbed out for now
}

void
SendMsgDialog::invokeAttachmentAction(
				      int index
				      )
{
    DtMailEditor *editor = this->get_editor();
    AttachArea *attacharea = editor->attachArea();
    Attachment *attachment = attacharea->getSelectedAttachment();
    
    attachment->invokeAction(index);
}

void
SendMsgDialog::selectAllAttachments()
{
    
    DtMailEditor *editor = this->get_editor();
    AttachArea *attachArea = editor->attachArea();
    
    attachArea->selectAllAttachments();
    
}


void
SendMsgDialog::activate_default_attach_menu()
{
    _att_select_all->activate();
}

void
SendMsgDialog::deactivate_default_attach_menu()
{
    _att_select_all->deactivate();
}


void
SendMsgDialog::delete_selected_attachments()
{
    DtMailEnv mail_error;
    
    // Initialize the mail_error.

    mail_error.clear();
    
    AttachArea *attachArea = _my_editor->attachArea();
    attachArea->deleteSelectedAttachments(mail_error);
    
    if (mail_error.isSet()) {
	// do something
    }
    
    // Activate this button to permit the user to undelete.
    
    _att_undelete->activate();
    
    // Deactivate buttons that will be activated when another
    // selection applies.
    
    _att_save->deactivate();
    _att_delete->deactivate();
    _att_rename->deactivate();

    if (_attachmentActionsList) {
	_menuBar->removeCommands(_attachmentMenu, _attachmentActionsList);
	_menuPopupAtt->removeCommands(_attachmentPopupMenu, 
				      _attachmentActionsList);
	delete _attachmentActionsList;
	_attachmentActionsList = NULL;
    }
}

void
SendMsgDialog::undelete_last_deleted_attachment()
{
    DtMailEnv mail_error;
    
    // Initialize the mail_error.
    
    mail_error.clear();
    
    AttachArea *attachArea = _my_editor->attachArea();
    attachArea->undeleteLastDeletedAttachment(mail_error);
    
    if (mail_error.isSet()) {
	// do something
    }
    
    if(_my_editor->attachArea()->getIconSelectedCount()) 
    	_att_delete->activate();

    if (attachArea->getDeleteCount() == 0) {
	_att_undelete->deactivate();
    }
}

Boolean
SendMsgDialog::renameAttachmentOK()
{
    AttachArea *attachArea = _my_editor->attachArea();
    char buf[512];
    
    if (attachArea->getIconSelectedCount() > 1) {
	
     sprintf(buf, catgets(DT_catd, 5, 4, "Select only one attachment\n\
     and then choose rename"));
	
	_genDialog->setToQuestionDialog(
					catgets(DT_catd, 5, 2, "Mailer"),
					buf);
	
	char * helpId = DTMAILHELPSELECTONEATTACH;
	
	int answer = _genDialog->post_and_return(helpId);
	
	return(FALSE);
    }
    else {
	return(TRUE);
    }
}

void
SendMsgDialog::showAttachArea()
{
    DtMailEditor *editor = this->get_editor();
    editor->showAttachArea();
    ((ToggleButtonCmd *)_att_show_pane)->setButtonState(TRUE, FALSE);
}

void
SendMsgDialog::hideAttachArea()
{
    DtMailEditor *editor = this->get_editor();
    editor->hideAttachArea();
    ((ToggleButtonCmd *)_att_show_pane)->setButtonState(FALSE, FALSE);
}

int
SendMsgDialog::lookupHeader(const char * name)
{
    for (int h = 0; h < _header_list.length(); h++) {
	HeaderList * hl = _header_list[h];
	if (hl->show != SMD_NEVER && 
		strcmp(hl->label, name) == 0) {
	    return(h);
	}
    }

    return(-1);
}

void
SendMsgDialog::reattachHeaders(void)
{
    // We have to walk through the entire list of headers, attaching
    // the shown headers to the ones above them.
    //
    HeaderList * hl = _header_list[0];
    Widget previous_form = hl->form_widget;

    for (int h = 1; h < _header_list.length(); h++) {
	hl = _header_list[h];

	switch(hl->show) {
	  case SMD_ALWAYS:
	    previous_form = hl->form_widget;
	    break;

	  case SMD_SHOWN:
	    XtVaSetValues(hl->form_widget,
			  XmNtopAttachment, XmATTACH_WIDGET,
			  XmNtopWidget, previous_form,
			  NULL);
	    previous_form = hl->form_widget;
	    break;

	  default:
	    break;
	}
    }

    forceFormResize(_main_form);
    forceFormResize(_header_form);
}

void
SendMsgDialog::justifyHeaders(void)
{
    // Find out which header label has the longest display width to right
    // justify all labels.
    //
    Dimension longest = 0;
    Dimension w = 0;
    Dimension margin;

    for (int count = 0;  count < _header_list.length(); count++) {
	HeaderList * hl = _header_list[count];
	if (hl->show == SMD_HIDDEN || hl->show == SMD_NEVER) {
	    continue;
	}
	XtVaGetValues(hl->display_widget,
		XmNwidth, &w,
		XmNmarginLeft, &margin, 
		NULL);
	w -= margin;
	if ( w > longest ) {
	    longest = w;
	}
    }

    for (int adjust = 0;  adjust < _header_list.length();  adjust++) {
	HeaderList * hl = _header_list[adjust];
	if (hl->show == SMD_HIDDEN || hl->show == SMD_NEVER) {
	    continue;
	}
	XtVaGetValues(hl->display_widget,
	      XmNwidth, &w,
	      XmNmarginLeft, &margin,
	      NULL);
	w -= margin;
	XtVaSetValues(hl->display_widget, XmNmarginLeft, (longest-w) > 0 ? longest-w : 1, NULL );
    }
}

void
SendMsgDialog::forceFormResize(Widget form)
{
    // The Motif Form widget is at least a little bit brain damaged.
    // We need to convince it to do the right thing after we make
    // minor adjustments in the children.
    //
    Dimension width, height, border;
    XtVaGetValues(form,
		  XmNwidth, &width,
		  XmNheight, &height,
		  XmNborderWidth, &border,
		  NULL);

    XtVaSetValues(form,
		  XmNwidth, width + 1,
		  XmNheight, height + 1,
		  NULL);

    XtVaSetValues(form,
		  XmNwidth, width,
		  XmNheight, height,
		  NULL);
}
// Method checks if self has text in it.  

Boolean
SendMsgDialog::checkDirty()
{
    if (!_edits_made)
	// return FALSE so quit() can go ahead.
 	return(FALSE);

    return(TRUE);
}

Boolean
SendMsgDialog::handleQuitDialog()
{
    DtMail::Session *m_session = theRoamApp.session()->session();
    char * value = NULL;
    DtMailEnv error;

    m_session->mailRc(error)->getValue(error, "expert", &value);
    if (error.isNotSet() && value != NULL) {
	free(value);
	return (TRUE); 
    }
    if (value) free(value);

    DtMailGenDialog *dialog = this->genDialog();

    dialog->setToQuestionDialog(
	catgets( DT_catd, 1, 99, "Mailer"),
	catgets( DT_catd, 3, 58, "If you close this window\nyou will lose your edits."));
    if ( dialog->post_and_return(
			catgets( DT_catd, 1, 118, "Close"),
			catgets( DT_catd, 1, 101, "Cancel"),
			DTMAILHELPCLOSECOMPOSEWINDOW) == 1 ) 
		// Close selected
		return(TRUE);

    return(FALSE);	// Cancel selected
}

void
SendMsgDialog::goAway(
    Boolean checkForDirty
)
{
    
    if (!checkForDirty) {
	_takeDown = TRUE;
	this->quit();
    }
    else {
	// Check to see if self has contents (ie., is dirty)

	Boolean is_dirty = this->checkDirty();

	if (is_dirty) {
	    if (isIconified()) {
		MainWindow::manage();
	    }

	    // Enquire if user really wants this window to go away

	    Boolean really_quit = this->handleQuitDialog();
	    if (!really_quit) {
		return;
	    }
	}
	_takeDown = TRUE;
	this->quit();
    }
    _edits_made = FALSE;
}

void
SendMsgDialog::manage()
{
    MenuWindow::manage();
    // Set focus 
    HeaderList * hl = _header_list[0];
    (void) XmProcessTraversal(hl->field_widget, XmTRAVERSE_CURRENT);
   
}

void
SendMsgDialog::unmanage()
{
    MenuWindow::unmanage();
    XFlush(XtDisplay(this->_main_form));
    XSync(XtDisplay(this->_main_form), False);
}
    

void
Compose::fitIntoList(
    SendMsgDialog *smd,
    Boolean is_new	// TRUE when smd is new; FALSE otherwise.
)
{

    Compose::Compose_Win* a_node = NULL; 

    if (!is_new) {// Window already in cache.  
	//Locate node and set in_use to FALSE
    
	for (a_node = theCompose._compose_head;
	    a_node;
	    a_node = a_node->next) {
	
	    if (a_node->win == smd) {
		a_node->in_use = FALSE;
	    }
	}
	return;
    }

    // Need new node with smd.  Set in_use to TRUE
    Compose::Compose_Win *tmp;

    tmp = (Compose::Compose_Win *)malloc(sizeof(Compose::Compose_Win));
    tmp->win = smd;
    tmp->next = NULL;
    tmp->in_use = TRUE;

    // If nothing is cached so far, begin a cache.

    if ( theCompose._compose_head == NULL ) {
	// Put this Compose window in recycle list.
	theCompose._compose_head = tmp;
	return;
    }
    
    // There exists a cache.
    // We need to see if the window is already in the list.
    // If it is, reset its in_use field.
    // If it is not in the list, place tmp containing this SMD in the list.

    Boolean set = FALSE;

    for (a_node = theCompose._compose_head;
	a_node && !set;
	a_node = a_node->next) {
	
	if (!a_node->next) {
	    a_node->next = tmp;
	    set = TRUE;
	}
    }

}

SendMsgDialog*
Compose::getAnUnusedSMD()
{
    // If cache empty
   if ( theCompose._compose_head == NULL ) {
       return(NULL);
   }
   
    Compose::Compose_Win* a_node = NULL; 
    Compose::Compose_Win* the_node = NULL; 
    
   // Find a node with unused smd.  Return smd
    for (a_node = theCompose._compose_head;
	a_node;
	a_node = a_node->next) {

	if (!a_node->in_use) {
	    a_node->in_use = TRUE;
	    return(a_node->win);
	}
    }

   // All cached smds are in use.  
   return(NULL);
   
}

void
SendMsgDialog::resetHeaders(void)
{
    DtMail::Session *m_session = theRoamApp.session()->session();
    char * value = NULL;
    DtMailEnv error;
    int i, j;

    m_session->mailRc(error)->getValue(error, "additionalfields", &value);

    // First hide all shown headers 
    for (i=0; i < _header_list.length(); i++) {
    	HeaderList * hl = _header_list[i];
    	if (hl->show == SMD_SHOWN && hl->show != SMD_ALWAYS) {
		hl->show = SMD_HIDDEN;
		XtUnmanageChild(hl->form_widget);
    	}
    }

    // Now remove the old list.
    for (i=0; i < _header_list.length(); i++) {
    	HeaderList * hl = _header_list[i];
    	int slot = lookupHeader(hl->label);
	// dont allow removal of default headers.
	hl = _header_list[slot];
	if (slot != -1) {
		if (!reservedHeader(hl->label)) 
			hl->show = SMD_NEVER;
		else if (hl->value != NULL) {
			free(hl->value);
			hl->value = NULL;
        		XtRemoveCallback(hl->field_widget,
                  		XmNvalueChangedCallback, editorContentsChangedCB,
                  		this);
			if (hl->field_widget) 
				XtVaSetValues(hl->field_widget,
				XmNvalue, "",
				NULL);
        		XtAddCallback(hl->field_widget,
                  		XmNvalueChangedCallback, editorContentsChangedCB,
                  		this);
		}
        }
    }

    DtVirtArray<PropStringPair *> results(8);
    parsePropString(value, results);

    if (_additionalfields != NULL) {
	free(_additionalfields);
	_additionalfields = NULL;
    }
    if (value)
	if (*value)
		_additionalfields = value;
	else 
		free(value);

    // New List...
    for (j=results.length(), i=0; i < j; i++) {
    	PropStringPair * psp = results[i];
    	int slot = lookupHeader(psp->label);
    	if (slot != -1) {
		// Already in list
		HeaderList * hl = _header_list[slot];
		if (!reservedHeader(hl->label)) 
                	hl->show = SMD_HIDDEN;
		if (hl->value != NULL) {
			free(hl->value);
			hl->value = NULL;
		}
		if (psp->value != NULL)
			hl->value = strdup(psp->value);
		continue;
    	}	
    	HeaderList * copy_hl = new HeaderList;
    	copy_hl->label = strdup(psp->label);
    	copy_hl->header = strdup(psp->label);
    	if (psp->value)
		copy_hl->value = strdup(psp->value);
	copy_hl->show = SMD_HIDDEN;
	_header_list.append(copy_hl);
    }
    while(results.length()) {
    	PropStringPair * psp = results[0];
    	delete psp;
    	results.remove(0);
    }
    createHeaders(_header_form);
    doDynamicHeaderMenus();
}

void
SendMsgDialog::setInputFocus(const int mode)
{
    if (mode == 0) {
        // Set focus
        HeaderList * hl = _header_list[0];
        (void) XmProcessTraversal(hl->field_widget, XmTRAVERSE_CURRENT);
    }
    else if (mode == 1) {
        Widget edWid = _my_editor->textEditor()->get_editor();
        (void) XmProcessTraversal(edWid, XmTRAVERSE_CURRENT);
    }
}
 
void
SendMsgDialog::attachmentFeedback(
    Boolean bool
)
{
    if (bool) {
        this->busyCursor();
    }
    else {
        this->normalCursor();
    }
}

Boolean
SendMsgDialog::hasAddressee()
{
  DtMailEnv error;
  
  DtMail::Envelope * env = _msgHandle->getEnvelope(error);
  
  // Walk through the headers. 
  // Return TRUE if the message has a value for either of the
  // following headers: To:, Cc:, or Bcc:.
  // Return FALSE if none of the three headers have any value.
  
  for (int scan = 0; scan < _header_list.length(); scan++) {
    HeaderList * hl = _header_list[scan];
    if (hl->show != SMD_ALWAYS && hl->show != SMD_SHOWN) {
      continue;
    }
    if ((strcmp(hl->label, "To") == 0) ||
	(strcmp(hl->label, "Cc") == 0) || 
	(strcmp(hl->label, "Bcc") == 0)) {
      char* value = XmTextGetString(hl->field_widget);
      if (value) {
	for (char *cv = value; *cv; cv++) {
	  if (!isspace(*cv)) {
	    XtFree(value);
	    return(TRUE);	// text value contains contents
	  }
	}
	XtFree(value);		// text value is "content free" - try the next one
      }
    }
  }
  return(FALSE);		// no field has contents 
}
SendMsgDialog::HeaderList*
SendMsgDialog::getFirstHeader()
{
	_header_list_index = 0;
	if (_header_list.length() > 0) {
		HeaderList *hl = _header_list[_header_list_index];
		return (hl);
	}
	return NULL;
}
SendMsgDialog::HeaderList*
SendMsgDialog::getNextHeader()
{
	if ((_header_list_index+1) < _header_list.length()) {
		HeaderList *hl = _header_list[++_header_list_index];
		return (hl);
	}
	return NULL;
}
