/*
 * Header file for sun attachement to MIME translation */
#include "UnixDefs.h"
int sun_att_to_mime(FILECACHE **ofc, long *msglen, int keep_mime);
void init_mime_trans(SOLARISBODY *corps);
void initialize_transbuf(void);
int parse_sun_attachment(FILECACHE *fc);

#define MORCEAU 8196
#define CTYPEBUF 256
#define XSUNATTLEN 16
#define MAXBOUNDARY 69
/* Content-Type: 
 * 1234567890123 */
#define CONTYPELEN 13
/* Content-Length:
 * 123456789012345 */
#define CONTENTLENLEN 15
#define TEXTLEN 4		/* "Text" */
/* X-Sun-Text-Type: */
/* 1234567890123456 */
#define STEXTTYPELEN 16

#define MIMEVERSION "Mime-Version: 1.0\n"
#define TEXTPLAIN "Content-Type: text/plain; charset=us-ascii\n"
#define MTPHDR "Mime-Version: 1.0\nContent-Type: text/plain; charset=us-ascii\n"
#define MTPHDR1 "Mime-Version: 1.0\nContent-Type: text/plain; charset="

#define MULTIPARTMIXED "Content-Type: multipart/mixed;boundary="
#define BOUNDARY "73-rue-MF+40-rue-chapon+VF+"
/*                123456789022345678903334567 37 chars */
#define DELIMITER "\n--"

/* X sun attachment stuff */
#define SECTION "----------\n"
#define SECTIONLEN 11






