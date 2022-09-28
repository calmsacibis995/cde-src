/* $XConsortium: emacs.c /main/cde1_maint/3 1995/10/14 00:04:39 montyb $ */
/***************************************************************
*                                                              *
*                      AT&T - PROPRIETARY                      *
*                                                              *
*        THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF        *
*                    AT&T BELL LABORATORIES                    *
*         AND IS NOT TO BE DISCLOSED OR USED EXCEPT IN         *
*            ACCORDANCE WITH APPLICABLE AGREEMENTS             *
*                                                              *
*                Copyright (c) 1995 AT&T Corp.                 *
*              Unpublished & Not for Publication               *
*                     All Rights Reserved                      *
*                                                              *
*       The copyright notice above does not evidence any       *
*      actual or intended publication of such source code      *
*                                                              *
*               This software was created by the               *
*           Advanced Software Technology Department            *
*                    AT&T Bell Laboratories                    *
*                                                              *
*               For further information contact                *
*                    {research,attmail}!dgk                    *
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
#include	<ast.h>
#include	<ctype.h>
#ifdef KSHELL
#   include	"defs.h"
#endif	/* KSHELL */
#include	"io.h"

#include	"history.h"
#include	"edit.h"
#include	"terminal.h"

#define ESH_NFIRST
#define ESH_KAPPEND
#define ESH_BETTER

#undef putchar
#define putchar(c)	ed_putchar(c)
#define beep()		ed_ringbell()


#ifdef SHOPT_MULTIBYTE
#   define gencpy(a,b)	ed_gencpy(a,b)
#   define genncpy(a,b,n)	ed_genncpy(a,b,n)
#   define genlen(str)	ed_genlen(str)
static int	print __PROTO__((int));
static int	isword __PROTO__((int));

#else
#   define gencpy(a,b)	strcpy((char*)(a),(char*)(b))
#   define genncpy(a,b,n)	strncpy((char*)(a),(char*)(b),n)
#   define genlen(str)	strlen(str)
#   define print(c)	isprint(c)
#   define isword(c)	isalnum(out[c])
#endif /*SHOPT_MULTIBYTE */

#define eol		editb.e_eol
#define cur		editb.e_cur
#define mark		editb.e_fchar
#define hline		editb.e_hline
#define hloff		editb.e_hloff
#define hismin		editb.e_hismin
#define usrkill		editb.e_kill
#define usreof		editb.e_eof
#define usrerase	editb.e_erase
#define crallowed	editb.e_crlf
#define Prompt		editb.e_prompt
#define plen		editb.e_plen
#define kstack		editb.e_killbuf
#define lstring		editb.e_search
#define lookahead	editb.e_index
#define env		editb.e_env
#define raw		editb.e_raw
#define histlines	editb.e_hismax
#define w_size		editb.e_wsize
#define drawbuff	editb.e_inbuf
#define killing		editb.e_mode
#define in_mult		editb.e_saved
#define LBUF	100
#define KILLCHAR	UKILL
#define ERASECHAR	UERASE
#define EOFCHAR		UEOF
#define DELETE		'\177'

/**********************
A large lookahead helps when the user is inserting
characters in the middle of the line.
************************/


static genchar *screen;		/* pointer to window buffer */
static genchar *cursor;		/* Cursor in real screen */
static enum
{
	CRT=0,	/* Crt terminal */
	PAPER	/* Paper terminal */
} terminal ;

typedef enum
{
	FIRST,		/* First time thru for logical line, prompt on screen */
	REFRESH,	/* Redraw entire screen */
	APPEND,		/* Append char before cursor to screen */
	UPDATE,		/* Update the screen as need be */
	FINAL		/* Update screen even if pending look ahead */
} Draw_t;

static void draw_mb __PROTO__((Draw_t, int));
static int count_chars __PROTO__((genchar*, genchar*));
static int real_offset __PROTO__((genchar*, int));
static int index_next_char __PROTO__((genchar*, int));
static int escape __PROTO__((genchar*, int));
static void putstring __PROTO__((char*));
static void search __PROTO__((genchar*,int));
static void setcursor __PROTO__((int, int));
static void show_info __PROTO__((const char*));
static void xcommands __PROTO__((int));

static int cr_ok;
static	Histloc_t location = { -5, 0 };

