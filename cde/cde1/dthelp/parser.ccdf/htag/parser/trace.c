/* $XConsortium: trace.c /main/cde1_maint/1 1995/07/17 22:51:02 drk $ */
/*
Copyright 1988, 1989 Hewlett-Packard Co.
*/

#include "userinc.h"
#include "globdec.h"

/* Write debugging trace information */
void m_wctrace(p)
M_WCHAR *p ;
{
char *mb_p;

mb_p = MakeMByteString(p);
fputs(mb_p, m_outfile) ;
m_free(mb_p,"multi-byte string");
}

void m_trace(p)
char *p ;
{
fputs(p, m_outfile);
}
