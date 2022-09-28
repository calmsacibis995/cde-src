/* $XConsortium: chkfmt.c /main/cde1_maint/1 1995/07/17 16:37:42 drk $ */
static char sccsid[] = "@(#)chkfmt.c	1.12  @(#)chkfmt.c	1.12 01/20/96 15:41:17";
/*
 * COMPONENT_NAME: (PIITOOLS) AIX PII Tools
 *
 * FUNCTIONS: chkfmt
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
#include <cur01.h>


/****************************          NOTE           **************************
*  The functions followed are used for checking formats in printf and NLprintf *
*  Assuming maximum number of parameters is 64        	         	 
*  The parser used table driven techniqu that making change easily  
*
*  Position table sructure:
*	PRCNT		ORDER		DOLAR		FTLEN
*	%		number		$		length
*  eg.
*	" this is a %3s "
*	11		0		0		2
*	" is this a %1$10s ?"
*	11		1		2		3
*
*************************          FEB 24 1989         ************************/

    	

#define   LEVEL		9		/* char set max level	*/		
#define	  COUNT		2		/* contrl table count   */
#define	  PRCNT 	0		/* percent sign position */
#define	  ORDER 	1		/* order number		*/
#define	  DOLAR 	2		/* dollar sign position */
#define	  FTLEN 	3		/* format length	*/

static	char tbl[LEVEL + 2][20] = {
	"%",
	" -+#BN",
	"0123456789",
	".",
	"0123456789",
	"lh",
	"w",
	"eEfgGSC",
	"douxX",
	"sc",
	};

static 	int ctl[LEVEL + 2] = {
	1,
	2,
	2,
	4,
	4,
	8,
	9,
	LEVEL+1,
	LEVEL+1,
	LEVEL+1,
	};

static	char dls[COUNT + 2][20] = {
	"%",
	"0123456789",
	"$",
	};

static	int cnt[COUNT + 2] = {
	1,
	1,
	COUNT+1,
	};


/*
 * NAME: 	ck_tbl
 *
 * FUNCTION:	check a character in character set or not 
 *
 * RETURN VALUE DESCRIPTION:  next level of character set 
 */


#ifdef	_ANSI
ck_tbl(char  *s,int  n,int  l,char  tb[][20],int   ct[])
#else		/*_ANSI */
ck_tbl(s,n,l,tb,ct)
char *s;		/*	check str	*/
int n;			/*	start level	*/
int l;			/*	max level	*/
char tb[][20];		/*	compare table	*/
int  ct[];		/*	control table	*/
#endif		/*_ANSI */
{
	int i;
	for( ; n <= l; n++ )
		for( i=0; tb[n][i] ; i++ )
			if( *s == tb[n][i] )
				return (ct[n]);
	return ( 0 );
}



/*
 * NAME: 	ck_val
 *
 * FUNCTION:	check format validation 
 *
 * RETURN VALUE DESCRIPTION: return level of ending character 
 */

#ifdef	_ANSI
ck_val(char  *s,int  l,int  e,char  tb[][20],int   ct[],int  *n)
#else		/*_ANSI */
ck_val(s,l,e,tb,ct,n)
char *s;		/*	check str	*/
int l;			/*	max level	*/
int e;			/*	end level	*/
char tb[][20];		/*	compare table	*/
int  ct[];		/*	control table	*/
int *n;			/*	digit length	*/
#endif		/*_ANSI */
{
	int  rn = 1;
	int i = 1;
	char *p;
	p = s;
	while ( i > 0 && rn != e && *s ){ 
		rn = ck_tbl( s++ , i, l, tb, ct); 
		i = ( rn < i )? -1 : rn;
		}
	*n = (rn == e) ? s - p : 0;
	return ( rn );
}



/*
 * NAME: 	ck_fmt	
 *
 * FUNCTION:  	set the format position information in table 
 *
 * RETURN VALUE DESCRIPTION:  return number of format 
 */

