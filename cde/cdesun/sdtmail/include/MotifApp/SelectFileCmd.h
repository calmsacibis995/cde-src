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


///////////////////////////////////////////////////////////////////
// SelectFileCmd.h:  Allow the user to select a file interactively
///////////////////////////////////////////////////////////////////
#ifndef SELECTFILECMD_H
#define SELECTFILECMD_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)SelectFileCmd.h	1.17 01/09/97"
#endif

#include "NoUndoCmd.h"
#include <Xm/Xm.h>
#include <Xm/ToggleB.h>

typedef void (*FileCallback) ( void *, char * );
typedef void (*FileCallback2) ( Widget, XmFileSelectionBoxCallbackStruct * );

class SelectFileCmd : public NoUndoCmd {
    
  private:
    
    static void fileSelectedCallback ( Widget, XtPointer, XtPointer );
    static void cancelCallback ( Widget, XtPointer, XtPointer );
    FileCallback2 _defDirSearchProc;
    
  protected:
    
    void doit();              // Called by base class
    char * 	_ok_label;
    char *	_title;
    FileCallback _callback;   // Function to be called 
    FileCallback2 _dircallback;   // Function to be called 
    // when user selects a file.
    void        *_clientData; // Data provided by caller
    void        *_clientData2; // Data provided by caller
    
    Widget      _fileBrowser; // The Motif widget used to get file

    Widget	_parentWidget;  // Need it to determine who to parent
				// fileBrowser.

    XmString	_directory;	// The default directory to point to.
    
    virtual void fileSelected ( char * );
    
  public:
    
    SelectFileCmd (const char * name,
		   const char * label,
		   const char * title,
		   const char * ok_label,
		   int active,
		   FileCallback,
		   FileCallback2,
		   void *clientData,
		   void *clientData2,
		   Widget);
    ~SelectFileCmd ();

    void	setDirectory(char *);
    Widget	fileBrowser(void) { return _fileBrowser; }
    FileCallback2 getDefSearchProc() { return _defDirSearchProc; }
    void*   getClientData() { return _clientData; }
};
#endif
