/*
 * This code translates sun attachements to MIME format - C'est pas du tout rigolo.
 * C'est chouette!!
 *
 *     Auteur        Bill Yeager - 29 aout 93
 *
 *     Copyright (c) Sun Micro Systems, Inc. All rights reserved 
 * 
 *  20-Mai-96   Fixed V3 translation with NO X-Sun-Content-Type: 
 *  39-july-96  Fixed V3 preamble problem. Needed to skip it.
 *
 *  [11-08-96 clin] stream passed in mm_log calls.
 *  [01-02-97 clin] a NIL stream passed in mm_notify calls.
 */
#include "UnixDefs.h"
#include <sys/utsname.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include "mail.h"
#include "os_sv5.h"
#include "solaris.h"
#include "rfc822.h"
#include "misc.h"
#include "sun_mime.h"
/*
 * Edit November 23, 1994  Fix "x-compress" bogus Mailtool
 *                         conversion problems.
 */

char *sun_mime_version= ".m7)";
typedef struct _charset_trans0_ {
  char *mime_cs, *xsun_cs;
  int  mlen, sunlen;
} CSTRANS0;

/* Known MIME charsets - Only 1 X-Sun-Att. equivalent 
 * Added KNOWN iso-2022-jp. WJY/LS 8-feb-95 */
static CSTRANS0 chrset_trans0[] = {
  {"us-ascii", "ascii", 8, 5},
  {"us-ascii", "us_ascii", 8, 8},
  {"iso-2022-jp", "iso-2022-jp", 11, 11},
  {"iso-8859-1", "X-iso-8859-1", 10, 12}, /* BUG FIX: John Cooper 26-jan-96 */
  {"iso-8859-2", "X-iso-8859-2", 10, 12},
  {"iso-8859-3", "X-iso-8859-3", 10, 12},
  {"iso-8859-4", "X-iso-8859-4", 10, 12},
  {"iso-8859-5", "X-iso-8859-5", 10, 12},
  {"iso-8859-6", "X-iso-8859-6", 10, 12},
  {"iso-8859-7", "X-iso-8859-7", 10, 12},
  {"iso-8859-8", "X-iso-8859-8", 10, 12},
  {"iso-8859-9", "X-iso-8859-9", 10, 12},
  {NIL, NIL},
};

void do_text_plain_translation(FILECACHE **ofc, long *buflen, int keep_mime);
void do_multipart_translation(FILECACHE **ofc, long *buflen, int keep_mime);
void free_the_parts(ATTPART *part);

/*
 * (1) We are passed a FILECACHE *fc for a message which has been read
 * by the solaris.c driver.
 * (2) We have also been passed the length of the message,*msglen. 
 *
 * We scan looking for "Content-Type: <field>" and if appropriate,
 * translate the message to MIME. This will change the message length
 * which is then modified.
 *
 * If we change the message body length, then we need to also update the
 * "Content-Length: n" field. It may be that we need only change the
 * header itself in the case where "Content-Type: text\n" is found. In
 * this simple case we substitute the following:
 * "Content-Type: text/plain; charset=us-ascii\nMIME-Version: 1.0\n"
 */
