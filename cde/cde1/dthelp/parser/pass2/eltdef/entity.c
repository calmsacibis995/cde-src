/* $XConsortium: entity.c /main/cde1_maint/1 1995/07/17 22:09:00 drk $ */
/* Copyright (c) 1986, 1987, 1988, 1989 Hewlett-Packard Co. */

/* Entity.c has ELTDEF procedures relevant to entities */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "basic.h"
#include "trie.h"
#include "dtdext.h"
#include "eltdef.h"
#include "entext.h"

/* Add an entity */
void addent(M_NOPAR)
  {
    M_ENTITY *new ;

    new = (M_ENTITY *) m_malloc(sizeof(M_ENTITY), "entity") ;
    if (entity = (M_ENTITY *) m_ntrtrie(name, m_enttrie, (M_TRIE *) new)) {
      m_free(new, "entity") ;
      if (! entity->wheredef) return ;
      if (entity->wheredef == M_DBUILD) {
        warning1("Redefining %s: entity defined in BUILD", entity->name) ;
        entity->type = M_GENERAL ;
        entity->wheredef = FALSE ;
        entity->content = NULL ;
        return ;
        }
      else
        m_err1("Attempt to redefine %s", entity->name) ;
      return ;
      }
    entity = new ;
    if (lastent) lastent->next = entity ;
    else firstent = entity ;
    lastent = entity ;
    entity->type = M_GENERAL ;
    entity->wheredef = FALSE ;
    entity->content = NULL ;
    entity->name =
      (M_WCHAR *) m_malloc(w_strlen(name) + 1, "entity name") ;
    entity->index = ++m_entcnt ;
    entity->codeindex = M_NULLVAL ;
    entity->next = NULL ;
    w_strcpy(entity->name, name) ;
    return ;
    }

#include "entout.c"
