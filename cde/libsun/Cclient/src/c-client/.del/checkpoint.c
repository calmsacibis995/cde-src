/*
 * Program:  Checkpoint c-client cache to disk routines.
 *
 * Author:   Bill Yeager - SunSoft
 *
 * Copyright 1995 by Sun Micro Systems, Inc. All rights reserved.
 * Edit -   
 *    (1) UNLINK if an error occured - 8-oct-96 Bill.
 *	[10-21-96, clin] Passing stream in routine:
 *			  make_checkpoint_name
 *			  mail_restore_cpcache
 *			  mail_checkpoint_filename
 *			  mail_parameters
 *	[01-03-97, clin] Call mail_stream_setNIL. 
 */
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <sys/param.h>
#include "mail.h"
#include "os_sv5.h"
#include "misc.h"
#include "checkpoint.h"

typedef struct _data_blk_ {
  unsigned char *buf;
  int offset;
} DATAPAGE;

int cp_rebuild_cache(FILE *f, MAILSTREAM *stream);
static void cp_free_stream(MAILSTREAM *stream);
static int cp_rebuild_elt(FILE *f, MAILSTREAM *stream, unsigned long msgno);
int cp_read_elt(FILE *f,MESSAGECACHE *elt,unsigned long msgno);

DATAPAGE wrt_buf;
#define CPPAGESIZE 8192
/*
 * The checkpointing call
 */
int mail_checkpoint_cache(MAILSTREAM *stream, int *errno)
{
  char buf[MAXPATHLEN+1];
  FILE *f;
  int ret;
  long t0 = elapsed_ms();
  char *cpfile = checkpoint_filename(stream, buf, MAXPATHLEN+1);
  if (!cpfile) {
    if (errno) *errno = CHECKPOINT_SYSERR;
    return NIL;
  }
  /*
   * remove any already existing such file */
  if (f = fopen(cpfile, "r")) {
    if (errno) *errno = CHECKPOINT_SYSERR;
    if (fclose(f) != 0)  return NIL;
    else if (chmod(cpfile, 0644) < 0) 
      return NIL;
    else if (unlink(cpfile) < 0) 
      return NIL;
  }
  /*
   * See if we have enough disk space */
  if (!(f = checkpoint_spaceok(stream, cpfile))) {
    if (errno) *errno =  CHECKPOINT_DISKFULL;
    return NIL;
  }
  /* Allocate our write buffer */
  wrt_buf. buf = (unsigned char *)fs_get(CPPAGESIZE+sizeof(long));
  /*
   * OK, checkpoint the cache */
  ret = checkpoint_cache(stream, f);
  fs_give((void **)&wrt_buf.buf);
  if (ret == NIL) {
    if (errno) *errno = CHECKPOINT_FAILED;
    fclose(f);
  } else {
    if (errno) *errno = CHECKPOINT_SYSERR;
    if (fsync(fileno(f)) < 0) ret = NIL;
    else if (fclose(f) != 0) ret = NIL;
    else if (chmod(cpfile, 0444) < 0) ret = NIL;
    else {
      mm_cache_size(stream,elapsed_ms()-t0,T);
      ret = T;
    }
  }
  /* UNLINK if an error occured - 8-oct-96 */
  if (!ret) unlink(cpfile);
  return ret;
}

/*
 * The restore call
 */
MAILSTREAM *mail_restore_cpcache(char *mbox, int *errno, MAILSTREAM *stream)
{
  char buf[MAXPATHLEN+1];
  FILE *f;
  MAILSTREAM *rstream;
  unsigned long csize;
  void *userdata;
  mail_stream_setNIL(stream);
  make_checkpoint_name(mbox, buf, MAXPATHLEN+1, stream);
  if (!(f = fopen(buf, "r"))) {
    if (errno) *errno = MBOXNOTCACHED;
    return NIL;
  }
  /*
   * Verify the checksum */
  if (!verify_cache_checksum(f,&csize)) {
    if (errno) *errno = CACHECORRUPTED;
    return NIL;
  }
  /*
   * First restore the MAILSTREAM */
  rstream = cp_restore_stream(f,errno);
  if (!rstream) return NIL;
  userdata = mail_parameters(stream,GET_USERDATA,NIL);
  mail_stream_unsetNIL(stream);
  mm_restore_callback(userdata,csize,rstream->nmsgs);
  /*
   * OK, now on y va! */
  if (!cp_rebuild_cache(f,rstream)) {
    cp_free_stream(rstream);
    if (errno) *errno = CACHECORRUPTED;
    return NIL;
  }
  /*
   * chmod to write and unlink the file */
  if (fclose(f) != EOF && chmod(buf,0644) == 0)
    unlink(buf);
  return rstream;
}
/*
 * free what we allocated */
static void cp_free_stream(MAILSTREAM *stream)
{
  if (stream->mailbox) fs_give ((void **) &stream->mailbox);
  if (stream->flagstring) fs_give ((void **) &stream->flagstring);
  mail_free_cache (stream);	/* finally free the stream's storage */
  fs_give ((void **) &stream);
}

/*
 * Rebuild the cache */
int cp_rebuild_cache(FILE *f, MAILSTREAM *stream)
{
  unsigned long i;
  /* First, allocate the cache elements */
  stream->cachesize = 0;		/* force reallocation */
  for (i = 1; i <= stream->nmsgs; ++i) {
    if (stream->scache) {
      if (!cp_rebuild_elt(f,stream,i)) return NIL;
    } else {
      if ((i % 10) == 0) {
	void *userdata;
	mail_stream_setNIL(stream);
	userdata = mail_parameters(stream,GET_USERDATA,NIL);
	mail_stream_unsetNIL(stream);
	mm_restore_callback(userdata, 0,i);
      }
      if (!cp_rebuild_lelt(f,stream,i)) return NIL;
    }
  }
  return T;
}
/*
 * Read a MESSAGECACHE elt from the cache */
static int cp_rebuild_elt(FILE *f, MAILSTREAM *stream, unsigned long msgno)
{
  MESSAGECACHE *elt =  mail_elt(stream,msgno);
  if (!cp_read_elt(f,elt,msgno)) return NIL;
  else return T;
}
/*
 * Read a LONGCACHE element from the cache */
int cp_rebuild_lelt(FILE *f, MAILSTREAM *stream, unsigned long msgno)
{
  LONGCACHE *lelt = mail_lelt(stream,msgno);
  if (!cp_read_lelt(f,lelt,msgno)) return NIL;
  else return T;
}

