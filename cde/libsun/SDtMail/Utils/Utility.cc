/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Utils base class.

#pragma ident "@(#)Utility.cc	1.50 97/05/13 SMI"

// Include Files
#include <SDtMail/Utility.hh>
#include <SDtMail/SystemUtility.hh>
#include <string.h>
#include <ctype.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/param.h>
#include <SDtMail/MailRc.hh>
#include <DataStructs/MutexEntry.hh>
#include <nl_types.h>
#include <errno.h>

static const int MAXIMUM_PATH_LENGTH = 2048;
static const char* MAILFOLDER = "~/Mail/";

SdmVector<SdmObjectKey>	SdmUtility::_ValidKeys;
SdmObjectKey SdmUtility::_CurrentKey = 0;

static mutex_t* gKeysLock = NULL;

//
// InitializeMutexes function is used to initialize 
// the mutex in SdmUtility before the program starts 
// execution.  The pragma belows specifies that this 
// function is an initialization function.  
//
// NOTE that this pragma is specific to Solaris 2.x only.
// 
static void SdmInitMutex()
{
  gKeysLock = new mutex_t;
  int rc;
  rc = mutex_init(gKeysLock, USYNC_THREAD, NULL);
  assert (rc == 0);
  assert (gKeysLock != NULL);
}
#pragma init(SdmInitMutex)


const char* SdmUtility::kFolderResourceName = "folder";
const char* SdmUtility::kImapFolderResourceName = "imapfolder";


SdmUtility::SdmUtility()
{
}

SdmUtility::~SdmUtility()
{
}

SdmErrorCode SdmUtility::ExpandAndMakeAbsolutePath(SdmError& err, SdmString& r_pathName, const SdmString& pathName, const SdmBoolean followLinks)
{
  err = Sdm_EC_Success;

  char *tempExpandedPath = 0;
  SdmMailRc* mrc = SdmMailRc::GetMailRc();

  // expand mailbox file name
  // Must be careful: tempExpandedpath is filled in with a malloc()ed pointer that
  // we must free before we return...

  if (SdmUtility::ExpandPath(err, tempExpandedPath, (const char *)pathName, *mrc) != Sdm_EC_Success) {
    if (err == Sdm_EC_Fail)
      err.SetMajorErrorCode(Sdm_EC_CannotAccessMessageStore);
    else
      err.AddMajorErrorCode(Sdm_EC_CannotAccessMessageStore);
    return(err);
  }
  assert(tempExpandedPath);

  // Now, examine the name returned - if it does NOT begin with a /, then we
  // need to prepend our current working directory so as to make the path
  // name absolute

  if (*tempExpandedPath == '/') {
    // Already absolute, simply assign to return string and return
    r_pathName = tempExpandedPath;
  }
  else {
    // NOT absolute - get current working directory and append
    char* cwdp = getcwd(NULL, MAXPATHLEN);
    if (!cwdp) {
      int lerrno = errno;
      err.SetMajorErrorCodeAndMessage(Sdm_EC_CannotDetermineCurrentWorkingDirectory, 
				      (const char *)SdmUtility::ErrnoMessage(lerrno));
      return(err);
    }
    r_pathName = cwdp;
    r_pathName += ((const char *)"/");
    // now append the mailbox path: if the path begins with a ./ then pass it
    // by as we have just appended an absolute path to this directory; othewise,
    // accept the mailbox path as it
    if (tempExpandedPath[0] == '.' && tempExpandedPath[1] == '/') {
      r_pathName += ((const char *)tempExpandedPath+2);
    }
    else {
      r_pathName += ((const char *)tempExpandedPath);
    }
  }
  free((void *)tempExpandedPath);

  // Now, we need to see if the target of this is a link and if so chase down
  // all of the links so that we come up with the real, absolute path name

  if (followLinks == Sdm_True) {
    struct stat buf;
    int lerrno;
    if (SdmSystemUtility::SafeLStat((const char *)r_pathName, &buf) == 0) {
      char real_path[MAXPATHLEN+1];
      char link_path[MAXPATHLEN+1];
      strcpy(real_path, (const char *)r_pathName);
      strcpy(link_path, (const char *)r_pathName);
      while(S_ISLNK(buf.st_mode)) {
	int size = ::readlink(link_path, real_path, sizeof(real_path));
	if (size < 0) {
	  lerrno = errno;
	  err.SetMajorErrorCodeAndMessage(Sdm_EC_CannotAccessMailboxPathNameComponent,
					  (const char *)r_pathName, 
					  real_path, 
					  (const char *)SdmUtility::ErrnoMessage(lerrno));
	  return(err);
	}

	real_path[size] = 0;
	if (real_path[0] == '/') {
	  strcpy(link_path, real_path);
	}
	else {
	  char * last_slash = strrchr(link_path, '/');
	  if (last_slash) {
	    strcpy(last_slash + 1, real_path);
	  }
	  else {
	    strcpy(link_path, "./");
	    strcat(link_path, real_path);
	  }
	}

	strcpy(real_path, link_path);

	if (SdmSystemUtility::SafeLStat(link_path, &buf)) {
	  lerrno = errno;
	  err.SetMajorErrorCodeAndMessage(Sdm_EC_CannotAccessMailboxPathNameComponent,
					  (const char *)r_pathName, 
					  link_path,
					  (const char *)SdmUtility::ErrnoMessage(lerrno));
	  return(err);
	}
      }
      r_pathName = (const char *)real_path;
    }
  }

  return(err);  
}


