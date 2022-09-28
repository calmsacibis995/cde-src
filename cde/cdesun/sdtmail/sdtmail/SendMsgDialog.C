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
#pragma ident "@(#)SendMsgDialog.C	1.352 06/16/97"
#else
static char *sccs__FILE__ = "@(#)SendMsgDialog.C	1.352 06/16/97";
#endif

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
#include <Dt/Dt.h>
#include <DtMail/PropSource.hh>
#include <DtMail/DtMailProps.h>
#include <DtMail/OptCmd.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <sys/param.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <Dt/Editor.h>

#include "MailMsg.h"
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Session.hh>
#include <SDtMail/SystemUtility.hh>
#include <SDtMail/Utility.hh>
#include <SDtMail/DeliveryResponse.hh>
#include <SDtMail/DataTypeUtility.hh>

#ifdef DTMAIL_TOOLTALK
// Time to self destruct
#define DESTRUCT_TIMEOUT   60000    // 1 minutes
#endif

#define AUTOSAVE_TIME 2

const char *XDTMAILEMPTYHEADERS = "X-DTMAIL-EMPTY-HEADERS";
const char *XDTMAILFROMHEADER = "X-DTMAIL-FROM-HEADER";
const char *XDTMAILDATEHEADER = "X-DTMAIL-DATE-HEADER";

static const char *ComposeIcon = "IcMcomp";

struct DefaultHeaders {
  const char *		label;
  const char *		header;
  SendMsgDialog::ShowState	show;
};

static DefaultHeaders DefaultHeaderList[] = {
  { "To",		DtMailMessageTo,	SendMsgDialog::SMD_ALWAYS }, 
  { "Subject",	        DtMailMessageSubject,	SendMsgDialog::SMD_ALWAYS },
  { "Cc",		DtMailMessageCc,	SendMsgDialog::SMD_ALWAYS },
  { "Bcc",	        DtMailMessageBcc,	SendMsgDialog::SMD_HIDDEN },
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
  "Message-Id",
  "Mime-Version",
  "X-Sun-Charset",
  "From",
  "Date",
  XDTMAILEMPTYHEADERS,
  XDTMAILFROMHEADER,
  XDTMAILDATEHEADER,
  NULL
};


static SdmBoolean
block(const char * header)
{
  for (const char ** test = BlockedHeaders; *test; test++) {
    if (strcasecmp(header, *test) == 0) {
      return(Sdm_True);
    }
  }

  return(Sdm_False);
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

SdmBoolean SendMsgDialog::reservedHeader(const char *label)
{
  for (DefaultHeaders * hl = DefaultHeaderList; hl->label; hl++)
    if (strcmp(label, hl->label) == 0)
      return Sdm_True;
  return Sdm_False;
}

SendMsgDialog::SendMsgDialog(Tt_message tt_msg) :
  MenuWindow ( "ComposeDialog", tt_msg ), _header_list(16)
{
  _main_form = NULL;
  _num_sendAs = 0;
  _genDialog = NULL;

  _show_attach_area = Sdm_False;
  _edits_made = Sdm_False;

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
  _auto_save_interval = NULL;
  _auto_save_path = NULL;
  _auto_save_file = NULL;
    
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
  _change_charset = NULL;
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

  _separator = new SeparatorCmd( "Separator", Sdm_True );

  // Help Menu
  _overview = NULL;
  _tasks = NULL;
  _reference = NULL;
  _messages = NULL;
  _on_item = NULL;
  _using_help = NULL;
  _about_mailer = NULL;
  _iconic_session = Sdm_False;  

  _isQuitDialogUp = Sdm_False;

  // Now we need to get the additional headers from the property system.
  //   
  SdmError error;
  SdmConnection *con = theRoamApp.connection()->connection();
  SdmMailRc * mail_rc;
  con->SdmMailRcFactory(error, mail_rc);
  assert(!error); 
  char * value=NULL;  
  mail_rc->GetValue(error, "additionalfields", &value);
 
  DtVirtArray<PropStringPair *> results(8);
  if (error == Sdm_EC_Success) {
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

  if (error == Sdm_EC_Success) {

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
  mail_rc->GetAliasList(alias_stuffing_func, _aliases);
  _aliasCmdList = new CmdList ("aliases", "aliases");
  addAliases();
  addAliasOption();
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
    
  _attachmentActionsList->deleteMe();
    
  // Format
    
  delete _format_word_wrap;
  delete _format_settings;
  delete _change_charset;
  delete _format_find_change;
  delete _format_spell;
  _format_cmds->deleteMe();
  delete _aliasCmd;
  delete _sigCmd;
  delete _vacationCmd;

  if (_aliasCmdList) {
    while (_aliasCmdList->size()) {
      delete (*_aliasCmdList)[0];
      _aliasCmdList->remove(0);
    }
    _aliasCmdList->deleteMe();
  }

  while (_aliases->length()) {
    delete (*_aliases)[0];
    _aliases->remove(0);
  }
  delete _aliases;

    
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
  //[Commented back in the deletion of editor.  8/12/96 - esthert
  delete _my_editor;
  delete _send_button;
  delete _close_button;
  _savecmdList->deleteMe();
    
  delete  _auto_save_path;
  if (_auto_save_file) {
    delete [] _auto_save_file;
  }

  // Loop through the array of header lists and clean up the memory used for
  // each HeaderList.
  while(_header_list.length()) {
    HeaderList * header = _header_list[0];
    _header_list.remove(0);
    delete header;
  }

  _attachmentMenuList->deleteMe();
  _attachmentPopupMenuList->deleteMe();
  if (_textPopupMenuList != NULL) {
    _textPopupMenuList->deleteMe();
  }
  delete _attachment_popup_title;
  delete _compose_popup_title;
  _templates->deleteMe();

  if (_drafts) {
    if (_drafts->getClientData() != NULL) {
      RAMenuClientData* mcd = (RAMenuClientData*) _drafts->getClientData();
      for (int i=0; i<2; i++) {
        if (mcd[i].filename != NULL) 
          free (mcd[i].filename);
      }
      free (mcd);
    }
    _drafts->deleteMe();
  }  
  if (_additionalfields != NULL)
    free(_additionalfields);
  delete _separator;
}

void
SendMsgDialog::parseIt(const char *buf, int len, SdmStrStrL *headers, SdmString *body)
{
  char *eol, *headEnd;
  char *end = (char*)(buf + len);
  char *line = (char*)buf;
  char *cp;
  int eof = Sdm_False;
  char headerName[1024];
  char headerValue[1048];
  char lineBuf[2048];
  SdmBoolean formFlag = Sdm_True;

  // Parse the headers
  while ( !eof ) {
    eol = strchr(line, '\n');
    if ( eol == (char *)NULL) {
      eol = (char*)(buf + len);
    }

    // Skip white spaces
    while ( isspace(*line) && (line < eol) )
      line++;

    if ( line >= (buf + len) )
      return;

    strncpy(lineBuf, line,
	    ((eol - line) > sizeof(lineBuf)) ? sizeof(lineBuf) - 1 : (eol - line));
    lineBuf[((eol - line) > sizeof(lineBuf)) ? sizeof(lineBuf) - 1 : (eol - line)] = '\0';

    // Skip the dreaded Unix "From " header
    if ( formFlag && strstr(lineBuf, "From ") != NULL ) {
      line = eol + 1; // Skip to the next line
      formFlag = Sdm_False;
      continue;
    }

    if ( (headEnd = strchr(lineBuf, ':')) != NULL ) {

      // Check if this is a "real" header
      for (cp = lineBuf; cp < headEnd; cp++) {
	if ( isspace(*cp) )
	  goto bodyPart; // Cheap shot.  Yeah, but it works.
      }

      strncpy(headerName, lineBuf, headEnd - lineBuf);
      headerName[headEnd - lineBuf] = '\0';
      strcpy(headerValue, isspace(*(headEnd + 1)) ? headEnd + 2 : headEnd + 1);
      (*headers)(-1).SetBothStrings(headerName, headerValue);

      line = eol + 1;
    }
    else {
      // This is the end of headers part
      break;
    }
  }

bodyPart:

  if ( line == eol )
    line++;

  // Get the body part
  int strlength = len - (int)(line - buf);
  char *bodyBuf = (char *)malloc(strlength + 1);
  if ( !bodyBuf ) {
    return;
  }

  strncpy(bodyBuf, line, strlength);
  bodyBuf[strlength] = '\0';

  *body = bodyBuf;
  free(bodyBuf);
}

SdmBoolean
SendMsgDialog::parseFile(const char *filename, SdmStrStrL *headers, SdmString *body)
{
  int fd;
  int answer;
  struct stat st;
  char buf[2048];
  char *buffer = NULL;

  DtMailGenDialog * dialog = theRoamApp.genDialog();

  if (SdmSystemUtility::SafeAccess(filename, F_OK) != 0) {
    // We might have started this SMD from session file, dialog is not 
    // initialized yet. Don't put up error dialog, just bring up an empty window
    if (dialog) {
     	sprintf(buf, catgets(DT_catd, 3, 34, "%s does not exist."), filename);
    	dialog->setToErrorDialog(catgets(DT_catd, 1, 203, "Mailer"), buf);
    	answer = dialog->post_and_return(catgets(DT_catd, 3, 9, "OK"), DTMAILHELPNOOPEN);
	}
    return Sdm_False;
  }

  SdmSystemUtility::SafeStat(filename, &st);

  if ((fd = SdmSystemUtility::SafeOpen(filename, O_RDONLY)) == -1) {
    sprintf(buf, catgets(DT_catd, 3, 35, "Unable to open %s."), filename);
    dialog->setToErrorDialog(catgets(DT_catd, 1, 203, "Mailer"), buf);
    answer = dialog->post_and_return(catgets(DT_catd, 3, 9, "OK"), DTMAILHELPNOOPEN);
    return Sdm_False;
  }

  int page_size = (int)sysconf(_SC_PAGESIZE);
  size_t map_size = (size_t) (st.st_size +
                              (page_size - (st.st_size % page_size)));

  char *map;
  map = mmap(0, map_size, PROT_READ, MAP_PRIVATE|MAP_NORESERVE, fd, 0);
  if (map == (char *)-1) {
    // We could not map it for some reason. Let's just read it into buffer
 
    buffer = new char[st.st_size + 1];
 
    if (!buffer) {
      sprintf(buf, catgets(DT_catd, 3, 36, "Unable to allocate memory."));
      dialog->setToErrorDialog(catgets(DT_catd, 1, 203, "Mailer"), buf);
      answer = dialog->post_and_return(DTMAILHELPNOALLOCMEM);
      return Sdm_False;
    }
 
    if (SdmSystemUtility::SafeRead(fd, buffer, (unsigned int) st.st_size) < 0) {
      SdmSystemUtility::SafeClose(fd);
      delete [] buffer;
      return Sdm_False;
    }
 
    buffer[st.st_size] = 0;

    parseIt(buffer, st.st_size, headers, body);
  }
  else {
    parseIt(map, st.st_size, headers, body);
  }

  // Unmap the mapped file
  if ( map != (char *)-1)
    munmap(map, map_size);
  else if (buffer != NULL)
    delete [] buffer;

  SdmSystemUtility::SafeClose(fd);

  return Sdm_True;
}

void
SendMsgDialog::nextTextField(Widget w)
{
  HeaderList *hl=NULL;
  int i, len = _header_list.length();
  SdmBoolean found_it = Sdm_False;

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
	  found_it = Sdm_True;
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
SdmMessage *
SendMsgDialog::makeMessage(SdmError &error)
{
  SdmSession * d_session = theRoamApp.osession()->osession();
  SdmMessageStore *mstore = theRoamApp.GetOutgoingStore(error);
  SdmMessage *msg = NULL;

  if (!error && mstore != NULL) {
    mstore->SdmMessageFactory(error, msg);
  }

  return(msg);
}


// Initialize _msgHandle
void
SendMsgDialog::setMsgHnd()
{
  SdmError error;
  _msgHandle = makeMessage(error);
}

// Set timeout to two minutes (as milliseconds)
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

  SdmSystemUtility::SafeUnlink(_auto_save_file);
  delete [] _auto_save_file;
  _auto_save_file = NULL;
}

static const char *BACKUP_DEAD_LETTER = "./DeadLetters";
static const char *PREFIX = "mail.dead.letter";

void
SendMsgDialog::mkAutoSavePath(void)
{
  int fd;
  // First, see if we need to set up the path.
  //

  if (!_auto_save_path) 
    _auto_save_path = theRoamApp.getResource(DeadletterDir, Sdm_True);

  if ( SdmSystemUtility::SafeAccess(_auto_save_path, W_OK) != 0) {
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
    if ( SdmSystemUtility::SafeAccess(_auto_save_file, F_OK) != 0) {
      // Create the auto save file - since this is a behind the
      // scenes kind of deal and it is an e-mail to be sent, we use
      // mode 0600 as opposed to mode 0666 for security reasons.
      fd = SdmSystemUtility::SafeOpen(_auto_save_file, O_CREAT | O_WRONLY | O_TRUNC, 0600);
      SdmSystemUtility::SafeClose(fd);
      break;
    }
  }
}

void
SendMsgDialog::loadDeadLetter(const char * path)
{
  if (_auto_save_file) {
    SdmSystemUtility::SafeUnlink(_auto_save_file);
    delete [] _auto_save_file;
  }

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
  SdmError error;
  SdmConnection *con = theRoamApp.connection()->connection();
  SdmMailRc * mail_rc;
  con->SdmMailRcFactory(error, mail_rc);
  assert(!error);
  char * value=NULL;
  int elapsed_time, inact_int;

  mail_rc->GetValue(error, "inactivityinterval", &value);
  if (error != Sdm_EC_Success)
        inact_int = 30;
  else {
        inact_int = (int) strtol(value, NULL, 10);
	free(value);
  }

  if ((elapsed_time = (time(0) - theApplication->lastInteractiveEventTime())) 
		>= inact_int) {
	if (self->checkDirty()) {
	    self->doAutoSave();
	}
        self->_auto_save_interval = XtAppAddTimeOut(theApplication->appContext(),
                                              self->getAutoSaveInterval(),
                                              autoSaveCallback,
                                              self);
  } else {
        self->_auto_save_interval = XtAppAddTimeOut(theApplication->appContext(),
                                              (inact_int-elapsed_time) * 1000,
                                              autoSaveCallback,
                                              self);
    
  }
}

// Write out the current contents of SMD
void
SendMsgDialog::writeAutoSave(void)
{
  int		fd;
  int		i;
  int		textLen;
  SdmMessageEnvelope *env;
  SdmString	str;
  SdmStrStrL	headers;
  SdmString	first, second;
  SdmError	error;
  char		buf[2048];
  char		hdrPtr[2048];

  DtMailGenDialog *genDialog = theRoamApp.genDialog();

  // Create the auto save file - since this is a behind the
  // scenes kind of deal and it is an e-mail to be sent, we use
  // mode 0600 as opposed to mode 0666 for security reasons.

  if ((fd = SdmSystemUtility::SafeOpen(_auto_save_file, O_CREAT | O_WRONLY | O_TRUNC, 0600)) == -1) {
    return;
  }

  if (!_msgHandle)
     _msgHandle = makeMessage(error);

  if (!error) {
    // First, write out the envelope
    _msgHandle->SdmMessageEnvelopeFactory(error, env);
  }

  if ( !error ) {
    env->GetHeaders(error, headers);
    if ( error == Sdm_EC_RequestedDataNotFound ) {
      error = Sdm_EC_Success;
    }

    for (i = 0; i < headers.ElementCount(); i++) {
      first = headers[i].GetFirstString();
      second = headers[i].GetSecondString();

      sprintf(hdrPtr, "%s: %s\n", (const char *)first, (const char *)second);

      if (SdmSystemUtility::SafeWrite(fd, hdrPtr, strlen(hdrPtr)) < strlen(hdrPtr)) {
	sprintf(buf,
		catgets(DT_catd, 3, 53, "Unable to write to %s."), _auto_save_file);
	genDialog->setToErrorDialog(catgets(DT_catd, 3, 54, "Mailer"), buf);
	genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOWRITE);
	SdmSystemUtility::SafeClose(fd);
	SdmSystemUtility::SafeUnlink(_auto_save_file);
	return;
      }
    }

    // Write out newline after the last header
    if (SdmSystemUtility::SafeWrite(fd, "\n", 1) < 1) {
	sprintf(buf,
		catgets(DT_catd, 3, 53, "Unable to write to %s."), _auto_save_file);
	genDialog->setToErrorDialog(catgets(DT_catd, 3, 54, "Mailer"), buf);
	genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOWRITE);
	SdmSystemUtility::SafeClose(fd);
	SdmSystemUtility::SafeUnlink(_auto_save_file);
	return;
    }
  }

  // Now, write out the contents of the text editor
  char * widget_text = text();
  if (widget_text == NULL) {
    textLen = 0;
  } else if (widget_text && *widget_text == NULL) {
    textLen = 0;
    widget_text = NULL;
  } else
    textLen = strlen(widget_text);

  if ( widget_text && textLen > 0 ) {
    if (SdmSystemUtility::SafeWrite(fd, widget_text, textLen) < textLen ) {
	sprintf(buf,
		catgets(DT_catd, 3, 53, "Unable to write to %s."), _auto_save_file);
	genDialog->setToErrorDialog(catgets(DT_catd, 3, 54, "Mailer"), buf);
	genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOWRITE);
	SdmSystemUtility::SafeClose(fd);
	SdmSystemUtility::SafeUnlink(_auto_save_file);
	return;
    }
  }
 
  SdmSystemUtility::SafeClose(fd);
}

