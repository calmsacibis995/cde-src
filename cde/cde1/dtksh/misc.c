/* $XConsortium: misc.c /main/cde1_maint/1 1995/07/18 00:56:41 drk $ */
/*	Copyright (c) 1991, 1992 UNIX System Laboratories, Inc. */
/*	All Rights Reserved     */

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF    */
/*	UNIX System Laboratories, Inc.			*/
/*	The copyright notice above does not evidence any       */
/*	actual or intended publication of such source code.    */

#include        "name.h"
#include        "shell.h"
#include "stdio.h"
#include <X11/X.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#define NO_AST
#include "dtksh.h"
#undef NO_AST
#include "exksh.h" /* which includes sys/types.h */
/* #include <sys/param.h> */
#include <string.h>
/* #include <search.h> */
#include <ctype.h>
#include "docall.h"
#include "msgs.h"

static char use[] = "0x%x";
static char use2[] = "%s=0x%x";

int
#ifdef _NO_PROTO
symcomp( sym1, sym2 )
        void *sym1 ;
        void *sym2 ;
#else
symcomp(
        void *sym1,
        void *sym2 )
#endif /* _NO_PROTO */

{
	return(strcmp(((struct symarray *) sym1)->str, ((struct symarray *) sym2)->str));
}


void *
#ifdef _NO_PROTO
getaddr( str )
        char *str ;
#else
getaddr(
        char *str )
#endif /* _NO_PROTO */
{
	if (isdigit(str[0]))
		return((void *) strtoul(str, NULL, 0));
	else
		return((void *) fsym(str, -1));
}

int
#ifdef _NO_PROTO
do_deref( argc, argv )
        int argc ;
        char **argv ;
#else
do_deref(
        int argc,
        char **argv )
#endif /* _NO_PROTO */
{
	unsigned char *ptr;
	long i, len = 0;
	short longwise = -1;
	char printit = 0;
	static char xk_ret_buffer[1024];
        char * errmsg;

	for (i = 1; (i < argc) && argv[i] != NULL && argv[i][0] == '-'; i++) {
		if (isdigit(argv[i][1])) {
			if (longwise < 0)
				longwise = 0;
			ptr = (unsigned char *) argv[i] + 1;
			xk_par_int(&ptr, &len, NULL);
			if (!len) {
			   errmsg = strdup(GETMESSAGE(10,1, 
                              "An invalid length parameter was specified: %s"));
			   printerrf(argv[0], errmsg, 
                                    argv[i], NULL, NULL, NULL, NULL, NULL,
                                    NULL, NULL);
                           free(errmsg);
			   return(SH_FAIL);
			}
		}
		else if (argv[i][1] == 'l')
			longwise = 1;
		else if (argv[i][1] == 'p')
			printit = 1;
	}
	if (longwise < 0)
		longwise = 1;
	if (!len)
		len = sizeof(long);
	if (i >= argc) {
		XK_USAGE(argv[0]);
	}	
	ptr = (unsigned char *) getaddr(argv[i++]);
	if (ptr) {
		if ( ((i < argc) && argv[i]) || printit) {
			char *dbuf, *p;
			int totlen;
			char buf[10 * BUFSIZ];
			int incr;

			if (printit)
				totlen = len + 1 + 1;
			else
				totlen = len + strlen(argv[i]) + 1 + 1;
			dbuf = (char *) (totlen < (10 * BUFSIZ - 1)) ? buf : malloc(totlen);
			if (printit)
				strcpy(dbuf, "0x");
			else
				sprintf(dbuf, "%s=0x", argv[i]);
			p = dbuf + strlen(dbuf);
			incr = longwise ? sizeof(long) : sizeof(char);
			for (i=0; i < len; i += incr, p += 2 * incr)
				sprintf(p, "%*.*x", incr * 2, incr * 2, longwise ? *((unsigned long *) (ptr + i)) : (unsigned long) (ptr[i]));
			if (printit)
				ALTPUTS(dbuf);
			else
				env_set(dbuf);
			if (dbuf != buf)
				free(dbuf);
		}
		else {
			if (len > sizeof(unsigned long)) {
                           char tmpBuf[30];

                           sprintf(tmpBuf, "%d", sizeof(unsigned long));
                           errmsg = strdup(GETMESSAGE(10,2, 
                              "To set RET, the length must be less than: %s"));
			   printerrf(argv[0], errmsg,
                                 tmpBuf, NULL, NULL, NULL, NULL, NULL, NULL,
                                 NULL);
                           free(errmsg);
			   return(SH_FAIL);
			}
			sprintf(xk_ret_buffer, use, *((unsigned long *) ptr));
			xk_ret_buf = xk_ret_buffer;
		}
		return(SH_SUCC);
	}
        errmsg = strdup(GetSharedMsg(DT_UNDEF_SYMBOL));
	printerrf(argv[0], errmsg, argv[--i], NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
        free(errmsg);
	return(SH_FAIL);
}

void *
#ifdef _NO_PROTO
nop( var )
        void *var ;
#else
nop(
        void *var )
#endif /* _NO_PROTO */
{
	return(var);
}

void *
#ifdef _NO_PROTO
save_alloc( var )
        void *var ;
#else
save_alloc(
        void *var )
#endif /* _NO_PROTO */
{
	return(var);
}
