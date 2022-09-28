/* $XConsortium: triedump.c /main/cde1_maint/1 1995/07/17 21:32:17 drk $ */
/*
                   Copyright 1986, 1987, 1988, 1989 Hewlett-Packard Co.
*/

#include <stdio.h>
#include "basic.h"
#include "trie.h"

void countdown(
#if defined(M_PROTO)
  M_TRIE *parent, int *count
#endif
  ) ;

void m_dumptrie(
#if defined(M_PROTO)
  FILE *file, M_TRIE *xtrie, char *extname, int *count,
  void (*proc)(void *value)
#endif
  ) ;

/* Write the C declaration of a trie */
void m_dumptrie(file, xtrie, extname, count, proc)
  FILE *file ;
  M_TRIE *xtrie ;
  char *extname ;
  int *count ;
  void (*proc)(
#if defined(M_PROTO)
    void *value
#endif
    ) ;
  {
    int firstson ;
    M_TRIE *p ;

    firstson = *count + 1 ;
    for (p = xtrie ; p ; p = p->next) firstson++ ;

    for (p = xtrie ; p ; p = p->next) {
      (*count)++ ;
      fprintf(file, ",\n  %d, ", p-> symbol) ;
      if (p->next) fprintf(file, "&%s[%d], ", extname, *count + 1) ;
      else fputs("NULL, ", file) ;
      if (p->symbol) {
        fprintf(file, "&%s[%d]", extname, firstson) ;
        countdown(p, &firstson) ;
        }
      else (*proc) (p->data) ;
      }

    for (p = xtrie ; p ; p = p->next)
      if (p->symbol)
        m_dumptrie(file, p->data, extname, count, proc) ;
    } 