void
SendMsgDialog::doAutoSave(void)
{
  SdmSession * d_session = theRoamApp.isession()->isession();
  SdmError error;

  busyCursor();
  setStatus(catgets(DT_catd, 3, 70, "Writing dead letter..."));

  // Update the message envelope
  storeHeaders(error, NULL);

  writeAutoSave();

  normalCursor();
  clearStatus();

  // Since we have checkpointed the compose window, we set _edits_made to false,
  // because we don't want the dead letter file updated unnecessarily.
  _edits_made = Sdm_False;
}

int
SendMsgDialog::getAutoSaveInterval(void)
{
  int save_interval;
  SdmError error;

  error.Reset();

  SdmConnection *con = theRoamApp.connection()->connection();
  SdmMailRc * mail_rc;
  con->SdmMailRcFactory(error, mail_rc);
  assert(!error);
    
  char *value;
  mail_rc->GetValue(error, "composeinterval", &value);
  if ((error != Sdm_EC_Success) || value == NULL) {
    save_interval = AUTOSAVE_TIME * 60 * 1000; 
  }
  else {
    save_interval = (int) strtol(value, NULL, 10) * 60 * 1000;
    save_interval = (save_interval <= 0) ? AUTOSAVE_TIME * 60 * 1000 : save_interval;
  }
  if (value) free(value);
    
  return(save_interval);
}


void
SendMsgDialog::setFirstBPHandled(SdmBoolean handle)
{
  _firstBPHandled = handle;
}

void
SendMsgDialog::setHeader(const char * name, const char * value, SdmBoolean append_header)
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

    XmTextFieldSetCursorPosition(hl->field_widget, strlen(hl->value));

    if (hl->show == SMD_HIDDEN)
      changeHeaderState(name);
    XtAddCallback(hl->field_widget,
		  XmNvalueChangedCallback, editorContentsChangedCB,
		  this);
  }
}

void
SendMsgDialog::setHeader(const char * name,
			 SdmStringL & value,
			 SdmBoolean append_header)
{
  if (value.ElementCount() == 0) {
    return;
  }

  if (value.ElementCount() == 1) {
    setHeader(name, (const char *)value[0], append_header);
  }
  else {
    int max_len = 0;
    for (int slen = 0; slen < value.ElementCount(); slen++) {
      max_len += strlen((const char *)value[slen]);
    }

    char * new_str = new char[max_len + (value.ElementCount() * 3)];

    strcpy(new_str, "");
    for (int copy = 0; copy < value.ElementCount(); copy++) {
      if (copy != 0) {
	strcat(new_str, " ");
      }

      strcat(new_str, (const char *)value[copy]);
    }

    setHeader(name, new_str, append_header);
    delete [] new_str;
  }
}

void
SendMsgDialog::loadHeaders(SdmError &error, SdmMessage * input,
			   SdmBoolean load_all, SdmBoolean append_header)
{
  // We are going to go through every header in the message.
  // If it is not one of the headers we block, then we will
  // load it into the header pane, depending on the setting of
  // load_all. If true, then we load every header we allow and
  // create new dynamic headers as necessary. If load_all is false,
  // then we only load headers that already are available in the
  // memory pane.
  //
  error.Reset();
  SdmMessageEnvelope * env;
  SdmStrStrL headers;

  SdmMessage * msg;
  if (input) {
    msg = input;
  } else {
    if (!_msgHandle)
       _msgHandle = makeMessage(error);

    if (error) return;
    msg = _msgHandle;
  }


  msg->SdmMessageEnvelopeFactory(error, env);
  if (error) return;

  // Get all the headers if the message
  env->GetHeaders(error, headers);
  // if there are no headers, reset error and continue.
  if (error == Sdm_EC_RequestedDataNotFound) {
    error = Sdm_EC_Success;
  }
  if (error) return;

  int hcount = 0;
  for (int i = 0; i < headers.ElementCount(); i++) {
    SdmString name;
    SdmString value;
    char *valueStr;
    char *token;
    name = headers[i].GetFirstString();

    // Always ignore the Unix from line.
    //
    if (hcount == 0 &&  name == "From" ) {
      hcount += 1;
      continue;
    }
    hcount += 1;

    // Fix for "empty" headers
    if ( name == XDTMAILEMPTYHEADERS ) {
	value = headers[i].GetSecondString();
	valueStr = strdup(value);

	token = strtok(valueStr, ", ");
	while ( token ) {

	    // Skip this one if we block it
	    if ( block(token) ) {
		continue;
	    }

	    int slot = lookupHeader(token);
	    if (slot < 0) {
		// We don't have a place for this information. We may need
		// to create a new header.
		if (load_all) {
		    HeaderList *hl = new HeaderList;
		    hl->label = strdup(token);
		    hl->header = strdup(token);
		    hl->show = SMD_SHOWN;
		    _header_list.append(hl);
		    createHeaders(_header_form);
		    doDynamicHeaderMenus();
		}
		else {
		    continue;
		}
	    }

	    setHeader(token, "", append_header);

	    token = strtok((char *)0, ", ");
	}

	free(valueStr);
    }

    // Check for user defined From: or Date: headers
    if ( name == "From" || name == "Date" ) {
	for (int j = 0; j < headers.ElementCount(); j++) {
	    SdmString hName;
	    SdmString hValue;
	    int slot;
	    hName = headers[j].GetFirstString();
	    if ( ((slot = lookupHeader(name)) >= 0 ) &&
		 ((name == "From" && hName == XDTMAILFROMHEADER) ||
		  (name == "Date" && hName == XDTMAILDATEHEADER)) ) {

#ifdef INCLUDE_FROM_DATE
		int slot = lookupHeader(name);
		if (slot < 0) {
		    if (load_all) {
		        HeaderList *hl = new HeaderList;
		        hl->label = strdup(name);
		        hl->header = strdup(name);
		        hl->show = SMD_SHOWN;
		        _header_list.append(hl);
			XtManageChild(hl->form_widget);
		    }
		}
		else {
			HeaderList *hl = _header_list[slot];
			hl->show = SMD_SHOWN;
			XtManageChild(hl->form_widget);
		}
#endif
		HeaderList *hl = _header_list[slot];
		hl->show = SMD_SHOWN;
		XtManageChild(hl->form_widget);

		hValue = headers[i].GetSecondString();
		setHeader(name, hValue, Sdm_False);
		justifyHeaders();
		reattachHeaders();
		break;
	    }
	}
    }

    // See if the name is one we always block.
    //
    if (block(name)) {
      continue;
    }

    // I18N - decode only the Subject, To, and Cc headers, 
    // concatenate the returned segments , then append them to the line.
    if ( strcasecmp((const char *)name,"subject")==0
	 || strcasecmp((const char *)name,"to")==0
	 || strcasecmp((const char *)name,"cc")==0)
      {

	// I18N Decode the header and concatenate the segments
	// that are returned, then append them to the line.
	SdmString charset;
	env->GetCharacterSet(error, charset);
	if (error)
	  return;
	env->DecodeHeader (error, value,
			   (const char *)headers[i].GetSecondString(), charset);

      }
    else      // Not Subject, To or Cc, no need to decode
      value = headers[i].GetSecondString();
    // I18N end

    int slot = lookupHeader(name);
    if (slot < 0) {
      // We don't have a place for this information. We may need
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
        continue;
      }
    }

    setHeader(name, value, append_header);
  }
}

void
SendMsgDialog::dumpHeaders(void)
{
  for (int i = 0; i < _header_list.length(); i++) {
    HeaderList * hl = _header_list[i];
    printf("\n");
    if (hl->label)
      printf("%s\n", hl->label);
    if (hl->header)
      printf("%s\n", hl->header);
    if (hl->value)
      printf("%s\n", hl->value);
    if (hl->show)
      printf("%d\n", hl->show);
    if (hl->form_widget)
      printf("%ld\n", hl->form_widget);
    if (hl->label_widget)
      printf("%ld\n", hl->label_widget);
    if (hl->field_widget)
      printf("%ld\n", hl->field_widget);
    if (hl->display_widget)
      printf("%ld\n", hl->display_widget);
    printf("\n");
  }
}

