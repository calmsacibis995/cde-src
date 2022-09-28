/* $XConsortium: trierset.c /main/cde1_maint/1 1995/07/17 21:33:01 drk $ */
/*
                   Copyright 1988, 1989 Hewlett-Packard Co.
*/

#include <stdio.h>
#include "basic.h"
#include "common.h"
#include "trie.h"

extern M_CHARTYPE m_ctarray[M_CHARSETLEN] ;

/* Changes the value associated with an entry in a trie. */
void *m_resettrie(xtrie, p, value)
  M_TRIE *xtrie ;
  M_WCHAR *p ;
  void *value ;
  {
    M_TRIE *currentnode ;

    currentnode = xtrie->data ;
    while (TRUE) {
      if (! currentnode) return(FALSE) ;
      if ((int) currentnode->symbol == m_ctupper(*p))
        if (! *p) return((void *)(currentnode->data = (M_TRIE *) value)) ;
        else {
          p++ ;
          currentnode = currentnode->data ;
          continue ;
          }
      else if (currentnode->symbol < *p) {
        currentnode = currentnode->next ;
        continue ;
        }
      else return(NULL) ;
      }
    }


