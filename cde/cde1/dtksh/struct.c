/* $XConsortium: struct.c /main/cde1_maint/1 1995/07/18 00:57:32 drk $ */
/*	Copyright (c) 1991, 1992 UNIX System Laboratories, Inc. */
/*	All Rights Reserved     */

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF */
/*	UNIX System Laboratories, Inc.			*/
/*	The copyright notice above does not evidence any       */
/*	actual or intended publication of such source code.    */

#undef printf


#include	"shell.h" 
#include <signal.h>
#include <fcntl.h>
#include <X11/X.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>
#include <X11/StringDefs.h>
#include <Xm/XmStrDefs.h>
#include <Xm/List.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>
#include <Xm/Xm.h>
#include <Xm/Protocols.h>
#include "hash.h"
#include "stdio.h"
#define NO_AST
#include "dtksh.h"
#undef NO_AST
#include "exksh.h"
#include "xmksh.h"
#include "dtkcmds.h"
#include "xmcvt.h"
#include "widget.h"
#include "extra.h"
#include "xmwidgets.h"
#include "struct.h"
#include "basetbl.h"
#include "docall.h"
#include "exksh_tbls.h"
#include "msgs.h"

const static char use[] = "0x%x";
const static char use2[] = "%s=0x%x";

static char structInited = 0;
static void *Hashnams = NULL;

static struct memtbl **Dynmem = NULL;
static int Ndynmem = 0;
static int Sdynmem = 0;

struct structlist {
	char *prefix;
	int id;
	int size;
	struct memtbl **mem;
};

struct structlist *Structlist = NULL;
int Nstructlist;


#ifdef _NO_PROTO

static int freemem() ;
static growmem() ;
static char * endtok() ;
static int chg_structlist() ;
static struct_init() ;

#else

static int freemem( 
                        struct memtbl *mem) ;
static growmem( void ) ;
static char * endtok( 
                        char *start) ;
static int chg_structlist( 
                        struct memtbl **memptr,
                        int id) ;
static struct_init( void ) ;

#endif /* _NO_PROTO */


memtbl_t *
#ifdef _NO_PROTO
ffind( tbl, fld, pptr )
        memtbl_t *tbl ;
        char *fld ;
        char **pptr ;
#else
ffind(
        memtbl_t *tbl,
        char *fld,
        char **pptr )
#endif /* _NO_PROTO */
{
	static memtbl_t tbluse[2];
	memtbl_t *tbl2;
	char *p, *q, op;
	unsigned int len, sub;

	if (!fld || !(*fld))
		return(tbl);
	tbl2 = tbluse;
	tbluse[0] = *tbl;
	tbluse[1] = Null_tbl;
	q = fld;
	while (tbl2 && q && *q) {
		p = q;
		if (*q == '[') {
			if (!tbl2->ptr)
				return(NULL);
			q++;
			xk_par_int(&q, &sub, NULL);
			if (*q != ']')
				return(NULL);
			*pptr = ((char **) (*pptr))[0];
			*pptr += sub * tbl2->size;
			q++;
			tbluse[0].ptr--;
			continue;
		}
		if ((len = strcspn(p, "[.")) < strlen(p)) {
			q = p + len;
			op = *q;
			*q = '\0';
		}
		else
			q = NULL;
		tbl2 = asl_find(NULL, tbluse, p, pptr);
		if (tbl2 && (tbl2 != tbluse)) {
			/* A field should not be a subfield of itself */

			tbluse[0] = *tbl2;
			tbl2 = tbluse;
			tbl2->name = ".";
			tbl2->offset = 0;
		}
		if (q) {
			if (op == '.')
				*q++ = op;
			else
				*q = op;
		}
	}
	return(tbl2);
}

static int
#ifdef _NO_PROTO
freemem( mem )
        struct memtbl *mem ;
#else
freemem(
        struct memtbl *mem )
