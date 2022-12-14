/* $XConsortium: chgrp.c /main/cde1_maint/3 1995/10/14 01:33:01 montyb $ */
/***************************************************************
*                                                              *
*                      AT&T - PROPRIETARY                      *
*                                                              *
*         THIS IS PROPRIETARY SOURCE CODE LICENSED BY          *
*                          AT&T CORP.                          *
*                                                              *
*                Copyright (c) 1995 AT&T Corp.                 *
*                     All Rights Reserved                      *
*                                                              *
*           This software is licensed by AT&T Corp.            *
*       under the terms and conditions of the license in       *
*       http://www.research.att.com/orgs/ssr/book/reuse        *
*                                                              *
*               This software was created by the               *
*           Software Engineering Research Department           *
*                    AT&T Bell Laboratories                    *
*                                                              *
*               For further information contact                *
*                     gsf@research.att.com                     *
*                                                              *
***************************************************************/

/* : : generated by proto : : */

#if !defined(__PROTO__)
#if defined(__STDC__) || defined(__cplusplus) || defined(_proto) || defined(c_plusplus)
#if defined(__cplusplus)
#define __MANGLE__	"C"
#else
#define __MANGLE__
#endif
#define __STDARG__
#define __PROTO__(x)	x
#define __OTORP__(x)
#define __PARAM__(n,o)	n
#if !defined(__STDC__) && !defined(__cplusplus)
#if !defined(c_plusplus)
#define const
#endif
#define signed
#define void		int
#define volatile
#define __V_		char
#else
#define __V_		void
#endif
#else
#define __PROTO__(x)	()
#define __OTORP__(x)	x
#define __PARAM__(n,o)	o
#define __MANGLE__
#define __V_		char
#define const
#define signed
#define void		int
#define volatile
#endif
#if defined(__cplusplus) || defined(c_plusplus)
#define __VARARG__	...
#else
#define __VARARG__
#endif
#if defined(__STDARG__)
#define __VA_START__(p,a)	va_start(p,a)
#else
#define __VA_START__(p,a)	va_start(p)
#endif
#endif
static const char id[] = "\n@(#)chgrp (AT&T Bell Laboratories) 05/09/95\0\n";

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide lchown
#else
#define lchown		______lchown
#endif

#include <cmdlib.h>
#include <hash.h>
#include <ls.h>
#include <ctype.h>
#include <ftwalk.h>

#include "FEATURE/symlink"

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide lchown
#else
#undef	lchown
#endif

typedef struct				/* uid/gid map			*/
{
	HASH_HEADER;			/* hash bucket header		*/
	int	uid;			/* id maps to this uid		*/
	int	gid;			/* id maps to this gid		*/
} Map_t;

#define NOID		(-1)

#define F_CHOWN		(1<<0)		/* chown			*/
#define F_DONT		(1<<1)		/* show but don't do		*/
#define F_FORCE		(1<<2)		/* ignore errors		*/
#define F_GID		(1<<3)		/* have gid			*/
#define F_RECURSE	(1<<4)		/* ftw				*/
#define F_UID		(1<<5)		/* have uid			*/

extern __MANGLE__ int	lchown __PROTO__((const char*, uid_t, gid_t));

/*
 * parse uid and gid from s
 */

static void
getids __PARAM__((register char* s, char** e, int* uid, int* gid, int flags), (s, e, uid, gid, flags)) __OTORP__(register char* s; char** e; int* uid; int* gid; int flags;){
	register char*	t;
	register int	n;
	char*		z;
	char		buf[64];

	*uid = *gid = NOID;
	while (isspace(*s)) s++;
	for (t = s; (n = *t) && n != ':' && n != '.' && !isspace(n); t++);
	if (n)
	{
		flags |= F_CHOWN;
		if ((n = t++ - s) >= sizeof(buf))
			n = sizeof(buf) - 1;
		*((s = (char*)memcpy(buf, s, n)) + n) = 0;
		while (isspace(*t)) t++;
	}
	if (flags & F_CHOWN)
	{
		if (*s)
		{
			if ((n = struid(s)) == NOID)
			{
				n = (int)strtol(s, &z, 0);
				if (*z) error(ERROR_exit(1), "%s: unknown user", s);
			}
			*uid = n;
		}
		for (s = t; (n = *t) && !isspace(n); t++);
		if (n)
		{
			if ((n = t++ - s) >= sizeof(buf))
				n = sizeof(buf) - 1;
			*((s = (char*)memcpy(buf, s, n)) + n) = 0;
		}
	}
	if (*s)
	{
		if ((n = strgid(s)) == NOID)
		{
			n = (int)strtol(s, &z, 0);
			if (*z) error(ERROR_exit(1), "%s: unknown group", s);
		}
		*gid = n;
	}
	if (e) *e = t;
}

