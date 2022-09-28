/* $XConsortium: getc.c /main/cde1_maint/1 1995/07/17 21:47:40 drk $ */
/* Copyright 1988, 1989 Hewlett-Packard Co. */

/* Default get-char procedure */

#include "userinc.h"
#include "globdec.h"

int m_getc(m_ptr)
  void *m_ptr ;
  {
    return(getc((FILE*) m_ptr)) ;
    }
