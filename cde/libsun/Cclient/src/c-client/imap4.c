/*
 * Program:	Interactive Message Access Protocol 4 (IMAP4) routines
 *
 * Author:	Mark Crispin
 *		Networks and Distributed Computing
 *		Computing & Communications
 *		University of Washington
 *		Administration Building, AG-44
 *		Seattle, WA  98195
 *		Internet: MRC@CAC.Washington.EDU
 *
 * Date:	15 June 1988
 *
 * Sponsorship:	The original version of this work was developed in the
 *		Symbolic Systems Resources Group of the Knowledge Systems
 *		Laboratory at Stanford University in 1987-88, and was funded
 *		by the Biomedical Research Technology Program of the National
 *		Institutes of Health under grant number RR-00785.
 *
 * Original version Copyright 1988 by The Leland Stanford Junior University
 * Copyright 1995 by the University of Washington
 *
 *  Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notices appear in all copies and that both the
 * above copyright notices and this permission notice appear in supporting
 * documentation, and that the name of the University of Washington or The
 * Leland Stanford Junior University not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written prior
 * permission.  This software is made available "as is", and
 * THE UNIVERSITY OF WASHINGTON AND THE LELAND STANFORD JUNIOR UNIVERSITY
 * DISCLAIM ALL WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD TO THIS SOFTWARE,
 * INCLUDING WITHOUT LIMITATION ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE UNIVERSITY OF
 * WASHINGTON OR THE LELAND STANFORD JUNIOR UNIVERSITY BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, TORT (INCLUDING NEGLIGENCE) OR STRICT LIABILITY, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Copyright (c) 1995 by SunSoft, Inc. All rights reserver.
 *   Last edited 01-10-97
 * Edit history:
 *   8-may        moved GET_USERDATA call in imap_open()
 *  23-august     added fetch envelope to eliminate body parse.
 *   3-sept-96    fix for NIL body in fetchshort simulated.
 *  26-oct-96     IMAP4SUNVERSION is sent iff the server is IMAP4
 *                and it has the IMAP4SUNVERSION capability.
 * [09-23-96 clin] strtok_r used to ensure MT-safe.
 * [10-21-96 clin] Pass a mailstream in the following routines:
  *			 imap_parameters. 
 *			 imap_valid,
 *		         imap_global_init
 *			 mail_parameters
 *			 mail_open
 *  [10-24-96 clin ]  all globals are on stream now.
 *  [10-25-96 clin ]  New routine: imap_global_init. 
 *  [10-31-96 clin ]  It's OK to pass a NIL to mail_open now. 
 *  [11-08-96 clin ]  Pass a stream in mm_log and tcp_open. 
 *  [11-19-96 clin ]  Pass a stream in fatal. 
 *  [12-04-96 clin ]  Pass a stream in all mail_open calls since
 *			we need to access the imapd_globals.
 *  [12-20-96 clin ]  Remove imap_global macros.
 *  [12-31-96 clin ]  Move imap_global data structure to imap4.h. 
 *  [01-01-97 clin ]  Add a new routine imap_global_free.
 *  [01-03-97 clin ]  Call mail_stream_setNIL.
 *  [01-07-97 clin ]  Remove the last five parameters from mail_open. 
 *  [01-10-97 clin ]  Fixed the core dump in imap_status. 
 *		      Test for stream_status in imap_open.
 *		      Re-structured imap_status.
 *
 *  [01-21-97 clin ]  Test for S_OPENED in imap_append and 
 *		      imapd_disconnet.
 *  [01-22-97 clin ]  Re-structured imap_list_work and imap_manage and
 *		      imap_append.
 *  [01-23-97 clin ]  Fix John's bug in imap_status.
 *  [01-30-97 clin ]  Fix Esther's bug in imap_send_spgm. 
 * 
 */


#include <ctype.h>
#include <stdio.h>
#include "mail.h"
#include "os_sv5.h"
#include "imap4.h"
#include "rfc822.h"
#include "dummy.h"
#include "misc.h"

/* Driver dispatch used by MAIL */

DRIVER imapdriver = {
  "imap4",			/* driver name */
  DR_MAIL|DR_NEWS|DR_NAMESPACE|DR_NOSTREAM,/* driver flags */
  (DRIVER *) NIL,		/* next driver */
  imap_valid,			/* mailbox is valid for us */
  imap_parameters,		/* manipulate parameters */
  imap_scan,			/* scan mailboxes */
  imap_list,			/* find mailboxes */
  imap_lsub,			/* find subscribed mailboxes */
  imap_subscribe,		/* subscribe to mailbox */
  imap_unsubscribe,		/* unsubscribe from mailbox */
  imap_create,			/* create mailbox */
  imap_delete,			/* delete mailbox */
  imap_rename,			/* rename mailbox */
  imap_status,			/* status of mailbox */
  imap_open,			/* open mailbox */
  imap_close,			/* close mailbox */
  imap_fetchfast,		/* fetch message "fast" attributes */
  imap_fetchflags,		/* fetch message flags */
  imap_fetchstructure,		/* fetch message envelopes */
  imap_fetchheader,		/* fetch message header only */
  imap_fetchtext,		/* fetch message body only */
  imap_fetchbody,		/* fetch message body section */
  imap_uid,			/* unique identifier */
  imap_setflag,			/* set message flag */
  imap_clearflag,		/* clear message flag */
  imap_search,			/* search for message based on criteria */
  imap_ping,			/* ping mailbox to see if still alive */
  imap_check,			/* check for new messages */
  imap_expunge,			/* expunge deleted messages */
  imap_copy,			/* copy messages to another mailbox */
  imap_append,			/* append string message to mailbox */
  imap_gc,			/* garbage collect stream */
  /* SUN Imap4 */
  dummy_fabricate_from,		/* not used */
  imap_set1flag,		/* set 1 flag extension */
  imap_clear1flag,		/* clear 1 flag extension */
  imap_sunversion,		/* send sunversion command  */
  imap_disconnect,		/* disconnect a stream */
  imap_serverecho,		/* imap "ping" */
  imap_cleario,			/* clear imap io-counters */
  imap_urgent,			/* enable TCP urgent byte to be sent */
  imap_fetchshort,		/* fetch short information */
  imap_checksum,		/* request checksum status */
  imap_validchecksum,		/* validate status checksum attributes */
  imap_sync_msgno,		/* synchronizes msgnos to uids */
  imap_imap4,			/* TRUE if imap4 server out there. */
  NIL,				/* NO driver free body */
  imap_fetchenvelope,		/* envelope only. */
  imap_global_init,             /* init imap globals */
  imap_global_free              /* init imap globals */
};

void set_use_maxwait(TCPSTREAM *stream, int value);

				/* prototype stream */
MAILSTREAM imapproto = {&imapdriver};

#define SEQBUFLEN 16
#define MAXSEQUENCE (size_t)1000

/* IMAP validate mailbox
 * Accepts: mailbox name
 * Returns: our driver if name is valid, NIL otherwise
 */

DRIVER *imap_valid (char *name, MAILSTREAM *stream)
{
  return mail_valid_net (name,&imapdriver,NIL,NIL);
}


/* IMAP manipulate driver parameters
 * Accepts: function code
 *	    function-dependent value
 * Returns: function-dependent return value
 */

void *imap_parameters (MAILSTREAM *stream, long function,void *value)
{
  IMAP_GLOBALS *ig = (IMAP_GLOBALS *)stream->imap_globals;
  switch ((int) function) {
  case SET_MAXLOGINTRIALS:
    ig->imap_maxlogintrials = (long) value;
    break;
  case GET_MAXLOGINTRIALS:
    value = (void *) ig->imap_maxlogintrials;
    break;
  case SET_LOOKAHEAD:
    if ((int)value > 0)
      ig->imap_lookahead    = 
	((int)value < IMAPMAXLOOKAHEAD ? (int)value : IMAPMAXLOOKAHEAD);
    break;
  case GET_LOOKAHEAD:
    value = (void *) ig->imap_lookahead     ;
    break;
  case SET_UIDLOOKAHEAD:
    ig->imap_uidlookahead = (long) value;
    break;
  case GET_UIDLOOKAHEAD:
    value = (void *) ig->imap_uidlookahead;
    break;
  case SET_IMAPPORT:
    ig->imap_defaultport = (long) value;
    break;
  case GET_IMAPPORT:
    value = (void *) ig->imap_defaultport;
    break;
  case SET_PREFETCH:
    ig->imap_prefetch = (long) value;
    break;
  case GET_PREFETCH:
    value = (void *) ig->imap_prefetch;
    break;
  case SET_CLOSEONERROR:
    ig->imap_closeonerror = (long) value;
    break;
  case GET_CLOSEONERROR:
    value = (void *) ig->imap_closeonerror;
    break;
  default:
    value = NIL;		/* error case */
    break;
  }
  return value;
}

/* IMAP scan mailboxes
 * Accepts: mail stream
 *	    reference
 *	    pattern to search
 *	    string to scan
 */

void imap_scan (MAILSTREAM *stream,char *ref,char *pat,char *contents)
{
  imap_list_work (stream,ref,pat,T,contents);
}

/* IMAP find list of mailboxes
 * Accepts: mail stream
 *	    reference
 *	    pattern to search
 */

void imap_list (MAILSTREAM *stream,char *ref,char *pat)
{
  imap_list_work (stream,ref,pat,T,NIL);
}


/* IMAP find list of subscribed mailboxes
 * Accepts: mail stream
 *	    reference
 *	    pattern to search
 */

void imap_lsub (MAILSTREAM *stream,char *ref,char *pat)
{
  imap_list_work (stream,ref,pat,NIL,NIL);
}

/* IMAP find list of mailboxes
 * Accepts: mail stream
 *	    reference
 *	    pattern to search
 *	    list flag
 *	    string to scan
 */

void imap_list_work (MAILSTREAM *stream,char *ref,char *pat,long list,
		     char *contents)
{
  MAILSTREAM *st = stream;
  int pl;
  char *s,prefix[MAILTMPLEN],mbx[MAILTMPLEN];
  IMAPARG *args[4],aref,apat,acont;

  if (ref && *ref) {		/* have a reference? */
  /* imap_valid doesn't need a stream so we pass a NIL here.
  But mail_open needs one. So we create a new stream. */
	
	/* make sure valid IMAP name and open stream */
  if (!(imap_valid (ref, NIL) &&
    ((stream && (stream->stream_status & S_OPENED) && 
    ((IMAPLOCAL *) stream->local) && ((IMAPLOCAL *) stream->local)->tcpstream)))) {
    stream = mail_stream_create(stream->userdata, 
		 NIL, NIL, NIL, NIL);
    stream = mail_open (stream,ref,OP_HALFOPEN|OP_SILENT);
    }
	/* Then we test the new stream. */
  if (!(stream->stream_status & S_OPENED)) {
    mail_stream_flush(stream);  
    return;
    }
				/* calculate prefix length */
    pl = strchr (ref,'}') + 1 - ref;
    strncpy (prefix,ref,pl);	/* build prefix */
    prefix[pl] = '\0';		/* tie off prefix */
    ref += pl;			/* update reference */
  }
  else { 
	/* make sure valid IMAP name and open stream */
  if (!(imap_valid (pat,NIL) &&
	((stream && (stream->stream_status & S_OPENED) && 
	((IMAPLOCAL *) stream->local) && ((IMAPLOCAL *) stream->local)->tcpstream)))) {
	stream = mail_stream_create(stream->userdata, 
		 NIL, NIL, NIL, NIL);
        stream = mail_open (stream,pat,OP_HALFOPEN|OP_SILENT);
	}
	
  if (!(stream->stream_status & S_OPENED)) {
	mail_stream_flush(stream);
	return;
	}
				/* calculate prefix length */
    pl = strchr (pat,'}') + 1 - pat;
    strncpy (prefix,pat,pl);	/* build prefix */
    prefix[pl] = '\0';		/* tie off prefix */
    pat += pl;			/* update reference */
  }
  ((IMAPLOCAL *) stream->local)->prefix = prefix;	/* note prefix */
  if (contents) {		/* want to do a scan? */
    if (((IMAPLOCAL *) stream->local)->use_scan) {	/* can we? */
      args[0] = &aref; args[1] = &apat; args[2] = &acont; args[3] = NIL;
      aref.type = ASTRING; aref.text = (void *) (ref ? ref : "");
      apat.type = LISTMAILBOX; apat.text = (void *) pat;
      acont.type = ASTRING; acont.text = (void *) contents;
      imap_send (stream,"SCAN",args);
    }
    else {
      mm_log ("Scan not valid on this IMAP server",WARN, stream);
      mm_log_stream(stream,"146 Directory scan not supported on server");
    }
  }
  else if (((IMAPLOCAL *) stream->local)->imap4) {	/* easy if just IMAP4 */
    args[0] = &aref; args[1] = &apat; args[2] = NIL;
    aref.type = ASTRING; aref.text = (void *) (ref ? ref : "");
    apat.type = LISTMAILBOX; apat.text = (void *) pat;
    imap_send (stream,list ? "LIST" : "LSUB",args);
  }
  else if (((IMAPLOCAL *) stream->local)->rfc1176) {	/* convert to IMAP2 format wildcard */
				/* kludgy application of reference */
    if (ref && *ref) sprintf (mbx,"%s%s",ref,pat);
    else strcpy (mbx,pat);
    for (s = mbx; *s; s++) if (*s == '%') *s = '*';
    args[0] = &apat; args[1] = NIL;
    apat.type = LISTMAILBOX; apat.text = (void *) mbx;

    /* try IMAP2bis, then RFC-1176 */
    if (!(list && strcasecmp(imap_send(stream,
				       "FIND ALL.MAILBOXES", args)->key,
			     "BAD") != 0)
	|| (strcasecmp(imap_send(stream, "FIND MAILBOXES", args)->key,
		       "BAD") == 0)) {
      ((IMAPLOCAL *) stream->local)->rfc1176 = NIL;	/* must be RFC-1064 */
    }
  }
  ((IMAPLOCAL *) stream->local)->prefix = NIL;		/* no more prefix */
				/* close temporary stream if we made one */
		/* If we get down here, we should close the stream. */
  if (stream != st) mail_close (stream);
}

/* IMAP subscribe to mailbox
 * Accepts: mail stream
 *	    mailbox to add to subscription list
 * Returns: T on success, NIL on failure
 */

long imap_subscribe (MAILSTREAM *stream,char *mailbox)
{
  return imap_manage (stream,mailbox,"Subscribe Mailbox",NIL);
}


/* IMAP unsubscribe to mailbox
 * Accepts: mail stream
 *	    mailbox to delete from manage list
 * Returns: T on success, NIL on failure
 */

long imap_unsubscribe (MAILSTREAM *stream,char *mailbox)
{
  return imap_manage (stream,mailbox,"Unsubscribe Mailbox",NIL);
}

/* IMAP create mailbox
 * Accepts: mail stream
 *	    mailbox name to create
 * Returns: T on success, NIL on failure
 */

long imap_create (MAILSTREAM *stream,char *mailbox)
{
  return imap_manage (stream,mailbox,"Create",NIL);
}


/* IMAP delete mailbox
 * Accepts: mail stream
 *	    mailbox name to delete
 * Returns: T on success, NIL on failure
 */

long imap_delete (MAILSTREAM *stream,char *mailbox)
{
  return imap_manage (stream,mailbox,"Delete",NIL);
}


/* IMAP rename mailbox
 * Accepts: mail stream
 *	    old mailbox name
 *	    new mailbox name
 * Returns: T on success, NIL on failure
 */

long imap_rename (MAILSTREAM *stream,char *old,char *newname)
{
  return imap_manage (stream,old,"Rename",newname);
}

/* IMAP manage a mailbox
 * Accepts: mail stream
 *	    mailbox to manipulate
 *	    command to execute
 *	    optional second argument
 * Returns: T on success, NIL on failure
 */

long imap_manage (MAILSTREAM *stream,char *mailbox,char *command,char *arg2)
{
  MAILSTREAM *st = stream;
  IMAPPARSEDREPLY *reply;
  long ret = NIL;
  char mbx[MAILTMPLEN],mbx2[MAILTMPLEN];
  IMAPARG *args[3],ambx,amb2;
  ambx.type = amb2.type = ASTRING; ambx.text = (void *) mbx;
  amb2.text = (void *) mbx2;
  args[0] = &ambx; args[1] = args[2] = NIL;
			/* require valid names and open stream */
  if (mail_valid_net (mailbox,&imapdriver,NIL,mbx) &&
      (arg2 ? mail_valid_net (arg2,&imapdriver,NIL,mbx2) : &imapdriver) &&
      ((stream && (stream->stream_status & S_OPENED) && ((IMAPLOCAL *) stream->local) && 
	((IMAPLOCAL *) stream->local)->tcpstream) ||
      ((stream = mail_stream_create(stream->userdata, NIL,NIL,NIL,NIL)) &&
	(stream = mail_open (stream,mailbox,OP_HALFOPEN)) && 
	(stream->stream_status & S_OPENED)))) {
    if (arg2) args[1] = &amb2;	/* second arg present? */
    ret = (imap_OK (stream,reply = imap_send (stream,command,args)));
    mm_log (reply->text,ret ? (long) NIL : ERROR, stream);
    /* Sun Imap4:  Special log function that passes the stream */
    mm_log_stream(stream, reply->text);
  }
				/* toss out temporary stream */
    if (st != stream) mail_close (stream);
  return ret;
}


/*
 * IMAP status
 * Accepts: mail stream
 *	    mailbox name
 *	    status flags
 *          userdata for recursive calls only
 * Returns: T on success, NIL on failure
 */
long
imap_status(MAILSTREAM * stream, char * mbx, long flags, void * userdata)
{
  IMAPARG 	*	args[4];
  IMAPARG		ambx;
  IMAPARG		aflg;
  char 			tmp[MAILTMPLEN];
  NETMBX 		mb;
  unsigned long 	i;
  long 			ret = _B_FALSE;
  MAILSTREAM 	*	tstream = stream; 

  mail_valid_net_parse(mbx, &mb);

  /*
   * can't use stream if not IMAP4 AND not halfopen 
   *   OR the hosts are different:
   */
  if (stream 
      && (stream->stream_status & S_OPENED)
      && !(stream->stream_status & S_NILCALL )
      && (!(((IMAPLOCAL *) stream->local)->use_status || stream->halfopen)
	  || strcmp(ucase(strcpy(tmp, imap_host(stream))), ucase(mb.host)))) { 

    /* We have to pass a "NIL" stream here. */
    mail_stream_setNIL(stream);	
    ret =  imap_status(stream, mbx, flags, stream->userdata);
    mail_stream_unsetNIL(stream);  /*unset it before return. */
    return(ret);
  }

  /* make stream if don't have one */
  if (!(stream->stream_status & S_OPENED )) {

    /*
     * We create a tstream here, pass it onto mail_open, use it
     * and close it before exit.
     */
    tstream = mail_stream_create(stream->userdata, NIL,NIL,NIL,NIL);
    tstream = mail_open (tstream,mbx,OP_HALFOPEN|OP_SILENT);

    /*
     * mail_open might fail here ...
     *
     * If mail_open fails, we'll have an unopened stream. 
     */
    if (!(tstream->stream_status & S_OPENED)) {
      /* We still need to flush the tstream.  */
      mail_stream_flush(tstream);
      return(ret);
    }
  }
  args[0] = &ambx;args[1] = NIL;/* set up first argument as mailbox */
  ambx.type = ASTRING; ambx.text = (void *) mb.mailbox;

  if (((IMAPLOCAL *) stream->local)->use_status) {	/* IMAP4 way */
    aflg.type = FLAGS; aflg.text = (void *) tmp;
    args[1] = &aflg; args[2] = NIL;
    tmp[0] = tmp[1] = '\0';	/* build flag list */

    if (flags & SA_MESSAGES) {
      strcat(tmp, " MESSAGES");
    }

    if (flags & SA_RECENT) {
      strcat(tmp, " RECENT");
    }

    if (flags & SA_UNSEEN) {
      strcat(tmp, " UNSEEN");
    }

    if (flags & SA_UIDNEXT) {
      strcat(tmp, " UID-NEXT");
    }

    if (flags & SA_UIDVALIDITY) {
      strcat(tmp, " UID-VALIDITY");
    }

    if (tstream->sunvset) {	/* only request if sunversion */
      if (flags & SA_CHECKSUM) {
	strcat(tmp, " CHECKSUM");
      }
    }
    tmp[0] = '(';
    strcat(tmp,")");

    /* send "STATUS mailbox flag" */
    if (imap_OK(tstream, imap_send(tstream, "STATUS", args))) {
      ret = _B_TRUE;
    }
  } else {			/* IMAP2 way */
    MAILSTATUS 		status;

    /*
     * JUST want status for checksumming and for some reason
     * used status command rather than mail_checksum()
     */
    if (((IMAPLOCAL *) stream->local)->csstatus || flags & SA_CKSUMONLY) { 
      /*
       * ((IMAP4 server with NO STATUS CAPABILITY) OR 
       * IMAP2 server) AND SUPPORT sunversion:  IMAP2 checksum only
       */
      if (tstream->sunvset && (flags & SA_CHECKSUM)) {
	if (imap_OK(tstream, imap_send(tstream, "CHECKSUM", NIL))) {
	  status.checksum = tstream->checksum;
	  status.checksum_bytes = tstream->mbox_size;
	} else {
	  status.checksum = 0;
	  status.checksum_bytes = 0L;
	}
      }
    } else if (imap_OK(tstream, imap_send(tstream, "EXAMINE", args))) {
      status.flags = flags & ~ (SA_UIDNEXT | SA_UIDVALIDITY);
      status.messages = tstream->nmsgs;
      status.recent = tstream->recent;
      status.unseen = 0;
      if (flags & SA_UNSEEN) {	/* must search to get unseen messages */
				/* clear search vector */
	for (i = 1; i <= tstream->nmsgs; ++i) {
	  mail_elt(tstream, i)->searched = NIL;
	}
	if (imap_OK(tstream, imap_send(tstream, "SEARCH UNSEEN", NIL))) {
	  for (i = 1,status.unseen = 0; i <= tstream->nmsgs; i++) {
	    if (mail_elt(tstream, i)->searched) {
	      status.unseen++;
	    }
	  }
	}
      }

      if (tstream->sunvset && (flags & SA_CHECKSUM)) {
	if (imap_OK(tstream, imap_send(tstream, "CHECKSUM", NIL))) {
	  status.checksum = tstream->checksum;
	  status.checksum_bytes = tstream->mbox_size;
	} else {
	  status.checksum = 0;
	  status.checksum_bytes = 0L;
	}
      }
    }
    sprintf(tmp, "{%s}%s", imap_host(tstream), mb.mailbox);

    /* pass status to main program */
    mm_status(tstream, tmp, &status);
    ret = _B_TRUE;			/* note success */
  }				/* end - Imap2 way */

  /*If we get to here, we know stream != tstream. */
  if (stream != tstream) {
    mail_close (tstream);
  }
  return(ret);
}

