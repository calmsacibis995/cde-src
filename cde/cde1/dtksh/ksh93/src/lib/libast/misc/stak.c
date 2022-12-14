/* $XConsortium: stak.c /main/cde1_maint/1 1995/07/18 00:08:16 drk $ */
/***************************************************************
*                                                              *
*                      AT&T - PROPRIETARY                      *
*                                                              *
*        THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF        *
*                    AT&T BELL LABORATORIES                    *
*         AND IS NOT TO BE DISCLOSED OR USED EXCEPT IN         *
*            ACCORDANCE WITH APPLICABLE AGREEMENTS             *
*                                                              *
*          Copyright (c) 1994 AT&T Bell Laboratories           *
*              Unpublished & Not for Publication               *
*                     All Rights Reserved                      *
*                                                              *
*       The copyright notice above does not evidence any       *
*      actual or intended publication of such source code      *
*                                                              *
*               This software was created by the               *
*           Software Engineering Research Department           *
*                    AT&T Bell Laboratories                    *
*                                                              *
*               For further information contact                *
*                   advsoft@research.att.com                   *
*                 Randy Hackbarth 908-582-5245                 *
*                  Dave Belanger 908-582-7427                  *
*                                                              *
***************************************************************/

/* : : generated by proto : : */

#line 1

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

#line 21
#include	<sfio_t.h>
#include	<ast.h>
#include	<stak.h>

/*
 *  A stack is a header and a linked list of frames
 *  The first frame has structure
 *	Sfile_t
 *	Sfdisc_t
 *	struct stak
 * Frames have structure
 *	struct frame
 *	data
 */
#define STAK_FSIZE	(1024*sizeof(int))
#define STAK_HDRSIZE	(sizeof(Sfile_t)+sizeof(Sfdisc_t))

typedef char* (*_stak_overflow_) __PROTO__((int));

static int stakexcept __PROTO__((Sfile_t*,int,Sfdisc_t*));
static Sfdisc_t stakdisc = { 0, 0, 0, stakexcept };
Sfile_t _Stak_data=SFNEW((char*)0,0,-1,SF_STATIC|SF_WRITE|SF_STRING,&stakdisc);

struct frame
{
	char	*prev;	/* address of previous frame */
	char	*end;	/* address of end this frame */
};

struct stak
{
	_stak_overflow_	stakoverflow;	/* called when malloc fails */
	short		stakref;	/* reference count; */
	short		stakflags;	/* stack attributes */
	char		*stakbase;	/* beginning of current stack frame */
	char		*stakend;	/* end of current stack frame */
};

static int		init;		/* 1 when initialized */
static struct stak	*stakcur;	/* pointer to current stak */
static char		*stakgrow __PROTO__((unsigned));

#define stream2stak(stream)	((struct stak*)(((char*)(stream))+STAK_HDRSIZE))
#define stak2stream(sp)		((Sfile_t*)(((char*)(sp))-STAK_HDRSIZE))
#define stakleft()		(staksp->endb-staksp->data)
	

#ifdef STAKSTATS
    static struct
    {
	int	create;
	int	delete;
	int	install;
	int	alloc;
	int	copy;
	int	puts;
	int	seek;
	int	set;
	int	grow;
	int	addsize;
	int	delsize;
	int	movsize;
    } _stakstats;
#   define increment(x)	(_stakstats.x++)
#   define count(x,n)	(_stakstats.x += (n))
#else
#   define increment(x)
#   define count(x,n)
#endif /* STAKSTATS */

static const char Omsg[] = "malloc failed while growing stack\n";

/*
 * default overflow exception
 */
static char *overflow __PARAM__((int n), (n)) __OTORP__(int n;)
#line 97
{
	NoP(n);
	write(2,Omsg, sizeof(Omsg)-1);
	exit(2);
	/* NOTREACHED */
	return(0);
}

/*
 * initialize staksp, sfio operations may have already occcured
 */
static void stakinit __PARAM__((int size), (size)) __OTORP__(int size;)
#line 109
{
	register Sfile_t *sp;
	init = size;
	sp = stakcreate(0);
	init = 1;
	stakinstall(sp,overflow);
}

static int stakexcept __PARAM__((Sfile_t* sp, int type, Sfdisc_t* dp), (sp, type, dp)) __OTORP__(Sfile_t* sp; int type; Sfdisc_t* dp;)
#line 118
{
	NoP(dp);
	switch(type)
	{
	    case SF_CLOSE:
		return(stakdelete(sp));
	    case SF_WRITE:
	    case SF_SEEK:
		if(init)
		{
			Sfile_t *old = 0;
			if(sp!=staksp)
				old = stakinstall(sp,NiL);
			stakgrow(sfslen()-(staksp->endb-staksp->data));
			if(old)
				stakinstall(old,NiL);
		}
		else
			stakinit(sfslen());
		return(1);
	    case SF_NEW:
		return(-1);
	}
	return(0);
}

