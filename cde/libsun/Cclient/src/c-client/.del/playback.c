/*
 * Program:	Record-mode playback mail routines
 *
 * Author:	William J. Yeager
 *		Independent consultant
 * Copyright 1993 by Sun Micro Systems, Inc.
 *
 * Edit Log:  
 * 	[09-23-96 clin ] strtok_r is used to ensure MT-Safe.
 * 	[1l-08-96 clin ] Pass stream in all mm_log calls and
 *			replay_empty 
 */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mail.h"
#include "os_sv5.h"
#include "record.h"
#include "misc.h"

typedef enum {
  bad_file_format = 1,
  got_next_block = 2,
  eof_on_file = 3
} RECORD_RESULT;

#define OPENERROR 1
#define EOFERROR  2
#define FORMATERROR 3
#define READONLYERROR 4
#define MAXERRNO 4
static char *errors[]= {
  "137 IMAP4: Unknown playback error",
  "137 IMAP4: Could not open recording file",
  "137 IMAP4: Replay aborted: Unexpected EOF for recording",
  "137 IMAP4: Replay aborted: Illegal record file format",
  "144 IMAP4: Replay aborted: Mailbox is read only",
};

/* forward declarations */
/* the imap2 and imap4 replayers */
int replay_imap2recording(MAILSTREAM *stream, FILE *rfile, int cksum_ok,
			  int rfile_ok, char *buf,int blen);
int replay_imap4recording(MAILSTREAM *stream, FILE *rfile, int cksum_ok,
			  int uid_valid, int rfile_ok, char *buf,int blen);
RECORD_RESULT next_recorded_command(MAILSTREAM *stream, FILE *rfile, RECORDBLK **rblk, 
				    char *buf, int blen,
				    long flags);
void playback_block(MAILSTREAM *stream, RECORDBLK *rblk, char *tmp);
/* The trucs we play back */
void playback_setflag(MAILSTREAM *stream, PLIST *params);
void playback_clearflag(MAILSTREAM *stream, PLIST *params);
void playback_set1flag(MAILSTREAM *stream, PLIST *params);
void playback_clear1flag(MAILSTREAM *stream, PLIST *params);
void playback_copy(MAILSTREAM *stream, PLIST *params);
void playback_create(MAILSTREAM *stream, PLIST *params);
void playback_delete(MAILSTREAM *stream, PLIST *params);
void playback_rename(MAILSTREAM *stream, PLIST *params);

int convert_ptype(char *ptype);
int fstr_to_type(char *function);
void free_recordblk(RECORDBLK **rblk);
int replay_checksum_valid(MAILSTREAM *stream, FILE *rfile, char *tmp);
int replay_uidvalidity_ok(MAILSTREAM *stream, FILE *rfile, char *tmp);
int reply_empty(FILE *rfile, char *tmp);
void closeupshop(MAILSTREAM *stream, int errno, FILE *rfile, char *fname);
int validate_recording_file (MAILSTREAM *stream, FILE *rfile, char *rname, 
			     int *imap4recording,
			     char *tmp);
void replay_free_cache(MAILSTREAM *stream, unsigned long msgno, 
		      long num_to_free);
void free_rlist(RECORDBLKLIST *rlist);

void mail_free_playback_cache (MAILSTREAM *stream);
int playback_synchronize(MAILSTREAM *stream);

/* Our default cache handler is the one that is used by mail.c */
static mailcache_t mailcache;

