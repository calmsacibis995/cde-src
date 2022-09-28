/*
 * File:	xgets.C $Revision: 1.2 $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */

#ifdef DOMAIN_ALLOW_MALLOC_OVERRIDE
#include "/usr/include/apollo/shlib.h"
#endif
#ifdef apollo
#include <malloc.h>
#endif

// "Xgets()" acts like the "gets/fgets(3)" function except that this
// works with dynamically allocated strings and the "stringx" package.
// Macros are defined in <stdiox.h> to make calls to this function
// much less painful.  Note that the string returned may NOT
// be the one that was returned from a previous call.
//
// by  Parag Patel

#include <stdio.h>
#if defined(apollo) || defined(__aix)
#include <stdlib.h>
#else
#include <malloc.h>
#endif /* apollo */
#include <limits.h>
#include "stdiox.h"

// MINCHUNKSIZE must be at least 1
#define MINCHUNKSIZE 16

#ifndef FOPEN_MAX
#define FOPEN_MAX	60
#endif

static struct filebuf_t
{
	char *str;
	int len;
} filebuf[FOPEN_MAX] = { 0 };

char *
xgets(FILE *file, int max, boolean keepnl)
{
	if (feof(file))
		return NULL;

	if (max == -1)
		max = INT_MAX;

	// Get the subscript into the stdio _iob[] array
	int filenum = file - stdin;
	//assert(filenum >= 0 && filenum < FOPEN_MAX);

	// buf is the corresponding xgets() buffer for that file pointer
	filebuf_t *buf = &filebuf[filenum];
	if (buf->str == NULL)
	{
                buf->str = (char *) malloc(MINCHUNKSIZE);
		if (buf->str == NULL)
			return NULL;
		buf->len = MINCHUNKSIZE;
	}

	register char *ptr = buf->str;	// clear the string
	register char *end = &ptr[buf->len];	// and mark the realloc pos

	while (max-- > 0)
	{
		register int ch;

		if ((ch = getc(file)) == EOF)
			if (ptr == buf->str)
				return NULL;
			else
			{
				// Missing newline at end of file
				*ptr = '\0';
				return buf->str;
			}

		// add the character to our buffer
		*ptr++ = ch;

		// filled our buffer - realloc it with a bigger size
		if (ptr >= end)
		{
			int pos = ptr - buf->str;

                        buf->str = (char *) realloc(buf->str, buf->len *= 2);
			if (buf->str == NULL)
				return NULL;

			ptr = &buf->str[pos];
			end = &buf->str[buf->len];
		}

		// quit if the character was a newline
		if (ch == '\n')
			break;
	}

	// remove the newline if appropriate
	if (ptr[-1] == '\n' && !keepnl)
		ptr--;

	// null-terminate the buffer and return it
	*ptr = '\0';
	return buf->str;
}
