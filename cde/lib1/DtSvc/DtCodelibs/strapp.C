/*
 * File:	strapp.C $Revision: 1.2 $
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
strapp(char *s1, const char *s2)
/*	Parameters:
 *
 *		this routine will append s2 to s1.  However,
 *		space will be allocated so that there is enough
 *		room in s1 for both.
 *
 *		to allocate new space, set s1 to NULL
 *		to de-allocate the space used, strfree(s1)
 *
 *		This returns the address of the resulting string.
 *
 */
{
	register int length = strlen((char *)s2);
	register char *ptr;

	if (s1 != NULL)
	{
		length += strlen(s1);
		ptr = strnew(length);
		strcpy(ptr, s1);
		strcat(ptr, s2);
		strfree(s1);
	}
	else
	{
		ptr = strnew(length);
		strcpy(ptr, s2);
	}

	return ptr;
}



char *
strnapp(char *s1, const char *s2, size_t n)
/*	Parameters:
 * 		s1			first string
 * 		s2			second string
 * 		n			max # of chars to copy
 *
 *		this routine will append s2 to s1.  However,
 *		space will be allocated so that there is enough
 *		room in s1 for both.  A maximum of n non-null
 *		characters will be copied.
 *
 *		to allocate new space, set s1 to NULL
 *		to de-allocate the space used, strfree(s1)
 *
 *		This returns the address of the resulting string.
 *
 */
{
	register int length = strlen((char *)s2);
	if (length > n)
		length = n;
	length = (s1 == NULL ? 0 : strlen(s1)) + length;

	register char *ptr = strnew(length);
	*ptr = '\0';

	if (s1 != NULL)
	{
		strcpy(ptr, s1);
		strfree(s1);
	}
	strncat(ptr, s2, n);

	return ptr;
}
