/* $XConsortium: extra.c /main/cde1_maint/3 1995/10/13 23:50:10 montyb $ */

/*	Copyright (c) 1991, 1992 UNIX System Laboratories, Inc. */
/*	All Rights Reserved     */

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF    */
/*	UNIX System Laboratories, Inc.			*/
/*	The copyright notice above does not evidence any       */
/*	actual or intended publication of such source code.    */


#include	"defs.h"
#include	"shell.h"
#include	"name.h"
#include	"stdio.h"
#include	"msgs.h"


void
#ifdef _NO_PROTO
env_set( var )
        char *var ;
#else
env_set(
        char *var )
#endif /* _NO_PROTO */
{
	(void)nv_open(var, sh.var_tree, NV_ASSIGN);
}

void
#ifdef _NO_PROTO
env_set_gbl( vareqval )
        char *vareqval ;
#else
env_set_gbl(
        char *vareqval )
#endif /* _NO_PROTO */
{
        env_set(vareqval);
}

char *
#ifdef _NO_PROTO
env_get( var )
        char *var ;
#else
env_get(
        char *var )
#endif /* _NO_PROTO */
{
	Namval_t *np;
	char *val;

	np = nv_search(var,sh.var_tree,0);
	if (np && (val = nv_getval(np)))
		return(val);
	return(NULL);
}


void *
#ifdef _NO_PROTO
xkhash_init( num )
        int num ;
#else
xkhash_init(
        int num )
#endif /* _NO_PROTO */
{
	return((void *) hashalloc(NULL,0));
}

void
#ifdef _NO_PROTO
xkhash_override( tbl, name, val )
        Hash_table_t *tbl ;
        const char *name ;
        void *val ;
#else
xkhash_override(
        Hash_table_t *tbl,
        const char *name,
        void *val )
#endif /* _NO_PROTO */
{
	hashput(tbl, name, val);
}

void *
#ifdef _NO_PROTO
xkhash_find( tbl, name )
        Hash_table_t *tbl ;
        const char *name ;
#else
xkhash_find(
        Hash_table_t *tbl,
        const char *name )
#endif /* _NO_PROTO */
{
	return(hashget(tbl, name));
}

void
#ifdef _NO_PROTO
xkhash_add( tbl, name, val )
        Hash_table_t *tbl ;
        const char *name ;
        char *val ;
#else
xkhash_add(
        Hash_table_t *tbl,
        const char *name,
        char *val )
#endif /* _NO_PROTO */
{
	hashput(tbl, name, val);
}

int
#ifdef _NO_PROTO
ksh_eval( cmd )
        char *cmd ;
#else
ksh_eval(
        char *cmd )
#endif /* _NO_PROTO */
{
        sh_eval(sfopen(NIL(Sfile_t*),cmd,"s"),0);
        sfsync(sh.outpool);
	return(sh.exitval);
}

void
#ifdef _NO_PROTO
env_set_var( var, val )
        char *var ;
        char *val ;
#else
env_set_var(
        char *var,
        char *val )
#endif /* _NO_PROTO */
{
	register int len;
	char tmp[512];
	char *set = &tmp[0];

	if ((len = strlen(var) + strlen(val?val:"") + 2) > sizeof(tmp)) /* 11/06 CHANGED */
		set = malloc(len);
	strcpy(set, var);
	strcat(set, "=");
	strcat(set, val?val:""); 			/* 11/06 CHANGED */
	env_set(set);
	if (set != &tmp[0])
		free(set);
}

void
#ifdef _NO_PROTO
env_blank( var )
        char *var ;
#else
env_blank(
        char *var )
#endif /* _NO_PROTO */
{
	env_set_var(var, "");
}

void
#ifdef _NO_PROTO
printerr( cmd, msg1, msg2 )
        char *cmd ;
        char *msg1 ;
        char *msg2 ;
#else
printerr(
        char *cmd,
        char *msg1,
        char *msg2 )
#endif /* _NO_PROTO */
{
	if (msg1 == NULL)
		msg1 = "";
	if (msg2 == NULL)
		msg2 = "";
        if (cmd && (strlen(cmd) > 0))
	   printf( "%s: %s %s\n", cmd, msg1, msg2);
        else
	   printf( "%s %s\n", msg1, msg2);
}

void
#ifdef _NO_PROTO
printerrf( cmd, fmt, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7 )
        char *cmd ;
        char *fmt ;
        char *arg0 ;
        char *arg1 ;
        char *arg2 ;
        char *arg3 ;
        char *arg4 ;
        char *arg5 ;
        char *arg6 ;
        char *arg7 ;
#else
printerrf(
        char *cmd,
        char *fmt,
        char *arg0,
        char *arg1,
        char *arg2,
        char *arg3,
        char *arg4,
        char *arg5,
        char *arg6,
        char *arg7 )
#endif /* _NO_PROTO */
{
	char buf[2048];
	if (arg0 == NULL)
		arg0 = "";
	if (arg1 == NULL)
		arg1 = "";
	if (arg2 == NULL)
		arg2 = "";
	if (arg3 == NULL)
		arg3 = "";
	if (arg4 == NULL)
		arg4 = "";
	if (arg5 == NULL)
		arg5 = "";
	if (arg6 == NULL)
		arg6 = "";
	if (arg7 == NULL)
		arg7 = "";

	sprintf(buf, fmt, arg0, arg1, arg2, arg3,arg4, arg5, arg6, arg7);
        if (cmd && (strlen(cmd) > 0))
	   printf("%s: %s\n", cmd, buf);
        else
	   printf("%s\n", buf);
}
