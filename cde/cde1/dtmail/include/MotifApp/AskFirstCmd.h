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


///////////////////////////////////////////////////////////////
// AskFirstCmd.h: Base class for Cmds that ask for confirmation
////////////////////////////////////////////////////////////////
#ifndef ASKFIRSTCMD_H
#define ASKFIRSTCMD_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)AskFirstCmd.h	1.12 11/30/95"
#endif

#include "Cmd.h"
#include "DialogManager.h"

class AskFirstCmd : public Cmd {
    
  private:
    
    // Callback for the yes choice on the dialog
    
    static void yesCallback ( void * );
    static void cancelCallback ( void *clientData );
    
    //  Derived classes should use setQuestion to change
    // the string displayed in the dialog
    
    char *_question;
    DialogManager *_dialog;

#ifndef CPLUSPLUS2_1
  protected:    

    Widget _dialogParentWidget;

    virtual void doit()   = 0;  // Specific actions must be defined    
    virtual void undoit()   = 0;  // Specific actions must be defined    
#endif

  public:
    
    AskFirstCmd ( char *, char *, int );
    virtual ~AskFirstCmd();
    
    void setQuestion ( char *str );
    
    virtual void execute(); // Overrides the Cmd member function

    virtual void doYesCallback();
    
    virtual const char *const className ()  { return "AskFirstCmd"; }
};
#endif
