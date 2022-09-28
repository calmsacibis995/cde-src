#include "UnixDefs.h"

/*
 * Record "disconnected mode" driver.
 *      Bill Yeager - 9 Avril 1993
 *
 * Copyright @ 1993, Sun Microsystems, Inc... All rights reserved */

/* Playback routines */
int playback(MAILSTREAM *stream);

/* Record I/O: stream local data */
#define RECORDTMPLEN 16*MAILTMPLEN	
#define RECSMLTMPLEN 256
#define IMAPTCPPORT (long) 143	/* assigned TCP contact port */

/* NOTE: tmp MUST be the first element in the following structure.
 * SEE imap2.h also wjy/lilly */
typedef struct local_local {
  char tmp[RECORDTMPLEN];
  FILE *rfile;			/* recording file stream */
  /* special stream to do searches via a connection to the
   * local driver of the same type as the original connection
   * utilized via the imapserver itself. */
  MAILSTREAM *local_driver;
} RECORDLOCAL;

#define LOCAL ((RECORDLOCAL *)stream->local)
#define RECORDFILENAME ".imap4_recording"
#define RECORDINTRO "IMAP4C-CLIENT"
#define RECORDDELIMETER ":"
#define IMAP4RECORD "IMAP4RECORD"
#define IMAP2RECORD "IMAP2RECORD"
/* For each recording there is a data record of the following format
 *
 * <function>
 * <n-parameters>
 * n*<parameter-record>
 * <end-of-record>
 *
 * <function> := function-name in text\n
 * <n-parameters> := integer number of parameter records\n
 * <parameter-record> := parameter-type:parameter\n
 * <end-of-record> := \n
 * EVERY ENTRY is in ascii */

/* data structures for playback of reocrded information */
typedef struct _p_list_ {
  struct _p_list_ *next;	/* next parameter */
  char *parameter;		/* parameter as a string */
  int parameter_type;
  long flags;			/* flags (PB_UID) */
} PLIST;

typedef struct _sun_recording_ {
  char *function;		/* function called */
  PLIST *params;		/* parameter list */
  struct _sun_recording_ *next; /* next if linked */
} RECORDBLK;

typedef struct _record_blk_list {
  int n_elements;		/* size of list */
  RECORDBLK *rblk;		/* first element */
} RECORDBLKLIST;

/* data structure to hold all record globals */
typedef struct record_globals {
  char *record_subdir;
} RECORD_GLOBALS;

/* parameters and their correspoinding types */
#define STRINGTYPE "string"
#define SEQUENCETYPE "sequence"
#define STRING_PARAM 0
#define SEQUENCE_PARAM 1

/* functions and their corresponding types */
#define SETFLAG   0
#define CLEARFLAG 1
#define COPY      2
#define CREATE    3
#define DELETE    4
#define SET1FLAG  5
#define CLEAR1FLAG 6
#define RENAME     7

#define RECORD_SETFLAG    "setflag"
#define RECORD_CLEARFLAG  "clearflag"
#define RECORD_SET1FLAG   "set1flag"
#define RECORD_CLEAR1FLAG "clear1flag"
#define RECORD_COPY       "copy"
#define RECORD_CREATE     "create"
#define RECORD_DELETE     "delete"
#define RECORD_RENAME     "rename"

/* Flag to indicate uid playback required */
#define PB_UID   (1<<0)
#define PB_IMAP4 (1<<1)
/* For flag testing ... corresponds to bezerk.h/solaris.h */
#define fSEEN 1
#define fDELETED 2
#define fFLAGGED 4
#define fANSWERED 8


DRIVER *record_valid (char *name, MAILSTREAM *stream);
void *map_parameters (long function,void *value);
long record_create (MAILSTREAM *stream, char *mailbox);
long record_delete (MAILSTREAM *stream, char *mailbox);
long record_rename (MAILSTREAM *stream, char *old, char *new);
MAILSTREAM *record_open (MAILSTREAM *stream);
void record_close (MAILSTREAM *stream, long options);
void record_fetchfast (MAILSTREAM *stream, char *sequence, long flags);
void record_fetchflags (MAILSTREAM *stream, char *sequence, long flags);
ENVELOPE *record_fetchstructure (MAILSTREAM *stream, unsigned long msgno, 
				 BODY **body, long flags);