int
sun_att_to_mime(FILECACHE **ofc, long *msglen, int keep_mime)
{
  char *lp, c;
  int translate= NIL;		/* true if we can translate  */
  int content_len= NIL;		/* true if found content-length: */
  int found_body= NIL;		/* true if found the body */
  char *con_length= NIL, *con_type= NIL;
  int textplain= NIL;
  char tmp[CTYPEBUF];
  mime_translation action= NOMIMEACTION;
  FILECACHE *fc= *ofc;		/* THE pointer */
  /* Start at the rfc822 header */
  lp = fc->internal;

  /* We scan each line looking for either:
   * Mime-Version:      Nothing to do, already MIME
   * Context-Type:      Text or X-Sun-Attachment. Must translate
   * Content-Length:    Need offset in the buffer for possible modification
   */

  while (c = *lp) {
    switch (c) {
    case 'C': case 'c':
      if ((lp[1] == 'o' || lp[1] == 'O') && 
	(lp[2] == 'n' || lp[2] == 'N') && 
	(lp[3] == 't' || lp[3] == 'T') && 
	(lp[4] == 'e' || lp[4] == 'E') && 
	(lp[5] == 'n' || lp[5] == 'N') && 
	(lp[6] == 't' || lp[6] == 'T') &&
	 lp[7] == '-') {
	switch (lp[8]) {	/* look for "type" or "length" */
	case 'L': case 'l':
	  if ((lp[9] == 'e' || lp[9] == 'E') &&
	      (lp[10] == 'n' || lp[10] == 'N') &&
	      (lp[11] == 'g' || lp[11] == 'G') &&
	      (lp[12] == 't' || lp[12] == 'T') &&
	      (lp[13] == 'h' || lp[13] == 'H') &&
	      (lp[14] == ':')) {
	    char *tptr= &lp[15];
	    int len= 1;		/* the terminating \n */

	    while (*tptr++ != '\n')
	      len += 1;
	    fc->slem.content_length = atol(&lp[15]); /* the length */
	    fc->slem.con_lenhdrlen = len + CONTENTLENLEN;
	    fc->slem.con_lenhdr = lp;
	    content_len = T;
	    break;
	  }
	case 'T': case 't':
	  if ((lp[9] == 'y' || lp[9] == 'Y') &&
	      (lp[10] == 'p' || lp[10] == 'P') &&
	      (lp[11] == 'e' || lp[11] == 'E') &&
	       lp[12] == ':') {
	    char *ctype= &lp[13];
	    char *tptr= tmp;
	    int len= 0, 
	      xtra= 1;		/* includes terminating "\n" */

	    while (*ctype == ' ') {
	      ++ctype;
	      ++xtra;		/* count these spaces */
	    }
	    /* Look for "TEXT" or "X-Sun-attachment" */
	    while ((c = *ctype++) && c != '\n') { /* copy rest of line  */
	      *tptr++ = toupper(c);
	      len += 1;
	      if (len == CTYPEBUF - 1) { /* Rather long line */
		while ((c = *lp++) && c != '\n');/* FLUSH it */
		break;
	      }
	    }
	    *tptr = '\0';
	    if (tmp[0] == 'T' && tmp[1] == 'E' && 
		tmp[2] == 'X' && tmp[3] == 'T' &&
		c == '\n') {
	      action = DOPLAINTEXT;
	      fc->slem.con_typehdr = lp;
	      fc->slem.con_typehdrlen = xtra + CONTYPELEN + TEXTLEN;
	      textplain = T;
	    } else {
	      if (len == XSUNATTLEN) { /* check saves a little time */
		int i;
		char *xsa= "X-SUN-ATTACHMENT";
		int found= T;

		for (i = 0, tptr = tmp; i < XSUNATTLEN; ++i) {
		  if (*xsa++ != *tptr++) {
		    found = NIL;
		    break;
		  }
		}
		if (found) {
		  action = DOSUNATTACHMENT;
		  translate = T;
		  fc->slem.con_typehdr = lp;
		  fc->slem.con_typehdrlen = xtra + CONTYPELEN + XSUNATTLEN;
		}
	      } 
	    }			/* end - else */
	  }			/* end - if Type: */
	  break;
	default:
	  break;
	}			
      }
      break;			/* from case 'C': case 'c': */
    case 'M':			/* look for Mime-version: */
      if ((lp[1] == 'i' || lp[1] == 'I') && 
	  (lp[2] == 'm' || lp[2] == 'M') && 
	  (lp[3] == 'e' || lp[3] == 'E') && 
	  (lp[4] == '-') &&
	  (lp[5] == 'v' || lp[5] == 'V') && 
	  (lp[6] == 'e' || lp[6] == 'E') &&
	  (lp[7] == 'r' || lp[7] == 'R')  &&
	  (lp[8] == 's' || lp[8] == 'S')  &&
	  (lp[9] == 'i' || lp[9] == 'I')  &&
	  (lp[10] == 'o' || lp[10] == 'O')  &&
	  (lp[11] == 'n' || lp[11] == 'N')  &&
	  (lp[12] == ':'))
	return NIL;		/* Nothing to do - deja MIME! */
      else
	break;
    case 'X':			/* look for Mime-version: */
      if ((lp[1] == '-') && 
	  (lp[2] == 'S' || lp[2] == 's') && 
	  (lp[3] == 'U' || lp[3] == 'u') && 
	  (lp[4] == 'N' || lp[4] == 'n') &&
	  (lp[5] == '-') && 
	  (lp[6] == 'T' || lp[6] == 't') &&
	  (lp[7] == 'E' || lp[7] == 'e')  &&
	  (lp[8] == 'X' || lp[8] == 'x')  &&
	  (lp[9] == 'T' || lp[9] == 't')  &&
	  (lp[10] == '-') &&
	  (lp[11] == 'T' || lp[11] == 't')  &&
	  (lp[12] == 'Y' || lp[12] == 'y')  &&
	  (lp[13] == 'P' || lp[13] == 'p')  &&
	  (lp[14] == 'E' || lp[14] == 'e')  &&
	  (lp[15] == ':')) {
	char *tptr= &lp[16];
	int len= 1;		/* the terminating \n */
	int start_of_cs= 0;
	char *charset;		/* Sun-Att charset */
	int cslen;
	CSTRANS0 *cst;

	while (*tptr++ != '\n') {
	  if (*tptr != ' ' && start_of_cs == 0)
	    start_of_cs = len;
	  len += 1;
	}
	cslen = len - start_of_cs - 1;
	fc->slem.txthdr_len = len + STEXTTYPELEN;
	fc->slem.txthdr = lp;	/* starting position of this truc */
	charset = &lp[16+start_of_cs]; /* the charset */
	/* Translate it to MIME charset */
	for (cst = chrset_trans0; cst->xsun_cs; cst++) {
	  if (cslen == cst->sunlen) 
	    if (strncasecmp(charset, cst->xsun_cs, cslen) == 0) 
	      break;		/* Voila! */
	}
	if (cst->mime_cs) {	/* On l'a. One has it. */
	  fc->slem.charset = cpystr(cst->mime_cs);
	  fc->slem.charsetlen = cst->mlen;
	} else {		/* use X-%s */
	  if ((charset[0] == 'X' || charset[0] == 'x') && charset[1] == '-') {
	    fc->slem.charset = cpystr(charset);
	    fc->slem.charsetlen = cslen;
	  } else {
	    fc->slem.charset = fs_get(cslen+4);
	    fc->slem.charset[0] = 'X';
	    fc->slem.charset[1] = '-';
	    strncpy(&fc->slem.charset[2], charset, cslen); /* "X-%s" */
	    fc->slem.charset[2+cslen] = NIL;
	    fc->slem.charsetlen = cslen + 2;
	  }
	}
	if (textplain)
	  content_len = translate = T; /* content len doesn't change */
	break;
      } else
	break;
    case '\n':			/* Found body */
      found_body = T;
      break;
    default: break;
    }
    if ((translate && content_len) || found_body) 
      break; /* from the loop */
    else {
      lp = strchr(lp, '\n');
      if (!lp++)		/* skip '\n' */
	break;			/* end of message - should not happen */
    }
  }
  /* OK -- act on the action we've found */
  switch (action) {
  case DOPLAINTEXT:
    /* Here, we need only build the header replacement */
    fc->slem.con_lenhdr = NIL;
    fc->slem.parts = NIL;
    do_text_plain_translation(ofc, msglen, keep_mime);
    return T;
  case DOSUNATTACHMENT:
    /* Parse the sun attachement */
    if (!parse_sun_attachment(fc))
      return NIL;
    else {
      /* Translate into MIME */
      do_multipart_translation(ofc, msglen, keep_mime);
      (*ofc)->text_trans = NIL;
    }
    return T;
  default:
    break;
  }
  return NIL;
}

/*
 * The simplest case - 
 *    1. Recalculate the buffer size. It should increase by a few bytes.
 *    2. resize fc to allow for these extra bytes.
 *    3. replace that header with the specified header, then copy the
 *    remainder of the message to the new buffer.
 *
 * We need to chanage:
 *
 *    fc->headersize.
 *
 */

static long tb_grandeur= 0L;

/*
 * NOTE: here we do the text/plain translation. The file cache pointer
 * will be replaced by a new one, and the msglen can change. */