/*
 * IMAP open
 * Accepts: stream to open
 * Returns: stream to use on success, NIL on failure
 */
MAILSTREAM *
imap_open(MAILSTREAM * stream)
{
  IMAP_GLOBALS 	*	ig = (IMAP_GLOBALS *)stream->imap_globals;
  long 			i;
  long			j;
  char 			c[2];
  char			usr[MAILTMPLEN];
  char			pwd[MAILTMPLEN];
  char			tmp[MAILTMPLEN];
  NETMBX 		mb;
  char 		*	s;
  void 		*	tstream;
  IMAPPARSEDREPLY *	reply = NIL;
  IMAPARG 	*	args[3];

  
  /* return prototype for OP_PROTOTYPE call */
  /* Need to test for stream_status here. */
  if (stream == NULL || !(stream->stream_status & S_OPENED)) {
    return(&imapproto);
  }
  mail_valid_net_parse(stream->mailbox, &mb);
  if (((IMAPLOCAL *) stream->local)) {			/* if stream opened earlier by us */
    if (strcmp(ucase(strcpy(tmp, mb.host)),
		ucase(strcpy(pwd, imap_host(stream))))) {

      /* if hosts are different punt it */
      sprintf(tmp, "Closing connection to %s", imap_host(stream));
      if (!stream->silent) {
	mm_log(tmp, (long)NIL, stream);
      }
      imap_close(stream, NIL);
    } else {			/* else recycle if still alive */
      /* Sun Imap4: NO io err callbacks */
      set_disallow_flag(((IMAPLOCAL *) stream->local)->tcpstream, _B_TRUE);

      i = stream->silent;	/* temporarily mark silent */
      stream->silent = _B_TRUE;	/* don't give mm_exists() events */
      j = imap_ping(stream);	/* learn if stream still alive */
      stream->silent = i;	/* restore prior state */

      /* Sun Imap4: turn on io err trucs */
      set_disallow_flag(((IMAPLOCAL *) stream->local)->tcpstream, NIL);
      if (j) {			/* was stream still alive? */
	sprintf (tmp, "Reusing connection to %s", mb.host);
	if (!stream->silent) {
	  mm_log (tmp,(long) NIL, stream);
	}
	imap_gc(stream, GC_TEXTS);
      } else {
	imap_close(stream, NIL);
      }
    }
    mail_free_cache(stream);
  }
				/* in case /debug switch given */
  if (mb.dbgflag) {
    stream->debug = _B_TRUE;
  }

  if (!((IMAPLOCAL *) stream->local)) {			/* open new connection if no recycle */
    stream->local = fs_get(sizeof(IMAPLOCAL));
    ((IMAPLOCAL *) stream->local)->reply.line = ((IMAPLOCAL *) stream->local)->reply.tag = ((IMAPLOCAL *) stream->local)->reply.key =
      ((IMAPLOCAL *) stream->local)->reply.text = ((IMAPLOCAL *) stream->local)->prefix = NIL;

    /* assume not IMAP4 (yet anyway) */
    ((IMAPLOCAL *) stream->local)->imap4 = ((IMAPLOCAL *) stream->local)->uidsearch = ((IMAPLOCAL *) stream->local)->csstatus = 
      ((IMAPLOCAL *) stream->local)->use_status = ((IMAPLOCAL *) stream->local)->sync_cache = NIL;

    ((IMAPLOCAL *) stream->local)->use_auth = NIL;
    ((IMAPLOCAL *) stream->local)->use_scan = NIL;
    ((IMAPLOCAL *) stream->local)->sync_start = 0;

    /* assume IMAP2bis server */
    ((IMAPLOCAL *) stream->local)->imap2bis = ((IMAPLOCAL *) stream->local)->rfc1176 = _B_TRUE;
    ((IMAPLOCAL *) stream->local)->tcpstream = NULL;	/* no TCP stream yet */
    ((IMAPLOCAL *) stream->local)->sunvcap = _B_FALSE;
	
    /* try authenticated open */
    if (mb.port || ig->imap_defaultport) {	/* set up host with port override */
      sprintf(s = tmp,
	      "%s:%ld",
	      mb.host,mb.port ? mb.port : ig->imap_defaultport);
    } else {
      s = mb.host;		/* simple host name */
    }

    /* try to open ordinary connection */
    if (((IMAPLOCAL *) stream->local)->tcpstream = tcp_open(s, "imap", IMAPTCPPORT, stream)) {
      /* Sun Imap4: stream saved on tcpstream */
      set_tcpstream_stream(((IMAPLOCAL *) stream->local)->tcpstream, (void *)stream); 	
      if (!imap_OK(stream, reply = imap_reply(stream, NIL))) {
	mm_log(reply->text, ERROR, stream);
	if (((IMAPLOCAL *) stream->local)->tcpstream) {
	  tcp_close(((IMAPLOCAL *) stream->local)->tcpstream);
	}
	((IMAPLOCAL *) stream->local)->tcpstream = NIL;
      } else {		
	
	/* note if IMAP4/Sunversion */
	imap_send(stream, "CAPABILITY", NIL);
	stream->imap4_connected = ((IMAPLOCAL *) stream->local)->imap4;

	/* Send our SUNVERSION command */
	imap_sunversion(stream);

	/* get any userdata for open */
	/* moved GET_USERDATA UP */
	if (!stream->userdata) {

	  mail_stream_setNIL(stream);
	  stream->userdata = mail_parameters(stream, GET_USERDATA, NULL);
	  mail_stream_unsetNIL(stream);
	}

	/* only so many tries to login */
	for (i = ig->imap_maxlogintrials; i && ((IMAPLOCAL *) stream->local) && ((IMAPLOCAL *) stream->local)->tcpstream;) {
	  if (((IMAPLOCAL *) stream->local)->use_auth && !(mb.anoflag || stream->anonymous)) {
	    imap_auth(stream,&mb,tmp,(unsigned long *)&i);
	  } else {		/* only so many tries to login */
	    char 		usr[MAILTMPLEN];
	    IMAPARG 		ausr;
	    IMAPARG		apwd;

	    ausr.type = apwd.type = ASTRING;
	    ausr.text = (void *) usr;
	    apwd.text = (void *) tmp;
	    args[0] = &ausr; args[1] = &apwd; args[2] = NIL;

	    tmp[0] = 0;	/* get password */

	    /* automatic if want anonymous access */
	    if (mb.anoflag || stream->anonymous) {
	      strcpy(usr, "anonymous");
	      strcpy(tmp, tcp_localhost(((IMAPLOCAL *) stream->local)->tcpstream));
	      i = 1;		/* only one trial */
	    }
	    /* otherwise prompt user */
	    else mm_login (stream,&mb,usr,tmp,ig->imap_maxlogintrials - i);
	    if (!tmp[0]) {	/* user refused to give a password */
	      mm_log ("Login aborted",ERROR, stream);
	      if (((IMAPLOCAL *) stream->local)->tcpstream) tcp_close (((IMAPLOCAL *) stream->local)->tcpstream);
	      ((IMAPLOCAL *) stream->local)->tcpstream = NIL;
	    }
	    /* send "LOGIN usr tmp" */
	    else if (imap_OK(stream,reply = imap_send(stream,"LOGIN",args))){
	      /* login successful, note if anonymous */
	      stream->anonymous = strcmp(usr,"anonymous") ? _B_FALSE : _B_TRUE;
	      i = 0;		/* exit login loop */
	    }
	    else mm_log (reply->text,WARN, stream);
	  }
	  if (i && !--i) {	/* login failed */
	    mm_log ("Too many login failures",ERROR,stream);
	    mm_log_stream(stream,"148 Too many login failures");
	    mm_login(stream,&mb,usr,tmp,-1);
	    if (((IMAPLOCAL *) stream->local)->tcpstream) tcp_close (((IMAPLOCAL *) stream->local)->tcpstream);
	    ((IMAPLOCAL *) stream->local)->tcpstream = NIL;
	  }
	}
      }
    }
 				/* failed utterly to open */
    if (((IMAPLOCAL *) stream->local) && !((IMAPLOCAL *) stream->local)->tcpstream) imap_close (stream,NIL);
  }

  if (((IMAPLOCAL *) stream->local)) {			/* have a connection now??? */
    IMAPARG ambx;
    ambx.type = ASTRING;
    ambx.text = (void *) mb.mailbox;
    args[0] = &ambx; args[1] = NIL;
    stream->perm_seen = stream->perm_deleted = stream->perm_answered = 
     stream->perm_draft = stream->kwd_create = ((IMAPLOCAL *) stream->local)->imap4 ? _B_FALSE : _B_TRUE;
    stream->perm_user_flags = ((IMAPLOCAL *) stream->local)->imap4 ? NIL : 0xffffffff;
    stream->sequence++;		/* bump sequence number */
	                        /* turn off stream dead callback */
    set_use_maxwait(((IMAPLOCAL *) stream->local)->tcpstream, NIL); /* Sun Imap4 */
    if ((i = imap_port (stream)) & 0xffff0000) i = ig->imap_defaultport ? 
      ig->imap_defaultport : IMAPTCPPORT;
    if (stream->halfopen ||	/* send "SELECT/EXAMINE mailbox" */
	!imap_OK (stream,reply = imap_send (stream,stream->rdonly ?
					    "EXAMINE": "SELECT",args))) {
				/* want die on error and not halfopen? */
      if (ig->imap_closeonerror && !stream->halfopen) {
	if (((IMAPLOCAL *) stream->local) && ((IMAPLOCAL *) stream->local)->tcpstream) imap_close (stream,NIL);
	return NIL;		/* return open failure */
      }
				/* half-open the stream */
      /* Sun cannot deal with fully qualified names in mailbox 
       * imap_host returns fully qualified name. This can cause
       * possible conflicts between DNS and NIS. C'est moche!
       */
#ifdef WANT_FULLY_QUALIFIED_HOSTNAME
      fs_give ((void **) &stream->mailbox);
      sprintf (tmp,"{%s:%lu/imap}<no_mailbox>",imap_host (stream),i);
      stream->mailbox = cpystr (tmp);
#else
      strcat (tmp,"<no_mailbox>");
#endif
      if (!stream->halfopen) {	/* output error message if didn't ask for it */
	sprintf (((IMAPLOCAL *) stream->local)->tmp,"114 Can't select mailbox: %.80s",reply->text);
	mm_log (((IMAPLOCAL *) stream->local)->tmp, ERROR, stream);
	mm_log_stream(stream, ((IMAPLOCAL *) stream->local)->tmp);
	stream->halfopen = _B_TRUE;
      }
      /* Sun Imap4: only keep the primary inbox streams
       *   See if failed secondary_mbox */
      if (stream->secondary_mbox)  return NIL;
				/* make sure dummy message counts */
      mail_exists (stream,(long) 0);
      mail_recent (stream,(long) 0);
    }
    else {			/* update mailbox name */
#ifdef WANT_FULLY_QUALIFIED_HOSTNAME
      fs_give ((void **) &stream->mailbox);
      sprintf (tmp,"{%s:%lu/imap}",imap_host (stream),i);
      strcat (tmp,mb.mailbox);	/* mailbox name */
      stream->mailbox = cpystr (tmp);
#endif
      reply->text[11] = '\0';	/* note if server said it was readonly */
      stream->rdonly = !strcmp(ucase(reply->text), "[READ-ONLY]");
    }
    if (!(stream->nmsgs || stream->silent || stream->halfopen))
      mm_log ("Mailbox is empty",(long) NIL, stream);
    set_use_maxwait(((IMAPLOCAL *) stream->local)->tcpstream, T); /* Sun Imap4: normal timeouts */
  }
				/* give up if nuked during startup */
  if (((IMAPLOCAL *) stream->local) && !((IMAPLOCAL *) stream->local)->tcpstream) imap_close (stream,NIL);
  return ((IMAPLOCAL *) stream->local) ? stream : NIL;	/* if stream is alive, return to caller */
}

/* IMAP authenticate
 * Accepts: stream to authenticate
 *	    parsed network mailbox structure
 *	    scratch buffer
 *	    pointer to trial countdown
 */

void imap_auth (MAILSTREAM *stream,NETMBX *mb,char *tmp,unsigned long *trial)
{
  IMAP_GLOBALS *ig = (IMAP_GLOBALS *)stream->imap_globals;
  int i;
  char tag[16];
  AUTHENTICATOR *auth;
  for (i = 0, auth = authenticators;
       ((IMAPLOCAL *) stream->local)->tcpstream && auth && (i < MAXAUTHENTICATORS);
       i++, auth = auth->next) {
  				/* gensym a new tag */
    sprintf (tag,"A%05ld",stream->gensym++);
				/* build command */
    sprintf (tmp,"%s AUTHENTICATE %s",tag,auth->name);
    if (stream->debug) mm_dlog (stream,tmp);
    strcat (tmp,"\r\n");
    if (!tcp_soutr (((IMAPLOCAL *) stream->local)->tcpstream,tmp)) break;
    if (!(*auth->client) (imap_challenge,imap_response,mb,stream,
			  ig->imap_maxlogintrials - *trial)) return;
				/* make sure have tagged response */
    while (((IMAPLOCAL *) stream->local)->tcpstream && strcmp(imap_reply(stream,tag)->tag,tag)) {
      if (stream->debug) mm_dlog (stream,"*");
      if (!tcp_soutr (((IMAPLOCAL *) stream->local)->tcpstream,"*\r\n")) return;
    }
				/* if got success response */
    if (((IMAPLOCAL *) stream->local)->tcpstream && imap_OK (stream,&((IMAPLOCAL *) stream->local)->reply)) {
      *trial = 0;
      return;
    }
  }
}

/* Get challenge to authenticator in binary
 * Accepts: stream
 *	    pointer to returned size
 * Returns: challenge or NIL if not challenge
 */

void *imap_challenge (void *s,unsigned long *len)
{
  MAILSTREAM *stream = (MAILSTREAM *) s;
  IMAPPARSEDREPLY *reply =
    imap_parse_reply (stream,tcp_getline (((IMAPLOCAL *) stream->local)->tcpstream));
  if (!reply) return NIL;
  return strcmp(reply->tag,"+") ? NIL :
    rfc822_base64 ((unsigned char *) reply->text,strlen (reply->text),len);
}


/* Send authenticator response in BASE64
 * Accepts: MAIL stream
 *	    string to send
 *	    length of string
 * Returns: T if successful, else NIL
 */

long imap_response (void *s,char *response,unsigned long size)
{
  MAILSTREAM *stream = (MAILSTREAM *) s;
  unsigned long len;
  char *t = (char *) rfc822_binary ((void *) response,size,&len);
  long ret = tcp_sout (((IMAPLOCAL *) stream->local)->tcpstream,t,len);
  if (stream->debug && (len > 2)) {
    t[len -2] = '\0';		/* tie off CRLF */
    mm_dlog (stream,t);
  }
  fs_give ((void **) &t);
  return ret;
}

/* IMAP close
 * Accepts: MAIL stream
 *	    option flags
 */

void imap_close (MAILSTREAM *stream,long options)
{
  IMAPPARSEDREPLY *reply;
  if (stream && ((IMAPLOCAL *) stream->local)) {	/* send "LOGOUT" */
				/* expunge silently if requested */
    if (options & CL_EXPUNGE) imap_send (stream,"EXPUNGE",NIL);
    if (((IMAPLOCAL *) stream->local)->tcpstream &&
	!imap_OK (stream,reply = imap_send (stream,"LOGOUT",NIL))) {
      mm_log (reply->text, WARN, stream);
      mm_log_stream(stream, reply->text);
    }
				/* close TCP connection if still open */
    if (((IMAPLOCAL *) stream->local)->tcpstream) tcp_close (((IMAPLOCAL *) stream->local)->tcpstream);
    ((IMAPLOCAL *) stream->local)->tcpstream = NIL;
				/* free up memory */
    if (((IMAPLOCAL *) stream->local)->reply.line) fs_give ((void **) &((IMAPLOCAL *) stream->local)->reply.line);
    /* if we are recording, then our attempt to reconnect failed.
     * Leave the cache intact... */
    fs_give ((void **) &stream->local);
  }
}

static int envelope_present (MAILSTREAM *stream, long i)
{
  ENVELOPE *env = mail_lelt (stream,i)->env;
  if (env) return T;
  else return NIL;
}

/* IMAP fetch fast information
 * Accepts: MAIL stream
 *	    sequence
 *	    option flags
 *
 * Generally, imap_fetchstructure is preferred
 */

void imap_fetchfast (MAILSTREAM *stream,char *sequence,long flags)
{				/* send "FETCH sequence FAST" */
  char *cmd = (((IMAPLOCAL *) stream->local)->imap4 && (flags & FT_UID)) ? "UID FETCH" : "FETCH";
  IMAPPARSEDREPLY *reply;
  IMAPARG *args[3],aseq,aatt;
  aseq.type = aatt.type = ATOM; aseq.text = (void *) sequence;
  aatt.text = (void *) "FAST";
  args[0] = &aseq; args[1] = &aatt; args[2] = NIL;
  if (!imap_OK (stream,reply = imap_send (stream,cmd,args)))
    mm_log (reply->text,ERROR,stream);
}


/* IMAP fetch flags
 * Accepts: MAIL stream
 *	    sequence
 *	    option flags
 */

void imap_fetchflags (MAILSTREAM *stream,char *sequence,long flags)
{				/* send "FETCH sequence FLAGS" */
  char *cmd = (((IMAPLOCAL *) stream->local)->imap4 && (flags & FT_UID)) ? "UID FETCH" : "FETCH";
  IMAPPARSEDREPLY *reply;
  IMAPARG *args[3],aseq,aatt;
  aseq.type = aatt.type = ATOM; aseq.text = (void *) sequence;
  aatt.text = (void *) "FLAGS";
  args[0] = &aseq; args[1] = &aatt; args[2] = NIL;
  if (!imap_OK (stream,reply = imap_send (stream,cmd,args)))
    mm_log (reply->text,ERROR, stream);
}

/* makes the lookhead sequence taking into account
 * gaps where data may be present, rather than just
 * stopping at the first non-empty data value */
static void make_lookahead (MAILSTREAM *stream, long msgno, char *seq, 
			   int (*data_present)(MAILSTREAM *stream, long i))
{
  IMAP_GLOBALS *ig = (IMAP_GLOBALS *)stream->imap_globals; 
  long i = msgno; 
  long j =  min (msgno + ig->imap_lookahead  - 1, stream->nmsgs);
  long k, l;
  if (msgno != stream->nmsgs) {	/* determine lookahead range */
    char *sptr = seq;
    char tmp[SEQBUFLEN];
    l = 0;				/* lower bound */
    for (k = l = 0; i <= j; i++) {
      if (!(*data_present)(stream, i)) {
	int next = i + 1;
	if (next <= j && !(*data_present)(stream, next)) {
	  if (l == 0) l = i;
	  continue;			/* consecutive */
	} else {
	  if (k++ > 0) *sptr++ = ','; /* have an entry */
	  if (l > 0) sprintf (tmp, "%ld:%ld", l, i);
	  else
	    sprintf(tmp, "%ld", i);
	  strcpy(sptr, tmp);
	  sptr += strlen(sptr);
	  l = 0;
	}
      }
    }
  } else
    sprintf (seq,"%ld",msgno);
}  


static int structure_present (MAILSTREAM *stream, long i)
{
  ENVELOPE *env = mail_lelt (stream,i)->env;
  BODY *b = mail_lelt (stream, i)->body;
  return (env && b ? T : NIL);
}

/* IMAP fetch structure
 * Accepts: MAIL stream
 *	    message # to fetch
 *	    pointer to return body
 *	    option flags
 * Returns: envelope of this message, body returned in body value
 *
 * Fetches the "fast" information as well
 */

