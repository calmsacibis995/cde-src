/* $XConsortium: startdoc.c /main/cde1_maint/1 1995/07/17 21:23:18 drk $ */
/*
Copyright 1989 Hewlett-Packard Co.
*/

#include "userinc.h"
#include "globdec.h"

/* Default function called at start of document instance.  Allows interface
   control after all entity declarations have been processed.  Not needed
   in valid documents (when it is called just prior to global start-code),
   but may be useful in some applications nevertheless. */
void m_startdoc(M_NOPAR)
  {
    }