void
SendMsgDialog::storeHeaders(SdmError &error, SdmMessage * input)
{
  error.Reset();
  SdmMessage * msg;
  if (input) {
    msg = input;
  } else {
    if (!_msgHandle)
       _msgHandle = makeMessage(error);

    if (error) return;
    msg = _msgHandle;
  }

  SdmMessageEnvelope * env;
  msg->SdmMessageEnvelopeFactory(error, env);
  if (error) return;
  SdmStrStrL headers;

  // Walk through the headers. Fetch the strings from the ones that are
  // visible to the user and stuff them into the message.
  for (int scan = 0; scan < _header_list.length(); scan++) {
    HeaderList * hl = _header_list[scan];
    if (hl->show != SMD_ALWAYS && hl->show != SMD_SHOWN) {
      continue;
    }

    char *value = XmTextGetString(hl->field_widget);

    // If the header has a value, we want to set the value in the back end.
    // Otherwise, we don't want to send out a blank header, so we remove it.
    if (value) {
      if (*value)
        headers(-1).SetBothStrings(hl->label, value);
      XtFree(value);
    }
  }

  // Now set all the headers
  env->SetHeaders(error, headers);
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
  char * value=NULL;
  XtVaGetValues(hl->field_widget, XmNvalue, &value, NULL);
  if (hl->show == SMD_SHOWN) {
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
  }
  else if (value)
    	XmTextFieldSetCursorPosition(hl->field_widget, strlen(value));

  if (value)
      XtFree(value);

  // Now we need to toggle the current state of the header.
  //
  char label[100];
  char button_name[100];

  if (hl->show == SMD_SHOWN) {
    XtUnmanageChild(hl->form_widget);
    hl->show = SMD_HIDDEN;
    sprintf(label, "%s ", catgets(DT_catd, 1, 228, "Add"));
    // Create old label for changing
    sprintf(button_name, "%s ", catgets(DT_catd, 1, 229, "Delete"));
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
  XmString label = XmStringCreateLocalized((char*)str);

  XtVaSetValues(_status_text,
		XmNlabelString, label,
		NULL);

  XmUpdateDisplay(baseWidget());
  XmStringFree(label);
}

void
SendMsgDialog::clearStatus(void)
{
  setStatus(" ");
}

// This function provides a means for handling an error that occurs when a Submit
// returns an error - given a DtMailGenDialog to use and an error object, it will
// popup an appropriate message and wait for the user to hit ok.

void
SendMsgDialog::submitErrorProc (DtMailGenDialog *genDialog, SdmError& error)
{
  char *helpId = NULL;
  char buf[2048];
  buf[0] = '\0';

  // Special case certain error conditions here; the reason for this
  // is certain mid end conditions may be generic (e.g. out of memory)
  // and we need to provide a better indication of why an operation
  // failed.

  if (error.IsErrorContained(Sdm_EC_NoMemory)) {
    // Mailer ran out of memory.  Ask the user to quit some other
    // applications so there will be more memory available.
    sprintf(buf, catgets(DT_catd, 5, 6,
			 "Mailer does not have enough memory\n\
available to send this message.\n\
Try quitting other applications and\n\
resend this message."));
    helpId = DTMAILHELPNOMEMORY;
  }
  else {
    // No more special case conditions - if there is only one
    // contained error (no minor count) then force "fail" to be the
    // major code so that the current major code becomes a minor code
    // - that will force up a "more" button so the user can see the
    // real reasons why the error occurred by hitting more. Then force
    // in a generic send message failed error message.

    if (error.MinorCount() == 0)
      error.AddMajorErrorCode(Sdm_EC_Fail);

    sprintf(buf, catgets(DT_catd, 5, 7,
			 "An error occurred while trying to send your message.\n\
Check to make sure the message was received.  If not,\n\
you may have to resend this message."));
    helpId = DTMAILHELPTRANSPORTFAILED;
  }

  genDialog->post_error(error, helpId, *buf ? buf : NULL);
}

void
SendMsgDialog::appendSignature(SdmBoolean always_append)
{
  SdmError error;
  SdmSession * d_session = theRoamApp.osession()->osession();
  SdmMailRc *mail_rc;
  theRoamApp.connection()->connection()->SdmMailRcFactory(error, mail_rc);
  assert(!error);
  int status;
  char *value = NULL;

  // Sometimes we want to append regardless of the property setting, such
  // as in the case of the Include Signature menu option on Compose Format menu
  if (!always_append) {
    if (mail_rc->IsValueDefined("dontincludesignature")) {
      get_editor()->textEditor()->set_to_top();
      return;
    }
  }

  MailRcSource *prop_source = (MailRcSource*) new MailRcSource (mail_rc, "signature");
  value = (char*)prop_source->getValue();
  delete prop_source;
  char * fullpath;
  SdmUtility::ExpandPath(error, fullpath, value, *mail_rc, SdmUtility::kFolderResourceName);

  if (value)
    free(value);

  FILE *fp;
  fp = fopen(fullpath, "r");
  if (fp != NULL) {
    status = get_editor()->textEditor()->append_to_contents("\n", 2);
    if (status == 0)
      status = get_editor()->textEditor()->append_to_contents(fileno(fp));
    fclose(fp);
  }

  free(fullpath);
  if (!always_append)
    get_editor()->textEditor()->set_to_top();
}

void
SendMsgDialog::send_message(int transport_type)
{
  SdmError error;
  AttachArea *attachArea = get_editor()->attachArea();
  int numPendingActions, answer;
  char *helpId = NULL;
  char buf[2048], buf2[1024];
  int bodyCount;
  const char *cont;
  SdmBoolean deleteDraft = Sdm_False;
  SdmString contents;
  SdmMailRc * mail_rc;
  theRoamApp.connection()->connection()->SdmMailRcFactory(error, mail_rc);
  assert(!error);

  buf2[0] = '\0';

  // Hide the window immediately to avoid user confusion.
  this->unmanage();

  // Check whether message has addressees.
  if (!this->hasAddressee()) {
    // Message has no valid addressee.  Pop up error dialog.

    sprintf(buf, catgets(DT_catd, 5, 8,
			 "Try Send after specifying  recipient(s) of the message in \nthe To:, Cc:, or Bcc: fields."));

    helpId = DTMAILHELPNEEDADDRESSEE;

    // popup the compose window
    // Set the window state to Normal before manage the window
    XtVaSetValues(this->baseWidget(), XtNinitialState, NormalState, NULL);
    this->manage();

    _genDialog->setToErrorDialog(catgets(DT_catd, 2, 21, "Mailer"),
				 (char *) buf);
    _genDialog->post_and_return(catgets(DT_catd, 3, 76, "OK"), helpId);

    return;
  }

  // Set up the envelope
  SdmMessageEnvelope *env;
  SdmStrStrL headers;
  SdmStringL customHeaders;
  SdmBoolean customXMailer = Sdm_False;

  // first call updateHeaders to get _header_list up to date.
  updateHeaders();

  for (int scan = 0; scan < _header_list.length(); scan++) {
    HeaderList * hl = _header_list[scan];
    if (hl->show != SMD_ALWAYS && hl->show != SMD_SHOWN) {
      continue;
    }

    if ((strcasecmp(hl->label, "To") == 0) ||
        (strcasecmp(hl->label, "Cc") == 0) || 
        (strcasecmp(hl->label, "Bcc") == 0) ||
        (strcasecmp(hl->label, "Reply-To") == 0) ||
        (strcasecmp(hl->label, "Subject") == 0)) {

      char* value = XmTextGetString(hl->field_widget);

      if (strcasecmp(hl->label, "Subject") != 0 && strcasecmp(hl->label, "Reply-To")) {
	// Make sure value contains something other than spaces.
	SdmBoolean only_spaces = Sdm_True;
	if (value) {
	  for (int i=0; i<strlen(value); i++) {
	    if (value[i] != ' ') {
	      only_spaces = Sdm_False;
	      break;
	    }
	  }
	  if (only_spaces) {
	    XtFree(value);
	    value = NULL;
	  }
	}
        if (value && !theRoamApp.validateAliases(error, value)) {
	    _genDialog->post_error(error, DTMAILHELPALIASESINVALIDALIST);
	    XtFree(value);
	    value = NULL;

	    XtVaSetValues(this->baseWidget(), XtNinitialState, NormalState, NULL);
	    this->manage();
	    return;
	}
      }

      headers(-1).SetBothStrings(hl->label, value);

      if (value) {
        XtFree(value);
	value = NULL;
      }
    }
    else {
      if (strcasecmp(hl->label, "x-mailer") == 0)
        customXMailer = Sdm_True;
      headers(-1).SetBothStrings(hl->label, hl->value);
      customHeaders(-1) = hl->label;
    }
  }

  if (!_msgHandle)
     _msgHandle = makeMessage(error);

  if (!error)
     _msgHandle->SdmMessageEnvelopeFactory(error, env);

  if (error) {
    // Message has no valid addressee. 

    // popup the compose window
    // Set the window state to Normal before manage the window
    XtVaSetValues(this->baseWidget(), XtNinitialState, NormalState, NULL);
    this->manage();

    // popup the error dialog
    _genDialog->post_error(error);

    return;
  }
  env->SetHeaders(error, headers);
  if (error) {
    // popup the compose window
    // Set the window state to Normal before manage the window
    XtVaSetValues(this->baseWidget(), XtNinitialState, NormalState, NULL);
    this->manage();

    // popup the error dialog
    _genDialog->post_error(error);

    return;
  }

  // Add an X-Mailer header if one hasn't been specified in the custom headers
  if (!customXMailer) {
    char version[32];
    sprintf(version, "%d.%d.%d ", SDtVERSION, SDtREVISION, SDtUPDATE_LEVEL);
    SdmString xmailerPrefix("dtmail ");
    xmailerPrefix += version;
    xmailerPrefix += SDtVERSION_STRING;

    env->AddXMailerHeader(error, xmailerPrefix, Sdm_True);
    if (!error) {
      customHeaders(-1) = "X-Mailer";
    }
  }

  // Now set up the body part(s)
  SdmMessageBody *bp;
  SdmMessageBodyStructure mbstruct;

  if (!error) {
    _msgHandle->GetBodyCount(error, bodyCount);
  }
  
  if (error) {
    // popup the compose window
    // Set the window state to Normal before manage the window
    XtVaSetValues(this->baseWidget(), XtNinitialState, NormalState, NULL);
    this->manage();

    _genDialog->post_error(error);

    return;
  }

  if ( bodyCount == 0) {
    _msgHandle->SdmMessageBodyFactory(error, bp,
				      (SdmMsgStrType)Sdm_MSTYPE_text, "plain");
  
  }
  else { 
    _msgHandle->SdmMessageBodyFactory(error, bp, 1);
    if (!error) {
      bp->GetStructure(error, mbstruct);
    }
    
    if (!error) {
      // if this is a multipart mixed, use the first body part in the
      // multipart body.  We already created multipart body part for
      // attachments
      if ( mbstruct.mbs_type == Sdm_MSTYPE_multipart ) {
        if (!error) {
          if ((_my_editor->attachArea()->getIconCount() -
               _my_editor->attachArea()->getDeleteCount()) == 0) {
            SdmBoolean prev;
            bp->DeleteMessageBodyFromMessage(error, prev, Sdm_True);
            _msgHandle->SdmMessageBodyFactory(error, bp,
                                              (SdmMsgStrType)Sdm_MSTYPE_text, "plain");
          }
          else {
            SdmMessage	*nestedMsg;
            bp->SdmMessageFactory(error, nestedMsg);
            nestedMsg->SdmMessageBodyFactory(error, bp, 1);
          }
        }
      }
    }
  }
  
  if (error) {
    // popup the compose window
    // Set the window state to Normal before manage the window
    XtVaSetValues(this->baseWidget(), XtNinitialState, NormalState, NULL);
    this->manage();

    // popup the error dialog
    _genDialog->post_error(error);

    return;
  }


  // Set up body parts.  First things first, i.e., first body part
  // always contains the contents of text editor.
  Boolean wrap;
  if (((WordWrapCmd *)_format_word_wrap)->wordWrap())
    wrap = True;
  else
    wrap = False;

  cont = _my_editor->textEditor()->get_contents(wrap);
  SdmContentBuffer content_buf(cont);
  // I18N set the user selected charset on the content buffer
  content_buf.SetCharacterSet(error, _user_charset);
  if (!error) {
    bp->SetContents(error, content_buf);
  }
  if (!error) {
    SdmDataTypeUtility::SetContentTypesForMessage(error, *bp);
    if (error) error.Reset();
  }
  
  if (error) {
    // Error occurred.  
    // popup the compose window
    // Set the window state to Normal before manage the window
    XtVaSetValues(this->baseWidget(), XtNinitialState, NormalState, NULL);
    this->manage();

    // popup the error dialog
    _genDialog->post_error(error);

    return;
  }
  
  char *rvalue = NULL;
  // The rest of body parts have been set during "add_att" actions.
  // So, all body parts are set at this point.

  // Just get text from text widget; attachment BPs are filled already when
  // they are included/forwarded/added.
  if (mail_rc->IsValueDefined("displaymsgsizewarning")) {
    //  display the warning popup 
    rvalue = NULL;
    mail_rc->GetValue(error, "msgsizelimit", &rvalue);
    if ((error == Sdm_EC_Success) && rvalue && *rvalue) {
      int len_total=0; 

      _msgHandle->SdmMessageBodyFactory(error, bp, 1);
      if ( !error ) {
        bp->GetStructure(error, mbstruct);
	if ( !error ) {
          if ( mbstruct.mbs_type == Sdm_MSTYPE_multipart ) {
            SdmMessage        *nestedMessage;
            bp->SdmMessageFactory(error, nestedMessage);
	    if ( !error ) {
              nestedMessage->GetBodyCount(error, bodyCount);
	      if ( !error ) {
                for (int i = 1; i <= bodyCount; i++) {
                  nestedMessage->SdmMessageBodyFactory(error, bp, i);
		  if ( !error ) {
                    bp->GetStructure(error, mbstruct);
		    if ( !error ) {
                      len_total += mbstruct.mbs_bytes;
                    } else {
                      break;
                    }
                  } else {
                    break;
                  }
                }
              }
            }
          } else {
            bp->GetStructure(error, mbstruct);
            if (!error) 
              len_total = mbstruct.mbs_bytes;
          }
        }
      }

      if (error) {
        // Error occurred.  Pop up error dialog.
        // popup the compose window
        // Set the window state to Normal before manage the window
        XtVaSetValues(this->baseWidget(), XtNinitialState, NormalState, NULL);
        this->manage();

        // popup the error dialog
	_genDialog->post_error(error, DTMAILHELPERROR, 
			       catgets(DT_catd, 5, 11, "Cannot get size for new message"));
        return;
      }      
      
      long lrvalue = strtol(rvalue, NULL, 10);
      if (len_total > (lrvalue*1000)) {
        char len_total_str [50];
        int bytes;
        if (len_total < 1000) 
          sprintf(len_total_str, "%d", len_total);
        else if (len_total < 1000000) {
          bytes = (int)ceil((double)len_total/(double)1000);
          sprintf(len_total_str, "%dKbytes", bytes);
        }
        else {
          bytes = (int)ceil((double)len_total/(double)1000000);
          sprintf(len_total_str, "%dMbytes", bytes);
        }
        sprintf(buf, catgets( DT_catd, 5, 9, 
                              "The size of this message ( %s ) exceeds the\nmessage size limit ( %dKbytes ) you have set\nin your mail options. You can:" ), 
                len_total_str, lrvalue);
            
        _genDialog->setToQuestionDialog(
                catgets(DT_catd, 5, 1, "Mailer"),
                buf);
        helpId = DTMAILHELPMSGSIZELIMIT;
        answer = _genDialog->post_and_return(
                                             catgets(DT_catd, 1, 230, "Send"),
                                             catgets(DT_catd, 3, 19, "Cancel"), helpId);

        if (answer == 2) {
          // Cancel selected
          this->manage();
          return;
        }
      }
    }
    if (rvalue != NULL) free (rvalue);
    error.Reset();
  }

  // Check if there are any pending attachments (open, print....)  If so,
  // pop up a dialog.  If the user wants to Send the message as is, continue
  // with the submission process.  If the user opted to Cancel, then return.
  numPendingActions = attachArea->getNumPendingActions();
  sprintf(buf, catgets(
		       DT_catd, 
		       3, 
		       77, 
		       "You have an attachment open that may have unsaved changes.\nSending this message will break the connection to the open\n attachment. Any unsaved changes will not be part of the\n message. You can use Save As to save Changes after the\n connection is broken, but the changes will not be part of\n the attachment." ));
    
  while (numPendingActions != 0) {
    // popup the compose window
    this->manage();

    // The user tried to send a messages without saving changes in some open
    // attachments.  This warning makes sure that is what the user intended.
    _genDialog->setToQuestionDialog(catgets(DT_catd, 5, 1, "Mailer"), buf);
    helpId = DTMAILHELPPENDINGACTIONS;
	
    answer = _genDialog->post_and_return(helpId);

    if (answer == 1) {
      // OK selected
      numPendingActions = 0;
      this->unmanage();
    }
    else if (answer == 2) {
      // Cancel selected
      return;
    }
  }

  rvalue = NULL;
  SdmBoolean expertFlag;
  expertFlag = mail_rc->IsValueDefined("expert");

  char *dir;
  char *file = getDraftFile();
  if ( file && expertFlag == Sdm_True ) { // Don't ask; just delete the draft file
    deleteDraft = Sdm_True;
    if (strncmp(file, "mail.dead.letter.", strlen("mail.dead.letter.")) == 0)
      dir = (char*)theRoamApp.getResource(DeadletterDir, Sdm_True);
    else 
      dir = (char*)theRoamApp.getResource(DraftDir, Sdm_True);
    sprintf(buf2, "%s/%s", dir, file);
    free(dir);
  }
  else if ( file && expertFlag == Sdm_False ) {
    // Drafts can be found in either the drafts dir or dead letter dir
    if (strncmp(file, "mail.dead.letter.", strlen("mail.dead.letter.")) == 0)
      dir = (char*)theRoamApp.getResource(DeadletterDir, Sdm_True);
    else 
      dir = (char*)theRoamApp.getResource(DraftDir, Sdm_True);
    sprintf(buf2, "%s/%s", dir, file);
    free(dir);
    sprintf(buf, catgets( DT_catd, 3, 118, 
		    "Sending the message will destroy the Draft:\n%s"), buf2);
    _genDialog->setToQuestionDialog( catgets(DT_catd, 5, 1, "Mailer"), buf);
    answer = _genDialog->post_and_return(catgets(DT_catd, 3, 119, "Send and Destroy"),
					 catgets(DT_catd, 3, 19, "Cancel"),
					 DTMAILHELPSENDDELETEDRAFT);
    if (answer == 2) { 
      // Cancel selected
      // popup the compose window
      this->manage();
      return;
    }
    else if ( answer == 1 ) {
      deleteDraft = Sdm_True;
    }
  }

  error.Reset();

  SubmitContext *ctx = new SubmitContext;
  ctx->deleteDraft = deleteDraft;
  ctx->smd = this;
  if (*buf2) {
    ctx->draft = strdup(buf2);
  } else {
    ctx->draft = NULL;
  }

  SdmServiceFunction submit_async_svf((const SdmCallback)&SendMsgDialog::submitAsyncCB,
				      (void *)ctx, Sdm_SVF_SolicitedEvent);
  _msgHandle->Submit_Async(error, submit_async_svf, (void *)ctx,
			   (transport_type == 0) ? Sdm_MSFMT_Mime : Sdm_MSFMT_SunV3,
			   _log_msg, customHeaders);
  if ( error ) {
    submitErrorProc(_genDialog, error);
    XtVaSetValues(this->baseWidget(), XtNinitialState, NormalState, NULL);
    this->manage();
  }
}

void
SendMsgDialog::submitAsyncCB(SdmError &,
			     void *client_data,
			     SdmServiceFunctionType type,
			     SdmEvent *event)
{
  char buf[MAXPATHLEN + MAXNAMELEN];
  SubmitContext *ctx = (SubmitContext *)client_data;
  DtMailGenDialog *genDialog = theRoamApp.genDialog();

  switch(type) {
  case Sdm_SVF_SolicitedEvent:
    if (*(event->sdmSubmitMessage.error) != Sdm_EC_Success) {
      submitErrorProc(genDialog, *(event->sdmSubmitMessage.error));
      XtVaSetValues(ctx->smd->baseWidget(), XtNinitialState, NormalState, NULL);
      ctx->smd->manage();
    } else {
      ctx->smd->goAway(Sdm_False);
      if (ctx->smd->_msgHandle) {
	delete ctx->smd->_msgHandle;
	ctx->smd->_msgHandle = NULL;
      }  
      if (ctx->deleteDraft && ctx->draft) {
	if ( SdmSystemUtility::SafeUnlink(ctx->draft) != 0 ) {
	    sprintf(buf, catgets(DT_catd, 3, 120,
                "Unable to destroy the draft file:\n%s"), ctx->draft);
	    genDialog->setToErrorDialog(catgets(DT_catd, 3, 48, "Mailer"), buf);
	    genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPERROR);
        }
      }
    }
    if (ctx->draft) {
      free(ctx->draft);
    }
    delete ctx;
    break;
  }
}

