/*
 * Header file for sun attachments.
 *
 * Copyright (c) 1993-1997, SMC Inc, all rights reserved
 */
#include "UnixDefs.h"
typedef struct _attachment_data_ {
  unsigned char *data;		/* this data part */
  struct _attachment_data_ *next;
} ADAT;

typedef struct _att_corps_ {
  long corps_longeur;		/* length of this corps */
  char *data_type;		/* X-Sun-Data-Type: fou<CRLF> */
  char *content_length;		/* X-Sun-Content-Length: n<CRLF> */
  char *encoding;		/* X-Sun-Encoding-Info: folie<CRLF> */
  char *data_name;		/* X-Sun-Data-Name: nom<CRLF> */
  char *data_desc;		/* X-Sun-Data-Description: blague<CRLF> */
  char *text_type;		/* X-Sun-Text-Type: ascii<CRLF> */
  ADAT corps;			/* The body itself */
  struct _att_corps_ *next;
} CORPS;

typedef struct _attachement_ {
  long body_length;		/* number of bytes in the body (CRLF) */
  int xsun_att;			/* T if multipart */
  int type_text;		/* if Content-Type: TEXT */
  CORPS *partie;		/* If x-sun-attachment required */
} ATTACHMENT;

#define HEADER_SEC "----------\015\012"
#define HEADER_SEC_LEN 11       /* Bug fix: was 12 */
#define XSDTYPE "X-Sun-Data-Type: "
#define XSDTYPELEN (17+2)	/* length + CRLF */
#define XSCLENGTH "X-Sun-Content-Length: "
#define XSCLENLEN (22 +2)	/* length + CRLF */
#define XSTEXTTYPE "X-Sun-Charset: ascii\015\012"
#define X_SUN_ 6                /* len of "x-sun-" Ho Yeah! */
#define NUMBUFLEN 32		/* for calculation content length string */
#define PROBUFLEN 256		/* for building provisoire strings */

/* X-Sun-Data-Types ... */
#define XS_TYPEMESSAGE "mail-message"
#define XS_TYPETEXT    "text"
#define XS_TYPERICH    "richtext"
#define XS_OCTSTREAM   "default"
#define XS_POSTSCRIPT  "postscript-file"
#define XS_ODA         "oda"
#define XS_UNKNOWN     "default"
#define XS_AUDIO       "audio-file"
#define XS_GIF         "gif-file"
#define XS_JPEG        "jpeg"
#define XS_UNKNOWN_IMAGE "default"
#define XS_VIDEO       "video-file"
#define ENCUUENCODE    69   /* Special encoding for sun attachments */


long sun_att_output (char *t, ENVELOPE *env, BODY *body, soutr_t f,
		     TCPSTREAM *s, int return_receipt, HEADERLIST *header_list,
		     MAILSTREAM *ms);
long sun_output (char *t, ENVELOPE *env, BODY *body, soutr_t f, TCPSTREAM *s);
void sun_encode_body (ENVELOPE *env, BODY *body, MAILSTREAM *ms);
void sun_header (char *obuf, ENVELOPE *env, BODY *body, ATTACHMENT *att, 
		 int return_receipt, HEADERLIST *header_list);
void sun_make_att_hdrs (BODY *body, ATTACHMENT *att);
long sun_output_body (ATTACHMENT *att, soutr_t f, TCPSTREAM *s, MAILSTREAM *ms);
void make_sun_header_line (char **header, char *type, char *text);
void sun_att_header_lines (char **obuf, BODY *body, ATTACHMENT *att);
