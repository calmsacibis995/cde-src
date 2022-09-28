/*
 * Program:	Record-mode  mail routines
 *
 * Author:	William J. Yeager
 *		Independent consultant
 * Copyright 1993 by Sun Micro Systems, Inc.
 *
 */
/*
 * Edit history
 *
 * July 9, 1994 - fixed record_valid to return NIL if one
 *     is opening a local file. Note the record driver is always
 *     first in the list, and so must reject any such open so
 *     that one can open files locally while disconnected.
 *                   WJY  
 * Sept. 5, 1995 - adapted for imap4 (we have uids!!)
 * Note that all recordings are coerced to msg numbers.
 * Playback will appropriately turn them into UID's 
 * if for imap4 playbacks, and leave them as msgnos for
 * imap2 playbacks. Il nous faut tous les deux.
 * 
 * Last edited:  
 * [09-23-96 clin ] strtok_r is used to ensure MT-Safe.
 * [10-21-96 clin ] Added a new routine: record_global_init.
 *		Passing stream onto record_parameters 
 *		and record_valid,
 *
 *		Globals put onto stream:
 *		stream->record_globals->record_subdir
 *
 * [11-08-96 clin ] Pass stream in all mm_log calls and tcp_open
 *		and record_search_text and record_search_string
 *		and record_search_addr.
 * 
 * [12-30-96 clin ] Define record_globals in record.h.
 * [01-01-97 clin ] Add routine record_global_free.
 * [01-02-97 clin ] Use mail_parameters to get imap_defaultport.
 * [01-03-97 clin ] Use mail_stream_setNIL. 
 * [01-07-97 clin ] Remove the last five parameters in mail_open. 
 * [01-22-97 clin ] Call mail_stream_create in record_status. 
 *
 */
char *record_version= ".r2";

#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "mail.h"
#include "os_sv5.h"
#include "record.h"
#include "misc.h"
#include "dummy.h"
#include "rfc822.h"

/* Driver dispatch used by MAIL in RECORD MODE */
/* This driver is initially disabled. One enables it and
 * disables the imapd or any other network driver when
 * disconnecting */
DRIVER recorddriver = {
  "record",			/* driver name */
  DR_MAIL|DR_RECORD|DR_DISABLE, /* flags */
  (DRIVER *) NIL,		/* next driver */
  record_valid,			/* mailbox is valid for us */
  record_parameters,		/* manipulate parameters */
  record_scan,                  /* scan list of mailboxes */
  record_list,			/* !find mailboxes */
  record_lsub,                  /* !find subscribed mailboxes */
  NIL,				/* No subscribe/unsubscribe. */
  NIL,				/* No subscribe/unsubscribe. */
  record_create,		/* create mailbox */
  record_delete,		/* delete mailbox */
  record_rename,		/* rename mailbox */
  record_status,                /* !status of mailbox */
  record_open,			/* open mailbox */
  record_close,			/* close mailbox */
  record_fetchfast,		/* fetch message "fast" attributes */
  record_fetchflags,		/* fetch message flags */
  record_fetchstructure,	/* fetch message envelopes */
  record_fetchheader,		/* fetch message header only */
  record_fetchtext,		/* fetch message body only */
  record_fetchbody,		/* fetch message body section */
  record_fetchuid,              /* fetch UID of messages */
  record_setflag,		/* set message flag */
  record_clearflag,		/* clear message flag */
  record_search,		/* search for message based on program */
  record_ping,			/* ping mailbox to see if still alive */
  record_check,			/* check for new messages */
  record_expunge,		/* expunge deleted messages */
  record_copy,			/* copy messages to another mailbox */
  record_append,		/* append string message to mailbox */
  record_gc,			/* garbage collect stream */
  /* Sun imap4 */
  dummy_fabricate_from,         /* no need if cached */
  record_set1flag,		/* clear one flag */
  record_clear1flag,		/* set one falg */
  record_sunversion,		/* sunversion */
  record_checkpoint,		/* checkpoint stream  */
  record_disconnect,		/* disconnect - illegal on a recorded str */
  record_echo,			/* ditto */
  dummy_clear_iocount,		/* !does nothing really */
  dummy_urgent,			/* not used in record mode */
  record_fetchshort,            /* the short info */
  record_checksum,		/* checksum the mailbox */
  record_validchecksum,		/* validate checksum */
  dummy_sync_msgno,		/* sync message numbers */
  record_imap4,			/* true if imap4 recording */
  NIL,				/* no driver free body */
  record_fetchenvelope,		/* fetch envelope */
  record_global_init,           /* init record globals */
  record_global_free            /* free record globals */
};

void record_write_record_header(MAILSTREAM *stream, char *function, 
				int n_params);
void record_write_string_param(MAILSTREAM *stream, char *param);
void record_write_record_end(MAILSTREAM *stream);
				/* prototype stream */
int resume_record(MAILSTREAM *stream, char **bienvenue,char *date);

MAILSTREAM recordproto = {&recorddriver};

/* This driver is NEVER called by the imapserver, but
 * rather, is linked by a client when it is in record-mode.
 *
 * We don't return true unless name would normally require
 * the imapd.
 *
 * Record mail validate mailbox
 * Accepts: mailbox name
 * Returns: recording driver.
 *
 * Currently, we only record for solaris mailboxes.
 * We are leaving in the option for "#<name>{"
 */
DRIVER *record_valid (char *name, MAILSTREAM *stream)
{	
  if (*name == '{' || (*name == '#' && strchr(name, '{'))) 
    return  &recorddriver;
  else
    return NIL;
}
/* Record manipulate parameters
 * Accepts: function code
 *	    function-dependent value
 * Returns: function-dependent return value
 */
void *record_parameters (MAILSTREAM *stream, long function, void *value)
{
  RECORD_GLOBALS *rg = (RECORD_GLOBALS *)stream->env_globals;
  int len;
  switch ((int) function) {
  case SET_RECORDSUBDIR:
    len = strlen((char *)value);
    if (len > 0) {
      if (rg->record_subdir) fs_give((void **)&rg->record_subdir);
      rg->record_subdir = cpystr((char*)value);
    }
    break;
  default:
    return (void *)NIL;
  }
  return value;
}

/*
 * Write the checksum, filesize and UID_VALIDITY to the recording file,
 * we do not use the driver name in imap4 */
void record_checksum_data(FILE *rfile, MAILSTREAM *stream)
{
  fprintf(rfile, "%d\n%ld\n%ld\n", 
	  stream->checksum, 
	  stream->mbox_size,
	  stream->uid_validity);
}

/* Record Open - sets record flag to true, and opens recording file.
 * Accepts: stream to open
 * Returns: stream that is passed.
 */