ed_emacsread __PARAM__((int fd,char *buff,int scend), (fd, buff, scend)) __OTORP__(int fd;char *buff;int scend;){
	register int c;
	register int i;
	register int nmb=1; /* number of byte being read */
	register genchar *out;
	register int count;
	int adjust,oadjust;
	char backslash;
	genchar *kptr;
static int CntrlO;
	char prompt[PRSIZE];
	genchar Screen[MAXWINDOW];
#if KSHELL && (2*CHARSIZE*MAXLINE)<IOBSIZE
	kstack = buff + MAXLINE*sizeof(genchar);
#else
	if(kstack==0)
	{
		kstack = (genchar*)malloc(sizeof(genchar)*(MAXLINE));
		kstack[0] = '\0';
	}
#endif
	Prompt = prompt;
	screen = Screen;
	drawbuff = out = (genchar*)buff;
	if(tty_raw(ERRIO,0) < 0)
	{
		 return(ed_read(fd,buff,scend));
	}
	raw = 1;
	/* This mess in case the read system call fails */
	
	ed_setup(fd);
#ifdef ESH_NFIRST
	if (location.hist_command == -5)		/* to be initialized */
	{
		kstack[0] = '\0';		/* also clear kstack... */
		location.hist_command = hline;
		location.hist_line = hloff;
	}
	if (location.hist_command <= hismin)	/* don't start below minimum */
	{
		location.hist_command = hismin + 1;
		location.hist_line = 0;
	}
	in_mult = hloff;			/* save pos in last command */
#endif /* ESH_NFIRST */
	i = sigsetjmp(env,0);
	if (i !=0)
	{
		tty_cooked(ERRIO);
		if (i == UEOF)
		{
			return(0); /* EOF */
		}
		return(-1); /* some other error */
	}
	*out = 0;
	if(scend+plen > (MAXLINE-2))
		scend = (MAXLINE-2)-plen;
	mark = eol = cur = 0;
	draw_mb(FIRST,1);
	adjust = -1;
	backslash = 0;
	if (CntrlO)
	{
#ifdef ESH_NFIRST
		ed_ungetchar(cntl('N'));
#else
		location = hist_locate(sh.hist_ptr,location.hist_command,location.hist_line,1);
		if (location.hist_command < histlines)
		{
			hline = location.hist_command;
			hloff = location.hist_line;
			hist_copy((char*)kstack,MAXLINE, hline,hloff);
#   ifdef SHOPT_MULTIBYTE
			ed_internal((char*)kstack,kstack);
#   endif /* SHOPT_MULTIBYTE */
			ed_ungetchar(cntl('Y'));
		}
#endif /* ESH_NFIRST */
	}
	CntrlO = 0;
	while ((c = ed_getchar(0)) != (-1))
	{
		/* Fix for bug 1201832. Basically backslash input is only
		** accepted if it is a single byte input. lookahead is > 1
		** if the input character is multi-byte - GT.
		*/
		if (backslash && !lookahead)
		{
			backslash = 0;
			if (c==usrerase||c==usrkill||(!print(c) &&
				(c!='\r'&&c!='\n')))
			{
				/* accept a backslashed character */
				cur--;
				out[cur++] = c;
				out[eol] = '\0';
				/* This has changed from APPEND to REFRESH 
				** to display the backslashed character.
				*/
				draw_mb(REFRESH, 1);
				continue;
			}
		}
		if (c == usrkill)
		{
			c = KILLCHAR ;
		}
		else if (c == usrerase)
		{
			c = ERASECHAR ;
		} 
		else if ((c == usreof)&&(eol == 0))
		{
			c = EOFCHAR;
		}
#ifdef ESH_KAPPEND
		if (--killing <= 0)	/* reset killing flag */
			killing = 0;
#endif
		oadjust = count = adjust;
		if(count<0)
			count = 1;
		adjust = -1;
		i = cur;
		switch(c)
		{
		case cntl('V'):
			show_info(&e_version[5]);
			continue;
		case '\0':
			mark = i;
			continue;
		case cntl('X'):
			xcommands(count);
			continue;
		case EOFCHAR:
			ed_flush();
			tty_cooked(ERRIO);
			return(0);
#ifdef u370
		case cntl('S') :
		case cntl('Q') :
			continue;
#endif	/* u370 */
		default:
		/* A loop is added here to read off all remaining of the 
		** multibyte character before it is displayed on screen.
		** This is needed to ensure character is broken at the correct
		** boundary when the line is longer than one line.
		*/
			do
			{
				if ((eol+1) >= (scend)) /*  will not fit on line */
				{
					ed_ungetchar(c); /* save character for next line */
					goto process;
				}
				for(i= ++eol; i>cur; i--)
					out[i] = out[i-1];
				backslash =  (c == '\\');
				out[cur++] = c;
			} while (lookahead && ((c=ed_getchar(0)) != -1) && nmb++);
			draw_mb(APPEND, nmb);
			nmb=1;	/* reset nmb */
			continue;
		case cntl('Y') :
			{
				c = genlen(kstack);
				if ((c + eol) > scend)
				{
					beep();
					continue;
				}
				mark = i;
				for(i=eol;i>=cur;i--)
					out[c+i] = out[i];
				kptr=kstack;
				while (i = *kptr++)
					out[cur++] = i;
				draw_mb(UPDATE, 1);
				eol = genlen(out);
				continue;
			}
		case '\n':
		case '\r':
			c = '\n';
			goto process;

		case DELETE:	/* delete char 0x7f */
		case '\b':	/* backspace, ^h */
		case ERASECHAR :
			if (count > i)
				count = i;
#ifdef ESH_KAPPEND
			kptr = &kstack[count];	/* move old contents here */
			if (killing)		/* prepend to killbuf */
			{
				c = genlen(kstack) + CHARSIZE; /* include '\0' */
				while(c--)	/* copy stuff */
					kptr[c] = kstack[c];
			}
			else
				*kptr = 0;	/* this is end of data */
			killing = 2;		/* we are killing */
			i -= count;
			eol -= count;
			genncpy(kstack,out+i,cur-i);
#else
			while ((count--)&&(i>0))
			{
				i--;
				eol--;
			}
			genncpy(kstack,out+i,cur-i);
			kstack[cur-i] = 0;
#endif /* ESH_KAPPEND */
			gencpy(out+i,out+cur);
			mark = i;
			goto update;
		case cntl('W') :
#ifdef ESH_KAPPEND
			++killing;		/* keep killing flag */
#endif
			if (mark > eol )
				mark = eol;
			if (mark == i)
				continue;
			if (mark > i)
			{
				adjust = mark - i;
				ed_ungetchar(cntl('D'));
				continue;
			}
			adjust = i - mark;
			ed_ungetchar(usrerase);
			continue;
		case cntl('D') :
			mark = i;
#ifdef ESH_KAPPEND
			if (killing)
				kptr = &kstack[genlen(kstack)];	/* append here */
			else
				kptr = kstack;
			killing = 2;			/* we are now killing */
#else
			kptr = kstack;
#endif /* ESH_KAPPEND */
			while ((count--)&&(eol>0)&&(i<eol))
			{
				*kptr++ = out[i];
				eol--;
				while(1)
				{
					if ((out[i] = out[(i+1)])==0)
						break;
					i++;
				}
				i = cur;
			}
			*kptr = '\0';
			goto update;
		case cntl('C') :
		case cntl('F') :
		{
			int cntlC = (c==cntl('C'));
			while (count-- && eol>i)
			{
				if (cntlC)
				{
					c = out[i];
#ifdef SHOPT_MULTIBYTE
					if((c&~STRIP)==0 && islower(c))
#else
					if(islower(c))
#endif /* SHOPT_MULTIBYTE */
					{
						c += 'A' - 'a';
						out[i] = c;
					}
				}
				i++;
			}
			goto update;
		}
		case cntl(']') :
			c = ed_getchar(1);
			if ((count == 0) || (count > eol))
                        {
                                beep();
                                continue;
                        }
			if (out[i])
				i++;
			while (i < eol)
			{
				if (out[i] == c && --count==0)
					goto update;
				i++;
			}
			i = 0;
			while (i < cur)
			{
				if (out[i] == c && --count==0)
					break;
				i++;
			};

update:
			cur = i;
			draw_mb(UPDATE, 1);
			continue;

		case cntl('B') :
			if (count > i)
				count = i;
			i -= count;
			goto update;
		case cntl('T') :
			if ((sh_isoption(SH_EMACS))&& (eol!=i))
				i++;
			if (i >= 2)
			{
				c = out[i - 1];
				out[i-1] = out[i-2];
				out[i-2] = c;
			}
			else
			{
				if(sh_isoption(SH_EMACS))
					i--;
				beep();
				continue;
			}
			goto update;
		case cntl('A') :
			i = 0;
			goto update;
		case cntl('E') :
			i = eol;
			goto update;
		case cntl('U') :
			adjust = 4*count;
			continue;
		case KILLCHAR :
			cur = 0;
			oadjust = -1;
		case cntl('K') :
			if(oadjust >= 0)
			{
#ifdef ESH_KAPPEND
				killing = 2;		/* set killing signal */
#endif
				mark = count;
				ed_ungetchar(cntl('W'));
				continue;
			}
			i = cur;
			eol = i;
			mark = i;
#ifdef ESH_KAPPEND
			if (killing)			/* append to kill buffer */
				gencpy(&kstack[genlen(kstack)], &out[i]);
			else
				gencpy(kstack,&out[i]);
			killing = 2;			/* set killing signal */
#else
			gencpy(kstack,&out[i]);
#endif /* ESH_KAPPEND */
			out[i] = 0;
			draw_mb(UPDATE, 1);
			if (c == KILLCHAR)
			{
				if (terminal == PAPER)
				{
					putchar('\n');
					putstring(Prompt);
				}
				c = ed_getchar(0);
				if (c != usrkill)
				{
					ed_ungetchar(c);
					continue;
				}
				if (terminal == PAPER)
					terminal = CRT;
				else
				{
					terminal = PAPER;
					putchar('\n');
					putstring(Prompt);
				}
			}
			continue;
		case cntl('L'):
			ed_crlf();
			draw_mb(REFRESH, 1);
			continue;
		case cntl('[') :
			adjust = escape(out,oadjust);
			continue;
		case cntl('R') :
			search(out,count);
			goto drawline;
		case cntl('P') :
                        if (count <= hloff)
                                hloff -= count;
                        else
                        {
                                hline -= count - hloff;
                                hloff = 0;
                        }
#ifdef ESH_NFIRST
			if (hline <= hismin)
#else
			if (hline < hismin)
#endif /* ESH_NFIRST */
			{
				hline = hismin+1;
				beep();
#ifndef ESH_NFIRST
				continue;
#endif
			}
			goto common;

		case cntl('O') :
			location.hist_command = hline;
			location.hist_line = hloff;
			CntrlO = 1;
			c = '\n';
			goto process;
		case cntl('N') :
#ifdef ESH_NFIRST
			hline = location.hist_command;	/* start at saved position */
			hloff = location.hist_line;
#endif /* ESH_NFIRST */
			location = hist_locate(sh.hist_ptr,hline,hloff,count);
			if (location.hist_command > histlines)
			{
				beep();
#ifdef ESH_NFIRST
				location.hist_command = histlines;
				location.hist_line = in_mult;
#else
				continue;
#endif /* ESH_NFIRST */
			}
			hline = location.hist_command;
			hloff = location.hist_line;
		common:
#ifdef ESH_NFIRST
			location.hist_command = hline;	/* save current position */
			location.hist_line = hloff;
#endif
			hist_copy((char*)out,MAXLINE, hline,hloff);
#ifdef SHOPT_MULTIBYTE
			ed_internal((char*)(out),out);
#endif /* SHOPT_MULTIBYTE */
		drawline:
			eol = genlen(out);
			cur = eol;
			draw_mb(UPDATE, 1);
			continue;
		}
		
	}
	
process:

	if (c == (-1))
	{
		lookahead = 0;
		beep();
		*out = '\0';
	}
	draw_mb(FINAL, 1);
	tty_cooked(ERRIO);
	if(c == '\n')
	{
		out[eol++] = '\n';
		out[eol] = '\0';
		ed_crlf();
	}
#ifdef SHOPT_MULTIBYTE
	ed_external(out,buff);
#endif /* SHOPT_MULTIBYTE */
	i = strlen(buff);
	if (i)
		return(i);
	return(-1);
}

