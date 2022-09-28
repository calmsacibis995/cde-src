/*
 * This is the code for 16bit 1's complement and rotate
 * checksumming. 
 * 
 * In imap4 this will be used for the UPDATE-NUMBER in the
 * status command. The imap4d will also return unsolicited
 * status responses.
 *
 * But, will still respond to the CHECKSUM command which is 
 * used by roam.
 *
 * 21-nov-96 Only checksum mailboxes which are opened by the imapserver.
 *           see solaris_checksum()
 *[11-06-96 ] mm_notify will always need a stream now.
 *	      Thus a stream is passed instead of a NIL.
 *[11-08-96 ] stream passed in all mm_log calls.
 *
 *[01-14-97 ] stream passed in myhomedir. 
 *
 *[01-15-97 ] Convert check_sum unions from global to local.
 *
 */
/*
 * The checksumming stuff */

#include "UnixDefs.h"
#include <sys/utsname.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include "mail.h"
#include "os_sv5.h"
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "solaris.h"
#include "misc.h"
#include "sun_mime.h"

int solaris_checksum_mbox(MAILSTREAM *stream, unsigned short *cksum,
			  ulong *cssize, unsigned long nbytes);

#ifdef CHECKSUM_DEBUG
#define CKUSER "personne"
#define CKUSER1 "nsimpson"
extern char *user;

void 
solaris_print_chksum_summary(MAILSTREAM * stream, char * name)
{
  int 		i;
  FILE 	*	fp;
  int 		length;

  /* do for just a couple of users and ONLY the inbox */
  if (user == NULL
      || (strcasecmp(user, CKUSER) != 0
	  &&  strcasecmp(user, CKUSER1) != 0)) {
    return;
  } else {
    sprintf(((SOLARISLOCAL*)stream->local)->buf, MAILFILE, user);
    if (strcasecmp(((SOLARISLOCAL*)stream->local)->buf, stream->mailbox) != 0) {
      return;
    }
  }

  sprintf(((SOLARISLOCAL*)stream->local)->buf,
	  "%s/checksum_summary.%s",
	  myhomedir(stream), 
	  name);

  fp = fopen(((SOLARISLOCAL*)stream->local)->buf, "w");
  if (fp == NULL) {
    return;
  }

  rfc822_date(((SOLARISLOCAL*)stream->local)->buf);

  fprintf(fp, "Checksum summary %s\n", ((SOLARISLOCAL*)stream->local)->buf);
  fprintf(fp, "Mailbox %s, %d messages\n", stream->mailbox, stream->nmsgs);
  fprintf(fp, "Msg#   Checksum    bytes  length\n");

  /*          1234xx123456789xx1234567x1234567 */
  for (i = 0, length = 0; i < stream->nmsgs; ++i) {
    int 	chksum;
    int		size;

    chksum = ((SOLARISLOCAL*)stream->local)->msgs[i]->chksum;
    size = ((SOLARISLOCAL*)stream->local)->msgs[i]->chksum_size;
    length += size;
    fprintf(fp, "%4d  %9d  %7d %7d\n", i+1, chksum, size, length);
  }
  fflush(fp);
  fclose(fp);
}
#endif

/*
 * compute checksum:
 *
 *    1. return size of the mbox in bytes
 *    2. checksum of the mbox
 *    3. driver name 
 *  
 * As a function of checksum type we have a fast checksum that
 * uses already accumulated checksums for each message, or the
 * full checksum that redoes the entire checksum. This is only
 * necessary once. Otherwise the checksums for each message are
 * maintained internally to this driver - see for example 
 * solaris_setflags() ...
 */
void
solaris_checksum(MAILSTREAM * stream)
{
  unsigned short 	checksum;
  ulong 		nbytes;
  int 			retval;

  /* If 
   *   (1) mailbox opened by a client, ie, imap server,
   *   (2) read only access
   * then punt
   */
  if (!stream->server_open
      || ((SOLARISLOCAL*)stream->local)->read_access
      || stream->rdonly) {

    if (!(stream->checksum_type & STATUSCHECKSUM)) {
      mm_checksum(stream, 0, 0L, "solaris");
    } else {
      stream->checksum = 0;
      stream->mbox_size = 0;
    }
    return;
  }

  /*
   * if we are doing indexing, then a fast checksum 
   * is all that is necessary
   */
  if (((SOLARISLOCAL*)stream->local)->fullck_done) {
    stream->checksum_type &= ~OPENCHECKSUM;
    stream->checksum_type |= FASTCHECKSUM;
  }

  switch (stream->checksum_type & (OPENCHECKSUM|FASTCHECKSUM)) {
  case OPENCHECKSUM:

  default:
    retval = solaris_checksum_mbox(stream, &checksum, &nbytes, 0);
#ifdef CHECKSUM_DEBUG
    solaris_print_chksum_summary(stream, "open");
#endif
    break;

  case FASTCHECKSUM:
    retval = solaris_checksum_by_msg(stream, &checksum, &nbytes, 0);
    break;
  }

  /*
   * If a bad return, then coerce full checksum on any validchecksum
   * call	
   */
  stream->validchecksum = (retval ? _B_TRUE : _B_FALSE);

  /* keep a copy for indexing */
  if (retval) {
    stream->checksum = checksum;
    stream->mbox_size = nbytes;
  }
  /* for status command we do not call mm_checksum */
  if (!(stream->checksum_type & STATUSCHECKSUM)) {
    mm_checksum(stream, checksum, nbytes, "solaris");
  }
}

