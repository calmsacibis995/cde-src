/*
 * $XConsortium: bufio.c /main/cde1_maint/1 1995/10/08 17:21:46 pascale $
 *
 * Copyright 1991 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */


/* #include    "fontmisc.h" */
#include    <errno.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <unistd.h>
#include    "bufioI.h"
extern int errno;

#ifndef	MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define FileDes(f)		((int) (f)->hidden)
#define CompressFileDes(f)	(((CECompressInfoPtr) (f)->hidden)->fd)
#define CompressSize(f)		(((CECompressInfoPtr) (f)->hidden)->size)

static int
#ifdef _NO_PROTO
BufFileRawSkip (f, count)
    BufFilePtr	f;
    int		count;
#else
BufFileRawSkip (
    BufFilePtr	f,
    int		count )
#endif /* _NO_PROTO */
{
    int	    curoff;
    int	    fileoff;
    int	    todo;

    curoff = f->bufp - f->buffer;
    fileoff = curoff + f->left;
    if (curoff + count <= fileoff) {
	f->bufp += count;
	f->left -= count;
    } else {
	todo = count - (fileoff - curoff);
	if (lseek (FileDes(f), (off_t) todo, 1) == -1) {
	    if (errno != ESPIPE)
		return BUFFILEEOF;
	    while (todo) {
		curoff = BUFFILESIZE;
		if (curoff > todo)
		    curoff = todo;
		fileoff = read (FileDes(f), f->buffer, curoff);
		if (fileoff <= 0)
		    return BUFFILEEOF;
		todo -= fileoff;
	    }
	}
	f->left = 0;
    }
    return count;
}

static
#ifdef _NO_PROTO
BufFileRawFlush (c, f)
    int		c;
    BufFilePtr	f;
#else
BufFileRawFlush (
    int		c,
    BufFilePtr	f )
#endif /* _NO_PROTO */
{
    int	cnt;

    if (c != BUFFILEEOF)
	*f->bufp++ = c;
    cnt = f->bufp - f->buffer;
    f->bufp = f->buffer;
    f->left = BUFFILESIZE;
    if (write (FileDes(f), f->buffer, cnt) != cnt)
	return BUFFILEEOF;
    return c;
}

BufFilePtr
#ifdef _NO_PROTO
_DtHelpCeBufFileOpenWr (fd)
    int	fd;
#else
_DtHelpCeBufFileOpenWr (int fd)
#endif /* _NO_PROTO */
{
    BufFilePtr	f;

    f = _DtHelpCeBufFileCreate ((char *) fd, BufFileRawFlush, NULL, _DtHelpCeBufFileFlush);
    f->bufp = f->buffer;
    f->left = BUFFILESIZE;
    return f;
}

#ifdef	obsolete_function
#ifdef _NO_PROTO
_DtHelpCeBufFileWrite (f, b, n)
    BufFilePtr	f;
    char	*b;
    int		n;
#else
_DtHelpCeBufFileWrite (
    BufFilePtr	f,
    char	*b,
    int		n )
#endif /* _NO_PROTO */
{
    int	    cnt;
    cnt = n;
    while (cnt--) {
	if (BufFilePut (*b++, f) == BUFFILEEOF)
	    return BUFFILEEOF;
    }
    return n;
}
#endif

int
#ifdef _NO_PROTO
_DtHelpCeBufFileFlush (f, doClose)
    BufFilePtr	f;
    int         doClose;
#else
_DtHelpCeBufFileFlush (BufFilePtr f, int doClose)
#endif /* _NO_PROTO */
{
    if (f->bufp != f->buffer)
	(*f->io) (BUFFILEEOF, f);

    if (doClose)
	return (close (FileDes(f)));

    return 0;
}

/*****************************************************************************
 *			Private Routines
 *****************************************************************************/
/*****************************************************************************
 *			Routines working on a File descriptor
 *****************************************************************************/
