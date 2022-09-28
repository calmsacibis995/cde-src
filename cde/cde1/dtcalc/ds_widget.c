/* $XConsortium: ds_widget.c /main/cde1_maint/1 1995/07/17 19:34:00 drk $ */
/*									*
 *  ds_widet.c                                                          *
 *   Contains some common functions which create some Xm widget which   *
 *   are used throughout the Desktop Calculator.                        *
 *                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc. 				*
 */

#include <stdio.h>
#include "ds_widget.h"

#ifndef  LINT_CAST
#ifdef   lint
#define  LINT_CAST(arg)    (arg ? 0 : 0)
#else
#define  LINT_CAST(arg)    (arg)
#endif /*lint*/
#endif /*LINT_CAST*/

struct tW_struct *
make_textW(owner, label)
Widget owner ;
char *label ;
{
  struct tW_struct *w ;
 
  w = (struct tW_struct *) LINT_CAST(calloc(1, sizeof(struct tW_struct))) ;
 
  w->manager = XtVaCreateManagedWidget("manager",
                                       xmRowColumnWidgetClass,
                                       owner,
                                       XmNorientation,  XmHORIZONTAL,
                                       XmNnumColumns,   1,
                                       NULL) ;
  w->label = XtVaCreateManagedWidget(label,
                                     xmLabelWidgetClass,
                                     w->manager,
                                     XmNalignment, XmALIGNMENT_BEGINNING,
                                     NULL) ;
  w->textfield = XtVaCreateManagedWidget("textfield",
                                         xmTextFieldWidgetClass,
                                         w->manager,
                                         NULL) ;
  return(w) ;
}

void
set_text_str(w, ttype, str)
struct tW_struct *w ;
enum text_type ttype ;
char *str ;
{
  XmString cstr ;

  switch (ttype)
    {
      case T_LABEL : cstr = XmStringCreateSimple(str) ;
                     XtVaSetValues(w->label, XmNlabelString, cstr, 0) ;
                     XmStringFree(cstr) ;
                     break ;
      case T_VALUE : XmTextFieldSetString(w->textfield, str) ;
    }
}
