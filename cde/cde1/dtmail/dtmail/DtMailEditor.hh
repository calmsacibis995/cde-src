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

#ifndef DTMAILEDITOR_H
#define DTMAILEDITOR_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)DtMailEditor.hh	1.20 17 Feb 1994"
#endif

#include <Xm/Xm.h>
#include "Editor.hh"
#include "AttachArea.h"


class DtMailEditor : public UIComponent {

  public:	

    DtMailEditor(
		Widget,
		AbstractEditorParent *
		);
    virtual ~DtMailEditor();

    void	initialize();
    AbstractEditorParent *owner(); 
    
    Editor	*textEditor();
    AttachArea  *attachArea();

    Widget	container();

    void	showAttachArea();
    void	hideAttachArea();
    void	manageAttachArea();
    void	unmanageAttachArea();
    void	setMsgHnd( DtMail::Message *);
    static void attachTransferCallback(Widget, XtPointer, XtPointer );
    void	attachDropRegister();
    void	attachDropEnable();
    void	attachDropDisable();
    static void attachConvertCallback(Widget, XtPointer, XtPointer);
    static void attachDragFinishCallback(Widget, XtPointer, XtPointer);
    void	attachDragStart(Widget, XEvent *);
    void 	attachDragMotionHandler(Widget, XEvent *);
    //void	attachDragSetup();
    void	setEditable(Boolean);
    Boolean	editable() { return _editable; }
    Boolean	doingDrag() { return _doingDrag; }
    void	setDoingDrag(Boolean doingDrag) { _doingDrag = doingDrag; }
    void	setDragX(int n) { _dragX = n; }
    void	setDragY(int n) { _dragY = n; }
    int		dragX() { return _dragX; }
    int		dragY() { return _dragY; }

	// Routines to null terminate buffer.
    void needBuf(char **, unsigned long *, unsigned long len);
    void stripCRLF(char **, const char * buf, const unsigned long len);

  private:

    DtMail::Message *_msgHandle;
    Editor	*_myTextEditor;
    AttachArea	*_myAttachArea;
    Widget	_container;
    Widget	_separator;

    Boolean	_editable;
    Boolean	_showAttachArea;
    Boolean	_doingDrag;
    int		_dragX;
    int		_dragY;

	// Can be RMW or VMD or SMD
    AbstractEditorParent *_myOwner;	

};

#endif // DTMAILEDITOR_HH
