/* $XConsortium: option.c /main/cde1_maint/1 1995/07/17 21:41:47 drk $ */
/*   Copyright (c) 1986, 1987, 1988, 1989, 1992 Hewlett-Packard Co. */
/* Functions for command-line options for Help Tag/Cache Creek translator */
#include "userinc.h"
#include "globdec.h"

#if defined(hpux) || defined(_AIX) || defined(sun) || defined(USL)
/* get definition of getenv(3c) for getting of environment variables */
#include <stdlib.h>
#endif


/* Interpret options from command line and specified files */
#define OPTLEN 512 /* If OPTLEN changes, change fscanf call below */
#define TAGDOTOPT "helptag.opt"
#define DOTOPT ".opt"

#if defined(M_PROTO)
void options(LOGICAL filelenonly)
#else
void options(filelenonly)
  LOGICAL filelenonly;
#endif
{
int i;
FILE *optfile;
char option[OPTLEN + 1];
char *nameofoptfile;

/* Check helptag.opt in installation directory */
nameofoptfile = (char *) m_malloc(strlen(install) + strlen(TAGDOTOPT) + 1,
				  "installation helptag.opt");
strcpy(nameofoptfile, install);
strcat(nameofoptfile, TAGDOTOPT);
if (optfile = fopen(nameofoptfile, "r"))
    {
    while (fscanf(optfile, "%512s", option) != EOF)
	setopt(option, filelenonly);
    fclose(optfile);
    }
m_free(nameofoptfile, "installation helptag.opt");

#if defined(hpux) || defined(_AIX) || defined(sun) || defined(USL)
{
char *opts;

/* Check options in $DTTAGOPT */
opts = getenv("DTTAGOPT");
if (opts)
    {
    while (m_whitespace(*opts)) opts++;
    while (sscanf(opts, "%512s", option) != EOF)
	{
	setopt(option, filelenonly);
	opts += strlen(option);
	while (m_whitespace(*opts)) opts++;
	}
    }
}
#endif /* hpux or _AIX or sun */

/* Check helptag.opt in input directory */
if (indir)
    {
    nameofoptfile = (char *) m_malloc(strlen(indir) + strlen(TAGDOTOPT) + 1,
				    "input helptag.opt");
    strcpy(nameofoptfile, indir);
    strcat(nameofoptfile, TAGDOTOPT);
    }
else nameofoptfile = TAGDOTOPT;

if (optfile = fopen(nameofoptfile, "r"))
    {
    while (fscanf(optfile, "%512s", option) != EOF)
	setopt(option, filelenonly);
    fclose(optfile);
    }

if (indir) m_free(nameofoptfile, "input helptag.opt");

/* Check basename.opt in input directory */
nameofoptfile = (char *) m_malloc((indir ? strlen(indir) : 0) +
				  strlen(nodirbase) + strlen(DOTOPT) + 1,
				  "basename.opt");
*nameofoptfile = M_EOS;

if (indir) strcpy(nameofoptfile, indir);

strcat(nameofoptfile, nodirbase);
strcat(nameofoptfile, DOTOPT);

if (optfile = fopen(nameofoptfile, "r"))
    {
    while (fscanf(optfile, "%512s", option) != EOF)
	setopt(option, filelenonly);
    fclose(optfile);
    }
m_free(nameofoptfile, "basename.opt");

/* Read command line options */
for (i = 3 ; i < m_argc ; i++)
    if (*m_argv[i] == '@')
	{
	if (optfile = fopen(m_argv[i] + 1, "r"))
	    {
	    while (fscanf(optfile, "%512s", option) != EOF)
	    setopt(option, filelenonly);
	    fclose(optfile);
	    }
	else
	    {
	    m_mberr1("Unable to open option file %s", (m_argv[i] + 1));
	    }
	}
    else setopt(m_argv[i], filelenonly);

if (filelenonly) return ;    

if (optval)
    {
    m_mberr1("%s: Expecting value for option on command line or in option file",
	     optkey[optval - 1]);
    }
optval = M_NULLVAL;
}


