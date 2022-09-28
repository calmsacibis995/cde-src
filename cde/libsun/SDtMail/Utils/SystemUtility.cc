/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Utils base class.

#pragma ident "@(#)SystemUtility.cc	1.31 97/02/05 SMI"

// Include Files
#include <sys/types.h>
#include <unistd.h>
#include <locale.h>
#include <grp.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/SystemUtility.hh>
#include <SDtMail/Utility.hh>
#include <SDtMail/MailRc.hh>
#include <DataStructs/MutexEntry.hh>

#if !defined(IOV_MAX)
#include <sys/stream.h>
#if !defined(DEF_IOV_MAX)
#define DEF_IOV_MAX 16
#endif
#define IOV_MAX DEF_IOV_MAX
#endif

// need to include external declaration for ctime_r
// because it doesn't get declared when REENTRANT is
// not defined.
extern "C" char *ctime_r(const time_t *, char *, int);
extern "C" struct tm *localtime_r(const time_t *, struct tm *);

mutex_t SdmSystemUtility::_localeMutex = DEFAULTMUTEX;
mutex_t SdmSystemUtility::_passwordEntryMutex = DEFAULTMUTEX;
mutex_t SdmSystemUtility::_personalNameMutex = DEFAULTMUTEX;
mutex_t SdmSystemUtility::_hostNameMutex = DEFAULTMUTEX;
mutex_t SdmSystemUtility::_randomMutex = DEFAULTMUTEX;
mutex_t SdmSystemUtility::_groupPrivilegeMasterMutex = DEFAULTMUTEX;
mutex_t SdmSystemUtility::_groupPrivilegeOperationMutex = DEFAULTMUTEX;
SdmBoolean SdmSystemUtility::_groupPrivilegeEnabled = Sdm_False;

static void _SdmInitializeRand()
{
  // "Seed" the random number generator
  ::srand((unsigned)time(NULL));
}
#pragma init(_SdmInitializeRand)


SdmSystemUtility::SdmSystemUtility()
{
}

SdmSystemUtility::~SdmSystemUtility()
{
}

int SdmSystemUtility::_hardwarePageSize = 8192;
static void _SdmRetrieveHardwarePageSize()
{
  SdmSystemUtility::_hardwarePageSize = (int)sysconf(_SC_PAGESIZE);
  assert(SdmSystemUtility::_hardwarePageSize);
}
#pragma init(_SdmRetrieveHardwarePageSize)

// This method is called whenever group permissions are to be enabled or disabled
// by a lower level function. It only performs a necessary action if an enable is
// requested and we are currently disabled, or a disable is requested and we are
// currently enabled. To enable we grab and hold an operation mutex that open and
// access use - effectively preventing an open or access to happen while special
// privileges are enabled. To disable we release the operation mutex.
// We return 'Sdm_True' if a state change occurred, and 'Sdm_False' otherwise.
//
SdmBoolean SdmSystemUtility::GroupPrivilegeLockoutControl(SdmBoolean enableFlag)
{
  SdmMutexEntry entry(_groupPrivilegeMasterMutex);
  if (_groupPrivilegeEnabled == Sdm_False && enableFlag == Sdm_True) {
    ::mutex_lock(&_groupPrivilegeOperationMutex);
    _groupPrivilegeEnabled = Sdm_True;
    return(Sdm_True);
  }
  else if (_groupPrivilegeEnabled == Sdm_True && enableFlag == Sdm_False) {
    ::mutex_unlock(&_groupPrivilegeOperationMutex);
    _groupPrivilegeEnabled = Sdm_False;
    return(Sdm_True);
  }
  else
    return(Sdm_False);
}

// The following I/O routines are wrappers for the normal routines,
// but they deal with EINTR, and partial read/write situations.
//
//
int
SdmSystemUtility::SafeOpen(const char * path, int oflag, mode_t mode)
{
  SdmMutexEntry entry(SdmSystemUtility::_groupPrivilegeOperationMutex);
  int status;
  do {
    status = ::open(path, oflag, mode);
  } while (status < 0 && errno == EINTR);
  
  return(status);
}

