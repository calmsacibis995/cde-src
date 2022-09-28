/* $XConsortium: widget.h /main/cde1_maint/1 1995/07/18 00:59:06 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        widget.h
 **
 **   Project:     CDE
 **
 **   Description: Public include file for widget.c
 **
 **
 **   (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992
 **       by Hewlett-Packard Company
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _Dtksh_widget_h
#define _Dtksh_widget_h

#include <X11/X.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#define NO_AST
#include "dtksh.h"
#undef NO_AST

extern int Wtab_free;
extern wtab_t **W;
extern int NumW;
extern int MaxW;


#ifdef _NO_PROTO

extern void init_widgets() ;
extern classtab_t * str_to_class() ;
extern Widget DtkshNameToWidget() ;
extern wtab_t * str_to_wtab() ;
extern Widget handle_to_widget() ;
extern wtab_t * widget_to_wtab() ;
extern void get_new_wtab() ;
extern int do_DtLoadWidget() ;
extern int do_DtWidgetInfo() ;
extern void alt_env_set_var() ;

#else

extern void init_widgets( void ) ;
extern classtab_t * str_to_class( 
                        char *arg0,
                        char *s) ;
extern Widget DtkshNameToWidget( 
                        String s) ;
extern wtab_t * str_to_wtab( 
                        char *arg0,
                        char *v) ;
extern Widget handle_to_widget( 
                        char *arg0,
                        char *handle) ;
extern wtab_t * widget_to_wtab( 
                        Widget w) ;
extern void get_new_wtab( 
                        wtab_t **w,
                        char *name) ;
extern int do_DtLoadWidget( 
                        int argc,
                        char *argv[]) ;
extern int do_DtWidgetInfo( 
                        int argc,
                        char *argv[]) ;
extern void alt_env_set_var( 
                        char *variable,
                        char *value) ;

#endif /* _NO_PROTO */


#endif /* _Dtksh_widget_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
