/* $XConsortium: parcount.c /main/cde1_maint/1 1995/07/17 22:46:20 drk $ */
/*
                   Copyright 1989 Hewlett-Packard Co.
*/

#include <stdio.h>
#include "basic.h"
#include "trie.h"
#include "dtdext.h"
#include "parser.h"

/* Function callable by interface designers.  Returns number of parameters
   of specified element (-1 indicates error). */
int m_parcount(elt)
  M_WCHAR *elt ;
  {
    M_ELEMENT eltid ;

    if (! (eltid = m_packedlook(m_entree, elt))) return(-1) ;
    return(m_element[eltid - 1].parcount) ;
    }

