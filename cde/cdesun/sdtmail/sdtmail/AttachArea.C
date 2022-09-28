/*
 *+SNOTICE
 *
 *	$Revision: 1.13 $
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
#pragma ident "@(#)AttachArea.C	1.79 24 Oct 1994"
#endif

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#if defined(USL)
#define S_ISSOCK(mode) ((mode & S_IFMT) == S_IFSOCK)
#endif
#include <stdio.h>
#include <Dt/Editor.h>

#include "EUSDebug.hh"

#if defined(NEED_MMAP_WRAPPER)
extern "C" {
#endif

#include <sys/mman.h>

#if defined(NEED_MMAP_WRAPPER)
}
#endif


#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>


#include <Xm/BulletinB.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>
#include <Xm/DrawingA.h>
#include <Xm/ScrollBar.h>
#include <Xm/FileSB.h>
#include <Xm/SelectioB.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/AtomMgr.h>
#include <Xm/Xm.h>
#include <Xm/Screen.h>
#include <Xm/ToggleB.h>
#include <X11/IntrinsicP.h>
#include <X11/Xatom.h>

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/MailRc.hh>
#include <SDtMail/MessageStore.hh>
#include <SDtMail/Message.hh>
#include <SDtMail/MessageEnvelope.hh>
#include <SDtMail/SystemUtility.hh>
#include <SDtMail/DataTypeUtility.hh>

#include "Attachment.h"
#include "AttachArea.h"
#include "Icon.h"
#include "RoamApp.h"
#include "RoamMenuWindow.h"

#include "MsgScrollingList.hh"
#include "ViewMsgDialog.h"
#include "SendMsgDialog.h"
#include "MailMsg.h"		// DT_catd defined here
#include "DtMailEditor.hh"
//#include <DtMail/DtMail.hh>    // time_t defined here
//#include <DtMail/IO.hh>                    // SafeAccess...

#include "Help.hh"
#include "DtMailHelp.hh"

extern nl_catd	DtMailMsgCat;

#define equal(a, b) (!strcmp(a,b))
#define HSPACE 10
#define VSPACE 10
#define MAXATOM 2048	// ?????

// This is the new one
AttachArea::AttachArea ( 
			Widget parent, 
			DtMailEditor *owner,
			char *name
			) : UIComponent (name)
{
  // Unique stuff

  _myOwner = owner;
  _parent  = parent;

  _attachmentList=NULL;
  _deleteBatch = 0;

  _w = NULL;
  _iconCount = 0;
  _iconSelectedCount = 0;
  _deleteCount = 0;

  _fsDialog = NULL;
  _fsState = NOTSET;
  _lastRow = 0;
  _currentRow = 0;
  _attachmentsSize = 0;
  _selectedAttachmentsSize = 0;
  _clientData = NULL;
  _renameDialog = NULL;
  _emptyString = NULL;

  _myRMW = NULL;
  _myVMD = NULL;
  _mySMD = NULL;
    
  _pendingAction = Sdm_False;
  _numPendingActions = 0;

  _attach_area_selection_state = AA_SEL_NONE;
  _cache_single_attachment = NULL;

}

void
AttachArea::initialize()
{

  // We're making the assumption here that this widget's parent`
  // is also a form

  XtWidgetGeometry size;
  Dimension parWid;

  _w = XtVaCreateManagedWidget (
				"AttachPane",
				xmFormWidgetClass, _parent, 
				NULL);
  XtVaSetValues(_w, 
		XmNleftAttachment, XmATTACH_FORM, 
		XmNrightAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM, NULL);


  _foreground = _myOwner->textEditor()->get_text_foreground();
  _background = _myOwner->textEditor()->get_text_background();
  parWid = _myOwner->textEditor()->get_text_width();

  _appBackground = _background;
  _appForeground = _foreground;

  _sw = XtVaCreateManagedWidget ( 
				 "AttachPane_ScrolledWindow", 
				 xmScrolledWindowWidgetClass, _w, 
				 XmNscrollingPolicy,  XmAPPLICATION_DEFINED,
				 XmNrightAttachment,  XmATTACH_FORM,
				 XmNleftAttachment,   XmATTACH_FORM,
				 XmNtopAttachment,    XmATTACH_FORM,
				 XmNshadowThickness, (Dimension)1,
				 XmNspacing, 2,
				 XmNwidth,	     parWid,
				 XmNheight,	     70,
				 NULL);

  _rowOfAttachmentsStatus = XtCreateManagedWidget("Attachments_Status",
						  xmFormWidgetClass,
						  _w, NULL, 0);


  XtVaSetValues(_rowOfAttachmentsStatus,
		XmNrightAttachment,	XmATTACH_FORM, 
		XmNleftAttachment,	XmATTACH_FORM, 
		XmNbottomAttachment,    XmATTACH_FORM,
		XmNtopAttachment,       XmATTACH_WIDGET,
		XmNtopWidget,	        _sw,
		XmNtopOffset,		5,
		XmNbottomOffset,	5,
		NULL );

  this->addToRowOfAttachmentsStatus();

  size.request_mode = CWHeight;
  XtQueryGeometry(_rowOfAttachmentsStatus, NULL, &size);

  XtVaSetValues(
		_rowOfAttachmentsStatus,
		XmNpaneMaximum, size.height,
		XmNpaneMinimum, size.height,
		NULL
		);

  _vsb = XtVaCreateManagedWidget("vsb", xmScrollBarWidgetClass, _sw,
				 XmNorientation, XmVERTICAL,
				 XmNsliderSize, 1,
				 XmNmaximum, 1,
				 XmNpageIncrement, 1,
				 NULL);

  XtAddCallback(
		_vsb, 
		XmNvalueChangedCallback,&AttachArea::valueChangedCallback,
		(XtPointer) this 
		);

  XtAddCallback(
		_vsb, 
		XmNdragCallback, &AttachArea::dragCallback,
		(XtPointer) this 
		);

  _clipWindow = XtVaCreateManagedWidget("AttachArea_clipWindow", 
					xmDrawingAreaWidgetClass, _sw,
					XmNresizePolicy, XmRESIZE_NONE,
					XmNbackground, _background,
					XmNwidth,	     parWid,
					XmNheight,	     70,
					NULL);
  XmScrolledWindowSetAreas(_sw, NULL, _vsb, _clipWindow);

  XtManageChild(_clipWindow);
  XtManageChild(_vsb);
  XtManageChild(_sw);

    
  // Set RowCol to NULL here.
  // It gets set in the expose_all_attachments.

  _rc = NULL;

  CalcSizeOfAttachPane();

  installDestroyHandler();
}

AttachArea::~AttachArea()
{
  if (_attachmentList) {
    // first, delete Attachment objects from list.
    for(int i=0; i < _iconCount; i++) {
      delete _attachmentList[i];
    }

    // next delete the list itself.
    delete []_attachmentList;
  }
  
  if (_emptyString) {
      XmStringFree(_emptyString);
  }
}

void AttachArea::addToList( Attachment *attachment )
{
  Attachment **newList;
  int i;

  newList = new Attachment*[ _iconCount + 1 ];
  for(i=0; i < _iconCount; i++)
    newList[i] = _attachmentList[i];
 
  if (_attachmentList)
    delete []_attachmentList;

  _attachmentList = newList;

  _attachmentList[ _iconCount ] = attachment;

  _iconCount++;
}

int AttachArea::getSelectedIconCount()
{
  Attachment **list = getList();
  int num_selected = 0;

  for (int i = 0; i < getIconCount(); i++) {
    if (!list[i]->isDeleted() && list[i]->isSelected())
      num_selected++;
  }
  return (num_selected);
}

Attachment *
AttachArea::getSelectedAttachment()
{
  return _cache_single_attachment;
}

void
AttachArea::MenuButtonHandler(
			      Widget ,
			      XtPointer cd,
			      XEvent *event,
			      Boolean *)
{
  AttachArea *obj = (AttachArea *)cd;

  if(event->xany.type != ButtonPress)
    return;

  XButtonEvent *be = (XButtonEvent *)event;

  if(be->button == theApplication->bMenuButton())
    obj->_myOwner->owner()->postAttachmentPopup(event);
}

void 
AttachArea::inputCallback(Widget, XtPointer client_data, XtPointer call_data)
{
  AttachArea *obj = (AttachArea *) client_data;
  XmDrawingAreaCallbackStruct *cb = (XmDrawingAreaCallbackStruct *)call_data;

  if(cb->reason != XmCR_INPUT 	||
     cb->event->xany.type != ButtonPress)
    return;

  if(((XButtonEvent *)cb->event)->button == Button1)
    obj->unselectOtherSelectedAttachments(NULL);
}

void AttachArea::resizeCallback ( 
				 Widget w, 
				 XtPointer clientData, 
				 XtPointer //callData
				 )
{
  Dimension wid;

  AttachArea *obj = (AttachArea *) clientData;
  XtVaGetValues(
		w,
		XmNwidth, &wid,
		NULL
		);

  obj->resize(wid);
}

void AttachArea::resize(
			Dimension wid
			)
{
  int i;
  Attachment **list = getList();

  _attachAreaWidth = wid;

  XtVaSetValues(
		_clipWindow,
		XmNwidth, _attachAreaWidth,
		NULL);

  for (i=0; i<getIconCount(); i++) {

    list[i]->unmanageIconWidget();
  }

  CalcAllAttachmentPositions();
  CalcLastRow();
  AdjustCurrentRow();
  SetScrollBarSize(getLastRow()+1);
  DisplayAttachmentsInRow(_currentRow);
}

void AttachArea::CalcSizeOfAttachPane( )
{
  Dimension parWid;

  parWid = _myOwner->textEditor()->get_text_width();

  _attachAreaWidth = parWid;

  XtVaSetValues(
		_clipWindow,
		XmNwidth, _attachAreaWidth,
		NULL);
}

void AttachArea::activateDeactivate()
{
  //
  // If exactly one icon is selected then activate the open command 
  //

  if(getIconSelectedCount() == 1) {
    openCmd()->activate();
  } else {
    openCmd()->deactivate();
  }

  // If no icons are selected then deactivate the OK button on the FS Dialog

  if(getIconSelectedCount() > 0) {
    if(getFsDialog())
      XtSetSensitive(
		     XmSelectionBoxGetChild( 
					    getFsDialog(), XmDIALOG_OK_BUTTON), TRUE
		     );
  } else {
    if(getFsDialog())
      XtSetSensitive(
		     XmSelectionBoxGetChild( 
					    getFsDialog(), XmDIALOG_OK_BUTTON
					    ), 
		     (getFsState() == SAVEAS) ? FALSE : TRUE
		     );
  }
}
void
AttachArea::setAttachAreaHeight(Attachment *att)
{
  int h = att->getHeight()+10;
  XtVaSetValues(sw(), XmNheight, h, NULL);
  XtVaSetValues(getClipWindow(), XmNheight, h, NULL);
}

// Used by Compose window (SMD)
// Given a filename, add it to message and to attachArea.

Attachment*
AttachArea::addAttachment( SdmMessage* msg,
			   const char *filename,
			   const char *name )
{
  int fd;
  struct stat s;
  SdmBoolean validtype = Sdm_True;
  char errormsg[512];
  SdmMessageBody *bp = NULL;
  SdmString	str;
  SdmError mail_error;
  int answer;
  char buf[2048];
  char *helpId = NULL;

  mail_error.Reset();

  char *buffer = NULL, *lbl = NULL;

  msg->SdmMessageBodyFactory(mail_error, bp,
			     (SdmMsgStrType)Sdm_MSTYPE_text, "plain");	// Create new body part
  if (mail_error) {
    sprintf(buf, catgets(DT_catd, 12, 9, "Cannot create message attachment."));
    answer = this->handleErrorDialog(catgets(DT_catd, 1, 81, "Mailer"), 
				     buf);
    return(NULL);
  }

  if (SdmSystemUtility::SafeAccess(filename, F_OK) != 0) {
    sprintf(buf, catgets(DT_catd, 3, 34, "%s does not exist."),
	    filename);
    answer = this->handleErrorDialog(catgets(DT_catd, 1, 81, "Mailer"), 
				     buf);
    return(NULL);
  }

  SdmSystemUtility::SafeStat(filename, &s);

  if(S_ISFIFO(s.st_mode)) {
    sprintf(errormsg,
	    catgets(DT_catd, 12, 4, "Cannot attach FIFO files: %s"), filename);
    validtype = Sdm_False;
  } else if(S_ISCHR(s.st_mode)) {
    sprintf(
	    errormsg,
	    catgets(DT_catd, 12, 5, "Cannot attach character special files: %s"), filename
	    );
    validtype = Sdm_False;
  } else if(S_ISDIR(s.st_mode)) {
    sprintf(
	    errormsg,
	    catgets(DT_catd, 12, 6, "Cannot attach directories: %s"), filename
	    );
    validtype = Sdm_False;
  } else if(S_ISBLK(s.st_mode)) {
    sprintf(errormsg,
	    catgets(DT_catd, 12, 7, "Cannot attach block special files: %s"), filename
	    );
    validtype = Sdm_False;
  } else if(S_ISSOCK(s.st_mode)) {
    sprintf(errormsg,
	    catgets(DT_catd, 12, 8, "Cannot attach socket files: %s"), filename
	    );
    validtype = Sdm_False;
  }

  if(validtype == Sdm_False) {
    answer = this->handleErrorDialog(catgets(DT_catd, 1, 81, "Mailer"), errormsg);
    return(NULL);
  }

  fd = SdmSystemUtility::SafeOpen(filename, O_RDONLY);
	
  if (fd < 0) {
    sprintf(buf, catgets(DT_catd, 3, 35, "Unable to open %s."), filename);
    helpId = DTMAILHELPNOOPEN;
    answer = this->handleErrorDialog(catgets(DT_catd, 1, 82, "Mailer"), 
				     buf, helpId);
    return(NULL);
  }

  // Cause the content buffer to be filled directly from the open file descriptor
  // Will use mmap() if it can, falls back to malloc/free if it cannot.

  SdmContentBuffer content_buf;
  if ( (content_buf.MapAgainstFile(mail_error, fd, s.st_size) != Sdm_EC_Success) 
       || (bp->SetContents(mail_error, content_buf) != Sdm_EC_Success)) {
    if (mail_error == Sdm_EC_NoMemory) {
      sprintf(buf, 
	      catgets(DT_catd, 3, 36, "Unable to allocate memory."));
      helpId = DTMAILHELPNOALLOCMEM;
      answer = this->handleErrorDialog(catgets(DT_catd, 1, 83, "Mailer"), 
				       buf, helpId);
    }
    else {
      sprintf(buf, 
	      catgets(DT_catd, 12, 10, "Cannot set contents for attachment."));
      answer = this->handleErrorDialog(catgets(DT_catd, 1, 81, "Mailer"), 
				       buf);
    }
    SdmSystemUtility::SafeClose(fd);
    return(NULL);
  }

  SdmSystemUtility::SafeClose(fd);

  // _iconCount + 1 because iconCount starts at 0 and we want 
  // attachmentCount to begin at 1.  attachmentCount is set to be
  // in the widget's userData.  

  if(name)
    lbl = strdup(name);
  else {
    if(strchr(filename, '/') == NULL) // The name does not include a slash
      lbl = strdup(filename);
    else			   // The name does include a slash
      lbl = strdup(strrchr(filename, '/')+1);
  }    

  // Save the attachment name and the unix mode
  SdmMessageBodyStructure bodyStruct;
  bp->GetStructure(mail_error, bodyStruct);
  if (mail_error == Sdm_EC_Success) {
    bodyStruct.mbs_attachment_name = lbl;
    bodyStruct.mbs_unix_mode = s.st_mode;
    bp->SetStructure(mail_error, bodyStruct);
  }
  
  if (mail_error) {
    sprintf(buf, catgets(DT_catd, 12, 11, "Cannot set name for attachment"));
    answer = this->handleErrorDialog(catgets(DT_catd, 1, 81, "Mailer"), 
				     buf);
    return(NULL);
  }
  
  // Set the content type for the body part
  // if there's an error, on setting the content type, just continue.
  SdmDataTypeUtility::SetContentTypesForMessage(mail_error, *bp);

  Attachment *attachment = new Attachment((AttachArea *)this,
					  (SdmString)lbl, bp, (int) _iconCount + 1);
  attachment->setAttachArea(this);
  attachment->initialize();
  this->setAttachAreaHeight(attachment);
  addToList( attachment );

  // Update the display.  The Compose Window needs immediate update.

  this->manageList();

  free(lbl);

  return(attachment);
}

Attachment*
AttachArea::addAttachment( SdmMessage *msg,
			   const char* name,
			   SdmString stringBuf )
{
  SdmError mail_error;
  SdmMessageBody * bp = NULL;
  SdmString theName(name);
  char buf[2048];
  int answer;

  // if the name is not defined, use default.
  if (theName.Length() == 0) {
    theName = "noname";
  }

  mail_error.Reset();

  msg->SdmMessageBodyFactory(mail_error, bp,
			     (SdmMsgStrType)Sdm_MSTYPE_text, "plain");
  if (mail_error) {
    sprintf(buf, catgets(DT_catd, 12, 9, "Cannot create message attachment"));
    answer = this->handleErrorDialog(catgets(DT_catd, 1, 81, "Mailer"), 
				     buf);
    return(NULL);
  }

  if ( stringBuf.Length() > 0 ) {
    // I18N - changed api - SetContents now takes a content buffer
    SdmContentBuffer content_buf(stringBuf);
    bp->SetContents(mail_error, content_buf);
    if (mail_error) {
      sprintf(buf, catgets(DT_catd, 12, 10, "Cannot set contents for attachment"));
      answer = this->handleErrorDialog(catgets(DT_catd, 1, 81, "Mailer"), 
               buf);
      return(NULL);
    }
  }

  // Save the attachment name
  SdmMessageBodyStructure bodyStruct;
  bp->GetStructure(mail_error, bodyStruct);
  if (mail_error == Sdm_EC_Success) {
    bodyStruct.mbs_attachment_name = theName;
    bp->SetStructure(mail_error, bodyStruct);
  }
  if (mail_error) {
    sprintf(buf, catgets(DT_catd, 12, 11, "Cannot set name for attachment"));
    answer = this->handleErrorDialog(catgets(DT_catd, 1, 81, "Mailer"), 
             buf);
    return(NULL);
  }


  SdmDataTypeUtility::SetContentTypesForMessage(mail_error, *bp);

  Attachment *attachment = new Attachment(this,
					  theName,
					  bp,
					  _iconCount + 1);
  attachment->setAttachArea(this);
  attachment->initialize();
  this->setAttachAreaHeight(attachment);
  addToList(attachment);

  // Update the display.  The Compose Window needs immediate update.

  this->manageList();

  return(attachment);
}

Attachment* 
AttachArea::addAttachment( SdmString name,
			   SdmMessageBody *body_part )
{
  // _iconCount + 1 because iconCount starts at 0 and we want 
  //  attachmentCount to begin at 1.  attachmentCount is set to be
  // in the widget's userData.  

  Attachment *attachment = new Attachment(
					  this, 
					  name, 
					  body_part, 
					  _iconCount + 1
					  );
  attachment->setAttachArea(this);
  attachment->initialize(Sdm_True);
  this->setAttachAreaHeight(attachment);
  addToList( attachment );

  return(attachment);
}

void AttachArea::manageList( )
{
  Attachment **list = getList();

  CalcAllAttachmentPositions();
  CalcLastRow();
  AdjustCurrentRow();
  SetScrollBarSize(getLastRow()+1);
  DisplayAttachmentsInRow(_currentRow);

}

//
// Display the attachments in row X
//

void AttachArea::DisplayAttachmentsInRow(unsigned int X)
{
  int i;
  int managecount, unmanagecount;

  Attachment **list = getList();
  WidgetList manageIconList, unmanageIconList;

  manageIconList = (WidgetList)XtMalloc(sizeof(Widget) * getIconCount());
  unmanageIconList = (WidgetList)XtMalloc(sizeof(Widget) * getIconCount());
     
  managecount = unmanagecount = 0;
  for(i=0;i<getIconCount();i++) {
    if(!list[i]->isDeleted()) {
      if(list[i]->getRow() == X) {
        if(!list[i]->isManaged()) {
          manageIconList[managecount] = list[i]->baseWidget();
          managecount++;
        } 
      }
      else { // if deleted
        if(list[i]->isManaged()) {
          unmanageIconList[unmanagecount] = list[i]->baseWidget();
          unmanagecount++;
        }
      }
    }
  }

  XtUnmanageChildren(unmanageIconList, unmanagecount);
  XtManageChildren(manageIconList, managecount);


  XtFree((char *)manageIconList);
  XtFree((char *)unmanageIconList);

  this->attachment_summary(_iconCount - _deleteCount, _deleteCount);
  this->bottomStatusMessage(" ");
}

//
// Calculate the position of every non-deleted Attachment
//

void AttachArea::CalcAllAttachmentPositions()
{
  int i, j;
  Attachment **list = getList();
 
  j = -1;
  for(i=0;i<getIconCount();i++) {
    if(!list[i]->isDeleted()) {
      calculate_attachment_position(
				    (j == -1) ? (Attachment *)NULL : list[j],
				    list[i]);
      j = i;
    }
  }
}

//
// Determine the position of attachment "item" given reference
// attachment "ref"
//

void AttachArea::calculate_attachment_position(
					       Attachment *ref,
					       Attachment *item
					       )
{
  if(ref == NULL) {
    item->setX(HSPACE);
    item->setY(VSPACE);
    item->setRow(0);
    return;
  }
  if(((Dimension)(ref->getX() + 
		  ref->getWidth() + 
		  HSPACE + 
		  item->getWidth())) 	> getAAWidth() ) {
    item->setX(HSPACE);
    item->setY(VSPACE);
    item->setRow(ref->getRow() + 1);
  } else {
    item->setX(ref->getX() + ref->getWidth() + HSPACE);
    item->setRow(ref->getRow());
    item->setY(VSPACE);
  }
}

//
// Invoked when the user moves the slider by any method
// If the user is dragging the slider then this callback
// is only invoked when the user releases the mouse button
//

void AttachArea::valueChangedCallback ( 
				       Widget, 
				       XtPointer clientData, 
				       XtPointer callData 
				       )
{
  AttachArea *obj = (AttachArea *) clientData;

  obj->valueChanged( callData );
}

void AttachArea::valueChanged( XtPointer callData )
{
  XmScrollBarCallbackStruct *cbs = (XmScrollBarCallbackStruct *)callData;

  _currentRow = cbs->value;
  DisplayAttachmentsInRow(_currentRow);
}

//
// Invoked when the user drags the slider
//

void AttachArea::dragCallback ( 
			       Widget, 
			       XtPointer clientData, 
			       XtPointer callData 
			       )
{
  AttachArea *obj = (AttachArea *) clientData;

  obj->dragSlider( callData );
}

void AttachArea::dragSlider( XtPointer callData )
{
  XmScrollBarCallbackStruct *cbs = (XmScrollBarCallbackStruct *)callData;

  if(cbs->value == _currentRow)
    return;
  _currentRow = cbs->value;
  DisplayAttachmentsInRow(_currentRow);
}

//
// Calculate the number of rows
//

void AttachArea::CalcLastRow()
{
  int i;
  unsigned row = 0;
  Attachment **list = getList();
 
  for(i=0;i<getIconCount();i++) {
    if(!list[i]->isDeleted()) {
      row = list[i]->getRow();
    }
  }
  _lastRow = row;
}

//
// Set the XmmNmaximum resource to size
//

void AttachArea::SetScrollBarSize(unsigned int size)
{
  XtVaSetValues(_vsb, 
		XmNmaximum, size,
		NULL);
}

//
// If the current row is greater than the last row, adjust the
// current row to be equal to the last row.
//

void AttachArea::AdjustCurrentRow()
{
  if(_currentRow > _lastRow) {
    _currentRow = _lastRow;
    XtVaSetValues(_vsb, XmNvalue, _currentRow, NULL);
  }
}

XmString
AttachArea::getSelectedAttachName()
{
  XmString str = (XmString)NULL;

  if(_cache_single_attachment)
    str = _cache_single_attachment->getLabel();

  return(str);
}

void
AttachArea::setSelectedAttachName(
				  XmString new_name
				  )
{
  int i;
  SdmBoolean set = Sdm_False;
  Attachment **list = this->getList();

  // Set name of first selected attachment to new_name
  for(i=0;i<this->getIconCount() && !set;i++)
    if(list[i]->isSelected()) {
      list[i]->rename(new_name);
      set = Sdm_True;
    }
  this->manageList();
}


void
AttachArea::attachmentSelected(
			       Attachment *attachment
			       ) 
{
  // First deselect other selected attachments
  this->unselectOtherSelectedAttachments(attachment);
    
    // Enable the menu item at the toplevel shell's menubar.
  if(_cache_single_attachment == NULL)
    _myOwner->owner()->attachment_selected();

    // Cache the single selected attachment
  _cache_single_attachment = attachment;
  _attach_area_selection_state = AA_SEL_SINGLE;
}

void
AttachArea::attachmentFeedback(
			       SdmBoolean bool
			       )
{
  _myOwner->owner()->attachmentFeedback(bool);
}
    
char *
AttachArea::calcKbytes(unsigned int bytes)
{
  static char kstring[64];

  if(bytes < 103)
    sprintf(kstring, "%d bytes",bytes);
  else if(bytes < 1024)
    sprintf(kstring, " .%dk", bytes/103);
  else
    sprintf(kstring, "%dk", bytes/1024);

  return kstring;
}

void
AttachArea::addToRowOfAttachmentsStatus()
{
  XmString labelStr;

  _emptyString = XmStringCreateLocalized(" ");

  labelStr = XmStringCreateLocalized(
				     catgets(DT_catd, 3, 37, "Summary of attachments"));

  _bottomStatusMessage = XtCreateManagedWidget(
					       "Message_Status_Text", xmLabelWidgetClass,
					       _rowOfAttachmentsStatus, NULL, 0);

  XtVaSetValues(_bottomStatusMessage,
		XmNalignment, XmALIGNMENT_BEGINNING,
		XmNlabelString, _emptyString,
		XmNleftAttachment, XmATTACH_FORM,
		XmNtopAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		NULL );


  _attachments_summary = XtCreateManagedWidget("Attachments_Summary",
					       xmLabelWidgetClass,
					       _rowOfAttachmentsStatus, NULL, 0);

  XtVaSetValues(_attachments_summary,
		XmNalignment, XmALIGNMENT_END,
		XmNlabelString, labelStr,
		XmNtopAttachment, XmATTACH_FORM,
		XmNrightAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		NULL );

  XmStringFree(labelStr);
 
  XtVaSetValues(_bottomStatusMessage,
                XmNrightAttachment, XmATTACH_WIDGET,
                XmNrightWidget, _attachments_summary,
                NULL);
 
}

int
processMessage(
	       SdmError &mail_error,
	       SdmMessage *msg,
	       int bodyCount,
	       SdmBoolean displayText,
	       AttachArea *thisAA)
{
  SdmMessageBodyStructure bpStruct;
  SdmMessage *nestedMsg;
  SdmMessageBody *tmpBP, *nestedBody;
  int nestedBodyCount;
  int num_attachments = 0;

  for (int i = 1; i <= bodyCount; i++) {

    msg->SdmMessageBodyFactory(mail_error, tmpBP, i);

    tmpBP->GetStructure(mail_error, bpStruct);

    switch ( bpStruct.mbs_type ) {

    case Sdm_MSTYPE_multipart:

      // if we can't get the nested message from a multipart mixed,
      // just continue.  we can't get the contents of the bodies in
      // the multipart.
      tmpBP->SdmMessageFactory(mail_error, nestedMsg);
      if (!mail_error) {
        nestedMsg->SdmMessageBodyFactory(mail_error, nestedBody, 1);
        if (mail_error) break;
        nestedMsg->GetBodyCount(mail_error, nestedBodyCount);
        if (mail_error) break;

        num_attachments += processMessage(mail_error,
					nestedMsg,
					nestedBodyCount,
					displayText,
					thisAA);
        if (mail_error) break;
      } else {
        mail_error = Sdm_EC_Success;  // reset error
      }
      break;

    case Sdm_MSTYPE_text:
      if ( !displayText ) {
        break;
      }
    default:
      char *name;
      const char *tmpName;
      tmpName = bpStruct.mbs_attachment_name;
      if ( !tmpName || *tmpName == '\0' )
        name = "NoName";
      else
        name = (char *)tmpName;
      thisAA->addAttachment(name, tmpBP);
      num_attachments++;
      break;
    }

    // Do it after the first iteration. (It should be done only once)
    displayText = Sdm_True;
    
    if (mail_error) break;  // break out of loop if there was an error.
  }

  return(num_attachments);
}

void
AttachArea::parseAttachments( SdmError &mail_error,
			      SdmMessage* msg,
			      SdmBoolean empty,
			      int startBodyNumber,
			      int &num_attachments)
{
  SdmMessageBody *tmpBP = NULL;
  SdmMessageBody *bodyPart = NULL;
  num_attachments = 0;
  int bodyCount;

  // First unmanage and empty out the current contents.

  // SMD sets this boolean to FALSE so that previous message's attachments
  // are not cleared.  E.g. Including/forwarding multiple messages each
  // with attachments.
  // RMW sets this boolean to TRUE so that all attachments are cleared in
  // the attachment pane everytime a new message is displayed.

  if ( empty ) {

    // First unmanage the clipWindow.
    // Unmanaging the attachment pane is visually ugly

    XtUnmanageChild(_clipWindow);
    this->clearAttachArea();
  }

  _deleteCount = 0;

  // Now fill list with new attachments.
  // Start adding attachments ...
  msg->GetBodyCount(mail_error, bodyCount);	// Get the number of bodies
  if (!mail_error) {
    if (startBodyNumber == 1) {
      num_attachments = processMessage(mail_error, msg, bodyCount, Sdm_True, this);
    } else {
      num_attachments = processMessage(mail_error, msg, bodyCount, Sdm_False, this);
    }
  }
}


void
AttachArea::bottomStatusMessage(char *msg)
{
  if (msg) {
    XmString msg_str = XmStringCreateLocalized(msg);
    XtVaSetValues(_bottomStatusMessage,
		  XmNlabelString, msg_str,
		  NULL );
    XmStringFree(msg_str);
  }
}
void
AttachArea::attachment_summary(
			       int live,
			       int dead
			       )
{

  char *buf = NULL;
  char * tmp1;
  char * tmp2;

  if ((live == 1) && (dead == 0)) {
    tmp1 = catgets(DT_catd, 3, 38, "attachment");
    buf = new char[strlen(tmp1) + 64];
    sprintf(buf, "%d %s", live, tmp1);
  }
  else if ((live >= 0) && (dead == 0)) {
    /* NL_COMMENT
     * "attachments" is the plural form of "attachment".
     */
    tmp1 = catgets(DT_catd, 3, 39, "attachments");
    buf = new char[strlen(tmp1) + 64];
    sprintf(buf, "%d %s", live, tmp1);
  }
  else if ((live >= 0) && (dead > 0)) {
    tmp1 = catgets(DT_catd, 3, 40, "attachments");
    tmp2 = catgets(DT_catd, 3, 41, "deleted");
    buf = new char[strlen(tmp1) + strlen(tmp2) + 64];
    sprintf(buf, "%d %s, %d %s", live, tmp1, dead, tmp2);
  }
    
  if (buf) {
    XmString buf_str = XmStringCreateLocalized(buf);
    XtVaSetValues(_attachments_summary,
		  XmNlabelString, buf_str,
		  NULL );
    
    delete buf;
    XmStringFree(buf_str);
  }
}