ENVELOPE *imap_fetchstructure (MAILSTREAM *stream,unsigned long msgno,
			       BODY **body,long flags)
{
  char *cmd = "FETCH";
  char *seq = ((IMAPLOCAL *) stream->local)->sbuf;
  ENVELOPE **env = &stream->env;
  BODY **b = &stream->body;
  IMAPPARSEDREPLY *reply = NIL;
  IMAPARG *args[3],aseq,aatt;
  args[0] = &aseq; args[1] = &aatt; args[2] = NIL;
  aseq.type = aatt.type = ATOM; aseq.text = (void *) seq;
				/* never lookahead with UID fetching */
  if (((IMAPLOCAL *) stream->local)->imap4 && (flags & FT_UID)) {
    cmd = "UID FETCH";
    ((IMAPLOCAL *) stream->local)->scache = _B_TRUE;		/* force short with local var. */
    stream->msgno = 0;		/* force refetching */
  } else
    ((IMAPLOCAL *) stream->local)->scache = NIL;
  if (((IMAPLOCAL *) stream->local)->scache || stream->scache) {/* short cache */
    if (msgno != stream->msgno){/* flush old poop if a different message */
      mail_free_envelope (env);
      mail_free_body (b);
    }
    stream->msgno = msgno;
    sprintf (seq,"%ld",msgno);	/* never lookahead with a short cache */
  }
  else {			/* long cache */
    LONGCACHE *lelt = mail_lelt (stream,msgno);
    env = &lelt->env;		/* get pointers to envelope and body */
    b = &lelt->body;
    make_lookahead (stream, msgno, seq, structure_present);
  }
  if (body && !*b) {		/* want body? */
    if (((IMAPLOCAL *) stream->local)->imap4) {		/* IMAP4 version */
      aatt.text = (void *)
	(*env ? "BODYSTRUCTURE" :
	 "(FLAGS INTERNALDATE RFC822.SIZE ENVELOPE BODYSTRUCTURE)");
      reply = imap_send (stream,cmd,args);
    }
    else if (((IMAPLOCAL *) stream->local)->imap2bis) {	/* IMAP2bis version */
      aatt.text = (void *) (*env ? "BODY" : "FULL");
      reply = imap_send (stream,cmd,args);
      if (strcasecmp(reply->key, "BAD") == 0) 
	((IMAPLOCAL *) stream->local)->imap2bis = NIL;
    }
  }
  if (!*env) {			/* get envelope if don't have it yet */
    aatt.text = (void *) "ALL";	/* basic IMAP2 version */
    reply = imap_send (stream,cmd,args);
  }
  if (reply && !imap_OK (stream,reply)) {
    mm_log (reply->text,ERROR, stream);
    return NIL;
  }
  if (body) *body = *b;		/* return the body */
  return *env;			/* return the envelope */
}

/* IMAP fetch envelope information only
 * Accepts: MAIL stream
 *	    sequence
 *	    option flags
 * NO short cache stuff.
 */
ENVELOPE *imap_fetchenvelope(MAILSTREAM *stream,unsigned long msgno,long flags)
{
  char *cmd = (((IMAPLOCAL *) stream->local)->imap4 && (flags & FT_UID)) ? "UID FETCH" : "FETCH";
  char *seq = ((IMAPLOCAL *) stream->local)->sbuf;
  LONGCACHE *lelt = mail_lelt (stream,msgno);
  ENVELOPE **env = &lelt->env;
  IMAPPARSEDREPLY *reply;
  IMAPARG *args[3],aseq,aatt;

  if (*env) return *env;                /* return existing envelope */

  aseq.type = aatt.type = ATOM; 
  aseq.text = (void *)seq;
  aatt.text = (void *) "ENVELOPE";
  args[0] = &aseq; args[1] = &aatt; args[2] = NIL;
  make_lookahead (stream,msgno,seq,envelope_present);
  if (!imap_OK (stream,reply = imap_send (stream,cmd,args)))
    mm_log (reply->text,ERROR, stream);
  else return *env;
}

/* IMAP fetch message header
 * Accepts: MAIL stream
 *	    message # to fetch
 * Returns: message header in RFC822 format
 */

char *imap_fetchheader (MAILSTREAM *stream,unsigned long msgno,
			STRINGLIST *lines,unsigned long *len,long flags)
{
  mailgets_t mg = (mailgets_t) mail_parameters (stream,GET_GETS,NIL);
		/* OK to pass just a stream */
  char *cmd = (((IMAPLOCAL *) stream->local)->imap4 && (flags & FT_UID)) ? "UID FETCH" : "FETCH";
  IMAPPARSEDREPLY *reply;
  MESSAGECACHE *elt = NIL;
  unsigned long i;
  IMAPARG *args[4],aseq,aatt,ahdr;

  if (len) *len = 0;		/* no data returned yet */
  if (flags & FT_UID) {		/* UID form of call */
    for (i = 1; i <= stream->nmsgs; i++)
      if (mail_elt (stream,i)->uid == msgno)
				/* USE real message no. We have an elt */
	return imap_fetchheader (stream,i,lines,len,flags & ~FT_UID);
  }
  else elt = mail_elt (stream,msgno);
  aseq.type = NUMBER; aseq.text = (void *) msgno;
  aatt.type = ATOM;
  args[0] = &aseq; args[1] = &aatt; args[2] = args[3] = NIL;
  /* Sun IMAP4: See if data is cached
   *   data1  full header
   *   data3  full header size
   *   filter1 filtered header
   *   filter2 filtered header size */
  if (elt) {
    if (elt->data1) {		/* We have the full header */
      if (lines) {		/* We can recompute the filtered data */
	if (elt->filter1) fs_give((void **)&elt->filter1);
	elt->filter1 = (unsigned long)cpystr((char *)elt->data1);
	elt->filter2 = mail_filter ((char *)elt->filter1,elt->data3,lines,
				    flags);
	if (len) *len = elt->filter2;
	return (char *)elt->filter1;
      } else {
	if (len) *len = elt->data3;
	return (char *) elt->data1;
      }
    } else if (lines) {
      if (elt->filter1) {
				/* lines and a cached filter set */
	if (mail_stringlist_cmp(lines,elt->lines)) {
				/* same filter so return the cache  */
	  if (len) *len = elt->filter2;
	  return (char *)elt->filter1;
	} else {		/* flush old filter. */
	  fs_give((void **)&elt->filter1);
	  mail_free_stringlist (&elt->lines);
				/* save current for this fetch */
	  elt->lines = mail_dup_stringlist(lines);
	}
      } else			/* no filter1, save initial lines */
	elt->lines = mail_dup_stringlist(lines);
    }
  }
  if (((IMAPLOCAL *) stream->local)->imap4 && lines) {
    aatt.text = (void *)
      ((flags & FT_NOT) ? "RFC822.HEADER.LINES.NOT" : "RFC822.HEADER.LINES");
    ahdr.type = LIST; ahdr.text = (void *) lines;
    args[2] = &ahdr;
    ((IMAPLOCAL *) stream->local)->hdr_lines = _B_TRUE;
  }
  else {
    aatt.text = (void *) "RFC822.HEADER";
    ((IMAPLOCAL *) stream->local)->hdr_lines = NIL;
  }
				/* force into memory */
	mail_stream_setNIL(stream);
  	mail_parameters (stream,SET_GETS,NIL);
	mail_stream_unsetNIL(stream);

  if (!imap_OK (stream,reply = imap_send (stream,cmd,args)))
    mm_log (reply->text,ERROR, stream);
				/* restore mailgets routine */
	mail_stream_setNIL(stream);
        mail_parameters (stream,SET_GETS,(void *) mg);
	mail_stream_unsetNIL(stream);

  if (flags & FT_UID) {		/* hunt for UID */
    for (i = 1; i <= stream->nmsgs; i++)
      if ((elt = mail_elt (stream,i))->uid == msgno) break;
    return "";			/* UID not found */
  }	
  /*
   * See if we requested lines. Then data is stashed
   * in elt->filter1 FOR IMAP4 ONLY!! */
  if (lines && elt->filter1) {	/* then have the filtered data */
    if (len) *len = elt->filter2;/* return data size */
    return (char *) elt->filter1;
  } else if (elt->data1) {	/* got returned data? */
    if (!((IMAPLOCAL *) stream->local)->imap4 && lines) {/* see if have to filter locally */
      /* IMAP2 server. WE need to filter */
      elt->filter1 = (unsigned long)cpystr((char *)elt->data1);
      elt->filter2 = mail_filter ((char *)elt->filter1,elt->data3,lines,
				  flags);
      if (len) *len = elt->filter2;
      return (char *)elt->filter1;
    } else {
      /* (imap4 or imap2 and !lines) */
      if (len) *len = elt->data3;	/* return data size */
      return (char *) elt->data1;
    }
  }
  else return "";		/* couldn't find anything */
}

/* IMAP fetch message text (body only)
 * Accepts: MAIL stream
 *	    message # to fetch
 *	    returned message length
 *	    option flags
 * Returns: message text in RFC822 format
 */

char *imap_fetchtext (MAILSTREAM *stream,unsigned long msgno,
		      unsigned long *len,long flags)
{
  char *cmd = (((IMAPLOCAL *) stream->local)->imap4 && (flags & FT_UID)) ? "UID FETCH" : "FETCH";
  IMAPPARSEDREPLY *reply;
  MESSAGECACHE *elt = NIL;
  unsigned long i;
  IMAPARG *args[3],aseq,aatt;
  if (len) *len = 0;		/* no data returned yet */
  if (flags & FT_UID) {		/* UID form of call */
    for (i = 1; i <= stream->nmsgs; i++)
      if (mail_elt (stream,i)->uid == msgno)
	return imap_fetchtext (stream,i,len,flags & ~FT_UID);
  }
  else elt = mail_elt (stream,msgno);
  if (!(elt && elt->data2)) {	/* not if already cached */
    aseq.type = NUMBER; aseq.text = (void *) msgno;
    aatt.type = ATOM;
    aatt.text = (void *)
      ((((IMAPLOCAL *) stream->local)->imap4 && (flags & FT_PEEK)) ? "RFC822.TEXT.PEEK":"RFC822.TEXT");
    args[0] = &aseq; args[1] = &aatt; args[2] = NIL;
    if (!imap_OK (stream,reply = imap_send (stream,cmd,args)))
      mm_log (reply->text,ERROR, stream);
  }
  if (flags & FT_UID) {		/* hunt for UID */
    for (i = 1; i <= stream->nmsgs; i++)
      if ((elt = mail_elt (stream,i))->uid == msgno) break;
    return "";			/* UID not found */
  }	
  if (elt->data2) {		/* got returned data? */
    if (len) *len = elt->data4;	/* return data size */
    return (char *) elt->data2;
  }
  else return "";		/* couldn't find anything */
}

/* IMAP fetch message body as a structure
 * Accepts: Mail stream
 *	    message # to fetch
 *	    section specifier
 *	    pointer to length
 *	    option flags
 * Returns: pointer to section of message body
 */

char *imap_fetchbody (MAILSTREAM *stream,unsigned long msgno,char *sec,
		      unsigned long *len,long flags)
{
  char *cmd = (((IMAPLOCAL *) stream->local)->imap4 && (flags & FT_UID)) ? "UID FETCH" : "FETCH";
  BODY *b;
  PART *pt;
  char *s = sec;
  char **ss = NIL;
  unsigned long i;
  IMAPPARSEDREPLY *reply;
  IMAPARG *args[3],aseq,aatt;
  if (len) *len = 0;		/* in case failure */
				/* if bodies not supported */
  if (!(((IMAPLOCAL *) stream->local)->imap4 || ((IMAPLOCAL *) stream->local)->imap2bis))
    return strcmp(sec, "1") ? NIL : imap_fetchtext (stream,msgno,len,flags);
  if (flags & FT_UID) {		/* UID form of call */
    for (i = 1; i <= stream->nmsgs; i++)
      if (mail_elt (stream,i)->uid == msgno)
	return imap_fetchbody (stream,i,sec,len,flags & ~FT_UID);
  }
				/* make sure have a body */
  if (!(imap_fetchstructure (stream,msgno,&b,flags) && b))
    return strcmp(sec, "1") ? NIL : imap_fetchtext (stream,msgno,len,flags);
  if (!(s && *s && isdigit (*s))) return NIL;
  if (!(i = strtoul (s,&s,10)))	/* get section number, header if 0 */
    return imap_fetchheader (stream,msgno,NIL,len,flags);
  aseq.type = NUMBER; aseq.text = (void *) msgno;
  aatt.type = (((IMAPLOCAL *) stream->local)->imap4 && (flags & FT_PEEK)) ? BODYPEEK : BODYTEXT;
  aatt.text = (void *) sec;
  args[0] = &aseq; args[1] = &aatt; args[2] = NIL;

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
      if (!((*s++ == '.') && isdigit (*s))) return NIL;
				/* body section? */
      if (i = strtoul (s,&s,10)) b = b->contents.msg.body;
      else {			/* header section */
	ss = &b->contents.msg.hdr;
				/* fetch data if don't have it yet */
	if (!b->contents.msg.hdr &&
	    !imap_OK (stream,reply = imap_send (stream,cmd,args)))
	  mm_log (reply->text,ERROR, stream);
				/* return data size if have data */
	if (len) *len = b->contents.msg.hdrsize;
	return b->contents.msg.hdr;
      }
      break;
    case TYPEMULTIPART:		/* multipart, get next section */
      if ((*s++ == '.') && isdigit (*s) && (i = strtoul (s,&s,10)) > 0) break;
    default:			/* bogus subpart specification */
      return NIL;
    }
  } while (i);
				/* lose if body bogus */
  if ((!b) || b->type == TYPEMULTIPART) return NIL;
  switch (b->type) {		/* decide where the data is based on type */
  case TYPEMESSAGE:		/* encapsulated message */
    if (!ss) ss = &b->contents.msg.text;
    break;
  case TYPETEXT:		/* textual data */
    ss = (char **) &b->contents.text;
    break;
  default:			/* otherwise assume it is binary */
    ss = (char **) &b->contents.binary;
    break;
  }
				/* fetch data if don't have it yet */
  if (!*ss && !imap_OK (stream,reply = imap_send (stream,cmd,args)))
    mm_log (reply->text,ERROR, stream);
				/* return data size if have data */
  if ((s = *ss) && len) *len = b->size.bytes;
  return s;
}

/* IMAP fetch UID
 * Accepts: MAIL stream
 *	    message number
 * Returns: UID
 */

unsigned long imap_uid (MAILSTREAM *stream,unsigned long msgno)
{
  IMAP_GLOBALS *ig = (IMAP_GLOBALS *)stream->imap_globals;
  if (((IMAPLOCAL *) stream->local)->imap4) {		/* only for IMAP4 */
    MESSAGECACHE *elt = mail_elt (stream,msgno);
    if (!elt->uid) {		/* do we know its UID yet? */
      IMAPPARSEDREPLY *reply;
      IMAPARG *args[3],aseq,aatt;
      char *s,seq[MAILTMPLEN];
      long i,j,k;
      long last = min (msgno + ig->imap_uidlookahead,stream->nmsgs);
      aseq.type = aatt.type = ATOM; aseq.text = (void *) seq;
      aatt.text = (void *) "UID";
      args[0] = &aseq; args[1] = &aatt; args[2] = NIL;
				/* build UID list */
      sprintf (seq,"%ld",msgno);
      if (k = ig->imap_uidlookahead) {/* build UID list */
	for (i = msgno + 1, s = seq; k && (i <= stream->nmsgs); i++)
	  if (!mail_elt (stream,i)->uid) {
	    s += strlen (s);	/* find string end, see if nearing end */
	    if ((s - seq) > (MAILTMPLEN - 20)) break;
	    sprintf (s,",%ld",i);	/* append message */
	    for (j = i + 1, k--;	/* hunt for last message without a UID */
		 k && (j <= stream->nmsgs) && !mail_elt (stream,j)->uid;
		 j++, k--);
	    /* if different, make a range */
	    if (i != --j) sprintf (s + strlen (s),":%ld",i = j);
	  }
      }
      if (!imap_OK (stream,reply = imap_send (stream,"FETCH",args)))
	mm_log (reply->text,ERROR, stream);
    }
    return elt->uid;		/* return our UID now */
  }
  return msgno;			/* IMAP2 didn't have UIDs */
}


/* IMAP fetch synchronized msgno
 * Accepts: MAIL stream
 *	    uidseq to validate.
 * Returns: T on success other wise NIL;
 * Here UID FETCH <uidsequence> UID". This is for imap_uid CONSECUTIVE
 * uids. 
 * Note: The current UID may no longer exist on the server if that message
 * was expunged. What this call does is find all of the msgno's which do
 * exist for our given set of uids. EG:
 *   elt->msgno == 10   elt->uid == 1000 may return * 5 FETCH (UID 1000)
 *   thus setting for THIS elt whose msgno is 10, elt->sync_msgo == 5.
 * Note 2: In the usual case the caller will provide the uid sequence
 *   because we have reconnected and stream->nmsgs may be SMALLER
 *   than before we disconnected.
 */
int imap_sync_msgno (MAILSTREAM *stream,char *uidseq)
{
  if (((IMAPLOCAL *) stream->local)->imap4) {		/* only for IMAP4 */
    IMAPPARSEDREPLY *reply;
    IMAPARG *args[3],aseq,aatt;
    int ret = NIL;
    int free_seq;
    aseq.type = aatt.type = ATOM; 
    aatt.text = (void *) "UID";
    args[0] = &aseq; args[1] = &aatt; args[2] = NIL;
    /* If uid sequence is NOT provided, get them all */
    if (uidseq == NIL) {
      if (stream->nmsgs == 0) return NIL;
      else {
	sprintf(((IMAPLOCAL *) stream->local)->tmp,"1:%ld",stream->nmsgs);
	uidseq = mail_seq_to_uidseq(stream,((IMAPLOCAL *) stream->local)->tmp,T);
	free_seq = _B_TRUE;
      }
    } else 
      free_seq = NIL;
				/* send "UID FETCH <sequence> UID" */
    aseq.text = (void *) uidseq;
    ((IMAPLOCAL *) stream->local)->sync_cache = _B_TRUE;	/* Synchronizing cache */
    ((IMAPLOCAL *) stream->local)->sync_start = 1;      /* For the first search */
    if (uidseq && *uidseq != '\0')
      if (!imap_OK (stream,reply = imap_send (stream,"UID FETCH",args))) {
	mm_log (reply->text,ERROR, stream);
      } else ret = _B_TRUE;
    ((IMAPLOCAL *) stream->local)->sync_cache = NIL;	/* Clear synchronizing cache */
    if (free_seq && uidseq) fs_give((void **)&uidseq);
    return ret;
  }
  return T;			/* IMAP2 didn't have UIDs */
}

/* IMAP set flag
 * Accepts: MAIL stream
 *	    sequence
 *	    flag(s)
 *	    option flags
 */

void imap_setflag (MAILSTREAM *stream,char *sequence,char *flag,long flags)
{
  char *cmd = (((IMAPLOCAL *) stream->local)->imap4 && (flags & ST_UID)) ? "UID STORE" : "STORE";
  IMAPPARSEDREPLY *reply;
  IMAPARG *args[4],aseq,ascm,aflg;
  aseq.type = ATOM; aseq.text = (void *) sequence;
  ascm.type = ATOM; ascm.text = (void *)
    ((((IMAPLOCAL *) stream->local)->imap4 && (flags & ST_SILENT)) ? "+Flags.silent" : "+Flags");
  aflg.type = FLAGS; aflg.text = (void *) flag;
  args[0] = &aseq; args[1] = &ascm; args[2] = &aflg; args[3] = NIL;
				/* send "STORE sequence +Flags flag" */
  if (!imap_OK (stream,reply = imap_send (stream,cmd,args)))
    mm_log (reply->text,ERROR, stream);
}


/* IMAP clear flag
 * Accepts: MAIL stream
 *	    sequence
 *	    flag(s)
 *	    option flags
 */

void imap_clearflag (MAILSTREAM *stream,char *sequence,char *flag,long flags)
{
  char *cmd = (((IMAPLOCAL *) stream->local)->imap4 && (flags & ST_UID)) ? "UID STORE" : "STORE";
  IMAPPARSEDREPLY *reply;
  IMAPARG *args[4],aseq,ascm,aflg;
  aseq.type = ATOM; aseq.text = (void *) sequence;
  ascm.type = ATOM; ascm.text = (void *)
    ((((IMAPLOCAL *) stream->local)->imap4 && (flags & ST_SILENT)) ? "-Flags.silent" : "-Flags");
  aflg.type = FLAGS; aflg.text = (void *) flag;
  args[0] = &aseq; args[1] = &ascm; args[2] = &aflg; args[3] = NIL;
				/* send "STORE sequence +Flags flag" */
  if (!imap_OK (stream,reply = imap_send (stream,cmd,args)))
    mm_log (reply->text,ERROR, stream);
}

/* IMAP search for messages
 * Accepts: MAIL stream
 *	    character set
 *	    search program
 *	    option flags
 */

