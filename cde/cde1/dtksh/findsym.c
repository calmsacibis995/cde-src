/* $XConsortium: findsym.c /main/cde1_maint/1 1995/07/17 23:11:18 drk $ */

/*	Copyright (c) 1991, 1992 UNIX System Laboratories, Inc. */
/*	All Rights Reserved     */

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF    */
/*	UNIX System Laboratories, Inc.			*/
/*	The copyright notice above does not evidence any       */
/*	actual or intended publication of such source code.    */

#include "stdio.h"
#include <sys/types.h>

#ifdef DYNLIB
#ifdef __aix
#include <sys/ldr.h>
#else
#include <dlfcn.h>
#endif
#endif
#ifdef HPUX_DYNLIB
#include <dl.h>
#endif

#include <string.h>
#include <search.h>
#include <ctype.h>
#include "xmdtksym.h"
#include "msgs.h"


/*
 * This function is currently only used to locate a widget class record,
 * as requested by a DtLoadWidget request.  In the future, if the exksh
 * commands are ever added back in, then it will also need to be able
 * to locate any arbitrary symbol.
 */

unsigned long
#ifdef _NO_PROTO
fsym( str, lib )
        char *str ;
        int lib ;
#else
fsym(
        char *str,
        int lib )
#endif /* _NO_PROTO */
{
#ifdef DYNLIB
   void ** liblist;
   int i = 0;
   long addr;
#endif
#ifdef HPUX_DYNLIB
   void *found;
   shl_t handle;
#endif

#ifdef DYNLIB
   if ((liblist = sh_getliblist()) == NULL)
        return(NULL);

   while (liblist[i])
   {
      if (addr = dlsym(liblist[i], str))
         return((unsigned long)addr);
      i++;
   }
#else
#ifdef HPUX_DYNLIB
   handle = NULL;
   if ((shl_findsym(&handle, str, TYPE_PROCEDURE, &found)) == 0)
      return((unsigned long) found);
   if ((shl_findsym(&handle, str, TYPE_DATA, &found)) == 0)
      return((unsigned long) found);
   handle = PROG_HANDLE;
   if ((shl_findsym(&handle, str, TYPE_PROCEDURE, &found)) == 0)
      return((unsigned long) found);
   if ((shl_findsym(&handle, str, TYPE_DATA, &found)) == 0)
      return((unsigned long) found);
#endif
#endif

   return(NULL);
}