ENVELOPE *record_fetchenvelope(MAILSTREAM *stream,unsigned long msgno,
			       long flags); /* 25-aout-96 WJY */
char *record_fetchheader (MAILSTREAM *stream, unsigned long msgno,
			  STRINGLIST *lines,
			  unsigned long *len, long flags);
char *record_fetchtext (MAILSTREAM *stream, unsigned long msgno,
			unsigned long *len, long flags);
char *record_fetchbody (MAILSTREAM *stream, unsigned long m, char *s, 
			unsigned long *len, long flags);
unsigned long record_fetchuid (MAILSTREAM *stream, unsigned long msgno);
void record_fetchshort(MAILSTREAM *stream, unsigned long msgno,  
		      SHORTINFO *sinfo, long flags);  /* VF */
void record_setflag (MAILSTREAM *stream, char *sequence, char *flag, 
		     long flags);
void record_clearflag (MAILSTREAM *stream, char *sequence, char *flag,
		       long flags);
short record_getflags (MAILSTREAM *stream, char *flag, unsigned long *uf);
long record_ping (MAILSTREAM *stream);
int record_echo (MAILSTREAM *stream);
void record_check (MAILSTREAM *stream);
void record_expunge (MAILSTREAM *stream);
long record_copy (MAILSTREAM *stream, char *sequence, char *mailbox,
		  long options);
long record_append (MAILSTREAM *stream, char *mailbox, 
		    char *flags, char *date, STRING *message);
void record_gc (MAILSTREAM *stream, long gcflags);
int record_sunversion(MAILSTREAM *stream);
void record_simple_search(MAILSTREAM *stream, char *criteria, char *charset,
			  long flags);
void record_checkpoint(MAILSTREAM *stream, unsigned long min_msgno,
		       long flags);	/* VF */
void make_recordfile_name(MAILSTREAM *stream, char *buf);
void record_checksum(MAILSTREAM *stream); /* VF */
void record_disconnect(MAILSTREAM *stream); /* VF */
int record_validchecksum(MAILSTREAM *stream,
			 unsigned short checksum,
			 unsigned long mbox_size);
void dummy_cleario(MAILSTREAM *stream);
void dummy_urgent (MAILSTREAM *stream);	    /* VF */
int dummy_serverecho (MAILSTREAM *stream);  /* vlf */
char *dummy_fabricate_from (MAILSTREAM *stream, long msgno); /* VLS */
int record_set1flag (MAILSTREAM *stream, char *sequence, char *oneflag,
		     long flags);
int record_clear1flag (MAILSTREAM *stream, char *sequence, char *oneflag,
		       long flags);
void record_lsub (MAILSTREAM *stream,char *ref,char *pat);
void record_scan (MAILSTREAM *stream,char *ref,char *pat,char *contents);
void record_list (MAILSTREAM *stream,char *ref,char *pat);
long record_status (MAILSTREAM *stream,char *mbx,long flags,void *udata);
void *record_parameters (MAILSTREAM *stream, long function, void *value);

/* The search stuff */
void record_search (MAILSTREAM *stream, char *charset,SEARCHPGM *pgm,
		    long flags);
long record_search_msg (MAILSTREAM *stream,unsigned long msgno,char *charset,
		      SEARCHPGM *pgm);
long record_search_text (char *txt,long len,char *charset,STRINGLIST *st,
			MAILSTREAM *stream);
long record_search_keyword (MAILSTREAM *stream,MESSAGECACHE *elt,
			    STRINGLIST *st);
long record_search_addr (ADDRESS *adr,char *charset,STRINGLIST *st,
			MAILSTREAM *stream);
long record_search_string (char *txt,char *charset,STRINGLIST *st,
			MAILSTREAM *stream);
int record_imap4(MAILSTREAM *stream);
void record_global_init(MAILSTREAM *stream);
void record_global_free(MAILSTREAM *stream);