int play_it_back_sam(MAILSTREAM *stream)
{
  FILE *rfile;
  char fname[MAILTMPLEN], tmp[MAILTMPLEN];
  char *input;
  char *cp;
  int imap4recording;
  int nothing_to_replay;
  int what_happened;
  int cksum_ok;
  int uid_valid;
  int flushcache;
  int rfile_ok;
  int imap4support;
  char *pbuf;				/* 15-feb-97 */
  int pblen;				/* 15-feb-97 */
  struct stat sbuf;			/* 15-feb-97 */

  if (stream->checkpointed || stream->record) {
    mm_log("playback: Cannot replay checkpointed/record stream", NIL,stream);
    return FAILEDPLAYBACK;
  }
  make_recordfile_name(stream, fname);
  rfile =  fopen(fname, "r");
  if (!rfile) {
    mm_log(errors[OPENERROR], ERROR,stream);
    /* flush the cache off all saved information */
    mail_free_playback_cache(stream);
    return PLAYBACKLOCALERROR;
  } else if (fstat(fileno(rfile),&sbuf) < 0) { /* 15-feb-97 */
    mm_log(errors[OPENERROR], ERROR,stream);
    /* flush the cache off all saved information */
    mail_free_playback_cache(stream);
    return PLAYBACKLOCALERROR;
  } else pblen = 2*sbuf.st_size;

  /*
   * Validate the recording file. VALIDRECORDING returned if OK */
  what_happened = validate_recording_file(stream, rfile, fname, 
					  &imap4recording, tmp);
  rfile_ok = (what_happened == VALIDRECORDING ? T : NIL);
  /*
   * If we are doing an imap2bis playback to an imap2bis server,
   * then we can only do the checksum thing.
   *
   * If this is an imap4 playback to an imap4 server, then
   * we can always playback. But, if the checksum is valid,
   * then we do not have to synchronize the cache. 
   */
				/* See if our checksum is OK */
  cksum_ok = replay_checksum_valid(stream, rfile, tmp);
  uid_valid = replay_uidvalidity_ok(stream, rfile, tmp);
				/* Cannot playback read/only files */
  if (stream->rdonly) {
    closeupshop(stream, READONLYERROR, rfile, fname);
    if (cksum_ok)
      return CANNOTPLAYBACKRO;
    else {
      mail_free_playback_cache(stream);
      return FLUSHCACHEONLY;
    }
  }
  /* Get the mailcache manipulation function */
  mailcache = (mailcache_t)mail_parameters(stream,GET_CACHE,NIL);
  /* If we are taking to an imap2 server and were originally
   * connect to an imap4 server, then we reset imap4recording
   * and do an imap2 playback */
  imap4support = mail_imap4support(stream);
					/* allocate buffer */
  pbuf = fs_get(pblen);			/* 15-feb-97 */
  if (imap4recording && imap4support)
    what_happened  = replay_imap4recording(stream, rfile, cksum_ok, 
					   uid_valid, 
					   rfile_ok,
					   pbuf,pblen);
  else
    /* Here we can replay to either imap2 or imap4 servers.
     * If chksum_ok, then all goes well */
    what_happened = replay_imap2recording(stream, rfile, cksum_ok, 
					  rfile_ok,
					  pbuf,pblen);
  fs_give((void **)&pbuf);		/* 15-feb-97 free the buffer */
					/* Now do the replay */
  switch (what_happened) {
  case NOTHINGTOPLAYBACK:
  case SUCCESSFULLPLAYBACK:
    mm_log("Replay successfully completed", NIL,stream);
    mm_log_stream(stream, "139 IMAP4: Replay succeeded");
    flushcache = NIL;
    break;
  case FLUSHCACHEONLY: 
    mm_log("Cache Synchronization error(Replay successfully completed)",
	   NIL, stream);
    mm_log_stream(stream, "141 IMAP4: Cache Sync error");
    flushcache = T;
    break;
  case FAILEDPLAYBACK:
    mm_log("Playback failed. Mailbox corrupted!", ERROR,stream);
    mm_log_stream(stream, "142 IMAP4: Playback failed");
    flushcache = T;
    break;
  case PLAYBACKLOCALERROR:
    mm_log("Playback failed. Local system error!", ERROR,stream);
    mm_log_stream(stream, "143 IMAP4: Local system error");
    flushcache = T;
    break;
  }
  fclose(rfile);
  unlink(fname);
  if (flushcache) 
    /* flush the cache off all saved information */
    mail_free_playback_cache(stream);
  return what_happened;
}
/*
 * Validate the first line of the recording file. It should
 * be "IMAP4-CLIENT:<servertype>" */