/*
 * Compute checksum, and compare it to passed checksum:
 *    If mbox_size != 0  checksum over that many bytes,
 *                       else for the entire file.
 *    Insure that the correct driver is called.
 * if all goes well, then return _B_TRUE.
 */
int
solaris_validchecksum(MAILSTREAM * 	stream,
		      unsigned short 	checksum,
		      unsigned long 	mbox_size)
{
  unsigned short 	loc_chksum;
  ulong 		loc_size;
  
  if (!stream->validchecksum){	/* then full checksum required */
    solaris_checksum_mbox(stream, &checksum, &loc_size, 0);
    stream->validchecksum = _B_TRUE;
  }

#ifdef CHECKSUM_DEBUG
    solaris_print_chksum_summary(stream, "validate");
#endif

  /* The following call returns NIL if the size is not exact */
  stream->checksum_type = NIL;
  if (!solaris_checksum_by_msg(stream, &loc_chksum, &loc_size, mbox_size)) {
    stream->validchecksum = _B_FALSE; /* coerce full checksum next time */
    mm_log("Invalid Checksum length", WARN, stream);
    return(_B_FALSE);
  } else if (loc_chksum != checksum) {
    mm_log("Invalid Checksum", WARN, stream);
    return(_B_FALSE);			/* invalid checksum */
  } else
    return(_B_TRUE);
}

/*
 * structures for checksumming:
 *   32 bit long (trente deux is 32 in French)
 *   unioned with two 16 bit short, carry, sum
 *   which are used to do 1's complement arithmetic.
 */
typedef struct _ck_shorts_ {
  unsigned short carry, sum;
} CKSHORTS;

typedef union {
  unsigned long l;
  CKSHORTS ck_s;
} check_sum;

/* MBOX checksum */
#define CARRY(x) (((check_sum *)(x))->ck_s.carry)
#define CHECKSUM(x) (((check_sum *)(x))->ck_s.sum)
#define TRENTE_DEUX(x) (((check_sum *)(x))->l)

typedef union {
  unsigned long l;
  CKSHORTS ck_t;
} status_cksum;

/* STATUS checksum */
#define S_CARRY(x) (((status_cksum *)(x))->ck_t.carry)
#define S_CHECKSUM(x) (((status_cksum *)(x))->ck_t.sum)
#define S_TRENTE_DEUX(x) (((status_cksum *)(x))->l)

typedef union {
  unsigned long l;
  CKSHORTS ck_u;
} msg_cksum;

/* MESSAGE checksum */
#define M_CARRY(x) (((msg_cksum *)(x))->ck_u.carry)
#define M_CHECKSUM(x) (((msg_cksum *)(x))->ck_u.sum)
#define M_TRENTE_DEUX(x) (((msg_cksum *)(x))->l)

/*
 * checksum bezerk mailbox - 1's complement arithmetic to allow
 * quick checksums when the flags change.
 *
 * Note: All arithmetic is 1's complement resulting in 16 bit
 * checksums. This allows us to quickly checksum a mbox when
 * the flags change in the status of messages. See solaris_quick_checksum().
 *
 * 1. We checksum each messge separately, and the mbox checksum
 * is the then 1's comp. sum of the msg checksums.
 *
 * 2. We checksum the status of each message and add this to the
 *    checksum of the message. Having this separate short
 *    allows us to simply recalculate the checksum of the
 *    message as mentioned above. Note also that the status as it
 *    appears in the mbox does not necessarily reflect the actual
 *    status flags, and as a string is never valid. See the
 *    portion for checksumming the status for clarification.
 *
 *
 * if nbytes == 0, then do entire mailbox, else only nbytes of data.
 * On failure return 0 checksum and size 
 * NOTE: We are doing all of the code inline because we don't
 * want the overhead of procedure calls.
 *
 * We return the checksum and the "checksummable" size of the mbox.
 */