#define RECINTROLEN 18
MAILSTREAM *record_open (MAILSTREAM *stream)
{
  char date[RECSMLTMPLEN];
  char *bienvenue;
  int number;
  if (!stream->checkpointed) {
    mm_log("record_open: Stream not checkpointed", ERROR,stream);
    return NIL;
  } else {
    /* clear input_flushed flag which could be set
     * as a result of a flush before diconnecting */
    stream->input_flushed = NIL;
  }
				/* allocate driver local data */
  stream->local = fs_get(sizeof(RECORDLOCAL));
				/* Open recording file */
  make_recordfile_name(stream, LOCAL->tmp);
  /*
   * We may be resuming a recording session. Then we just open for
   * append. Otherwise, we open a zero length file for write */
  stream->record = NIL;
  if (stream->rec_resume) {
    if (!resume_record(stream,&bienvenue,date))
      stream->rec_resume = NIL;		/* Force new record session */
    else number = 145;
  }
  if (!stream->rec_resume) {
    /*
     * New recording session */
    char *servertype;
    LOCAL->rfile = fopen(LOCAL->tmp, "w");
    if (!LOCAL->rfile) {
      fs_give(&stream->local);
      stream->record = NIL;
      mm_log("record_open: Failure opening recording file.", ERROR,stream);
      mm_log_stream (stream, 
		     "133 record resume: open error on recording file");
      return NIL;
    } else {
      rfc822_date(date);
      servertype = (stream->imap4_recording_session ? IMAP4RECORD :
		    IMAP2RECORD);
      /* recording client, server, date */
      fprintf(LOCAL->rfile, "%s%s%s\n%s\n", 
	      RECORDINTRO,RECORDDELIMETER,
	      servertype, date);
				/* the checksum data for replay */
      record_checksum_data(LOCAL->rfile, stream);
      /* Get this stuff to disk */
      fflush(LOCAL->rfile);
      fsync(fileno(LOCAL->rfile));
      bienvenue = "Recording session on";
      number = 135;
    }
  }
  sprintf(LOCAL->tmp, "%d IMAP4: %s %s",number,bienvenue,date);
  mm_log_stream (stream, LOCAL->tmp);
  stream->record = T;
  return stream;
}

int resume_record(MAILSTREAM *stream, char **bienvenue,char *date)
{
  /*
   * resuming recording session: Open read/update */
  LOCAL->rfile = fopen(LOCAL->tmp, "r+");
  if (!LOCAL->rfile) {
    mm_log("record_open: could not open file for append.", ERROR,stream);
    return NIL;
  }
  /* First line should be IMAP4 */
  if (fgets (LOCAL->tmp, RECSMLTMPLEN, LOCAL->rfile) == 0 || 
      feof(LOCAL->rfile)) {
    fclose (LOCAL->rfile);
    mm_log ("133 record resume: read error on recording file", ERROR,stream);
    mm_log_stream (stream, "133 record resume: read error on recording file");
    return NIL;
  } 
  if (strncasecmp("IMAP4C-CLIENT:IMAP", LOCAL->tmp, RECINTROLEN)) {
    fclose (LOCAL->rfile);
    mm_log ("133 record resume: read error on recording file", ERROR,stream);
    mm_log_stream (stream, "134 record resume: not IMAP4 recording file");
    return NIL;
  }
  *bienvenue = "Resuming recording session of";
  if (fgets (date, RECSMLTMPLEN, LOCAL->rfile) == 0 || 
      feof(LOCAL->rfile)) {
    fclose (LOCAL->rfile);
    mm_log ("133 record resume: read error on recording file", ERROR,stream);
    mm_log_stream (stream, "133 record resume: read error on recording file");
    return NIL;
  }
  date[strlen(date) - 1] = NIL; /* step on "\n" */
  /*
   * Seek end-of-file */
  if (fseek (LOCAL->rfile, 0L, SEEK_END) < 0) {
    fclose (LOCAL->rfile);
    mm_log("record resume: Seek error on recording file", ERROR,stream);
    mm_log_stream (stream, 
		   "133 record resume: seek error on recording file");
    return NIL;
  }
  return T;
}


/* Record-mode: list subscribed
 * Accepts: mail stream
 *	    pattern to search
 * does this locally.
 */

void record_lsub (MAILSTREAM *stream, char *ref, char *pat)
{
  if (stream && (stream->stream_status & S_OPENED ) && 
	!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR,stream);
  } else
    if (stream && (stream->stream_status & S_OPENED)) 
	dummy_lsub (stream,ref,pat);
  return;
}


/* does this locally
 * Accepts: mail stream
 *	    pattern to search
 */
void record_list (MAILSTREAM *stream,char *ref,char *pat)
{
  if (stream && (stream->stream_status & S_OPENED) &&
	 !stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR,stream);
  } else 
    if (stream && (stream->stream_status & S_OPENED) ) 
	dummy_list (stream,ref,pat);
}
/*
 * do a scan locally */
void record_scan (MAILSTREAM *stream,char *ref,char *pat,char *contents)
{
  if (stream && (stream->stream_status & S_OPENED) &&
	!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR,stream);
  } else 
    if (stream && (stream->stream_status & S_OPENED)) 
	dummy_scan (stream,ref,pat,contents);
}

/* Record delete mailbox
 * Accepts: mail stream
 *	    mailbox name to delete
 * Returns: T on success, NIL on failure
 */
long record_delete (MAILSTREAM *stream,char *mailbox)
{
  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR,stream);
  }

  if (!mailbox) {
    mm_log("record_delete: mailbox required", ERROR,stream);
    return NIL;
  }
  /* Write recording record to file */
  record_write_record_header(stream, RECORD_DELETE, 1);
  record_write_string_param(stream, mailbox);
  record_write_record_end(stream);
  sprintf(LOCAL->tmp, "record_delete: mailbox = %s", mailbox);
  mm_log (LOCAL->tmp, NIL,stream);
  return T;
}


/* Does nothing ...
 * Accepts: mail stream
 *	    old mailbox name
 *	    new mailbox name
 * Returns: T on success, NIL on failure
 */

long record_rename (MAILSTREAM *stream,char *old,char *new)
{
  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR,stream);
  } 
  if (!old || *old == '\0' || !new || *new == '\0') {
    mm_log("record_rename: Illegal file name", ERROR,stream);
    return NIL;
  }
  /* Write recording record to file */
  record_write_record_header(stream, RECORD_RENAME, 2);
  record_write_string_param(stream, old);
  record_write_string_param(stream, new);
  record_write_record_end(stream);
  sprintf(LOCAL->tmp, "record_rename: %s to %s", old, new);
  mm_log (LOCAL->tmp, NIL,stream);
  return T;
}

/* Mail status of mailbox while recording
 * Accepts: mail stream
 *	    mailbox name
 *	    status flags
 * Returns: T on success, NIL on failure
 * We generate a status for a local mailbox
 * or current mailbox. We have no connection.
 */