#endif /* _NO_PROTO */
{
	free(mem->name);
	/*
	int i;

	** Because structures and typedefs now inherit fields (i.e. copy
	** the memtbl entry) we must keep the fields of a structure
	** around permanently, (unless we implement a reference count).
	** Let's keep the code handy in case we do.
		if (mem->kind == K_STRUCT) {
			struct memtbl *fmem;

			fmem = Dynmem[mem->tbl];
			for (i = 0; fmem[i].name; i++) {
				free(fmem[i].name);
				if (fmem[i].tname)
					free(fmem[i].tname);
			}
		}
	*/
	free(mem);
}

static
#ifdef _NO_PROTO
growmem()
#else
growmem( void )
#endif /* _NO_PROTO */
{
	if (!(Dynmem = (struct memtbl **) realloc(Dynmem, (Sdynmem + 20) * sizeof(memtbl_t *))))
		return(SH_FAIL);
	chg_structlist(Dynmem, DYNMEM_ID);
	memset(((char *) Dynmem) + Sdynmem * sizeof(memtbl_t *), '\0', 20 * sizeof(memtbl_t *));
	Sdynmem += 20;
}

int
#ifdef _NO_PROTO
do_struct( argc, argv )
        int argc ;
        char **argv ;
#else
do_struct(
        int argc,
        char **argv )
#endif /* _NO_PROTO */
{
	struct memtbl *mem, *fmem;
	int i, j, argstart, redo;
	char *name, *fname;
	char *p;

        if (!structInited)
           struct_init();

	if (argc > 1 && C_PAIR(argv[1], '-', 'R')) {
		redo = 0;
		argstart = 2;
	}
	else {
		argstart = 1;
		redo = 1;
	}
        if ((argstart + 1) >= argc)
        {
		XK_USAGE(argv[0]);
	}
	name = argv[argstart++];
	for (i = 0; i < Ndynmem; i++)
		if (!(Dynmem[i]->flags & F_FIELD) && (strcmp(name, Dynmem[i]->name) == 0))
			break;
	if ((i < Ndynmem) && !redo) {
		if (!redo)
			return(SH_SUCC);
		if (Sdynmem - Ndynmem < 1)
			growmem();
	}
	else if (Sdynmem - Ndynmem < 2)
		growmem();
	/*
	** Number of memtbls needed: two for structure table and one for
	** each field plus one for null termination.  The number of
	** fields is argc - 2.
	*/
	if (!(mem = (struct memtbl *) malloc(2 * sizeof(struct memtbl))))
		return(SH_FAIL);
	if (!(fmem = (struct memtbl *) malloc((argc - 1) * sizeof(struct memtbl))))
		return(SH_FAIL);
	memset(mem, '\0', 2 * sizeof(struct memtbl));
	memset(fmem, '\0', (argc - 1) * sizeof(struct memtbl));
	if (i < Ndynmem) {
		mem->tbl = Ndynmem++;
		freemem(Dynmem[i]);
		xkhash_override(Hashnams, name, mem);
	}
	else {
		Ndynmem += 2;
		mem->tbl = i + 1;
	}
	Dynmem[i] = mem;
	Dynmem[mem->tbl] = fmem;
	mem->flags = F_TBL_IS_PTR;
	mem->id = DYNMEM_ID;
	mem->name = strdup(name);
	mem->kind = K_STRUCT;
	for (j = argstart; (j < argc) && argv[j]; j++) {
		if (p = strchr(argv[j], ':')) {
			fname = malloc(p - argv[j] + 1);
			strncpy(fname, argv[j], p - argv[j]);
			fname[p - argv[j]] = '\0';
			parse_decl(argv[0], fmem + j - argstart, p + 1, 0);
		}
		else {
			fname = strdup(argv[j]);
			fmem[j - argstart] = T_unsigned_long[0];
		}
		fmem[j - argstart].name = fname;
		fmem[j - argstart].flags |= F_FIELD;
		fmem[j - argstart].delim = 0;
		fmem[j - argstart].offset = mem->size;
		mem->size += (fmem[j - argstart].ptr) ? sizeof(void *) : fmem[j - argstart].size;
	}
	return(SH_SUCC);
}

