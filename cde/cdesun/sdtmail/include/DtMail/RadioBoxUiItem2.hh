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
#pragma ident "%W %G"
#endif

//#include <X11/Intrinsic.h>
#include <stdlib.h>
#include <DtMail/PropUi.hh>

#ifndef _RADIOBOXUIITEM2_HH
#define _RADIOBOXUIITEM2_HH


class RadioBoxUiItem2 : public PropUiItem {

private:
	int _values[2];
  
public:
    RadioBoxUiItem2(Widget, int source, char * search_key);
    void writeFromUiToSource();
    void writeFromSourceToUi();
};

#endif