void imap_search (MAILSTREAM *stream,char *charset,SEARCHPGM *pgm,long flags)
{
  long i,j,k;
  char *s;
  char *cmd;
  IMAPPARSEDREPLY *reply;
  MESSAGECACHE *elt;
  IMAPARG *args[3],apgm,aseq,aatt;
  IMAP_GLOBALS *ig = (IMAP_GLOBALS *)stream->imap_globals;

  args[1] = args[2] = NIL;
  apgm.type = SEARCHPROGRAM; apgm.text = (void *) pgm;
  aseq.type = aatt.type = ATOM;
  
  if (charset) {
    args[0] = &aatt; args[1] = &apgm;
    aatt.text = (void *) charset;
  }
  else args[0] = &apgm;
				/* do the SEARCH/UID SEARCH */
  /* Do UID truc. */
  if ((flags & SE_UID) && ((IMAPLOCAL *) stream->local)->imap4) { 
    ((IMAPLOCAL *) stream->local)->uidsearch = _B_TRUE;
    cmd = "UID SEARCH";
  } else {
    ((IMAPLOCAL *) stream->local)->uidsearch = NIL;
    cmd = "SEARCH";
  }
  if (!imap_OK (stream,reply = imap_send (stream,cmd,args)))
    mm_log (reply->text,ERROR, stream);
				/* can never pre-fetch with a short cache */
  else if ((k = ig->imap_prefetch) && !(flags & SE_NOPREFETCH) && !stream->scache) {
    s = ((IMAPLOCAL *) stream->local)->tmp;		/* build sequence in temporary buffer */
    *s = '\0';			/* initially nothing */
				/* search through mailbox */
				/* search through mailbox */
    for (i = 1; k && (i <= stream->nmsgs); ++i) 
				/* for searched messages with no envelope */
      if ((elt = mail_elt (stream,i)) && elt->searched &&
	  !mail_lelt (stream,i)->env) {
				/* prepend with comma if not first time */
	if (((IMAPLOCAL *) stream->local)->tmp[0]) *s++ = ',';
	sprintf (s,"%ld",j = i);/* output message number */
	s += strlen (s);	/* point at end of string */
	k--;			/* count one up */
				/* search for possible end of range */
	while (k && (i < stream->nmsgs) && (elt = mail_elt (stream,i+1)) &&
	       elt->searched && !mail_lelt (stream,i+1)->env) i++,k--;
	if (i != j) {		/* if a range */
	  sprintf (s,":%ld",i);	/* output delimiter and end of range */
	  s += strlen (s);	/* point at end of string */
	}
      }
    if (((IMAPLOCAL *) stream->local)->tmp[0]) {	/* anything to pre-fetch? */
      args[0] = &aseq; args[1] = &aatt; args[2] = NIL;
      aseq.text = (void *) cpystr (((IMAPLOCAL *) stream->local)->tmp);
      aatt.text = (void *) "ALL";
      if (!imap_OK (stream,reply = imap_send (stream,"FETCH",args)))
	mm_log (reply->text,ERROR, stream);
				/* flush copy of sequence */
      fs_give ((void **) &aseq.text);
    }
  }
}

/* IMAP ping mailbox
 * Accepts: MAIL stream
 * Returns: T if stream still alive, else NIL
 */

long imap_ping (MAILSTREAM *stream)
{
  return (((IMAPLOCAL *) stream->local)->tcpstream &&	/* send "NOOP" */
	  imap_OK (stream,imap_send (stream,"NOOP",NIL))) ? T : NIL;
}


/* IMAP check mailbox
 * Accepts: MAIL stream
 */

void imap_check (MAILSTREAM *stream)
{
				/* send "CHECK" */
  IMAPPARSEDREPLY *reply = imap_send (stream,"CHECK",NIL);
  mm_log (reply->text,imap_OK (stream,reply) ? (long) NIL : ERROR, stream);
}


/* IMAP expunge mailbox
 * Accepts: MAIL stream
 */

void imap_expunge (MAILSTREAM *stream)
{
				/* send "EXPUNGE" */
  IMAPPARSEDREPLY *reply = imap_send (stream,"EXPUNGE",NIL);
  mm_log (reply->text,imap_OK (stream,reply) ? (long) NIL : ERROR, stream);
}

/* IMAP copy message(s)
 * Accepts: MAIL stream
 *	    sequence
 *	    destination mailbox
 *	    option flags
 * Returns: T if successful else NIL
 */

long imap_copy (MAILSTREAM *stream,char *sequence,char *mailbox,long flags)
{
  char *cmd = (((IMAPLOCAL *) stream->local)->imap4 && (flags & CP_UID)) ? "UID COPY" : "COPY";
  IMAPPARSEDREPLY *reply;
  IMAPARG *args[3],aseq,ambx;
  aseq.type = ATOM; aseq.text = (void *) sequence;
  ambx.type = ASTRING; ambx.text = (void *) mailbox;
  args[0] = &aseq; args[1] = &ambx; args[2] = NIL;
				/* send "COPY sequence mailbox" */
  if (!imap_OK (stream,reply = imap_send (stream,cmd,args))) {
    mm_log (reply->text,ERROR, stream);
    return NIL;
  }
  if (flags & CP_MOVE)		/* delete the messages if the user said to */
    imap_set1flag (stream,sequence,"\\Deleted",(flags & CP_UID) ? ST_UID : NIL);
  return T;
}

/*
 * IMAP append message string
 * Accepts: mail stream
 *	    destination mailbox
 *	    stringstruct of message to append
 * Returns: _B_TRUE on success, _B_FALSE on failure
 */
long
imap_append(MAILSTREAM	* stream,
	    char 	* mailbox,
	    char 	* flags,
	    char 	* date,
	    char	* xUnixFrom,
	    STRING 	* msg)
{
  char			tmp[MAILTMPLEN];
  boolean_t		ret = _B_FALSE;
  IMAPPARSEDREPLY *	reply = NULL;
  IMAPARG 	  *	args[6];
  IMAPARG		ambx;
  IMAPARG		aflg;
  IMAPARG		adat;
  IMAPARG		amsg;

  STRING		newString;
  char	  	  *	newMsg = NULL;

  /* make sure useful stream given */
  MAILSTREAM 	  *	st = NULL;

  if (stream
      && (stream->stream_status & S_OPENED)	
      && ((IMAPLOCAL *) stream->local)	
      && ((IMAPLOCAL *) stream->local)->tcpstream) {
    st = stream;  /* use the old one */
  } else {  /* Create a new one. */
    st = mail_stream_create(stream->userdata, NIL, NIL, NIL, NIL);
    st = mail_open(st,mailbox, OP_HALFOPEN|OP_SILENT);
  }

  if (st && (st->stream_status & S_OPENED)) {	

    /* do the operation if valid stream */
    if (mail_valid_net(mailbox, &imapdriver, NIL, tmp)) {
      ambx.type = ASTRING; ambx.text = (void *) tmp;

      aflg.type = FLAGS;
      aflg.text = (void *) flags;

      adat.type = ASTRING;
      adat.text = (void *) date;

      if (xUnixFrom != NULL) {
	const char *	xUnixFromHeader = "X-Unix-From";

	if (strstr(msg->curpos, xUnixFromHeader) == NULL) {
	  int		len = SIZE(msg)
	    + strlen(xUnixFromHeader)
	    + strlen(xUnixFrom)
	    + 5;	/* 5 == space, ':', \r, \n, \0 */
	  
	  newMsg = malloc(len); 

	  sprintf(newMsg, "%s: %s\r\n", xUnixFromHeader, xUnixFrom);
	  strncat(newMsg, msg->curpos, SIZE(msg));

	  INIT(&newString, mail_string, newMsg, strlen(newMsg));

	  msg = &newString;
	}
      }

      amsg.type = LITERAL;
      amsg.text = (void *) msg;

      if (flags || date) {	/* IMAP4 form? */
	int 	i = 0;

	args[i++] = &ambx;
	if (flags) {
	  args[i++] = &aflg;
	}
	if (date != NULL) {
	  args[i++] = &adat;
	}

	args[i++] = &amsg;
	args[i++] = NIL;

	if (strcasecmp((reply = imap_send(st, "APPEND", args))->key,
		       "OK") == 0)
	  ret = _B_TRUE;
      }

      /* try IMAP2bis form if IMAP4 form fails */
      if (!(ret || (reply && (strcasecmp(reply->key, "BAD") != 0)))) {
	args[0] = &ambx;
	args[1] = &amsg;
	args[2] = NULL;
	if (imap_OK(st, reply = imap_send(st, "APPEND", args))) {
	  ret = _B_TRUE;
	}
      }
      if (!ret) {
	mm_log(reply->text, ERROR, stream);
      }
    }
  } else {
    mm_log("Can't access server for append", ERROR, stream);
  }

  /* toss out temporary stream */
  /* for both cases. */
  if (st != stream) {
    mail_close(st);
  }
  if (newMsg != NULL) {
    free(newMsg);
  }
  return(ret);			/* return */
}

/* IMAP garbage collect stream
 * Accepts: Mail stream
 *	    garbage collection flags
 */

void imap_gc (MAILSTREAM *stream,long gcflags)
{
  unsigned long i;
  MESSAGECACHE *elt;
  mailcache_t mc = (mailcache_t)mail_parameters(stream,GET_CACHE,NIL);
		/* OK to pass just a stream */
				/* make sure the cache is large enough */
  (*mc) (stream,stream->nmsgs,CH_SIZE);
  if (gcflags & GC_TEXTS) {	/* garbage collect texts? */
    for (i = 1; i <= stream->nmsgs; ++i)
      if (elt = (MESSAGECACHE *) (*mc) (stream,i,CH_ELT)) {
	if (elt->data1) fs_give ((void **) &elt->data1);
	if (elt->data2) fs_give ((void **) &elt->data2);
	if (elt->filter1) fs_give ((void **) &elt->filter1);
	if (elt->lines) mail_free_stringlist(&elt->lines);
	if (!stream->scache) imap_gc_body (mail_lelt (stream,i)->body);
      }
    imap_gc_body (stream->body);/* free texts from short cache body */
  }
				/* gc cache if requested and unlocked */
  if (gcflags & GC_ELT) for (i = 1; i <= stream->nmsgs; ++i)
    if ((elt = (MESSAGECACHE *) (*mc) (stream,i,CH_ELT)) &&
	(elt->lockcount == 1)) (*mc) (stream,i,CH_FREE);
}

/* IMAP garbage collect body texts
 * Accepts: body to GC
 */

void imap_gc_body (BODY *body)
{
  PART *part;
  if (body) switch (body->type) {
  case TYPETEXT:		/* unformatted text */
    if (body->contents.text) fs_give ((void **) &body->contents.text);
    break;
  case TYPEMULTIPART:		/* multiple part */
    if (part = body->contents.part) do imap_gc_body (&part->body);
    while (part = part->next);
    break;
  case TYPEMESSAGE:		/* encapsulated message */
    imap_gc_body (body->contents.msg.body);
    if (body->contents.msg.hdr)
      fs_give ((void **) &body->contents.msg.hdr);
    if (body->contents.msg.text)
      fs_give ((void **) &body->contents.msg.text);
    break;
  case TYPEAPPLICATION:		/* application data */
  case TYPEAUDIO:		/* audio */
  case TYPEIMAGE:		/* static image */
  case TYPEVIDEO:		/* video */
    if (body->contents.binary) fs_give (&body->contents.binary);
    break;
  default:
    break;
  }
}

/* Internal routines */

/* IMAP send command
 * Accepts: MAIL stream
 *	    command
 *	    argument list
 * Returns: parsed reply
 */

IMAPPARSEDREPLY *imap_send (MAILSTREAM *stream,char *cmd,IMAPARG *args[])
{
  IMAPPARSEDREPLY *reply;
  IMAPARG *arg;
  STRINGLIST *list;
  char c,*s,*t,tag[16];

  if (!((IMAPLOCAL *) stream->local)->tcpstream) return imap_fake (stream,tag,"No-op dead stream");
  mail_lock (stream);		/* lock up the stream */
  				/* gensym a new tag */
  sprintf (tag,"A%05ld",stream->gensym++);
				/* write tag */
  for (s = ((IMAPLOCAL *) stream->local)->tmp,t = tag; *t; *s++ = *t++);
  *s++ = ' ';			/* delimit and write command */
  for (t = cmd; *t; *s++ = *t++);
  if (args) while (arg = *args++) {
    *s++ = ' ';			/* delimit argument with space */
    switch (arg->type) {
    case ATOM:			/* atom */
      for (t = (char *) arg->text; *t; *s++ = *t++);
      break;
    case NUMBER:		/* number */
      sprintf (s,"%lu",(unsigned long) arg->text);
      s += strlen (s);
      break;
    case FLAGS:			/* flag list as a single string */
      if (*(t = (char *) arg->text) != '(') {
	*s++ = '(';		/* wrap parens around string */
	while (*t) *s++ = *t++;
	*s++ = ')';		/* wrap parens around string */
      }
      else while (*t) *s++ = *t++;
      break;

    case ASTRING:		/* atom or string, must be literal? */
      if (reply = imap_send_astring (stream,tag,&s,arg->text,
				     (unsigned long) strlen (arg->text),
				     _B_FALSE))
	return reply;
      break;
    case LITERAL:		/* literal, as a stringstruct */
      if (reply = imap_send_literal (stream,tag,&s,arg->text)) return reply;
      break;
    case LIST:			/* list of strings */
      list = (STRINGLIST *) arg->text;
      c = '(';			/* open paren */
      do {			/* for each list item */
	*s++ = c;		/* write prefix character */
	if (reply = imap_send_astring (stream,tag,&s,list->text,list->size,
				       _B_FALSE))
	  return reply;
	c = ' ';		/* prefix character for subsequent strings */
      }
      while (list = list->next);
      *s++ = ')';		/* close list */
      break;
    case SEARCHPROGRAM:		/* search program */
      if (reply = imap_send_spgm (stream,tag,&s,arg->text)) return reply;
      break;
    case BODYTEXT:		/* body section */
      for (t = "BODY["; *t; *s++ = *t++);
      for (t = (char *) arg->text; *t; *s++ = *t++);
      *s++ = ']';		/* close section */
      break;
    case BODYPEEK:		/* body section */
      for (t = "BODY.PEEK["; *t; *s++ = *t++);
      for (t = (char *) arg->text; *t; *s++ = *t++);
      *s++ = ']';		/* close section */
      break;
    case SEQUENCE:		/* sequence */
      while (t = (strlen ((char *) arg->text) > MAXSEQUENCE) ?
	     strchr (((char *) arg->text) + MAXSEQUENCE - 20,',') : NIL) {
	*t++ = '\0';		/* tie off sequence */
				/* recurse to do part */
	imap_send (stream,cmd,args);
	arg->text = (void *) t;	/* now do rest of data */
      }
    case LISTMAILBOX:		/* astring with wildards */
      if (reply = imap_send_astring (stream,tag,&s,arg->text,
				     (unsigned long) strlen (arg->text),
				     _B_TRUE))
	return reply;
      break;
    default:
      fatal ("Unknown argument type in imap_send()!", stream);
    }
  }
				/* send the command */
  reply = imap_sout (stream,tag,((IMAPLOCAL *) stream->local)->tmp,&s);
  mail_unlock (stream);		/* unlock stream */
  return reply;
}

/*
 * IMAP send atom-string
 * Accepts: MAIL stream
 *	    reply tag
 *	    pointer to current position pointer of output bigbuf
 *	    atom-string to output
 *	    string length
 *	    flag if list_wildcards allowed
 * Returns: error reply or NIL if success
 */
IMAPPARSEDREPLY *
imap_send_astring(MAILSTREAM 	*  stream,
		  char 		*  tag,
		  char 		** s,
		  char 		*  text,
		  unsigned long    size,
		  boolean_t	   wildok)
{
  unsigned long 	textOffset;
  STRING 		st;
  /* default to not quoted unless empty */
  boolean_t		quoted = size ? _B_FALSE : _B_TRUE;

  for (textOffset = 0; textOffset < size; textOffset++) {
    switch (text[textOffset]) {
    case '\0':			/* not a CHAR */
    case '\n': case '\r':	/* not a TEXT-CHAR */
    case '"': case '\\':	/* quoted-specials (IMAP2 required this) */
      INIT(&st, mail_string, (void *)text, size);
      return(imap_send_literal(stream, tag, s, &st));

  default:			/* all other characters */
    if (text[textOffset] > ' ') {
      break;	/* break if not a CTL */
    }
    /*FALLTHRU*/

    case '*':
    case '%':		/* list_wildcards */
      if (wildok) {
	break;		/* allowed if doing the wild thing */
      }
    /*FALLTHRU*/
			/* atom_specials */
    case '(':
    case ')':
    case '{':
    case ' ':
#if 0
    case '"':
    case '\\':		/* quoted-specials (could work in IMAP4) */
#endif
      quoted = _B_TRUE;	/* must use quoted string format */
      break;
    }
  }
  if (quoted) {
    *(*s)++ = '"';	/* write open quote */
  }
  while (size--) {
    *(*s)++ = *text++;/* write the characters */
  }
  if (quoted) {
    *(*s)++ = '"';	/* write close quote */
  }
  return(NULL);
}

/*
 * IMAP send literal
 * Accepts: MAIL stream
 *	    reply tag
 *	    pointer to current position pointer of output bigbuf
 *	    literal to output as stringstruct
 * Returns: error reply or NULL if success
 */
IMAPPARSEDREPLY *
imap_send_literal(MAILSTREAM	*  stream,
		  char 		*  tag,
		  char 		** s,
		  STRING 	*  st)
{
  IMAPPARSEDREPLY *	reply;
  unsigned long 	stSize = SIZE(st);

  sprintf(*s, "{%ld}",stSize);	/* write literal count */
  *s += strlen (*s);		/* size of literal count */
				/* send the command */
  reply = imap_sout(stream, tag, ((IMAPLOCAL *)stream->local)->tmp, s);
  if (strcmp(reply->tag, "+") != 0) {/* prompt for more data? */
    mail_unlock(stream);	/* no, give up */
    return(reply);
  }
  while (stSize) {		/* dump the text */
    if (!tcp_sout(((IMAPLOCAL *)stream->local)->tcpstream,
		  st->curpos,
		  st->cursize)) {
      mail_unlock(stream);
      return(imap_fake(stream, tag, "IMAP connection broken (data)"));
    }
    stSize -= st->cursize;	/* note that we wrote out this much */
    st->curpos += (st->cursize - 1);
    st->cursize = 0;
    (*st->dtb->next)(st);	/* advance to next buffer's worth */
  }
  return(NULL);			/* success */
}

/* IMAP send search program
 * Accepts: MAIL stream
 *	    reply tag
 *	    pointer to current position pointer of output bigbuf
 *	    search program to output
 * Returns: error reply or NIL if success
 */

IMAPPARSEDREPLY *imap_send_spgm (MAILSTREAM *stream,char *tag,char **s,
				 SEARCHPGM *pgm)
{
  IMAPPARSEDREPLY *reply;
  SEARCHHEADER *hdr;
  SEARCHPGMLIST *pgl;
  SEARCHOR *pgo;
  char *t = "ALL";
  while (*t) *(*s)++ = *t++;	/* default initial text */
				/* message sequences */
  if (pgm->msgno) imap_send_sset (s,pgm->msgno);
  if (pgm->uid) {		/* UID sequence */
    for (t = " UID"; *t; *(*s)++ = *t++);
    imap_send_sset (s,pgm->msgno);
  }
				/* message sizes */
  if (pgm->larger) {
    sprintf (*s," LARGER %ld",pgm->larger);
    *s += strlen (*s);
  }
  if (pgm->smaller) {
    sprintf (*s," SMALLER %ld",pgm->smaller);
    *s += strlen (*s);
  }
				/* message flags */
  if (pgm->answered) for (t = " ANSWERED"; *t; *(*s)++ = *t++);
  if (pgm->unanswered) for (t =" UNANSWERED"; *t; *(*s)++ = *t++);
  if (pgm->deleted) for (t =" DELETED"; *t; *(*s)++ = *t++);
  if (pgm->undeleted) for (t =" UNDELETED"; *t; *(*s)++ = *t++);
  if (pgm->draft) for (t =" DRAFT"; *t; *(*s)++ = *t++);
  if (pgm->undraft) for (t =" UNDRAFT"; *t; *(*s)++ = *t++);
  if (pgm->flagged) for (t =" FLAGGED"; *t; *(*s)++ = *t++);
  if (pgm->unflagged) for (t =" UNFLAGGED"; *t; *(*s)++ = *t++);
  if (pgm->recent) for (t =" RECENT"; *t; *(*s)++ = *t++);
  if (pgm->old) for (t =" OLD"; *t; *(*s)++ = *t++);
  if (pgm->seen) for (t =" SEEN"; *t; *(*s)++ = *t++);
  if (pgm->unseen) for (t =" UNSEEN"; *t; *(*s)++ = *t++);
  if ((pgm->keyword &&		/* keywords */
       (reply = imap_send_slist (stream,tag,s,"KEYWORD",pgm->keyword))) ||
      (pgm->unkeyword &&
       (reply = imap_send_slist (stream,tag,s,"UNKEYWORD",pgm->unkeyword))))
    return reply;

				/* sent date ranges */
  if (pgm->sentbefore) imap_send_sdate (s,"SENTBEFORE",pgm->sentbefore);
  if (pgm->senton) imap_send_sdate (s,"SENTON",pgm->senton);
  if (pgm->sentsince) imap_send_sdate (s,"SENTSINCE",pgm->sentsince);
				/* internal date ranges */
  if (pgm->before) imap_send_sdate (s,"BEFORE",pgm->before);
  if (pgm->on) imap_send_sdate (s,"ON",pgm->on);
  if (pgm->since) imap_send_sdate (s,"SINCE",pgm->since);
				/* search texts */
  if ((pgm->bcc && (reply = imap_send_slist (stream,tag,s,"BCC",pgm->bcc))) ||
      (pgm->cc && (reply = imap_send_slist (stream,tag,s,"CC",pgm->cc))) ||
      (pgm->from && (reply = imap_send_slist(stream,tag,s,"FROM",pgm->from)))||
      (pgm->to && (reply = imap_send_slist (stream,tag,s,"TO",pgm->to))))
    return reply;
  if ((pgm->subject &&
       (reply = imap_send_slist (stream,tag,s,"SUBJECT",pgm->subject))) ||
      (pgm->body && (reply = imap_send_slist(stream,tag,s,"BODY",pgm->body)))||
      (pgm->text && (reply = imap_send_slist (stream,tag,s,"TEXT",pgm->text))))
    return reply;
  if (hdr = pgm->header) do {
    for (t = " HEADER "; *t; *(*s)++ = *t++);
    for (t = hdr->line; *t; *(*s)++ = *t++);
    *(*s)++ = ' ';
    if (reply = imap_send_astring (stream,tag,s,hdr->text,
				   (unsigned long) strlen (hdr->text),
				   _B_FALSE))
      return reply;
  }
  while (hdr = hdr->next);
  if (pgo = pgm->or) do {
    for (t = " OR ("; *t; *(*s)++ = *t++);
    if (reply = imap_send_spgm (stream,tag,s,pgo->first)) return reply;
    for (t = ") ("; *t; *(*s)++ = *t++);
    if (reply = imap_send_spgm (stream,tag,s,pgo->second)) return reply;
    *(*s)++ = ')';
  }
  while (pgo = pgo->next);
  if (pgl = pgm->not) do {
    for (t = " NOT ("; *t; *(*s)++ = *t++);
    if (reply = imap_send_spgm (stream,tag,s,pgl->pgm)) return reply;
    *(*s)++ = ')';
  }
  while (pgl = pgl->next);
  return NIL;			/* search program written OK */
}


