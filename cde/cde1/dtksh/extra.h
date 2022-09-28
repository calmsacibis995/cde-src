/* $XConsortium: extra.h /main/cde1_maint/1 1995/07/17 23:11:05 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        extra.h
 **
 **   Project:     CDE
 **
 **   Description: Public include file for extra.c
 **
 **
 **   (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992
 **       by Hewlett-Packard Company
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _Dtksh_extra_h
#define _Dtksh_extra_h


#ifdef _NO_PROTO

extern void env_set() ;
extern void env_set_gbl() ;
extern char * env_get() ;
extern void * xkhash_init() ;
extern void xkhash_override() ;
extern void * xkhash_find() ;
extern void xkhash_add() ;
extern int ksh_eval() ;
extern void env_set_var() ;
extern void env_blank() ;
extern void printerr() ;
extern void printerrf() ;

#else

extern void env_set( 
                        char *var) ;
extern void env_set_gbl( 
                        char *vareqval) ;
extern char * env_get( 
                        char *var) ;
extern void * xkhash_init( 
                        int num) ;
extern void xkhash_override( 
                        Hash_table_t *tbl,
                        const char *name,
                        void *val) ;
extern void * xkhash_find( 
                        Hash_table_t *tbl,
                        const char *name) ;
extern void xkhash_add( 
                        Hash_table_t *tbl,
                        const char *name,
                        char *val) ;
extern int ksh_eval( 
                        char *cmd) ;
extern void env_set_var( 
                        char *var,
                        char *val) ;
extern void env_blank( 
                        char *var) ;
extern void printerr( 
                        char *cmd,
                        char *msg1,
                        char *msg2) ;
extern void printerrf( 
                        char *cmd,
                        char *fmt,
                        char *arg0,
                        char *arg1,
                        char *arg2,
                        char *arg3,
                        char *arg4,
                        char *arg5,
                        char *arg6,
                        char *arg7) ;

#endif /* _NO_PROTO */


#endif /* _Dtksh_extra_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
