/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Utils Base class.

#ifndef _SDM_SYSTEMUTILITY_H
#define _SDM_SYSTEMUTILITY_H

#pragma ident "@(#)SystemUtility.hh	1.16 97/01/29 SMI"

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Error.hh>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <utime.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/utsname.h>

// Forward declaration for _SdmRetrieveHardwarePageSize, so we can declare it
// as a friend of this class..

static void _SdmRetrieveHardwarePageSize();

class SdmSystemUtility {

public:
  static int SafeOpen(const char * path, int oflag, mode_t mode = 0644);
  static off_t SafeLseek(int fd, off_t offset, int whence);
  static int SafeClose(int fd);
  static int SafeRename(const char * oldPath, const char * newPath);
  static int SafeLink(const char * existingPath, const char * newPath);
  static int SafeRemove(const char * path);
  static int SafeUnlink(const char * path);
  static ssize_t SafeRead(int fd, void * buf, size_t bytes);
  static ssize_t SafeWrite(int fd, const void * buf, size_t bytes);
  static ssize_t SafeWriteStrip(int fd, const void * buf, size_t bytes);
  static unsigned long SafeWritev(int fd, struct iovec *iov, int iovcnt);
  static int SafeStat(const char * path, struct stat * buf);
  static int SafeLStat(const char * path, struct stat * buf);
  static int SafeFStat(int fd, struct stat * buf);
  static int SafeGuaranteedStat(const char * path, struct stat * buf);
  static int SafeTruncate(const char * path, off_t len);
  static int SafeFTruncate(int fd, off_t len);
  static int SafeAccess(const char * path, int amode);
  static int SafeLockf(int fd, int func, long size);
  static int SafeFchown(int fd, uid_t owner, gid_t group);
  static int SafeFChmod(int fd, mode_t mode);
  static int SafeDup2(int fd1, int fd2);
  static int SafeExecvp(const char * file, char *const *argv);
  static int SafeWaitpid(pid_t proc, int * status, int options);
  static int SafeWait(int * status);
  static int SafeUTime(const char * path, utimbuf * ntime);
  static int SafeFsync(int fd);

  // The time routines deal with MT and reentrant issues in a platform
  // independent way.
  //
  static void SafeCtime(const time_t *clock, char * buf, int buflen = 26);
  static void SafeLocaltime(const time_t *clock, tm & result);
  static time_t SafeMktime(tm * timeptr);
  static size_t SafeStrftime(char * buf, size_t buf_size,
          const char * format, const tm * timeptr);

  // The group and password routines will retrieve the group and
  // password names in an MT-Safe way.
  //
  static void GetGroupName(char * grp_name);
  static gid_t GetIdForGroupName(char * grp_name);
  static void GetPasswordEntry(passwd & result);
  static char *GetHostName();
  static char *GetQuotedPersonalName();
  static int Random();
  static const char* GetCurrentLocaleName();
  static SdmBoolean InCLocale();
  static int ReadLine(FILE *ibuf, char *linebuf, unsigned long lineSize);
  static SdmErrorCode TcpServiceVerify(SdmError& err, char *host, char *service, long port);
  static SdmErrorCode ImapServiceVerify(SdmError& err, char *host, long port);
  static SdmErrorCode SmtpServiceVerify(SdmError& err, char *host, long port);
  static SdmBoolean GroupPrivilegeLockoutControl(SdmBoolean enableFlag);
  static HardwarePageSize() { return (SdmSystemUtility::_hardwarePageSize); }

private:
  friend void _SdmRetrieveHardwarePageSize();
  // this class should never be instantiated.  clients should
  // call static methods directly.
  SdmSystemUtility();
  ~SdmSystemUtility();

  static mutex_t _localeMutex;
  static mutex_t _randomMutex;
  static mutex_t _passwordEntryMutex;
  static mutex_t _personalNameMutex;
  static mutex_t _hostNameMutex;
  static mutex_t _groupPrivilegeMasterMutex;
  static mutex_t _groupPrivilegeOperationMutex;
  static SdmBoolean _groupPrivilegeEnabled;
  static int _hardwarePageSize;
};

#endif
