#include "UnixDefs.h"

#define READBUFLEN 1024
static FILE *checkpoint_spaceok(MAILSTREAM *stream, char *cpfile);
static char *checkpoint_filename(MAILSTREAM *stream, char *buf, int len);
static int checkpoint_cache(MAILSTREAM *stream, FILE *f);
static int make_checkpoint_name(char *mbox, char *buf, int blen, MAILSTREAM *s);
MAILSTREAM *cp_restore_stream(FILE *f, int *erro);

unsigned long checkpoint_mcache_size(MESSAGECACHE *elt);
unsigned long checkpoint_lelt_size(LONGCACHE *lelt);
unsigned long checkpoint_body_size(BODY *body, unsigned long *size);
unsigned long checkpoint_env_size(ENVELOPE *env);
unsigned long checkpoint_string_size(char *str);
unsigned long checkpoint_address_size(ADDRESS *address);
unsigned long checkpoint_parameter_size(PARAMETER *p);
int checkpoint_write_caches(FILE *f,MAILSTREAM *stream);
int checkpoint_write_stream(FILE *f,MAILSTREAM *stream);
int cp_write_sinfo(SHORTINFO *sinfo,FILE *f);
int cp_write_parlist(FILE *f,PARAMETER *p);
int cp_write_parts(MAILSTREAM *stream,FILE *f,PART *p,char *seq);
int cp_write_body(MAILSTREAM *stream,BODY *b, FILE *f);
int cp_write_bodylist(MAILSTREAM *stream,BODY *b, FILE *f);
int cp_write_address(FILE *f,ADDRESS *a);
int cp_write_env(ENVELOPE *env, FILE *f);
int cp_write_uflags(FILE *f, char **user_flags);
int cp_write_string(FILE *f, char *str);
int cp_write_ulong(FILE *f, unsigned long i);
int cp_write_int(FILE *f, int i);
int cp_write_data(MAILSTREAM *stream,FILE *f,unsigned char *data,
		  unsigned long size);
int cp_nil_elt(MESSAGECACHE *elt);
int cp_write_elt(MAILSTREAM *stream,MESSAGECACHE *elt, FILE *f);
int cp_write_lelt(MAILSTREAM *stream,LONGCACHE *lelt, FILE *f);
int cp_write_end(FILE *f);
int cp_read_int(FILE *f, int *i);
int cp_read_short(FILE *f,unsigned short *i);
int cp_read_ushort(FILE *f,unsigned short *i);
int cp_ulong_int(FILE *f, int *i);
int cp_read_ulong(FILE *f, unsigned long *i);
int cp_read_string(FILE *f, char **str);
int cp_read_data(FILE *f,char **ibuf,unsigned long *size);
int cp_read_uflags(FILE *f, char **uflags);
static int cp_rebuild_elt(FILE *f, MAILSTREAM *stream, unsigned long msgno);
int cp_rebuild_lelt(FILE *f, MAILSTREAM *stream, unsigned long msgno);
int cp_read_lelt(FILE *f, LONGCACHE *lelt,unsigned long msgno);
int cp_read_env(FILE *f,ENVELOPE **env);
int cp_read_address(FILE *f, ADDRESS **adr);
int cp_read_bodylist(FILE *f, BODY **b);
int cp_read_sinfo(FILE *f,SHORTINFO *sinfo);
int cp_copy_string(FILE *f, char *buf, char **str);
static void cp_free_stream(MAILSTREAM *stream);
int cp_read_body (FILE *f,BODY **b);
int cp_read_body_ptr (FILE *f,BODY *b);
int cp_read_parts(FILE *f,PART **p);
int cp_read_parlist(FILE *f,PARAMETER **p);

int checksum_cache(FILE *f);
void cp_checksum_cache(FILE *f, unsigned long *size, 
		       unsigned short *cksum);
int verify_cache_checksum(FILE *f,unsigned long *csize);

#define STREAMDESC "STREAM\n"
#define NULLSTRING "#NULLSTRING#\n"	/* NIL string */
#define STREAMEND  "#ENDSTREAM#\n"	/* end of stream */
#define ENVEND     "#ENDENV#\n"		/* end of envelope */
#define LELTEND    "#ENDLELT#\n"	/* eof of long element */
#define BODYEND    "#LE_CUL#\n"		/* body end */
#define NILELT  "NILELT=%lu\n"		/* NILELT=msgno */
#define NILENV  "NILENV\n"		/* NIL envelope */
#define NILBODY "NILBODY\n"		/* NIL body */
#define NILPART "NILPART\n"		/* END of part list */
#define ELT     "ELT=%lu\n"		/* ELT=msgno */
#define BODYDESC "BODYDESC\n"		/* Non NIL body description */
#define BODYMARK "BODY\n"               /* Marks start of body */
#define ADDR    "A=%d\n"		/* ADDR=naddresses */
#define DATABLK "DATA=%lu\n"		/* DATA=nbytes */
#define PARTDESC "PART=%s\n"		/* multipart part description */
#define CACHEEND "#ENDCACHE#\n"		/* end of cache */
#define CACHEPREFIX "CACHED="

#define ISINT    "I="
#define ISUSHORT "US="
#define ISULONG  "U="
#define ISSTR    "S="
#define ISDATA   "D="
#define ISLIST   "L="
#define ISADDR   "A="
#define ISENV    "ENV\n"
#define ISPLIST  "PLIST="
#define ISPARTD  "PART="
#define DDLEN 2
#define USLEN 3
#define PLLEN 6
#define NLEN  7
#define ELEN  4
#define NSLEN 12
#define PDLEN 5
#define INTLEN 16

typedef struct _ck_shorts_ {
  unsigned short carry, sum;
} CKSHORTS;

/* cache checksum */
#define CARRY cache_check_sum.ck_s.carry
#define CHECKSUM cache_check_sum.ck_s.sum
#define TRENTE_DEUX cache_check_sum.l
