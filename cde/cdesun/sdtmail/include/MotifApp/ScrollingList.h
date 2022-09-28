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

#ifndef SCROLLINGLIST_H
#define SCROLLINGLIST_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)ScrollingList.h	1.7 03/12/94"
#endif

#include "UIComponent.h"
#include <Xm/List.h>

#include <stdlib.h>
class ScrollingList : public UIComponent {
private:
  static void defaultActionCallback( Widget, XtPointer, XmListCallbackStruct * );

public:

  ScrollingList ( Widget, char * );
  ~ScrollingList ();
  virtual const char *const className() { return ( "ScrollingList" ); }
  virtual void defaultAction( Widget, XtPointer, XmListCallbackStruct * ) = 0;
};

#endif
