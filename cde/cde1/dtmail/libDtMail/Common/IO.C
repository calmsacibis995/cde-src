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
#pragma ident "@(#)IO.C	1.24 06/23/95"
#endif

#include <sys/types.h>
#include <unistd.h>
#include <grp.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <assert.h>

#include <DtMail/DtMail.hh>
#include <DtMail/IO.hh>
#include <DtMail/Threads.hh>

#if defined(_AIX)
#include <stdio.h>
#include <sys/socket.h>
extern "C" ssize_t writev(int, const struct iovec *, int);
#define IOVMAX MSG_MAXIOVLEN
#endif

#if defined(HPUX)
#define IOV_MAX MAXIOV
#endif

#if !defined(IOV_MAX)
#include <sys/stream.h>
#if !defined(DEF_IOV_MAX)
#define DEF_IOV_MAX 16
#endif
#define IOV_MAX DEF_IOV_MAX
#endif

#if defined(_AIX)
extern "C" fchmod(int, mode_t);
#endif

// The following I/O routines are wrappers for the normal routines,
// but they deal with EINTR, and partial read/write situations.
//
//
int
SafeOpen(const char * path, int oflag, mode_t mode)
{
    int status;
    do {
	status = open(path, oflag, mode);
    } while (status < 0 && errno == EINTR);

    return(status);
}

int
SafeClose(int fd)
{
    int status;
    do {
	status = close(fd);
    } while (status < 0 && errno == EINTR);

    return(status);
}

int
SafeLink(const char * existingPath, const char * newPath)
{
  int status;
  do {
    status = link(existingPath, newPath);
  } while (status < 0 && errno == EINTR);

  return(status);
}

int
SafeRename(const char * oldPath, const char * newPath)
{
  int status;
  do {
    status = rename(oldPath, newPath);
  } while (status < 0 && errno == EINTR);

  return(status);
}

int
SafeRemove(const char *path)
{
    int status;
    do {
      status = remove(path);
    } while (status < 0 && errno == EINTR);

    return(status);
}

int
SafeUnlink(const char *path)
{
    int status;
    do {
      status = unlink(path);
    } while (status < 0 && errno == EINTR);

    return(status);
}

ssize_t
SafeRead(int fd, void * buf, size_t bytes)
{
    ssize_t status = 0;
    do {
	status = read(fd, buf, bytes);
    } while(status < 0 && errno == EINTR);

    return(status);
}

ssize_t
SafeWrite(int fd, const void * buf, size_t bytes)
{
    ssize_t status = 0;
    do {
	status = write(fd, buf, bytes);
    } while(status < 0 && errno == EINTR);
    
    return(status);
}

// SafeWritev -- safe multiply vectored write
// Description:
//   SafeWritev provides an interrupt safe way to call the writev() system
//   call. In addition, it removes the IOV_MAX limitation on the size of the
//   iovec structure, and it will not return until either all bytes specified
//   by the iovec structure are written or writev() returns an error.
// Arguments:
//   int fd	-- file descriptor to write to
//   struct iovec *iov -- iovec structure describing writes to be done
//   int iovcnt	-- number of elements in passed in iov structure
// Outputs:
//   Will effectively destroy the contents of the passed in iovec structure.
//   The caller must deallocate the storage associated with the structure
//   upon regaining control.
// Returns:
//   == -1 : error returned from writev() - errno set to specific error number
//   != -1 : number of bytes actually written to the file

unsigned long
SafeWritev(int fd, struct iovec *iov, int iovcnt)
{
  ssize_t status;
  unsigned long bytesWritten = 0;

  // outer loop: starting with the first write vector, as long as there is
  // at least one vector left to feed writev(), do so
  //
  for(int curIov = 0; curIov < iovcnt; ) {

    // inner loop: feed writev() allowing for interrupted system call
    do {
      status = writev(fd, &iov[curIov], (iovcnt-curIov) > IOV_MAX ? IOV_MAX : (iovcnt-curIov));
    } while (status < 0 && errno == EINTR);

    if (status == (ssize_t)-1)		// true error from writev??
      return((unsigned long)-1);	// yes: bail at this point to caller
    bytesWritten += status;		// bump # bytes written count

    // must now "walk through" the io vector until we are up the to point
    // indicated by the number of bytes written by writev() - any leftover
    // in status indicates a partial write of a vector
    //
    while ((status > 0) && (curIov < iovcnt) && (iov[curIov].iov_len <= status)) {
      status -= iov[curIov++].iov_len;
    }

    // Check to see if we have reached the end of the io vector array; also
    // check to see if more bytes were written than called for; as crazy as
    // this sounds, in at least one instance when we finish spinning through
    // the io vectors we still had bytes left that had been written but not
    // accounted for in the io vectors (status > 0 && curIov >= iovcnt)
    //
    if (curIov >= iovcnt) {		// out of IO vectors?
      if (status > 0) {			// yes: all data accounted for?
	DtMailEnv error;		// NO:: log error condition
	error.logError(DTM_TRUE, "SafeWritev: writev(): too many bytes written (%d/%d)\n",
		       status, bytesWritten);
      }
      continue;				// continue and let for loop exit
    }
    
    // Check for a partial write: if the current vector contains more data
    // than what has been written, writev() bailed in the middle of writing
    // a vector - adjust the vector and and feed it back to writev() again
    // OTHERWISE writev() ended with the current vector so move on to the next
    //
    if (iov[curIov].iov_len == status)	// full write of this vector?
      curIov++;				// yes: move on to the next vector
    else if (status != 0) {		// no: adjust this vector and retry
      iov[curIov].iov_len -= status;
      iov[curIov].iov_base += status;
    }
  }
  return(bytesWritten);
}

