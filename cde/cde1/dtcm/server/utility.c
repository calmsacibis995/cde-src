/* $XConsortium: utility.c /main/cde1_maint/2 1995/10/03 12:32:12 barstow $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)utility.c	1.11 96/11/11 Sun Microsystems, Inc."

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <syslog.h>
#include <unistd.h>
#if defined(SunOS) || defined(USL) || defined(__uxp__)
#include <netdb.h>
#include <sys/systeminfo.h>
#endif
#include "utility.h"
#include "lutil.h"
#include "access.h"

const char *ETC_DT_CMSD_CONF=	"/etc/dt/config/cmsd.conf";
const char *USR_DT_CMSD_CONF=	"/usr/dt/config/cmsd.conf";
const char *TRUSTED_CONFIG=	"trusted";
const char *UNTRUSTED_CONFIG=	"untrusted";

extern int debug;
extern char *pgname;

/* 0 if invoked by inetd, 1 otherwise, initialized in main() */
int	standalone = 1;

extern void
_DtCmsComposeReferenceId(long id, char *calname, char buf[])
{
	char	*ptr1, *ptr2;
	char	*domain;

	ptr1 = strchr(calname, '@');
	ptr2 = (ptr1 ? strchr(ptr1, '.') : NULL);
	if (ptr1) {
		if (ptr2 == NULL)
			domain = NULL;

		sprintf(buf, "%d:%s%s%s", id, calname,
			(ptr2 ? "" : (domain ? domain : "")),
			(ptr2 ? "" : (domain ? domain : "")));
	} else {
		sprintf(buf, "%d:%s@%s", id, calname, _DtCmGetHostAtDomain()); 
	}
}

extern int
_DtCmsGetConfigInfo()
{
	FILE	*fs;
	char	line[BUFSIZ], *ptr, *key, *value;
	int	i = 0, res = 0;
	int	saveuid;

#ifndef AIX
#ifdef HPUX
	saveuid = getuid();
	setuid(0);
#else
	saveuid = geteuid();
	seteuid(0);
#endif
#endif

	/* try /etc/dt/config/cmsd.conf first */
	if ((fs = fopen(ETC_DT_CMSD_CONF, "r")) == NULL) {
		if (errno != ENOENT) {
			_DtCmsSyslog("Failed to open %s\n", ETC_DT_CMSD_CONF);
			res = -1;
			goto done;
		} else {
			if (debug)
				_DtCmsSyslog("%s does not exist\n",
						ETC_DT_CMSD_CONF);
		}
	}

	/* try /usr/dt/config/cmsd.conf */
	if (fs == NULL) {
		if ((fs = fopen(USR_DT_CMSD_CONF, "r")) == NULL) {
			if (errno != ENOENT) {
				_DtCmsSyslog("Failed to open %s\n",
						USR_DT_CMSD_CONF);
				res = -1;
				goto done;
			} else {
				if (debug)
					_DtCmsSyslog("%s does not exist\n",
						USR_DT_CMSD_CONF);

				/* reset all options */
				_DtCmsUnsetAccessControlList();
				goto done;
			}
		}
	}

	/* reset all options */
	_DtCmsUnsetAccessControlList();

	while (ptr = fgets(line, BUFSIZ, fs)) {
		if (debug) {
			fprintf(stderr, "line %d:'%s'\n", ++i, line);
		}

		/* may want to allow entries not starting at column 1 */
		if (isspace(*line) || *line == '#')
			continue;

		key = _DtCmsGetNextWord(&ptr);

		if (strcmp(key, TRUSTED_CONFIG) == 0) {
			if (_DtCmsSetAccessControlList(ptr, _B_TRUE)
			    != CSA_SUCCESS) {
				_DtCmsSyslog("failed to set trusted list\n");
				res = -1;
				break;
			}
		} else if (strcmp(key, UNTRUSTED_CONFIG) == 0) {
			if (_DtCmsSetAccessControlList(ptr, _B_FALSE)
			    != CSA_SUCCESS) {
				_DtCmsSyslog("failed to set untrusted list\n");
				res = -1;
				break;
			}
		} else {
			if (debug)
				_DtCmsSyslog("%s: unknown keyword in /etc/cmsd.conf\n",
					key);
		}
	}

	fclose(fs);
done:

#ifndef AIX
#ifdef HPUX
	setuid(saveuid);
#else
	seteuid(saveuid);
#endif
#endif
	return (res);
}


/*
 * parse the given character string pointed to by the string placeholder
 * for the next word.
 * pointer to the word in the line is returned
 * the line will be modified (the word null terminated) and the
 * string placeholder updated 
 */
extern char *
_DtCmsGetNextWord(char **line)
{
	char *ptr = *line, *head;

	while (isspace(*ptr)) /* skip space */
		ptr++;

	if (*ptr == NULL) {
		*line = NULL;
		return (NULL);
	}

	head = ptr;
	while (*ptr && !isspace(*ptr)) /* skip to next space or eol */
		ptr++;

	if (*ptr != NULL)
		*ptr++ = NULL;
	*line = ptr;

	return (head);
}

/*
 * if we are invoked by inetd, use syslog to print messsage
 * otherwise, print it out to stderr
 */
extern void
_DtCmsSyslog(const char *format, ...)
{
	va_list	args;

	va_start(args, format);

	if (standalone) {
		fprintf(stderr, "%s: ", pgname);
		vfprintf(stderr, format, args);
	} else {
		static	int	inited = _B_FALSE;

		if (inited == _B_FALSE) {
			openlog(pgname, LOG_CONS, LOG_DAEMON);
			inited = _B_TRUE;
		}
		vsyslog(LOG_ERR, format, args);
	}
}

