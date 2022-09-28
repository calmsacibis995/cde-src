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


////////////////////////////////////////////////////////////////////
// PixmapCycler.h: Abstract class that supports a continuous cycle
//                 of pixmaps for short animation sequences.
////////////////////////////////////////////////////////////////////
#ifndef PIXMAPCYCLER_H
#define PIXMAPCYCLER_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)PixmapCycler.h	1.7 03/12/94"
#endif

#include <Xm/Xm.h>

class PixmapCycler {
    
  protected:
    
    int       _numPixmaps;     // Total number of pixmaps in cycle
    int       _current;        // Index of the current pixmap
    Pixmap   *_pixmapList;     // The array of pixmaps
    Dimension _width, _height; // Pixmap size
    
    virtual void createPixmaps() = 0; // Derived class must implement
    PixmapCycler ( int, Dimension, Dimension  );   
    
  public:
    
    virtual ~PixmapCycler();
    Dimension width()  { return _width; }
    Dimension height() { return _height; }
    
    Pixmap next();       // Return the next pixmap in the cycle
};
#endif
