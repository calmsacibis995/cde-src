/*
 * File:         pathwexp.c $XConsortium: pathwexp.c /main/cde1_maint/1 1995/07/14 20:16:31 drk $
 * Language:     C
 *
 * (c) Copyright 1988, Hewlett-Packard Company, all rights reserved.
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc.				*
 */

#include <bms/sbport.h>
#include <bms/pathwexp.h>

#include <codelibs/shellutils.h>

/*------------------------------------------------------------------------+*/
XeString Xe_shellexp(XeString path)
/*------------------------------------------------------------------------+*/
{
   int num;
   XeString *res = (XeString *) shellscan(path, &num,
			 SHX_NOGLOB | SHX_NOSPACE | SHX_NOMETA | SHX_NOGRAVE);
      
   if (num == 0)
      return NULL;
   else
      return res[0];
}

