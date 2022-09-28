/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $XConsortium: isam_impl.h /main/cde1_maint/2 1995/10/07 19:12:06 pascale $ 			 				 */
/* @(#)isam_impl.h	1.11 93/09/07 */

/*
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */

/*
 * isam_impl.h
 *
 * Description:
 *	NetISAM implementation specific definitions
 *
 */

#ifndef _ISAM_IMPL_H
#define _ISAM_IMPL_H

#include <assert.h> 
#include <stdio.h>
#include <fcntl.h>
#include <setjmp.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/param.h>
/* AIX does not define FD_SETSIZE in sys/types.h. */
#if !defined(FD_SETSIZE)
#include <sys/select.h>
#endif

/*
 * #include sys/types.h is to get u_long, u_short, dev_t
 *
 * #include sys/param.h is to get MAXHOSTNAMELEN
 */

#include "isam.h"

extern int isdupl;

typedef long Recno;			     /* record numbers */
typedef long Blkno;			     /* block numbers */
typedef long Time;			     /* standard UNIX time */
typedef int  Isfd;			     /* ISAM file descriptor */

typedef int  Bool;
#ifndef FALSE
#define FALSE	0
#define TRUE	1
#endif

#define ISOPENMODE	0xff		     /* Mask to get ISINPUT/ISOUTPUT */
					     /* or ISINOUT */
#define ISLOCKMODE	0xff00		     /* Mask to get lock mode */
#define ISREADMODE	0xff		     /* Mask to get ISFIRST, etc. */

#define ISLENMODE	0x10000		     /* Mask to get FIX/VAR length */

#define ISPAGESIZE	1024		     /* page size */

#define N_CNTL_PAGES	2		     /* 
					      * Number of control pages 
					      * at the beg. of .rec file 
					      */
#define ISCNTLSIZE	(ISPAGESIZE * N_CNTL_PAGES)


#define ISMAGIC		"NetISAM"	     /* 'magic number' in ISAM files */

#ifndef ISVERSION
#define ISVERSION	"Unknown"
#endif

#define MAXFCB_UNIXFD	30		     /* Maximum number of UNIX fd
					      * used by the FCB module
					      */

#define ISLEAFSLACK	10		     /* Default slack in leaves */

/* Default RPC timeout values */
#define ISRPCS_TO_DEF	60		     /* secs for short oper. */
#define ISRPCL_TO_DEF  (120 * 60)	     /* secs for long oper. */
#define ISTCP_TO_DEF   (3 * 60)		     /* secs to reconnect TCP/IP */
#define ISRPC_POLL_TO_DEF 30		     /* interval to hear from polled client */
#define ISTCP_RETRY_SLEEP 5		     /* time between tries to
					      * establish a connection 
					      */

/* 
 * The X/OPEN ISAM internal data representations.
 * The #define statements are used to declare type lengths and offsets
 * in structures in order for the definitions to be compiler independent.
 */

#define TIMETYPE	LONGTYPE	     /* standard UNIX time */
#define TIMESIZE	LONGSIZE	

#define BLKNOTYPE	LONGTYPE	     /* block (page) numbers */
#define BLKNOSIZE	LONGSIZE

#define RECNOTYPE	LONGTYPE	     /* record numbers */
#define RECNOSIZE	LONGSIZE

/* 
 * SHORTTYPE is a safeguard - if the X/OPEN ISAM changes INTTYPE to 4 bytes 
 * all the internals of NetISAM will work. The only change will be
 * re-defining SHORTTYPE.	
 */

#define SHORTTYPE	INTTYPE		     
#define SHORTSIZE	INTSIZE

/* The following defines are used in isstart() */
#define WHOLEKEY	0
#define USE_PHYS_ORDER(keydesc)  (keydesc->k_nparts == 0)

/* Structure for storing and passing variable length byte array data */
typedef struct bytearray {
    u_short	length;
    char	*data;
} Bytearray;


/* Client identification - used to identify owners of locks */
/*
   #define IPADDRLEN	4
*/

/* Definitions related to ISAM file descriptor (isfd.c file) */
#define MAXISFD		FD_SETSIZE
#define NOISFD		(-1)		     /* Not a valid file descriptor */


/* Definitions related to File access block (isfab.c file */

/* File open mode */
enum openmode  { OM_INPUT = 0, OM_OUTPUT = 1, OM_INOUT = 2, OM_BADMODE = 3};
enum lockmode  { LM_FAST = 0, LM_EXCL = 1, LM_AUTOMATIC = 2, LM_MANUAL = 3, LM_BADMODE = 4}; 