static void show_info __PARAM__((const char *str), (str)) __OTORP__(const char *str;){
	register genchar *out = drawbuff;
	register int c;
	genchar string[LBUF];
	int sav_cur = cur;
	/* save current line */
	genncpy(string,out,sizeof(string)/CHARSIZE-1);
	*out = 0;
	cur = 0;
#ifdef SHOPT_MULTIBYTE
	ed_internal(str,out);
#else
	gencpy(out,str);
#endif	/* SHOPT_MULTIBYTE */
	draw_mb(UPDATE, 1);
	c = ed_getchar(0);
	if(c!=' ')
		ed_ungetchar(c);
	/* restore line */
	cur = sav_cur;
	genncpy(out,string,sizeof(string)/CHARSIZE-1);
	draw_mb(UPDATE,1);
}

static void putstring __PARAM__((register char *sp), (sp)) __OTORP__(register char *sp;){
	register int c;
	while (c= *sp++)
		 putchar(c);
}


static int escape __PARAM__((register genchar *out,int count), (out, count)) __OTORP__(register genchar *out;int count;){
	register int i,value;
	int digit,ch;
	digit = 0;
	value = 0;
	while ((i=ed_getchar(0)),isdigit(i))
	{
		value *= 10;
		value += (i - '0');
		digit = 1;
	}
	if (digit)
	{
		ed_ungetchar(i) ;
#ifdef ESH_KAPPEND
		++killing;		/* don't modify killing signal */
#endif
		return(value);
	}
	value = count;
	if(value<0)
		value = 1;
	switch(ch=i)
	{
		case ' ':
			mark = cur;
			return(-1);

#ifdef ESH_KAPPEND
		case '+':		/* M-+ = append next kill */
			killing = 2;
			return -1;	/* no argument for next command */
#endif

		case 'p':	/* M-p == ^W^Y (copy stack == kill & yank) */
			ed_ungetchar(cntl('Y'));
			ed_ungetchar(cntl('W'));
#ifdef ESH_KAPPEND
			killing = 0;	/* start fresh */
#endif
			return(-1);

		case 'l':	/* M-l == lower-case */
		case 'd':
		case 'c':
		case 'f':
		{
			i = cur;
			while(value-- && i<eol)
			{
				while ((out[i])&&(!isword(i)))
					i++;
				while ((out[i])&&(isword(i)))
					i++;
			}
			if(ch=='l')
			{
				value = i-cur;
				while (value-- > 0)
				{
					i = out[cur];
#ifdef SHOPT_MULTIBYTE
					if((i&~STRIP)==0 && isupper(i))
#else
					if(isupper(i))
#endif /* SHOPT_MULTIBYTE */
					{
						i += 'a' - 'A';
						out[cur] = i;
					}
					cur++;
				}
				draw_mb(UPDATE, 1);
				return(-1);
			}

			else if(ch=='f')
				goto update;
			else if(ch=='c')
			{
				ed_ungetchar(cntl('C'));
				return(i-cur);
			}
			else
			{
				if (i-cur)
				{
					ed_ungetchar(cntl('D'));
#ifdef ESH_KAPPEND
					++killing;	/* keep killing signal */
#endif
					return(i-cur);
				}
				beep();
				return(-1);
			}
		}
		
		
		case 'b':
		case DELETE :
		case '\b':
		case 'h':
		{
			i = cur;
			while(value-- && i>0)
			{
				i--;
				while ((i>0)&&(!isword(i)))
					i--;
				while ((i>0)&&(isword(i-1)))
					i--;
			}
			if(ch=='b')
				goto update;
			else
			{
				ed_ungetchar(usrerase);
#ifdef ESH_KAPPEND
				++killing;
#endif
				return(cur-i);
			}
		}
		
		case '>':
			ed_ungetchar(cntl('N'));
#ifdef ESH_NFIRST
			if (in_mult)
			{
				location.hist_command = histlines;
				location.hist_line = in_mult - 1;
			}
			else
			{
				location.hist_command = histlines - 1;
				location.hist_line = 0;
			}
#else
			hline = histlines-1;
			hloff = 0;
#endif /* ESH_NFIRST */
			return(0);
		
		case '<':
			ed_ungetchar(cntl('P'));
			hloff = 0;
#ifdef ESH_NFIRST
			hline = hismin + 1;
			return 0;
#else
			return(hline-hismin);
#endif /* ESH_NFIRST */


		case '#':
			ed_ungetchar('\n');
			ed_ungetchar((out[0]=='#')?cntl('D'):'#');
			ed_ungetchar(cntl('A'));
			return(-1);
		case '_' :
		case '.' :
		{
			genchar name[MAXLINE];
			char buf[MAXLINE];
			char *ptr;
			ptr = hist_word(buf,MAXLINE,(count?count:-1));
#ifndef KSHELL
			if(ptr==0)
			{
				beep();
				break;
			}
#endif	/* KSHELL */
			if ((eol - cur) >= sizeof(name))
			{
				beep();
				return(-1);
			}
			mark = cur;
			gencpy(name,&out[cur]);
			while(*ptr)
			{
				out[cur++] = *ptr++;
				eol++;
			}
			gencpy(&out[cur],name);
			draw_mb(UPDATE, 1);
			return(-1);
		}
#ifdef KSHELL

		/* file name expansion */
		case cntl('[') :	/* filename completion */
			i = '\\';
		case '*':		/* filename expansion */
		case '=':	/* escape = - list all matching file names */
			mark = cur;
			if(ed_expand((char*)out,&cur,&eol,i) < 0)
				beep();
			else if(i=='=')
				draw_mb(REFRESH, 1);
			else
				draw_mb(UPDATE, 1);
			return(-1);

		/* search back for character */
		case cntl(']'):	/* feature not in book */
		{
			int c = ed_getchar(1);
			if ((value == 0) || (value > eol))
			{
				beep();
				return(-1);
			}
			i = cur;
			if (i > 0)
				i--;
			while (i >= 0)
			{
				if (out[i] == c && --value==0)
					goto update;
				i--;
			}
			i = eol;
			while (i > cur)
			{
				if (out[i] == c && --value==0)
					break;
				i--;
			};

		}
		update:
			cur = i;
			draw_mb(UPDATE, 1);
			return(-1);

		case '[':	/* feature not in book */
			i = '_';

		default:
			/* look for user defined macro definitions */
			if(ed_macro(i))
#   ifdef ESH_BETTER
				return(count);	/* pass argument to macro */
#   else
				return(-1);
#   endif /* ESH_BETTER */
#else
		update:
			cur = i;
			draw_mb(UPDATE,1);
			return(-1);

		default:
#endif	/* KSHELL */
		beep();
		return(-1);
	}
}