int
#ifdef _NO_PROTO
do_typedef( argc, argv )
        int argc ;
        char **argv ;
#else
do_typedef(
        int argc,
        char **argv )
#endif /* _NO_PROTO */
{
	struct memtbl *mem;
	int i, redo;
	char *name, *decl;

        if (!structInited)
           struct_init();

        i = 1;
	if (argc > 1 && C_PAIR(argv[i], '-', 'R')) 
        {
		redo = 0;
                i++;
	}
	else 
		redo = 1;

        if ((i + 1) >= argc)
        {
		XK_USAGE(argv[0]);
	}
        decl = argv[i++];
        name = argv[i++];

	for (i = 0; i < Ndynmem; i++)
		if (!(Dynmem[i]->flags & F_FIELD) && (strcmp(name, Dynmem[i]->name) == 0))
			break;
	if ((i < Ndynmem) && !redo) {
		if (!redo)
			return(SH_SUCC);
	}
	else if (Sdynmem - Ndynmem < 1)
		growmem();
	if (!(mem = (struct memtbl *) malloc(2 * sizeof(struct memtbl))))
		return(SH_FAIL);
	mem[1] = Null_tbl;
	if (i < Ndynmem) {
		freemem(Dynmem[i]);
		xkhash_override(Hashnams, name, mem);
	}
	else
		Ndynmem++;
	Dynmem[i] = mem;
	parse_decl(argv[0], mem, decl, 0);
	mem->name = strdup(name);
	return(SH_SUCC);
}

static char *
#ifdef _NO_PROTO
endtok( start )
        char *start ;
#else
endtok(
        char *start )
#endif /* _NO_PROTO */
{
	while(*start && !isspace(*start))
		start++;
	return(start);
}

int
#ifdef _NO_PROTO
parse_decl( argv0, mem, decl, tst )
        char * argv0;
        struct memtbl *mem ;
        char *decl ;
        int tst ;
#else
parse_decl(
        char * argv0,
        struct memtbl *mem,
        char *decl,
        int tst )
