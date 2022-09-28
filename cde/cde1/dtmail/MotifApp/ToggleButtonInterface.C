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
#pragma ident "03/09/95 95/03/09"
#endif


//////////////////////////////////////////////////////////////
// ToggleButtonInterface.C: A toggle button interface to a Cmd object
///////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <ctype.h>
#include "Cmd.h"
#include "ToggleButtonInterface.h"
#include <Xm/ToggleB.h>
#include "Help.hh"

ToggleButtonInterface::ToggleButtonInterface ( 
    Widget parent, 
    Cmd *cmd 
) : CmdInterface ( cmd )
{

    // We need to generate a button name that doesn't have illegal characters.
    //
    char w_name[200];
    strcpy(w_name, _name);
    for (char * cur = w_name; *cur; cur++) {
	if (isspace(*cur) || *cur == ',') {
	    *cur = '_';
	    continue;
	}

	if (*cur == '.') {
	    *cur = 0;
	    break;
	}
    }

    XmString label = XmStringCreateLocalized(cmd->getLabel());

    _w = XtVaCreateWidget (w_name, 
		xmToggleButtonWidgetClass,
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
		   XmNvalueChangedCallback, 
		   executeCmdCallback,
		   (XtPointer) this );  
#else

    XtAddCallback ( _w,  
		   XmNvalueChangedCallback,
		   &CmdInterface::executeCmdCallback,
		   (XtPointer) this );  
#endif
    
}

#ifndef CAN_INLINE_VIRTUALS
ToggleButtonInterface::~ToggleButtonInterface (void)
{
}
#endif /* ! CAN_INLINE_VIRTUALS */
