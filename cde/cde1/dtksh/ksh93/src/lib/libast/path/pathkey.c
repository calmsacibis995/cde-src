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
#include <ast.h>
#include <ctype.h>
#include <hash.h>
#include <fs3d.h>
#include <preroot.h>

char*
pathkey __PARAM__((char* key, char* attr, const char* lang, const char* apath), (key, attr, lang, apath)) __OTORP__(char* key; char* attr; const char* lang; const char* apath;){
	register char*		path = (char*)apath;
	register char*		s;
	register char*		k;
	char*			t;
	char*			flags;
	char**			p;
	int			c;
	unsigned long		n;
	char			buf[15];
	char*			usr[16];
	char*			env[elementsof(usr) + 3];
	char*			ver[2];
	char			tmp[PATH_MAX];

	for (c = 0; c < elementsof(env); c++) env[c] = 0;
	if (!key) key = buf;
	n = 0;

	/*
	 * trailing flags in path
	 */

	if (flags = strchr(path, ' '))
	{
		if (flags == path) flags = 0;
		else
		{
			strcpy(tmp, path);
			*(flags = tmp + (flags - path)) = 0;
			path = tmp;
		}
	}

	/*
	 * 3D
	 */

	if (fs3d(FS3D_TEST) && (c = mount(path, tmp, FS3D_GET|FS3D_ALL|FS3D_SIZE(PATH_MAX), NiL)) > 1 && c < PATH_MAX)
		path = tmp;

	/*
	 * preroot
	 */

	if (attr) attr = strcopy(attr, "PREROOT='");
#if FS_PREROOT
	if (k = getenv(PR_BASE))
	{
		if (s = strrchr(k, '/')) k = s + 1;
		n = memsum(k, strlen(k), n);
	}
	if (attr && (getpreroot(attr, path) || getpreroot(attr, NiL))) attr += strlen(attr);
#else
	if ((k = getenv("VIRTUAL_ROOT")) && *k == '/')
	{
		n = memsum(k, strlen(k), n);
		if (attr) attr = strcopy(attr, k);
	}
#endif

	/*
	 * universe
	 */

	if (attr) attr = strcopy(attr, "' UNIVERSE='");
	if (k = astconf("UNIVERSE", NiL, NiL))
	{
		n = memsum(k, strlen(k), n);
		if (attr) attr = strcopy(attr, k);
	}

	/*
	 * environment
	 *
	 *	${VERSION_ENVIRONMENT}	: list of alternate env vars
	 *	${VERSION_<lang>}
	 *	${VERSION_<base(path)>}
	 *	${<toupper(base(path))>VER}
	 *	${OBJTYPE}
	 */

	if (attr) *attr++ = '\'';
	c = 0;
	usr[c++] = "OBJTYPE";
	k = getenv("VERSION_ENVIRONMENT");
	while (c < elementsof(usr) && (usr[c++] = k))
	{
		while (*k && *k++ != ':');
		if (!*k) k = 0;
	}
	ver[0] = (char*)lang;
	ver[1] = k = (s = strrchr(path, '/')) ? s + 1 : path;
	s = buf;
	if (isdigit(*k))
	{
		if (*k == '3' && *(k + 1) == 'b')
		{
			/*
			 * cuteness never pays
			 */

			k += 2;
			*s++ = 'B';
			*s++ = 'B';
			*s++ = 'B';
		}
		else *s++ = 'U';
	}
	for (; (c = *k) && s < &buf[sizeof(buf) - 1]; k++)
	{
		if (!isalnum(c)) c = '_';
		else if (islower(c)) c = toupper(c);
		*s++ = c;
	}
	*s = 0;
	for (p = environ; *p; p++)
	{
		s = "VERSION_";
		for (k = *p; *k && *k == *s; k++, s++);
		if (*k && !*s)
		{
			for (c = 0; c < elementsof(ver); c++)
				if (!env[c] && (s = ver[c]))
				{
					for (t = k; *t && *t != '=' && *t++ == *s; s++);
					if (*t == '=' && (!*s || (s - ver[c]) > 1))
					{
						env[c] = *p;
						goto found;
					}
				}
		}
		if (!env[2])
		{
			s = buf;
			for (k = *p; *k && *s++ == *k; k++);
			if ((s - buf) > 2 && k[0] == 'V' && k[1] == 'E' && k[2] == 'R' && k[3] == '=')
			{
				env[2] = *p;
				goto found;
			}
		}
		for (c = 0; c < elementsof(usr) && (s = usr[c]); c++)
			if (!env[c + elementsof(env) - elementsof(usr)])
			{
				for (k = *p; *k && *k == *s; k++, s++);
				if (*k == '=' && (!*s || *s == ':'))
				{
					env[c + elementsof(env) - elementsof(usr)] = *p;
					goto found;
				}
			}
	found:	;
	}
	for (c = 0; c < elementsof(env); c++)
		if (k = env[c])
		{
			if (attr)
			{
				*attr++ = ' ';
				while ((*attr++ = *k++) != '=');
				*attr++ = '\'';
				attr = strcopy(attr, k);
				*attr++ = '\'';
			}
			else while (*k && *k++ != '=');
			n = memsum(k, strlen(k), n);
		}
	if (attr)
	{
		attr = strcopy(attr, " ATTRIBUTES='PREROOT UNIVERSE");
		for (c = 0; c < elementsof(env); c++)
			if (k = env[c])
			{
				*attr++ = ' ';
				while ((*attr = *k++) != '=') attr++;
			}
		*attr++ = '\'';
		*attr = 0;
	}

	/*
	 * now the normal stuff
	 */

	if (flags) *flags = ' ';
	s = path + strlen(path);
	sfsprintf(key, 15, "%08lX", memsum(path, s - path, n));
	k = key + 14;
	*k = 0;
	if (!flags) t = path;
	else if ((t = s - 4) < flags) t = flags + 1;
	for (;;)
	{
		if (--s < t)
		{
			if (t == path) break;
			s = flags - 2;
			t = path;
		}
		if (*s != '/' && *s != ' ')
		{
			*--k = *s;
			if (k <= key + 8) break;
		}
	}
	while (k > key + 8) *--k = '.';
	return(key == buf ? strdup(key) : key);
}