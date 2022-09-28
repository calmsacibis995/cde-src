/*
 * Program:	Solaris .h file
 *
 *              A definite derivative of MRC's work at UW.
 *
 * Sub Author:	Mark Crispin
 *		Networks and Distributed Computing
 *		Computing & Communications
 *		University of Washington
 *		Administration Building, AG-44
 *		Seattle, WA  98195
 *		Internet: MRC@CAC.Washington.EDU
 *
 * Date:	20 December 1989
 * Last Edited:	23 December 1992
 *
 * Major edits between April and Sept, 1993 by
 *
 * Copyright 1992 by the University of Washington
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appears in all copies and that both the
 * above copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the University of Washington not be
 * used in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  This software is made
 * available "as is", and
 * THE UNIVERSITY OF WASHINGTON DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED,
 * WITH REGARD TO THIS SOFTWARE, INCLUDING WITHOUT LIMITATION ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, AND IN
 * NO EVENT SHALL THE UNIVERSITY OF WASHINGTON BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE) OR STRICT LIABILITY, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include "UnixDefs.h"
#include <sys/mman.h>
#include "VALID.h"		/* the valid macro  */

/* Build parameters */

#define KODRETRY 90		/* kiss-of-death retry in seconds */
#define LOCKFILELIFETIME (5*60)	/* Secs: Lifetime of mailbox.lock file */
#define DEFAULTLOCKTIMEOUT 5    /* Secs: Timeout for waiting on mailbox.lock */
#define NOWAITLOCKTIMEOUT  2    /* Seconds for quick lock check. */
#define QUICK_TIMEOUT 2                  /* QUick timeout only */
#define FLOCKTIMEOUT 5          /* in seconds. solaris_lock() */
#define CHUNK 32768		/* read-in chunk size */
#define IMAPPAGE 4096           /* PAGE size */
#define MAXMAPPAGE (262144<<1)  /* .5 megs: Max map size for expunge */
#define BUFF_EXTRA  8           /* Used when reading a message */
#define MAXWRITE_LOOP 2         /* Only two tries before quiting */
#define SOLARISCACHEINIT 512    /* initial cache size */
#define INVALID_822_DATE -9999  /* for VALID check */
/* Command bits from bezerk_getflags() */

#define fSEEN 1
#define fDELETED 2
#define fFLAGGED 4
#define fANSWERED 8

/* chunk from Nakul Saraiya */
#define	REMAP_THRESH	8
struct chunk {
	caddr_t	chunk_base;
	size_t	chunk_size;
};

int	chunk_alloc(size_t, int, struct chunk *);
int	chunk_realloc(size_t, int, struct chunk *);
int	chunk_free(struct chunk *);

/* Stuff for sun attachments to MIME translation */

typedef struct _att_header_desc_ {
  int present;			/* T if present */
  char *att_hdr;		/* ptr to sun attachment header */
  int len;			/* its length in bytes including '\n' */
} AHDRDESC;

typedef struct _att_headers_ {
  AHDRDESC xs_data_type;	/* This MUST be first in this structure */
  AHDRDESC xs_text_type;
  AHDRDESC xs_data_desc;
  AHDRDESC xs_data_name;
  AHDRDESC xs_encd_info;
  AHDRDESC xs_char_set;
  AHDRDESC xs_cont_len;
  AHDRDESC xs_cont_lines;
  AHDRDESC xs_extension;
} ATTHDRS;
#define NATTHDRS 9		/* reflects the number in the above list */

typedef struct _part_translation_ {
  char *bop;			/* beginning of part, ie,  "----------" */
  char *boc;			/* beginning of contents */
  ATTHDRS xsathdrs;		/* x-sun-attachement headers present */
  char *mime_hdrs;		/* the mime translation of this part */
  int mime_hdr_len;		/* length of these headers as a string */
  long content_len;		/* the content length if present */
  long content_lines;		/* the number of content lines if present */
  struct _part_translation_ *next;	/* for multiparts or NIL */
} ATTPART;

typedef enum {
  NOMIMEACTION = 0,
  DOPLAINTEXT = 1,
  DOSUNATTACHMENT = 2
} mime_translation;

