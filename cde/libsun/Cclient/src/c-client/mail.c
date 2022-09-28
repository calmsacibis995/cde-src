/*
 * @(#)mail.c	1.15 97/06/11
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
 * Last Edited:	February 1997 
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
 * Edit log (mieux tard que jamais):
 * 19-july-86: mail.c/mail.h
 *     mail_open() passed 4 extra parameters to set/clear ttlock, and to
 *     set/clear group permissions.
 * 25-july-96
 *     added function to aid in freeing mmapped in solaris.c data
 *     freed body->md5
 *  1-aug-96  changed names of mail_driver_free_body.
 *  8-aug-96  error message problem in mail_parse_flags()
 * 13-aug-96  mail_rename: stream->mailbox NIL bug fixed.
 * 23-aug-96  session lockf added for dtmail.
 *  3-sept-96: nokodsig flag. Used by opens that FAIL immediately if session
 *             lock is busy.
 *    9-20-96: strtok_r is used to ensure MT-Safe. 
 * 26-sept-96: stream->dead = NIL on recycled stream.
 * 26-oct-96:  OP_NOUSERSIGS implemented
 *             copyfull returns NIL is sequence is NIL.
 * 31-oct-96: fixed mail_fetchsubject to use shortinfo
 * 22-nov-23  And an oversight in that fix!
 * 26-nov-96  mm_log(.., ERROR) in status command.
 * +++++++++++++++++++++++more on MT-safe++++++++++++++++++++
 * [10-04-96:clin] gmtime_r and localtime_r implemented to ensure MT-Safe.
 * [10-21-96:clin] Merge before 10/18/96.
 *              Two new routine added for globals:
 *              mail_global_init and mail_stream_create.
 *	        Pass stream in the following routines:
 *		all X_parameters routine,
 *		default_proto,  
 *		(*auth->server),  
 *		mail_auth 
 *	        sm_XXX,
 *		(*factory->valid)
 *		mail_restore_cache
 *		mail_search_gets
 *		mail_cached_filename
 *		mail_parameters calls
 *		d->list
 *		d->lsub
 *		d->scan
 *		mail_open	
 * 		
 *		Added stream_status bit test in:
 *		mail_list
 *		mail_open
 *		mail_lsub
 *		mail_parameters
 *		mail_rename
 *
 *		Major changes made in:
 *		mail_open
 *		mail_valid
 *
 *[10-24-96:clin]  Put the following globals on the stream:
 *
 *		sbname 
 *		userdata 
 * 		mg->sstring
 * 		mg->scharset
 *
 *[10-25-96:clin]  Check for NIL sequence in mail_sequence		
 *		call fs_give in mail_close to give up the globals.
 *
 *[10-28-96:clin]  Stream passed in all mail_valid calls.
 *		Note that a stream is always needed in a mail_valid call,
 *		because the globals will be referenced in the subsequent
 *		calls.		
 *
 *		Pass a new option "default_port" in mail_echo,
 *		because we're worried that the default_port is different
 *		from the initial value which is created by mail_stream_create.
 *
 *		Call mail_stream_create in mail_echo.
 *		
 *		Pass a mailstream in all dummy_file call which was
 *		missed by the compiler.
 *
 *		Test stream_status bit in mail_stream_unset in case
 *		the bit was set in during the call.
 *
 *[10-30-96:clin] Add mail_stream_create in mail_open. Now mail_open
 *		is the (only) routine that you can call with a NIL stream.   
 *		--It's OK to pass a NIL and return a NIL in mail_open. 
 *
 *[10-31-96:clin] Test stream_status in mail_create, mail_delete, 
 *		mail_(un)subscribe, mail_find_all_local, mail_status. 
 *
 *		Undo passing a stream in mail_open in mail_status; pass
 *		a NIL now.
 *
 *		Call memset in mail_global_init.
 *
 *[11-01-96:clin] Give up imapd_globals in mail_close_full.
 *[11-02-96:clin] Init imapd_globals in mail_stream_create.
 *[11-08-96:clin] Pass stream in all mm_log, mail_criteria and 
 *		  mail_search_text calls.  
 *[11-11-96:clin] Pass a tmpstream in mm_log calls in mail_create
 *		because the original stream will be altered.  
 *
 *[11-12-96:clin] We can't return a NIL in mail_open since we need
 *		the most current globals.
 *[11-14-96:clin] Add stream_status testing in mail_append_full. 
 *[11-19-96:clin] Pass a stream in all fatal calls. 
 *[11-20-96:clin] Flush individual globals in env_globals on stream
 *		in mail_close_full. 
 *
 *		Create a new routine: mail_stream_flush to free up
 *		all globals allocated even though the mail_close
 *		is not called.
 *
 *[11-21-96:clin] Create a new routine: default_proto_flush to free up
 *		the storage allocated in default_proto.
 *
 *[12-01-96:clin] We can't call mail_close in mail_open anymore even
 *		though the mailbox is invalid.
 *
 *[12-05-96:clin] If we use the old stream, we need to flush the old
 *		mailbox before assigning a new one in mail_open.
 *
 *		Flush the old mailbox as well in mail_stream_flush.
 *
 *[12-18-96:clin] Get ride of all the macro for mail_globals on stream.
 *
 *[12-20-96:clin] Move imapd_global_init to imapd.c.
 *
 *[12-30-96:clin] Define env_globals on stream to be a "void" type.
 *		  Cast it in each routine before referring to it. 
 *
 *[01-01-97: clin ] No need to pass a NIL stream in mail_stream_create.
 *
 *[01-01-97: clin ] Add a new routine mail_global_free.
 *
 *[01-02-97: clin ] Remove default_port option from mail_echo.
 *
 *[01-02-97: clin ] Add two new routines: mail_stream_setNIL and
 *		    mail_stream_unsetNIL.
 *
 *[01-03-97: clin ] Need to pass a stream in mail_open now because
 *		    imapd_globals needs to be passed on. 
 *
 *[01-06-97: clin ] Get rid of default_proto_flush in mail_create. 
 *
 *[01-07-97: clin ] Call mail_stream_create in mail_open.
 *
 *[01-07-97: clin ] Pass the last five parameters in mail_open to
 *		    mail_stream_create.
 *
 *[01-10-97: clin ] Re-structured mail_status.
 *		    Pass a stream to factory->open in mail_open.
 *
 *[01-14-97: clin ] Re-structured mail_prototype. Merge with
 *
 *[01-15-97: clin ] Check for stream_status in mail_status and mail_close_full.
 *
 *[01-16-97: clin ] Check for stream_status in mail_close_full.
 *
 *[01-21-97: clin ] Check for S_OPENED in all the stream checking cases.
 *
 *[01-22-97: clin ] Call mail_stream_flush in mail_status.
 *		    Remove S_NILCALL test in mail_open.
 *
 * Bill -- 5 feb 97 Memory access fix for strncpy(text,hdrs->text,i);
 *   
 *[02-30-97 ] Add Bill's fix in mail_expand_uid_sequence.
 */

#include "UnixDefs.h"
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include "mail.h"
#include "os_sv5.h"
#include <time.h>
#include "misc.h"
#include "rfc822.h"
#include "dummy.h"

void tcp_global_free(MAILSTREAM *stream);
int ndigits(unsigned long v);
void tcp_global_init(MAILSTREAM *stream);

/* c-client global data */

				/* list of mail drivers */
static DRIVER *maildrivers = NIL;
/* mail cache manipulation function */
static mailcache_t mailcache = mm_cache;
/* list of authenticators (used by imapd) */
AUTHENTICATOR *authenticators = NIL;
/* for saving c-client cache */

/* Default mail cache handler
 * Accepts: pointer to cache handle
 *	    message number
 *	    caching function
 * Returns: cache data
 */

void *mm_cache (MAILSTREAM *stream,unsigned long msgno,long op)
{
  size_t newsize;
  void *ret = NIL;
  long i = msgno - 1;
  unsigned long j = stream->cachesize;
  SHORTINFO *sinfo;		/* Sun Imap4 .. */
  switch ((int) op) {		/* what function? */
  case CH_LFINIT:
    /* Sun Imap4: In this case we have just done a select. A playback failed,
     * BUT our nmsgs is valid. We have not loaded anything from
     * the server. */
    if (stream->cachesize) {	/* flush old cache contents */
      while (stream->cachesize) mm_cache (stream,stream->cachesize--, CH_FREE_ALWAYS);
      fs_give ((void **) &stream->cache.c);
      stream->nmsgs = 0;	/* can't have any messages now */
      stream->min_dirty_msgno = 0; /* no messages */
    }
    break;
  case CH_INIT:			/* initialize cache */
    if (stream->cachesize) {	/* flush old cache contents */
      while (stream->cachesize) mm_cache (stream,stream->cachesize--,CH_FREE);
      fs_give ((void **) &stream->cache.c);
      stream->nmsgs = 0;	/* can't have any messages now */
      stream->min_dirty_msgno = 0; /* Sun Imap4: no messages */
    }
    break;
  case CH_SIZE:			/* (re-)size the cache */
    if (msgno > j) {		/* do nothing if size adequate */
      newsize = (stream->cachesize = msgno + CACHEINCREMENT) * sizeof (void *);
      if (stream->cache.c) fs_resize ((void **) &stream->cache.c,newsize);
      else stream->cache.c = (void **) fs_get (newsize);
      /* init cache */
      while (j < stream->cachesize) stream->cache.c[j++] = NIL;
    }
    break;
  case CH_MAKELELT:		/* return long elt, make if necessary */
    if (!stream->cache.c[i]) {	/* have one already? */
      /* no, instantiate it */
      stream->cache.l[i] = (LONGCACHE *) fs_get (sizeof (LONGCACHE));
      memset (&stream->cache.l[i]->elt,0,sizeof (MESSAGECACHE));
      stream->cache.l[i]->elt.lockcount = 1;
      stream->cache.l[i]->elt.msgno = msgno;
      stream->cache.l[i]->env = NIL;
      stream->cache.l[i]->body = NIL;
      /* Sun Imap4 - clear short info stash */
      sinfo = &stream->cache.l[i]->sinfo;
      sinfo->date = sinfo->personal = sinfo->mailbox = NIL;
      sinfo->host = sinfo->subject = sinfo->bodytype = NIL;
      sinfo->size = sinfo->from = NIL;
    }
    /* drop in to CH_LELT */
  case CH_LELT:			/* return long elt */
    ret = stream->cache.c[i];	/* return void version */
    break;
    
  case CH_MAKEELT:		/* return short elt, make if necessary */
    if (!stream->cache.c[i]) {	/* have one already? */
      if (stream->scache) {	/* short cache? */
	stream->cache.s[i] = (MESSAGECACHE *) fs_get (sizeof(MESSAGECACHE));
	memset (stream->cache.s[i],0,sizeof (MESSAGECACHE));
	stream->cache.s[i]->lockcount = 1;
	stream->cache.s[i]->msgno = msgno;
      }
      else mm_cache (stream,msgno,CH_MAKELELT);
    }
    /* drop in to CH_ELT */
  case CH_ELT:			/* return short elt */
    ret = stream->cache.c[i] && !stream->scache ?
      (void *) &stream->cache.l[i]->elt : stream->cache.c[i];
    break;
  case CH_FREE:			/* free (l)elt */
    if (stream->scache) mail_free_elt (&stream->cache.s[i]);
    else mail_free_lelt (&stream->cache.l[i]);
    break;
  case CH_FREE_ALWAYS:		/* Sun Imap4: free (l)elt */
    if (stream->scache) mail_always_free_elt (&stream->cache.s[i]);
    else mail_always_free_lelt (&stream->cache.l[i]);
    break;
  case CH_EXPUNGE:		/* expunge cache slot */
    /* slide down remainder of cache */
    for (i = msgno; i < stream->nmsgs; ++i)
      if (stream->cache.c[i-1] = stream->cache.c[i])
	((MESSAGECACHE *) mm_cache (stream,i,CH_ELT))->msgno = i;
    stream->cache.c[stream->nmsgs-1] = NIL;
    break;
  default:
    ret = NULL;
    mm_log("Bad mm_cache op", ERROR, stream);
    break;
  }
  return ret;
}

/* Dummy string driver for complete in-memory strings */

STRINGDRIVER mail_string = {
  mail_string_init,		/* initialize string structure */
  mail_string_next,		/* get next byte in string structure */
  mail_string_setpos		/* set position in string structure */
};


/* Initialize mail string structure for in-memory string
 * Accepts: string structure
 *	    pointer to string
 *	    size of string
 */

void mail_string_init (STRING *s,void *data,unsigned long size)
{
  /* set initial string pointers */
  s->chunk = s->curpos = (char *) (s->data = data);
  /* and sizes */
  s->size = s->chunksize = s->cursize = size;
  s->data1 = s->offset = 0;	/* never any offset */
}


/* Get next character from string
 * Accepts: string structure
 * Returns: character, string structure chunk refreshed
 */

char mail_string_next (STRING *s)
{
  return *s->curpos++;		/* return the last byte */
}


/* Set string pointer position
 * Accepts: string structure
 *	    new position
 */

void mail_string_setpos (STRING *s,unsigned long i)
{
  s->curpos = s->chunk + i;	/* set new position */
  s->cursize = s->chunksize - i;/* and new size */
}

/* Mail routines
 *
 *  mail_xxx routines are the interface between this module and the outside
 * world.  Only these routines should be referenced by external callers.
 *
 *  Note that there is an important difference between a "sequence" and a
 * "message #" (msgno).  A sequence is a string representing a sequence in
 * {"n", "n:m", or combination separated by commas} format, whereas a msgno
 * is a single integer.
 *
 */

/* Mail link driver
 * Accepts: driver to add to list
 */

void mail_link (DRIVER *driver)
{
  DRIVER **d = &maildrivers;
  while (*d) d = &(*d)->next;	/* find end of list of drivers */
  *d = driver;			/* put driver at the end */
  driver->next = NIL;		/* this driver is the end of the list */
}

/* Mail manipulate driver parameters
 * Accepts: mail stream
 *	    function code
 *	    function-dependent value
 * Returns: function-dependent return value
 */

void *mail_parameters (MAILSTREAM *stream,long function,void *value)
{
  MAIL_GLOBALS *mg = (MAIL_GLOBALS *)stream->mail_globals;
  void *r,*ret = NIL;
  DRIVER *d;
 
  switch ((int) function) {
  case SET_DRIVERS:
    mm_log("SET_DRIVERS not permitted", ERROR, stream);
    break;
  case GET_DRIVERS:
    ret = (void *) maildrivers;
    break;
  case SET_GETS:
    mg->mailgets = (mailgets_t) value;
    break;
  case GET_GETS:
    ret = (void *) mg->mailgets;
    break;
  case SET_CACHE:
    mailcache = (mailcache_t) value;
  case GET_CACHE:
    ret = (void *) mailcache;
    break;
  case SET_USERDATA:
    mg->userdata = value;  /* on the stream */
    break;
  case GET_USERDATA:
    ret = (void *) mg->userdata;
    break;
  case SET_SAVECACHEPATH:
    if (mg->savecachepath != NIL)
      fs_give((void **)&mg->savecachepath);
    mg->savecachepath = fs_get(strlen((char *)value)+1);
    strcpy(mg->savecachepath,(char *)value);
    break;
  case GET_SAVECACHEPATH:
    ret = (void *)mg->savecachepath;
    break;

  default:
    if ( stream && !(stream->stream_status & S_NILCALL) && stream->dtb)	
      /* if have stream_status set, do for that stream only */
      ret = (*stream->dtb->parameters) (stream, function,value);
    /* else do all drivers */
    else for (d = maildrivers; d; d = d->next)
      if (r = (d->parameters) (stream, function,value)) ret = r;
    /* do environment */
    if (r = env_parameters (stream, function,value)) ret = r;
    /* do TCP/IP */
    if (r = tcp_parameters (stream, function,value)) ret = r;
    break;
  }
  return ret;
}

/*
 * Mail validate mailbox name
 * Accepts: MAIL stream
 *	    mailbox name
 *	    purpose string for error message
 * Return: driver factory on success, NIL on failure
 */
DRIVER *
mail_valid(MAILSTREAM * stream, char * mailbox,char * purpose)
{
  char 			tmp[MAILTMPLEN];
  DRIVER	*	factory;

  for (factory = maildrivers; factory; factory = factory->next) {
    if (((factory->flags & DR_LOCAL) && (*mailbox == '{'))
	|| ((!(factory->flags & DR_NAMESPACE)) && (*mailbox == '#'))) {
      continue;
    } else if ((*factory->valid)(mailbox,stream)) {
      break;
    }
  }

  /* must match stream if not dummy */
  if (factory && stream && (stream->dtb != factory) ) {
    if (purpose != NULL && strcmp(purpose, "delete mailbox" ) != 0) { 

      /* If we are not deleting a mailbox then check if it is opened */
      if (stream->stream_status & S_OPENED) {
	factory = strcmp(factory->name, "dummy") ? NULL : stream->dtb;
      }
    }
  }

  if (!factory && purpose) {	/* if want an error message */
    sprintf(tmp,"Can't %s %s: %s", purpose, mailbox,
	    (*mailbox == '{')
	    ? "invalid remote specification"
	    : "no such mailbox");
    mm_log(tmp, ERROR, stream);
  }
  return(factory);		/* return driver factory */
}

/*
 * Mail validate network mailbox name
 * Accepts: mailbox name
 *	    mailbox driver to validate against
 *	    pointer to where to return host name if non-NIL
 *	    pointer to where to return mailbox name if non-NIL
 * Returns: driver on success, NIL on failure
 */
DRIVER *
mail_valid_net(char * name, DRIVER * drv, char * host, char * mailbox)
{
  NETMBX	mb;

  if (!mail_valid_net_parse(name, &mb)
      || (strcmp(mb.service, "imap")
	  ? strcmp (mb.service,drv->name) : strncmp (drv->name,"imap",4))) {
    return(NULL);
  }
  if (host != NULL) {
    strcpy(host, mb.host);
  }
  if (mailbox != NULL) {
    strcpy(mailbox, mb.mailbox);
  }
  return(drv);
}


/* Mail validate network mailbox name
 * Accepts: mailbox name
 *	    NETMBX structure to return values
 * Returns: T on success, NIL on failure
 */

