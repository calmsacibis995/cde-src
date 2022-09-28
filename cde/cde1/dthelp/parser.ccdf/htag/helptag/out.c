/* $XConsortium: out.c /main/cde1_maint/1 1995/07/17 22:38:34 drk $ */
/*   Copyright (c) 1986, 1987, 1988, 1989 Hewlett-Packard Co. */
/* Functions for HPTag/TeX translator relevant to output of heads and
   data characters */

#include "userinc.h"
#include "globdec.h"


/* Echo part of a head to the screen to indicate how much of the document
   has been processed */
void echohead(p)
M_WCHAR *p;
{
char *mb_p,*mb_string;

mb_string = MakeMByteString(p);

#ifdef AIX_ILS
/* This is a bug fix for AIX. Using putc() for EUC Gaiji characters on aixterm
 * has a problem.
 */
fprintf(stderr, "%s", mb_string);
#else /* AIX_ILS */
for (mb_p = mb_string ; *mb_p ; mb_p++)
    {
    putc(*mb_p, stderr);
    }
#endif /* AIX_ILS */

m_free(mb_string,"multi-byte string");
}

/* call echohead with a literal string */
void mb_echohead(p)
char *p;
{
M_WCHAR *wc;

wc = MakeWideCharString(p);
echohead(wc);
m_free(wc,"wide character string");
}
  

/* Called at end of a head in a chapter, appendix, section, or
   reference section */
void endhead(M_NOPAR)
{
if (savid)
    {
    if (w_strlen(xrefstring) + 1 > (sizeof(xrefstring) / sizeof(M_WCHAR)))
	{
	m_error("ID expansion too long");
	m_exit(TRUE);
	}

    mb_shchar(M_EOS,
	      &xstrlen,
	      (sizeof(xrefstring) / sizeof(M_WCHAR)),
	      xrefstring,
	      idstring,
	      "Too many characters in corresponding cross-reference",
	      &iderr);
    setid(savid,
	  TRUE,
	  TRUE,
	  inchapter,
	  chapstring,
	  xrffile,
	  xrfline,
	  TRUE);
    }

savehead[svheadlen] = M_EOS;
echo = savhd = FALSE;
}

/* Process a character in an environment where spaces must be escaped */
#if defined(M_PROTO)
void esoutchar(M_WCHAR textchar)
#else
void esoutchar(textchar)
M_WCHAR textchar;
#endif
{
static M_WCHAR wsp = 0;

if (!wsp)
    {
    char space[2];

    space[0] = M_SPACE;
    space[1] = 0;
    mbtowc(&wsp, space, 1);
    }

if (textchar == wsp)
    {
    mb_strcode("\\", outfile);
    }
outchar(textchar, outfile);
}

/* Process a character in an example or an image paragraph */
#if defined(M_PROTO)
void exoutchar(M_WCHAR textchar)
#else
void exoutchar(textchar)
M_WCHAR textchar;
#endif
{
static M_WCHAR wsp = 0, wre;

if (!wsp)
    {
    char string[2];

    string[1] = 0;
    string[0] = M_SPACE;
    mbtowc(&wsp, string, 1);
    string[0] = M_RE;
    mbtowc(&wre, string, 1);
    }

if (textchar == wsp) fputs("\\ ", outfile);
else if (textchar == wre)
    {
    fputs("\n", outfile);
    exlineno++;
    if (tonumexlines) fprintf(outfile, " %d:  ", exlineno);
    }
else outchar(textchar, outfile);
}

/* Write a character to be passed to the index file */
#if defined(M_PROTO)
void indexchar(M_WCHAR textchar)
#else
void indexchar(textchar)
M_WCHAR textchar;
#endif
{
if (idxsavlen + 4 > (sizeof(idxsav) / sizeof(M_WCHAR)))
    {
    m_error("Internal error. Exceeded save buffer for index");
    m_exit(TRUE);
    }
idxsav[idxsavlen++] = textchar;
}


/* Processes output for call out text */
#if defined(M_PROTO)
void outcall(M_WCHAR textchar, FILE *outfile)
#else
void outcall(textchar, outfile)
M_WCHAR textchar;
FILE *outfile;
#endif
{
if (textchar != M_RE)
    outchar(textchar, outfile);
}

