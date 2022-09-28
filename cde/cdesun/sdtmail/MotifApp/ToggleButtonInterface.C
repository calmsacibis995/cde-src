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
#pragma ident "03/28/97 97/03/28"
#endif

// ToggleButtonInterface.C: A toggle button interface to a Cmd object

#include <stdlib.h>
#include <ctype.h>
#include "Cmd.h"
#include "ToggleButtonInterface.h"
#include <Xm/ToggleB.h>
#include "Help.hh"

ToggleButtonInterface::ToggleButtonInterface (Widget parent, Cmd *cmd)
  : CmdInterface ( cmd )
{
  // We need to generate a button name that doesn't have illegal characters.
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

  _w = XtVaCreateWidget (w_name, xmToggleButtonWidgetClass,
                         parent, XmNlabelString, label, NULL);

  XmStringFree(label);

  installDestroyHandler();
    
  // The _active member is set when each instance is registered
  // with an associated Cmd object. Now that a widget exists,
  // set the widget's sensitivity according to its active state.
  if ( _active )
    activate();     
  else
    deactivate();   

  XtAddCallback ( _w, XmNvalueChangedCallback,
                  &CmdInterface::executeCmdCallback, (XtPointer) this );  
}