static int
#ifdef _NO_PROTO
FdRawRead (f)
    BufFilePtr	f;
#else
FdRawRead (BufFilePtr f)
#endif /* _NO_PROTO */
{
    int	left;

    left = read (FileDes(f), f->buffer, BUFFILESIZE);
    if (left <= 0) {
	f->left = 0;
	return BUFFILEEOF;
    }
    f->left = left - 1;
    f->bufp = f->buffer + 1;
    return f->buffer[0];
}

static	int
#ifdef _NO_PROTO
FdClose (f, doClose)
    BufFilePtr	f;
    int         doClose;
#else
FdClose (
    BufFilePtr	f,
    int         doClose)
#endif /* _NO_PROTO */
{
    if (doClose)
	close (FileDes (f));
    return 1;
}

/*****************************************************************************
 *			Routines working on a Raw Compressed file
 *****************************************************************************/
static int
#ifdef _NO_PROTO
CompressRawRead (f)
    BufFilePtr	f;
#else
CompressRawRead (BufFilePtr f)
#endif /* _NO_PROTO */
{
    int	left;


    left = read (CompressFileDes(f), f->buffer,
					MIN(CompressSize(f),BUFFILESIZE));
    if (left <= 0) {
	f->left = 0;
	CompressSize(f) = 0;
	return BUFFILEEOF;
    }
    CompressSize(f) -= left;
    f->left = left - 1;
    f->bufp = f->buffer + 1;
    return f->buffer[0];
}

static	int
#ifdef _NO_PROTO
CompressRawClose (f, doClose)
    BufFilePtr	f;
    int         doClose;
#else
CompressRawClose (
    BufFilePtr	f,
    int         doClose)
#endif /* _NO_PROTO */
{
    if (doClose)
	close (CompressFileDes (f));
    free(f->hidden);
    return 1;
}

/*****************************************************************************
 *			Routines working on a Pipe
 *****************************************************************************/
/*****************************************************************************
 * Function:	int RdPipeStream (BufFilePtr f)
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
static int
#ifdef _NO_PROTO
RdPipeStream (f)
    BufFilePtr  f;
#else
RdPipeStream (BufFilePtr f)
#endif /* _NO_PROTO */
{
    int    left;

    left = fread(f->buffer, 1, BUFFILESIZE, FileStream(f));

    if (left <= 0)
      {
        f->left = 0;
        return BUFFILEEOF;
      }

    clearerr(FileStream(f));
    f->left = left - 1;
    f->bufp = f->buffer + 1;
    return f->buffer[0];
}

/*********************************************************************
 * Procedure: int ClosePipeStream (BufFilePtr f);
 *
 * Returns:
 *
 * Purpose:
 *
 ********************************************************************/
static int
#ifdef _NO_PROTO
ClosePipeStream (f, doClose)
    BufFilePtr  f;
    int         doClose;
#else
ClosePipeStream (
    BufFilePtr  f,
    int         doClose)
#endif /* _NO_PROTO */
{
    if (doClose)
        pclose(FileStream(f));

    return 1;
}

/*****************************************************************************
 *			Semi-Public Routines
 *****************************************************************************/
/*****************************************************************************
 * Function:	BufFilePtr _DtHelpCeBufFileCreate (char *hidden,
 *					int (*io)(), int (*skip)(),
 *					int (*close)())
 *
 * Returns:	A pointer to malloc'ed memory or NULL.
 *
 * Purpose:	Create a buffered i/o mechanism.
 *
 *****************************************************************************/
BufFilePtr
#ifdef _NO_PROTO
_DtHelpCeBufFileCreate (hidden, io, skip, close)
    char    *hidden;
    int	    (*io)();
    int	    (*skip)();
    int	    (*close)();
#else
_DtHelpCeBufFileCreate (
    char    *hidden,
    int	    (*io)(),
    int	    (*skip)(),
    int	    (*close)() )
