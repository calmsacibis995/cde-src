/* $XConsortium: parvalok.c /main/cde1_maint/1 1995/07/17 22:47:51 drk $ */
/*
                   Copyright 1989 Hewlett-Packard Co.
*/

#include <stdio.h>
#include "basic.h"
#include "trie.h"
#include "dtdext.h"
#include "parser.h"

/* Function callable by interface designers.  Returns TRUE if specified value
   is a legal value for the indicated parameter of the given element,
   FALSE otherwise. */
#if defined(M_PROTO)
LOGICAL m_parvalok( M_WCHAR *elt , M_WCHAR *param , const M_WCHAR *value )
#else
LOGICAL m_parvalok(elt, param, value)
  M_WCHAR *elt ;
  M_WCHAR *param ;
  M_WCHAR *value ;
#endif /* M_PROTO */
  {
    M_ELEMENT eltid ;
    int par ;
    int i ;

    if (! (eltid = m_packedlook(m_entree, elt))) return(FALSE) ;
    for (par = m_element[eltid - 1].parptr, i = 1 ;
         i <= m_element[eltid - 1].parcount ;
         par++, i++)
      if (! m_wcupstrcmp(&m_pname[m_parameter[par - 1].paramname], param))
        if (m_partype(par, value)) return(TRUE) ;
        else return(FALSE) ;

    /* No such parameter */
    return(FALSE) ;
    }