int b_printpart(char *str, unsigned short cs, int cr, int i);
int b_printcs(char *str, unsigned short cs, unsigned short scs,int msgno);
#define ACKTIME 5000
int solaris_checksum_mbox(MAILSTREAM *stream, unsigned short *cksum,
			  ulong *cssize, unsigned long nbytes)
{
  struct stat sbuf;
  int fd;
  char lock[MAILTMPLEN];
  unsigned short word;
  unsigned long size_check;
  int i;
  unsigned short d_chk;		/* for debugging */
  unsigned long d_size;		/*    meme  */
  long size;
  unsigned long time0, time1, time_elapsed;/* Seek correction for broken pseudo msg */
  unsigned long cstime;
  check_sum y; 			/* for MBOX checksum */

  cstime = time0 = elapsed_ms();	/* for our "Server busy ..." messages */

  *cssize = *cksum = 0;		/* default return values */
  /*
   * Append any new mail ... */
  if (!(stream->checksum_type & PARSECALL)) {
    char *perr;
    if ((fd = solaris_parse (stream, lock, LOCK_SH, NIL,"checksum",
			     &perr)) < 0) {
      sprintf(lock,"110 Could not checksum mailbox(%s)",perr);
      mm_log(perr, ERROR,stream);
      return NIL;
    }
  } else fd = ((SOLARISLOCAL*)stream->local)->fd;
  /* NOT used any longer:
   *
   * get the "checksummable" bytesize:
   * Since we checksum from the cache, and the final
   * `\n` is NULLED out for each message in
   * solaris_parse(), we subtract one char for each message, ie,
   * size is used to determine how many bytes to checksum by
   * the requesting client (passed as nbytes above in the call
   * nbytes in subsequent calls. 
   */
  size = (unsigned long)((SOLARISLOCAL*)stream->local)->filesize -  stream->nmsgs;

  /* do the checksum */
  size_check = size; /* sanity check */
  TRENTE_DEUX((check_sum *)(check_sum *)&y) = 0;	/* init cksum long (32 in French) */
  for (i = 0; i < stream->nmsgs; i++) {
    char *msg;
    char *status;		/* real status */
    int status_len;		/* length of above status */
    char *f_status;		/* in mbox */
    char *body;			/* NIL if no body */
    int nwords_to_status;	/* number of 16 bit words to f_status */
    int nbytes_to_status;	/* number of bytes to f_status */
    int even;			/* true if even number of bytes */
    unsigned short *cur_word;	/* current word in our sum */
    unsigned short word;	/* current word we are checksumming */
    int last_word;		/* index of word preceding the status */
    int j;
    int n_crs;
    unsigned char c;
    int change;
    unsigned long msg_size;	/* used to compute the size of the msg */
    int cycles;			/* cycles from status to end-of-msg */
    FILECACHE *m;
    msg_cksum x;		/* our checksum union */
    status_cksum s;             /* for status */
    

    if ((i % 103) == 0) {	/* tous les 103 messages */
      /* Little keep alive messages ... */
      time1 = elapsed_ms();
      time_elapsed = time1 - time0;
      if (time_elapsed >= ACKTIME) {
	mm_notify (stream,"131 Server busy(CHECKSUM)", PARSE);
	time0 = elapsed_ms();     /* reset timer */
      }
    }
    /* If message is not cached, then read it */
    m = ((SOLARISLOCAL*)stream->local)->msgs[i];
    if (!m->msg_cached) {
      if (!solaris_buffer_msg(stream,fd,m,lock,NIL))
	return NIL;
    } 
    /* calculate various part pointers */
    msg = m->internal;
    f_status = msg + m->status_offset;/* in mbox */
    status = m->status;		/* real status */
    if (m->bodysize == 0) body = NIL;
    else 
      body = msg + m->body_offset;

    nbytes_to_status = f_status - msg;
    nwords_to_status = nbytes_to_status/2;
    even = (nbytes_to_status & 1) == 0;
    last_word = nwords_to_status - 1;
    M_TRENTE_DEUX(&x) = 0;	/* clear this message's accumulator */
    cur_word = (unsigned short *)msg; /* start of message */
    size_check -= nbytes_to_status;   /* sanity check */
    msg_size = nbytes_to_status;      /* msg size count */
    /* CHECKSUM a MESSAGE upto the STATUS */
    for (j = 0; j < nwords_to_status; ++j, ++cur_word) {
      word = *cur_word;
      if ((j == last_word) && !even)
	word &= 0xFF00;		/* set unused odd byte to 0 */

      M_TRENTE_DEUX(&x) += word;/* add to 32 bit long */
      if (M_CARRY(&x)) {	/* 1's complement + */
	M_CARRY(&x) = 0;
	M_CHECKSUM(&x) += 1;
      }
      M_TRENTE_DEUX(&x) <<= 1;	/* we now rotate */
      if (M_CARRY(&x)) {
	M_CARRY(&x) = 0;
	M_CHECKSUM(&x) += 1;
      }
    }

    b_printpart("JS: ", M_CHECKSUM(&x), NIL, i);

    /* Since we are checksumming from the filecache, the
     * Status information in the cache has been altered as
     * follows(by examaple):
     *   "\ntatus: RO\nX-Status: D\n\n". This should be
     *   "Status: RO\nX-Status: D\n\n".
     * Also, if the stream is dirty, then the status is
     * not necessarily accurate since flags are not
     * immediately written back to the file. 
     *
     * Finally, the status MAY NOT be present in the file, in which case
     * m->no_status == _B_TRUE. Ca se rend mal au coeur!
     *
     * So, we use the status from the FILECACHE in our checksum.
     *   1. We need to adjust the size by the difference between
     *      the cache status len and the file status len to reflect
     *      the true mbox size.
     */
    if (!(((SOLARISLOCAL*)stream->local)->msgs[i])->no_status) {	/* we have a status line */
      n_crs = 0;
      while (c = *f_status++) {
	if (c == '\n') ++n_crs;
	else
	  n_crs = 0;
	if (n_crs == 2)
	  break;			/* \n\n terminates it */
      }
    } else {
      /* Fix of 9-fev-95. If no body, then *f_status = NIL,
       * and we cannot increment it. f_status points to 
       * where the last of the two status lines WOULD be
       * if it were there, it is not. */
      if (body)
	++f_status;			/* skip to the body */
    }
    /* NOTE: At this point f_status addresses the first byte of data
     * after the status, and is used below to finish up */

    /* status == "Status: ..\nX-Status: ..\nX-UID: ..\n" */
    status_len = strlen(status);        /* Only the "X-Status:" merde  */
    msg_size += status_len;		/* add status len to message size */

    /*
     * Remember: We are checksumming the status line in the message cache
     * which may not be what is in the mbox, and in fact it is possible
     * that there is NO status in the mbox.
     * "change" represents the difference between the OLD status and the
     * CURRENT status in the cache if the cache has changed. This does
     * not apply if the OLD status has length 0 which is true on the
     * first pass through a mbox */
    if ((((SOLARISLOCAL*)stream->local)->msgs[i])->status_len == 0) { /* no current status in mbox */
      /* first time through on open mbox call */
      change = 0;
    } else {
      change = status_len - (((SOLARISLOCAL*)stream->local)->msgs[i])->status_len;
    }
    /* Now set/initialize the current status length */
    (((SOLARISLOCAL*)stream->local)->msgs[i])->status_len = status_len;
    if (!(((SOLARISLOCAL*)stream->local)->msgs[i])->no_status)   /* then, flags have changed */
      size += change;			/* adjust size for the difference */
    else
      /* size is the length of the cached message and does NOT yet
       * include the status. Add it to the returned size */
      size += status_len - 1;	        /* It's not in the file yet */

    size_check += change;		/* adjust our size sanity check */
    size_check -= status_len;		/* and checksumming the status */
    /* We do a byte checksum of the status line and add its value as
     * a short to our accumulated checksum. This is cached so that
     * changes in the status can be checksummed without checksumming
     * the entire file. Still 1's complement arithmetic.
     */
    S_TRENTE_DEUX(&s) = 0;		/* clear the long accumulator */
    while (c = *status++) {	
      S_TRENTE_DEUX(&s) += c;	/* add to 32 bit long */
      if (S_CARRY(&s)) {
	S_CARRY(&s) = 0;
	S_CHECKSUM(&s) += 1;
      }
      S_TRENTE_DEUX(&s) <<= 1;	/* we now rotate */
      if (S_CARRY(&s)) {
	S_CARRY(&s) = 0;
	S_CHECKSUM(&s) += 1;
      }
    }
    /*
     * Add in the status checksum (1's complement),
     * Save the status sum in the local FILECACHE,
     * and checksum
     * the rest of the message ... Est-ce que tu en crois? */
    (((SOLARISLOCAL*)stream->local)->msgs[i])->status_chksum = S_CHECKSUM(&s);
    M_TRENTE_DEUX(&x) += S_CHECKSUM(&s);
    if (M_CARRY(&x)) {
      M_CHECKSUM(&x) += 1;
      M_CARRY(&x) = 0;
    }
    M_TRENTE_DEUX(&x) <<= 1;
    if (M_CARRY(&x)) {
      M_CHECKSUM(&x) += 1;
      M_CARRY(&x) = 0;
    }

    b_printpart("SCS: ", S_CHECKSUM(&s), NIL, i);
    b_printpart("CS+S: ", M_CHECKSUM(&x), T, i);

    cycles = 1;			/* cycle count of status checksum */

    /* checksum the rest of the message. The first byte may not be
     * on a word boundary. If not, then we gather successive bytes
     * into a short and checksum until done.
     * (C'est difficile a croire que cet algorithme marche, non! */
    if ((unsigned long)f_status & 1) { /* it's odd */
      union {
	unsigned short w;
	struct BYTE_SUM {
	  unsigned char a, b;
	} b_s;
      } zut;

      while (c = *f_status++) {
	if (!c) 		/* high order byte NIL */
	  break;

	zut.b_s.a = c;		/* stash this one */
	zut.b_s.b = *f_status++;/* prochain - next dude */

	M_TRENTE_DEUX(&x) += zut.w; /* add in the word */
	if (M_CARRY(&x)) {
	  M_CHECKSUM(&x) += 1;
	  M_CARRY(&x) = 0;
	}
	M_TRENTE_DEUX(&x) <<= 1;
	if (M_CARRY(&x)) {
	  M_CHECKSUM(&x) += 1;
	  M_CARRY(&x) = 0;
	}
	cycles += 1;		/* count the cycles */
				/* check end condition */
	if (!zut.b_s.b) {	/* had <byte><NIL> */
	  size_check -= 1;	/* sanity check encore */
	  msg_size += 1;	/* msg size counter  */
	  break;
	} else {		/* had <byte><byte> */
	  size_check -= 2;	/* sanity check encore */
	  msg_size += 2;	/* msg size counter  */
	}	  
      }
      b_printpart("FS: ", M_CHECKSUM(&x), T, i);
    } else {			/*on short boundary, sum shorts-peu quicker */
      cur_word = (unsigned short *)f_status;
      /* Do the rest of the message as before. We terminate
       * on a NULL byte */
      last_word = NIL;
      while (word = *cur_word) {
	char *voir= (char *)cur_word;
	
	/* Since each message is NIL terminated:
	 *   look for <NIL><char> --> done now OR
	 +            <char><NIL> --> done after this byte 
	 *               In such cases the garbage byte is 0 */
	if (*voir++ == NIL)
	  break;			/* done now */
	else 
	  if (*voir == NIL)
	    last_word = _B_TRUE;	/* after this byte */
	
	M_TRENTE_DEUX(&x) += word;	/* add to 32 bit long */
	if (M_CARRY(&x)) {
	  M_CARRY(&x) = 0;
	  M_CHECKSUM(&x) += 1;
	}
	M_TRENTE_DEUX(&x) <<= 1;	/* we now rotate */
	if (M_CARRY(&x)) {
	  M_CARRY(&x) = 0;
	  M_CHECKSUM(&x) += 1;
	}
	
	b_printpart("LW: ", M_CHECKSUM(&x), T, i);
	
	cycles += 1;		/* one more cycled short */
	if (last_word) {
	  msg_size += 1;
	  size_check -= 1;
	  break;
	} else {
	  msg_size += 2;
	  size_check -= 2;
	}
	++cur_word;		/* next short */
      }				/* Fin de la fin de message */
    }

    /* save the totals so far */
    (((SOLARISLOCAL*)stream->local)->msgs[i])->chksum_size = msg_size;
    *cssize += msg_size;		/* total checksummed size */

    /* save the message checksum */
    (((SOLARISLOCAL*)stream->local)->msgs[i])->chksum = M_CHECKSUM(&x);
					/*16 (0 <= x <= 15) cycles max for 16 bits */
    (((SOLARISLOCAL*)stream->local)->msgs[i])->quick_chksum_cycles = cycles & 0xF;

    b_printcs("mbox: ", M_CHECKSUM(&x), S_CHECKSUM(&s), i+1);

    TRENTE_DEUX((check_sum *)&y) += M_CHECKSUM(&x);
    if (CARRY((check_sum *)&y)) {
      CHECKSUM((check_sum *)&y) += 1;
      CARRY((check_sum *)&y) = 0;
    }
    TRENTE_DEUX((check_sum *)&y) <<= 1;
    if (CARRY((check_sum *)&y)) {
      CHECKSUM((check_sum *)&y) += 1;
      CARRY((check_sum *)&y) = 0;
    }
    /* Checksum next message au haut - for (i..) loop */
  }
  *cksum = (long)CHECKSUM((check_sum *)&y);
  if (!(stream->checksum_type & PARSECALL)) {
    solaris_unlock (fd, NIL, lock);
    mail_unlock (stream);	/* and stream */
  }
  /*
   * log time for checksumming */
  cstime = (elapsed_ms() - cstime)/1000;
  sprintf(lock,"Checksum time = %d secs",cstime);
  mm_notify(stream,lock,PARSE);  /* Pass a stream -clin */
  /* free our mapped memory */
  if (PARSE_SIZE > CHUNK) {
    solaris_give(PARSE_DATA);
    solaris_get(CHUNK,PARSE_DATA);
  }
  return(_B_TRUE);
}

