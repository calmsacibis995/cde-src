/**---------------------------------------------------------------------
***	
***    (c)Copyright 1991 Hewlett-Packard Co.
***    
***                             RESTRICTED RIGHTS LEGEND
***    Use, duplication, or disclosure by the U.S. Government is subject to
***    restrictions as set forth in sub-paragraph (c)(1)(ii) of the Rights in
***    Technical Data and Computer Software clause in DFARS 252.227-7013.
***                             Hewlett-Packard Company
***                             3000 Hanover Street
***                             Palo Alto, CA 94304 U.S.A.
***    Rights for non-DOD U.S. Government Departments and Agencies are as set
***    forth in FAR 52.227-19(c)(1,2).
***
***-------------------------------------------------------------------*/

/* Copyright 1990,91 GROUPE BULL -- See licence conditions in file COPYRIGHT */
/*****************************************************************************\
* data.c:                                                                     *
*                                                                             *
*  XPM library                                                                *
*  IO utilities                                                               *
*                                                                             *
*  Developed by Arnaud Le Hors                                                *
\*****************************************************************************/

/* Official version number */
static char *RCS_Version = "$XpmVersion: 3.0 $";

/* Internal version number */
static char *RCS_Id = "$Id: xpm.shar,v 3.0 1991/10/04 10:38:20 lehors Exp $";

#include "ilxpmP.h"
#ifdef VMS
#include "sys$library:stat.h"
#include "sys$library:ctype.h"
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#endif

LFUNC(atoui, unsigned int, (char *p, unsigned int l, unsigned int *ui_return));

static unsigned int
atoui(p, l, ui_return)
    register char *p;
    unsigned int l;
    unsigned int *ui_return;
{
    register int n, i;

    n = 0;
    for (i = 0; i < l; i++)
	if (*p >= '0' && *p <= '9')
	    n = n * 10 + *p++ - '0';
	else
	    break;

    if (i != 0 && i == l) {
	*ui_return = n;
	return 1;
    } else
	return 0;
}

/*
 * skip to the end of the current string and the beginning of the next one
 */
_ilefsxpmNextString(mdata)
    xpmData *mdata;
{
    int c;

    switch (mdata->type) {
    case XPMARRAY:
	mdata->cptr = (mdata->stream.data)[++mdata->line];
	break;
    case XPMFILE:
    case XPMPIPE:
	if (mdata->Eos)
	    while ((c = _ilefsxpmGetC(mdata)) != mdata->Eos && c != EOF);
	if (mdata->Bos)			/* if not natural XPM2 */
	    while ((c = _ilefsxpmGetC(mdata)) != mdata->Bos && c != EOF);
	break;
    }
}

/*
 * skip whitespace and compute the following unsigned int,
 * returns 1 if one is found and 0 if not
 */
int
_ilefsxpmNextUI(mdata, ui_return)
    xpmData *mdata;
    unsigned int *ui_return;
{
    char buf[BUFSIZ];
    int l;

    l = _ilefsxpmNextWord(mdata, buf);
    return atoui(buf, l, ui_return);
}

/*
 * return the current character, skipping comments
 */
_ilefsxpmGetC(mdata)
    xpmData *mdata;
{
    int c;
    register unsigned int n = 0, a;
    unsigned int notend;

    switch (mdata->type) {
    case XPMARRAY:
	return (*mdata->cptr++);
    case XPMFILE:
    case XPMPIPE:
	c = getc(mdata->stream.file);

	if (mdata->Bos && mdata->Eos
	    && (c == mdata->Bos || c == mdata->Eos)) {
	    /* if not natural XPM2 */
	    mdata->InsideString = !mdata->InsideString;
	    return (c);
	}
	if (!mdata->InsideString && mdata->Bcmt && c == mdata->Bcmt[0]) {
	    mdata->Comment[0] = c;

	    /*
	     * skip the string begining comment 
	     */
	    do {
		c = getc(mdata->stream.file);
		mdata->Comment[++n] = c;
	    } while (c == mdata->Bcmt[n] && mdata->Bcmt[n] != '\0'
		     && c != EOF);

	    if (mdata->Bcmt[n] != '\0') {
		/* this wasn't the begining of a comment */
		/* put characters back in the order that we got them */
		for (a = n; a > 0; a--)
		    _ilefsxpmUngetC(mdata->Comment[a], mdata);
		return (mdata->Comment[0]);
	    }

	    /*
	     * store comment 
	     */
	    mdata->Comment[0] = mdata->Comment[n];
	    notend = 1;
	    n = 0;
	    while (notend) {
		while (mdata->Comment[n] != mdata->Ecmt[0] && c != EOF) {
		    c = getc(mdata->stream.file);
		    mdata->Comment[++n] = c;
		}
		mdata->CommentLength = n;
		a = 0;
		do {
		    c = getc(mdata->stream.file);
		    n++;
		    a++;
		    mdata->Comment[n] = c;
		} while (c == mdata->Ecmt[a] && mdata->Ecmt[a] != '\0'
			 && c != EOF);
		if (mdata->Ecmt[a] == '\0') {
		    /* this is the end of the comment */
		    notend = 0;
		    _ilefsxpmUngetC(mdata->Comment[n], mdata);
		}
	    }
	    c = _ilefsxpmGetC(mdata);
	}
	return (c);
    }
}

