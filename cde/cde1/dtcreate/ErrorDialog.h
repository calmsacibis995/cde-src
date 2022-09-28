/* $XConsortium: ErrorDialog.h /main/cde1_maint/1 1995/07/17 20:25:43 drk $ */

/*******************************************************************************
       ErrorDialog.h
       This header file is included by ErrorDialog.c

*******************************************************************************/

#ifndef _ERRORDIALOG_H_INCLUDED
#define _ERRORDIALOG_H_INCLUDED


#include <stdio.h>
#include <Xm/MessageB.h>

extern Widget   ErrorDialog;

/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

#ifdef _NO_PROTO

void    display_error_message ();

#else /* _NO_PROTO */

void    display_error_message (Widget parent, char *message);

#endif /* _NO_PROTO */

#endif  /* _ERRORDIALOG_H_INCLUDED */