/*
 * This routine process all commands starting with ^X
 */

static void xcommands __PARAM__((int count), (count)) __OTORP__(int count;){
        register int i = ed_getchar(0);
	NOT_USED(count);
        switch(i)
        {
                case cntl('X'):	/* exchange dot and mark */
                        if (mark > eol)
                                mark = eol;
                        i = mark;
                        mark = cur;
                        cur = i;
                        draw_mb(UPDATE,1);
                        return;

#ifdef KSHELL
#   ifdef ESH_BETTER
                case cntl('E'):	/* invoke emacs on current command */
			if(ed_fulledit()==-1)
				beep();
			else
				ed_ungetchar('\n');
			return;

#	define itos(i)	fmtbase((long)(i),0,0)/* want signed conversion */

		case cntl('H'):		/* ^X^H show history info */
			{
				char hbuf[MAXLINE];

				strcpy(hbuf, "Current command ");
				strcat(hbuf, itos(hline));
				if (hloff)
				{
					strcat(hbuf, " (line ");
					strcat(hbuf, itos(hloff+1));
					strcat(hbuf, ")");
				}
				if ((hline != location.hist_command) ||
				    (hloff != location.hist_line))
				{
					strcat(hbuf, "; Previous command ");
					strcat(hbuf, itos(location.hist_command));
					if (location.hist_line)
					{
						strcat(hbuf, " (line ");
						strcat(hbuf, itos(location.hist_line+1));
						strcat(hbuf, ")");
					}
				}
				show_info(hbuf);
				return;
			}
#	if 1	/* debugging, modify as required */
		case cntl('D'):		/* ^X^D show debugging info */
			{
				char debugbuf[MAXLINE];

				strcpy(debugbuf, "count=");
				strcat(debugbuf, itos(count));
				strcat(debugbuf, " eol=");
				strcat(debugbuf, itos(eol));
				strcat(debugbuf, " cur=");
				strcat(debugbuf, itos(cur));
				strcat(debugbuf, " crallowed=");
				strcat(debugbuf, itos(crallowed));
				strcat(debugbuf, " plen=");
				strcat(debugbuf, itos(plen));
				strcat(debugbuf, " w_size=");
				strcat(debugbuf, itos(w_size));

				show_info(debugbuf);
				return;
			}
#	endif /* debugging code */
#   endif /* ESH_BETTER */
#endif /* KSHELL */

                default:
                        beep();
                        return;
	}
}

