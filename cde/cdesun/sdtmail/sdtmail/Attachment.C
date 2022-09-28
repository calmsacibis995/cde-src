/*
 *+SNOTICE
 *
 *	$Revision: 1.9 $
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
 *	Copyright 1993, 1994, 1995 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)Attachment.C       1.142 01/28/97"
#endif

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <memory.h>
#include <errno.h>
#include <sys/utsname.h>
#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/BulletinB.h>
#include <Xm/AtomMgr.h>
#include <Xm/DragDrop.h>
#include <Xm/Screen.h>
#include <Xm/LabelG.h>
#include <X11/IntrinsicP.h>
#include <X11/Xatom.h>

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/MessageBody.hh>
#include <SDtMail/Session.hh>
#include <SDtMail/SystemUtility.hh>
#include <SDtMail/MessageBody.hh>
#include <SDtMail/Utility.hh>
#include <SDtMail/DataTypeUtility.hh>
#include "Attachment.h"
#include "Icon.h"
#include "RoamMenuWindow.h"
#include "SendMsgDialog.h"
#include "RoamApp.h"
#include "InfoDialogManager.h"
#include "ViewMsgDialog.h"
#include "RoamMenuWindow.h"
#include "MailMsg.h"
#include "MailSession.hh"
#include <Dt/Dts.h>
#include "MemUtils.hh"
#include "DtMailHelp.hh"

extern nl_catd	DtMailMsgCat;

static SdmString kEmptyString;

// Remove this after Nolan defines it in the libDtSvc library.
extern "C" {
    extern Boolean DtDtsIsTrue(char *);
}

#define equal(a, b) (!strcasecmp(a,b))

static void okcb(XtPointer);

Attachment::Attachment(
    AttachArea *classparent,
    SdmString name,
    SdmMessageBody *body_part,
    int indx
)
    : UIComponent (name), _myActionIds(5)
{
  _parent = classparent;

  _body_part = body_part;

  _myContents = kEmptyString;	
  _myContentsSize = 0;
  _cdeDataType = NULL;
  _approxDataType = NULL;

  _index = indx;
  _canKillSelf = Sdm_True;

  // I18N Decode the attachment name and concatenate 
  // the segments that are returned
  FSState fsstate = _parent->getFsState();
  SdmString tmpStr;
  /* Don't decode when adding the new Attchment via File Selection */
  if (fsstate != ADD) {
    SdmString charset;
    SdmError error;
    _body_part->GetCharacterSet(error, charset);
    SdmMessageEnvelope::DecodeHeader (error, tmpStr,
           (const char *)name, charset);
    
    if (error)
      tmpStr = name;
  } else {
    tmpStr = name;
    _parent->setFsState(NOTSET);
  }

  // I18N - end
  const char *label_name = (const char *)tmpStr;

  if(strchr(label_name, '/') == NULL) // The name does not include a slash
    _label = XmStringCreateLocalized((char*)label_name);
  else {			   // The name does include a slash
    char *s = (char *)(strrchr(label_name,'/'));
    if (s)
      s++;
    _label = XmStringCreateLocalized(s);
  }

  _myActionsList = NULL;
  _key = SdmUtility::NewObjectKey();
}

Attachment::~Attachment(
)
{
  SdmUtility::RemoveObjectKey(_key);

  if (_label)
    XmStringFree(_label);
  if (_cdeDataType)
    free (_cdeDataType);
  if (_approxDataType)
    free (_approxDataType);

  delete myIcon;
  if (_myActionsList) {
    for (int i=0; _myActionsList[i]; i++) {
      free (_myActionsList[i]);
    }
    delete []_myActionsList;
  }
}

void
Attachment::setAttachArea(AttachArea *aa)
{
    _parent = aa;
}

void
Attachment::initialize(SdmBoolean considerBandwidth)
{
  Widget widgetparent;

  assert( _parent != NULL);

  // revisit: esthert - should not get contents until we need it.
  // this->getContents();

  widgetparent = _parent->getClipWindow();

  _foreground = _parent->owner()->textEditor()->get_text_foreground();
  _background = _parent->owner()->textEditor()->get_text_background();

  _selected = Sdm_False;

  myIcon = new Icon(this, _name, _label, widgetparent, _index, considerBandwidth);

  _w = myIcon->baseWidget();

   _deleted = Sdm_False;
   XmUpdateDisplay(_w);

  {
    Arg args[2];
    int n;

    n = 0;
    XtSetArg(args[n], XmNwidth, &_attachmentWidth);  n++;
    XtSetArg(args[n], XmNheight, &_attachmentHeight);  n++;
    XtGetValues(_w, args, n);
  }

  installDestroyHandler();
}

