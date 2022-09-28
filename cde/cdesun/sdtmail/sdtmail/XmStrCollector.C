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

/*
 * The XmStrCollector class is simply a dynamic array of XmStrings.
 * It is useful to build an XmStringTable to pass as an argument to
 * any Motif widget that needs one (eg. XmList).
 */
#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)XmStrCollector.C	1.3 08/06/96"
#else
static char *sccs__FILE__ = "@(#)XmStrCollector.C	1.3 08/06/96";
#endif

#include <Xm/List.h>
#include <stdlib.h>
#include <stdio.h>
#include "XmStrCollector.h"

XmStrCollector::XmStrCollector()
{
    // Initialize the class variables
    num_items = 0;
    max_num_items = 0;
    increment = 10;
    list = NULL;
}

XmStrCollector::~XmStrCollector() 
{
    // Free the list items
    for (int i=0; i<num_items; i++)
	XmStringFree (list[i]);

    // Free the list
    XtFree ((char *)list);
}

//
// Add an item to the string collector list
//
void
XmStrCollector::AddItemToList (XmString item)
{
    // Allocate memory for 10 items at a time.
    // When this memory is exceeded, allocate
    // space for 10 more.  
    if (num_items >= max_num_items)
    {
        max_num_items += increment;
 
        list = (XmString *) XtRealloc ((char *)list,
		sizeof (XmString) *max_num_items);
    }
 
    list[num_items] = item;

    num_items++;
}

//
// Get the XmStringTable
//
XmString *
XmStrCollector::GetItems()
{
    return (list);
}

//
// Get the number of items
// in the XmStringTable
//
int
XmStrCollector::GetNumItems()
{
    return (num_items);
}


