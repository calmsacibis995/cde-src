/* $XConsortium: struct.h /main/cde1_maint/1 1995/07/18 00:57:46 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        struct.h
 **
 **   Project:     CDE
 **
 **   Description: Public include file for struct.c
 **
 **
 **   (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992
 **       by Hewlett-Packard Company
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _Dtksh_struct_h
#define _Dtksh_struct_h

#include "exksh.h"


#ifdef _NO_PROTO

extern memtbl_t * ffind() ;
extern int do_struct() ;
extern int do_typedef() ;
extern int parse_decl() ;
extern int do_structlist() ;
extern int add_structlist() ;
extern int strparse() ;
extern int strfree() ;
extern int do_sizeof() ;
extern memtbl_t * all_tbl_find() ;
extern memtbl_t * all_tbl_search() ;
extern memtbl_t * asl_find() ;

#else

extern memtbl_t * ffind( 
                        memtbl_t *tbl,
                        char *fld,
                        char **pptr) ;
extern int do_struct( 
                        int argc,
                        char **argv) ;
extern int do_typedef( 
                        int argc,
                        char **argv) ;
extern int parse_decl( 
                        char * argv0,
                        struct memtbl *mem,
                        char *decl,
                        int tst) ;
extern int do_structlist( 
                        int argc,
                        char **argv) ;
extern int add_structlist( 
                        struct memtbl **memptr,
                        char *prefix,
                        int id) ;
extern int strparse( 
                        memtbl_t *tbl,
                        char **pbuf,
                        char *val) ;
extern int strfree( 
                        char *buf,
                        char *type) ;
extern int do_sizeof( 
                        int argc,
                        char **argv) ;
extern memtbl_t * all_tbl_find( 
                        char *name,
                        int tbl,
                        long id) ;
extern memtbl_t * all_tbl_search( 
                        char *name,
                        int flag) ;
extern memtbl_t * asl_find( 
                        memtbl_t *ptbl,
                        memtbl_t *tbls,
                        char *fld,
                        char **pptr) ;

#endif /* _NO_PROTO */


#endif /* _Dtksh_struct_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