SdmErrorCode
SdmUtility::ExpandPath(SdmError &err, char *&r_absolutePath,
                  const char *relativePath, const SdmMailRc &mail_rc,
                  const char *folderResourceName)
{
  char * fold_path = NULL;
  err = Sdm_EC_Success;

  if (relativePath == NULL) {
    err = Sdm_EC_BadArgument;
    r_absolutePath = NULL;
    return err;
  }

  char * exp_name = (char *)malloc(MAXIMUM_PATH_LENGTH);
  if (exp_name == NULL)  {
    err = Sdm_EC_NoMemory;
    r_absolutePath = NULL;
    return err;
  }

  // Expand env variables
  if (strchr(relativePath, '$') != NULL) {
    // I stole this code from mailtool (NH)
    char *ptr = (char*)relativePath, *save_p, *b_p = exp_name;
    char env[256], *e_p;
    while (*ptr) {
      if (*ptr == '$') {
	/* Expand environment variable */
	save_p = ptr;
	e_p = env;
	ptr++;
	while ((isalnum(*ptr) || *ptr == '_') && e_p - env < 256) {
	  *e_p++ = *ptr++;
	}
	*e_p = NULL;
	if (e_p = (char *) getenv((char *)env)) {
	  while (*e_p) {
	    *b_p++ = *e_p++;
	  }
	} else {   
	  ptr = save_p;
	  *b_p++ = *ptr++;
	}
      } else {
	*b_p++ = *ptr++;
      }
    }
    *b_p = NULL;
  }
  else 
    {
      strcpy(exp_name, relativePath);
    }

  char * exp_name2 = (char *)malloc(MAXIMUM_PATH_LENGTH);
  exp_name2[0] = '\0';

  switch (exp_name[0]) {
  case '+':
    // This is relative to the folder relativePath. Figure out what that is.
    {
      SdmMailRc *n_MailRc = (SdmMailRc*)&mail_rc;
      if (folderResourceName) {
        n_MailRc->GetValue(err, folderResourceName, &fold_path);
      }
      if (!err && folderResourceName) {
        if (*fold_path != '/' && *fold_path != '.' && 
	    *fold_path != '~' && *fold_path != '$') 
	  {
	    strcpy(exp_name2, "~/");
	  }

        strcat(exp_name2, fold_path);
        strcat(exp_name2, "/");
      }
      else // Use the default folder
	{
	  strcpy(exp_name2, MAILFOLDER);
	}

      strcat(exp_name2, &exp_name[1]);

      // free memory for fold_path since  we don't keed it any more.
      if (fold_path)
	free(fold_path);

      // We need to call ourselves again to deal with 
      // relative paths in the folder directory.
      // 
      char * old_exp = exp_name2;
      exp_name2 = NULL;
      ExpandPath(err, exp_name2, old_exp, mail_rc, folderResourceName);
      free(old_exp);
      break;
    }

  case '~':
    // This is relative to the user's home directory.
    {
      passwd pw;
      const char * start;

      // if ~/ or ~ is specified, use the users home directory as a reference

      if (exp_name[1] == '/' || exp_name[1] == '\0') {
        SdmSystemUtility::GetPasswordEntry(pw);
        start = &exp_name[1];
      }
      else {
        passwd * pw_p;

        char * slash = strchr(&exp_name[1], '/');
        if (slash == NULL) {
	  // User specified "~blah" - must be followed by a path to be valid

	  err.SetMajorErrorCode(Sdm_EC_CannotAccessMessageStore);
	  err.AddMinorErrorCodeAndMessage(Sdm_EC_OBJ_HomeDirPathMissingMessageStoreName, exp_name);
	  break;
        }

        int len = slash - &exp_name[1];
        char * name = new char[len + 1];
        memset(name, 0, len+1);
        strncpy(name, &exp_name[1], len);
        name[len] = 0;
        pw_p = getpwnam(name);

        if (!pw_p) {
	  err.SetMajorErrorCode(Sdm_EC_CannotAccessMessageStore);
	  err.AddMinorErrorCodeAndMessage(Sdm_EC_OBJ_UserNameNotFound, name, exp_name);
          break;
        }
        pw = *pw_p;

        delete name;

        start = slash;
      }

      strcpy(exp_name2, pw.pw_dir);
      strcat(exp_name2, start);
      break;
    }

    // We have a directory or no specials. Just copy the relativePath and
    // return.
  case '.':
  case '/':
  default:
    strcpy(exp_name2, exp_name);
    break;
  }

  free(exp_name);
  r_absolutePath = (char *)realloc((void*)exp_name2, strlen(exp_name2)+1);
  return(err);
}