void
AttachArea::manage()
{

  // If Compsose window or View Message window
  if (isOwnerShellEditable() || isOwnerShellVMD()) 
    XtManageChild(_w);
  else {
    // If RoamMenuWindow
    XtManageChild(_sw);
    XtVaSetValues(_rowOfAttachmentsStatus,
		  XmNtopAttachment, XmATTACH_WIDGET,
		  XmNtopWidget, _sw,
		  NULL);
  }

  XtAddCallback(_clipWindow, 
		XmNresizeCallback, &AttachArea::resizeCallback,
		(XtPointer) this );

  XtAddCallback(_clipWindow, 
		XmNinputCallback, &AttachArea::inputCallback,
		(XtPointer) this );

  XtAddEventHandler(_clipWindow, ButtonPressMask,
		    FALSE, MenuButtonHandler, (XtPointer) this);

  if (!XtIsManaged(_clipWindow)) {
    // Manage the clipWindow back
    XtManageChild(_clipWindow);
  }

  XtVaSetValues(_bottomStatusMessage, XmNlabelString, _emptyString, NULL);
  XtVaSetValues(_attachments_summary, XmNlabelString, _emptyString, NULL);

  this->manageList();
}

void
AttachArea::unmanage()
{

  Attachment **list = getList();
  int i;

  // If this is an SMD or a VMD window unmanage the entire attachment area
  // If its a RMW, then just unmanage the scroll area and keep the status 
  // bar visibile.
  if (isOwnerShellEditable() || isOwnerShellVMD()) {
    XtUnmanageChild(_w);
    // SendMsgDialog...
    if (isOwnerShellEditable()) {
      SendMsgDialog *smd = getSMDOwnerShell();
      XtVaSetValues(smd->get_editor()->separator(),
		    XmNbottomAttachment, XmATTACH_FORM,
		    NULL);
    }
    // ViewMessageDialog...
    else {
      ViewMsgDialog *vmd = getVMDOwnerShell();
      XtVaSetValues(vmd->get_editor()->separator(),
		    XmNbottomAttachment, XmATTACH_FORM,
		    NULL);
    }
  }
  // RoamMenuWindow
  else {
    XtUnmanageChild(_sw);
    XtVaSetValues(_rowOfAttachmentsStatus, 
		  XmNtopAttachment,      XmATTACH_FORM,
		  NULL);
  }

  // Unmanage the widgets it currently has

  for (i=0; i<getIconCount(); i++) 
    list[i]->unmanageIconWidget();

  XtRemoveCallback(_clipWindow, 
		   XmNresizeCallback, &AttachArea::resizeCallback,
		   (XtPointer) this );

  XtRemoveCallback(_clipWindow, 
		   XmNinputCallback, &AttachArea::inputCallback,
		   (XtPointer) this );

  XtRemoveEventHandler(_clipWindow, ButtonPressMask,
		       FALSE, MenuButtonHandler, (XtPointer) this);

  XtVaSetValues(_bottomStatusMessage, XmNlabelString, _emptyString, NULL);
  XtVaSetValues(_attachments_summary, XmNlabelString, _emptyString, NULL);
}

