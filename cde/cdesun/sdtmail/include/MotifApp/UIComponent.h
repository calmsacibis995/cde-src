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

///////////////////////////////////////////////////////////////
// UIComponent.h: Base class for all C++/Motif UI components
///////////////////////////////////////////////////////////////
#ifndef UICOMPONENT_H
#define UICOMPONENT_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)UIComponent.h	1.9 11/27/96"
#endif

#include <Xm/Xm.h>
// #include "BasicComponent.h"

class UIComponent {
    
private:
    
  // Interface between XmNdestroyCallback and this class
  static void widgetDestroyedCallback (Widget, XtPointer, XtPointer );

protected:
  char *_name;
  Widget _w;

  // Protect constructor to prevent direct instantiation
  UIComponent ( const char * );
    
  void installDestroyHandler(); // Easy hook for derived classes
    
  // Called by widgetDestroyedCallback() if base widget is destroyed
  virtual void widgetDestroyed(); 

  // Retrieve resources for this clsss from the resource manager
  void getResources ( const XtResourceList, const int );
    
public:

  virtual ~UIComponent();

  // Manage the entire widget subtree represented by this component.
  const Widget baseWidget() { return _w; }
  virtual void manage();
  virtual void unmanage();

  // Public access functions
  virtual const char *const className() { return "UIComponent"; }
};

#endif