/*
 * Here we checksum a single message, and update the FILECACHE for
 * that message. This function is called for things like new messages.
 *
 * imports the MAILSTREAM and he message number, i, fd of mbox, and
 * current lock name.
 */
int solaris_checksum_msg(MAILSTREAM *stream, unsigned long i,
			 int fd,char *lock,int resize)
{
  char *msg;
  char *status;
  int status_len;		/* length of above status */
  char *f_status;
  int nwords_to_status;	/* number of 16 bit words to f_status */
  int nbytes_to_status;	/* number of bytes to f_status */
  int even;			/* true if even number of bytes */
  unsigned short *cur_word;	/* current word in our sum */
  unsigned short word;	/* current word we are checksumming */
  int last_word;		/* index of word preceding the status */
  int j;
  int n_crs;
  char c;
  unsigned long msg_size;
  int cycles;			/* cycles of status checksum short */
  unsigned short d_chk;		/* DEBUGGING variable */
  FILECACHE *m;

  status_cksum s;		/* STATUS checksum */
  msg_cksum x;			/* MSG checksum */

  /* We assume that i is correct because we can be called
   * before stream->nmsgs has been updated to reflect the
   * actual number of messages. The data structures that
   * are relevant have been created though. This is in
   * solaris_parse(..). */
  i -= 1;			/* turn into an index */
  m = ((SOLARISLOCAL*)stream->local)->msgs[i];
				/* If not cached, then read the mesasge */
  if (!m->msg_cached) {
				/* T forces original values */
    if (!solaris_buffer_msg(stream,fd,m,lock,T))
      return NIL;
  }
  msg = m->internal;
				/* real status - may not be written */
  status = m->status;
				/* status (almost) as written in mbox */
  f_status =  msg + m->status_offset;
				/* for computing the size of the message */

  nbytes_to_status = f_status - msg;
  nwords_to_status = nbytes_to_status/2;
  even = (nbytes_to_status & 1) == 0;
  last_word = nwords_to_status - 1;
  cur_word = (unsigned short *)msg; /* start of message */
  msg_size = nbytes_to_status;   /* count bytes to status line */

  /* CHECKSUM a MESSAGE upto the STATUS */
  M_TRENTE_DEUX(&x) = 0; /* clear this message's accumulator */
  for (j = 0; j < nwords_to_status;++j, ++cur_word) {
    /* If last word, then see if both bytes in the last word are viable */
    word = *cur_word;
    if ((j == last_word) && !even) 
      word &= 0xFF00;		/* Nset unused odd byte to 0 */
    M_TRENTE_DEUX(&x) += word;	/* add to 32 bit long */
    if (M_CARRY(&x)) {		/* 1's complement + */
      M_CARRY(&x) = 0;
      M_CHECKSUM(&x) += 1;
    }
    M_TRENTE_DEUX(&x) <<= 1;	/* we now rotate */
    if (M_CARRY(&x)) {
      M_CARRY(&x) = 0;
      M_CHECKSUM(&x) += 1;
    }
  }
  /* Since we are checksumming from the filecache, the
   * Status information in the cache has been altered as
   * follows(by examaple):
   *   "\ntatus: RO\nX-Status: D\nX-UID: 0000000069\n\n". This should be
   *   "Status: RO\nX-Status: D\nX-UID: 0000000069\n\n".
   * Also, if the stream is dirty, then the status is
   * not necessarily accurate since flags are not
   * immediately written back to the file. 
   *
   * So, we use the status from the FILECACHE in our checksum.
   *   1. We need to adjust the size by the difference between
   *      the cache status len and the file status len to reflect
   *      the true mbox size */
  status_len = strlen(status);			  /* Only "X-Status:" merde */
  m->status_len = status_len; /* update this length */
  if (!m->no_status) {	/* we have a status line */
    n_crs = 0;
    while (c = *f_status++) {
      if (c == '\n') ++n_crs;
      else
	n_crs = 0;
      if (n_crs == 2)
	break;			/* \n\n terminates it */
    }
  } else {
    ++f_status;			/* skip to the body */
  }
  /* NOTE: At this point f_status addresses the first byte of data
   * after the status, and is used below to finish up */
  msg_size += status_len;	/* add size of status */

  /* We do a byte checksum of the status line and add its value as
   * a short to our accumulated checksum. This is cached so that
   * changes in the status can be checksummed without checksumming
   * the entire file. Still 1's complement arithmetic */
  S_TRENTE_DEUX((status_cksum *)&s) = 0;	/* clear the long accumulator */
  while (c = *status++) {	
    S_TRENTE_DEUX((status_cksum *)&s) += c;		/* add to 32 bit long */
    if (S_CARRY((status_cksum *)&s)) {
      S_CARRY((status_cksum *)&s) = 0;
      S_CHECKSUM((status_cksum *)&s) += 1;
    }
    S_TRENTE_DEUX((status_cksum *)&s) <<= 1;	/* we now rotate */
    if (S_CARRY((status_cksum *)&s)) {
      S_CARRY((status_cksum *)&s) = 0;
      S_CHECKSUM((status_cksum *)&s) += 1;
    }
  }
  /*
   * Add in the status checksum (1's complement),
   * Save the status sum in the local FILECACHE,
   * and  checksum
   * the rest of the message ... Est-ce que tu en crois? */
  m->status_chksum = S_CHECKSUM(&s);
  M_TRENTE_DEUX(&x) += S_CHECKSUM(&s);
  if (M_CARRY(&x)) {
    M_CHECKSUM(&x) += 1;
    M_CARRY(&x) = 0;
  }
  M_TRENTE_DEUX(&x) <<= 1;
  if (M_CARRY(&x)) {
    M_CHECKSUM(&x) += 1;
    M_CARRY(&x) = 0;
  }
  /* We count the cycles for each word checksummed until the end
   * of the message. This is used in quick checksumming */
  cycles = 1;
  /* checksum the rest of the message. The first byte may not be
   * on a word boundary. If not, then get odd byte only
   * (C'est difficile a croire que cet algorithme marche, non! */
  if ((unsigned long)f_status & 1) { /* it's odd */
    union {
      unsigned short w;
      struct BYTE_SUM {
	unsigned char a, b;
      } b_s;
    } zut;
    unsigned char c;
    
    while (c = *f_status++) {
      if (!c) 		/* high order byte NIL */
	break;
      
      zut.b_s.a = c;		/* stash this one */
      zut.b_s.b = *f_status++;/* prochain - next dude */
      
      M_TRENTE_DEUX(&x) += zut.w; /* add in the word */
      if (M_CARRY(&x)) {
	M_CHECKSUM(&x) += 1;
	M_CARRY(&x) = 0;
      }
      M_TRENTE_DEUX(&x) <<= 1;
      if (M_CARRY(&x)) {
	M_CHECKSUM(&x) += 1;
	M_CARRY(&x) = 0;
      }
      cycles += 1;		/* count the cycles */
				/* check end condition */
      if (!zut.b_s.b) {	/* had <byte><NIL> */
	msg_size += 1;	/* msg size counter  */
	break;
      } else {		/* had <byte><byte> */
	msg_size += 2;	/* msg size counter  */
      }	  
    }
  } else {
    cur_word = (unsigned short *)f_status;
    /* Do the rest of the message as before. We terminate
     * on a NULL byte */
    last_word = NIL;
    while (word = *cur_word) {
      char *voir= (char *)cur_word;
      
      /* Since each message is NIL terminated:
       *   look for <NIL><char> --> done now OR
       +            <char><NIL> --> done after this byte 
       *               In such cases the garbage byte is 0 */
      if (*voir++ == NIL)		/* <NIL><char> */
	break;			/* done now */
      if (*voir == NIL) {	/* <char><NIL> */
	last_word = _B_TRUE;
	msg_size += 1;
      } else
	msg_size += 2;		/* <char><char> */
      
      M_TRENTE_DEUX(&x) += word;/* add to 32 bit long */
      if (M_CARRY(&x)) {
	M_CARRY(&x) = 0;
	M_CHECKSUM(&x) += 1;
      }
      M_TRENTE_DEUX(&x) <<= 1;	/* we now rotate */
      if (M_CARRY(&x)) {
	M_CARRY(&x) = 0;
	M_CHECKSUM(&x) += 1;
      }
      cycles += 1;
      if (last_word)
	break;			/* odd-byte NIL, this message done */
      ++cur_word;			/* next short */
    }				/* Fin de la fin de message */
  }
  /* DEBUGGING STUFF */
  d_chk = M_CHECKSUM(&x);
  d_chk = m->chksum;
  /* END DEBUGGING STUFF */

  /* save message checksum and msg checksum size */
  m->chksum = M_CHECKSUM(&x);
  m->chksum_size = msg_size;
  m->quick_chksum_cycles = cycles & 0xF;

  b_printcs("msg: ", M_CHECKSUM(&x), S_CHECKSUM(&s), i+1);
  /* free our mapped memory:
   *    added resize  WJY++ 16-dec-96
   */
  if (resize && PARSE_SIZE > CHUNK) {
    solaris_give(PARSE_DATA);
    solaris_get(CHUNK,PARSE_DATA);
    ((SOLARISLOCAL*)stream->local)->max_pchunk = CHUNK;
  }
  return(_B_TRUE);
}
/*
 * checksum by 1's complement summation of the checksums of
 * each message.
 *
 * if size_check != 0, then if our checksummed size >= size_check
 * we stop checksumming,
 *
 * return NIL if size_check != 0 AND mbox_size != size_check.
 *
 * Used after an expunge for example, or for validating the
 * checksum of size_check bytes:
 *   We can be called DIRECTLY from solaris_parse. IN this case we
 *   have already locked things up. */
