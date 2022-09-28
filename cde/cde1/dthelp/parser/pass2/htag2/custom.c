/* $XConsortium: custom.c /main/cde1_maint/1 1995/07/17 22:09:40 drk $ */
/*
Copyright (c) 1988, 1989 Hewlett-Packard Co.
*/

/* Custom.c contains standard PARSER functions, customized for the HP
   HelpTag formatting system. */

#include "userinc.h"
#include "globdec.h"
#include <stdlib.h>


#if defined(MSDOS)
/* Standard startup code doesn't have room to load inherited environments
   in some cases.  Since they're not used, don't bother.  (Using Microsoft
   C compiler).  */
void _setenvp(M_NOPAR);
void _setenvp(){}
#endif

/* Write input file and line number for an error message */
void m_dumpline(file, line)
M_WCHAR *file;
int line;
{
char buffer[10];
char *mbyte;

m_errline("Line ");
sprintf(buffer, "%d", line);
m_errline(buffer);
if (!file)
    { /* no entity file */
    if (inFileName)
	{
	m_errline(" of ");
	m_errline(inFileName);
	}
    }	
else
    { /* yes, entity file */
    mbyte = MakeMByteString(file);
    m_errline(" of ");
    m_errline(mbyte);
    m_free(mbyte, "multi-byte string");
    }
}

/* Write error message prefix */
void m_eprefix(M_NOPAR)
{
m_errline("\n*****\n");
m_dumpline(m_thisfile(), m_thisline());
m_errline(",\n");
}

/* Process error message text */
void m_errline(p)
char *p;
{
char c;

for ( ; *p ; p++)
    {
    if (m_errfile) putc(*p, m_errfile);
    putc(*p, stderr);
    }
}

#if defined(MSDOS)
#include <process.h>
#endif
/* Write error message suffix */
void m_esuffix(M_NOPAR)
{
m_errline(":\n");
m_lastchars();
if (++m_errcnt == m_errlim)
    {
    m_error("Too many errors, processing stopped");
    m_exit(TRUE);
    }
}

/* Exit procedure */
void m_exit(status)
int status;
{

if (status)
    {
    if (status == 77) /* tell helptag to re-run for forward xrefs */
	{
	if (stoponerror)
	    {
	    if (m_errcnt == 0)
		exit(77);
	    else
		exit(1);
	    }
	else
	    exit(66);
	}

    if (stoponerror)
	exit(1); /* tell helptag to quit */

    exit(2); /* tell helptag to continue to next phases */
    }

exit(0);
}

/* Get-char procedure */
int m_getc(m_ptr)
void *m_ptr;
{
int  c;
M_WCHAR wc;
char badch[2];
char mbyte[32]; /* make this bigger than any possible multi-byte char */
int  length;
static M_WCHAR wcr = 0, wsb, wsp, wtb;
char tab, space;

/* Unix/Dos compatibility: 0D0A handling */ 
if (!wcr)
    {
    mbtowc(&wcr, "\r", 1);
    mbtowc(&wsb, "\032", 1);

    space = M_SPACE;
    mbtowc(&wsp, &space, 1);

    tab = M_TAB;
    mbtowc(&wtb, &tab, 1);
    }

do  {
    length = 0;
    if ((c = getc((FILE *)m_ptr)) == EOF) return(EOF);
    while (1)
	{
	mbyte[length++] = c;
	mbyte[length]   = 0;
	if (mblen(mbyte,length) != -1) break; /* hurray! */
	if (length == MB_CUR_MAX)
	    { /* reached max without a hit */
	    m_error("An invalid multi-byte character was found in the input");
	    c = ' ';
	    length = 1;
	    break;
	    }
	if ((c = getc((FILE *) m_ptr)) == EOF)
	    { /* huh? */
	    m_error("End-of-file found in within a multi-byte character");
	    return(EOF);
	    }
	}
    mbtowc(&wc,mbyte,length);
    }
while ((wc == wcr) || (wc == wsb));

/* Change tabs to spaces */
if (wc == wtb) return((int) wsp);
return((int) wc);
}

/* Open SYSTEM entity procedure */
void *m_openent(entcontent)
M_WCHAR *entcontent;
{
FILE *open;
char *filename;
SEARCH *searchp;
char *mb_entcontent;

mb_entcontent = MakeMByteString(entcontent);
if (!*mb_entcontent) return NULL; /* null file name, don't open a directory */

open = fopen(mb_entcontent, "r");
if (open)
    {
    m_free(mb_entcontent, "multi-byte string");
    return((void *) open);
    }

for (searchp = path ; searchp ; searchp = searchp->next)
    {
    filename = (char *)
	     m_malloc(strlen(searchp->directory) +
			strlen(mb_entcontent) + 1,
		      "filename");
    strcpy(filename, searchp->directory);
    strcat(filename, mb_entcontent);
    open = fopen(filename, "r");
    m_free(filename, "filename");
    if (open)
	{
	m_free(mb_entcontent, "multi-byte string");
	return((void *) open);
	}
    }

m_free(mb_entcontent, "multi-byte string");
return(NULL);
}

/* Open input file */
void *m_openfirst(M_NOPAR)
{
return((void *) inFile);
}

/* Set program options */
void m_setoptions()
{
if (m_argc > 2)
    {
    m_optstring(m_argv[2]);
    }
}

/* Process signon message text, stripping out MARKUP version number, so
   only one version number will appear */
void m_signmsg(p)
  char *p;
  {
    char *q;
    char *pCopy;

    if (q = strstr(p, VERSION)) {
      pCopy = strdup(p);
      q = strstr(pCopy, VERSION);
      *q = M_EOS;
      m_errline(pCopy);
      free(pCopy);
      return;
      }
    m_errline(p);
    }

/* All entity declarations have been processed. */
void m_startdoc()
{
}

/* Write debugging trace information */
void m_trace(p)
char *p;
{
}


void m_wctrace(p)
M_WCHAR *p;
{
char *mb_p;

mb_p = MakeMByteString(p);
m_trace(mb_p);
m_free(mb_p,"multi-byte string");
}