void 
do_text_plain_translation(FILECACHE **ofc, long *msglen, int keep_mime)
{
  long resize_bytes;
  char *tbptr;
  size_t nbytes, n_data;
  long mime_bytes;
  long mh1len;
  char *cp;
  FILECACHE *fc= *ofc, *nfc;
  size_t original_len;
  size_t required_len;
  char *body_pos;
  if (fc->slem.txthdr) {
    mh1len = strlen(MTPHDR1);
    mime_bytes = mh1len + fc->slem.charsetlen + 1; /* 1 for '\n' */
    resize_bytes = mime_bytes - fc->slem.con_typehdrlen;
    resize_bytes -= fc->slem.txthdr_len;
  } else  {
    mime_bytes = strlen(MTPHDR);
    /* compute additional message data */
    resize_bytes = mime_bytes - fc->slem.con_typehdrlen;
  }
  original_len = *msglen + 1;
  required_len = (((original_len + resize_bytes + 1) >> 1) << 1) + 16;
  /* do the translation */
  tb_grandeur = required_len;
  nfc = (FILECACHE *)fs_get(sizeof(FILECACHE));/* new FILECACHE */
  memcpy(nfc,fc,sizeof(FILECACHE));
				/* need bigger translation buffer */
  if (keep_mime) {		/* Use mmap for tmp data */
    solaris_get(tb_grandeur,INTERNAL_DATA(nfc));
    tbptr = INTERNAL_BASE(nfc);
  } else 			/* goes on our heap */
    tbptr = fs_get(tb_grandeur);/* for the  msg data traduite. */
  nfc->internal = tbptr;
  /* We copy the data in blocks:
   * If the txthdr is NOT present:
   *   FILECACHE+MSG-UPTO-CONTENT_TYPE
   *   NEW-MIME-HEADERS 
   *   MSG-AFTER-CONTENT_TYPE+data to the END
   * else
   *   FILECACHE+MSG upto first of the two headers.
   *   DATA between the two headers.
   *   NEW-MIME-HEADERS
   *   MSG-AFTER 2nd of two headers to the END.
   *
   * The following could be prettier, but who has the time. */
  /* Copy the fc upto the Content-type line */
  if (fc->slem.txthdr == NIL) {	/* No "X-Sun-Text-Type: <charset>" merde.  */

    nbytes = fc->slem.con_typehdr - (char *)fc->internal;
    memcpy(tbptr, fc->internal, nbytes);
    tbptr += nbytes;
    /* Copy in the mime-version and content-type headers */
    memcpy(tbptr, MTPHDR,  mime_bytes);
    tbptr += mime_bytes;
    /* Now copy the rest of the the fc into our temp buffer */
    cp = fc->slem.con_typehdr + fc->slem.con_typehdrlen; /* adjust source */
    n_data = cp - fc->internal;	/* internal hdr to cur. position. */
    nbytes = *msglen - n_data;	/* end - curpos: message data remaining */
    memcpy(tbptr, cp, nbytes);

  } else {			/* Dans la merde de traduction encore! */
    unsigned long txthp, ctypep;

    txthp = (unsigned long)fc->slem.txthdr;
    ctypep = (unsigned long)fc->slem.con_typehdr;
    if (txthp < ctypep) {
      nbytes = fc->slem.txthdr - (char *)fc->internal;
      memcpy(tbptr, fc->internal, nbytes);	/* upto X-Sun-Text-Type */
      tbptr += nbytes;
      /* SKIP this header */
      cp = fc->slem.txthdr + fc->slem.txthdr_len;
      /* Copy between the two headers */
      nbytes = fc->slem.con_typehdr - cp;
      if (nbytes) {
	memcpy(tbptr, cp, nbytes);
	tbptr += nbytes;
      }
      /* NOW move the MIME headers into place */
      memcpy(tbptr, MTPHDR1, mh1len);
      tbptr += mh1len;
      memcpy(tbptr, fc->slem.charset, fc->slem.charsetlen); /* the charset */
      tbptr += fc->slem.charsetlen;
      *tbptr++ = '\n';		/* needs a newline too */
      /* NOW move from after the content type to the end */
      cp = fc->slem.con_typehdr + fc->slem.con_typehdrlen; /* adjust source */
      n_data = cp - fc->internal;	/* internal hdr to cur. position. */
      nbytes = *msglen - n_data;	/* end - curpos: msg data remaining */
      memcpy(tbptr, cp, nbytes);

    } else {			/* UPTO content-length first */

      nbytes = fc->slem.con_typehdr - (char *)fc->internal;
      memcpy(tbptr, fc->internal, nbytes);/* upto Content-Type */
      tbptr += nbytes;
      /* SKIP this header */
      cp = fc->slem.con_typehdr + fc->slem.con_typehdrlen;
      /* Copy between the two headers */
      nbytes = fc->slem.txthdr - cp;
      if (nbytes) {
	memcpy(tbptr, cp, nbytes);
	tbptr += nbytes;
      }
      /* NOW move the MIME headers into place */
      memcpy(tbptr, MTPHDR1, mh1len);
      tbptr += mh1len;
      memcpy(tbptr, fc->slem.charset, fc->slem.charsetlen); /* the charset */
      tbptr += fc->slem.charsetlen;
      *tbptr++ = '\n';		/* needs a newline too */
      /* NOW move from after the text-type hdr to the end */
      cp = fc->slem.txthdr + fc->slem.txthdr_len; /* adjust source */
      n_data = cp - fc->internal;	/* internal hdr to cur. position. */
      nbytes = *msglen - n_data;	/* end - curpos: msg data remaining */
      memcpy(tbptr, cp, nbytes);
    }
    fs_give((void **)&fc->slem.charset); /* free this now */
  }
  /* free our old fc and use our new one */
  body_pos = tbptr;
  tbptr += nbytes;
  *tbptr = '\0';
  /*  WE DO NOT FREE THE ofc->internal data. 
   *  THIS is volatile. */
  if (keep_mime) {
    fs_give((void **)ofc);
    *ofc = nfc;			/* our new cache */
    *msglen = *msglen + resize_bytes;
    /* We ALWAYS keep TEXT/PLAIN MIME translations, ie, rewrite
     * them back to the mail file replacing the Content-Type: TEXT
     * messages. bodysize, etc, set during header parse */
    (*ofc)->mime_tr = NULL;
    /* This message will be in the cache for the parse */
    (*ofc)->text_trans = T;
    (*ofc)->body_offset = 0;
    (*ofc)->bodysize = 0;
    (*ofc)->body = NULL;
  } else {
    /* READ ONLY MAILBOX - keep_mime is overridden:
     * We keep it on our heap forever */
    char *fh,c;
    unsigned long header_len;
    unsigned long content_len;
    (*ofc)->mime_tr = nfc;
    fh = strchr(nfc->internal,'\n') + 1;
    header_len = (fh - nfc->internal);	/* size of internal header */

    nfc->header = fh;		/* rfc822 header */
    /* find the body start */
    while (c = *body_pos++) {
      if (c == '\n' && *body_pos == '\n')
	break;
    }

    if (c == '\0') {		/* NO real body */
      --body_pos;		/* Where the NULL is */
      nfc->headersize = body_pos - nfc->internal;
      content_len = 0;
    } else {
      ++body_pos;		/* after "\n\n": First char of body */
      nfc->headersize = body_pos - nfc->internal;
      content_len = tbptr - body_pos;
    }
    nfc->rfc822_hdrsize = nfc->headersize - header_len;
				/* in case a totally blasted msg is found */
    if ((int)nfc->rfc822_hdrsize < 0) nfc->rfc822_hdrsize = 0;
    nfc->body = body_pos;
    nfc->content_length = nfc->bodysize = content_len;
    nfc->broken_content_len = NIL;
  }
  /* for debugging only */
  tbptr = (char *)nfc->internal;
}

/*
 * NOTE: here we do the multipart translation. The file cache pointer
 * will be replaced by a new one, and the msglen will change */
typedef struct _hdr_data_blk_ {
  char *dst;
  int max_len;
  int nxt_byte;
} HDRDATABLK;

long translate_hdrs_to_mime(ATTPART *parts, char *boundary);
void append_mime_hdr(HDRDATABLK *hdb, char *str);