SdmErrorCode
SdmUtility::GetRelativePath(SdmError & err, char *&r_relativePath, 
  const char * path, const SdmMailRc &mail_rc, const char* folderResourceName)
{
    char * fold_path = NULL;
    
    err = Sdm_EC_Success;

    if (path == NULL) {
      err = Sdm_EC_BadArgument;
      r_relativePath = NULL;
      return err;
    }

    char * exp_name = (char *)malloc(MAXIMUM_PATH_LENGTH);
    if (!exp_name) {
      err = Sdm_EC_NoMemory;
      r_relativePath = NULL;
      return(err);
    }
    exp_name[0] = '\0'; // Just for errors.

    switch (path[0]) {
      case '/':
        // This is an absolute path, so there is a chance that
        // we can trim it down to a relative path if it goes down
        // the same way as the folder path.
      {
        
        SdmMailRc *n_MailRc = (SdmMailRc*)&mail_rc;
        if (folderResourceName) {
          n_MailRc->GetValue(err, folderResourceName, &fold_path);
        }

        if (err || folderResourceName == NULL) {
          // Check out the default folder
          strcpy(exp_name, MAILFOLDER);
        }
        else {
          // Make sure it is fully qualified and relative to home dir. 
          if (fold_path[0] != '/') 
            sprintf(exp_name, "~/%s", fold_path);
          else
            strcpy(exp_name, fold_path);

          // Make sure it ends in / so we can do a perfect match
          if (exp_name[strlen(exp_name)-1] != '/') {
            strcat(exp_name, "/");
          }
        }

        if (fold_path) 
          free(fold_path);

        // We need to call ourselves again to deal with 
        // relative paths in the folder directory.
        // 
        char * old_exp = exp_name;
        if (ExpandPath(err, exp_name, old_exp, mail_rc, folderResourceName)) {
          free(old_exp);
          return err;
        } else {
          free(old_exp);
        }

        // Check to see if the path starts with the folder path.
        char * matched_path = strstr(path, exp_name);
	exp_name = (char *)realloc(exp_name, MAXIMUM_PATH_LENGTH);
        if (matched_path == path) {
          // Make it a relative path to the folder dir.
          int folder_path_length = strlen(exp_name);
          strcpy(exp_name, &path[folder_path_length]);
        } else { 
          strcpy(exp_name, path);
        }
        err = Sdm_EC_Success;
        break;
      }
      case '+':
          // This is relative to the folder path. Leave it alone.
          // The only time we are likely to see a leading '+' is
          // when the path was carried over from mailtool in the .mailrc.
      case '~':
          // This is relative to the user's home directory.  Leave it alone.
          // The only time we are likely to see a leading '~' is
          // when the path was carried over from mailtool in the .mailrc.
      case '.':
          // This is relative to the current directory where dtmail is
          // running.  Leave it alone.  The only time we are likely to see
          // a leading '.' is when the path was carried over from mailtool
          // in the .mailrc.
      default:
      {
          strcpy(exp_name, path);
          break;
      }
    }
    r_relativePath = (char *)realloc((void*)exp_name, strlen(exp_name)+1);
    return err;
}