int cp_read_elt(FILE *f,MESSAGECACHE *elt,unsigned long msgno)
{
  char rbuf[READBUFLEN];
  int index;
  unsigned long fmsgno;
  int i;
  /* Get the first line. Validate msgno */
  if (!fgets(rbuf, READBUFLEN, f)) return NIL;
  if (strncmp(rbuf,"NILELT=",NLEN) == 0)
    index = NLEN;
  else if (strncmp(rbuf,"ELT=",ELEN) == 0)
    index = ELEN;
  else return NIL;
  fmsgno = strtoul(&rbuf[index],NIL,10);
  if (fmsgno != msgno) return NIL;
  else if (index == NLEN) return T;
  /* OK. Valid msgno and some data to stock in the elt */
  if (!cp_read_ulong(f,&elt->sync_msgno)) return NIL; /* sync_msgno */
  if (!cp_read_ulong(f,&elt->uid)) return NIL;
  if (!cp_read_int(f,&i)) return NIL;
  else elt->hours = i;
  if (!cp_read_int(f,&i)) return NIL;
  else elt->minutes = i;
  if (!cp_read_int(f,&i)) return NIL;
  else elt->seconds = i;
  if (!cp_read_int(f,&i)) return NIL;
  else elt->zoccident = i;
  if (!cp_read_int(f,&i)) return NIL;
  else elt->zhours = i;
  if (!cp_read_int(f,&i)) return NIL;
  else elt->zminutes = i;
  if (!cp_read_int(f,&i)) return NIL;
  else elt->seen = i;
  if (!cp_read_int(f,&i)) return NIL;
  else elt->deleted = i;
  if (!cp_read_int(f,&i)) return NIL;
  else elt->flagged = i;
  if (!cp_read_int(f,&i)) return NIL;
  else elt->answered = i;
  if (!cp_read_int(f,&i)) return NIL;
  else elt->draft = i;
  if (!cp_read_int(f,&i)) return NIL;
  else elt->valid = i;
  if (!cp_read_int(f,&i)) return NIL;
  else elt->recent = i;
  if (!cp_read_int(f,&i)) return NIL;
  else elt->searched = i;
  if (!cp_read_int(f,&i)) return NIL;
  else elt->sequence = i;
  if (!cp_read_int(f,&i)) return NIL;
  else elt->spare = i;
  if (!cp_read_int(f,&i)) return NIL;
  else elt->spare2 = i;
  if (!cp_read_int(f,&i)) return NIL;
  else elt->spare3 = i;
  /* Lockcount not read. It is by default 1.
   *  See *messagecache(stream,msngo,MAKE_ELT) */
  if (!cp_read_int(f,&i)) return NIL;
  else elt->day = i;
  if (!cp_read_int(f,&i)) return NIL;
  else elt->month = i;
  if (!cp_read_int(f,&i)) return NIL;
  else elt->year = i;
  if (!cp_read_ulong(f,&elt->user_flags)) return NIL;
  if (!cp_read_ulong(f,&elt->rfc822_size)) return NIL;
  if (!cp_read_data(f,(char **)&elt->data1,&elt->data3)) return NIL;
  if (!cp_read_data(f,(char **)&elt->data2,&elt->data4)) return NIL;
  if (!cp_read_data(f,(char **)&elt->filter1,&elt->filter2))
    return NIL;
  else return T;
}

int cp_read_lelt(FILE *f, LONGCACHE *lelt,unsigned long msgno)
{
  /* first read the elt */
  if (!cp_read_elt(f,&lelt->elt,msgno)) return NIL;
  if (!cp_read_env(f,&lelt->env)) return NIL;
  if (!cp_read_bodylist(f,&lelt->body)) return NIL;
  if (!cp_read_sinfo(f,&lelt->sinfo)) return NIL;
  else return T;
}

int cp_read_env(FILE *f,ENVELOPE **envel)
{
  ENVELOPE *env;
  char rbuf[READBUFLEN];
  if (!fgets(rbuf, READBUFLEN, f)) return NIL;
  /* See if NIL */
  if (strncmp(rbuf,NILENV,NLEN) == 0) return T;
  else if (strcmp(rbuf,ISENV)!= 0) return NIL;
  env = *envel = mail_newenvelope();
  /* OK, read the envelope slots */
  if (!cp_read_string(f,&env->remail)) return NIL;
  if (!cp_read_address(f,&env->return_path)) return NIL;
  if (!cp_read_string(f,&env->date)) return NIL;
  if (!cp_read_address(f,&env->from)) return NIL;
  if (!cp_read_address(f,&env->sender)) return NIL;
  if (!cp_read_address(f,&env->reply_to)) return NIL;
  if (!cp_read_string(f,&env->subject)) return NIL;
  if (!cp_read_address(f,&env->to)) return NIL;
  if (!cp_read_address(f,&env->cc)) return NIL;
  if (!cp_read_address(f,&env->bcc)) return NIL;
  if (!cp_read_string(f,&env->in_reply_to)) return NIL;
  if (!cp_read_string(f,&env->message_id)) return NIL;
  if (!cp_read_string(f,&env->newsgroups)) return NIL;
  /* OK, verify the END */
  if (!fgets(rbuf, READBUFLEN, f)) return NIL;
  else if (strcmp(rbuf,ENVEND) != 0) return NIL;
  else return T;
}

int cp_read_address(FILE *f, ADDRESS **adr)
{
  int n;
  ADDRESS **ap = adr;
  char rbuf[READBUFLEN];
  if (!fgets(rbuf, READBUFLEN, f)) return NIL;
  else if (strncmp(ISADDR,rbuf,DDLEN) != 0) return NIL;
  else n = atoi(&rbuf[DDLEN]);
  if (n == 0) return T;			/* no addresses in list */
  while (n--) {
    *ap = mail_newaddr();
    if (!cp_read_string(f,&(*ap)->personal)) return NIL;
    if (!cp_read_string(f,&(*ap)->adl)) return NIL;
    if (!cp_read_string(f,&(*ap)->mailbox)) return NIL;
    if (!cp_read_string(f,&(*ap)->host)) return NIL;
    if (!cp_read_string(f,&(*ap)->error)) return NIL;
    ap = &(*ap)->next;
  }    
  return T;
}
/* to be finished */
int cp_read_bodylist(FILE *f, BODY **b)
{
  char rbuf[READBUFLEN];
  if (!fgets(rbuf, READBUFLEN, f)) return NIL;
  /* check for NIL body */
  if (strcmp(rbuf,NILBODY) == 0) return T;
  /* OK, read in the body definition */
  if (strcmp(rbuf,BODYDESC) != 0) return NIL;
  if (!cp_read_body(f,b)) return NIL;
  if ((*b)->type == TYPEMULTIPART)
    return cp_read_parts(f,&(*b)->contents.part);
  else return T;
}

int cp_read_sinfo(FILE *f,SHORTINFO *sinfo)
{

  if (!cp_read_string(f,&sinfo->date)) return NIL;
  if (!cp_read_string(f,&sinfo->personal)) return NIL;
  if (!cp_read_string(f,&sinfo->mailbox)) return NIL;
  if (!cp_read_string(f,&sinfo->host)) return NIL;
  if (!cp_read_string(f,&sinfo->subject)) return NIL;
  if (!cp_read_string(f,&sinfo->bodytype)) return NIL;
  if (!cp_read_string(f,&sinfo->size)) return NIL;
  if (!cp_read_string(f,&sinfo->from)) return NIL;
  else return T;  
}

