/*
 * File:	strcmpi.C $Revision: 1.2 $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */

/* This file has to be C code so that we use the HP-UX _toupper/_tolower
 * instead of the C++ version, which doesn't handle characters which are
 * already upper or lower case.  Also, the HP-UX macros are NLS-ized for
 * 8-bit characters.  Case does not make any sense for 16-bit characters.
 * Unfortunately, all this slows down the routines.  Oh well...
 */

#include <stdio.h>
#include <ctype.h>
#include "stringx.h"

// use these to keep local string buffers for upper-case copies
// - grow them if necessary
static char *str1 = NULL;
static char *str2 = NULL;
static size_t len1 = 0;
static size_t len2 = 0;


static void copy(const char *s1, const char *s2, const int len)
{
    /* if len < 0, then copy whole string (assume nul termination
     * otherwise, copy only len chars (bytes), and nul terminate
     */
    register size_t l1, l2;

    if( (l1 = strlen((char *)s1)) > len && len > 0 )
	l1 = len;
    if (l1 > len1 || str1 == NULL)
    {
	strfree(str1);
	str1 = strnew(len1 = l1);	/* allocs 1 extra byte */
    }

    if( (l2 = strlen((char *)s2)) > len && len > 0 )
	l2 = len;
    if (l2 > len2 || str2 == NULL)
    {
	strfree(str2);
	str2 = strnew(len2 = l2);	/* allocs 1 extra byte */
    }
    (void)strupper(strncpy(str1, s1, l1));
    (void)strupper(strncpy(str2, s2, l2));
    str1[l1] = '\0';
    str2[l2] = '\0';
}


int
strcmpi(const char *s, const char *t)

/*
 *	  Parameters:
 *			  Pointers to strings to compare
 *	  Operation:
 *			  Return lexicographic(s - t), but do it without
 *			  sensitivity to case.  Analogous to strcmp().
 *			  Characters with parity bit set are compared as-is.
 *			  Treats NULL pointers as null strings.
 *			  Uses table lookup case-conversion macro for efficiency.
 *			  Used to use (unsigned)UPPER for consistency with strcmp().
 *			  Now Uses (unsigned)_toupper
 *	  RETURN
 *			  0  - strings equal
 *			  <0 - t > s
 *			  >0 - t < s
 */
{
	if (s == NULL)
		s = "";

	if (t == NULL)
		t = "";

	copy(s, t, -1);
	return strcoll(str1, str2);

    /****************************
	// original code for strcmpi() - this is fast, but does not work
	// for anything except ASCII
	register char c;
	while ((c = (unsigned)_toupper((unsigned char)*s++)) ==
		(unsigned)_toupper((unsigned char)*t++))
	{
		if (c == '\0')
		{
			break;
		}
	}
	s--;
	t--;
	return (unsigned)_toupper((unsigned char)*s) -
		(unsigned)_toupper((unsigned char)*t);
    ****************************/
}


int
strncmpi(const char *s, const char *t, size_t n)

/*
 *	  Parameters:
 *			  Pointers to strings to compare and maximum
 *			  number of characters to compare.
 *	  Operation:
 *			  Return lexicographic(s - t), but do it without
 *			  sensitivity to case.  Analogous to strncmp().
 *			  Characters with parity bit set are compared as-is.
 *			  NULL pointers are treated as null strings.
 *			  Uses table lookup case-conversion macro for efficiency.
 *	  RETURN
 *			  0  - strings equal
 *			  <0 - t > s
 *			  >0 - t < s
 */
{
	if (s == NULL)
		s = "";
	if (t == NULL)
		t = "";
	if (n < 0)
	    n = 0;
	copy(s, t, n);
	return strcoll(str1, str2);

    /****************************
	// original code for strncmpi() - this is fast, but does not work
	// for anything except ASCII
	register unsigned char c;
	while (n-- > 1)
	{
		if (((c = (unsigned)_toupper((unsigned char)*s)) !=
			(unsigned)_toupper((unsigned char)*t))
		|| (c == '\0'))
		{
			break;
		}
		s++;
		t++;
	}
	return (unsigned)_toupper((unsigned char)*s) -
		(unsigned)_toupper((unsigned char)*t);
    ****************************/
}
