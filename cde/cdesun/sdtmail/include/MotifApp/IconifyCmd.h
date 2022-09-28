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


////////////////////////////////////////////////////////////////
// IconifyCmd.h: Iconify all windows in a MotifApp application.
////////////////////////////////////////////////////////////////
#ifndef ICONIFYCMD_H
#define ICONIFYCMD_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)IconifyCmd.h	1.8 03/03/95"
#endif

#include "NoUndoCmd.h"

class IconifyCmd : public NoUndoCmd {
    
  protected:
    
    virtual void doit();      // Iconify all windows
    
  public:
    
    IconifyCmd ( char *, char *, int );
    virtual const char *const className () { return "IconifyCmd"; }
};
#endif
