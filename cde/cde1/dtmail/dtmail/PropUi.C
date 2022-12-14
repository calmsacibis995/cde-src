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
#pragma ident " @(#)PropUi.C	1.1 08/06/94 "
#endif

#include "RoamApp.h"
#include <DtMail/PropUi.hh>
#include <DtMail/PropSource.hh>
#include <string.h>

// PropUiItem::PropUiItem 
// PropUiItem ctor
////////////////////////////////////////////////////////////////
PropUiItem::PropUiItem(Widget w, int source, char *search_key )
{
  // init variables
  

 
  prop_widget = w;
  dirty_bit = FALSE;
  source = source;
  key = strdup( search_key );
  DtMailEnv error;
  DtMail::Session * d_session = NULL;
  DtMail::MailRc * m_rc = NULL;

  switch (source)
    {
    case _FROM_MAILRC:
      
      d_session = theRoamApp.session()->session();
      m_rc = d_session->mailRc(error);
      prop_source = (PropSource *)new MailRcSource(m_rc, key );
      break;
      
    default:
      prop_source = NULL;
      fprintf(stderr, "Error in PropUiItem ctor\n");
      break; 
  };

}

// PropUiItem::~PropUiItem
// PropUiItem dtor
////////////////////////////////////////////////////////////////
PropUiItem::~PropUiItem()
{
  // release memory alloced by the class
  free(key);

  delete prop_source;

}