void
SendMsgDialog::editorContentsChangedCB(Widget, XtPointer cd, XtPointer)
{
  SendMsgDialog * self = (SendMsgDialog *)cd;
  self->_edits_made = Sdm_True;
}

Widget 
SendMsgDialog::createWorkArea ( Widget parent )
{
  FORCE_SEGV_DECL(CmdInterface, ci);
  Widget send_form;
  // SdmSession *m_session = theRoamApp.session()->session();
  SdmError error;
  SdmConnection *con = theRoamApp.connection()->connection();
  SdmMailRc * mail_rc;
  con->SdmMailRcFactory(error, mail_rc);
  char *value = NULL;
    
  // Create the parent form
  _main_form = XmCreateForm( parent, "Work_Area", NULL, 0 );
    
  XtAddCallback(_main_form, XmNhelpCallback, HelpCB, DTMAILCOMPOSEWINDOW);
    
    
  // Create the area for status messages.
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
  mail_rc->GetValue(error, "composewincols", &value);
  if ((error == Sdm_EC_Success) && value && *value) {
    long cols = strtol(value, NULL, 10);
    XtVaSetValues(twid, DtNcolumns, cols, NULL);
  }
  if (value != NULL)
    free(value);
	
  _my_editor->attachArea()->setOwnerShell(this);
  _my_editor->setEditable(Sdm_True);
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
			       Sdm_True, 
			       this, 
			       -1);
  ci  = new ButtonInterface (send_form, _send_button);

  XtVaSetValues(ci->baseWidget(),
		XmNleftAttachment, XmATTACH_FORM,
		XmNleftOffset, 10,
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
				       Sdm_True,
				       this,
				       baseWidget());

  _savecmdList->add(_save_as_draft);

  _save_as_template = new SaveAsTemplateCmd ("Template...",
					     catgets(DT_catd, 1, 240, "Template..."),
					     Sdm_True,
					     get_editor()->textEditor(),
					     this,
					     baseWidget());

  _savecmdList->add(_save_as_template);

  _save_as_text = new SaveAsTextCmd ("saveAsText",
				     catgets(DT_catd, 1, 242, "Text..."),
				     catgets(DT_catd, 1, 126, "Mailer - Save As Text"),
				     Sdm_True,
				     get_editor(),
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
  XtVaSetValues(_savecmdList->getPaneWidget(),
                  XmNtearOffModel, XmTEAR_OFF_ENABLED,
                  NULL);
    
  // Create Close Button
  _close_button = new CloseCmd (
				"Close",
				catgets(DT_catd, 1, 118, "Close"),
				Sdm_True, 
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

  // Force creation of just the menu
  (*button)->addCommands(NULL);

  Widget menubutton = (*button)->baseWidget();
  Widget menupane = (*button)->getPaneWidget();

  // The following call has to be registered to allow the tear-off's title
  // to be set, since Motif only sets the title for tear-off menus which are
  // Pulldown; the menubutton's tear-off is a Popup.
  XtAddCallback(menupane, XmNtearOffMenuActivateCallback,
                &SendMsgDialog::setTearOffTitle, menubutton);

  XtVaSetValues(menubutton, XmNuserData, *button, NULL);

  // The following callback is where the menu items are created - it's done
  // "lazily" for performance reasons.
  XtAddCallback(menubutton, XmNcascadingCallback,
                &SendMsgDialog::mapAliasMenu, this);
}

void
SendMsgDialog::setTearOffTitle(Widget widget, XtPointer clientData, XtPointer)
{
  Widget menubutton = (Widget)clientData;
  XmString xmstring;
  String string;

  XtVaGetValues(menubutton, XmNlabelString, &xmstring, NULL);
  XmStringGetLtoR(xmstring, XmSTRING_DEFAULT_CHARSET, &string);
  XmStringFree(xmstring);

  XtVaSetValues(XtParent(widget), XmNtitle, string, NULL);
  XtFree(string);
}

void
SendMsgDialog::mapAliasMenu(Widget widget, XtPointer clientData, XtPointer)
{
  MenuButton *mb;
  XtVaGetValues(widget, XmNuserData, &mb, NULL);

  SendMsgDialog *smd = (SendMsgDialog*)clientData;

  if (mb->requireUpdate()) {
    mb->setRequireUpdate(FALSE);

    if (smd->_aliasCmdList) {
      // The following side-effects destruction of any old menu and
      // recreation of a new menu!
      mb->addCommands (smd->_aliasCmdList);
      Widget menu = mb->getPaneWidget();
      XtVaSetValues(menu,
                    XmNpacking, XmPACK_COLUMN,
                    XmNorientation, XmVERTICAL,
                    XmNtearOffModel, XmTEAR_OFF_ENABLED,
                    NULL);

      theRoamApp.configure_menu(menu);
    }
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
			      XmNtraversalOn,	Sdm_True,
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
	strcasecmp("subject", hl->label) != 0) {
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
  // This is really a pain, but we have to blow away the list to build
  // another one. This could probably be done more efficiently, but we wont
  // try to figure out how right now.
  if (_format_cmds) {
    _menuBar->removeOnlyCommands(_format_menu, _format_cmds); // JSC , FALSE);
    // Delete the actual commands themselves, since they're never used
    // again elsewhere and would otherwise be leaked.
    for (int i = 0; i < _format_cmds->size(); i++)
      delete (*_format_cmds)[i];
    _format_cmds->deleteMe();
    _format_cmds = NULL;
  }

  _format_cmds = new CmdList("DynamicFormatCommands", "DynamicFormatCommands");

  // Only put on commands that are shown or hidden. The items that
  // are always are never should not be presented to the user as
  // an option to change.
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

    Cmd *new_cmd = new HideShowCmd(label, label, 1, this, hl->label);
    // Add the commands one at a time with addCommand() vs. all at once with
    // addCommands(). That way new commands will be created instead of
    // reusing old ones.
    _menuBar->addCommand(_format_menu, new_cmd);
    _format_cmds->add(new_cmd);
  }
}

static void
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
SendMsgDialog::include_file(char *selection)
{
  FILE *fp;
  int status;

  // Open the file to be included
  fp = fopen(selection, "r");
  if (fp != NULL) {
    // File could be opened - insert it into the editor widget at the
    // current cursor position
    status = this->_my_editor->textEditor()->insert_at_cursor("\n", 2);
    if (status == 0)
      status = this->_my_editor->textEditor()->insert_at_cursor(fileno(fp));
    if (status == 0)
      status = this->_my_editor->textEditor()->insert_at_cursor("\n", 2);
    fclose(fp);
    this->_my_editor->textEditor()->set_to_top();
    if (status == 0)
      return;
  }

  // If we get here, it means the file could not be included...
  char buf[MAXPATHLEN];
  sprintf(buf, catgets(DT_catd, 2, 18, "Error: Cannot include file %s"), 
	  selection);
  theInfoDialogManager->post("Mailer", 
			     buf, 
			     (void *)this->_file_include, 
			     bogus_cb);
}

void
SendMsgDialog::add_att_cb( void *client_data, char *selection )
{
  SendMsgDialog *obj = (SendMsgDialog *)client_data;
  obj->add_att(selection);
  obj->_att_select_all->activate();
}

void
SendMsgDialog::add_att(const char *file)
{
  SdmError		error;
  SdmMessage		*nestedMsg;
  SdmMessageBody	*bp;
  SdmMessageBody	*firstBP;

  if (!_msgHandle)
     _msgHandle = makeMessage(error);

  if (!error) {
    if ( this->get_editor()->attachArea()->getIconCount() == 0 ) {
      _msgHandle->SdmMessageBodyFactory(error, bp,
				        (SdmMsgStrType)Sdm_MSTYPE_multipart, "mixed");
      if (!error) {
        bp->SdmMessageFactory(error, nestedMsg);
      }
      if (!error) {
        nestedMsg->SdmMessageBodyFactory(error, firstBP,
				       (SdmMsgStrType)Sdm_MSTYPE_text, "plain");
      }
    }
    else {
      _msgHandle->SdmMessageBodyFactory(error, bp, 1);
      if (!error) {
        bp->SdmMessageFactory(error, nestedMsg);
      }
    }
  }
  
  if (error) {
      // popup the compose window
      // Set the window state to Normal before manage the window
      XtVaSetValues(this->baseWidget(), XtNinitialState, NormalState, NULL);
      this->manage();

      // popup the error dialog
      _genDialog->post_error(error, DTMAILHELPERROR, 
			     catgets(DT_catd, 12, 9, "Cannot create message attachment."));
      return;
  }

  /* Set ADD flag for adding Attachment via File Selection */
  this->get_editor()->attachArea()->setFsState(ADD);

  this->get_editor()->attachArea()->addAttachment(nestedMsg, 
						  file, NULL);
    
  this->activate_default_attach_menu();

  // This will manage the attach pane too.
  ((ToggleButtonCmd *)_att_show_pane)->setButtonState(Sdm_True, Sdm_True);
  _edits_made = Sdm_True;
}

void
SendMsgDialog::add_att(const char *name, SdmString buf)
{
  SdmError		error;
  SdmMessage		*nestedMsg;
  SdmMessageBody	* bp;
  SdmMessageBody	* firstBP;

  if (!_msgHandle)
     _msgHandle = makeMessage(error);

  if (!error) {
    if ( this->get_editor()->attachArea()->getIconCount() == 0 ) {
      _msgHandle->SdmMessageBodyFactory(error, bp,
				      (SdmMsgStrType)Sdm_MSTYPE_multipart, "mixed");
      if (!error) {
        bp->SdmMessageFactory(error, nestedMsg);
      }
      if (!error) {
        nestedMsg->SdmMessageBodyFactory(error, firstBP,
				     (SdmMsgStrType)Sdm_MSTYPE_text, "plain");
      }
    }
    else {
      _msgHandle->SdmMessageBodyFactory(error, bp, 1);
      if (!error) {
        bp->SdmMessageFactory(error, nestedMsg);
      }
    }
  }

  if (error) {
    // popup the compose window
    // Set the window state to Normal before manage the window
    XtVaSetValues(this->baseWidget(), XtNinitialState, NormalState, NULL);
    this->manage();
    
    // popup the error dialog
    _genDialog->post_error(error, DTMAILHELPERROR,
			   catgets(DT_catd, 12, 9, "Cannot create message attachment."));
    return;
  }

  this->get_editor()->attachArea()->addAttachment(nestedMsg,
						  SdmString(name),
						  buf);
  this->activate_default_attach_menu();
    
  // This will manage the attach pane too.
  ((ToggleButtonCmd *)_att_show_pane)->setButtonState(Sdm_True, Sdm_True);
  _edits_made = Sdm_True;
}

void
SendMsgDialog::add_att(SdmString buf)
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
  SdmError mail_error;

  mail_error.Reset();

  AttachArea *attarea = this->get_editor()->attachArea();
  Attachment *attachment = attarea->getSelectedAttachment();

  // Get selected attachment, if none selected, then return.
  if ( attachment == NULL ) {
    // Let User know that no attachment has been selected???
    int answer = 0;
    char *helpId = NULL;

    _genDialog->setToErrorDialog(catgets(DT_catd, 1, 120, "Mailer"),
				 catgets(DT_catd, 2, 19, "An attachment needs to be selected before issuing the\n\"Save As\" command to save to a file.") );
    helpId = DTMAILHELPSELECTATTACH;
    answer = _genDialog->post_and_return(
					 catgets(DT_catd, 3, 74, "OK"), helpId );
    return;
  }

  // Save selected attachments.
  attachment->saveToFile(mail_error, selection);
  if ( mail_error != Sdm_EC_Success) {
    _genDialog->post_error(mail_error);
    return;
  }
}