void
AttachArea::removeCurrentAttachments()
{

  Attachment **list = getList();
  int i;
  SdmError mail_error;

  // Unmanage the widgets it currently has

  for (i=0; i<getIconCount(); i++) {

    list[i]->unmanageIconWidget();
    // in this case we don't want to post any error dialog because
    // this method is called to clear the attachment area.
    list[i]->deleteIt(mail_error);
  }

  // Reset
  if (_attachmentList)
    delete []_attachmentList;
    
  _attachmentList = NULL;
  _iconCount = 0;
  _deleteCount = 0;
  _attach_area_selection_state = AA_SEL_NONE;
  _cache_single_attachment = NULL;

  this->attachment_summary(_iconCount, _deleteCount);
}

// Similar to removeCurrentAttachments().
// Except we don't display a summary that there are no attachments.
// Plus has potential for other (different) usage.

void
AttachArea::clearAttachArea()
{

  Attachment **list = getList();
  int i;

  // Unmanage the widgets it currently has

  for (i=0; i<getIconCount(); i++) {

    list[i]->unmanageIconWidget();
    delete list[i];
  }

  // Reset
  if (_attachmentList)
    delete []_attachmentList;
    
  _attachmentList = NULL;
  _iconCount = 0;
  _deleteCount = 0;
  _attach_area_selection_state = AA_SEL_NONE;
  _cache_single_attachment = NULL;
}

