/* $XConsortium: openent.c /main/cde1_maint/1 1995/07/17 21:18:25 drk $ */
/* Copyright 1988, 1989 Hewlett-Packard Co. */

/* Default open SYSTEM entity procedure */

#include "userinc.h"
#include "globdec.h"

void *m_openent(entcontent)
  M_WCHAR *entcontent ;
  {
    return((void *) fopen(entcontent, "r")) ;
    }

