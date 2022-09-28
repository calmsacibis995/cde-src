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
// WorkingDialogManager.h
//////////////////////////////////////////////////////////
#ifndef WORKINGDIALOGMANAGER_H
#define WORKINGDIALOGMANAGER_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)WorkingDialogManager.h	1.9 03/12/94"
#endif

#include "DialogManager.h"

class PixmapCycler;

class WorkingDialogManager : public DialogManager {
    
  protected:
    
    Widget createDialog ( Widget );
    PixmapCycler *_busyPixmaps; // Source of animated pixmap sequence
    
    XtIntervalId _intervalId;   // ID of the last timeout
    
    static void unpostCallback ( Widget, XtPointer, XtPointer );
    static void timerCallback ( XtPointer, XtIntervalId * );    
    
    void timer ( );
    
  public:
    
    WorkingDialogManager ( char * );
    
    virtual Widget post (char *,
			 char *, 
			 void *clientData      = NULL,
			 DialogCallback ok     = NULL,
			 DialogCallback cancel = NULL,
			 DialogCallback help   = NULL );

    virtual Widget post (char *,
			 char *, 
			 Widget ,
			 void *clientData      = NULL,
			 DialogCallback ok     = NULL,
			 DialogCallback cancel = NULL,
			 DialogCallback help   = NULL );
    
    void unpost();    // Remove the dialog from the screen
    
    void updateMessage ( char * );  // Change the text in the dialog
};

extern WorkingDialogManager *theWorkingDialogManager;

#endif