#define MIMECHUNK 256		/* initial header size */
void 
do_multipart_translation(FILECACHE **ofc, long *msglen, int keep_mime)
{
  FILECACHE *fc= *ofc, *nfc;
  char boundary[MAXBOUNDARY];
  char le_cul[MAXBOUNDARY];	/* the tail boundary */
  int cul_grandeur;
  char tmp[16];
  HDRDATABLK hdb;
  long content_len, delta_len;
  long required_len;
  char *new_guy;
  char *tail_pos;
  unsigned long cl_pos, ct_pos;
  size_t al_bytes, cp_bytes;
  size_t bytes_copied;
  size_t loc_msg_len;
  ATTPART *part;
  char *msg;
  long tmp_msglen;
  char *body_pos;
  long header_len;
  int sop_offset;		/* for possible preamble: 29-ju-96 wjy */

  /* nice random one that allows recreation in microseconds */
  set_mime_boundary(boundary);
  /* First we build the mime translated headers from the parsed
   * sun attachment headers:
   * Returns the sum of the new mime hdrs and content lengths
   * of parts */
  content_len = translate_hdrs_to_mime(fc->slem.parts, boundary);

  /* Get offset from start of body to start of first part which
   * is usually 0, but there may be a preamble here */
  sop_offset = fc->slem.parts->bop - fc->slem.body_ptr;
  content_len += sop_offset;	/* Include preamble in content-length */

  strcpy(le_cul, "\n--");       /* Make the last boundary */
  strcat(le_cul, boundary);
  strcat(le_cul, "--\n");	/* this is the tail of it all */
  cul_grandeur = strlen(le_cul); /* add this to our change */
  content_len += cul_grandeur;	 /* message content length */
  /*
   * Make the new Content-Length: XXX and
   * Content-Type: Multipart/Mixed; boundary=<boundary> */
  hdb.nxt_byte = 0;
  hdb.max_len = MIMECHUNK;
  hdb.dst = fs_get(MIMECHUNK);
  *hdb.dst = '\0';
  append_mime_hdr(&hdb, "Content-Length: ");
  sprintf(tmp, "%d\n", content_len);
  append_mime_hdr(&hdb, tmp);
  append_mime_hdr(&hdb, "MIME-Version: 1.0\n");
  append_mime_hdr(&hdb, "Content-Type: Multipart/Mixed;\n\tboundary=\"");
  append_mime_hdr(&hdb, boundary);
  append_mime_hdr(&hdb, "\"\n");
  /*
   * calculate buffer size:
   * Content_len + old hdr len + new hdr field - old hdr field */
  al_bytes = hdb.nxt_byte;	/* length of new headers */
  al_bytes -= fc->slem.con_typehdrlen + fc->slem.con_lenhdrlen;
  al_bytes += content_len;
  /* add (beginning of first part - beginning of message), ie, 
   * old header length */
  al_bytes += (long)((size_t)fc->slem.parts->bop - 
		     (size_t)fc->internal);
  /*
   * Reallocate a file cache, and translate the message into
   * MIME format. */
  al_bytes +=  1;
  al_bytes = (((al_bytes + 1) >> 1) << 1) + 16; /* Meme a little extra */
  
  nfc = fs_get(sizeof(FILECACHE));
  memcpy(nfc,fc,sizeof(FILECACHE));
  /* For the msg data buffer */
  if (keep_mime) {	 /* from mmap world */
    solaris_get(al_bytes,INTERNAL_DATA(nfc));
    new_guy = INTERNAL_BASE(nfc);
  } else		 /* frpm our heap */
    new_guy = fs_get(al_bytes);
  nfc->internal = new_guy;
  /* write out the header:
   * 1) Copy the filecache up to either the content-type or content-len
   *    header, whichever comes first */
  cl_pos = (unsigned long) fc->slem.con_lenhdr;
  ct_pos = (unsigned long) fc->slem.con_typehdr;
  if ((ct_pos < cl_pos) || /* content-type before content-length */
      (cl_pos == 0)) {	   /* or NO content-length field */
    char *tcp;

    /* so, we have:
     * <Content-Type> <stuff or empty> <Content-Length> */
    cp_bytes = ct_pos - (unsigned long)fc->internal;
    bytes_copied = cp_bytes;	/* sanity check */
    memcpy(new_guy, fc->internal, cp_bytes); /* start of FILECACHE to Content-Length */
    new_guy += cp_bytes;

    memcpy(new_guy, hdb.dst, hdb.nxt_byte); /* new headers */
    new_guy += hdb.nxt_byte;
    bytes_copied += hdb.nxt_byte;
				/* Byte after content type header */
    tcp = fc->slem.con_typehdr + fc->slem.con_typehdrlen;
    if (cl_pos != 0) {		/* We have a content-length */
      ct_pos += fc->slem.con_typehdrlen;
      cp_bytes = cl_pos - ct_pos;	/* bytes between the headers */
      if (cp_bytes > 0) {		/* could be 0 if hdrs are ajacent */
	memcpy(new_guy, tcp, cp_bytes); /* between the headers */
	new_guy += cp_bytes;
	bytes_copied += cp_bytes;
      }
				/* point to after content-length hdr */
      tail_pos = fc->slem.con_lenhdr + fc->slem.con_lenhdrlen;
    } else
      tail_pos = tcp;		/* After content type header */
  } else {			/* content-length before content-type */
    char *tcp;

    /* so, we have:
     * <Content-Length> <stuff or empty> <Content-Type> */
    cp_bytes = cl_pos - (unsigned long)fc->internal; /* n bytes upto here */
    bytes_copied = cp_bytes;	/* sanity check */
    memcpy(new_guy, fc->internal, cp_bytes); /* start of FILECACHE to Content-Type */
    new_guy += cp_bytes;

    memcpy(new_guy, hdb.dst, hdb.nxt_byte); /* new headers */
    new_guy += hdb.nxt_byte;
    bytes_copied += hdb.nxt_byte;
				/* byte after content length hdr */
    tcp = fc->slem.con_lenhdr + fc->slem.con_lenhdrlen;
    cl_pos += fc->slem.con_lenhdrlen;
    cp_bytes = ct_pos - cl_pos;	/* bytes between the headers */
    if (cp_bytes > 0) {		/* could be 0 if hdrs are ajacent */
      memcpy(new_guy, tcp, cp_bytes); /* between the headers */
      new_guy += cp_bytes;
      bytes_copied += cp_bytes;
    }
				/* point to after content-type hdr */
    tail_pos = fc->slem.con_typehdr + fc->slem.con_typehdrlen;
  }
  /* Now, copy from the tail position to the beginning of the 
   * first part */
  cp_bytes = (size_t)(fc->slem.parts->bop) - (size_t)tail_pos;
  memcpy(new_guy, tail_pos, cp_bytes);
  new_guy += cp_bytes;
  bytes_copied += cp_bytes;

  /* Calculate the message length:
   * Header length + Content Length */
  tmp_msglen = (long)((size_t)new_guy - (size_t)nfc->internal);
  header_len = tmp_msglen;      /* header length */
  body_pos = new_guy;           /* position of body of msg */
  body_pos -= sop_offset;       /* adjust for preamble. WJY 29-jul-96 */
  header_len -= sop_offset;     /* Also the header len. WJY 5-aout-96 */
  tmp_msglen += content_len;	/* new message length */

  msg = nfc->internal;		/* for debugging purposes */

  /* write out the body parts -- we've copied up to the start
   * of the first body part, ie, "----------" */
  part = fc->slem.parts;
  while (part) {
				/* the MIME headers */
    memcpy(new_guy, part->mime_hdrs, part->mime_hdr_len);
    new_guy += part->mime_hdr_len;
    bytes_copied += part->mime_hdr_len;

    /* Use either the found content length of this part, or
     * it's computed content length */
    memcpy(new_guy, part->boc, part->content_len); 
    new_guy += part->content_len;
    bytes_copied += part->content_len;
    part = part->next;
  }
  /* See if the last char of the last part's contents is NIL. If so, then
   * replace it will "\n". The high level parser puts a NIL at the end
   * of the buffer. We need to step on it. Ugly!!! */
  if (new_guy[-1] == '\0')
    new_guy[-1] = '\n';
  /* append the \n--boundary--\n */
  strcpy(new_guy, le_cul);
  new_guy += cul_grandeur;
  loc_msg_len = (size_t)new_guy - (size_t)nfc->internal; /* message len */
  bytes_copied += cul_grandeur;

  /* dispose of the parts */
  free_the_parts(fc->slem.parts);

  /* free header data */
  fs_give ((void **)&hdb.dst);  

  /* return the ofc as the new_guy if we are keeping 
   * our MIME translations. */
  if (keep_mime) {
    /*
     * We free the old X-Sun-Attachment cache,
     *  point the old cache to the MIME cache,
     *  and point to the same element for use
     *  in computing body, etc...
     *  WE DO NOT FREE THE ofc->internal data. 
     *  THIS is volatile */
    fs_give ((void **)ofc);
    *ofc = nfc;
    nfc->body_offset = body_pos - nfc->internal;
    nfc->bodysize = content_len;
    nfc->body = NIL;
    (*ofc)->mime_tr = NIL;
    *msglen = tmp_msglen;
  } else {
    /* Not keeping the MIME. But, we need the MIME translation
     * for computing body, etc ... */
    char *fh;
    (*ofc)->mime_tr = nfc;
    fh = strchr(nfc->internal, '\n') + 1;
    nfc->headersize = header_len;	/* internal header */
    header_len -= (fh - nfc->internal);	/* rfc822 header */
    nfc->rfc822_hdrsize = header_len;
    nfc->header = fh;		/* rfc822 header */
    nfc->body = body_pos;
    nfc->content_length = nfc->bodysize = content_len;
    nfc->broken_content_len = NIL;
  }
}