SdmBoolean
AttachArea::isDeleted(int att)
{
  Attachment **list = getList();

  if ( att < 0 || att > getIconCount() )
    return(Sdm_False);

  return( list[att]->isDeleted() );
}

void
AttachArea::deleteSelectedAttachments(
				      SdmError &	mail_error
				      )
{
  Attachment **list = getList();
  int i;
  char buf[2048];
  int answer;

  int anyDeleted = 0;
  for (i = 0; i<getIconCount(); i++) {
    if (list[i]->isSelected() && !list[i]->isDeleted()) {

      // unselect it first.  Else, when undeleted it comes
      // off selected
      list[i]->unselect();
      
      // if delete is successful, then get the batch value and
      // update the delete count.  if not successful, post error dialog.
      //
      list[i]->deleteIt(mail_error);
      if (mail_error == Sdm_EC_Success) {
        if ( anyDeleted == 0 ) {
          anyDeleted = 1;
          _deleteBatch++;
        }
        list[i]->setBatch(_deleteBatch);
        _deleteCount++;
      } else {
        sprintf(buf, catgets(DT_catd, 12, 12, "Cannot delete attachment: %s"),
          list[i]->getLabel());
        answer = this->handleErrorDialog(catgets(DT_catd, 1, 81, "Mailer"), 
				     buf);
        mail_error = Sdm_EC_Success;  // reset error.
      }
    }
  }

  // Unmanage all.
  // Their positions need to get recomputed and the undeleted
  // ones get remanaged in manageList().

  for (i=0; i<getIconCount(); i++) {
    list[i]->unmanageIconWidget();
  }

  _cache_single_attachment = NULL;
  _attach_area_selection_state = AA_SEL_NONE;

  this->manageList();
}

