/* $XConsortium: openfrst.c /main/cde1_maint/1 1995/07/17 22:16:29 drk $ */
/* Copyright 1988, 1989 Hewlett-Packard Co. */

/* Default open SYSTEM entity procedure */

#include "userinc.h"
#include "globdec.h"

void *m_openfirst(M_NOPAR)
  {
    return((void *) stdin) ;
    }