/* isread() read modes */
enum readmode { RM_FIRST = 0, RM_LAST = 1, RM_NEXT = 2, RM_PREV = 3, 
		    RM_CURR = 4, RM_EQUAL = 5, RM_GREAT = 6, RM_GTEQ = 7,
		    RM_LESS = 8, RM_LTEQ = 9, RM_BADMODE = 10 };

/* lock flag is passed to Access Method module for every record oriented op. */
/* Bit position in the lock flag: */

enum openmode _getopenmode();
enum readmode _getreadmode();

/* Error code structure */
struct errcode {
    int		iserrno;
    char	isstat [4];
};

typedef struct fab {
    Isfd		isfd;		     /* ISAM file descriptor */
    enum openmode 	openmode;	     /* File access mode */
    enum lockmode	lockmode;	     /* File lock mode */
    Bool		locked;		     /* TRUE if file is locked */
    Bool		varlength;	     /* TRUE if variable length records
					      * supported by this ISAM file */
    int			minreclen;	     /* Minimum record length */
    int			maxreclen;	     /* Maximum record length */
    char		*isamhost;	     /* IP host name */
    char		*isfname;	     /* ISAM file local pathname */
    Bytearray		isfhandle;	     /* ISAM file handle */
    Bytearray		curpos;		     /* Current record position */
    struct errcode	errcode;	     /* Error codes structure */
} Fab;

/* values for filemode */
#define LOCAL_FILE	0
#define REMOTE_FILE	1
#define NFS_FILE	2		     /* File is remote, but is
					      * accessed via NFS
					      */

#define FAB_ISFDSET(fab, isfd) (fab->isfd = isfd)


/* ISAM file identification for locking purposes. */
typedef struct lckfid {
    dev_t		diskid;		     /* Disk device id */
    ino_t		inode;		     /* .rec file inode number */
} Lckfid; 

/* File Control Block */
typedef struct fcb {
    char		*isfname;	     /* ISAM file name */
    int			blocksize;	     /* Block size */
    Bool		rdonly;		     /* file is read-only */
    Lckfid		lckfid;		     /* File id for locking */
    int			datfd;		     /* UNIX file descr. of .rec file */
    int			indfd;		     /* UNIX file descr. of .ind file */
    int			varfd;		     /* UNIX file descr. of .var file */
    Blkno		datsize;	     /* dat file size in blocks */
    Blkno		indsize;	     /* ind file size in blocks */
    Blkno		varsize;	     /* var file size in blocks */
    Bool		varflag;	     /* TRUE if variable length*/
    long		nrecords;	     /* Number of records */
    int			minreclen;	     /* Minimum record length */
    int			maxreclen;	     /* Maximum record length */
    Recno		lastrecno;	     /* Last recno in use */
    Recno		freerecno;	     /* Pointer to first free record */
    int			lastkeyid;	     /* Last key identifier used */
    int			changestamp1;	     /* Stamp 1 of last change */
    int			changestamp2;	     /* Stamp 2 of last change */
    Blkno		indfreelist;	     /* Head of freepage list of .ind */

    int			nkeys;		     /* Number of keys */
    struct keydesc2	*keys;		     /* Key descriptors */
    long		varend;		     /* Offset of the last byte */
					     /* in .var file */
    int                 lockfd;        /* lock file fd for locking .rec for 5.0 */
} Fcb;

#define FCB_NOPRIMARY_KEY(fcb) ((fcb)->keys[0].k2_nparts == 0)

/* Current Record Position (interpreted only by Access Method) */
enum crpflag { CRP_UNDEF = 0, CRP_ON = 1, CRP_AFTER = 2, CRP_BEFORE = 3,
	       CRP_BEFOREANY = 4, CRP_AFTERANY = 5};
typedef struct crp {
    int			keyid;		     /* Key index */
    enum crpflag	flag;		     /* ON/BEFORE/AFTER/UNDEF */
    Recno		recno;		     /* Record number */
    int			matchkeylen;	     /* match so many bytes of key */
    char		key[1];		     /* Index position */
} Crp;

#define PHYS_ORDER   (-1)		     /* value for keyid when physical
					      order is used*/

/* 
 * keydesc2 is the 'internal copy'of keydesc. 
 * keydesc2 contains all information needed by internally by NetISAM,
 * whereas keydesc conforms to the X/OPEN ISAM. 
 */

#define NPARTS2 (NPARTS+2)		     /* duplid and recno */
#define DUPSMASK   001			     /* Mask to get ISDUPS/ISNODUPS */
#define ISPRIMKEY	040		     /* this is primary key */
#define ALLOWS_DUPS2(pkdesc2) (((pkdesc2)->k2_flags & DUPSMASK) == ISDUPS)
#define ALLOWS_DUPS(pkdesc) (((pkdesc)->k_flags & DUPSMASK) == ISDUPS)