void
SendMsgDialog::resetAliasMenus(void)
{
  for (int i = 0; i < _header_list.length(); i++) {
    HeaderList *hl = _header_list[i];
    hl->display_widget = hl->label_widget;
    if (reservedHeader(hl->label) && strcasecmp("subject", hl->label) != 0) {
      hl->display_widget = (hl->mb_widget)->baseWidget();

      hl->mb_widget->setRequireUpdate(TRUE);
    }
  }
}

void
SendMsgDialog::propsChanged(void)
{
  SdmError error;
  SdmConnection *con = theRoamApp.connection()->connection();
  SdmMailRc * mail_rc;
  con->SdmMailRcFactory(error, mail_rc);
  assert(!error);
  char *value = NULL;

  mail_rc->GetValue(error, "hideattachments", &value);

  if (value) {
    if (_show_attach_area) {
      _show_attach_area = Sdm_False;
      this->hideAttachArea();
    }
    free(value);
  }
  else if (!_show_attach_area) {
    _show_attach_area = Sdm_True;
    this->showAttachArea();
  }
  mail_rc->GetValue(error, "dontlogmessages", &value);
  if (error == Sdm_EC_Success) {
    setLogState(Sdm_False);
    ((ToggleButtonCmd *)_file_log)->setButtonState(Sdm_False, Sdm_True);
  }
  else {
    setLogState(Sdm_True);
    ((ToggleButtonCmd *)_file_log)->setButtonState(Sdm_True, Sdm_True);
  }
  OptCmd *oc = theRoamApp.mailOptions();
  if (!oc) return;
  DtbOptionsDialogInfo od = (DtbOptionsDialogInfo)oc->optionsDialog();
  if (!od) return;
  int alias_changed=0;
  XtVaGetValues(od->alias_tf, XmNuserData, &alias_changed, NULL);
  if (alias_changed) {
    extern void alias_stuffing_func(char *, void *, void *);
    while (_aliases->length()) {
      delete (*_aliases)[0];
      _aliases->remove(0);
    }
    mail_rc->GetAliasList(alias_stuffing_func, _aliases);
    if (_aliasCmdList) {
      _aliasCmdList->deleteMe();
    }

    _aliasCmdList = new CmdList ("aliases", "aliases");
    addAliases();
    addAliasOption();
    resetAliasMenus();
  }
}

void
SendMsgDialog::createFileMenu()
{
  CmdList *cmdList;

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
					 Sdm_True, 
					 SendMsgDialog::include_file_cb, 
					 NULL,
					 this,
					 NULL,
					 this->baseWidget());
    

  CmdList *subcmdList = new CmdList ( "Save As", catgets(DT_catd, 1, 243 , "Save As") );

  subcmdList->add(_save_as_draft);
  subcmdList->add(_save_as_template);
  subcmdList->add(_save_as_text);
    
  if (!_file_log)
    _file_log = new LogMsgCmd ( 
			       "Save Outgoing Mail",
			       catgets(DT_catd, 1, 127, "Save Outgoing Mail"), Sdm_True, this);
    
  // 0 for default transport.
    
  if (!_file_send)
    _file_send = new SendCmd (
			      "Send",
			      catgets(DT_catd, 1, 117, "Send"), 
			      Sdm_True, 
			      this, 
			      -1);
    
  CmdList *subcmdList1 = new CmdList ( "Send As", catgets(DT_catd, 1, 128, "Send As") );

  // Hardcode "Internet MIME" & "Sun Mail Tool" transport protocols.
  _file_sendAs[_num_sendAs] = new SendCmd( 
					  "Internet MIME",
					  "Internet MIME",
					  Sdm_True, 
					  this, 
					  0);
  subcmdList1->add( _file_sendAs[_num_sendAs++] );

  _file_sendAs[_num_sendAs] = new SendCmd( 
					  "Sun Mail Tool",
					  "Sun Mail Tool",
					  Sdm_True, 
					  this, 
					  1);
  subcmdList1->add( _file_sendAs[_num_sendAs++] );

  if (!_file_close)
    _file_close = new CloseCmd ( 
				"Close",
				catgets(DT_catd, 1, 129, "Close"), 
				Sdm_True, 
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
    
  _menuBar->addCommands ( &_file_cascade, cmdList, Sdm_False, XmMENU_BAR);

  subcmdList->deleteMe();
  subcmdList1->deleteMe();
  cmdList->deleteMe();
}

