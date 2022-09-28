/* $XConsortium: ds_widget.h /main/cde1_maint/1 1995/07/17 19:34:20 drk $ */
/*									*
 *  ds_widget.h                                                         *
 *   Contains the NO_PROTO defines for functions is ds_widget.h.        *
 *                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc. 				*
 */

#ifndef _ds_widget_h
#define _ds_widget_h

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/Separator.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/TextF.h>

/* For all function declarations, if ANSI then use a prototype. */

#if  defined(__STDC__)
#define P(args)  args
#else  /* ! __STDC__ */
#define P(args)  ()
#endif  /* STDC */

enum choice_type { CH_ACTIVE, CH_VALUE } ;    /* Choice operations. */

/* Text widget operations. */
enum text_type   { T_ACTIVE, T_CARET, T_LABEL, T_VALUE } ;

struct cW_struct {                 /* Choice widget structure. */
  Widget lmanager ;                /* xmRowColumnWidget Class. */
  Widget label ;                   /* xmLabelWidgetClass. */
  Widget cmanager ;                /* xmRowColumnWidgetClass. */
  Widget *choices ;                /* xmToggleButtonWidgetClass. */
  int maxno ;                      /* Number of choice elements. */
  int isexclusive ;                /* Exclusive choice. */
  void (*func)() ;                 /* Callback (if any) for this choice. */
} ;

struct tW_struct {                 /* Text widget structure. */
  Widget manager ;                 /* xmRowColumnWidgetClass. */
  Widget label ;                   /* xmLabelWidgetClass. */
  Widget textfield ;               /* xmTextWidgetClass. */
} ;

int get_choice                  P((struct cW_struct *, enum choice_type)) ;

struct cW_struct *make_choiceW  P((Widget, char *, char **, int, int,
                                   int, void (*)())) ;
struct tW_struct *make_textW    P((Widget,  char *)) ;
 
Widget make_butW         P((Widget, char *)) ;
Widget make_labelW       P((Widget,  char *)) ;
Widget make_manW         P((char *, Widget, int, int)) ;
Widget make_toggleW      P((Widget, char *)) ;

void set_choice          P((struct cW_struct *, enum choice_type, int)) ;
void set_choice_help     P((struct cW_struct *, char *)) ;
void set_text_help       P((struct tW_struct *, char *)) ;
void set_text_str        P((struct tW_struct *, enum text_type, char *)) ;

#endif /*_ds_widget_h*/
