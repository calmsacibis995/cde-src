/* $XConsortium: cut.c /main/cde1_maint/3 1995/10/14 01:34:26 montyb $ */
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
static const char id[] = "\n@(#)cut (AT&T Bell Laboratories) 04/01/93\0\n";

#include <cmdlib.h>
#include <ctype.h>

typedef struct
{
	int	cflag;
	int	sflag;
	int	wdelim;
	int	ldelim;
	int	seqno;
	int	list[2];
} Cut_t;

#define HUGE		(1<<14)
#define BSIZE		8*1024
#define C_BYTES		1
#define C_CHARS		2
#define C_FIELDS	4
#define C_SUPRESS	8
#define C_NOCHOP	16

static int seqno;

/*
 * compare the first of an array of integers
 */

static int mycomp __PARAM__((register const __V_ *a,register const __V_ *b), (a, b)) __OTORP__(register const __V_ *a;register const __V_ *b;){
	return(*((int*)a) - *((int*)b));
}

static Cut_t *cutinit __PARAM__((int mode,char *str,int wdelim,int ldelim), (mode, str, wdelim, ldelim)) __OTORP__(int mode;char *str;int wdelim;int ldelim;){
	register int *lp, c, n=0;
	register int range = 0;
	register char *cp = str;
	Cut_t *cuthdr;
	cuthdr = (Cut_t*)stakalloc(sizeof(Cut_t)+strlen(cp)*sizeof(int));
	cuthdr->cflag = ((mode&C_CHARS)!=0);
	cuthdr->sflag = (cuthdr->cflag ||((mode&C_SUPRESS)!=0));
	cuthdr->wdelim = (cuthdr->cflag?0:wdelim);
	cuthdr->ldelim = ldelim;
	cuthdr->seqno = ++seqno;
	lp = cuthdr->list;
	while(1) switch(c= *cp++)
	{
		case ' ':
		case '\t':
			while(*cp==' ' || *cp=='\t')
				cp++;
		case 0:
		case ',':
			if(range)
			{
				--range;
				if((n = (n==0?HUGE:n-range)) < 0)
					error(ERROR_exit(1),"invalid range for c/f option");
				*lp++ = range;
				*lp++ = n;
			}
			else
			{
				*lp++ = --n;
				*lp++ = 1;
			}
			if(c==0)
			{
				register int *dp;
				*lp = HUGE;
				n = 1 + (lp-cuthdr->list)/2;
				qsort(lp=cuthdr->list,n,2*sizeof(*lp),mycomp);
				/* eliminate overlapping regions */
				for(n=0,range= -2,dp=lp; *lp!=HUGE; lp+=2)
				{
					if(lp[0] <= range)
					{
						if(lp[1]==HUGE)
						{
							dp[-1] = HUGE;
							break;
						}
						if((c = lp[0]+lp[1]-range)>0)
						{
							range += c;
							dp[-1] += c;
						}
					}
					else
					{
						range = *dp++ = lp[0];
						if(lp[1]==HUGE)
						{
							*dp++ = HUGE;
							break;
						}
						range += (*dp++ = lp[1]);
					}
				}
				*dp = HUGE;
				/* if first field is copy, then -s doesn't
				 * matter, so enable it for efficiency
				 */
				if(*(lp=cuthdr->list)==0)
					cuthdr->sflag = 1;
				/* convert ranges into gaps */
				for(n=0; *lp!=HUGE; lp+=2)
				{
					c = *lp;
					*lp -= n;
					n = c+lp[1];
				}
				return(cuthdr);
			}
			n = range = 0;
			break;

		case '-':
			if(range)
				error(ERROR_exit(1),"bad list for c/f option");
			range = n?n:1;
			n = 0;
			break;

		default:
			if(!isdigit(c))
				error(ERROR_exit(1),"bad list for c/f option");
			n = 10*n + (c-'0');
	}
	/* NOTREACHED */
}

/*
 * cut each line of file <fdin> and put results to <fdout> using list <list>
 */

static int cutcols __PARAM__((const Cut_t *cuthdr,Sfio_t *fdin,Sfio_t *fdout), (cuthdr, fdin, fdout)) __OTORP__(const Cut_t *cuthdr;Sfio_t *fdin;Sfio_t *fdout;){
	register int		c, ncol=0;
	register const int	*lp = cuthdr->list;
	register char		*inp, *inend;
	register int		skip; /* non-zero for don't copy */
	int			inword = 0;
	while(inp = sfgetr(fdin,'\n', 0))
	{
		inend = inp+sfslen();
		if(!inword && (ncol = skip  = *(lp = cuthdr->list)) == 0)
			ncol = *++lp;
		inword = (inend[-1]!='\n');
		while(1)
		{
			if((c=ncol) > inend-inp)
				c = inend-inp;
			ncol -= c;
			if(!skip && sfwrite(fdout,(char*)inp,c)<0)
				return(-1);
			inp += c;
			if(ncol)
				break;
			ncol = *++lp;
			skip = !skip;
		}
		if(skip && !inword)
			sfputc(fdout,'\n');
	}
	return(c);
}

/*
 * cut each line of file <fdin> and put results to <fdout> using list <list>
 * stream <fdin> must be line buffered
 */

#define endline(c)	(((signed char)-1)<0?(c)<0:(c)==((char)-1))