long record_status (MAILSTREAM *stream,char *mbx,long flags,void *udata)
{
  MAILSTATUS status;
  unsigned long i;
  MAILSTREAM *tstream;
  char tmp[MAILTMPLEN];
  DRIVER *factory;

  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR,stream);
  } else
    factory = mail_valid (stream,mbx,"get status of mailbox");
  /* Need a driver that is local */
  if (!factory ||		/* bad name */
      !(factory->flags & DR_LOCAL))/* NOT local */
    return NIL;
  /* If record driver, then we must have the same mailbox */
  if ((factory->flags & DR_RECORD) && strcmp (mbx, stream->mailbox) != 0)
    return NIL;
				/* use driver's routine if have one 
				 * and we did not get us! */
  if (!(factory->flags & DR_RECORD) && factory->status) 
    return (*factory->status) (stream,mbx,flags,NIL);
				/* canonicalize mbx */
  dummy_file (tmp, mbx, stream);
  mbx = tmp;
  /* make temporary stream:
   *   Just passed stream if recording, otherwise open local mbox */
  if (factory->flags & DR_RECORD ) tstream = stream;
  else {
  	tstream = mail_stream_create(stream->userdata,
                  NIL,NIL,NIL,NIL);
  	tstream = mail_open(tstream,mbx,OP_READONLY|OP_SILENT); 
	/* If still an un-opened stream, just return. -clin */
	if (!(tstream->stream_status & S_OPENED )) {
     	mail_stream_flush(tstream);
	return NIL;
		}
	}
  status.flags = flags;		/* return status values */
  status.messages = tstream->nmsgs;
  status.recent = tstream->recent;
  if (flags & SA_UNSEEN)	/* must search to get unseen messages */
    for (i = 1,status.unseen = 0; i <= tstream->nmsgs; i++)
      if (!mail_elt (tstream,i)->seen) status.unseen++;
  status.uidnext = tstream->uid_last + 1;
  status.uidvalidity = tstream->uid_validity;
  if (flags & SA_CHECKSUM) {
    if (stream->dtb->checksum) {
      stream->checksum_type |= STATUSCHECKSUM;
      (*stream->dtb->checksum)(stream);
      status.checksum = stream->checksum;
      status.checksum_bytes = stream->mbox_size;
    } else {
      status.checksum = 0;
      status.checksum_bytes = 0L;
    }
  }
				/* pass status to main program */
  mm_status (tstream,mbx,&status);
  if (stream != tstream) mail_close (tstream);
  return T;			/* success */
}


/* This client always runs the sunversion */
int record_sunversion(MAILSTREAM *stream)
{
  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR,stream);
    return NIL;
  }
  return T;
}
/*
 * Cannot checkpoint while recording! */
void record_checkpoint(MAILSTREAM *stream, unsigned long min_msgno, long flags)
{
  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR,stream);
  } else
    mm_log("record_checkpoint: Illegal attempt to checkpoint", ERROR,stream);
}
void record_disconnect(MAILSTREAM *stream)
{
  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR,stream);
  } else
    mm_log("record_disconnect: Illegal attempt to disconnect", ERROR,stream);
}
/* Makes NO sense to checksum in record mode:
 */
void record_checksum(MAILSTREAM *stream)
{
  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR,stream);
  } else
    mm_log("record_checksum: Checksum called while recording", ERROR,stream);
}
/* Ditto for valid checksum */
int record_validchecksum(MAILSTREAM *stream,
			 unsigned short checksum,
			 unsigned long mbox_size)
{
  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR,stream);
  } else
    mm_log("record_validchecksum: Illegal call to validchecksum", ERROR,stream);
  return NIL;
}


/* Close record mode ...
 * Accepts: MAIL stream
 */

void record_close (MAILSTREAM *stream, long options)
{
  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR,stream);
    return;
  }
  fclose(LOCAL->rfile);
  fs_give(&stream->local);
  if (stream->driver_name)
    fs_give((void **)&stream->driver_name);
  stream->checkpointed = stream->record = NIL;
  /* disable the record driver */
  stream->dtb->flags |= DR_DISABLE;
}


/* Recording mode - fetch fast information
 * Accepts: MAIL stream
 *	    sequence
 *
 * record_fetchstructure is required. 
 */

void record_fetchfast (MAILSTREAM *stream,char *sequence, long flags)
{		
  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR,stream);
    return;
  }
  mm_log("record_fetchfast: Illegal call in recording mode.", ERROR,stream);
}

static unsigned long uid_to_msgno(MAILSTREAM *stream, unsigned long uid)
{
  unsigned long msgno0 = mail_msgno (stream,uid);
  if (msgno0 == 0) return 0;
  else
    return msgno0;
}
/*
 * We return the info if we have it cached. Otherwise NIL ... */
void record_fetchshort (MAILSTREAM *stream, unsigned long msgno, 
		       SHORTINFO *user_sinfo, long flags)
{				/* see if we have fetch short data cached */
  LONGCACHE *lelt;
  SHORTINFO *sinfo;
  unsigned long msgno0;

  user_sinfo->date = NIL;
  user_sinfo->personal = NIL;
  user_sinfo->mailbox = NIL;
  user_sinfo->host = NIL;
  user_sinfo->subject = NIL;
  user_sinfo->bodytype = NIL;
  user_sinfo->size = NIL;
  user_sinfo->from = NIL;

  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR,stream);
    return;
  }
  if (stream->scache) {		/* short cache */
    mm_log("record_fetchinfo: \"scache\" call unsupported in recording mode",
	   ERROR,stream);
    return;
  }
  /* See if this is a UID fetch, and if so, get the message number
   * associated with this UID */
  if (flags & FT_UID) {
    msgno0 = uid_to_msgno(stream, msgno);
    if (msgno0 == 0) return;
    msgno = msgno0;
  }

  /* Using the long cache */
  lelt = mail_lelt (stream, msgno);
  sinfo = &lelt->sinfo;
  /* make sure there */
  if (sinfo->date == NIL) return;
  user_sinfo->date = sinfo->date;
  user_sinfo->personal = sinfo->personal;
  user_sinfo->mailbox = sinfo->mailbox;
  user_sinfo->host = sinfo->host;
  user_sinfo->subject = sinfo->subject;
  user_sinfo->bodytype = sinfo->bodytype;
  user_sinfo->size = sinfo->size;
  user_sinfo->from = sinfo->from;
}                                                           /* END VF */

/* Mail Access Protocol fetch flags
 * Accepts: MAIL stream
 *	    sequence
 * The flags are either cached or not cached when one
 * disconnects.
 */

void record_fetchflags (MAILSTREAM *stream, char *sequence,long flags)
{				/* send "FETCH sequence FLAGS" */
  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR,stream);
    return;
  }
  mm_log ("record_fetchflags: Illegal call in recording mode", ERROR, stream);
}

/* Record fetch_structure
 * Accepts: MAIL stream
 *	    message # to fetch
 *	    pointer to return body
 * Returns: envelope of this message if the data is already cached.
 *
 */
ENVELOPE *record_fetchstructure (MAILSTREAM *stream,unsigned long msgno,
				 BODY **body, long flags)
{
  LONGCACHE *lelt;
  ENVELOPE **env;
  BODY **b;

  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR,stream);
    return NIL;
  }

  if (stream->scache) {		/* short cache */
    mm_log("record_fetchstructure: \"scache\" call unsupported", ERROR, stream);
    return NIL;
  }
  /*
   * See if UID fetch is happening */
  if (flags & FT_UID) {
    unsigned long msgno0 = uid_to_msgno (stream, msgno);
    if (msgno0 == 0) return NIL;
    msgno = msgno0;
  }
  lelt = mail_lelt (stream,msgno);
  env = &lelt->env;		/* get pointers to envelope and body */
  b = &lelt->body;
  
				/* have the merde we need? */
  if ((body && !*b) || !*env) 
    return NIL;
  /* Nous avons le data */
  if (body) *body = *b;		/* return the body */
  return *env;			/* return the envelope */
}

