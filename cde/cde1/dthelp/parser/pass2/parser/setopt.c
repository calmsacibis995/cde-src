/* $XConsortium: setopt.c /main/cde1_maint/1 1995/07/17 22:19:48 drk $ */
/*
Copyright 1988, 1989 Hewlett-Packard Co.
*/

#include "userinc.h"
#include "globdec.h"


/* Set program options */
void m_setoptions(M_NOPAR)
  {
    if (m_argc > 1) m_optstring(m_argv[1]) ;
    }