static int cutfields __PARAM__((const Cut_t *cuthdr,Sfio_t *fdin,Sfio_t *fdout), (cuthdr, fdin, fdout)) __OTORP__(const Cut_t *cuthdr;Sfio_t *fdin;Sfio_t *fdout;){
	static signed char space[1<<CHAR_BIT];
	static int lastseq, lastwdelim = 0, lastldelim = '\n';
	register unsigned char *cp;
	register int c, nfields;
	register const int *lp = cuthdr->list;
	register unsigned char *copy;
	register int nodelim, empty, inword=0;
	register unsigned char *endbuff;
	unsigned char *inbuff, *first;
	int lastchar;
	Sfio_t *fdtmp = 0;
	long offset = 0;
	if(cuthdr->seqno != lastseq)
	{
		space[lastldelim] = 0;
		space[lastwdelim] = 0;
		space[(lastwdelim=cuthdr->wdelim)] = 1;
		space[(lastldelim=cuthdr->ldelim)] = -1;
		lastseq = cuthdr->seqno;
	}
	/* process each buffer */
	while ((inbuff = (unsigned char*)sfreserve(fdin, SF_UNBOUND, 0)) && (c = sfslen()) > 0)
	{
		cp = inbuff;
		endbuff = cp + --c;
		if((lastchar = cp[c]) != cuthdr->ldelim)
			*endbuff = cuthdr->ldelim;
		/* process each line in the buffer */
		while(cp <= endbuff)
		{
			first = cp;
			if(!inword)
			{
				nodelim = empty = 1;
				copy = cp;
				if(nfields = *(lp = cuthdr->list))
					copy = 0;
				else
					nfields = *++lp;
			}
			else if(copy)
				copy = cp;
			inword = 0;
			while(!inword)
			{
				/* skip over non-delimiter characters */
				while(!(c=space[*cp++]));
				/* check for end-of-line */
				if(endline(c))
				{
					if(cp<=endbuff)
						break;
					if((c=space[lastchar]),endline(c))
						break;
					/* restore last character */
					if(lastchar != cuthdr->ldelim)
						*endbuff = lastchar;
					inword++;
					if(!c)
						break;
				}
				nodelim = 0;	
				if(--nfields >0)
					continue;
				nfields = *++lp;
				if(copy)
				{
					empty = 0;
					if((c=(cp-1)-copy)>0 && sfwrite(fdout,(char*)copy,c)< 0)
						goto failed;
					copy = 0;
				}
				else
					/* set to delimiter unless the first field */
					copy = cp -!empty;
			}
			if(!inword)
			{
				if(!copy)
				{
					if(nodelim)
					{
						if(!cuthdr->sflag)
						{
							if(offset)
							{
								sfseek(fdtmp,0L,0);
								sfmove(fdtmp,fdout,offset,-1);
							}
							copy = first;
						}
					}
					else
						sfputc(fdout,'\n');
				}
				if(offset)
					sfseek(fdtmp,offset=0L,0);
			}
			if(copy && (c=cp-copy)>0 && sfwrite(fdout,(char*)copy,c)< 0)
				goto failed;
		}
		/* see whether to save in tmp file */
		if(nodelim && inword && !cuthdr->sflag && (c=cp-first)>0)
		{
			/* copy line to tmpfile in case no fields */
			if(!fdtmp)
				fdtmp = sftmp(BSIZE);
			sfwrite(fdtmp,(char*)first,c);
			offset +=c;
		}
	}
failed:
	if(fdtmp)
		sfclose(fdtmp);
	return(0);
}

int
b_cut __PARAM__((int argc,char *argv[]), (argc, argv)) __OTORP__(int argc;char *argv[];){
	register char *cp;
	register Sfio_t *fp;
	int	n;
	Cut_t	*cuthdr;
	int	mode = 0;
	int	wdelim = '\t';
	int	ldelim = '\n';

	NoP(id[0]);
	NoP(argc);
	cmdinit(argv);
	while (n = optget(argv, "b:[blist]c:[clist]f:[flist]d:[delim]D:[delim]ns [file...]")) switch (n)
	{
	  case 'b':
	  case 'c':
		if(mode&C_FIELDS)
		{
			error(2, "f option already specified");
			break;
		}
		cp = opt_info.arg;
		if(n=='b')
			mode |= C_BYTES;
		else
			mode |= C_CHARS;
		break;
	  case 'D':
		ldelim = *(unsigned char*)opt_info.arg;
		break;
	  case 'd':
		wdelim = *(unsigned char*)opt_info.arg;
		break;
	  case 'f':
		if(mode&(C_CHARS|C_BYTES))
		{
			error(2, "c option already specified");
			break;
		}
		cp = opt_info.arg;
		mode |= C_FIELDS;
		break;
	  case 'n':
		mode |= C_NOCHOP;
		break;
	  case 's':
		mode |= C_SUPRESS;
		break;
	  case ':':
		error(2, opt_info.arg);
		break;
	  case '?':
		error(ERROR_usage(2), opt_info.arg);
		break;
	}
	argv += opt_info.index;
	if (error_info.errors) error(ERROR_usage(2), optusage(NiL));
	if(!mode)
	{
		error(2, "c or f option must be specified");
		error(ERROR_usage(2), optusage(NiL));
	}
	cuthdr = cutinit(mode,cp,wdelim,ldelim);
	if(cp = *argv)
		argv++;
	do
	{
		if(!cp || streq(cp,"-"))
			fp = sfstdin;
		else if(!(fp = sfopen(NiL,cp,"r")))
		{
			error(ERROR_system(0),"%s: cannot open",cp);
			continue;
		}
		if(mode&C_FIELDS)
			cutfields(cuthdr,fp,sfstdout);
		else
			cutcols(cuthdr,fp,sfstdout);
		if(fp!=sfstdin)
			sfclose(fp);
	}
	while(cp= *argv++);
	return(0);
}
