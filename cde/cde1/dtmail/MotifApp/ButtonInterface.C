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
#pragma ident "@(#)ButtonInterface.C	1.13 03/03/95"
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


//////////////////////////////////////////////////////////////
// ButtonInterface.C: A push button interface to a Cmd object
///////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <ctype.h>
#include "ButtonInterface.h"
#include <Xm/PushB.h>
#include "Help.hh"
#include "Cmd.h"

ButtonInterface::ButtonInterface ( Widget parent, 
				  Cmd *cmd ) : CmdInterface ( cmd )
{

    // We need to generate a button name that doesn't have illegal characters.
    //
    char w_name[200];
    mapName(_name, w_name);

    XmString label = XmStringCreateLocalized(_cmd->getLabel());
    _w = XtVaCreateWidget (w_name, 
			 xmPushButtonWidgetClass,
			 parent,
			 XmNlabelString, label, NULL);
    XmStringFree(label);

    printHelpId("_w", _w);
    // XtAddCallback(_w, XmNhelpCallback, HelpCB, helpId);
    // free(helpId);


    installDestroyHandler();
    
    // The _active member is set when each instance is registered
    // with an associated Cmd object. Now that a widget exists,
    // set the widget's sensitivity according to its active state.
    
    if ( _active )
	activate();     
    else
	deactivate();   

#ifdef GNU_CC  // No idea what the right ifdef is for automatically recognizing g++
    
    // G++ reportedly doesn't like the form expected by cfront. I'm
    // told this will work, but I haven't tested it myself.
    
    XtAddCallback ( _w,  
		   XmNactivateCallback, 
		   executeCmdCallback,
		   (XtPointer) this );  
#else

    XtAddCallback ( _w,  
		   XmNactivateCallback, 
		   &CmdInterface::executeCmdCallback,
		   (XtPointer) this );  
#endif
    
}

#ifndef CAN_INLINE_VIRTUALS
ButtonInterface::~ButtonInterface(void)
{
}
#endif /* ! CAN_INLINE_VIRTUALS */

void
ButtonInterface::mapName(const char * input, char * output)
{
    strcpy(output, input);
    for (char * cur = output; *cur; cur++) {
	if (isspace(*cur) || *cur == ',') {
	    *cur = '_';
	    continue;
	}

	if (*cur == '.' || *cur == ':') {
	    *cur = 0;
	    break;
	}
    }
}