void
AttachArea::undeleteLastDeletedAttachment()
{
  Attachment **list;
  int i;
  char buf[2048];
  int answer;
  SdmError mail_error;
  SdmBoolean  errorDetected = Sdm_False;

  if (_deleteCount == 0) {
    return;
  }

  list = getList();

  for (i=0; i<getIconCount(); i++) {
    if ( list[i]->isDeleted() &&
       list[i]->getDeletedBatch() == _deleteBatch ) 
    {
      // if undelete is successful, then update the delete count.  
      // if not successful, post error dialog and set flag so we
      // know that undelete did not work.
      //
      list[i]->undeleteIt(mail_error);
      if (mail_error == Sdm_EC_Success) {
        _deleteCount--;
      } else {
        sprintf(buf, catgets(DT_catd, 12, 13, "Cannot undelete attachment: %s"),
          list[i]->getLabel());
        answer = this->handleErrorDialog(catgets(DT_catd, 1, 81, "Mailer"), 
				     buf);
        errorDetected = Sdm_True;
        mail_error = Sdm_EC_Success;  // reset error.
      }
    }
  }

  // decrement the batch value only if the undelete worked for all
  // attachments that should have been undeleted.
  //
  if (errorDetected = Sdm_False) {
    _deleteBatch--;
  }

  // Unmanage all.
  // Their positions need to get recomputed and the deleted
  // ones get remanaged in manageList().

  for (i=0; i<getIconCount(); i++) {
    list[i]->unmanageIconWidget();
  }

  this->manageList();
}

