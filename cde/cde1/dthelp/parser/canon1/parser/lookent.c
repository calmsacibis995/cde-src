/* $XConsortium: lookent.c /main/cde1_maint/1 1995/07/17 21:17:57 drk $ */
/*
              Copyright 1986 Tandem Computers Incorporated.
This product and information is proprietary of Tandem Computers Incorporated.
                   Copyright 1986, 1987, 1988, 1989 Hewlett-Packard Co.
*/

/* Lookent.c contains procedure m_lookent(), callable by interface
   designers, to return entity type and content.  Useful for entity
   parameters */

#include <stdio.h>
#include "basic.h"
#include "trie.h"
#include "dtdext.h"
#include "parser.h"
#include "entext.h"

LOGICAL m_lookent(name, type, content, wheredef)
  M_WCHAR *name ;
  unsigned char *type ;
  M_WCHAR **content ;
  unsigned char *wheredef ;
  {
    M_ENTITY *entity ;

    if (entity = (M_ENTITY *) m_lookfortrie(name, m_enttrie)) {
      *type = entity->type ;
      *content = entity->content ;
      *wheredef = entity->wheredef ; 
      return(TRUE) ;
      }
    else return(FALSE) ;
    }
