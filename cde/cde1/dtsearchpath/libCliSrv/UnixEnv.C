/*******************************************************************
**  (c) Copyright Hewlett-Packard Company, 1990, 1991, 1992, 1993.
**  All rights are reserved.  Copying or other reproduction of this
**  program except for archival purposes is prohibited without prior
**  written consent of Hewlett-Packard Company.
********************************************************************
****************************<+>*************************************/

#include "Environ.h"
#include "DirIterator.h"
#include <sys/stat.h>
#if defined(USL) || defined(__uxp__)
#define S_ISLNK(mode) ((mode & S_IFMT) == S_IFLNK)
#endif
#include <stdlib.h>
#include <string.h>
#include <iostream.h>
#include <stdio.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#if defined(USL) || defined(__uxp__)
#include <regexpr.h>
#else
#include <regex.h>
#endif
#include <errno.h>

#if defined(sun) || defined(_AIX) || defined(__osf__) || defined(USL) || defined(__uxp__)
#define UID_NO_CHANGE ((uid_t) -1)
#define GID_NO_CHANGE ((gid_t) -1)
#endif

UnixEnvironment::UnixEnvironment()
{
    dtMountPoint = getEnvironmentVariable("DTMOUNTPOINT");
    if (dtMountPoint.isNull())
#if defined(sun) || defined(USL) || defined(__uxp__)
	dtMountPoint = "/net/";
#else
	dtMountPoint = "/nfs/";
#endif

    CString temp = getEnvironmentVariable("MANPATH");
    if (temp.isNull())
#if defined(sun) || defined(USL) || defined(__uxp__)
	manpath = "/usr/share/man";
#elif defined(_AIX)
	manpath = "/usr/share/man:/usr/lpp/info";
#elif defined(hpux)
	manpath = "/usr/man:/usr/contrib/man:/usr/local/man";
#elif defined(__osf__)
        manpath = "/usr/share/man:/usr/local/man";
#endif
    else
	manpath = temp;

    lang = getEnvironmentVariable("LANG");
    if (lang.isNull())
	lang = "C";

    localHost = "localhost";

    CString sh(getEnvironmentVariable("SHELL"));

    if ( sh == ksh() )
        shell_ = new KShell();
    else if ( sh == csh() )
        shell_ = new CShell();
    else
        shell_ = new KShell();
        
    uid_ = getuid();
    gid_ = getgid();
}


UnixEnvironment::~UnixEnvironment()
{
	delete shell_;
}

int UnixEnvironment::FileExists 
	(
	const CString & filename
	) const
{
struct stat file;

    if (stat((const char *)filename.data(),&file) == 0)
	return 1;

    return 0;
}

void UnixEnvironment::MakeDirectory 
	(
	const CString & dirname,
	mode_t          permissions
	)
{
    if (mkdir (dirname.data(), permissions) == -1) {
	CString errorStr("MakeDirectory: " + dirname);
	perror(errorStr.data());
    }	
}

CString UnixEnvironment::getEnvironmentVariable 
	(
	const char * envvar
	)
{
  char * value = getenv(envvar);
  if (value == 0)
    return CString("");

  return CString(value);
}

int UnixEnvironment::isDirectory 
	(
	const CString & directory
	)
{
struct stat file;

    if (stat((const char *)directory.data(),&file) == 0)
	if (S_ISDIR(file.st_mode))
	    return 1;

    return 0;
}

int UnixEnvironment::isFile
	(
	const CString & filespec
	)
{
struct stat file;

    if (stat((const char *)filespec.data(),&file) == 0)
	if (S_ISREG(file.st_mode))
	    return 1;

    return 0;
}

int UnixEnvironment::isLink
	(
	const CString & filespec
	)
{
struct stat file;

    if (lstat((const char *)filespec.data(),&file) == 0)
	if (S_ISLNK(file.st_mode))
	    return 1;

    return 0;
}