#define SWS_BUFFERSIZE 1024
ssize_t
SafeWriteStrip(int fd, const void * buf, size_t bytes)
{
    ssize_t status = 0;
    int i, j;
    char *ptr = (char*)buf, *writebuf; 
    
    // bug 5856: dont write out control M 
    // make a finite size buffer for writing
    writebuf = (char*)malloc(bytes < SWS_BUFFERSIZE ? bytes : SWS_BUFFERSIZE); 

    for (i = 0, j = 0; i < bytes; i++, ptr++) {
	if (*ptr == '' && *(ptr+1) == '\n')
		continue;
	writebuf[j++] = *ptr;
	if (j == SWS_BUFFERSIZE || i == (bytes-1)) {
    		do {
			status = write(fd, writebuf, j);
    		} while(status < 0 && errno == EINTR);
		j = 0;
	}
    }
    free(writebuf);
    
    return(status);
}

int
SafeStat(const char * path, struct stat * buf)
{
    int status;
    do {
	status = stat(path, buf);
    } while (status < 0 && errno == EINTR);

    return(status);
}

int
SafeFchown(int fd, uid_t owner, gid_t group)
{
  int status;
  do {
    status = fchown(fd, owner, group);
  } while (status < 0 && errno == EINTR);

  return(status);
}

int
SafeLStat(const char * path, struct stat * buf)
{
    int status;
    do {
	status = lstat(path, buf);
    } while (status < 0 && errno == EINTR);

    return(status);
}

int
SafeFStat(int fd, struct stat * buf)
{
    int status;
    do {
	status = fstat(fd, buf);
    } while (status < 0 && errno == EINTR);

    return(status);
}

// SafeGuaranteedStat - return stat info for object given path name
// If NFS attribute caching is enabled (which is the default), a
// stat/fstat of a NFS file may not return the correct true size of the
// mailbox if it has been appended to since the last time it was read.
// To get around this problem, this function will perform a open(),
// read() of 1 byte, fstat(), close() which will force the attributes
// for the named file to be retrieved directly from the server.
// 
int
SafeGuaranteedStat(const char * path, struct stat * buf)
{
  int saveErrno;

  int tempFd = SafeOpen(path, O_RDONLY|O_RSYNC|O_SYNC);
  if (tempFd == -1) {
    return(-1);
  }

  char tempBuf;  
  if (SafeRead(tempFd, &tempBuf, 1) == -1) {
    saveErrno = errno;
    (void) SafeClose(tempFd);
    errno = saveErrno;
    return(-1);
  }

  if (SafeFStat(tempFd, buf) == -1) {
    saveErrno = errno;
    (void) SafeClose(tempFd);
    errno = saveErrno;
    return(-1);
  }

  (void) SafeClose(tempFd);

  return(0);
}

int
SafeTruncate(const char * path, off_t len)
{
    int status;
    do {
	status = truncate((char *)path, len);	// The cast is for AIX
    } while (status < 0 && errno == EINTR);

    return(status);
}

int
SafeFTruncate(int fd, off_t len)
{
    int status;
    do {
	status = ftruncate(fd, len);
    } while (status < 0 && errno == EINTR);

    return(status);
}

int
SafeAccess(const char * path, int amode)
{
    int status;
    do {
	status = access(path, amode);
    } while (status < 0 && errno == EINTR);

    return(status);
}

int
SafeLockf(int fd, int func, long size)
{
    int status;
    do {
	status = lockf(fd, func, size);
    } while (status < 0 && errno == EINTR);

    return(status);
}

