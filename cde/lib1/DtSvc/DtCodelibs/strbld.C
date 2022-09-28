/*
 * File:	strbld.C $Revision: 1.2 $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <codelibs/stdiox.h>
#include "stringx.h"


#ifndef BUFFSIZE
#define BUFFSIZE 1024
#endif
#ifndef BUFFBUMP
#define BUFFBUMP 256
#endif


// Catenate a bunch of string arguments into one string.
// Overwrites a static buffer on each call.
// Grows that buffer if necessary.

static char *
strbldv(char *&buf, size_t &buflen, const char *arg, va_list ap)
{
    if (buf == NULL)
        buf = (char *) malloc(buflen = BUFFSIZE);
    if (buf == NULL)
	return NULL;

    register int length = 0;
    register int pos = 0;

    buf[0] = '\0';

    while (arg != NULL)
    {
	register int arglen = strlen((char *)arg);

	length += arglen;
	if (length + 1 >= buflen)
	{
            buf = (char *) realloc(buf, buflen = length + BUFFBUMP);
	    if (buf == NULL)
		return NULL;
	}
	strcpy(buf + pos, arg);
	pos += arglen;
	arg = va_arg(ap, char*);
    }

    return buf;
}

static char *buf = NULL;
static size_t len = 0;

const char *
strblds(const char *arg, ...)
{
    va_list ap;
    va_start(ap, arg);
    char *ret = strbldv(buf, len, arg, ap);
    va_end(ap);
    return ret;
}

#define __sTr const char*

const char *
strbld(__sTr a1,
	__sTr, __sTr, __sTr, __sTr, __sTr,
	__sTr, __sTr, __sTr, __sTr, __sTr,
	__sTr, __sTr, __sTr, __sTr, __sTr,
	__sTr, __sTr, __sTr, __sTr, __sTr, ...)
{
    va_list ap;
    va_start(ap, a1);
    char *ret = strbldv(buf, len, a1, ap);
    va_end(ap);
    return ret;
}

// Acts just like msprintf(), which does the same thing.
// We need our own buffer for backward-compatibility though.
//
const char *
strbldf(const char *format, ...)
{
    static char *buf = NULL;
    static size_t len = 0;
    va_list ap;
    va_start(ap, format);
    char *ret = mvsprintf(buf, len, format, ap);
    va_end(ap);
    return ret;
}