/*
 * Approximate the space requirements, and allocate the
 * file space */
static FILE *checkpoint_spaceok(MAILSTREAM *stream, char *cpfile)
{
  /*
   * Run through the cache to approximate the space requirements
   */
  unsigned long space = sizeof(MAILSTREAM);
  unsigned long i;
  FILE *f;

  for (i = 1; i <= stream->nmsgs; ++i) {
    MESSAGECACHE *elt = mail_elt(stream, i);
    space += checkpoint_mcache_size(elt);
    if (!stream->scache)
      space += checkpoint_lelt_size (mail_lelt(stream,i));
  }
  /* Open the file for create */
  f = fopen(cpfile,"w+");
  if (f) {
    /* Notify the caller of the cache size */
    mm_cache_size(stream,space,NIL);
    return f;
  } else return NIL;
}

static int checkpoint_cache(MAILSTREAM *stream, FILE *f)
{
  /* write space for our checksum */
  rewind(f);
  if (fprintf(f,"0123456789 12345\n") < 0) return NIL;
  /*  checkpoint away. On y fout la donnee */
  if (!checkpoint_write_stream(f,stream)) return NIL;
  /* OK, checkpoint the caches */
  if (!checkpoint_write_caches(f,stream)) return NIL;
  /* OK, checksum the file for a sanity check */
  rewind(f);				/* beginning of file */
  if (!checksum_cache(f)) return NIL;
  return T;
}

union {
  unsigned long l;
  CKSHORTS ck_s;
} cache_check_sum;

void cp_checksum_cache(FILE *f, unsigned long *size, 
		       unsigned short *cksum)
{
  char c;
  *size = 0;
  TRENTE_DEUX = 0L;
  while ((c = fgetc(f)) != EOF) {
    *size += 1;
    TRENTE_DEUX += c;
    if (CARRY) {			/* 1's complement + */
      CARRY = 0;
      CHECKSUM += 1;
    }
    TRENTE_DEUX <<= 1;			/* we now rotate */
    if (CARRY) {
      CARRY = 0;
      CHECKSUM += 1;
    }
  }
  *cksum = CHECKSUM;
}

int checksum_cache(FILE *f)
{
  char buf[READBUFLEN];
  unsigned long size;
  unsigned short cksum;
  int c;
  /* skip first line. This is where we will save or checksum */
  if (!fgets(buf, READBUFLEN, f)) return NIL;
  cp_checksum_cache(f,&size,&cksum);
  /* write the size and checksum */
  rewind(f);
  if (fprintf(f,"%010lu %05u\n",size,cksum) < 0)
    return NIL;
  else
    return T;
}

int verify_cache_checksum(FILE *f,unsigned long *csize)
{
  char buf[READBUFLEN];
  unsigned short fchksum,checksum;
  unsigned long fsize,size;
  int c;
  char *b;
  rewind(f);				/* beginning of file */
  /* read size, checksum */
  if (!fgets(buf, READBUFLEN, f)) return NIL;
  fsize = strtoul(buf,&b,10);
  if (b) {
    ++b;
    fchksum = (unsigned short)strtoul(b,NIL,10);
  } else return NIL;
  cp_checksum_cache(f,&size,&checksum);
  /* Make sure all is intact */
  if (size != fsize || fchksum != checksum) return NIL;
  /* place cache at line after checksum info */
  rewind(f);
  if (!fgets(buf, READBUFLEN, f)) return NIL;
  else {
    *csize = size;
    return T;
  }
}

unsigned long checkpoint_mcache_size(MESSAGECACHE *elt)
{
  unsigned long size = sizeof (MESSAGECACHE);
  size += elt->data3 + elt->data4 + elt->filter2;
  return size;
}

unsigned long checkpoint_lelt_size(LONGCACHE *lelt)
{
  /* get the envelope size */
  unsigned long size = checkpoint_env_size (lelt->env);
  /* And roundeur du corps (: */
  checkpoint_body_size (lelt->body, &size);
  return size;
}
/*
 * routines to check sizes of various c-client structures */
unsigned long checkpoint_body_size(BODY *body, unsigned long *size)
{
  PART *part;

  if (body == NIL) return 0L;
  /* We have a body - et plutot beau */
  *size += checkpoint_string_size(body->subtype);
  *size += checkpoint_parameter_size(body->parameter);
  *size += checkpoint_string_size(body->id);
  *size += checkpoint_string_size(body->description);
  switch (body->type) {		/* free contents */
  case TYPETEXT:		/* unformatted text */
    if (body->contents.text) *size += body->size.bytes;
    break;
  case TYPEMULTIPART:		/* multiple part */
    if (part = body->contents.part) do {
      checkpoint_body_size (&part->body, size);
    } while (part = part->next);
    break;
  case TYPEMESSAGE:		/* encapsulated message */
    if (body->contents.msg.text ||
	body->contents.msg.hdr)
      *size += body->size.bytes;
    break;
  case TYPEAPPLICATION:		/* application data */
  case TYPEAUDIO:		/* audio */
  case TYPEIMAGE:		/* static image */
  case TYPEVIDEO:		/* video */
    if (body->contents.binary) 
      *size += body->size.bytes;      
    break;
  default:
    break;
  }
}

unsigned long checkpoint_env_size(ENVELOPE *env)
{
  unsigned long size = 0L;
  if (env) {
    checkpoint_string_size(env->remail);
    size += checkpoint_address_size(env->return_path);
    size += checkpoint_string_size(env->date);
    size += checkpoint_address_size(env->from);
    size += checkpoint_address_size(env->sender);
    size += checkpoint_address_size(env->reply_to);
    size += checkpoint_string_size(env->subject);
    size += checkpoint_address_size(env->to);
    size += checkpoint_address_size(env->cc);
    size += checkpoint_address_size(env->bcc);
    size += checkpoint_string_size(env->in_reply_to);
    size += checkpoint_string_size(env->message_id);
    size += checkpoint_string_size(env->newsgroups);
  }
  return size;
}

unsigned long checkpoint_string_size(char *str)
{
  if (str) return (unsigned long)strlen(str);
  else return 0L;
}
unsigned long checkpoint_address_size(ADDRESS *address)
{
  unsigned long size = 0L;
  while (address) {
    if (address->personal) size += strlen(address->personal);
    if (address->adl) size += strlen(address->adl);
    if (address->mailbox) size += strlen(address->mailbox);
    if (address->host) size += strlen(address->host);
    if (address->error) size += strlen(address->error);
    address = address->next;
  }
  return size;
}

unsigned long checkpoint_parameter_size(PARAMETER *p)
{
  unsigned long size = 0L;
  while (p) {
    size += checkpoint_string_size(p->attribute);
    size += checkpoint_string_size(p->value);
    p = p->next;
  }
  return size;
}

/* Interface to mail_* calls */
char *mail_checkpoint_filename(char *name, char *buf, int len, MAILSTREAM *stream)
{
  /* make sure there is a buffer here */
  if (buf == NIL) return NIL;
  else if (make_checkpoint_name(name,buf,len, stream)) return buf;
  else return NIL;
}

