/* $XConsortium: exit.c /main/cde1_maint/1 1995/07/17 21:16:21 drk $ */
/*
Copyright 1988, 1989 Hewlett-Packard Co.
*/

#include "userinc.h"
#include "globdec.h"
#if defined(MSDOS)
#include <process.h>
#endif

/* Default version of PARSER m_exit().  Allows interface control if
   internal error causes PARSER to halt. */
void m_exit(status)
  int status ;
  {
    exit(status) ;
    }

