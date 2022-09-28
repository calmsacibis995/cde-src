/* $XConsortium: dosync.h /main/cde1_maint/2 1995/08/29 19:43:15 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           dosync.h
 *
 *
 *   DESCRIPTION:    Header file for dosync.c, main_dtcopy.c
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _dosync_h
#define _dosync_h

/* file type flags used in syncConfirmCallback */
#define ft_noent 0x01   /* file does not exist */
#define ft_isdir 0x02   /* file is a directory */
#define ft_islnk 0x04   /* file is a symbolic link */

/* file operations used in syncConfirmCallback and syncErrorCallback */
typedef enum {
  op_sync,        /* beginning directory sync */
  op_opendir,     /* opendir */
  op_lstat,       /* lstat */
  op_stat,        /* stat */
  op_readlink,    /* readlink */
  op_delete,      /* deleting file in target dir */
  op_copy,        /* copy/replace file in target dir */
  op_mkdir,       /* create new sub directory */
  op_cplink,      /* copy/replace symbolic link in target dir */
  op_mklink,      /* create link to source file/dir in target dir */
  op_info_working,/* informational: still working on the current operation */
  op_info_copy,   /* informational: target is real copy of source */
  op_info_link,   /* informational: target is only sym link to source */
  op_rename       /* used in main_dtcopy.c to report an error */
} FileOp;

/* array of strings naming the file operaions */
extern char *FileOpNames[];

/* Callback functions */
#ifdef _NO_PROTO
extern int (*syncConfirmCallback)();
extern int (*syncErrorCallback)();
#else
extern int (*syncConfirmCallback)(FileOp op,
                                  char *sname, int stype,
                                  char *tname, int ttype,
                                  char *link);
extern int (*syncErrorCallback)(FileOp op, char *fname, int errno);
#endif

/* info for mapping symbolic links (-ml option) */
typedef struct ml {
  char *from;
  char *to;
  struct ml *next;
} MapList;


/* list of file name patterns for -exclude and -skip */
typedef struct pl {
  char *pattern;
  struct pl *next;
} PatternList;


/* parameters for syncdir function */
typedef struct {
  char *source;     /* source directory */
  char *target;     /* target directory */

  int verbose;      /* verbose output */
  int quiet;        /* quiet output */
  int dontdoit;     /* only print what would be done */

  int keepnew;      /* keep target files that are newer than the source */
  char *keepold;    /* save target files that would be deleted/replaced */
  int dontdelete;   /* don't delete any files that no longer exist */
  int dontadd;      /* don't create new files */
  int dontreplace;  /* don't replace existing files */
  int dontrecur;    /* don't go into subdirectories */

  MapList *maplink;
  int keeplinks;    /* keep links that point to source file */
  int keepcopies;   /* keep copies of files that are links in source */
  int forcecopies;  /* copy files even if unmodified */
  int listlinks;    /* list links that point to source file */
  int listcopies;   /* list copies of links */
  int linkdirs;     /* when new dir is found, just create a link to it */
  int linkfiles;    /* when new file is found, just create a link to it */
  int copydirs;     /* when link to dir is found, copy it */
  int copyfiles;    /* when link to file is found, copy it */
  int copytop;      /* if top-level is link, copy it */

  PatternList *exclude;  /* source files to be excluded */
  PatternList *skip;     /* files to be skipped */
} SyncParams;


#ifdef _NO_PROTO
extern void SyncDirectory();
#else
extern void SyncDirectory(SyncParams *p);
#endif

#endif /* _dosync_h */
