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

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)NoOpCmd.C	1.14 03/03/95"
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
// NoOpCmd.C: Example, dummy command class
//////////////////////////////////////////////////////////
#include <stdio.h>
#include "EUSDebug.hh"
#include "NoOpCmd.h"
#include "Application.h"

void okCallback (void *);

NoOpCmd::NoOpCmd ( char *name, 
		   char *label,
		   int active ) : Cmd ( name, label, active )
{

}

void NoOpCmd::doit()
{
    // Just print a message that allows us to trace the execution
    
    char *buf = new char[1024];
    char *tmpName;

    tmpName = (char *) name();
    
    sprintf(buf, "%s : Sorry, not implemented.", tmpName);

    theInfoDialogManager->post(tmpName, buf, (void *) this,
			       okCallback);

//    this->setQuestion(buf);
//
//    this->execute();

    delete buf;

//    cout <<  name() << ":" << "doit\n" << flush;
}      

void NoOpCmd::undoit()
{
    // Just print a message that allows us to trace the execution

    DebugPrintf(1, "%s:\n", name());
}       


void okCallback( void *)
{
    
}

#ifndef CAN_INLINE_VIRTUALS
LabelCmd::~LabelCmd(void)
{
}

void
LabelCmd::doit(void)
{
    return;
}

void
LabelCmd::undoit(void)
{
    return;
}

const char *const
LabelCmd::className(void)
{
    return "LabelCmd";
}
#endif /* ! CAN_INLINE_VIRTUALS */