/* serial number of duplicate keys */
#define DUPIDTYPE	LONGTYPE
#define DUPIDSIZE	LONGSIZE

#define KEY_RECNO_OFF	0
#define KEY_DUPS_OFF	RECNOSIZE

#define stdupser(n, b) stlong((long)(n), (b))
#define lddupser(b)  ((int)ldlong(b))

struct keypart2 {
    u_short 	kp2_start;		     /* starting byte of key part */
    short	kp2_leng;		     /* length in bytes */
    short	kp2_type;		     /* type of key part */
    u_short	kp2_offset;		     /* offset in key buffer */
};

typedef struct keydesc2 {
    short 	k2_flags;		     /* flags */
    short	k2_nparts;		     /* number of parts in key */
    short	k2_len;			     /* length of the whole key */
    Blkno	k2_rootnode;		     /* pointer to root node */
    int		k2_keyid;		     /* Key identifier */
    struct keypart2 k2_part[NPARTS2];	     /* each part */
} Keydesc2;

typedef unsigned long rel_addr;
/* Double linked list element */
struct dlink {
    rel_addr	 dln_forward;		     /* Forward link */
    rel_addr	 dln_backward;		     /* Backward link */
};

/* Disk buffer management definitions */

/* cache buffer header */
typedef struct bufhdr {
    Fcb		*isb_fcb;		     /* Pointer to FCB */
    int		isb_unixfd;		     /* UNIX file descriptor */
    Blkno	isb_blkno;		     /* block number */
    struct dlink isb_hash;		     /* hashed list*/
    struct dlink isb_aclist;		     /* available or changed list */
    struct dlink isb_flist;		     /* list of fixed blocks */
    char	*isb_buf_w;		     /* malloc() buffer with dirty data */
    char	*isb_buf_r;		     /* points to mapped segment */
    char	*isb_buffer;		     /* set to isb_buf_r or isb_buf_w */
    char	isb_flags;		     /* flags - defs. see below */
    struct bufhdr *isb_oldcopy;		     /* pointer to old copy */
} Bufhdr;

/* mapped segment header */
typedef struct maphdr {
    Fcb		*m_fcb;			     /* Pointer to FCB */
    int		m_unixfd;		     /* UNIX file descriptor */
    char       	*m_addr;		     /* Pointer to beginning of seg. */
    int		m_segm_num;		     /* Segment number in the file */
    unsigned long m_stamp;		     /* Assigned when touched */
} Maphdr;


/* values of isb_flags */
#define ISB_NODATA	00		     /* block has no data */
#define ISB_READ	01		     /* page has valid data */
#define ISB_CHANGE	02		     /* page must be flushed */
#define ISB_RFIXED	04		     /* block is fixed for read*/
#define ISB_WFIXED     010		     /* block is fixed for write*/
#define ISB_OLDCOPY    020		     /* block is old copy */

/* mode values to is__cache_fix() */
#define ISFIXREAD	1		     /* fix for read */
#define ISFIXWRITE	2		     /* fix for update */
#define ISFIXNOREAD	3		     /* fix for update, don't read */


/* 
 * Macro to get pointer to structure if pointer to some element is known 
 */
#define GETBASE(p,s,e) ((struct s *) ((char *)(p) - (int)&((struct s *)0)->e))

/* In memory sorting object */
typedef struct issort {
    int		ist_reclength;		     /* record length in bytes */
    int		ist_allocrecs;		     /* memory allocated for so */
					     /* many records */
    int		ist_nrecs;		     /* number of records inserted */
    int		ist_currec;		     /* current position */
    int		(*ist_compf) ();	     /* comparison function */
    char	*ist_array;		     /* array of records */
} Issort;


/* btree is object used for processing B-tree operations */

#define ISMAXBTRLEVEL	12		     /* Maximum level of B-tree */

typedef struct btree {
    Fcb		*fcb;
    Keydesc2	*keydesc2;
    int		depth;			     /* depth of the B-tree */
    Bufhdr 	*bufhdr[ISMAXBTRLEVEL];	     /* fixed blocks buf. headers */
    int		curpos[ISMAXBTRLEVEL];	     /* current position on block */
} Btree;

/* Lock manager related definitions. */
/* Entry in lock table */

/* Number of buckets for hashing by clientid. Must be a power of 2*/
#ifndef HASHPROCSIZE
#define HASHPROCSIZE	64
#endif

