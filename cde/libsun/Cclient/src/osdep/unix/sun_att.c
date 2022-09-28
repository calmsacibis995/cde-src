/*
 * @(#)sun_att.c	1.11 97/06/11
 *
 * This code sends a sun attachment message translating MIME to sun
 * attachment format from the body build in the c-client. C'est une idee
 * bien laide mais on me paie d'en faire beaucoup.  :-)
 *
 *  Copyright (c) 1993, SMI, Inc. All rights reserved
 *
 *  Some of the included code was derived from smtp.c whcih includes
 *  the following two copyrights:
 *
 * Original version Copyright 1988 by The Leland Stanford Junior University.
 * Copyright 1993 by the University of Washington.
 *
 * Edit Log:
 *
 *	[10-26-96, clin] Passing a mailstream in sun_att_output
 *	[10-26-96, clin] Passing a mailstream in sun_output_body
 *      	Note that in these cases a MAILSTREAM is explicitly
 *		needed 'cause at the ending routine needs a mailstream
 *		not a TCPSTREAM or SENDSTREAM.
 *	[11-08-96, clin] Passing a mailstream in mm_log calls. 
 */
#include "UnixDefs.h"
#include <ctype.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>

#include "mail.h"
#include "os_sv5.h"
#include "smtp.h"
#include "rfc822.h"
#include "misc.h"
#include "sunatt.h"

/*
 * Utilities */
static void init_attachment (ATTACHMENT *att)
{
  att->body_length = 0;	      /* Used in content length field. */
  att->xsun_att = 0;	      /* flags a MIME multipart message */
  att->type_text = 0;	      /* flags type TEXT output */
  att->partie = 0;	      /* our linked list of attachments if xsun_att */
}
static void free_attachement (ATTACHMENT *att)
{
  CORPS *partie= att->partie, *tmp;
  ADAT *ad;

  while (partie) {
    if (partie->data_type)
      fs_give ((void **)&partie->data_type);
    if (partie->content_length)
      fs_give ((void **)&partie->content_length);
    if (partie->encoding)
      fs_give ((void **)&partie->encoding);
    if (partie->data_name)
      fs_give ((void **)&partie->data_name);
    if (partie->data_desc)
      fs_give ((void **)&partie->data_desc);
    /*
     * Free any extra att. data quanti */
    ad = partie->corps.next;	/* begin with next link. 1st is static. */
    while (ad) {
      ADAT *tad= ad->next;
      fs_give((void **)ad);
      ad = tad;
    }
    tmp = partie->next;
    fs_give((void **)&partie);
    partie = tmp;
  }
}


/* Major stuff here ... */

/* Output sun-attachment message
 * Accepts: temporary buffer
 *	    envelope
 *	    body
 *	    I/O routine
 *	    stream for I/O routine
 * Returns: T if successful, NIL if failure
 */

long sun_att_output (char *t, ENVELOPE *env, BODY *body, soutr_t f,
		     TCPSTREAM *s, int return_receipt, HEADERLIST *header_list, 
		     MAILSTREAM *mstream)
{
  ATTACHMENT att;
  long retval;

  sun_encode_body (env, body,mstream);/* encode body as necessary */
  init_attachment(&att);
  sun_make_att_hdrs (body, &att); /* make the attachment headers */
  sun_header (t, env, body, &att, return_receipt, header_list); /* Content-Length, Content-Type */
				     /* output header and body */
  retval = (*f) (s, t, mstream) && 
	(body ? sun_output_body (&att, f, s, mstream) : T);
				/* free our translation description */
  free_attachement(&att);

  return retval;
}

typedef struct _mimeparams_to_xsuna_ {
  char *mimep;
  char *xsa;
  int xslen;
  int quel;			/* which type */
} MIMEP;
#define DODATANAME 0
#define DOENCODING 1
#define DODATATYPE 2
#define DOTYPETEXT 3
#define DODESCRIPT 4
MIMEP conv_params[]= {
  {"name",        "X-Sun-Data-Name: ",        17, DODATANAME},
  {"conversions", "X-Sun-Encoding-Info: ",    21, DOENCODING},
  {"type",        "X-Sun-Data-Type: ",        17, DODATATYPE},
  {"charset",     "X-Sun-Charset: ",          15, DOTYPETEXT},
  {"vfrcmfl",     "X-Sun-Data-Description: ", 24, DODESCRIPT}, /* FAKE PARAM */
  {NIL, NIL, 0},
};
typedef struct _charset_trans_ {
  char *mime_cs, *xsun_cs;
} CSTRANS;

