/* $XConsortium: errline.c /main/cde1_maint/1 1995/07/17 21:15:39 drk $ */
/*
Copyright 1988, 1989 Hewlett-Packard Co.
*/

#include "userinc.h"
#include "globdec.h"

/* Process error message text */
void m_errline(p)
M_WCHAR *p ;
{
char mb_p;

mb_p = MakeMByteString(p);
fputs(mb_p, m_errfile) ;
fputs(mb_p, stderr) ;
m_free(mb_p,"multi-byte string");
}

