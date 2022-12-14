/*
 *+SNOTICE
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement bertween
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel wihtout
 *	Sun's specific written approval.  This documment and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)AskFirstCmd.C	1.15 11/30/95"
#endif


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
// AskFirstCmd.C
//////////////////////////////////////////////////////////
#include "AskFirstCmd.h"
#include "QuestionDialogManager.h"

#include <nl_types.h>
#include <stdlib.h>

extern nl_catd catd;

AskFirstCmd::AskFirstCmd ( char *name, 
			   char *label, 
			   int active ) : Cmd ( name, label, active )
{
    _dialog = NULL;
    _question = NULL;
    _dialogParentWidget = NULL;
    setQuestion ( catgets(catd, 1, 1,
	   "Do you really want to execute this command?"));
}

AskFirstCmd::~AskFirstCmd()
{
   if (_question)
      free(_question);
     
}

void AskFirstCmd::setQuestion ( char *str )
{
    if (_question)
	free (_question);
    _question = strdup ( str );
}

void AskFirstCmd::execute()
{
    char *name_str;
    char *label_str;
    
    name_str = (char *) name();
    label_str = (char *) getLabel();


    if (!_dialogParentWidget) return;

    if (!_dialog) {
	_dialog = new QuestionDialogManager(name_str);
    }
    
    _dialog->post(
		label_str,
		_question,
		_dialogParentWidget,
		(void *) this,
		&AskFirstCmd::yesCallback,
		&AskFirstCmd::cancelCallback);
}       

void AskFirstCmd::yesCallback ( void *clientData )
{
    AskFirstCmd *obj = (AskFirstCmd *) clientData;
    
    obj->doYesCallback();
 
}

void 
AskFirstCmd::cancelCallback ( void *)
{

}

void
AskFirstCmd::doYesCallback()
{

    // unmanage the dialog right away
   _dialog->unmanage();


   // Call the base class execute()
   // member function to do all the
   // usual processing of the command

   this->Cmd::execute();  

}
