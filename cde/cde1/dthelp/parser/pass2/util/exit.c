/* $XConsortium: exit.c /main/cde1_maint/1 1995/07/17 22:25:22 drk $ */
/*
Copyright 1988, 1989 Hewlett-Packard Co.
*/

#if defined(MSDOS)
#include <process.h>
#endif
#include "basic.h"

void m_exit(
#if defined(M_PROTO)
  int status
#endif
  ) ;

/* Exit function called by utilities used in PARSER and other programs in
   the MARKUP system */
void m_exit(status)
  int status ;
  {
    exit(status) ;
    }

