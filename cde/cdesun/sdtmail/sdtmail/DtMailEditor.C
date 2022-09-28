/*
 *+SNOTICE
 *
 *	$Revision: 1.42 $
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
 *	Copyright 1993, 1994, 1995, 1996 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)DtMailEditor.C	1.42 06/12/97"
#else
static char *sccs__FILE__ = "@(#)DtMailEditor.C	1.42 06/12/97";
#endif

#include <Xm/Form.h>
#include <Xm/SeparatoG.h>
#include <Dt/Dnd.h>
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Error.hh>
#include <SDtMail/String.hh>
#include <SDtMail/RepSWMR.hh>
#include "DtMailEditor.hh"
#include "AttachArea.h"
#include "Attachment.h"
#ifdef DTEDITOR
#include "DtEditor.hh"
#endif

#include "EUSDebug.hh"

#ifndef ABS
#define ABS(x) (((x) > 0) ? (x) : (-(x)))
#endif

#ifndef DRAG_THRESHOLD
#define DRAG_THRESHOLD 4
#endif

// The "buf" methods can be made to use either new/delete or the
// libsdtmail memory allocation functions - the latter provide for
// allocation against /dev/zero for large requests.

#define USEMEMREP	// use libsdtmail memory allocation functions

DtMailEditor::DtMailEditor(
    Widget parent,
    AbstractEditorParent *owner
) : UIComponent("DtMailEditor")
{
    _myOwner      = owner;
    
    // Create a manager widget (say a form) and set it to the private
    // variable _container. Parent private instances to this widget.
    // Expose only _container externally (for attachment stuff...)

    _w = XmCreateForm(parent, "DtMailEditor", NULL, 0);

    installDestroyHandler();

    XtVaSetValues(_w, 
			XmNbottomAttachment, XmATTACH_FORM, 
			XmNtopAttachment, XmATTACH_FORM, 
			XmNleftAttachment, XmATTACH_FORM, 
			XmNrightAttachment, XmATTACH_FORM, 
			NULL);

    _myTextEditor = new CDEM_DtWidgetEditor(_w, this);
    _myAttachArea = new AttachArea(_w, this, "AttachPane");

    _doingDrag = Sdm_False;
    _separator = NULL;
    _dragX = -1;
    _dragY = -1;
    _editable = Sdm_False;

}

DtMailEditor::~DtMailEditor()
{
    delete _myAttachArea;
    delete _myTextEditor;
}

void
DtMailEditor::initialize()
{
    Widget editor_widget;

    _myTextEditor->initialize();

    _separator = XtVaCreateManagedWidget("Sep1",
					 xmSeparatorGadgetClass,
					 _w,
					 XmNtopOffset, 1,
					 XmNbottomOffset, 1,
					 XmNrightAttachment, XmATTACH_FORM,
					 XmNleftAttachment, XmATTACH_FORM,
					 NULL);

    // Create an *UNMANAGED* attachArea.
    // If the message has an attachment, the DtMailEditor instance
    // will receive a manageAttachArea where it will adjust the
    // attachments and manage the attachArea accordingly 
    
    _myAttachArea->initialize();

    attachDropRegister();
    if (!_editable)
	attachDropDisable();

    editor_widget = _myTextEditor->get_editor();

    XtVaSetValues(editor_widget,
		  XmNtopAttachment,XmATTACH_FORM, 
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, _separator,
		  XmNrightAttachment,XmATTACH_FORM,
		  XmNrightOffset, 4,
		  XmNleftAttachment,XmATTACH_FORM, 
		  XmNleftOffset, 3,
		  NULL );

    XtVaSetValues(_separator,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, _myAttachArea->baseWidget(),
		  NULL);

    XtVaSetValues(_myAttachArea->baseWidget(),
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNrightOffset, 3,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNleftOffset, 5,
		  XmNbottomAttachment, XmATTACH_FORM,
		  NULL);
    

#if 0
    // Unmanage the attachArea.  If a message has attachments,
    // manageAttachArea() will get called by the consumer of this
    // class.

    this->unmanageAttachArea(); 
#endif
    XtManageChild(_w);
}

AbstractEditorParent *
DtMailEditor::owner()
{
    return (_myOwner);
}

Editor*
DtMailEditor::textEditor()
{
    return(_myTextEditor);
}

AttachArea*
DtMailEditor::attachArea()
{
    return(_myAttachArea);
}

Widget
DtMailEditor::container()
{
    return(_w);
}

void
DtMailEditor::setEditable(SdmBoolean bool)
{
    textEditor()->set_editable(bool);
    _editable = bool;
    if (_editable)
	attachDropEnable();
    else
	attachDropDisable();
}

void
DtMailEditor::manageAttachArea()
{
    _myAttachArea->manage();

    XtVaSetValues(_separator,
                  XmNbottomAttachment, XmATTACH_WIDGET,
                  XmNbottomWidget, _myAttachArea->baseWidget(),
                  NULL);

    XtVaSetValues(_myAttachArea->baseWidget(),
                XmNbottomAttachment, XmATTACH_FORM,
                NULL);
}

void
DtMailEditor::unmanageAttachArea()
{
    if (!XtIsManaged(_myAttachArea->sw())) return;

    _myAttachArea->unmanage();
}


// attachTransferCallback
//
// Handles the transfer of data that is dropped on the attachment list.
// The data is turned into an attachment and appended to the list.
//
void
DtMailEditor::attachTransferCallback(
    Widget	/* widget */,
    XtPointer	client_data,
    XtPointer	call_data)
{
    DtDndTransferCallbackStruct *transferInfo =
				(DtDndTransferCallbackStruct *) call_data;
    DtDndContext *dropData = transferInfo->dropData;
    int		 numItems = dropData->numItems, ii;
    DtMailEditor *editor = (DtMailEditor *) client_data;
    SdmError	 mail_error;
    char 	 *attachname;

    DebugPrintf(3, "In DtMailEditor::attachTransferCallback\n");

    // Initialize mail_error.
    mail_error.Reset();

    switch (transferInfo->dropData->protocol) {

	case DtDND_FILENAME_TRANSFER:

	    // Loop through the dropped files and turn each
	    // into an attachment.

	    for (ii = 0; ii < numItems; ii++) {
		editor->owner()->add_att(dropData->data.files[ii]);
	    }
	    break;

	case DtDND_BUFFER_TRANSFER:

	    // Loop through the dropped buffers and turn each
	    // into an attachment.

	    for (ii = 0; ii < numItems; ii++) {

    // note: we need to specify the size when constructing the
    // buffer below.  otherwise, if there are valid null characters
    // in bp (eg. audio file) the contents is truncated.
		SdmString buf((const char *)dropData->data.buffers[ii].bp, 
                    dropData->data.buffers[ii].size);
		attachname = dropData->data.buffers[ii].name;

		if (!attachname)
		    attachname = "Untitled";

		editor->owner()->add_att(attachname, buf);
	    }
	    break;

	default:
	    transferInfo->status = DtDND_FAILURE;
	    return;
    }
}