SdmBoolean
SdmUtility::ValidObjectKey(SdmObjectKey key)
{
    assert (gKeysLock != NULL);
    SdmMutexEntry entry(*gKeysLock);
    return(_ValidKeys.FindElement(key) < 0 ? Sdm_False : Sdm_True);
}

SdmObjectKey
SdmUtility::NewObjectKey(void)
{
    assert (gKeysLock != NULL);
    SdmMutexEntry entry(*gKeysLock);

    _CurrentKey += 1;
    _ValidKeys.AddElementToList(_CurrentKey);

    return(_CurrentKey);
}

void
SdmUtility::RemoveObjectKey(SdmObjectKey key)
{
    assert (gKeysLock != NULL);
    SdmMutexEntry entry(*gKeysLock);

    int slot = _ValidKeys.FindElement(key);
    if (slot >= 0) {
      _ValidKeys.RemoveElement(slot);
    }
}


// Provide interface to the DtSvc function DtSimpleError
// When this interface is better defined, this can be removed
// and replaced with the appropriate include file

typedef enum {
  DtIgnore,
  DtInformation,
  DtWarning,
  DtError,
  DtFatalError,
  DtInternalError
} DtSeverity;


extern "C" void _DtSimpleError( 
                        char *progName,
                        DtSeverity severity,
                        char *help,
                        char *format,
                        ...) ;

//
// we define _DtSimpleError as a weak symbol so that libSDtMail and it's
// clients are not required to link in the Dt Service library.  If client
// chooses to link in this library, then _DtSimpleError will be used for
// logging errors (see SdmUtility::LogError below).  If not linked in,
// then the error will be printed to standard error.
//
#pragma weak _DtSimpleError


void
SdmUtility::LogError(SdmBoolean criticalError, const char *format, ...)
{
  const int MessageBufferSize = 4096;
  char messageBuffer[MessageBufferSize+1];
  memset (&messageBuffer, 0, MessageBufferSize+1);

  va_list	var_args;

  // format the error message.
  va_start(var_args, format);
  (void) vsprintf(messageBuffer, format, var_args);
  assert(strlen(messageBuffer) < MessageBufferSize);
  va_end(var_args);

  // if _DtSimpleError symbol is defined, call this function.  Otherwise,
  // print error message to standard error.  
  if (_DtSimpleError) {
    _DtSimpleError("libSDtMail", criticalError  == Sdm_True ? DtError : DtWarning,
		   NULL, messageBuffer);
  }
  
  fprintf(stderr, "%s: %s: %s", "libSDtMail", 
	  criticalError  == Sdm_True ? "Error" : "Warning",
	  messageBuffer);
}

