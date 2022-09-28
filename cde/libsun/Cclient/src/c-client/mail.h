/*
 * @(#)mail.h	1.8 97/06/11
 *
 * Program:	Mailbox Access routines
 *
 * Author:	Mark Crispin
 *		Networks and Distributed Computing
 *		Computing & Communications
 *		University of Washington
 *		Administration Building, AG-44
 *		Seattle, WA  98195
 *		Internet: MRC@CAC.Washington.EDU
 *
 * Date:	22 November 1989
 * Last Edited:	Jan. 1997  
 *
 * Copyright 1995 by the University of Washington
 *
 *  Permission to use, copy, modify, and distribute this software and its
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
 * 
 * 23-aout-96   add OP_SESSIONLOCKF,stream->lockf_session
 * 25-aout-96   add driver fetch_envelope 
 *              (used only for solaris driver to avoid body parse)
 * 26-aout-96   OP_NOKODSIGNAL used by pop to terminate competing
 *              sessions.
 * 09-23-96	add local parameter "lasts" in mail_criteria_date and
 *		mail_criteria_string which is used for strtok_r. 
 * 11-aout-96   TYPENOTSET (u_short)(-1)
 * 26-octo-96   OP_NOUSERSIGS prevents sending usr1,usr2 signals on open.
 * [10-21-96]   MT-safe features:
  *		New routines: 	mail_stream_create.
 *				mail_stream_set.
 *				mail_stream_unset.
 *				imapd_global_init.
 *
 *		Defined the following new data structures:
 *		ENV_GLOBALS,
 *		SOLARIS_GLOBALS,
 *		IMAP_GLOBALS,
 *		MAIL_GLOBALS,
 *		TCP_GLOBALS,
 *		IMAPD_GLOBALS,
 *
 *		Passing MAILSTREAM stream in the following routines:
 *		sm_subscribe 
 * 		sm_unsubscribe 
 *		sm_read
 *		DRIVER->valid.
 *		mail_cached_filename
 *		mail_restore_cache
 *		mail_checkpoint_filename
 *		mail_restore_cpcache
 *		auth_link
 *		mm_diskerror
 *		mm_fatal
 *	
 *		Added the following new fileds in MAILSTREAM:
 *		
 * 		unsigned int stream_status
 * 		MAIL_GLOBALS *mail_globals
 * 		IMAP_GLOBALS *imap_globals
 * 		ENV_GLOBALS *env_globals
 * 		SOLARIS_GLOBALS *solaris_globals
 * 		TCP_GLOBALS *tcp_globals
 * 		void *dummy_globals
 *
 *
 *		Added the following new fileds in DRIVER:
 *		*(void *) global_init
 *
 *		stream_status bit usage:
 *		Normally this bit is declared to be true (T) when
 *		the stream is openned. In the case where a NIL stream
 *		is needed to pass to a funtion, this bit is set to
 *		NIL. The bit is tested instead of the stream itself
 *		inside the function. Upon exiting the function call
 *		the bit is reset to be true (T) again IF it was T
 *		prior to the function call.
 *
 * [11-04-96] imapd_global_init defined in mail.c 
 * [12-18-96] rename stream_userdata to userdata and remove macros.
 * [12-20-96] Remove imapd_global_init from mail.h. 
 * [12-23-96] Remove tcp_globals from mail.h. 
 * [12-23-96] Move tcptimeout_t definition to tcp_sv5.h. 
 * [12-23-96] Remove env_globals from mail.h. 
 * [12-30-96] Remove record_globals from mail.h. 
 * [12-31-96] Remove imap_globals and solaris_globals. 
 * [01-01-97] No need to return a stream in global_init. 
 * [01-01-97] Add a new routine mail_global_free. 
 * [01-07-97] Pass the last five parameters from mail_open to
 *	      mail_stream_create. 
 * [01-09-97] Remove imapd_globals from MAILSTREAM.
 *
 */

#ifndef _MAIL_H_
#define _MAIL_H_
#include "UnixDefs.h"
#include <sys/types.h>
#include <libintl.h>
#include <sys/types.h>
#include "tcp_sv5.h"

