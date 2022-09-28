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
#pragma ident "@(#)Attachment.C	1.88 03/04/96"
#endif

#include <EUSCompat.h>   // For strcasecmp()
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
#include <DtMail/DtMailError.hh>
#include <DtMail/IO.hh>			// SafeAccess...
#include "MemUtils.hh"
#include "DtMailHelp.hh"

extern nl_catd	DtMailMsgCat;

// Remove this after Nolan defines it in the libDtSvc library.
extern "C" {
    extern Boolean DtDtsIsTrue(char *);
}

unsigned char validbits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x07, 0x00,
   0x00, 0xfc, 0x3f, 0x00, 0x00, 0xfe, 0x7f, 0x00, 0x00, 0x1f, 0xf8, 0x00,
   0x80, 0x07, 0xe0, 0x01, 0x80, 0x03, 0xc0, 0x01, 0xc0, 0x01, 0x80, 0x03,
   0xc0, 0x01, 0x80, 0x03, 0xe0, 0x00, 0x00, 0x07, 0xe0, 0x80, 0x01, 0x07,
   0xe0, 0xc0, 0x03, 0x07, 0xe0, 0xc0, 0x03, 0x07, 0xe0, 0x80, 0x01, 0x07,
   0xe0, 0x00, 0x00, 0x07, 0xc0, 0x01, 0x80, 0x03, 0xc0, 0x01, 0x80, 0x03,
   0x80, 0x03, 0xc0, 0x01, 0x80, 0x07, 0xe0, 0x01, 0x00, 0x1f, 0xf8, 0x00,
   0x00, 0xfe, 0x7f, 0x00, 0x00, 0xfc, 0x3f, 0x00, 0x00, 0xf0, 0x0f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

unsigned char invalidbits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x07, 0x00,
   0x00, 0xfc, 0x3f, 0x00, 0x00, 0xfe, 0x7f, 0x00, 0x00, 0x1f, 0xf8, 0x00,
   0x80, 0x0f, 0xe0, 0x01, 0x80, 0x1f, 0xc0, 0x01, 0xc0, 0x3f, 0x80, 0x03,
   0xc0, 0x7d, 0x80, 0x03, 0xe0, 0xf8, 0x00, 0x07, 0xe0, 0xf0, 0x01, 0x07,
   0xe0, 0xe0, 0x03, 0x07, 0xe0, 0xc0, 0x07, 0x07, 0xe0, 0x80, 0x0f, 0x07,
   0xe0, 0x00, 0x1f, 0x07, 0xc0, 0x01, 0xbe, 0x03, 0xc0, 0x01, 0xfc, 0x03,
   0x80, 0x03, 0xf8, 0x01, 0x80, 0x07, 0xf0, 0x01, 0x00, 0x1f, 0xf8, 0x00,
   0x00, 0xfe, 0x7f, 0x00, 0x00, 0xfc, 0x3f, 0x00, 0x00, 0xf0, 0x0f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define equal(a, b) (!strcasecmp(a,b))

String typeName;

#ifdef DEAD_WOOD
void runit_callback( int *data );
void norunit_callback( int *data );
#endif /* DEAD_WOOD */
static void okcb(XtPointer);

Attachment::Attachment(
    AttachArea *classparent,
    String name,
    DtMail::BodyPart *body_part,
    int indx
)
    : UIComponent (
	name
), _myActionIds(5)
{

    _parent = classparent;

    _body_part = body_part;

    _haveContents = FALSE;
    _myContents = NULL;
    _myContentsSize = 0;
    _myType = NULL;

    _index = indx;
    _canKillSelf = TRUE;

    _subtype = NULL;

    if(strchr(name, '/') == NULL) // The name does not include a slash
	_label = XmStringCreateLocalized(name);
    else			   // The name does include a slash
	_label = XmStringCreateLocalized(strrchr(name, '/')+1);

    _myActionsList = NULL;
    _key = theRoamApp.session()->session()->newObjectKey();
}