/* 
 * Number of buckets for hashing by record number and fileid.
 * Must be a power of 2 
 */
#ifndef HASHENTSIZE
#define HASHENTSIZE	1024
#endif

/* Pseudo record numbers used by file locking. */
#define AVAIL_RECNO	(0L)		     /* Entry is available */
#define ALLFILE_RECNO	(-1L)		     /* Entire file lock */
#define	OPENFILE_RECNO	(-2L)		     /* Used by isopen() */

/* Lock table file header */
struct lockfilehdr {
    int			size;		     /* size of LOCKTABLEFILE 
					      * set to 0 forces reinitialization
					      * at next lock request.
					      */
    int			prochdsn;	     /* Number of lists hashed by 
					      *	clientid */
    int			enthdsn;	     /* Number of list hashed by
					      * record number */
    int			lockentriesn;	     /* total number of lock entries */
    struct dlink	avail;		     /* Double linked list of 
					      * available lock entries */
};

/* Exported filesystem options structure and constants*/
 
#define RW_CLIENTS_SIZE 1     /* Not currently needed */
#define UID_SIZE 10           /* Big enough to hold a Unix UID */
#define ROOT_OPT_SIZE 240     /* List of hosts for which root access
                                 is allowed -- can be quite long */
#define SHARE_BUFFER_SIZE 400
 
struct export_opts {
  int read_write; /* 0 => read-only, 1 => read/write */
  char rw_clients[1]; /* Not interesting, len=1 to save space */
  char export_dir[1]; /* ditto here */
  char anon[UID_SIZE];
  char root[ROOT_OPT_SIZE];
  int secure; /* RPC authentication flag: 0 => AUTH_UNIX, 1 => AUTH_DES */
};

#define DAT_SUFFIX	".rec"
#define IND_SUFFIX	".ind"
#define VAR_SUFFIX	".var"
#define LOCK_SUFFIX     ".lock"         /* yh: lock file for .rec to solve mmap in 5.0 */

/* Values of deleted flag for fixed length records */
#define FL_RECDELETED	0
#define FL_RECEXISTS    1

/* Special values of tail pointer for variable length records */
#define VL_RECDELETED	((long ) -1L)
#define VL_RECNOTAIL	((long ) -2L)


/* `C' prototypes  for  isam function calls */

extern int _add1key(Fcb *, Keydesc2 *, char *, Recno, char *);

extern int isaddindex(int, struct keydesc *);

extern int isaddprimary(int, struct keydesc *);

extern char *_ismalloc(unsigned int), *_isrealloc(char *, unsigned int), *_isallocstring(char  *);

extern void _isfreestring(char *);

extern int _iskeycmp(char *, char *);

extern void _iskeycmp_set(Keydesc2 *, int);

extern int _amaddindex(Bytearray *, struct keydesc *, struct errcode *);

extern int _amaddprimary(Bytearray *, struct keydesc *, struct errcode *);

extern int _create_index(Fcb *, Keydesc2 *);

extern void _delkeys(register Fcb *, char *, Recno);

extern int _amdelrec(Bytearray *, Recno, struct errcode *);

extern void _amseterrcode(struct errcode *, int);

extern void _isam_entryhook(), _isam_exithook();

extern int _amopen(char *, enum openmode, Bool *, int *, int *, Bytearray *, Bytearray *, struct errcode *);

extern Fcb *_openfcb(Bytearray *, struct errcode *);

extern char *_getisfname(Bytearray *);

extern Bytearray _makeisfhandle(char *);

extern int _amwrite(Bytearray *, char *, int , Bytearray *, Recno *, struct errcode *);

extern int _addkeys(register Fcb *, char *, Recno);

extern int _isapplmr(int, char *);

extern int _isapplmw(int, char *);

extern char *_isbsearch(char *, char *, int, int, int (*cmpf) ());

extern Btree * _isbtree_create(Fcb *, Keydesc2 *);

extern void _isbtree_destroy(register Btree *);

extern void _isbtree_search(register Btree *, char *);

extern char *_isbtree_current(register Btree *);

extern char * _isbtree_next(register Btree *);

extern void _isbtree_insert(register Btree *, char *);

extern void leftkey_up(register Btree *, int);

extern void splitblock(register Btree *, register char *, register char *, int);

extern void move_from_right(Btree *, char *, char *, int);

extern void move_from_left(Btree *, char *, char *, int);

extern int isbuild(char *, int, struct keydesc *, int);

extern Bytearray _bytearr_new(u_short, char *);

extern Bytearray _bytearr_dup(Bytearray *);

extern Bytearray _bytearr_getempty(void);

extern void _bytearr_free(register Bytearray *);