long mail_valid_net_parse (char *name,NETMBX *mb)
{
  int i;
  char c,*s,*t,*v;
  mb->port = 0;			/* initialize structure */
  *mb->host = *mb->mailbox = *mb->service = '\0';
  /* init flags */
  mb->anoflag = mb->dbgflag = NIL;
  /* have host specification? */
  if (!(*name == '{' && (t = strchr (s = name+1,'}')) && (i = t - s)))
    return NIL;			/* not valid host specification */
  strncpy (mb->host,s,i);	/* set host name */
  mb->host[i] = '\0';		/* tie it off */
  strcpy (mb->mailbox,t+1);	/* set mailbox name */
  /* any switches or port specification? */
  if (t = strpbrk (mb->host,"/:")) {
    c = *t;			/* yes, remember delimiter */
    *t++ = '\0';		/* tie off host name */
    lcase (t);			/* coerce remaining stuff to lowercase */
    do switch (c) {		/* act based upon the character */
    case ':':			/* port specification */
      if (mb->port || !(mb->port = strtoul (t,&t,10))) return NIL;
      c = t ? *t++ : '\0';	/* get delimiter, advance pointer */
      break;
      
    case '/':			/* switch */
      /* find delimiter */
      if (t = strpbrk (s = t,"/:=")) {
	c = *t;			/* remember delimiter for later */
	*t++ = '\0';		/* tie off switch name */
      }
      else c = '\0';		/* no delimiter */
      if (c == '=') {		/* parse switches which take arguments */
	if (t = strpbrk (v = t,"/:")) {
	  c = *t;		/* remember delimiter for later */
	  *t++ = '\0';		/* tie off switch name */
	}
	else c = '\0';		/* no delimiter */
	if (!strcmp (s,"service")) {
	  if (*mb->service) return NIL;
	  else strcpy (mb->service,v);
	}
	else return NIL;	/* invalid argument switch */
      }
      else {			/* non-argument switch */
	if (!strcmp (s,"anonymous")) mb->anoflag = T;
	else if (!strcmp (s,"debug")) mb->dbgflag = T;
	else if (!strcmp (s,"imap") || !strcmp (s,"imap2") ||
		 !strcmp (s,"imap4") || !strcmp (s,"pop3") ||
		 !strcmp (s,"nntp")) {
	  if (*mb->service) return NIL;
	  strcpy (mb->service,
		  (!strcmp (s,"imap2") || !strcmp (s,"imap4")) ? "imap" : s);
	} 
	else return NIL;	/* invalid non-argument switch */
      }
      break;
    default:			/* anything else is bogus */
      return NIL;
    } while (c);		/* see if anything more to parse */
  }
  /* default mailbox name */
  if (!*mb->mailbox) strcpy (mb->mailbox,"INBOX");
  /* default service name */
  if (!*mb->service) strcpy (mb->service,"imap");
  return T;
}

/*
 * Sun Imap4: Holdover from imap2bis. 
 */

void mail_find_all_local(MAILSTREAM *stream, char *pat)
{
  mail_list (stream, NIL, pat);
}

/* Mail scan mailboxes for string
 * Accepts: mail stream
 *	    reference
 *	    pattern to search
 *	    contents to search
 */

void mail_scan (MAILSTREAM *stream,char *ref,char *pat,char *contents)
{
  int remote = ((*pat == '{') || (ref && *ref == '{'));
  int local = !remote;
  int namespace = ((*pat == '#') || (ref && *ref == '#'));
  DRIVER *d;
  unsigned long flags;
  unsigned long driver_local;
  unsigned long driver_remote;
  if (*pat == '{') ref = NIL;	/* ignore reference if pattern is remote */
  if (stream && (stream->stream_status & S_OPENED) && stream->dtb) {	
    /* if have a stream, do it for that stream */
    /* stream_status bit checked ensuring a valid stream */
    d = stream->dtb;
    flags = d->flags;
    driver_local = flags & DR_LOCAL;
    driver_remote = !driver_local;
    if ((driver_local && local) ||     /* (local-driver && local-mbox) */
	(driver_remote && remote) ||   /* (remote-driver && remote-mbox) */
	((flags & DR_NAMESPACE) && namespace)) {
      (*d->scan)(stream,ref,pat,contents);
    }
    return;
  }
  /* otherwise do for all DTB's */
  for (d = maildrivers; d; d = d->next) {
    flags = d->flags;
    driver_local = flags & DR_LOCAL;
    driver_remote = !driver_local;
    if ((flags & DR_NOSTREAM) && /* allows non-stream ops */
	((driver_remote && remote) ||
	 (driver_local && local) ||
	 ((flags & DR_NAMESPACE) && namespace))) {
      (d->scan)(stream,ref,pat,contents); /* else stream_status should be NIL.*/  
    }
  }
}

/* Mail list mailboxes
 * Accepts: mail stream
 *	    reference
 *	    pattern to search
 */

void mail_list (MAILSTREAM *stream,char *ref,char *pat)
{
  int remote = ((*pat == '{') || (ref && *ref == '{'));
  int local = !remote;
  int namespace = ((*pat == '#') || (ref && *ref == '#'));
  DRIVER *d;
  unsigned long flags;
  unsigned long driver_local;
  unsigned long driver_remote;
  if (*pat == '{') ref = NIL;	/* ignore reference if pattern is remote */

  if (stream && (stream->stream_status & S_OPENED) && stream->dtb) {	
    /* if have a stream, do it for that stream */
    d = stream->dtb;
    flags = d->flags;
    driver_local = flags & DR_LOCAL;
    driver_remote = !driver_local;
    if ((driver_local && local) ||     /* (local-driver && local-mbox) */
	(driver_remote && remote) ||   /* (remote-driver && remote-mbox) */
	((flags & DR_NAMESPACE) && namespace)) {
      (*d->list) (stream,ref,pat);
    }
    return;
  }
  /* otherwise do for all DTB's */
  for (d = maildrivers; d; d = d->next) {
    flags = d->flags;
    driver_local = flags & DR_LOCAL;
    driver_remote = !driver_local;
    if ((flags & DR_NOSTREAM) && /* allows non-stream ops */
	((driver_remote && remote) ||
	 (driver_local && local) ||
	 ((flags & DR_NAMESPACE) && namespace))) {
      (d->list) (stream,ref,pat);  /* If we get to here, stream_status is
				      not S_OPENED.*/
    }
  }
}


/* Mail list subscribed mailboxes
 * Accepts: mail stream
 *	    pattern to search
 */

void mail_lsub (MAILSTREAM *stream,char *ref,char *pat)
{
  int remote = ((*pat == '{') || (ref && *ref == '{'));
  int local = !remote;
  int namespace = ((*pat == '#') || (ref && *ref == '#'));
  DRIVER *d;
  unsigned long flags;
  unsigned long driver_local;
  unsigned long driver_remote;
  if (*pat == '{') ref = NIL;	/* ignore reference if pattern is remote */
			
  if (stream && ( stream->stream_status & S_OPENED) && stream->dtb) 
    {	/* if stream_status bit set, do it for that stream */
      d = stream->dtb;
      flags = d->flags;
      driver_local = flags & DR_LOCAL;
      driver_remote = !driver_local;
      if ((driver_local && local) ||     /* (local-driver && local-mbox) */
	  (driver_remote && remote) ||   /* (remote-driver && remote-mbox) */
	  ((flags & DR_NAMESPACE) && namespace)) {
	(*d->lsub) (stream,ref,pat);
      }
      return;
    }
  /* otherwise do for all DTB's */
  for (d = maildrivers; d; d = d->next) {
    flags = d->flags;
    driver_local = flags & DR_LOCAL;
    driver_remote = !driver_local;
    if ((flags & DR_NOSTREAM) && /* allows non-stream ops */
	((driver_remote && remote) ||
	 (driver_local && local) ||
	 ((flags & DR_NAMESPACE) && namespace))) {
      (d->lsub) (stream,ref,pat); /* else stream_status should be NIL */
    }
  }
}


/*
 * Mail subscribe to mailbox
 * Accepts: mail stream
 *	    mailbox to add to subscription list
 * Returns: _B_TRUE on success, _B_FALSE on failure
 */
long
mail_subscribe(MAILSTREAM * stream, char * mailbox)
{
  DRIVER *	factory;

  factory = mail_valid(stream, mailbox, "subscribe to mailbox");
  return factory ?
    (factory->subscribe ?
     (*factory->subscribe)(stream, mailbox) : sm_subscribe(mailbox, stream)) : _B_FALSE;
}



/*
 * Mail unsubscribe to mailbox
 * Accepts: mail stream
 *	    mailbox to delete from subscription list
 * Returns: _B_TRUE on success, _B_FALSE on failure
 */
long mail_unsubscribe (MAILSTREAM * stream, char * mailbox)
{
  DRIVER *	factory;

  factory = mail_valid(stream, mailbox, "unsubscribe to mailbox");
  return (factory && factory->unsubscribe) ?
    (*factory->unsubscribe)(stream, mailbox) : sm_unsubscribe(mailbox, stream);
}

/*
 * Mail create mailbox
 * Accepts: mail stream
 *	    mailbox name to create
 * Returns: _B_TRUE on success, _B_FALSE on failure
 */
long
mail_create(MAILSTREAM * stream, char * mailbox)
{
  /*
   * A local mailbox is one that is not qualified as being a remote or a
   * namespace mailbox.  Any remote or namespace mailbox driver must check
   * for itself whether or not the mailbox already exists.
   */
  boolean_t 	remote = (*mailbox == '{') ? _B_TRUE : _B_FALSE;
  boolean_t 	namespace = (*mailbox == '#') ? _B_TRUE : _B_FALSE;
  char 		tmp[MAILTMPLEN];

  /*
   * guess at driver if stream not specified.
   *
   * At least a created string is required 
   */
  if (!stream || !(stream->stream_status & S_CREATED)) {
    return(_B_FALSE);
  }

  if (!(stream->stream_status & S_OPENED)) {

   /* Have a CREATED by NOT OPEN stream */
    if (remote || namespace) {
      stream =  mail_open(stream, mailbox, OP_PROTOTYPE);
    } else {
      stream->dtb = default_proto(stream)->dtb;
      stream->stream_status |= S_OPENED;
      /* Then we a stream that's equivalent to OPENED. */
      
      if (!(stream->stream_status & S_OPENED)) {
	sprintf(tmp, "Can't create mailbox %s: indeterminate format", mailbox);
	mm_log(tmp,ERROR,stream);
	return(_B_FALSE);
      }
    }
  }

  /* must not already exist if local */
  if (!remote && mail_valid(stream, mailbox, NULL)) {
    sprintf(tmp, "Can't create mailbox %s: mailbox already exists", mailbox);
    mm_log(tmp, ERROR, stream);
    return(_B_FALSE);
  }

  return(stream->dtb ? (*stream->dtb->create) (stream,mailbox) : _B_FALSE);
}

/*
 * Mail delete mailbox
 * Accepts: mail stream
 *	    mailbox name to delete
 * Returns: _B_TRUE on success, _B_FALSE on failure
 */
long 
mail_delete(MAILSTREAM * stream, char * mailbox)
{
  DRIVER 	*	factory;
  char 			tmp[MAILTMPLEN];

  factory = mail_valid(stream, mailbox, "delete mailbox");
  if ((*mailbox != '{')
      && (*mailbox != '#')
      && stream
      && ( stream->stream_status & S_OPENED ) ) {

    dummy_file(tmp, mailbox, stream);
    if (strcasecmp(stream->mailbox, tmp) == 0) {
      sprintf (tmp,"Can't delete open mailbox %s: ",mailbox);
      mm_log (tmp,ERROR,stream);
      return(_B_FALSE);
    }
  }
  return (factory ? (*factory->mbxdel)(stream, mailbox) : _B_FALSE);
}


/* Mail rename mailbox
 * Accepts: mail stream
 *	    old mailbox name
 *	    new mailbox name
 * Returns: T on success, NIL on failure
 */

long mail_rename (MAILSTREAM *stream,char *old,char *newname)
{
  char tmp[MAILTMPLEN];
  DRIVER *factory;

  factory = mail_valid(stream,old,"rename mailbox");
  if ((*old != '{') && (*old != '#')) {
    /* set stream_status to NIL and pass it to mail_valid */
    mail_stream_setNIL(stream);
    if (mail_valid(stream,newname, NULL)) {
      sprintf (tmp,"Can't rename to mailbox %s: mailbox already exists",newname);
      mm_log (tmp,ERROR,stream);
      mail_stream_unsetNIL(stream);
      return NIL;
    } else {
      mail_stream_unsetNIL(stream);
      dummy_file(tmp,old, stream); 
      /* make sure stream->mailbox is non-nil  */
      if (stream && (stream->stream_status & S_OPENED ) && 
	  stream->mailbox && strcasecmp(stream->mailbox,tmp) == 0) {
	sprintf (tmp,"Can't rename open mailbox %s: ",old);
	mm_log (tmp,ERROR,stream);
	return NIL;
      }
    }
  }
  mail_stream_unsetNIL(stream);
  /* Be sure the bit is unset */
  return factory ? (*factory->rename) (stream,old,newname) : NIL;
}
/* Mail status of mailbox
 * Accepts: mail stream
 *	    mailbox name
 *	    status flags
 * Returns: T on success, NIL on failure
 */