/*
 * create a stack
 */
Sfile_t *stakcreate __PARAM__((int flags), (flags)) __OTORP__(int flags;)
#line 148
{
	register int bsize;
	register Sfile_t *stream;
	register struct stak *sp;
	register struct frame *fp;
	register Sfdisc_t *dp;
	register char *cp;
	if(!(stream=newof((char*)0,Sfile_t, 1, sizeof(*dp)+sizeof(*sp))))
		return(0);
	increment(create);
	count(addsize,sizeof(*stream)+sizeof(*dp)+sizeof(*sp));
	dp = (Sfdisc_t*)(stream+1);
	dp->exceptf = stakexcept;
	sp = (struct stak*)(dp+1);
	sp->stakref = 1;
	sp->stakflags = (flags&STAK_SMALL);
	sp->stakoverflow = stakcur?stakcur->stakoverflow:overflow;
	bsize = init+sizeof(struct frame);
#ifndef USE_REALLOC
	if(flags&STAK_SMALL)
		bsize = roundof(bsize,STAK_FSIZE/16);
	else
#endif /* USE_REALLOC */
		bsize = roundof(bsize,STAK_FSIZE);
	bsize -= sizeof(struct frame);
	if(!(fp=newof((char*)0,struct frame, 1,bsize)))
	{
		free(stream);
		return(0);
	}
	count(addsize,sizeof(*fp)+bsize);
	cp = (char*)(fp+1);
	sp->stakbase = (char*)fp;
	fp->prev = 0;
	fp->end = sp->stakend = cp+bsize;
	if(!sfnew(stream,cp,bsize,-1,SF_STRING|SF_WRITE|SF_STATIC|SF_EOF))
		return((Sfile_t*)0);
	sfdisc(stream,dp);
	return(stream);
}

/*
 * return a pointer to the current stack
 * if <stream> is not null, it becomes the new current stack
 * <oflow> becomes the new overflow function
 */
Sfile_t *stakinstall __PARAM__((Sfile_t *stream, _stak_overflow_ oflow), (stream, oflow)) __OTORP__(Sfile_t *stream; _stak_overflow_ oflow;)
#line 195
{
	Sfile_t *old;
	register struct stak *sp;
	if(!init)
	{
		stakinit(1);
		if(oflow)
			stakcur->stakoverflow = oflow;
		return((Sfile_t*)0);
	}
	increment(install);
	old = stakcur?stak2stream(stakcur):0;
	if(stream)
	{
		sp = stream2stak(stream);
		while(sfstack(staksp, SF_POPSTACK));
		if(stream!=staksp)
			sfstack(staksp,stream);
		stakcur = sp;
#ifdef USE_REALLOC
		/*** someday ***/
#endif /* USE_REALLOC */
	}
	else
		sp = stakcur;
	if(oflow)
		sp->stakoverflow = oflow;
	return(old);
}

/*
 * increase the reference count on the given <stack>
 */
int staklink __PARAM__((register Sfile_t* stream), (stream)) __OTORP__(register Sfile_t* stream;)
#line 229
{
	register struct stak *sp = stream2stak(stream);
	return(sp->stakref++);
}

/*
 * terminate a stack and free up the space
 */
int stakdelete __PARAM__((Sfile_t* stream), (stream)) __OTORP__(Sfile_t* stream;)
#line 238
{
	register struct stak *sp = stream2stak(stream); 
	register char *cp = sp->stakbase;
	register struct frame *fp;
	if(--sp->stakref>0)
		return(1);
	increment(delete);
	if(stream==staksp)
		stakset((char*)0,0);
	else while(1)
	{
		fp = (struct frame*)cp;
		if(fp->prev)
		{
			cp = fp->prev;
			free(fp);
		}
		else
		{
			free(fp);
			break;
		}
	}
	return(0);
}

/*
 * reset the bottom of the current stack back to <loc>
 * if <loc> is not in this stack, then the stack is reset to the beginning
 * otherwise, the top of the stack is set to stakbot+<offset>
 *
 */