extern int _bytearr_cmp(register Bytearray *, register Bytearray *);

extern int _change1key(Fcb *, Keydesc2 *, char *, char *, Recno, char *);

extern int isclose(int);

extern int iscntl(int, int, ...);

extern int isgetcurpos(int, int *, char **);

extern int issetcurpos(int, char *);

extern long ldlong(register char *);

extern void stlong(register long, register char *);

extern short ldint(register char *);

extern u_short ldunshort(register char *);

extern void stint(register short, register char *);

extern float ldfloat(register char *);

extern void stfloat(float, register char *);

extern double lddbl(register char *);

extern void stdbl(double, register char *);

extern double lddbl(register char *);

extern void stdbl(double, register char *);

extern void _del1key(Fcb *, Keydesc2 *, char *, Recno);

extern int isdelcurr(int);

extern int isdelrec(int, long);

extern Bufhdr *_isdisk_fix(Fcb *, int, Blkno, int);

extern Bufhdr  *_isdisk_refix();

extern void _isdisk_unfix (register Bufhdr *);

extern void _isdisk_commit1 (Bufhdr *);

extern void _isdisk_commit(void);

extern void _isdisk_rollback(void);

extern Bufhdr *_isdisk_refix(Bufhdr *, int);

extern void _isdisk_sync(void);

extern void _isdisk_inval(void);

#if ISDEBUG
extern void _isdisk_dumphd(void);
extern void aclistdump(struct dlink *);
extern void flistdump(struct dlink *);
#endif

extern void _isdln_base_insert(register char *, register struct dlink *, register struct dlink *);

extern void _isdln_base_append(register char *, register struct dlink *, register struct dlink *);

extern void _isdln_base_remove(register char *, register struct dlink *);

extern struct dlink * _isdln_base_first(register char *, register struct dlink *);

extern struct dlink * _isdln_base_next(register char *, register struct dlink *);

extern struct dlink * _isdln_base_prev(register char *, register struct dlink *);

extern void _isdln_base_makeempty(register char *, register struct dlink *);

extern int _isdln_base_isempty(register char *, register struct dlink *);

extern int iserase(char *);

extern void _isfatal_error(char *, char *);

extern void _isam_warning(char *);

extern void _setiserrno2(int, int, int);

extern void _seterr_errcode(register struct errcode *);

extern Fab *_fab_new(char *, enum openmode, Bool, int, int);

extern void _fab_destroy(register Fab *);

extern Fcb *_isfcb_create(char *, int, int, int, uid_t, gid_t, mode_t, struct errcode *);

extern void _isfcb_setreclength(register Fcb *, Bool, int, int);

extern Fcb *_isfcb_open(char *, struct errcode *);

extern int _isfcb_nfds(register Fcb *);

extern void _isfcb_remove(register Fcb *);

extern void _isfcb_close(register Fcb *);

extern int _isfcb_cntlpg_w(register Fcb *);

extern int _isfcb_cntlpg_w2(register Fcb *);

extern int _isfcb_cntlpg_r(register Fcb *);

extern int _isfcb_cntlpg_r2(register Fcb *);

extern void _isfcb_setprimkey();

extern int _open2_indfile(Fcb *);

extern int _isfcb_primkeyadd(Fcb *, Keydesc2 *);

extern int _isfcb_primkeydel(Fcb *);

extern int _isfcb_altkeyadd(Fcb *, Keydesc2 *);

extern Keydesc2 * _isfcb_findkey(register Fcb *, Keydesc2 *);

extern int _isfcb_altkeydel(register Fcb *, Keydesc2 *);

extern Keydesc2 * _isfcb_indfindkey(register Fcb *, int);

extern int _watchfd_incr(int);

extern int _watchfd_decr(int);

extern int _watchfd_check(void);

extern int _watchfd_max_set(int);

extern int _watchfd_max_get(void);

extern Isfd _isfd_insert(Fab *);

extern Fab *_isfd_find(register Isfd);

extern void _isfd_delete(register Isfd);

extern void _cp_tofile(Fcb *, int, char *, long, int);

extern void _cp_fromfile(Fcb *, int, char *, long, int);

extern Blkno _extend_file(Fcb *, int, Blkno);

extern int _flrec_write(register Fcb *, char *, Recno *, int);

extern int _flrec_read(register Fcb *, char *, Recno, int *);

extern long _fl_getpos(Fcb *, Recno);

extern int _flrec_rewrite(register Fcb *, char *, Recno, int);

extern int _flrec_delete(register Fcb *, Recno);

extern int _flrec_wrrec(register Fcb *, char *, Recno, int);