void
Attachment::invokeAction(
    int index
)
{
    char	*actions_list = NULL;	//Comma separated list of actions
    char	*actionCommand;		// an Action 
    DtActionBuffer	bufArg;
    DtActionArg	*actionArg;
    SdmError	mail_error;

    if (_myActionsList == NULL) return;

    memset(&bufArg, 0, sizeof(bufArg));

    actionCommand = _myActionsList[index];
    
    // Get the contents for the attachment for this action
    // If an error is encountered, post an error dialog and return

    this->getContents(mail_error);
    if (mail_error != Sdm_EC_Success) {
      parent()->handlePostErrorDialog(mail_error, 
				      DTMAILHELPERROR, 
				      catgets(DT_catd, 2, 31, "Mailer encountered an error while processing the contents of the attachment.\n"));
      return;
    }

    if (actionCommand != NULL) {
      bufArg.bp = (void*)((const char*)_myContents);
      bufArg.size = (int) _myContentsSize;
      bufArg.name = _name;

      // Determine the type based on the contents.
      // This is to compensate for errors that other MUAs could have
      // generated:  some claim a bodyPart is rfc822 but deliver
      // something else.  

      char * type = SdmDataTypeUtility::SafeDtDtsBufferToDataType(
            (const char *)_myContents, 
            (const int)_myContentsSize, 
            _name);
      bufArg.type = type;
      

      if (_parent->isOwnerShellEditable()) {
          bufArg.writable = TRUE;
      }
      else {
          bufArg.writable = FALSE;
      }

      // Passing a buffer
      actionArg = (DtActionArg *) malloc(sizeof(DtActionArg) * 1);
      memset(actionArg, 0, sizeof(DtActionArg));

      actionArg->argClass = DtACTION_BUFFER;
      actionArg->u.buffer = bufArg;

      ActionCallback *acb = new ActionCallback(_key,  this);

      DtActionInvoke(
        _parent->ownerShellWidget(),
        actionCommand,
        actionArg,
        1,
        NULL,
        NULL,
        NULL,
        1,
        (DtActionCallbackProc)&Attachment::actionCallback,
        acb);	   
         
      // we can free the action argument after DtActionInvoke returns.
      free (actionArg);
    }
}

void
Attachment::handleDoubleClick()
{
    
  char	*actions_list = NULL;	//Comma separated list of actions
  char	*actionCommand = NULL;		// an Action 
  DtActionBuffer	bufArg;
  DtActionArg	*actionArg;
  SdmError	mail_error;

  _parent->attachmentFeedback(Sdm_True);

  memset(&bufArg, 0, sizeof(bufArg));

  // Resync _myContents with the BE Store for the bodypart.
  // Get the contents for the attachment for this action
  // If an error is encountered, post an error dialog and return

  this->getContents(mail_error);
  if (mail_error != Sdm_EC_Success) {
    parent()->handlePostErrorDialog(mail_error, 
				    DTMAILHELPERROR, 
				    catgets(DT_catd, 2, 31, "Mailer encountered an error while processing the contents of the attachment.\n"));
    parent()->attachmentFeedback(Sdm_False);
    return;
  }

  _cdeDataType = getDtType();

  // Retrieve the actions list
  actions_list = SdmDataTypeUtility::SafeDtDtsDataTypeToAttributeValue(
      _cdeDataType,
      DtDTS_DA_ACTION_LIST,
      NULL
     );

  if (actions_list != NULL)
    actionCommand = strtok(actions_list, ",");

  if (actionCommand != NULL) {
    bufArg.bp = (void*)((const char *)_myContents);
    bufArg.size = (int) _myContentsSize;
    bufArg.name = _name;

    // Determine the type based on the contents.
    // This is to compensate for errors that other MUAs could have
    // generated:  some claim a bodyPart is rfc822 but deliver
    // something else.  

    char * type = SdmDataTypeUtility::SafeDtDtsBufferToDataType(
          (const char *)_myContents, 
          (const int)_myContentsSize, 
          _name);
          
    bufArg.type = type;
    if (_parent->isOwnerShellEditable()) {
        bufArg.writable = TRUE;
    }
    else {
        bufArg.writable = FALSE;
    }

    // Do some due diligence now to see if the buffer is executable.
    // It either is executable or it is not.  If it is not, skip ahead to 
    // stuff the ActionArg and call DtActionInvoke.  If it is executable,
    // we clarify with the user whether they are really sure they want
    // to execute it.  We don't know the risk potential of running this
    // attachment.  Hopefully, the user knows!

    char *executable = NULL;
    executable = SdmDataTypeUtility::SafeDtDtsDataTypeToAttributeValue(
            type,
            DtDTS_DA_IS_EXECUTABLE,
            _name);

    if (executable) {
      Boolean is_executable = DtDtsIsTrue(executable);

      if (is_executable) {
        int answer;
        char buf[2048];
        char *helpId = DTMAILHELPEXECUTEOK;
        char *name;

        XmStringGetLtoR(_label, XmSTRING_DEFAULT_CHARSET, &name);
        sprintf(buf, catgets( DT_catd, 3, 93, 
          "The selected icon %s is an executable.\nClick OK to run it or Cancel to quit."), name);
	free(name);

        answer = parent()->handleQuestionDialog(
              catgets(DT_catd, 1, 86, "Mailer"),
              buf,
              helpId);

        if (answer == 2) {  // Cancel
            _parent->attachmentFeedback(Sdm_False);
            return;
        }
      }
    }


    // Passing a buffer
    actionArg = (DtActionArg *) malloc(sizeof(DtActionArg) * 1);
    memset(actionArg, 0, sizeof(DtActionArg));
    actionArg->argClass = DtACTION_BUFFER;
    actionArg->u.buffer = bufArg;

    ActionCallback *acb = new ActionCallback(_key, this);

    DtActionInvoke(	
      _parent->ownerShellWidget(),
      actionCommand,
      actionArg,
      1,
      NULL,
      NULL,
      NULL,
      1,
      (DtActionCallbackProc)&Attachment::actionCallback,
      acb);	  
      
    // we can free the action argument after DtActionInvoke returns.
    free (actionArg);
  }
	
  if (actions_list != NULL)
    SdmDataTypeUtility::SafeDtDtsFreeAttributeValue(actions_list);
    	
  _parent->attachmentFeedback(Sdm_False);
}

