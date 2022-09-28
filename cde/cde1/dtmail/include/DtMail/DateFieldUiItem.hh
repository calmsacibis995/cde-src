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
#pragma ident " @(#)DateFieldUiItem.hh	1.1 06 Aug 1994 "
#endif

#include <stdlib.h>
#include <X11/Intrinsic.h>

#ifndef _DATEFIELDUIITEM_HH
#define _DATEFIELDUIITEM_HH

// CLASS DateFieldUiItem
// derived class for prop sheet glue items for datefield
///////////////////////////////////////////////////////////
class DateFieldUiItem : public PropUiItem {
  
public:
  	DateFieldUiItem(Widget w, int source, char *search_key);
  	virtual ~DateFieldUiItem(){;}; // we don't alloc any memory
  	virtual void writeFromUiToSource();
  	virtual void writeFromSourceToUi();
};

#endif
