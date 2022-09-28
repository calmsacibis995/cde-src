/* $XConsortium: docall.h /main/cde1_maint/1 1995/07/17 23:07:32 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        docall.h
 **
 **   Project:     CDE
 **
 **   Description: Public include file for docall.c
 **
 **
 **   (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992
 **       by Hewlett-Packard Company
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _Dtksh_docall_h
#define _Dtksh_docall_h

#include "exksh.h"

extern int _Prdebug;
extern struct memtbl Null_tbl;
extern char xk_ret_buffer[];
extern char * xk_ret_buf;
extern struct Bfunction xk_prdebug;


#ifdef _NO_PROTO

extern int do_field_get() ;
extern int do_call() ;
extern int asl_set() ;
extern int do_field_comp() ;

#else

extern int do_field_get( 
                        int argc,
                        char **argv) ;
extern int do_call( 
                        int argc,
                        char **argv) ;
extern int asl_set( 
                        char *argv0,
                        memtbl_t *tblarray,
                        char *desc,
                        unsigned char **pargs) ;
extern int do_field_comp( 
                        int argc,
                        char **argv) ;

#endif /* _NO_PROTO */


#endif /* _Dtksh_docall_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
