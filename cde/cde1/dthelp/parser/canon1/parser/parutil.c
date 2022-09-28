/* $XConsortium: parutil.c /main/cde1_maint/2 1995/08/26 22:51:53 barstow $ */
/*
              Copyright 1986 Tandem Computers Incorporated.
This product and information is proprietary of Tandem Computers Incorporated.
                   Copyright 1986, 1987, 1988, 1989 Hewlett-Packard Co.
*/

/*  Parutil.c contains utility functions for program PARSER */

#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <locale.h>
#if defined(MSDOS)
#include <process.h>
#endif
#include "basic.h"
#include "trie.h"
#include "dtdext.h"
#include "delim.h"
#include "context.h"
#include "parser.h"
#include "entext.h"
#include "if.h"

#include "version.h"

/* Adjust parsing state after each token if necessary */
void m_adjuststate(M_NOPAR)
  {
    if (m_curcon == START) {
      if (m_netlevel) {
        if (m_stacktop->intext) m_curcon = NETCDATA ;
        else if (m_textpermitted()) m_curcon = NETDATACON ;
        else m_curcon = NETELCON ;
        }
      else {
        if (m_stacktop->intext) m_curcon = POUNDCDATA ;
        else if (m_textpermitted()) m_curcon = DATACON ;
        else m_curcon = ELCON ;
        }
      }
    else if (m_netlevel) switch(m_curcon) {
      case POUNDCDATA:
        m_curcon = NETCDATA ;
        break ;
      case CDATAEL:
        if (m_stacktop->neednet) m_curcon = NETELCDATA ;
        break ;
      case RCDATAEL:
        if (m_stacktop->neednet) m_curcon = NETRCDATA ;
        break ;
      default:
        break ;
      }
    }

/* Tests whether the element named in a tag is defined; if so, returns its
   index; otherwise return FALSE */
M_ELEMENT m_eltname(M_NOPAR)
  {
    M_ELEMENT eltid ;

    if (eltid = m_packedlook(m_entree, m_name)) return(eltid) ;
    else {
      m_err1("Undefined element %s", m_name) ;
      return(FALSE) ;
      }
    }

int get_mb_cur_max()
{
char *l;
int   i;

l = getenv("LANG");

i = MB_CUR_MAX;

return i;
}

/* Program initialization */
void m_initialize(M_NOPAR)
{
char    **mb_delims;
M_WCHAR **wc_delims;
m_signmsg("MARKUP PARSER - (canonical SGML)");
m_signmsg(M_VERSION) ;
m_signmsg("\n") ;
m_signmsg("(c) Copyright 1993, 1994, 1995 Hewlett-Packard Company\n");
m_signmsg("(c) Copyright 1993, 1994, 1995 International Business Machines Corp.\n");
m_signmsg("(c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.\n");
m_signmsg("(c) Copyright 1993, 1994, 1995 Unix System Labs, Inc., a subsidiary of Novell, Inc.\n");
/* commented out since interfers with testing --bad reason I know
m_getsignon() ;
*/
setlocale(LC_ALL, "");
get_mb_cur_max();

mb_delims = mb_dlmptr;
wc_delims = m_dlmptr;

while (*mb_delims)
    {
    *wc_delims++ = MakeWideCharString(*mb_delims);
    mb_delims++;
    }
*wc_delims = 0;
}
