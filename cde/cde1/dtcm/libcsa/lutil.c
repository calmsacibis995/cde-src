/* $XConsortium: lutil.c /main/cde1_maint/2 1995/10/03 12:29:48 barstow $ */

/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)lutil.c	1.13 97/04/08 Sun Microsystems, Inc."

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpc/rpc.h>
#include <unistd.h>
#if defined(SunOS) || defined(USL) || defined(__uxp__)
#include <netdb.h>
#include <sys/systeminfo.h>
#endif
#include <pwd.h>
#include <pthread.h>

extern char * strdup(const char *);

extern char *
_DtCmGetPrefix(char *str, char sep)
{
        char buf[BUFSIZ];
        char *ptr;

        if (str == NULL)
                return(NULL);

        ptr = buf;
        while (*str && *str != sep)
                *ptr++ = *str++;
        if (ptr == buf)
                return(NULL);
        else {
                *ptr = NULL;
                return(strdup(buf));
        }
}

extern char *
_DtCmGetLocalHost()
{
	/* host_lock synchronizes access to host */
	static pthread_mutex_t	host_lock;
	static char	*host = NULL;

	pthread_mutex_lock(&host_lock);

	if (host == NULL) {
		host = (char *)malloc(MAXHOSTNAMELEN+1);
#if defined(SunOS) || defined(USL) || defined(__uxp__)
		(void)sysinfo(SI_HOSTNAME, host, MAXHOSTNAMELEN);
#else
		(void)gethostname(host, MAXHOSTNAMELEN);
#endif /* SunOS || USL || __uxp__ */
	}

	pthread_mutex_unlock(&host_lock);

	return (host);
}

#define PORTMAPPER_PROG	100000

extern char *
_DtCmGetLocalDomain(char *hostname)
{
	/* domain_lock synchronizes access to domain */
	static pthread_mutex_t	domain_lock;

	static char	*domain = NULL;
	static boolean_t done = _B_FALSE;
	char		buf[BUFSIZ], *ptr;
	CLIENT		*cl;

	if (hostname == NULL) hostname = _DtCmGetLocalHost();

	pthread_mutex_lock(&domain_lock);

	if (domain == NULL && done == _B_FALSE) {
		done = _B_TRUE;

		domain = (char *)malloc(BUFSIZ);
#if defined(SunOS) || defined(USL) || defined(__uxp__)
		sysinfo(SI_SRPC_DOMAIN, domain, BUFSIZ - 1);
#else
		getdomainname(domain, BUFSIZ - 1);
#endif /* SunOS || USL || __uxp__ */

		/* check domain name */
		/* this is a hack to find out the domain name that
		 * is acceptable to the rpc interface, e.g.
		 * DGDO.Eng.Sun.COM is returned by sysinfo but
		 * this name is not acceptable to the rpc interface
		 * hence we need to stripe out the first component.
		 * sysinfo returns the yp domain name. if that looks
		 * completely different from the dns domainname
		 * the following loop will fail and NULL will be
		 * returned
		 */
		ptr = domain;
		while (1) {
			sprintf(buf, "%s.%s", hostname, ptr);
			if ((cl = clnt_create(buf, PORTMAPPER_PROG, 1, "udp"))
			    == NULL) {
				ptr = strchr(ptr, '.');
				if (ptr)
					ptr++;
				else
					break;
			} else {
				clnt_destroy(cl);
				break;
			}
		}

		if (ptr == NULL) {
			free(domain);
			domain = NULL;
		} else if (ptr != domain)
			domain = ptr;
	}

	pthread_mutex_unlock(&domain_lock);

	return (domain);
}

extern char *
_DtCmGetHostAtDomain()
{
	/* hostname_lock synchronizes access to hostname */
	static pthread_mutex_t	hostname_lock;

	static char	*hostname = NULL;
	char		*host, *domain = NULL;

	pthread_mutex_lock(&hostname_lock);

	if (hostname == NULL) {
		host = _DtCmGetLocalHost();
		if (strchr(host, '.') == NULL)
			domain = NULL;

		hostname = malloc(BUFSIZ);

		sprintf(hostname, "%s%s%s", host, (domain ? "." : ""),
			(domain ? domain : ""));
	}

	pthread_mutex_unlock(&hostname_lock);

	return (hostname);
}

extern char *
_DtCmGetUserName()
{
	/* name_lock synchronizes access to name */
	static pthread_mutex_t	name_lock;

        static char	*name = NULL;

#ifdef SunOS
	struct passwd	*pw, pwstruct;
	char		buf[BUFSIZ];
#else
	struct passwd	*pw;
#endif

	pthread_mutex_lock(&name_lock);

        if (name == NULL) {
		name = malloc(BUFSIZ);

#if defined(SunOS) && defined(_REENTRANT)
		if ((getpwuid_r(geteuid(),
				&pwstruct, buf, BUFSIZ-1, &pw)) > 0)
#else
	        if ((pw = getpwuid(geteuid())) == NULL)
#endif
			strcpy(name, "nobody");
		else
			strcpy(name, pw->pw_name);
        }

	pthread_mutex_unlock(&name_lock);

	return name;
}

/*
 * this routine checks whether the given name is a valid user name
 */
extern boolean_t
_DtCmIsUserName(char *user)
{
#ifdef SunOS
	struct passwd	*pw, pwstruct;
	char		buf[BUFSIZ];
#else
	struct passwd	*pw;
#endif

#if defined(SunOS) && defined(_REENTRANT)
	int err = getpwnam_r(user, &pwstruct, buf, BUFSIZ-1, &pw);
	if (err > 0) {
	  pw = NULL;
	}
#else
	pw = getpwnam(user);
#endif
	if (pw == NULL)
		return (_B_FALSE);
	else
		return (_B_TRUE);
}