/* Envelope only */
ENVELOPE *record_fetchenvelope(MAILSTREAM *stream,unsigned long msgno,
			       long flags)
{
  ENVELOPE *env = record_fetchstructure(stream,msgno,NIL,flags);
  return env;
}


/* record_fetchheader
 * Accepts: MAIL stream
 *	    message # to fetch
 * Returns: message header in RFC822 format if we already have it, otherise NIL.
 */
/* !!!! do something with the lines */
char *record_fetchheader (MAILSTREAM *stream,unsigned long msgno, 
			  STRINGLIST *lines,
			  unsigned long *len, long flags)
{
  MESSAGECACHE *elt;
  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR, stream);
    return "";
  }
  /*
   * See if UID fetch is happening */
  if (flags & FT_UID) {
    unsigned long msgno0 = uid_to_msgno (stream, msgno);
    if (msgno0 == 0) return "";
    msgno = msgno0;
  }
  elt = mail_elt (stream,msgno);
  if (elt->data1) {		/* We have the full header */
    if (lines) {		/* We can recompute the filtered data */
      if (elt->filter1) fs_give((void **)&elt->filter1);
      elt->filter1 = (unsigned long)cpystr((char *)elt->data1);
      elt->filter2 = mail_filter ((char *)elt->filter1,elt->data3,lines,flags);
      if (len) *len = elt->filter2;
      return (char *)elt->filter1;
    } else {
      if (len) *len = elt->data3;
      return (char *) elt->data1;
    }
  } else if (lines && elt->filter1) {
				/* Give the old filtered stuff */
    if (len) *len = elt->filter2;
    return (char *)elt->filter1;
  } else {
				/* nothing cached */
    if (len) *len = 0;
    return (char *)"";
  }
}


/* Recording mode: fetch message text (body only)
 * Accepts: MAIL stream
 *	    message # to fetch
 * Returns: message text in RFC822 format
 */
#define TMPSEQLEN 64
char *record_fetchtext (MAILSTREAM *stream, unsigned long msgno,
			unsigned long *len, long flags)
{
  char seq[TMPSEQLEN];
  long i = msgno - 1;
  MESSAGECACHE *elt;

  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR, stream);
    return "";
  } else {
    /*
     * See if UID fetch is happening */
    if (flags & FT_UID) {
      unsigned long msgno0 = uid_to_msgno (stream, msgno);
      if (msgno0 == 0) return "";
      msgno = msgno0;
    }
    elt = mail_elt (stream,msgno);
    if (!elt->data2) {		/* send "FETCH msgno RFC822.TEXT" */
      if (len) *len = 0;
      return "";
    } else {
      if (len) *len = elt->data4;
      /* If NOT peeking, then set the seen flag */
      if (!(flags & (FT_PEEK+FT_RSEARCH))) {
	sprintf(seq, "%ld", msgno);
	record_set1flag (stream, seq, "\\Seen", NIL);
      }
      return (char *)elt->data2;
    }
  }
}

/* Recording mode: fetch message body as a structure
 * Accepts: Mail stream
 *	    message # to fetch
 *	    section specifier
 *	    pointer to length
 * Returns: pointer to section of message body
 */
char *record_fetchbody (MAILSTREAM *stream,unsigned long m,char *sec,
			unsigned long *len, long flags)
{
  BODY *b;
  PART *pt;
  char *s = sec;
  char **ss;
  unsigned long i;
  char seq[TMPSEQLEN];

  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR, stream);
    return "";
  }
  if (len) *len = 0;			/* in case failure */
				/* make sure have a body */
  if (!(record_fetchstructure (stream,m,&b,flags) && b)) {
				/* bodies not supported, wanted section 1? */
    if (strcmp (sec, "1")) return NIL;
				/* yes, return text if possible.*/
    s = record_fetchtext (stream,m,len,flags);
    return s;
  }
  if (!(s && *s && ((i = strtol (s,&s,10)) > 0))) return NIL;
  do {				/* until find desired body part */
				/* multipart content? */
    if (b->type == TYPEMULTIPART) {
      pt = b->contents.part;	/* yes, find desired part */
      while (--i && (pt = pt->next));
      if (!pt) return NIL;	/* bad specifier */
				/* note new body, check valid nesting */
      if (((b = &pt->body)->type == TYPEMULTIPART) && !*s) return NIL;
    }
    else if (i != 1) return NIL;/* otherwise must be section 1 */
				/* need to go down further? */
    if (i = *s) switch (b->type) {
    case TYPEMESSAGE:		/* embedded message, calculate new base */
      b = b->contents.msg.body;	/* get its body, drop into multipart case */
    case TYPEMULTIPART:		/* multipart, get next section */
      if ((*s++ == '.') && (i = strtol (s,&s,10)) > 0) break;
    default:			/* bogus subpart specification */
      return NIL;
    }
  } while (i);

				/* lose if body bogus */
  if ((!b) || b->type == TYPEMULTIPART) return NIL;
  switch (b->type) {		/* decide where the data is based on type */
  case TYPEMESSAGE:		/* encapsulated message */
    ss = &b->contents.msg.text;
    break;
  case TYPETEXT:		/* textual data */
    ss = (char **) &b->contents.text;
    break;
  default:			/* otherwise assume it is binary */
    ss = (char **) &b->contents.binary;
    break;
  }
				/* return data size if have data */
  if (s = *ss) {
    if (len) *len = b->size.bytes;
    /* If NOT peeking, then set the seen flag */
    if (!(flags & FT_PEEK)) {
      sprintf(seq, "%ld", m);
      record_set1flag (stream, seq, "\\Seen", NIL);
    }
  }
  return s;
}
/*
 * Just return the uid from the message cache */
unsigned long record_fetchuid (MAILSTREAM *stream, unsigned long msgno)
{
  MESSAGECACHE *elt;
  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR, stream);
    return 0;
  }
  elt = mail_elt(stream,msgno);
  /* non zero if cached */
  return elt->uid;
}

/*
 * Recording mode: functions for writing data records in
 * the recording file */
void record_write_record_header(MAILSTREAM *stream, char *function, int n_params)
{
  fprintf(LOCAL->rfile, "%s\n%d\n", function, n_params); 
}

void record_write_string_param(MAILSTREAM *stream, char *param)
{
  fprintf(LOCAL->rfile, "%s:%s\n", STRINGTYPE, param);
}

void record_write_sequence_param(MAILSTREAM *stream, char *param)
{
  fprintf(LOCAL->rfile, "%s:%s\n", SEQUENCETYPE, param);
}


void record_write_record_end(MAILSTREAM *stream)
{
  fprintf(LOCAL->rfile, "\n");
  fflush(LOCAL->rfile);
  fsync(fileno(LOCAL->rfile));
}


/* Parse flag list
 * Accepts: MAIL stream
 *	    flag list as a character string
 *	    pointer to user flags to return
 * Returns: flag command list
 */