Attachment::~Attachment(
)
{
    
    theRoamApp.session()->session()->removeObjectKey(_key);

    if (_label)
      XmStringFree(_label);
    if (_subtype)
      free (_subtype);
    if (_myType)
      free (_myType);
    
    delete myIcon;
    if (_myActionsList)
	delete []_myActionsList;
}


void
Attachment::setAttachArea(AttachArea *aa)
{
    _parent = aa;
}
    

void
Attachment::initialize()
{
    Widget widgetparent;

    assert( _parent != NULL);

    this->setContents();

    widgetparent = _parent->getClipWindow();

    _foreground = _parent->owner()->textEditor()->get_text_foreground();
    _background = _parent->owner()->textEditor()->get_text_background();

    _selected = FALSE;

    // We already have the name from the classing engine.  Now map
    // the name to the MIME type and subtype
    name_to_type();

    myIcon = new Icon(this, _name, _label, _type, widgetparent, _index);
    
    _w = myIcon->baseWidget();

     _deleted = FALSE;
     XmUpdateDisplay(_w);

    {
    Arg args[2];
    int n;

    n = 0;
    XtSetArg(args[n], XmNwidth, &_attachmentWidth);  n++;
    XtSetArg(args[n], XmNwidth, &_attachmentHeight);  n++;
    XtGetValues(_w, args, n);
    }
    
    installDestroyHandler();
}

#ifdef DEAD_WOOD
Boolean
Attachment::check_if_binary(String contents, unsigned long size)
{
    int i;

    for(i=0;i<size;i++) {
	if((!isprint(contents[i])) && (!isspace(contents[i])))
	    return True;
    }
    return False;
}
#endif /* DEAD_WOOD */

//
// Map the name (which we got from the classing engine) to a 
// type and subtype
//

