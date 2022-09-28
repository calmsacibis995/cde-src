/*
 * File:	strchg.C $Revision: 1.2 $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */

#include <stdio.h>
#include <string.h>
#include "stringx.h"


char *
strchg(char *s1, const char *s2, register char *ptr, register long n)
{
	char *newstr;		/* start of output string */
	char *tail;			/* pointer to tail of s1 to copy */
	char *end;			/* pointer to '\0' in s1 */
	const char *p;		/* pointer to next character to copy */
	register char *newptr;  /* current position in output string */

	/* error if s1 is NULL or ptr doesn't point into s1 */
	if (s1 == NULL)
		return NULL;
	else
	{
		end = &s1[strlen(s1)];
		if (ptr < s1 || ptr > end)
			return NULL;
	}

	/* Set tail to point to the tail of s1 to copy.    */
	/* n < 0 is the same as n == 0.  If n is too large */
	/* for s1, tail = end of s1.                       */
	if (n <= 0)
		tail = ptr;
	else
	{
		tail = ptr + n;
		if (tail > end)
			tail = end;
	}

	/* Allocate space for the result.  */
	newstr = strnew((ptr - s1) + strlen((char *)s2) + (end - tail));

	/* Copy the initial substring of s1 to new.     */
	p = s1;
	newptr = newstr;
	while (p < ptr)
		*newptr++ = *p++;

	/* Then add a copy of s2 after that.             */
	/* If s2 is NULL, ignore it (equivalent to "").  */
	if (s2 != NULL)
	{
		p = s2;
		while ((*newptr++ = *p++) != '\0')
			;
		newptr--;		/* back up over the trailing '\0' */
	}

	/* Finally, copy the trailing substring of s1 after that:  */
	p = tail;
	while ((*newptr++ = *p++) != '\0')
		;

	strfree(s1);	/* now free the old version of s1 */

	return newstr;	/* and return the new version */
}
