/* $XConsortium: exksh_prpar.h /main/cde1_maint/1 1995/07/17 23:10:12 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        exksh_prpar.h
 **
 **   Project:     CDE
 **
 **   Description: Public include file for exksh_prpar.c
 **
 **
 **   (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992
 **       by Hewlett-Packard Company
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _Dtksh_exksh_prpar_h
#define _Dtksh_exksh_prpar_h

extern unsigned int Pr_format;


#ifdef _NO_PROTO

extern int xk_skipwhite() ;
extern int xk_backskip() ;
extern int xk_parexpect() ;
extern int xk_parpeek() ;
extern int xk_prin_int() ;
extern int xk_par_int() ;
extern int xk_prin_nts() ;
extern int xk_prin_charstr() ;
extern int xk_prin_hexstr() ;
extern int xk_par_chararr() ;
extern int xk_par_nts() ;
extern int xk_par_charstr() ;
extern int xk_Strncmp() ;

#else

extern int xk_skipwhite( 
                        char **buf) ;
extern int xk_backskip( 
                        char **buf,
                        int *n) ;
extern int xk_parexpect( 
                        char **buf,
                        char *str) ;
extern int xk_parpeek( 
                        char **buf,
                        char *str) ;
extern int xk_prin_int( 
                        memtbl_t *tbl,
                        char **buf,
                        unsigned long *old_v) ;
extern int xk_par_int( 
                        char **buf,
                        long *v,
                        struct envsymbols *env) ;
extern int xk_prin_nts( 
                        char **buf,
                        char *str) ;
extern int xk_prin_charstr( 
                        char **buf,
                        unsigned char *str,
                        int len) ;
extern int xk_prin_hexstr( 
                        char **buf,
                        char *str,
                        int len) ;
extern int xk_par_chararr( 
                        char **buf,
                        char *str,
                        int *len) ;
extern int xk_par_nts( 
                        char **buf,
                        char **str) ;
extern int xk_par_charstr( 
                        char **buf,
                        char **str,
                        int *len) ;
extern int xk_Strncmp( 
                        char *s1,
                        char *s2,
                        int len) ;

#endif /* _NO_PROTO */


#endif /* _Dtksh_exksh_prpar_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
