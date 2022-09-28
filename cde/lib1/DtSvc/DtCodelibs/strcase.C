/*
 * File:	strcase.C $XConsortium: strcase.C /main/cde1_maint/1 1995/07/14 20:07:50 drk $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */

#include <codelibs/nl_hack.h>
#include "stringx.h"

char *strupper(char *str)
{
    int len;

    if (str != NULL)
      {
	for (register char *s = str; *s != '\0'; s++)
	  if ((len = mblen(s, MB_CUR_MAX)) > 1)
	    s += len;
	  else
	    *s = toupper((unsigned char)*s);
      }

    return str;
}

char *strlower(char *str)
{
    int len;

    if (str != NULL)
      {
	for (register char *s = str; *s != '\0'; s++)
	  if ((len = mblen(s, MB_CUR_MAX)) > 1)
	    s += len;
	  else
	    *s = tolower((unsigned char)*s);
      }

    return str;
}