int validate_recording_file (MAILSTREAM *stream, FILE *rfile, char *fname, 
			     int *imap4recording,
			     char *tmp) {
  char *cp, *input;
  char tmp1[MAILTMPLEN];
  char *lasts;
				/* validate file/server type */
  input = fgets(tmp, MAILTMPLEN, rfile);
  if (!input) {
    closeupshop(stream, EOFERROR, rfile, fname);
    return NOTHINGTOPLAYBACK;
  }
  cp = strtok_r(tmp, RECORDDELIMETER, &lasts);
  if (!cp) {
    closeupshop(stream, FORMATERROR, rfile, fname);
    return NOTHINGTOPLAYBACK;
  }
				/* Check the INTRODUCTION */
  if (strcmp(cp, RECORDINTRO)) {
    closeupshop(stream, FORMATERROR, rfile, fname);
    return  NOTHINGTOPLAYBACK;
  } else {
				/* Now, get the server type */
    cp = strtok_r(NIL, "\n", &lasts);
    if (!cp) {
      closeupshop(stream, FORMATERROR, rfile, fname);
      return NOTHINGTOPLAYBACK;
    }
    if (strcmp(IMAP4RECORD, cp) == 0)
      *imap4recording = T;
    else if (strcmp(IMAP2RECORD, cp) == 0)
      *imap4recording = NIL;
    else {
      closeupshop(stream, FORMATERROR, rfile, fname);
      return NOTHINGTOPLAYBACK;
    }      
  }
				/* fetch the recording date */
  input = fgets(tmp, MAILTMPLEN, rfile);
  if (!input) {
    closeupshop(stream, EOFERROR, rfile, fname);
    return NOTHINGTOPLAYBACK;
  }
  sprintf(tmp1, "Replay: %s ", fname);
  mm_log(tmp1, NIL,stream);
  mm_log_stream(stream, "138 IMAP4: Playback started");
  sprintf(tmp1, "Created on %s", input);
  cp = strrchr(tmp1, '\n');
  if (cp)			/* null terminating '\n' */
    *cp = '\0';
  mm_log(tmp1, NIL,stream);
  return VALIDRECORDING;
}

/*
 * error on the opening */
void closeupshop(MAILSTREAM *stream, int errno, FILE *rfile, char *fname)
{
  if (errno > MAXERRNO) errno = 0;
  mm_log(errors[errno], ERROR,stream);
  mm_log_stream(stream, errors[errno]);
  fclose(rfile);
  unlink(fname);
}

int replay_getline(FILE *rfile, char *inbuf, MAILSTREAM *stream) 
{
  char *input = fgets(inbuf, MAILTMPLEN, rfile);
  if (!input) {
    mm_log("Replay input: Unexpected EOF", ERROR,stream);
    return NIL;
  } else {
    char *cp= strrchr(inbuf, '\n');
    if (cp)
      *cp = '\0';
    return T;
  }
}
/*
 * Peek ahead in the recording file to see if it is
 * empty - nothing recorded
 *   expect:
 *   checksum
 *   mailbox size in bytes
 *   driver name
 *   DATA  */
#define NREQUIREDLINES 4
int replay_empty(FILE *rfile, char *tmp, MAILSTREAM *stream)
{
  long saved_pos = ftell(rfile);
  int i;
  int no_replay = NIL;

  /* read the required number of lines for replaying
   * data . If not all there, then replay is empty.
   */
  for (i = 0; i < NREQUIREDLINES; ++i) {
    if (!replay_getline(rfile, tmp, stream)) {
	no_replay = T;
	break;
      }
  }
  /* Put file pointer it back where it was */
  if (fseek(rfile, saved_pos, SEEK_SET) < 0)
    return T;                   /* Seek error. No replay possible. */
  else
    return no_replay;           /* return replay status */
}
/*
 * replay_checksum_valid:
 *
 * Validate saved checksum for a consistency check */
int replay_checksum_valid(MAILSTREAM *stream, FILE *rfile, char *tmp)
{
  unsigned long checksum, mbox_size;
				/* Read the data from the recording */
  if (!replay_getline(rfile, tmp, stream))
    return NIL;
  checksum = strtol(tmp, NIL, 10);
  if (!replay_getline(rfile, tmp, stream))
    return NIL;
  mbox_size = strtol(tmp, NIL, 10);
  /* validate the checksum:
   * This must be done by the server, since the current
   * checksum may not be the same as the saved checksum,
   * BUT, the saved checksum may be valid, eg: The file
   * increased in size with new mail but the checksummed data
   * is still intact. */
  if (!(*stream->dtb->validchecksum)(stream, checksum, 
				     mbox_size))
    return NIL;
  else {
    mm_log("Checksum validated", NIL,stream);
    return T;
  }
}
int replay_uidvalidity_ok(MAILSTREAM *stream, FILE *rfile, char *tmp)
{
  unsigned long uidv;
  if (!replay_getline(rfile, tmp, stream))
    return NIL;
  uidv = strtol(tmp, NIL, 10);
  /* compare to the current value:
   * A caveat: If we are connected to an imap2 server, and
   * stream->uid_validity is set from a restored cache, then
   * the following test is TRUE because the STATUS command
   * at startup failed to reset it (STATUS is NOT imap2bis).
   * We take care of this in replay_imap4... */
  if (stream->uid_validity != uidv)
    return NIL;
  else
    return T;
}
/*
 * Read each recorded block from the file, and
 * play it back on the stream */