void UnixEnvironment::symbolicLink
	(
	const CString & linkto,
	const CString & linkee
	)
{
    if (symlink (linkto.data(), linkee.data()) == -1) {
	CString errorStr("symbolicLink: " + linkee + " -> " + linkto + "| ");
	perror(errorStr.data());
    }
}

void UnixEnvironment::setUserId
	(
	const char * name
	)
{
uid_t uid = -1;

    if (name && name[0]) {
	struct passwd * pwent = getpwnam(name);
	uid = pwent->pw_uid;
    }
    else if (name == 0)
	uid = uid_;

    if (setuid(uid) == -1) {
	CString errorStr("setUserId: ");
	errorStr += name;
	errorStr += "| ";
	perror(errorStr.data());
    }
}

void UnixEnvironment::changePermissions
	(
	const CString & filespec,
	mode_t mode
	)
{
    if (chmod (filespec.data(), mode) == -1) {
	CString errorStr("changePermissions: " + filespec + "| ");
	perror(errorStr.data());
    }
}

void UnixEnvironment::changeOwnerGroup
	(
	const CString & filespec,
	const char *    owner,
	const char *    group
	)
{
uid_t uid = UID_NO_CHANGE;
gid_t gid = GID_NO_CHANGE;

    if (owner) {
	if (owner[0]) {
	    struct passwd * pwent = getpwnam(owner);
	    uid = pwent->pw_uid;
	}
	else
	    uid = getuid();
    }
    if (group) {
	if (group[0]) {
	    struct group * grent = getgrnam(group);
	    gid = grent->gr_gid;
	}
	else
	    gid = getgid();
    }

    if (chown(filespec.data(),uid,gid) == -1) {
	CString errorStr("changeOwnerGroup: " + filespec + "| ");
	perror(errorStr.data());
    }
}

void UnixEnvironment::removeDirectory 
	(
	const CString & dirspec
	)
{
    if (rmdir (dirspec.data()) == -1) {
	CString errorStr("removeDirectory: " + dirspec + "| ");
	perror(errorStr.data());
    }
}

void UnixEnvironment::removeFiles 
	(
	const CString & dirspec,
	const CString & filespec
	)
{
#if defined(USL) || defined(__uxp__)
char buffer[100];
    sprintf(buffer,"/bin/rm -rf %s/%s", dirspec.data(),filespec.data());
    system(buffer);

#else

    DirectoryIterator dir(dirspec);
    struct dirent * direntry;
    while (direntry = dir()) {
	if (strcmp(direntry->d_name,".") == 0 ||
	    strcmp(direntry->d_name,"..") == 0)
		continue;
	if (filespec == "[.]*") {
	    removeFile(dirspec + "/" + direntry->d_name);
	    continue;
	}
# ifdef should_be_sun_but_this_dont_work
	char * re = NULL;
	re = compile (filespec.data(), NULL, NULL);
	if (step (direntry->d_name,re)) {
# else
	regex_t re;
	regcomp (&re, filespec.data(), 0);
	if (regexec (&re, direntry->d_name, 0, NULL, 0) == 0) {
# endif
	    removeFile(dirspec + "/" + direntry->d_name);
	}
    }
#endif
}

void UnixEnvironment::removeFile 
	(
	const CString & filespec
	)
{
    if (isDirectory(filespec) && !isLink(filespec))
	removeDirectory(filespec);
    else {
        if (unlink (filespec.data()) == -1) {
	    CString errorStr("removeFile(unlink): " + filespec + "| ");
	    perror(errorStr.data());
	}
    }
}

void UnixEnvironment::removeDeadLinks 
	(
	const CString & dirspec
	)
{
    DIR * dir = opendir(dirspec.data());
    struct dirent * direntry;
    while (direntry = readdir(dir)) {
	if (isLink(dirspec + "/" + direntry->d_name))
	    if (!isFile(dirspec + "/" + direntry->d_name) &&
		!isDirectory(dirspec + "/" + direntry->d_name))
	        removeFile(dirspec + "/" + direntry->d_name);
    }
    closedir(dir);	    
}

