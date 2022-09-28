/* $XConsortium: define.h /main/cde1_maint/1 1995/07/17 23:07:05 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        define.h
 **
 **   Project:     CDE
 **
 **   Description: Public include file for define.c
 **
 **
 **   (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992
 **       by Hewlett-Packard Company
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _Dtksh_define_h
#define _Dtksh_define_h


#ifdef _NO_PROTO

extern int do_define() ;
extern int fdef() ;
extern int do_deflist() ;
extern int do_finddef() ;

#else

extern int do_define( 
                        int argc,
                        char **argv) ;
extern int fdef( 
                        char *str,
                        unsigned long *val) ;
extern int do_deflist( 
                        int argc,
                        char **argv) ;
extern int do_finddef( 
                        int argc,
                        char **argv) ;

#endif /* _NO_PROTO */


#endif /* _Dtksh_define_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
