/*
 *+SNOTICE
 *
 *	$Revision: 1.54 $
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

#ifndef ATTACHMENT_H
#define ATTACHMENT_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)Attachment.h	1.54 06/12/97"
#endif

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/MessageBody.hh>
#include <Dt/Dts.h>
#include <Dt/Action.h>
#include <DtVirtArray.hh>

#include "UIComponent.h"
#include "AttachArea.h"
#include "DtMailTypes.hh"

#define MARGIN 2

typedef enum { ROAM_CACHED, ROAM_LOCAL, ROAM_AVAILABLE, ROAM_UNAVAILABLE } ACState;

typedef enum { DONTKNOW, EXECUTABLE, NOTEXECUTABLE } ExecState;

class Icon;
class Attachment;

class ActionCallback {
    friend Attachment;	
  public:	
    ActionCallback(DtMailObjectKey, Attachment *);
    ~ActionCallback();
    
  private:
    DtMailObjectKey _myKey;
    Attachment *_myAttachment;
};

class Attachment : public UIComponent {

  public:	

    // This version takes a filename as a parameter
	
    Attachment ( AttachArea *, SdmString, SdmMessageBody *, int);

    virtual			~Attachment();	// Destructor
    virtual void		initialize(SdmBoolean considerBandwidth = Sdm_False);
    void			action( 
				       DtActionInvocationID, 
				       DtActionArg *,
				       int,
				       int );
    void			invokeAction(int);

    Attachment::operator==( const Attachment & );

    // Accessors
    //
    // Note: functions calling getLabel are responsible for freeing
    //       the XmString it returns
    //
    XmString 			getLabel() 	{ return ( XmStringCopy ( _label ) ); }
    void			rename(XmString);
    void 			*getContents(SdmError& mail_error);


    unsigned long		getContentsSize() 	
					{ return ( _myContentsSize ); }

    Dimension 			getWidth() 	{ return ( _attachmentWidth ); }
    Dimension 			getHeight() 	{ return ( _attachmentHeight );}
    Position 			getX() 		{ return ( _positionX ); }
    Position 			getY() 		{ return ( _positionY );}
    int				getRow()	{ return ( _row ); }

    SdmBoolean 			isManaged() 	{ return ( (SdmBoolean)XtIsManaged(_w) ); }
    SdmBoolean 			isDeleted() 	{ return ( _deleted ); }
    int				getDeletedBatch() { return ( _deletedBatch ); }
    AttachArea*			parent() 	{ return ( _parent ); }
    Widget			getIconWidget() { return ( _w); }
    void			manageIconWidget(void);
    void			unmanageIconWidget(void);

    SdmString 			getSaveAsFilename()
						{ return ( _saveAsFilename ); }
    SdmMsgStrType		getType();
    SdmString 			getSubType();
    SdmMessageBody		*getBodyPart()	{ return ( _body_part ); }
    SdmBoolean			isBinary()   	{ return ( _binary ); }
    SdmBoolean			isSelected()	{ return ( _selected ); }


    void			saveToFile(SdmError & error,char *filename);
    virtual const char *const 	className() 	{ return ( "Attachment" ); }

    // Mutators
    void 			setLabel( XmString str );	

    void	 		setX(Position);
    void	 		setY(Position);
    void	 		setRow(int row) { _row = row; }

    void			setBatch(int deleteBatch);
    void	 		deleteIt(SdmError &mail_error);
    void 			undeleteIt(SdmError &mail_error);
    void 			setSaveAsFilename(SdmString str) 
						{ ( _saveAsFilename = str); }

    void 			set_binary(SdmBoolean binary) 
						{ ( _binary = binary); }

    void          		handleDoubleClick();

    void			name_to_type();
    void	   		setAttachArea(AttachArea *);
#ifdef DEAD_WOOD
    SdmBoolean        		check_if_binary(SdmString, unsigned long);
#endif /* DEAD_WOOD */

    // SR -- Added methods below
    
    void			primitive_select(); 
    void			set_selected();
    SdmBoolean			is_selected();

    char *			getDtType(SdmBoolean approximate = Sdm_False);
    void			setContents(SdmError& mail_error);

    void			unselect();

    void			quit();

    void			registerAction(DtActionInvocationID);
    void			unregisterAction(DtActionInvocationID);
  protected:
    static void    actionCallback( 
			  DtActionInvocationID	id,
			  XtPointer	client_data,
			  DtActionArg	*actionArgPtr,
			  int		actionArgCount,
			  int		status
		  );

  private:

    Icon*	   myIcon;		// my Icon instance

    Pixel          _background;		// The background color
    Pixel          _foreground;		// The foreground color
    AttachArea    *_parent;		// The Parent class
    XmString         _label;		// The name (no absolute path)
    SdmMessageBody *_body_part;	// Pointer to back end body part.

    Dimension      _attachmentWidth;	// The width of the attachment
    Dimension      _attachmentHeight;	// The height of the attachment
    Position       _positionX;		// The X Position
    Position       _positionY;		// The Y Position

    SdmBoolean     _deleted;		// True is it has been deleted
    SdmBoolean	   _selected;		// True if selected.
    int		   _deletedBatch;	// What "delete batch" was this att. part of
    int		   _row;		// Which row this is displayed in
    SdmString      _saveAsFilename;	// Name for Save As
    int		   _index;		// Index in attachArea
    unsigned short _type;
    SdmString	   _subtype;
    SdmBoolean	   _binary;

    SdmBoolean	   _executable;
    SdmString      _myContents;
    unsigned long  _myContentsSize;
    char *         _cdeDataType;
    char *         _approxDataType;
    SdmBoolean	   _canKillSelf;
    
    char **	   _myActionsList;
    DtVirtArray<DtActionInvocationID> _myActionIds;
    DtMailObjectKey	_key;

};

#endif
