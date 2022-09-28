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

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//         This example code is from the book:
//
//           Object-Oriented Programming with C++ and OSF/Motif
//         by
//           Douglas Young
//           Prentice Hall, 1992
//           ISBN 0-13-630252-1	
//
//         Copyright 1991 by Prentice Hall
//         All Rights Reserved
//
//  Permission to use, copy, modify, and distribute this software for 
//  any purpose except publication and without fee is hereby granted, provided 
//  that the above copyright notice appear in all copies of the software.
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////
// DialogManager.h: A base class for cached dialogs
//////////////////////////////////////////////////////////
#ifndef DIALOGMANAGER_H
#define DIALOGMANAGER_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)DialogManager.h	1.10 03/12/94"
#endif

#include "UIComponent.h"
#include "DialogCallbackData.h"

class DialogManager : public UIComponent {
    
  private:
    

    
    static void destroyTmpDialogCallback ( Widget, 
					  XtPointer, 
					  XtPointer );
    
  void cleanup ( Widget, DialogCallbackData* );


    
  protected:

  void forceUpdate( Widget );
    
  // Called to get a new dialog

    Widget getDialog(); 
    Widget getDialog(Widget );
    
    virtual Widget createDialog ( Widget ) = 0;   

    static void okCallback ( Widget, 
			     XtPointer, 
			    XtPointer );
    
    static void cancelCallback ( Widget, 
				XtPointer, 
				XtPointer );
    
    static void helpCallback ( Widget, 
			      XtPointer, 
			      XtPointer );

    
  public:
    
    DialogManager ( char * );
    
    virtual Widget post (
			 char *,
			 char *, 
			 Widget w,
			 void *clientData      = NULL,
			 DialogCallback ok     = NULL,
			 DialogCallback cancel = NULL,
			 DialogCallback help   = NULL );

    virtual Widget post (
			 char *,
			 char *, 
			 void *clientData      = NULL,
			 DialogCallback ok     = NULL,
			 DialogCallback cancel = NULL,
			 DialogCallback help   = NULL );

    virtual int	   post_and_return(
				char *, 
				char *,
				Widget);
    virtual int	   post_and_return(
				char *, 
				char *, 
				char *,
				Widget);
    virtual int	   post_and_return(
				char *, 
				char *, 
				char *, 
				char *,
				Widget);

};
#endif