void
Attachment::name_to_type()
{
    char subtype[128];

    // Hack hack!
    // Hardcode ce_name to be text for now.
    // It should actually be determined dynamically from the type of
    // the attachment.

    _ce_name = "text";

    // If the type and subtype are already set then we don't need to
    // map a classing engine type to a MIME type; We already have
    // the MIME type, so just return
    if(_subtype != NULL)
	return;
    if(equal(_ce_name, "text")) {			// text
	_type = TYPETEXT;
	_subtype = strdup("plain");
    } else if(equal(_ce_name, "richtext")) {		// richtext
	_type = TYPETEXT;
	sprintf(subtype, "X-sun-%s",_ce_name);
	_subtype = strdup(subtype);
    } else if(equal(_ce_name, "audio-file")) {		// audio-file
	_type = TYPEAUDIO;
	_subtype = strdup("basic");
    } else if(equal(_ce_name, "default")) {		// default
	_type = TYPEAPPLICATION;
	sprintf(subtype, "X-sun-%s",_ce_name);
	_subtype = strdup(subtype);
    } else if(equal(_ce_name, "oda")) {			// oda
	_type = TYPEAPPLICATION;
	_subtype = strdup("oda");
    } else if(equal(_ce_name, "postscript-file")) {	// postscript
	_type = TYPEAPPLICATION;
	_subtype = strdup("PostScript");
    } else if(equal(_ce_name, "sun-raster")) {		// sun-raster
	_type = TYPEIMAGE;
	sprintf(subtype, "X-%s",_ce_name);
	_subtype = strdup(subtype);
    } else if(equal(_ce_name, "jpeg-file")) {		// jpeg-file
	_type = TYPEIMAGE;
	_subtype = strdup("jpeg");
    } else if(equal(_ce_name, "g3-file")) {		// g3-file
	_type = TYPEIMAGE;
	sprintf(subtype, "X-sun-%s",_ce_name);
	_subtype = strdup(subtype);
    } else if(equal(_ce_name, "gif-file")) {		// gif-file
	_type = TYPEIMAGE;
	_subtype = strdup("gif");
    } else if(equal(_ce_name, "pbm-file")) {		// pbm-file
	_type = TYPEIMAGE;
	sprintf(subtype, "X-sun-%s",_ce_name);
	_subtype = strdup(subtype);
    } else if(equal(_ce_name, "pgm-file")) {		// pgm-file
	_type = TYPEIMAGE;
	sprintf(subtype, "X-sun-%s",_ce_name);
	_subtype = strdup(subtype);
    } else if(equal(_ce_name, "ppm-file")) {		// ppm-file
	_type = TYPEIMAGE;
	sprintf(subtype, "X-sun-%s",_ce_name);
	_subtype = strdup(subtype);
    } else if(equal(_ce_name, "xpm-file")) {		// xpm-file
	_type = TYPEIMAGE;
	sprintf(subtype, "X-sun-%s",_ce_name);
	_subtype = strdup(subtype);
    } else if(equal(_ce_name, "tiff-file")) {		// tiff-file
	_type = TYPEIMAGE;
	sprintf(subtype, "X-sun-%s",_ce_name);
	_subtype = strdup(subtype);
    } else if(equal(_ce_name, "troff")) {		// troff
	_type = TYPETEXT;
	sprintf(subtype, "X-sun-%s",_ce_name);
	_subtype = strdup(subtype);
    } else if(equal(_ce_name, "nroff")) {		// nroff
	_type = TYPETEXT;
	sprintf(subtype, "X-sun-%s",_ce_name);
	_subtype = strdup(subtype);
    } else if(equal(_ce_name, "h-file")) {		// h-file
	_type = TYPETEXT;
	sprintf(subtype, "X-sun-%s",_ce_name);
	_subtype = strdup(subtype);
    } else if(equal(_ce_name, "c-file")) {		// c-file
	_type = TYPETEXT;
	sprintf(subtype, "X-sun-%s",_ce_name);
	_subtype = strdup(subtype);
    } else if(equal(_ce_name, "makefile")) {		// makefile
	_type = TYPETEXT;
	sprintf(subtype, "X-sun-%s",_ce_name);
	_subtype = strdup(subtype);
    } else if(equal(_ce_name, "mail-file")) {		// mail-file
	_type = TYPETEXT;
	sprintf(subtype, "X-sun-%s",_ce_name);
	_subtype = strdup(subtype);
    } else if(equal(_ce_name, "mail-message")) {	// mail-message
	_type = TYPETEXT;
	sprintf(subtype, "X-sun-%s",_ce_name);
	_subtype = strdup(subtype);
    } else {
	_type = TYPEAPPLICATION;
	sprintf(subtype, "X-sun-%s",_ce_name);
	_subtype = strdup(subtype);
    }
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

    if (_myActionsList == NULL) return;

    memset(&bufArg, 0, sizeof(bufArg));

    actionCommand = _myActionsList[index];
    
    this->setContents();

#ifdef undef
    // This text will eventually be included in a dialog, but
    // the dialog isn't implemented yet, so we have it here as
    // a placeholder for the message catalog.

    catgets(DT_catd, 1, 84, "%s is an executable attachment. Do you want to run it?");
#endif // undef

    if (actionCommand != NULL) {
	bufArg.bp = (void *)_myContents;
	bufArg.size = (int) _myContentsSize;

	// Determine the type based on the contents.
	// This is to compensate for errors that other MUAs could have
	// generated:  some claim a bodyPart is rfc822 but deliver
	// something else.  

	char * type = DtDtsBufferToDataType(
				(char *)_myContents, 
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

	actionArg->argClass = 2;
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
    }
}

void
Attachment::handleDoubleClick()
{
    
    char	*actions_list = NULL;	//Comma separated list of actions
    char	*actionCommand = NULL;		// an Action 
    DtActionBuffer	bufArg;
    DtActionArg	*actionArg;

    _parent->attachmentFeedback(TRUE);

    memset(&bufArg, 0, sizeof(bufArg));

    // Resync _myContents with the BE Store for the bodypart.

    this->setContents();

    // Retrieve the actions list
    actions_list = DtDtsDataTypeToAttributeValue(
				_myType,
				DtDTS_DA_ACTION_LIST,
				NULL
		   );

    if (actions_list != NULL)
    	actionCommand = strtok(actions_list, ",");

    if (actionCommand != NULL) {
	bufArg.bp = (void *)_myContents;
	bufArg.size = (int) _myContentsSize;
	
	// Determine the type based on the contents.
	// This is to compensate for errors that other MUAs could have
	// generated:  some claim a bodyPart is rfc822 but deliver
	// something else.  

	char * type = DtDtsBufferToDataType(
				(char *)_myContents, 
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
	executable = DtDtsDataTypeToAttributeValue(
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

#ifdef NotDefined
		sprintf(buf, 
		    catgets(
			DT_catd, 3, 81, "This attachment may contain commands that can cause serious\n\damage.  It is recommended that you only execute it after you\nare certain it is safe to do so.  Press OK if you are certain\nit is safe, Cancel to cancel execution."));
#endif
		answer = parent()->handleQuestionDialog(
					catgets(DT_catd, 1, 86, "Mailer"),
					buf,
					helpId);

		if (answer == 2) {  // Cancel
		    _parent->attachmentFeedback(FALSE);
		    return;
		}
	    }
	}

	

	// Passing a buffer

	actionArg = (DtActionArg *) malloc(sizeof(DtActionArg) * 1);
	memset(actionArg, 0, sizeof(DtActionArg));
	actionArg->argClass = 2;
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
    }
	
    if (actions_list != NULL)
    	DtDtsFreeAttributeValue(actions_list);
    _parent->attachmentFeedback(FALSE);

}

static void okcb( XtPointer )
{
    //Empty
    // This function exists so that the OK button will appear on the
    // Info Dialog. It doesn't have to do anything because the dialog
    // automatically pops down. It is for information only.
}

#ifdef DEAD_WOOD
void
runit_callback(int *data)
{
    *data=1;
}

void
norunit_callback(int *data)
{
    *data=2;
}
#endif /* DEAD_WOOD */

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
    char	*actions_list = NULL;	//Comma separated list of actions
    char*	anAction=NULL;
    int		numActions = 0;

    _selected = TRUE;

    parent()->attachmentSelected(this);

    // Crude Hack.
    // Assuming that only 10 actions are possible.
    // Need a mechanism to query, determine how many possible and
    // allocate memory for that many.

    _myActionsList = new char*[10];

    // Retrieve the actions list.  Walk through the list and
    // for each item in it, ask the parent to create a menu item
    // in its parent's menubar.

    actions_list = DtDtsDataTypeToAttributeValue(
				_myType,
				DtDTS_DA_ACTION_LIST,
				NULL
		   );

    char **tmpActionCommand = _myActionsList;

    if (actions_list != NULL)
    	anAction = strtok(actions_list, ",");
    if (anAction == NULL) {
	return;
    }

    *tmpActionCommand = strdup(anAction);

    while (*tmpActionCommand != NULL) {
	
	// strtok() requires that calls other than the first have NULL as
	// the first arg..

	tmpActionCommand++;
	numActions++;

	anAction = strtok(NULL, ",");
	if (anAction == NULL) {
	    *tmpActionCommand = NULL;
	}
	else {
	    *tmpActionCommand = strdup(anAction);
	}
    }
    parent()->addAttachmentActions(
			_myActionsList,
			numActions
		);

    XtFree(actions_list);
}

