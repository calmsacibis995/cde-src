/* $XConsortium: inctest.c /main/cde1_maint/1 1995/07/17 21:59:55 drk $ */
/*
                   Copyright 1986, 1987, 1988, 1989 Hewlett-Packard Co.
*/

#if defined(MSDOS)
#include <process.h>
#endif
#include "basic.h"

void m_err1(
#if defined(M_PROTO)
  const char *text, const char *arg
#endif
  ) ;

void m_exit(
#if defined(M_PROTO)
  int status
#endif
  ) ;

void m_inctest(
#if defined(M_PROTO)
  int *count, int limit, char *message
#endif
  ) ;

/* Increment a count and test against a limit */
void m_inctest(count, limit, message)
  int *count ;
  int limit ;
  char *message ;
  {
    if (++*count <= limit) return ;
    m_err1("Internal error: %s exceeded", message) ;
    m_exit(TRUE) ;
    }
