/* $XConsortium: allwhite.c /main/cde1_maint/1 1995/07/17 21:56:18 drk $ */
/*
                   Copyright 1987, 1988, 1989 Hewlett-Packard Co.
*/
#include "basic.h"

LOGICAL m_allwhite(
#if defined(M_PROTO)
  const M_WCHAR *string
#endif
  ) ;

LOGICAL m_whitespace(
#if defined(M_PROTO)
  M_WCHAR c
#endif
  ) ;

/* Test if a string contains only white space characters */
#if defined(M_PROTO)
LOGICAL m_allwhite(const M_WCHAR *string )
#else
LOGICAL m_allwhite(string)
M_WCHAR *string ;
#endif /* M_PROTO */
{
const M_WCHAR *p ;

for (p = string; *p ; p++)
    if (! m_whitespace(*p)) return(FALSE) ;
return(TRUE) ;
}
