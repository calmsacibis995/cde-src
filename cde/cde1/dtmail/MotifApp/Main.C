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
#pragma ident "@(#)Main.C	1.9 03/12/94"
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


//////////////////////////////////////////////////////////
// Main.C: Generic main program used by all applications
//////////////////////////////////////////////////////////
#include "Application.h"
#include <assert.h>

// We can implement main() in the library because the 
// framework completely encapsulates all Xt boilerplate 
// and all central flow of control. 

#if (XlibSpecificationRelease>=5)
int main ( int argc, char **argv )
#else
int main ( unsigned int argc, char **argv )
#endif
{
    // Make sure the programmer has remembered to 
    // instantiate an Application object
    
    assert ( theApplication ); 
    
    // Init Intrinsics, build all windows, and enter event loop
    
    theApplication->initialize ( &argc, argv );
    
    theApplication->handleEvents();
    return(0);
}