/* IMAP send search set
 * Accepts: output bigbuf
 *	    search set to output
 */
void imap_send_sset (char **s,SEARCHSET *set)
{
  char c = ' ';
  do {				/* run down search set */
    sprintf (*s,set->last ? "%c%ld:%ld" : "%c%ld",c,set->first,set->last);
    *s += strlen (*s);
    c = ',';			/* if there are any more */
  }
  while (set = set->next);
}

/* IMAP send search list
 * Accepts: MAIL stream
 *	    reply tag
 *	    output bigbuf
 *	    search list to output
 * Returns: NIL if success, error reply if error
 */
IMAPPARSEDREPLY *imap_send_slist (MAILSTREAM *stream,char *tag,char **s,
				  char *name,STRINGLIST *list)
{
  char *t;
  IMAPPARSEDREPLY *reply;
  do {
    *(*s)++ = ' ';		/* output name of search list */
    for (t = name; *t; *(*s)++ = *t++);
    *(*s)++ = ' ';
    reply = imap_send_astring (stream,tag,s,list->text,
			       (unsigned long) strlen (list->text),
			       _B_FALSE);
  }
  while (!reply && (list = list->next));
  return reply;
}

/* IMAP send search date
 * Accepts: output bigbuf
 *	    field name
 *	    search date to output
 */

void imap_send_sdate (char **s,char *name,unsigned short date)
{
  sprintf (*s," %s %d-%s-%d",name,date & 0x1f,
	   months[((date >> 5) & 0xf) - 1],BASEYEAR + (date >> 9));
  *s += strlen (*s);
}

/*
 * IMAP send buffered command to sender
 * Accepts: MAIL stream
 *	    reply tag
 *	    string
 *	    pointer to string tail pointer
 * Returns: reply
 */
IMAPPARSEDREPLY *
imap_sout(MAILSTREAM * stream, char * tag, char * base, char ** s)
{
  IMAPPARSEDREPLY *	reply;

  if (stream->debug) {		/* output debugging telemetry */
    **s = '\0';
    mm_dlog(stream, base);
  }

  *(*s)++ = '\r';		/* append CRLF */
  *(*s)++ = '\n';
  **s = '\0';

  if (tcp_sout(((IMAPLOCAL *)stream->local)->tcpstream, base, *s - base)) {
    reply = imap_reply(stream, tag);
  } else {
    reply = imap_fake(stream, tag, "IMAP connection broken (command)");
  }

  *s = base;			/* restart buffer */
  return(reply);
}

/* IMAP get reply
 * Accepts: MAIL stream
 *	    tag to search or NIL if want a greeting
 * Returns: parsed reply, never NIL
 */

IMAPPARSEDREPLY *imap_reply (MAILSTREAM *stream,char *tag)
{
  IMAPPARSEDREPLY *reply;
  while (((IMAPLOCAL *) stream->local)->tcpstream) {	/* parse reply from server */
    if (reply = imap_parse_reply (stream,tcp_getline (((IMAPLOCAL *) stream->local)->tcpstream))) {
				/* continuation ready? */
      if (strcmp(reply->tag,"+") == 0) return reply;
				/* untagged data? */
      else if (strcmp (reply->tag,"*") == 0) {
	imap_parse_unsolicited (stream,reply);
	if (!tag) return reply;	/* return if just wanted greeting */
      }
      else {			/* tagged data */
	if (tag && (strcmp(tag, reply->tag) == 0)) return reply;
				/* report bogon */
	sprintf (((IMAPLOCAL *) stream->local)->tmp,"Unexpected tagged response: %.80s %.80s %.80s",
		 reply->tag,reply->key,reply->text);
	mm_log (((IMAPLOCAL *) stream->local)->tmp,WARN,stream);
      }
    }
  }
  return imap_fake (stream,tag,"IMAP connection broken (server response)");
}

/*
 * IMAP parse reply
 * Accepts: MAIL stream
 *	    text of reply
 * Returns: parsed reply, or NULL if can't parse at least a tag and key
 */
IMAPPARSEDREPLY *
imap_parse_reply(MAILSTREAM * stream,char * text)
{
  IMAPLOCAL	*	imap4 = ((IMAPLOCAL*)stream->local);
  char 		*	lasts;

  if (imap4->reply.line) {
    fs_give((void **)&imap4->reply.line);
  }

  if ((imap4->reply.line = text) == NULL) {

    /* NULL text means the stream died */
    if (imap4->tcpstream != NULL) {
      tcp_close(imap4->tcpstream);
    }
    imap4->tcpstream = NULL;
    return(NULL);
  }

  if (stream->debug) {
    mm_dlog(stream, imap4->reply.line);
  }

  imap4->reply.key = NULL;	/* init fields in case error */
  imap4->reply.text = NULL;

  if ((imap4->reply.tag = strtok_r(imap4->reply.line, " ", &lasts )) == NULL) {
    if (stream->debug) {
      mm_log("IMAP server sent a blank line", WARN, stream);
    }
    return(NULL);
  }

  /* non-continuation replies */
  if (strcmp(imap4->reply.tag, "+") != 0) {

    /* parse key */
    if ((imap4->reply.key = strtok_r(NULL, " ", &lasts )) == NULL) {

      /* determine what is missing */
      sprintf(imap4->tmp, "Missing IMAP reply key: %.80s", imap4->reply.tag);

      /* pass up the barfage */
      mm_log(imap4->tmp, WARN, stream);
      return(NULL);		/* can't parse this text */
    }

    /* make sure key is upper case */
    ucase(imap4->reply.key);

    /* get text as well, allow empty text */
    if ((imap4->reply.text = strtok_r(NULL, "\n", &lasts )) == NULL) {
      imap4->reply.text	= imap4->reply.key + strlen(imap4->reply.key);
    }
  } else {			/* special handling of continuation */
    imap4->reply.key = "BAD";	/* so it barfs if not expecting continuation */
    if ((imap4->reply.text = strtok_r(NULL, "\n", &lasts )) == NULL) {
      imap4->reply.text = "Ready for more command";
    }
  }
  return (&imap4->reply);	/* return parsed reply */
}

/* IMAP fake reply
 * Accepts: MAIL stream
 *	    tag
 *	    text of fake reply
 * Returns: parsed reply
 */

IMAPPARSEDREPLY *imap_fake (MAILSTREAM *stream,char *tag,char *text)
{
  mm_notify (stream,text,BYE);	/* send bye alert */
  if (((IMAPLOCAL *) stream->local)->tcpstream) tcp_close (((IMAPLOCAL *) stream->local)->tcpstream);
  ((IMAPLOCAL *) stream->local)->tcpstream = NIL;	/* farewell, dear TCP stream... */
				/* build fake reply string */
  sprintf (((IMAPLOCAL *) stream->local)->tmp,"%s NO [CLOSED] %s",tag ? tag : "*",text);
				/* parse and return it */
  return imap_parse_reply (stream,cpystr (((IMAPLOCAL *) stream->local)->tmp));
}


/* IMAP check for OK response in tagged reply
 * Accepts: MAIL stream
 *	    parsed reply
 * Returns: T if OK else NIL
 */

long
imap_OK(MAILSTREAM * stream,IMAPPARSEDREPLY * reply)
{
				/* OK - operation succeeded */
  if ((strcasecmp(reply->key, "OK") == 0)
      || ((strcmp(reply->tag, "*") == 0)
	  && (strcasecmp(reply->key, "PREAUTH") == 0))) {
    return(_B_TRUE);
  } else if (strcasecmp(reply->key, "NO") == 0) {/* NO - operation failed */

    /* BAD - operation rejected */
    if (strcasecmp(reply->key, "BAD") == 0) {
      sprintf(((IMAPLOCAL *) stream->local)->tmp, "IMAP error: %.80s", reply->text);
    } else {
      sprintf(((IMAPLOCAL *) stream->local)->tmp, 
	      "Unexpected IMAP response: %.80s %.80s",
	      reply->key,
	      reply->text);
    }
    mm_log(((IMAPLOCAL *) stream->local)->tmp,WARN,stream);	/* log the sucker */
  }
  return(_B_FALSE);
}

/*
 * IMAP parse and act upon unsolicited reply
 * Accepts: MAIL stream
 *	    parsed reply
 */
void imap_parse_unsolicited (MAILSTREAM *stream,IMAPPARSEDREPLY *reply)
{
  unsigned long 		i = 0;
  unsigned long 		msgno;
  char 			*	s;
  char			*	t;
  char 			*	keyptr;
  char			*	txtptr;
  char 			*	lasts;

  /* see if key is a number */
  msgno = strtoul(reply->key, &s, 10);

  /* Imap4 Sun: We may receive a sequence here */
  if (*s && *s != ',' && *s != ':') {			/* if non-numeric */
    if (strcasecmp(reply->key, "FLAGS") == 0) {

      /* flush old flagstring and flags if any */
      if (stream->flagstring) {
	fs_give((void **) &stream->flagstring);
      }

      /* remember this new one */
      stream->flagstring = reply->line;
      reply->line = NIL;

      /* add flags */
      if (s = (char *) strtok_r(reply->text+1, " )", &lasts )) {
	do {
	  if (*s != '\\') {
	    stream->user_flags[i++] = s;
	  }
	} while (s = (char *) strtok_r (NIL," )", &lasts ));
      }
      while (i < NUSERFLAGS) {
	stream->user_flags[i++] = NIL;
      }

    } else if (strcasecmp(reply->key, "SEARCH") == 0) {

      /* only do something if have text */
      if (reply->text
	  && (t = (char *) strtok_r(reply->text, " ",  &lasts))) {
	do {
	  i = atol(t);
	  if (!((IMAPLOCAL *) stream->local)->uidsearch) {
	    mail_elt(stream, i)->searched = _B_TRUE;
	  }
	  mm_searched(stream,i);
	} while (t = (char *) strtok_r(NIL, " ", &lasts ));
      }

    } else if (strcasecmp(reply->key, "STATUS") == 0) {

      MAILSTATUS 	status;
      char 	*	txt;

      /*
       * This STATUS may be for this mailbox to solicit a
       * checksum and uid-validity for later disconnected use.
       * See ((IMAPLOCAL *) stream->local)->csstatus = T in map_checksum(...)
       */
      switch (*reply->text) {	/* mailbox is an astring */
      case '"':			/* quoted string? */
      case '{':			/* literal? */
	txt = reply->text;	/* status data is in reply */
	t = imap_parse_string(stream, &txt, reply, NIL, NIL);
	break;

      default:			/* must be atom */
	t = reply->text;
	if (txt = strchr(t, ' ')) {
	  *txt++ = '\0';
	}
	break;
      }

      if (t
	  && txt
	  && (*txt++ == '(')
	  && (s = strchr(txt, ')'))
	  && (s - txt)
	  && !s[1]) {

	*s = '\0';		/* tie off status data */
	
	/* initialize data block */
	status.flags = status.messages = status.recent = status.unseen =
	status.uidnext = status.uidvalidity = status.checksum = 0;
	ucase(txt);		/* do case-independent match */
	while (*txt && (s = strchr(txt, ' '))) {
	  *s++ = '\0';		/* tie off status attribute name */

	  /*
	   * Imap4 Sun version: Checksum is actually a short
           * of 5 chars followed by an unsigned long:
           * ssssslll...l
	   */
	  if (strcasecmp(txt, "CHECKSUM") == 0) {
	    char 	*	cs = ((IMAPLOCAL *) stream->local)->tmp;
	    int 		len = strlen(s);
				
	    /* maybe broken response */
	    int 		cslen = (len < 5 ? len : 5);

	    strncpy(cs, s, cslen);
	    cs[cslen] = '\0';
	    i = strtoul(cs, NIL, 10);
	    status.checksum = (unsigned short)i;
	    s += cslen;			/* skip the checksum bytes */
	    status.checksum_bytes = strtoul(s, &s, 10);

	    /*
	     * we stash this on the stream too for disconnected
	     * mode. We need to stay current
	     */
	    stream->checksum = status.checksum;
	    stream->mbox_size = status.checksum_bytes;

	  } else {
	    i = strtoul(s, &s, 10);/* get attribute value */
	    if (strcasecmp(txt, "MESSAGES") == 0) {
	      status.flags |= SA_MESSAGES;
	      status.messages = i;

	    } else if (strcasecmp(txt, "RECENT") == 0) {
	      status.flags |= SA_RECENT;
	      status.recent = i;

	    } else if (strcasecmp(txt, "UNSEEN") == 0) {
	      status.flags |= SA_UNSEEN;
	      status.unseen = i;

	    } else if (strcasecmp(txt, "UID-NEXT") == 0) {
	      status.flags |= SA_UIDNEXT;
	      stream->uid_last = status.uidnext = i;

	    } else if (strcasecmp(txt, "UID-VALIDITY") == 0) {
	      status.flags |= SA_UIDVALIDITY;
	      status.uidvalidity = i;
	      stream->uid_validity = i;
	    }
	  }		/* next attribute */
	  txt = (*s == ' ') ? s + 1 : s;
	}
	sprintf(((IMAPLOCAL *) stream->local)->tmp, "{%s}%s", imap_host (stream), t);

	/* pass status to main program */
	mm_status(stream, ((IMAPLOCAL *) stream->local)->tmp, &status);
      }
    } else if (strcasecmp(reply->key, "CHECKSUM") == 0){/* Begin "Sun Imap4" */
      imap_parse_checksum (stream, reply);

    } else if (strcasecmp(reply->key, "VALIDCHECKSUM") == 0) {
      imap_parse_validchecksum(stream, reply->text);

      /* Done "Sun Imap4" */
    } else if (((strcasecmp(reply->key, "LIST") == 0)
		|| (strcasecmp(reply->key, "LSUB") == 0))

	       && ((*reply->text == '(') && (s = strchr(reply->text, ')'))
		   && (s[1] == ' ')
		   && (((s[2] == '"') && (s[4] == '"'))
		       || (strncasecmp(&s[2], "NIL", 3) == 0))
		   && (s[5] == ' '))) {

      char 		delimiter = '\0';

      *s = '\0';		/* tie off attribute list */

      /* parse attribute list */
      if (t = (char *) strtok_r(reply->text+1, " ", &lasts )) {
	do {
	  if (strcasecmp(ucase(t), "\\NOINFERIORS") == 0) {
	    i |= LATT_NOINFERIORS;

	  } else if (strcasecmp(t, "\\NOSELECT") == 0) {
	    i |= LATT_NOSELECT;

	  } else if (strcasecmp(t, "\\MARKED") == 0) {
	    i |= LATT_MARKED;

	  } else if (strcasecmp(t, "\\UNMARKED") == 0) {
	    i |= LATT_UNMARKED;
	  }
	  /* ignore extension flags */
	} while (t = (char *) strtok_r(NIL, " ", &lasts ));
      }

      if (s[2] == '"') {
	delimiter = s[3];
      }

      t = ((*(s += 6) == '"') || (*s == '{')) ?
	imap_parse_string(stream, &s, reply, NIL, NIL) : s;

      if (((IMAPLOCAL *) stream->local)->prefix) {	/* need to prepend a prefix? */
	sprintf(((IMAPLOCAL *) stream->local)->tmp, "%s%s", ((IMAPLOCAL *) stream->local)->prefix, t);
	t = ((IMAPLOCAL *) stream->local)->tmp;		/* do so then */
      }
      if (reply->key[1] == 'S') {
	mm_lsub(stream, delimiter, t, i);
      } else {
	mm_list(stream,delimiter, t, i);
      }

    } else if (strcasecmp(reply->key, "MAILBOX") == 0) {
      if (((IMAPLOCAL *) stream->local)->prefix) {
	sprintf(t = ((IMAPLOCAL *) stream->local)->tmp, "%s%s", ((IMAPLOCAL *) stream->local)->prefix, reply->text);
      } else {
	t = reply->text;
      }
      mm_list(stream, NIL, t, LATT_IMAP2BIS);

    } else if ((strcasecmp(reply->key, "OK") == 0)
	       || (strcasecmp(reply->key,"PREAUTH") == 0)) {

      if ((*reply->text == '[')
	  && (t = strchr(s = reply->text + 1,']'))
	  && ((i = t - s) < IMAPTMPLEN)) {

	/* get text code */
	strncpy(((IMAPLOCAL *) stream->local)->tmp, s, (size_t) i);
	((IMAPLOCAL *) stream->local)->tmp[i] = '\0';	/* tie off text */

	if (strcmp(ucase(((IMAPLOCAL *) stream->local)->tmp), "READ-ONLY") == 0) {
	  stream->rdonly = _B_TRUE;

	} else if (strcmp(((IMAPLOCAL *) stream->local)->tmp, "READ-WRITE") == 0) {
	  stream->rdonly = _B_FALSE;

	} else if (strncmp(((IMAPLOCAL *) stream->local)->tmp, "UIDVALIDITY ", 12) == 0) {
	  stream->uid_validity = strtoul (((IMAPLOCAL *) stream->local)->tmp+12,NIL,10);
	  return;

	} else if (strncmp(((IMAPLOCAL *) stream->local)->tmp, "PERMANENTFLAGS (", 16) == 0) {
	  if (((IMAPLOCAL *) stream->local)->tmp[i-1] == ')') {
	    ((IMAPLOCAL *) stream->local)->tmp[i-1] = '\0';
	  }
	  stream->perm_seen = stream->perm_deleted = stream->perm_answered =
	    stream->perm_draft = stream->kwd_create = NIL;
	  stream->perm_user_flags = NIL;

	  if (s = strtok_r(((IMAPLOCAL *) stream->local)->tmp+16, " ", &lasts )) {
	    do {
	      if (strcmp(s, "\\SEEN") == 0) {
		stream->perm_seen = _B_TRUE;

	      } else if (strcmp(s, "\\DELETED") ==0) {
		stream->perm_deleted = _B_TRUE;

	      } else if (strcmp(s, "\\FLAGGED") == 0) {
		stream->perm_flagged = _B_TRUE;

	      } else if (strcmp(s, "\\ANSWERED") == 0) {
		stream->perm_answered = _B_TRUE;

	      } else if (strcmp(s, "\\DRAFT") == 0) {
		stream->perm_draft = _B_TRUE;

	      } else if (strcmp(s, "\\*") == 0) {
		stream->kwd_create = _B_TRUE;

	      } else {
		stream->perm_user_flags |= imap_parse_user_flag(stream, s);
	      }
	    } while (s = strtok_r(NIL," ", &lasts ));
	  }
	  return;
	}
      }
      mm_notify (stream,reply->text,(long) NIL);

    } else if (strcasecmp(reply->key, "NO") == 0) {
      if (!stream->silent) {
	mm_notify(stream, reply->text, WARN);
      }

    } else if (strcasecmp(reply->key, "BYE") == 0) {
      if (!stream->silent) {
	mm_notify (stream, reply->text, BYE);
      }

    } else if (strcasecmp(reply->key, "BAD") == 0) {
      mm_notify (stream,reply->text,ERROR);

    } else if (strcasecmp(reply->key, "CAPABILITY") == 0) {

      /* only do something if have text */
      if (reply->text
	  && (t = (char *) strtok_r(reply->text," ", &lasts ))) {
	do {
	  if (strcasecmp(t, "IMAP4") == 0) {
	    ((IMAPLOCAL *) stream->local)->imap4 = _B_TRUE;

	  } else if (strcasecmp(t, "IMAP4REV1") == 0) {
	    ((IMAPLOCAL *) stream->local)->imap4rev1 = _B_TRUE;
	    ((IMAPLOCAL *) stream->local)->use_status = _B_TRUE;

	  } else if (strcasecmp(t, "STATUS") == 0 ) {
	    ((IMAPLOCAL *) stream->local)->use_status = _B_TRUE;

	  } else if (strcasecmp(t, "SCAN") == 0) {
	    ((IMAPLOCAL *) stream->local)->use_scan = _B_TRUE;

	  } else if ((strncasecmp(t, "AUTH-", 5) == 0) || /* imap4 */
		     (strncasecmp(t, "AUTH=", 5) == 0)) {  /* imap4rev1 */

	    AUTHENTICATOR *	auth;

	    for (i = 0, auth = authenticators
		   ; auth
		   && (i < MAXAUTHENTICATORS)
		   && strcmp(t+5, auth->name)
		   ; i++, auth = auth->next);
	    if (auth && (i < MAXAUTHENTICATORS)) {
	      ((IMAPLOCAL *) stream->local)->use_auth |= 1 << i;
	    }
	  } else {
	    /* SUN IMAP4 - set sunversion if there. */
	    if ((strcasecmp(t, "SUNVERSION") == 0)
		|| (strcasecmp(t, "IMAP4SUNVERSION") == 0)) {
	      ((IMAPLOCAL *) stream->local)->sunvcap = _B_TRUE;
	    }
	    /* ignore other capabilities for now */
	  }
	} while (t = (char *) strtok_r (NIL," ", &lasts ));
      } else {
	sprintf(((IMAPLOCAL *) stream->local)->tmp,
		"Unexpected unsolicited message: %.80s",
		reply->key);
	mm_log(((IMAPLOCAL *) stream->local)->tmp, WARN, stream);
      }
    }
  } else {			/* if numeric, a keyword follows */
    
    /* deposit null at end of keyword */
    keyptr = ucase((char *)strtok_r(reply->text, " ", &lasts ));
	
    /* and locate the text after it */
    txtptr = (char *)strtok_r(NIL, "\n", &lasts);

    /* now take the action */
    /* change in size of mailbox */
    if (strcasecmp(keyptr, "EXISTS") == 0) {
      mail_exists (stream,msgno);

    } else if (strcasecmp(keyptr, "RECENT") == 0) {
      mail_recent (stream,msgno);

    } else if (strcasecmp(keyptr, "EXPUNGE") == 0) {
      imap_expunged (stream,msgno);

    } else if (strcasecmp(keyptr, "FETCH") == 0) {
      imap_parse_data (stream,msgno,txtptr,reply);

    } else if (strcasecmp(keyptr, "STORE") == 0) {
      /* obsolete alias for FETCH */
      imap_parse_data (stream,msgno,txtptr,reply);
    } else  if ((strcasecmp(keyptr, "+1FLAG") == 0)
		|| (strcasecmp(keyptr, "-1FLAG") == 0)) {

      /* Imap4 sun version: Imap2 reply style */
      /* reply->key is a sequence, txtptr flag */
      imap2_process_1flag(stream, keyptr[0], reply->key, txtptr);

    } else if (strcasecmp(keyptr, "COPY") == 0) {
      /* obsolete response to COPY  */
      sprintf(((IMAPLOCAL *) stream->local)->tmp,
	      "Unknown message data: %ld %.80s",
	      msgno, keyptr);
      mm_log(((IMAPLOCAL *) stream->local)->tmp,WARN,stream);
    }				
  }
}

