/* $XConsortium: fsrtns.c /main/cde1_maint/2 1995/08/29 19:44:56 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           fsrtns.c
 *
 *
 *   DESCRIPTION:    Routines to manipulate files and directores
 *
 *   FUNCTIONS: CopyDir
 *		CopyFile
 *		CopyLink
 *		CopyObject
 *		EmptyDir
 *		EraseObject
 *		fsCopy
 *		fsCopyLink
 *		fsEmpty
 *		fsErase
 *		fsMove
 *		fsRename
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <utime.h>
#include <dirent.h>
#include <sys/file.h>
#include <string.h>
#include "fsrtns.h"


/*--------------------------------------------------------------------
 * Callback functions
 *------------------------------------------------------------------*/

#ifdef _NO_PROTO
int (*progressCallback)() = NULL;
int (*errorCallback)() = NULL;
int (*periodicCallback)() = NULL;
#else
int (*progressCallback)(char *fname) = NULL;
int (*errorCallback)(char *fname, int errno) = NULL;
int (*periodicCallback)() = NULL;
#endif

/*--------------------------------------------------------------------
 * Local subroutines
 *------------------------------------------------------------------*/

#ifdef _NO_PROTO
static int CopyObject();
static int EraseObject();
#else
static int CopyObject(char *sourceP, char *targetP, int repl, int link);
static int EraseObject(char *nameP);
#endif


static int
#ifdef _NO_PROTO
CopyFile(sourceP, targetP, repl, statP)
	char *sourceP;
	char *targetP;
	int repl;
	struct stat *statP;
#else
CopyFile(char *sourceP, char *targetP, int repl, struct stat *statP)
#endif
/* copy a file; if repl non-zero, overwrite any existing file */
{
  int src, tgt;
  int nread, nwrite;
  char buffer[2048];
  struct utimbuf ut;
  int rc;

  /* open source file for read */
  src = open(sourceP, O_RDONLY, 0);
  if (src < 0)
    return errno;

  /* create target file */
  tgt = open(targetP, O_CREAT | O_EXCL | O_WRONLY, statP->st_mode & 0777);
  if (tgt < 0 && errno == EEXIST && repl) {
    rc = EraseObject(targetP);
    if (rc) {
      close(src);
      return rc;
    }
    tgt = open(targetP, O_CREAT | O_EXCL | O_WRONLY, statP->st_mode & 0777);
  }
  if (tgt < 0) {
    rc = errno;
    close(src);
    return rc;
  }

  /* if we have root privileges, make sure file ownership is preserved */
  if (geteuid() == 0) {
    if (statP->st_uid != 0 || statP->st_gid != getegid()) {
      rc = fchown(tgt, statP->st_uid, statP->st_gid);
      if (rc) {
        rc = errno;
        close(src);
        close(tgt);
        return rc;
      }
    }
  }

  /* copy data */
  for (;;) {
    /* read data from source file */
    do {
      errno = 0;
      nread = read(src, buffer, sizeof(buffer));
    } while (nread < 0 && errno == EINTR);
    if (nread <= 0)
      break;

    /* write data to target file */
    do {
      errno = 0;
      nwrite = write(tgt, buffer, nread);
    } while (nwrite < 0 && errno == EINTR);
    if (nwrite != nread)
      break;

    if (periodicCallback)
      if (periodicCallback() != 0) {
        unlink(targetP);
        close(src);
        close(tgt);
        return -1;
      }
  }

  /* check if data copy ended abnormally */
  rc = (nread == 0)? 0: (errno != 0)? errno: -1;

  /* close files */
  close(src);
  if (rc) {
    close(tgt);
    return rc;
  }

  if (close(tgt) != 0)
    return errno;

  /* set target file attributes */
  ut.actime = statP->st_atime;
  ut.modtime = statP->st_mtime;
  rc = utime(targetP, &ut);

  return (rc != 0)? errno: 0;
}


static int
#ifdef _NO_PROTO
CopyLink(sourceP, targetP, repl, statP)
	char *sourceP;
	char *targetP;
	int repl;
	struct stat *statP;
#else
CopyLink(char *sourceP, char *targetP, int repl, struct stat *statP)
#endif
/* copy a symbolic link */
{
  int l, rc;
  char buf[1024];

  do {
    errno = 0;
    l = readlink(sourceP, buf, sizeof(buf));
  } while (l < 0 && errno == EINTR);
  if (l < 0)
    return errno;
  buf[l] = 0;
  if (symlink(buf, targetP) == 0)
    return 0;
  else if (errno != EEXIST || !repl)
    return errno;

  if ((rc = EraseObject(targetP)) != 0)
    return rc;
  if (symlink(buf, targetP) == 0)
    return 0;
  else
    return errno;
}