static void search __PARAM__((genchar *out,int direction), (out, direction)) __OTORP__(genchar *out;int direction;){
	static int prevdirection =  1 ;
#ifndef ESH_NFIRST
	Histloc_t location;
#endif
	register int i,sl;
	genchar str_buff[LBUF];
	register genchar *string = drawbuff;
	/* save current line */
	char sav_cur = cur;
	genncpy(str_buff,string,sizeof(str_buff)/CHARSIZE-1);
	string[0] = '^';
	string[1] = 'R';
	string[2] = '\0';
	sl = 2;
	cur = sl;
	draw_mb(UPDATE, 1);
	while ((i = ed_getchar(1))&&(i != '\r')&&(i != '\n'))
	{
		if (i==usrerase)
		{
			if (sl > 2)
			{
				string[--sl] = '\0';
				cur = sl;
				draw_mb(UPDATE,1);
			}
			else
				beep();
			continue;
		}
		if (i==usrkill)
		{
			beep();
			goto restore;
		}
		if (i == '\\')
		{
			string[sl++] = '\\';
			string[sl] = '\0';
			cur = sl;
			draw_mb(APPEND, 1);
			i = ed_getchar(1);
			string[--sl] = '\0';
		}
		string[sl++] = i;
		string[sl] = '\0';
		cur = sl;
		draw_mb(APPEND, 1);
	}
	i = genlen(string);
	
	if (direction < 1)
	{
		prevdirection = -prevdirection;
		direction = 1;
	}
	else
		direction = -1;
	if (i != 2)
	{
#ifdef SHOPT_MULTIBYTE
		ed_external(string,(char*)string);
#endif /* SHOPT_MULTIBYTE */
		strncpy(lstring,((char*)string)+2,SEARCHSIZE);
		prevdirection = direction;
	}
	else
		direction = prevdirection ;
	location = hist_find(sh.hist_ptr,(char*)lstring,hline,1,direction);
	i = location.hist_command;
	if(i>0)
	{
		hline = i;
#ifdef ESH_NFIRST
		hloff = location.hist_line = 0;	/* display first line of multi line command */
#else
		hloff = location.hist_line;
#endif /* ESH_NFIRST */
		hist_copy((char*)out,MAXLINE, hline,hloff);
#ifdef SHOPT_MULTIBYTE
		ed_internal((char*)out,out);
#endif /* SHOPT_MULTIBYTE */
		return;
	}
	if (i < 0)
	{
		beep();
#ifdef ESH_NFIRST
		location.hist_command = hline;
		location.hist_line = hloff;
#else
		hloff = 0;
		hline = histlines;
#endif /* ESH_NFIRST */
	}
restore:
	genncpy(string,str_buff,sizeof(str_buff)/CHARSIZE-1);
	cur = sav_cur;
	return;
}


