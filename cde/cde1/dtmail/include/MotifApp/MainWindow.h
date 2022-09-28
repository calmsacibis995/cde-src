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


////////////////////////////////////////////////////////////////////
// MainWindow.h: Support a toplevel window
////////////////////////////////////////////////////////////////////
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)MainWindow.h	1.14 08/17/94"
#endif

#include "UIComponent.h"

class MainWindow : public UIComponent {
    
protected:
    
    Widget   _main;        // The XmMainWindow widget
    Widget   _workArea;    // Widget created by derived class
    Pixmap	_icon;
    GC		_icon_invert;
    GC		_window_invert;
    int		_last_state;
    
    // Derived classes must define this function to 
    // create the application-specific work area.
    
    virtual Widget createWorkArea ( Widget ) = 0;

    virtual void getIconColors(Pixmap & fore, Pixmap & back);
    
  public:
    
    MainWindow ( char * );   // Constructor requires only a name
    virtual ~MainWindow();
    
    // The Application class automatically calls initialize() 
    // for all registered main window objects
    
    virtual void initialize();
    
    virtual void manage();   // popup the window
    virtual void unmanage(); // pop down the window
    virtual void iconify();
    virtual void setIconTitle(const char * title);
    virtual void setIconName(const char * name);
    virtual void title(const char *);
    virtual void flash(const int count);
    virtual void quit()=0;
    virtual Boolean isIconified();

    virtual void busyCursor(void);
    virtual void normalCursor(void);

    virtual void setStatus(const char *);
    virtual void clearStatus(void);

    virtual void propsChanged(void) = 0;

  private:
    static void quitCallback( Widget, XtPointer, XmAnyCallbackStruct * );

    static void flashCallback(XtPointer, XtIntervalId *);
    void doFlash(XtIntervalId *);

    int		_flashing;
};
#endif