/* internal call: our buffer is sufficient */
static char *checkpoint_filename(MAILSTREAM *stream,char *buf,int len) 
{
  make_checkpoint_name(stream->mailbox,buf,len, stream);
  return buf;
}
/* Do length check and return NIL if buf is too small */
static int make_checkpoint_name(char *mbox, char *buf, int blen, MAILSTREAM *stream)
{
  char *mbname, c;
  char *subdir = (char *)mail_parameters(stream,GET_SAVECACHEPATH,NIL);
  char *chez_moi = myhomedir(stream);
  int the_len = strlen(CACHEPREFIX)+strlen(chez_moi)+strlen(mbox);
  if (subdir) {
    the_len += strlen(subdir)+3;
    if (the_len > blen) return NIL;
    sprintf(buf,"%s/%s/%s",chez_moi,subdir,CACHEPREFIX);
  } else {					/* make hidden on homedir */
    the_len += 4;
    if (the_len > blen) return NIL;
    sprintf(buf, "%s/%s/.",chez_moi,CACHEPREFIX);
  }
  mbname = buf + strlen(buf);
  strcat(mbname, mbox);
  /* replace '/' with '\', '*', '{' or '}' with # */
  while (c = *mbname) {
    if (c == '/') *mbname = '\\';
    else if (c == '*' || c == '{' || c == '}')
      *mbname = '#';
    ++mbname;
  }
  return T;
}

/*
 * checkpoint the stream variables */
int checkpoint_write_stream(FILE *f, MAILSTREAM *stream)
{
  /* header line */
  if (fprintf(f,"%s%s%ld\n", STREAMDESC,ISULONG,stream->nmsgs) < 0)
    return NIL;
  /* OK, cache the values */
  if (!cp_write_string(f,stream->mailbox)) return NIL;
  if (!cp_write_int(f,0)) return NIL;	/* use */
  if (!cp_write_int(f,0)) return NIL;	/* sequence */
  if (!cp_write_int(f,0)) return NIL;	/* lock */
  if (!cp_write_int(f,0)) return NIL;	/* debug */
  if (!cp_write_int(f,0)) return NIL;	/* silent */
  if (!cp_write_int(f,stream->anonymous)) return NIL; 
  if (!cp_write_int(f,stream->scache)) return NIL;
  if (!cp_write_int(f,0)) return NIL;	/* halfopen */
  if (!cp_write_int(f,stream->perm_seen)) return NIL;
  if (!cp_write_int(f,stream->perm_deleted)) return NIL;
  if (!cp_write_int(f,stream->perm_flagged)) return NIL;
  if (!cp_write_int(f,stream->perm_answered)) return NIL;
  if (!cp_write_int(f,stream->perm_draft)) return NIL;
  if (!cp_write_int(f,stream->kwd_create)) return NIL;
  /* record_resume not cached */
  if (!cp_write_ulong(f,stream->perm_user_flags)) return NIL;
  if (!cp_write_ulong(f,stream->gensym)) return NIL;
  if (!cp_write_ulong(f,stream->nmsgs)) return NIL;
  if (!cp_write_ulong(f,stream->recent)) return NIL;
  if (!cp_write_ulong(f,stream->cached_nmsgs)) return NIL;
  if (!cp_write_ulong(f,stream->uid_validity)) return NIL;
  if (!cp_write_ulong(f,stream->uid_last)) return NIL;
  if (!cp_write_string(f,stream->flagstring)) return NIL;
  if (!cp_write_uflags(f,stream->user_flags)) return NIL;
  if (!cp_write_ulong(f,0L)) return NIL; /* cachesize */
  /* We do not write a values for:
   *  (1) cache union 
   *  (2) *env, *body, *text [short cache stuff]
   *  (3) userdata, sinfo, checkpointed
   * These are set at the cache restore time */
  if (!cp_write_int(f,stream->record)) return NIL;  
  if (!cp_write_int(f,stream->imap4_recording_session)) return NIL;  
  if (!cp_write_ulong(f,stream->checksum)) return NIL;
  if (!cp_write_ulong(f,stream->mbox_size)) return NIL;
  if (!cp_write_string(f,stream->driver_name)) return NIL;
  if (!cp_write_int(f,stream->sunvset)) return NIL;  
  if (!cp_write_int(f,stream->keep_mime)) return NIL;  
  /* We do not cache values for
   *   validchecksum
   *   checksum_type
   *   send_checksum
   *   mail_send_urgent
   *   imap_send_urgent
   *   input_flushed
   *   min_dirty_msgno
   *   have_all_sinfo */
  if (!cp_write_int(f,stream->secondary_mbox)) return NIL;  
  if (!cp_write_string(f,stream->sync_uidseq)) return NIL;
  if (fputs(STREAMEND,f) == EOF) return NIL;
  return T;
}

/* Write the MESSAGE cache for each message, and
 * LONG cache if being used (99.99%) */
int checkpoint_write_caches(FILE *f,MAILSTREAM *stream) 
{
  int i;
  if (stream->scache) {
    for (i = 1; i < stream->nmsgs; ++i)
      if (!cp_write_elt(stream,mail_elt(stream,i),f)) return NIL;
  } else {
    for (i = 1; i <= stream->nmsgs; ++i) {
      if (i % 10 == 0) {
	mm_io_callback(stream,i,CBDISKIO);
	fflush(f);
      }
      if (!cp_write_lelt(stream,mail_lelt(stream,i),f)) return NIL;
    }
  }
  if (cp_write_end(f)) return T;
  else return NIL;
}

int cp_write_end(FILE *f)
{
  if (fprintf(f,CACHEEND) < 0) return NIL;
  else return T;
}

int cp_write_lelt(MAILSTREAM *stream,LONGCACHE *lelt, FILE *f)
{
  if (!cp_write_elt(stream,&lelt->elt,f)) return NIL;
  if (!cp_write_env(lelt->env,f)) return NIL;
  if (!cp_write_bodylist(stream,lelt->body,f)) return NIL;
  if (!cp_write_sinfo(&lelt->sinfo,f)) return NIL;
  else  return T;
}