void
SendMsgDialog::createMenuPanes()
{
  CmdList *cmdList;
  Cardinal n = 0;

  // File

  createFileMenu();
    
  // Edit
    
  cmdList = new CmdList( "Edit", catgets(DT_catd, 1, 130, "Edit") );
    
  _edit_undo = new EditUndoCmd ( "Undo", catgets(DT_catd, 1, 131, "Undo"), Sdm_True, this );
  _edit_cut = new EditCutCmd ( "Cut", catgets(DT_catd, 1, 132, "Cut"), Sdm_True, this );
  _edit_copy = new EditCopyCmd ( "Copy", catgets(DT_catd, 1, 133, "Copy"), Sdm_True, this );
  _edit_paste = new EditPasteCmd ( "Paste", catgets(DT_catd, 1, 134 , "Paste"), 
				   Sdm_True, 
				   this );
    
  // Begin Paste Special submenu
  CmdList *subcmdList1 = new CmdList ( "Paste Special", catgets(DT_catd, 1, 135 , "Paste Special") );
  _edit_paste_special[0] = new EditPasteSpecialCmd (
						    "Bracketed",
						    catgets(DT_catd, 1, 136 , "Bracketed"),
						    Sdm_True, this, Editor::IF_BRACKETED
						    );
  subcmdList1->add(_edit_paste_special[0]);
  _edit_paste_special[1] = new EditPasteSpecialCmd (
						    "Indented",
						    catgets(DT_catd, 1, 137 , "Indented"), 
						    Sdm_True, this, Editor::IF_INDENTED );
  subcmdList1->add(_edit_paste_special[1]);
  // End Paste Special submenu
    
  _edit_clear = new EditClearCmd ( "Clear", catgets(DT_catd, 1, 138, "Clear"), 
				   Sdm_True, 
				   this );
  _edit_delete = new EditDeleteCmd ( "Delete", catgets(DT_catd, 1, 139, "Delete"), 
				     Sdm_True, 
				     this );
  _edit_select_all = new EditSelectAllCmd ( 
					   "Select All",
					   catgets(DT_catd, 1, 140, "Select All"), 
					   Sdm_True, 
					   this );
  _format_find_change = new FindChangeCmd ( 
					   "Find/Change...",
					   catgets(DT_catd, 1, 155, "Find/Change..."), 
					   Sdm_True, 
					   this );
  _format_spell = new SpellCmd (
				"Check Spelling...",
				catgets(DT_catd, 1, 156, "Check Spelling..."), 
				Sdm_True, 
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
  cmdList->deleteMe();
  subcmdList1->deleteMe();


  // Compose Popup CmdList
  construct_text_popup();
    
  // Attachment
    
  cmdList = new CmdList( "Attachments", catgets(DT_catd, 1, 141, "Attachments") );
    
  _att_add   = new SelectFileCmd (
				  "Add File...",
				  catgets(DT_catd, 1, 142, "Add File..."),
				  catgets(DT_catd, 1, 143, "Mailer - Attachments - Add"), 
				  catgets(DT_catd, 1, 144, "Add"),
				  Sdm_True, 
				  SendMsgDialog::add_att_cb,
				  NULL,
				  this,
				  NULL,
				  this->baseWidget());

  _att_save  = new SaveAttachCmd (
				  "Save As...",
				  catgets(DT_catd, 1, 145, "Save As..."),
				  catgets(DT_catd, 1, 146, "Mailer - Attachments - Save As"),
				  Sdm_False, 
				  SendMsgDialog::save_att_cb,
				  this,
				  this->baseWidget());
  _att_delete = new DeleteAttachCmd (
				     "Delete",
				     catgets(DT_catd, 1, 147, "Delete"),
				     Sdm_False, 
				     this);
  _att_undelete = new UndeleteAttachCmd (
					 "Undelete",
					 catgets(DT_catd, 1, 148, "Undelete"),
					 Sdm_False, 
					 this);
  _att_rename = new RenameAttachCmd(
				    "Rename",
				    catgets(DT_catd, 1, 149, "Rename"),
				    Sdm_False,
				    this);
    
  _att_select_all = new SelectAllAttachsCmd( 
					    "Select All",
					    catgets(DT_catd, 1, 150, "Select All"), 
					    this
					    );
    
  // This is the label for a toggle item in a menu.  When the item is set to
  // "Show List", the Attachment List is mapped in the Compose Window.  This
  // message replaces message 151 in set 1.
  _att_show_pane = new ShowAttachPaneCmd(
					 "Show List",
					 catgets(DT_catd, 1, 226, "Show List"),
					 this
					 );
  cmdList->add( _att_add );
  cmdList->add( _att_save );
  cmdList->add( _separator );
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
  cmdList->deleteMe();
    
  createFormatMenu();
    
  _overview = new OnAppCmd("Overview", catgets(DT_catd, 1, 71, "Overview"),
			   Sdm_True, this);
  _tasks = new TasksCmd("Tasks", catgets(DT_catd, 1, 72, "Tasks"), 
			Sdm_True, this);
  _reference = new ReferenceCmd("Reference", catgets(DT_catd, 1, 73, "Reference"), 
				Sdm_True, this);
  _messages = new MessagesCmd("Messages", catgets(DT_catd, 1, 301, "Messages"), 
			      Sdm_True, this);
  _on_item = new OnItemCmd("On Item", catgets(DT_catd, 1, 74, "On Item"),
			   Sdm_True, this);
  _using_help = new UsingHelpCmd("Using Help", catgets(DT_catd, 1, 75, "Using Help"), 
				 Sdm_True, this);
  _about_mailer = new RelNoteCmd("About Mailer...", catgets(DT_catd, 1, 77, "About Mailer..."),
				 Sdm_True, this);
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
  _menuBar->addCommands(cmdList, Sdm_True);
  cmdList->deleteMe();
}

void
SendMsgDialog::addAliases()
{
  int list_len = _aliases->length();
  for (int i = 0; i < list_len; i++) {
    PropStringPair *psp = (*_aliases)[i];
    if (psp->label && psp->value) {
      Cmd * subCmd = new AliasCmd(psp->label, psp->label, 
				  1, this, psp->label);
      _aliasCmdList->insert(subCmd);
    }
  }
}
void
SendMsgDialog::addAliasOption()
{
  Cmd * cmd = new OptionsCmd ("Aliases",
                              catgets(DT_catd, 1, 269, "Aliases..."),
                              1, this->baseWidget(), OPT_AL);
  _aliasCmdList->add(cmd);
}

void
SendMsgDialog::construct_attachment_popup(void)
{
  _attachmentPopupMenuList = new CmdList( "AttachmentsPopup", "AttachmentsPopup");

  _attachment_popup_title = new LabelCmd
    ("Mailer - Attachments", catgets(DT_catd, 1, 158, "Mailer - Attachments"), Sdm_True);

  _attachmentPopupMenuList->add( _attachment_popup_title);
  _attachmentPopupMenuList->add( _separator);
  _attachmentPopupMenuList->add( _att_add );
  _attachmentPopupMenuList->add( _att_save );
  _attachmentPopupMenuList->add( _att_delete );
  _attachmentPopupMenuList->add( _att_undelete );
  _attachmentPopupMenuList->add( _att_rename );
  _attachmentPopupMenuList->add( _att_select_all );

  _menuPopupAtt = new MenuBar(_my_editor->attachArea()->getClipWindow(), 
			      "RoamAttachmentPopup", XmMENU_POPUP);
  _attachmentPopupMenu = _menuPopupAtt->addCommands(_attachmentPopupMenuList, 
						    Sdm_False, XmMENU_POPUP);
}

void
SendMsgDialog::construct_text_popup(void)
{
  if (theApplication->bMenuButton() != Button3)
    return;

  _textPopupMenuList = new CmdList( "TextPopup", "TextPopup");

  _compose_popup_title = new LabelCmd
    ("Mailer - Compose", catgets(DT_catd, 1, 159, "Mailer - Compose"), Sdm_True);

  _textPopupMenuList->add( _compose_popup_title);
  _textPopupMenuList->add( _separator);
  _textPopupMenuList->add( _file_send);
#ifdef undef
  _textPopupMenuList->add( _edit_undo );
  _textPopupMenuList->add( _edit_cut );
  _textPopupMenuList->add( _edit_copy );
  _textPopupMenuList->add( _edit_paste );
  _textPopupMenuList->add( _edit_delete );
  _textPopupMenuList->add( _edit_select_all );
#endif

  Widget parent = _my_editor->textEditor()->get_editor();
  _menuPopupText = new MenuBar(parent, "SendMsgTextPopup", XmMENU_POPUP);
  _textPopupMenu = _menuPopupText->addCommands(_textPopupMenuList, 
					       Sdm_False, XmMENU_POPUP);

   DtEditorCreateFileMenu(parent, _textPopupMenu);
   DtEditorCreateEditMenu(parent, _textPopupMenu);
   DtEditorCreateFormatMenu(parent, _textPopupMenu);
   DtEditorCreateOptionsMenu(parent, _textPopupMenu);
   DtEditorCreateExtrasMenu(parent, _textPopupMenu);
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
					 Sdm_True, 
					 this );
  if (!_format_settings)
    _format_settings = new FormatCmd ( "Settings...",
				       catgets(DT_catd, 1, 154, "Settings..."), 
				       Sdm_True, 
				       this );
    
  // I18N add the Change Char Set command
  if (!_change_charset)
    _change_charset = new GetPossibleCharsetsCmd ( "Change Char Set",
						   catgets(DT_catd, 1, 316,
							   "Change Char Set"), 
						   Sdm_True, 
						   this,
						   Sdm_False );
    
  cmdList->add( _format_word_wrap );
  cmdList->add( _format_settings );
  cmdList->add( _change_charset );
  cmdList->add( _separator);
    
  if (_templates)
    _templates->deleteMe();
  _templates = new CmdList ( "Use Template", catgets(DT_catd, 1, 157, "Use Template") );

  addTemplates(_templates);
  cmdList->add(_templates);

  if (_drafts) {
    if (_drafts->getClientData() != NULL) {
      RAMenuClientData* mcd = (RAMenuClientData*) _drafts->getClientData();
      for (int i=0; i<2; i++) {
        if (mcd[i].filename != NULL) 
          free (mcd[i].filename);
      }
      free (mcd);
    }
    _drafts->deleteMe();
  }
  _drafts = new CmdList ( "Use Draft", catgets(DT_catd, 3, 108, "Use Draft") );
  cmdList->add(_drafts);

  if (!_signature)
    _signature = new ComposeSigCmd ( "Include Signature", 
				     catgets(DT_catd, 1, 299, "Include Signature"),
				     Sdm_True,
				     NULL,
				     this );
  cmdList->add(_signature);

  cmdList->add( _separator );


  if (!_aliasCmd)
    _aliasCmd = new OptionsCmd(
			       "Aliases",
			       catgets(DT_catd, 1, 269, "Aliases..."),
			       1,
				this->baseWidget(),
			       OPT_AL);
  if (!_sigCmd)
    _sigCmd = new OptionsCmd(
			     "Signature",
			     catgets(DT_catd, 1, 266, "Signature..."),
			     1,
				this->baseWidget(),
			     OPT_SIG);

  if (!_vacationCmd)
    _vacationCmd = new OptionsCmd(
				  "Vacation Message",
				  catgets(DT_catd, 1, 267, "Vacation Message..."),
				  1,
				this->baseWidget(),
				  OPT_VAC);

  cmdList->add( _sigCmd );
  cmdList->add( _vacationCmd );
  cmdList->add( _aliasCmd );
  cmdList->add( _separator );

  // If there is no main mailbox window then deactivate the tunnels to the options dialog.
  // The toolbar config stuff requires a RoamMenuWindow object to be around for it to work
  // correctly.  The better solution might be to just disable those option panes that
  // require a RoamMenuWindow.
  if (theRoamApp.numOpenRMW() < 1) {
      _sigCmd->deactivate();
      _vacationCmd->deactivate();
      _aliasCmd->deactivate();
  }

  _format_menu = _menuBar->addCommands ( &_format_cascade, cmdList, 
					 Sdm_False, XmMENU_BAR);

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
  RAMenuClientData *mcd = (RAMenuClientData*) calloc (2, sizeof(RAMenuClientData));
  mcd[0].timestamp = 0;
  mcd[0].filename = theRoamApp.getResource(DraftDir, Sdm_True);
  mcd[0].smd = this;
  mcd[1].timestamp = 0;
  mcd[1].filename = theRoamApp.getResource(DeadletterDir, Sdm_True);
  mcd[1].smd = this;
  _drafts->setClientData((void*)mcd);
  XtAddCallback(menu, XmNmapCallback, &RoamApp::map_menu, _drafts);

  cmdList->deleteMe();

  doDynamicHeaderMenus();
}