short record_getflags (MAILSTREAM *stream, char *flag, unsigned long *uf)
{
  char *t,*s,tmp[MAILTMPLEN],key[MAILTMPLEN];
  char *toker;
  char *lasts;
  short f = 0;
  long i;
  short j;
  *uf = 0;			/* initially no user flags */
  if (flag && *flag) {		/* no-op if no flag string */
				/* check if a list and make sure valid */
    if ((i = (*flag == '(')) ^ (flag[strlen (flag)-1] == ')')) {
      mm_log ("Bad flag list",ERROR, stream);
      return NIL;
    }
				/* copy the flag string w/o list construct */
    strncpy (tmp,flag+i,(j = strlen (flag) - (2*i)));
    tmp[j] = '\0';
    t = ucase (tmp);		/* uppercase only from now on */
    strtok_r(t, " ", &lasts);	/* init out strtok: BUG FIX - 7 sep 95 */
    while (t && *t) {		/* parse the flags */
      i = 0;
      if (*t == '\\') {		/* system flag? */
	switch (*++t) {		/* dispatch based on first character */
	case 'S':		/* possible \Seen flag */
	  if (t[1] == 'E' && t[2] == 'E' && t[3] == 'N') i = fSEEN;
	  t += 4;		/* skip past flag name */
	  break;
	case 'D':		/* possible \Deleted or \Draft flag */
	  if (t[1] == 'E' && t[2] == 'L' && t[3] == 'E' && t[4] == 'T' &&
	      t[5] == 'E' && t[6] == 'D') {
	    i = fDELETED;
	    t += 7;		/* skip past flag name */
	  }
	  else if (t[1] == 'R' && t[2] == 'A' && t[3] == 'F' && t[4] == 'T') {
	    i = fDRAFT;
	    t += 5;		/* skip past flag name */
	  }
	  break;
	case 'F':		/* possible \Flagged flag */
	  if (t[1] == 'L' && t[2] == 'A' && t[3] == 'G' && t[4] == 'G' &&
	      t[5] == 'E' && t[6] == 'D') i = fFLAGGED;
	  t += 7;		/* skip past flag name */
	  break;
	case 'A':		/* possible \Answered flag */
	  if (t[1] == 'N' && t[2] == 'S' && t[3] == 'W' && t[4] == 'E' &&
	      t[5] == 'R' && t[6] == 'E' && t[7] == 'D') i = fANSWERED;
	  t += 8;		/* skip past flag name */
	  break;
	default:		/* unknown */
	  i = 0;
	  break;
	}
				/* add flag to flags list */
	if (i && (*t == '\0')) f |= i;
      }
				/* user flag, search through table */
      else {
	for (j = 0; !i && j < NUSERFLAGS && (s =stream->user_flags[j]); ++j)
	  if (!strcmp (t,ucase (strcpy (key,s)))) *uf |= i = 1 << j;
      }
      if (!i) {			/* didn't find a matching flag? */
	sprintf (key,"Unknown flag: %.80s",t);
	mm_log (key,ERROR, stream);
      }
				/* parse next flag */
      t = strtok_r(NIL," ", &lasts );
    }
  }
  return f;
}

/* Recording-mode: set one flag, set flag
 * Accepts: MAIL stream
 *	    sequence
 *	    flag(s)
 * Record information for later replay ...
 */
int record_set1flag (MAILSTREAM *stream, char *sequence, char *flag, long flags)
{
  MESSAGECACHE *elt;
  short f;
  int ret= T;
  char *msgseq, *oseq;
  unsigned long uf;

  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR, stream);
    return NIL;
  }
  /* Validate the flag */
  f = record_getflags(stream, flag, &uf);
  if (!f) {
    sprintf(LOCAL->tmp, "setflag: Rejected - Illegal Flags, %s", flag);
    mm_log(LOCAL->tmp, NIL, stream);
    return NIL;
  }
  /* If a UID sequence is passed, then make a sequence from it */
  if (flags & ST_UID) {
    msgseq = mail_uidseq_to_sequence(stream, sequence);
    if (*msgseq == NULL) { 
      fs_give((void **)&msgseq);
      return NIL;
    }
    oseq = sequence;
    sequence = msgseq;
  } else
    oseq = NIL;
  /* Validate the sequence */
  if (mail_sequence (stream, sequence)) {
    int i;

    for (i = 1; i <= stream->nmsgs; ++i) {
      if ((elt = mail_elt(stream, i))->sequence) { /* in the sequence */
         if (f&fSEEN) elt->seen = T;
	 if (f&fDELETED) elt->deleted = T;
	 if (f&fFLAGGED) elt->flagged = T;
	 if (f&fANSWERED) elt->answered = T;
       }
    }
    /* Write recording record to file */
    record_write_record_header(stream, RECORD_SET1FLAG, 2);
    record_write_sequence_param(stream, sequence);
    record_write_string_param(stream, flag);
    record_write_record_end(stream);
    sprintf(LOCAL->tmp, "set1flag: sequence %s, flag = %s",
	    sequence, flag);
  } else {
    sprintf(LOCAL->tmp, "set1flag: Rejected - Illegal sequence %s", sequence);
    ret = NIL;
  }
  mm_log (LOCAL->tmp, NIL, stream);
  if (oseq) {
    fs_give((void **)&sequence);
    sequence = oseq;
  }
  return ret;
}

void record_setflag (MAILSTREAM *stream, char *sequence, char *flag,
		     long flags)
{
  MESSAGECACHE *elt;
  short f;
  char *msgseq, *oseq;
  unsigned long uf;

  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR, stream);
    return;
  }
  /* Validate the flag */
  f = record_getflags(stream, flag, &uf);
  if (!f) {
    sprintf(LOCAL->tmp, "setflag: Rejected - Illegal Flags, %s", flag);
    mm_log(LOCAL->tmp, NIL, stream);
    return;
  }
  /* If UID sequence is passed, then make a sequence from it */
  if (flags & ST_UID) {
    msgseq = mail_uidseq_to_sequence(stream, sequence);
    if (*msgseq == NULL) {
      fs_give((void **)&msgseq);
      return;
    }
    oseq = sequence;
    sequence = msgseq;
  } else
    oseq = NIL;

  /* Validate the sequence */
  if (mail_sequence (stream, sequence)) {
    int i;

    for (i = 1; i <= stream->nmsgs; ++i) {
      if ((elt = mail_elt(stream, i))->sequence) { /* in the sequence */
         if (f&fSEEN) elt->seen = T;
	 if (f&fDELETED) elt->deleted = T;
	 if (f&fFLAGGED) elt->flagged = T;
	 if (f&fANSWERED) elt->answered = T;
       }
    }
    /* Write recording record to file */
    record_write_record_header(stream, RECORD_SETFLAG, 2);
    record_write_sequence_param(stream, sequence);
    record_write_string_param(stream, flag);
    record_write_record_end(stream);
    sprintf(LOCAL->tmp, "setflag: sequence %s, flag = %s",
	    sequence, flag);
  } else {
    sprintf(LOCAL->tmp, "setflag: Rejected - Illegal sequence %s", sequence);
  }
  mm_log (LOCAL->tmp, NIL, stream);
  if (oseq) {
    fs_give((void **)&sequence);
    sequence = oseq;
  }
}


/* Recording mode - clear one flag, clear flag
 * Accepts: MAIL stream
 *	    sequence
 *	    flag(s)
 */