void
AttachArea::unselectOtherSelectedAttachments(
					     Attachment *attachment
					     )
{
  if(_attach_area_selection_state == AA_SEL_NONE)
    return;

  if(_attach_area_selection_state == AA_SEL_ALL) {
    int i;
    Attachment **list;

    list = getList(); 

    for (i=0; i < getIconCount(); i++) 
      if (list[i]->isSelected() && list[i] != attachment) 
        list[i]->unselect();
  } 
  else if (_cache_single_attachment && 
	   (attachment != _cache_single_attachment)) {
    _cache_single_attachment->unselect();
    _cache_single_attachment = NULL;
  }

  if(attachment == NULL) {
    // Grey out the appropriate menu items in the RMW...
    _myOwner->owner()->all_attachments_deselected();
    _attach_area_selection_state = AA_SEL_NONE;
    _cache_single_attachment = NULL;
  }
}

void
AttachArea::addAttachmentActions(
				 char **actions,
				 int indx
				 )
{
  _myOwner->owner()->addAttachmentActions(
					  actions,
					  indx
					  );
}

void
AttachArea::setOwnerShell(
			  RoamMenuWindow *rmw
			  )
{
  _myRMW = rmw;
}

void
AttachArea::setOwnerShell(
			  ViewMsgDialog *vmd
			  )
{
  _myVMD = vmd;
}