void
Attachment::unselect()
{
    _selected = FALSE;
    myIcon->unselect(); 
}

// Save the attachment to the specified file.
void
Attachment::saveToFile(DtMailEnv &, char *filename)
{
    int answer;
    char buf[2048];
    char *helpId = NULL;

    if (SafeAccess(filename, F_OK) == 0) {

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

	if (unlink(filename) < 0) {
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


//     if (_myContentsSize == 0) {// Oops! BE thinks attachment is of size 0.
// 	sprintf(buf, "Mailer BE thinks attachment is size 0.\nPlease call a Dtmail engineer to attach a debugger\n to this process to begin debugging.  Only after attaching the debugger should you click OK.\n");
// 	answer = parent()->handleErrorDialog("BUG!", buf);
// 	
// 	// Call setContents again.  
// 	// This will help us debug why the body part has bogus contents.
// 	this->setContents();
//     }

    // Create or truncate, and then write the bits.
    //
    int fd = SafeOpen(filename, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) {
	sprintf(buf, catgets(DT_catd, 3, 44, "Unable to create %s."), filename);
        helpId = DTMAILHELPNOCREATE;
	answer = parent()->handleErrorDialog(catgets(DT_catd, 1, 87, "Mailer"), 
					     buf,
                                             helpId);
	return;
    }

    if (SafeWrite(fd, _myContents, (unsigned int)_myContentsSize) < _myContentsSize) {
	sprintf(buf, catgets(DT_catd, 3, 45, "Unable to create %s."), 
		filename);
        helpId = DTMAILHELPNOCREATE;
	answer = parent()->handleErrorDialog(catgets(DT_catd, 1, 88, "Mailer"), 
					     buf,
                                             helpId);
	SafeClose(fd);
	unlink(filename);
	return;
    }
    
    SafeClose(fd);

    // Stat the created file and see if it is of size 0.
    // If it is, engage the user in a dialog and involve a dtmail engineer
    // to attach a debugger to this process.

//     struct stat sbuf;
//     SafeStat(filename, &sbuf);
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

#ifdef DEAD_WOOD
void
Attachment::setRow(
    int row
)
{
    _row = row;
}
#endif /* DEAD_WOOD */


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
    if (theRoamApp.session()->session()->validObjectKey(
			acb->_myKey) == DTM_FALSE) {
	// Error out. Post an error?
	return;
    }
    else {
	acb->_myAttachment->action(id, action_arg, argCount, status);
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
    DtMailEnv	mail_error;
    int answer;
    char buf[2048];

    // Initialize the mail_error.
    mail_error.clear();

    if (status == DtACTION_INVOKED) {
	registerAction(id);
	_parent->setPendingAction(TRUE);
    }
    else if (status == DtACTION_DONE) {
	unregisterAction(id);	
	_parent->setPendingAction(FALSE);
	
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
		if (bufArg.writable) {
		    // Assume user edited launched attachment -- as
		    // optimization, we can compare buffers to see if
		    // content actually changed.  For now, assume it changed.
		    // Make a copy of incoming buffer and set body part's
		    // contents, size. Let BE determine type.
		    // Reset private variables.
		    _body_part->setContents(
					mail_error, 
					(char *)bufArg.bp,
					bufArg.size,
					NULL,
					NULL,
					NULL,
					NULL
				);
			
// 		    if (mail_error.isSet()) {
// 			//handle error
// 		    }

		    assert(mail_error.isNotSet());

		    if (_myType) {
			free(_myType);
			_myType = NULL;
		    }
		    _body_part->getContents(
					mail_error, 
					&_myContents,
					&_myContentsSize,
					&_myType, 
					NULL, 
					NULL, 
					NULL);
// 		    if (mail_error.isSet()) {
// 			//handle error
// 		    }
		    assert(mail_error.isNotSet());
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

		if ( SafeStat ( fileArg.name, &stat_buf ) == -1 ) {
		    mail_error.setError(DTME_ObjectAccessFailed);
		    mail_error.logError(DTM_FALSE, "Mailer: Unable to process action, stat failed on file %s.\n", fileArg.name);
		    return;
		}
		tmp_file = SafeOpen(fileArg.name, O_RDONLY);
		char *tmp_buf = (char *)malloc(stat_buf.st_size);
		SafeRead(tmp_file, (void *)tmp_buf, stat_buf.st_size);
		SafeClose(tmp_file);

		_body_part->setContents(mail_error,
					tmp_buf,
					stat_buf.st_size,
					NULL, NULL, NULL, NULL);
			
		assert(mail_error.isNotSet());

		// Free the buffer 
		free(tmp_buf);

		if (_myType) {
		    free(_myType);
		    _myType = NULL;
		}

		_body_part->getContents(mail_error,
					&_myContents,
					&_myContentsSize,
					&_myType,
					NULL, NULL, NULL);

		assert(mail_error.isNotSet());
 	    }
	}
    }
    else if (status == DtACTION_INVALID_ID) {
	/* NL_COMMENT
	 * Post a dialog explaining that the action was invalid
	 */
	sprintf(buf, 
		catgets(
			DT_catd, 3, 91, "Cannot execute invalid action."));

	answer = parent()->handleErrorDialog(catgets(DT_catd, 1, 86, "Mailer"),
					     buf);

	unregisterAction(id);	
	_parent->setPendingAction(FALSE);
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
	_parent->setPendingAction(FALSE);
    }
    else if (status == DtACTION_STATUS_UPDATE) {
	// Check if ownerShell is an SMD.  Make sure the message has not
	// been sent before attempting to update things.

	if(actionArg != NULL && _parent->isOwnerShellEditable() && !_deleted) {
	    SendMsgDialog *smd = _parent->getOwnerShell();
	    smd->setEditsMade(TRUE);
	    if (actionArg->argClass == DtACTION_BUFFER) {
		bufArg = actionArg->u.buffer;
		if (bufArg.writable) {
		    // Assume user edited launched attachment -- as
		    // optimization, we can compare buffers to see if
		    // content actually changed.  For now, assume it changed.
		    // Make a copy of incoming buffer and set body part's
		    // contents, size. Let BE determine type.
		    // Reset private variables.
		    _body_part->setContents(
					mail_error, 
					(char *)bufArg.bp,
					bufArg.size,
					NULL,
					NULL,
					NULL,
					NULL);
			
// 		    if (mail_error.isSet()) {
// 			//handle error
// 		    }

		    assert(mail_error.isNotSet());

		    if (_myType) {
			free(_myType);
			_myType = NULL;
		    }
		    _body_part->getContents(
					mail_error, 
					&_myContents,
					&_myContentsSize,
					&_myType, 
					NULL,
					NULL,
					NULL);
// 		    if (mail_error.isSet()) {
// 			//handle error
// 		    }

		    assert(mail_error.isNotSet());

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

		if ( SafeStat ( fileArg.name, &stat_buf ) == -1 ) {
		    mail_error.setError(DTME_ObjectAccessFailed);
		    mail_error.logError(DTM_FALSE, "Mailer: Unable to process action, stat failed on file %s.\n", fileArg.name);
		    return;
		}
		tmp_file = SafeOpen(fileArg.name, O_RDONLY);
		char *tmp_buf = (char *)malloc(stat_buf.st_size);
		SafeRead(tmp_file, (void *)tmp_buf, stat_buf.st_size);
		SafeClose(tmp_file);

		_body_part->setContents(mail_error,
					(char *)tmp_buf,
					(int) stat_buf.st_size,
					NULL,
					NULL,
					NULL,
					NULL);
			
		assert(mail_error.isNotSet());

		if (_myType) {
		    free(_myType);
		    _myType = NULL;
		}

		_body_part->getContents(mail_error,
					&_myContents,
					&_myContentsSize,
					&_myType,
					NULL,
					NULL,
					NULL);

		assert(mail_error.isNotSet());

		// Free the buffer 
		free(tmp_buf);
	    }
	}
    }
    else if (status == DtACTION_CANCELED) {
	unregisterAction(id);	
	_parent->setPendingAction(FALSE);

	if (actionArg != NULL) {
		XtFree((char *)(actionArg->u.buffer.bp));
	}
    }
}

void
Attachment::deleteIt()
{
    DtMailEnv mail_error;

    // Initialize the mail_error.
    
    mail_error.clear();

    _deleted = TRUE;

    // Need to remove the view from display

    // Get the BE to mark the bodyPart as deleted
    _body_part->setFlag(mail_error, DtMailBodyPartDeletePending);

}

void
Attachment::undeleteIt()
{
    DtMailEnv mail_error;

    // Initialize the mail_error.

    mail_error.clear();

    _deleted = FALSE;

    _body_part->resetFlag(mail_error, DtMailBodyPartDeletePending);

}

void
Attachment::registerAction(
    DtActionInvocationID id
)
{
    _canKillSelf = FALSE;
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
    _canKillSelf = TRUE;
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

	n = 0;
	XtSetArg(args[n], XmNwidth, &_attachmentWidth); n++;
	XtSetArg(args[n], XmNheight, &_attachmentHeight); n++;
	XtGetValues(_w, args, n);
}

void
Attachment::primitive_select()
{
	_selected = TRUE;
	myIcon->primitiveSelect();

}

void
Attachment::rename(
    XmString new_name
)
{
    char *name;
    DtMailEnv mail_error;
    
    mail_error.clear();

    XmStringGetLtoR(new_name, XmSTRING_DEFAULT_CHARSET, &name);
    
    _body_part->setContents(
			mail_error,
			NULL,
			1,
			NULL,
			name,
			NULL,
			NULL
		);

    this->setLabel(new_name);
}

void
Attachment::setContents()
{
    DtMailEnv mail_error;

    // Initialize the mail_error.
    mail_error.clear();

    if (_myType) {
	free(_myType);
	_myType = NULL;
    }
    _body_part->getContents(
			mail_error, 
			&_myContents,
			&_myContentsSize,
			&_myType,
			NULL, 
			NULL, 
			NULL);

    // BE has returned an error condition...

// It would be nice to popup a dialog to let the user know that 
// dtmail has run into a problem that it can't resolve.  Unfortunately,
// the code for postFatalErrorDialog() has been ifdef'ed out.  Perhaps
// the error dialog can be enabled at some point in the future.

//     if (mail_error.isSet()) {
// 	parent()->myRMW->postFatalErrorDialog(mail_error);
//     }
    assert ( mail_error.isNotSet() );

    _haveContents = TRUE;

    // If it is a rfc822 message, check if it has From stuffing.
    // From stuffing is ">From".
    // If it has, advance the pointer to step past the ">" character.

    if (_myType && !strcmp(_myType,"DTMAIL_FILE")) {

	// If it has a From header, return.  The classing engine uses
	// that to determine the client that needs to be launched.

	if (!strncasecmp((char *)_myContents, "From", 4)) {
	    return;
	}

	// Message doesn't begin with From. It may have From stuffing --
	// ">From".  Or it may have no From header at all. The MIME 
	// specs are vague on what headers an Message/RFC822 body part
	// has to have.  We need From to help the classing engine and
	// therefore, we will by force (or hack) make it have a From :-)

	if (!strncasecmp((char *)_myContents, ">From", 5)) {

	    // Has From stuffing.

	    char *ptr = (char *)_myContents;
	    ptr++;			    // Step past the first char
	    _myContents = (void *) ptr;	   // Reset contents
	    _myContentsSize--;
	}

	else { // No From header.  See if it has any headers.

	    // We do this by checking if there is a colon present in the
	    // first line.  If we succeed at finding one, we presume the
	    // line is an RFC header line.  And we introduce a new From
	    // header line before the other headers.

	    char *sptr = NULL;
	    sptr = (char *)_myContents;

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

		char *str = "From UNKNOWN \n";
		char *buffer = new char[strlen(str) + _myContentsSize + 1];
		strcpy(buffer, str);
		strncat(buffer, (char *)_myContents, (unsigned int)_myContentsSize);
		_myContentsSize = strlen(buffer);
		_myContents = (void *)buffer;
	    }
	    else {  
		// Doesn't contain any header.  Introduce From header.
		// Throw an extra newline to make rest of contents a
		// the attachment body.

		char *str = "From UNKNOWN \n\n";
		char *buffer = new char[strlen(str) + _myContentsSize + 1];
		strcpy(buffer, str);
		strncat(buffer, (char *)_myContents, (unsigned int)_myContentsSize);
		_myContentsSize = strlen(buffer);
		_myContents = (void *)buffer;

	    }
	}
    }
}

void *
Attachment::getContents()
{
    if (_myContents) {
	return ( (void *)_myContents ); 
    }
    else {
	this->setContents();
	return ( (void *)_myContents ); 
    }
}
	
	