#define PETITBUF 128
int solaris_checksum_by_msg(MAILSTREAM *stream, 
			    unsigned short *chksum, 
			    unsigned long *size,
			    unsigned long size_check)
{
  int i;
  unsigned long mbox_size= 0;
  int fd;
  char lock[MAILTMPLEN];
  int parsing = stream->checksum_type & PARSECALL;
  check_sum y;		/* for MBOX checksum */

  if (!parsing) {
    char *perr;
    if ((fd = solaris_parse (stream, lock, LOCK_SH, NIL, "checksum by msg",
			     &perr)) < 0) {
      sprintf(lock,"110 Could not checksum mailbox(%s)",perr);
      mm_log(perr, ERROR,stream);
      return NIL;
    }
  } else fd = ((SOLARISLOCAL*)stream->local)->fd;
  /* force a parse mailbox because new messages may have been
   * appended by a copy ... */


  TRENTE_DEUX((check_sum *)&y) = 0;
  for (i = 0; i < stream->nmsgs; ++i) {
    TRENTE_DEUX((check_sum *)&y)  += (((SOLARISLOCAL*)stream->local)->msgs[i])->chksum;
    if (CARRY((check_sum *)&y)) {
      CARRY((check_sum *)&y) = 0;
      CHECKSUM((check_sum *)&y) += 1;
    }
    TRENTE_DEUX((check_sum *)&y) <<= 1;	/* we now rotate */
    if (CARRY((check_sum *)&y)) {
      CARRY((check_sum *)&y) = 0;
      CHECKSUM((check_sum *)&y) += 1;
    }
    mbox_size += (((SOLARISLOCAL*)stream->local)->msgs[i])->chksum_size;
    if (size_check != 0 & mbox_size >= size_check)
      break;
  }
  *size = mbox_size;
  *chksum = CHECKSUM((check_sum *)&y);

  if (!parsing) {
    solaris_unlock (fd, NIL, lock);
    mail_unlock (stream);	/* and stream */
  }
  /*
   * mbox_size is the size of the mailbox the requestor had
   * associated with the checksum. It must always be precisely
   * equal to size check because both end on a message boundary */
  if (size_check != 0 && mbox_size != size_check) {
    return(_B_FALSE);
  }
  return(_B_TRUE);
}
/* 
 * Here the flags have changed. We do a quick checksum by taking
 * advantage of 1's complement arithmetic.
 */