int record_clear1flag (MAILSTREAM *stream, char *sequence, char *flag, 
		       long flags)
{
  MESSAGECACHE *elt;
  short f;
  int ret= T;
  char *msgseq, *oseq;
  unsigned long uf;
  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR, stream);
    return NIL;
  }
  /* Validate the flag */
  f = record_getflags(stream, flag, &uf);
  if (!f) {
    sprintf(LOCAL->tmp, "clear1flag: Rejected - Illegal Flags, %s", flag);
    mm_log(LOCAL->tmp, NIL, stream);
    return NIL;
  }
  /* If UID sequence is passed, then make a sequence from it */
  if (flags & ST_UID) {
    msgseq = mail_uidseq_to_sequence(stream, sequence);
    if (*msgseq == NULL) {
      fs_give((void **)&msgseq);
      return NIL;
    }
    oseq = sequence;
    sequence = msgseq;
  } else
    oseq = NIL;
  /* Validate the sequence */
  if (mail_sequence (stream, sequence)) {
    int i;

    for (i = 1; i <= stream->nmsgs; ++i) {
      if ((elt = mail_elt(stream, i))->sequence) { /* in the sequence */
         if (f&fSEEN) elt->seen = NIL;
	 if (f&fDELETED) elt->deleted = NIL;
	 if (f&fFLAGGED) elt->flagged = NIL;
	 if (f&fANSWERED) elt->answered = NIL;
       }
    }
    /* Write recording record to file */
    record_write_record_header(stream, RECORD_CLEAR1FLAG, 2);
    record_write_sequence_param(stream, sequence);
    record_write_string_param(stream, flag);
    record_write_record_end(stream);
    sprintf(LOCAL->tmp, "clear1flag: sequence %s, flag = %s",
	    sequence, flag);
  } else {
    sprintf(LOCAL->tmp, "clear1flag: Rejected - Illegal sequence %s", sequence);
    ret = NIL;
  }
  mm_log (LOCAL->tmp, NIL, stream);
  if (oseq) {
    fs_give((void **)&sequence);
    sequence = oseq;
  }
  return ret;
}

void record_clearflag (MAILSTREAM *stream,char *sequence,char *flag,long flags)
{
  MESSAGECACHE *elt;
  short f;
  char *msgseq, *oseq;
  unsigned long uf;

  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR, stream);
    return;
  }
  /* Validate the flag */
  f = record_getflags(stream, flag, &uf);
  if (!f) {
    sprintf(LOCAL->tmp, "clearflag: Rejected - Illegal FLAGS, %s", flag);
    mm_log(LOCAL->tmp, NIL, stream);
    return;
  }
  /* If UID sequence is passed, then make a sequence from it */
  if (flags & ST_UID) {
    msgseq = mail_uidseq_to_sequence(stream, sequence);
    if (*msgseq == NULL) {
      fs_give((void **)&msgseq);
      return;
    }
    oseq = sequence;
    sequence = msgseq;
  } else
    oseq = NIL;
  /* Validate the sequence */
  if (mail_sequence (stream, sequence)) {
    int i;

    for (i = 1; i <= stream->nmsgs; ++i)
      if ((elt = mail_elt(stream, i))->sequence) { /* in the sequence */
         if (f&fSEEN) elt->seen = NIL;
	 if (f&fDELETED) elt->deleted = NIL;
	 if (f&fFLAGGED) elt->flagged = NIL;
	 if (f&fANSWERED) elt->answered = NIL;
       }
    /* Write recording record to file */
    record_write_record_header(stream, RECORD_CLEARFLAG, 2);
    record_write_sequence_param(stream, sequence);
    record_write_string_param(stream, flag);
    record_write_record_end(stream);
    sprintf(LOCAL->tmp, "clearflag: sequence %s, flag = %s",
	    sequence, flag);
  } else {
    sprintf(LOCAL->tmp, "clearflag: Rejected - Illegal sequence %s",
	    sequence);
  }
  mm_log (LOCAL->tmp, NIL, stream);
  if (oseq) {
    fs_give((void **)&sequence);
    sequence = oseq;
  }
}

void record_search (MAILSTREAM *stream,char *charset, SEARCHPGM *pgm,
		    long flags)
{
  unsigned long i;
				/* initialize for search results */
  for (i = 1; i <= stream->nmsgs; ++i) mail_elt(stream, i)->searched = NIL;
  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR, stream);
    return;
  } 
  if (!pgm) return;
  /* Ok. Search locally */
  for (i = 1; i <= stream->nmsgs; ++i)
    if (record_search_msg (stream,i,charset,pgm)) {
      if (flags & SE_UID) mm_searched (stream,mail_uid (stream,i));
      else {		/* mark as searched, notify mail program */
	mail_elt (stream,i)->searched = T;
	mm_searched (stream,i);
      }
    }
}
/* Local mail search message
 * Modifcation of mail_search_msg [by WJY]
 *  ALL SEARCHED DATA MUST BE RESIDENT FOR SEARCH TO SUCCEED.
 *  We are just seaching the cache.
 * Accepts: MAIL stream
 *	    message number
 *	    search charset
 *	    search program
 * Returns: T if found, NIL otherwise
 *
 * Slight modification to use SHORTINFO data when present
 * allows searches on:
 *   rfc822_date, From, Subject, Flags, rfc822_size, and UID.
 *   Only Froma nd Subject must be extracted from the sinfo
 *   structure. The rest are cached in the elt.
 * This will allow us to search from the cache in lieu of
 * fetching the data for searches that only involve the above
 * data.
 */

static STRINGLIST *sstring =NIL;/* stringlist for searching */
static char *scharset = NIL;	/* charset for searching */