typedef struct _xsa_content_hdrs_to_MIME_ {
  char *soleil;
  char *mime;
} XSCHDRTOMIME;

XSCHDRTOMIME sun_to_mime[]= {
  {"text", "Text/plain"},
  {"troff", "Text/X-sun-troff"},
  {"nroff", "Text/X-sun-nroff"},
  {"h-file", "Text/X-sun-h-file"},
  {"c-file", "Text/X-sun-c-file"},
  {"makefile", "Text/X-sun-makefile"},
  {"filemgr-prog", "Text/X-sun-filemgr-prog"},
  {"richtext", "Text/richtext"},
  {"audio-file", "Audio/basic"},
  {"default", "Application/X-sun-default"},
  {"default", "Application/octet-stream"},
  {"oda", "Application/oda"},
  {"sunwrite-document", "Application/X-sun-sunwrite-document"},
  {"tapetool-prog", "Application/X-sun-tapetool-prog"},
  {"textedit-prog", "Application/X-sun-textedit-prog"},
  {"postscript-file", "Application/PostScript"},
  {"sun-raster", "Image/X-sun-raster"},
  {"shell-script", "Application/X-sun-shell-script"},
  {"jpeg", "Image/jpeg"},
  {"g3fax", "Image/X-sun-g3fax"},
  {"g3-file", "Image/X-sun-g3-file"},
  {"gif-file", "Image/gif"},
  {"pbm", "Image/X-sun-pbm"},
  {"pgm", "Image/X-sun-pgm"},
  {"ppm", "Image/X-sun-ppm"},
  {"xpm-file", "Image/X-sun-xpm-file"},
  {"tiff", "Image/X-sun-tiff-file"},
  {"tiff-file", "Image/X-sun-tiff-file"},
  {"mail-file", "Text/X-sun-mail-file"},  
  {"message", "Message/rfc822"},
  {NIL, NIL},
};
typedef struct _charset_trans_ {
  char *mime_cs, *xsun_cs;
} CSTRANS;

/* Known MIME charsets - Only 1 X-Sun-Att. equivalent */
static CSTRANS chrset_trans[] = {
  {"us-ascii", "ascii"},
  {"X-latin-1", "latin-1"},
  {"X-iso-2022-JPN-7", "iso-2022-JPN-7"},
  {"X-iso-2022-KOR-7", "iso-2022-KOR-7"},
  {"X-iso-2022-TWN-7", "iso-2022-TWN-7"},
  {"X-iso-2022-CHN-7", "iso-2022-CHN-7"},
  /* May have been translated FROM Mime to SUN. WE can translate back */
  {"iso-8859-1", "X-iso-8859-1"},
  {"iso-8859-2", "X-iso-8859-2"},
  {"iso-8859-3", "X-iso-8859-3"},
  {"iso-8859-4", "X-iso-8859-4"},
  {"iso-8859-5", "X-iso-8859-5"},
  {"iso-8859-6", "X-iso-8859-6"},
  {"iso-8859-7", "X-iso-8859-7"},
  {"iso-8859-9", "X-iso-8859-8"},
  {"iso-8859-8", "X-iso-8859-9"},
  {NIL, NIL},
};

/* These convert to the MIME Content-Transfer-Encoding field */
char *mime_enc_table[]= {
  "base64",
  "quoted-printable",
  "8bit",
  "7bit",
  "binary",
  NIL,
};
/* These convert to the MIME conversions parameter */
XSCHDRTOMIME xsun_enc_table[]= {
  {"compress", ";\n\tconversions=\"X-compress\""},
  {"uuencode", ";\n\tconversions=\"X-uuencode\""},
  {"compress,uuencode", ";\n\tconversions=\"X-compress, X-uuencode\""},
  {"default-compress,uuencode", ";\n\tconversions=\"X-compress, X-uuencode\""},
  {"adpcm-compress,uuencode", 
   ";\n\tconversions=\"X-adpcm-compress, X-uuencode\""},
  /* These guys ARE MIME conversions. If there is a match,
   * then no conversions parameter is made, rather, the
   * exact MIME Content-Transfer-Encoding header is created */
  {"base64", NIL},
  {"quoted-printable", NIL},
  {"8bit", NIL},
  {"7bit", NIL},
  {"binary", NIL},
  {NIL, NIL},
};
static char *conversion_template= {";\n\tconversions=\"X-"};
void *construir_mime_conversion (char *att_conv, char *obuf);
/*
 * Return: Length of body so far. Still lacks terminating
 * "--boundary-- length */
