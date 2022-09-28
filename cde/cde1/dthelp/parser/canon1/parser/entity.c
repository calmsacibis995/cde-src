/* $XConsortium: entity.c /main/cde1_maint/1 1995/07/17 21:15:11 drk $ */
/*
              Copyright 1986 Tandem Computers Incorporated.
This product and information is proprietary of Tandem Computers Incorporated.
                   Copyright 1986, 1987, 1988, 1989 Hewlett-Packard Co.
*/

/* Entity.c contains procedures pertaining to entities */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#if defined(MSDOS)
#include <process.h>
#endif
#include "basic.h"
#include "trie.h"
#include "context.h"
#include "dtdext.h"
#include "parser.h"
#include "entext.h"

/* Process the name in a usemap delaration */
#if defined(M_PROTO)
void m_ckmap(M_WCHAR *name, LOGICAL useoradd)
#else
void m_ckmap(name, useoradd)
  M_WCHAR *name ;
  LOGICAL useoradd ;
#endif
{
    int mapid ;

    if (mapid = m_packedlook(m_maptree, name))
      m_setmap(mapid + 1, useoradd) ;
    else m_err1("Undefined short reference map %s", name) ;
    }

/* Check type specified in entity declaration for previously defined
   entity.  Testing to see if the new declaration is identical to the
   original one. */
void m_eduptype(type)
  int type ;
  {
    if ((int) m_entity->type != type) {
      m_err1("Redefinition of entity %s ignored", m_entity->name) ;
      m_entity = NULL ;
      m_curcon = TYPEDENTVAL ;
      } 
    }

/* Tests if an entity is too long */
void m_longent(context)
  int context ;
  {
    if (m_entclen >= M_LITLEN) {
      m_curcon = context ;
      m_error("Entity content too long") ;
      }
    else m_entcontent[m_entclen++] = m_scanval ;
    }

/* Enters an entity name into the entity name tree */
void m_ntrent(p)
  M_WCHAR *p ;
  {
    M_ENTITY *new ;

    new = (M_ENTITY *) m_malloc(sizeof(M_ENTITY), "entity") ;
    if (m_entity = (M_ENTITY *) m_ntrtrie(p, m_enttrie, (M_TRIE *) new)) {
      m_free(new, "entity") ;
      if (m_entity->wheredef == M_DPARSER) {
        if (m_entdupchk) {
          m_err1("Redefinition of entity %s ignored", p) ;
          m_entity = NULL ;
          }
        else m_curcon = DUPENT ;
        }
      else {
        if (m_entity->content) {
          m_err1("Warning: Redefinition of predefined entity %s", p) ;
          m_entity->type = M_GENERAL ;
          m_entity->content = NULL ;
          }
        m_entity->wheredef = M_DPARSER ;
        }
      return ;
      }
    m_entity = new ;
    m_entity->type = M_GENERAL ;
    m_entity->wheredef = M_DPARSER ;
    m_entity->content = NULL ;
    m_entity->name = (M_WCHAR *) m_malloc(w_strlen(p) + 1, "entity name") ;
    w_strcpy(m_entity->name, p) ;
    }