int replay_imap2recording(MAILSTREAM *stream, FILE *rfile,
			  int chksum_ok,
			  int rfile_ok,
			  char *buf,int blen)
{
  RECORDBLK *enregistre;
  RECORD_RESULT what_happened;
  int fini= NIL;
  int nothing_to_replay = replay_empty(rfile, buf, stream) || !rfile_ok;

  if (!chksum_ok) {
    /* If nothing to do, then only need to flush the
     * the cache. */
    if (nothing_to_replay)
      return FLUSHCACHEONLY;
    else
      /* User playback failure */
      return FAILEDPLAYBACK;
  } else {
    /* IF valid checksum but nothing to playback.
     * Leave cache intact */
    /* Peek to see if there is anything to playback */
    if (nothing_to_replay) {
      return NOTHINGTOPLAYBACK;      
    }
  }
  /* OK, playback the imap2 world */
  while (!fini) {
    what_happened = next_recorded_command(stream, rfile, &enregistre, 
					  buf, blen, NIL);
    switch (what_happened) {
    case bad_file_format:
      mm_log ("Replay: Bad file format encounterd", ERROR,stream);
      free_recordblk(&enregistre);
      return PLAYBACKLOCALERROR;
    case got_next_block:
      playback_block(stream, enregistre, buf);
      break;
    case eof_on_file:
      enregistre = NIL;
      fini = T;
      break;
    }
    if (enregistre)
      free_recordblk(&enregistre);
  }
  return SUCCESSFULLPLAYBACK;
}
/*
 * We can always playback. But, if the checksum is not valid,
 * then we need to resynchronize the cache.
 * NOTE: BEFORE SYNCING the cache we run through all of our
 *       recorded blocks and change the msg sequences to UID 
 *       sequences. 
 */