int cp_write_elt(MAILSTREAM *stream,MESSAGECACHE *elt, FILE *f)
{
  /* If ELEMENT is NIL, then write NILELT=<msgno> */
  if (cp_nil_elt(elt)) {
    if (fprintf(f,NILELT,elt->msgno) < 0) return NIL;
    else return T;
  }
  /* OK, we need to output the elt. First the header: */
  if (fprintf(f,ELT,elt->msgno) < 0) return NIL;
  if (!cp_write_ulong(f,0L)) return NIL; /* sync_msgno */
  if (!cp_write_ulong(f,elt->uid)) return NIL;
  if (!cp_write_int(f,elt->hours)) return NIL;
  if (!cp_write_int(f,elt->minutes)) return NIL;
  if (!cp_write_int(f,elt->seconds)) return NIL;
  if (!cp_write_int(f,elt->zoccident)) return NIL;
  if (!cp_write_int(f,elt->zhours)) return NIL;
  if (!cp_write_int(f,elt->zminutes)) return NIL;
  if (!cp_write_int(f,elt->seen)) return NIL;
  if (!cp_write_int(f,elt->deleted)) return NIL;
  if (!cp_write_int(f,elt->flagged)) return NIL;
  if (!cp_write_int(f,elt->answered)) return NIL;
  if (!cp_write_int(f,elt->draft)) return NIL;
  if (!cp_write_int(f,elt->valid)) return NIL;
  if (!cp_write_int(f,elt->recent)) return NIL;
  if (!cp_write_int(f,elt->searched)) return NIL;
  if (!cp_write_int(f,elt->sequence)) return NIL;
  if (!cp_write_int(f,elt->spare)) return NIL;
  if (!cp_write_int(f,elt->spare2)) return NIL;
  if (!cp_write_int(f,elt->spare3)) return NIL;
  /* Lock count not cached. Set to 1 on cache restore. */
  if (!cp_write_int(f,elt->day)) return NIL;
  if (!cp_write_int(f,elt->month)) return NIL;
  if (!cp_write_int(f,elt->year)) return NIL;
  if (!cp_write_ulong(f,elt->user_flags)) return NIL;
  if (!cp_write_ulong(f,elt->rfc822_size)) return NIL;
  if (!cp_write_data(stream,f,(unsigned char *)elt->data1,elt->data3)) return NIL;
  if (!cp_write_data(stream,f,(unsigned char *)elt->data2,elt->data4)) return NIL;
  if (!cp_write_data(stream,f,(unsigned char *)elt->filter1,elt->filter2))
    return NIL;
  else return T;
}

/*
 * Here we use the fact that the only Non-nil values
 * in a null elt are the msgno and (lockcount == 1) */
int cp_nil_elt(MESSAGECACHE *elt)
{
  int nbytes = sizeof(MESSAGECACHE) - sizeof(unsigned long);
  unsigned char *ucp = (unsigned char *)elt + sizeof(unsigned long);
  unsigned long total;
  for (total = 0L; nbytes > 0; nbytes--) total += *ucp++;
  if (total == 1) return T;
  else return NIL;
}

int cp_write_data(MAILSTREAM *stream,FILE *f,unsigned char *data,unsigned long size)
{
  if (fprintf(f,"%s%lu\n",ISDATA,size) < 0) return NIL;
  if (size > 0) {
    int i,j,k;
    int nblks = size/CPPAGESIZE;
    int ovflw = size % CPPAGESIZE;
    unsigned char *bp,*cp = data;
    wrt_buf.offset = 0;  
    /* write the data */
    for (i = 0; i < nblks; ++i) {
      bp = wrt_buf.buf;
      for (j = 0; j < CPPAGESIZE; ++j)
	*bp++ = *cp++;
      *bp = '\0';
      k = fwrite(wrt_buf.buf,1,CPPAGESIZE,f);
      if (k == 0 || k != CPPAGESIZE) return NIL;
      fflush(f);
      if ((i % 2) == 0)
	mm_io_callback(stream,0,CBDISKIO);
    }
    if (ovflw > 0) {
      bp = wrt_buf.buf;
      for (i = 0; i < ovflw; ++i) *bp++ = *cp++;
      k = fwrite(wrt_buf.buf,1,ovflw,f);
      if (k == 0 || k != ovflw) return NIL;
      fflush(f);
    }
  }
  return T;
}

int cp_write_int(FILE *f, int i)
{
  if (fprintf(f, "%s%d\n", ISINT,i) < 0) return NIL;
  else return T;
}

int cp_write_ushort(FILE *f, unsigned short i)
{
  if (fprintf(f, "%s%d\n", ISUSHORT,i) < 0) return NIL;
  else return T;
}


int cp_write_ulong(FILE *f, unsigned long i)
{
  if (fprintf(f, "%s%lu\n", ISULONG,i) < 0) return NIL;
  else return T;
}
/*
 * write S=N,<string>
 *         N  := length of <string>
 *   <string> := %s\n */
int cp_write_string(FILE *f, char *str)
{
  if (str) {
    int len = strlen(str) + 1;		/* include '\n' */
    if (fprintf(f,"%s%d,%s\n",ISSTR,len,str) < 0) return NIL;
  } else {
    if (fprintf(f,"%s%s",ISSTR,NULLSTRING) < 0) return NIL;
  }
  return T;
}

int cp_write_uflags(FILE *f, char **user_flags)
{
  /* first count the flag list length */
  int n, i;
  for (n = i = 0; i < NUSERFLAGS; ++i)
    if (user_flags[i]) ++n;
  if (fprintf(f, "L=%d\n", n) < 0) return NIL;
  if (n > 0) {
    for (i = 0; i < NUSERFLAGS && n > 0; ++i)
      if (user_flags[i]) {
	if (cp_write_string(f, user_flags[i]) < 0) return NIL;
	else
	  n -= 1;
      }
  }
  return T;
}

/* Write envelope:
 * if env != NIL, then write
 * ENV
 *  <each slot in order>
 * #ENVEND# 
 * if env == NIL, then write
 * NILENV
 */
int cp_write_env(ENVELOPE *env, FILE *f)
{
  if (env == NIL) {
    if (fprintf(f,NILENV) < 0) return NIL;
    else return T;
  } else if (fprintf(f,"%s",ISENV) < 0) return NIL;
  if (!cp_write_string(f,env->remail)) return NIL;
  if (!cp_write_address(f,env->return_path)) return NIL;
  if (!cp_write_string(f,env->date)) return NIL;
  if (!cp_write_address(f,env->from)) return NIL;
  if (!cp_write_address(f,env->sender)) return NIL;
  if (!cp_write_address(f,env->reply_to)) return NIL;
  if (!cp_write_string(f,env->subject)) return NIL;
  if (!cp_write_address(f,env->to)) return NIL;
  if (!cp_write_address(f,env->cc)) return NIL;
  if (!cp_write_address(f,env->bcc)) return NIL;
  if (!cp_write_string(f,env->in_reply_to)) return NIL;
  if (!cp_write_string(f,env->message_id)) return NIL;
  if (!cp_write_string(f,env->newsgroups)) return NIL;
  if (fprintf(f,ENVEND) < 0) return NIL;
  else return T;
}
/* write address:
 * ADDR=<n-members> | 0
 *  <each member in slot order>
 */