extern void _makedat_isfname(char *);

extern void _makeind_isfname(char *);

extern void _makevar_isfname(char *);

extern void _removelast(char *);

extern char * _lastelement(char *);

extern int isgarbage(char *);

extern void stkey(Keydesc2 *, char *);

extern void ldkey(register struct keydesc2 *, register char *);

extern  int isindexinfo(int, struct keydesc *, int);

extern Blkno _isindfreel_alloc(Fcb *);

extern void _isindfreel_free(Fcb *, Blkno);

extern void _iskey_extract(Keydesc2 *, char *, char *);

extern Bufhdr *_allockpage(Fcb *, int, int, Blkno *);

extern int getkeysperleaf (int);

extern int getkeyspernode (int);

extern int le_odd(int);

extern void _iskeycmp_set (Keydesc2 *, int);

extern int _iskeycmp(char *, char *);

extern int _validate_keydesc(register struct keydesc *, int);

extern void _iskey_fillmax(struct keydesc2 *, register char *);

extern void _iskey_fillmin(struct keydesc2 *, register char *);

extern void _mngfcb_insert(Fcb *, Bytearray *);

extern Fcb * _mngfcb_find(Bytearray *);

extern void _mngfcb_delete(Bytearray *);

extern Bytearray * _mngfcb_victim(void);

extern int isopen(char *, int);

extern void _isseekpg(int, Blkno);

extern void _isreadpg(int, char *);

extern void _iswritepg(int, char *);

extern enum openmode _getopenmode(int);

extern enum readmode _getreadmode(int);

extern int isread(int, char *, int);

extern int isrename(char *, char *);

extern int isrepair(char *, int);

extern int isrewcurr(int, char *);

extern int isrewrec(int, long, char *);

extern int _issignals_cntl(int);

extern void _issignals_mask(void);

extern void _issignals_unmask(void);

extern Issort * _issort_create(int, int, int (*compfunc)());

extern void _issort_destroy(Issort *);

extern void _issort_insert(register Issort *, char *);

extern void _issort_sort(Issort *);

extern void _issort_rewind(Issort *);

extern char * _issort_read(register Issort *);

extern int isstart(int, struct keydesc *, int, char *, int);

extern int issync(void);

extern int isfsync(int);

extern int _issync(void);

extern int _isfsync(int);

extern int _vlrec_write(register Fcb *, char *, Recno *, int);

extern int _vlrec_read(register Fcb *, char *, Recno, int *);

extern long _vl_getpos(Fcb *, Recno);

extern int _vlrec_rewrite(register Fcb *, char *, Recno, int);

extern int _vlrec_delete(register Fcb *, Recno);

extern int _vlrec_wrrec(register Fcb *, char *, Recno, int);

extern long _istail_insert(Fcb *, char *, int);

extern int _istail_modify(Fcb *, long, char *, int);

extern int iswrite(int, char *);

extern int iswrrec(int, long, char *);

extern void _iskey_itox(register struct keydesc2 *, register struct keydesc *);

extern void _iskey_xtoi(register struct keydesc2 *, register struct keydesc *);

extern int _check_isam_magic(Fcb *);

extern void _isbtree_remove(register Btree *);


#define stshort(n,p) stint((n), (p))
#define ldshort(p) ldint(p)

#define strecno(n,p) stlong((long)(n), (p))
#define ldrecno(p) ((Recno)ldlong(p))

#define stblkno(n,p) stlong((long)(n), (p))
#define ldblkno(p) ((Blkno)ldlong(p))


/*------------ UNIX file formats ---------------------------------------------*/

#define ISCNTLPGOFF	((Blkno) 0)	     /* offset of Control Page */

/* internal key descriptor */

#define KP2_START_OFF	0		     /* see struct keypart2 */
#define KP2_LENGTH_OFF	SHORTSIZE
#define KP2_TYPE_OFF	(2*SHORTSIZE)
#define KP2_OFFSET_OFF	(3*SHORTSIZE)
#define KP2_LEN		(4*SHORTSIZE)

#define K2_FLAGS_OFF	0		     /* see struct keydesc2 */
#define K2_NPARTS_OFF	SHORTSIZE	
#define K2_LEN_OFF	(2*SHORTSIZE)
#define K2_ROOT_OFF	(3*SHORTSIZE)
#define K2_KEYID_OFF    (3*SHORTSIZE+BLKNOSIZE)
#define K2_KEYPART_OFF	(3*SHORTSIZE+2*BLKNOSIZE)
#define K2_LEN		(3*SHORTSIZE+2*BLKNOSIZE+ NPARTS2*KP2_LEN)