typedef struct _solaris_body_ {
  char *con_lenhdr;		/* current content-length header if !NIL */
  int con_lenhdrlen;		/* length of this header */
  long content_length;		/* the content length */
  char *con_typehdr;		/* current content-type header */
  int  con_typehdrlen;		/* length of this header */
  char *txthdr;			/* X-Sun-Text-Type merde */
  int txthdr_len;		/* its longeur */
  char *charset;		/* translated charset. */
  int charsetlen;		/* length of translated charset */
  char *body_ptr;		/* 29-jul-96:  body (may be BEFORE bop) */
  ATTPART *parts;		/* body part list if applicable */
} SOLARISBODY;


/* Imap4: Status string */

/* BEZERK per-message cache information */

typedef struct file_cache {
  /* Bezerk/Solaris dependent slots */
  char *header;			/* pointer to RFC 822 header */
  char *full_header;            /* pointer to FULL header for non-caching */
  unsigned long headersize;	/* size of ENTIRE header */
  unsigned long rfc822_hdrsize; /* size of rfc822 header */
  char *body;			/* pointer to message body */
  unsigned long bodysize;	/* size of message body */

  /* Solaris stuff */
  int no_status;		/* true if no status merde in file at open */
  char *header_status;		/* Where the actual status is in header VF */
  unsigned short status_chksum;	/* 1's comp checksum of status VF */
  unsigned short chksum;	/* 1's comp. checsum of this msg VF */
  unsigned long chksum_size;	/* msg len used in checksum calculation */
  short status_len;		/* length of status[] below */
  short quick_chksum_cycles;	/* number of rotations for quick checksum */
  /* STATUSLEN + XSTATUSLEN + XUIDLEN + 4) and cruft to make it even */
#define D_INDEXSTATUSTOTAL	(((10+14+17+4+1)>>1)<<1)
  char status[D_INDEXSTATUSTOTAL+2]; /* status flags */
  long content_length;		/* >= 0 if length - set by solaris_eom() */
  int broken_content_len;	/* T, content length corrupted (solaris_parse) */
  SOLARISBODY slem;		/* solaris body description */
  u_short content_type;		/* Content-Type only. Used for shortinfo */
  /* copy of this element with MIME translation of THIS  message If
   * the original message is X-Sun-Attachment.
   * 1. IF we are rewritting the mail file with the MIME translation
   * then this is NULL, and this message contains the translation.
   * 2. Otherwise, a new element is made with the translated MIME message. 
   * and THIS cache element is for the X-Sun-Attachment. The latter
   * stays in the mail file.
   * See the stream->keep_mime flag. */
  struct file_cache *mime_tr;
  int dirty;			/* = T if flags are dirty for this msg only */
  unsigned long tmp_seek_offset; /* Used in expunge */
  unsigned long hdr_offset;     /* Offset of rfc822 header from "From " */
  unsigned long original_body_offset;	/* As found in the file during the parse */
  unsigned long original_mem_len;	/* As originally in file, after mime trans */
  unsigned long body_offset;    /* Offset of body from "From " */
  unsigned long status_offset;  /* Offset of Status stuff from "From " */
  unsigned long seek_offset;	/* seek offset in FILE of this msg */
  unsigned long real_mem_len;   /* length of message in memory */
  unsigned long rfc822_size;    /* rfc822 size of msg */
  char *eom;			/* End of cached msg below in tmp buf */
  int  msg_cached;              /* True if internal data is present */
  int  hdr_cached;              /* True if only the hdr is cached */
  int  body_cached;             /* True if only the body is cached */
  int  msg_buffered;            /* True if msg buffered in volatile mem */
  int  text_trans;              /* True if a V3 to MIME text translation */
  int  nulls_found;             /* True if nulls seen in initial parse */
  struct chunk intern_buf;	/* mmap chunck for internal data */
  char *internal;		/* start of internal header and data */
} FILECACHE;
/* the pmap functions */
#define INTERNAL_BASE(x) (((FILECACHE *)(x))->intern_buf.chunk_base)
#define INTERNAL_SIZE(x) (((FILECACHE *)(x))->intern_buf.chunk_size)
#define INTERNAL_DATA(x) (&((FILECACHE *)(x))->intern_buf)
/*
 * Definintions for the index file entry
 */

