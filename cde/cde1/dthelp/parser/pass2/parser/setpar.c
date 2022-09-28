/* $XConsortium: setpar.c /main/cde1_maint/1 1995/07/17 22:20:01 drk $ */
/*
                   Copyright 1986, 1987, 1988, 1989 Hewlett-Packard Co.
*/

/* Setparam.c is used by interfaces where code accesses an element's
   attributes */

#include <stdio.h>
#include "basic.h"
#include "trie.h"
#include "dtdext.h"
#include "parser.h"

/* Set a user-defined C variable to the corresponding parameter value */
void m_setparam(cvar, par)
  M_WCHAR **cvar ;
  int par ;
  {
    *cvar = m_stackpar->param[par] ;
    }