/*
 * push the given character back
 */
_ilefsxpmUngetC(c, mdata)
    int c;
    xpmData *mdata;
{
    switch (mdata->type) {
    case XPMARRAY:
	return (*--mdata->cptr = c);
    case XPMFILE:
    case XPMPIPE:
	if (mdata->Bos && (c == mdata->Bos || c == mdata->Eos))
	    /* if not natural XPM2 */
	    mdata->InsideString = !mdata->InsideString;
	return (ungetc(c, mdata->stream.file));
    }
}

/*
 * skip whitespace and return the following word
 */
unsigned int
_ilefsxpmNextWord(mdata, buf)
    xpmData *mdata;
    char *buf;
{
    register unsigned int n = 0;
    int c;

    switch (mdata->type) {
    case XPMARRAY:
	while (isspace(c = *mdata->cptr) && c != mdata->Eos)
	    mdata->cptr++;
	do {
	    c = *mdata->cptr++;
	    buf[n++] = c;
	} while (!isspace(c) && c != mdata->Eos && c != '\0' && n < BUFSIZ);
	n--;
	mdata->cptr--;
	break;
    case XPMFILE:
    case XPMPIPE:
	while (isspace(c = _ilefsxpmGetC(mdata)) && c != mdata->Eos);
	while (!isspace(c) && c != mdata->Eos && c != EOF && n < BUFSIZ) {
	    buf[n++] = c;
	    c = _ilefsxpmGetC(mdata);
	}
	_ilefsxpmUngetC(c, mdata);
	break;
    }
    return (n);
}

/*
 * get the current comment line
 */
_ilefsxpmGetCmt(mdata, cmt)
    xpmData *mdata;
    char **cmt;
{
    switch (mdata->type) {
    case XPMARRAY:
	*cmt = NULL;
	break;
    case XPMFILE:
    case XPMPIPE:
	if (mdata->CommentLength) {
	    *cmt = (char *) malloc(mdata->CommentLength + 1);
	    strncpy(*cmt, mdata->Comment, mdata->CommentLength);
	    (*cmt)[mdata->CommentLength] = '\0';
	    mdata->CommentLength = 0;
	} else
	    *cmt = NULL;
	break;
    }
}

/*
 * open the given file to be read as an xpmData which is returned.
 */
int
_ilefsxpmReadFile(filename, mdata)
    char *filename;
    xpmData *mdata;
{
    char *compressfile, buf[BUFSIZ];
    struct stat status;

	if (!(mdata->stream.file = fopen(filename, "r"))) {
	    return (XpmOpenFailed);
	}
	mdata->type = XPMFILE;
    mdata->CommentLength = 0;
    mdata->InsideString = 0;
    return (XpmSuccess);
}

#ifdef UNUSED
/*
 * open the given file to be written as an xpmData which is returned
 */
int
xpmWriteFile(filename, mdata)
    char *filename;
    xpmData *mdata;
{
    char buf[BUFSIZ];

    if (!filename) {
	mdata->stream.file = (stdout);
	mdata->type = XPMFILE;
    } else {
#ifdef ZPIPE
	if (strlen(filename) > 2
	    && !strcmp(".Z", filename + (strlen(filename) - 2))) {
	    sprintf(buf, "compress > %s", filename);
	    if (!(mdata->stream.file = popen(buf, "w")))
		return (XpmOpenFailed);

	    mdata->type = XPMPIPE;
	} else {
#endif
	    if (!(mdata->stream.file = fopen(filename, "w")))
		return (XpmOpenFailed);

	    mdata->type = XPMFILE;
#ifdef ZPIPE
	}
#endif
    }
    return (XpmSuccess);
}

/*
 * open the given array to be read or written as an xpmData which is returned
 */
int
_ilefsxpmOpenArray(data, mdata)
    char **data;
    xpmData *mdata;
{
    mdata->type = XPMARRAY;
    mdata->stream.data = data;
    mdata->cptr = *data;
    mdata->line = 0;
    mdata->CommentLength = 0;
    mdata->Bcmt = mdata->Ecmt = NULL;
    mdata->Bos = mdata->Eos = '\0';
    mdata->InsideString = 0;
    return (XpmSuccess);
}
#endif

/*
 * close the file related to the xpmData if any
 */
_ilefsXpmDataClose(mdata)
    xpmData *mdata;
{
    switch (mdata->type) {
    case XPMARRAY:
	break;
    case XPMFILE:
	if (mdata->stream.file != (stdout) && mdata->stream.file != (stdin))
	    fclose(mdata->stream.file);
	break;
    case XPMPIPE:
	pclose(mdata->stream.file);
    }
}