static void okcb( XtPointer )
{
    //Empty
    // This function exists so that the OK button will appear on the
    // Info Dialog. It doesn't have to do anything because the dialog
    // automatically pops down. It is for information only.
}

int
Attachment::operator==
(
const Attachment &
)
{
    return 1;
};

void
Attachment::set_selected()
{
  char *actionsString = NULL;	// Comma separated list of actions
  char *p = NULL;
  int numActions = 0;

  _selected = Sdm_True;

  parent()->attachmentSelected(this);

  if (_myActionsList) {
    for (int i=0; _myActionsList[i]; i++) {
      free (_myActionsList[i]);
    }
    delete [] _myActionsList;
    _myActionsList = NULL;
  }

  _cdeDataType = getDtType();

  // Retrieve the actions list.  Walk through the list and for each item
  // in it, ask the parent to create a menu item in its parent's menubar.
  actionsString = SdmDataTypeUtility::SafeDtDtsDataTypeToAttributeValue
    (_cdeDataType, DtDTS_DA_ACTION_LIST, NULL);

  // Count the number of commas in the action string
  for (p = actionsString; p && *p; p++)
    if (*p == ',')
      numActions++;

  if (actionsString != NULL)
    p = strtok(actionsString, ",");

  if (actionsString == NULL || p == NULL)
    return;

  numActions += 2; // Actual number of actions is one greater than number of
                   // commas.  Also need to add one for the NULL terminator.

  _myActionsList = new char*[numActions];
  _myActionsList[0] = strdup(p);

  for (int i = 1; p = strtok(NULL, ","); i++) {
    _myActionsList[i] = strdup(p);
  }
  _myActionsList[i] = NULL;  // Terminate the list

  parent()->addAttachmentActions(_myActionsList, i);

  XtFree(actionsString);
}

void
Attachment::unselect()
{
    _selected = Sdm_False;
    myIcon->unselect(); 
}