int cp_write_address(FILE *f,ADDRESS *a)
{
  ADDRESS *ap = a;
  int n;
  /* compute and write the header */
  for (n = 0; ap; ap = ap->next, ++n);
  if (fprintf(f,ADDR,n) < 0) return NIL;
  else if (n == 0) return T;
  /* OK, write each address slot */
  while (a) {
    if (!cp_write_string(f,a->personal)) return NIL;
    if (!cp_write_string(f,a->adl)) return NIL;
    if (!cp_write_string(f,a->mailbox)) return NIL;
    if (!cp_write_string(f,a->host)) return NIL;
    if (!cp_write_string(f,a->error)) return NIL;
    a = a->next;
  }
  return T;
}
/*
 * write recursive body, EG:
 * BODYDESC
 * BODY
 * PART[1] 
 * BODY
 * PART[2]
 * BODY
 * PART[2.1]  if BODY.TYPE is MULTIPART
 * BODY
 * PART[2.2]
 * BODY
 *    :
 * PART[2.j]
 * PART[3]    
 * #BODYDESCEND#
 * EXAMPLE:
 * BOD
 */ 
int cp_write_bodylist(MAILSTREAM *stream,BODY *b, FILE *f)
{
  char part_buf[MAILTMPLEN];
  /* Check for NIL body */
  if (b == NIL) {
    if (fprintf(f,NILBODY) < 0) return NIL;
    else return T;
  }
  /* Output initial BODY description */
  if (fprintf(f, BODYDESC) < 0) return NIL;
  if (!cp_write_body(stream,b,f)) return NIL;
  else
    if (b->type == TYPEMULTIPART) {
      part_buf[0] = NIL;
      return cp_write_parts(stream,f,b->contents.part,part_buf);
    } else return T;
}
/*
 * Body description + data is output */
int cp_write_body (MAILSTREAM *stream,BODY *b, FILE *f)
{
  unsigned long n;
  MESSAGE *msg;
  if (fprintf(f,"%s",BODYMARK) < 0) return NIL;
  /* basic body slots */
  if (!cp_write_ushort(f,b->type)) return NIL;
  if (!cp_write_ushort(f,b->encoding)) return NIL;
  if (!cp_write_string(f,b->subtype)) return NIL;
  if (!cp_write_parlist(f,b->parameter)) return NIL;
  if (!cp_write_string(f,b->id)) return NIL;      
  if (!cp_write_string(f,b->description)) return NIL;
  /* Write any data */
  switch (b->type) {
  case TYPEMULTIPART: return T;
  case TYPEMESSAGE:
    msg = &b->contents.msg;
    if (!cp_write_env(msg->env,f)) return NIL;
    if (!cp_write_data(stream,f,(unsigned char *)msg->hdr,
		       msg->hdrsize)) return NIL;
    if (!cp_write_ulong(f,msg->hdrsize)) return NIL;
    if (msg->text == NIL) n = 0; else n = strlen(msg->text);
    if (!cp_write_data(stream,f,(unsigned char *)msg->text,n)) return NIL;
    if (!cp_write_ulong(f,msg->offset)) return NIL;
    if (!cp_write_bodylist(stream,msg->body,f)) return NIL;
    /* Fall into default to write sizes only */
  default:
    /* Write the sizes in lines */
    if (!cp_write_ulong(f,b->size.lines)) return NIL;
    if (!cp_write_ulong(f,b->size.bytes)) return NIL;
    if (!cp_write_ulong(f,b->size.ibytes)) return NIL;
    /* did typemessage above */
    if (b->type == TYPEMESSAGE) break;
    /* Write the data which may not be there */
    if (b->contents.text == NIL) n = 0;
    else n = b->size.bytes;
    if (!cp_write_data(stream,f,b->contents.text,n)) return NIL;
    else break;
  }
  return T;
}

int cp_write_parts(MAILSTREAM *stream,FILE *f,PART *p,char *seq)
{
  int i = 1;
  char *base;

  if (*seq != NIL) strcat(seq,".");	/* recuring for Multipart */
  base = &seq[strlen(seq)];		/* base for adjoining */
  while (p) {
    /* make part sequence number */
    sprintf(base,"%d",i++);
    if (fprintf(f,"%s%s\n",ISPARTD,seq) < 0) return NIL;
    if (!cp_write_body(stream,&p->body,f) < 0) return NIL;
    if (!cp_write_ulong(f,p->offset) < 0) return NIL;
    switch (p->body.type) {
    case TYPEMULTIPART:
      if (cp_write_parts(stream,f,p->body.contents.part,seq) < 0) 
	return NIL;
      break;
    default:
      break;
    }
    p = p->next;
  }
  if (fprintf(f,NILPART) < 0) return NIL;
  else return T;
}

int cp_write_parlist(FILE *f,PARAMETER *p)
{
  PARAMETER *pp = p;
  int n;
  for (n = 0; pp; pp = pp->next,++n);
  if (fprintf(f,"%s%d\n",ISPLIST,n) < 0) return NIL;
  else if (n == 0) return T;
  /* OK, write the parameters */
  for (pp = p; pp; pp = pp->next) {
    if (!cp_write_string(f,pp->attribute)) return NIL;
    if (!cp_write_string(f,pp->value)) return NIL;
  }
  return T;
}

int cp_write_sinfo(SHORTINFO *sinfo,FILE *f)
{
  if (!cp_write_string(f,sinfo->date)) return NIL;
  if (!cp_write_string(f,sinfo->personal)) return NIL;
  if (!cp_write_string(f,sinfo->mailbox)) return NIL;
  if (!cp_write_string(f,sinfo->host)) return NIL;
  if (!cp_write_string(f,sinfo->subject)) return NIL;
  if (!cp_write_string(f,sinfo->bodytype)) return NIL;
  if (!cp_write_string(f,sinfo->size)) return NIL;
  if (!cp_write_string(f,sinfo->from)) return NIL;
  else return T;  
}