long record_search_msg (MAILSTREAM *stream,unsigned long msgno,char *charset,
		      SEARCHPGM *pgm)
{
  unsigned short d;
  unsigned long i,uid;
  char *s;
  MESSAGECACHE *elt = mail_elt (stream,msgno);
  SHORTINFO *sinfo = &mail_lelt (stream,msgno)->sinfo;
  MESSAGECACHE delt;
  SEARCHHEADER *hdr;
  SEARCHSET *set;
  SEARCHPGMLIST *not;
				/* message sequences */
  if (set = pgm->msgno) {	/* must be inside this sequence */
    while (set) {		/* run down until find matching range */
      if (set->last ? ((msgno < set->first) || (msgno > set->last)) :
	  msgno != set->first) set = set->next;
      else break;
    }
    if (!set) return NIL;	/* not found within sequence */
  }
  if (set = pgm->uid) {		/* must be inside this sequence */
    uid = mail_uid (stream,msgno);
    while (set) {		/* run down until find matching range */
      if (set->last ? ((uid < set->first) || (uid > set->last)) :
	  uid != set->first) set = set->next;
      else break;
    }
    if (!set) return NIL;	/* not found within sequence */
  }
				/* require fast data for size ranges */
  if ((pgm->larger || pgm->smaller) && !elt->rfc822_size) {
    return NIL;
  }
				/* size ranges */
  if ((pgm->larger && (elt->rfc822_size <= pgm->larger)) ||
      (pgm->smaller && (elt->rfc822_size >= pgm->smaller))) return NIL;
				/* message flags */
  if ((pgm->answered && !elt->answered) ||
      (pgm->unanswered && elt->answered) ||
      (pgm->deleted && !elt->deleted) ||
      (pgm->undeleted && elt->deleted) ||
      (pgm->draft && !elt->draft) ||
      (pgm->undraft && elt->draft) ||
      (pgm->flagged && !elt->flagged) ||
      (pgm->unflagged && elt->flagged) ||
      (pgm->recent && !elt->recent) ||
      (pgm->old && elt->recent) ||
      (pgm->seen && !elt->seen) ||
      (pgm->unseen && elt->seen)) return NIL;

				/* keywords */
  if (pgm->keyword && !record_search_keyword (stream,elt,pgm->keyword))
    return NIL;
  if (pgm->unkeyword && record_search_keyword (stream,elt,pgm->unkeyword))
    return NIL;
				/* sent date ranges */
  if (pgm->sentbefore || pgm->senton || pgm->sentsince) {
    ENVELOPE *env = record_fetchstructure (stream,msgno,NIL,NIL);
    if (!env) return NIL;
    if (!(env->date && mail_parse_date (&delt,env->date) &&
	  (d = (delt.year << 9) + (delt.month << 5) + delt.day))) return NIL;
    if (pgm->sentbefore && (d >= pgm->sentbefore)) return NIL;
    if (pgm->senton && (d != pgm->senton)) return NIL;
    if (pgm->sentsince && (d < pgm->sentsince)) return NIL;
  }
				/* internal date ranges */
  if (pgm->before || pgm->on || pgm->since) {
    /* If short info/date is cached, then the sum 
     * of the fields will be positive */
    d = (elt->year << 9) + (elt->month << 5) + elt->day;
    if (!d) return NIL;
    if (pgm->before && (d >= pgm->before)) return NIL;
    if (pgm->on && (d != pgm->on)) return NIL;
    if (pgm->since && (d < pgm->since)) return NIL;
  }
				/* search headers */
  if (pgm->bcc) { 
    ENVELOPE *env = record_fetchstructure (stream,msgno,NIL,NIL);
    if (!env) return NIL;
    else  if (!record_search_addr (env->bcc, charset,pgm->bcc, stream)) 
      return NIL;
  }
  if (pgm->cc) { 
    ENVELOPE *env = record_fetchstructure (stream,msgno,NIL,NIL);
    if (!env) return NIL;
    else  if (!record_search_addr (env->cc, charset,pgm->cc,stream)) 
      return NIL;
  }
  /* Use Shortinfo From in lieu of envelope if there */
  if (pgm->from) {
    int from;
    if (sinfo->from) 
      from = record_search_string(sinfo->from,charset,pgm->from, stream);
    else {
      ENVELOPE *env = record_fetchstructure (stream,msgno,NIL,NIL);
      if (!env) return NIL;
      else
	from = record_search_addr (env->from, charset,pgm->from,stream);
    }
    if (!from) return NIL;
  }
  if (pgm->to) { 
    ENVELOPE *env = record_fetchstructure (stream,msgno,NIL,NIL);
    if (!env) return NIL;
    else  if (!record_search_addr (env->to, charset,pgm->to,stream)) 
      return NIL;
  }
  /* Use sinfo->subject if there */
  if (pgm->subject) {
    int subject;
    if (sinfo->subject)
      subject = record_search_string(sinfo->subject,charset,pgm->subject,stream);      
    else {
      ENVELOPE *env = record_fetchstructure (stream,msgno,NIL,NIL);
      if (!env) return NIL;
      else
	subject = record_search_string (env->subject, charset,pgm->subject,stream);
    }
    if (!subject) return NIL;
  }
  if (hdr = pgm->header) {
    STRINGLIST sth,stc;
    sth.next = stc.next = NIL;	/* only one at a time */
    do {			/* check headers one at a time */
      sth.size = strlen (sth.text = hdr->line);
      stc.size = strlen (stc.text = hdr->text);
      s = record_fetchheader (stream,msgno,&sth,&i,FT_INTERNAL);
      if (i == 0 || !record_search_text (s,i,charset,&stc, stream)) return NIL;
    }
    while (hdr = hdr->next);
  }

  /* search strings: LOW memory is not here because we can only
   * search on what is cached. THis could be fixed to give some
   * kind of callback to a user's cache for each msg I guess...
   * A mon avis ce n'est pas la peine!
   *     Bill Yeager  13-sep-95  */
  if (pgm->body) {
    s = record_fetchtext (stream,msgno,&i,FT_INTERNAL+FT_RSEARCH);
    if (i == 0 || !record_search_text (s,i,charset,pgm->body,stream)) 
	return NIL;
  }
  if (pgm->text) {
    /* check the header first */
    s = record_fetchheader (stream,msgno,NIL,&i,FT_INTERNAL);
    if (i == 0 || !record_search_text (s,i,charset,pgm->text,stream)) {
      /* Not in the header, check the body */
      s = record_fetchtext (stream,msgno,&i,FT_INTERNAL+FT_RSEARCH);
      if (i == 0 || !record_search_text (s,i,charset,pgm->text,stream)) 
	return NIL;
    }
  }
  if (pgm->or &&
      !(record_search_msg (stream,msgno,charset,pgm->or->first) ||
	record_search_msg (stream,msgno,charset,pgm->or->second))) 
    return NIL;
  if (not = pgm->not) {
    do if (record_search_msg (stream,msgno,charset,not->pgm))
      return NIL;
    while (not = not->next);
  }
  return T;
}

/* Record search keyword
 * Accepts: MAIL stream
 *	    elt to get flags from
 *	    keyword list
 * Returns: T if search found a match
 */

long record_search_keyword (MAILSTREAM *stream,MESSAGECACHE *elt,STRINGLIST *st)
{
  int i;
  char tmp[MAILTMPLEN],tmp2[MAILTMPLEN];
  do {				/* get uppercase form of flag */
    ucase (strcpy (tmp,st->text));
    for (i = 0;; ++i) {		/* check each possible keyword */
      if (i < NUSERFLAGS && stream->user_flags[i]) {
	if ((elt->user_flags & (1 << i)) &&
	    !strcmp (tmp,ucase (strcpy (tmp2,stream->user_flags[i])))) break;
      }
      else return NIL;
    }
  }
  while (st = st->next);	/* try next keyword */
  return T;
}


/* Record search an address list
 * Accepts: address list
 *	    character set
 *	    string list
 * Returns: T if search found a match
 */

long record_search_addr (ADDRESS *adr,char *charset,STRINGLIST *st,
			MAILSTREAM *stream)
{
  char t[8*MAILTMPLEN];
  t[0] = '\0';			/* initially empty string */
  rfc822_write_address (t,adr);	/* get text for address */
  return record_search_string (t,charset,st, stream);
}


/* Mail search string modified in name only
 * Accepts: text string
 *	    character set
 *	    string list
 * Returns: T if search found a match
 */

long record_search_string (char *txt,char *charset,STRINGLIST *st,
			MAILSTREAM *stream)
{
  /* WJY - 8 aout 95. Crashes if txt is NIL! */
  if (!txt || *txt == NIL) return NIL;
  return record_search_text (txt,(long) strlen (txt),charset,st, stream);
}



/* record search text[modification of mail_search_text]
 * Accepts: text string
 *	    text length
 *	    character set
 *	    string list
 * Returns: T if search found a match
 */