int replay_imap4recording(MAILSTREAM *stream, FILE *rfile, int cksum_ok,
			  int uid_valid, int rfile_ok, 
			  char *buf, int blen)
{
  RECORDBLK *enregistre;
  RECORDBLKLIST rblist;
  RECORD_RESULT what_happened;
  int fini= NIL;
  int something_to_replay = !replay_empty(rfile,buf,stream) && rfile_ok;
  int retval;
  /* uid-validity   chksum
   *    OK            OK      playback
   *    OK           bad      playback (we use UID's only)
   *   BAD            OK      no playback (*)
   *   BAD           BAD      NO playback
   *   (*) if uid-validity changed, then the uid's on the server are
   *       ALL recreated. Case 3 is impossible.
   */
  if (!uid_valid){
    if (something_to_replay)
      return FAILEDPLAYBACK;
    else
      return FLUSHCACHEONLY;
  }
  /* (1) We convert the msg sequences to UID sequences
   *     in our recorded commands using the existing cache.
   * (2) Then we synchronize the cache.
   * (3) Finally, we playback the recorded commands with UID's 
   *     which may update our syncronized cache */
  if (something_to_replay) {
    RECORDBLK *cblk;
    long nmsgs = stream->nmsgs;
    long replay_flags = PB_UID;
    /*
     * Use the number of cached msgs here. Could be
     * that stream->nmsgs is smaller, and we need our cache size
     * to convert msg sequences to uid sequences */
    stream->nmsgs = stream->cached_nmsgs;
    rblist.n_elements = 0;
    rblist.rblk = NIL;
    while (!fini) {
      what_happened = next_recorded_command(stream, rfile, &enregistre, 
					    buf, blen,
					    replay_flags + PB_IMAP4);
      switch (what_happened) {
      case bad_file_format:
	mm_log ("Replay: Local record file broken", ERROR,stream);
	/* free the list */
	free_rlist(&rblist);
	/* and the current block */
	free_recordblk(&enregistre);
	return PLAYBACKLOCALERROR;
      case got_next_block:
	rblist.n_elements += 1;
	if (rblist.n_elements == 1) {
	  cblk = rblist.rblk = enregistre;
	} else {
	  cblk->next = enregistre;
	  cblk = enregistre;
	}
	enregistre->next = NIL;
	break;
      case eof_on_file:
	fini = T;
	break;
      }
    }
    /*
     * reset the stream message count */
    stream->nmsgs = nmsgs;
  }
  /*
   * If the checksum is NOT OK, then we must synchronize, ie,
   * our cache does not correspond to the server's cache */
  if (!cksum_ok) {
    mm_log("Replay: Synchronizing the cache ...", WARN,stream);
    mm_log_stream(stream, "140 IMAP4: Synchronizing the cache");
    if (playback_synchronize(stream))
      retval =  SUCCESSFULLPLAYBACK;  
    else
      retval =  FLUSHCACHEONLY;
  } else
    retval = SUCCESSFULLPLAYBACK;
  /* We can ALWAYS replay even if some sync screwup happend */
  if (something_to_replay && rblist.n_elements > 0) {
    enregistre = rblist.rblk;
    /* If some sync problem happend, then silence the server
     * on replies. We will flush the cache before quiting */
    if (retval == FLUSHCACHEONLY) stream->silent = T;
    while (enregistre) {
      playback_block(stream, enregistre, buf);
      enregistre = enregistre->next;
    }
    free_rlist(&rblist);
    stream->silent = NIL;
  }
  return retval;
}
void free_rlist(RECORDBLKLIST *rlist)
{
  if (rlist->n_elements == 0) return;
  else {
    RECORDBLK *rblk, *tmp;
    rblk = rlist->rblk;
    while (rblk) {
      tmp = rblk->next;
      free_recordblk(&rblk);     
      rblk = tmp;
    }
  }
}

/*
 * Synchronize the cache with the server cache state:
 */
