/* $XConsortium: genlib.h /main/cde1_maint/1 1995/07/17 23:11:44 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        genlib.h
 **
 **   Project:     CDE
 **
 **   Description: Public include file for genlib.c
 **
 **
 **   (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992
 **       by Hewlett-Packard Company
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _Dtksh_genlib_h
#define _Dtksh_genlib_h

#include "exksh.h"

extern struct libstruct *All_libs;


#ifdef _NO_PROTO

extern int xk_usage() ;
extern unsigned long fsym() ;
extern int do_findsym() ;
#ifndef SPRINTF_RET_LEN
extern int lsprintf() ;
#endif

#else

extern int xk_usage( 
                        char *funcname) ;
extern unsigned long fsym( 
                        char *str,
                        int lib) ;
extern int do_findsym( 
                        int argc,
                        char **argv) ;
#ifndef SPRINTF_RET_LEN
extern int lsprintf( 
                        char *buf,
                        char *fmt,
                        unsigned long arg1,
                        unsigned long arg2,
                        unsigned long arg3,
                        unsigned long arg4,
                        unsigned long arg5,
                        unsigned long arg6,
                        unsigned long arg7) ;
#endif

#endif /* _NO_PROTO */


#endif /* _Dtksh_genlib_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
