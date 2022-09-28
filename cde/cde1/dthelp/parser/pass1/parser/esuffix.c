/* $XConsortium: esuffix.c /main/cde1_maint/1 1995/07/17 21:46:56 drk $ */
/*
Copyright 1988, 1989 Hewlett-Packard Co.
*/

#include "userinc.h"
#include "globdec.h"
#if defined(MSDOS)
#include <process.h>
#endif

/* Write error message suffix */
void m_esuffix(M_NOPAR)
  {
    m_errline("\n") ;
    m_dumpline(m_thisfile(), m_thisline()) ;
    m_errline(":\n") ;
    m_lastchars() ;
    if (++m_errcnt == m_errlim) {
      m_error("Too many errors, processing stopped") ;
      m_exit(TRUE) ;
      }
    }

