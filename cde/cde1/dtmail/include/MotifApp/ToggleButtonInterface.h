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


//////////////////////////////////////////////////////////////
// ToggleButtonInterface.h: A togglebutton interface to a Cmd object
///////////////////////////////////////////////////////////////
#ifndef TOGGLEBUTTONINTERFACE
#define TOGGLEBUTTONINTERFACE

#ifndef I_HAVE_NO_IDENT
#pragma ident "02/22/95 95/02/22"
#endif

#include "CmdInterface.h"

class ToggleButtonInterface : public CmdInterface {
    
  public:
    
    ToggleButtonInterface ( Widget, Cmd * );
#ifndef CAN_INLINE_VIRTUALS
    ~ToggleButtonInterface ( void );
#endif /* ! CAN_INLINE_VIRTUALS */

};

#endif