char *stakset __PARAM__((register char* loc, unsigned offset), (loc, offset)) __OTORP__(register char* loc; unsigned offset;)
#line 271
{
	register struct stak *sp=stakcur;
	register char *cp;
	register struct frame *fp;
	register int frames = 0;
	if(!init)
		stakinit(offset+1);
	increment(set);
	while(1)
	{
		/* see whether <loc> is in current stack frame */
		if(loc>=(cp=sp->stakbase) && loc<=sp->stakend)
		{
			cp += sizeof(struct frame);
			if(frames)
				sfsetbuf(staksp,cp,sp->stakend-cp);
			staksp->data = (unsigned char*)(cp + roundof(loc-cp,sizeof(char*)));
			staksp->next = (unsigned char*)loc+offset;
			goto found;
		}
		fp = (struct frame*)cp;
		if(fp->prev)
		{
			sp->stakbase = fp->prev;
			sp->stakend = ((struct frame*)(fp->prev))->end;
			free(cp);
		}
		else
			break;
		frames++;
	}
	/* set stack back to the beginning */
	cp = (char*)(fp+1);
	if(frames)
		sfsetbuf(staksp,cp,sp->stakend-cp);
	else
		staksp->data = staksp->next = (unsigned char*)cp;
found:
	return((char*)staksp->data);
}

/*
 * allocate <n> bytes on the current stack
 */
char *stakalloc __PARAM__((register unsigned int n), (n)) __OTORP__(register unsigned int n;)
#line 316
{
	register unsigned char *old;
	if(!init)
		stakinit(n);
	increment(alloc);
	n = roundof(n,sizeof(char*));
	if(stakleft() <= (int)n)
		stakgrow(n);
	old = staksp->data;
	staksp->data = staksp->next = old+n;
	return((char*)old);
}

/*
 * begin a new stack word of at least <n> bytes
 */
char *_stakseek __PARAM__((register unsigned n), (n)) __OTORP__(register unsigned n;)
#line 333
{
	if(!init)
		stakinit(n);
	increment(seek);
	if(stakleft() <= (int)n)
		stakgrow(n);
	staksp->next = staksp->data+n;
	return((char*)staksp->data);
}

/*
 * advance the stack to the current top
 * if extra is non-zero, first add a extra bytes and zero the first
 */
char	*stakfreeze __PARAM__((register unsigned extra), (extra)) __OTORP__(register unsigned extra;)
#line 348
{
	register unsigned char *old, *top;
	if(!init)
		stakinit(extra);
	old = staksp->data;
	top = staksp->next;
	if(extra)
	{
		if(extra > (staksp->endb-staksp->next))
		{
			top = (unsigned char*)stakgrow(extra);
			old = staksp->data;
		}
		*top = 0;
		top += extra;
	}
	staksp->next = staksp->data += roundof(top-old,sizeof(char*));
	return((char*)old);
}

/*
 * copy string <str> onto the stack as a new stack word
 */
char	*stakcopy __PARAM__((const char* str), (str)) __OTORP__(const char* str;)
#line 372
{
	register unsigned char *cp = (unsigned char*)str;
	register int n;
	while(*cp++);
	n = roundof(cp-(unsigned char*)str,sizeof(char*));
	if(!init)
		stakinit(n);
	increment(copy);
	if(stakleft() <= n)
		stakgrow(n);
	strcpy((char*)(cp=staksp->data),str);
	staksp->data = staksp->next = cp+n;
	return((char*)cp);
}

/*
 * add a new stack frame of size >= <n> to the current stack.
 * if <n> > 0, copy the bytes from stakbot to staktop to the new stack
 * if <n> is zero, then copy the remainder of the stack frame from stakbot
 * to the end is copied into the new stack frame
 */

static char *stakgrow __PARAM__((unsigned size), (size)) __OTORP__(unsigned size;)
#line 395
{
	register int n = size;
	register struct stak *sp = stakcur;
	register struct frame *fp;
	register char *cp;
	register unsigned m = staktell();
	register int reused = 0;
	n += (m + sizeof(struct frame)+1);
	if(sp->stakflags&STAK_SMALL)
#ifndef USE_REALLOC
		n = roundof(n,STAK_FSIZE/16);
	else
#endif /* !USE_REALLOC */
		n = roundof(n,STAK_FSIZE);
	/* see whether current frame can be extended */
	if((char*)(staksp->data) == sp->stakbase+sizeof(struct frame))
	{
		cp = newof(sp->stakbase,char,n,0);
		reused++;
	}
	else
		cp = newof((char*)0, char, n, 0);
	if(!cp) cp = (*sp->stakoverflow)(n);
	increment(grow);
	count(addsize,n);
	fp = (struct frame*)cp;
	if(!reused)
	{
		fp->prev = sp->stakbase;
	}
	sp->stakbase = cp;
	sp->stakend = fp->end = cp+n;
	cp = (char*)(fp+1);
	if(m && !reused)
		memcpy(cp,(char*)staksp->data,m);
	count(movsize,m);
	sfsetbuf(staksp,cp,sp->stakend-cp);
	return((char*)(staksp->next = staksp->data+m));
}