long translate_hdrs_to_mime(ATTPART *parts, char *boundary)
{
  ATTPART *part= parts;
  long content_len= 0, mime_hdr_len= 0;
  long body_len;
  int this_hdr;
  HDRDATABLK data_blk;
  int n_parts= 0;

  while (part) {
    XSCHDRTOMIME *htab= sun_to_mime;
    char tmp[CTYPEBUF];
    char name[CTYPEBUF];
    char type[CTYPEBUF];
    char obuf[CTYPEBUF];
    char *hdr;
    char c;
    char *tptr;
    int namep, typep;
    AHDRDESC *atthp;
    int i;
    long sun_hdr_len;

    /* Allocate space for the mime header for this part */
    part->mime_hdrs = fs_get(MIMECHUNK); /* for the mime headers */
    part->mime_hdrs[0] = '\0';
    data_blk.dst = part->mime_hdrs;
    data_blk.max_len = MIMECHUNK;
    data_blk.nxt_byte = 0;
    n_parts += 1;

    /*
     * "\n" before 1st boundary */
    if (n_parts == 1)
      append_mime_hdr(&data_blk, "--");
    else
      append_mime_hdr(&data_blk, "\n--");

    append_mime_hdr(&data_blk, boundary);
    append_mime_hdr(&data_blk, "\n");
    append_mime_hdr(&data_blk, "Content-Type: ");
    /* Translate the X-Sun-Data-Type:
     *   BUG FIX: 20-May-96 WJY - Make sure data-type present */
    if (part->xsathdrs.xs_data_type.present) {
      tptr = tmp;
      hdr = part->xsathdrs.xs_data_type.att_hdr; /* data portion of header */
      while ((c = *hdr++) != '\n')
	*tptr++ = tolower(c);
      *tptr = '\0';
      while (htab->soleil) {
	if (strcmp(tmp, htab->soleil) == 0) break;
	else ++htab;
      } 
      /* See if we found a match */
      if (htab->mime)
	append_mime_hdr(&data_blk, htab->mime);
      else {
	append_mime_hdr(&data_blk, "Application/X-sun-");
	append_mime_hdr(&data_blk, tmp);
      }
    } else {
      /* xs_data_type NOT present */
      append_mime_hdr(&data_blk, "Application/X-sun-");
      append_mime_hdr(&data_blk, "default");
    }
    /* now check for paramters to append, eg, charset=us-ascii */
    if (part->xsathdrs.xs_char_set.present) {
      CSTRANS *chrsets= chrset_trans;
      char *tmp1;
      tptr = tmp;

      hdr = part->xsathdrs.xs_char_set.att_hdr;	/* data portion of header */
      while ((c = *hdr++) != '\n')
	*tptr++ = tolower(c);
      *tptr = '\0';
      tmp1 = tmp;
      if (strncasecmp(tmp, "X-", 2) == 0) {
	tmp1 = tmp + 2;		/* in case no match, don't duplicate "X-" */
      }
      /* look for acceptable charsets */
      while (chrsets->xsun_cs)
	if (strcasecmp(tmp, chrsets->xsun_cs) == 0)
	  break;
	else
	  ++chrsets;
      append_mime_hdr(&data_blk, ";\n\tcharset=");
      if (chrsets->xsun_cs) 
	append_mime_hdr(&data_blk, chrsets->mime_cs);
      else {
	append_mime_hdr(&data_blk, "X-"); /* not supported */
	append_mime_hdr(&data_blk, tmp1);
      }
    }
    /*
     * Look for standard sun encoding that translates to
     * a "conversions=" paremeter */
    if (part->xsathdrs.xs_encd_info.present) {
      XSCHDRTOMIME *xsun_enc= xsun_enc_table;
      char *mime_conversion;

      tptr = tmp;
      hdr = part->xsathdrs.xs_encd_info.att_hdr;	/* data portion */
      while ((c = *hdr++) != '\n') {
	if (c == ' ') continue;	/* suppress spaces */
	else
	  *tptr++ = tolower(c);
      }
      *tptr = '\0';
      /* Look for a "legal" encoding -- let's not all laugh at once! */
      while (xsun_enc->soleil) {
	if (strcmp(tmp, xsun_enc->soleil) == 0)
	  break;
	else
	  ++xsun_enc;
      }
      if (!xsun_enc->soleil) {	/* not in our table: Contruct something */
	construir_mime_conversion(tmp, obuf);
	mime_conversion = obuf;
      } else
	mime_conversion = NIL;
      /*
	  Append either the conversion from the table or the
	  contructed conversion UNLESS
	     xsun_enc->mime == NIL && mime_conversion == NIL.
	  In this latter case we have found an EXACT MIME encoding
          in a Sun attachment. We take care of that below */
      if (mime_conversion || xsun_enc->mime)
	append_mime_hdr(&data_blk, 
			(char *)(mime_conversion ? mime_conversion :
				 xsun_enc->mime));
    }

    /* Look for Content-Description fields. We use 
     * <X-sun-data-name>; <X-sun-data-description>,
     * in the content-description and also
     * name is used as a "name=<parameter>" */
    if (part->xsathdrs.xs_data_name.present) {
      namep = T;
      append_mime_hdr(&data_blk, ";\n\tname=\"");

      tptr = tmp;
      hdr = part->xsathdrs.xs_data_name.att_hdr;	/* data portion  */
      while ((c = *hdr++) != '\n')
	*tptr++ = c;		/* no lower case - 11 Nov 93 */
      *tptr = '\0';
      append_mime_hdr(&data_blk, tmp);      
      append_mime_hdr(&data_blk, "\"");      
      strncpy(name, tmp, CTYPEBUF); /* save a copy for later */
    } else
      namep = NIL;
    /* Look for the XS data description hdr:
     * If present, then we use it
     * as part of the Content-description */
    if (part->xsathdrs.xs_data_desc.present) {
      typep = T;
      tptr = tmp;
      hdr = part->xsathdrs.xs_data_desc.att_hdr; /* data portion of header */
      while ((c = *hdr++) != '\n')
	*tptr++ = c;
      *tptr = '\0';
      strncpy(type, tmp, CTYPEBUF);
    } else
      typep = NIL;
    /*  Use either "Content-Description: <X-sun-data-description>" 
     *          or "Content-Description: <X-sun-data-name> */
    if (namep || typep) {	/* make Content-Description hdr line */
      append_mime_hdr(&data_blk, "\nContent-Description: ");
      if (typep) {
	append_mime_hdr(&data_blk, type);
      } else if (namep)
	append_mime_hdr(&data_blk, name);
    }
    /* Look for any non-sun encoding parameters. There really
     * should not be any. If so, we make a Content-Transfer-Encoding 
     * header if encoding info is present. 
     * That is we only put LEGAL MIME encoding values in this
     * header. 
     *
     * All sun encoding should be registered too. */
    if (part->xsathdrs.xs_encd_info.present) {
      char **mime_enc= mime_enc_table;

      tptr = tmp;
      hdr = part->xsathdrs.xs_encd_info.att_hdr;	/* data portion */
      while ((c = *hdr++) != '\n') {
	if (c == ' ') continue;	/* suppress spaces */
	else
	  *tptr++ = tolower(c);
      }
      *tptr = '\0';
      /* Look for a legal encoding -- let's not all laugh at once! */
      while (*mime_enc) {
	if (strcmp(tmp, *mime_enc) == 0)
	  break;
	else
	  ++mime_enc;
      }
      if (*mime_enc) {
	append_mime_hdr(&data_blk, "\nContent-Transfer-Encoding: ");
	append_mime_hdr(&data_blk, *mime_enc);
      }
    }
    /* OK, '\n' to terminate last header line AND
     * the blank line after the MIME-header */
    append_mime_hdr(&data_blk, "\n\n");

    /* compute delta, ie, the change in message size because of the
     * change in header */
    this_hdr = data_blk.nxt_byte; /* length of the MIME headers */
    part->mime_hdr_len = this_hdr; /* save this for later */
    mime_hdr_len += this_hdr;	      /* total of mime hdr lengths */
    content_len += part->content_len; /* total of content lengths */

    part = part->next;
  }
  /* Return new body length. Needs last boundary to be completed */
  body_len = content_len + mime_hdr_len;
  return body_len;
}
/* This is a little faster than appending because we have saved
 * an index of the next byte. Therefor, one does not have to rescan
 * the string for the null */