/* Version 1005 */
typedef struct _index_hdr_ {
  char		index_version[5];	/* "1005" + \0 */
  char		arch[10];		/* Used to detect arch of data */
  unsigned long	nmsgs;			/* 32 bits */
  u_longlong_t	filesize;		/* 64 bit file system safe */
  ino_t		ino;			/* 32 bits */
  time_t 	modify_time;		/* 32 bits */
  time_t 	access_time;		/* 32 bits */
  unsigned long new_index;		/* 32 bits */
  long 		uid_validity;		/* 32 bits */
  long		uid_last;		/* 32 bits */
  unsigned long	checksum;		/* 32 bits */
  unsigned long mbox_size;		/* 32 bits */
} INDEXHDR;

typedef struct _index_entry_ {
  unsigned long		msgno;
  boolean_t		seen;
  boolean_t		deleted;
  boolean_t		flagged;
  boolean_t		answered;
  boolean_t		draft;
  unsigned long		uid;
  unsigned long 	headersize;	/* full header */
  unsigned long 	rfc822_hdrsize;	/* 822 header */
  unsigned long 	bodysize;	/* body length >= 0 */
  unsigned long 	body_offset;	/* offset from start of message */
  unsigned long 	status_offset;	/* offset from start of message */
  unsigned long 	real_mem_len;	/* length of message in memory  */
  unsigned long 	rfc822_size;	/* 882 size of message */
  u_longlong_t	 	seek_offset;	/* (64bits) off_t of message in file */
  unsigned short 	chksum;		/* messge checksum */
  unsigned short 	status_chksum;	/* 1's comp checksum of status VF */
  unsigned long 	chksum_size;	/*msglen used in checksum calculation*/
  short 		status_len;	/* length of status */
  short 		quick_chksum_cycles;	/* cycles to checksum */
  char 			hours;
  char			minutes;
  char			seconds;
  char 			zoccident;
  char 			zhours;
  char 			zminutes;
  char 			day;
  char 			month;
  char 			year;
} INDEXENTRY;
#define INDEXENTRYSIZE sizeof(INDEXENTRY)

/* BEZERK I/O stream local data */

typedef struct solaris_local {
  unsigned int dirty : 1;	/* disk copy needs updating */
  unsigned int pseudo_dirty : 1;/* for dirty pseudo header */
  unsigned int index_hdr_dirty : 1;/* index hdr needs updating if True  */
  unsigned int fsync;		/* sync back flags 4-sept-96 */
  int pseudo_size;		/* length of pseudo block */
  unsigned long first_msg_len;	/* length of first message */
  int fd;			/* mailbox file descriptor */
  int id;			/* index file descriptor */
  caddr_t id_mapregion;		/* location of mmap() region for index file in memory, 0 if none */
  size_t id_mapregion_size;	/* size of mmap() region for index file in memory, 0 if none */
  caddr_t mbox_mapregion;	/* location of mmap() region for mbox */
  size_t mbox_mapregion_size;   /* size of mmap() region for mbox */
  char *name;			/* local file name for recycle case */
  char *iname;			/* index name */
  int read_access;              /* True if ACCESS is readonly */
  int read_index;		/* True if using index file */
  int inbox;                    /* True if mailbox is "INBOX" */
  int fullck_done;		/* True if full cksum is done  */
  off_t filesize;		/* file size parsed */
  ino_t ino;			/* file inode number */
  uid_t uid;			/* Owner of the file */
  gid_t gid;			/* Group owner of the file */
  off_t tmp_filesize;		/* used during expunge */
  time_t filetime;		/* last file modification time */
  time_t accesstime;            /* last file access */
  unsigned long cachesize;	/* size of local cache */
  FILECACHE **msgs;		/* pointers to message-specific information */
  char *buf;			/* temporary buffer */
  unsigned long buflen;		/* current size of temporary buffer */
  struct chunk parse_buf;       /* parsing buffer */
  size_t max_pchunk;		/* max parsing chunk for current parse */
  struct chunk body_buf;        /* for caching and freeing bodies */
  struct chunk hdr_buf;         /* for caching and freeing bodies */
  struct chunk tmp_buf;		/* miscellaneous tmp data. */
  int body_data_cached;		/* true if we must free the data  */
  int tmp_data_cached;		/* true if we must free the data  */
  int hdr_data_cached;		/* true if we must free the hdr   */
} SOLARISLOCAL;

				/* global data structure    */
