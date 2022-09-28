/* $XConsortium: misc.h /main/cde1_maint/1 1995/07/18 00:56:54 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        misc.h
 **
 **   Project:     CDE
 **
 **   Description: Public include file for misc.c
 **
 **
 **   (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992
 **       by Hewlett-Packard Company
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _Dtksh_misc_h
#define _Dtksh_misc_h


#ifdef _NO_PROTO

extern int symcomp() ;
extern void * getaddr() ;
extern int do_deref() ;
extern void * nop() ;
extern void * save_alloc() ;

#else

extern int symcomp( 
                        const void *sym1,
                        const void *sym2) ;
extern void * getaddr( 
                        char *str) ;
extern int do_deref( 
                        int argc,
                        char **argv) ;
extern void * nop( 
                        void *var) ;
extern void * save_alloc( 
                        void *var) ;

#endif /* _NO_PROTO */

#endif /* _Dtksh_misc_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
