/*
 * File:	strhash.C $XConsortium: strhash.C /main/cde1_maint/3 1995/10/18 12:44:34 drk $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */

#include <codelibs/nl_hack.h>
#include "stringx.h"


/**************************************************
// This is quick but does a pretty crummy job
unsigned strhash(register const unsigned char *key)
{
	register unsigned hash = 0;
	while (*key != '\0')
		hash = (hash << 3) + *key++;
	return hash;
}
**************************************************/


#include <limits.h>

#define BITS(type) 	(CHAR_BIT * (int)sizeof(type))

// This is from the "dragon" Compilers book.
// It is much better than the above but somewhat slower.
//
unsigned strhash(register const char *p)
{
	register unsigned h = 0;
	register unsigned g;
	if (p != NULL)
		while (*p != '\0')
		{
			h = (h << 4) + (unsigned)CHARADV(p);
			if (g = h & ((unsigned)0xF << BITS(unsigned) - 4))
			{
				h ^= g >> BITS(unsigned) - 4;
				h ^= g;
			}
		}
	return h;
}

// Same as above but case insensitive.  Returns the same value as the
// above function if there are no upper case letters in the string.
//
unsigned strhashi(register const char *p)
{
	register unsigned h = 0;
	register unsigned g;
	if (p != NULL)
		while (*p != '\0')
		{
			if (mblen(p, MB_CUR_MAX) > 1)
				h = (h << 4) + (unsigned)CHARADV(p);
			else
			{
				h = (h << 4) + (unsigned)tolower(*p++);
			}

			if (g = h & ((unsigned)0xF << BITS(unsigned) - 4))
			{
				h ^= g >> BITS(unsigned) - 4;
				h ^= g;
			}
		}
	return h;
}


/**************************************************
// This is about twice as slow as the above but 
// does a slightly better hash.
// by  TJ Merritt

unsigned int
hashfunc(buf, len)
register unsigned char *buf;
register int len;
{
	register unsigned int in;
	register unsigned int xor;
	register unsigned int t;

	xor = len << 8;
	in = 0;

	while (len-- > 0)
	{
		in <<= 8;
		in += *buf++;
		xor ^= in;
		t = ((xor & 0x3) << 29) | (xor >> 3);
		xor ^= t;
	}

	return xor ^ (xor >> 16);
}
**************************************************/
