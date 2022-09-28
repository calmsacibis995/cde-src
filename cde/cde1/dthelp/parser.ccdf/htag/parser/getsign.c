/* $XConsortium: getsign.c /main/cde1_maint/1 1995/07/17 22:44:06 drk $ */
/*
Copyright 1988, 1989 Hewlett-Packard Co.
*/

/* Issue interface-generated sign-on message */

#include <stdio.h>
#include "basic.h"
#include "trie.h"
#include "dtdext.h"
#include "parser.h"
#include "signonx.h"

void m_getsignon(M_NOPAR)
  {
    m_errline(m_signon) ;
    }