#ifdef	_ANSI
ck_fmt(char  *s,int   ps[][4])
#else		/*_ANSI */
ck_fmt(s,ps)
char *s;		/* 	check str	*/
int  ps[][4];		/*	position table	*/
#endif		/*_ANSI */
{
	int n;
	int i = 0;
	char *p;
	p = s;
	while ( *s ) {
		if ( *s == '%' ) {
			if ( *(s+1) == '%' ) s+=2;
			else {
				ps[i][PRCNT] = ( s - p );
				ps[i][ORDER] = ps[i][DOLAR] = 0;
				if (ck_val( s+1, COUNT, COUNT+1, dls, cnt, &n ) == COUNT+1){
					if( n>1 ) {
					ps[i][ORDER] = atoi(s+1);
					s+=( ps[i][DOLAR] = n );
					}
				}
			i += ( ck_val( ++s, LEVEL, LEVEL+1, tbl, ctl, &ps[i][FTLEN]) == LEVEL+1);
			}
		}		
		else s++;
	}
	ps[i][PRCNT] = -1;
	return ( i );
}


/*
 * NAME: chk_format
 *
 * FUNCTION:  check format in string
 *
 * RETURN VALUE DESCRIPTION: number of format characters 
 */



#ifdef	_ANSI
int chk_format( NLSCHAR *str )
#else		/*_ANSI */
int chk_format( str )
NLSCHAR *str;
#endif		/*_ANSI */
{
	int  n = 0;
	int  i = 0;
	char *s, stmp[5000];
	s = stmp;
	cvnls(s,str);
	if( *s == '%' ) {
		if (ck_val( s+1, COUNT, COUNT+1, dls, cnt, &n ) == COUNT+1){
			if( n>1 ) {
				s+= n ;
			}
		}
		if (ck_val( ++s, LEVEL, LEVEL+1, tbl, ctl, &i) == LEVEL+1){
			n += i + 1; 
		}
		else n = 0;
	}
	return ( n );
}



/*
 * NAME: 	ck_num
 *
 * FUNCTION:    check actual number of format  
 *
 * RETURN VALUE DESCRIPTION: actual number of format 
 */

#ifdef	_ANSI
ck_num(int  n,int  ps[][4])
#else		/*_ANSI */
ck_num( n, ps )
int n;			/*	max number	*/
int ps[][4];		/*	position table  */
#endif		/*_ANSI */
{
	int i, rn = 0;
	for( i=0; i<n; i++) {
		if( (!!ps[0][DOLAR]) != (!!ps[i][DOLAR]) )
			return( 0 );
		rn = (ps[0][DOLAR])? ((rn < ps[i][ORDER])? ps[i][ORDER]: rn) : rn+1;
	}
	return( rn>n? 0: rn);
}


/*
 * NAME: 	ck_ord
 *
 * FUNCTION:  	check order match or not
 *
 * RETURN VALUE DESCRIPTION: return TRUE if match otherwise FALSE 
 */

#ifdef	_ANSI
ck_ord(int  tn,int  ti,int  pt[][4])
#else		/*_ANSI */
ck_ord( tn, ti, pt)
int tn;			/*	max number	*/
int ti;			/*	max value	*/
int pt[][4];		/*	position table	*/
#endif		/*_ANSI */
{
	int i;
	while ( ti ) {
		for( i=0; i<tn; i++ )
			if( ti == pt[i][ORDER] )
				break;
		if (i < tn) ti--;
		else return( 0 );
	}
	return( 1 );
}


/*
 * NAME:	ck_cmp 
 *
 * FUNCTION:	check two string match or not in length 
 *
 * RETURN VALUE DESCRIPTION: return TRUE if match otherwise FALSE 
 */

#ifdef	_ANSI
ck_cmp(char  *t,char  *s,int  l)
#else		/*_ANSI */
ck_cmp( t, s, l )
char *t;		/*	compare str	*/		
char *s;		/*	compared str	*/	
int l;			/*	compare length	*/
#endif		/*_ANSI */
{
	int rn = 1;
	while ( l && rn ) {
		rn = ( *(t+l) == *(s+l) ); 
		l -- ;
	}
	return ( rn );
}


