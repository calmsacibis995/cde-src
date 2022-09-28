/*
 * File:	safeprintf.C $Revision: 1.2 $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */
#if defined(__aix)
#include <sys/types.h>
#endif

#ifdef DOMAIN_ALLOW_MALLOC_OVERRIDE
#include "/usr/include/apollo/shlib.h"
#endif
#ifdef apollo
#include <malloc.h>
#endif

#include <stdio.h>
#include <stdlib.h>  

extern "C" {
    char *mktemp(char *);	/* Needed since it's not part of XPG3 */
}

#include <osfcn.h>
#include <stdarg.h>
#include <string.h>
#include "stdiox.h"

char *
mvsprintf(char *&buf, size_t &buflen, const char *fmt, va_list ap)
{
    static FILE *fp = NULL;
    if (fp == NULL)
    {
	char tnam[25];
	strcpy(tnam, "/tmp/mvsp.XXXXXX");
	mktemp(tnam);
	fp = fopen(tnam, "w+");
	if (fp == NULL)
	    return NULL;
	unlink(tnam);
    }
    if (fmt == NULL)
    {
	if (fp != NULL)
	    fclose(fp);
	fp = NULL;
	return NULL;
    }

    rewind(fp);
    vfprintf(fp, fmt, ap);
    size_t len = (size_t)ftell(fp);
    if (len >= buflen)
    {
	if (buf != NULL)
	    free(buf);
        buf = (char *) malloc(buflen = len + 1);
	if (buf == NULL)
	    return NULL;
    }
    rewind(fp);
    fread(buf, sizeof (char), len, fp);
    buf[len] = '\0';
    return buf;
}

const char *
msprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    static char *buf = NULL;
    static size_t buflen = 0;
    char *ret = mvsprintf(buf, buflen, fmt, ap);
    va_end(ap);
    return ret;	// may be NULL if mvsprintf() fails
}