typedef struct solaris_globals {
  long solaris_fromwidget;
  long lock_timeout;
} SOLARIS_GLOBALS;

/* the pmap functions */
#define PARSE_BASE (((SOLARISLOCAL *) stream->local)->parse_buf.chunk_base)
#define PARSE_SIZE (((SOLARISLOCAL *) stream->local)->parse_buf.chunk_size)
#define PARSE_DATA (&((SOLARISLOCAL *) stream->local)->parse_buf)

#define BODY_SIZE (((SOLARISLOCAL *) stream->local)->body_buf.chunk_size)
#define BODY_BASE (((SOLARISLOCAL *) stream->local)->body_buf.chunk_base)
#define BODY_DATA (&((SOLARISLOCAL *) stream->local)->body_buf)

#define HDR_SIZE (((SOLARISLOCAL *) stream->local)->hdr_buf.chunk_size)
#define HDR_BASE (((SOLARISLOCAL *) stream->local)->hdr_buf.chunk_base)
#define HDR_DATA (&((SOLARISLOCAL *) stream->local)->hdr_buf)

#define TMP_SIZE (((SOLARISLOCAL *) stream->local)->tmp_buf.chunk_size)
#define TMP_BASE (((SOLARISLOCAL *) stream->local)->tmp_buf.chunk_base)
#define TMP_DATA (&((SOLARISLOCAL *) stream->local)->tmp_buf)

/* Function prototypes */

DRIVER *solaris_valid (char *name, MAILSTREAM *stream);
static int solaris_isvalid(char 	* name,
			   char 	* tmp,
			   MAILSTREAM 	* stream,
			   boolean_t 	  keepFdOpen);
long solaris_isvalid_fd (int fd,char 	* tmp);
void *solaris_parameters (MAILSTREAM *stream, long function,void *value);
void solaris_scan (MAILSTREAM *stream,char *ref,char *pat,char *contents);
void solaris_list (MAILSTREAM *stream,char *ref,char *pat);
void solaris_lsub (MAILSTREAM *stream,char *ref,char *pat);
long solaris_subscribe (MAILSTREAM *stream,char *mailbox);
long solaris_unsubscribe (MAILSTREAM *stream,char *mailbox);
long solaris_create (MAILSTREAM *stream,char *mailbox);
long solaris_delete (MAILSTREAM *stream,char *mailbox);
long solaris_rename (MAILSTREAM *stream,char *old,char *new);
MAILSTREAM *solaris_open (MAILSTREAM *stream);
void solaris_close (MAILSTREAM *stream, long options);
void solaris_fetchfast (MAILSTREAM *stream, char *sequence, long flags);
void solaris_fetchflags (MAILSTREAM *stream, char *sequence, long flags);
ENVELOPE *solaris_fetchstructure (MAILSTREAM *stream, unsigned long msgno,
				  BODY **body, long flags);
ENVELOPE *solaris_fetchenvelope(MAILSTREAM *stream,unsigned long msgno,
				long flags);
char *solaris_fetchbody (MAILSTREAM *stream, unsigned long msgno, 
			 char *sec, 
			 unsigned long *len, long flags);

char *solaris_fetchheader (MAILSTREAM *stream,unsigned long msgno, 
			   STRINGLIST *lines,unsigned long *len,long flags);
char *solaris_fetchtext (MAILSTREAM *stream,unsigned long msgno,
			 unsigned long *len,long flags);
void solaris_setflag (MAILSTREAM *stream,char *sequence,char *flag, 
		      long flags);
void solaris_clearflag (MAILSTREAM *stream,char *sequence,char *flag, 
			long flags);
long solaris_ping (MAILSTREAM *stream);
void solaris_check (MAILSTREAM *stream);
void solaris_expunge (MAILSTREAM *stream);
long solaris_copy (MAILSTREAM *stream,char *sequence,char *mailbox, 
		    long options); 