#endif /* _NO_PROTO */
{
    BufFilePtr	f;

    f = (BufFilePtr) malloc (sizeof *f);
    if (!f)
	return 0;
    f->hidden = hidden;
    f->bufp = f->buffer;
    f->left = 0;
    f->io = io;
    f->skip = skip;
    f->close = close;
    return f;
}

/*****************************************************************************
 * Function:	BufFilePtr _DtHelpCeBufFileRdWithFd (int fd)
 *
 * Returns:	A pointer to malloc'ed memory or NULL.
 *
 * Purpose:	Create a buffered i/o mechanism using a file descriptor
 *		as private data and attaching a raw read to the i/o
 *		routine.
 *
 *****************************************************************************/
BufFilePtr
#ifdef _NO_PROTO
_DtHelpCeBufFileRdWithFd (fd)
    int	fd;
#else
_DtHelpCeBufFileRdWithFd (int fd)
#endif /* _NO_PROTO */
{
    return _DtHelpCeBufFileCreate ((char *) fd, FdRawRead, BufFileRawSkip, FdClose);
}

/*****************************************************************************
 * Function:	BufFilePtr _DtHelpCeBufFileRdRawZ (CECompressInfoPtr file)
 *
 * Returns:	A pointer to malloc'ed memory or NULL.
 *
 * Purpose:	Create a buffered i/o mechanism using a file descriptor
 *		as private data and attaching a raw read of compressed
 *		data to the i/o routine.
 *
 *****************************************************************************/
BufFilePtr
#ifdef _NO_PROTO
_DtHelpCeBufFileRdRawZ (file)
    CECompressInfoPtr	file;
#else
_DtHelpCeBufFileRdRawZ (CECompressInfoPtr file)
#endif /* _NO_PROTO */
{
    return _DtHelpCeBufFileCreate ((char *) file, CompressRawRead, NULL,
							CompressRawClose);
}

/*****************************************************************************
 * Function:	void _DtHelpCeBufFileClose (BufFilePtr file, int doClose)
 *
 * Returns:	nothing
 *
 * Purpose:	Calls the close routine associated with the pointer.
 *		Frees the BufFile information.
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeBufFileClose (f, doClose)
    BufFilePtr	f;
    int         doClose;
#else
_DtHelpCeBufFileClose (
    BufFilePtr	f,
    int         doClose )
#endif /* _NO_PROTO */
{
    (void) (*f->close) (f, doClose);
    free (f);
}

/*****************************************************************************
 * Function:	void _DtHelpCeBufFileRd (BufFilePtr file, int doClose)
 *
 * Returns:	nothing
 *
 * Purpose:	Calls the close routine associated with the pointer.
 *		Frees the BufFile information.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeBufFileRd (f, b, n)
    BufFilePtr	f;
    char	*b;
    int		n;
#else
_DtHelpCeBufFileRd (
    BufFilePtr	f,
    char	*b,
    int		n )
#endif /* _NO_PROTO */
{
    int	    c, cnt;
    cnt = n;
    while (cnt--) {
	c = BufFileGet (f);
	if (c == BUFFILEEOF)
	    break;
	*b++ = c;
    }
    return n - cnt - 1;
}

/*****************************************************************************
 * Function:	BufFilePtr _DtHelpCeCreatePipeBufFile (FILE *stream)
 *
 * Returns:	A pointer to malloc'ed memory or NULL.
 *
 * Purpose:	Create a buffered i/o mechanism using a pipe descriptor
 *		as private data and attaching a raw read to the i/o
 *		routine.
 *
 *****************************************************************************/
BufFilePtr
#ifdef _NO_PROTO
_DtHelpCeCreatePipeBufFile (stream)
    FILE	*stream;
#else
_DtHelpCeCreatePipeBufFile (FILE *stream)
#endif /* _NO_PROTO */
{
    return _DtHelpCeBufFileCreate ((char *) stream,
					RdPipeStream, NULL, ClosePipeStream);
}