long record_search_text (char *txt,long len,char *charset,
			STRINGLIST *st, MAILSTREAM *stream)
{
  char tmp[MAILTMPLEN];
  if (!(charset && *charset &&	/* if US-ASCII search */
	strcmp (ucase (strcpy (tmp,charset)),"US-ASCII"))) {
    do if (!search (txt,len,st->text,st->size)) return NIL;
    while (st = st->next);
    return T;
  }
  sprintf (tmp,"Unknown character set %s",charset);
  mm_log (tmp,ERROR, stream);
  return NIL;			/* not found */
}

/* Recording mode: ping mailbox
 * Accepts: MAIL stream
 * Returns: Toujours NIL
 */

long record_ping (MAILSTREAM *stream)
{
  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR, stream);
  } else
    mm_log ("record_ping: illegal call", ERROR, stream);
  return NIL;
}
/*
 * echo to the server. Nope. Not when recording */
int record_echo (MAILSTREAM *stream)
{
  NETMBX mb;
  char *s;
  TCPSTREAM *tcpstream;
  char tmp[MAILTMPLEN];
  long imap_defaultport;
  
  mail_stream_setNIL(stream);
  imap_defaultport = (long ) mail_parameters(stream,GET_IMAPPORT,NIL);
  mail_stream_unsetNIL(stream);

  mail_valid_net_parse (stream->mailbox,&mb);
  if (mb.host[0] == '\0') {
    mm_log ("ECHO: Host not specified",ERROR, stream);
    return NIL;
  }
  if (mb.port || imap_defaultport) /* set up host with port override */
    sprintf (s = tmp,"%s:%ld",mb.host,mb.port ? mb.port : 
	     imap_defaultport);
  else s = mb.host;		/* simple host name */
  tcpstream = tcp_open (s,"imap", IMAPTCPPORT, stream);
  if (tcpstream) tcp_close (tcpstream);
  return tcpstream ? T : NIL;
}

/* Recording mode - check
 * Accepts: MAIL stream
 */

void record_check (MAILSTREAM *stream)
{
  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR, stream);
  } else
    mm_log ("record_check: illegal call", ERROR, stream);
}



/* Recording mode - expunge mailbox
 * Accepts: MAIL stream
 */

void record_expunge (MAILSTREAM *stream)
{
  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR, stream);
  } else
    mm_log ("record_check: illegal call", ERROR, stream);
}

/* Recording mode - copy message(s)
 * Accepts: MAIL stream
 *	    sequence
 *	    destination mailbox
 * Returns: T if successful else NIL
 */
long record_copy (MAILSTREAM *stream,char *sequence,char *mailbox,
		  long options)
{
 char *msgseq, *oseq;
  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR, stream);
    return NIL;
  }
  if (!sequence) {
    mm_log("record_copy: Illegal sequence", ERROR, stream);
    return NIL;
  }
  /* If UID sequence is passed, then make a sequence from it */
  if (options & SE_UID) {
    msgseq = mail_uidseq_to_sequence(stream, sequence);
    /* Here we cancel if the sequence is empty */
    if (*msgseq == '\0') {
      fs_give((void **)&msgseq);
      return NIL;
    }
    oseq = sequence;
    sequence = msgseq;
  } else
    oseq = NIL;
  /* Write recording record to file */
  record_write_record_header(stream, RECORD_COPY, 2);
  record_write_sequence_param(stream, sequence);
  record_write_string_param(stream, mailbox);
  record_write_record_end(stream);
  /* See if we must also delete the message */
  if (options & CP_MOVE) 
    record_set1flag(stream, sequence, "\\Deleted", options);
  sprintf(LOCAL->tmp, "record_copy: sequence %s, mailbox = %s",
	  sequence, mailbox);
  mm_log (LOCAL->tmp, NIL, stream);
  if (oseq) {
    fs_give((void **)&sequence);
    sequence = oseq;
  }
  return T;
}
/*
 * Accepts: mail stream
 *	    mailbox name to create
 * Returns: T on success, NIL on failure
 */

long record_create (MAILSTREAM *stream, char *mailbox)
{
  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR, stream);
  }
  if (!mailbox) {
    mm_log("record_create: mailbox required", ERROR, stream);
    return NIL;
  }
  /* Write recording record to file */
  record_write_record_header(stream, RECORD_CREATE, 1);
  record_write_string_param(stream, mailbox);
  record_write_record_end(stream);
  sprintf(LOCAL->tmp, "record_create: mailbox = %s", mailbox);
  mm_log (LOCAL->tmp, NIL, stream);
  return T;
}

/* Record - append message string
 * Accepts: mail stream
 *	    destination mailbox
 *	    stringstruct of message to append
 * Returns: T on success, NIL on failure
 */

long record_append (MAILSTREAM *stream, char *mailbox,
		    char *flags, char *date, STRING *message)
{
  /* NOTE - We can record this call if necessary - */
  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR, stream);
  } else
    mm_log ("record_append: illegal call", ERROR, stream);
  return NIL;
}

/* Recording mode - garbage collect stream
 * Accepts: Mail stream
 *	    garbage collection flags
 */

void record_gc (MAILSTREAM *stream,long gcflags)
{
  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR, stream);
  } else
    mm_log ("record_gc: illegal call", ERROR, stream);
}


/* recording mode - garbage collect stream worker routine
 * Accepts: Mail stream
 *	    garbage collection flags
 */

void record_do_gc (MAILSTREAM *stream,long gcflags)
{
  if (!stream->record) {
    mm_log("Record driver called while NOT in recording mode", ERROR, stream);
  } else
    mm_log ("record_do_gc: illegal call", ERROR, stream);
}


/* Recording mode - garbage collect body texts
 * Accepts: body to GC
 */

void record_gc_body (BODY *body, MAILSTREAM *stream)
{
  mm_log ("record_do_gc: illegal call", ERROR, stream);
}

/* Utilities for recording here */
void make_recordfile_name(MAILSTREAM *stream, char *buf)
{
  RECORD_GLOBALS *rg = (RECORD_GLOBALS *)stream->env_globals;
  char *mbname, c;
  if (rg->record_subdir)
    sprintf(buf, "%s/%s/%s.", myhomedir(stream), rg->record_subdir, 
		RECORDFILENAME);
  else
    sprintf(buf, "%s/%s.", myhomedir(stream), RECORDFILENAME);
  mbname = buf + strlen(buf);
  strcat(mbname, stream->mailbox);
  /* replace '/' with '\', '*', '{' or '}' with # */
  while (c = *mbname) {
    if (c == '/') *mbname = '\\';
    else if (c == '*' || c == '{' || c == '}')
      *mbname = '#';
    ++mbname;
  }
}

int record_imap4(MAILSTREAM *stream)
{
  return stream->imap4_recording_session;
}

/* record globals initializer
 * Accepts: MAIL stream
 * Returns: NONE
 */

void record_global_init (MAILSTREAM *stream)
{
  RECORD_GLOBALS *rg = (RECORD_GLOBALS *)fs_get(sizeof(RECORD_GLOBALS));

  rg->record_subdir = NIL;
  stream->record_globals = rg;
}

/* record globals free 
 * Accepts: MAIL stream
 * Returns: NONE
 */

void record_global_free (MAILSTREAM *stream)
{
 if (stream->record_globals) 
     fs_give ((void **) &stream->record_globals);

}
