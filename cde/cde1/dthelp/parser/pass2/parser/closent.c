/* $XConsortium: closent.c /main/cde1_maint/1 1995/07/17 22:11:42 drk $ */
/* Copyright 1988, 1989 Hewlett-Packard Co. */

/* Default close SYSTEM entity procedure */

#include "userinc.h"
#include "globdec.h"

void m_closent(m_ptr)
  void *m_ptr ;
  {
    fclose((FILE *) m_ptr) ;
    }

