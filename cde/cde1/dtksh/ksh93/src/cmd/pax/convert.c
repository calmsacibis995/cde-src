/* $XConsortium: convert.c /main/cde1_maint/3 1995/10/14 00:20:56 montyb $ */
/***************************************************************
*                                                              *
*                      AT&T - PROPRIETARY                      *
*                                                              *
*         THIS IS PROPRIETARY SOURCE CODE LICENSED BY          *
*                          AT&T CORP.                          *
*                                                              *
*                Copyright (c) 1995 AT&T Corp.                 *
*                     All Rights Reserved                      *
*                                                              *
*           This software is licensed by AT&T Corp.            *
*       under the terms and conditions of the license in       *
*       http://www.research.att.com/orgs/ssr/book/reuse        *
*                                                              *
*               This software was created by the               *
*           Software Engineering Research Department           *
*                    AT&T Bell Laboratories                    *
*                                                              *
*               For further information contact                *
*                     gsf@research.att.com                     *
*                                                              *
***************************************************************/

/* : : generated by proto : : */

#if !defined(__PROTO__)
#if defined(__STDC__) || defined(__cplusplus) || defined(_proto) || defined(c_plusplus)
#if defined(__cplusplus)
#define __MANGLE__	"C"
#else
#define __MANGLE__
#endif
#define __STDARG__
#define __PROTO__(x)	x
#define __OTORP__(x)
#define __PARAM__(n,o)	n
#if !defined(__STDC__) && !defined(__cplusplus)
#if !defined(c_plusplus)
#define const
#endif
#define signed
#define void		int
#define volatile
#define __V_		char
#else
#define __V_		void
#endif
#else
#define __PROTO__(x)	()
#define __OTORP__(x)	x
#define __PARAM__(n,o)	o
#define __MANGLE__
#define __V_		char
#define const
#define signed
#define void		int
#define volatile
#endif
#if defined(__cplusplus) || defined(c_plusplus)
#define __VARARG__	...
#else
#define __VARARG__
#endif
#if defined(__STDARG__)
#define __VA_START__(p,a)	va_start(p,a)
#else
#define __VA_START__(p,a)	va_start(p)
#endif
#endif
#include "pax.h"

/*
 * convert string to lower case in place
 */

char*
strlower __PARAM__((register char* s), (s)) __OTORP__(register char* s;){
	register int	c;
	register char*	t;

	for (t = s; c = *t; t++)
		if (isupper(c))
			*t = tolower(c);
	return(s);
}

/*
 * convert string to upper case in place
 */

char*
strupper __PARAM__((register char* s), (s)) __OTORP__(register char* s;){
	register int	c;
	register char*	t;

	for (t = s; c = *t; t++)
		if (islower(c))
			*t = toupper(c);
	return(s);
}

/*
 * convert binary header shorts to long
 */

long
cpio_long __PARAM__((register unsigned short* s), (s)) __OTORP__(register unsigned short* s;){
	Integral_t	u;

	u.l = 1;
	if (u.c[0])
	{
		u.s[0] = s[1];
		u.s[1] = s[0];
	}
	else
	{
		u.s[0] = s[0];
		u.s[1] = s[1];
	}
	return(u.l);
}

/*
 * convert long to binary header shorts
 */

void
cpio_short __PARAM__((register unsigned short* s, long n), (s, n)) __OTORP__(register unsigned short* s; long n;){
	Integral_t	u;

	u.l = 1;
	if (u.c[0])
	{
		u.l = n;
		s[0] = u.s[1];
		s[1] = u.s[0];
	}
	else
	{
		u.l = n;
		s[0] = u.s[0];
		s[1] = u.s[1];
	}
}

/*
 * compute tar_header checksum
 */

int
tar_checksum __PARAM__((void), ()){
	register char*	p;
	register int	n;

	p = tar_header.chksum;
	while (p < &tar_header.chksum[sizeof(tar_header.chksum)]) *p++ = ' ';
	n = 0;
	p = tar_block;
	while (p < &tar_block[TAR_HEADER]) n += *p++;
	return(n);
}

/*
 * compute running s5r4 file content checksum
 */

long
asc_checksum __PARAM__((char* ab, int n, register unsigned long sum), (ab, n, sum)) __OTORP__(char* ab; int n; register unsigned long sum;){
	register unsigned char* b = (unsigned char*)ab;
	register unsigned char*	e;

	e = b + n;
	while (b < e) sum += *b++;
	return(sum);
}

/*
 * get label header number
 */

long
getlabnum __PARAM__((register char* p, int byte, int width, int base), (p, byte, width, base)) __OTORP__(register char* p; int byte; int width; int base;){
	register char*	e;
	register int	c;
	long		n;

	p += byte - 1;
	c = *(e = p + width);
	*e = 0;
	n = strtol(p, NiL, base);
	*e = c;
	return(n);
}

/*
 * get label header string
 */

char*
getlabstr __PARAM__((register char* p, int byte, int width, register char* s), (p, byte, width, s)) __OTORP__(register char* p; int byte; int width; register char* s;){

	register char*	e;
	char*		v;

	v = s;
	p += byte - 1;
	e = p + width;
	while (p < e && (*s = *p++) != ' ') s++;
	*s = 0;
	return(v);
}

/*
 * this is an obsolete version of the ast library implementation
 */

#undef	OHASHPART
#define OHASHPART(b,h,c,l,r)	(h = ((h = ((unsigned)h << (l)) ^ (h >> (r)) ^ (c)) & ((unsigned)1 << (b)) ? ~h : h) & (((((unsigned)1 << ((b) - 1)) - 1) << 2) | 3))

#undef	OHASHLPART
#define OHASHLPART(h,c)		OHASHPART(31, h, c, 3, 2)

unsigned long
omemsum __PARAM__((const __V_* b, int n, register unsigned long c), (b, n, c)) __OTORP__(const __V_* b; int n; register unsigned long c;){
	register unsigned char*	p;
	register unsigned char*	e;

	p = (unsigned char*)b;
	e = p + n;
	while (p < e) OHASHLPART(c, *p++ + 1);
	return(c);
}
