/* $XConsortium: letter.c /main/cde1_maint/1 1995/07/17 21:29:18 drk $ */
/* Copyright 1987, 1988, 1989 Hewlett-Packard Co. */

/* Tests whether a character is a letter */

#include "basic.h"

LOGICAL m_letter(
#if defined(M_PROTO)
  M_WCHAR c
#endif
  ) ;

#if defined(M_PROTO)
LOGICAL m_letter(M_WCHAR c)
#else
LOGICAL m_letter(c)
  M_WCHAR c ;
#endif
{
if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) return(TRUE) ;
return(FALSE) ;
}
