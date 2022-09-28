/* $XConsortium: ds_common.c /main/cde1_maint/1 1995/07/17 19:32:42 drk $ */
/*                                                                      *
 *  ds_common.c                                                         *
 *   Contains some common functions used throughout the Desktop         *
 *   Calculator.                                                        *
 *                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc. 				*
 */

#include <stdio.h>
#include "ds_common.h"

#define  FREE    (void) free
#define  STRCPY  (void) strcpy


void
read_str(str, value)
char **str, *value ;
{
  if (*str != NULL) FREE(*str) ;
  if (value != NULL && strlen(value))
    {
      *str = (char *) malloc((unsigned) (strlen(value) + 1)) ;
      STRCPY(*str, value) ;
    }
  else *str = NULL ;
}


char *
set_bool(value)
int value ;
{
  return((value) ? "true" : "false") ;
}
