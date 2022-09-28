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

#ifndef ATTACHMENT_H
#define ATTACHMENT_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)Attachment.h	1.10 02 Mar 1994"
#endif

#include <Dt/Dts.h>
#include <Dt/Action.h>
#include <DtMail/DtMail.hh>
#include <DtMail/DtVirtArray.hh>

#include "UIComponent.h"
#include "AttachArea.h"

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
	
    Attachment ( AttachArea *, String, DtMail::BodyPart *, int);

    virtual			~Attachment();	// Destructor
    virtual void		initialize();
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
    void 			*getContents();


    unsigned long		getContentsSize() 	
					{ return ( _myContentsSize ); }

    Dimension 			getWidth() 	{ return ( _attachmentWidth ); }
    Dimension 			getHeight() 	{ return ( _attachmentHeight );}
    Position 			getX() 		{ return ( _positionX ); }
    Position 			getY() 		{ return ( _positionY );}
    int				getRow()	{ return ( _row ); }

    Boolean 			isManaged() 	{ return ( XtIsManaged(_w) ); }
    Boolean 			isDeleted() 	{ return ( _deleted ); }
    AttachArea*			parent() 	{ return ( _parent ); }
    Widget			getIconWidget() { return ( _w); }
    void			manageIconWidget(void);
    void			unmanageIconWidget(void);

    String 			getSaveAsFilename()
						{ return ( _saveAsFilename ); }
    char* 			getCeName() 	{ return ( _ce_name ); }
    unsigned short		getType() 	{ return ( _type ); }
    String 			getSubType() 	{ return ( _subtype ); }
    DtMail::BodyPart		*getBodyPart()	{ return ( _body_part ); }
    Boolean			isBinary()   	{ return ( _binary ); }
    Boolean			isSelected()	{ return ( _selected ); }


    void			saveToFile(DtMailEnv & error,char *filename);
    virtual const char *const 	className() 	{ return ( "Attachment" ); }

    // Mutators
    void 			setLabel( XmString str );	

    void	 		setX(Position);
#ifdef DEAD_WOOD
    void	 		setY(Position);
    void	 		setRow(int);
#else /* ! DEAD_WOOD */
    void	 		setY(Position);
    void	 		setRow(int row) { _row = row; }
#endif /* ! DEAD_WOOD */

    void	 		deleteIt();
    void 			undeleteIt();
    void 			setSaveAsFilename(String str) 
						{ ( _saveAsFilename = str); }

    void 			set_binary(Boolean binary) 
						{ ( _binary = binary); }

    void          		handleDoubleClick();

    void			name_to_type();
    void	   		setAttachArea(AttachArea *);
#ifdef DEAD_WOOD
    Boolean        		check_if_binary(String, unsigned long);
#endif /* DEAD_WOOD */

    // SR -- Added methods below
    
    void			primitive_select(); 
    void			set_selected();
    Boolean			is_selected();

    void			setContents();

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
    DtMail::BodyPart *_body_part;	// Pointer to back end body part.

    Dimension      _attachmentWidth;	// The width of the attachment
    Dimension      _attachmentHeight;	// The height of the attachment
    Position       _positionX;		// The X Position
    Position       _positionY;		// The Y Position

    Boolean        _deleted;		// True is it has been deleted
    Boolean	   _selected;		// True if selected.
    int		   _row;		// Which row this is displayed in
    String         _saveAsFilename;	// Name for Save As
    int		   _index;		// Index in attachArea
    char          *_ce_name;
    char          *_ce_type;
    unsigned short _type;
    String	   _subtype;
    Boolean	   _binary;

    Boolean	   _executable;
    Boolean	   _haveContents;
    const void	  *_myContents;
    unsigned long  _myContentsSize;
    char *	   _myType;
    Boolean	   _canKillSelf;
    
    char **	   _myActionsList;
    DtVirtArray<DtActionInvocationID> _myActionIds;
    DtMailObjectKey	_key;

};

#endif