/* Known MIME charsets - Only 1 X-Sun-Att. equivalent */
static CSTRANS chrset_trans[] = {
  {"us-ascii", "ascii"},
  {"iso-2022-jp", "iso-2022-jp"},
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


CORPS *faire_le_corps(ATTACHMENT *att)
{
  CORPS *c= (CORPS *)fs_get (sizeof(CORPS));
  /* Non-required fields */
  c->data_type = c->encoding = c->data_name = c->data_desc = NIL;
  c->text_type = c->content_length = NIL;
  c->corps_longeur = 0L;
  /* allocate and link our body part (corps) descriptor */
  if (!att->partie)	                 /* initialize list  */
    att->partie = c;
  else {			         /* new list link */
    CORPS *nc= att->partie;
    
    while (nc->next) nc = nc->next;
    nc->next = c;
  }
  c->next = NIL;
  c->corps.data = NIL;
  c->corps.next = NIL;
  return c;
}
/* text is a sequence of lines terminated by CRLF:
 * Return the length of the text less the number of CR's */
long sun_unix_length (char *text)
{
  char c;
  long len= 0;

  while (c = *text++) {
    if (c != '\015') len += 1;
  }
  return len;
}

/*
 * Summation of the sizes of all of the parts of a multipart
 * part -- */
long somme_de_parts(ADAT *ad, PART *part)
{
  long somme= 0;
  ADAT *nad;

  while (part) {
    BODY *femme= &part->body;

    switch (femme->type) {
    case TYPEMULTIPART:		/* Not very likely, mais juste en cas ... */
      ad->next = (ADAT *)fs_get(sizeof(ADAT));
      nad->data = NIL;
      nad->next = NIL;
      somme += somme_de_parts(nad, femme->contents.part);
      break;
    case TYPEMESSAGE:
      somme += sun_unix_length (femme->contents.msg.text);
      ad->data = (unsigned char *)femme->contents.msg.text;
      ad->next = NIL;
      break;
    default:
      somme += sun_unix_length ((char *)femme->contents.text);
      ad->data = femme->contents.text;
      ad->next = NIL;
      break;
    }
    part = part->next;
  }
  return somme;
}

typedef struct _subtype_translation__ {
  char *soleil;			/* sun attachment data type */
  char *mime;			/* MIME SUBTYPE */
} SUBTYPETRANS;

SUBTYPETRANS stype_text[]= {
  {"text", "plain"},
  {"troff", "X-sun-troff"},
  {"nroff", "X-sun-nroff"},
  {"h-file", "X-sun-h-file"},
  {"c-file", "X-sun-c-file"},
  {"makefile", "X-sun-makefile"},
  {"filemgr-prog", "X-sun-filemgr-prog"},
  {"richtext", "richtext"},
  {"mail-file", "X-sun-mail-file"},
  {NIL, NIL},
};

SUBTYPETRANS stype_audio[]= {
  {"audio-file", "basic"},
  {NIL, NIL}
};

SUBTYPETRANS stype_video[]= {
  {NIL, NIL}
};

SUBTYPETRANS stype_application[]= {
  {"default", "X-sun-default"},
  {"default", "octet-stream"},
  {"oda", "oda"},
  {"sunwrite-document", "X-sun-sunwrite-document"},
  {"tapetool-prog", "X-sun-tapetool-prog"},
  {"textedit-prog", "X-sun-textedit-prog"},
  {"postscript-file", "PostScript"},
  {"shell-script", "X-sun-shell-script"},
  {NIL, NIL}
};

SUBTYPETRANS stype_image[]= {
  {"sun-raster", "X-sun-raster"},
  {"jpeg", "jpeg"},
  {"g3fax", "X-sun-g3fax"},
  {"g3-file", "X-sun-g3-file"},
  {"gif-file", "gif"},
  {"pbm", "X-sun-pbm"},
  {"pgm", "X-sun-pgm"},
  {"ppm", "X-sun-ppm"},
  {"xpm-file", "X-sun-xpm-file"},
  {"tiff", "X-sun-tiff-file"},
  {"tiff-file", "X-sun-tiff-file"},
  {NIL, NIL}
};

SUBTYPETRANS stype_message[] = {
  {"message", "rfc822"},  
  {NIL, NIL},
};

/*
 * Here we run through the body and make corresponding sun attachment
 * parts when possible - sun attachments are a small subset of MIME
 */

/*
 * We have added UUENCODE for SUN-ATTACHMENTS and keep this private
 */
long
text_strlen(char * text)
{
  /* count all chars less CR ('\015') */
  long 	cr_count;

  for (cr_count = 0; *text != NULL; text++) {
    if (*text != '\015') {
      cr_count++;
    }
  }
  return(cr_count);
}

#define PARAMBUFLEN 256
#define CRCHAR 1		/* Used to remove CR from CRLF */
void sun_make_att_hdrs (BODY *body, ATTACHMENT *att)
{
  PART *part;
  long longeur;
  CORPS *torso;
  char *data_type;
  char *def_data_type;
  SUBTYPETRANS *stt;
  int extra;			/* extra length. */
  int fabriquer= 0;		/* notes fabricated data_type */

  /*
    The simple case first - and the most common - MIME TYPETEXT
    or no body which defaults to the same thing.
    If there is a body encoding, then we make this an attachment. */
  if (!att->xsun_att) {
    if (!body || (body->type == TYPETEXT && !body->encoding)) {
      if (body) {
	att->partie = faire_le_corps(att);
	att->body_length = text_strlen ((char *)body->contents.text);
	att->partie->corps.data = body->contents.text;
      } else {
	att->body_length = 0;
	att->partie = NIL;
      }
      att->type_text = T;		/* attachement type TEXT */
      return;
    }
  }
  /*!!! PART BY PART !!!*/
  extra = 0;                            /* Possible header length */
  /*
   * first determine the length of the part,
   * and store a pointer to its data */
  switch (body->type) {
  case TYPEMULTIPART:
    break;
  case TYPEMESSAGE:
    torso = faire_le_corps(att); /* our "body" description. */
    longeur = sun_unix_length ((char *)body->contents.msg.text);
    att->body_length += 1;        /* FOR CRLF after header block LS */
    torso->corps.data = (unsigned char *)body->contents.msg.text;
    break;
  default:
    torso = faire_le_corps(att); /* our "body" description. */
    torso->corps.data = body->contents.text;
    longeur = sun_unix_length ((char *)body->contents.text);
    att->body_length += 1;        /* FOR CRLF after header block LS */
    break;
  }
  /*
   * OK. Some translations of body type:
   *  We can only have a Content-Type of
   *  1. X-Sun-Attachment  OR
   *  2. text
   *  So, even non-multipart MIME messages force
   *  sun attachments if they are not of TYPETEXT.
   *  These attachments all must have
   *  1. X-Sun-Data-Type: <type>
   *  2. X-Sun-Content-Length: <nbytes after headers CRLF separator>
   *  There are also other none required headers which we will derive
   *  from the paramters. */
  data_type = NIL;
  switch (body->type) {
  case TYPEMULTIPART:
    if (att->xsun_att) {	/* X-Sun-Attachments do not recur */
      CORPS *c= faire_le_corps(att);
      char prov[PROBUFLEN];
      int plong;
      /*
	Generate:
	1. X-Sun-Data-Type: default
	2. X-Sun-Content-Length: <longeur>
	3. X-Sun-Data-Description: Recursive MIME Multipart
	We build the att->partie, do usual length calculation for
	the Content-Length, and return. C'est raisonable, non?*/
      strcpy(prov, "X-Sun-Data-Type: default\015\12");
      plong = strlen(prov);
      c->data_type = fs_get(plong + 16);
      strcpy(c->data_type, prov);
      att->body_length += plong - CRCHAR; /* Don't count the \015 */
				/* Make this dude */
      longeur = somme_de_parts(&c->corps, body->contents.part);
      sprintf(prov, " X-Sun-Content-Length: %ld\015\012", longeur);
      plong = strlen(prov);
      c->content_length = fs_get(plong + 16);
      strcpy(c->content_length, prov);
      att->body_length += plong - CRCHAR;
      
      strcpy(prov,
	     "X-Sun-Data-Description: Recursive MIME Multipart\015\012");
      plong = strlen(prov);
      c->data_desc = fs_get(plong + 16);
      strcpy(c->data_desc, prov);
      att->body_length += plong - CRCHAR;
      att->body_length += HEADER_SEC_LEN; /* ----------<CRLF> */
      return;
    }
    att->xsun_att = T;
    part = body->contents.part;	/* The next part. */
    while (part) {
      sun_make_att_hdrs (&part->body, att);
      part = part->next;
    }
    return;
  case TYPEMESSAGE:
    att->xsun_att = T;
    def_data_type = XS_TYPEMESSAGE;
    stt = stype_message;
    break;
  case TYPETEXT:
    /* This is part of a sun attachment only if the message is
     * Multipart. Then att->xsun_att was set to TRUE in the 
     * multipart section above */
				/* "X-Sun-Charset: ascii" for part.*/
    torso->text_type = cpystr(XSTEXTTYPE);
				/* add to total length calculation */
    extra = strlen(XSTEXTTYPE) - CRCHAR; 
    def_data_type = XS_TYPETEXT;
    stt = stype_text;
    break;
  case TYPEAPPLICATION:
    att->xsun_att = T;
    /* Hack to send Application/octet-stream with a description of
     * "mail-file" with as a sun attachment with a data type of
     * "mail-file". */
    if (body->description && 
	strcasecmp(body->description, "mail-file") == 0) {
      data_type = "mail-file";
      fabriquer = NIL;
    } else {
      /* We may manufacture an "X-%s" as per the specs. */
      fabriquer = T;
      stt = stype_application;
      def_data_type = XS_OCTSTREAM;
    }
    break;
  case TYPEAUDIO:
    att->xsun_att = T;
    def_data_type = XS_AUDIO;
    stt = stype_audio;
    break;
  case TYPEIMAGE:
    att->xsun_att = T;
    def_data_type = XS_UNKNOWN_IMAGE;
    stt = stype_image;
    break;
  case TYPEVIDEO:
    att->xsun_att = T;
    def_data_type = XS_VIDEO;
    stt = stype_video;
    break;
  default: case TYPEOTHER:	/* default data type here */
    att->xsun_att = T;
    /* As per spec we make "X-%s" */
    fabriquer = T;
    def_data_type = XS_UNKNOWN;
    stt = NIL;
    break;
  }
  /* OK, make the data type from the body->subtype
   * IF IT has not been already set (5-jun-95: tu veux dire!!)
   *  (1) No subtype then just take default
   *  (2) Else see if in translation table.
   *  (3) If not found, then 
   *      a. Look for "x-sun-%s" and take %s, or
   *      b.  fabricate from subtype (fabricate == T), or
   *      c.  take the default.
   */
  if (!data_type && body->subtype) {		/* subtype supplied (should be!). */
    /* IF we have a trans. table entry, ie, not default */
    if (stt) {
      while (stt->soleil) {
	if (strcasecmp(stt->mime, body->subtype) == 0) {
	  data_type = stt->soleil;
	  break;
	} else
	  stt += 1;
      }
      /*
       * If not found, then see if "x-sun-%s" */
      if (data_type == NIL) {
	if (strncasecmp("x-sun-", body->subtype, X_SUN_) == 0) {
	  /* Passed from MIME engine as "x-sun-%s". We use the
	   * %s field */
	  data_type = body->subtype + X_SUN_; /* Just skip "X-sun-" */
	  fabriquer = NIL;		      /* not made here so to speak. */
	} else
	  if (!fabriquer)	/* take the default type. */
	    data_type = def_data_type;
      }
    } 
    if (data_type == NIL) {	/* still unresolved */
      /* If we fabricate, then we need to free the memory */
      if (fabriquer) {		/* fabricate it ourself */
	/* If does NOT begin with "X-", then use description OR "X-%s" */
	if (strncasecmp("X-", body->subtype, 2) == 0) {
	  data_type = fs_get(strlen(body->subtype) + 4); /* the space */
	  strcpy(data_type, body->subtype); /* 1 more extra - the NULL */
	} else {
	  /* Use description if present: 3 juin 95 WJY. */
	  if (body->description)
	    data_type = strdup(body->description);
	  else {
	    /* Use the X-%s */
            data_type = fs_get(strlen(body->subtype) + 4); /* the space */ 
	    sprintf(data_type, "X-%s", body->subtype);
	  }
	}
      } else
	data_type = def_data_type;      /* take the default.*/
    } else				/* we succeeded. */
      fabriquer = NIL;			/* so, not made here.. */
  } else {				/* no subtype so take default */
    if (data_type == NIL)
      data_type = def_data_type;
    fabriquer = NIL;		        /* static value not freed. */
  }

  att->body_length += longeur + extra; /* attachment body length */
  if (att->xsun_att) {
    char num[NUMBUFLEN];
    char *prov;
    int plong;
    CORPS *c= torso;
    PARAMETER *p;
    MIMEP *tdt;
    const char *body_enc;
    int body_enc_len;
    
    /*
     * This is the part's body length */
    c->corps_longeur = longeur; /* for X-Sun-Content-Length:  */
    /*
     * Now add (may be 0) additional length for possible 
     * X-Sun-Text-Type header to be included in the body's
     * content length */
    longeur += extra;	
    
    /* The header section separator line and the <CRLF> that separates
     * the headers from the attachment data itself */
    att->body_length += HEADER_SEC_LEN; /* ----------<CRLF> */
    /* Look at parameters for various header descriptions */
    /* 1. "X-Sun-Data-Type:  "*/
    plong = strlen(data_type);
    prov = fs_get(XSDTYPELEN + plong + 16);
    sprintf(prov, "%s%s\015\012", XSDTYPE, data_type);
    if (fabriquer)		/* We fabricated this beast */
      fs_give((void **)&data_type); /* free the space */
    c->data_type = prov;	                 /* save for output */
    att->body_length += strlen(prov) - CRCHAR;

    /* X-Sun-Content-Length: */
    sprintf(num, "%ld", c->corps_longeur);	
    plong = strlen(num);	                 /* Add length of n-bytes */
    prov = fs_get(XSCLENLEN + plong + 16);
    sprintf(prov, "%s%s\015\012", XSCLENGTH, num);
    c->content_length = prov;	                 /* save for output */
    att->body_length += strlen(prov) - CRCHAR;
    /*
     * Look at body description and encoding types to
     * set the corresponding attachment fields */
    if (body->encoding) {
      /* We use UUENCODE in lieu of BASE64 for sun-attachments */
      if (body->encoding == ENCUUENCODE)
	body_enc = "uuencode";
      else
	body_enc = body_encodings[body->encoding];
      
      tdt = &conv_params[DOENCODING];
      c->encoding = (char *)fs_get(strlen(body_enc) + tdt->xslen + 16);
      sprintf(c->encoding, "%s%s\015\012", tdt->xsa, body_enc);
      body_enc_len = strlen(c->encoding) - CRCHAR;
      att->body_length += body_enc_len;
    }
    if (body->description) {
      tdt = &conv_params[DODESCRIPT];
      c->data_desc = (char *)fs_get(strlen(body->description) + 
				    tdt->xslen + 16);
      sprintf(c->data_desc, "%s%s\015\012", tdt->xsa, body->description);
      att->body_length += strlen(c->data_desc) - CRCHAR;
    }
    /* Now the parameters */
    for (p = body->parameter; p; p = p->next) {
      for (tdt = &conv_params[0]; tdt->mimep; ++tdt) {
	if (strcasecmp(tdt->mimep, p->attribute) == 0) {
	  char **donnee;
	  int adonnee= NIL;
	  int use_local_param = NIL;
	  char local_param[PARAMBUFLEN];

	  plong = strlen(p->value);
	  /* Extra byte for LF because LF in not included
	   * in the xslen value */
	  att->body_length += plong + tdt->xslen + 1; /* (***) */
	  switch (tdt->quel) {
	  case DODATANAME:
	    donnee = &c->data_name;
	    break;
	  case DOENCODING:
	    /* Make sure that "X-compress" or "X-uuncode"
             * become "compress" or "uuencode" */
	    if (strcasecmp(p->value, "x-compress") == 0 ||
		strcasecmp(p->value, "x-uuencode") == 0) {
	      use_local_param = T;
	      strncpy(local_param, p->value + 2, PARAMBUFLEN-1); /* skip "x-" */
	      local_param[PARAMBUFLEN-1] = NULL;
	      att->body_length -= 2;
	      plong -= 2;
	    }
	    /* If a body encoding already exists, then we
             * build an encoding with the parameter and
             * concatenate the body encoding. Last thing
             * in the list is always applied first on decode */
	    if (c->encoding) {
	      char *param;
	      if (use_local_param)
		param = local_param;
	      else
		param = p->value;
	      fs_give ((void **)&c->encoding);
	      /* subtract original encoding length which includes the
               * LF. 
               */
	      att->body_length -=  body_enc_len;
	      /* make our own encoding here */
	      c->encoding = (char *)fs_get(tdt->xslen + plong + strlen(body_enc) + 16);
	      sprintf(c->encoding, "%s%s,%s\015\012", tdt->xsa, param, body_enc);
	      /* 
               * add only the concatenated parameter and the "," because
               *    plong + tdt->xslen + 1, ie,
               *  Header: p->valueLF  already counted.
	       */
	      att->body_length += strlen(body_enc) + 1;
	      donnee = &c->encoding;
	      adonnee = T;
	    } else 
	      donnee = &c->encoding;
	    break;
	  case DODATATYPE:
	    if (c->data_type) fs_give ((void **)&c->data_type);
	    donnee = &c->data_type;
	    break;
	  case DOTYPETEXT:
	    if (body->type == TYPETEXT) { /* only for type text */
	      char *charset;
	      CSTRANS *cst= chrset_trans;

	      if (c->text_type) {
		int diff;
		fs_give ((void **)&c->text_type);
				/* override default length */
		/* BUG fix - 2 may 96: 
		 * WAS 
		 * att->body_length -= strlen(XSTEXTTYPE) + CRCHAR;
		 * which removed 1 extra char since the TERMINATING
                 * CRCHAR was NOT included in the length. WJY */
		diff = strlen(XSTEXTTYPE) - CRCHAR;
		att->body_length -= diff;
	      }
	      donnee = &c->text_type;
	      /* Now translate the charset */
	      while (cst->mime_cs) {
		if (strcasecmp(cst->mime_cs, p->value) == 0) {
		  charset = cst->xsun_cs;
		  break;
		} else
		  ++cst;
	      }
	      if (cst->mime_cs) { /* have a translations */
		int dlen = strlen(charset);
		int len= dlen + tdt->xslen + 16;
		int delta = strlen(p->value) - dlen;
		prov = (char *)fs_get(len);
		sprintf(prov, "%s%s\015\012", tdt->xsa, charset);
		att->body_length -= delta;
	      } else {		 /* just "X-%s" */
		int len= strlen(p->value) + tdt->xslen + 16;

		prov = (char *)fs_get(len);
		sprintf(prov, "X-%s%s\015\012", tdt->xsa, p->value);
	      }
	      *donnee = prov;
	      adonnee = T;
	      break;
	    } else
	      continue;
	  }
	  if (!adonnee) {
	      char *param;
	      if (use_local_param)
		param = local_param;
	      else
		param = p->value;
	    prov = (char *)fs_get(plong + tdt->xslen + 16);
	    sprintf(prov, "%s%s\015\012", tdt->xsa, param);
	    *donnee = prov;	/* save the header string */
	  }
	  break;
	}
      } /* for (tdt = ...) */
    } /* for (p = .. ). */
  }				/* end sun attachment length calculations */
}


/* Write RFC822 header with sun-attachment Content-type 
 * and Content-Length.
 *
 * Accepts: scratch buffer to write into
 *	    message envelope
 *	    message body
 */

void sun_header (char *header, ENVELOPE *env, BODY *body, ATTACHMENT *att,
		 int return_receipt, HEADERLIST *header_list)
{
  int i;
  STRINGLIST *new_fields, *new_data;

  if (env->remail) {		/* if remailing */
    long i = strlen (env->remail);
				/* flush extra blank line */
    if (i > 4 && env->remail[i-4] == '\015') env->remail[i-2] = '\0';
    strcpy (header,env->remail);/* start with remail header */
  }
  else *header = '\0';		/* else initialize header to null */
  rfc822_header_line (&header,"Newsgroups",env,env->newsgroups);
  rfc822_header_line (&header,"Date",env,env->date);
  rfc822_address_line (&header,"From",env,env->from);
  rfc822_address_line (&header,"Sender",env,env->sender);
  rfc822_address_line (&header,"Reply-To",env,env->reply_to);
  rfc822_header_line (&header,"Subject",env,env->subject);
  rfc822_address_line (&header,"To",env,env->to);
  rfc822_address_line (&header,"Cc",env,env->cc);
  rfc822_header_line (&header,"In-Reply-To",env,env->in_reply_to);
  rfc822_header_line (&header,"Message-ID",env,env->message_id);
  /* add the return-receipt-to header   LS may 12 1995 */
  if (return_receipt)
    rfc822_address_line (&header, "Return-Receipt-To", env, env->from);

  /* additional headers */
  if (header_list) {
    new_fields = header_list->fields;
    new_data = header_list->data;
    for (i=0; i<header_list->n_entries; i++) {
      if (new_data->text && new_fields->text)
	rfc822_header_line(&header, new_fields->text, env, new_data->text);
      else
	break;
      new_data = new_data->next;
      new_fields = new_fields->next;
    }
  }
  /*
   * Now the two sun-attachment header lines - never prefixed with
   * "Remail-: */
  sun_att_header_lines (&header, body, att);
}

/*
 * Extraire le nom de fichier pour encoder.
 * On cherche le parametre de nom dans la liste:
 */
#define TMPUUNAME "uuencoded."
static char *extract_name_param(PARAMETER *parameter)
{
  char *nom;
  struct timeval tv;

  if (parameter != NIL) {
    PARAMETER *p= parameter;

    while (p) {
      if (strcasecmp (p->attribute, "name") == 0) {
	if (p->value) {		/* SHOULD be there */
	  nom = cpystr(p->value);
	  return nom;
	} else
	  break;		/* Il n'y a pas de nom, grosse erreur!! */
      } else
	p = p->next;
    }
  }
  /*
   * generate a random name */
  gettimeofday(&tv, NIL);
  tv.tv_usec /= 100000;		/* random 5 digits */
  nom = fs_get(strlen(TMPUUNAME) + 5 + 1);
  sprintf(nom, "%s%05d", TMPUUNAME, tv.tv_usec);

  return nom;
}
#define ENCBYTE(c) ((char)((c & 0x3F) + ' ')) /* mask 6 bits + SPACE */
/*
 * uu Encode one line of chars */
static char *uuencode_bytes (unsigned char *src, char *dst, int n)
{
  int i;

  *dst++ = ENCBYTE(n);		              /* The linecount */
  for (i = 0; i < n; i += 3) {
    int c1, c2, c3, c4;
    unsigned char s1, s2, s3;

    s1 = *src++;		              /* Three source bytes */
    s2 = *src++;
    s3 = *src++;

    /* 12345678  --> 00123456 */
    c1 = s1 >> 2;                                 /* H6 bits */
    /* 12345678 --> 00780000 | 12345678 --> 00001234 */
    c2 = ((s1 << 4) & 0x30)  | ((s2 >> 4) & 0xF); /* L2 + H4 bits */
    /* 12345678 --> 00567800 | 12345678 --> 00000012 */
    c3 = ((s2 << 2) & 0x3C) | ((s3 >> 6) & 0x3);  /* L4 + H2 bits */
    /* 12345678 --> 00345678 */
    c4 = s3 & 0x3F;				       /* L6 bits */
    *dst++ = ENCBYTE(c1);			       /* Store 4 bytes */
    *dst++ = ENCBYTE(c2);
    *dst++ = ENCBYTE(c3);
    *dst++ = ENCBYTE(c4);
  }
  *dst++ = '\015';		              /* <CRLF> */
  *dst++ = '\012';

  return dst;
}
/*
 * Here we WILL do uuencoding of an 8 bit source 
 *
 *  Header line of "begin 600 <filename><CRLF>
 * (1) Each 3bytes of src yields 4 bytes of destination
 * (2) Each line begins with the TRUE byte length of the line
 * (3) Each line is terminated by <crlf>
 * (4) The final line is SPACE<crlf>
 *  Trailer line of "end<crlf>"
 *
 *  Maximum line length is 45 bytes which prints as an M.
 *  
 */

/*
 * Macro to make a printable 6 bits. SPACE <= ENCBYTE <= 0137 */
static unsigned char *uuencode_binary(void *src, unsigned long src_len,
				      unsigned long *ret_len,
				      char *file_name)
{
  unsigned char *encbuf;
  char *cob, *bptr;
  unsigned char *sbytes= (unsigned char *)src;
  long enc_buf_len;
  long i, n_morceaux;
  int le_reste;

  if (src_len != 0) {		/* ce n'est pas vide ... */
    enc_buf_len= ((src_len - 1)/3 + 1) * 4; /* N 4-tuples */
    enc_buf_len += ((src_len - 1)/45 + 1) * 3;   /* [BYTECT]..<CRLF> */
    enc_buf_len += 12 + 3 + 5;		      /* header trailer blank line */
    enc_buf_len += strlen(file_name) + 32;    /* name et  d'espace de plus */
  } else				      /* Rien a faire ... */
    enc_buf_len = strlen(file_name) + 32;     /* name et  d'espace de plus */

  /* Now do uuencoding merde */
  cob = bptr = fs_get(enc_buf_len);
  encbuf = (unsigned char *)cob;
  memset((void *)encbuf, 0, (size_t)enc_buf_len); /* clear it out */
				               /* header  */
  sprintf((char *)encbuf, "begin 600 %s\015\012", file_name);
  bptr += strlen((char *)encbuf);
  /*
   * The uuencoded data: */
  n_morceaux = src_len/45;	               /* pieces of 45  */
  le_reste = src_len % 45;		       /* what is left over */
  for (i = 0; i < n_morceaux; ++i) {
    bptr = uuencode_bytes(sbytes, bptr, 45);
    sbytes += 45;
  }
  if (le_reste != 0)		              /* do short line */
    bptr = uuencode_bytes(sbytes, bptr, le_reste);

  strcpy(bptr, " \015\12");	               /* blank line */
  bptr += 3;				       /* skip it */
  strcpy(bptr,"end\015\012");	               /* trailer */
  *ret_len = strlen(cob);		       /* the final count */

  return encbuf;
}

/* Encode a body for 7BIT transmittal
 * Accepts: envelope
 *	    body
 */
void sun_encode_body (ENVELOPE *env, BODY *body, MAILSTREAM *stream)
{
  void *f;
  PART *part;
  char *file_name;

  if (body) switch (body->type) {
  case TYPEMULTIPART:		/* multi-part */
    part = body->contents.part;	/* encode body parts */
    do sun_encode_body (env, &part->body,stream);
    while (part = part->next);	/* until done */
    break;
  case TYPEMESSAGE:
    break;
  default:			/* all else has some encoding */
    switch (body->encoding) {
    case ENC8BIT:		/* encode 8BIT into QUOTED-PRINTABLE */
				/* remember old 8-bit contents */
      f = (void *) body->contents.text;
      body->contents.text = rfc822_8bit (body->contents.text,body->size.bytes,
					  &body->size.bytes);
      body->encoding = ENCQUOTEDPRINTABLE;
      if (f) {
	  (*body->contents_deallocate)((void **)&f);	/* flush old binary contents */
	  body->contents_deallocate = fs_give;
      }
      break;
    case ENCBINARY:		/* encode binary into uuencode */
      f = body->contents.binary;/* remember old binary contents */
      /*
       * We need a file name for the uuencoding. If no "name" parameter
       * is present, then we generate a random tmp name */

      mm_log("UUENCODING data ...", NIL, stream);

      file_name = extract_name_param(body->parameter);
      body->contents.text = uuencode_binary (body->contents.binary,
					     body->size.bytes,
					     &body->size.bytes,
					     file_name);

      mm_log ("UUENCODING complete", NIL, stream);

      body->encoding = ENCUUENCODE; /* We use UUENCODE rather than BASE64 */
      fs_give ((void **)&file_name);/* Toss temp for name storage */
      if (f) {
	  (*body->contents_deallocate)((void **)&f);	/* flush old binary contents */
	  body->contents_deallocate = fs_give;
      }
    default:			/* otherwise OK */
      break;
    }
    break;
  }
}

/*
 * Faire (to make) the standard sun attachment header */
void sun_faire_att_header (unsigned char *t, CORPS *seins)
{
  /* D'abord, des choses requeries - the required things */
  *t = NIL;
  sprintf ((char *)t, HEADER_SEC);         /* "----------\015\1012" */
  strcat((char *)t, seins->data_type);	   /* X-sun-data-type */
  strcat((char *)t, seins->content_length);/* X-sun-content-length */
  /*
   * Des autres .. the others */
  if (seins->encoding) 
    strcat((char *)t, seins->encoding);
  if (seins->data_name)
    strcat((char *)t, seins->data_name);
  if (seins->data_desc)
    strcat((char *)t, seins->data_desc);
  if (seins->text_type)
    strcat((char *)t, seins->text_type);
  strcat((char *)t, "\015\012");	/* La fin - the end */
}

/* Output sun attachment body
 * Accepts: body
 *	    I/O routine
 *	    stream for I/O routine
 * Returns: T if successful, NIL if failure
 */

long sun_output_body (ATTACHMENT *att, soutr_t f, TCPSTREAM *s, MAILSTREAM *ms)
{
  CORPS *seins;			/* An interesting body part :-) */
  unsigned char tmp[MAILTMPLEN];
  unsigned char *t;

  if (att->xsun_att) {
    ADAT *adata;

    /* X-Sun-attachement */
    seins = att->partie;	/* attachment parts */
    t = tmp;			/* our scratch buffer */
    while (seins) {		/* output each one.. */
      sun_faire_att_header (t, seins); /* attachement header */
      if (!(*f) (s, (char *)tmp, ms)) return NIL;  /* send it */
      /* 
       * Now output data */
      adata = &seins->corps;
      while (adata) {
	if (!(*f) (s, (char *)adata->data,ms)) return NIL;
	else
	  adata = adata->next;
      }
      seins = seins->next;	/* next attachement corps */
    } 
  } else {
    t = att->partie->corps.data; /* the TEXT body ... */
    if (!(*f) (s, (char *)t, ms)) return NIL;
  }
  return T;
}

/*
 * Make sun Content-type and Content-Length header lines */
void sun_att_header_lines (char **obuf, BODY *body, ATTACHMENT *att)
{
  int len= strlen(*obuf); 
  long body_len;

  *obuf += len;
  if (att->type_text) {		/* then just type TEXT */
    char *charset;

    strcpy (*obuf, "Content-Type: text\015\12");
    /* Need X-Sun-Text-Type: */
    strcat (*obuf, "X-Sun-Text-Type: ");
    if (!body || !body->parameter)
      charset = "ascii";
    else {			/* look for a "charset" param */
      PARAMETER *p= body->parameter;

      charset = "ascii";
      while (p) {
	if ((strcasecmp (p->attribute, "charset") == 0) &&
	    (strcasecmp (p->value, "us-ascii") != 0)) {
	  charset = p->value; 
	  break;
	} else
	  p = p->next;
      }
    }
    strcat (*obuf, charset);
    strcat (*obuf, "\015\12");
  } else
    strcat (*obuf, "Content-Type: X-sun-attachment\015\12");
  *obuf += strlen (*obuf);	/* end-of-buffer */    
  /*
   * Now the Content-Length field and header terminator "\012" */
  sprintf (*obuf, "Content-Length: %d\015\012\015\012", att->body_length);
}