/* Set a single option */
/* Workonly parameter described with function options()*/
#if defined(M_PROTO)
void setopt(char *string, LOGICAL filelenonly)
#else
void setopt(string, filelenonly)
char *string;
LOGICAL filelenonly;
#endif
{
char *p;
int thisopt;
LOGICAL ok;

if (optval)
    {
    /* Ignore '=' by itself */
    if (*string == '=' && *(string + 1) == M_EOS) return;
    setvalopt(optval, string, filelenonly);
    optval = M_NULLVAL;
    return;
    }

if (p = strchr(string, '='))
    {
    *p = M_EOS;
    for (thisopt = 0 ; thisopt < NUMOPTIONS ; thisopt++)
	if (! m_mbmbupstrcmp(string, optkey[thisopt]))
	    break;

    /* Note: call setvalopt only if thisopt < NUMOPTIONS */
    ok = (LOGICAL)
	((thisopt < NUMOPTIONS) && setvalopt(thisopt + 1, p + 1, filelenonly));
    *p = '=';
    if (! ok && ! filelenonly)
	m_mberr1("%s: Unrecognized option on command line or in option file",
		 string);
    return;
    } /* End strchr(string, '=') */

for (thisopt = 0 ; thisopt < NUMOPTIONS ; thisopt++)
    if (! m_mbmbupstrcmp(string, optkey[thisopt]))
	break;

if (thisopt >= NUMOPTIONS)
    {
    if (! filelenonly)
	m_mberr1("%s: Unrecognized option on command line or in option file",
		 string);
    return;
    }
else switch(thisopt + 1)
    {
    case ONERROR:
    case CHARSET:
    case SEARCHKEY:
	optval = thisopt + 1;
	break;
    case DRAFT:
	final = FALSE;
	break;
    case FINAL:
	final = TRUE;
	break;
#if defined(FUTURE_FEATURE)
    case GRAPHICS:
	dofigure = SMOOTH;
	break;
    case NOGRAPHICS:
	dofigure = FALSE;
	break;
    case REV:
	prntrevs = TRUE;
	break;
    case NOREV:
	prntrevs = FALSE;
	break;
    case TOC:
	toc = TRUE;
	if (tocfile!=NULL)
	    {
	    m_free(tocfile, "TOC option");
	    tocfile = NULL;
	    }
	break;
    case NOTOC:
	toc = FALSE;
	if (tocfile!=NULL)
	    {
	    m_free(tocfile, "NOTOC option");
	    tocfile = NULL;
	    }
	break;
    case INDEX:
	break;
    case NOINDEX:
	break;
#endif /* FUTURE_FEATURE */
    case MEMO:
	memo = TRUE;
	break;
    case NOMEMO:
	memo = FALSE;
	break;
    case CLEARSEARCH:
	{
	SEARCH *searchp, *searchq;

	for (searchp = path ; searchp ;)
	    {
	    searchq = searchp;
	    searchp = searchp->next;
	    m_free(searchq->directory, "search directory");
	    m_free(searchq, "search path");
	    }
	path = NULL;
	endpath = &path;
	break;
	}
    case LONGFILES:
    case LONGFILE:
    case LONG:
	usingshortnames = FALSE;
    break;
    case SHORTFILES:
    case SHORTFILE:
    case SHORT:
	usingshortnames = TRUE;
    break;
    }
}

/* Process a value for a command line option */
#if defined(M_PROTO)
LOGICAL setvalopt(int thisopt, char *string, LOGICAL filelenonly)
#else
LOGICAL setvalopt(thisopt, string, filelenonly)
int thisopt;
char *string;
LOGICAL filelenonly;
#endif
{
char *p;
int i;
char *tempstr;

/* Ignore leading = (occurs if "option =val" is entered in .opt file) */
if (*string == '=') string++;

/* Check for empty string (occurs if "option= val" is entered in .opt file) */
if (! *string)
    switch(thisopt)
	{
        case SEARCHKEY:
	case CHARSET:
	case ONERROR:
	    optval = thisopt;
	    return TRUE;
	default:
	    return FALSE;
	}

switch (thisopt)
    {
    case SEARCHKEY:
	tempstr = (char *) m_malloc( strlen(string) + 1, "tempstr search");
	strcpy(tempstr, string);

	*endpath = (SEARCH *) m_malloc(sizeof(SEARCH), "search path");
	p = strchr(tempstr, M_EOS);
	if (p != tempstr && *(p - 1) != dirsep)
	    {
	    i = strlen(tempstr);
	    (*endpath)->directory = (char *) m_malloc(i + 2,
						      "search directory");
	    strcpy((*endpath)->directory, tempstr);
	    (*endpath)->directory[i] = dirsep;
	    (*endpath)->directory[i + 1] = M_EOS;
	    }
	else
	    {
	    (*endpath)->directory = (char *) m_malloc(strlen(tempstr) + 1,
						    "search directory");
	    strcpy((*endpath)->directory, tempstr);
	    }
	(*endpath)->next = NULL;
	endpath = &(*endpath)->next;
	m_free(tempstr, "tempstr search");
	return(TRUE);
    case CHARSET:
	m_free(helpcharset, "help charset");
	helpcharset = MakeWideCharString(string);
	return TRUE;
    case ONERROR:
	if (! m_mbmbupstrcmp(string, "GO"))
	    stoponerror = FALSE;
	else if (! m_mbmbupstrcmp(string, "STOP"))
	    stoponerror = TRUE;
	else m_mberr1("Invalid ONERROR option: %s", string);
	return(TRUE);
    default:
	return(FALSE);
    }
}