int solaris_copy_messages (MAILSTREAM *stream, char *mailbox);
long solaris_append(MAILSTREAM 	* stream,
		    char 	* mailbox,
		    char 	* flags,
		    char 	* date,
		    char	* xUnixFrom,
		    STRING 	* message);
long solaris_append_putc (int fd,char *s,int *i,char c); 
/* SUN Imap4 error message */
int solaris_invalid_error(MAILSTREAM *stream, char *mailbox, char *cmd, int inboxok);

void solaris_gc (MAILSTREAM *stream,long gcflags);
char *solaris_pseudo (MAILSTREAM *stream,char *hdr);
void solaris_fetchshort(MAILSTREAM *stream, unsigned long msgno, SHORTINFO *sinfo, 
			long flags);
/* Two optimizations for low bandwidth flags operations */
int solaris_set1flag (MAILSTREAM *stream, char *sequence,
		      char *oneflag, long flags);
int solaris_clear1flag (MAILSTREAM *stream, char *sequence,
			char *oneflag, long flags);

/* Gets the sender from the hidden "From lpl@hk.as" that prefixes each msg */
char *solaris_fab_from (MAILSTREAM *stream, long msgno);
int solaris_lock (MAILSTREAM *stream, char *file,int flags,int mode,
			     char *lock,int op,int quick);
int solaris_nowait_lock (MAILSTREAM *stream,char *file,int flags,int mode,char *lock,int op);
void solaris_abort (MAILSTREAM *stream,int rm_index);
void solaris_unlock (int fd,MAILSTREAM *stream,char *lock);
int solaris_parse (MAILSTREAM *stream,char *lock,int op,int parsecall,
		   char *caller,char **perr);
int solaris_write_pseudo_msg (MAILSTREAM *stream, int fd, int *pseudo_size);
void solaris_save (MAILSTREAM *stream,int fd);
char *solaris_pseudo (MAILSTREAM *stream,char *hdr);
long solaris_write_message (int fd, int *e, FILECACHE *m);
int solaris_extend (MAILSTREAM *stream,int fd,char *error, off_t filesize);
void solaris_parse_msg (ENVELOPE **en,BODY **bdy,char *s,unsigned long i,
			STRING *bs,char *host,char *tmp, int trashed,
			MAILSTREAM *stream);
			/* NOT IN */
			         	    /* BEZERK */
static unsigned long data_buflen(unsigned long len,int chunk);

/* 1. Checksumming */
void solaris_checksum(MAILSTREAM *stream);
int solaris_validchecksum(MAILSTREAM *stream,
			  unsigned short checksum,
			  unsigned long mbox_size);
int solaris_checksum_by_msg(MAILSTREAM *stream, 
			   unsigned short *chksum, 
			   unsigned long *size,
			   unsigned long size_check);
int solaris_checksum_msg(MAILSTREAM *stream, unsigned long i,int fd,
			 char *lock,int resize);
void solaris_free_fcache(MAILSTREAM *stream, unsigned long i);
int solaris_readhdr(MAILSTREAM *stream,int fd,FILECACHE *m,char *buf,
		    char *lock);
void solaris_rfc822_size (MAILSTREAM *stream,FILECACHE *m,
			  char *msg,unsigned long msglen);

void solaris_resize_inbuf(MAILSTREAM *stream, unsigned long size);
int solaris_buffer_msg(MAILSTREAM *stream,int fd,FILECACHE *m,char *lock,
		       int original);
int solaris_acl_header(MAILSTREAM *stream,FILECACHE *m);
int solaris_acl_body(MAILSTREAM *stream,FILECACHE *m);
void solaris_free_body(MAILSTREAM *stream,unsigned long msgno);
char *solaris_get(unsigned long size,struct chunk *cp);
char *solaris_get_internal(unsigned long osize,struct chunk *cp);
int solaris_give(struct chunk *cp);
void solaris_resize (struct chunk *cp,unsigned long nsize);
static void solaris_chunk_init(void);
void solaris_global_init(MAILSTREAM *stream);
void solaris_global_free(MAILSTREAM *stream);