/* Processes a data character */
#if defined(M_PROTO)
void outchar(M_WCHAR textchar, FILE *outfile)
#else
void outchar(textchar, outfile)
M_WCHAR textchar;
FILE *outfile;
#endif
{
M_WCHAR buffer[2];
static M_WCHAR wnl = 0;
char mb_textchar[32]; /* arbitrarily large */
unsigned char index;
int length;

if (!wnl)
    mbtowc(&wnl, "\n", 1);

if (textchar == wnl)
    {
    if (multi_cr_flag)
	{
	return;
	}
    multi_cr_flag = TRUE;
    }
else
    {
    multi_cr_flag = FALSE;
    }

if (echo)
    {
    buffer[0] = textchar;
    buffer[1] = M_EOS;
    echohead(buffer);
    }

cur_char = textchar;

length = wctomb(mb_textchar, textchar);

index = (unsigned char) mb_textchar[0];
if ((length == 1) && special[index])
    mb_strcode(special[index], outfile);
else
    realoutchar(textchar, outfile);
} /* end procedure outchar() */


/* Called for processing instruction */
void outpi(enttype, pi, entname)
int enttype;
M_WCHAR *pi;
M_WCHAR *entname;
{
if (enttype == M_SDATA)
    {
    rshnewclear();
    }

strcode(pi, outfile);

if (echo)
    {
    if (entname)
	{
	mb_echohead(m_ero);
	echohead(entname);
	mb_echohead(m_refc);
	}
    else
	{
	mb_echohead(m_pio);
	echohead(pi);
	mb_echohead(m_pic);
	}
    }
}

/* Called for normal paragraph spacing */
void parspace(M_NOPAR)
{
if (unleaded)  /* unleaded is a one-shot flag */
    unleaded = FALSE;
}

/* Really output a character */
#if defined(M_PROTO)
void realoutchar(M_WCHAR textchar, FILE *outfile)
#else
void realoutchar(textchar, outfile)
M_WCHAR textchar;
FILE *outfile;
#endif
{
rshnewclear();
if (! abbrev || fabbrev)
    {
    char mb_textchar[32]; /* arbitrarily large */
    int length, i;

    length = wctomb(mb_textchar, textchar);
    if (tooutput)
	for (i = 0; i < length; i++)
	    putc(mb_textchar[i], outfile);

    if (savid)
	shchar(textchar,
	       &xstrlen, 
	       (sizeof(xrefstring) / sizeof(M_WCHAR)),
	       xrefstring,
	       idstring,
	       "Too many characters in corresponding cross-reference",
	       &iderr);

    if (savhd)
	shchar(textchar,
	       &svheadlen,
	       (sizeof(savehead) / sizeof(M_WCHAR)),
	       savehead,
	       svhdstring,
	       "Too many characters in head or caption",
	       &hderr);

    if (savtc)
	shchar(textchar,
	       &svtclen,
	       (sizeof(savetabcap) / sizeof(M_WCHAR)),
	       savetabcap,
	       svtcstring,
	       "Too many characters in table caption",
	       &hderr);
    }
}

/* Save a character in the array used to store table of contents entries
   when processing a head */
#if defined(M_PROTO)
void shchar(M_WCHAR textchar, int *len, int max, M_WCHAR *string, void (*proc)(M_WCHAR *string), char *msg, LOGICAL *errflg)
#else
void shchar(textchar, len, max, string, proc, msg, errflg)
M_WCHAR textchar;
int *len;
int max;
M_WCHAR *string;
void (*proc)(
#if defined(M_PROTO)
             M_WCHAR *string
#endif
    );
char *msg;
LOGICAL *errflg;
#endif
{
char mb_textchar[32]; /* arbitrarily large */
unsigned char index;
int  length;

length = wctomb(mb_textchar, textchar);

index = (unsigned char) mb_textchar[0];
if ((length == 1) && special[index])
    {
    M_WCHAR *wc_special;

    wc_special = MakeWideCharString(special[index]);
    (*proc)(wc_special);
    m_free(wc_special,"wide character string");
    }
else
    {
    if (*len + 1 + 1 > max)
	{
	if (! *errflg)
	    {
	    m_error(msg);
	    *errflg = TRUE;
	    }
	}
    else
	string[(*len)++] = textchar;
    }
}


#if defined(M_PROTO)
void mb_shchar(char textchar,
	       int *len,
	       int max,
	       M_WCHAR *string,
	       void (*proc)(M_WCHAR *string),
	       char *msg, LOGICAL *errflg)
#else
void mb_shchar(textchar, len, max, string, proc, msg, errflg)
char textchar;
int *len;
int max;
M_WCHAR *string;
void (*proc)(
#if defined(M_PROTO)
    M_WCHAR *string
#endif
    );
char *msg;
LOGICAL *errflg;
#endif
{
M_WCHAR wc_textchar;

mbtowc(&wc_textchar, &textchar, 1);

shchar(wc_textchar, len, max, string, proc, msg, errflg);
}


/* Save a string in the array used to store table of contents entries
   when processing a head */
