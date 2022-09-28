/* $XConsortium: pi.c /main/cde1_maint/1 1995/07/17 22:48:04 drk $ */
/*
              Copyright 1986 Tandem Computers Incorporated.
This product and information is proprietary of Tandem Computers Incorporated.
                   Copyright 1986, 1987, 1988, 1989 Hewlett-Packard Co.
*/

/* Pi.c executes pi-code specified in the interface. */

#include "userinc.h"
#include "globdec.h"

/* Include generated code */
#include "pfile.c"


/* When a processing instruction or SDATA entity other than a CODE entity
   occurs */
void m_piaction(m_pi, m_entname, m_enttype)
  M_WCHAR *m_pi ;
  M_WCHAR *m_entname ;
  int m_enttype ;
  {
    m_stackpar = m_stacktop->piparam ;
    (*m_ptable[m_stacktop->picase])(m_pi, m_entname, m_enttype) ;
    }