#ifdef __cplusplus
extern "C" {
#endif
/* Build parameters */


#define CACHEINCREMENT 100	/* cache growth increments */
#define MAILTMPLEN 1024		/* size of a temporary buffer */
#define MAILSEARCHBUFLEN 64000  /* size of search buffer length */
#define MAXMESSAGESIZE 65000	/* MS-DOS: maximum text buffer size
				 * other:  initial text buffer size */
#define NUSERFLAGS 30		/* # of user flags (current servers 30 max) */
#define BASEYEAR 1969		/* the year time began */
				/* default for unqualified addresses */
#define SMALLBUFLEN 256
#define BADHOST ".MISSING-HOST-NAME."
#define MAXDIGSPERMSGNO 10      /* For 32 bit longs */

/* Coddle certain compilers' 6-character symbol limitation */

/* Constants */

#define NIL 0			/* convenient name */
#define T _B_TRUE		/* opposite of NIL */
#define LONGT (long) _B_TRUE	/* long T */

#define  WARN  1
#define  ERROR 2
#define  PARSE 3
#define  BYE   4

typedef enum {
  WARN_t  = WARN,	/* mm_log warning type */
  ERROR_t = ERROR,	/* mm_log error type */
  PARSE_t = PARSE,	/* mm_log parse error type */
  BYE_t   = BYE			/* mm_notify stream dying */
} mm_log_t;

#define DELIM '\377'		/* strtok delimiter character */


/* Bits from mail_parse_flags().  Don't change these, since the header format
 * used by tenex, mtx, dawz, and tenexdos corresponds to these bits.
 */

#define fSEEN 1
#define fDELETED 2
#define fFLAGGED 4
#define fANSWERED 8
#define fOLD 16
#define fDRAFT 32


/* Bits for mm_list() and mm_lsub() */

#define LATT_NOINFERIORS (long) (1<<0)
#define LATT_NOSELECT (long) (1<<1)
#define LATT_MARKED (long)  (1<<2)
#define LATT_UNMARKED (long)(1<<3)
#define LATT_IMAP2BIS (long)(1<<4)	/* Used find all.mailboxes */


/* Global and Driver Parameters */

	/* 1xx: c-client globals */
#define GET_DRIVERS (long) 101
#define SET_DRIVERS (long) 102
#define GET_GETS (long) 103
#define SET_GETS (long) 104
#define GET_CACHE (long) 105
#define SET_CACHE (long) 106
#define GET_KERBEROS_V4 (long) 107
#define SET_KERBEROS_V4 (long) 108
#define GET_GSSAPI (long) 109
#define SET_GSSAPI (long) 110
#define GET_SKEY (long) 111
#define SET_SKEY (long) 112
#define SET_USERDATA (long)113
#define GET_USERDATA (long)114
#define SET_SAVECACHEPATH (long)115
#define GET_SAVECACHEPATH (long)116
	/* 2xx: environment */
#define GET_USERNAME (long) 201
#define SET_USERNAME (long) 202
#define GET_HOMEDIR (long) 203
#define SET_HOMEDIR (long) 204
#define GET_LOCALHOST (long) 205
#define SET_LOCALHOST (long) 206
#define GET_SYSINBOX (long) 207
#define SET_SYSINBOX (long) 208
	/* 3xx: TCP/IP */
#define GET_OPENTIMEOUT (long) 300
#define SET_OPENTIMEOUT (long) 301
#define GET_READTIMEOUT (long) 302
#define SET_READTIMEOUT (long) 303
#define GET_WRITETIMEOUT (long) 304
#define SET_WRITETIMEOUT (long) 305
#define GET_CLOSETIMEOUT (long) 306
#define SET_CLOSETIMEOUT (long) 307
#define GET_TIMEOUT (long) 308
#define SET_TIMEOUT (long) 309
	/* 4xx: network drivers */
#define GET_MAXLOGINTRIALS (long) 400
#define SET_MAXLOGINTRIALS (long) 401
#define GET_LOOKAHEAD (long) 402
#define SET_LOOKAHEAD (long) 403
#define GET_IMAPPORT (long) 404
#define SET_IMAPPORT (long) 405
#define GET_PREFETCH (long) 406
#define SET_PREFETCH (long) 407
#define GET_LOGINFULLNAME (long) 408
#define SET_LOGINFULLNAME (long) 409
#define GET_CLOSEONERROR (long) 410
#define SET_CLOSEONERROR (long) 411
#define GET_POP3PORT (long) 412
#define SET_POP3PORT (long) 413
#define GET_UIDLOOKAHEAD (long) 414
#define SET_UIDLOOKAHEAD (long) 415
	/* 5xx: local file drivers */
#define GET_MBXPROTECTION (long) 500
#define SET_MBXPROTECTION (long) 501
#define GET_DIRPROTECTION (long) 502
#define SET_DIRPROTECTION (long) 503
#define GET_LOCKPROTECTION (long) 504
#define SET_LOCKPROTECTION (long) 505
#define GET_FROMWIDGET (long) 506
#define SET_FROMWIDGET (long) 507
#define GET_NEWSACTIVE (long) 508
#define SET_NEWSACTIVE (long) 509
#define GET_NEWSSPOOL (long) 510
#define SET_NEWSSPOOL (long) 511
#define GET_NEWSRC (long) 512
#define SET_NEWSRC (long) 513
#define GET_EXTENSION (long) 514
#define SET_EXTENSION (long) 515
#define GET_DISABLEFCNTLLOCK (long) 516
#define SET_DISABLEFCNTLLOCK (long) 517
#define GET_LOCKEACCESERROR (long) 518
#define SET_LOCKEACCESERROR (long) 519
#define GET_LISTMAXLEVEL (long) 520
#define SET_LISTMAXLEVEL (long) 521
#define SET_EGID         (long) 522
#define GET_EGID         (long) 523
	/* 6xx: record driver  */
#define SET_LOCKTIMEOUT  (long)602

/* Driver flags */

#define DR_LOCAL   (long)(1<<1)		/* local file driver */
#define DR_MAIL    (long)(1<<2)		/* supports mail */
#define DR_NEWS    (long)(1<<3)		/* supports news */
#define DR_READONLY (long)(1<<4)	/* driver only allows readonly access */
#define DR_NOFAST   (long)(1<<5)	/* "fast" data is slow (whole msg fetch) */
#define DR_NAMESPACE (long)(1<<6)	/* driver accepts namespace format names */
#define DR_LOWMEM (long)(1<<7)		/* low amounts of memory available */
#define DR_NOSTREAM (long)(1<<9)	/* supports non-stream ops like list */

/* Open options */

#define OP_DEBUG (long)      (1<<0)     /* debug protocol negotiations */
#define OP_READONLY (long)   (1<<1)	/* read-only open */
#define OP_ANONYMOUS (long)  (1<<2)	/* anonymous open of newsgroup */
#define OP_SHORTCACHE (long) (1<<3)	/* short (elt-only) caching */
#define OP_SILENT (long)     (1<<4)	/* don't pass up events (internal use) */
#define OP_PROTOTYPE (long)  (1<<5)	/* return driver prototype */
#define OP_HALFOPEN (long)   (1<<6)	/* half-open (IMAP connect but no select) */
#define OP_EXPUNGE (long)    (1<<7)	/* silently expunge recycle stream */
/* SUN IMAP4 -- 12 June 194 */
#define OP_KEEP_MIME (long)  (1<<11)    /* write MIME trans. back to mail files */
#define OP_SECONDARY_MBOX (long)(1<<12) /* not the inbox */
#define OP_SUNVERSION     (long)(1<<13) /* sets sunvset in the stream at open */
#define OP_SERVEROPEN     (long)(1<<14) /* Server is doing the open if set */
#define OP_SESSIONLOCKF   (long)(1<<15)	/* use lockf for session too */
#define OP_NOKODSIGNAL    (long)(1<<16) /* NO wait for KOD on session lock */
#define OP_NOUSERSIGS     (long)(1<<17) /* Do not send user signals on open */
#define KEEPMIMEDEFAULT T               /* default is write MIME back to mail file */

/* Checksum types - used by imapserver calls */
#define OPENCHECKSUM   (1<<0)
#define FASTCHECKSUM   (1<<1)
#define STATUSCHECKSUM (1<<2)
#define PARSECALL      (1<<3)
#define SERVERCALL     (1<<4)
/* Acknowledge timeout for server */

#define ACKTIME 5000
/* Close options */

#define CL_EXPUNGE (long) 1	/* expunge silently */


/* Fetch options */

#define FT_UID (long) (1<<0)	/* argument is a UID */
#define FT_PEEK (long)(1<<1)	/* peek at data */
#define FT_NOT (long)(1<<2)	/* NOT flag for header lines fetch */
#define FT_INTERNAL (long)(1<<3)/* text can be internal strings */
#define FT_RSEARCH (long)(1<<4)	/* fetch for record search */
#define FT_CACHESYNC (long)(1<<5)/* syncing cache */
#define FT_ZEROCOUNT (long)(1<<6)/* zero io_byte counter for tcp */
#define FT_SHORTENV (long)(1<<7) /* fetch shortinfo and envelope */
/* Store options */

#define ST_UID (long) 1		/* argument is a UID sequence */
#define ST_SILENT (long) 2	/* don't return results */


/* Copy options */

#define CP_UID (long) 1		/* argument is a UID sequence */
#define CP_MOVE (long) 2	/* delete from source after copying */


/* Search options */

#define SE_UID (long) 1		/* return UID */
#define SE_FREE (long) 2	/* free search program after finished */
#define SE_NOPREFETCH (long) 4	/* no search prefetching */
#define SE_SERVER (long) 8      /* imap server search */
/* Status options */

#define SA_MESSAGES (long) 1	/* number of messages */
#define SA_RECENT (long) 2	/* number of recent messages */
#define SA_UNSEEN (long) 4	/* number of unseen messages */
#define SA_UIDNEXT (long) 8	/* next UID to be assigned */
#define SA_UIDVALIDITY (long) 16/* UID validity value */
#define SA_CHECKSUM (long) 32   /* Checksum value */
#define SA_CKSUMONLY (long) 64  /* Use status call to get checksum */
/* Cache management function codes */

#define CH_INIT (long) 10	/* initialize cache */
#define CH_SIZE (long) 11	/* (re-)size the cache */
#define CH_LFINIT (long) 12     /* Used in replay Sun IMAP4. */
#define CH_MAKELELT (long) 20	/* return long elt, make if needed */
#define CH_LELT (long) 21	/* return long elt if exists */
#define CH_MAKEELT (long) 30	/* return short elt, make if needed */
#define CH_ELT (long) 31	/* return short elt if exists */
#define CH_FREE (long) 40	/* free space used by elt */
#define CH_FREE_ALWAYS 42       /* ALWAYS free even if locked */
#define CH_EXPUNGE (long) 45	/* delete elt pointer from list */

/* 6-jan-97: Record flags */
#define RE_STOP (long) (1<<0)	/* stop recording */

/* Garbage collection flags */

#define GC_ELT (long) 1		/* message cache elements */
#define GC_ENV (long) 2		/* envelopes and bodies */
#define GC_TEXTS (long) 4	/* cached texts */


/* Stream Status Flags */

#define S_CREATED (long)(1<<0) 
#define S_OPENED  (long)(1<<1) 
#define S_CLOSED  (long)(1<<2) 
#define S_PROTOTYPE (long)(1<<3) 
#define S_NILCALL   (long)(1<<4) 


/* Message structures */


/* Item in an address list */
	
#define ADDRESS struct mail_address
ADDRESS {
  char *personal;		/* personal name phrase */
  char *adl;			/* at-domain-list source route */
  char *mailbox;		/* mailbox name */
  char *host;			/* domain name of mailbox's host */
  char *error;			/* error in address from SMTP module */
  ADDRESS *next;		/* pointer to next address in list */
};

typedef long (*readfn_t) (void *stream,unsigned long size,char *buffer);
typedef char *(*mailgets_t) (readfn_t f,void *stream,unsigned long size);

#define STRINGLIST struct string_list

/* NEW - Global data structures on the stream.  */

typedef struct mail_globals {
	/* pointer to alternate gets function */
  mailgets_t mailgets; 
        	/* SunSoft: for stream->userdata on open */
  void *userdata;
        	/* SunSoft: for saving c-client cache */
  char *savecachepath;
		/* for sm_read in smanager.c */
  char sbname[MAILTMPLEN];
		/* stringlist for mail_search_msg */
  STRINGLIST *sstring;
		/* scharset for mail_search_msg */
  char *scharset;
} MAIL_GLOBALS;

/* Size of temporary buffers */
#define TMPLEN (4*8192)
#define LITSTKLEN 20 /* Length of literal stack (I hope 20 is plenty) */

/* Autologout timer */
#define TIMEOUT 60*30

/* Message envelope */

typedef struct mail_envelope {
  char *remail;			/* remail header if any */
  ADDRESS *return_path;		/* error return address */
  char *date;			/* message composition date string */
  ADDRESS *from;		/* originator address list */
  ADDRESS *sender;		/* sender address list */
  ADDRESS *reply_to;		/* reply address list */
  char *subject;		/* message subject string */
  ADDRESS *to;			/* primary recipient list */
  ADDRESS *cc;			/* secondary recipient list */
  ADDRESS *bcc;			/* blind secondary recipient list */
  char *in_reply_to;		/* replied message ID */
  char *message_id;		/* message ID */
  char *newsgroups;		/* USENET newsgroups */
} ENVELOPE;

/* Primary body types */
/* If you change any of these you must also change body_types in rfc822.c */

extern const char *body_types[];	/* defined body type strings */

#define TYPENOTSET (u_short)(-1)/* no type set yet. */
#define TYPETEXT 0		/* unformatted text */
#define TYPEMULTIPART 1		/* multiple part */
#define TYPEMESSAGE 2		/* encapsulated message */
#define TYPEAPPLICATION 3	/* application data */
#define TYPEAUDIO 4		/* audio */
#define TYPEIMAGE 5		/* static image */
#define TYPEVIDEO 6		/* video */
#define TYPEOTHER 7		/* unknown */
#define TYPEMAX 15		/* maximum type code */


/* Body encodings */
/* If you change any of these you must also change body_encodings in rfc822.c
 */

extern const char *body_encodings[];	/* defined body encoding strings */

#define ENC7BIT 0		/* 7 bit SMTP semantic data */
#define ENC8BIT 1		/* 8 bit SMTP semantic data */
#define ENCBINARY 2		/* 8 bit binary data */
#define ENCBASE64 3		/* base-64 encoded data */
#define ENCQUOTEDPRINTABLE 4	/* human-readable 8-as-7 bit data */
#define ENCOTHER 5		/* unknown */
#define ENCMAX 10		/* maximum encoding code */


/* Body contents */
#define BINARY void
#define BODY struct mail_body
#define MESSAGE struct mail_body_message
#define PARAMETER struct mail_body_parameter
#define PART struct mail_body_part

/* Message content (ONLY for parsed messages) */
MESSAGE {
  ENVELOPE *	env;		/* message envelope */
  BODY 	   *	body;		/* message body */
  char 	   *	hdr;		/* message header */
  unsigned long hdrsize;	/* message header size */
  char 	   *	text;		/* message in RFC-822 form */
  unsigned long offset;		/* offset of text from header */
};


/* Parameter list */

PARAMETER {
  char 	*	attribute;	/* parameter attribute name */
  char  *	value;		/* parameter value */
  PARAMETER *	next;		/* next parameter in list */
};


/* Message body structure */
BODY {
  unsigned short type;		/* body primary type */
  unsigned short encoding;	/* body transfer encoding */
  char 	*	 subtype;	/* subtype string */
  PARAMETER *	parameter;	/* parameter list */
  char	*	id;		/* body identifier */
  char *	description;	/* body description */
  void (*contents_deallocate)(void **block);  /* function to deallocate "contents" object */
  union {			/* different ways of accessing contents */
    unsigned char * text;	/* body text (+ enc. message in composing) */
    BINARY 	  * binary;	/* body binary */
    PART 	  * part;		/* body part list */
    MESSAGE 	    msg;	/* body encapsulated message (PARSE ONLY) */
  } contents;
  struct {
    unsigned long lines;	/* size in lines */
    unsigned long bytes;	/* size in bytes */
    unsigned long ibytes;	/* internal size in bytes (drivers ONLY!!) */
  } size;
  char 		 * md5;		/* MD5 checksum */
};


/* Multipart content list */
PART {
  BODY 		  body;		/* body information for this part */
  unsigned long   offset;	/* offset from body origin */
  PART 		* next;		/* next body part */
};

#define STRINGLIST struct string_list

/* Entry in the message cache array:
 * ALWAYS LEAVE msgno as the first entry */
typedef struct message_cache {
  unsigned long msgno;		/* message number: MUST BE FIRST CACHE ENTRY*/
  unsigned long sync_msgno;     /* used when synchronizing the cache */
  unsigned long uid;		/* message unique ID */

  /*
   * The next 8 bytes is ordered in this way so that it will be reasonable even
   * on a 36-bit machine.  Maybe someday I'll port this to TOPS-20.  ;-)
   */
  /* internal time/zone, system flags (4 bytes) */
  unsigned int hours: 5;	/* hours (0-23) */
  unsigned int minutes: 6;	/* minutes (0-59) */
  unsigned int seconds: 6;	/* seconds (0-59) */

  /*
   * It may seem easier to have zhours be signed.  Unfortunately, a certain
   * C compiler from a well-known software vendor in Redmond, WA
   * does not allow signed bit fields.
   */
  unsigned int zoccident : 1;	/* non-zero if west of UTC */
  unsigned int zhours    : 4;	/* hours from UTC (0-12) */
  unsigned int zminutes  : 6;	/* minutes (0-59) */
  unsigned int seen      : 1;	/* system Seen flag */
  unsigned int deleted   : 1;	/* system Deleted flag */
  unsigned int flagged   : 1; 	/* system Flagged flag */
  unsigned int answered  : 1;	/* system Answered flag */

  /* flags, lock count (2 bytes) */
  unsigned int draft     : 1;	/* system Draft flag */
  unsigned int valid     : 1;	/* elt has valid flags */
  unsigned int recent    : 1;	/* message is new as of this mailbox open */
  unsigned int searched  : 1;	/* message was searched */
  unsigned int sequence  : 1;	/* (driver use) message is in sequence */
  unsigned int spare     : 1;	/* reserved for use by main program */
  unsigned int spare2    : 1;	/* reserved for use by main program */
  unsigned int spare3    : 1;	/* reserved for use by main program */
  unsigned int lockcount : 8;	/* non-zero if multiple references */

  /* internal date (2 bytes) */
  unsigned int  day 	 : 5;		/* day of month (1-31) */
  unsigned int  month 	 : 4;	/* month of year (1-12) */
  unsigned int  year 	 : 7;	/* year since 1969 (expires 2097) */
  unsigned long user_flags;	/* user-assignable flags */
  unsigned long rfc822_size;	/* # of bytes of message as raw RFC822 */
  unsigned long data1;		/* (driver use) first data item */
  unsigned long data2;		/* (driver use) second data item */
  unsigned long data3;		/* (driver use) third data item */
  unsigned long data4;		/* (driver use) fourth data item */
  /* Imap4 - Sun additions */
  unsigned long filter1;	/* for filtered header */
  unsigned long filter2;	/* for filter header size */
  STRINGLIST *	lines;          /* header.lines fetch */
} MESSAGECACHE;

/* SUN IMAP4 -- */
typedef struct short_information {
  char 	* date;			/* internal date */
  char 	* personal;		/* personal name */
  char 	* mailbox;		/* the mailbox */
  char 	* host;			/* the host */
  char 	* subject;		/* subject */
  char 	* bodytype;		/* body type, text, multipart,etc ...*/
  char 	* size;			/* rfc822.size */
  char 	* from;			/* made from "personal" <mailbox@host> above */
} SHORTINFO;
#define NSHORTSLOTS 8		/* number of slots above */

typedef struct long_cache {
  MESSAGECACHE elt;
  ENVELOPE *   env;		/* pointer to message envelope */
  BODY 	   *   body;		/* pointer to message body */
  /* SUN IMAP4 */
  SHORTINFO 	sinfo;		/* structure for short info */
} LONGCACHE;


/* Mailbox status structure */
typedef struct mbx_status {
  long 		flags;		/* validity flags */
  long 		messages;	/* number of messages */
  long 		recent;		/* number of recent messages */
  long 		unseen;		/* number of unseen messages */
  long 		uidnext;	/* next UID to be assigned */
  long 		uidvalidity;	/* UID validity value */
  unsigned short checksum;      /* checksum value */
  unsigned long checksum_bytes; /* n bytes checksummed */
} MAILSTATUS;


/* String list */
STRINGLIST {
  char 	*	text;		/* string text */
  unsigned long size;		/* string length */
  STRINGLIST *	next;
};

/* Header List */
typedef struct header_list {
  STRINGLIST *	fields;		/* Name of the header field */
  STRINGLIST *	data;		/* Content of the header field */
  unsigned long n_entries;      /* Number of headers */
} HEADERLIST;
  

/* String structure */
#define STRINGDRIVER struct string_driver
typedef struct mailstring {
  void 		* data;		/* driver-dependent data */
  unsigned long   data1;	/* driver-dependent data */
  unsigned long   size;		/* total length of string */
  char 		* chunk;	/* base address of chunk */
  unsigned long   chunksize;	/* size of chunk */
  unsigned long   offset;	/* offset of this chunk in base */
  char 		* curpos;	/* current position in chunk */
  unsigned long   cursize;	/* number of bytes remaining in chunk */
  STRINGDRIVER  * dtb;		/* driver that handles this type of string */
} STRING;


/* Dispatch table for string driver */
STRINGDRIVER {

  /* initialize string driver */
  void (* init)(STRING * s, void * data, unsigned long size);

  /* get next character in string */
  char (* next)(STRING * s);

  /* set position in string */
  void (* setpos)(STRING * s, unsigned long i);
};


/* Stringstruct access routines */
#define INIT(s,d,data,size) ((*((s)->dtb = &d)->init) (s,data,size))
#define SIZE(s) ((s)->size - GETPOS (s))
#define CHR(s) (*(s)->curpos)
#define SNX(s) (--(s)->cursize ? *(s)->curpos++ : (*(s)->dtb->next) (s))
#define GETPOS(s) ((s)->offset + ((s)->curpos - (s)->chunk))
#define SETPOS(s,i) (*(s)->dtb->setpos) (s,i)

/* Search program */
#define SEARCHPGM struct search_program
#define SEARCHHEADER struct search_header
#define SEARCHSET struct search_set
#define SEARCHOR struct search_or
#define SEARCHPGMLIST struct search_pgm_list


SEARCHHEADER {			/* header search */
  char *	line;		/* header line */
  char *	text;		/* text in header */
  SEARCHHEADER *next;		/* next in list */
};


SEARCHSET {			/* message set */
  unsigned long first;		/* sequence number */
  unsigned long last;		/* last value, if a range */
  SEARCHSET *	next;		/* next in list */
};


SEARCHOR {
  SEARCHPGM *	first;		/* first program */
  SEARCHPGM *	second;		/* second program */
  SEARCHOR  *	next;		/* next in list */
};


SEARCHPGMLIST {
  SEARCHPGM 	* pgm;		/* search program */
  SEARCHPGMLIST * next;		/* next in list */
};

SEARCHPGM {			/* search program */
  SEARCHSET 	* msgno;	/* message numbers */
  SEARCHSET 	* uid;		/* unique identifiers */
  SEARCHOR 	* or;		/* or'ed in programs */
  SEARCHPGMLIST * not;		/* and'ed not program */
  SEARCHHEADER  * header;	/* list of headers */
  STRINGLIST 	* bcc;		/* bcc recipients */
  STRINGLIST 	* body;		/* text in message body */
  STRINGLIST 	* cc;		/* cc recipients */
  STRINGLIST 	* from;		/* originator */
  STRINGLIST 	* keyword;	/* keywords */
  STRINGLIST 	* unkeyword;	/* unkeywords */
  STRINGLIST 	* subject;	/* text in subject */
  STRINGLIST 	* text;		/* text in headers and body */
  STRINGLIST 	* to;		/* to recipients */
  unsigned long   larger;	/* larger than this size */
  unsigned long   smaller;	/* smaller than this size */
  unsigned short  sentbefore;	/* sent before this date */
  unsigned short  senton;	/* sent on this date */
  unsigned short  sentsince;	/* sent since this date */
  unsigned short  before;	/* before this date */
  unsigned short  on;		/* on this date */
  unsigned short  since;		/* since this date */
  unsigned int    answered   : 1;	/* answered messages */
  unsigned int    unanswered : 1;	/* unanswered messages */
  unsigned int    deleted    : 1;	/* deleted messages */
  unsigned int    undeleted  : 1;	/* undeleted messages */
  unsigned int 	  draft      : 1;	/* message draft */
  unsigned int 	  undraft    : 1;	/* message undraft */
  unsigned int 	  flagged    : 1;	/* flagged messages */
  unsigned int	  unflagged  : 1;	/* unflagged messages */
  unsigned int 	  recent     : 1;	/* recent messages */
  unsigned int 	  old        : 1;	/* old messages */
  unsigned int 	  seen       : 1;	/* seen messages */
  unsigned int 	  unseen     : 1;	/* unseen messages */
  unsigned int 	  only_all   : 1;    	/* Set if ONLY all search */
};

/* Mail Access I/O stream */

/* Structure for mail driver dispatch */
#define DRIVER struct driver	


/* Mail I/O stream */
typedef struct mail_stream {
  DRIVER 	*  dtb;		/* dispatch table for this driver */
  void 		*  local;	/* pointer to driver local data */
  char 		*  mailbox;	/* mailbox name */
  unsigned short   use;		/* stream use count */
  unsigned short   sequence;	/* stream sequence */
  unsigned int 	   lock          : 1;	/* stream lock flag */
  unsigned int 	   debug         : 1;	/* stream debug flag */
  unsigned int 	   silent        : 1;	/* silent stream from Tenex */
  unsigned int 	   rdonly        : 1;	/* stream read-only flag */
  unsigned int 	   anonymous     : 1;	/* stream anonymous access flag */
  unsigned int 	   scache        : 1;	/* stream short cache flag */
  unsigned int 	   halfopen      : 1;	/* stream half-open flag */
  unsigned int 	   perm_seen     : 1;	/* permanent Seen flag */
  unsigned int 	   perm_deleted  : 1;	/* permanent Deleted flag */
  unsigned int 	   perm_flagged  : 1;	/* permanent Flagged flag */
  unsigned int 	   perm_answered : 1;	/* permanent Answered flag */
  unsigned int 	   perm_draft 	 : 1;	/* permanent Draft flag */
  unsigned int 	   kwd_create 	 : 1;	/* can create new keywords */
  unsigned long    perm_user_flags;	/* mask of permanent user flags */
  unsigned long    gensym;		/* generated tag */
  unsigned long    nmsgs;		/* # of associated msgs */
  unsigned long    recent;		/* # of recent msgs */
  unsigned long    cached_nmsgs;/* number of messages when we disconnected */
  unsigned long    uid_validity;	/* UID validity sequence */
  unsigned long    uid_last;		/* last assigned UID */
  char 		*  flagstring;		/* buffer of user keyflags */
  char 		*  user_flags[NUSERFLAGS];/* pointers to user flags in bit order */
  unsigned long    cachesize;		/* size of message cache */
  union {
    void 	 ** c;		/* to get at the cache in general */
    MESSAGECACHE ** s;		/* message cache array */
    LONGCACHE 	 ** l;		/* long cache array */
  } cache;
  unsigned long   msgno;	/* message number of `current' message */
  ENVELOPE 	* env;		/* pointer to `current' message envelope */
  BODY 		* body;		/* pointer to `current' message body */
  char 		* text;		/* pointer to `current' text */
  /* !!!! Sun IMAP4 parameters */
  void 		* userdata;	/* pointer to arbitrary data */
  /* for the short attributes */
  SHORTINFO 	  sinfo;	/* structure for short info */
  /* Indicates if in record mode */
  unsigned long   checksum;	/* checksum value */
  unsigned long   mbox_size;	/* size of checksummed mailbox */
  char 		* driver_name;  /* if we are talking to imap2bis server */
  int 		  validchecksum;/* set T if current checksum is valid */
  int 		  checksum_type;/* OPENCHECKSUM, OTHERCHECKSUM */
  int 		  sunvset;	/* true if server has set sunversion */
  int 		  send_checksum;/* true if server must send a new checksum */
  int 		  mail_send_urgent;/* true if c-client permits urgent data */
  int 		  imap_send_urgent;/* true if mail_send_urgent enables it */
  int 		  input_flushed;   /* true if input was flushed */
  long 		  min_dirty_msgno;/* smallest dirty msg no  */
  int 		  keep_mime;	  /* true if keeping MIME translations */
  int 		  have_all_sinfo; /* true if ALL shortinfo cached */
  int 		  secondary_mbox; /* not the inbox */
  char 		* sync_uidseq;	  /* used for resynchronization */
  int  		  dead;           /* set for fatalities */
  int 		  hierarchy_dlm;  /* hierarchy delimiter */
  int 		  imap4_connected;/* true if we connected to an imap4 server */
  int 		  server_open;  /* true only if server is doing the open */
  int 		  session_lockf;/* true if OP_SESSIONLOCKF on open */
  int 		  nokodsig;	/* true if OP_NOKODSIGNAL on open */
  int 		  nousersig;    /* true if OP_NOUSERSIGS on open */
  int 		  rcvkor;	/* kiss of read only received x*/
  /* Function calls registered by client a open */

  int (*set_ttlock)(char *mailbox,void *udata); /* set tool talk locking */
  int (*clear_ttlock)(char *mailbox,void *udata); /* clear tool talk locking */
  int (*set_group_access)(void); /* set/clear group access */
  int (*clear_group_access)(void); /* clear group access */

  unsigned int stream_status;     /* a stream status flag */
  void *mail_globals;      	  /*a place to hide all globals */
  void *record_globals;
  void *imap_globals;
  void *env_globals;
  void *solaris_globals;
  void *tcp_globals;
  void *dummy_globals;

} MAILSTREAM;
#include "tcp.h"


/* Mail I/O stream handle */
typedef struct mail_stream_handle {
  MAILSTREAM *	 stream;	/* pointer to mail stream */
  unsigned short sequence;	/* sequence of what we expect stream to be */
} MAILHANDLE;

/* Mail driver dispatch */

DRIVER {
  char 	 *	name;		/* driver name */
  unsigned long flags;		/* driver flags */
  DRIVER *	next;		/* next driver */

  /* mailbox is valid for us */
  DRIVER *(*valid) (char *mailbox, MAILSTREAM *stream);

  /* manipulate driver parameters */
  void *(*parameters) (MAILSTREAM *stream, long function,void *value);

  /* scan mailboxes */
  void (*scan) (MAILSTREAM *stream,char *ref,char *pat,char *contents);

  /* list mailboxes */
  void (*list) (MAILSTREAM *stream,char *ref,char *pat);

  /* list subscribed mailboxes */
  void (*lsub) (MAILSTREAM *stream,char *ref,char *pat);

  /* subscribe to mailbox */
  long (*subscribe) (MAILSTREAM *stream,char *mailbox);

  /* unsubscribe from mailbox */
  long (*unsubscribe) (MAILSTREAM *stream,char *mailbox);

  /* create mailbox */
  long (*create) (MAILSTREAM *stream,char *mailbox);

  /* delete mailbox */
  long (*mbxdel) (MAILSTREAM *stream,char *mailbox);

  /* rename mailbox */
  long (*rename) (MAILSTREAM *stream,char *old,char *newname);

  long (*status) (MAILSTREAM *stream,char *mbx,long flags,void *userdata);


  /* open mailbox */
  MAILSTREAM *(*open) (MAILSTREAM *stream);

  /* close mailbox */
  void (*close) (MAILSTREAM *stream,long options);

  /* fetch message "fast" attributes */
  void (*fetchfast) (MAILSTREAM *stream,char *sequence,long flags);

  /* fetch message flags */
  void (*fetchflags) (MAILSTREAM *stream,char *sequence,long flags);

  /* fetch message envelopes */
  ENVELOPE *(*fetchstructure) (MAILSTREAM *stream,unsigned long msgno,
			       BODY **body,long flags);

  /* fetch message header only */
  char *(*fetchheader) (MAILSTREAM *stream,unsigned long msgno,
			STRINGLIST *lines,unsigned long *len,long flags);
  
  /* fetch message body only */
  char *(*fetchtext) (MAILSTREAM *stream,unsigned long msgno,
		      unsigned long *len,long flags);

  /* fetch message body section */
  char *(*fetchbody) (MAILSTREAM *stream,unsigned long msgno,char *sec,
		      unsigned long* len,long flags);

  /* return UID for message */
  unsigned long (*uid) (MAILSTREAM *stream,unsigned long msgno);
	
  /* set message flag */
  void (*setflag) (MAILSTREAM *stream,char *sequence,char *flag,long flags);
	
  /* clear message flag */
  void (*clearflag) (MAILSTREAM *stream,char *sequence,char *flag,long flags);
	
  /* search for message based on criteria */
  void (*search) (MAILSTREAM *stream,char *charset,SEARCHPGM *pgm,long flags);
	
  /* ping mailbox to see if still alive */
  long (*ping) (MAILSTREAM *stream);
	
  /* check for new messages */
  void (*check) (MAILSTREAM *stream);
	
  /* expunge deleted messages */
  void (*expunge) (MAILSTREAM *stream);
	
  /* copy messages to another mailbox */
  long (*copy) (MAILSTREAM *stream,char *sequence,char *mailbox,long options);
	
  /* append string message to mailbox */
  long (*append) (MAILSTREAM 	* stream,
		  char 		* mailbox,
		  char 		* flags,
		  char 		* date,
		  char		* xUnixFrom,	/* "From " folder line */
		  STRING 	* message);
	
  /* garbage collect stream */
  void (*gc) (MAILSTREAM *stream,long gcflags);
  
  /* 
   *  SUN Imap4 additions here
   */
  char *(*fabricate_from) (MAILSTREAM *stream, long msgno);
  int (*set1flag) (MAILSTREAM *stream,char *sequence,char *flag,long flags);
	
  /* clear message flag */
  int (*clear1flag) (MAILSTREAM *stream,char *sequence,char *flag,long flags);
	
  /* send sunversion command */
  int (*sunversion) (MAILSTREAM *stream);
	
  /* disconnect a stream */
  void (*disconnect) (MAILSTREAM *stream);
	
  /* open/close imapd connection */
  int (*serverecho) (MAILSTREAM *stream);
	
  /* clear imap io-counters */
  void (*cleario) (MAILSTREAM *stream);
	
  /* enable sending urgent signal */
  void (*urgent) (MAILSTREAM *stream);
	
  /* fetch short info */
  void (*fetchshort)(MAILSTREAM *stream, unsigned long msgno, SHORTINFO *sinfo, long flags);
	
  /* solaris_checksum  */
  void (*checksum)(MAILSTREAM *stream);
	
  /* valid checksum status */
  int (*validchecksum)(MAILSTREAM *stream, unsigned short cksum, 
		       unsigned long nbytes);
  int (*sync_msgno)(MAILSTREAM *stream, char *uidseq);
  int (*imap4_supported) (MAILSTREAM *stream);
  void (*driver_freebody) (MAILSTREAM *stream,unsigned long msgno);
  ENVELOPE *(*fetchenvelope)(MAILSTREAM *stream,unsigned long msgno,
			     long flags);

  /* routine to initialize all the driver globals */
  void (*global_init) (MAILSTREAM *stream );
  void (*global_free) (MAILSTREAM *stream );
};


/* Parse results from mail_valid_net_parse */

#define MAXSRV 20
typedef struct net_mailbox {
  char		host[MAILTMPLEN];	/* host name */
  char 		mailbox[MAILTMPLEN];	/* mailbox name */
  char 		service[MAXSRV+1];	/* service name */
  unsigned long port;			/* TCP port number */
  unsigned int  anoflag : 1;		/* anonymous */
  unsigned int  dbgflag : 1;		/* debug flag */
} NETMBX;

/* Mail delivery I/O stream */

typedef struct send_stream {
  void *	tcpstream;		/* TCP I/O stream */
  char *	reply;			/* last reply string */
  unsigned long size;		/* size limit */
  unsigned int  debug : 1;	/* stream debug flag */
  unsigned int  ok_8bitmime : 1;	/* supports 8-bit MIME */
  union {
    struct {
      unsigned int ok_ehlo : 1;	/* support ESMTP */
      unsigned int ok_send : 1;	/* ESMTP supports SEND */
      unsigned int ok_soml : 1;	/* ESMTP supports SOML */
      unsigned int ok_saml : 1;	/* ESMTP supports SAML */
      unsigned int ok_expn : 1;	/* ESMTP supports EXPN */
      unsigned int ok_help : 1;	/* ESMTP supports HELP */
      unsigned int ok_turn : 1;	/* ESMTP supports TURN */
      unsigned int ok_size : 1;	/* ESMTP supports SIZE */
    } esmtp;
    struct {
      unsigned int ok_post : 1;	/* supports POST */
    } nntp;
  } local;
} SENDSTREAM;

/* Other symbols */

extern const char * days[];	/* day name strings */
extern const char * months[];	/* month name strings */


/* Jacket into external interfaces */
typedef void *(*mailcache_t) (MAILSTREAM *stream,unsigned long msgno,long op);
typedef void *(*authchallenge_t) (void *stream,unsigned long *len);
typedef long (*authrespond_t) (void *stream,char *s,unsigned long size);
typedef long (*authclient_t) (authchallenge_t challenger,
			      authrespond_t responder,NETMBX *mb,void *s,
			      unsigned long trial);
typedef char *(*authresponse_t) (void *challenge,unsigned long clen,
		unsigned long *rlen, void *imapd_globals, MAILSTREAM *s );
typedef char *(*authserver_t) (authresponse_t responder,int argc,char *argv[], MAILSTREAM *s);


#define AUTHENTICATOR struct mail_authenticator

AUTHENTICATOR {
  char *	  name;		/* name of this authenticator */
  authclient_t 	  client;	/* client function that supports it */
  authserver_t 	  server;	/* server function that supports it */
  AUTHENTICATOR * next;		/* next authenticator */
};


/* Other symbols */

/* list of authenticators */
extern AUTHENTICATOR *authenticators;

#include "linkage.h"

/* For mail_playback */
#define FAILEDPLAYBACK      0
#define SUCCESSFULLPLAYBACK 1		/* all went well */
#define NOTHINGTOPLAYBACK   2		/* nothing valid in recording file */
#define FLUSHCACHEONLY      3		/* Playback OK, but cache flushed(sync error) */
#define PLAYBACKLOCALERROR  4		/* Playback failed. Cache flushed */
#define CANNOTPLAYBACKRO    5		/* Cache fine but cannot playback(file r/o) */

/* Additional support names */

#define mail_close(stream) mail_close_full (stream,NIL);
#define mail_fetchfast(stream,sequence) mail_fetchfast_full (stream,sequence,NIL)
#define mail_fetchflags(stream,sequence) mail_fetchflags_full (stream,sequence,NIL)
#define mail_fetchenvelope(stream,msgno) mail_fetchenvelope_full (stream,msgno,NIL)
#define mail_fetchstructure(stream,msgno,body) mail_fetchstructure_full (stream,msgno,body,NIL)
#define mail_fetchheader(stream,msgno) mail_fetchheader_full (stream,msgno,NIL,NIL,NIL)
#define mail_simple_fetchheader(stream,msgno) mail_fetchheader_full (stream,msgno,NIL,NIL,NIL) /* Sun Imap4 */
#define mail_fetchtext(stream,msgno) mail_fetchtext_full (stream,msgno,NIL,NIL)
#define mail_fetchbody(stream,msgno,section,len) mail_fetchbody_full (stream,msgno,section,len,NIL)
#define mail_setflag(stream,sequence,flag) mail_setflag_full (stream,sequence,flag,NIL)
#define mail_clearflag(stream,sequence,flag) mail_clearflag_full (stream,sequence,flag,NIL)
#define mail_search(stream,criteria) mail_search_full (stream,NIL,mail_criteria (criteria),SE_FREE);
#define mail_copy(stream,sequence,mailbox) mail_copy_full (stream,sequence,mailbox,NIL)
#define mail_move(stream,sequence,mailbox) mail_copy_full (stream,sequence,mailbox,CP_MOVE)
#define mail_append(stream,mailbox,message) mail_append_full (stream,mailbox,NULL,NULL,NULL,message)

/* SUN Imap4 */
#define mail_set1flag(stream,sequence,flag) mail_set1flag_full (stream,sequence,flag,NIL)
#define mail_clear1flag(stream,sequence,flag) mail_clear1flag_full (stream,sequence,flag,NIL)
#define mail_fetchshort(stream,msgno,user_info) mail_fetchshort_full(stream,msgno,user_info,NIL)

/* Function prototypes */
void 	mm_searched(MAILSTREAM *stream,unsigned long number);
void 	mm_exists(MAILSTREAM *stream,unsigned long number);
void 	mm_expunged(MAILSTREAM *stream,unsigned long number);
void	mm_flags(MAILSTREAM *stream,unsigned long number);
void 	mm_notify(MAILSTREAM *stream,char *string,long errflg);
void 	mm_list(MAILSTREAM *stream,char delimiter,char *name,long attributes);
void 	mm_lsub(MAILSTREAM *stream,char delimiter,char *name,long attributes);
void 	mm_status(MAILSTREAM *stream,char *mailbox,MAILSTATUS *status);
void 	mm_log(char *string, long errflg, MAILSTREAM *stream);
void 	mm_dlog(MAILSTREAM *stream,char *string);
void 	mm_login(MAILSTREAM *stream,NETMBX *mb,char *user,char *pwd,long trial);
void 	mm_critical(MAILSTREAM *stream);
void 	mm_nocritical(MAILSTREAM *stream);
long 	mm_diskerror(MAILSTREAM *stream,long errcode,long serious);
void 	mm_fatal(char *string, MAILSTREAM *stream );
char *	mm_gets(readfn_t f,void *stream,unsigned long size);
void *	mm_cache(MAILSTREAM *stream,unsigned long msgno,long op);
void 	mm_mailbox(MAILSTREAM *stream, char *name);

extern STRINGDRIVER mail_string;
void 	mail_string_init(STRING *s,void *data,unsigned long size);
char 	mail_string_next(STRING *s);
void 	mail_string_setpos(STRING *s,unsigned long i);
void 	mail_link(DRIVER *driver);
void *	mail_parameters(MAILSTREAM *stream,long function,void *value);
DRIVER *mail_valid(MAILSTREAM *stream,char *mailbox,char *purpose);
DRIVER *mail_valid_net(char *name,DRIVER *drv,char *host,char *mailbox);
long 	mail_valid_net_parse(char *name,NETMBX *mb);
void 	mail_scan(MAILSTREAM *stream,char *ref,char *pat,char *contents);
void 	mail_list(MAILSTREAM *stream,char *ref,char *pat);
void 	mail_lsub(MAILSTREAM *stream,char *ref,char *pat);
long 	mail_subscribe(MAILSTREAM *stream,char *mailbox);
long 	mail_unsubscribe(MAILSTREAM *stream,char *mailbox);
long 	mail_create(MAILSTREAM *stream,char *mailbox);
long 	mail_delete(MAILSTREAM *stream,char *mailbox);
long 	mail_rename(MAILSTREAM *stream,char *old,char *newname);
long 	mail_status(MAILSTREAM *stream,char *mbx,long flags);
MAILSTREAM *mail_open(MAILSTREAM *oldstream,char *name,long options);
MAILSTREAM *mail_close_full(MAILSTREAM *stream,long options);
MAILHANDLE *mail_makehandle(MAILSTREAM *stream);
void 	mail_free_handle(MAILHANDLE **handle);
MAILSTREAM *mail_stream(MAILHANDLE *handle);
void 	mail_fetchfast_full(MAILSTREAM *stream,char *sequence,long flags);
void 	mail_fetchflags_full(MAILSTREAM *stream,char *sequence,long flags);
ENVELOPE *mail_fetchstructure_full(MAILSTREAM *stream,unsigned long msgno,
				    BODY **body,long flags);
char *	mail_fetchheader_full(MAILSTREAM *stream,unsigned long msgno,
			     STRINGLIST *lines,unsigned long *len,long flags);
char *	mail_fetchtext_full(MAILSTREAM *stream,unsigned long msgno,
			   unsigned long *len,long flags);
char *	mail_fetchbody_full(MAILSTREAM *stream,unsigned long msgno,char *sec,
			   unsigned long *len,long flags);
unsigned long mail_uid(MAILSTREAM *stream,unsigned long msgno);
void 	mail_fetchfrom(char *s,MAILSTREAM *stream,unsigned long msgno,
		     long length);
void 	mail_fetchsubject(char *s,MAILSTREAM *stream,unsigned long msgno,
			long length);
LONGCACHE *mail_lelt(MAILSTREAM *stream,unsigned long msgno);
MESSAGECACHE *mail_elt(MAILSTREAM *stream,unsigned long msgno);

void 	mail_setflag_full(MAILSTREAM *stream,char *sequence,char *flag,
			long flags);
void 	mail_clearflag_full(MAILSTREAM *stream,char *sequence,char *flag,
			  long flags);
void 	mail_search_full(MAILSTREAM *stream,char *charset,SEARCHPGM *pgm,
		       long flags);
long 	mail_ping(MAILSTREAM *stream);
void 	mail_check(MAILSTREAM *stream);
void	mail_expunge(MAILSTREAM *stream);
long 	mail_copy_full(MAILSTREAM *stream,char *sequence,char *mailbox,
		     long options);
long 	mail_append_full(MAILSTREAM 	* stream,
			 char 		* mailbox,
			 char 		* flags,
			 char 		* date,
			 char		* xUnixFrom, /* "From " folder line */
			 STRING 	* message);
void 	mail_gc(MAILSTREAM *stream,long gcflags);

char *	mail_date(char *string,MESSAGECACHE *elt);
char *	mail_cdate(char *string,MESSAGECACHE *elt);
long 	mail_parse_date(MESSAGECACHE *elt,char *string);
void 	mail_exists(MAILSTREAM *stream,unsigned long nmsgs);
void 	mail_recent(MAILSTREAM *stream,unsigned long recent);
void 	mail_expunged(MAILSTREAM *stream,unsigned long msgno);
void 	mail_lock(MAILSTREAM *stream);
void 	mail_unlock(MAILSTREAM *stream);
void 	mail_debug(MAILSTREAM *stream);
void 	mail_nodebug(MAILSTREAM *stream);
unsigned long mail_filter(char *text,unsigned long len,STRINGLIST *lines,
			   long flags);
long 	mail_search_msg(MAILSTREAM *stream,unsigned long msgno,char *charset,
		      SEARCHPGM *pgm);
long 	mail_search_keyword(MAILSTREAM *stream,MESSAGECACHE *elt,STRINGLIST *st);
long 	mail_search_addr(ADDRESS *adr,char *charset,STRINGLIST *st, 
			MAILSTREAM *stream);
long 	mail_search_string(char *txt,char *charset,STRINGLIST *st,
			MAILSTREAM *stream);
char *	mail_search_gets(readfn_t f, void *anystream,unsigned long size, 
			MAILSTREAM *stream);
long 	mail_search_text(char *txt,long len,char *charset,STRINGLIST *st, 
			MAILSTREAM *stream);
SEARCHPGM *mail_criteria(char *criteria, MAILSTREAM *stream);
int 	mail_criteria_date(unsigned short *date, char **lasts);
int 	mail_criteria_string(STRINGLIST **s, char **lasts);
long 	mail_sequence(MAILSTREAM *stream,char *sequence);
long 	mail_sequence_with_uid(MAILSTREAM *stream,char *sequence);
long 	mail_uid_sequence(MAILSTREAM *stream,char *sequence);
unsigned long mail_msgno(MAILSTREAM *stream,unsigned long uid);
long 	mail_parse_flags(MAILSTREAM *stream,char *flag,unsigned long *uf);
ENVELOPE *mail_newenvelope(void);
ADDRESS *mail_newaddr(void);
BODY *	mail_newbody(void);
BODY *	mail_initbody(BODY *body);
PARAMETER *mail_newbody_parameter(void);
PART *	mail_newbody_part(void);
STRINGLIST *mail_newstringlist(void);
HEADERLIST *mail_newheaderlist(void);
SEARCHPGM *mail_newsearchpgm(void);
SEARCHHEADER *mail_newsearchheader(char *line);
SEARCHSET *mail_newsearchset(void);
SEARCHOR *mail_newsearchor(void);
SEARCHPGMLIST *mail_newsearchpgmlist(void);
void 	mail_free_body(BODY **body);
void 	mail_free_body_data(BODY *body);
void 	mail_free_body_parameter(PARAMETER **parameter);
void 	mail_free_body_part(PART **part);
void 	mail_free_cache(MAILSTREAM *stream);
void 	mail_free_elt(MESSAGECACHE **elt);
void 	mail_free_lelt(LONGCACHE **lelt);
/* Sun4 imap */
void 	mail_always_free_elt(MESSAGECACHE **elt);
void 	mail_always_free_lelt(LONGCACHE **lelt);

void 	mail_free_envelope(ENVELOPE **env);
void 	mail_free_address(ADDRESS **address);
void 	mail_free_stringlist(STRINGLIST **string);
void 	mail_free_headerlist(HEADERLIST **header);
void 	mail_free_searchpgm(SEARCHPGM **pgm);
void 	mail_free_searchheader(SEARCHHEADER **hdr);
void 	mail_free_searchset(SEARCHSET **set);
void 	mail_free_searchor(SEARCHOR **orl);
void 	mail_free_searchpgmlist(SEARCHPGMLIST **pgl);
void 	auth_link(AUTHENTICATOR *auth);
char *	mail_auth(char *mechanism,authresponse_t resp,int argc,
		char *argv[], MAILSTREAM *stream);

long 	sm_subscribe(char *mailbox, MAILSTREAM *stream);
long 	sm_unsubscribe(char *mailbox, MAILSTREAM *stream);
char *	sm_read(void **sdb, MAILSTREAM *stream);

/* SUN imap4 */
void 	mail_flush_searchpgm(SEARCHPGM *pgm);
void 	mail_set1flag_full(MAILSTREAM *stream,char *sequence,char *flag,
			long flags);
void 	mail_clear1flag_full(MAILSTREAM *stream,char *sequence,char *flag,
			  long flags);
char *	mail_uidseq_to_sequence(MAILSTREAM *stream, char *uidseq);
char *	mail_seq_to_uidseq(MAILSTREAM *stream, char *msgseq, int mort);
char *	mail_uidseq(MAILSTREAM *stream, char *uid_seq);
char *	mail_msgseq(MAILSTREAM *stream, char *msg_seq);
char *	mail_msgseq_with_uid(MAILSTREAM *stream, char *msg_seq);
unsigned long *mail_expand_uid_sequence(MAILSTREAM *stream,char *sequence);
char *	mail_make_from(SHORTINFO *sinfo);
void	mail_free_fshort_data(SHORTINFO *sinfo);
void 	mail_checksum(MAILSTREAM *stream);
int 	mail_sunversion(MAILSTREAM *stream);
void 	mail_disconnect(MAILSTREAM *stream);
void 	mail_clear_iocount(MAILSTREAM *stream);
int 	mail_count_tokens(char *str);
void 	mail_simple_search(MAILSTREAM *stream, char *criteria, char *charset,
			 long flags);
int 	mail_fetchshort_full(MAILSTREAM *stream, unsigned long msgno, 
			  SHORTINFO *uinfo,
			  long flags);
int 	mail_playback(MAILSTREAM *stream);
int 	play_it_back_sam(MAILSTREAM *stream);
int 	mail_imap4support(MAILSTREAM *stream);
int 	mail_echo(char *name);
void 	mail_increment_send_urgent(MAILSTREAM *stream);
ENVELOPE *mail_fetchenvelope_full(MAILSTREAM *stream,unsigned long msgno,
				  long flags);
/* This function is really in playback.c */
int	mail_playback(MAILSTREAM *stream);
void	mail_find_all_local(MAILSTREAM *stream, char *pat);
int	mail_fullheader_cached(MAILSTREAM *stream, unsigned long msgno);
#ifdef SUPPORT_DISCONECTED
void	mail_enable_recording(MAILSTREAM *stream, char *subdir);
void	mail_disable_recording(MAILSTREAM *stream,long flags);

/* Call for saving the c-client cache to disk */
int 	mail_set_disconnected_state(MAILSTREAM *stream);
int 	mail_clear_disconnected_state(MAILSTREAM *stream);
#endif
int 	mail_cached_filename(char *name, char *buf, int buflen, MAILSTREAM *stream);
MAILSTREAM *mail_prototype(MAILSTREAM *stream);
int 	mail_sinfo_present(MAILSTREAM *stream,unsigned long msgno,unsigned long flags);
void 	mail_free_cached_buffers_seq(MAILSTREAM *stream,char *seq,
				  int use_sequence);
void 	mail_free_cached_buffers_msg(MAILSTREAM *stream,unsigned long msgno);
int 	mail_stringlist_cmp(STRINGLIST *sl1,STRINGLIST *sl2);
STRINGLIST *mail_dup_stringlist(STRINGLIST *src);
int 	mail_get_hierarchy_dlm(MAILSTREAM *stream);
void 	mail_set_hierarchy_dlm(MAILSTREAM *stream, int dlm);
MAILSTREAM *mail_stream_create( void *userdata,
		       int(*set_ttlock)(char *mailbox,void *userdata),
		       int(*clear_ttlock)(char *mailbox,void *userdata),
		       int(*set_group_access)(void),
		       int(*clear_group_access)(void));
void 	mail_stream_setNIL(MAILSTREAM *stream);
void 	mail_stream_unsetNIL(MAILSTREAM *stream);
void 	mail_stream_flush(MAILSTREAM *stream);
void 	mail_global_free(MAILSTREAM *stream);


#define MBOXNOTCACHED       1
#define RESTORE_SYSERR      2
#define CACHECORRUPTED      3

/* Sun Imap4: mm_*() */
void 	mm_io_callback( MAILSTREAM *, long, int );
void 	mm_tcp_log(MAILSTREAM *stream, char *err_msg);
char *	mail_fabricate_from(MAILSTREAM *stream, long msgno);
void 	mm_set_ignore(void);
void 	mm_checksum(MAILSTREAM *stream, unsigned short checksum,
		    long nbytes, char *driver);
void 	mm_log_stream(MAILSTREAM *stream, char *info);
void 	mm_cache_size(MAILSTREAM *stream, unsigned long size,int done);
void 	mm_restore_callback(void *userdata,unsigned long size,
			 unsigned long count);

#ifdef __cplusplus
}
#endif

#endif /* _MAIL_H_ */
