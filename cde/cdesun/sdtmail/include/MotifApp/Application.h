/*
 *+SNOTICE
 *
 *	$Revision: 1.3 $
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


////////////////////////////////////////////////////////////
// Application.h: 
////////////////////////////////////////////////////////////
#ifndef APPLICATION_H
#define APPLICATION_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)Application.h	1.17 01/21/97"
#endif

#include "UIComponent.h"
#include <sys/types.h>
#include <unistd.h>

class Application : public UIComponent {
    
    // Allow main and MainWindow to access protected member functions

#if (XlibSpecificationRelease>=5)
    friend int main ( int, char ** );
#else
    friend int main ( unsigned int, char ** );
#endif
    
    friend class MainWindow;  // MainWindow needs to call 

    // private functions for registration

  private:    
    
    // Functions for registering and unregistering toplevel windows
    
    void registerWindow ( MainWindow * );
    void unregisterWindow ( MainWindow * );
    
  protected:
    
    // Support commonly needed data structures as a convenience
    
    Display     *_display;
    XtAppContext _appContext;
    int		 _bMenuButton;
    
    // Functions to handle Xt interface
#if (XlibSpecificationRelease>=5)   
    virtual void initialize ( int *, char ** );  
#else
    virtual void initialize ( unsigned int *, char ** );  
#endif
    virtual void handleEvents();

    virtual void open_catalog();  
    inline void extractAndRememberEventTime( XEvent * );

    char   *_applicationClass;    // Class name of this application
    MainWindow  **_windows;       // top-level windows in the program
    int           _numWindows;
    long	_lastInteractiveEventTime;
    gid_t	_originalEgid;	  // startup effective gid
    gid_t	_originalRgid;	  // startup real gid
    
  public:
    
    Application ( char * );
    virtual ~Application();     
    
    // Functions to manipulate application's top-level windows
    
    void manage();
    void unmanage();
    void iconify();
    
    // Convenient access functions
    
    Display      *display()     { return _display; }
    XtAppContext  appContext()  { return _appContext; }
    int		  bMenuButton() { return _bMenuButton; }
    const char   *applicationClass()  { return _applicationClass; }
    virtual void shutdown() = 0;
    
    virtual const char *const className() { return "Application"; }
    long 	lastInteractiveEventTime(void) { return (_lastInteractiveEventTime); }

    // Functions to manipulate group execution privileges
    //
    gid_t	originalEgid(void) { return (_originalEgid); }
    gid_t	originalRgid(void) { return (_originalRgid); }
    void	enableGroupPrivileges(void) { (void) setgid(_originalEgid); }
    void	disableGroupPrivileges(void) { (void) setgid(_originalRgid); }

    // Added this to enable dynamic querying of the number of windows
    // an application has.  Useful to determine what to enquire when 
    //  user wants to quit - quit the the window or quit the session?
    // 

    int		num_windows();
    int		windowExists(MainWindow *window);
};

// Pointer to single global instance

extern Application *theApplication; 

#endif