/* This function takes two pointers and returns the number of character
** positions enclosed within the array of these two pointers. 
*/

static int count_chars __PARAM__((genchar *start, genchar *current), (start, current))  __OTORP__(genchar *start; genchar *current;)
{
int count=0, d;
genchar *begin=start;
genchar c;

  /* current is pointed to the start of the array, so return 0 */
  if ( begin == current )
  {
    return(count);
  }

  /* Otherwise, start from beginning and works towards current */
  while ( begin < current)
  {
    c = *begin++;

    /* Find out how many character width does the character, c requires */
    d = out_csize(icharset(c));
    if ( d == 1) count++ ; /* if it's single width */
    else
    {
      count += d;	/* increment the character width */
      while ((d-1) > 0) /* As we have already scanned one genchar */
      {
        if (*begin++ != MARKER) /* only decrement char if it is not Marker */
	  d--;
      }
      if (*begin == MARKER) begin++;
    }
  }
  return(count);
}

/* This function is similar to that of count_chars except that it takes a
** logical offset and returns the real offset in the array by skipping all
** the position taken up by the MARKER.
*/
static int real_offset(genchar *start, int l_offset)
{
genchar *loc= start;
int c;
	while(l_offset-- > 0)
	{
		c = *loc++;
		if (c == MARKER) l_offset++;	
	} 
	return(loc - start);
}