// attachDropRegister
//
// Register the attachment list to accept drops of buffer and files
//
void
DtMailEditor::attachDropRegister()
{
    static XtCallbackRec transferCBRec[] = { 
	{&DtMailEditor::attachTransferCallback, NULL}, {NULL, NULL} };

    // Pass the DtMailEditor object (this) as clientData.
    transferCBRec[0].closure = (XtPointer) this;

    DtDndVaDropRegister(_myAttachArea->baseWidget(),
	DtDND_FILENAME_TRANSFER | DtDND_BUFFER_TRANSFER,
	(unsigned char)(XmDROP_COPY), transferCBRec,
	DtNtextIsBuffer, Sdm_True,
	NULL);
}

// attachDropEnable
//
// Enable the attachment list for drops by restoring the operation
//
void
DtMailEditor::attachDropEnable()
{
    Arg	args[1];

    XtSetArg(args[0], XmNdropSiteOperations, XmDROP_MOVE | XmDROP_COPY);
    XmDropSiteUpdate(_myAttachArea->baseWidget(), args, 1);
}

// attachDropDisable
//
// Disable the attachment list for drops by setting the operation to noop
//
void
DtMailEditor::attachDropDisable()
{
    Arg	args[1];

    XtSetArg(args[0], XmNdropSiteOperations, XmDROP_NOOP);
    XmDropSiteUpdate(_myAttachArea->baseWidget(), args, 1);
}