#define SYNCLOOKAHEAD 1000
int playback_synchronize(MAILSTREAM *stream)
{
  char tmp[MAILTMPLEN];
  /*
   * Set lookahead to stream->nmsgs. Note that our cache may
   * contain data for more messages.  */
  int interval = stream->nmsgs;
  /* Get our synchronization information */
  if (stream->dtb->sync_msgno) {
    long i;
    int gap;
    int uidcount = 0;
    int nfreed;
    char *uidseq;
    /* If NO cached messages, then nothing to do */
    if (stream->cached_nmsgs == 0) return T;
    /* Nothing synced yet */
    for (i = 1; i <= stream->cached_nmsgs; ++i) 
      mail_elt(stream,i)->sync_msgno = 0;
    /* Get the synced message numbers:
     * Caller should provide this sequence for things to work right.
     * Must be built on disconnected stream, and saved.
     * mail_disconnect(..) creates this sequence correctly. */
    uidseq = (stream->sync_uidseq ? stream->sync_uidseq : NIL);
    (*stream->dtb->sync_msgno)(stream, uidseq);
    if (uidseq)
      fs_give((void **)&stream->sync_uidseq);
    /*
     * OK. Find our first cache entry
     * (1) Possible that NONE of our uids is valid - check this 
     * (2) We look for cases where sync_msgno < msgno - gap:
     *     In this case messages have been expunged, and we
     *     free (msgno - sync_msgno + gap) elements, and left
     *     shift our message cache appropriately.
     * (3) When done we renumber our messages, and for a
     *     sanity check sync_msgno == msgno for cached elements.
     * (4) Then  we refetch the flags for
     *     all cached elements.
     */
    for (i = 1, gap = 0, nfreed = 0; i <= stream->cached_nmsgs; ++i) {
      MESSAGECACHE *elt = mail_elt(stream,i);
      if (elt->sync_msgno) {
	++uidcount;
	if (elt->sync_msgno < elt->msgno) {
	  unsigned long diff = elt->msgno - elt->sync_msgno; 
	  if (diff > gap) {
	    long liberer = diff - gap;
	    sprintf(tmp, "Msgno %ld != Synced Msgno %ld: Free %d elements", 
		    elt->msgno,
		    elt->sync_msgno,
		    liberer);
	    mm_log(tmp, WARN,stream);
	    replay_free_cache(stream, i, liberer);
	    /* The gap accumulates */
	    gap = diff;
	    nfreed += liberer;
	    /* Fewer cached elements exist after freeing */
	    stream->cached_nmsgs -= liberer;
	    /* We shifted left liberer elements, so we need to
             * realign i to the current REAL msg number.
             * It is then incremented  */
	    i = elt->sync_msgno;
	  }
	}
      }
    }
    /* We may have cache elements at the end of the cache which are
     * NOT in the mailbox on the server. Thus, elt->sync_msgno == 0.
     * We need to free them. If this is the case, then 
     *    we trace the cache backwards until we find a valid cache element.
     *    we drop all cache elements with sync_msgno == 0 since we did not
     *    then find a UID for it */
    for (i = stream->cached_nmsgs; i >= 1; --i) {
      MESSAGECACHE *elt = (MESSAGECACHE *)(*mailcache)(stream,i,CH_ELT);
      if (elt)
	if (elt->sync_msgno == 0) {
	  (*mailcache)(stream,i,CH_FREE_ALWAYS);
	  stream->cached_nmsgs -= 1;
	} else
	  break;
    }
    /* See if we freed any elements[Does NOT include those just above which 
     * are no longer valid messages] */
    if (nfreed > 0) {
      /* Renumber the messages */
      for (i = 1; i <= stream->cached_nmsgs; ++i) {
	MESSAGECACHE *elt = mail_elt(stream,i);
	elt->msgno = i;
	if (elt->sync_msgno && i != elt->sync_msgno) {
	  sprintf(tmp, "Cache syncronization error: msgno = %ld, should = %ld",
		  i, elt->sync_msgno);
	  mm_log(tmp, ERROR,stream);
	}
	elt->sync_msgno = 0;
      }
    }
    /* Now fetch the flags for all cached messages */
    if (stream->nmsgs > 0) {
      char *seq;
      sprintf(tmp, "1:%ld", stream->nmsgs);
      seq = mail_msgseq_with_uid(stream, tmp);
      if (*seq != '\0')
	mail_fetchflags_full(stream, seq, NIL);
      fs_give((void **)&seq);
    }
    /* Make sure ONE of our UIDS was OK */
    if (uidcount > 0)
      return T;
  }
  return NIL;
}
/* Starting with (msgno - num_to_free), free num_to_free 
 * cache elements. Here we override any locks:
 * Remember the cache starts at 0 so use indices rather than msgnos!  */
void replay_free_cache(MAILSTREAM *stream, unsigned long msgno, 
		      long num_to_free)
{
  long i = msgno - num_to_free;
  unsigned long left_index = i - 1, start_index, 
     endmsg = i + num_to_free - 1;
  int j;
  /* (1) Free the elements[use message numbers here] */
  for (;i <= endmsg;++i) {
    MESSAGECACHE *elt = (MESSAGECACHE *)(*mailcache)(stream,i,CH_ELT);
    if (elt)
      (*mailcache)(stream,i,CH_FREE_ALWAYS);
  }
  /* (2) Shift cache left filling the gap which may be more than 1
   *     message[use indices here]*/
  for (start_index = msgno - 1; start_index < stream->cached_nmsgs; 
       ++start_index,++left_index)
    stream->cache.c[left_index] = stream->cache.c[start_index];
  /* NIL out "num_to_free" non-valid entries at end. */
  for (i = left_index, j = 0; 
       j < num_to_free && i < stream->cachesize; ++i,++j) 
    stream->cache.c[i] = NIL;
}

