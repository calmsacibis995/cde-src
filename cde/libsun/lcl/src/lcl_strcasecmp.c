/*
 * Copyright (c) 1996, Sun Microsystems, Inc.
 * All rights reserved.
 */
 
#pragma ident	"@(#)lcl_strcasecmp.c 1.2	96/06/27 SMI"

/*//////////////////////////////////////////////////////////////////////*/
#include <ctype.h>

lcl_strncasecmp(a,b,n)
	register char *a,*b;
	register int n;
{	register char ac,bc;
	register int i;

	for( i = 1; i < n; i++ ){
		ac = *a++;
		bc = *b++;

		if(ac == 0){
			if(bc == 0)
				return 0;
			else	return -1;
		}else
		if(bc == 0)
			return 1;
		else
		if(ac != bc){
			if(islower(ac)) ac = toupper(ac);
			if(islower(bc)) bc = toupper(bc);
			if( ac != bc )
				return ac - bc;
		}
	}
	if(islower(*a)) ac = toupper(*a); else ac = *a;
	if(islower(*b)) bc = toupper(*b); else bc = *b;
	return ac - bc;
}

lcl_strcasecmp(a,b)
	char *a,*b;
{
	return strncasecmp(a,b,0xFFFFFFF);
}
