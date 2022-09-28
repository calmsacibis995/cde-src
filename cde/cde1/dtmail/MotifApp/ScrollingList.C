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
#pragma ident "@(#)ScrollingList.C	1.7 05/26/94"
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


////////////////////////////////////////////////////////////
// ScrollingList.C: 
////////////////////////////////////////////////////////////

#include "ScrollingList.h"
#include "Help.hh"


ScrollingList::ScrollingList( Widget parent, char *name ) : UIComponent( name )
{

  _w = XmCreateScrolledList( parent,
		     name,
		     NULL, 0 );
  printHelpId("_w", _w);
  /* add help callback */
  // XtAddCallback(_w, XmNhelpCallback, HelpCB, helpId);

  XtAddCallback( _w,
		 XmNdefaultActionCallback,
		 (XtCallbackProc) &ScrollingList::defaultActionCallback,
		 this ); 
  installDestroyHandler();
}

ScrollingList::~ScrollingList () 
{
}

void
ScrollingList::defaultActionCallback( Widget w,
			      XtPointer clientData,
			      XmListCallbackStruct *cbs )
{
  ScrollingList *obj = ( ScrollingList *) clientData;
  obj->defaultAction( w, clientData, cbs );
}