/* This function takes pointer to array of genchar and a current index to that 
** array. It then indexes to the start of the next character in the array and 
** returns the new index.
*/
static int index_next_char(genchar *nscreen, int idx)
{
int d, inc=0;
genchar *curp=nscreen+idx;
genchar c;

	c = *curp;

	/* Find out the out put character width */
	d = out_csize(icharset(c));

     	if ( d == 1)  /* It is a single width character */
	  inc = 1;
	else 
	{
	  /* Go through the next section of the array until d-1 < 0 */
	  while (1)
	  {
  	    if ( (d-1) < 0 ) break;
	    /* If the *curp is part of the multi-byte char then decrement d */
	    if (*curp++ != MARKER) d--;

	    /* Increment count for every genchar in the array */
	    inc++;
	  }
	  /* If the next genchar is a MARKER after exiting the while loop.
  	     count this as well */
	  if (*curp == MARKER)
 	    inc++;
	}
	return(idx + inc);
}

/* Adjust screen to agree with inputs: logical line and cursor */
/* If 'first' assume screen is blank */
/* Prompt is always kept on the screen */
/* The paramter nmb is the number of byte that the user has just typed in. */
/* This number is > 1 when a multi-byte character is entered or when a string 
** is cut and paste.
*/

static void draw_mb __PARAM__((Draw_t option, int nmb), (option, nmb)) __OTORP__(Draw_t option; int nmb; ){
#define	NORMAL ' '
#define	LOWER  '<'
#define	BOTH   '*'
#define	UPPER  '>'

	static char overflow;		/* Screen overflow flag set */
	register genchar *sptr;		/* Pointer within screen */
	
	static int offset=0;		/* Screen offset */
	static int ptr_offset=0;	/* offset for pointer arithmethics */
	static char scvalid;		/* Screen is up to date */
	
	genchar nscreen[2*MAXLINE];	/* New entire screen */
	genchar *ncursor;		/* New cursor */
	register genchar *nptr;		/* Pointer to New screen */
	char  longline;			/* Line overflow */
	genchar *logcursor;
	genchar *nscend;		/* end of logical screen */
	register int i, j, t;
	register int midscr;		/* Midddle of display screen */

	
        memset(nscreen, 0, 2*MAXLINE*CHARSIZE); /* Initialise array */
	nptr = nscreen;
	sptr = drawbuff;
	logcursor = sptr + cur;
	longline = NORMAL;
	
	if (option == FIRST || option == REFRESH)
	{
		overflow = NORMAL;
		cursor = screen;
		ptr_offset = offset = 0;
		cr_ok = crallowed;
		if (option == FIRST)
		{
			scvalid = 1;
			return;
		}
		*cursor = '\0';
		putstring(Prompt);	/* start with prompt */
	}
	
	/*********************
	 Do not update screen if pending characters
	**********************/
	
	if ((lookahead)&&(option != FINAL))
	{
		scvalid = 0; /* Screen is out of date, APPEND will not work */
		return;
	}
	
	/***************************************
	If in append mode, cursor at end of line, screen up to date,
	offset is 0, 
	and the window has room for 'nmb'  of character.
	Then output the character and adjust the screen only.
	*****************************************/
	

	i = *(logcursor-nmb);	/* last multibyte character inserted */
	

	if ((option == APPEND)&&(scvalid)&&(*logcursor == '\0')&&
	    (!offset)&& ( (cursor-screen+nmb) <(w_size-1)))
	{
		for (j=nmb; j>0; j--)
		{
		i = *(logcursor-j);
		putchar(i);
		*cursor++ = i;
		*cursor = '\0';
		}
		return;
	}

	/* copy the line */
	ncursor = nptr + ed_virt_to_phys(sptr,nptr,cur,0,0);
	nptr += genlen(nptr);
	sptr += genlen(sptr);
	nscend = nptr - 1;
	if(sptr == logcursor)
		ncursor = nptr;
	
	/*********************
	 Does ncursor appear on the screen?
	 If not, adjust the screen offset so it does.
	**********************/
	
	i = count_chars(nscreen, ncursor);
	
	if ((offset && i<=offset)||(i >= (offset+w_size)))
	{
		/* Center the cursor on the screen */
		/* Work out the centre of screen based on w_size */
		if ((w_size>>1)%2) midscr = (w_size>>1)-1;
		else midscr = w_size>>1;

		offset  = i - midscr;
		if (--offset < 0)
		{
			ptr_offset = offset = 0;
		}
		else
		{
			/* Add 1 to offset due to previous decrment */
			ptr_offset = real_offset(nscreen, ++offset);
		       
			/* The ptr_offset may be pointed to the middle
			** of a multibye character, so step through it
			** to make sure that the line is broken at the
			** start of the character boundary.
			*/
			t = 0;
			while(t < ptr_offset)
				t = index_next_char(nscreen, t);
			if (t !=  ptr_offset) 
			{
				ptr_offset = t; 
			}
		}
	}
			
	/*********************
	 Is the range of screen[0] thru screen[w_size] up-to-date
	 with nscreen[offset] thru nscreen[offset+w_size] ?
	 (ptr_offset is actual index for offset in nscreen.)
	 If not, update as need be.
	***********************/
	
	nptr = &nscreen[ptr_offset];
	sptr = screen;
	
	i = real_offset(nptr, w_size);
	
	while (i-- > 0)
	{
		
		if (*nptr == '\0')
		{
			*(nptr + 1) = '\0';
			*nptr = ' ';
		}
		if (*sptr == '\0')
		{
			*(sptr + 1) = '\0';
			*sptr = ' ';
		}
		if (*nptr == *sptr)
		{
			nptr++;
			sptr++;
			continue;
		}
		setcursor(sptr-screen,*nptr); 
		*sptr++ = *nptr++;
#ifdef SHOPT_MULTIBYTE
		while(*nptr==MARKER)
		{
			if(*sptr=='\0')
				*(sptr + 1) = '\0';
			*sptr++ = *nptr++;
			i--;
			cursor++;
		}
#endif /* SHOPT_MULTIBYTE */
	}
	
	/******************
	
	Screen overflow checks 
	Have to find the index in nscreen array using the real_offset function.
	
	********************/
	j = real_offset(nscreen, offset+w_size);
	
	if (nscend >= &nscreen[j])
	{
		if (offset > 0)
			longline = BOTH;
		else
			longline = UPPER;
	}
	else
	{
		if (offset > 0)
			longline = LOWER;
	}
	
	/* Update screen overflow indicator if need be */
	
	if (longline != overflow)
	{
		j = real_offset(screen, w_size);
		setcursor(j,longline);
		overflow = longline;
	}
	i = (ncursor-nscreen) - ptr_offset; 
	setcursor(i,0);
	scvalid = 1;
	return;
}


/*
 * put the cursor to the <newp> position within screen buffer
 * if <c> is non-zero then output this character
 * cursor is set to reflect the change
 */

static void setcursor __PARAM__((register int newp,int c), (newp, c)) __OTORP__(register int newp;int c;){
	register int oldp = cursor - screen;
	if (oldp > newp)
	{
		if (!cr_ok || (2*(newp+plen)>(oldp+plen)))
		{
			while (oldp > newp)
			{
				putchar('\b');
				oldp--;
			}
			goto skip;
		}
		putstring(Prompt);
		oldp = 0;
	}
	while (newp > oldp)
		putchar(screen[oldp++]);
skip:
	if(c)
	{
		putchar(c);
		newp++;
	}
	cursor = screen+newp;
	return;
}

#ifdef SHOPT_MULTIBYTE
static int print __PARAM__((register int c), (c)) __OTORP__(register int c;){
	return((c&~STRIP)==0 && isprint(c));
}

static int isword __PARAM__((register int i), (i)) __OTORP__(register int i;){
	register int c = drawbuff[i];
	return((c&~STRIP) || isalnum(c));
}
#endif /* SHOPT_MULTIBYTE */