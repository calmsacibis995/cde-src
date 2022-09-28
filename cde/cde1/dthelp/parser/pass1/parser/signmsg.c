/* $XConsortium: signmsg.c /main/cde1_maint/1 1995/07/17 21:54:04 drk $ */
/*
Copyright 1988, 1989 Hewlett-Packard Co.
*/

#include "userinc.h"
#include "globdec.h"

/* Process signon message text */
void m_signmsg(p)
M_WCHAR *p ;
{
char mb_p;

mb_p = MakeMByteString(p);
m_errline(mb_p) ;
m_free(mb_p,"multi-byte string");
}
