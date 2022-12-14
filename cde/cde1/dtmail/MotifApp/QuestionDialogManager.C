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
#pragma ident "@(#)QuestionDialogManager.C	1.8 03/09/95"
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


///////////////////////////////////////////////////////////
// QuestionDialogManager.C: 
//////////////////////////////////////////////////////////
#include "QuestionDialogManager.h"
#include <Xm/Xm.h>
#include <Xm/MessageB.h>

#ifdef DEAD_WOOD
// Define an instance to be available throughout the framework.

QuestionDialogManager *theQuestionDialogManager = 
    new QuestionDialogManager ( "QuestionDialog", "QuestionDialog" );
#endif /* DEAD_WOOD */

QuestionDialogManager::QuestionDialogManager ( char   *name ) : 
                                 DialogManager ( name )
{
    // Empty
}

Widget QuestionDialogManager::createDialog ( Widget parent )
{
    Widget dialog = XmCreateQuestionDialog ( parent, _name, NULL, 0);
    
    XtVaSetValues ( dialog,
		   XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
		   NULL );
    
    return dialog;
}
