/* $XConsortium: upper.c /main/cde1_maint/1 1995/07/17 21:33:29 drk $ */
/*
                   Copyright 1986, 1987, 1988, 1989 Hewlett-Packard Co.
*/

/* Force a letter to uppercase */

#include "basic.h"

int m_upper(
#if defined(M_PROTO)
  int c
#endif
  ) ;

int m_upper(c)
  int c ;
  {
    if (c >= 'a' && c <= 'z') c += 'A' - 'a' ;
    return(c) ;
    }