/*
 * NAME:	ck_sgl	
 *
 * FUNCTION: 	check single format with source format strig 
 *
 * RETURN VALUE DESCRIPTION: return TRUE if  valid otherwise FALSE 
 */

#ifdef	_ANSI
ck_sgl(char  *t,int  l,int  n,int  sn,char  *s,int  ps[][4])
#else		/*_ANSI */
ck_sgl(t,l,n,sn,s,ps)
char *t;		/*	compare str	*/
int l;			/*	compare length	*/
int n;			/*	compare index	*/
int sn;			/*	compared number */
char *s;		/*	compared str	*/
int ps[][4];		/*	postion table	*/
#endif		/*_ANSI */
{
	int i;
	int rn = 0;
	if( ps[0][ORDER] ) {
		for( i=0; !rn && i<sn; i++ ) 
			if( n == ps[i][ORDER] && l == ps[i][FTLEN] )
				rn = ck_cmp( t, (s + ps[i][PRCNT] + ps[i][DOLAR]), l);
	}
	else if ( l == ps[--n][FTLEN] )
		rn = ck_cmp( t, (s + ps[n][PRCNT] + ps[n][DOLAR]), l);
	return( rn );
}


/*
 * NAME:	ck_all 
 *
 * FUNCTION:	check every format validation in each format string 
 *
 * RETURN VALUE DESCRIPTION: return TRUE if successful otherwise FALSE 
 */

#ifdef	_ANSI
ck_all(int  sn,char  *s,int  ps[][4],int  tn,char  *t,int  pt[][4])
#else		/*_ANSI */
ck_all(sn,s,ps,tn,t,pt)
int sn;			/*	compare number	*/
char *s;		/*	compare str	*/
int ps[][4];		/*	position table  */
int tn;			/*	compared number */
char *t;		/*	compared str	*/
int pt[][4];		/*	postion table	*/
#endif		/*_ANSI */
{
	int i;
	int rn = 1;
	for( i=0; rn && i<tn; i++) 
		rn = ck_sgl( ( t + pt[i][PRCNT] + pt[i][DOLAR]), pt[i][FTLEN], pt[i][DOLAR]?
			pt[i][ORDER] : i+1, sn, s, ps); 
	return( rn );
}


/*
 * NAME: 	ck_mat
 *
 * FUNCTION: 	check match or not for given two strings and their format
 *		positions
 *
 * RETURN VALUE DESCRIPTION:  return TRUE if match otherwise FALSE 
 */

#ifdef	_ANSI
ck_mat(int  sn,char  *s,int  ps[][4],int  tn,char  *t,int  pt[][4])
#else		/*_ANSI */
ck_mat(sn,s,ps,tn,t,pt)
int sn;			/*	compare number	*/
char *s;		/*	compare str	*/
int ps[][4];		/*	position table	*/
int tn;			/*	compared number	*/
char *t;		/*	compared str	*/
int pt[][4];		/*	position table	*/
#endif		/*_ANSI */
{
	int rn, si, ti;
	si = ck_num(sn, ps);
	ti = ck_num(tn, pt);
	if (!si && !ti) 
		rn = 1;
	else if( rn = (	si==ti )) {
		if( pt[0][DOLAR] )
			rn = ck_ord( tn, ti, pt );  
		if (rn) rn = ck_all(sn,s,ps,tn,t,pt); 
		}
	return (rn);
}



/*
 * NAME: 	ck_str
 *
 * FUNCTION: 	check two strings  whether their print format is match
 *
 * RETURN VALUE DESCRIPTION: return TRUE if match else FALSE for unmatch 
 */

#ifdef	_ANSI
ck_str(char  *s,char  *t)
#else		/*_ANSI */
ck_str(s, t)
char *s;		/* 	source str	*/
char *t;		/*	target str	*/
#endif		/*_ANSI */
{
#define		PARMAX	64
	int sn,tn;
	int pos[PARMAX][4], pot[PARMAX][4];

	sn = ck_fmt(s,pos);
	tn = ck_fmt(t,pot);
	return(ck_mat(sn,s,pos,tn,t,pot));
}