void
AttachArea::setOwnerShell(
			  SendMsgDialog *smd
			  )
{
  _mySMD = smd;
}

Widget
AttachArea::ownerShellWidget()
{
  if (_myRMW) {
    return(_myRMW->baseWidget());
  }
  else if (_myVMD) {
    return(_myVMD->baseWidget());
  }
  else if (_mySMD) {
    return(_mySMD->baseWidget());
  }
  else {
    // Error out
  }
}
SdmBoolean
AttachArea::isOwnerShellVMD()
{
  if (_myVMD != NULL) {
    return Sdm_True;
  }
  else {
    return Sdm_False;
  }
}

SdmBoolean
AttachArea::isOwnerShellEditable()
{
  // only SMD is editable
  if (_mySMD != NULL) {
    return Sdm_True;
  }
  else {
    return Sdm_False;
  }
}

void
AttachArea::setPendingAction(
			     SdmBoolean bool
			     )
{
  _pendingAction = bool;
  if (bool) {
    _numPendingActions++;
  }
  else {
    if (_numPendingActions > 0) {
      _numPendingActions--;
    }
  }
}

void
AttachArea::resetPendingAction()
{

  _numPendingActions = 0;
}

int
AttachArea::getNumPendingActions()
{
  return(_numPendingActions);
}

