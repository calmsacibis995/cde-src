/* $XConsortium: genlib.c /main/cde1_maint/1 1995/07/17 23:11:31 drk $ */

/*	Copyright (c) 1991, 1992 UNIX System Laboratories, Inc. */
/*	All Rights Reserved     */

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF    */
/*	UNIX System Laboratories, Inc.			*/
/*	The copyright notice above does not evidence any       */
/*	actual or intended publication of such source code.    */

#include "stdio.h"
#include <sys/types.h>

#ifdef DYNLIB
#include <dlfcn.h>
#endif
#ifdef HPUX_DYNLIB
#include <dl.h>
#endif

#include <string.h>
#include <search.h>
#include <ctype.h>
#include "docall.h"
#include "exksh.h"
#include "misc.h"
#include "xmdtksym.h"
#include "msgs.h"

#ifdef _NO_PROTO

static char * ReturnUsageMsg();

#else

static char * ReturnUsageMsg(
                       int msgIndex);

#endif /* _NO_PROTO */

struct libstruct *All_libs;
static int Nlibs;
int sh_errno;

static struct usage {
        char *funcname;
        int msgId;
} Xk_usage[] = {
   "call",        0,
   "define",      1,
   "deflist",     2,
   "deref",       3,
   "field_comp",  4,
   "field_get",   5,
   "finddef",     6,
   "findsym",     7,
   "sizeof",      8,
   "struct",      9,
   "structlist", 10,
   "symbolic",   11,
   "typedef",    12,
};


static char *
#ifdef _NO_PROTO
ReturnUsageMsg( msgId )
        int funcname ;
#else
ReturnUsageMsg(
        int msgId )
#endif /* _NO_PROTO */
{
   char * errmsg;

   switch (msgId)
   {
      case 0: errmsg = GETMESSAGE(9,1, "call [-F] [-n] [-r] function [arg] ... [++] [argModifier ...]");
              return(errmsg);
      case 1: errmsg = GETMESSAGE(9,2, "define [-R] name value");
              return(errmsg);
      case 2: errmsg = GETMESSAGE(9,3, "deflist [-p prefix] address");
              return(errmsg);
      case 3: errmsg = GETMESSAGE(9,4, "deref [-p] [-l] [-len] address [variable]");
              return(errmsg);
      case 4: errmsg = GETMESSAGE(9,5, "field_comp type address [criterion ...]");
              return(errmsg);
      case 5: errmsg = GETMESSAGE(9,6, "field_get [-v variable] type address [fieldName ...]");
              return(errmsg);
      case 6: errmsg = GETMESSAGE(9,7, "finddef definitionName [variable]");
              return(errmsg);
      case 7: errmsg = GETMESSAGE(9,8, "findsym symbolName [variable]");
              return(errmsg);
      case 8: errmsg = GETMESSAGE(9,9, "sizeof typeName [variable]");
              return(errmsg);
      case 9: errmsg = GETMESSAGE(9,10,"struct [-R] name fieldName[:type] ...");
              return(errmsg);
      case 10: errmsg = GETMESSAGE(9,11, "structlist [-i id] [-p prefix] address");
              return(errmsg);
      case 11: errmsg = GETMESSAGE(9,12, "symbolic [-m] -t type ... symbolic ...");
              return(errmsg);
      case 12: errmsg = GETMESSAGE(9,13,"typedef [-R] typeDescriptor typeName");
              return(errmsg);
   }

   return("");
}

int
#ifdef _NO_PROTO
xk_usage( funcname )
        char *funcname ;
#else
xk_usage(
        char *funcname )
#endif /* _NO_PROTO */
{
   int i;
   char * errhdr;
   char * errmsg;

   for (i = 0; i < sizeof(Xk_usage) / sizeof(struct usage); i++)
   {
      if (!funcname)
      {
         errmsg = strdup(ReturnUsageMsg(Xk_usage[i].msgId));
         ALTPUTS(errmsg);
         free(errmsg);
      }
      else if (!funcname || (strcmp(funcname, Xk_usage[i].funcname) == 0))
      {
         errhdr = strdup(GETMESSAGE(9,14, "Usage: %s"));
         errmsg = strdup(ReturnUsageMsg(Xk_usage[i].msgId));
         printerrf("", errhdr, errmsg, NULL, NULL, NULL, NULL, NULL, NULL, 
                   NULL);
         free(errhdr);
         free(errmsg);
         return(SH_SUCC);
      }
   }
   return(funcname ? SH_FAIL : SH_SUCC);
}

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
#if defined(STATICLIB) || ( !defined(DYNLIB) && !defined(HPUX_DYNLIB))
        struct symarray dummy;
#endif
#if defined(DYNLIB)
        int i, j;
#endif
        void *found;

#if defined(STATICLIB) || (!defined(DYNLIB) && !defined(HPUX_DYNLIB))
        dummy.str = str;       
	
if ((found = (void *) bsearch((char *) &dummy, Symarray,
	Symsize-1, sizeof(struct symarray), symcomp)) != NULL)
                return(((struct symarray *) found)->addr);
#endif /* STATICLIB */

#ifdef DYNLIB
        for (i = 0; i < Nlibs; i++)
                for (j = 0; j < All_libs[i].nlibs; j++)
                        if ((found = dlsym(All_libs[i].libs[j].handle,
str)) != NULL)
                                return((unsigned long) found);
#endif /* DYNLIB */
#ifdef HPUX_DYNLIB
        {
                shl_t handle;

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
        }
#endif /* HPUX_DYNLIB */
        return(NULL);
}



int
#ifdef _NO_PROTO
do_findsym( argc, argv )
        int argc ;
        char **argv ;
#else
do_findsym(
        int argc,
        char **argv )
#endif /* _NO_PROTO */
{
	unsigned long found;
	struct symarray dummy;
        char * errmsg;

	if (argc == 1) {
		XK_USAGE(argv[0]);
	}
	if ((found = fsym(argv[1], -1)) != NULL) {
		if (argc >= 3) {
			char buf[50];

			sprintf(buf, "%s=0x%lx", argv[2], found);
			env_set(buf);
		}
		else {
			sprintf(xk_ret_buffer, "0x%x", found);
			xk_ret_buf = xk_ret_buffer;
		}
	}
	else {
                errmsg = strdup(GetSharedMsg(DT_UNDEF_SYMBOL));
		printerrf(argv[0], errmsg, argv[1], NULL, NULL,
                          NULL, NULL, NULL, NULL, NULL);
                free(errmsg);
		return(SH_FAIL);
	}
	return(SH_SUCC);
}

#ifndef SPRINTF_RET_LEN
/*
 * SYSTEM V sprintf() returns the length of the buffer, other versions
 * of the UNIX System don't.  So, if the SPRINTF_RET_LEN flag is not true,
 * then we define an alternate function, lsprintf(), which has the SYSV
 * behavior. Otherwise, lsprintf is #defined in exksh.h to be the
 * same as sprintf.
 */

int
#ifdef _NO_PROTO
lsprintf( buf, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7 )
        char *buf ;
        char *fmt ;
        unsigned long arg1 ;
        unsigned long arg2 ;
        unsigned long arg3 ;
        unsigned long arg4 ;
        unsigned long arg5 ;
        unsigned long arg6 ;
        unsigned long arg7 ;
#else
lsprintf(
        char *buf,
        char *fmt,
        unsigned long arg1,
        unsigned long arg2,
        unsigned long arg3,
        unsigned long arg4,
        unsigned long arg5,
        unsigned long arg6,
        unsigned long arg7 )
#endif /* _NO_PROTO */

{
	sprintf(buf, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
	return(strlen(buf));
}
#endif