/* IMAP message has been expunged
 * Accepts: MAIL stream
 *	    message number
 *
 * Calls external "mail_expunged" function to notify main program
 */

void imap_expunged (MAILSTREAM *stream,unsigned long msgno)
{
  mailcache_t mc = (mailcache_t) mail_parameters (stream,GET_CACHE,NIL);
  MESSAGECACHE *elt = (MESSAGECACHE *) (*mc) (stream,msgno,CH_ELT);
  if (elt) {
    if (elt->data1) fs_give ((void **) &elt->data1);
    if (elt->data2) fs_give ((void **) &elt->data2);
    if (elt->filter1) fs_give ((void **) &elt->filter1);
    if (elt->lines) mail_free_stringlist(&elt->lines);
  }
  mail_expunged (stream,msgno);	/* notify upper level */
}


/* IMAP parse data
 * Accepts: MAIL stream
 *	    message #
 *	    text to parse
 *	    parsed reply
 *
 *  This code should probably be made a bit more paranoid about malformed
 * S-expressions.
 */

void imap_parse_data (MAILSTREAM *stream,unsigned long msgno,char *text,
		      IMAPPARSEDREPLY *reply)
{
  char *prop;
  char *lasts;

  MESSAGECACHE *elt = mail_elt (stream,msgno);
  /* SUN Imap4 - enable urgent processing */
  if (stream->mail_send_urgent) {
    stream->mail_send_urgent = NIL;
    stream->imap_send_urgent = _B_TRUE;
    os_clear_send_urgent(((IMAPLOCAL *) stream->local)->tcpstream);
  }   
    
  ++text;			/* skip past open parenthesis */
				/* parse Lisp-form property list */
  while (prop = (char *) strtok_r (text," )", &lasts )) {
				/* point at value */
    text = (char *) strtok_r (NIL,"\n", &lasts );
				/* parse the property and its value */
    imap_parse_prop (stream,elt,ucase (prop),&text,reply);
  }
  /* SUN imap4: Disable urgent processing */
  stream->imap_send_urgent = NIL;  
}

/* IMAP parse property
 * Accepts: MAIL stream
 *	    cache item
 *	    property name
 *	    property value text pointer
 *	    parsed reply
 */

void
imap_parse_prop(MAILSTREAM 	*  stream,
		MESSAGECACHE 	*  elt,
		char 		*  prop,
		char 		** txtptr,
		IMAPPARSEDREPLY *  reply)
{
  char 		*  s;
  ENVELOPE 	** env;
  BODY 		** body;
  SHORTINFO 	* sinfo;

  if (strcasecmp(prop, "ENVELOPE") == 0) {
    if (stream->scache || ((IMAPLOCAL *) stream->local)->scache) {/* short cache, flush old stuff */
      mail_free_body(&stream->body);
      stream->msgno =elt->msgno;/* set new current message number */
      env = &stream->env;	/* get pointer to envelope */
    } else {
      env = &mail_lelt(stream, elt->msgno)->env;
    }
    imap_parse_envelope(stream, env, txtptr, reply);
  } else if (strcasecmp(prop, "SHORTINFO") == 0) {/* SUn Imap4 */
    if (stream->scache || ((IMAPLOCAL *) stream->local)->scache) {/* short cache, flush old stuff */
      sinfo = &stream->sinfo;
      mail_free_fshort_data(sinfo);
      stream->msgno = elt->msgno;/* set new current message number */
    } else {
      sinfo = &mail_lelt(stream, elt->msgno)->sinfo;
    }
    imap_parse_shortinfo(stream, sinfo, txtptr, reply);

    /* If we have the date, then stash it in the cache elt too */
    if (sinfo->date) { 
      strcpy(((IMAPLOCAL *) stream->local)->tmp, sinfo->date);
      mail_parse_date(elt, ((IMAPLOCAL *) stream->local)->tmp);
    }

  } else if (strcasecmp(prop,"FLAGS") == 0) {
    imap_parse_flags (stream,elt,txtptr);
  }  else if ((strcasecmp(prop, "+1FLAG") == 0)
	      || (strcasecmp (prop, "-1FLAG") == 0)) {
    /* Sun Imap4: FETCH ([UID <seq>] +1flag (flag)). We do +1flag part here */

    /* reply->key is a sequence, txtptr flag */
    imap4_process_1flag(stream, prop[0], reply->key, txtptr);

  } else if (strcasecmp(prop, "INTERNALDATE") == 0) {
    if (s = imap_parse_string(stream, txtptr, reply, (long)NIL, NIL)) {
      if (!mail_parse_date(elt,s)) {
	sprintf(((IMAPLOCAL *) stream->local)->tmp, "Bogus date: %.80s", s);
	mm_log(((IMAPLOCAL *) stream->local)->tmp, WARN, stream);
      }
      fs_give((void **)&s);
    }
  }  else if (strcasecmp (prop,"UID") == 0) {
    /* Sun IMap4: <uid-sequence> is possible here */

    char      	* fin;
    unsigned long uid = strtoul(*txtptr, &fin, 10);

    if (*fin && (*fin == ',' || *fin == ':')) {
      /* reply->key is MSG SEQUENCE which is 1-1 with UID sequence */
      imap_parse_uidsequence (stream, reply->key, txtptr);

    }  else {
      /*
       * If syncronizing the cache, then we CANNOT store this
       * uid because it may not YET belong to the element. So,
       * we need to FIND the element to which it belongs,
       * and we set this latter elt->sync_msgno to elt->msgno
       */
      if (((IMAPLOCAL *) stream->local)->sync_cache) {
	imap_set_uidelt(stream, uid, elt->msgno);
      } else {
	elt->uid = uid;

	/* Maintaint max_uid  */
	if (elt->uid > stream->uid_last)
	  stream->uid_last = elt->uid;
      }
      *txtptr = fin;
    }
  } else if (strcasecmp(prop, "RFC822.HEADER") == 0) {
    if (((IMAPLOCAL *) stream->local)->hdr_lines) {
      /*
       * Then stash this "most likely" filtered header
       * in the filter space. 
       */
      if (elt->filter1) {
	fs_give((void **) &elt->filter1);
      }
      elt->filter1 = (unsigned long)
	imap_parse_string(stream,txtptr,reply,elt->msgno,
			  (unsigned long *) &elt->filter2);
    } else {
      if (elt->data1) fs_give((void **) &elt->data1);
      elt->data1 = (unsigned long)
	imap_parse_string(stream,txtptr,reply,elt->msgno,
			  (unsigned long *) &elt->data3);
    }

  } else if (strcasecmp(prop, "RFC822.SIZE") == 0) {
    elt->rfc822_size = strtoul (*txtptr,txtptr,10);

  } else if ((strcasecmp(prop, "RFC822.TEXT") == 0)
	     || (strcasecmp(prop, "RFC822") == 0)) {
    if (elt->data2) {
      fs_give((void **) &elt->data2);
    }
    elt->data2 = (unsigned long)
      imap_parse_string(stream,txtptr,reply,elt->msgno,
			(unsigned long *) &elt->data4);

  } else if (strncasecmp(prop, "BODY", 4) == 0) {
    s = cpystr(prop+4);	/* copy segment specifier */
    if (stream->scache || ((IMAPLOCAL *) stream->local)->scache) {/* short cache, flush old stuff */
      if (elt->msgno != stream->msgno) {
		
	/* losing real bad here */
	mail_free_envelope(&stream->env);
	sprintf(((IMAPLOCAL *) stream->local)->tmp, "Body received for %ld when current is %ld",
		 elt->msgno,stream->msgno);
	mm_log (((IMAPLOCAL *) stream->local)->tmp,WARN,stream);
	stream->msgno = elt->msgno;
      }
      body = &stream->body;	/* get pointer to body */

    } else {
      body = &mail_lelt(stream, elt->msgno)->body;
    }
    imap_parse_body(stream, elt->msgno, body, s, txtptr, reply);
    fs_give ((void **) &s);

  } else {
    sprintf(((IMAPLOCAL *) stream->local)->tmp, "Unknown message property: %.80s", prop);
    mm_log(((IMAPLOCAL *) stream->local)->tmp, WARN, stream);
  }
}

/* IMAP parse envelope
 * Accepts: MAIL stream
 *	    pointer to envelope pointer
 *	    current text pointer
 *	    parsed reply
 *
 * Updates text pointer
 */

void imap_parse_envelope (MAILSTREAM *stream,ENVELOPE **env,char **txtptr,
			  IMAPPARSEDREPLY *reply)
{
  char c = *((*txtptr)++);	/* grab first character */
				/* ignore leading spaces */
  while (c == ' ') c = *((*txtptr)++);
				/* free any old envelope */
  if (*env) mail_free_envelope (env);
  switch (c) {			/* dispatch on first character */
  case '(':			/* if envelope S-expression */
    *env = mail_newenvelope ();	/* parse the new envelope */
    (*env)->date = imap_parse_string (stream,txtptr,reply,(long) NIL,NIL);
    (*env)->subject = imap_parse_string (stream,txtptr,reply,(long) NIL,NIL);
    (*env)->from = imap_parse_adrlist (stream,txtptr,reply);
    (*env)->sender = imap_parse_adrlist (stream,txtptr,reply);
    (*env)->reply_to = imap_parse_adrlist (stream,txtptr,reply);
    (*env)->to = imap_parse_adrlist (stream,txtptr,reply);
    (*env)->cc = imap_parse_adrlist (stream,txtptr,reply);
    (*env)->bcc = imap_parse_adrlist (stream,txtptr,reply);
    (*env)->in_reply_to = imap_parse_string(stream,txtptr,reply,(long)NIL,NIL);
    (*env)->message_id = imap_parse_string(stream,txtptr,reply,(long) NIL,NIL);
    if (**txtptr != ')') {
      sprintf (((IMAPLOCAL *) stream->local)->tmp,"Junk at end of envelope: %.80s",*txtptr);
      mm_log (((IMAPLOCAL *) stream->local)->tmp,WARN,stream);
    }
    else ++*txtptr;		/* skip past delimiter */
    break;
  case 'N':			/* if NIL */
  case 'n':
    ++*txtptr;			/* bump past "I" */
    ++*txtptr;			/* bump past "L" */
    break;
  default:
    sprintf (((IMAPLOCAL *) stream->local)->tmp,"Not an envelope: %.80s",*txtptr);
    mm_log (((IMAPLOCAL *) stream->local)->tmp,WARN,stream);
    break;
  }
}

/* IMAP parse address list
 * Accepts: MAIL stream
 *	    current text pointer
 *	    parsed reply
 * Returns: address list, NIL on failure
 *
 * Updates text pointer
 */

ADDRESS *imap_parse_adrlist (MAILSTREAM *stream,char **txtptr,
			     IMAPPARSEDREPLY *reply)
{
  ADDRESS *adr = NIL;
  char c = **txtptr;		/* sniff at first character */
				/* ignore leading spaces */
  while (c == ' ') c = *++*txtptr;
  ++*txtptr;			/* skip past open paren */
  switch (c) {
  case '(':			/* if envelope S-expression */
    adr = imap_parse_address (stream,txtptr,reply);
    if (**txtptr != ')') {
      sprintf (((IMAPLOCAL *) stream->local)->tmp,"Junk at end of address list: %.80s",*txtptr);
      mm_log (((IMAPLOCAL *) stream->local)->tmp,WARN,stream);
    }
    else ++*txtptr;		/* skip past delimiter */
    break;
  case 'N':			/* if NIL */
  case 'n':
    ++*txtptr;			/* bump past "I" */
    ++*txtptr;			/* bump past "L" */
    break;
  default:
    sprintf (((IMAPLOCAL *) stream->local)->tmp,"Not an address: %.80s",*txtptr);
    mm_log (((IMAPLOCAL *) stream->local)->tmp,WARN,stream);
    break;
  }
  return adr;
}

/* IMAP parse address
 * Accepts: MAIL stream
 *	    current text pointer
 *	    parsed reply
 * Returns: address, NIL on failure
 *
 * Updates text pointer
 */

ADDRESS *imap_parse_address (MAILSTREAM *stream,char **txtptr,
			     IMAPPARSEDREPLY *reply)
{
  ADDRESS *adr = NIL;
  ADDRESS *ret = NIL;
  ADDRESS *prev = NIL;
  char c = **txtptr;		/* sniff at first address character */
  switch (c) {
  case '(':			/* if envelope S-expression */
    while (c == '(') {		/* recursion dies on small stack machines */
      ++*txtptr;		/* skip past open paren */
      if (adr) prev = adr;	/* note previous if any */
      adr = mail_newaddr ();	/* instantiate address and parse its fields */
      adr->personal = imap_parse_string (stream,txtptr,reply,(long) NIL,NIL);
      adr->adl = imap_parse_string (stream,txtptr,reply,(long) NIL,NIL);
      adr->mailbox = imap_parse_string (stream,txtptr,reply,(long) NIL,NIL);
      adr->host = imap_parse_string (stream,txtptr,reply,(long) NIL,NIL);
      if (**txtptr != ')') {	/* handle trailing paren */
	sprintf (((IMAPLOCAL *) stream->local)->tmp,"Junk at end of address: %.80s",*txtptr);
	mm_log (((IMAPLOCAL *) stream->local)->tmp,WARN,stream);
      }
      else ++*txtptr;		/* skip past close paren */
      c = **txtptr;		/* set up for while test */
				/* ignore leading spaces in front of next */
      while (c == ' ') c = *++*txtptr;
      if (!ret) ret = adr;	/* if first time note first adr */
				/* if previous link new block to it */
      if (prev) prev->next = adr;
    }
    break;

  case 'N':			/* if NIL */
  case 'n':
    *txtptr += 3;		/* bump past NIL */
    break;
  default:
    sprintf (((IMAPLOCAL *) stream->local)->tmp,"Not an address: %.80s",*txtptr);
    mm_log (((IMAPLOCAL *) stream->local)->tmp,WARN,stream);
    break;
  }
  return ret;
}

/* IMAP parse flags
 * Accepts: current message cache
 *	    current text pointer
 *
 * Updates text pointer
 */

void imap_parse_flags (MAILSTREAM *stream,MESSAGECACHE *elt,char **txtptr)
{
  char *flag;
  char c;
  elt->valid = _B_TRUE;		/* mark have valid flags now */
  elt->user_flags = NIL;	/* zap old flag values */
  elt->seen = elt->deleted = elt->flagged = elt->answered = elt->recent = NIL;
  while (T) {			/* parse list of flags */
    flag = ++*txtptr;		/* point at a flag */
				/* scan for end of flag */
    while (**txtptr != ' ' && **txtptr != ')') ++*txtptr;
    c = **txtptr;		/* save delimiter */
    **txtptr = '\0';		/* tie off flag */
    if (*flag != '\0') {	/* if flag is non-null */
      if (*flag == '\\') {	/* if starts with \ must be sys flag */
	if (strcmp(ucase(flag), "\\SEEN") == 0) {
	  elt->seen = _B_TRUE;

	} else if (strcmp(flag, "\\DELETED") == 0) {
	  elt->deleted = _B_TRUE;

	} else if (strcmp(flag, "\\FLAGGED") == 0) {
	  elt->flagged = _B_TRUE;

	} else if (strcmp(flag, "\\ANSWERED") == 0) {
	  elt->answered = _B_TRUE;

	} else if (strcmp(flag, "\\RECENT")) {
	  elt->recent = _B_TRUE;

	} else if (strcmp(flag, "\\DRAFT")) {
	  elt->draft = _B_TRUE;
	}
      } else {
	/* otherwise user flag */
	elt->user_flags |= imap_parse_user_flag (stream,flag);
      }
    }
    if (c == ')') {
      break;	/* quit if end of list */
    }
  }
  ++*txtptr;			/* bump past delimiter */
  mm_flags(stream, elt->msgno);	/* make sure top level knows */
}


/*
 * IMAP parse user flag
 * Accepts: MAIL stream
 *	    flag name
 * Returns: flag bit position
 */
unsigned long
imap_parse_user_flag(MAILSTREAM * stream,char * flag)
{
  char tmp[MAILTMPLEN];
  long i;
  				/* sniff through all user flags */
  for (i = 0; i < NUSERFLAGS; ++i) {
    if (stream->user_flags[i] &&
	(strcasecmp(flag, ucase(strcpy(tmp, stream->user_flags[i]))) == 0)) {
      return (1 << i);		/* found it! */
    }
  }
  return(unsigned long)0;	/* not found */
}

/* IMAP parse string
 * Accepts: MAIL stream
 *	    current text pointer
 *	    parsed reply
 *	    flag that it may be kept outside of free storage cache
 *	    returned string length
 * Returns: string
 *
 * Updates text pointer
 */

char *imap_parse_string (MAILSTREAM *stream,char **txtptr,
			 IMAPPARSEDREPLY *reply,long special,
			 unsigned long *len)
{
  char *st;
  char *string = NIL;
  unsigned long i,j;
  char c = **txtptr;		/* sniff at first character */
  mailgets_t mg;
  mail_stream_setNIL(stream);
  mg = (mailgets_t) mail_parameters(stream, GET_GETS, NIL);
  mail_stream_unsetNIL(stream);
				/* ignore leading spaces */
  while (c == ' ') c = *++*txtptr;
  st = ++*txtptr;		/* remember start of string */
  switch (c) {
  case '"':			/* if quoted string */
    i = 0;			/* initial byte count */
    while (**txtptr != '"') {	/* search for end of string */
      if (**txtptr == '\\') ++*txtptr;
      ++i;			/* bump count */
      ++*txtptr;		/* bump pointer */
    }
    ++*txtptr;                  /* bump past delimeter */
    string = (char *) fs_get ((size_t) i + 1);
				/* copy the string */
    for (j = 0; j < i; ++j) {
      if (*st == '\\') ++st;
      string[j] = *st++;
    }
    string[j] = '\0';		/* tie off string */
    if (len) *len = i;		/* set return value too */
    break;
  case 'N':			/* if NIL */
  case 'n':
    ++*txtptr;			/* bump past "I" */
    ++*txtptr;			/* bump past "L" */
    if (len) *len = 0;
    break;


  case '{':			/* if literal string */
				/* get size of string */
    i = strtoul (*txtptr, txtptr, 10);

    /* have special routine to slurp string? */
    if (special && mg) {
      string = (*mg)((readfn_t)tcp_getbuffer, ((IMAPLOCAL *) stream->local)->tcpstream, i);
    }  else {			/* must slurp into free storage */
      int read_ok;
      string = (char *) fs_get ((size_t) i + 1);
      *string = '\0';		/* init in case getbuffer fails */
				/* get the literal */
      read_ok = tcp_getbuffer (((IMAPLOCAL *) stream->local)->tcpstream,i,string);
      /* Check out urgent processing */
      if (!read_ok || tcp_flushed_input(((IMAPLOCAL *) stream->local)->tcpstream)) {
	stream->input_flushed = _B_TRUE;
	fs_give((void **)&string);
	string = NIL;
	i = 0;
      }
    }
    fs_give ((void **) &reply->line);
    if (len) *len = i;		/* set return value */
				/* get new reply text line */
    reply->line = tcp_getline (((IMAPLOCAL *) stream->local)->tcpstream);
    if (stream->debug) mm_dlog (stream,reply->line);
    *txtptr = reply->line;	/* set text pointer to point at it */
    break;
  default:
    sprintf (((IMAPLOCAL *) stream->local)->tmp,"Not a string: %c%.80s",c,*txtptr);
    mm_log (((IMAPLOCAL *) stream->local)->tmp,WARN,stream);
    if (len) *len = 0;
    break;
  }
  return string;
}

