/* $XConsortium: bufioI.h /main/cde1_maint/1 1995/07/17 17:56:40 drk $ */
#ifndef	_DtHelpbufioI_h
#define	_DtHelpbufioI_h

#ifndef	NULL
#define	NULL	0
#endif

#define BUFFILESIZE	4096
#define BUFFILEEOF	-1

typedef unsigned char BufChar;

typedef struct _buffile {
    BufChar *bufp;
    int	    left;
    BufChar buffer[BUFFILESIZE];
    int	    (*io)(/* BufFilePtr f */);
    int	    (*skip)(/* BufFilePtr f, int count */);
    int	    (*close)(/* BufFilePtr f */);
    char    *hidden;
} BufFileRec, *BufFilePtr;

typedef	struct _compressInfo{
    int     fd;
    int     size;
} CECompressInfo, *CECompressInfoPtr;

extern BufFilePtr   __DtBufFileCreate ();
extern BufFilePtr   _DtHelpCeBufFilePushZ ();
extern BufFilePtr   _DtHelpCeBufFileOpenWr ();
extern int	    _DtHelpCeBufFileFlush ();
#define BufFileGet(f)	((f)->left-- ? *(f)->bufp++ : (*(f)->io) (f))
#define BufFilePut(c,f)	(--(f)->left ? *(f)->bufp++ = (c) : (*(f)->io) (c,f))
#define BufFilePutBack(c,f) { (f)->left++; *(--(f)->bufp) = (c); }
#define BufFileSkip(f,c)    ((*(f)->skip) (f, c))

#define FileStream(f)   ((FILE *)(f)->hidden)

#ifdef	_NO_PROTO
extern	void		_DtHelpCeBufFileClose ();
extern	BufFilePtr	_DtHelpCeBufFileCreate ();
extern	int		_DtHelpCeBufFileRd ();
extern	BufFilePtr	_DtHelpCeBufFileRdWithFd ();
extern	BufFilePtr	_DtHelpCeBufFileRdRawZ ();
extern	BufFilePtr	_DtHelpCeCreatePipeBufFile ();
extern	int		_DtHelpCeUncompressFile ();
#else
extern	void		_DtHelpCeBufFileClose (
				BufFilePtr	f,
				int		doClose);
extern	BufFilePtr	_DtHelpCeBufFileCreate (
				char		*hidden,
				int		(*io)(),
				int		(*skip)(),
				int		(*close)());
extern	int		_DtHelpCeBufFileRd (
				BufFilePtr	 f,
				char		*buffer,
				int		 request_size);
extern	BufFilePtr	_DtHelpCeBufFileRdWithFd (
				int		fd);
extern	BufFilePtr	_DtHelpCeBufFileRdRawZ (
				CECompressInfoPtr  file_info);
extern	BufFilePtr	_DtHelpCeCreatePipeBufFile (
				FILE		*stream);
extern	int		_DtHelpCeUncompressFile (
				char		*in_file,
				char		*out_file);
#endif /* _NO_PROTO */

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#endif /* _DtHelpbufioI_h */
