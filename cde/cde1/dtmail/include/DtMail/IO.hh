/*
 *+SNOTICE
 *
 *
 *	$Revision: 1.3 $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement bertween
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel wihtout
 *	Sun's specific written approval.  This documment and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)IO.hh	1.19 06/23/95"
#endif

#ifndef _IO_HH
#define _IO_HH

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <utime.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>

// The following I/O routines are wrappers for the normal routines,
// but they deal with EINTR, and partial read/write situations.
//
//
int SafeOpen(const char * path, int oflag, mode_t mode = 0644);
int SafeClose(int fd);
int SafeRename(const char * oldPath, const char * newPath);
int SafeLink(const char * existingPath, const char * newPath);
int SafeRemove(const char * path);
int SafeUnlink(const char * path);
ssize_t SafeRead(int fd, void * buf, size_t bytes);
ssize_t SafeWrite(int fd, const void * buf, size_t bytes);
ssize_t SafeWriteStrip(int fd, const void * buf, size_t bytes);
unsigned long SafeWritev(int fd, struct iovec *iov, int iovcnt);
int SafeStat(const char * path, struct stat * buf);
int SafeLStat(const char * path, struct stat * buf);
int SafeFStat(int fd, struct stat * buf);
int SafeGuaranteedStat(const char * path, struct stat * buf);
int SafeTruncate(const char * path, off_t len);
int SafeFTruncate(int fd, off_t len);
int SafeAccess(const char * path, int amode);
int SafeLockf(int fd, int func, long size);
int SafeFchown(int fd, uid_t owner, gid_t group);
int SafeFChmod(int fd, mode_t mode);
int SafeDup2(int fd1, int fd2);
int SafeExecvp(const char * file, char *const *argv);
int SafeWaitpid(pid_t proc, int * status, int options);
int SafeWait(int * status);
int SafeUTime(const char * path, utimbuf * ntime);

// The time routines deal with MT and reentrant issues in a platform
// independent way.
//
void SafeCtime(const time_t *clock, char * buf, int buflen = 26);
void SafeLocaltime(const time_t *clock, tm & result);
time_t SafeMktime(tm * timeptr);
size_t SafeStrftime(char * buf, size_t buf_size,
		    const char * format, const tm * timeptr);

// The group and password routines will retrieve the group and
// password names in an MT-Safe way.
//
void GetGroupName(char * grp_name);
gid_t GetIdForGroupName(char * grp_name);
void GetPasswordEntry(passwd & result);

#endif