long mail_status (MAILSTREAM *stream,char *mbx,long flags)
{
  MAILSTATUS status;
  unsigned long i;
  MAILSTREAM *tstream = NIL;
  char tmp[MAILTMPLEN];
  DRIVER *factory = mail_valid(stream,mbx,"get status of mailbox");

  if (!factory) return NIL;	/* bad name */
  /* use driver's routine if have one */
  if (factory->status) return (*factory->status) 
			 (stream,mbx,flags,stream->userdata);
  /* canonicalize mbx */
  dummy_file (tmp, mbx, stream);
  mbx = tmp;
  /* make temporary stream (unless this mbx) */
  /* We create a tstream here, pass it onto mail_open and
     later on close it.  */
  if (stream && (stream->stream_status & S_OPENED) && 
      !strcmp (mbx,stream->mailbox)) 
    tstream = stream;
  else {
    tstream = mail_stream_create(stream->userdata,
				 NIL,NIL,NIL,NIL);
    tstream = mail_open (tstream,mbx,OP_READONLY|OP_SILENT);
    if (!(tstream->stream_status & S_OPENED) ) {
      mail_stream_flush(stream); /* Should free it */	
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
    if (tstream->dtb->checksum) {
      if (stream && (stream->stream_status & S_OPENED )
	  && tstream == stream)
	tstream->checksum_type = stream->checksum_type;
      else
	tstream->checksum_type = OPENCHECKSUM;
      tstream->checksum_type |= STATUSCHECKSUM;
      (*tstream->dtb->checksum)(tstream);
      tstream->checksum_type &= ~STATUSCHECKSUM;
      status.checksum = tstream->checksum;
      status.checksum_bytes = tstream->mbox_size;
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


/* Mail open
 * Accepts: candidate stream for recycling
 *	    mailbox name
 *	    open options
 *      User functionality required on the stream
 *          user data    user specific data
 * Returns: stream to use on success, NIL on failure
 */

MAILSTREAM *mail_open (MAILSTREAM *stream,char *name,long options)
{
  DRIVER *factory;
  MAIL_GLOBALS *mg;

  /* We should normally have a stream when we get to here.
     But a NIL stream is allowed IFF OP_PROTOTYPE is passed.
     */

  if (!stream) {
    stream = mail_stream_create(NIL,NIL,NIL,NIL,NIL);
    if (options & OP_PROTOTYPE)
      stream->stream_status |= S_PROTOTYPE;
    else {                               /* bug halt */
      mm_log("Open called with illegal NIL stream!", ERROR, stream);
      return(NULL);
    }
  }
  /* Do the casting IFF we have a stream; else it dumps core. */
  mg = (MAIL_GLOBALS *)stream->mail_globals;

  factory = mail_valid (stream,name,(options & OP_SILENT) ?
			(char *) NIL : "open mailbox");

  if (factory) {		/* must have a factory */
    if (!(stream->stream_status & S_OPENED ))  { 
      /* instantiate stream if none to recycle */
      if (options & OP_PROTOTYPE) return (*factory->open) (stream);
      /* initialize stream */
      /* Fill in the rest of data in stream */
      stream->dtb = factory;	/* set dispatch */
      /* set mailbox name */
      /* We can use the old stream but we need to flush old mailbox.*/
      if (stream->mailbox ) fs_give ( ( void ** ) &stream->mailbox );
      stream->mailbox = cpystr (name);
      /* initialize cache */
      (*mailcache) (stream,(long) 0,CH_INIT);
      /* default UID validity */
      stream->uid_validity = time (0);
      stream->uid_last = 0;
      /* Sun Imap4: We do these only on a new stream since a viable stream
       * may be checkpointed if this is the record factory,
       * and this is symbolic since the stream is Zeroed above.*/
      stream->validchecksum = NIL;/* checksum is not yet valid */
      stream->sunvset = _B_FALSE;  /* sun version not yet on this stream */
      /* initialize cache */
      stream->mail_send_urgent = 0; /* TRUE if mail_%s allows urgent signals */
      stream->imap_send_urgent = 0; /* Only invoked by imap. enables urgent */
      stream->have_all_sinfo = 0;   /* True if all shortinfo cached(used by */
      /* mail_simple_search(.) mail_search(.) */
      stream->dead = NIL;      
      stream->stream_status |= S_OPENED ; /* set status bit to S_OPENED. */
      /* 
       * See if we keep our mime translations */
      if (options & OP_KEEP_MIME)
	stream->keep_mime = T;    
      else
	stream->keep_mime = NIL;
    } else {			/* close driver if different from factory */
      char *tmp_name;		/* Sun Imap4  */
      if (stream->dtb != factory) {
	if (stream->dtb) (*stream->dtb->close) (stream,NIL);
	stream->dtb = factory;/* establish factory as our driver */
	mail_free_cache (stream);
	/* reset UID validity */
	stream->uid_validity = time (0);
	stream->uid_last = 0;
	/* Note: stream->sunvset is left viable across opens.
	 *       It is a client property */
	stream->local = NIL;	/* old driver's local data now flushed */
	stream->dtb = factory;	/* establish factory as our driver */
      }
      /* clean up old mailbox name for recycling */
      /* clean up old mailbox name for recycling */
      tmp_name = cpystr(name);
      if (stream->mailbox) fs_give ((void **) &stream->mailbox);
      stream->mailbox = tmp_name;
    }
    stream->lock = NIL;		/* initialize lock and options */
    stream->debug = (options & OP_DEBUG) ? _B_TRUE : _B_FALSE;
    stream->rdonly = (options & OP_READONLY) ? _B_TRUE : _B_FALSE;
    stream->anonymous = (options & OP_ANONYMOUS) ? _B_TRUE : _B_FALSE;
    stream->scache = (options & OP_SHORTCACHE) ? _B_TRUE : _B_FALSE;
    stream->silent = (options & OP_SILENT) ? _B_TRUE : _B_FALSE;
    stream->halfopen = (options & OP_HALFOPEN) ? _B_TRUE : _B_FALSE;
    stream->hierarchy_dlm = 0;

    /* Server opening? */
    stream->server_open = (options & OP_SERVEROPEN) ? _B_TRUE : _B_FALSE;
    /* Set client session lockf flag */
    if (factory->flags & DR_LOCAL)
      stream->session_lockf = (options & OP_SESSIONLOCKF ? _B_TRUE : _B_FALSE);
    else stream->session_lockf = NIL;
    /*  Tell open to quit immediately is session lock busy */
    stream->nokodsig = (options & OP_NOKODSIGNAL ? _B_TRUE : _B_FALSE);
    stream->nousersig = (options & OP_NOUSERSIGS ? _B_TRUE : _B_FALSE);
    stream->rcvkor = NIL;		/* kiss of read only signal flag */
    stream->dead = NIL;			/* NO longer dead */

    /*
     * Only turn OFF recording mode if open succeeds.
     * This will allow record resume to work 
     */
    stream->secondary_mbox = _B_FALSE;

    if (strcasecmp("INBOX", name) == 0) {
      stream->secondary_mbox = _B_FALSE;
    } else {
      if (name[0] == '{') {
	const char	* endBrace = strchr(name+1, '}');
	if (endBrace != NULL) {
	  if (strcasecmp("INBOX", endBrace + 1) == 0) {
	    stream->secondary_mbox = _B_FALSE;
	  }
	}
      }
    }
    stream->sunvset = (options & OP_SUNVERSION ? _B_TRUE : _B_FALSE);
    /* have driver open, flush if failed */
    if (!(*factory->open) (stream)) {
      stream->stream_status &= ~S_OPENED; 
      /* We can't close the stream here even though it's an
	 invalid mailbox. Again we need to maintain the globals. */
    }
  }
  /* We just return the stream regardless. If it's invalid,
     it's indicated by the stream_status bit. We can't return 
     a NIL here because we don't want to lose the globals. */ 

  return stream;		/* return the stream */
}

/* Mail close
 * Accepts: mail stream
 *	    close options
 * Returns: NIL
 */

MAILSTREAM *mail_close_full (MAILSTREAM *stream,long options)
{
  DRIVER *d;

  if (stream) {			/* make sure argument given */
    /* do the driver's close action */
    /* only if the stream is opened */
    if (stream->dtb && (stream->stream_status & S_OPENED )) 
      (*stream->dtb->close) (stream,options);
    if (stream->mailbox) fs_give ((void **) &stream->mailbox);
    stream->sequence++;		/* invalidate sequence */
    if (stream->flagstring) fs_give ((void **) &stream->flagstring);

    /* give up all the globals */
    env_global_free(stream);
    mail_global_free(stream);
    tcp_global_free(stream);

    /* Flush the driver globals  */
    for (d = maildrivers; d; d = d->next)
      (d->global_free)(stream);

    mail_free_cache (stream);	/* finally free the stream's storage */
    if (!stream->use) fs_give ((void **) &stream);
  }
  return NIL;
}

/* Mail make handle
 * Accepts: mail stream
 * Returns: handle
 *
 *  Handles provide a way to have multiple pointers to a stream yet allow the
 * stream's owner to nuke it or recycle it.
 */

MAILHANDLE *mail_makehandle (MAILSTREAM *stream)
{
  MAILHANDLE *handle = (MAILHANDLE *) fs_get (sizeof (MAILHANDLE));
  handle->stream = stream;	/* copy stream */
  /* and its sequence */
  handle->sequence = stream->sequence;
  stream->use++;		/* let stream know another handle exists */
  return handle;
}


/* Mail release handle
 * Accepts: Mail handle
 */

void mail_free_handle (MAILHANDLE **handle)
{
  MAILSTREAM *s;
  if (*handle) {		/* only free if exists */
    /* resign stream, flush unreferenced zombies */
    if ((!--(s = (*handle)->stream)->use) && !s->dtb) fs_give ((void **) &s);
    fs_give ((void **) handle);	/* now flush the handle */
  }
}


/* Mail get stream handle
 * Accepts: Mail handle
 * Returns: mail stream or NIL if stream gone
 */

MAILSTREAM *mail_stream (MAILHANDLE *handle)
{
  MAILSTREAM *s = handle->stream;
  return (s->dtb && (handle->sequence == s->sequence)) ? s : NIL;
}

/*
 * Mail fetch long cache element
 * Accepts: mail stream
 *	    message # to fetch
 * Returns: long cache element of this message
 * Can also be used to create cache elements for new messages.
 */
LONGCACHE *
mail_lelt(MAILSTREAM * stream, unsigned long msgno)
{
  if (stream->scache) {
    mm_log("Short cache in mail_lelt",ERROR, stream);
    return(NULL);
  }
  /* be sure it the cache is large enough */
  (*mailcache)(stream, msgno, CH_SIZE);
  return (LONGCACHE *) (*mailcache)(stream, msgno, CH_MAKELELT);
}


/*
 * Mail fetch cache element
 * Accepts: mail stream
 *	    message # to fetch
 * Returns: cache element of this message
 * Can also be used to create cache elements for new messages.
 */
MESSAGECACHE *
mail_elt(MAILSTREAM * stream, unsigned long msgno)
{
  if (msgno < 1) {
    mm_log("Bad msgno in mail_elt",ERROR, stream);
    return(NULL);
  }
  /* be sure the cache is large enough */
  (*mailcache) (stream,msgno,CH_SIZE);
  return (MESSAGECACHE *) (*mailcache) (stream,msgno,CH_MAKEELT);
}

/* Mail fetch fast information
 * Accepts: mail stream
 *	    sequence
 *	    option flags
 *
 * Generally, mail_fetchstructure_full is preferred
 */

void mail_fetchfast_full (MAILSTREAM *stream,char *sequence,long flags)
{
  /* do the driver's action */
  if (stream->dtb) (*stream->dtb->fetchfast) (stream,sequence,flags);
}


/* Mail fetch flags
 * Accepts: mail stream
 *	    sequence
 *	    option flags
 */

void mail_fetchflags_full (MAILSTREAM *stream,char *sequence,long flags)
{
  /* do the driver's action */
  if (stream->dtb) (*stream->dtb->fetchflags) (stream,sequence,flags);
}


/*
 * Mail fetch message structure
 * Accepts: mail stream
 *	    message # to fetch
 *	    pointer to return body
 *	    option flags
 * Returns: envelope of this message, body returned in body value
 *
 * Fetches the "fast" information as well
 */
ENVELOPE *
mail_fetchstructure_full(MAILSTREAM 	*  stream,
			 unsigned long 	   msgno,
			 BODY 		** body,
			 long 		   flags)
{
  if ((msgno < 1 || msgno > stream->nmsgs) && !(flags & FT_UID)) {
    mm_log("Bad msgno in mail_fetchstructure_full", ERROR, stream);
    return(NULL);
  }
  return stream->dtb ?		/* do the driver's action */
    (*stream->dtb->fetchstructure) (stream,msgno,body,flags) : NULL;
}

/*
 * Mail fetch envelope_full:
 *  like mail_fetchstructure_full except that it avoids a
 *  full body parse for envelope information
 */
ENVELOPE *
mail_fetchenvelope_full(MAILSTREAM 	* stream,
			unsigned long 	  msgno,
			long 		  flags)
{
  if ((msgno < 1 || msgno > stream->nmsgs) && !(flags & FT_UID)) {
    mm_log("Bad msgno in mail_fetchenvelope_full", ERROR, stream);
    return(NULL);
  }

  if (stream->dtb && stream->dtb->fetchenvelope) {
    return (*stream->dtb->fetchenvelope)(stream,msgno,flags);
  } else {
    return(NULL);
  }
}


/*
 * Mail fetch message header
 * Accepts: mail stream
 *	    message # to fetch
 *	    list of lines to fetch
 *	    pointer to returned length
 *	    flags
 * Returns: message header in RFC822 format
 *
 * mail_simple_fetchheader() is now identical to this call.
 */
char *
mail_fetchheader_full(MAILSTREAM    * stream,
		      unsigned long   msgno,
		      STRINGLIST    * lines,
		      unsigned long * len,
		      long 	      flags)
{
  char *	empty = "";

  if ((msgno < 1 || msgno > stream->nmsgs) && !(flags & FT_UID)) {
    mm_log("Bad msgno in mail_fetchheader",ERROR, stream);
    return(NULL);
  }
  if (flags && (flags & FT_ZEROCOUNT)) {
    (*stream->dtb->cleario)(stream);
  }
  return stream->dtb ?		/* do the driver's action */
    (*stream->dtb->fetchheader)(stream, msgno, lines, len, flags) : empty;
}

/* Mail fullheader cached
 * Accepts: mail stream
 *	    message # to check
 * Returns: T if indeed it is or is available locally.
 */

int mail_fullheader_cached(MAILSTREAM *stream, unsigned long msgno)
{
  /* non-local drivers only */
  if (stream->dtb && !(stream->dtb->flags & DR_LOCAL)) {
    MESSAGECACHE *elt = mail_elt (stream, msgno);
    if (elt->data1) return T;
    else return NIL;
  } else
    return T;
} 

/*
 * Mail fetch message text (body only)
 * Accepts: mail stream
 *	    message # to fetch
 *	    pointer to returned length
 *
 *	    flags
 * Returns: message text in RFC822 format
 */
char *
mail_fetchtext_full(MAILSTREAM 	  * stream,
		    unsigned long   msgno,
		    unsigned long * len,
		    long 	    flags)
{
  if ((msgno < 1 || msgno > stream->nmsgs) && !(flags & FT_UID)) {
    mm_log("Bad msgno in mail_fetchtext",ERROR, stream);
    return(NULL);
  }
  /* do the driver's action */
  if (flags && (flags & FT_ZEROCOUNT)) {
    (*stream->dtb->cleario)(stream);
  }
  return stream->dtb ? (*stream->dtb->fetchtext) (stream,msgno,len,flags) : "";
}


/*
 * Mail fetch message body part text
 * Accepts: mail stream
 *	    message # to fetch
 *	    section specifier (#.#.#...#)
 *	    pointer to returned length
 *	    flags
 * Returns: pointer to section of message body
 */
char *
mail_fetchbody_full(MAILSTREAM 	  * stream,
		    unsigned long   msgno,
		    char 	  * sec,
		    unsigned long * len,
		    long 	    flags)
{
  if ((msgno < 1 || msgno > stream->nmsgs) && !(flags & FT_UID)) {
    mm_log("Bad msgno in mail_fetchbody", ERROR, stream);
    return(NULL);
  }

  stream->mail_send_urgent = T;	/* can interrupt with URGENT o-of-b data */
  /* do the driver's action */
  if (flags && (flags & FT_ZEROCOUNT)) (*stream->dtb->cleario)(stream);
  return stream->dtb ?
    (*stream->dtb->fetchbody) (stream,msgno,sec,len,flags) : "";
}

/*
 * Mail fetch UID
 * Accepts: mail stream
 *	    message number
 * Returns: UID
 */
unsigned long 
mail_uid(MAILSTREAM * stream, unsigned long msgno)
{
  if (!stream->dtb) {
    return(0);	/* error if dead stream */
  }
  if (msgno < 1 || msgno > stream->nmsgs) {
    mm_log("Bad msgno in mail_uid",ERROR, stream);
    return(0);
  }
  /* do the driver's action */
  return stream->dtb->uid ? (*stream->dtb->uid) (stream,msgno) :
    mail_elt (stream,msgno)->uid;
}
/* Mail fetch msgno
 * Accepts: mail stream
 *	    UID
 * Returns: msgno or 0
 */

unsigned long mail_msgno (MAILSTREAM *stream,unsigned long uid)
{
  long i;

  if (!stream->dtb) return 0;	/* error if dead stream */
  for (i = 1; i <= stream->nmsgs; ++i)
    if (mail_elt(stream,i)->uid == uid) return i;
  return 0;
}


/* Mail fetch From string for menu
 * Accepts: destination string
 *	    mail stream
 *	    message # to fetch
 *	    desired string length
 * Returns: string of requested length
 */

void mail_fetchfrom (char *s,MAILSTREAM *stream,unsigned long msgno,
		     long length)
{
  char *t;
  char tmp[MAILTMPLEN];
  ENVELOPE *env = mail_fetchenvelope (stream,msgno);
  ADDRESS *adr = env ? env->from : NIL;
  memset (s,' ',(size_t)length);/* fill it with spaces */
  s[length] = '\0';		/* tie off with null */
  /* get first from address from envelope */
  while (adr && !adr->host) adr = adr->next;
  if (adr) {			/* if a personal name exists use it */
    if (!(t = adr->personal)) sprintf (t = tmp,"%s@%s",adr->mailbox,adr->host);
    memcpy (s,t,(size_t) min (length,(long) strlen (t)));
  }
}


/* Mail fetch Subject string for menu
 * Accepts: destination string
 *	    mail stream
 *	    message # to fetch
 *	    desired string length
 * Returns: string of no more than requested length
 * USE SHORTINFO
 */

void mail_fetchsubject (char *s,MAILSTREAM *stream,unsigned long msgno,
			long length)
{
  SHORTINFO *sinfo = &mail_lelt(stream,msgno)->sinfo;
  ENVELOPE *env = mail_fetchenvelope (stream,msgno);
  char *subject = sinfo->subject ? sinfo->subject : env->subject;
  memset (s,'\0',(size_t) length+1);
  /* copy subject from envelope */
  if (subject && *subject != '\0') strncpy (s,subject,(size_t) length);
  else *s = ' ';		/* if no subject then just a space */
}

/* Mail set flag
 * Accepts: mail stream
 *	    sequence
 *	    flag(s)
 *	    option flags
 */

void mail_setflag_full (MAILSTREAM *stream,char *sequence,char *flag,
			long flags)
{
  /* do the driver's action */
  if (stream->dtb) (*stream->dtb->setflag) (stream,sequence,flag,flags);
}


/* Mail clear flag
 * Accepts: mail stream
 *	    sequence
 *	    flag(s)
 *	    option flags
 */

void mail_clearflag_full (MAILSTREAM *stream,char *sequence,char *flag,
			  long flags)
{
  /* do the driver's action */
  if (stream->dtb) (*stream->dtb->clearflag) (stream,sequence,flag,flags);
}
/*
 * determine if the search can be done out of the local cache, ie,
 * sinfo information. 
 */
int valid_local_search (SEARCHPGM *pgm) {
  /* If we have all of the short info cached, then
   * we can do all BUT the below locally. 
   */
  SEARCHPGMLIST *not;
  if (pgm->keyword ||
      pgm->unkeyword ||
      pgm->sentbefore || 
      pgm->senton ||
      pgm->sentsince ||
      pgm->bcc ||
      pgm->cc ||
      pgm->to ||
      pgm->header ||
      pgm->text ||
      pgm->body)
    return NIL;
  /* See if we have any OR programs */
  if (pgm->or &&
      !(valid_local_search (pgm->or->first) ||
	valid_local_search (pgm->or->second)))
    return NIL;
  if (not = pgm->not) {
    do if (!valid_local_search (not->pgm))
      return NIL;
    while (not = not->next);
  }
  return T;
}
/*
 * Check here for searching FROM the local cache rather than
 * using the imapd. If we do NOT have ALL short info, then only
 * search "ALL" is permitted.
 */
int mail_local_search(MAILSTREAM *stream, SEARCHPGM *pgm)
{
  int local_ok;
  if (!pgm) return NIL;
  else if (!stream->have_all_sinfo)
    if (pgm->only_all) return T;
    else return NIL;
  /* OK, we have all of the short info */
  local_ok = valid_local_search (pgm);
  return local_ok;
}

/* BEGIN - Mail simple search for messages
 * Accepts: mail stream
 *	    search criteria
 * Does NOT automatically return flags, envelopes, etc ...
 * Also, will do search from local cache if possible.
 */

void mail_simple_search (MAILSTREAM *stream, char *criteria, char *charset,
			 long flags)
{
  long i;
  SEARCHPGM *pgm;
  /* reset for the results */
  for (i = 1; i <= stream->nmsgs; ++i) mail_elt (stream,i)->searched = NIL;
  if (stream->scache) 
    stream->have_all_sinfo = NIL;
  else {
    /* have a long cache - see if all shortinfo is present */
    stream->have_all_sinfo = T;
    for (i = 1; i <= stream->nmsgs; i++) {
      LONGCACHE *lelt = mail_lelt(stream, i);

      if (lelt->sinfo.date == NIL) {
	stream->have_all_sinfo = NIL;
	break;
      }
    }
  }
  /*
   * Now check to see if we can do a simple/local search with the criteria:
   */
  pgm = mail_criteria(criteria, stream);
  if (!pgm) return;			/* bad criteria */
  if (!stream->dtb->search || mail_local_search (stream, pgm)) {
    for (i = 1; i <= stream->nmsgs; ++i)
      if (mail_search_msg (stream,i,charset,pgm)) {
	if (flags & SE_UID) mm_searched (stream,mail_uid (stream,i));
	else {			/* mark as searched, notify mail program */
	  mail_elt (stream,i)->searched = T;
	  mm_searched (stream,i);
	}
      }
  } else if (stream->dtb->search)
    (*stream->dtb->search)(stream,charset,pgm,flags|SE_NOPREFETCH);
  if (flags & SE_FREE) mail_free_searchpgm (&pgm);  
}

/* Mail search for messages
 * Accepts: mail stream
 *	    character set
 *	    search program
 *	    option flags
 */

void mail_search_full (MAILSTREAM *stream,char *charset,SEARCHPGM *pgm,
		       long flags)
{
  long i;
  unsigned long time0,time1;    /* For server busy ... */
  /* clear search vector */
  for (i = 1; i <= stream->nmsgs; ++i) mail_elt (stream,i)->searched = NIL;
  /* can we do this from our sinfo cache? */
  if (pgm && stream->dtb) {	/* must have a search program and driver */
    if (stream->scache)  stream->have_all_sinfo = NIL;
    else {
      /* have a long cache - see if all shortinfo is present */
      stream->have_all_sinfo = T;
      for (i = 1; i <= stream->nmsgs; i++) {
	LONGCACHE *lelt = mail_lelt(stream, i);
	if (lelt->sinfo.date == NIL) {
	  stream->have_all_sinfo = NIL;
	  break;
	}
      }
    }
    /* If no driver search supplied [local file rather than imap4],
     * or the search can be done out of our local
     * cache, then do it from the cache */
    if (!stream->dtb->search || mail_local_search (stream, pgm)) {
      unsigned long time0,time1;    /* For server busy ... */
      if (flags & SE_SERVER) time0 = elapsed_ms();
      for (i = 1; i <= stream->nmsgs; ++i) {
	if (mail_search_msg (stream,i,charset,pgm)) {
	  if (flags & SE_UID) mm_searched (stream,mail_uid (stream,i));
	  else {		/* mark as searched, notify mail program */
	    mail_elt (stream,i)->searched = T;
	    mm_searched (stream,i);
	  }
	}
	/* Do we ACK the client */
	if (flags & SE_SERVER) {
	  time1 = elapsed_ms() - time0;
	  if (time1 >= ACKTIME) {
	    time0 = elapsed_ms();
	    mm_log("131 busy[SEARCHING]",PARSE,stream);
	  }
	}
      }
    } else			/* do the driver's action if requested */
      (*stream->dtb->search) (stream,charset,pgm,flags);
  }
  /* flush search program if requested */
  if (flags & SE_FREE) mail_free_searchpgm (&pgm);
}

/* Mail ping mailbox
 * Accepts: mail stream
 * Returns: stream if still open else NIL
 */

long mail_ping (MAILSTREAM *stream)
{
  /* do the driver's action */
  return stream->dtb ? (*stream->dtb->ping) (stream) : NIL;
}


/* Mail check mailbox
 * Accepts: mail stream
 */

void mail_check (MAILSTREAM *stream)
{
  /* do the driver's action */
  if (stream->dtb) (*stream->dtb->check) (stream);
}


/* Mail expunge mailbox
 * Accepts: mail stream
 */

void mail_expunge (MAILSTREAM *stream)
{
  mail_check(stream);	/* Look for others changing */

  /* do the driver's action */
  if (stream->dtb) (*stream->dtb->expunge) (stream);
}

/* Mail copy message(s)
 * Accepts: mail stream
 *	    sequence
 *	    destination mailbox
 *	    flags
 */

long mail_copy_full (MAILSTREAM *stream,char *sequence,char *mailbox,
		     long options)
{
  if (!sequence) return NIL;
  return stream->dtb ?		/* do the driver's action */
    (*stream->dtb->copy) (stream,sequence,mailbox,options) : NIL;
}


/* Mail append message string
 * Accepts: mail stream
 *	    destination mailbox
 *	    initial flags
 *	    message internal date
 *	    stringstruct of message to append
 * Returns: T on success, NIL on failure
 */

long
mail_append_full(MAILSTREAM 	* stream,
		 char 		* mailbox,
		 char 		* flags,
		 char 		* date,
		 char		* xUnixFrom,
		 STRING 	* message)
{
  DRIVER	*	factory = mail_valid(stream,mailbox,
					     "append to mailbox");

  if (!factory) {		/* got a driver to use? */

    /* ask default for TRYCREATE if no stream */
    if (stream && !(stream->stream_status & S_OPENED)
	&& default_proto (stream)
	&& (*default_proto(stream)->dtb->append)(stream,
						 mailbox,
						 flags,
						 date,
						 xUnixFrom,
						 message)) {
      /* timing race? */
      mm_notify(stream, "Append validity confusion", WARN);
      return(_B_TRUE);
    }

    /* now generate error message */
    mail_valid(stream, mailbox, "append to mailbox");
    return(_B_FALSE);			/* return failure */
  }

  /*
   * validate stream if given:
   *  IE: The mbox to which the string is to be appended should be
   *  consistent with the stream which has been passed.
   */
  if (stream
      && (stream->stream_status & S_OPENED)
      && stream->dtb && (factory != stream->dtb)) {
    return(_B_FALSE);

  } else {
    /* do the driver's action */
    if (stream && (stream->stream_status & S_OPENED) ) {
      (*stream->dtb->cleario)(stream);
    }
  }
  return (factory->append)(stream, mailbox, flags, date, xUnixFrom, message);
}

/*
 * Mail garbage collect stream
 * Accepts: mail stream
 *	    garbage collection flags
 */
void mail_gc(MAILSTREAM *stream,long gcflags)
{
  unsigned long i = 1;
  LONGCACHE *lelt;

  /* do the driver's action first */
  if (stream->dtb) (*stream->dtb->gc) (stream,gcflags);
  if (gcflags & GC_ENV) {	/* garbage collect envelopes? */
    /* yes, free long cache if in use */
    if (!stream->scache) while (i <= stream->nmsgs)
      if (lelt = (LONGCACHE *) (*mailcache) (stream,i++,CH_LELT)) {
	mail_free_envelope (&lelt->env);
	mail_free_body (&lelt->body);
      }
    stream->msgno = 0;		/* free this cruft too */
    mail_free_envelope (&stream->env);
    mail_free_body (&stream->body);
  }
  /* free text if any */
  if ((gcflags & GC_TEXTS) && (stream->text)) fs_give ((void **)&stream->text);
}

/* Mail output date from elt fields
 * Accepts: character string to write into
 *	    elt to get data data from
 * Returns: the character string
 */

const char *days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

char *mail_date (char *string,MESSAGECACHE *elt)
{
  const char *s = (elt->month && elt->month < 13) ?
    months[elt->month - 1] : (const char *) "???";
  sprintf (string,"%2d-%s-%d %02d:%02d:%02d %c%02d%02d",
	   elt->day,s,elt->year + BASEYEAR,
	   elt->hours,elt->minutes,elt->seconds,
	   elt->zoccident ? '-' : '+',elt->zhours,elt->zminutes);
  return string;
}


/* Mail output cdate format date from elt fields
 * Accepts: character string to write into
 *	    elt to get data data from
 * Returns: the character string
 */

char *mail_cdate (char *string,MESSAGECACHE *elt)
{
  const char *s = (elt->month && elt->month < 13) ?
    months[elt->month - 1] : (const char *) "???";
  int m = elt->month;
  int y = elt->year + BASEYEAR;
  if (elt->month <= 2) {	/* if before March, */
    m = elt->month + 9;		/* January = month 10 of previous year */
    y--;
  }
  else m = elt->month - 3;	/* March is month 0 */
  sprintf (string,"%s %s %2d %02d:%02d:%02d %4d\n",
	   days[(int)(elt->day+2+((7+31*m)/12)+y+(y/4)+(y/400)-(y/100)) % 7],s,
	   elt->day,elt->hours,elt->minutes,elt->seconds,elt->year + BASEYEAR);
  return string;
}

/* Mail parse date into elt fields
 * Accepts: elt to write into
 *	    date string to parse
 * Returns: T if parse successful, else NIL 
 * This routine parses dates as follows:
 * . leading three alphas followed by comma and space are ignored
 * . date accepted in format: mm/dd/yy, mm/dd/yyyy, dd-mmm-yy, dd-mmm-yyyy,
 *    dd mmm yy, dd mmm yyyy
 * . space or end of string required
 * . time accepted in format hh:mm:ss or hh:mm
 * . end of string accepted
 * . timezone accepted: hyphen followed by symbolic timezone, or space
 *    followed by signed numeric timezone or symbolic timezone
 * Examples of normal input:
 * . IMAP date-only (SEARCH): dd-mmm-yy, dd-mmm-yyyy, mm/dd/yy, mm/dd/yyyy
 * . IMAP date-time (INTERNALDATE):
 *    dd-mmm-yy hh:mm:ss-zzz
 *    dd-mmm-yyyy hh:mm:ss +zzzz
 * . RFC-822:
 *    www, dd mmm yy hh:mm:ss zzz
 *    www, dd mmm yyyy hh:mm:ss +zzzz
 */

long mail_parse_date (MESSAGECACHE *elt,char *s)
{
  unsigned long d,m,y;
  int mi,ms;
  struct tm *t, res_gm, res_local;
  time_t tn;
  char tmp[MAILTMPLEN];
  /* make a writeable uppercase copy */
  if (s && *s && (strlen (s) < (size_t)MAILTMPLEN)) s = ucase (strcpy (tmp,s));
  else return NIL;
  /* skip over possible day of week */
  if (isalpha (*s) && isalpha (s[1]) && isalpha (s[2]) && (s[3] == ',') &&
      (s[4] == ' ')) s += 5;
  /* parse first number (probable month) */
  if (!(s && (m = strtoul ((const char *) s,&s,10)))) return NIL;
  
  switch (*s) {			/* different parse based on delimiter */
  case '/':			/* mm/dd/yy format */
    if (!((d = strtoul ((const char *) ++s,&s,10)) && *s == '/' &&
	  (y = strtoul ((const char *) ++s,&s,10)) && *s == '\0')) return NIL;
    break;
  case ' ':			/* dd mmm yy format */
  case '-':			/* dd-mmm-yy format */
    d = m;			/* so the number we got is a day */
    /* make sure string long enough! */
    if (strlen (s) < (size_t) 5) return NIL;
    /* Some compilers don't allow `<<' and/or longs in case statements. */
    /* slurp up the month string */
    ms = ((s[1] - 'A') * 1024) + ((s[2] - 'A') * 32) + (s[3] - 'A');
    switch (ms) {		/* determine the month */
    case (('J'-'A') * 1024) + (('A'-'A') * 32) + ('N'-'A'): m = 1; break;
    case (('F'-'A') * 1024) + (('E'-'A') * 32) + ('B'-'A'): m = 2; break;
    case (('M'-'A') * 1024) + (('A'-'A') * 32) + ('R'-'A'): m = 3; break;
    case (('A'-'A') * 1024) + (('P'-'A') * 32) + ('R'-'A'): m = 4; break;
    case (('M'-'A') * 1024) + (('A'-'A') * 32) + ('Y'-'A'): m = 5; break;
    case (('J'-'A') * 1024) + (('U'-'A') * 32) + ('N'-'A'): m = 6; break;
    case (('J'-'A') * 1024) + (('U'-'A') * 32) + ('L'-'A'): m = 7; break;
    case (('A'-'A') * 1024) + (('U'-'A') * 32) + ('G'-'A'): m = 8; break;
    case (('S'-'A') * 1024) + (('E'-'A') * 32) + ('P'-'A'): m = 9; break;
    case (('O'-'A') * 1024) + (('C'-'A') * 32) + ('T'-'A'): m = 10; break;
    case (('N'-'A') * 1024) + (('O'-'A') * 32) + ('V'-'A'): m = 11; break;
    case (('D'-'A') * 1024) + (('E'-'A') * 32) + ('C'-'A'): m = 12; break;
    default: return NIL;
    }
    if ((s[4] == *s) &&	(y = strtoul ((const char *) s+5,&s,10)) &&
	(*s == '\0' || *s == ' ')) break;
  default: return NIL;		/* unknown date format */
  }
  /* minimal validity check of date */
  if ((d > 31) || (m > 12)) return NIL; 
  /* Tenex/ARPAnet began in 1969 */
  if (y < 100) y += (y >= (BASEYEAR - 1900)) ? 1900 : 2000;
  /* set values in elt */
  elt->day = d; elt->month = m; elt->year = y - BASEYEAR;
  
  if (*s) {			/* time specification present? */
    /* parse time */
    d = strtoul ((const char *) s,&s,10);
    if (*s != ':') return NIL;
    m = strtoul ((const char *) ++s,&s,10);
    y = (*s == ':') ? strtoul ((const char *) ++s,&s,10) : 0;
    /* validity check time */
    if ((d > 23) || (m > 59) || (y > 59)) return NIL; 
    /* set values in elt */
    elt->hours = d; elt->minutes = m; elt->seconds = y;
    switch (*s) {		/* time zone specifier? */
    case ' ':			/* numeric time zone */
      if (!isalpha (s[1])) {	/* treat as '-' case if alphabetic */
	/* test for sign character */
	if ((elt->zoccident = (*++s == '-')) || (*s == '+')) s++;
	/* validate proper timezone */
	if (!(isdigit (*s) && isdigit (s[1]) && isdigit (s[2]) &&
	      isdigit (s[3])) || (s[4] && (s[4] != ' '))) return NIL;
	elt->zhours = (*s - '0') * 10 + (s[1] - '0');
	elt->zminutes = (s[2] - '0') * 10 + (s[3] - '0');
	break;
      }
      /* falls through */
      
    case '-':			/* symbolic time zone */
      if (!(ms = *++s)) return NIL;
      if (*++s) {		/* multi-character? */
	ms -= 'A'; ms *= 1024;	/* yes, make compressed three-byte form */
	ms += ((*s++ - 'A') * 32);
	if (*s) ms += *s++ - 'A';
	if (*s) return NIL;	/* more than three characters */
      }
      /* This is not intended to be a comprehensive list of all possible
       * timezone strings.  Such a list would be impractical.  Rather, this
       * listing is intended to incorporate all military, north American, and
       * a few special cases such as Japan and the major European zone names,
       * such as what might be expected to be found in a Tenex format mailbox
       * and spewed from an IMAP server.  The trend is to migrate to numeric
       * timezones which lack the flavor but also the ambiguity of the names.
       */
      switch (ms) {		/* determine the timezone */
	/* oriental (from Greenwich) timezones */
	/* Middle Europe */
      case (('M'-'A')*1024)+(('E'-'A')*32)+'T'-'A':
      case 'A': elt->zhours = 1; break;
	/* Eastern Europe */
      case (('E'-'A')*1024)+(('E'-'A')*32)+'T'-'A':
      case 'B': elt->zhours = 2; break;
      case 'C': elt->zhours = 3; break;
      case 'D': elt->zhours = 4; break;
      case 'E': elt->zhours = 5; break;
      case 'F': elt->zhours = 6; break;
      case 'G': elt->zhours = 7; break;
      case 'H': elt->zhours = 8; break;
	/* Japan */
      case (('J'-'A')*1024)+(('S'-'A')*32)+'T'-'A':
      case 'I': elt->zhours = 9; break;
      case 'K': elt->zhours = 10; break;
      case 'L': elt->zhours = 11; break;
      case 'M': elt->zhours = 12; break;
	
	/* occidental (from Greenwich) timezones */
      case 'N': elt->zoccident = 1; elt->zhours = 1; break;
      case 'O': elt->zoccident = 1; elt->zhours = 2; break;
      case (('A'-'A')*1024)+(('D'-'A')*32)+'T'-'A':
      case 'P': elt->zoccident = 1; elt->zhours = 3; break;
	/* Atlantic */
      case (('A'-'A')*1024)+(('S'-'A')*32)+'T'-'A':
      case (('E'-'A')*1024)+(('D'-'A')*32)+'T'-'A':
      case 'Q': elt->zoccident = 1; elt->zhours = 4; break;
	/* Eastern */
      case (('E'-'A')*1024)+(('S'-'A')*32)+'T'-'A':
      case (('C'-'A')*1024)+(('D'-'A')*32)+'T'-'A':
      case 'R': elt->zoccident = 1; elt->zhours = 5; break;
	/* Central */
      case (('C'-'A')*1024)+(('S'-'A')*32)+'T'-'A':
      case (('M'-'A')*1024)+(('D'-'A')*32)+'T'-'A':
      case 'S': elt->zoccident = 1; elt->zhours = 6; break;
	/* Mountain */
      case (('M'-'A')*1024)+(('S'-'A')*32)+'T'-'A':
      case (('P'-'A')*1024)+(('D'-'A')*32)+'T'-'A':
      case 'T': elt->zoccident = 1; elt->zhours = 7; break;
	/* Pacific */
      case (('P'-'A')*1024)+(('S'-'A')*32)+'T'-'A':
      case (('Y'-'A')*1024)+(('D'-'A')*32)+'T'-'A':
      case 'U': elt->zoccident = 1; elt->zhours = 8; break;
	/* Yukon */
      case (('Y'-'A')*1024)+(('S'-'A')*32)+'T'-'A':
      case (('H'-'A')*1024)+(('D'-'A')*32)+'T'-'A':
      case 'V': elt->zoccident = 1; elt->zhours = 9; break;
	/* Hawaii */
      case (('H'-'A')*1024)+(('S'-'A')*32)+'T'-'A':
      case (('B'-'A')*1024)+(('D'-'A')*32)+'T'-'A':
      case 'W': elt->zoccident = 1; elt->zhours = 10; break;
	/* Bering */
      case (('B'-'A')*1024)+(('S'-'A')*32)+'T'-'A':
      case 'X': elt->zoccident = 1; elt->zhours = 11; break;
      case 'Y': elt->zoccident = 1; elt->zhours = 12; break;
	/* Universal */
      case (('U'-'A')*1024)+(('T'-'A')*32):
      case (('G'-'A')*1024)+(('M'-'A')*32)+'T'-'A':
      case 'Z': elt->zhours = 0; break;
	
      default:			/* assume local otherwise */
	tn = time (0);		/* time now... MT-safe */
	t = localtime_r(&tn, &res_local);/* get local minutes since midnight */
	mi = t->tm_hour * 60 + t->tm_min;
	ms = t->tm_yday;	/* note Julian day */
	t = gmtime_r(&tn, &res_gm);/* minus UTC minutes since midnight */
	mi -= t->tm_hour * 60 + t->tm_min;
	/* ms can be one of:
	 *  36x  local time is December 31, UTC is January 1, offset -24 hours
	 *    1  local time is 1 day ahead of UTC, offset +24 hours
	 *    0  local time is same day as UTC, no offset
	 *   -1  local time is 1 day behind UTC, offset -24 hours
	 * -36x  local time is January 1, UTC is December 31, offset +24 hours
	 */
	if (ms -= t->tm_yday)	/* correct offset if different Julian day */
	  mi += ((ms < 0) == (abs (ms) == 1)) ? -24*60 : 24*60;
	if (mi < 0) {		/* occidental? */
	  mi = abs (mi);	/* yup, make positive number */
	  elt->zoccident = 1;	/* and note west of UTC */
	}
	elt->zhours = mi / 60;	/* now break into hours and minutes */
	elt->zminutes = mi % 60;
	break;
      }
      elt->zminutes = 0;	/* never a fractional hour */
      break;
    case '\0':			/* no time zone */
      break;
    default:
      return NIL;
    }
  }
  else {			/* make sure all time fields zero */
    elt->hours = elt->minutes = elt->seconds = elt->zhours = elt->zminutes =
      elt->zoccident = 0;
  }
  return T;
}

/* Mail n messages exist
 * Accepts: mail stream
 *	    number of messages
 *
 * Calls external "mm_exists" function that notifies main program prior
 * to updating the stream
 */

void mail_exists (MAILSTREAM *stream,unsigned long nmsgs)
{
  /* make sure cache is large enough */
  (*mailcache) (stream,nmsgs,CH_SIZE);
  /* notify main program of change */
  if (!stream->silent) mm_exists (stream,nmsgs);
  stream->nmsgs = nmsgs;	/* update stream status */
}

/* Mail n messages are recent
 * Accepts: mail stream
 *	    number of recent messages
 */

void mail_recent (MAILSTREAM *stream,unsigned long recent)
{
  stream->recent = recent;	/* update stream status */
}


/* Mail message n is expunged
 * Accepts: mail stream
 *	    message #
 *
 * Calls external "mm_expunged" function that notifies main program prior
 * to updating the stream
 */

void mail_expunged (MAILSTREAM *stream,unsigned long msgno)
{
  MESSAGECACHE *elt = (MESSAGECACHE *) (*mailcache) (stream,msgno,CH_ELT);
  if (elt) {			/* if an element is there */
    elt->msgno = 0;		/* invalidate its message number and free */
    (*mailcache) (stream,msgno,CH_FREE);
  }
  /* expunge the slot */
  (*mailcache) (stream,msgno,CH_EXPUNGE);
  --stream->nmsgs;		/* update stream status */
  stream->msgno = 0;		/* nuke the short cache too */
  mail_free_envelope (&stream->env);
  mail_free_body (&stream->body);
  /* notify main program of change */
  if (!stream->silent) mm_expunged (stream,msgno);
}

/* Mail stream status routines */


/*
 * Mail lock stream
 * Accepts: mail stream
 */
void 
mail_lock(MAILSTREAM * stream)
{
  if (stream->lock) {
    mm_log("Lock when already locked",ERROR, stream);
  } else {
    stream->lock = _B_TRUE;	/* lock stream */
  }
}


/*
 * Mail unlock stream
 * Accepts: mail stream
 */
void
mail_unlock(MAILSTREAM * stream)
{
  if (!stream->lock) {
    mm_log("Unlock when not locked", ERROR, stream);
  } else {
    stream->lock = _B_FALSE;	/* unlock stream */
  }
}


/* Mail turn on debugging telemetry
 * Accepts: mail stream
 */
void mail_debug (MAILSTREAM *stream)
{
  if (stream)
    stream->debug = T;		/* turn on debugging telemetry */
}


/* Mail turn off debugging telemetry
 * Accepts: mail stream
 */

void mail_nodebug (MAILSTREAM *stream)
{
  if (stream)
    stream->debug = NIL;		/* turn off debugging telemetry */
}

/* Mail parse UID sequence
 * Accepts: mail stream
 *	    sequence to parse
 * Returns: T if parse successful, else NIL
 */

long mail_uid_sequence (MAILSTREAM *stream,char *sequence)
{
  unsigned long i,j,k,x;
  for (i = 1; i <= stream->nmsgs; i++) mail_elt (stream,i)->sequence = NIL;
  while (*sequence) {		/* while there is something to parse */
    if (*sequence == '*') {	/* maximum message */
      i = stream->nmsgs ? mail_uid (stream,stream->nmsgs) : stream->uid_last;
      sequence++;		/* skip past * */
    }
    /* parse and validate message number */
    else if (!(i = strtoul ((const char *) sequence,&sequence,10))) {
      mm_log ("UID sequence invalid",ERROR,stream);
      return NIL;
    }
    switch (*sequence) {	/* see what the delimiter is */
    case ':':			/* sequence range */
      if (*++sequence == '*') {	/* maximum message */
	j = stream->nmsgs ? mail_uid (stream,stream->nmsgs) : stream->uid_last;
	sequence++;		/* skip past * */
      }
      /* parse end of range */
      else if (!(j = strtoul ((const char *) sequence,&sequence,10))) {
	mm_log ("UID sequence range invalid",ERROR,stream);
	return NIL;
      }
      if (*sequence && *sequence++ != ',') {
	mm_log ("UID sequence range syntax error",ERROR,stream);
	return NIL;
      }
      if (i > j) {		/* swap the range if backwards */
	x = i; i = j; j = x;
      }
      /* mark each item in the sequence */
      for (x = 1; x <= stream->nmsgs; x++) {
	if (((k = mail_uid (stream,x)) >= i) && (k <= j))
	  mail_elt (stream,x)->sequence = T;
      }
      break;
    case ',':			/* single message */
      ++sequence;		/* skip the delimiter, fall into end case */
    case '\0':			/* end of sequence, mark this message */
      for (x = 1; x <= stream->nmsgs; x++)
	if (i == mail_uid (stream,x)) mail_elt (stream,x)->sequence = T;
      break;
    default:			/* anything else is a syntax error! */
      mm_log ("UID sequence syntax error",ERROR,stream);
      return NIL;
    }
  }
  return T;			/* successfully parsed sequence */
}

/*
 * Mail filter text by header lines
 * Accepts: text to filter
 *	    length of text
 *	    list of lines
 *	    fetch flags
 * Returns: new text size
 */
unsigned long
mail_filter(char *text, unsigned long len, STRINGLIST * lines, long flags)
{
  STRINGLIST 	*	hdrs;
  int 			hdrLen;
  boolean_t		found = _B_FALSE;
  char 			c;
  char	       	*	textPtr1;
  char		*	tmpPtr;
  char			tmp[MAILTMPLEN];
  char 		*	textPtr2 = text;
  char 		*	dst = text;
  char 		*	end = text + len;

  while (textPtr2 < end) {		/* process header */

    /* Copy over the header, stopping at specific characters */
    for (textPtr1 = textPtr2, tmpPtr = tmp
	   ; (textPtr1 < end)
	   && (*textPtr1 != ' ')
	   && (*textPtr1 != '\t')
	   && (*textPtr1 != ':')
	   && (*textPtr1 != '\r')
	   && (*textPtr1 != '\n')
	   ; *tmpPtr++ = *textPtr1++);

    *tmpPtr = '\0';		/* tie off */

    if (tmpPtr == tmp) {
      break;
    }

    /* see if found in header */
    if ((hdrLen = tmpPtr - tmp) > 0) {
      for (hdrs = lines; hdrs != NULL && !found; hdrs = hdrs->next) {
	if (hdrs->size == hdrLen) {

	  /* header matches? */
	  if (strncasecmp(tmp, hdrs->text, hdrLen) == 0) {
	    found = _B_TRUE;
	    break;
	  }
	}
      }

      /* skip header line if not wanted */
      if (hdrs != NULL && (flags & FT_NOT) ? found : !found) {
	while ((textPtr2 < end)
	       && ((*textPtr2++ != '\n')
		   || ((*textPtr2 == ' ')
		       || (*textPtr2 == '\t'))));
	continue;
      }
      if (found) {
	do {
	  c = *dst++ = *textPtr2++;
	} while ((textPtr2 < end)
		 && ((c != '\n')
		     || ((*textPtr2 == ' ')
			 || (*textPtr2 == '\t'))));
	found = _B_FALSE;

      } else {
	do {
	  textPtr2 = strchr(textPtr2, '\n');
	} while (textPtr2 < end
		  && (*(textPtr2+1) == ' '
		      || *(textPtr2+1) == '\t'));

	if (textPtr2 == NULL) {
	  textPtr2 = end;
	}
      }
    }
  }
  *dst = '\0';			/* tie off destination */
  return dst - text;
}

/* Local mail search message
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

long mail_search_msg (MAILSTREAM *stream,unsigned long msgno,char *charset,
		      SEARCHPGM *pgm)
{
  MAIL_GLOBALS *mg = (MAIL_GLOBALS *)stream->mail_globals;
  unsigned short d;
  unsigned long i,uid;
  char *s;
  MESSAGECACHE *elt = mail_elt (stream,msgno);
  SHORTINFO *sinfo = &mail_lelt (stream,msgno)->sinfo;
  ENVELOPE *env;
  MESSAGECACHE delt;
  SEARCHHEADER *hdr;
  SEARCHSET *set;
  SEARCHPGMLIST *not;
  SEARCHOR *sor;
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
    char tmp[MAILTMPLEN];
    sprintf (tmp,"%ld",elt->msgno);
    mail_fetchfast (stream,tmp);
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
  if (pgm->keyword && !mail_search_keyword (stream,elt,pgm->keyword))
    return NIL;
  if (pgm->unkeyword && mail_search_keyword (stream,elt,pgm->unkeyword))
    return NIL;
  /* sent date ranges */
  if (pgm->sentbefore || pgm->senton || pgm->sentsince) {
    env = mail_fetchenvelope (stream,msgno);
    if (!env) return NIL;
    if (!(env->date && mail_parse_date (&delt,env->date) &&
	  (d = (delt.year << 9) + (delt.month << 5) + delt.day))) return NIL;
    if (pgm->sentbefore && (d >= pgm->sentbefore)) return NIL;
    if (pgm->senton && (d != pgm->senton)) return NIL;
    if (pgm->sentsince && (d < pgm->sentsince)) return NIL;
  }
  /* internal date ranges */
  if (pgm->before || pgm->on || pgm->since) {
    d = (elt->year << 9) + (elt->month << 5) + elt->day;
    if (pgm->before && (d >= pgm->before)) return NIL;
    if (pgm->on && (d != pgm->on)) return NIL;
    if (pgm->since && (d < pgm->since)) return NIL;
  }
  /* search headers */
  if (pgm->bcc) {
    env = mail_fetchenvelope (stream,msgno);
    if (!env || !mail_search_addr (env->bcc,charset,pgm->bcc,stream)) 
      return NIL;
  }
  if (pgm->cc) {
    env = mail_fetchenvelope (stream,msgno);
    if (!env || !mail_search_addr (env->cc,charset,pgm->cc,stream)) 
      return NIL;
  }
  /* Use Shortinfo From in lieu of envelope if there */
  if (pgm->from) {
    int from;
    if (sinfo->from) 
      from = mail_search_string(sinfo->from,charset,pgm->from,stream);
    else {
      env = mail_fetchenvelope (stream,msgno);
      from = (!env ? NIL : mail_search_addr (env->from, charset,pgm->from,
					     stream));
    }
    if (!from) return NIL;
  }
  if (pgm->to) {
    env = mail_fetchenvelope (stream,msgno);
    if (!env || !mail_search_addr (env->to,charset,pgm->to,stream)) return NIL;
  }
  /* Use sinfo->subject if there */
  if (pgm->subject) {
    int subject;
    if (sinfo->subject)
      subject = mail_search_string(sinfo->subject,charset,pgm->subject,
				   stream);      
    else {
      env = mail_fetchenvelope (stream,msgno);
      subject = (!env ? NIL : mail_search_string (env->subject,
						  charset,pgm->subject, stream));
    }
    if (!subject) return NIL;
  }
  if (hdr = pgm->header) {
    STRINGLIST sth,stc;
    sth.next = stc.next = NIL;	/* only one at a time */
    do {			/* check headers one at a time */
      int reusit = NIL;		/* return flag */

      sth.size = strlen (sth.text = hdr->line);
      stc.size = strlen (stc.text = hdr->text);
      s = mail_fetchheader_full (stream,msgno,&sth,&i,FT_INTERNAL);
      reusit = mail_search_text (s,i,charset,&stc,stream);
      /* Free the INTERNALLY cached buffer because mail_fetchheader
       * calls mail_filter() which modifies the full header text:
       */
      if (stream->dtb->flags & DR_LOCAL)
	mail_free_cached_buffers_msg(stream,msgno);
      if (!reusit) return NIL;
    }
    while (hdr = hdr->next);
  }
  
  /* search strings */
  if (stream->dtb->flags & DR_LOWMEM) {
    mailgets_t omg;
    omg = (mailgets_t) mail_parameters (stream,GET_GETS,NIL);

    mail_parameters (stream,SET_GETS,(void *) mail_search_gets);

    if (mg->scharset || mg->sstring) {
      mm_log("Re-entrant low-mem text search",ERROR, stream);
      return(_B_FALSE);
    }
    mg->scharset = charset;		/* pass down charset */
    if ((mg->sstring = pgm->body) &&
	!mail_fetchtext_full (stream,msgno,NIL,FT_PEEK)) return NIL;
    if ((mg->sstring = pgm->text) &&
	!(mail_fetchheader_full (stream,msgno,NIL,NIL,NIL) ||
	  mail_fetchtext_full (stream,msgno,&i,FT_PEEK))) return NIL;
    mg->sstring = NIL;		/* end search */
    mg->scharset = NIL;
    /* restore former gets routine */
    mail_parameters (stream,SET_GETS,(void *) omg);
  }
  else {
    if (pgm->body) {
      s = mail_fetchtext_full (stream,msgno,&i,FT_INTERNAL+FT_PEEK);
      if (!mail_search_text (s,i,charset,pgm->body, stream)) return NIL;
    }
    if (pgm->text) {
      s = mail_fetchheader_full (stream,msgno,NIL,&i,FT_INTERNAL);
      if (!mail_search_text (s,i,charset,pgm->text,stream) &&
	  (s = mail_fetchtext_full (stream,msgno,&i,FT_INTERNAL+FT_PEEK)) &&
	  !mail_search_text (s,i,charset,pgm->text,stream)) return NIL;
    }
  }
  if (sor = pgm->or) do
    if (!(mail_search_msg (stream,msgno,charset,sor->first) ||
	  mail_search_msg (stream,msgno,charset,sor->second))) return NIL;
		     while (sor = sor->next);

  if (not = pgm->not) do if (mail_search_msg (stream,msgno,charset,not->pgm))
    return NIL;
		      while (not = not->next);
  return T;
}

/* Mail search keyword
 * Accepts: MAIL stream
 *	    elt to get flags from
 *	    keyword list
 * Returns: T if search found a match
 */

long mail_search_keyword (MAILSTREAM *stream,MESSAGECACHE *elt,STRINGLIST *st)
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


/* Mail search an address list
 * Accepts: address list
 *	    character set
 *	    string list
 * Returns: T if search found a match
 */

long mail_search_addr (ADDRESS *adr,char *charset,STRINGLIST *st,
		       MAILSTREAM *stream)
{
  char t[64*MAILTMPLEN];
  t[0] = '\0';			/* initially empty string */
  rfc822_write_address (t,adr);	/* get text for address */
  return mail_search_string (t,charset,st, stream);
}


/* Mail search string
 * Accepts: text string
 *	    character set
 *	    string list
 * Returns: T if search found a match
 */

long mail_search_string (char *txt,char *charset,STRINGLIST *st,
			 MAILSTREAM *stream)
{
  /* Crashes if txt is NIL! */
  if (!txt || *txt == NIL) return NIL;
  return mail_search_text (txt,(long) strlen (txt),charset,st,stream);
}

/* Get string for searching
 * Accepts: readin function pointer
 *	    stream to use
 *	    number of bytes
 * Returns: non-NIL if error
 */

#define SEARCHSLOP 128

char *mail_search_gets (readfn_t f,void *anystream,unsigned long size, MAILSTREAM *stream)
{		/* Notice two streams could be different. */
  MAIL_GLOBALS *mg = (MAIL_GLOBALS *)stream->mail_globals;
  char tmp[MAILTMPLEN+SEARCHSLOP+1];
  char *ret = NIL;
  unsigned long i;
  /* make sure buffer clear */
  memset (tmp,'\0',(size_t) MAILTMPLEN+SEARCHSLOP+1);
  /* read first buffer */
  (*f) (anystream,i = min (size,(long) MAILTMPLEN),tmp);
  /* search for text */
  if (mail_search_text (tmp,i,mg->scharset,
			mg->sstring,stream)) ret = "ok";
  if (size -= i) {		/* more to do, blat slop down */
    if (!ret) memmove (tmp,tmp+MAILTMPLEN-SEARCHSLOP,(size_t) SEARCHSLOP);
    do {			/* read subsequent buffers one at a time */
      (*f) (anystream,i = min (size,(long) MAILTMPLEN),tmp+SEARCHSLOP);
      if (!ret && mail_search_text (tmp,i+SEARCHSLOP,
				    mg->scharset,mg->sstring,stream)) {
	ret = "ok";
	memmove (tmp,tmp+MAILTMPLEN,(size_t) SEARCHSLOP);
      }
    }
    while (size -= i);
  }
  return ret;
}


/* Mail search text
 * Accepts: text string
 *	    text length
 *	    character set
 *	    string list
 * Returns: T if search found a match
 */

long mail_search_text (char *txt,long len,char *charset,STRINGLIST *st,
		       MAILSTREAM *stream)
{
  char tmp[MAILTMPLEN];
  /* bail out if nothing to search */
  if (txt == NIL || len == 0) return NIL;

  if (!(charset && *charset &&	/* if US-ASCII search */
	strcmp (ucase (strcpy (tmp,charset)),"US-ASCII"))) {
    do if (!search (txt,len,st->text,st->size)) return NIL;
    while (st = st->next);
    return T;
  }
  sprintf (tmp,"Unknown character set %s",charset);
  mm_log (tmp,ERROR,stream);
  return NIL;			/* not found */
}

/* Mail parse search criteria
 * Accepts: criteria
 * Returns: search program if parse successful, else NIL
 */

SEARCHPGM *mail_criteria (char *criteria, MAILSTREAM *stream)
{
  char *lasts;
  SEARCHPGM *pgm;
  char tmp[MAILTMPLEN];
  int f = NIL, g = NIL;
  if (!criteria) return NIL;	/* return if no criteria */
  pgm = mail_newsearchpgm ();	/* make a basic search program */
  /* for each criterion */
  for (criteria = strtok_r (criteria," ", &lasts); criteria;
       (criteria = strtok_r (NIL," ", &lasts ))) {
    f = NIL;			/* init then scan the criterion */
    switch (*ucase (criteria)) {
    case 'A':			/* possible ALL, ANSWERED */
      if (!strcmp (criteria+1,"LL")) g = pgm->only_all = T;
      else if (!strcmp (criteria+1,"NSWERED")) f = pgm->answered = T;
      break;
    case 'B':			/* possible BCC, BEFORE, BODY */
      /* Need to pass lasts pointer */
      if (!strcmp (criteria+1,"CC")) f = mail_criteria_string (&pgm->bcc, &lasts);
      else if (!strcmp (criteria+1,"EFORE"))
	f = mail_criteria_date (&pgm->before, &lasts);
      else if (!strcmp (criteria+1,"ODY")) 
	f = mail_criteria_string (&pgm->body, &lasts);
      break;
    case 'C':			/* possible CC */
      if (!strcmp (criteria+1,"C")) f = mail_criteria_string (&pgm->cc, &lasts);
      break;
    case 'D':			/* possible DELETED */
      if (!strcmp (criteria+1,"ELETED")) f = pgm->deleted = T;
      break;
    case 'F':			/* possible FLAGGED, FROM */
      if (!strcmp (criteria+1,"LAGGED")) f = pgm->flagged = T;
      else if (!strcmp (criteria+1,"ROM")) 
	f = mail_criteria_string (&pgm->from, &lasts);
      break;
    case 'K':			/* possible KEYWORD */
      if (!strcmp (criteria+1,"EYWORD"))
	f = mail_criteria_string (&pgm->keyword, &lasts);
      break;
      
    case 'N':			/* possible NEW */
      if (!strcmp (criteria+1,"EW")) f = pgm->recent = pgm->unseen = T;
      break;
    case 'O':			/* possible OLD, ON */
      if (!strcmp (criteria+1,"LD")) f = pgm->old = T;
      else if (!strcmp (criteria+1,"N")) f = mail_criteria_date (&pgm->on, &lasts);
      break;
    case 'R':			/* possible RECENT */
      if (!strcmp (criteria+1,"ECENT")) f = pgm->recent = T;
      break;
    case 'S':			/* possible SEEN, SINCE, SUBJECT */
      if (!strcmp (criteria+1,"EEN")) f = pgm->seen = T;
      else if (!strcmp (criteria+1,"INCE")) 
	f = mail_criteria_date (&pgm->since, &lasts);
      else if (!strcmp (criteria+1,"UBJECT"))
	f = mail_criteria_string (&pgm->subject, &lasts);
      break;
    case 'T':			/* possible TEXT, TO */
      if (!strcmp (criteria+1,"EXT")) f = mail_criteria_string (&pgm->text, &lasts);
      else if (!strcmp (criteria+1,"O")) f = mail_criteria_string (&pgm->to, &lasts);
      break;
    case 'U':			/* possible UN* */
      if (criteria[1] == 'N') {
	if (!strcmp (criteria+2,"ANSWERED")) f = pgm->unanswered = T;
	else if (!strcmp (criteria+2,"DELETED")) f = pgm->undeleted = T;
	else if (!strcmp (criteria+2,"FLAGGED")) f = pgm->unflagged = T;
	else if (!strcmp (criteria+2,"KEYWORD"))
	  f = mail_criteria_string (&pgm->unkeyword, &lasts);
	else if (!strcmp (criteria+2,"SEEN")) f = pgm->unseen = T;
      }
      break;
    default:			/* we will barf below */
      break;
    }
    if (!f && !g) {			/* if can't determine any criteria */
      sprintf (tmp,"Unknown search criterion: %.30s",criteria);
      mm_log (tmp,ERROR,stream);
      mail_free_searchpgm (&pgm);
      break;
    }
  }
  /* See if we just had "ALL". Used for searching local cache. */
  if (f && g) 
    pgm->only_all = NIL;
  return pgm;
}

/* Parse a date
 * Accepts: pointer to date integer to return
 * Returns: T if successful, else NIL
 */

int mail_criteria_date (unsigned short *date, char **lasts)
{
  STRINGLIST *s = NIL;
  MESSAGECACHE elt;
  /* parse the date and return fn if OK */
  int ret = (mail_criteria_string (&s, lasts) && mail_parse_date (&elt,s->text) &&
	     (*date = (elt.year << 9) + (elt.month << 5) + elt.day)) ? _B_TRUE : _B_FALSE;
  if (s) mail_free_stringlist (&s);
  return ret;
}


/* Parse a string
 * Accepts: pointer to stringlist
 * Returns: T if successful, else NIL
 */

int mail_criteria_string (STRINGLIST **s, char **lasts)
{
  unsigned long n;
  char e,*d,*end = " ",*c = strtok_r (NIL,"", lasts);
  if (!c) return NIL;		/* missing argument */
  switch (*c) {			/* see what the argument is */
  case '{':			/* literal string */
    n = strtoul (c+1,&d,10);	/* get its length */
    if ((*d++ == '}') && (*d++ == '\015') && (*d++ == '\012') &&
	(!(*(c = d + n)) || (*c == ' '))) {
      e = *--c;			/* store old delimiter */
      *c = DELIM;		/* make sure not a space */
      strtok_r (c," ", lasts);		/* reset the strtok_r mechanism */
      *c = e;			/* put character back */
      break;
    }
  case '\0':			/* catch bogons */
  case ' ':
    return NIL;
  case '"':			/* quoted string */
    if (strchr (c+1,'"')) end = "\"";
    else return NIL;		/* falls through */
  default:			/* atomic string */
    if (d = strtok_r (c,end, lasts )) n = strlen (d);
    else return NIL;
    break;
  }
  while (*s) s = &(*s)->next;	/* find tail of list */
  *s = mail_newstringlist ();	/* make new entry */
  (*s)->text = cpystr (d);	/* return the data */
  (*s)->size = n;
  return T;
}

/* Mail parse sequence
 * Accepts: mail stream
 *	    sequence to parse
 * Returns: T if parse successful, else NIL
 */

long mail_sequence (MAILSTREAM *stream,char *sequence)
{
  unsigned long i,j,x;
  for (i = 1; i <= stream->nmsgs; i++) mail_elt (stream,i)->sequence = NIL;
  if (!sequence) {	/* one way to prevent core dump */
    mm_log ("Missing sequence number",ERROR,stream);
    return NIL;
  }
  while (*sequence) {		/* while there is something to parse */
    if (*sequence == '*') {	/* maximum message */
      if (stream->nmsgs) i = stream->nmsgs;
      else {
	mm_log ("No messages, so no maximum message number",ERROR,stream);
	return NIL;
      }
      sequence++;		/* skip past * */
    }
    /* parse and validate message number */
    else if (!(i = strtoul ((const char *) sequence,&sequence,10)) ||
	     (i > stream->nmsgs)) {
      mm_log ("Sequence invalid",ERROR,stream);
      return NIL;
    }
    switch (*sequence) {	/* see what the delimiter is */
    case ':':			/* sequence range */
      if (*++sequence == '*') {	/* maximum message */
	if (stream->nmsgs) j = stream->nmsgs;
	else {
	  mm_log ("No messages, so no maximum message number",ERROR,stream);
	  return NIL;
	}
	sequence++;		/* skip past * */
      }
      /* parse end of range */
      else if (!(j = strtoul ((const char *) sequence,&sequence,10)) ||
	       (j > stream->nmsgs)) {
	mm_log ("Sequence range invalid",ERROR,stream);
	return NIL;
      }
      if (*sequence && *sequence++ != ',') {
	mm_log ("Sequence range syntax error",ERROR,stream);
	return NIL;
      }
      if (i > j) {		/* swap the range if backwards */
	x = i; i = j; j = x;
      }
      /* mark each item in the sequence */
      while (i <= j) mail_elt (stream,j--)->sequence = T;
      break;
    case ',':			/* single message */
      ++sequence;		/* skip the delimiter, fall into end case */
    case '\0':			/* end of sequence, mark this message */
      mail_elt (stream,i)->sequence = T;
      break;
    default:			/* anything else is a syntax error! */
      mm_log ("Sequence syntax error",ERROR,stream);
      return NIL;
    }
  }
  return T;			/* successfully parsed sequence */
}

/* Mail parse sequence for which the elt has a uid
 * Accepts: mail stream
 *	    sequence to parse
 * Returns: T if parse successful, else NIL
 */

long mail_sequence_with_uid (MAILSTREAM *stream,char *sequence)
{
  unsigned long i,j,x;
  MESSAGECACHE *elt;
  for (i = 1; i <= stream->nmsgs; i++) mail_elt (stream,i)->sequence = NIL;
  while (*sequence) {		/* while there is something to parse */
    if (*sequence == '*') {	/* maximum message */
      if (stream->nmsgs) i = stream->nmsgs;
      else {
	mm_log ("No messages, so no maximum message number",ERROR,stream);
	return NIL;
      }
      sequence++;		/* skip past * */
    }
    /* parse and validate message number */
    else if (!(i = strtoul ((const char *) sequence,&sequence,10)) ||
	     (i > stream->nmsgs)) {
      mm_log ("Sequence invalid",ERROR,stream);
      return NIL;
    }
    /* Reject if no uid is there */
    switch (*sequence) {	/* see what the delimiter is */
    case ':':			/* sequence range */
      if (*++sequence == '*') {	/* maximum message */
	if (stream->nmsgs) j = stream->nmsgs;
	else {
	  mm_log ("No messages, so no maximum message number",ERROR,stream);
	  return NIL;
	}
	sequence++;		/* skip past * */
      }
      /* parse end of range */
      else if (!(j = strtoul ((const char *) sequence,&sequence,10)) ||
	       (j > stream->nmsgs)) {
	mm_log ("Sequence range invalid",ERROR,stream);
	return NIL;
      }
      if (*sequence && *sequence++ != ',') {
	mm_log ("Sequence range syntax error",ERROR,stream);
	return NIL;
      }
      if (i > j) {		/* swap the range if backwards */
	x = i; i = j; j = x;
      }
      /* mark each item in the sequence */
      while (i <= j) {
	elt = mail_elt (stream,j--);
	if (elt->uid != 0) elt->sequence = T;
      }
      break;
    case ',':			/* single message */
      ++sequence;		/* skip the delimiter, fall into end case */
    case '\0':			/* end of sequence, mark this message */
      elt = mail_elt (stream,i);
      if (elt->uid != 0) elt->sequence = T;
      break;
    default:			/* anything else is a syntax error! */
      mm_log ("Sequence syntax error",ERROR,stream);
      return NIL;
    }
  }
  return T;			/* successfully parsed sequence */
}

/* Parse flag list
 * Accepts: MAIL stream
 *	    flag list as a character string
 *	    pointer to user flags to return
 * Returns: system flags
 */

long mail_parse_flags (MAILSTREAM *stream,char *flag,unsigned long *uf)
{
  char *lasts;
  char *t,*s,tmp[MAILTMPLEN],key[MAILTMPLEN];
  char *toker;
  char *t0;			/* flag place holder */
  short f = 0;
  long i;
  short j;
  *uf = 0;			/* initially no user flags */
  if (flag && *flag) {		/* no-op if no flag string */
    /* check if a list and make sure valid */
    if ((i = (*flag == '(')) ^ (flag[strlen (flag)-1] == ')')) {
      mm_log ("Bad flag list",ERROR,stream);
      return NIL;
    }
    /* copy the flag string w/o list construct */
    strncpy (tmp,flag+i,(j = strlen (flag) - (2*i)));
    tmp[j] = '\0';
    t = ucase (tmp);		/* uppercase only from now on */
    strtok_r(t, " ", &lasts);	/* init out strtok_r */
    while (t && *t) {		/* parse the flags */
      i = 0;
      t0 = t;			/* for error message */
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
	if (stream && (stream->stream_status & S_OPENED )) 
	  for (j = 0; !i && j < NUSERFLAGS && (s =stream->user_flags[j]); ++j)
	    if (!strcmp (t,ucase (strcpy (key,s)))) *uf |= i = 1 << j;
      }
      if (!i) {			/* didn't find a matching flag? */
	sprintf (key,"Unknown flag: %.80s",t0);
	mm_log (key,ERROR,stream);
      }
      /* parse next flag */
      t = strtok_r (NIL," ", &lasts);
    }
  }
  return f;
}

/* Mail data structure instantiation routines */


/* Mail instantiate envelope
 * Returns: new envelope
 */

ENVELOPE *mail_newenvelope (void)
{
  ENVELOPE *env = (ENVELOPE *) fs_get (sizeof (ENVELOPE));
  env->remail = NIL;		/* initialize all fields */
  env->return_path = NIL;
  env->date = NIL;
  env->subject = NIL;
  env->from = env->sender = env->reply_to = env->to = env->cc = env->bcc = NIL;
  env->in_reply_to = env->message_id = env->newsgroups = NIL;
  return env;
}


/* Mail instantiate address
 * Returns: new address
 */

ADDRESS *mail_newaddr (void)
{
  ADDRESS *adr = (ADDRESS *) fs_get (sizeof (ADDRESS));
  /* initialize all fields */
  adr->personal = adr->adl = adr->mailbox = adr->host = adr->error = NIL;
  adr->next = NIL;
  return adr;
}


/* Mail instantiate body
 * Returns: new body
 */

BODY *mail_newbody (void)
{
  return mail_initbody ((BODY *) fs_get (sizeof (BODY)));
}

/* Mail initialize body
 * Accepts: body
 * Returns: body
 */

BODY *mail_initbody (BODY *body)
{
  body->type = TYPETEXT;	/* content type */
  body->encoding = ENC7BIT;	/* content encoding */
  body->subtype = body->id = body->description = NIL;
  body->parameter = NIL;
  body->contents_deallocate = fs_give;	/* initially cause fs_give to be called to deallocate contents */
  body->contents.text = NIL;	/* no contents yet */
  body->contents.binary = NIL;
  body->contents.part = NIL;
  body->contents.msg.env = NIL;
  body->contents.msg.body = NIL;
  body->contents.msg.hdr = NIL;
  body->contents.msg.hdrsize = 0;
  body->contents.msg.text = NIL;
  body->size.lines = body->size.bytes = body->size.ibytes = 0;
  body->md5 = NIL;
  return body;
}


/* Mail instantiate body parameter
 * Returns: new body part
 */

PARAMETER *mail_newbody_parameter (void)
{
  PARAMETER *parameter = (PARAMETER *) fs_get (sizeof (PARAMETER));
  parameter->attribute = parameter->value = NIL;
  parameter->next = NIL;	/* no next yet */
  return parameter;
}


/* Mail instantiate body part
 * Returns: new body part
 */

PART *mail_newbody_part (void)
{
  PART *part = (PART *) fs_get (sizeof (PART));
  mail_initbody (&part->body);	/* initialize the body */
  part->offset = 0;		/* no offset yet */
  part->next = NIL;		/* no next yet */
  return part;
}

/* Mail instantiate string list
 * Returns: new string list
 */

STRINGLIST *mail_newstringlist (void)
{
  STRINGLIST *string = (STRINGLIST *) fs_get (sizeof (STRINGLIST));
  string->text = NIL;		/* no text yet */
  string->size = 0;
  string->next = NIL;		/* no next yet */
  return string;
}

/* Mail instantiate headerlist
 * Returns: new header list
 */

HEADERLIST *mail_newheaderlist (void) 
{
  HEADERLIST *header = (HEADERLIST *) fs_get (sizeof (HEADERLIST));
  header->n_entries = 0;
  header->data = NIL;
  header->fields = NIL;
  return header;
}

/*
 * String lists are identical if
 *    both NIL
 *    Each entry is identical. This is an ordered stringlist
 *    compare.
 */
int mail_stringlist_cmp(STRINGLIST *sl1,STRINGLIST *sl2)
{
  /* Both NIL? */
  if (sl1 == NIL || sl2 == NIL)
    if (sl1 == sl2) return T;
    else return NIL;
  /* Compare each node */
  while (sl1 && sl2) {
    if (sl1->size == sl2->size) {
      if (sl1->text == NIL || sl2->text == NIL) {
	if (sl1->text != sl2->text) return NIL;	/* One not NIL */
      } else if (strcasecmp(sl1->text,sl2->text) != 0)
	return NIL;		/* Different text strings */
    } else return NIL;		/* Different lengths */
    sl1 = sl1->next;
    sl2 = sl2->next;
  }
  /* both must be NIL */
  if (sl1 == sl2) return T;
  else return NIL;
}
/*
 * duplicate a string list */
STRINGLIST *mail_dup_stringlist(STRINGLIST *src)
{
  STRINGLIST *sl = NIL,**sl1 = &sl;
  while (src) {
    *sl1 = mail_newstringlist();
    if (sl == NIL) sl = *sl1;	/* ptr to the tete */
    (*sl1)->size = src->size;
    if (src->size > 0 && src->text) 
      (*sl1)->text = strdup(src->text);
    src = src->next;
    sl1 = &(*sl1)->next;
  }
  return sl;
}

/* Mail instantiate new search program
 * Returns: new search program
 */

SEARCHPGM *mail_newsearchpgm (void)
{
  SEARCHPGM *pgm = (SEARCHPGM *) fs_get (sizeof (SEARCHPGM));
  memset (pgm,0,sizeof (SEARCHPGM));
  return pgm;
}


/* Mail instantiate new search program
 * Accepts: header line name   
 * Returns: new search program
 */

SEARCHHEADER *mail_newsearchheader (char *line)
{
  SEARCHHEADER *hdr = (SEARCHHEADER *) fs_get (sizeof (SEARCHHEADER));
  hdr->line = cpystr (line);	/* not defined */
  hdr->text = NIL;
  hdr->next = NIL;		/* no next yet */
  return hdr;
}

/* Mail instantiate new search set
 * Returns: new search set
 */

SEARCHSET *mail_newsearchset (void)
{
  SEARCHSET *set = (SEARCHSET *) fs_get (sizeof (SEARCHSET));
  set->first = set->last = 0;	/* nothing in the set yet */
  set->next = NIL;		/* no next yet */
  return set;
}


/* Mail instantiate new search or
 * Returns: new search or
 */

SEARCHOR *mail_newsearchor (void)
{
  SEARCHOR *or = (SEARCHOR *) fs_get (sizeof (SEARCHOR));
  or->first = mail_newsearchpgm ();
  or->second = mail_newsearchpgm ();
  or->next = NIL;		/* no next yet */
  return or;
}


/* Mail instantiate new searchpgmlist
 * Returns: new searchpgmlist
 */

SEARCHPGMLIST *mail_newsearchpgmlist (void)
{
  SEARCHPGMLIST *pgl = (SEARCHPGMLIST *) fs_get (sizeof (SEARCHPGMLIST));
  pgl->pgm = mail_newsearchpgm ();
  pgl->next = NIL;		/* no next yet */
  return pgl;
}

/* Mail garbage collection routines */


/* Mail garbage collect body
 * Accepts: pointer to body pointer
 */

void mail_free_body (BODY **body)
{
  if (*body) {			/* only free if exists */
    mail_free_body_data (*body);/* free its data */
    fs_give ((void **) body);	/* return body to free storage */
  }
}


/* Mail garbage collect body data
 * Accepts: body pointer
 */

void mail_free_body_data (BODY *body)
{
  if (body->subtype) fs_give ((void **) &body->subtype);
  mail_free_body_parameter (&body->parameter);
  if (body->id) fs_give ((void **) &body->id);
  if (body->description) fs_give ((void **) &body->description);
  /* free md5 data */
  if (body->md5) fs_give ((void **) &body->md5);
  switch (body->type) {		/* free contents */
  case TYPETEXT:		/* unformatted text */
    if (body->contents.text) {
	(*body->contents_deallocate)((void **) &body->contents.text);
	body->contents_deallocate = fs_give;
    }
    break;
  case TYPEMULTIPART:		/* multiple part */
    mail_free_body_part (&body->contents.part);
    break;
  case TYPEMESSAGE:		/* encapsulated message */
    mail_free_envelope (&body->contents.msg.env);
    mail_free_body (&body->contents.msg.body);
    if (body->contents.msg.text) {
	(*body->contents_deallocate)((void **) &body->contents.msg.text);
	body->contents_deallocate = fs_give;
    }
    break;
  case TYPEAPPLICATION:		/* application data */
  case TYPEAUDIO:		/* audio */
  case TYPEIMAGE:		/* static image */
  case TYPEVIDEO:		/* video */
    if (body->contents.binary) {
	(*body->contents_deallocate)((void **) &body->contents.binary);
	body->contents_deallocate = fs_give;
    }
    break;
  default:
    break;
  }
}

/* Mail garbage collect body parameter
 * Accepts: pointer to body parameter pointer
 */

void mail_free_body_parameter (PARAMETER **parameter)
{
  if (*parameter) {		/* only free if exists */
    if ((*parameter)->attribute) fs_give ((void **) &(*parameter)->attribute);
    if ((*parameter)->value) fs_give ((void **) &(*parameter)->value);
    /* run down the list as necessary */
    mail_free_body_parameter (&(*parameter)->next);
    /* return body part to free storage */
    fs_give ((void **) parameter);
  }
}


/* Mail garbage collect body part
 * Accepts: pointer to body part pointer
 */

void mail_free_body_part (PART **part)
{
  if (*part) {			/* only free if exists */
    mail_free_body_data (&(*part)->body);
    /* run down the list as necessary */
    mail_free_body_part (&(*part)->next);
    fs_give ((void **) part);	/* return body part to free storage */
  }
}

/* Mail garbage collect message cache
 * Accepts: mail stream
 *
 * The message cache is set to NIL when this function finishes.
 */

void mail_free_cache (MAILSTREAM *stream)
{
  /* flush the cache */
  (*mailcache) (stream,(long) 0,CH_INIT);
  stream->msgno = 0;		/* free this cruft too */
  mail_free_envelope (&stream->env);
  mail_free_body (&stream->body);
  if (stream->text) fs_give ((void **) &stream->text);
  mail_free_fshort_data (&stream->sinfo); /* Sun Imap4 */
}
/* same as above but we have a good connection with
 * a possibly invalid cache because inconsistent with
 * server state.  Hmmm. THIS WILL CHANGE WITH cache resolution?
 * KEEP the msgno as valid, it is. Free the rest */
void mail_free_playback_cache (MAILSTREAM *stream)
{
  /* flush the cache */
  (*mailcache) (stream,(long) 0,CH_LFINIT);
  mail_free_envelope (&stream->env);
  mail_free_body (&stream->body);
  mail_free_fshort_data (&stream->sinfo);
}


/* Mail garbage collect cache element
 * Accepts: pointer to cache element pointer
 */

void mail_free_elt (MESSAGECACHE **elt)
{
  /* only free if exists and no sharers */
  if (*elt && !--(*elt)->lockcount) fs_give ((void **) elt);
  else *elt = NIL;		/* else simply drop pointer */
}


/* Mail garbage collect long cache element
 * Accepts: pointer to long cache element pointer
 */

void mail_free_lelt (LONGCACHE **lelt)
{
  /* only free if exists and no sharers */
  if (*lelt && !--(*lelt)->elt.lockcount) {
    mail_free_envelope (&(*lelt)->env);
    mail_free_body (&(*lelt)->body);
    /* Sun Imap4: Also free the short info. Can you believe
     */
    mail_free_fshort_data (&(*lelt)->sinfo);
    fs_give ((void **) lelt);	/* return cache element to free storage */
  }
  else *lelt = NIL;		/* else simply drop pointer */
}
/* Mail garbage collect cache element
 * Accepts: pointer to cache element pointer
 */

void mail_always_free_elt (MESSAGECACHE **elt)
{
  /*  Always free if exists */
  if (*elt) fs_give ((void **) elt);
}

void mail_always_free_lelt (LONGCACHE **lelt)
{
  /*  Always free if exists */
  if (*lelt) {
    mail_free_envelope (&(*lelt)->env);
    mail_free_body (&(*lelt)->body);
    mail_free_fshort_data (&(*lelt)->sinfo);
    fs_give ((void **) lelt);
  }
}



/* 
 * Mail garbage collect envelope
 * Accepts: pointer to envelope pointer
 */
void
mail_free_envelope (ENVELOPE **env)
{
  if (*env) {			/* only free if exists */
    if ((*env)->remail) {
      free((*env)->remail);
    }
    mail_free_address (&(*env)->return_path);

    if ((*env)->date) {
      free((*env)->date);
    }
    mail_free_address (&(*env)->from);
    mail_free_address (&(*env)->sender);
    mail_free_address (&(*env)->reply_to);
    if ((*env)->subject) {
      free((*env)->subject);
    }
    mail_free_address (&(*env)->to);
    mail_free_address (&(*env)->cc);
    mail_free_address (&(*env)->bcc);
    if ((*env)->in_reply_to) {
      free((*env)->in_reply_to);
    }
    if ((*env)->message_id) {
      free((*env)->message_id);
    }
    if ((*env)->newsgroups) {
      free((*env)->newsgroups);
    }
    fs_give ((void **) env);	/* return envelope to free storage */
  }
  return;
}


/* Mail garbage collect address
 * Accepts: pointer to address pointer
 */

void mail_free_address (ADDRESS **address)
{
  if (*address) {		/* only free if exists */
    if ((*address)->personal) fs_give ((void **) &(*address)->personal);
    if ((*address)->adl) fs_give ((void **) &(*address)->adl);
    if ((*address)->mailbox) fs_give ((void **) &(*address)->mailbox);
    if ((*address)->host) fs_give ((void **) &(*address)->host);
    if ((*address)->error) fs_give ((void **) &(*address)->error);
    mail_free_address (&(*address)->next);
    fs_give ((void **) address);/* return address to free storage */
  }
}


/* Mail garbage collect stringlist
 * Accepts: pointer to stringlist pointer
 */

void mail_free_stringlist (STRINGLIST **string)
{
  if (*string) {		/* only free if exists */
    if ((*string)->text) fs_give ((void **) &(*string)->text);
    mail_free_stringlist (&(*string)->next);
    fs_give ((void **) string);	/* return string to free storage */
  }
}

/* Mail garbage collect headerlist
 * Accepts: pointer to headerlist pointer
 */

void mail_free_headerlist (HEADERLIST **header)
{
  if (*header) {		/* only free if exists */
    if ((*header)->data) mail_free_stringlist(&(*header)->data);
    if ((*header)->fields) mail_free_stringlist(&(*header)->fields);
    fs_give ((void **) header);	/* return header to free storage */
  }
}


/* Mail garbage collect searchpgm
 * Accepts: pointer to searchpgm pointer
 */

void mail_free_searchpgm (SEARCHPGM **pgm)
{
  if (*pgm) {			/* only free if exists */
    mail_free_searchset (&(*pgm)->msgno);
    mail_free_searchset (&(*pgm)->uid);
    mail_free_searchor (&(*pgm)->or);
    mail_free_searchpgmlist (&(*pgm)->not);
    mail_free_searchheader (&(*pgm)->header);
    mail_free_stringlist (&(*pgm)->bcc);
    mail_free_stringlist (&(*pgm)->body);
    mail_free_stringlist (&(*pgm)->cc);
    mail_free_stringlist (&(*pgm)->from);
    mail_free_stringlist (&(*pgm)->keyword);
    mail_free_stringlist (&(*pgm)->subject);
    mail_free_stringlist (&(*pgm)->text);
    mail_free_stringlist (&(*pgm)->to);
    fs_give ((void **) pgm);	/* return program to free storage */
  }
}
/* Mail flush searchpgm - does NOT FREE the program itself.
 * Accepts: pointer to searchpgm pointer
 */

void mail_flush_searchpgm (SEARCHPGM *pgm)
{
  if (pgm) {			/* only free if exists */
    mail_free_searchset (&(pgm)->msgno);
    mail_free_searchset (&(pgm)->uid);
    mail_free_searchor (&(pgm)->or);
    mail_free_searchpgmlist (&(pgm)->not);
    mail_free_searchheader (&(pgm)->header);
    mail_free_stringlist (&(pgm)->bcc);
    mail_free_stringlist (&(pgm)->body);
    mail_free_stringlist (&(pgm)->cc);
    mail_free_stringlist (&(pgm)->from);
    mail_free_stringlist (&(pgm)->keyword);
    mail_free_stringlist (&(pgm)->subject);
    mail_free_stringlist (&(pgm)->text);
    mail_free_stringlist (&(pgm)->to);
  }
}



/* Mail garbage collect searchheader
 * Accepts: pointer to searchheader pointer
 */

void mail_free_searchheader (SEARCHHEADER **hdr)
{
  if (*hdr) {			/* only free if exists */
    fs_give ((void **) &(*hdr)->line);
    if ((*hdr)->text) fs_give ((void **) &(*hdr)->text);
    mail_free_searchheader (&(*hdr)->next);
    fs_give ((void **) hdr);	/* return header to free storage */
  }
}


/* Mail garbage collect searchset
 * Accepts: pointer to searchset pointer
 */

void mail_free_searchset (SEARCHSET **set)
{
  if (*set) {			/* only free if exists */
    mail_free_searchset (&(*set)->next);
    fs_give ((void **) set);	/* return set to free storage */
  }
}


/* Mail garbage collect searchor
 * Accepts: pointer to searchor pointer
 */

void mail_free_searchor (SEARCHOR **orl)
{
  if (*orl) {			/* only free if exists */
    mail_free_searchpgm (&(*orl)->first);
    mail_free_searchpgm (&(*orl)->second);
    mail_free_searchor (&(*orl)->next);
    fs_give ((void **) orl);	/* return searchor to free storage */
  }
}


/* Mail garbage collect search program list
 * Accepts: pointer to searchor pointer
 */

void mail_free_searchpgmlist (SEARCHPGMLIST **pgl)
{
  if (*pgl) {			/* only free if exists */
    mail_free_searchpgm (&(*pgl)->pgm);
    mail_free_searchpgmlist (&(*pgl)->next);
    fs_give ((void **) pgl);	/* return searchpgmlist to free storage */
  }
}

/* Link authenicator
 * Accepts: authenticator to add to list
 */

void auth_link (AUTHENTICATOR *auth)
{
  AUTHENTICATOR **a = &authenticators;
  while (*a) a = &(*a)->next;	/* find end of list of authenticators */
  *a = auth;			/* put authenticator at the end */
  auth->next = NIL;		/* this authenticator is the end of the list */
}


/* Authenticate access
 * Accepts: mechanism name
 *	    responder function
 *	    argument count
 *	    argument vector
 * Returns: authenticated user name or NIL
 */

char *mail_auth (char *mechanism,authresponse_t resp,int argc,char *argv[],MAILSTREAM *stream)
{
  char tmp[MAILTMPLEN];
  AUTHENTICATOR *auth;
  /* make upper case copy of mechanism name */
  ucase (strcpy (tmp,mechanism));
  for (auth = authenticators; auth; auth = auth->next)
    if (!strcmp (auth->name,tmp)) return (*auth->server) (resp,argc,argv,stream);
  return NIL;			/* no authenticator found */
}


/* 
 * SUN Imap4 additions here
 */
/* 
 *called from the imapd when "From: .." is missing
 * we call the driver to fabricate a line
 */
char *
mail_fabricate_from(MAILSTREAM *stream, long msgno)
{
  if (msgno < 1 || msgno > stream->nmsgs) {
    mm_log("Bad msgno in mail_fabricate_from", ERROR, stream);
    return(NULL);
  }
  if (stream->dtb) {
    return (*stream->dtb->fabricate_from) (stream, msgno);
  } else {
    return(NULL);		/* if this returns happens we crash */
  }
}        

/*
 * Convert a uid sequence to a msg sequence
 */
#define MAXINTBUFLEN 32
char *mail_uidseq_to_sequence(MAILSTREAM *stream, char *uidseq)
{
  char *buf,  *msgseq;
  MESSAGECACHE *elt;
  unsigned long i, j, bas;
  unsigned long uid;
  int buflen;
  int entries;
  /* set the msgnos for which this sequence is valid */
  if (!mail_uid_sequence(stream, uidseq)) {
    buf = cpystr("");
    return buf;
  }
  /* OK build the msgnum sequence 
   *  1. Compute the max length of buffer.
   *     Allow for "n,"
   */
  for (entries = buflen = 0, i = 1; i <= stream->nmsgs; ++i) {
    if (mail_elt(stream, i)->sequence) {
      buflen += ndigits(i) + 1;
      entries += 1;
      /* set the first entry */
      if (entries == 1) j = i;
    }
  }
  buf = msgseq = fs_get(buflen + 16);
  if (entries == 0) {		/* illegal uid sequence */
    *buf = '\0';
    return buf;
  }
  /* Case where j == nmsgs, or a single hit */
  if (j == stream->nmsgs || entries == 1) {
    sprintf(buf, "%d", j);
    return msgseq;
  }
  for (bas = entries = 0, i = j; i <= stream->nmsgs; ++i) {
    char tmp[MAXINTBUFLEN];
    if (mail_elt(stream, i)->sequence) {
      if (i < stream->nmsgs && mail_elt(stream, i+1)->sequence) {
	if (bas == 0) bas = i;
	continue;
      } else {
	if (entries++ > 0)
	  *buf++ = ',';
	if (bas > 0) {
	  sprintf(tmp, "%d:%d", bas, i);
	} else
	  sprintf(tmp, "%d", i);
	strcpy(buf, tmp);
	buf += strlen(tmp);
	bas = 0;
      }
    }
  }    
  return msgseq;
}
/* mail_uidseq:
 *  Generate a uid sequence from a any uid sequence
 *  Compress consecutive uid seq nos using :
 *  Returns  "" if nothing set, else a sequnce.
 */
char *mail_uidseq (MAILSTREAM *stream, char *uid_seq)
{
  char *seq, *new_seq, *buf;
  char c;
  int ndigs;
  int len;
  int i, j, entries, bas;
  seq = uid_seq;
  /* Make sure last_uid is set */
  mail_uid(stream, stream->nmsgs);

  /* set the msgnos for which this sequence is valid */
  if (!mail_uid_sequence(stream, uid_seq)) {
    new_seq = cpystr("");
    return new_seq;
  }
  /* OK, make the sequence */
  for (entries = 0, i = 1; i <= stream->nmsgs; ++i) {
    if (mail_elt(stream, i)->sequence) {
      entries += 1;
      /* set the first entry */
      if (entries == 1) j = i;
    }
  }
  /* allocate enough space for n copies of the max uid */
  ndigs = ndigits(stream->uid_last);
  len = (entries * ndigs) + entries + 16;
  buf = new_seq = fs_get(len);
  /* a single hit */
  if (entries == 1) {
    sprintf(new_seq, "%ld", mail_elt(stream, j)->uid);
    return new_seq;
  } else if (entries == 0) {
    *buf = '\0';
    return buf;
  }
  for (bas = entries = 0, i = j; i <= stream->nmsgs; ++i) {
    char tmp[MAXINTBUFLEN];
    if (mail_elt(stream, i)->sequence) {
      /* check consecutive uid */
      if (i < stream->nmsgs && mail_elt(stream, i+1)->sequence &&
	  (mail_elt(stream, i)->uid + 1) == mail_elt(stream, i+1)->uid) {
	if (bas == 0) bas = i;
	continue;
      } else {
	if (entries++ > 0)
	  *buf++ = ',';
	if (bas > 0) {
	  sprintf(tmp, "%ld:%ld", 
		  mail_elt(stream, bas)->uid,
		  mail_elt(stream, i)->uid);
	} else
	  sprintf(tmp, "%ld", mail_elt(stream, i)->uid);
	strcpy(buf, tmp);
	buf += strlen(tmp);
	bas = 0;
      }
    }
  }    
  return new_seq;
}
/* mail_uidseq:
 *  Generate a uid sequence from a any sequence
 *  Compress consecutive uid seq nos using :
 *  Returns  "" if nothing set, else a sequnce.
 */
char *mail_seq_to_uidseq (MAILSTREAM *stream, char *msgseq, 
			  int mort)
{
  char *new_seq, *buf;
  char c;
  int ndigs;
  int len;
  int i, j, entries, bas;

  /* set the msgnos for which this sequence is valid:
   * NOTE: if we are on the server, then every message has
   * a uid, but on the client, not ALL uid's may be cached.
   * SO, we build sequences of messages which have uids. */
  if (!mail_sequence_with_uid(stream, msgseq)) {
    new_seq = cpystr("");
    return new_seq;
  }
  /* OK, make the sequence */
  for (entries = 0, i = 1; i <= stream->nmsgs; ++i) {
    if (mail_elt(stream, i)->sequence) {
      entries += 1;
      /* set the first entry */
      if (entries == 1) j = i;
    }
  }
  /* Make sure last_uid is set. Only if not DEAD */
  if (!mort)
    mail_uid(stream, stream->nmsgs);

  /* allocate enough space for n copies of the max uid */
  if (stream->uid_last == 0) ndigs = MAXDIGSPERMSGNO;
  else ndigs = ndigits(stream->uid_last);
  len = (entries * ndigs) + entries + 16;
  buf = new_seq = fs_get(len);
  /* a single hit */
  if (entries == 1) {
    sprintf(new_seq, "%ld", mail_elt(stream, j)->uid);
    return new_seq;
  } else if (entries == 0) {
    *buf = '\0';
    return buf;
  }
  for (bas = entries = 0, i = j; i <= stream->nmsgs; ++i) {
    char tmp[MAXINTBUFLEN];
    if (mail_elt(stream, i)->sequence) {
      /* check consecutive uid */
      if (i < stream->nmsgs && mail_elt(stream, i+1)->sequence &&
	  (mail_elt(stream, i)->uid + 1) == mail_elt(stream, i+1)->uid) {
	if (bas == 0) bas = i;
	continue;
      } else {
	if (entries++ > 0)
	  *buf++ = ',';
	if (bas > 0) {
	  sprintf(tmp, "%ld:%ld", 
		  mail_elt(stream, bas)->uid,
		  mail_elt(stream, i)->uid);
	} else
	  sprintf(tmp, "%ld", mail_elt(stream, i)->uid);
	strcpy(buf, tmp);
	buf += strlen(tmp);
	bas = 0;
      }
    }
  }    
  return new_seq;
}

/*
 * compress message sequence */
char *mail_msgseq (MAILSTREAM *stream, char *msg_seq)
{
  char *new_seq, *buf;
  char c;
  int len;
  int ndigs;
  int i, j, entries, bas;

  /* set the msgnos for which this sequence is valid */
  if (!mail_sequence(stream, msg_seq)) {
    new_seq = cpystr("");
    return new_seq;
  }

  /* OK, make the sequence */
  for (entries = 0, i = 1; i <= stream->nmsgs; ++i) {
    if (mail_elt(stream, i)->sequence) {
      entries += 1;
      /* set the first entry */
      if (entries == 1) j = i;
    }
  }
  /* digits of maximum message number */
  ndigs = ndigits(stream->nmsgs);
  len = (entries * ndigs) + entries + 16;
  buf = new_seq = fs_get(len);
  /* a single hit */
  if (entries == 1) {
    sprintf(new_seq, "%ld", mail_elt(stream,j)->msgno);
    return new_seq;
  }
  for (bas = entries = 0, i = j; i <= stream->nmsgs; ++i) {
    char tmp[MAXINTBUFLEN];
    if (mail_elt(stream, i)->sequence) {
      if (i < stream->nmsgs && mail_elt(stream, i+1)->sequence) {
	if (bas == 0) bas = i;
	continue;
      } else {
	if (entries++ > 0)
	  *buf++ = ',';
	if (bas > 0) {
	  sprintf(tmp, "%ld:%ld", bas, i); 
	} else
	  sprintf(tmp, "%ld", i);
	strcpy(buf, tmp);
	buf += strlen(tmp);
	bas = 0;
      }
    }
  }    
  return new_seq;
}
/*
 * compress message sequence to those which have uids only  */
char *mail_msgseq_with_uid (MAILSTREAM *stream, char *msg_seq)
{
  char *new_seq, *buf;
  char c;
  int len;
  int ndigs;
  int i, j, entries, bas;

  /* set the msgnos for which this sequence is valid */
  if (!mail_sequence_with_uid(stream, msg_seq)) {
    new_seq = cpystr("");
    return new_seq;
  }

  /* OK, make the sequence */
  for (entries = 0, i = 1; i <= stream->nmsgs; ++i) {
    if (mail_elt(stream, i)->sequence) {
      entries += 1;
      /* set the first entry */
      if (entries == 1) j = i;
    }
  }
  /* digits of maximum message number */
  ndigs = ndigits(stream->nmsgs);
  len = (entries * ndigs) + entries + 16;
  buf = new_seq = fs_get(len);
  /* a single hit */
  if (entries == 1) {
    sprintf(new_seq, "%ld", mail_elt(stream,j)->msgno);
    return new_seq;
  }
  for (bas = entries = 0, i = j; i <= stream->nmsgs; ++i) {
    char tmp[MAXINTBUFLEN];
    if (mail_elt(stream, i)->sequence) {
      if (i < stream->nmsgs && mail_elt(stream, i+1)->sequence) {
	if (bas == 0) bas = i;
	continue;
      } else {
	if (entries++ > 0)
	  *buf++ = ',';
	if (bas > 0) {
	  sprintf(tmp, "%ld:%ld", bas, i); 
	} else
	  sprintf(tmp, "%ld", i);
	strcpy(buf, tmp);
	buf += strlen(tmp);
	bas = 0;
      }
    }
  }    
  return new_seq;
}
  
/* Mail set ONE flag
 * Accepts: mail stream
 *	    sequence
 *	    flag(s)
 *	    option flags
 */

void mail_set1flag_full (MAILSTREAM *stream,char *sequence,char *flag,
			 long flags)
{
  /* do the driver's action */
  if (stream->dtb) (*stream->dtb->set1flag) (stream,sequence,flag,flags);
}


/* Mail clear ONE flag
 * Accepts: mail stream
 *	    sequence
 *	    flag(s)
 *	    option flags
 */

void mail_clear1flag_full (MAILSTREAM *stream,char *sequence,char *flag,
			   long flags)
{
  /* do the driver's action */
  if (stream->dtb) (*stream->dtb->clear1flag) (stream,sequence,flag,flags);
}
/*
 * make a from line from the short info */
char *mail_make_from(SHORTINFO *sinfo)
{
  char tmp[MAILTMPLEN];
  char *from;

  if (sinfo->personal)
    sprintf(tmp, "\"%s\" <%s@%s>",sinfo->personal, 
	    (char *)(sinfo->mailbox ? sinfo->mailbox : ""), 
	    (char *)(sinfo->host ? sinfo->host : ""));
  else
    if (sinfo->mailbox && sinfo->host)
      sprintf(tmp, "%s@%s", sinfo->mailbox, sinfo->host);
    else
      tmp[0] = '\0';
  from = cpystr(tmp);
  return from;
}

void mail_free_fshort_data (SHORTINFO *sinfo)
{
  if (sinfo->date) {
    fs_give ((void **)&sinfo->date);
    sinfo->date = NIL;
  }
  if (sinfo->personal) {
    fs_give ((void **)&sinfo->personal);
    sinfo->personal = NIL;
  }
  if (sinfo->mailbox) {
    fs_give ((void **)&sinfo->mailbox);
    sinfo->mailbox = NIL;
  }
  if (sinfo->host) {
    fs_give ((void **)&sinfo->host);
    sinfo->host = NIL;
  }
  if (sinfo->subject) {
    fs_give ((void **)&sinfo->subject);
    sinfo->subject = NIL;
  }
  if (sinfo->bodytype) {
    fs_give ((void **)&sinfo->bodytype);
    sinfo->bodytype = NIL;
  }
  if (sinfo->size) {
    fs_give ((void **)&sinfo->size);
    sinfo->size = NIL;
  }
  if (sinfo->from) {
    fs_give ((void **)&sinfo->from);
    sinfo->from = NIL;
  }
}
/*
 * mail_checksum: 
 *
 * checksum the associated mailbox */
void mail_checksum(MAILSTREAM *stream)
{
  (*stream->dtb->checksum)(stream);
}

/*
 *  BEGIN  Mail sunversion 
 * Accepts: mail stream
 *
 * Returns T if the server supports the sunversion
 */
int
mail_sunversion(MAILSTREAM * stream)
{
  if (stream->dtb) {
    return (*stream->dtb->sunversion)(stream);
  } else {
    return(_B_FALSE);
  }
}

/*
 * mail_imap4server:
 * Accepts: mail stream
 *
 * Returns T if imap4 is supported 
 */
int mail_imap4support(MAILSTREAM *stream)
{
  if (stream && (stream->stream_status & S_OPENED )
      && stream->dtb) {
    if (strcmp(stream->dtb->name,"imap4") == 0) {
      return (*stream->dtb->imap4_supported)(stream);
    }
  }
  return(_B_FALSE);
}

void mail_disconnect(MAILSTREAM *stream)
{
  (*stream->dtb->disconnect)(stream);
}

void mail_clear_iocount (MAILSTREAM *stream)
{
  if (stream && stream->dtb) (*stream->dtb->cleario)(stream);
}

/* BEGIN - Mail fetch short
 * Accepts: Mail stream
 *	    message # to fetch
 *          pointer to shortinfo structure for the data.
 * Returns: T on success with shortinfo supplied.
 */

int
mail_fetchshort_full (MAILSTREAM *stream, unsigned long msgno, 
		      SHORTINFO *user_info,
		      long flags)
{
  if ((msgno < 1 || msgno > stream->nmsgs) &&  !(flags & FT_UID)) {
    mm_log("Bad msgno in mail_fetchshort", ERROR, stream);
    return(_B_FALSE);
  }
  if (stream->dtb) {
    (*stream->dtb->fetchshort) (stream, msgno, user_info, flags);
    if (user_info->date) {
      return(_B_TRUE);
    }
  }
  return(_B_FALSE);
} 
       
/*
 * call the driver routine to increment the urgent 
 * data sending */
void mail_increment_send_urgent(MAILSTREAM *stream)
{
  /* 
   * must test for stream_status since a stream is always there. 
   */
  if (stream && (stream->stream_status & S_OPENED) && stream->dtb)
    (*stream->dtb->urgent) (stream);  
}
/* Mail echo
 *         Attempts to open a connection to the server.
 *         If successful, then closes the connection.
 *         Just verify the path exists.
 * Accepts: 
 *	    mailbox name
 * Returns: T if a connection succeeds, else NIL
 */

int mail_echo (char *name)
{
  long i;
  char tmp[MAILTMPLEN];
  DRIVER *factory=NULL;
  MAILSTREAM *stream=NULL;
  int success;
  /* a NIL stream is passed in this mail_valid call. This would normally
     endanger the subsequent call (*factory->valid) (mailbox,stream)
     which needs to reference the globals on the stream. But this case
     is special since it's calling imap_valid which doesn't require
     any global references on the stream. So it's safe.
     */
  factory =  mail_valid (NIL, name, "echo imap4d");
  if (factory) {		/* must have a factory */
    stream = mail_stream_create(NIL,NIL,NIL,NIL,NIL); /*initialize stream */
    stream->dtb = factory;	/* set dispatch */
    stream->mailbox = cpystr (name); /* set mailbox name */
  } else {
    sprintf(tmp, "ECHO: No driver available for %s", name);
    mm_log (tmp, WARN,stream);
    return NIL;
  }
  /* OK, try and connect to the server */
  success = (*factory->serverecho) (stream);

  fs_give ((void **)&stream->mailbox);

  /* Remember to give up all the globals on this tmp stream. */
  mail_stream_flush(stream);  

  return success;
}
/* Mail expand UID sequence
 * Accepts: mail stream
 *	    sequence to expand
 * Returns: expanded sequence array (must be freed by caller).
 * EG:  1000:1003,2001 ==> 1001,1001,1002,1003,2001
 */

ulong *mail_expand_uid_sequence (MAILSTREAM *stream,char *sequence)
{
  unsigned long i,j,k,x;
  int n = 0;
  char *seq0 = sequence;	/* save ptr to start */
  ulong *e0, *epanouir;
  /* first count the number of elements in the
   * expanded sequence */
  while (*sequence) {		/* while there is something to parse */
    if (*sequence == '*') {	/* maximum message */
      i = stream->nmsgs ? mail_uid (stream,stream->nmsgs) : stream->uid_last;
      sequence++;		/* skip past * */
      n += 1;
    }
    /* parse and validate message number */
    else if (!(i = strtoul ((const char *) sequence, &sequence,10))) {
      mm_log ("UID sequence invalid",ERROR,stream);
      return NIL;
    }
    switch (*sequence) {	/* see what the delimiter is */
    case ':':			/* sequence range */
      if (*++sequence == '*') {	/* maximum message */
	j = stream->nmsgs ? mail_uid (stream,stream->nmsgs) : stream->uid_last;
	sequence++;		/* skip past * */
      }
      /* parse end of range */
      else if (!(j = strtoul ((const char *) sequence,&sequence,10))) {
	mm_log ("UID sequence range invalid",ERROR,stream);
	return NIL;
      }
      if (*sequence && *sequence++ != ',') {
	mm_log ("UID sequence range syntax error",ERROR,stream);
	return NIL;
      }
      if (i > j) {		/* swap the range if backwards */
	x = i; i = j; j = x;
      }
      n += j - i + 1;
      break;
    case ',':			/* single message */
      ++sequence;		/* skip the delimiter, fall into end case */
    case '\0':			/* end of sequence, mark this message */
      n += 1;
      break;
    default:			/* anything else is a syntax error! */
      mm_log ("UID sequence syntax error",ERROR,stream);
      return NIL;
    }
  }
  /* allocate the space */
  e0 = epanouir = (ulong *)fs_get (sizeof(ulong) * (n + 1));
  /* store the sequence numbers */
  sequence = seq0;
  while (*sequence) {		/* while there is something to parse */
    if (*sequence == '*') {	/* maximum message */
      i = stream->nmsgs ? mail_uid (stream,stream->nmsgs) : stream->uid_last;
      sequence++;		/* skip past * */
      *e0++ = i;		/* save the value */
    }
    /* parse and validate message number */
    else i = strtoul ((const char *) sequence,&sequence,10);
    switch (*sequence) {	/* see what the delimiter is */
    case ':':			/* sequence range */
      if (*++sequence == '*') {	/* maximum message */
	j = stream->nmsgs ? mail_uid (stream,stream->nmsgs) : stream->uid_last;
	sequence++;		/* skip past * */
      }
      /* parse end of range */
      else { 
	j = strtoul ((const char *) sequence,&sequence,10);
	if (*sequence == ',') ++sequence;
      }
	
      if (i > j) {		/* swap the range if backwards */
	x = i; i = j; j = x;
      }
      /* store i, i+1, ..., j */
      for (k = i; k <= j; ++k)
	*e0++ = k;
      break;
    case ',':			/* single message */
      ++sequence;		/* skip the delimiter, fall into end case */
    case '\0':			/* end of sequence, mark this message */
      *e0++ = i;
      break;
    }
  }
  return epanouir;		/* successfully parsed sequence */
}

/*
 * Return T if a checkpointed file name can
 * be copied into buf else NIL */
int mail_cached_filename(char *name,char *buf,int buflen, MAILSTREAM *stream)
{
  return NIL;
}

/*
 * create a prototype stream:
 *   parameters  driver pointer
 * Returns prototype stream with driver set 
 * NOTE: This is to allow a stream with FULL stream structure
 *       for prototypes.
 */
MAILSTREAM *mail_prototype(MAILSTREAM *pstream)
{
  MAILSTREAM *stream;

  /* Be sure the pstream exists; else we're in trouble. */
  if (pstream ) stream = mail_stream_create(pstream->userdata,
					    NIL,NIL,NIL,NIL);
  if (pstream && (pstream->stream_status & S_OPENED) && pstream->dtb) 
    stream->dtb = pstream->dtb;
  else stream->dtb = default_proto(pstream)->dtb;
  /* use pstream here  */

  return stream;
}

/*
 * Return T if short info is there. Otherwise NIL.
 */
int
mail_sinfo_present(MAILSTREAM *stream,unsigned long msgno,unsigned long flags)
{
  
  if ((msgno < 1 || msgno > stream->nmsgs) &&  !(flags & FT_UID)) {
    mm_log("Bad msgno in mail_fetchshort", ERROR, stream);
    return(_B_FALSE);
  }
  if (stream->scache) {
    return(_B_FALSE);
  } else if (flags & FT_UID) {
    unsigned long 	i;

    for (i = 1; i <= stream->nmsgs; ++i) {
      if (mail_elt (stream,i)->uid == msgno) {
	return(mail_sinfo_present(stream, i, flags &= ~FT_UID));
      }
    }
    return(_B_FALSE);
  } else {

    LONGCACHE *	lelt = mail_lelt(stream,msgno);

    if (lelt->sinfo.date) {
      return(_B_TRUE);
    } else {
      return(_B_FALSE);
    }
  }
}

/*
 * Free any driver buffers that are volatile over
 * calls:
 *   mail_fetch*(stream, ...);
 * BEST called AFTER a sequence is done because the driver
 * strategy is to use the same buffers until freed, resizing them
 * upwards as necessary. So, one can be left with the largest 
 * possible buffer.
 *
 * Currently, only applies to solaris driver.
 * 
 * It is more efficient to use a sequence, use_sequence == T.
 */
void mail_free_cached_buffers_seq(MAILSTREAM *stream,char *seq,
				  int use_sequence)
{
  unsigned long i;
  if (stream && (stream->stream_status & S_OPENED)) {
    /* use sequence if indicated */
    if (use_sequence && !mail_sequence(stream,seq)) 
      return;
    /*
     * Free up internal buffers */
    for (i = 1; i <= stream->nmsgs; ++i)
      if (use_sequence && mail_elt(stream,i)->sequence)
	mail_free_cached_buffers_msg(stream,i); 
      else mail_free_cached_buffers_msg(stream,i);
  }
}

/*
 * mail_driver_freebody:
 *  Used only by the imapd for now. Hook for freeing 
 *  large body allocations. */
void mail_free_cached_buffers_msg(MAILSTREAM *stream,unsigned long msgno)
{
  if (stream && (stream->stream_status & S_OPENED) && 
      stream->dtb && stream->dtb->driver_freebody)
    (*stream->dtb->driver_freebody)(stream,msgno);

}

/* Hierarchy delimiter used in mm_list */
int mail_get_hierarchy_dlm(MAILSTREAM *stream)
{
  if (stream && (stream->stream_status & S_OPENED) ) 
    return stream->hierarchy_dlm;
  else return NIL;
}
void mail_set_hierarchy_dlm(MAILSTREAM *stream,int dlm)
{
  if (stream && (stream->stream_status & S_OPENED))
    stream->hierarchy_dlm = dlm;
}

/* mail globals initializer
 * Accepts: MAIL stream
 * Returns: MAIL stream
 */
void mail_global_init( MAILSTREAM *stream)
{
  MAIL_GLOBALS *mg = (MAIL_GLOBALS *)fs_get(sizeof(MAIL_GLOBALS));
  memset ((void *)mg,0,sizeof(MAIL_GLOBALS));

  mg->mailgets = NIL;
  mg->userdata = NIL;
  mg->savecachepath = NIL;
  mg->sstring = NIL;
  mg->scharset = NIL;

  stream->mail_globals = mg;

}

/*
 Input:
 *      Set/clear tool talk locking
 *        int (*set_ttlock)(void);      
 *        int (*clear_ttlock)(void); 
 *      Set/clear group access
 *        int (*set_group_access)(void);
 *        int (*clear_group_access)(void);
  Output: a stream with all the globals variables initialized.

*/
MAILSTREAM *mail_stream_create( void *udata,
				int (*set_ttlock)(char *mailbox, void *userdata),
				int (*clear_ttlock)(char *mailbox, void *userdata),
				int (*set_group_access)(void),
				int (*clear_group_access)(void))
{
  DRIVER *d;

  MAILSTREAM *stream = (MAILSTREAM *) fs_get (sizeof (MAILSTREAM));
  memset ((void *) stream,0,sizeof (MAILSTREAM));

  env_global_init(stream);  /*init env globals */
  mail_global_init(stream); /*init mail globals*/
  tcp_global_init(stream);  /*init tcp globals */

  /* Initialize the driver globals  */
  for (d = maildrivers; d; d = d->next)
    (d->global_init)(stream);

  stream->stream_status = 0;		/* Clear it and */
  stream->stream_status |= S_CREATED;   /* set it to S_CREATED only.*/

  /* Now set the parameters */
  stream->userdata = udata;
  /* Passed function calls */
  stream->set_ttlock = set_ttlock;
  stream->clear_ttlock = clear_ttlock;
  stream->set_group_access = set_group_access;
  stream->clear_group_access = clear_group_access;

  return stream;
}

/*
 * Set and Unset the stream_status bit -- used mainly for
 * passing a NIL stream to mail_parameters.
 *
 */
void mail_stream_setNIL (MAILSTREAM *stream )
{
  stream->stream_status |= S_NILCALL;
}

void mail_stream_unsetNIL(MAILSTREAM *stream )
{
  stream->stream_status &= ~S_NILCALL;
}
/*

  Input: a mailstream.
  Return: None.
  Purpose: flush the globals on the globals data structure.
*/

void mail_global_free(MAILSTREAM *stream)
{
  if (stream->mail_globals) 
    fs_give ((void **) &stream->mail_globals);
}

/*
  Input: a non-empty stream.
  Output: a NIL stream.
  Purpose: We need to flush all the initialized globals
	on the stream even though the stream has not
	been used. There are cases where the stream is 
	created but not opened. Thus mail_close is not called
	to flush the globals. Then memory leaks may occur.
*/

void mail_stream_flush (MAILSTREAM *stream)
{
  DRIVER *d;

  if (stream ) {
    if (stream->mailbox ) fs_give ( ( void ** ) &stream->mailbox );

    env_global_free(stream);
    mail_global_free(stream);
    tcp_global_free(stream);

    /* Flush the driver globals  */
    for (d = maildrivers; d; d = d->next)
      (d->global_free)(stream);

    fs_give ((void **) &stream); /* Finally the stream. */	
  }
}