#endif /* _NO_PROTO */
{
	struct memtbl *tbl;
	char *p, *end;
	char hold;
	int flag = 0, done;
        char * msg;
        char * errbuf;
        char * errmsg;

	end = decl;
	do {
		p = end;
		xk_skipwhite(&p);
		end = endtok(p);
		hold = *end;
		*end = '\0';
		done = ((strcmp(p, (const char *) "struct") != 0) &&
			(strcmp(p, (const char *) "const") != 0) &&
			(strcmp(p, (const char *) "unsigned") != 0) &&
			(strcmp(p, (const char *) "signed") != 0) &&
			(strcmp(p, (const char *) "union") != 0));
		*end = hold;
	} while (!done && hold);
	if (!p[0]) {
		if (tst) {
			return(FAIL);
		}
		errmsg = strdup(GetSharedMsg(DT_BAD_DECL));
		printerrf(argv0, errmsg,
                           decl,NULL, NULL, NULL, NULL, NULL, NULL, NULL);
                free(errmsg);
		mem[0] = T_unsigned_long[0];
		return(SUCCESS);
	}
	hold = *end;
	*end = '\0';
	tbl = all_tbl_search(p, flag|NOHASH);
	*end = hold;
	if (!tbl) {
		if (tst) {
			return(FAIL);
		}
		errmsg = strdup(GetSharedMsg(DT_BAD_DECL));
		printerrf(argv0, errmsg,
                          decl, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
                free(errmsg);
		mem[0] = T_unsigned_long[0];
		return(SUCCESS);
	}
	mem[0] = tbl[0];
	for (p = end; *p; p++) {
		switch(*p) {
		case '[':
			{
				char *q = strchr(p, ']');

				if (!q) {
                                        errmsg=strdup(GETMESSAGE(12,1, 
                                           "Found a '[' character without a matching ']'"));
					printerr(argv0, errmsg, NULL);
                                        free(errmsg);
					continue;
				}
				p++;
				xk_par_int(&p, &(mem->subscr), NULL);
				mem->flags &= ~(F_SIMPLE);
				if (mem->subscr)
					mem->size *= mem->subscr;
				p = q;
				break;
			}
		case '*':
			if ((mem->kind == K_CHAR) && !(mem->ptr)) {
				char *name;

				name = mem->name;
				mem[0] = T_string_t[0];
				mem->name = name;
			}
			else {
				mem->ptr++;
				mem->flags &= ~(F_SIMPLE);
			}
			break;
		}
	}
	return(SUCCESS);
}

int
#ifdef _NO_PROTO
do_structlist( argc, argv )
        int argc ;
        char **argv ;
#else
do_structlist(
        int argc,
        char **argv )
#endif /* _NO_PROTO */
{
	int i, j, id = 0;
	char *prefix = NULL;
	struct memtbl **memptr = NULL;
        char * errmsg;

	for (i = 1; (i < argc) && argv[i]; i++) {
		if (argv[i][0] == '-') {
			for (j = 1; argv[i][j]; j++) {
				switch(argv[i][j]) {
				case 'i':
				   if (argv[i][j + 1])
					fdef(argv[i] + j + 1, &id);
				   else
					fdef(argv[++i], &id);
				   j = strlen(argv[i]) - 1;
			  	   break;
				case 'p':
				   if (argv[i][j + 1])
					prefix = argv[i] + j + 1;
				   else
					prefix = argv[++i];
				   j = strlen(prefix) - 1;
				   break;
				default:
                                   errmsg = strdup(GetSharedMsg(
                                                    DT_UNKNOWN_OPTION));
				   printerrf(argv[0], errmsg,
                                            argv[i], NULL, NULL, NULL, NULL,
                                            NULL, NULL, NULL);
                                   free(errmsg);
                                   xk_usage(argv[0]);
				   return(SH_FAIL);
				}
			}
		}
		else {
			if ((memptr = (memtbl_t **) getaddr(argv[i])) == NULL) 
                        {
                           errmsg=strdup(GETMESSAGE(12,2, 
                                "Unable to locate the following symbol: %s"));
			   printerrf(argv[0], errmsg, argv[i],
                                          NULL, NULL, NULL, NULL, NULL, NULL,
                                          NULL);
                           free(errmsg);
			   return(SH_FAIL);
			}
		}
	}

        if (memptr == NULL)
        {
           XK_USAGE(argv[0]);
        }

	for (i = 0; i < Nstructlist; i++)
        {
		if ((Structlist[i].mem == memptr) && 
                    (!prefix || (Structlist[i].prefix && 
                        (strcmp(Structlist[i].prefix, prefix) == 0))) &&
                    (!id || (Structlist[i].id == id)))
                {
			return(SH_SUCC);
                }
        }

	add_structlist(memptr, prefix, id);
}

static int
#ifdef _NO_PROTO
chg_structlist( memptr, id )
        struct memtbl **memptr ;
        int id ;
#else
chg_structlist(
        struct memtbl **memptr,
        int id )
#endif /* _NO_PROTO */
{
	int i;

	for (i = 0; i < Nstructlist; i++)
		if (Structlist[i].id == id) {
			Structlist[i].mem = memptr;
			return;
		}
}

int
#ifdef _NO_PROTO
add_structlist( memptr, prefix, id )
        struct memtbl **memptr ;
        char *prefix ;
        int id ;
#else
add_structlist(
        struct memtbl **memptr,
        char *prefix,
        int id )
#endif /* _NO_PROTO */
{
	int i;

	if (!Structlist)
		Structlist = (struct structlist *) malloc((Nstructlist + 1) * sizeof(struct structlist));
	else
		Structlist = (struct structlist *) realloc(Structlist, (Nstructlist + 1) * sizeof(struct structlist));
	if (!Structlist)
		return(SH_FAIL);
	Structlist[Nstructlist].mem = memptr;
	Structlist[Nstructlist].id = id;
	Structlist[Nstructlist].prefix = prefix ? strdup(prefix) : (char *)NULL;
	if (memptr[0] && memptr[0][0].name) {
		for (i = 1; memptr[i] && memptr[i][0].name && memptr[i][0].name[0]; i++)
			if (strcmp(memptr[i][0].name, memptr[i - 1][0].name) < 0)
				break;
		if (!(memptr[i] && memptr[i][0].name && memptr[i][0].name[0]))
			Structlist[Nstructlist].size = i - 1;
		else
			Structlist[Nstructlist].size = -1;
	}
	else
			Structlist[Nstructlist].size = 0;
	Nstructlist++;
	return(SH_SUCC);
}

int
#ifdef _NO_PROTO
strparse( tbl, pbuf, val )
        memtbl_t *tbl ;
        char **pbuf ;
        char *val ;
#else
strparse(
        memtbl_t *tbl,
        char **pbuf,
        char *val )
#endif /* _NO_PROTO */
{
	char *p, *phold;
	int ret;

	if (!IS_SIMPLE(tbl) && !tbl->ptr && !(tbl->flags & F_TYPE_IS_PTR))
		tbl->ptr = 1;
	phold = p = strdup(val);
	ret = XK_PARSE(tbl, &p, (char *)pbuf, 0, 0, NULL, all_tbl_find);
	free(phold);
	return(ret != FAIL);
}

int
#ifdef _NO_PROTO
strfree( buf, type )
        char *buf ;
        char *type ;
#else
strfree(
        char *buf,
        char *type )
#endif /* _NO_PROTO */
{
	memtbl_t tbl;

	if (parse_decl("strfree", &tbl, type, 1) == FAIL)
		return(SH_FAIL);
	if (!IS_SIMPLE(&tbl) && !tbl.ptr && !(tbl.flags & F_TYPE_IS_PTR))
		tbl.ptr = 1;
	if (XK_FREE(&tbl, (char *)&buf, 0, 0, all_tbl_find) == FAIL)
		return(SH_FAIL);
	return(SH_SUCC);
}

int
#ifdef _NO_PROTO
do_sizeof( argc, argv )
        int argc ;
        char **argv ;
#else
do_sizeof(
        int argc,
        char **argv )
#endif /* _NO_PROTO */
{
	memtbl_t *tbl;
        char * errmsg;

	if (argc <= 1) {
		XK_USAGE(argv[0]);
	}
	if ((tbl = all_tbl_search(argv[1], 0)) == NULL) {
                errmsg=strdup(GETMESSAGE(12,3, 
                  "The following is not a valid data type or structure name: %s"));
		printerrf(argv[0], errmsg, argv[1], NULL, NULL,
                          NULL, NULL, NULL, NULL, NULL);
                free(errmsg);
		return(SH_FAIL);
	}
        if (argc >= 3) {
		char buf[50];

		sprintf(buf, use2, argv[2], tbl->ptr ? sizeof(void *) : tbl->size);
		env_set(buf);
	}
	else {
		sprintf(xk_ret_buffer, use, tbl->ptr ? sizeof(void *) : tbl->size);
		xk_ret_buf = xk_ret_buffer;
	}
	return(SH_SUCC);
}

memtbl_t *
#ifdef _NO_PROTO
all_tbl_find( name, tbl, id )
        char *name ;
        int tbl ;
        long id ;
#else
all_tbl_find(
        char *name,
        int tbl,
        long id )
#endif /* _NO_PROTO */
{
	int i;

	if (tbl != -1) {
		for (i = 0; i < Nstructlist; i++)
			if (id == Structlist[i].id)
				return(Structlist[i].mem[tbl]);
		return(NULL);
	}
	return(all_tbl_search(name, TYPEONLY));
}

memtbl_t *
#ifdef _NO_PROTO
all_tbl_search( name, flag )
        char *name ;
        int flag ;
#else
all_tbl_search(
        char *name,
        int flag )
#endif /* _NO_PROTO */
{
	register int i;
	void *found;

        if (!structInited)
           struct_init();

	if (found = (void *) xkhash_find(Hashnams, name))
		return((memtbl_t *) found);
	else {
		register int j;
		register memtbl_t **subtbl;

		for (i = 0; i < Nstructlist; i++) {
			if (subtbl = Structlist[i].mem)
				for (j = 0; subtbl[j]; j++)
					if (!(subtbl[j]->flags & F_FIELD) && (strcmp(name, subtbl[j]->name) == 0) && ((subtbl[j]->kind != K_TYPEDEF) || (subtbl[j]->tbl != -1))) {
						if (!(flag & NOHASH))
							xkhash_add(Hashnams, name, (char *)subtbl[j]);
						return(subtbl[j]);
					}
		}
	}
	return(NULL);
}

memtbl_t *
#ifdef _NO_PROTO
asl_find( ptbl, tbls, fld, pptr )
        memtbl_t *ptbl ;
        memtbl_t *tbls ;
        char *fld ;
        char **pptr ;
#else
asl_find(
        memtbl_t *ptbl,
        memtbl_t *tbls,
        char *fld,
        char **pptr )
#endif /* _NO_PROTO */
{
	int i;
	memtbl_t *tbl;

	if (!Structlist)
		return(NULL);
	if (!pptr && (ptbl == tbls))
		return(NULL);
	for (i = 0; tbls[i].name; i++) {
		if ((xk_Strncmp(tbls[i].name, fld, strlen(fld)) == 0) && (strlen(fld) == strlen(tbls[i].name))) {
			if (pptr && ptbl && ((ptbl->kind == K_STRUCT) || (ptbl->kind == K_ANY)))
				*pptr += tbls[i].offset;
			return(tbls + i);
		}
	}
	for (i = 0; tbls[i].name; i++) {
		if ((tbls[i].kind == K_TYPEDEF) || (tbls[i].kind == K_STRUCT) || (tbls[i].kind == K_UNION) || (tbls[i].kind == K_ANY)) {
			char *hold;

			if (!pptr) {
				if ((tbl = asl_find(tbls + i, all_tbl_find(tbls[i].tname, tbls[i].tbl, tbls[i].id), fld, pptr)) != NULL)
					return(tbl);
				continue;
			}
			hold = *pptr;
			if (tbls[i].ptr) {
				int nptr;

				nptr = tbls[i].ptr;
				/* if you hit a NULL, stop the loop */
				do {
					*pptr = *((char **) *pptr);
				} while (*pptr && --nptr);
			}
			if (*pptr) {
				if (!tbls[i].ptr)
					*pptr += tbls[i].offset;
				if ((tbl = asl_find(tbls + i, all_tbl_find(tbls[i].tname, tbls[i].tbl, tbls[i].id), fld, pptr)) != NULL)
					return(tbl);
				*pptr = hold;
			}
		}
	}
	return(NULL);
}

static
#ifdef _NO_PROTO
struct_init()
#else
struct_init( void )
#endif /* _NO_PROTO */
{
        char * errhdr;
        char * errmsg;

        structInited = 1;
	Hashnams = (void *) xkhash_init(50);
	if (!(Dynmem = (struct memtbl **) malloc(20 * sizeof(struct memtbl *))))
        {
                errhdr = strdup(GetSharedMsg(DT_ERROR));
                errmsg = strdup(GetSharedMsg(DT_ALLOC_FAILURE));
		printerr(errhdr, errmsg, NULL);
                free(errhdr);
                free(errmsg);
		exit(1);
	}
	Dynmem[0] = NULL;
	Sdynmem = 20;
	Ndynmem = 0;
	add_structlist(basemems, "base", BASE_ID);
	add_structlist(Dynmem, "dynamic", DYNMEM_ID);
}

