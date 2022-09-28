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
// MenuBar.h: A menu bar, whose panes support items
//            that execute Cmd's

//////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// MODIFIED TO SUPPORT SUBMENUS - not described in Book
///////////////////////////////////////////////////////////


#ifndef MENUBAR_H
#define MENUBAR_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)MenuBar.h	1.19 04/01/97"
#endif

#include "UIComponent.h"
#include <Xm/RowColumn.h>

class Cmd;
class CmdList;

class MenuBar : public UIComponent {
  private:
    struct MenuBarContext {
      MenuBar *mb;
      Widget menupane;
      CmdList *list;
    };

  protected:
    virtual Widget createPulldown (Widget,
				   CmdList *,
				   Boolean,
				   unsigned char,
				   Boolean = FALSE);

    virtual Widget createPulldown (Widget,
				   CmdList *,
				   Widget *,
				   Boolean,
				   unsigned char,
				   Boolean = FALSE);
    
  public:
    static Boolean createPulldownWP(XtPointer client_data);
    
    MenuBar ( Widget, char *, unsigned char = XmMENU_BAR );
    MenuBar ( char *, Widget );
    
    // Create a named menu pane from a list of Cmd objects
    
    virtual Widget addCommands ( Widget *,
				 CmdList *,
				 Boolean = FALSE, 
				 unsigned char = XmMENU_BAR,
				 Boolean = FALSE); // use XtWorkProc

    virtual Widget addCommands ( CmdList *,
				 Boolean = FALSE,
				 unsigned char = XmMENU_BAR,
				 Boolean = FALSE); // use XtWorkProc

    virtual Widget addCommands ( Widget, CmdList *);

    virtual void addCommand ( Widget, Cmd *);

    virtual  void   removeCommands( Widget, CmdList *);

    virtual  void   removeCommand( Widget, int at);

    virtual  void   removeOnlyCommands( Widget, CmdList *);


    virtual void changeLabel ( Widget, int, char *);

    virtual void changeLabel(Widget, const char * wid_name, const char * label);

    virtual void rotateLabels ( Widget, int, int );

    virtual const char *const className() { return "MenuBar"; }
};
#endif   