void shstring(addstring, len, max, storestring, msg, errflg)
M_WCHAR *addstring;
int *len;
int max;
M_WCHAR *storestring;
char *msg;
LOGICAL *errflg;
{
int addlength;

addlength = w_strlen(addstring);
if (*len + addlength + 1 > max)
    {
    if (! *errflg)
	{
	m_error(msg);
	*errflg = TRUE;
	}
    }
else
    {
    w_strcpy(&storestring[*len], addstring);
    *len += addlength;
    }
}


void mb_shstring(addstring, len, max, storestring, msg, errflg)
char *addstring;
int *len;
int max;
M_WCHAR *storestring;
char *msg;
LOGICAL *errflg;
{
M_WCHAR *wc_addstring;

wc_addstring = MakeWideCharString(addstring);
shstring(wc_addstring, len, max, storestring, msg, errflg);
m_free(wc_addstring,"wide character string");
}

/* Writes a string to the output file, and if appropriate saves it */
void strcode(string, outfile)
M_WCHAR *string;
FILE *outfile;
{
if (! abbrev || fabbrev)
    {
    if (tooutput)
	{
	char *mb_string;

	mb_string = MakeMByteString(string);
	fputs(mb_string, outfile);
	m_free(mb_string,"multi-byte string");
	}

    if (savid)
	shstring(string,
		 &xstrlen,
		 (sizeof(xrefstring) / sizeof(M_WCHAR)),
		 xrefstring,
		 "Too many characters in corresponding cross-reference",
		 &iderr);

    if (savhd)
	shstring(string,
		 &svheadlen,
		 (sizeof(savehead) / sizeof(M_WCHAR)),
		 savehead,
		 "Too many characters in head or caption",
		 &hderr);

    if (savtc)
	shstring(string,
		 &svtclen,
		 (sizeof(savetabcap) / sizeof(M_WCHAR)),
		 savetabcap,
		 "Too many characters in table caption",
		 &hderr);
    }
}


void mb_strcode(string, outfile)
char *string;
FILE *outfile;
{
M_WCHAR *wc;

wc = MakeWideCharString(string);
strcode(wc, outfile);
m_free(wc,"wide character string");
}

/* Copies string to end of buffer where saving head for table of contents */
void svhdstring(string)
M_WCHAR *string;
{
int length;

length = w_strlen(string);
if (svheadlen + length + 1 > (sizeof(savehead) / sizeof(M_WCHAR)))
    {
    if (! hderr)
	{
	m_error("Too many characters in head or caption");
	hderr = TRUE;
	}
    return;
    }
w_strcpy(&savehead[svheadlen], string);
svheadlen += length;
}

/* Copies string to end of buffer where saving table caption */
void svtcstring(string)
M_WCHAR *string;
{
int length;

length = w_strlen(string);
if (svtclen + length + 1 > (sizeof(savetabcap) / sizeof(M_WCHAR)))
    {
    if (! hderr)
	{
	m_error("Too many characters in table caption");
	hderr = TRUE;
	}
    return;
    }
w_strcpy(&savetabcap[svtclen], string);
svtclen += length;
}

/* Process a character in a term */
#if defined(M_PROTO)
void termchar(M_WCHAR textchar)
#else
void termchar(textchar)
M_WCHAR textchar;
#endif
{
if (termp - term >= MAXTERM)
    {
    if (termp - term == MAXTERM)
	{
	M_WCHAR *wc_stago, *wc_tagc;

	wc_stago = MakeWideCharString(m_stago);
	wc_tagc  = MakeWideCharString(m_tagc);
	m_err4("Too many characters in %sTERM%s or %sDTERM%s",
	       wc_stago,
               wc_tagc,
               wc_stago,
               wc_tagc);
	m_free(wc_stago,"wide character string");
	m_free(wc_tagc,"wide character string");
	}
    termp++;
    }
if (! m_whitespace(textchar))
    {
    *termp++ = textchar;
    wsterm = FALSE;
    }
else if (! wsterm)
    {
    wsterm = TRUE;
    mbtowc(termp, " ", 1);
    termp++;
    }
}

/* Process a PI in a term */
void termpi(m_enttype, m_pi, m_entname)
int m_enttype;
M_WCHAR *m_pi;
M_WCHAR *m_entname;
{
M_WCHAR *p, wc;
char    *pc;

outpi(m_enttype, m_pi, m_entname);
if (m_enttype == M_SDATA)
    {
    /* check for output of \needbegin, SDATA only */
    mbtowc(&wc, " ", 1);
    if (! m_wcmbupstrcmp(m_entname, "SIGSPACE")) termchar(wc);
    else
	{
	for (pc = m_ero ; *pc ; pc++)
	    {
	    mbtowc(&wc, pc, 1);
	    termchar(wc);
	    }
	for (p = m_entname ; *p ; p++) termchar(*p);
	for (pc = m_refc ; *pc ; pc++)
	    {
	    mbtowc(&wc, pc, 1);
	    termchar(wc);
	    }
	}
    }
}