int
SdmSystemUtility::SafeClose(int fd)
{
  int status;
  do {
    status = ::close(fd);
  } while (status < 0 && errno == EINTR);
  
  return(status);
}

off_t
SdmSystemUtility::SafeLseek(int fd, off_t offset, int whence)
{
  off_t status;
  do {
    status = ::lseek(fd, offset, whence);
  } while (status == -1 && errno == EINTR);
  
  return(status);
}

int
SdmSystemUtility::SafeLink(const char * existingPath, const char * newPath)
{
  int status;
  do {
    status = ::link(existingPath, newPath);
  } while (status < 0 && errno == EINTR);
  
  return(status);
}

int
SdmSystemUtility::SafeRename(const char * oldPath, const char * newPath)
{
  int status;
  do {
    status = ::rename(oldPath, newPath);
  } while (status < 0 && errno == EINTR);
  
  return(status);
}

int
SdmSystemUtility::SafeFsync(int fd)
{
  int status;
  do {
    status = ::fsync(fd);
  } while (status < 0 && errno == EINTR);
  
  return(status);
}

int
SdmSystemUtility::SafeRemove(const char *path)
{
  int status;
  do {
    status = ::remove(path);
  } while (status < 0 && errno == EINTR);
  
  return(status);
}

int
SdmSystemUtility::SafeUnlink(const char *path)
{
  int status;
  do {
    status = ::unlink(path);
  } while (status < 0 && errno == EINTR);
  
  return(status);
}

ssize_t
SdmSystemUtility::SafeRead(int fd, void * buf, size_t bytes)
{
  ssize_t status = 0;
  do {
    status = ::read(fd, buf, bytes);
  } while(status < 0 && errno == EINTR);
  
  return(status);
}

