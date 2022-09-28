/*
 * File:	pathcollapse.c $XConsortium: pathcollapse.c /main/cde1_maint/1 1995/07/14 20:07:09 drk $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */
#ifdef DOMAIN_ALLOW_MALLOC_OVERRIDE
#include "/usr/include/apollo/shlib.h"
#endif

#include <stdio.h>
#if defined(apollo) || defined(__aix)
#include <stdlib.h>
#else
#include <malloc.h>
#endif /* apollo */
#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include <codelibs/nl_hack.h>
#include "pathutils.h"

#define SEP(P) (CHARAT(P) == '\0' || CHARAT(P) == '/')
#define DOT(P) (CHARAT(P) == '.' && SEP((P) + 1))
#define DOTDOT(P) (CHARAT(P) == '.' && DOT((P) + 1))

char *
pathcollapse(const char *src, char *dst, boolean show_dir)
{
    int ch;
    const char *srcp;
    char *dstp,  *sep;
    char *components[MAXPATHLEN / 2 + 1];
    char **comp = components;
    int length = src == NULL ? 0 : strlen(src);
    int dir_comp;		/* TRUE if last component was . or .. */
#ifdef apollo
    int double_slash = 0;
#endif

    if (length == 0 || length > MAXPATHLEN)
    {
	errno = EINVAL;
	return NULL;
    }

    if (dst == NULL)
	if ((dst = (char *)malloc(length + 1)) == NULL)
	    return NULL;

#ifdef apollo
    /*
     * On apollo, a leading double-slash must be preserved, so we
     * copy the first slash and hide it from the rest of the code.
     */
    if (CHARAT(src) == '/' && CHARAT(src + 1) == '/')
    {
	*dst++ = '/';
	src++;
	double_slash = 1;
    }
#endif

    srcp = src;
    dstp = dst;

    do				/* for each component of src */
    {
	*comp = dstp;

	/* copy the component and trailing separator to dst */
	do
	{
	    ch = (int)CHARAT(srcp);
	    sep = dstp;
	    WCHARADV(ch, dstp);
	    if (ch == '\0')
		break;
	    ADVANCE(srcp);
	} while (ch != '/');

	dir_comp = ch == '/';	/* true if trailing '/' */

	/* skip all adjacent '/' characters [the first is preserved] */
	while (CHARAT(srcp) == '/')
	    ADVANCE(srcp);

	/* remove redundant trailing slash */
	if (CHARAT(srcp) == '\0')
	    if (sep > dst)
		*sep = ch = '\0';

	if (DOT(*comp))
	{
	    dir_comp = 1;
	    dstp = *comp;
	    if (dstp > dst)
	    {
		sep = dstp - 1;
		*dstp = '\0';
	    }
	    continue;
	}
	else if (DOTDOT(*comp))
	{
	    dir_comp = 1;
	    if (*comp > dst)
	    {
	    comp--;
	    if (!DOTDOT(*comp))
	    {
		dstp = *comp;
		if (dstp > dst)
		{
		    sep = dstp - 1;
		    *dstp = '\0';
		}
		else
		{
		    if (CHARAT(dst) == '/')
		    {
			/* /.. is same as / */
			dstp = dst + 1;
			*dstp = '\0';
			comp++;
		    }
		    else
			dst[0] = '.';
		    sep = dst + 1;
		    *sep = '\0';
		}
		continue;
	    }
	    }
	}

	comp++;
    } while (ch != '\0');

    if (show_dir)
    {
	if (dir_comp && (sep > dst + 1 || dst[0] != '/'))
	{
	    *sep++ = '/';
	    *sep = '\0';
	}
    }
    else if (sep > dst)
	*sep = '\0';		/* remove trailing '/' */

#ifdef apollo
    if (double_slash)
	dst--;
#endif

    return dst;
}
