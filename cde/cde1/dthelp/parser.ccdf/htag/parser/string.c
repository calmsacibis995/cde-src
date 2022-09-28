/* $XConsortium: string.c /main/cde1_maint/1 1995/07/17 22:50:05 drk $ */
/* Copyright 1988, 1989 Hewlett-Packard Co. */

/* String.c executes string-code specified in the interface. */

#include "userinc.h"
#include "globdec.h"

/* Include generated code */
#include "stfile.c"

#if defined(M_PROTO)
void m_stcaction(M_WCHAR *m_string, LOGICAL M_START, LOGICAL M_END)
#else
void m_stcaction(m_string, M_START, M_END)
  M_WCHAR *m_string ;
  LOGICAL M_START ;
  LOGICAL M_END ;
#endif
{
    m_stackpar = m_stacktop->stparam ;
    (*m_sttable[m_stacktop->stccase])(m_string, M_START, M_END) ;
    }