// attachConvertCallback
//
// Provides the selected attachments for the drag
//
void
DtMailEditor::attachConvertCallback(
    Widget	/* dragContext */,
    XtPointer	clientData,
    XtPointer	callData)
{
  DtDndConvertCallbackStruct *convertInfo =
    (DtDndConvertCallbackStruct *) callData;
  DtDndBuffer		*buffers = convertInfo->dragData->data.buffers;
  DtMailEditor	*editor = (DtMailEditor *) clientData;
  int			numIcons = editor->attachArea()->getIconCount();
  Attachment		**list = editor->attachArea()->getList();
  int 		ii, current = 0;
  char		*name = NULL;
  XmString		str;
  SdmError		mail_error;

  mail_error.Reset();

  DebugPrintf(3, "In DtMailEditor::attachConvertCallback\n");

  switch(convertInfo->reason) {
  case DtCR_DND_CONVERT_DATA:
    for (ii = 0; ii < numIcons; ii++) {
      if (!list[ii]->isDeleted() && list[ii]->isSelected()) {
	buffers[current].bp = list[ii]->getContents(mail_error);
	if (mail_error != Sdm_EC_Success) {
	  // The get contents failed -  set the return status to failure
	  convertInfo->status = DtDND_FAILURE;
	  editor->attachArea()->handlePostErrorDialog(mail_error, DTMAILHELPERROR, "An error occurred while processing the contents of the attachment.\n");
	}
	buffers[current].size = (int)list[ii]->getContentsSize();
	str = list[ii]->getLabel();
	XmStringGetLtoR(str,
			XmSTRING_DEFAULT_CHARSET, 
			&buffers[current].name);
	XmStringFree(str);
	current++;
      }
    }
    break;

  case DtCR_DND_CONVERT_DELETE:
    editor->attachArea()->deleteSelectedAttachments(mail_error);
    if (mail_error != Sdm_EC_Success) {
      convertInfo->status = DtDND_FAILURE;
    }
    break;

  default:
    convertInfo->status = DtDND_FAILURE;
  }
}

// attachDragFinishCallback
//
// Clean up from the convert callback and restore state
//
void
DtMailEditor::attachDragFinishCallback(
    Widget	/* widget */,
    XtPointer	clientData,
    XtPointer	callData)
{
    DtDndDragFinishCallbackStruct *finishInfo =
		(DtDndDragFinishCallbackStruct *) callData;
    DtDndContext 	*dragData = finishInfo->dragData;
    DtMailEditor *editor = (DtMailEditor *) clientData;
    int		 ii;

    DebugPrintf(3, "In DtMailEditor::attachDragFinishCallback\n");

    editor->setDoingDrag(Sdm_False);
    editor->setDragX(-1);
    editor->setDragY(-1);

    if (editor->editable())
	editor->attachDropEnable();

    for (ii = 0; ii < dragData->numItems; ii++) {
	XtFree((char *)dragData->data.buffers[ii].name);
    }
}

