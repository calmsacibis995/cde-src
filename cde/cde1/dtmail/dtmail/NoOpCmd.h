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


/////////////////////////////////////////////////////////////
// NoOpCmd.h: Example, dummy command class
/////////////////////////////////////////////////////////////
#ifndef NOOPCMD_H
#define NOOPCMD_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)NoOpCmd.h	1.13 03/03/95"
#endif

#include "Cmd.h"
#include "AskFirstCmd.h"
#include "InfoDialogManager.h"

class NoOpCmd : public Cmd {
    
  protected:
    
    virtual void doit();   
    virtual void undoit(); 
//    virtual void okCallback(void *);

  public:
    
    NoOpCmd ( char *, char *, int );
    virtual const char *const className () { return "NoOpCmd"; }
};

class LabelCmd: public Cmd {
protected:

#ifdef CAN_INLINE_VIRTUALS
    virtual void doit() {}
    virtual void undoit() {}
#else /* ! CAN_INLINE_VIRTUALS */
    virtual void doit();
    virtual void undoit();
#endif /* ! CAN_INLINE_VIRTUALS */

public:
    LabelCmd ( char *name, char *label, int active) :Cmd(name, label, active){}
#ifndef CAN_INLINE_VIRTUALS
    ~LabelCmd();
    virtual const char *const className ();
#else /* CAN_INLINE_VIRTUALS */
    virtual const char *const className () { return "LabelCmd"; }
#endif /* CAN_INLINE_VIRTUALS */
};

#endif