// Save the attachment to the specified file.
void
Attachment::saveToFile(SdmError &, char *filename)
{
    int answer;
    char buf[2048];
    char *helpId = NULL;
    SdmError mail_error;

    if (SdmSystemUtility::SafeAccess(filename, F_OK) == 0) {

      sprintf(buf, catgets(DT_catd, 3, 42, "%s already exists. Replace?"),
        filename);
        helpId = DTMAILHELPALREADYEXISTS;

      answer = parent()->handleQuestionDialog(catgets(DT_catd,
						1, 85,
						"Mailer"), 
						buf,
						helpId);

      if (answer == 2) { // Pressed cancel
          return;
      }

      if (SdmSystemUtility::SafeUnlink(filename) < 0) {
          sprintf(buf, catgets(DT_catd, 3, 43, "Unable to replace %s."), filename);
                helpId = DTMAILHELPNOREPLACE;
          answer = parent()->handleErrorDialog(catgets(DT_catd,
                  1, 86,
                  "Mailer"), 
                  buf,
                  helpId);
          return;
      }
    }

    // Get the contents for the attachment for this function.
    // If an error is encountered, post an error dialog and return

    this->getContents(mail_error);
    if (mail_error != Sdm_EC_Success) {
      parent()->handlePostErrorDialog(mail_error, 
				      DTMAILHELPERROR, 
				      catgets(DT_catd, 2, 31, "Mailer encountered an error while processing the contents of the attachment.\n"));
      return;
    }

    mode_t unix_mode;
    SdmMessageBodyStructure bpStruct;
    mail_error.Reset();
    // Preserve the file permission mode, if the mode is not set,
    // default to 0600 
    _body_part->GetStructure(mail_error, bpStruct);
    if (mail_error || (bpStruct.mbs_unix_mode == 0))
      unix_mode = 0666;	// allow umask to control final mode bit settings
    else
      unix_mode = bpStruct.mbs_unix_mode;

    // Create or truncate, and then write the bits.
    //
    int fd = SdmSystemUtility::SafeOpen(filename, O_RDWR | O_CREAT | O_TRUNC, unix_mode);
    if (fd < 0) {
      sprintf(buf, catgets(DT_catd, 3, 44, "Unable to create %s."), filename);
      helpId = DTMAILHELPNOCREATE;
      answer = parent()->handleErrorDialog(catgets(DT_catd, 1, 87, "Mailer"), 
							buf,
							helpId);
      return;
    }

    if (SdmSystemUtility::SafeWrite(fd, (const char*)_myContents, (unsigned int)_myContentsSize) < _myContentsSize) {
    	sprintf(buf, catgets(DT_catd, 3, 45, "Unable to create %s."), filename);
      helpId = DTMAILHELPNOCREATE;
      answer = parent()->handleErrorDialog(catgets(DT_catd, 1, 88, "Mailer"), 
							buf,
							helpId);
      SdmSystemUtility::SafeClose(fd);
      SdmSystemUtility::SafeUnlink(filename);
      return;
    }
    
    SdmSystemUtility::SafeClose(fd);

    // Stat the created file and see if it is of size 0.
    // If it is, engage the user in a dialog and involve a dtmail engineer
    // to attach a debugger to this process.

//     struct stat sbuf;
//     SdmSystemUtility::SafeStat(filename, &sbuf);
// 
//     if (sbuf.st_size == 0) {
// 	sprintf(buf, "Mailer produced a zero length file.\nPlease call a Dtmail engineer to attach a debugger\n to this process to begin debugging.  Only after attaching the debugger should you click OK.\n");
// 	answer = parent()->handleErrorDialog("BUG!", buf);
//     }

}

void
Attachment::setX(
    Position x
)
{
    Arg args[2];
    int n = 0;

    _positionX = x;

    n = 0;
    XtSetArg(args[n], XtNx, x);	n++;
    XtSetValues( _w, args,n);
}

void
Attachment::setY(
    Position y
)
{
    Arg args[2];
    int n = 0;

    _positionY = y;

    n = 0;
    XtSetArg(args[n], XtNy, y);	n++;
    XtSetValues( _w, args,n);
}

void 
Attachment::actionCallback(
  DtActionInvocationID id,
  XtPointer	clientData,
  DtActionArg	*action_arg,
  int		argCount,
  int		status	
)
{
    ActionCallback *acb = (ActionCallback *) clientData;
    if (SdmUtility::ValidObjectKey(acb->_myKey) == Sdm_False) {
      // Error out. Post an error?
      return;
    }
    else {
      acb->_myAttachment->action(id, action_arg, argCount, status);
      if (status == DtACTION_DONE) {
        delete acb;
      }
    }
}    

