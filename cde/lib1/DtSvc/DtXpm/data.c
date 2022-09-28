/*
 * (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.                *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

/*
 * Duplicate code in Motif library, this is a compatibility layer
 *
 * Note that _DtxpmWriteFile() is not currently in Motif, so it
 * really is here.
 */
#include "_xpmI.h"

/* #include "xpmP.h" */
FUNC(_DtxpmNextString, int, (xpmData * mdata));
FUNC(_DtxpmNextUI, int, (xpmData * mdata, unsigned int *ui_return));
FUNC(_DtxpmGetCmt, int, (xpmData * mdata, char **cmt));
FUNC(_DtxpmNextWord, unsigned int, (xpmData * mdata, char *buf,
				    unsigned int buflen));
FUNC(_DtxpmReadFile, int, (char *filename, xpmData * mdata));
FUNC(_DtxpmWriteFile, int, (char *filename, xpmData * mdata));
FUNC(_DtXpmDataClose, int, (xpmData * mdata));
FUNC(_DtxpmOpenArray, void, (char **data, xpmData * mdata));

/* no prototype for _DtxpmGetString() */
 
/* Global prototype inside the .c file (!) */
static char *RCS_Id = "$XConsortium: data.c /main/cde1_maint/3 1995/09/06 02:19:29 lehors $";
FUNC(atoui, unsigned int, (char *p, unsigned int l, unsigned int *ui_return));

unsigned int
atoui(p, l, ui_return)
    register char *p;
    unsigned int l;
    unsigned int *ui_return;
{
    return _XmXpmatoui(p, l, ui_return);
}

int
_DtxpmNextString(mdata)
    xpmData *mdata;
{
    return _XmxpmNextString(mdata);
}

int
_DtxpmNextUI(mdata, ui_return)
    xpmData *mdata;
    unsigned int *ui_return;
{
    return _XmxpmNextUI(mdata, ui_return);
}

unsigned int
_DtxpmNextWord(mdata, buf, buflen)
    xpmData *mdata;
    char *buf;
    unsigned int buflen;
{
    return _XmxpmNextWord(mdata, buf, BUFSIZ);
}

int
_DtxpmGetString(mdata, sptr, l)
    xpmData *mdata;
    char **sptr;
    unsigned int *l;
{
    return _XmxpmGetString(mdata, sptr, l);
}

int
_DtxpmGetCmt(mdata, cmt)
    xpmData *mdata;
    char **cmt;
{
    return _XmxpmGetCmt(mdata, cmt);
}

int
_DtxpmReadFile(filename, mdata)
    char *filename;
    xpmData *mdata;
{
    return _XmxpmReadFile(filename, mdata);
}

void
_DtxpmOpenArray(data, mdata)
    char **data;
    xpmData *mdata;
{
    _XmxpmOpenArray(data, mdata);
}

int
_DtXpmDataClose(mdata)
    xpmData *mdata;
{
    return _XmXpmDataClose(mdata);
}

/*
 * open the given file to be written as an xpmData which is returned
 */
int
_DtxpmWriteFile(filename, mdata)
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