void
DtMailEditor::attachDragStart( Widget widget, 
				XEvent *event)
{
    static XtCallbackRec convertCBRec[] = {
	{&DtMailEditor::attachConvertCallback, NULL}, {NULL, NULL} };
    static XtCallbackRec dragFinishCBRec[] = {
	{&DtMailEditor::attachDragFinishCallback, NULL}, {NULL, NULL} };
    int 	itemCount;

    convertCBRec[0].closure = (XtPointer) this;
    dragFinishCBRec[0].closure = (XtPointer) this;

    attachDropDisable();

    // Count the number of items to be dragged.
    itemCount = attachArea()->getSelectedIconCount();

    setDoingDrag(Sdm_True);

    if (DtDndVaDragStart(widget, event, DtDND_BUFFER_TRANSFER, itemCount,
		(unsigned char)(XmDROP_COPY), convertCBRec, dragFinishCBRec,
//		DtNsourceIcon, dragIcon,
		NULL)
	    == NULL) {
	    
	    DebugPrintf(3, "DragStart returned NULL.\n");
    }
}

void
DtMailEditor::attachDragMotionHandler(
    Widget	widget,
    XEvent	*event)
{
    int		diffX, diffY;

    if (!doingDrag()) {
	// If the drag is just starting, set initial button down coordinates.
	if (dragX() == -1 && dragY() == -1) {
	    setDragX(event->xmotion.x);
	    setDragY(event->xmotion.y);
	}

	// Find out how far the pointer has moved since the button press.
	diffX = dragX() - event->xmotion.x;
	diffY = dragY() - event->xmotion.y;

	if ((ABS(diffX) >= DRAG_THRESHOLD) ||
	    (ABS(diffY) >= DRAG_THRESHOLD)) {
		attachDragStart(widget, event);
	}
    }
}

#ifdef notdef
void
DtMailEditor::attachDragSetup()
{
    SdmBoolean	btn1_transfer;
    Widget	widget = _myAttachArea->baseWidget();

    DebugPrintf(3, "In DtMailEditor::attachDragSetup()\n");

    XtAddEventHandler(widget, Button1MotionMask, Sdm_False,
		(XtEventHandler)&DtMailEditor::attachDragMotionHandler,
		(XtPointer)this);

    XtVaGetValues(
	(Widget)XmGetXmDisplay(XtDisplayOfObject(widget)),
	"enableBtn1Transfer", &btn1_transfer,
	NULL);

    if (!btn1_transfer) {
	XtAddEventHandler(widget, Button2MotionMask, Sdm_False,
		(XtEventHandler)&DtMailEditor::attachDragMotionHandler,
		(XtPointer)this);
    }
}
#endif

int
DtMailEditor::stripCRLF(char **buffer, const char * buf, const unsigned long len)
{
  char * out = *buffer;

  for (const char * in = buf; in < (buf + len);) {
    if (*in == '\r') {
      in += 1;
    }

    *out++ = *in++;
  }
  *out = 0;
  return(out-*buffer);
}

void
DtMailEditor::deleteBuf(char **buffer)
{
  assert(*buffer);
#if defined(USEMEMREP)
  SdmMemoryRepository::_ODel(*buffer,  Sdm_MM_AnonymousAllocation);
#else
  delete [] *buffer;
#endif
  *buffer = NULL;
}

void
DtMailEditor::needBuf(char **buffer, unsigned long *buflen, unsigned long newlen, int zeroEntireBuffer)
{
  if ((newlen > *buflen)
      || (newlen <= 16384 && *buflen > 16384)
      || ((newlen > 16384) && (*buflen-newlen)>16384)) {

    // Need a different sized buffer.

    if (*buffer) {
      deleteBuf(buffer);
    }

#if defined(USEMEMREP)
    *buffer = (char *)SdmMemoryRepository::_ONew(newlen, Sdm_MM_AnonymousAllocation);
#else
    *buffer = new char[newlen];
#endif
    if (!*buffer) {
      *buflen = 0;
      return;
    }
    *buflen = newlen;
    **buffer = '\0';
  } 
  else {
    // Clear buffer content -- get ready for new data
    if (*buffer) {
      if (zeroEntireBuffer)
	memset(*buffer, 0, (unsigned int)*buflen);	
      else
	**buffer = '\0';
    }
  }
}

void
DtMailEditor::showAttachArea()
{
    this->manageAttachArea();

}

void
DtMailEditor::hideAttachArea()
{
    this->unmanageAttachArea();
}
