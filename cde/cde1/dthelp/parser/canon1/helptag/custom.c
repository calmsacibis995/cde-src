/* $XConsortium: custom.c /main/cde1_maint/1 1995/07/17 21:08:23 drk $ */
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
    if (inputname)
	{ /* use main input, instead.  Only if set though. */
	mbyte = MakeMByteString(inputname);
	m_errline(" of ");
	m_errline(mbyte);
	m_free(mbyte, "multi-byte string");
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
if (filefound)
    {
    if (m_outfile != stdout)
	{
	if (have_index)
	    {  /* sph: is this really necessary? */
	    fseek(m_outfile, 0L, SEEK_END);
	    }
	fclose(m_outfile);
	if (prebye == postpreamble)
	    m_error("No text in document");
	}
    }

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
    if ((c = getc((FILE *) m_ptr)) == EOF) return(EOF);
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
FILE *first;
char *input;
int   length;

if (defaultext)
    {
    input = (char *)
	m_malloc(strlen(m_argv[1]) + strlen(".htg") + 1, "input file name");
    strcpy(input, m_argv[1]);
    strcat(input, ".htg");
    m_openchk(&first, input, "r");
    if (filelist) puts(input);
    /* keep name for global use */
    length = strlen(input);
    inputname = (M_WCHAR *) m_malloc(length + 1, "saved input file name");
    mbstowcs(inputname, input, length + 1);
    m_free(input, "input file name");
    }
else
    {
    if (filelist) puts(m_argv[1]);
    m_openchk(&first, m_argv[1], "r");
    length = strlen(m_argv[1]);
    inputname = (M_WCHAR *) m_malloc(length + 1, "saved input file name");
    mbstowcs(inputname, m_argv[1], length + 1);
    }

/* Set E option (to suppress error message on duplicate entity
   declarations) if file begins with "<!--Index" */
if (first)
    {
    filefound = TRUE;
    }
return((void *) first);
}

/* Set program options */
void m_setoptions()
  {
    /* F option used for FILELIST (checking done in basename, which is
       called before this function is called) */
    if (m_argc > 2) {
      m_optstring(m_argv[2]);
      if (strchr(m_argv[2], 'o')) tracetostd = TRUE;
      if (strchr(m_argv[2], 'O')) tracetostd = TRUE;
      /* Option "p" tells the search path to look at paths one level
         higher when relative pathes are specified.
				
         The following two lines of code should be here, but they are moved
         to global START-CODE of tex.if.  For some reason, global START-CODE
         is executed before this procedure is called (parser.c), but we need
         to have the order reversed.  Moving this procedure to a spot earlier
         is risky, since we don't know full reasons of why things are in a
         particular order.  This way, we minimize the damage:

			if (strchr(m_argv[2], 'p')) parentsrch = TRUE;
      if (strchr(m_argv[2], 'P')) parentsrch = TRUE;
      */
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

/* All entity declarations have been processed.  Can now check if .TEX
   file uptodate and open appropriate output file */
void m_startdoc()
{
LOGICAL init = TRUE;
unsigned char type;
M_WCHAR *content;
unsigned char wheredef;
M_WCHAR *name;
M_WCHAR *qfile;
char *mbyte;

/* set locale */  
SetDefaultLocale();

if (! filelist)
    {
    texinit();
    }
else
    { /* list files that make up document */
    while (name = m_cyclent(init, &type, &content, &wheredef))
	{
	init = FALSE;
	qfile = NULL;
	if (type == M_SYSTEM)
	    qfile = searchforfile(content);
	if (qfile)
	    {
	    mbyte = MakeMByteString(qfile);
	    puts(mbyte);
	    m_free(qfile, "figure filename");
	    m_free(mbyte, "multi-byte string");
	    }
	else if (type == M_SYSTEM)
	  m_err2("Can't find file %s (declared in entity %s)", content, name);
	}
    if (idxpath) puts(idxpath);
    exit(m_errexit);
    }
}

/* Write debugging trace information */
void m_trace(p)
char *p;
{
if (tracetostd) fputs(p, stdout);
else fputs(p, m_outfile);
}


void m_wctrace(p)
M_WCHAR *p;
{
char *mb_p;

mb_p = MakeMByteString(p);
m_trace(mb_p);
m_free(mb_p,"multi-byte string");
}