void
AttachArea::selectAllAttachments()
{
  Attachment **list;
  int numAttachments = getIconCount();

  list = getList(); 
    
  if(list == NULL)
    return;

  // if there's only 1 attachment, select it and
  // add its actions to the menu bar...

  if (numAttachments == 1) {
    list[0]->primitive_select();
    list[0]->set_selected();
  }
  else {
    // More than 1 attachment.
    // Select them all. Don't enable their actions however.

    for (int i=0; i < numAttachments; i++) 
      list[i]->primitive_select();

    // Grey out the appropriate menu items in the RMW...
    _myOwner->owner()->all_attachments_selected();
    _cache_single_attachment = NULL;
    _attach_area_selection_state = AA_SEL_ALL;
  }
}

int
AttachArea::handleQuestionDialog(
				 char *title,
				 char *buf,
				 char * helpId
				 )
{
  DtMailGenDialog *dialog;
  int answer;

  if (_myRMW) {
    dialog = _myRMW->genDialog();
  }
  else if (_myVMD) {
    dialog = _myVMD->genDialog();
  }
  else if ( _mySMD) {
    dialog = _mySMD->genDialog();
  }
  else return(-1);

  if (!dialog) return(-1);

  dialog->setToQuestionDialog(
			      title,
			      buf);
  answer = dialog->post_and_return(helpId);
  return(answer);
}

int
AttachArea::handleErrorDialog(
			      char *title,
			      char *buf,
			      char *helpId
			      )
{
  DtMailGenDialog *dialog;
  int answer;

  if (_myRMW) {
    dialog = _myRMW->genDialog();
  }
  else if (_myVMD) {
    dialog = _myVMD->genDialog();
  }
  else if ( _mySMD) {
    dialog = _mySMD->genDialog();
  }
  else return(-1);

  if (!dialog) return(-1);

  dialog->setToErrorDialog(
			   title,
			   buf);
  answer = dialog->post_and_return(helpId);
  return(answer);
}

void
AttachArea::handlePostErrorDialog(SdmError& error, 
				  const char* help, 
				  const char* errorMessageText)
{
  DtMailGenDialog *dialog = (DtMailGenDialog*)0;

  if (_myRMW) {
    dialog = _myRMW->genDialog();
  }
  else if (_myVMD) {
    dialog = _myVMD->genDialog();
  }
  else if ( _mySMD) {
    dialog = _mySMD->genDialog();
  }

  if (dialog)
    dialog->post_error(error, help, errorMessageText);

  return;
}