/* NOW, the symmetric stuff for restoring */
MAILSTREAM *cp_restore_stream(FILE *f, int *errno)
{
  MAILSTREAM *stream;
  char readbuf[READBUFLEN];
  int g;

  /* Get stream descriptor */
  if (!fgets(readbuf, READBUFLEN, f)) {
    if (errno) *errno = CACHECORRUPTED;
    return NIL;
  }
  if (strcmp(STREAMDESC,readbuf) != 0) {
    if (errno) *errno = CACHECORRUPTED;
    return NIL;
  }
  /* Allocate our stream */
  stream = (MAILSTREAM *) fs_get (sizeof (MAILSTREAM));
			/* initialize stream */
  memset ((void *) stream,0,sizeof (MAILSTREAM));
  /* Defaults for dtb, and local */
  stream->dtb = NIL;
  stream->local = NIL;
  if (errno) *errno = CACHECORRUPTED;
  /* Get stream->nmsgs */
  if (!cp_read_ulong(f, &stream->nmsgs)) return NIL;
  if (!cp_read_string(f,&stream->mailbox)) return NIL;
  if (!cp_read_short(f,&stream->use)) return NIL;	/* use */
  if (!cp_read_short(f,&stream->sequence)) return NIL;/* sequence */
  if (!cp_read_int(f,&g)) return NIL;/* lock */
  else stream->lock = (g ? T : NIL);
  if (!cp_read_int(f,&g)) return NIL;
  else stream->debug = (g ? T : NIL);
  if (!cp_read_int(f,&g)) return NIL;
  else stream->silent = (g ? T : NIL);
  if (!cp_read_int(f,&g)) return NIL; 
  else stream->anonymous = (g ? T : NIL);
  if (!cp_read_int(f,&g)) return NIL;
  else stream->scache = (g ? T : NIL);
  if (!cp_read_int(f,&g)) return NIL;
  else stream->halfopen = (g ? T : NIL);
  if (!cp_read_int(f,&g)) return NIL;
  else stream->perm_seen = (g ? T : NIL);
  if (!cp_read_int(f,&g)) return NIL;
  else stream->perm_deleted = (g ? T : NIL);
  if (!cp_read_int(f,&g)) return NIL;
  else stream->perm_flagged = (g ? T : NIL);
  if (!cp_read_int(f,&g)) return NIL;
  else stream->perm_answered = (g ? T : NIL);
  if (!cp_read_int(f,&g)) return NIL;
  else stream->perm_draft = (g ? T : NIL);
  if (!cp_read_int(f,&g)) return NIL;
  else stream->kwd_create = (g ? T : NIL);
  /* record_resume not cached */
  if (!cp_read_ulong(f,&stream->perm_user_flags)) return NIL;
  if (!cp_read_ulong(f,&stream->gensym)) return NIL;
  if (!cp_read_ulong(f,&stream->nmsgs)) return NIL;
  if (!cp_read_ulong(f,&stream->recent)) return NIL;
  if (!cp_read_ulong(f,&stream->cached_nmsgs)) return NIL;
  if (!cp_read_ulong(f,&stream->uid_validity)) return NIL;
  if (!cp_read_ulong(f,&stream->uid_last)) return NIL;
  if (!cp_read_string(f,&stream->flagstring)) return NIL;
  if (!cp_read_uflags(f,stream->user_flags)) return NIL;
  if (!cp_read_ulong(f,&stream->cachesize)) return NIL; /* cachesize */
  /* We do not write a values for:
   *  (1) cache union 
   *  (2) *env, *body, *text [short cache stuff]
   *  (3) userdata, sinfo, checkpointed
   * These are set at the cache restore time */
  if (!cp_read_int(f,&stream->record)) return NIL;  
  if (!cp_read_int(f,&stream->imap4_recording_session)) return NIL;  
  if (!cp_read_ulong(f,&stream->checksum)) return NIL;
  if (!cp_read_ulong(f,&stream->mbox_size)) return NIL;
  if (!cp_read_string(f,&stream->driver_name)) return NIL;
  if (!cp_read_int(f,&stream->sunvset)) return NIL;  
  if (!cp_read_int(f,&stream->keep_mime)) return NIL;  
  /* We do not cache values for
   *   validchecksum
   *   checksum_type
   *   send_checksum
   *   mail_send_urgent
   *   imap_send_urgent
   *   input_flushed
   *   min_dirty_msgno
   *   have_all_sinfo */
  if (!cp_read_int(f,&stream->secondary_mbox)) return NIL;  
  if (!cp_read_string(f,&stream->sync_uidseq)) return NIL;
  if (!fgets(readbuf,READBUFLEN,f)) {
    if (errno) *errno = CACHECORRUPTED;
    return NIL;
  }
  if (strcmp(STREAMEND,readbuf) != 0) {
    if (errno) *errno = CACHECORRUPTED;
    return NIL;
  }
  return stream;
}

/*
 * Read I=<int> */
int cp_read_int(FILE *f, int *i)
{
  char buf[READBUFLEN];
  if (!fgets(buf,READBUFLEN-1,f)) return NIL;
  if (strncmp(ISINT,buf,DDLEN) != 0) return NIL;
  *i = atoi(&buf[DDLEN]);
  return T;
}

int cp_read_short(FILE *f, unsigned short *i)
{
  char buf[READBUFLEN];
  if (!fgets(buf,READBUFLEN-1,f)) return NIL;
  if (strncmp(ISINT,buf,DDLEN) != 0) return NIL;
  *i = (short)atoi(&buf[DDLEN]);
  return T;
}

int cp_read_ushort(FILE *f, unsigned short *i)
{
  char buf[READBUFLEN];
  if (!fgets(buf,READBUFLEN-1,f)) return NIL;
  if (strncmp(ISUSHORT,buf,USLEN) != 0) return NIL;
  *i = (short)atoi(&buf[USLEN]);
  return T;
}


/*
 * Read U=<ulong> */
int cp_read_ulong(FILE *f, unsigned long *i)
{
  char buf[READBUFLEN];
  if (!fgets(buf,READBUFLEN-1,f)) return NIL;
  if (strncmp(ISULONG,buf,DDLEN) != 0) return NIL;
  *i = strtoul(&buf[DDLEN],NIL,10);
  return T;
}

/*
 * Read S=N,<string> OR
 *      S=#NULLSTRING#\n */
int cp_read_string(FILE *f, char **str)
{
  char rbuf[READBUFLEN];
  char *cp = rbuf;
  int v;
  /* Read S= */
  while ((v = getc(f)) != EOF)
    if (v != 'S' && v != '=') break;
  if (v == EOF) return NIL;
  else {
    /* Either #NULLSTRING#\n OR N, */
    *cp = v;
    if (v == '#') {
      if(!fgets(&rbuf[1],READBUFLEN,f)) return NIL;
      if (strcmp(rbuf,NULLSTRING) != 0) return NIL; 
      else *str = NIL;
    } else if (!cp_copy_string(f,cp,str)) return NIL;
  return T;  
  }
}

/*
 * read S=N,<string> from an input buffer,
 *        N == nchars, *buf == N  */
int cp_copy_string(FILE *f, char *buf, char **str)
{
  int len;
  char *cp = buf;			/* pointer to length */
  char *op;
  int v;
  /* calculate the string length */
  ++cp;					/* get next char */
  while ((v = getc(f)) != EOF && v != ',') *cp++ = v;
  *cp = '\0';
  len = atoi(buf);
  op = *str = (char *)fs_get(len+1);
  /* OK, read the string. */
  while ((len-- > 0) && (v = getc(f)) != EOF) *op++ = v;
  --op;					/* decrement to terminating '\n' */
  *op = '\0';
  return T;
}

/*
 * Read the user flags string list: L=<nstrings> */
int cp_read_uflags(FILE *f, char **uflags)
{
  int n;
  char buf[READBUFLEN];

  if (!fgets(buf,READBUFLEN-1,f)) return NIL;
  if (strncmp(ISLIST,buf,DDLEN) != 0) return NIL;
  n = atoi(&buf[DDLEN]);
  /* See if list is empty */
  if (n == 0) {
    *uflags = NIL;
    return T;
  } else if (n > NUSERFLAGS) 
      return NIL;
  else {
    int i;
    for (i = 0; i < NUSERFLAGS; ++i,++uflags) *uflags = NIL;
    for (i = 0; i < n; ++i,++uflags)
      if (!cp_read_string(f, uflags)) return NIL;
    return T;
  }
}
/*
 * Read "D=<n>\n<n-data-bytes". 
 * If n == 0, then set *size = 0L, *ibuf = NIL. */