void
Attachment::action(
  DtActionInvocationID id,
  DtActionArg	*actionArg,
  int,		// argCount,
  int		status
)
{
  DtActionBuffer	bufArg;
  SdmError	mail_error;
  SdmString	tmpStr;
  int answer;
  char buf[2048];

  // Initialize the mail_error.
  mail_error.Reset();

  if (status == DtACTION_INVOKED) {
    registerAction(id);
    _parent->setPendingAction(Sdm_True);
  }
  else if (status == DtACTION_DONE) {
    unregisterAction(id);	
    _parent->setPendingAction(Sdm_False);

    // Check first if ownerShell is an SMD.
    // Secondly, check if the SMD is still available.  If the user had
    // sent the message while this attachment was up, the SMD's reset()
    // method would have set the attachments' _deleted flag to TRUE.
    // So, check to see that the attachment is still valid before setting
    // its contents.
    //
    // Note: actionArg can be NULL if there were no actions associated
    // with the data type.  Is this an Actions bug? Could be, but 
    // we check for it anyway.

    if (actionArg != NULL && _parent->isOwnerShellEditable() && !_deleted) {
        if (actionArg->argClass == DtACTION_BUFFER) {
          bufArg = actionArg->u.buffer;
	  // When doing action on COMPRESSed file Dt returns null buffer of length 0.
          if ( bufArg.writable && bufArg.size > 0 ) {
            // Assume user edited launched attachment -- as
            // optimization, we can compare buffers to see if
            // content actually changed.  For now, assume it changed.
            // Make a copy of incoming buffer and set body part's
            // contents, size. Let BE determine type.
            // Reset private variables.
            // note: we need to create the string using the size.
            // otherwise, if the buffer has valid null characters in
            // it (eg. audio file), it will not be copied to tmpStr below.
            tmpStr = SdmString((char *)bufArg.bp, bufArg.size);
            // I18N - changed api - SetContents now takes content buffer
            SdmContentBuffer content_buf(tmpStr);
            _body_part->SetContents(mail_error, content_buf);
            if (mail_error) {
              SdmUtility::LogError(Sdm_False, 
                "Mailer: Unable to process action, failed to set contents for attachment using buffer\n");
	      if (_cdeDataType) {
		free(_cdeDataType);
		_cdeDataType = NULL;
	      }
	      if (_approxDataType) {
		free(_approxDataType);
		_approxDataType = NULL;
	      }
	      if (_myContents != kEmptyString) {
		_myContents = kEmptyString;
		_myContentsSize = 0;
	      }

	      XtFree((char *)bufArg.bp);

              return;
            }

            // just continue if we can't set the content type for this attachment.
            SdmDataTypeUtility::SetContentTypesForMessage(mail_error, *_body_part);

            if (_cdeDataType) {
              free(_cdeDataType);
              _cdeDataType = NULL;
            }
            if (_approxDataType) {
              free(_approxDataType);
              _approxDataType = NULL;
            }
            if (_myContents != kEmptyString) {
              _myContents = kEmptyString;
              _myContentsSize = 0;
            }
          }
          // Free the buffer...
          XtFree((char *)bufArg.bp);
        }
        else {
          // DtACTION_FILE
          // Read the file into a buffer and do the same stuff
          // as above.

          int tmp_file;
          DtActionFile fileArg = actionArg->u.file;
          struct stat stat_buf;

          if ( SdmSystemUtility::SafeStat ( fileArg.name, &stat_buf ) == -1 ) {
              SdmUtility::LogError(Sdm_False, 
                "Mailer: Unable to process action, stat failed on file %s.\n", fileArg.name);
              return;
          }
          if ((tmp_file = SdmSystemUtility::SafeOpen(fileArg.name, O_RDONLY)) == -1) {
              SdmUtility::LogError(Sdm_False, 
                "Mailer: Unable to process action, open failed on file %s.\n", fileArg.name);
              return;
	  }
          // I18N - changed api - SetContents now takes a content buffer
          SdmContentBuffer content_buf;
	  if (content_buf.MapAgainstFile(mail_error, tmp_file, stat_buf.st_size) != Sdm_EC_Success) {
	    if (mail_error == Sdm_EC_NoMemory)
              SdmUtility::LogError(Sdm_False, 
                "Mailer: Unable to process action, not enough memory to access data in %s.\n", fileArg.name);
	    else
              SdmUtility::LogError(Sdm_False, 
                "Mailer: Unable to process action, could not access data in %s.\n", fileArg.name);
	    SdmSystemUtility::SafeClose(tmp_file);
	    return;
	  }
	  SdmSystemUtility::SafeClose(tmp_file);
          _body_part->SetContents(mail_error, content_buf);
          if (mail_error) {
            SdmUtility::LogError(Sdm_False, 
              "Mailer: Unable to process action, failed to set contents for attachment using file %s\n", fileArg.name);
            return;
          }

          // just continue if we there's an error in finding the type for the content.
          SdmDataTypeUtility::SetContentTypesForMessage(mail_error, *_body_part);

          if (_cdeDataType) {
            free(_cdeDataType);
            _cdeDataType = NULL;
          }
          if (_approxDataType) {
            free(_approxDataType);
            _approxDataType = NULL;
          }
          if (_myContents != kEmptyString) {
            _myContents = kEmptyString;
            _myContentsSize = 0;
          }
      }
    }
  }
  else if (status == DtACTION_INVALID_ID) {
    /* NL_COMMENT
     * Post a dialog explaining that the action was invalid
     */
    sprintf(buf, 
      catgets(DT_catd, 3, 91, "Cannot execute invalid action."));

    answer = parent()->handleErrorDialog(catgets(DT_catd, 1, 86, "Mailer"),
                 buf);

    unregisterAction(id);	
    _parent->setPendingAction(Sdm_False);
  }
  else if (status == DtACTION_FAILED) {

    /* NL_COMMENT 
     * Post a dialog explaining that the action failed.
     */
    sprintf(buf, 
      catgets(DT_catd, 3, 92, "Executing action failed!"));

    answer = parent()->handleErrorDialog(catgets(DT_catd, 1, 86, "Mailer"),
                 buf);

    unregisterAction(id);	
    _parent->setPendingAction(Sdm_False);
  }
  else if (status == DtACTION_STATUS_UPDATE) {
    // Check if ownerShell is an SMD.  Make sure the message has not
    // been sent before attempting to update things.

    if(actionArg != NULL && _parent->isOwnerShellEditable() && !_deleted) {
        SendMsgDialog *smd = _parent->getSMDOwnerShell();
        smd->setEditsMade(Sdm_True);
        if (actionArg->argClass == DtACTION_BUFFER) {
          bufArg = actionArg->u.buffer;
          if (bufArg.writable) {
              // Assume user edited launched attachment -- as
              // optimization, we can compare buffers to see if
              // content actually changed.  For now, assume it changed.
              // Make a copy of incoming buffer and set body part's
              // contents, size. Let BE determine type.
              // Reset private variables.
              // note: we need to create the string using the size.
              // otherwise, if the buffer has valid null characters in
              // it (eg. audio file), it will not be copied to tmpStr below.
              tmpStr = SdmString((char *)bufArg.bp, bufArg.size);
              SdmContentBuffer content_buf(tmpStr);
              _body_part->SetContents(mail_error, content_buf);
              if (mail_error) {
                SdmUtility::LogError(Sdm_False, 
                  "Mailer: Unable to process action, failed to set contents for attachment using buffer\n");
                return;
              }

              // just continue if we're not able to find the type of the attachment.
              SdmDataTypeUtility::SetContentTypesForMessage(mail_error, *_body_part);

              if (_cdeDataType) {
                free(_cdeDataType);
                _cdeDataType = NULL;
              }
              if (_approxDataType) {
                free(_approxDataType);
                _approxDataType = NULL;
              }
              if (_myContents != kEmptyString) {
                _myContents = kEmptyString;
                _myContentsSize = 0;
              }

        }
        // Free the buffer 
        XtFree((char *)bufArg.bp);
      }
      else {
        // DtACTION_FILE
        // Read the file into a buffer and do the same stuff
        // as above.

        int tmp_file;
        DtActionFile fileArg = actionArg->u.file;
        struct stat stat_buf;

        if ( SdmSystemUtility::SafeStat ( fileArg.name, &stat_buf ) == -1 ) {
            SdmUtility::LogError(Sdm_False, 
              "Mailer: Unable to process action, stat failed on file %s.\n", 
              fileArg.name);
            return;
        }
        if ((tmp_file = SdmSystemUtility::SafeOpen(fileArg.name, O_RDONLY)) == -1) {
          SdmUtility::LogError(Sdm_False, 
            "Mailer: Unable to process action, cannot open attachment using file %s\n", fileArg.name);
	  return;
	}
        // I18N - changed api - SetContents now takes a content buffer
        SdmContentBuffer content_buf;
	if (content_buf.MapAgainstFile(mail_error, tmp_file, stat_buf.st_size) != Sdm_EC_Success) {
	  if (mail_error == Sdm_EC_NoMemory)
          SdmUtility::LogError(Sdm_False, 
            "Mailer: Unable to process action, not enough memory to get contents for attachment using file %s\n", fileArg.name);
	  else
	    SdmUtility::LogError(Sdm_False, 
				 "Mailer: Unable to process action, failed to get contents for attachment using file %s\n", fileArg.name);
	  SdmSystemUtility::SafeClose(tmp_file);
	  return;
	}
	SdmSystemUtility::SafeClose(tmp_file);

        _body_part->SetContents(mail_error, content_buf);
        if (mail_error) {
          SdmUtility::LogError(Sdm_False, 
            "Mailer: Unable to process action, failed to set contents for attachment using file %s\n", fileArg.name);
          return;
        }

        // just continue if we there's an error in finding the type for the content.
        SdmDataTypeUtility::SetContentTypesForMessage(mail_error, *_body_part);

        if (_cdeDataType) {
            free(_cdeDataType);
            _cdeDataType = NULL;
        }
        if (_approxDataType) {
            free(_approxDataType);
            _approxDataType = NULL;
        }
        if (_myContents != kEmptyString) {
          _myContents = kEmptyString;
          _myContentsSize = 0;
        }
      }
    }
  }
  else if (status == DtACTION_CANCELED) {
    unregisterAction(id);	
    _parent->setPendingAction(Sdm_False);

    if (actionArg != NULL) {
      XtFree((char *)(actionArg->u.buffer.bp));
    }
  }
}