#define NULL_BLKNO	0L
#define NULL_RECNO	0L
#define PAGE1_BLKNO	0		     /* Block number of control page */
/*------------------------- Control Page  layout -----------------------------*/

/* Magic number (must be equal to "NetISAM") */
#define CP_MAGIC_OFF	0
#define CP_MAGIC_LEN	8

/* Version stamp */
#define CP_VERSION_OFF (CP_MAGIC_OFF+CP_MAGIC_LEN)
#define CP_VERSION_LEN 8

/* Application stamp */
#define CP_APPLMAGIC_OFF (CP_VERSION_OFF+ CP_VERSION_LEN)
#define CP_APPLMAGIC_LEN ISAPPLMAGICLEN

/* Block size */
#define CP_BLOCKSIZE_OFF (CP_APPLMAGIC_OFF+CP_APPLMAGIC_LEN)
#define CP_BLOCKSIZE_LEN SHORTSIZE

/* file size in blocks */
#define CP_DATSIZE_OFF	(CP_BLOCKSIZE_OFF+CP_BLOCKSIZE_LEN)	
#define CP_DATSIZE_LEN	BLKNOSIZE

/* file size in blocks */
#define CP_INDSIZE_OFF	(CP_DATSIZE_OFF+CP_DATSIZE_LEN)	
#define CP_INDSIZE_LEN	BLKNOSIZE

/* file size in blocks */
#define CP_VARSIZE_OFF	(CP_INDSIZE_OFF+CP_INDSIZE_LEN)
#define CP_VARSIZE_LEN	BLKNOSIZE

/* 0/1 flag telling whether file supports variable length records. */
#define CP_VARFLAG_OFF	(CP_VARSIZE_OFF+CP_VARSIZE_LEN)
#define CP_VARFLAG_LEN	SHORTSIZE

/* number of record in the file */
#define CP_NRECORDS_OFF (CP_VARFLAG_OFF+CP_VARFLAG_LEN)
#define CP_NRECORDS_LEN LONGSIZE

/*  minimum record length */
#define CP_MINRECLEN_OFF (CP_NRECORDS_OFF+CP_NRECORDS_LEN)
#define CP_MINRECLEN_LEN  SHORTSIZE

/*  maximum record length */
#define CP_MAXRECLEN_OFF (CP_MINRECLEN_OFF+CP_MINRECLEN_LEN)
#define CP_MAXRECLEN_LEN  SHORTSIZE

/* 
 * Left 0 and not used in NetISAM 1.0.
 * Will be set at restructuring to some optimal value x 
 * (minreclen <= x <= maxreclen).
 * Record with length will be stored in .rec file only. 
 * In NetISAM 1.0  x == minreclen.
 */
/*  split record record length */
#define CP_SPLITRECLEN_OFF (CP_MAXRECLEN_OFF+CP_MAXRECLEN_LEN)
#define CP_SPLITRECLEN_LEN  SHORTSIZE

/* Recno of last record */
#define CP_LASTRECNO_OFF  (CP_SPLITRECLEN_OFF+CP_SPLITRECLEN_LEN)
#define CP_LASTRECNO_LEN  RECNOSIZE

/* Head of deleted records list */
#define CP_FREERECNO_OFF  (CP_LASTRECNO_OFF+CP_LASTRECNO_LEN)
#define CP_FREERECNO_LEN  RECNOSIZE

/* 0/1 flag telling whether file has a primary key */
#define CP_HASPRIMKEY_OFF (CP_FREERECNO_OFF+CP_FREERECNO_LEN)
#define CP_HASPRIMKEY_LEN SHORTSIZE

/* Last key indentifier. */
#define CP_LASTKEYID_OFF (CP_HASPRIMKEY_OFF+CP_HASPRIMKEY_LEN)
#define CP_LASTKEYID_LEN  LONGSIZE	

/* Stamp 1 of last change */
#define CP_CHANGESTAMP1_OFF (CP_LASTKEYID_OFF+CP_LASTKEYID_LEN)
#define CP_CHANGESTAMP1_LEN  LONGSIZE	

/* Stamp 2 of last change */
#define CP_CHANGESTAMP2_OFF (CP_CHANGESTAMP1_OFF+CP_CHANGESTAMP1_LEN)
#define CP_CHANGESTAMP2_LEN  LONGSIZE	

/* .ind file free list */
#define CP_INDFREELIST_OFF (CP_CHANGESTAMP2_OFF+CP_CHANGESTAMP2_LEN)
#define CP_INDFREELIST_LEN  BLKNOSIZE