void
append_mime_hdr(HDRDATABLK *hdb, char *str)
{
  char c;
  int nb= hdb->nxt_byte;

  while (c = *str++) {
    if (nb == hdb->max_len) {
      hdb->max_len += MIMECHUNK;
      fs_resize((void **)&hdb->dst, hdb->max_len);
    }
    hdb->dst[nb++] = c;
  }
  if (nb == hdb->max_len) {
    hdb->max_len += MIMECHUNK;
    fs_resize((void **)&hdb->dst, hdb->max_len);
  }
  hdb->dst[nb] = '\0';
  hdb->nxt_byte = nb;
}

/*
 * Here we parse sun attachments which sets up things for
 * the translation to MIME */
typedef enum {
  nulltype = 0,
  datatype = 1,
  texttype = 2,
  datadesc = 3,
  dataname = 4,
  encoding = 5,
  charset = 6,
  xscontlen = 7,
  xsconlines = 8
} XSHDRTYPE;

typedef struct _xsun_att_tab_ {
  char *header;
  XSHDRTYPE type;
} XSUNATTTAB;

XSUNATTTAB x_sun_att_hdrs[]= {
  {"x-sun-data-type: ", datatype},
  {"x-sun-text-type: ", texttype},
  {"x-sun-data-description: ", datadesc},
  {"x-sun-data-name: ", dataname},
  {"x-sun-encoding-info: ", encoding},
  {"x-sun-charset: ", charset},
  {"x-sun-content-length: ", xscontlen},
  {"x-sun-content-lines: ", xsconlines},
  { NIL, nulltype},
};

char *attacment_header(char *header, char *candidate, char *real_hdr);
/* Called ONLY if we are using line count to find the next section,
 * and we are at the LAST line in a section. 
 *   Want to see if a buggy mail is off by one, and this is really
 *   the start of a section. Merci beaucoup pour en violant le spec mecs!
 */
static VM_start_found(char *lp)
{
  if (strncmp(lp,SECTION,SECTIONLEN) == 0) {
    lp += SECTIONLEN;			/* next line */
    if (!*lp || strncmp(lp,SECTION,SECTIONLEN) != 0)
      return T;			        /* NO next line OR not a section */
  }
  return NIL;
}
/* parses a sun attachment. We return T if parse was successful,
 * and NIL if it wasn't. In the former case one translates to MIME.
 * Dans celui-ci, on ne fait rien. IN the latter one does nothing */