ssize_t
SdmSystemUtility::SafeWrite(int fd, const void * buf, size_t bytes)
{
  ssize_t status = 0;
  do {
    status = ::write(fd, buf, bytes);
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
SdmSystemUtility::SafeWritev(int fd, struct iovec *iov, int iovcnt)
{
  ssize_t status;
  unsigned long bytesWritten = 0;
  
  // outer loop: starting with the first write vector, as long as there is
  // at least one vector left to feed writev(), do so
  //
  for(int curIov = 0; curIov < iovcnt; ) {
    
    // inner loop: feed writev() allowing for interrupted system call
    do {
      status = ::writev(fd, &iov[curIov], (iovcnt-curIov) > IOV_MAX ? IOV_MAX : (iovcnt-curIov));
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
        SdmUtility::LogError(Sdm_True, "SafeWritev: writev(): too many bytes written (%d/%d)\n",
                       status, bytesWritten);   }
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

static const int SWS_BUFFERSIZE = 1024;

ssize_t
SdmSystemUtility::SafeWriteStrip(int fd, const void * buf, size_t bytes)
{
  ssize_t status = 0;
  int i, j;
  char *ptr = (char*)buf, *writebuf; 
  
  // bug 5856: dont write out control M
  // make a finite size buffer for writing
  writebuf = (char*)::malloc(bytes < SWS_BUFFERSIZE ? bytes : SWS_BUFFERSIZE); 
  
  for (i = 0, j = 0; i < bytes; i++, ptr++) {
    if (*ptr == '\015' && *(ptr+1) == '\n')
      continue;
    writebuf[j++] = *ptr;
    if (j == SWS_BUFFERSIZE || i == (bytes-1)) {
      do {
	status = ::write(fd, writebuf, j);
      } while(status < 0 && errno == EINTR);
      j = 0;
    }
  }
  ::free(writebuf);
  
  return(status);
}

int
SdmSystemUtility::SafeStat(const char * path, struct stat * buf)
{
  int status;
  do {
    status = ::stat(path, buf);
  } while (status < 0 && errno == EINTR);
  
  return(status);
}

int
SdmSystemUtility::SafeFchown(int fd, uid_t owner, gid_t group)
{
  int status;
  do {
    status = ::fchown(fd, owner, group);
  } while (status < 0 && errno == EINTR);
  
  return(status);
}

int
SdmSystemUtility::SafeLStat(const char * path, struct stat * buf)
{
  int status;
  do {
    status = ::lstat(path, buf);
  } while (status < 0 && errno == EINTR);
  
  return(status);
}

int
SdmSystemUtility::SafeFStat(int fd, struct stat * buf)
{
  int status;
  do {
    status = ::fstat(fd, buf);
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
SdmSystemUtility::SafeGuaranteedStat(const char * path, struct stat * buf)
{
  int saveerrno;
  
  int tempFd = SafeOpen(path, O_RDONLY|O_RSYNC|O_SYNC);
  if (tempFd == -1) {
    return(-1);
  }
  
  char tempBuf;  
  if (SafeRead(tempFd, &tempBuf, 1) == -1) {
    saveerrno = errno;
    (void) SafeClose(tempFd);
    errno = saveerrno;
    return(-1);
  }
  
  if (SafeFStat(tempFd, buf) == -1) {
    saveerrno = errno;
    (void) SafeClose(tempFd);
    errno = saveerrno;
    return(-1);
  }
  
  (void) SafeClose(tempFd);
  
  return(0);
}

int
SdmSystemUtility::SafeTruncate(const char * path, off_t len)
{
  int status;
  do {
    status = ::truncate((char *)path, len);	// The cast is for AIX
  } while (status < 0 && errno == EINTR);
  
  return(status);
}

int
SdmSystemUtility::SafeFTruncate(int fd, off_t len)
{
  int status;
  do {
    status = ::ftruncate(fd, len);
  } while (status < 0 && errno == EINTR);
  
  return(status);
}

int
SdmSystemUtility::SafeAccess(const char * path, int amode)
{
  SdmMutexEntry entry(SdmSystemUtility::_groupPrivilegeOperationMutex);
  int status;
  do {
    status = ::access(path, amode);
  } while (status < 0 && errno == EINTR);
  
  return(status);
}

int
SdmSystemUtility::SafeLockf(int fd, int func, long size)
{
  int status;
  do {
    status = ::lockf(fd, func, size);
  } while (status < 0 && errno == EINTR);
  
  return(status);
}

int
SdmSystemUtility::SafeFChmod(int fd, mode_t mode)
{
  int status;
  do {
    status = ::fchmod(fd, mode);
  } while (status < 0 && errno == EINTR);
  
  return(status);
}

int
SdmSystemUtility::SafeDup2(int fd1, int fd2)
{
  int status;
  do {
    status = ::dup2(fd1, fd2);
  } while (status < 0 && errno == EINTR);
  
  return(status);
}

int
SdmSystemUtility::SafeExecvp(const char * file, char *const *argv)
{
  int status;
  do {
    
#if defined(_AIX)
    status = ::execvp(file, (const char **)argv);
#else
    status = ::execvp(file, argv);
#endif
    
  } while (status < 0 && errno == EINTR);
  
  return(status);
}

int
SdmSystemUtility::SafeWaitpid(pid_t proc, int * p_stat, int options)
{
  pid_t status;
  do {
    status = ::waitpid(proc, p_stat, options);
  } while (status < 0 && errno == EINTR);
  
  return((int)status);
}

int
SdmSystemUtility::SafeWait(int * p_stat)
{
  pid_t status;
  do {
    status = ::wait(p_stat);
  } while (status < 0 && errno == EINTR);
  
  return((int)status);
}

int
SdmSystemUtility::SafeUTime(const char * path, utimbuf * ntime)
{
  int status;
  do {
    status = ::utime(path, ntime);
  } while (status < 0 && errno == EINTR);
  
  return(status);
}

static void * time_mutex = NULL;

void
SdmSystemUtility::SafeCtime(const time_t *clock, char * buf, int buflen)
{
#if defined(POSIX_THREADS)
  
  if (!time_mutex) {
    time_mutex = ::mutexInit();
  }
  
  ::mutexLock lock_scope(time_mutex);
  
  ::ctime_r(clock, buf, buflen);
#else
  ::strcpy(buf, ctime(clock));
#endif
}

void
SdmSystemUtility::SafeLocaltime(const time_t *clock, tm & result)
{
#if defined(POSIX_THREADS)
  ::localtime_r(clock, &result);
#else
  result = *::localtime(clock);
#endif
}

time_t
SdmSystemUtility::SafeMktime(tm * timeptr)
{
#if defined(POSIX_THREADS)
  
  if (!time_mutex) {
    time_mutex = ::mutexInit();
  }
  
  ::mutexLock lock_scope(time_mutex);
#endif
  
  return(::mktime(timeptr));
}

size_t
SdmSystemUtility::SafeStrftime(char * buf, size_t buf_size,
			       const char * format, const tm * timeptr)
{
#if defined(POSIX_THREADS)
  
  if (!time_mutex) {
    time_mutex = ::mutexInit();
  }
  
  ::mutexLock lock_scope(time_mutex);
#endif
  
  return(::strftime(buf, buf_size, format, timeptr));
}

void
SdmSystemUtility::GetGroupName(char * grp_name)
{
#if defined(POSIX_THREADS)
  char buf[1024];
  group	gbuf;
  group	*grp = &gbuf;
  
  grp = ::getgrgid_r(getegid(), &gbuf, buf, 1024);
  
#else
  
  group *grp;
  
  grp = ::getgrgid(getegid());
  
#endif
  
  if (grp) {
    ::strcpy(grp_name, grp->gr_name);
  }
  else {
    ::strcpy(grp_name, "UNKNOWN_GROUP");
  }
  
}

gid_t
SdmSystemUtility::GetIdForGroupName(char * grp_name)
{
  assert(grp_name);
  
#if defined(POSIX_THREADS)
  char buf[1024];
  group gbuf;
  group *grp = &gbuf;
  
  grp = ::getgrname_r(grp_name, &gbuf, buf, 1024);
#else
  group *grp;
  
  grp = ::getgrnam(grp_name);
#endif
  
  return(grp ? grp->gr_gid : (gid_t)-1);
}

// static method to decode the "gecos" field of the password file
// This code was adapted from the Solaris 2.5.1 FCS version of finger
// from /ws/on596-gate/usr/src/cmd/cmd-inet/usr.bin/finger.c

static char* decodePwGecosField(const char* pwgecos, const char* pwname)
{
  char* realname = 0;
  char buffer[512];
  memset(&buffer, 0, 512);
  char *bp;

  assert(pwgecos);
  assert(pwname);

  realname = 0;
  bp = buffer;
  if (*pwgecos == '*')				// Leading '*'s in real name are ignored
    pwgecos++;
  while (*pwgecos && *pwgecos != ',')		// Stop processing data at first "," (the "gecos separator")
    if (*pwgecos == '&') {			// A "&" means substitute the user name in place of the "&"
      const char* lp = pwname;
      if (*lp)
	*bp++ = islower(*lp) ? toupper(*lp++) : *lp++;	// If first character is lower case, make it upper case (how nice...)
      while (*bp++ = *lp++)
	;
      bp--;
      pwgecos++;
    } else
      *bp++ = *pwgecos++;
  *bp++ = 0;
  realname = ::strdup(buffer[0] == '\0' ? pwname : buffer);
  assert(::strlen(realname));
  return(realname);
}

// Get the password entry for the user
// Caches the entry so its only fetched once
// Fixes up the "gecos" data field to take into account the "&" specifier
void
SdmSystemUtility::GetPasswordEntry(passwd & result)
{
  static struct passwd passwordEntry;
  static char passwordData[1024];
  static int oneTimeFlag = 0;

  SdmMutexEntry entry(_passwordEntryMutex);

  if (!oneTimeFlag) {
    struct passwd *dummy;

    dummy = ::getpwuid_r(getuid(), &passwordEntry, passwordData, sizeof(passwordData));
    assert(dummy);
    passwordEntry.pw_gecos = decodePwGecosField(passwordEntry.pw_gecos, passwordEntry.pw_name);
    oneTimeFlag++;
  }

  ::memcpy(&result, &passwordEntry, sizeof(struct passwd));
}

char *
SdmSystemUtility::GetQuotedPersonalName()
{
  static char *gecos;
  static int oneTimeFlag = 0;

  SdmMutexEntry entry(_personalNameMutex);

  if (!oneTimeFlag) {
    SdmMailRc* mrc = SdmMailRc::GetMailRc();
    SdmError localError;
    passwd pw;
    char *p, *s;
    int len = 0;

    // Determine source of personal name for the user
    // If the .mailrc variable "mypersonalname" is defined, use that; otherwise
    // If the .mailrc variable "myfullname" is defined, use that; otherwise, 
    // use the GECOS field from the passwd file
    char* mypersonalname = 0;
    localError.Reset();
    mrc->GetValue(localError, "mypersonalname", &mypersonalname);
    if (localError == Sdm_EC_Success && mypersonalname) {
      p = gecos = mypersonalname;
    } else {
      char* myfullname = 0;
      localError.Reset();
      mrc->GetValue(localError, "myfullname", &myfullname);
      if (localError == Sdm_EC_Success && myfullname) {
	p = gecos = myfullname;
      }
      else {
	GetPasswordEntry(pw);
	p = gecos = pw.pw_gecos;
      }
    }

    // Look for any character that rfc822 states cannot appear unquoted
    if (strpbrk(p, "()<>@,;:\".[]")) {
      char *p1 = p;
      for (p = p1; *p; p++, len++) {
        if (*p == '"' || *p == '\\') {
          ++len;
        }
      }

      gecos = (char*)malloc(len + 2 + 1);  // Starting and trailing quote + NULL terminator
      assert(gecos);

      s = gecos;
      *s++ = '"';
      for (p = p1; *p; p++) {
        if (*p == '"' || *p == '\\')
          *s++ = '\\';
        *s++ = *p;
      }
      *s++ = '"';
      *s = '\0';
    }

    oneTimeFlag++;
  }

  return gecos;
}

char *
SdmSystemUtility::GetHostName()
{
  static struct utsname name;
  static int oneTimeFlag = 0;

  SdmMutexEntry entry(_hostNameMutex);

  if (!oneTimeFlag) {
    ::uname(&name);
    oneTimeFlag++;
  }

  return name.nodename;
}

int
SdmSystemUtility::Random()
{
  SdmMutexEntry entry(_randomMutex);

  return ::rand();
}

/*
 * Read up a line from the specified input into the line
 * buffer.  Return the number of characters read.  Do not
 * include the newline at the end.
 */

SdmSystemUtility::ReadLine(FILE *ibuf, char *linebuf, unsigned long lineSize)
{
  register char *cp;
  register int c;
  int seennulls = 0;

  clearerr(ibuf);
  c = getc(ibuf);

  for (cp = linebuf; c != '\n' && c != EOF; c = getc(ibuf)) {
    if (c == 0) {
        if (!seennulls) {
         SdmUtility::LogError(Sdm_False, "ignoring NULL characters in mail\n");
          seennulls++;
        }
        continue;
    }
    if (cp - linebuf < 10240-2)
        *cp++ = c;
  }

  *cp = 0;
  if (c == EOF && cp == linebuf)
    return(0);

  return(cp - linebuf + 1);
}

static char *lcase (char *s)
{
  char c;
  char *ret = s;
  /* if uppercase covert to lower */
  for (; c = *s; s++) if (isupper (c)) *s += 'a'-'A';
  return (ret);                 /* return string */
}

SdmErrorCode SdmSystemUtility::SmtpServiceVerify(SdmError& err, char *host, long port)
{
  return(SdmSystemUtility::TcpServiceVerify(err, host, port ? NULL : "smtp", port ? port : 25));
}

SdmErrorCode SdmSystemUtility::ImapServiceVerify(SdmError& err, char *host, long port)
{
  return(SdmSystemUtility::TcpServiceVerify(err, host, port ? NULL : "imap", port ? port : 143));
}

SdmErrorCode SdmSystemUtility::TcpServiceVerify(SdmError& err, char *host, char *service, long port)
{
  int i,sock;
  char *s;
  struct sockaddr_in sin;
  struct hostent *host_name;
  char hostname[MAXPATHLEN];
  struct protoent *pt = ::getprotobyname ("ip");
  struct servent *sv = service ? ::getservbyname (service,"tcp") : NULL;
  if (s = ::strchr (host,':')) {	/* port number specified? */
    *s++ = '\0';		/* yes, tie off port */
    port = ::strtol (s,&s,10);	/* parse port */
    if (s && *s) {
      err = Sdm_EC_NET_BadPortNumberSpecified;	// "Junk after port number", s
      return(err);
    }
    sin.sin_port = htons (port);
  }
  /* copy port number in network format */
  else sin.sin_port = sv ? sv->s_port : htons (port);
  /* The domain literal form is used (rather than simply the dotted decimal
     as with other Unix programs) because it has to be a valid "host name"
     in mailsystem terminology. */
  /* look like domain literal? */
  if (host[0] == '[' && host[(::strlen (host))-1] == ']') {
    ::strcpy (hostname,host+1);	/* yes, copy number part */
    hostname[(::strlen (hostname))-1] = '\0';
    if ((sin.sin_addr.s_addr = inet_addr (hostname)) != -1) {
      sin.sin_family = AF_INET;	/* family is always Internet */
      ::strcpy (hostname,host);	/* hostname is user's argument */
    }
    else {
      err = Sdm_EC_NET_HostNameSyntaxError;	// Bad format domain-literal: %.80s",host
      return(err);
    }
  }
  else {			/* lookup host name, note that brain-dead Unix
				   requires lowercase! */
    ::strcpy (hostname,host);	/* in case host is in write-protected memory */
    if ((host_name = ::gethostbyname (lcase (hostname)))) {
      /* copy address type */
      sin.sin_family = host_name->h_addrtype;
      /* copy host name */
      /* Do not use possibly fully-qualified hostname under solaris.
       * We keep what the user passed to maintain consistency.
       *   WJY 10 juin 1996 */
      /* strcpy (hostname,host_name->h_name); */
      /* copy host addresses */
      ::memcpy (&sin.sin_addr,host_name->h_addr,host_name->h_length);
    }
    else {
      err = Sdm_EC_NET_HostDoesNotExist;	// No such host as %.80s",host
      return(err);
    }
  }
  /* get a TCP stream */
  if ((sock = ::socket (sin.sin_family,SOCK_STREAM,pt ? pt->p_proto : 0)) < 0) {
    err = Sdm_EC_NET_CannotCreateConnection;		// Unable to create TCP socket: %s",strerror (errno)
    return(err);
  }
  /* open connection */
  while ((i = ::connect (sock,(struct sockaddr *)&sin,sizeof (sin))) < 0 &&
	 errno == EINTR);
  if ((i < 0) && (errno != EISCONN)) {
    err = Sdm_EC_NET_ServiceNotAvailable;		// Can't connect to %.80s,%d: %s",hostname,port,strerror (errno)
    return(err);
  }

  SafeClose(sock);
  return(err = Sdm_EC_Success);
}

// Returns indication of whether or not we are in the "C" locale

SdmBoolean SdmSystemUtility::InCLocale()
{
  const char* locale = SdmSystemUtility::GetCurrentLocaleName();
  assert(locale);
  return(  (strcasecmp(locale,"C")==0)  ? Sdm_True : Sdm_False);
}

// Returns the current locale that the program is running in

const char* SdmSystemUtility::GetCurrentLocaleName()
{
  static char* locale = NULL;

  SdmMutexEntry entry(_localeMutex);

  if (locale == NULL) {
    locale = setlocale(LC_CTYPE, NULL);
    if (locale == NULL)
      locale = "C";
  }

  return(locale);
}

