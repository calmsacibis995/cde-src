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
#pragma ident "11/27/96 96/11/27"
#endif

#include "MotifCmds.h"

// Returns TRUE if button is ON, FALSE if OFF.
Boolean
ToggleButtonCmd::getButtonState()
{
   CmdInterface *ci;
   Widget toggleButton;

   if (!_ci)
     return(NULL);

   ci = _ci[0];

   toggleButton = ci->baseWidget();

   return (XmToggleButtonGetState(toggleButton));
}

void
ToggleButtonCmd::setButtonState(Boolean state, Boolean notify)
{
  CmdInterface *ci;
  Widget toggleButton;

  if (!_ci) return;

  ci = _ci[0];

  if (ci) {
    toggleButton = ci->baseWidget();

    XmToggleButtonSetState(toggleButton, state, notify);
  }
}