/*
 * IMAP parse body structure or contents
 * Accepts: MAIL stream
 *	    pointer to body pointer
 *	    pointer to segment
 *	    current text pointer
 *	    parsed reply
 *
 * Updates text pointer, stores body
 */
void
imap_parse_body(MAILSTREAM 	* stream,
		unsigned long 	  msgno,
		BODY 		**body,
		char 		* seg,
		char 		**txtptr,
		IMAPPARSEDREPLY * reply)
{
  char 		*	s;
  unsigned long 	i;
  unsigned long		size;
  BODY 		*	b;
  PART 		*	part;

  switch (toupper(*seg++)) {	/* dispatch based on type of data */
  case 'S':			/* extensible body structure */
    if (strcasecmp(seg,"TRUCTURE") != 0) {
      sprintf(((IMAPLOCAL *) stream->local)->tmp, "Bad body fetch: %.80s", seg);
      mm_log(((IMAPLOCAL *) stream->local)->tmp, WARN, stream);
      return;
    }
				/* falls through */
  case '\0':			/* body structure */
    mail_free_body(body);	/* flush any prior body */
				/* instantiate and parse a new body */
    imap_parse_body_structure(stream, *body = mail_newbody (), txtptr, reply);
    break;

  case '[':			/* body section text */
    if ((!(s = strchr(seg, ']'))) || s[1]) {
      sprintf(((IMAPLOCAL *) stream->local)->tmp,"Bad body index: %.80s", seg);
      mm_log(((IMAPLOCAL *) stream->local)->tmp, WARN, stream);
      return;
    }
    *s = '\0';			/* tie off section specifier */

    /* get the body section text */
    s = imap_parse_string(stream, txtptr, reply, msgno, &size);
    if (!(b = *body)) {		/* must have structure first */
      mm_log("Body contents received when body structure unknown",WARN,stream);
      if (s) fs_give ((void **) &s);
      return;
    }

				/* get first section number */
    if (!(seg && *seg && isdigit (*seg))) {
      mm_log ("Bogus section number",WARN,stream);
      if (s) fs_give ((void **) &s);
      return;
    }
				/* non-zero section? */
    if (i = strtoul (seg,&seg,10)) {
      do {			/* multipart content? */
	if (b->type == TYPEMULTIPART) {
				/* yes, find desired part */
	  part = b->contents.part;
	  while (--i && (part = part->next));
	  if (!part || (((b = &part->body)->type == TYPEMULTIPART) &&
			!(s && *s))){
	    mm_log ("Bad section number",WARN,stream);
	    if (s) fs_give ((void **) &s);
	    return;
	  }
	}
	else if (i != 1) {	/* otherwise must be section 1 */
	  mm_log ("Invalid section number",WARN,stream);
	  if (s) fs_give ((void **) &s);
	  return;
	}
				/* need to go down further? */
	if (i = *seg) switch (b->type) {
	case TYPEMESSAGE:	/* embedded message, get body */
	  b = b->contents.msg.body;
	  if (!((*seg++ == '.') && isdigit (*seg))) {
	    mm_log ("Invalid message section",WARN,stream);
	    if (s) fs_give ((void **) &s);
	    return;
	  }
	  if (!(i = strtoul (seg,&seg,10))) {
	    if (b->contents.msg.hdr) fs_give ((void **) &b->contents.msg.hdr);
	    b->contents.msg.hdr = s;
	    b->contents.msg.hdrsize = size;
	    b = NIL;
	  }
	  break;
	case TYPEMULTIPART:	/* multipart, get next section */
	  if ((*seg++ == '.') && (i = strtoul (seg,&seg,10)) > 0) break;
	default:		/* bogus subpart */
	  mm_log ("Invalid multipart section",WARN,stream);
	  if (s) fs_give ((void **) &s);
	  return;
	}
      } while (i);
      if (b) switch (b->type) {	/* decide where the data goes based on type */
      case TYPEMULTIPART:	/* nothing to fetch with these */
	mm_log ("Textual body contents received for MULTIPART body part",WARN,stream);
	if (s) fs_give ((void **) &s);
	return;
      case TYPEMESSAGE:		/* encapsulated message */
	if (b->contents.msg.text) fs_give ((void **) &b->contents.msg.text);
	b->contents.msg.text = s;
	break;
      case TYPETEXT:		/* textual data */
	if (b->contents.text) fs_give ((void **) &b->contents.text);
	b->contents.text = (unsigned char *) s;
	break;
      default:			/* otherwise assume it is binary */
	if (b->contents.binary) fs_give ((void **) &b->contents.binary);
	b->contents.binary = (void *) s;
	break;
      }
    }
    else {
      MESSAGECACHE *elt = mail_elt (stream,msgno);
      if (elt->data1) fs_give ((void **) &elt->data1);
      elt->data1 = (unsigned long) s;
      elt->data3 = size;
    }
      break;
    default:			/* bogon */
      sprintf (((IMAPLOCAL *) stream->local)->tmp,"Bad body fetch: %.80s",seg);
      mm_log (((IMAPLOCAL *) stream->local)->tmp,WARN,stream);
    return;
  }
}

/* IMAP parse body structure
 * Accepts: MAIL stream
 *	    body structure to write into
 *	    current text pointer
 *	    parsed reply
 *
 * Updates text pointer
 */

void imap_parse_body_structure (MAILSTREAM *stream,BODY *body,char **txtptr,
				IMAPPARSEDREPLY *reply)
{
  int i;
  char *s;
  PART *part = NIL;
  char c = *((*txtptr)++);	/* grab first character */
				/* ignore leading spaces */
  while (c == ' ') c = *((*txtptr)++);
  switch (c) {			/* dispatch on first character */
  case '(':			/* body structure list */
    if (**txtptr == '(') {	/* multipart body? */
      body->type= TYPEMULTIPART;/* yes, set its type */
      do {			/* instantiate new body part */
	if (part) part = part->next = mail_newbody_part ();
	else body->contents.part = part = mail_newbody_part ();
				/* parse it */
	imap_parse_body_structure (stream,&part->body,txtptr,reply);
      } while (**txtptr == '(');/* for each body part */
      if (!(body->subtype = imap_parse_string (stream,txtptr,reply,(long) NIL,
					       NIL)))
	mm_log ("Missing multipart subtype",WARN,stream);
      if (**txtptr == ' ')	/* if extension data */
	body->parameter = imap_parse_body_parameter (stream,txtptr,reply);
      while (**txtptr == ' ') imap_parse_extension (stream,body,txtptr,reply);
      if (**txtptr != ')') {	/* validate ending */
	sprintf (((IMAPLOCAL *) stream->local)->tmp,"Junk at end of multipart body: %.80s",*txtptr);
	mm_log (((IMAPLOCAL *) stream->local)->tmp,WARN,stream);
      }
      else ++*txtptr;		/* skip past delimiter */
    }

    else {			/* not multipart, parse type name */
      if (**txtptr == ')') {	/* empty body? */
	++*txtptr;		/* bump past it */
	break;			/* and punt */
      }
      body->type = TYPEOTHER;	/* assume unknown type */
      body->encoding = ENCOTHER;/* and unknown encoding */
				/* parse type */
      if (s = imap_parse_string (stream,txtptr,reply,(long) NIL,NIL)) {
	ucase (s);		/* make parse easier */

	for (i=0
	       ;(i<=TYPEMAX)
	       && body_types[i]
	       && strcasecmp(s, body_types[i])
	       ;i++);

	if (i <= TYPEMAX) {	/* only if found a slot */
	  if (body_types[i]) fs_give ((void **) &s);
				/* assign empty slot */
	  else body_types[i] = cpystr (s);
	  body->type = i;	/* set body type */
	}
      }
				/* parse subtype */
      body->subtype = imap_parse_string (stream,txtptr,reply,(long) NIL,NIL);
      body->parameter = imap_parse_body_parameter (stream,txtptr,reply);
      body->id = imap_parse_string (stream,txtptr,reply,(long) NIL,NIL);
      body->description = imap_parse_string(stream,txtptr,reply,(long)NIL,NIL);

      if (s = imap_parse_string(stream,txtptr,reply,(long) NIL,NIL)) {
	ucase(s);		/* make parse easier */
				/* search for body encoding */
	for (i = 0
	       ; (i <= ENCMAX)
	       && body_encodings[i]
	       && strcasecmp(s, body_encodings[i])
	       ; i++);

	if (i > ENCMAX) body->type = ENCOTHER;
	else {			/* only if found a slot */
	  if (body_encodings[i]) fs_give ((void **) &s);
				/* assign empty slot */
	  else body_encodings[i] = cpystr (s);
	  body->encoding = i;	/* set body encoding */
	}
      }
				/* parse size of contents in bytes */
      body->size.bytes = strtoul (*txtptr,txtptr,10);
      switch (body->type) {	/* possible extra stuff */
      case TYPEMESSAGE:		/* message envelope and body */
	if (strcasecmp(body->subtype,"RFC822") != 0) {
	  break;
	}
	imap_parse_envelope (stream,&body->contents.msg.env,txtptr,reply);
	body->contents.msg.body = mail_newbody ();
	imap_parse_body_structure(stream,body->contents.msg.body,txtptr,reply);
				/* drop into text case */
      case TYPETEXT:		/* size in lines */
	body->size.lines = strtoul (*txtptr,txtptr,10);
	break;
      default:			/* otherwise nothing special */
	break;
      }
      if (**txtptr == ' ')	/* if extension data */
	body->md5 = imap_parse_string (stream,txtptr,reply,(long) NIL,NIL);
      while (**txtptr == ' ') imap_parse_extension (stream,body,txtptr,reply);
      if (**txtptr != ')') {	/* validate ending */
	sprintf (((IMAPLOCAL *) stream->local)->tmp,"Junk at end of body part: %.80s",*txtptr);
	mm_log (((IMAPLOCAL *) stream->local)->tmp,WARN,stream);
      }
      else ++*txtptr;		/* skip past delimiter */
    }
    break;
  case 'N':			/* if NIL */
  case 'n':
    ++*txtptr;			/* bump past "I" */
    ++*txtptr;			/* bump past "L" */
    break;
  default:			/* otherwise quite bogus */
    sprintf (((IMAPLOCAL *) stream->local)->tmp,"Bogus body structure: %.80s",*txtptr);
    mm_log (((IMAPLOCAL *) stream->local)->tmp,WARN,stream);
    break;
  }
}

/* IMAP parse body parameter
 * Accepts: MAIL stream
 *	    current text pointer
 *	    parsed reply
 * Returns: body parameter
 * Updates text pointer
 */

PARAMETER *imap_parse_body_parameter (MAILSTREAM *stream,char **txtptr,
				      IMAPPARSEDREPLY *reply)
{
  PARAMETER *ret = NIL;
  PARAMETER *par = NIL;
  char c,*s;
				/* ignore leading spaces */
  while ((c = *(*txtptr)++) == ' ');
				/* parse parameter list */
  if (c == '(') while (c != ')') {
				/* append new parameter to tail */
    if (ret) par = par->next = mail_newbody_parameter ();
    else ret = par = mail_newbody_parameter ();
    if(!(par->attribute=imap_parse_string(stream,txtptr,reply,(long)NIL,NIL))){
      mm_log ("Missing parameter attribute",WARN,stream);
      par->attribute = cpystr ("UNKNOWN");
    }
    if (!(par->value = imap_parse_string (stream,txtptr,reply,(long)NIL,NIL))){
      sprintf (((IMAPLOCAL *) stream->local)->tmp,"Missing value for parameter %.80s",par->attribute);
      mm_log (((IMAPLOCAL *) stream->local)->tmp,WARN,stream);
      par->value = cpystr ("UNKNOWN");
    }
    switch (c = **txtptr) {	/* see what comes after */
    case ' ':			/* flush whitespace */
      while ((c = *++*txtptr) == ' ');
      break;
    case ')':			/* end of attribute/value pairs */
      ++*txtptr;		/* skip past closing paren */
      break;
    default:
      sprintf (((IMAPLOCAL *) stream->local)->tmp,"Junk at end of parameter: %.80s",*txtptr);
      mm_log (((IMAPLOCAL *) stream->local)->tmp,WARN,stream);
      break;
    }
  }
				/* empty parameter, must be NIL */
  else if (((c == 'N') || (c == 'n')) &&
	   ((*(s = *txtptr) == 'I') || (*s == 'i')) &&
	   ((s[1] == 'L') || (s[1] == 'l')) && (s[2] == ' ')) *txtptr += 2;
  else {
    sprintf (((IMAPLOCAL *) stream->local)->tmp,"Bogus body parameter: %c%.80s",c,(*txtptr) - 1);
    mm_log (((IMAPLOCAL *) stream->local)->tmp,WARN,stream);
  }
  return ret;
}

/* IMAP parse body structure
 * Accepts: MAIL stream
 *	    current text pointer
 *	    parsed reply
 *	    body structure to write into
 *
 * Updates text pointer
 */

void imap_parse_extension (MAILSTREAM *stream,BODY *body,char **txtptr,
			   IMAPPARSEDREPLY *reply)
{
  unsigned long i,j;
  switch (*++*txtptr) {		/* action depends upon first character */
  case '(':
    while (*++*txtptr != ')') imap_parse_extension (stream,body,txtptr,reply);
    ++*txtptr;			/* bump past closing parenthesis */
    break;
  case '"':			/* if quoted string */
    while (*++*txtptr != '"') if (**txtptr == '\\') ++*txtptr;
    ++*txtptr;			/* bump past closing quote */
    break;
  case 'N':			/* if NIL */
  case 'n':
    ++*txtptr;			/* bump past "N" */
    ++*txtptr;			/* bump past "I" */
    ++*txtptr;			/* bump past "L" */
    break;
  case '{':			/* get size of literal */
    ++*txtptr;			/* bump past open squiggle */
    if (i = strtoul (*txtptr,txtptr,10)) do
      tcp_getbuffer (((IMAPLOCAL *) stream->local)->tcpstream,j = max (i,(long)IMAPTMPLEN),((IMAPLOCAL *) stream->local)->tmp);
    while (i -= j);
				/* get new reply text line */
    reply->line = tcp_getline (((IMAPLOCAL *) stream->local)->tcpstream);
    if (stream->debug) mm_dlog (stream,reply->line);
    *txtptr = reply->line;	/* set text pointer to point at it */
    break;
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
    strtoul (*txtptr,txtptr,10);
    break;
  default:
    sprintf (((IMAPLOCAL *) stream->local)->tmp,"Unknown extension token: %.80s",*txtptr);
    mm_log (((IMAPLOCAL *) stream->local)->tmp,WARN,stream);
				/* try to skip to next space */
    while ((*++*txtptr != ' ') && (**txtptr != ')') && **txtptr);
    break;
  }
}
/* SUN Imap4 */
/* IMAP set 1 flag:
 * Accepts: MAILSTREAM,
 *          sequence (message or UID)
 *          flags
 * Will send either imap4 or imap2 style command. We assume our
 * caller has insured that the server will accept this command, ie,
 * is imap4/imap2 sunversion compliant */
int imap_set1flag (MAILSTREAM *stream, char *sequence, char *flag, long flags)
{
  char *cmd = (((IMAPLOCAL *) stream->local)->imap4 && (flags & ST_UID)) ? "UID STORE" : "STORE";
  IMAPPARSEDREPLY *reply;
  IMAPARG *args[4],aseq,ascm,aflg;
  aseq.type = ATOM; aseq.text = (void *) sequence;
  ascm.type = ATOM; 
  /* If we have a sun server, then do set1flag, else do ordinary truc */
  if (stream->sunvset) {
    ascm.text = (void *)
      ((((IMAPLOCAL *) stream->local)->imap4 && (flags & ST_SILENT)) ? "+1Flag.silent" : "+1Flag");
  } else {
    ascm.text = (void *)
      ((((IMAPLOCAL *) stream->local)->imap4 && (flags & ST_SILENT)) ? "+Flags.silent" : "+Flags");
  }
  aflg.type = FLAGS; aflg.text = (void *) flag;
  args[0] = &aseq; args[1] = &ascm; args[2] = &aflg; args[3] = NIL;
				/* send "STORE sequence +Flags flag" */
  if (!imap_OK (stream,reply = imap_send (stream,cmd,args))) {
    sprintf (((IMAPLOCAL *) stream->local)->tmp,"Set 1flag rejected: %.80s",reply->text);
    mm_log (((IMAPLOCAL *) stream->local)->tmp,ERROR,stream);
    mm_log_stream(stream, reply->text);
    return NIL;
  } else
    return T;
}


/* IMAP clear ONE flag
 * Accepts: MAIL stream
 *	    sequence
 *	    flag(s)
 *	    option flags
 */

int imap_clear1flag (MAILSTREAM *stream,char *sequence,char *flag,long flags)
{
  char *cmd = (((IMAPLOCAL *) stream->local)->imap4 && (flags & ST_UID)) ? "UID STORE" : "STORE";
  IMAPPARSEDREPLY *reply;
  IMAPARG *args[4],aseq,ascm,aflg;
  aseq.type = ATOM; aseq.text = (void *) sequence;
  ascm.type = ATOM; 
  /* If we have a sun server, then do set1flag, else do ordinary truc */
  if (stream->sunvset) {
    ascm.text = (void *)
      ((((IMAPLOCAL *) stream->local)->imap4 && (flags & ST_SILENT)) ? "-1Flag.silent" : "-1Flag");
  } else {
    ascm.text = (void *)
      ((((IMAPLOCAL *) stream->local)->imap4 && (flags & ST_SILENT)) ? "-Flags.silent" : "-Flags");
  }
  aflg.type = FLAGS; aflg.text = (void *) flag;
  args[0] = &aseq; args[1] = &ascm; args[2] = &aflg; args[3] = NIL;
				/* send "STORE sequence -1Flag flag" */
  if (!imap_OK (stream,reply = imap_send (stream,cmd,args))) {
    sprintf (((IMAPLOCAL *) stream->local)->tmp,"Set 1flag rejected: %.80s",reply->text);
    mm_log (((IMAPLOCAL *) stream->local)->tmp,ERROR,stream);
    mm_log_stream(stream, reply->text);
    return NIL;
  } else
    return T;
}

int
imap_sunversion(MAILSTREAM * stream)
{
  /*
   * If imap4 world, and stream->sunvset send IMAP4SUNVERSION
   * else SUNVERSION
   */
  IMAPPARSEDREPLY *	reply;
  char 		  *	lequel;

  if (((IMAPLOCAL *) stream->local)->imap4
      || ((IMAPLOCAL *) stream->local)->imap4rev1) {

    /* server advertises "IMAP4" / "IMAP4REV1" */
    if (((IMAPLOCAL *) stream->local)->sunvcap) {
      /* Server has "IMAP4SUNVERSION" capability */
      if (stream->keep_mime) {
	lequel = "IMAP4SUNVERSION MIME YES";
      } else {
	lequel = "IMAP4SUNVERSION MIME NO";
      }
    } else { /* imap4 - NO IMAP4SUNVERSION capability */
      return(_B_FALSE);
    }
  } else {	/* imap2 server - send and hope */
    if (stream->keep_mime) {
      lequel = "SUNVERSION MIME YES";
    } else {
      lequel = "SUNVERSION MIME NO";
    }
  }
  reply = imap_send(stream, lequel, NIL);
  if (!imap_OK(stream, reply)) {
    mm_log("\"Version du soleil\" rejected!",ERROR,stream);
    return(_B_FALSE);
  } else {
    /* Set in case just imap2 out there */
    stream->sunvset = _B_TRUE;
    return(_B_TRUE);
  }
}

/* Disconnect:
 *
 * We close the tcp connection but do not checkpoint the
 * data. Used for when we think the network connection may
 * have been abruptly broken, or we just want to disconnect
 * BUT NOT flush the cache */
#define PETIT 64
void imap_disconnect (MAILSTREAM *stream)
{
  IMAPPARSEDREPLY *reply;
  char *uidseq;
  char tmp[PETIT];
  int mort;
  /* make sure ((IMAPLOCAL *) stream->local) is there. Otherwise, not connected */
  if (!stream || !(stream->stream_status & S_OPENED) || !((IMAPLOCAL *) stream->local)) return;
  /* Get final checksum update, max uid if tcpstream is there.
   * Could be that we have been disconnected by an outage, ie,
   * STREAM IS DEAD. */
  if (((IMAPLOCAL *) stream->local)->tcpstream) {
    mort = NIL;
    imap_checksum(stream);
    /* Get max uid */
    if (stream->nmsgs > 0)
      imap_uid (stream,stream->nmsgs);
  } else
    mort = _B_TRUE;
  /* build the uidseq to be used for synchronizing.
   * Must be done at disconnect time so that stream->nmsgs
   * is correct */
  sprintf(tmp, "1:%ld", stream->nmsgs);
  stream->sync_uidseq = mail_seq_to_uidseq(stream, tmp, mort);
				/* send "LOGOUT" */
  if (((IMAPLOCAL *) stream->local)->tcpstream &&
      !imap_OK (stream,reply = imap_send (stream,"LOGOUT",NIL))) {
    sprintf (((IMAPLOCAL *) stream->local)->tmp,"map_disconned logout failed: %.80s",reply->text);
    mm_log (((IMAPLOCAL *) stream->local)->tmp,WARN,stream);
    mm_log_stream(stream, reply->text);
  }
				/* TRY to close TCP connection if still open */
  if (((IMAPLOCAL *) stream->local)->tcpstream) tcp_close (((IMAPLOCAL *) stream->local)->tcpstream);
  ((IMAPLOCAL *) stream->local)->tcpstream = NIL;
				/* free up memory */
  if (((IMAPLOCAL *) stream->local)->reply.line) fs_give ((void **) &((IMAPLOCAL *) stream->local)->reply.line);

  stream->cached_nmsgs = stream->nmsgs;
  /* disable the driver */
  /*stream->dtb->flags |= DR_DISABLE;*/
  mm_log("Stream successfully disconnected", NIL,stream);
  mm_log_stream(stream, "136 imapd: Disconnected");
				/* nuke the local data */
  fs_give ((void **) &stream->local);
  stream->local = NIL;
}