void
SendMsgDialog::addTemplates(CmdList * subCmd)
{
  int count;
  char *value=NULL;
  Cmd *button;

  SdmError error;
  SdmConnection *con = theRoamApp.connection()->connection();
  SdmMailRc * mail_rc;
  con->SdmMailRcFactory(error, mail_rc);
  assert(!error);

  mail_rc->GetValue(error, "templates", &value);

  if (!value || !(*value)) {
    button = new LoadFileCmd("No Templates",
                             catgets(DT_catd, 3, 114, "No Templates"),
			     1, this, NULL, TYPE_TEMPLATE);

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
      button = new LoadFileCmd(psp->label,
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
SendMsgDialog::initialize(int argc, char **argv)
{
  Cardinal n = 0;
  Arg args[1];
  char * hideAttachPane = NULL;
  SdmError error;
    
  // Without the TearOffModelConverter call, there will be warning messages:
  // Warning: No type converter registered for 'String' to 'TearOffModel' 
  // conversion.
  XmRepTypeInstallTearOffModelConverter();
  MenuWindow::initialize(argc, argv);
  XtSetArg(args[n], XmNdeleteResponse, XmDO_NOTHING); n++;
  XtSetValues( _w, args, n);
    
  _genDialog = new DtMailGenDialog("Dialog", _main, 
                     XmDIALOG_PRIMARY_APPLICATION_MODAL);
    
  // See if the .mailrc specifies if attachPane is to be shown or hid 
  // at SMD startup time.
    
  SdmConnection *con = theRoamApp.connection()->connection();
  SdmMailRc * mail_rc;
  con->SdmMailRcFactory(error, mail_rc);
  assert(!error);

  mail_rc->GetValue(error, "hideattachments", &hideAttachPane);
    
  if (!hideAttachPane) {
    _show_attach_area = Sdm_True;
  }
  else {
    _show_attach_area = Sdm_False;
    // The user wants to hide attachments

    this->hideAttachArea();
  }
    
  // Log Message Toggle button.  A LogMsgCmd is a ToggleButtonCmd....
  error.Reset();
  char * value = NULL;
  mail_rc->GetValue(error, "dontlogmessages", &value);
  if (value) free (value);
  if (error == Sdm_EC_Success) {
    // "dontlogmessages" is Sdm_True
    setLogState(Sdm_False);
    ((ToggleButtonCmd *)_file_log)->setButtonState(Sdm_False, Sdm_True);
  } else {
    // "dontlogmessages" is Sdm_False
    setLogState(Sdm_True);
    ((ToggleButtonCmd *)_file_log)->setButtonState(Sdm_True, Sdm_True);
  }
    
  // Word Wrap Toggle button.  A WordWrapCmd is a ToggleButtonCmd...
  ((ToggleButtonCmd *)_format_word_wrap)->setButtonState(
							 ((WordWrapCmd *)_format_word_wrap)->wordWrap(), 
							 Sdm_False
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
  XtRemoveAllCallbacks(theApplication->baseWidget(), XmNdestroyCallback);
  delete theApplication;
}

// Clears Compose window title, header fields, text, and attachment areas.
void
SendMsgDialog::reset()
{
  SdmError error;
  
  _my_editor->textEditor()->clear_contents();
  _my_editor->attachArea()->resetPendingAction();

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

  if (_msgHandle) {
    // Need to destroy current Message handle.
    delete _msgHandle;    // All its body parts are deleted.
    _msgHandle = NULL;
  }

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
  SdmConnection *con = theRoamApp.connection()->connection();
  SdmMailRc * mail_rc;
  con->SdmMailRcFactory(error, mail_rc);
  assert(!error);

  if (mail_rc->IsValueDefined("dontlogmessages") == Sdm_True) {
    // "dontlogmessages" is Sdm_True
    setLogState(Sdm_False);
    ((ToggleButtonCmd *)_file_log)->setButtonState(Sdm_False, Sdm_True);
  } else {
    // "dontlogmessages" is Sdm_False
    setLogState(Sdm_True);
    ((ToggleButtonCmd *)_file_log)->setButtonState(Sdm_True, Sdm_True);
  }

}

// Given a file name, include the file as attachment.
void
SendMsgDialog::inclAsAttmt(char *file, char *name)
{
  SdmError error;
  SdmMessage *nestedMsg;
  SdmMessageBody *bp, *firstBP;

  if (!_msgHandle)
     _msgHandle = makeMessage(error);

  if (!error) {
    // Create a nested SdmMessage object from the sole bodypart of the
    // main SdmMessage object.  We will add the attachment to this nested
    // message.
    _msgHandle->SdmMessageBodyFactory(error, bp, Sdm_MSTYPE_multipart,
                                    "mixed");
  }

  if (!error) {
    bp->SdmMessageFactory(error, nestedMsg);
  }
  if (!error) {
    nestedMsg->SdmMessageBodyFactory(error, firstBP, Sdm_MSTYPE_text, "plain");
  }

  if (error) {
    // popup the compose window
    // Set the window state to Normal before manage the window
    XtVaSetValues(this->baseWidget(), XtNinitialState, NormalState, NULL);
    this->manage();

    // popup the error dialog
    _genDialog->post_error(error, DTMAILHELPERROR,
			   catgets(DT_catd, 12, 9, "Cannot create message attachment."));
    return;
  }

  this->get_editor()->attachArea()->addAttachment(nestedMsg, file, name);
}

// Given a buffer, include its content as an attachment.
void
SendMsgDialog::inclAsAttmt(unsigned char *contents, int, char *name)
{
  SdmString mbuf;
  SdmError error;
  SdmMessage *nestedMsg;
  SdmMessageBody *bp, *firstBP;

  if (!_msgHandle)
     _msgHandle = makeMessage(error);

  if (!error) {

    // Create a nested SdmMessage object from the sole bodypart of the main
    // SdmMessage object.  We will add the attachment to this nested
    // message.
    _msgHandle->SdmMessageBodyFactory(error, bp, Sdm_MSTYPE_multipart, "mixed");
  }

  if (!error) {
    bp->SdmMessageFactory(error, nestedMsg);
  }
  if (!error) {
    nestedMsg->SdmMessageBodyFactory(error, firstBP, Sdm_MSTYPE_text, "plain");
  }
    
  if (error) {
    // popup the compose window
    // Set the window state to Normal before manage the window
    XtVaSetValues(this->baseWidget(), XtNinitialState, NormalState, NULL);
    this->manage();

    // popup the error dialog
    _genDialog->post_error(error, DTMAILHELPERROR,
			   catgets(DT_catd, 12, 9, "Cannot create message attachment."));
    return;
  }

  mbuf = (const char *)contents;
  this->get_editor()->attachArea()->addAttachment(nestedMsg,
                                                  (SdmString)name, mbuf);
}

void
SendMsgDialog::parseNplace(const char *fullpath)
{
  SdmError error;
  SdmSession *d_session = theRoamApp.osession()->osession();
  SdmSession *i_session = theRoamApp.isession()->isession();
  int status, reset_compose = FALSE;
  int i;
  SdmMessageBody *tmpBP = NULL;
  SdmMailRc *mail_rc;
  SdmToken token;
  char *value;

  error.Reset();

  // Unable to access the file?
  if ( SdmSystemUtility::SafeAccess(fullpath, R_OK ) != 0) {
    return;
  }

  theRoamApp.connection()->connection()->SdmMailRcFactory(error, mail_rc);
  assert(!error);

  DtMailGenDialog * dialog = genDialog();

  token.SetValue("servicechannel", "cclient");
  token.SetValue("serviceclass", "messagestore");
  token.SetValue("servicetype", "local");
  token.SetValue("serviceoption", "readonly", "x");
  token.SetValue("serviceoption", "messagestorename", fullpath);
  mail_rc->GetValue(error, "dontsavemimemessageformat", &value);
  if (!error) {
    token.SetValue("serviceoption", "preservev3messages", "x");
    if (value) free(value);
  }

  SdmMessageStore *tmpmbox = NULL;
  SdmStrStrL headers;
  SdmString bodyStr;
  SdmMessage *msg;
  SdmMessageEnvelope *env;
  SdmMessageBody *body;
  int fd;
  static const char   *temp = "mbox.XXXXXX";
  char                fileName[MAXPATHLEN];

  i_session->SdmMessageStoreFactory(error, tmpmbox);
  if ( !tmpmbox || error ) {
    // don't free full path.  it's a const that is passed into this function.
    return;
  }

  tmpmbox->StartUp(error);
  if ( error ) {
    // don't free full path.  it's a const that is passed into this function.
    return;
  }

  // Peek at the file named in fullpath and make sure there is a unix From line
  theRoamApp.checkForUnixFromLine(fullpath);

  fileName[0] = '\0';

  SdmBoolean readOnly;
  SdmMessageNumber nmsgs = 0;
  tmpmbox->Open(error, nmsgs, readOnly, token);
  if ( error ) {
    if ( parseFile(fullpath, &headers, &bodyStr) != Sdm_True ) {
      // don't free full path.  it's a const that is passed into this function.
      return;
    }

    // Create a temporary file to serve as an outgoing mailbox
    strcpy(fileName, "/tmp/");
    strcat(fileName, temp);
    mktemp(fileName);

    // Create the outgoing mailbox - since this is a behind the
    // scenes kind of deal and it is a mailbox, we use mode 0600 as
    // opposed to mode 0666 for security reasons.

    if ( (fd = SdmSystemUtility::SafeOpen(fileName, O_CREAT | O_WRONLY, 0600)) == -1 ) {
      return;
    }
 
    SdmSystemUtility::SafeClose(fd);

    // Change the token with appropriate values
    token.ClearValue("serviceoption", "messagestorename", fullpath);
    token.SetValue("serviceoption", "messagestorename", fileName);
    token.ClearValue("serviceoption", "readonly");
    token.SetValue("serviceoption", "readwrite", "x");

    // don't free full path.  it's a const that is passed into this function.
    // free(fullpath);

    if ( tmpmbox )
      delete tmpmbox;
 
    d_session->SdmMessageStoreFactory(error, tmpmbox);
    if ( error ) {
      delete tmpmbox;
      SdmSystemUtility::SafeUnlink(fileName);
      return;
    }

    tmpmbox->StartUp(error);
    if ( error ) {
      delete tmpmbox;
      SdmSystemUtility::SafeUnlink(fileName);
      return;
    }

    tmpmbox->Open(error, nmsgs, readOnly, token);
    if ( error ) {
      delete tmpmbox;
      SdmSystemUtility::SafeUnlink(fileName);
      return;
    }

    // Create a message in this auxiliary outgoing message store
    tmpmbox->SdmMessageFactory(error, msg);
    if ( error ) {
      delete tmpmbox;
      SdmSystemUtility::SafeUnlink(fileName);
      return;
    }

    msg->SdmMessageEnvelopeFactory(error, env);
    if ( error ) {
      SdmSystemUtility::SafeUnlink(fileName);
      return;
    }

    if (headers.ElementCount() > 0) {
      env->SetHeaders(error, headers);
      if ( error ) {
        delete tmpmbox;
        SdmSystemUtility::SafeUnlink(fileName);
        return;
      }
    }

    msg->SdmMessageBodyFactory(error, body,
			       (SdmMsgStrType)Sdm_MSTYPE_text, "plain");
    if ( error ) {
      delete tmpmbox;
      SdmSystemUtility::SafeUnlink(fileName);
      return;
    }

    SdmContentBuffer content_buf(bodyStr);
    // I18N set the user selected charset on the content buffer
    content_buf.SetCharacterSet(error, getCharacterSet());
    if ( error ) {
      delete tmpmbox;
      SdmSystemUtility::SafeUnlink(fileName);
      return;
    }

    body->SetContents(error, content_buf);
    if ( error ) {
      delete tmpmbox;
      SdmSystemUtility::SafeUnlink(fileName);
      return;
    }

    SdmDataTypeUtility::SetContentTypesForMessage(error, *body);
    if (error) {
      error.Reset();  // just continue.
    }
  }
  else { // File successfully opened.
    // don't free full path.  it's a const that is passed into this function.
    tmpmbox->SdmMessageFactory(error, msg, 1);
    if ( error ) {
      delete tmpmbox;
      return;
    }
  }

  SdmMessageBody *bp = NULL;
  msg->SdmMessageBodyFactory(error, bp, 1); // Get the first message body
  if (bp == NULL || error) {
#ifdef DOLATER
    dialog = genDialog();
    if (_type == TYPE_TEMPLATE) {
      dialog->setToWarningDialog(catgets(DT_catd, 1, 203, "Mailer"),
				 catgets(DT_catd, 1, 249, "The template is empty."));
      dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPTEMPLATEEMPTY);
    }
    else if (_type == TYPE_DRAFT) {
      dialog->setToWarningDialog(catgets(DT_catd, 1, 203, "Mailer"),
				 catgets(DT_catd, 1, 290, "The draft is empty."));
      dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPDRAFTEMPTY);
    }
#endif
    if ( *fileName )
      SdmSystemUtility::SafeUnlink(fileName);
      
    if (tmpmbox)
      delete tmpmbox;
      
    return;
  }

  XtRemoveCallback(getTextW(), XmNmodifyVerifyCallback,
		   &SendMsgDialog::editorContentsChangedCB, this);

  if (reset_compose)
    loadHeaders(error, msg, Sdm_True, Sdm_False);
  else
    loadHeaders(error, msg, Sdm_True, Sdm_True);
 
  SdmContentBuffer contentBuf;
  SdmString contents;
  SdmMessageBodyStructure bodyStruct;

  if (!error) {
    bp->GetStructure(error, bodyStruct);
  }

  if ( !error ) {
    if (bodyStruct.mbs_type == Sdm_MSTYPE_multipart ) {
      SdmMessage *nestedMsg;
      int bodyCount;
      const char *name;

      bp->SdmMessageFactory(error, nestedMsg);
      if (!error) {
        nestedMsg->GetBodyCount(error, bodyCount);
      }

      if (!error) {
        for (i = 1; i <= bodyCount; i++) {

          nestedMsg->SdmMessageBodyFactory(error, bp, i);
          if (error) break;

          if ( i == 1 ) { // Set text editor
            bp->GetContents(error, contentBuf);
            if (error) break;
            contentBuf.GetContents(error, contents);
            if (error) break;
            status = get_editor()->textEditor()->insert_at_cursor((const char *)contents, contents.Length());
            get_editor()->textEditor()->set_to_top();
            continue;
          }

          bp->GetContents(error, contentBuf);
          if (error) break;        
          contentBuf.GetContents(error, contents);
          if (error) break;
          bp->GetStructure(error, bodyStruct);
          if (error) break;
          name = bodyStruct.mbs_attachment_name;
          add_att((char *)name, contents);
        }
      }
    }
    else {
      bp->GetContents(error, contentBuf);
      if (!error) {
        contentBuf.GetContents(error, contents);
      }
      if (!error) {
        status = get_editor()->textEditor()->insert_at_cursor((const char *)contents, contents.Length());
        get_editor()->textEditor()->set_to_top();
      }
    }
  }

  if (tmpmbox) 
    delete tmpmbox;

  if ( *fileName )
    SdmSystemUtility::SafeUnlink(fileName);
}

// Given a RFC_822_Message formatted file, parse it and fill the
// Compose Window.
void
SendMsgDialog::parseNplace(char *contents, int len)
{
  int                 fd;
  static const char   *temp = "mbox.XXXXXX";
  char                fileName[32];
 
  strcpy(fileName, "/tmp/");
  strcat(fileName, temp);
  mktemp(fileName);
 
  // Create the parse in place file - since this is a behind the
  // scenes kind of deal and it is an e-mail message, we use mode 0600
  // as opposed to mode 0666 for security reasons.

  if ((fd = SdmSystemUtility::SafeOpen(fileName, O_CREAT|O_WRONLY, 0600)) == -1)
    return;

  // Write out the buffer contents
  write(fd, contents, len);
 
  SdmSystemUtility::SafeClose(fd);
  parseNplace(fileName);

  SdmSystemUtility::SafeUnlink(fileName);
}

void
SendMsgDialog::text(const char *text)
{
  _my_editor->textEditor()->set_contents( text, strlen(text) );
}

void
SendMsgDialog::append(const char *text)
{
  _my_editor->textEditor()->insert_at_cursor( text, strlen(text) );
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
    newsend->initialize(0, NULL);
    theCompose.fitIntoList(newsend, Sdm_True);
    theRoamApp.unbusyAllWindows();
    _num_created++;
  } else {
    newsend->resetHeaders();
    _not_in_use--;
    // If we have unused SMD, it might have been set as iconic state if
    // it's started from session file. Reset it to normal state
    if (newsend->is_iconic_session() == Sdm_True) 
      XtVaSetValues(newsend->baseWidget(), XtNinitialState, NormalState, NULL);

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
Compose::SessiongetWin(int argc, char **argv)
{
  SendMsgDialog *newsend = NULL;
    
#ifdef DTMAIL_TOOLTALK
  if ( _timeout_id ) {
    XtRemoveTimeOut( _timeout_id );
    _timeout_id = 0;
  }
#endif

  newsend = new SendMsgDialog();
  newsend->initialize(argc, argv);
  theCompose.fitIntoList(newsend, Sdm_True);
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
    _attachmentActionsList->deleteMe();
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
    _attachmentActionsList->deleteMe();
    _attachmentActionsList = NULL;
  }
}

void
SendMsgDialog::addAttachmentActions(char **actions, int indx)
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
    _attachmentActionsList->deleteMe();
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
    free (actionLabel);
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
  SdmError mail_error;

  // Initialize the mail_error.
  mail_error.Reset();

  AttachArea *attachArea = _my_editor->attachArea();
  attachArea->deleteSelectedAttachments(mail_error);

  // Activate this button to permit the user to undelete.
  _att_undelete->activate();

  // Deactivate buttons that will be activated when another
  // selection applies.

  _att_save->deactivate();
  _att_delete->deactivate();
  _att_rename->deactivate();
  if ((_my_editor->attachArea()->getIconCount() -
	 _my_editor->attachArea()->getDeleteCount()) == 0)
  	_att_select_all->deactivate();

  if (_attachmentActionsList) {
    _menuBar->removeCommands(_attachmentMenu, _attachmentActionsList);
    _menuPopupAtt->removeCommands(_attachmentPopupMenu, 
				  _attachmentActionsList);
    _attachmentActionsList->deleteMe();
    _attachmentActionsList = NULL;
  }
  _edits_made = Sdm_True;
}

void
SendMsgDialog::undelete_last_deleted_attachment()
{
  AttachArea *attachArea = _my_editor->attachArea();
  attachArea->undeleteLastDeletedAttachment();
    
    
  if(_my_editor->attachArea()->getIconSelectedCount()) 
    _att_delete->activate();

  if (attachArea->getDeleteCount() == 0) {
    _att_undelete->deactivate();
  }
  if ((_my_editor->attachArea()->getIconCount() -
	 _my_editor->attachArea()->getDeleteCount()) > 0)
  	_att_select_all->activate();
}

SdmBoolean
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
	
    return(Sdm_False);
  }
  else {
    return(Sdm_True);
  }
}