/* number of keys */
#define	CP_NKEYS_OFF	(CP_INDFREELIST_OFF+CP_INDFREELIST_LEN)
#define CP_NKEYS_LEN	SHORTSIZE

/* use 4 bytes of reserved space */
#define CP_VAREND_OFF   (CP_NKEYS_OFF+CP_NKEYS_LEN)
#define CP_VAREND_LEN	LONGSIZE

/* use 4 bytes of reserved space for time stamp*/
#define CP_UNIQUE_OFF   (CP_VAREND_OFF+CP_VAREND_LEN)
#define CP_UNIQUE_LEN	LONGSIZE

/* reserve some space for future */
#define CP_RESERVE_OFF	(CP_UNIQUE_OFF+CP_UNIQUE_LEN)
#define CP_RESERVE_LEN   120		     /* 128 was reserved originally */

/* table of key descriptors */
#define CP_KEYS_OFF	(CP_RESERVE_OFF+CP_RESERVE_LEN)

#define MAXNKEYS2 ((ISCNTLSIZE - CP_KEYS_OFF) / K2_LEN)
#define MAXNKEYS	17

#if (MAXNKEYS2 < MAXNKEYS)
Cause a compiler error here. There is not enought space in the control
page to hold MAXNKEYS.
#endif

/*------ page type indicator values -------------*/
#define PT_INDEX	1		     /* B-tree page */
#define PT_FREELIST	2		     /* free list */

/*-------------- B-Tree page layout -----------------------------------*/
/* page type */
#define BT_TYPE_OFF	0
#define BT_TYPE_LEN	SHORTSIZE

/* level (leaves have level 0) */
#define BT_LEVEL_OFF	(BT_TYPE_OFF+BT_TYPE_LEN)
#define BT_LEVEL_LEN	SHORTSIZE

/* page capacity */
#define BT_CAPAC_OFF	(BT_LEVEL_OFF+BT_LEVEL_LEN)
#define BT_CAPAC_LEN	SHORTSIZE

/* current number of keys */
#define BT_NKEYS_OFF	(BT_CAPAC_OFF+BT_CAPAC_LEN)
#define BT_NKEYS_LEN	SHORTSIZE

/* array of key entries */
#define BT_KEYS_OFF	(BT_NKEYS_OFF+BT_NKEYS_LEN)

/* down pointers are stored at the end of the page */

/*----------- Free List Page Layout --------------------------------------*/
/* page type */
#define FL_TYPE_OFF	0
#define FL_TYPE_LEN	SHORTSIZE

/* pointer to next block in the list */
#define FL_NEXT_OFF	(FL_TYPE_OFF + FL_TYPE_LEN)
#define FL_NEXT_LEN	BLKNOSIZE

/* number of free page pointers */
#define FL_NPOINTERS_OFF  (FL_NEXT_OFF + FL_NEXT_LEN)
#define FL_NPOINTERS_LEN  SHORTSIZE

/* free page pointers table */
#define FL_POINTERS_OFF   (FL_NPOINTERS_OFF + FL_NPOINTERS_LEN)

/* maximum number of pointers that can be stored in a page */
#define FL_MAXNPOINTERS ((ISPAGESIZE - FL_POINTERS_OFF) / BLKNOSIZE)

#define FREELIST_NOPAGE ((Blkno) -1)

/* .var file related defines */
#define VR_FRAMELEN_OFF  0
#define VR_TAILLEN_OFF   2


/* maximum and minimum values are in B-tree seaches */
#if LONG_BIT == 64
#define ISMAXLONG   { 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }
#define ISMINLONG   { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
#else
#define ISMAXLONG	{ 0x7f, 0xff, 0xff, 0xff }
#define ISMINLONG	{ 0x80, 0x00, 0x00, 0x00 }
#endif

#define ISMAXSHORT	{ 0x7f, 0xff }
#define ISMINSHORT	{ 0x80, 0x00 }

#define ISMINCHAR	('\0')
#define ISMAXCHAR	('\377')

#define ISMINBIN	('\0')
#define ISMAXBIN	('\377')

/* double and float are declared using IEEE bit paterns */

#define ISMAXDOUBLE      { 0x7f, 0xf0, 0, /* rest is 0 */ }
#define ISMINDOUBLE      { 0xff, 0xf0, 0, /* rest is 0 */ }

#define ISMAXFLOAT       { 0x7f, 0x80, 0, /* rest is 0 */ }
#define ISMINFLOAT       { 0xff, 0x80, 0, /* rest is 0 */ }


/* Hide these from user till we return error code */
extern char	isstat3;
extern char	isstat4;

#define OP_BUILD		0
#define OP_OPEN			1

#endif
