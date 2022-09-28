/*
 * File:	strsep.C $Revision: 1.5 $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */

//  strsep  --  separate a string into an array like "split" in "awk"
//
//  compile with "-DDEBUG" to add a simple "main()" to test "strsep()"
//      and use like:    strsep ":/@" yup </etc/passwd
//                or:    strsep ":" </etc/passwd
//

#include <stdio.h>
#include <string.h>
#include <codelibs/nl_hack.h>

extern "C" {
    extern char * strdup(const char *);
};

#include "stringx.h"
#define __PRIVATE_
#include <codelibs/privbuf.h>
#include <codelibs/mbstring.h>

#define issep(c) (_mb_schr(sep, (c)) != NULL)
#define notsep(c) (_mb_schr(sep, (c)) == NULL)

extern void *_strsep_privbuf;  /* in str_shlib.C */
extern void *_strsepb_privbuf; /* in str_shlib.C */

char **
strsepb(const char *str, const char *sep, boolean whsp, int *retnum, void **v)
{
    if (v == NULL)
	v = &_strsepb_privbuf;
    privbuf_buffer *b = (privbuf_buffer *)*v;
    if (b == NULL)
	*v = b = privbuf_allocprivbuf();
    privbuf_charbuf &buf = b->buf;
    privbuf_strvec &vec = b->vec;

    buf.reset(strlen((char *)str) + 1);
    register char *s = buf.getarr();
    strcpy(s, str);

    register int num = 0;
    while (*s != '\0')
    {
	if (whsp)		// skip leading separators?
	    while (*s != '\0' && issep(CHARAT(s)))
		ADVANCE(s);

	vec[num++] = s;

	// find next separator
	while (*s != '\0' && notsep(CHARAT(s)))
	    ADVANCE(s);

	if (*s != '\0')
	{
	    char *t = s;
	    ADVANCE(s);
	    *t = '\0';
	    // check for empty last column:
	    if (*s == '\0' && !whsp)
		vec[num++] = s;
	}
    }

    vec[num] = NULL;

    if (retnum != NULL)
	*retnum = num;

    if (v == &_strsepb_privbuf)
#ifdef _AIX
	return strvdup((const char **) vec.getarr());
#else
	return strvdup(vec.getarr());
#endif /* _AIX */
    else
	return vec.getarr();
}


extern void *_strcmbn_privbuf;  /* in str_shlib.C */
extern void *_strcmbnb_privbuf; /* in str_shlib.C */

char *strcmbnb(const char **vec, const char *sep, void **v)
{
// Just in case vec is NULL
    if (vec == NULL)
	return NULL;

    if (v == NULL)
	v = &_strcmbnb_privbuf;
    privbuf_buffer *b = (privbuf_buffer*)*v;
    if (b == NULL)
	*v = b = privbuf_allocprivbuf();
    privbuf_charbuf &buf = b->buf;

    buf.reset();
    for (const char *s = *vec++; s != NULL; s = *vec++)
    {
	while (*s != '\0')
	    buf[buf.size()] = *s++;
	if (*vec != NULL)
	    for (s = sep; *s != '\0';)
		buf[buf.size()] = *s++;
    }
    buf[buf.size()] = '\0';

    if (v == &_strcmbnb_privbuf)
	return strdup(buf.getarr());
    else
	return buf.getarr();
}