void
Attachment::setBatch(int deleteBatch)
{
    _deletedBatch = deleteBatch;
}

void
Attachment::deleteIt(SdmError &mail_error)
{
    SdmBoolean prev;
    SdmMessage *parent;
    
    // Initialize the mail_error.
    mail_error.Reset();

    // Get the BE to mark the bodyPart as deleted
    _body_part->Parent(mail_error, parent);

    if ( !mail_error && parent )
	_body_part->DeleteMessageBodyFromMessage(mail_error, prev, Sdm_True);

    _deleted = Sdm_True;
}

void
Attachment::undeleteIt(SdmError &mail_error)
{
    SdmBoolean prev;

    // Initialize the mail_error.
    mail_error.Reset();

    // Get the BE to mark the bodyPart as not deleted.
    _body_part->DeleteMessageBodyFromMessage(mail_error, prev, Sdm_False);
    if (!mail_error) {
      _deleted = Sdm_False;
    }
}

void
Attachment::registerAction(
    DtActionInvocationID id
)
{
    _canKillSelf = Sdm_False;
    _myActionIds.append(id);
}	

void
Attachment::unregisterAction(
    DtActionInvocationID id
)
{
    if (_myActionIds.length() == 0) {
	// error.  Choke!
    }
    else {
      _myActionIds.remove(id);
      if (_canKillSelf) {
          // See copious documentation above.
          delete this;
      }
    }
}

