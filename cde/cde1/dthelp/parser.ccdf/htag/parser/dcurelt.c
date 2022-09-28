/* $XConsortium: dcurelt.c /main/cde1_maint/1 1995/07/17 22:41:27 drk $ */
/*
Copyright 1988, 1989 Hewlett-Packard Co.
*/

#include "userinc.h"
#include "globdec.h"

/* Displays current element after some error messages */
void m_dispcurelt(file, line)
M_WCHAR *file ;
int line ;
{
char *mb_parent;

m_errline("Current element is ") ;
mb_parent = MakeMByteString(m_parent(0));
m_errline(mb_parent) ;
m_free(mb_parent,"multi-byte string");
m_errline(" begun on ") ;
m_dumpline(file, line) ;
m_errline(".\n") ;
}