void
SendMsgDialog::showAttachArea()
{
  DtMailEditor *editor = this->get_editor();
  editor->showAttachArea();
  ((ToggleButtonCmd *)_att_show_pane)->setButtonState(Sdm_True, Sdm_False);
}

void
SendMsgDialog::hideAttachArea()
{
  DtMailEditor *editor = this->get_editor();
  editor->hideAttachArea();
  ((ToggleButtonCmd *)_att_show_pane)->setButtonState(Sdm_False, Sdm_False);
}

int
SendMsgDialog::lookupHeader(const char * name)
{
  for (int h = 0; h < _header_list.length(); h++) {
    HeaderList * hl = _header_list[h];
    if ( strcmp(hl->label, name) == 0 )
      return(h);
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
  WidgetList manageList = new Widget[_header_list.length()];
  WidgetList unmanageList = new Widget[_header_list.length()];
  Cardinal mli = 0, uli = 0;

  for (int h = 1; h < _header_list.length(); h++) {
    hl = _header_list[h];

    switch(hl->show) {
    case SMD_ALWAYS:
      previous_form = hl->form_widget;
      manageList[mli++] = hl->form_widget;
      // XtManageChild(hl->display_widget);
      break;

    case SMD_SHOWN:
      XtVaSetValues(hl->form_widget,
		    XmNtopAttachment, XmATTACH_WIDGET,
		    XmNtopWidget, previous_form,
		    NULL);
      previous_form = hl->form_widget;
      manageList[mli++] = hl->form_widget;
      //XtManageChild(hl->display_widget);
      break;

    default:
      unmanageList[uli++] = hl->form_widget;
      // XtUnmanageChild(hl->display_widget);
      break;
    }
  }
  XtManageChildren(manageList, mli);
  XtUnmanageChildren(unmanageList, uli);
  delete manageList;
  delete unmanageList;
}

void
SendMsgDialog::justifyHeaders(void)
{
  // Find out which header label has the longest display width to right
  // justify all labels.
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

SdmBoolean
SendMsgDialog::handleQuitDialog()
{
  char * value = NULL;
  SdmError error;
  SdmConnection *con = theRoamApp.connection()->connection();
  SdmMailRc * mail_rc;
  con->SdmMailRcFactory(error, mail_rc);
  assert(!error);

  mail_rc->GetValue(error, "expert", &value);
  if ((error == Sdm_EC_Success) && value != NULL) {
    free(value);
    return (Sdm_True); 
  }
  if (value) free(value);

  if ( _isQuitDialogUp == Sdm_True )
    return Sdm_False;

  _isQuitDialogUp = Sdm_True;

  DtMailGenDialog *dialog = this->genDialog();

  dialog->setToQuestionDialog(
			      catgets( DT_catd, 1, 99, "Mailer"),
			      catgets( DT_catd, 3, 58, "If you close this window\nyou will lose your edits."));
  if ( dialog->post_and_return(
			       catgets( DT_catd, 1, 118, "Close"),
			       catgets( DT_catd, 1, 101, "Cancel"),
			       DTMAILHELPCLOSECOMPOSEWINDOW) == 1 ) {
    // Close selected
    _isQuitDialogUp = Sdm_False;
    return(Sdm_True);
  }

  _isQuitDialogUp = Sdm_False;

  return(Sdm_False);	// Cancel selected
}

// Recycles Compose window.
// There are several ways we could have reached here.
// 1) From the user choosing Send.
// 2) From the user clicking on the Close button or Close menu item
// 3) The user choosing Close from window manager menu.

// For (1), we just forge ahead.  For that, the _takeDown boolean is set in
// send_message() method.  For (2), the boolean is set in goAway().  For
// (3), we call goAway() which sets the _takeDown depending on a dialog
// negotiation if SMD has contents.
void
SendMsgDialog::goAway(SdmBoolean checkForDirty)
{
  if (checkForDirty) {
    // Check to see if self has contents (ie., is dirty)
    if (checkDirty()) {
      if (isIconified())
	MainWindow::manage();
      // Enquire if the user really wants this window to go away
      if (!handleQuitDialog())
	return;
    }
  }

  if (_file_include->fileBrowser() != NULL) 
    XtPopdown(XtParent(_file_include->fileBrowser())); 
  if (_att_add->fileBrowser() != NULL) 
    XtPopdown(XtParent(_att_add->fileBrowser())); 
  if (_save_as_text->fileBrowser() != NULL) 
    XtPopdown(XtParent(_save_as_text->fileBrowser())); 
  if (_att_save->fileBrowser() != NULL) 
    XtPopdown(XtParent(_att_save->fileBrowser())); 

  stopAutoSave();

  theCompose._not_in_use++;
  reset();
  theCompose.fitIntoList(this, Sdm_False);
  _edits_made = Sdm_False;

#ifdef DTMAIL_TOOLTALK
  if ( started_by_tt && (theCompose._timeout_id == 0) &&
       (theCompose._not_in_use == theCompose._num_created) &&
       !dtmail_mapped ) {    //  For explanation of dtmail_mapped, look at RoamApp.h.
    theCompose._timeout_id = XtAppAddTimeOut( 
					     theApplication->appContext(), 
					     (unsigned long)DESTRUCT_TIMEOUT, 
					     Self_destruct, 
					     NULL);
  }
#endif
  // If user has quit the last Container window and he just quit (send or
  // close) the last Compose window, then terminate the application.
  if ( (theApplication->num_windows() == theCompose._not_in_use) && 
       (theCompose._timeout_id == 0) ) {
    // call shutdown below instead of exit(0) because the shutdown
    // method closes the outgoing message store.
    theRoamApp.shutdown();
  }

  char *buf_ptr = getTemplateFile();
  if (buf_ptr) {
    free(buf_ptr);
    setTemplateFile(NULL);
  }

  buf_ptr = getTemplateFullPath();
  if (buf_ptr) {
    free(buf_ptr);
    setTemplateFullPath(NULL);
  }

  buf_ptr = getDraftFile();
  if (buf_ptr) {
    free(buf_ptr);
    setDraftFile(NULL);
  }
  clearStatus();
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
Compose::fitIntoList(SendMsgDialog *smd, SdmBoolean is_new)
{

  Compose::Compose_Win* a_node = NULL; 

  if (!is_new) {// Window already in cache.  
    //Locate node and set in_use to Sdm_False
    
    for (a_node = theCompose._compose_head;
	 a_node;
	 a_node = a_node->next) {
	
      if (a_node->win == smd) {
	a_node->in_use = Sdm_False;
      }
    }
    return;
  }

  // Need new node with smd.  Set in_use to Sdm_True
  Compose::Compose_Win *tmp;

  tmp = (Compose::Compose_Win *)malloc(sizeof(Compose::Compose_Win));
  tmp->win = smd;
  tmp->next = NULL;
  tmp->in_use = Sdm_True;

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

  SdmBoolean set = Sdm_False;

  for (a_node = theCompose._compose_head;
       a_node && !set;
       a_node = a_node->next) {
	
    if (!a_node->next) {
      a_node->next = tmp;
      set = Sdm_True;
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
      a_node->in_use = Sdm_True;
      return(a_node->win);
    }
  }

  // All cached smds are in use.  
  return(NULL);
   
}

void
SendMsgDialog::resetHeaders(void)
{
  char * value = NULL;
  int i, j;
  SdmError error;
  SdmConnection *con = theRoamApp.connection()->connection();
  SdmMailRc * mail_rc;
  con->SdmMailRcFactory(error, mail_rc);
  assert(!error);

  mail_rc->GetValue(error, "additionalfields", &value);

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
SendMsgDialog::attachmentFeedback(SdmBoolean bool)
{
  if (bool)
    busyCursor();
  else
    normalCursor();
}

SdmBoolean
SendMsgDialog::hasAddressee()
{
  SdmError error;

  // Walk through the headers. 
  // Return Sdm_True if the message has a value for either of the
  // following headers: To:, Cc:, or Bcc:.
  // Return Sdm_False if none of the three headers have any value.

  for (int scan = 0; scan < _header_list.length(); scan++) {
    HeaderList * hl = _header_list[scan];
    if (hl->show != SMD_ALWAYS && hl->show != SMD_SHOWN) {
      continue;
    }

    if ((strcasecmp(hl->label, "To") == 0) ||
	(strcasecmp(hl->label, "Cc") == 0) || 
	(strcasecmp(hl->label, "Bcc") == 0)) {

      char* value = XmTextGetString(hl->field_widget);
      if (value) {
	for (char *cv = value; *cv; cv++) {
	  if (!isspace(*cv)) {
	    XtFree(value);
	    return(Sdm_True);	// text value contains contents
	  }
	}
	XtFree(value);		// text value is "content free" - try the next one
      }
    }
  }

  return(Sdm_False);		// no field has contents 
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

void
SendMsgDialog::updateHeaders(void)
{
  for (int scan = 0; scan < _header_list.length(); scan++) {
    HeaderList * hl = _header_list[scan];
    if (hl->show != SMD_ALWAYS && hl->show != SMD_SHOWN) 
      continue;

    char* value = XmTextGetString(hl->field_widget);

    if (hl->value) 
      free(hl->value);
    if (value && *value)
      hl->value = value;
    else {
      hl->value = NULL;
      if (value)
        XtFree(value);
    }

  }
}

