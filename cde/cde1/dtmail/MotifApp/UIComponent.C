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
#pragma ident "@(#)UIComponent.C	1.7 02/22/95"
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


///////////////////////////////////////////////////////////////
// UIComponent.C: Base class for all C++/Motif UI components
///////////////////////////////////////////////////////////////
#include "UIComponent.h"
#include <assert.h>
#include <stdio.h>

UIComponent::UIComponent ( const char *name ) : BasicComponent ( name )
{
    // Empty
}


void
UIComponent::widgetDestroyedCallback( Widget, 
				      XtPointer clientData, 
				      XtPointer )
{
    UIComponent * obj = (UIComponent *) clientData;	
    
    obj->widgetDestroyed();
}

void
UIComponent::widgetDestroyed()
{
    _w = NULL;
}

void
UIComponent::installDestroyHandler()
{
    assert ( _w != NULL );
    XtAddCallback ( _w, 
		   XmNdestroyCallback,
		   &UIComponent::widgetDestroyedCallback, 
		   (XtPointer) this );
}

void
UIComponent::manage()
{
    assert ( _w != NULL );
    assert ( XtHasCallbacks ( _w, XmNdestroyCallback ) ==
	    XtCallbackHasSome );
    XtManageChild ( _w );
}

UIComponent::~UIComponent()
{
    // Make sure the widget hasn't already been destroyed
    
    if ( _w ) 
    {
	// Remove destroy callback so Xt can't call the callback
	// with a pointer to an object that has already been freed
	
	XtRemoveCallback ( _w, 
			  XmNdestroyCallback,
			  &UIComponent::widgetDestroyedCallback,
			  (XtPointer) this );	
    }
}

void
UIComponent::getResources( const XtResourceList resources, 
			   const int numResources )
{
    // Check for errors
    
    assert ( _w != NULL );
    assert ( resources != NULL );
    
    // Retrieve the requested resources relative to the 
    // parent of this object's base widget
    // Added support for doing getResources on the Application
    
    if ( XtParent( _w ) ) 
	XtGetSubresources ( XtParent( _w ), 
			    (XtPointer) this, 
			    _name,
			    className(),
			    resources, 
			    numResources,
			    NULL, 
			    0 );
    else 
	XtGetSubresources ( _w , 
			    (XtPointer) this, 
			    _name,
			    className(),
			    resources, 
			    numResources,
			    NULL, 
			    0 );
}


#ifdef DEAD_WOOD
void
UIComponent::setDefaultResources( const Widget w, 
				  const String *resourceSpec )
{
    int         i;	
    Display    *dpy = XtDisplay ( w );	// Retrieve the display pointer
    XrmDatabase rdb = NULL;		// A resource data base
    
    // Create an empty resource database

    rdb = XrmGetStringDatabase ( "" );

    // Add the Component resources, prepending the name of the component

    i = 0;
    while ( resourceSpec[i] != NULL )
	{
	    char buf[1000];

	    sprintf(buf, "*%s%s", _name, resourceSpec[i++]);
	    XrmPutLineResource( &rdb, buf );
	}

    // Merge them into the Xt database, with lowest precendence
    
    if ( rdb )
	{
#if (XlibSpecificationRelease>=5)
	    XrmDatabase db = XtDatabase(dpy);
	    XrmCombineDatabase(rdb, &db, FALSE);
#else
	    XrmMergeDatabases ( dpy->db, &rdb );
	    dpy->db = rdb;
#endif
	}
}
#endif /* DEAD_WOOD */

#ifndef CAN_INLINE_VIRTUALS
const char *const
UIComponent::className(void)
{
    return "UIComponent";
}
#endif /* ! CAN_INLINE_VIRTUALS */
