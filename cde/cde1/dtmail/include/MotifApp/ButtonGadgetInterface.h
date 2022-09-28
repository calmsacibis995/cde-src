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


//////////////////////////////////////////////////////////////
// ButtonGadgetInterface.h: A push button interface to a Cmd object
///////////////////////////////////////////////////////////////
#ifndef BUTTONGADGETINTERFACE
#define BUTTONGADGETINTERFACE

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)ButtonGadgetInterface.h	1.9 22 Feb 1995"
#endif

#include "CmdInterface.h"

class ButtonGadgetInterface : public CmdInterface {
    
  public:
    
    ButtonGadgetInterface ( Widget, Cmd * );

#ifndef CAN_INLINE_VIRTUALS
    ~ButtonGadgetInterface ( void );
#endif /* CAN_INLINE_VIRTUALS */

};
#endif