// This function takes a system errno code and returns a const char* to the error
// message string for that error messages - used when the message catalog fails to
// provide the error message in question

static const char* _convertErrnoToErrorMessage(int systemErrorNumber)
{
  const char* the_message = NULL;

  switch (systemErrorNumber) {
  case EPERM:
    return((const char *)"You do not have the permissions to access this mailbox, folder, or file.");
  case ENOENT:
    return((const char *)"The mailbox, folder, or file does not exist.");
  case ESRCH:
    return((const char *)"No such process.");
  case EINTR:
    return((const char *)"interrupted system call.");
  case EIO:
    return((const char *)"A read or write error occurred with this mailbox, folder, or file.");
  case ENXIO:
    return((const char *)"No such device or address.");
  case E2BIG:
    return((const char *)"Arg list too long.");
  case ENOEXEC:
    return((const char *)"Exec format error.");
  case EBADF:
    return((const char *)"Bad file number.");
  case ECHILD:
    return((const char *)"No children.");
  case EAGAIN:
    return((const char *)"Resource temporarily unavailable.");
  case ENOMEM:
    return((const char *)"Your system is out of memory.");
  case EACCES:
    return((const char *)"You do not have the permissions to access this mailbox, folder, or file.");
  case EFAULT:
    return((const char *)"Bad address.");
  case ENOTBLK:
    return((const char *)"Block device required.");
  case EBUSY:
    return((const char *)"Mount device busy.");
  case EEXIST:
    return((const char *)"File exists.");
  case EXDEV:
    return((const char *)"Cross-device link.");
  case ENODEV:
    return((const char *)"No such device.");
  case ENOTDIR:
    return((const char *)"You specified a file name in error.  Specify a directory.");
  case EISDIR:
    return((const char *)"You specified a directory name in error.  Specify a file.");
  case EINVAL:
    return((const char *)"Invalid argument.");
  case ENFILE:
    return((const char *)"You have opened the maximum number of files allowed.");
  case EMFILE:
    return((const char *)"You have reached the maximum number of files you can open on your system.");
  case ENOTTY:
    return((const char *)"Inappropriate ioctl for device.");
  case ETXTBSY:
    return((const char *)"Text file busy.");
  case EFBIG:
    return((const char *)"File too large.");
  case ENOSPC:
    return((const char *)"You have run out of disk space on your system.");
  case ESPIPE:
    return((const char *)"Illegal seek.");
  case EROFS:
    return((const char *)"The mailbox, folder, or file you specified is on a disk or network device\nfor which you do not have write permission.");
  case EMLINK:
    return((const char *)"Too many links.");
  case EPIPE:
    return((const char *)"Broken pipe.");
  case EDOM:
    return((const char *)"Math arg out of domain of func.");
  case ERANGE:
    return((const char *)"Math result not representable.");
  case ENOMSG:
    return((const char *)"No message of desired type.");
  case EIDRM:
    return((const char *)"Identifier removed.");
  case ECHRNG:
    return((const char *)"Channel number out of range.");
  case EL2NSYNC:
    return((const char *)"Level 2 not synchronized.");
  case EL3HLT:
    return((const char *)"Level 3 halted.");
  case EL3RST:
    return((const char *)"Level 3 reset.");
  case ELNRNG:
    return((const char *)"Link number out of range.");
  case EUNATCH:
    return((const char *)"Protocol driver not attached.");
  case ENOCSI:
    return((const char *)"No CSI structure available.");
  case EL2HLT:
    return((const char *)"Level 2 halted.");
  case EDEADLK:
    return((const char *)"Deadlock condition.");
  case ENOLCK:
    return((const char *)"No record locks available.");
  case ECANCELED:
    return((const char *)"Operation canceled.");
  case ENOTSUP:
    return((const char *)"Operation not supported.");
#ifndef EDQUOT
#define EDQUOT 49
#endif
  case EDQUOT:
    return((const char *)"You have exceeded the disk limit for your system.");
  case EBADE:
    return((const char *)"invalid exchange.");
  case EBADR:
    return((const char *)"invalid request descriptor.");
  case EXFULL:
    return((const char *)"exchange full.");
  case ENOANO:
    return((const char *)"no anode.");
  case EBADRQC:
    return((const char *)"invalid request code.");
  case EBADSLT:
    return((const char *)"invalid slot.");
  case EDEADLOCK:
    return((const char *)"file locking deadlock error.");
  case EBFONT:
    return((const char *)"bad font file fmt.");
  case ENOSTR:
    return((const char *)"Device not a stream.");
  case ENODATA:
    return((const char *)"no data (for no delay io).");
  case ETIME:
    return((const char *)"timer expired.");
  case ENOSR:
    return((const char *)"out of streams resources.");
  case ENONET:
    return((const char *)"Machine is not on the network.");
  case ENOPKG:
    return((const char *)"Package not installed.");
  case EREMOTE:
    return((const char *)"The object is remote.");
  case ENOLINK:
    return((const char *)"the link has been severed.");
  case EADV:
    return((const char *)"advertise error.");
  case ESRMNT:
    return((const char *)"srmount error.");
  case ECOMM:
    return((const char *)"Communication error on send.");
  case EPROTO:
    return((const char *)"Protocol error.");
  case EMULTIHOP:
    return((const char *)"multihop attempted.");
  case EBADMSG:
    return((const char *)"trying to read unreadable message.");
  case ENAMETOOLONG:
    return((const char *)"This path name has exceeded the maximum number of characters allowed.");
  case EOVERFLOW:
    return((const char *)"value too large to be stored in data type.");
  case ENOTUNIQ:
    return((const char *)"given log. name not unique.");
  case EBADFD:
    return((const char *)"f.d. invalid for this operation.");
  case EREMCHG:
    return((const char *)"Remote address changed.");
  case ELIBACC:
    return((const char *)"Can't access a needed shared lib.");
  case ELIBBAD:
    return((const char *)"Accessing a corrupted shared lib.");
  case ELIBSCN:
    return((const char *)".lib section in a.out corrupted.");
  case ELIBMAX:
    return((const char *)"Attempting to link in too many libs.");
  case ELIBEXEC:
    return((const char *)"Attempting to exec a shared library.");
  case EILSEQ:
    return((const char *)"Illegal byte sequence.");
  case ENOSYS:
    return((const char *)"Unsupported file system operation.");
  case ELOOP:
    return((const char *)"Symbolic link loop.");
  case ERESTART:
    return((const char *)"Restartable system call.");
  case ESTRPIPE:
    return((const char *)"if pipe/FIFO, don't sleep in stream head.");
  case ENOTEMPTY:
    return((const char *)"directory not empty.");
  case EUSERS:
    return((const char *)"Too many users (for UFS).");
  case ENOTSOCK:
    return((const char *)"Socket operation on non-socket.");
  case EDESTADDRREQ:
    return((const char *)"Destination address required.");
  case EMSGSIZE:
    return((const char *)"Message too long.");
  case EPROTOTYPE:
    return((const char *)"Protocol wrong type for socket.");
  case ENOPROTOOPT:
    return((const char *)"Protocol not available.");
  case EPROTONOSUPPORT:
    return((const char *)"Protocol not supported.");
  case ESOCKTNOSUPPORT:
    return((const char *)"Socket type not supported.");
  case EOPNOTSUPP:
    return((const char *)"Operation not supported on socket.");
  case EPFNOSUPPORT:
    return((const char *)"Protocol family not supported.");
  case EAFNOSUPPORT:
    return((const char *)"Address family not supported by protocol family.");
  case EADDRINUSE:
    return((const char *)"Address already in use.");
  case EADDRNOTAVAIL:
    return((const char *)"Can't assign requested address.");
  case ENETDOWN:
    return((const char *)"Network is down.");
  case ENETUNREACH:
    return((const char *)"Network is unreachable.");
  case ENETRESET:
    return((const char *)"Network dropped connection because of reset.");
  case ECONNABORTED:
    return((const char *)"Software caused connection abort.");
  case ECONNRESET:
    return((const char *)"Connection reset by peer.");
  case ENOBUFS:
    return((const char *)"Your system is out of memory.");
  case EISCONN:
    return((const char *)"Socket is already connected.");
  case ENOTCONN:
    return((const char *)"Socket is not connected.");
  case ESHUTDOWN:
    return((const char *)"Can't send after socket shutdown.");
  case ETOOMANYREFS:
    return((const char *)"Too many references: can't splice.");
  case ETIMEDOUT:
    return((const char *)"Connection timed out.");
  case ECONNREFUSED:
    return((const char *)"Connection refused.");
  case EHOSTDOWN:
    return((const char *)"Host is down.");
  case EHOSTUNREACH:
    return((const char *)"No route to host.");
  case EALREADY:
    return((const char *)"operation already in progress.");
  case EINPROGRESS:
    return((const char *)"operation now in progress.");
  case ESTALE:
    return((const char *)"Stale NFS file handle.");
  default:
    // Get the system error message for the given system error
    //
    const char *the_message = strerror(systemErrorNumber);
    if (!the_message)
      the_message = (const char *)"unknown system error occurred";
  }

  assert(the_message);
  return(the_message);
}