/* read next record from the record file */
RECORD_RESULT next_recorded_command(MAILSTREAM *stream, FILE *rfile, 
				    RECORDBLK **rblk, 
				    char *buf,int blen,
				    long flags)
{
  char *cp;
  char *lasts;
  int nparams;
  PLIST *plptr,*prev_plptr;

				/* read the function name */
  if (!fgets(buf, blen-1, rfile))
    return eof_on_file;
				/* allocate the record block */
  *rblk = fs_get(sizeof(RECORDBLK));
				/* NULL will be placed on '\n' */
  (*rblk)->function = fs_get(strlen(buf));
  (*rblk)->params = NIL;
  cp = (char *)strrchr(buf, '\n');
  if (!cp)
    return  bad_file_format;
  *cp = '\0';
  strcpy((*rblk)->function, buf);
				/* number of parameters */
  if (!fgets(buf, blen-1, rfile))
    return  bad_file_format;
  nparams = atoi(buf);
  if (nparams <= 0)
    return bad_file_format;
				/* for our link list */
  prev_plptr = NIL;
				/* read the parameters */
  while (nparams--) {
    char *ptype, *param;
				/* Anymore parameters? */
    plptr = fs_get(sizeof(PLIST));
    plptr->parameter = NIL;
    plptr->next = NIL;    
    if (!prev_plptr)		/* init head of list */
      prev_plptr = (*rblk)->params = plptr;
    else {			/* maintain linked list */
      prev_plptr->next = plptr;
      prev_plptr = plptr;
    }

    if (!fgets(buf, blen-1, rfile))
      return  bad_file_format;
				/* get parameter type */
    ptype = strtok_r(buf, ":", &lasts);
    plptr->parameter_type = convert_ptype(ptype);
    if (plptr->parameter_type < 0)
      return  bad_file_format;
				/* get parameter as a string */
    param = strtok_r(NIL, "\n", &lasts);
    if (!param)
      return bad_file_format;
    plptr->parameter = fs_get(strlen(param) + 1);
    strcpy(plptr->parameter, param);
    /* Make a UID sequence of message sequences */
    if (plptr->parameter_type == SEQUENCE_PARAM &&
	(flags & PB_IMAP4)) {
      char *uidseq = mail_seq_to_uidseq(stream, plptr->parameter, NIL);
      fs_give((void **)&plptr->parameter);
      plptr->parameter = uidseq;
    }
				/* the flags */
    plptr->flags = flags;
  }
  /* read the end-of-record mark */
  if (!fgets(buf, blen-1, rfile))
    return bad_file_format;
  else
    return got_next_block;
}

void playback_block(MAILSTREAM *stream, RECORDBLK  *rblk, char *tmp)
{
  int function_type;

  function_type = fstr_to_type(rblk->function);
  switch (function_type) {
  case SETFLAG:
    playback_setflag(stream, rblk->params);
    break;
  case CLEARFLAG:
    playback_clearflag(stream, rblk->params);
    break;
  case COPY:
    playback_copy(stream, rblk->params);
    break;
  case CREATE:
    playback_create(stream, rblk->params);
    break;
  case RENAME:
    playback_rename(stream, rblk->params);
  case DELETE:
    playback_delete(stream, rblk->params);
    break;
  case SET1FLAG:
    playback_set1flag(stream, rblk->params);
    break;
  case CLEAR1FLAG:
    playback_clear1flag(stream, rblk->params);
    break;
  default:
    sprintf(tmp, "playback_block: illegal function: %s", rblk->function);
    mm_log(tmp, NIL,stream);
    break;
  }
}
/* Playback setflag:
 * Since all parameters are by default strings,
 * we don't have to convert them */
void playback_setflag(MAILSTREAM *stream, PLIST *params)
{
  char *seq, *flag;
  char tmp[MAILTMPLEN];
  long options;

  seq = params->parameter;
  /* Set options */
  if (params->flags & PB_UID) {
    options = ST_UID;
  } else
    options = NIL;
  flag = params->next->parameter;
  sprintf(tmp, "Replay setflag 20%s %s", seq, flag);
  mm_log(tmp, NIL,stream);
  (*stream->dtb->setflag) (stream, seq, flag, options);
}

void playback_clearflag(MAILSTREAM *stream, PLIST *params)
{
  char *seq, *flag;
  char tmp[MAILTMPLEN];
  long options;

  seq = params->parameter;
  if (params->flags & PB_UID) {
    options = ST_UID;
  } else
    options = NIL;
  flag = params->next->parameter;
  sprintf(tmp, "Replay clearflag 20%s %s", seq, flag);
  mm_log(tmp, NIL,stream);
  (*stream->dtb->clearflag) (stream, seq, flag, options);
}
void playback_set1flag(MAILSTREAM *stream, PLIST *params)
{
  char *seq, *flag;
  char tmp[MAILTMPLEN];
  long options;
  seq = params->parameter;
  if (params->flags & PB_UID) {
    options = ST_UID;
  } else
    options = NIL;
  flag = params->next->parameter;
  sprintf(tmp, "Replay setflag 20%s %s", seq, flag);
  mm_log(tmp, NIL,stream);
  (*stream->dtb->set1flag) (stream, seq, flag, options);
}