static int
#ifdef _NO_PROTO
CopyDir(sourceP, targetP, repl, link, statP)
	char *sourceP;
	char *targetP;
	int repl;
	int link;
	struct stat *statP;
#else
CopyDir(char *sourceP, char *targetP, int repl, int link, struct stat *statP)
#endif
/* copy a directory and recursively all its subdirectories */
{
  DIR *dirP;                      /* open directory */
  struct dirent *entryP;          /* directory entry */
  char srcname[1024], tgtname[1024];
  int srclen, tgtlen;
  int rc;

  /* open source directory */
  dirP = opendir(sourceP);
  if (dirP == NULL)
    return errno;

  /* create target directory */
  rc = mkdir(targetP, statP->st_mode & 0777);
  if (rc < 0 && errno == EEXIST && repl) {
    rc = EraseObject(targetP);
    if (rc)
      return rc;
    rc = mkdir(targetP, statP->st_mode & 0777);
  }
  if (rc < 0) {
    rc = errno;
    closedir(dirP);
    return rc;
  }

  /* if we have root privileges, make sure directory ownership is preserved */
  if (geteuid() == 0) {
    if (statP->st_uid != 0 || statP->st_gid != getegid()) {
      rc = chown(targetP, statP->st_uid, statP->st_gid);
      if (rc) {
        rc = errno;
        closedir(dirP);
        return rc;
      }
    }
  }

  /* prepare source and target names */
  strcpy(srcname, sourceP);
  srclen = strlen(srcname);
  if (srcname[srclen - 1] != '/')
    srcname[srclen++] = '/';
  strcpy(tgtname, targetP);
  tgtlen = strlen(tgtname);
  if (tgtname[tgtlen - 1] != '/')
    tgtname[tgtlen++] = '/';

  for (rc = 0; rc == 0; ) {
    do {
      errno = 0;
      entryP = readdir(dirP);
    } while (entryP == NULL && errno == EINTR);
    if (entryP == NULL) {
      rc = errno;
      break;
    }
    if (strcmp(entryP->d_name, ".") == 0 || strcmp(entryP->d_name, "..") == 0)
      continue;

    strcpy(srcname + srclen, entryP->d_name);
    strcpy(tgtname + tgtlen, entryP->d_name);

    rc = CopyObject(srcname, tgtname, repl, link);
  }

  closedir(dirP);
  return rc;
}


static int
#ifdef _NO_PROTO
CopyObject(sourceP, targetP, repl, link)
	char *sourceP;
	char *targetP;
	int repl;
	int link;
#else
CopyObject(char *sourceP, char *targetP, int repl, int link)
#endif
/* copy a directory, file, or symbolic link */
{
  struct stat src_stat;
  int rc;

  if (progressCallback)
    if (progressCallback(sourceP) != 0)
      return -1;

  if (periodicCallback)
    if (periodicCallback() != 0)
      return -1;

  if (lstat(sourceP, &src_stat) < 0)
    rc = errno;

  else {

copy_switch:

    switch(src_stat.st_mode & S_IFMT) {

      case S_IFDIR:
        rc = CopyDir(sourceP, targetP, repl, link, &src_stat);
        break;

      case S_IFREG:
        rc = CopyFile(sourceP, targetP, repl, &src_stat);
        break;

      case S_IFLNK:
        if (link)
          rc = CopyLink(sourceP, targetP, repl, &src_stat);
        else if (stat(sourceP, &src_stat) < 0)
          rc = errno;
        else
          goto copy_switch;
        break;

      default:
        rc = EINVAL;
    }
  }

  /*
   * Return code zero means everything is ok;
   * return code -1 means the operation is aborted.
   * In either case, propagated the return code up.
   */
  if (rc <= 0)
    return rc;

  /*
   * Return code > 0 means an error occurred in the last operation.
   * Call the the error callback function.  If the callback returns
   * zero, we return zero; this will cause the error to be ignored.
   * Otherwise we return -1 to signal that the operation is aborted.
   */
  if (!errorCallback)
    return rc;
  else if (errorCallback(sourceP, rc) == 0)
    return 0;
  else
    return -1;
}


int
#ifdef _NO_PROTO
EmptyDir(sourceP, rm)
	char *sourceP;
	int rm;
#else
EmptyDir(char *sourceP, int rm)
#endif
{
  DIR *dirP;                      /* open directory */
  struct dirent *entryP;          /* directory entry */
  char srcname[1024];
  int srclen;
  int rc;

  /* open source directory */
  dirP = opendir(sourceP);
  if (dirP == NULL)
    return errno;

  /* prepare source name */
  strcpy(srcname, sourceP);
  srclen = strlen(srcname);
  if (srcname[srclen - 1] != '/')
    srcname[srclen++] = '/';

  rc = 0;
  while (rc == 0 && (entryP = readdir(dirP)) != NULL) {
    if (strcmp(entryP->d_name, ".") == 0 || strcmp(entryP->d_name, "..") == 0)
      continue;
    strcpy(srcname + srclen, entryP->d_name);
    rc = EraseObject(srcname);
  }

  closedir(dirP);
  if (rc == 0 && rm) {
    rc = rmdir(sourceP);
    if (rc < 0)
      rc = errno;
  }
  return rc;
}