int
b_chgrp __PARAM__((int argc, char *argv[]), (argc, argv)) __OTORP__(int argc; char *argv[];){
	register int	flags = 0;
	register char*	file;
	register char*	s;
	register Map_t*	m;
	Hash_table_t*	map = 0;
	int		resolve;
	int		n;
	int		uid;
	int		gid;
	struct stat	st;
	char*		op;
	int		(*chownf) __PROTO__((const char*, uid_t, gid_t));
	int		(*statf) __PROTO__((const char*, struct stat*));

	NoP(id[0]);
	cmdinit(argv);
	resolve = ftwflags();
	if (error_info.id[2] == 'g')
		s = "fmnHLPR [owner:]group file...";
	else
	{
		flags |= F_CHOWN;
		s = "fmnHLPR owner[:group] file...";
	}
	while (n = optget(argv, s)) switch (n)
	{
	case 'f':
		flags |= F_FORCE;
		break;
	case 'm':
		if (!(map = hashalloc(NiL, HASH_set, HASH_ALLOCATE, HASH_namesize, sizeof(int), HASH_name, "ids", 0)))
			error(ERROR_exit(1), "out of space space [id map]");
		break;
	case 'n':
		flags |= F_DONT;
		break;
	case 'H':
		resolve |= FTW_META|FTW_PHYSICAL;
		break;
	case 'L':
		resolve &= ~(FTW_META|FTW_PHYSICAL);
		break;
	case 'P':
		resolve &= ~FTW_META;
		resolve |= FTW_PHYSICAL;
		break;
	case 'R':
		flags |= F_RECURSE;
		break;
	case ':':
		error(2, opt_info.arg);
		break;
	case '?':
		error(ERROR_usage(2), opt_info.arg);
		break;
	}
	argv += opt_info.index;
	argc -= opt_info.index;
	if (error_info.errors || argc < 2)
		error(ERROR_usage(2), optusage(NiL));
	s = *argv;
	if (map)
	{
		char*	t;
		Sfio_t*	sp;
		int	nuid;
		int	ngid;

		if (streq(s, "-"))
			sp = sfstdin;
		else if (!(sp = sfopen(NiL, s, "r")))
			error(ERROR_exit(1), "%s: cannot read", s);
		while (s = sfgetr(sp, '\n', 1))
		{
			getids(s, &t, &uid, &gid, flags);
			getids(t, NiL, &nuid, &ngid, flags);
			if (uid != NOID)
			{
				if (m = (Map_t*)hashlook(map, (char*)&uid, HASH_LOOKUP, NiL))
				{
					m->uid = nuid;
					if (m->gid == NOID)
						m->gid = ngid;
				}
				else if (m = (Map_t*)hashlook(map, NiL, HASH_CREATE|HASH_SIZE(sizeof(Map_t)), NiL))
				{
					m->uid = nuid;
					m->gid = ngid;
				}
				else error(ERROR_exit(1), "out of space space [id hash]");
			}
			if (gid != NOID)
			{
				if (gid == uid || (m = (Map_t*)hashlook(map, (char*)&gid, HASH_LOOKUP, NiL)))
					m->gid = ngid;
				else if (m = (Map_t*)hashlook(map, NiL, HASH_CREATE|HASH_SIZE(sizeof(Map_t)), NiL))
				{
					m->uid = NOID;
					m->gid = ngid;
				}
				else error(ERROR_exit(1), "out of space space [id hash]");
			}
		}
		if (sp != sfstdin)
			sfclose(sp);
	}
	else
	{
		getids(s, NiL, &uid, &gid, flags);
		if (uid != NOID)
			flags |= F_UID;
		if (gid != NOID)
			flags |= F_GID;
	}
	if (flags & F_RECURSE)
	{
		char*	pfxv[5];

		n = 0;
		if (flags & F_FORCE) pfxv[n++] = "-f";
		if (map) pfxv[n++] = "-m";
		if (flags & F_DONT) pfxv[n++] = "-n";
		if (resolve & FTW_META) pfxv[n++] = "-H";
		else if (resolve & FTW_PHYSICAL) pfxv[n++] = "-P";
		pfxv[n] = 0;
		return(cmdrecurse(argc, argv, n, pfxv));
	}
	statf = (resolve & FTW_PHYSICAL) ? lstat : stat;
#if _lib_lchown
	if (resolve & FTW_PHYSICAL)
	{
		chownf = lchown;
		op = "lchown";
	}
	else
#endif
	{
		chownf = chown;
		op = "chown";
	}
	while (file = *++argv)
	{
		if (!(*statf)(file, &st))
		{
			if (map)
			{
				flags &= ~(F_UID|F_GID);
				uid = st.st_uid;
				gid = st.st_gid;
				if ((m = (Map_t*)hashlook(map, (char*)&uid, HASH_LOOKUP, NiL)) && m->uid != NOID)
				{
					uid = m->uid;
					flags |= F_UID;
				}
				if (gid != uid)
					m = (Map_t*)hashlook(map, (char*)&gid, HASH_LOOKUP, NiL);
				if (m && m->gid != NOID)
				{
					gid = m->gid;
					flags |= F_GID;
				}
				if (!(flags & (F_UID|F_GID)))
					continue;
			}
			else
			{
				if (!(flags & F_UID))
					uid = st.st_uid;
				if (!(flags & F_GID))
					gid = st.st_gid;
			}
			if (flags & F_DONT)
				sfprintf(sfstdout, "%s uid:%05d->%05d gid:%05d->%05d %s\n", op, st.st_uid, uid, st.st_gid, gid, file);
			else if ((*chownf)(file, uid, gid) && !(flags & F_FORCE))
			{
				switch (flags & (F_UID|F_GID))
				{
				case F_UID:
					s = ERROR_translate(" owner", 0);
					break;
				case F_GID:
					s = ERROR_translate(" group", 0);
					break;
				case F_UID|F_GID:
					s = ERROR_translate(" owner and group", 0);
					break;
				}
				error(ERROR_system(0), "%s: cannot change%s", file, s);
				error_info.errors++;
			}
		}
		else if (!(flags & F_FORCE))
		{
			error(ERROR_system(0), "%s: not found", file);
			error_info.errors++;
		}
	}
	return(error_info.errors != 0);
}
