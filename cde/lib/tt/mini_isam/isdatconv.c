/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $XConsortium: isdatconv.c /main/cde1_maint/2 1995/10/07 19:12:22 pascale $ 			 				 */
/*
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */

/*
 * isdatconv.c
 *
 * Description:
 *      Conversion function between machine dependent and the X/OPEN 
 *	machine	independent formats.
 *
 * Some pieces of code may not be very "structured", but they result in
 * optimized code with the -O compiler option.
 *
 */

#include "isam_impl.h"

#define BLANK	' '

/* conversion functions for sparc architecture */

/* ldlong() - Load a long integer from a potentially  unaligned address */

long 
ldlong(p)
    register char     	*p;
{
    int i;
#if LONG_BIT == 64
    register unsigned long val;
#else
    register unsigned int val;
#endif

    val = 0;
    for (i=0; i<LONGSIZE ; i++)  
        val = (val << 8) + *((unsigned char *)p++);
    
    return ((long)val);
}

/* stlong() - Store a long integer at a potentially unaligned address */

void
stlong(val, p)
    register long	val;
    register char     	*p;
{
    int i;
    p += LONGSIZE;

    for (i=0; i<LONGSIZE ; i++)
        *--p = (val >> 8*i) & 255;
}

/* ldint() - Load a short integer from a potentially  unaligned address */

short
ldint(p)
    register char     	*p;
{
    register unsigned int val;

    val = *((unsigned char *)p++);
    val = (val << 8) + *((unsigned char *)p++);

    return ((short)val);
}


/* ldunshort - load a unshort integer : for 64K record length */

u_short
ldunshort(p)
    register char     	*p;
{
    register unsigned int val;

    val = *((unsigned char *)p++);
    val = (val << 8) + *((unsigned char *)p++);

    return ((u_short)val);
}

/* stint() - Store a short integer at a potentially unaligned address */

void
stint(register short val, register char *p)
{
    p += SHORTSIZE;
    *--p = val & 255;
    *--p = (val >> 8) & 255;
}

 

/* ldfloat() - Load a float number from a potentially  unaligned address */

float
ldfloat(p)
    register char     	*p;
{
    union {
	float fval;
	int   ival;
    } uval;
    register unsigned int val;

    val = *((unsigned char *)p++);
    val = (val << 8) + *((unsigned char *)p++);
    val = (val << 8) + *((unsigned char *)p++);
    val = (val << 8) + *((unsigned char *)p++);

    uval.ival = val;
    return (uval.fval);
}

#ifdef  notdef
/* stfloat() - Store a float number at a potentially unaligned address */

void
stfloat(float f, register char  *p)
    float		f;		     /* Bug - it is passed as double */
    register char     	*p;
{
    register unsigned  	val;
    union {
	float fval;
	int   ival;
    } uval;

    uval.fval = f;			     /* This fixes compiler bug */
    val = uval.ival;

    p += LONGSIZE;
    *--p = val & 255;
    *--p = (val >> 8) & 255;
    *--p = (val >> 16) & 255;
    *--p = (val >> 24) & 255;
}
#endif

#if sparc | mc68000    /* MRJ */

/* ldbld() - Load a double float number from a potentially unaligned address */

double
lddbl(p)
    register char     	*p;
{
    double val;

    memcpy((void *)&val, (const void *) p, DOUBLESIZE);
    return (val);
}

/* stdbl() - Store a double float number at a potentially unaligned address */

void
stdbl(val, p)
    double		val;
    register char     	*p;
{
    memcpy ( p,(char *)&val, DOUBLESIZE);
}

#else      /* 386i -- do it the long way round....  */

/* ldbld() - Load a double float number from a potentially unaligned address */

double
lddbl(p)
    register char     	*p;
{
    union {
        double rval;
	char   sval[DOUBLESIZE];
    } x;

    char  *q;
    int  i;

    q  =  x.sval;
    p +=  DOUBLESIZE;
   
    for (i=0; i<DOUBLESIZE; i++)
    	*q++ = *--p;
    return (x.rval);
}

/* stdbl() - Store a double float number at a potentially unaligned address */

void
stdbl(val, p)
    double		val;
    register char     	*p;
{
    union {
        double rval;
	char   sval[DOUBLESIZE];
    } x;

    char  *q;
    int  i;

    x.rval = val;
    q  =  x.sval;
    p +=  DOUBLESIZE;
   
    for (i=0; i<DOUBLESIZE; i++)
    	*--p = *q++ ;
}


#endif    /* sparc */


