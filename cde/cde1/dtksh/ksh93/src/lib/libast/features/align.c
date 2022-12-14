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
union _u_
{
	long		u1;
	char*		u2;
	double		u3;
	char		u4[1024];
};

struct _s_
{
	char		s1;
	union _u_	s2;
};

#define roundof(x,y)	(((x)+((y)-1))&~((y)-1))

extern __MANGLE__ int		printf __PROTO__((const char*, ...));

main()
{
	register int	i;
	register int	j;
	register int	k;

	int		align0;
	int		align1;
	int		align2;
	unsigned long	bit1;
	unsigned long	bit2;
	unsigned long	bits0;
	unsigned long	bits1;
	unsigned long	bits2;
	union _u_	u;
	union _u_	v;

	u.u2 = u.u4;
	v.u2 = u.u2 + 1;
	bit1 = u.u1 ^ v.u1;
	v.u2 = u.u2 + 2;
	bit2 = u.u1 ^ v.u1;
	align0 = sizeof(struct _s_) - sizeof(union _u_);
	bits0 = 0;
	k = 0;
	for (j = 0; j < align0; j++)
	{
		u.u2 = u.u4 + j;
		bits1 = 0;
		for (i = 0; i < align0; i++)
		{
			v.u2 = u.u2 + i;
			bits1 |= u.u1 ^ v.u1;
		}
		if (!bits0 || bits1 < bits0)
		{
			bits0 = bits1;
			k = j;
		}
	}
	align1 = roundof(align0, 2);
	u.u2 = u.u4 + k;
	for (bits1 = bits0; i < align1; i++)
	{
		v.u2 = u.u2 + i;
		bits1 |= u.u1 ^ v.u1;
	}
	align2 = roundof(align0, 4);
	for (bits2 = bits1; i < align2; i++)
	{
		v.u2 = u.u2 + i;
		bits2 |= u.u1 ^ v.u1;
	}
	printf("typedef unsigned %s ALIGN_INTEGRAL;\n", sizeof(char*) >= sizeof(long) ? "long" : sizeof(char*) >= sizeof(int) ? "int" : "short");
	printf("\n");
	printf("#define ALIGN_CHUNK		%d\n", sizeof(char*) >= 4 ? 8192 : 1024);
	printf("#define ALIGN_INTEGRAL		%s\n", sizeof(char*) >= sizeof(long) ? "long" : sizeof(char*) >= sizeof(int) ? "int" : "short");
	printf("#define ALIGN_INTEGER(x)	((ALIGN_INTEGRAL)(x))\n");
	printf("#define ALIGN_POINTER(x)	((char*)(x))\n");
	if (bits2 == (align2 - 1)) printf("#define ALIGN_ROUND(x,y)	ALIGN_POINTER(ALIGN_INTEGER((x)+(y)-1)&~((y)-1))\n");
	else printf("#define ALIGN_ROUND(x,y)	ALIGN_POINTER(ALIGN_INTEGER(ALIGN_ALIGN(x)+(((y)+%d)/%d)-1)&~((((y)+%d)/%d)-1))\n", align0, align0, align0, align0);
	printf("\n");
	if (align0 == align2)
	{
		printf("#define ALIGN_BOUND		ALIGN_BOUND2\n");
		printf("#define ALIGN_ALIGN(x)		ALIGN_ALIGN2(x)\n");
		printf("#define ALIGN_TRUNC(x)		ALIGN_TRUNC2(x)\n");
	}
	else if (align0 == align1)
	{
		printf("#define ALIGN_BOUND		ALIGN_BOUND1\n");
		printf("#define ALIGN_ALIGN(x)		ALIGN_ALIGN1(x)\n");
		printf("#define ALIGN_TRUNC(x)		ALIGN_TRUNC1(x)\n");
	}
	else
	{
		printf("#define ALIGN_BOUND		1\n");
		printf("#define ALIGN_ALIGN(x)		ALIGN_POINTER(x)\n");
		printf("#define ALIGN_TRUNC(x)		ALIGN_POINTER(x)\n");
	}
	printf("\n");
	printf("#define ALIGN_BIT1		0x%lx\n", bit1);
	if (align1 == align2)
	{
		printf("#define ALIGN_BOUND1		ALIGN_BOUND2\n");
		printf("#define ALIGN_ALIGN1(x)		ALIGN_ALIGN2(x)\n");
		printf("#define ALIGN_TRUNC1(x)		ALIGN_TRUNC2(x)\n");
	}
	else
	{
		printf("#define ALIGN_BOUND1		%d\n", align1);
		printf("#define ALIGN_ALIGN1(x)		ALIGN_TRUNC1((x)+%d)\n", align1 - 1);
		printf("#define ALIGN_TRUNC1(x)		ALIGN_POINTER(ALIGN_INTEGER((x)+%d)&0x%lx)\n", align1 - 1, ~(bits0|bits1));
	}
	printf("#define ALIGN_CLRBIT1(x)	ALIGN_POINTER(ALIGN_INTEGER(x)&0x%lx)\n", ~bit1);
	printf("#define ALIGN_SETBIT1(x)	ALIGN_POINTER(ALIGN_INTEGER(x)|0x%lx)\n", bit1);
	printf("#define ALIGN_TSTBIT1(x)	ALIGN_POINTER(ALIGN_INTEGER(x)&0x%lx)\n", bit1);
	printf("\n");
	printf("#define ALIGN_BIT2		0x%lx\n", bit2);
	printf("#define ALIGN_BOUND2		%d\n", align2);
	printf("#define ALIGN_ALIGN2(x)		ALIGN_TRUNC2((x)+%d)\n", align2 - 1);
	printf("#define ALIGN_TRUNC2(x)		ALIGN_POINTER(ALIGN_INTEGER(x)&0x%lx)\n", ~(bits0|bits1|bits2));
	printf("#define ALIGN_CLRBIT2(x)	ALIGN_POINTER(ALIGN_INTEGER(x)&0x%lx)\n", ~bit2);
	printf("#define ALIGN_SETBIT2(x)	ALIGN_POINTER(ALIGN_INTEGER(x)|0x%lx)\n", bit2);
	printf("#define ALIGN_TSTBIT2(x)	ALIGN_POINTER(ALIGN_INTEGER(x)&0x%lx)\n", bit2);
	printf("\n");
	return(0);
}