// ErrnoMessage -- convert system error number to readable text
// Description:
//  Given a valid system error number (as returned in the global errno),
//  return a pointer to a readable string that describes the error.
// Arguments:
//  systemErrorNumber - errno value
// Outputs:
//  <none>
// Returns:
//  SdmString -- string object containing ascii text describing system error
//			that is safe to refer to in an MT-hot environment
//

// This is the "set number" index into the message catalog where the messages for 
// the system errors are held

static const int LIBSDTMAIL_ERRNOSET = 2;

SdmString SdmUtility::ErrnoMessage(int systemErrorNumber)
{
  SdmString* the_message = new SdmString;

  *the_message = SdmUtility::MessageCatalogLookup(LIBSDTMAIL_ERRNOSET, systemErrorNumber, NULL);

  if (!*the_message)
    *the_message = _convertErrnoToErrorMessage(systemErrorNumber);

  assert(*the_message);
  return(*the_message);
}

// Lookup a message in the library message catalog
// Given a set number, message number, and default message, lookup the specified message
// in the catalog - either return a MT-HOT save pointer to the message if successful;
// or return the default message pointer if not successful

const char* SdmUtility::MessageCatalogLookup(int setnum, int msgnum, const char* defaultMsg)
{
  static nl_catd _errorCatalogD = (nl_catd) -1;
  static SdmBoolean _errorCatalogOpenAttempted = Sdm_False;
  static const char* LIBSDTMAIL_CATALOGDATAFILE = "SDtMail";
  static mutex_t _msgcatmutex = DEFAULTMUTEX;

  const char *the_message = NULL;

  SdmMutexEntry entry(_msgcatmutex);

  if (_errorCatalogOpenAttempted == Sdm_False) {
    _errorCatalogOpenAttempted = Sdm_True;
    _errorCatalogD = catopen((char *)LIBSDTMAIL_CATALOGDATAFILE, NL_CAT_LOCALE);
  }
  if (_errorCatalogD != (nl_catd)-1) {
    the_message = catgets(_errorCatalogD, setnum, msgnum, defaultMsg);
  }
  return(the_message);
}