int solaris_quick_checksum(MAILSTREAM *stream, long msgno)
{
  unsigned short old_checksum;
  int cycles;
  int status_len;
  char *status;
  char c;
  long i;
  int n_crs;
  int r;
  int change;
  int vieux, saved;		/* DEBUGGING STUFF */
  unsigned short difference;
  union {
    unsigned long l;
    CKSHORTS r;
  } rotate;
  union {
    unsigned long l;
    CKSHORTS d;
  } moins;
  status_cksum s;		/* for status checksum */ 

  if ( msgno < 1 || msgno > stream->nmsgs) /* sanity check */
    return NIL;
  i = msgno - 1;

  status = (((SOLARISLOCAL*)stream->local)->msgs[i])->status;
  status_len = strlen(status);
  (((SOLARISLOCAL*)stream->local)->msgs[i])->status_len = status_len; /* update this length */

  /* checksum the new status */
  S_TRENTE_DEUX((status_cksum *)&s) = 0;	/* clear the long accumulator */
  while (c = *status++) {	
    S_TRENTE_DEUX((status_cksum *)&s) += c;		/* add to 32 bit long */
    if (S_CARRY((status_cksum *)&s)) {
      S_CARRY((status_cksum *)&s) = 0;
      S_CHECKSUM((status_cksum *)&s) += 1;
    }
    S_TRENTE_DEUX((status_cksum *)&s) <<= 1;	/* we now rotate */
    if (S_CARRY((status_cksum *)&s)) {
      S_CARRY((status_cksum *)&s) = 0;
      S_CHECKSUM((status_cksum *)&s) += 1;
    }
  }

  /* take the 1's complement difference (new "1's minus" old) */
  old_checksum = (((SOLARISLOCAL*)stream->local)->msgs[i])->status_chksum;
  moins.l = S_CHECKSUM((status_cksum *)&s);	/* new status checksum */
  moins.l -= old_checksum;	/* less old */
  if (S_CHECKSUM((status_cksum *)&s) < old_checksum) {/* new < old */
    moins.l -= 1;		  /* 2's complement to 1's comp */
  } else
    if (moins.d.carry)		/* check carry */
      moins.d.sum += 1;

  difference = moins.d.sum;	/* difference */

  (((SOLARISLOCAL*)stream->local)->msgs[i])->status_chksum = S_CHECKSUM((status_cksum *)&s);	
				/* save new status cksum */

  /* rotate this difference (cycles MOD 16) times */
  cycles = (((SOLARISLOCAL*)stream->local)->msgs[i])->quick_chksum_cycles & 0xF;
  rotate.l = difference;
  for (r = 0; r < cycles; ++r) {
    rotate.l <<= 1;
    if (rotate.r.carry) {
      rotate.r.sum += 1;
      rotate.r.carry = 0;
    }
  }
  /* 1's comp. add this rotated value to the old checksum, et voila! */
  moins.l = (((SOLARISLOCAL*)stream->local)->msgs[i])->chksum;
  moins.l += rotate.r.sum;
  if (moins.d.carry)
    moins.d.sum += 1;

  /* save the new checksum */
  (((SOLARISLOCAL*)stream->local)->msgs[i])->chksum = moins.d.sum;

  b_printcs("quick: ", moins.d.sum, S_CHECKSUM((status_cksum *)&s), i+1);

  return _B_TRUE;
}

int b_printcs(char *str, unsigned short cs, unsigned short scs,int msgno)
{
  char tmp[MAILTMPLEN];

  if (msgno != 1 || T) return NIL;

  sprintf(tmp, "-->%8s msg# %d cs %d scs %d\n", str, msgno, cs, scs);
  printf(tmp);
}

int b_printpart(char *str, unsigned short cs, int cr, int i)
{
  char tmp[MAILTMPLEN];

  if (i != 0 || T) return NIL;

  sprintf(tmp, "[%8s value %d] ", str, cs);
  printf(tmp);
  if (cr) printf("\n");
}