void
Attachment::quit()
{
    _canKillSelf = Sdm_True;
    if (_myActionIds.length() == 0) {
      delete this;
    }
}

// ActionCallback

ActionCallback::ActionCallback(
    DtMailObjectKey key,
    Attachment *att
)
{
    _myKey = key;
    _myAttachment = att;
}

ActionCallback::~ActionCallback()
{
}

void
Attachment::unmanageIconWidget(void)
{
    XtUnmanageChild(_w);
}

void
Attachment::manageIconWidget(void)
{
    XtManageChild(_w);
}

void
Attachment::setLabel(XmString str)
{
  Arg args[2];
  int n;

  if (_label) 
    XmStringFree(_label);

  _label = XmStringCopy(str);

  n = 0;
  XtSetArg(args[n], XmNstring, _label); n++;
  XtSetValues(_w, args, n);
}

void
Attachment::primitive_select()
{
  _selected = Sdm_True;
  myIcon->primitiveSelect();
}

void
Attachment::rename(
    XmString new_name
)
{
    char *name;
    SdmError mail_error;
    SdmString tmpStr;
    SdmMessageBodyStructure bpStruct;

    mail_error.Reset();

    XmStringGetLtoR(new_name, XmSTRING_DEFAULT_CHARSET, &name);

    tmpStr = (char *)name;
    XtFree(name);
    _body_part->GetStructure(mail_error, bpStruct);
    bpStruct.mbs_attachment_name = tmpStr;

    _body_part->SetStructure(mail_error, bpStruct);

    if (_cdeDataType) {
      free(_cdeDataType);
      _cdeDataType = NULL;
    }

    if (_approxDataType) {
      free(_approxDataType);
      _approxDataType = NULL;
    }

    this->setLabel(new_name);
}

char *
Attachment::getDtType(SdmBoolean approximate)
{
  SdmMessageBody *bp;
  SdmError mail_error;
  SdmString dataType;
  char *type = NULL;

  // If the full CDE data type is already cached, just return it.
  if (_cdeDataType)
    return _cdeDataType;

  // Otherwise, if an approximate type is cached and requested, return it.
  if (_approxDataType && approximate == Sdm_True)
    return _approxDataType;

  // Get the message body for this attachment.
  bp = getBodyPart();
  if (bp != NULL) {
    // revisit: esthert - call DetermineCdeDataTypeForMessage with additional argument oncde
    // fast/slow network support is available in sdtmail.
    SdmDataTypeUtility::DetermineCdeDataTypeForMessage(mail_error, dataType,
                                                       *bp, NULL, approximate);
    if (dataType.Length() > 0) {
      type = strdup((const char *)dataType);
    }
  }

  if (!type)
    type = strdup("UNKNOWN");

  // cache the type appropriately
  if (approximate)
    _approxDataType = type;
  else
    _cdeDataType = type;

  return type;
}