static int
#ifdef _NO_PROTO
EraseObject(nameP)
	char *nameP;
#else
EraseObject(char *nameP)
#endif
{
  struct stat src_stat;
  int rc;

  if (periodicCallback)
    if (periodicCallback() != 0)
      return -1;

  if (lstat(nameP, &src_stat) < 0)
    rc = errno;
  else if ((src_stat.st_mode & S_IFMT) == S_IFDIR)
    rc = EmptyDir(nameP, 1);
  else {
    rc = unlink(nameP);
    if (rc != 0)
      rc = errno;
  }

  /*
   * Return code zero means everything is ok;
   * return code -1 means the operation is aborted.
   * In either case, propagated the return code up.
   */
  if (rc <= 0)
    return rc;

  /*
   * Return code > 0 means an error occurred in the last operation.
   * Call the the error callback function.  If the callback returns
   * zero, we return zero; this will cause the error to be ignored.
   * Otherwise we return -1 to signal that the operation is aborted.
   */
  if (!errorCallback)
    return rc;
  else if (errorCallback(nameP, rc) == 0)
    return 0;
  else
    return -1;
}


/*--------------------------------------------------------------------
 * Exported routines
 *------------------------------------------------------------------*/

void
#ifdef _NO_PROTO
fsRename(sourceP, targetP, replace, rcP)
	char *sourceP;
	char *targetP;
	int replace;
	int *rcP;
#else
fsRename(char *sourceP, char *targetP, int replace, int *rcP)
#endif
{
  int rc;
  struct stat buf;

  if (!replace) {
    /* return error if target file already exists */
    rc = lstat(targetP, &buf);
    if (rc == 0) {
      *rcP = EEXIST;
      return;
    } else if (errno != ENOENT) {
      *rcP = errno;
      return;
    }
  }
  *rcP = rename(sourceP, targetP);
  if (*rcP < 0)
    *rcP = errno;

  if (replace && *rcP == ENOTDIR || *rcP == EISDIR) {
    /* error reason: tried to replace file by directory or vice versa */
    *rcP = EraseObject(targetP);
    if (*rcP < 0)
      return;
    *rcP = rename(sourceP, targetP);
    if (*rcP < 0)
      *rcP = errno;
  }
}


void
#ifdef _NO_PROTO
fsMove(sourceP, targetP, replace, rcP)
	char *sourceP;
	char *targetP;
	int replace;
	int *rcP;
#else
fsMove(char *sourceP, char *targetP, int replace, int *rcP)
#endif
{
  /* try to rename */
  fsRename(sourceP, targetP, replace, rcP);
  if (*rcP == 0 || *rcP != EXDEV)
    return;

  /* source and target on different file systems: do copy + erase */
  {
    /* first check if we have write permission in the source directory */
    char dir[1024], *p;

    strcpy(dir, sourceP);
    p = strrchr(dir, '/');
    if (p == 0)
      strcpy(dir, ".");
    else if (p == dir)
      strcpy(dir, "/");
    else
      *p = 0;

    if (access(dir, W_OK) != 0) {
      *rcP = errno;
      return;
    }
  }

  *rcP = CopyObject(sourceP, targetP, replace, 1);
  if (*rcP != 0)
    return;
  *rcP = EraseObject(sourceP);
}


void
#ifdef _NO_PROTO
fsCopy(sourceP, targetP, replace, rcP)
	char *sourceP;
	char *targetP;
	int replace;
	int *rcP;
#else
fsCopy(char *sourceP, char *targetP, int replace, int *rcP)
#endif
{
   *rcP = CopyObject(sourceP, targetP, replace, 0);
}


void
#ifdef _NO_PROTO
fsCopyLink(sourceP, targetP, replace, rcP)
	char *sourceP;
	char *targetP;
	int replace;
	int *rcP;
#else
fsCopyLink(char *sourceP, char *targetP, int replace, int *rcP)
#endif
{
  *rcP = CopyObject(sourceP, targetP, replace, 1);
}


void
#ifdef _NO_PROTO
fsErase(nameP, rcP)
	char *nameP;
	int *rcP;
#else
fsErase(char *nameP, int *rcP)
#endif
{
  *rcP = EraseObject(nameP);
}


void
#ifdef _NO_PROTO
fsEmpty(nameP, rcP)
	char *nameP;
	int *rcP;
#else
fsEmpty(char *nameP, int *rcP)
#endif
{
  *rcP = EmptyDir(nameP, 0);
}