int
parse_sun_attachment(FILECACHE *fc)
{
  char *lp;
  char *cptr;
  int i;
  ATTPART *part;
  int n_found10m = 0;
  int n_expected_attachments = 1;
  /* OK. Lets look for the first attachment - it may not exist! BUGs
     are always there */
  lp = fc->slem.con_typehdr;	/* start at the beginning of a line */
  /* Look for the '\n' body separator */
  while (lp) {
    lp = strchr(lp, '\n');
    ++lp;			/* skip eol */
    if (*lp++ == '\n') break;	/* "\n\n" Le corps - THE body */
  }
  fc->slem.parts = NIL;		/* none yet (encore aucune partie) */
  if (!lp) {			/* broken message - no *part* translation */
    return NIL;
  } else fc->slem.body_ptr = lp;/* pointer to START OF BODY: 29-jul-96 WJY */

  /* OK, we on the start of the message body - look for the 
   * "----------" section starts and parse each one */
  /* We do as much in-line as possible to speed things up. Ugly but fast */
  while (*lp) { /* THE BIG PARSE */
    long content_len= -1;
    long content_lines= -1;
    long max_part_len;
    char *bop;

    if (lp[0] == '-' && lp[1] == '-' && lp[2] == '-' && lp[3] == '-' &&
	lp[4] == '-' && lp[5] == '-' && lp[6] == '-' && lp[7] == '-' &&
	lp[8] == '-' && lp[9] == '-' && lp[10] == '\n') {
      bop = lp;			/* beginning of part */
				/* start of section */
      lp += SECTIONLEN;
      if (!*lp) return NIL;	/* one never knows */
      else n_found10m += 1;       /* Must be true for EVERY attachment */
      /* allocate the attachment part list */
      if (!fc->slem.parts)
	part = fc->slem.parts = fs_get(sizeof(ATTPART));
      else {
	part->next = fs_get(sizeof(ATTPART));
	part = part->next;
      }
      /* clear the part */
      memset((void *)part, 0, (size_t)sizeof(ATTPART));
      part->bop = bop;		/* save section start */
      while (*lp != '\n') {	/* now parse the headers */
	char tmp[CTYPEBUF], *tptr= tmp;
	char c;
	int len= 0;
	char *hdr_fld;
	char *hdr_start= lp;
	XSUNATTTAB *xtab= x_sun_att_hdrs;
	
	/* copy a line to lower case, and save its length */
	while (c = *lp++) {
	  *tptr++ = tolower(c);
	  ++len;
	  /* Check line length. Twice MAX smtp line is plenty big */
	  if (len == CTYPEBUF) {
	    free_the_parts(fc->slem.parts);
	    return NIL;
	  }
	  if (c == '\n')	/* end-of-line */
	    break;
	}
	if (!*lp) {		/* broken file */
	  free_the_parts(fc->slem.parts);
	  return NIL;		/* no translation */
	}
	/* lp now at start of next line */
	*tptr = '\0';		/* tie off the buffer */
	/* see if it is in our table - if it isn't - call it
	   an extension - they may exist. More code for the vapeur! */
	while (xtab->header) {
	  /* hdr_field points to the char after the "..: " */
	  if (hdr_fld = attacment_header(xtab->header, tmp, hdr_start))
	    break;
	  else 
	    ++xtab;
	}
	/* Dispatch on header type */
	if (xtab->header) {	/* build our part */
	  AHDRDESC *ahd;
	  switch (xtab->type) {
	  case datatype:
	    ahd = &part->xsathdrs.xs_data_type;
	    break;
	  case texttype:
	    ahd = &part->xsathdrs.xs_text_type;
	    break;
	  case datadesc:
	    ahd = &part->xsathdrs.xs_data_desc;
	    break;
	  case dataname:
	    ahd = &part->xsathdrs.xs_data_name;
	    break;
	  case encoding:
	    ahd = &part->xsathdrs.xs_encd_info;
	    break;
	  case charset:
	    ahd = &part->xsathdrs.xs_char_set;
	    break;
	  case xscontlen:
	    ahd = &part->xsathdrs.xs_cont_len;
	    content_len = atoi(hdr_fld);
	    max_part_len = fc->eom - hdr_fld;
	    /* 1-mars-96: Validate the content length */
	    if (content_len <= max_part_len)
	      part->content_len = content_len;
	    else {
	      mm_notify(NIL,"147 Broken MailTool Attachment",PARSE);
	      return NIL;
	    }
	    break;
	  case xsconlines:
	    ahd = &part->xsathdrs.xs_cont_lines;
	    content_lines = atoi(hdr_fld);
	    part->content_lines = content_lines;
	    break;
	  }
	  ahd->present = T;
	  ahd->att_hdr = hdr_fld; /* the data in the header */
	  ahd->len = len;
	} else {		/* have an extension - these are ignored */
	  /* We are pointing at the first line AFTER the extension,
	   * it may be '\n' */
	  if (*lp == '\n') break;	/* pointing at BLANK-LINE */
	  /* save extension information */
	  part->xsathdrs.xs_extension.present = T;
	  part->xsathdrs.xs_extension.att_hdr = NIL;
	  part->xsathdrs.xs_extension.len = len;
	}
      }				/* end-header-parse */
      /* Skip to next attachment/end-of-message using either the X-sun 
       * content-length or
       * content-lines - one hopes for content-length :-).
       */
      ++lp;			/* pointing at attachement content */
      part->boc = lp;		/* saved in the part */
      if (part->xsathdrs.xs_cont_len.present)
	lp += content_len;
      else {			/* merde! */
	if (content_lines < 0)
	  return NIL;		/* neither present. Punt. Au revoir dude. */
	content_len = 0;	/* compute this guy */
	while (content_lines-- > 0) {
	  char c;
	  /* Il y a une mauvaise bouge de V3:
           * From time to time the line-count is off by 1 and the last line
           * is really the start of the next attachment, ie, "----------\n"
           *   We check:
           *     (1) content-lines == 0 AND
           *     (2) lp == START of ATTACHMENT
           *     (3) And the NEXT LINE IS NOT START of ATTACHMENT */
	  if (content_lines == 0 && VM_start_found(lp)) break;
	  while (c = *lp++) {
	    content_len += 1;
	    if (c == '\n') break; /* end-of-line */
	  }
          /* May be that message does NOT end with a "\n"
           * Then !c and content_lines == 0. Otherwise something is broken. */
	  if (!c) {
	    if (content_lines > 0)
	      return NIL;		/* line-ct not accurate, broken */
					/* attachement */
	    else
	      --lp;                     /* back to NULL char to terminate the */
					/* loop */
	  }
	}
      }
      part->content_len = content_len;
      cptr = lp;
      /* see if we have at least SECTIONLEN chars */
      for (i = 1, cptr = lp; *cptr && (i < SECTIONLEN); ++i,++cptr);
      if (i == SECTIONLEN)		/* We should have another attachment */
	n_expected_attachments += 1;
    } else {			/* still looking for "----------" */
      /* and sometimes we find end of buffer */
      lp = strchr(lp, '\n');
      if (!lp || *lp++ == '\0') 
	break;			/* end of message found */
    }
  }				/* end of while (T) - all parsed */
  /* 
   * Make sure that the number of "----------\n" lines we've
   * found is the number we expected: Could be we had bad
   * content lengths/line counts, and had data beyond the
   * end of an attachment that was unrecognizable */
  if (n_found10m != n_expected_attachments) return NIL;
  else  return T;
}
/*
   output a mime conversion field from an unknown Sun attachment
   converstion type - the output buffer is twice the SMTP 
   allowable line length. If we overflow, then we simply
   terminate - bad translation will result. */
void *construir_mime_conversion (char *att_conv, char *obuf)
{
  char *out= obuf;
  char c, *cptr= conversion_template;
  char d, *tptr;
  int n= CTYPEBUF-2;

  while (c = *cptr++) {
    n -= 1;
    *out++ = c; /* ;\n\tconversions= "X- */
  }
  /* Look for "x-" and skip it if there. Mailtool bug fix */
  tptr = att_conv;
  c = *tptr++;
  d = *tptr;
  if (c == 'x' && d && d == '-')
    att_conv += 2;
  while (c = *att_conv++) {
    n -= 1;
    if (n <= 0) break;		/* buffer empty - should NEVER happen. */
    if (c != ',') *out++ = c;	/* ',' separates conversions */
    else {
      char *prefix= ", X-";	/* next part */

      while (c = *prefix++) {
	n -= 1;
	if (n == 0) break;
	*out++ = c;
      }
      tptr = att_conv;
      c = *tptr++;
      d = *tptr;
      if (c == 'x' && d && d == '-')
	att_conv += 2;
    }
  }
  /*
    For what's it is worth - Vraiment ce n'est pas la peine */
  if (n <= 0)
    mm_notify(NIL,"Illegal Sun conversion encountered!",PARSE);

  *out++ = '\"';		/* closing delimeter */
  *out++ = '\0';		/* null termination */

  return(0);
}

/* look for an exact match up to the end of the header */
char *
attacment_header(char *header, char *candidate, char *real_hdr)
{
  char *hptr= header, *cptr= candidate;
  char c;

  while (c = *header++) {
    ++real_hdr;
    if (c != *cptr++)
      return NIL;
  }
  return real_hdr;		/* pointer to field -  */
}

void 
init_mime_trans(SOLARISBODY *corps)
{
  corps->con_lenhdr = NIL;
  corps->con_lenhdrlen = 0;
  corps->content_length = 0;

  corps->con_typehdr = NIL;
  corps->con_typehdrlen = 0;

  corps->txthdr = NIL;
  corps->txthdr_len = 0;
  
  corps->charset = NIL;
  corps->charsetlen = 0;

  corps->body_ptr = NIL;	/* 29-July-96 */
  corps->parts = NIL;
}

void
free_the_parts(ATTPART *part)
{
  while (part) {
    ATTPART *next;

    if (part->mime_hdrs)
      fs_give((void **)&part->mime_hdrs);
    next = part->next;
    fs_give((void **)&part);
    part = next;
  }
}