/* Mail Access Protocol echo
 * Accepts: stream to open
 *          with mailbox == {hostname}
 * Returns: T if a connection can be opened, else NIL
 */

int imap_serverecho (MAILSTREAM *stream)
{
  NETMBX mb;
  char *s;
  TCPSTREAM *tcpstream;
  char tmp[MAILTMPLEN];
  IMAP_GLOBALS *ig = (IMAP_GLOBALS *)stream->imap_globals;

  mail_valid_net_parse (stream->mailbox,&mb);
  if (mb.host[0] == '\0') {
    mm_log ("ECHO: Host not specified",ERROR,stream);
    return NIL;
  }
  if (mb.port || ig->imap_defaultport) /* set up host with port override */
    sprintf (s = tmp,"%s:%ld",mb.host,mb.port ? mb.port : 
	     ig->imap_defaultport);
  else s = mb.host;		/* simple host name */
  tcpstream = tcp_open (s, "imap", IMAPTCPPORT, stream);
  if (tcpstream) tcp_close (tcpstream);
  return tcpstream ? T : NIL;
}

void imap_cleario (MAILSTREAM *stream)
{
  if (stream && ((IMAPLOCAL *) stream->local) && ((IMAPLOCAL *) stream->local)->tcpstream)
    clear_tcp_iocounter (((IMAPLOCAL *) stream->local)->tcpstream);
}

void imap_urgent (MAILSTREAM *stream)
{
  os_increment_send_urgent (((IMAPLOCAL *) stream->local)->tcpstream);
}

static int sinfo_present(MAILSTREAM *stream, long i)
{
  char *voila = mail_lelt (stream, i)->sinfo.date;
  return (voila ? T : NIL);
}

void imap_fetchshort (MAILSTREAM *stream, unsigned long msgno, 
		      SHORTINFO *user_sinfo,
		      long flags)
{
  IMAPPARSEDREPLY *reply;
  char *seq= ((IMAPLOCAL *) stream->local)->sbuf;
  LONGCACHE *lelt;
  int info_required = NIL;
  SHORTINFO *sinfo;
  IMAPARG *args[3],aseq,aatt;
  char *cmd = "FETCH";
  int success;
				/* never lookahead with UID fetching */
  if (((IMAPLOCAL *) stream->local)->imap4 && (flags & FT_UID)) {
    int i;
					/* Default to normal fetch? */
    for (i = 1; i <= stream->nmsgs; i++) {
      if (mail_elt(stream,i)->uid == msgno)
	imap_fetchshort (stream,i,user_sinfo,flags & ~FT_UID);
    }
    cmd = "UID FETCH";
    /* Must use secondary flag to force short
     * caching to make sure lelt's get created */
    ((IMAPLOCAL *) stream->local)->scache = _B_TRUE;
    stream->msgno = 0;		/* force refetching */
    info_required = _B_TRUE;          /* get the info */
    sinfo = &stream->sinfo;
    mail_free_fshort_data(sinfo);
    sprintf (seq,"%ld",msgno);	/* never lookahead with a short cache */
  } else {
    /* NOT uid thing */
    ((IMAPLOCAL *) stream->local)->scache = NIL;
    if (stream->scache) {		/* short cache */
      sinfo = &stream->sinfo;
      if (msgno != stream->msgno){/* flush old poop if a different message */
	mail_free_fshort_data(sinfo);
	info_required = 1;	/* We need to fetch from server. */
      } else 
	if (sinfo->date == NIL) info_required = 1;
      stream->msgno = msgno;
      sprintf (seq,"%ld",msgno);/* never lookahead with a short cache */
    }
    else {			/* long cache */
      lelt = mail_lelt (stream,msgno);
      sinfo = &lelt->sinfo;
      if (sinfo->date == NIL) info_required = 1;
      else 
	success = _B_TRUE;		/* Already have it */
    }
  }
  if (info_required)	{	/* then the sinfo structure is empty */
    if (!(stream->scache || ((IMAPLOCAL *) stream->local)->scache)) {
				/* look ahead only for long cache */
      make_lookahead (stream,msgno,seq,sinfo_present);
    }
    /* fetch the data */
    if (stream->sunvset) {	/*  then we can fetch shortinfo */
      success = NIL;
      args[0] = &aseq; args[1] = &aatt; args[2] = NIL;
      aseq.type = aatt.type = ATOM;
      aseq.text = (void *)seq;
      /* Allow ENVELOPE along with shortinfo */
      if (flags & FT_SHORTENV) aatt.text = (void *)"(SHORTINFO ENVELOPE)";
      else aatt.text = (void *)"SHORTINFO";
      reply = imap_send (stream, cmd, args);
      if (!imap_OK (stream, reply)) {
	sprintf (((IMAPLOCAL *) stream->local)->tmp, "Fetch short rejected: %.80s", reply->text);
	mm_log (((IMAPLOCAL *) stream->local)->tmp,ERROR,stream);
	mm_log_stream(stream, reply->text);
      } else {
	if (((IMAPLOCAL *) stream->local)->imap4 && (flags & FT_UID)) {
	  int i;		/* validate UID was found */
	  for (i = 1; i <= stream->nmsgs; i++) {
	    if (mail_elt(stream,i)->uid == msgno) {
	      success = _B_TRUE; break;
	    }
	  }
	} else success = _B_TRUE;
      }
    } else {			/* fetch structure/UIDs and fake short info */
      BODY *body;
      ENVELOPE *env = imap_fetchstructure(stream, msgno, &body, flags);
      if (!env) success = NIL;
      else {			/* fetch uids for these guys too */
	int type;
	imap_uid(stream, msgno);
	sinfo->date = cpystr(mail_date(((IMAPLOCAL *) stream->local)->tmp,mail_elt(stream,msgno)));
	if (env->from) {
	  sinfo->personal =  env->from->personal ? 
	    cpystr(env->from->personal) : NIL;
	  sinfo->mailbox = env->from->mailbox ? cpystr(env->from->mailbox) : NIL;
	  sinfo->host = env->from->host ? cpystr(env->from->host) : NIL;
	} else sinfo->personal = sinfo->mailbox = sinfo->host = NIL;
	sinfo->subject = cpystr(env->subject ? env->subject : "");
	sprintf(((IMAPLOCAL *) stream->local)->tmp, "%ld", mail_elt(stream,msgno)->rfc822_size);
	sinfo->size = cpystr(((IMAPLOCAL *) stream->local)->tmp);
	sinfo->from = mail_make_from(sinfo);
	if (body) {
	  type = body->type;
	  /* Set TYPETEXT iff TEXT/PLAIN or MULTIPART with one part
	   * that is TEXT/PLAIN. */
	  if (type == TYPEMULTIPART) {
	    PART *joint = body->contents.part;
	    if (joint && joint->body.type == TYPETEXT && 
		(!joint->body.subtype || 
		 strcasecmp(joint->body.subtype,"plain") == 0) &&
		joint->next == NIL)
	      type = TYPETEXT;
	    sinfo->bodytype = cpystr(body_types[type]); 
	  } else if (type == TYPETEXT && 
		     (body->subtype
		      && (strcasecmp(body->subtype,"plain") != 0)))
	    sinfo->bodytype = strdup("TEXT-OTHER");
	  else sinfo->bodytype = cpystr(body_types[type]);
	} else sinfo->bodytype = cpystr(body_types[TYPETEXT]);
	success = _B_TRUE;
      }
    }
  }
  if (user_sinfo && success) {
    user_sinfo->date = sinfo->date;
    user_sinfo->personal = sinfo->personal;
    user_sinfo->mailbox = sinfo->mailbox;
    user_sinfo->host = sinfo->host;
    user_sinfo->subject = sinfo->subject;
    user_sinfo->bodytype = sinfo->bodytype;
    user_sinfo->size = sinfo->size;
    user_sinfo->from = sinfo->from;
  } else if (user_sinfo) {
    user_sinfo->date = NIL;           /* no data returned */
    user_sinfo->personal = NIL;
    user_sinfo->mailbox = NIL;
    user_sinfo->host = NIL;
    user_sinfo->subject = NIL;
    user_sinfo->bodytype = NIL;
    user_sinfo->size = NIL;
    user_sinfo->from = NIL;
  }
}

/* IMAP checksum
 * Accepts: mail stream
 * Returns: T on success, NIL on failure
 * For compatibility with Imap2bis Roam
 */
void imap_checksum (MAILSTREAM *stream)
{
  if (!stream->sunvset) return;
  if (((IMAPLOCAL *) stream->local)->imap4) {
    /* Note that this is for checksumming/uid_validity of THIS mailbox */
    ((IMAPLOCAL *) stream->local)->csstatus = _B_TRUE;
    imap_status(stream, stream->mailbox, SA_CHECKSUM+SA_UIDVALIDITY,NIL);
    ((IMAPLOCAL *) stream->local)->csstatus = NIL;
  } else				/* imap2bis checksum  */
    imap_send (stream, "CHECKSUM", NIL);
}
/*
 * We send valid checksum only if sunvset:
 *  We have either the imap2bis or imap4 style.
 */
int imap_validchecksum (MAILSTREAM *stream, unsigned short cksum, 
		       unsigned long nbytes)
{
  IMAPPARSEDREPLY *reply;
  char tmp[MAILTMPLEN];
  IMAPARG *args[2], aref;
  stream->validchecksum = NIL;	/* invalidate checksum */
  /*
   * Ce n'est pas la peine si le serveur est bete:
   * Or it's no use if the server is stupid */
  if (!stream->sunvset) return NIL;
  /*
   * just cccccssssss or exactly 5bytes of checksum followed
   * by the size in bytes to checksum */
  if (((IMAPLOCAL *) stream->local)->imap4)
    sprintf(tmp, "%05d%d", cksum, nbytes);
  else
    /* IMAP2 - but send as ATOM.
     * make passed paramters - "checksum" "mbox_size" "driver name" */
    sprintf(tmp, "\"%d\" \"%d\" \"%s\"", cksum, nbytes, 
	    (stream->driver_name ? stream->driver_name : "solaris"));
  aref.type = ATOM;
  aref.text = (void *)tmp;
  args[0] = &aref; 
  args[1] = NIL;
  if (!imap_OK (stream, reply = imap_send (stream,"VALIDCHECKSUM", args))) {
    sprintf (((IMAPLOCAL *) stream->local)->tmp,"Checksum validation rejected: %.80s",reply->text);
    mm_log (((IMAPLOCAL *) stream->local)->tmp, ERROR,stream);
    mm_log_stream(stream, reply->text);
  }
  /* stream->validchecksum is set in parse_unsolicited by what was returned
   * from the server */
  return stream->validchecksum;
}

/* 
 * Parse checksum:   Imap2bis sun extensionism ...          
 *
 * ("checksum" "filesize" "driver-name") */
void imap_parse_checksum(MAILSTREAM *stream, IMAPPARSEDREPLY *reply)
{
  char *text= reply->text;
  char *str;

  ++text;			/* skip the '{' */
  str = imap_parse_string (stream, &text, reply, (long)NIL,(ulong *)NIL);
  if (str) {
    stream->checksum = strtol(str, NIL, 10);
    fs_give((void **)&str);
    str = imap_parse_string (stream, &text, reply, (long)NIL, (ulong *)NIL);
    if (str) {
      stream->mbox_size = strtol(str, NIL, 10);
      fs_give((void **)&str);
      if (stream->driver_name)
	fs_give((void **)&stream->driver_name);
      stream->driver_name = imap_parse_string (stream, &text, reply,
					       (long)NIL, (ulong *)NIL);
      ++text;			/* skip ')' */
    }
  }
}

void 
imap_parse_validchecksum(MAILSTREAM *stream, char *text)
{
  if (strcmp(text, "VALID") == 0) {
    stream->validchecksum = _B_TRUE;
  } else {
    stream->validchecksum = _B_FALSE;
  }
  return;
}


/*         
 * Parse short info:
 * ("date" from-addr "subject" "body type" "822 size")
 */
void imap_parse_shortinfo (
	MAILSTREAM *stream,
        SHORTINFO *sinfo,
	char **txtptr,
        IMAPPARSEDREPLY *reply)
{
  char *text, **data;
  ADDRESS *adr = NIL;
  char tmp[IMAPTMPLEN];

  (*txtptr)++;	/* skip first character, ie, '(' */
					/* get the date string */
  sinfo->date = imap_parse_string (stream, txtptr, reply, (long) NIL,
				   (ulong *)NIL);

  /* extract the  address fields*/
  if (adr = imap_parse_adrlist (stream,txtptr,reply)) {
    if (adr->personal) {
      sprintf(tmp, "%s", adr->personal);
      sinfo->personal = cpystr(tmp);
    } else 
      sinfo->personal = NIL;
    
    if (adr->mailbox) {
      sprintf(tmp, "%s", adr->mailbox);
      sinfo->mailbox = cpystr(tmp);
    } else 
      sinfo->mailbox = NIL;
    
    if (adr->host) {
      sprintf(tmp, "%s", adr->host);
      sinfo->host = cpystr(tmp);
    } else 
      sinfo->host = NIL;
    mail_free_address(&adr);
  } else {
    sinfo->personal = sinfo->mailbox = sinfo->host = NIL;
  }
  sinfo->subject = imap_parse_string (stream, txtptr, reply, (long) NIL,
				      (ulong *)NIL);
  sinfo->bodytype = imap_parse_string (stream, txtptr, reply, (long) NIL,
				       (ulong *)NIL);
  sinfo->size = imap_parse_string (stream, txtptr, reply, (long) NIL,
				   (ulong *)NIL);
  sinfo->from = mail_make_from(sinfo);

  if (**txtptr != ')') {
     sprintf (((IMAPLOCAL *) stream->local)->tmp,"Junk at end of shortinfo: %.80s",*txtptr);
     mm_log (((IMAPLOCAL *) stream->local)->tmp, WARN,stream);
   } else 
     *txtptr += 1;	/* skip past delimiter */
}

void
imap2_process_1flag(MAILSTREAM * stream, char pm, char * sequence, char * flag)
{
  if (mail_sequence (stream, sequence)) { /* then we can chase the msgnos */
    int i;
    int value = (pm == '+' ? _B_TRUE : _B_FALSE);

    for (i = 1; i <= stream->nmsgs; i++) {
      MESSAGECACHE *elt;
      if ((elt = mail_elt (stream,i))->sequence) {
	if (strcmp(ucase(flag), "(\\SEEN)") == 0) {
	  elt->seen = value;

	} else if (strcmp(flag, "(\\DELETED)")== 0) {
	  elt->deleted = value;

	} else if (strcmp(flag, "(\\FLAGGED)") == 0) {
	  elt->flagged = value;

	} else if (strcmp(flag, "(\\ANSWERED)") == 0) {
	  elt->answered = value;

	} else if (strcmp(flag, "(\\RECENT)") == 0) {
	  elt->recent = value;
	}
      }
    }
  }
  return;
}

/*
 * *flag == "(flag)" here 
 */
void
imap4_process_1flag(MAILSTREAM * stream, char pm, char * sequence, char ** txt)
{
  /* Advance flag past '(' */
  char 	*	flag;

  ++*txt;
  flag = *txt;
  while (**txt && **txt != ')') {
    ++*txt;
  }
  **txt = '\0';				/* Replace ')' by NULL */
  ++*txt;				/* skip past the NULL */
  if (mail_sequence(stream, sequence)) { /* then we can chase the msgnos */
    int 	i;
    int 	value = (pm == '+' ? _B_TRUE : _B_FALSE);

    for (i = 1; i <= stream->nmsgs; i++) {
      MESSAGECACHE 	*	elt;
      if ((elt = mail_elt(stream, i))->sequence) {
	if (strcmp(ucase(flag), "\\SEEN") == 0) {
	  elt->seen = value;

	} else if (strcmp(flag, "\\DELETED") == 0) {
	  elt->deleted = value;

	} else if (strcmp(flag, "\\FLAGGED") == 0) {
	  elt->flagged = value;

	} else if (strcmp(flag, "\\ANSWERED") == 0) {
	  elt->answered = value;

	} else if (strcmp(flag, "\\RECENT") == 0) {
	  elt->recent = value;
	}
      }
    }
  }
  return;
}

/*
 * IMAP parse uid sequence
 * Accepts MAILSTREAM, char **sequence
 */
void
imap_parse_uidsequence(MAILSTREAM * stream, char * msgseq, char ** txtptr)
{
  MESSAGECACHE *elt;
  long i;
  char *sequence = *txtptr, *tok;
  char *lasts;

  /* null end of sequence */
  tok = strtok_r(sequence, " )", &lasts );
  /* make sure proper delimeter is there,
   * point after delimeter */
  if (tok)
    *txtptr += strlen(tok) + 1;
  else
    *txtptr += strlen(*txtptr);
  /* We have a msg sequence which is in 1-1 corrspondence with
   * the uid sequence, EG: 1,2,11:13 <--> 101,102,111,119,200 */
  if (mail_sequence (stream, msgseq)) {
    /* Now, just run through the set message sequence storing the
     * corresponding UID. Est-ce qu'il vaut la peine?? De ma part ca
     * fout la merde! */
    unsigned long *expanded = mail_expand_uid_sequence (stream, sequence);
    long u;
    /* some error in sequence */
    if (!expanded) return;
    for (i = 1, u = 0; i <= stream->nmsgs; ++i) {
      if ((elt = mail_elt (stream,i))->sequence) {
	elt->uid = expanded[u++];
				/* Maintain max uid  */
	if (elt->uid > stream->uid_last)
	  stream->uid_last = elt->uid;
      }
    }
    fs_give ((void *)&expanded);
  }
}
/* imap_set_uidelt:
 *   [Called only when synchronizing the cache at reconnection time]
 * Given the uid, find the cache element which has this uid, and set
 * elt->sync_msgno = msgno:
 * UIDs are RETURNED monotonically increasing. Thus the search for the next uid
 * will begin at the next elt. EG:
 *   elt->msgno   elt->uid   elt->sync_msgno
 *      20          1111         15
 *      21          2111         16
 *      30          2155         17
 *
 * Note: stream->cached_nmsgs >= stream->nmsgs since messages may have been
 *       expunged on the server.
 */
void imap_set_uidelt(MAILSTREAM *stream, unsigned long uid, unsigned long msgno)
{
  unsigned long i;
  for (i = ((IMAPLOCAL *) stream->local)->sync_start; i <= stream->cached_nmsgs; ++i) {
    MESSAGECACHE *elt = mail_elt(stream,i);
    if (elt->uid == uid) {
      elt->sync_msgno = msgno;
      ((IMAPLOCAL *) stream->local)->sync_start = i+1;
      return;
    }
  }
}
/*
 * imap_imap4:
 *   returns T if ((IMAPLOCAL *) stream->local)->imap4 is T. */
int imap_imap4(MAILSTREAM *stream)
{
  /* If we have lost local, eg, canceled
   * login, then return the capability
   * that was stashed on the stream */
  if (((IMAPLOCAL *) stream->local)) return ((IMAPLOCAL *) stream->local)->imap4;
  else return stream->imap4_connected;
}

/* IMAP return host name
 * Accepts: MAIL stream
 * Returns: host name
 */

char *imap_host (MAILSTREAM *stream)
{
				/* return host name on stream if open */
  return (((IMAPLOCAL *) stream->local) && ((IMAPLOCAL *) stream->local)->tcpstream) ? tcp_host (((IMAPLOCAL *) stream->local)->tcpstream) :
    "<closed stream>";
}


/* IMAP return port number
 * Accepts: MAIL stream
 * Returns: port number
 */

unsigned long imap_port (MAILSTREAM *stream)
{
				/* return port number on stream if open */
  return (((IMAPLOCAL *) stream->local) && ((IMAPLOCAL *) stream->local)->tcpstream) ? tcp_port (((IMAPLOCAL *) stream->local)->tcpstream) :
    0xffffffff;
}

/* IMAP globals initializer
 * Accepts: MAIL stream
 * Returns: None
 */
void imap_global_init (MAILSTREAM *stream)
{
  IMAP_GLOBALS *ig = (IMAP_GLOBALS *)fs_get(sizeof(IMAP_GLOBALS));
  memset ((void *)ig,0,sizeof(IMAP_GLOBALS));

  ig->imap_maxlogintrials = MAXLOGINTRIALS;
  ig->imap_lookahead      = IMAPLOOKAHEAD;
  ig->imap_uidlookahead = IMAPUIDLOOKAHEAD;
  ig->imap_defaultport = 0;
  ig->imap_prefetch = IMAPLOOKAHEAD;
  ig->imap_closeonerror = NIL;
 
  stream->imap_globals = ig;
}

/* IMAP globals free 
 * Accepts: MAIL stream
 * Returns: None
 */
void imap_global_free (MAILSTREAM *stream)
{
 if (stream->imap_globals) 
     fs_give ((void **) &stream->imap_globals);
}