void
Attachment::setContents(SdmError& mail_error)
{
  SdmString tmpStr;
  SdmContentBuffer tmpBuf;
  SdmMessageBody	*bp;

  // Initialize the mail_error.
  mail_error.Reset();

  // Initialize contents and content size - right now tmpStr is a null
  // (empty) string as that is the default for the SdmString object
  // upon creation
  _myContents = tmpStr;
  _myContentsSize = tmpStr.Length();

  // esthert - this should be the only place that GetContents is called for the body.
  bp = this->getBodyPart();
  bp->GetContents(mail_error, tmpBuf);
  if (mail_error) return;

  tmpBuf.GetContents(mail_error, tmpStr);
  if (mail_error) return;

  // I18N
  SdmMessageBodyStructure mbstruct;
  bp->GetStructure(mail_error, mbstruct);
  if (mail_error) return;

  if (mbstruct.mbs_mime_content_type == "text/plain") {
    char *newData;
    int newLength;
    SdmString charset;

    // Get the user chosen charset, if there isn't one, get the one
    // specified by the MIME charset parameter
    tmpBuf.GetCharacterSet(mail_error, charset);
    if (charset.Length() == 0)
      tmpBuf.GetDataTypeCharSet(mail_error, charset);

    // Performance Optimization: if the incoming charset is us-ascii, then there
    // is no need to do a conversion

    if (!charset.Length() || (strcasecmp((const char *)charset, "us-ascii")!=0)) {
      SdmContentBuffer::DecodeCharset (mail_error, newData, newLength,
				       (const char *)tmpStr, tmpStr.Length(), charset);
      if (!mail_error) {
	// Note: we shouldn't null terminate the data because it is not
	// necessarily a string.
	tmpStr = SdmString(newData, newLength);
	free (newData);
      }
    }
    mail_error = Sdm_EC_Success;
  }
  // I18N end

  _myContents = tmpStr;
  _myContentsSize = tmpStr.Length();
  _cdeDataType = getDtType();

  // If it is a rfc822 message, check if it has From stuffing.
  // From stuffing is ">From".
  // If it has, advance the pointer to step past the ">" character.

  if (_cdeDataType && !strcmp(_cdeDataType,"DTMAIL_FILE")) {

    // If it has a From header, return.  The classing engine uses
    // that to determine the client that needs to be launched.

    if (!strncasecmp((const char *)_myContents, "From", 4)) {
      return;
    }

    // Message doesn't begin with From. It may have From stuffing --
    // ">From".  Or it may have no From header at all. The MIME 
    // specs are vague on what headers an Message/RFC822 body part
    // has to have.  We need From to help the classing engine and
    // therefore, we will by force (or hack) make it have a From :-)

    if (!strncasecmp((const char *)_myContents, ">From", 5)) {

      // Has From stuffing.

      const char *ptr = (const char *)_myContents;
      ptr++;			    // Step past the first char
      _myContentsSize--;
      // need to create temporary string with new content.
      SdmString tmpStr2(ptr, _myContentsSize);  
      _myContents = tmpStr2;  // Reset contents
    }

    else { // No From header.  See if it has any headers.

      // We do this by checking if there is a colon present in the
      // first line.  If we succeed at finding one, we presume the
      // line is an RFC header line.  And we introduce a new From
      // header line before the other headers.

      const char *sptr = NULL;
      sptr = (const char *)_myContents;

      for (; *sptr && *sptr != '\n'; sptr++) {
        if (*sptr == ':') {
	  // Found a colon before hitting a newline.
	  // Can assume that this is an RFC header line.
	  break;
        }
        else {
	  continue;
        }
      }

      if (*sptr== ':') { 

        // The attachment has headers.
        // Introduce a From header.  Maintain others.

        char *str = "From UNKNOWN Mon Jan  1 00:00:00 1996\n";
        char *buffer = new char[strlen(str) + _myContentsSize + 1];
        strcpy(buffer, str);
        strncat(buffer, (const char *)_myContents, (unsigned int)_myContentsSize);
        _myContentsSize += strlen(str);
        _myContents = SdmString(buffer, _myContentsSize);
      }
      else {  
        // Doesn't contain any header.  Introduce From header.
        // Throw an extra newline to make rest of contents a
        // the attachment body.

        char *str = "From UNKNOWN Mon Jan  1 00:00:00 1996\n\n";
        char *buffer = new char[strlen(str) + _myContentsSize + 1];
        strcpy(buffer, str);
        strncat(buffer, (const char *)_myContents, (unsigned int)_myContentsSize);
        _myContentsSize += strlen(str);
        _myContents = SdmString(buffer, _myContentsSize);
      }
    }
  }
}

void *
Attachment::getContents(SdmError& mail_error)
{
  mail_error = Sdm_EC_Success;

  if (_myContents == kEmptyString) {
    this->setContents(mail_error);
    if (mail_error != Sdm_EC_Success) {
      // error during the get - force the contents to be the empty
      // string so that repeated requests to process this attachment
      // will result in repeated attempts to get the contents.
      _myContents = kEmptyString;
      _myContentsSize = 0;
    }
  }

  return ( (void*)((const char *)_myContents) );
}

SdmMsgStrType
Attachment::getType()
{
  SdmError mail_error;
  SdmMessageBodyStructure bpStruct;
  _body_part->GetStructure(mail_error, bpStruct);
  if (mail_error) {
    return Sdm_MSTYPE_none;
  } else {
    return (bpStruct.mbs_type);
  }
}

SdmString 			
Attachment::getSubType()
{
  SdmError mail_error;
  SdmMessageBodyStructure bpStruct;
  _body_part->GetStructure(mail_error, bpStruct);
  if (mail_error) {
    SdmString tmp;
    return tmp;
  } else {
    return (bpStruct.mbs_subtype);
  }
}