void playback_clear1flag(MAILSTREAM *stream, PLIST *params)
{
  char *seq, *flag;
  char tmp[MAILTMPLEN];
  long options;
  seq = params->parameter;
  if (params->flags & PB_UID) {
    options = ST_UID;
  } else
    options = NIL;
  flag = params->next->parameter;
  sprintf(tmp, "Replay clearflag 20%s %s", seq, flag);
  mm_log(tmp, NIL,stream);
  (*stream->dtb->clear1flag) (stream, seq, flag, options);
}

void playback_copy(MAILSTREAM *stream, PLIST *params)
{
  char *seq, *mailbox;
  char tmp[MAILTMPLEN];
  long options;
  seq = params->parameter;
  if (params->flags & PB_UID) {
    options = ST_UID;
  } else
    options = NIL;
  mailbox = params->next->parameter;
  sprintf(tmp, "Replay copy 20%s %s", seq, mailbox);
  mm_log(tmp, NIL,stream);
 (*stream->dtb->copy) (stream, seq, mailbox, options);
}

void playback_create(MAILSTREAM *stream, PLIST *params)
{
  char *mailbox;
  char tmp[MAILTMPLEN];

  mailbox = params->parameter;
  sprintf(tmp, "Replay create %s ", mailbox);
  mm_log(tmp, NIL,stream);
 (*stream->dtb->create) (stream, mailbox);
}

void playback_delete(MAILSTREAM *stream, PLIST *params)
{
  char *mailbox;
  char tmp[MAILTMPLEN];

  mailbox = params->parameter;
  sprintf(tmp, "Replay delete %s ", mailbox);
  mm_log(tmp, NIL,stream);
  (*stream->dtb->mbxdel) (stream, mailbox);
}

void playback_rename(MAILSTREAM *stream, PLIST *params)
{
  char *old, *new;
  char tmp[2*MAILTMPLEN];

  old = params->parameter;
  new = params->next->parameter;
  sprintf(tmp, "Replay rename %s to %s ", old, new);
  mm_log(tmp, NIL,stream);
  (*stream->dtb->rename) (stream, old, new);
}

/* UTILS */
typedef struct _convert_ {
  char *string;
  int type;
} CONVERT;

static CONVERT convert_pstring[] = {
  STRINGTYPE, STRING_PARAM,
  SEQUENCETYPE, SEQUENCE_PARAM,
  NIL, NIL,
};

int convert_ptype(char *ptype)
{
  CONVERT *c= convert_pstring;
  while (c->string) {
    if (strcmp(c->string, ptype) == 0)
      return c->type;
    else
      ++c;
  }
  return -1;
}

static CONVERT convert_fn[]= {
  RECORD_SETFLAG, SETFLAG,
  RECORD_SET1FLAG, SET1FLAG,
  RECORD_CLEARFLAG, CLEARFLAG,
  RECORD_CLEAR1FLAG, CLEAR1FLAG,
  RECORD_COPY, COPY,
  RECORD_CREATE, CREATE,
  RECORD_DELETE, DELETE,
  RECORD_RENAME, RENAME,
  NIL, NIL,
};

int fstr_to_type(char *function)
{
  CONVERT *c= convert_fn;
  while (c->string) {
    if (strcmp(c->string, function) == 0)
      return c->type;
    else
      ++c;
  }
  return -1;
}


void free_recordblk(RECORDBLK **rblk)
{
  PLIST *p;

  if (*rblk)
    return;			/* Nothing to free */
  p = (*rblk)->params;
  while (p) {
    PLIST *tmp;

    if (p->parameter)
      fs_give((void **)&p->parameter);
    tmp = p->next;
    fs_give((void **)&p);
    p = tmp;
  }
  fs_give((void **)&(*rblk)->function);
  fs_give((void **)rblk);
  *rblk = NIL;
}