int
SafeFChmod(int fd, mode_t mode)
{
    int status;
    do {
	status = fchmod(fd, mode);
    } while (status < 0 && errno == EINTR);

    return(status);
}

int
SafeDup2(int fd1, int fd2)
{
    int status;
    do {
	status = dup2(fd1, fd2);
    } while (status < 0 && errno == EINTR);

    return(status);
}

int
SafeExecvp(const char * file, char *const *argv)
{
    int status;
    do {

#if defined(_AIX)
	status = execvp(file, (const char **)argv);
#else
	status = execvp(file, argv);
#endif

    } while (status < 0 && errno == EINTR);

    return(status);
}

int
SafeWaitpid(pid_t proc, int * p_stat, int options)
{
    pid_t status;
    do {
	status = waitpid(proc, p_stat, options);
    } while (status < 0 && errno == EINTR);

    return((int)status);
}

int
SafeWait(int * p_stat)
{
    pid_t status;
    do {
	status = wait(p_stat);
    } while (status < 0 && errno == EINTR);

    return((int)status);
}

int
SafeUTime(const char * path, utimbuf * ntime)
{
    int status;
    do {
	status = utime(path, ntime);
    } while (status < 0 && errno == EINTR);

    return(status);
}

static void * time_mutex = NULL;

void
SafeCtime(const time_t *clock, char * buf, int buflen)
{
#if defined(POSIX_THREADS)

    if (!time_mutex) {
	time_mutex = MutexInit();
    }

    MutexLock lock_scope(time_mutex);

    ctime_r(clock, buf, buflen);
#else
    strcpy(buf, ctime(clock));
#endif
}

void
SafeLocaltime(const time_t *clock, tm & result)
{
#if defined(POSIX_THREADS)
    localtime_r(clock, &result);
#else
    result = *localtime(clock);
#endif
}

time_t
SafeMktime(tm * timeptr)
{
#if defined(POSIX_THREADS)

    if (!time_mutex) {
	time_mutex = MutexInit();
    }

    MutexLock lock_scope(time_mutex);
#endif

    return(mktime(timeptr));
}

size_t
SafeStrftime(char * buf, size_t buf_size,
	     const char * format, const tm * timeptr)
{
#if defined(POSIX_THREADS)

    if (!time_mutex) {
	time_mutex = MutexInit();
    }

    MutexLock lock_scope(time_mutex);
#endif

    return(strftime(buf, buf_size, format, timeptr));
}

void
GetGroupName(char * grp_name)
{
#if defined(POSIX_THREADS)
    char buf[1024];
    group	gbuf;
    group	*grp = &gbuf;

    grp = getgrgid_r(getegid(), &gbuf, buf, 1024);

#else

    group *grp;

    grp = getgrgid(getegid());

#endif

    if (grp) {
	strcpy(grp_name, grp->gr_name);
    }
    else {
	strcpy(grp_name, "UNKNOWN_GROUP");
    }

}

gid_t
GetIdForGroupName(char * grp_name)
{
  assert(grp_name);
  
#if defined(POSIX_THREADS)
  char buf[1024];
  group gbuf;
  group *grp = &gbuf;

  grp = getgrname_r(grp_name, &gbuf, buf, 1024);
#else
  group *grp;

  grp = getgrnam(grp_name);
#endif
    
  return(grp ? grp->gr_gid : (gid_t)-1);
}

void
GetPasswordEntry(passwd & result)
{
  static struct passwd passwordEntry;
  static char passwordData[1024];
  static int oneTimeFlag = 0;

#if defined(POSIX_THREADS)
  if (!oneTimeFlag) {
    getpwuid_r(getuid(), &passwordEntry, passwordData, sizeof(passwordData));
    oneTimeFlag++;
  }
#else
  if (!oneTimeFlag) {
    struct passwd *tresult;
    tresult = getpwuid(getuid());
    assert(tresult);
    memcpy(&passwordEntry, tresult, sizeof(struct passwd));
    passwordEntry.pw_name = strdup(passwordEntry.pw_name);
    passwordEntry.pw_passwd = strdup(passwordEntry.pw_passwd);
#if !defined(_AIX)
    passwordEntry.pw_age = strdup(passwordEntry.pw_age);
    passwordEntry.pw_comment = strdup(passwordEntry.pw_comment);
#endif
    passwordEntry.pw_gecos = strdup(passwordEntry.pw_gecos);
    passwordEntry.pw_dir = strdup(passwordEntry.pw_dir);
    passwordEntry.pw_shell = strdup(passwordEntry.pw_shell);
    oneTimeFlag++;
  }
#endif
  memcpy(&result, &passwordEntry, sizeof(struct passwd));
  return;
}