int cp_read_data(FILE *f,char **ibuf,unsigned long *size)
{
  char rbuf[READBUFLEN];
  char c,*cp;
  unsigned long ct;
  /* Get the "D=<n>\n" line */
  if (!fgets(rbuf,READBUFLEN-1,f)) return NIL;
  if (strncmp(ISDATA,rbuf,DDLEN) != 0) return NIL;
  *size = strtoul(&rbuf[DDLEN],NIL,10);
  /* 0 bytes possible */
  if (*size == 0) {
    *ibuf = NIL;
    return T;
  }
  /* allocate for the data, and restore it */
  cp = *ibuf = (char *)fs_get(*size + 1);
  ct = *size;
  while ((c = fgetc(f)) != EOF) {
    *cp++ = c;
    ct -= 1;
    if (ct == 0) break;
  }
  /* Make sure the data was all there */
  if (ct != 0) return NIL;
  *cp = '\0';
  return T;
}

/*
 * allocated and read a body */
int cp_read_body (FILE *f,BODY **b)
{
  char rbuf[READBUFLEN];
  unsigned long size;
  MESSAGE *msg;
  if (!fgets(rbuf,READBUFLEN,f)) return NIL;
  if (strcmp(BODYMARK,rbuf) != 0) return NIL;
  /* basic body slots */
  *b = mail_newbody();
  if (!cp_read_ushort(f,&(*b)->type)) return NIL;
  if (!cp_read_ushort(f,&(*b)->encoding)) return NIL;
  if (!cp_read_string(f,&(*b)->subtype)) return NIL;
  if (!cp_read_parlist(f,&(*b)->parameter)) return NIL;
  if (!cp_read_string(f,&(*b)->id)) return NIL;      
  if (!cp_read_string(f,&(*b)->description)) return NIL;
  /* Read any data */
  switch ((*b)->type) {
  case TYPEMULTIPART: return T;
  case TYPEMESSAGE: 
    msg = &(*b)->contents.msg;
    if (!cp_read_env(f,&msg->env)) return NIL;
    if (!cp_read_data(f,&msg->hdr,&size)) return NIL;
    if (!cp_read_ulong(f,&msg->hdrsize)) return NIL;
    if (!cp_read_data(f,&msg->text,&size)) return NIL;
    if (!cp_read_ulong(f,&msg->offset)) return NIL;
    if (!cp_read_bodylist(f,&msg->body)) return NIL;
    /* Fall through to read sizes only */
  default:
    /* Write the sizes in lines */
    if (!cp_read_ulong(f,&(*b)->size.lines)) return NIL;
    if (!cp_read_ulong(f,&(*b)->size.bytes)) return NIL;
    if (!cp_read_ulong(f,&(*b)->size.ibytes)) return NIL;
    if ((*b)->type == TYPEMESSAGE) break;
    /* Read data */
    if (!cp_read_data(f,(char **)&(*b)->contents.text,
		      &size)) return NIL;
    else break;
  }
  return T;
}
/*
 * Same as above, but we don't need to allocate
 * the body */
int cp_read_body_ptr (FILE *f,BODY *b)
{
  char rbuf[READBUFLEN];
  unsigned long size;
  MESSAGE *msg;
  if (!fgets(rbuf,READBUFLEN,f)) return NIL;
  if (strcmp(BODYMARK,rbuf) != 0) return NIL;
  /* basic body slots */
  if (!cp_read_ushort(f,&b->type)) return NIL;
  if (!cp_read_ushort(f,&b->encoding)) return NIL;
  if (!cp_read_string(f,&b->subtype)) return NIL;
  if (!cp_read_parlist(f,&b->parameter)) return NIL;
  if (!cp_read_string(f,&b->id)) return NIL;      
  if (!cp_read_string(f,&b->description)) return NIL;
  /* Read any data */
  switch (b->type) {
  case TYPEMULTIPART: return T;
  case TYPEMESSAGE: 
    msg = &b->contents.msg;
    if (!cp_read_env(f,&msg->env)) return NIL;
    if (!cp_read_data(f,&msg->hdr,&size)) return NIL;
    if (!cp_read_ulong(f,&msg->hdrsize)) return NIL;
    if (!cp_read_data(f,&msg->text,&size)) return NIL;
    if (!cp_read_ulong(f,&msg->offset)) return NIL;
    if (!cp_read_bodylist(f,&msg->body)) return NIL;
    /* Fall through to read sizes only */
  default:
    /* Write the sizes in lines */
    if (!cp_read_ulong(f,&b->size.lines)) return NIL;
    if (!cp_read_ulong(f,&b->size.bytes)) return NIL;
    if (!cp_read_ulong(f,&b->size.ibytes)) return NIL;
    if (b->type == TYPEMESSAGE) break;
    /* Read data */
    if (!cp_read_data(f,(char **)&b->contents.text,
		      &size)) return NIL;
    else break;
  }
  return T;
}

/* Read parameter list:
 * PLIST=<n-parameters> */
int cp_read_parlist(FILE *f, PARAMETER **p)
{
  int n;
  PARAMETER **pp = p;
  char rbuf[READBUFLEN];
  if (!fgets(rbuf, READBUFLEN, f)) return NIL;
  else if (strncmp(ISPLIST,rbuf,PLLEN) != 0) return NIL;
  else n = atoi(&rbuf[PLLEN]);
  if (n == 0) return T;			/* no parameters in list */
  while (n--) {
    *pp = mail_newbody_parameter();
    if (!cp_read_string(f,&(*pp)->attribute)) return NIL;
    if (!cp_read_string(f,&(*pp)->value)) return NIL;
    pp = &(*pp)->next;
  }    
  return T;
}

/*
 * Not done yet */
int cp_read_parts(FILE *f,PART **p)
{
  char rbuf[READBUFLEN];
  int quit = NIL;
  while (!quit) {
    /* Read PART=<seq> OR NILPART */
    if (!fgets(rbuf, READBUFLEN, f)) return NIL;
    switch (rbuf[0]) {
    case 'N':
      if (rbuf[1] == 'I' && rbuf[2] == 'L' && rbuf[3] == 'P' &&
	  rbuf[4] == 'A' && rbuf[5] == 'R' && rbuf[6] == 'T' &&
	  rbuf[7] == '\n') {
	quit = T;	/* found END */
	continue;
      } else return NIL;
    case 'P':
      if (rbuf[1] == 'A' && rbuf[2] == 'R' && rbuf[3] == 'T' &&
	  rbuf[4] == '=') break;	/* another part */
      else return NIL;
    default:
      return NIL;
    }  
    *p = mail_newbody_part();
    if (!cp_read_body_ptr(f,&(*p)->body)) return NIL;
    if (!cp_read_ulong(f,&(*p)->offset)) return NIL;
    if ((*p)->body.type == TYPEMULTIPART)
      if (!cp_read_parts(f,&(*p)->body.contents.part))
	return NIL;
    p = &(*p)->next;
  }
  return T;
}
