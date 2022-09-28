/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the message store locking utility Base class.

#pragma ident "@(#)LockUtility.cc	1.20 97/05/08 SMI"

// Include Files
#include <Utils/TnfProbe.hh>
#include <Utils/LockUtility.hh>
#include <SDtMail/SystemUtility.hh>
#include <SDtMail/Utility.hh>
#include <SDtMail/MailRc.hh>
#include <ctype.h>
#include <errno.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/systeminfo.h>
#include <sys/types.h>
#include <unistd.h>

#define PRIV2(STATUSVARIABLE,STATEMENT) STATUSVARIABLE = STATEMENT;

// ms interval to check for tooltalk messages when waiting for one
static const int TooltalkMsCheckInterval = 5000;

// number of times to check at TooltalkMsCheckInterval interval before timing out
static const int TooltalkMsCheckCount = 18;

// number of milliseconds for timeout overall
static const int TooltalkMsTimeout = (TooltalkMsCheckInterval * TooltalkMsCheckCount);

// number of seconds for timeout
static const int TooltalkSecondsTimeout = (TooltalkMsTimeout/10000);

// mutex to serialize dobject registry access
static mutex_t* _luObjectRegistryMutex = NULL;

// mutex to serialize session initialization access
static mutex_t* _luTooltalkInitializeMutex = NULL;

// Static class data member initialization

SdmString		SdmLockUtility::_luTtAppName("");
int			SdmLockUtility::_luTtFd = -1;
char*			SdmLockUtility::_luTtChannel = 0;
void*			SdmLockUtility::_luTtAppContext = 0;
SdmBoolean		SdmLockUtility::_luTtInitialized = Sdm_False;
SdmVector<SdmLockObject> SdmLockUtility::_luLockObjectRegistry;

//
// SdmLockUtilityInitMutex function is used to initialize 
// the mutex in SdmUtility before the program starts 
// execution.  The pragma belows specifies that this 
// function is an initialization function.  
//
// NOTE that this pragma is specific to Solaris 2.x only.
// 
static void SdmLockUtilityInitMutex()
{
  // Initialize the single mutex that guards access to the object registry

  _luObjectRegistryMutex = new mutex_t;
  int rc;
  rc = mutex_init(_luObjectRegistryMutex, USYNC_THREAD, NULL);
  assert (rc == 0);
  assert (_luObjectRegistryMutex != NULL);

  // Initialize the single mutex that guards access to the tooltalk session initialization code

  _luTooltalkInitializeMutex = new mutex_t;
  rc = mutex_init(_luTooltalkInitializeMutex, USYNC_THREAD, NULL);
  assert (rc == 0);
  assert (_luTooltalkInitializeMutex != NULL);
}
#pragma init(SdmLockUtilityInitMutex)

// Constructors

SdmLockUtility::SdmLockUtility() : 
  SdmPrim(Sdm_CL_LockObject),
  _luGrabLockIfOwnedByOtherMailer(Sdm_False),
  _luLockCallback((SdmLockCallback)0),
  _luLockCallbackClientData(0),
  _luLockDeletePending(Sdm_False),
  _luLocksHeld(0),
  _luTtFilePatterns(0),
  _luTtHaveWriteAccess(Sdm_False),
  _luTtLockEnabled(Sdm_False),
  _luTtMailtoolBlockedFlag(MT_BLOCK_TIMEOUT),
  _luTtMailtoolLock(Sdm_False),
  _luTtMailtoolPattern(NULL),
  _luTtOtherModified(Sdm_False),
  _luTtOutstanding(TT_BLOCK_NO_REQUEST_OUTSTANDING),
  _luTtUnlockCallbackInProgress(Sdm_False)
{
  TNF_PROBE_0 (SdmLockUtility_constructor_start,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_constructor_start");

  mutex_init(&_luMutex, USYNC_THREAD, NULL);
  mutex_init(&_luCallbackMutex, USYNC_THREAD, NULL);
  RegisterLockObject(_ObjSignature, this);
  _luObjSignature = _ObjSignature;

  TNF_PROBE_1 (SdmLockUtility_constructor_end,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_constructor_end lock created",
	       tnf_ulong, ObjSignature, (long)_luObjSignature);
}

// Destructor

SdmLockUtility::~SdmLockUtility() 
{
  TNF_PROBE_2 (SdmLockUtility_destructor_start,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_destructor_start",
	       tnf_ulong, ObjSignature, (unsigned long)_luObjSignature,
	       tnf_opaque, LocksHeld, (void *)_luLocksHeld);

  SdmError localError;

  // Grab the mutex for this object

  mutex_trylock(&_luCallbackMutex);
  mutex_lock(&_luMutex);

  // Release all locks aquired during the life of this object

  if (_luLocksHeld)
    (void) ForceReleaseLocks(localError, _luLocksHeld);

  assert(!_luLocksHeld);

  UnregisterLockObject(_ObjSignature);

  TNF_PROBE_2 (SdmLockUtility_destructor_end,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_destructor_end",
	       tnf_ulong, ObjSignature, (unsigned long)_luObjSignature,
	       tnf_opaque, LocksHeld, (void *)_luLocksHeld);

  _luObjSignature = 0;	// invalidate this objects signature
  mutex_unlock(&_luCallbackMutex);
  mutex_destroy(&_luCallbackMutex);
  mutex_unlock(&_luMutex);
  mutex_destroy(&_luMutex);
}


// This method is used to allow a lock object to be "queued for deletion"
// This is required when the lock object is to be deleted by a callback that
// is invoked from within the object to be deleted - deleting the object out
// from under itself can be bad
//
// If we can lock the object then cause it to be destroyed; otherwise, queue the
// object up for destruction later

void SdmLockUtility::DeleteObject(SdmLockUtility& lo)
{
  TNF_PROBE_2 (SdmLockUtility_DeleteObject_start,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_DeleteObject_start",
	       tnf_opaque, lockObject, (void *) &lo,
	       tnf_long, objSignature, (unsigned long) lo._luObjSignature);

  int objSignature = lo._luObjSignature;
  if (SdmLockUtility::LookupLockObjectBySignature(objSignature) != 0) {
    // This object still exists (was not deleted by an intervening callback)
    // Process the delete object request

    TNF_PROBE_2 (SdmLockUtility_DeleteObject_message,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_DeleteObject_message",
		 tnf_ulong, lockDeletePending, (unsigned long) lo._luLockDeletePending,
		 tnf_ulong, TtUnlockCallbackInProgress, (unsigned long) lo._luTtUnlockCallbackInProgress);

    // First see if we can grab the callback mutex - if we can then we own the
    // object and can delete it outright
    // for the single threaded case, mutex_trylock will always return 0 so we
    // also need to check to _luTtUnlockCallbackInProgress flag to make sure we
    // are not in a callback.

    if (mutex_trylock(&lo._luCallbackMutex) == 0 && !lo._luTtUnlockCallbackInProgress) {
      // no callback in progress - object is toast
      delete &lo;
      TNF_PROBE_0 (SdmLockUtility_DeleteObject_end,
		   "lockapi SdmLockUtility",
		   "sunw%debug SdmLockUtility_DeleteObject_end: got callback mutex deleted object");
      return;
    }

    // Ok, a callback is in progress - need to queue up the delete action
    // if one is not already queued up

    // If an unlock is not in progress, wait for the callback to complete, then
    // see if the object still exists, and if so, delete it

    // If an unlock is in progress, set the pending delete flag, and then make sure
    // the unlock is still in progress - if so, allow the pending delete to take place
    // when the callback completes; otherwise, got in late - nuke the object

    if (!lo._luLockDeletePending) {
      if (!lo._luTtUnlockCallbackInProgress) {
	mutex_lock(&lo._luCallbackMutex);
	if (SdmLockUtility::LookupLockObjectBySignature(objSignature) != 0) {
	  delete &lo;
	  TNF_PROBE_0 (SdmLockUtility_DeleteObject_end,
		       "lockapi SdmLockUtility",
		       "sunw%debug SdmLockUtility_DeleteObject_end: no unlock in progress deleted object");
	  return;
	}
      }
      lo._luLockDeletePending = Sdm_True;
    }
    assert(lo._luLockDeletePending);
  }

  TNF_PROBE_2 (SdmLockUtility_DeleteObject_end,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_DeleteObject_end",
	       tnf_ulong, lockDeletePending, (unsigned long) lo._luLockDeletePending,
	       tnf_ulong, TtUnlockCallbackInProgress, (unsigned long) lo._luTtUnlockCallbackInProgress);
  return;
}


SdmErrorCode SdmLockUtility::GrabLockIfOwnedByOtherMailer
(
 SdmError& err,
 SdmBoolean grabLock
 )
{
  err = Sdm_EC_Success;

  // Grab the mutex for this object

  //SdmMutexEntry entry(_luMutex);
  mutex_lock(&_luMutex);

  _luGrabLockIfOwnedByOtherMailer = grabLock;

  mutex_unlock(&_luMutex);
  return(err);
}

// Provide the callback information for this lock object

SdmErrorCode SdmLockUtility::IdentifyLockCallback
  (SdmError& err,
   SdmLockCallback lockCallback,
   void* lockCallbackClientData)
{  
  TNF_PROBE_0 (SdmLockUtility_IdentifyLockCallback_start,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_IdentifyLockCallback_start");

  // Grab the mutex for this object

  //SdmMutexEntry entry(_luMutex);
  mutex_lock(&_luMutex);

  // if locks are already held, object identity cannot be changed

  if (!lockCallback || _luLocksHeld) {
    err = Sdm_EC_Fail;
  }
  else {
    _luLockCallback = lockCallback;
    _luLockCallbackClientData = lockCallbackClientData;
    err = Sdm_EC_Success;
  }

  TNF_PROBE_1 (SdmLockUtility_IdentifyLockCallback_end,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_IdentifyLockCallback_end",
	       tnf_ulong, err, (unsigned long)((SdmErrorCode)err));

  mutex_unlock(&_luMutex);
  return(err);
}

// Provide the name of the object that subsequent lock operations will operate on

SdmErrorCode SdmLockUtility::IdentifyLockObject
(
 SdmError& err,
 const SdmString& lockObjectName	// name of object lock operations will operate on
 )
{
  TNF_PROBE_0 (SdmLockUtility_IdentifyLockObject_start,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_IdentifyLockObject_start");

  assert(lockObjectName.Length());

  // Grab the mutex for this object

  // SdmMutexEntry entry(_luMutex);
  mutex_lock(&_luMutex);

  // if locks are already held, object identity cannot be changed

  if (_luLocksHeld)
    err = Sdm_EC_Fail;
  else {
    _luLockObjectName = (SdmString) lockObjectName;
    err = Sdm_EC_Success;
  }

  TNF_PROBE_2 (SdmLockUtility_IdentifyLockObject_end,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_IdentifyLockObject_end",
	       tnf_ulong, err, (SdmErrorCode) err,
	       tnf_string, LockObjectName, (const char *)_luLockObjectName);

  mutex_unlock(&_luMutex);
  return(err);
}

// Obtain one or more locks on the object identified via IdentifyLockObject

SdmErrorCode SdmLockUtility::ObtainLocks
(
 SdmError& err,
 SdmLockTypes locksToGet	// bit mask detailing locks to be obtained
 )
{
  TNF_PROBE_1 (SdmLockUtility_ObtainLocks_start,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_ObtainLocks_start",
	       tnf_opaque, locksToGet, (void *)locksToGet);

  err = Sdm_EC_Success;

  assert(locksToGet);		// must specify at least one lock to be obtained 

  // Grab the mutex for this object

  //SdmMutexEntry entry(_luMutex);
  mutex_lock(&_luMutex);

  // if no lock object has been specified, this is an error
  // if the requested lock is already obtained, this is an error

  if (!_luLockObjectName.Length() || (_luLocksHeld & locksToGet)) {
    err = Sdm_EC_Fail;
  }
  else {

    // if a session lock has been requested, get it first

    if (locksToGet & Sdm_LKT_Session) {
      if (ObtainSessionLock(err) != Sdm_EC_Success) {
	SdmError localError;
	(void) ForceReleaseLocks(localError, locksToGet);	// if one fails, they all fail
	err.AddMajorErrorCode(Sdm_EC_CannotObtainMailboxLock);
      }
    }

    // if an update lock has been requested, get it next

    if ((err == Sdm_EC_Success) && (locksToGet & Sdm_LKT_Update)) {
      if (ObtainUpdateLock(err) != Sdm_EC_Success) {
	SdmError localError;
	(void) ForceReleaseLocks(localError, locksToGet);	// if one fails, they all fail

	err.AddMajorErrorCode(Sdm_EC_CannotObtainMailboxLock);
      }
    }
  }

  TNF_PROBE_1 (SdmLockUtility_ObtainLocks_end,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_ObtainLocks_end",
	       tnf_ulong, err, (unsigned long)((SdmErrorCode)err));

  mutex_unlock(&_luMutex);

  return(err);
}

// Release specified locks previously obtained

SdmErrorCode SdmLockUtility::ReleaseLocks
(
 SdmError& err,
 SdmLockTypes locksToRelease	// bit mask detailing locks to be released
 )
{
  TNF_PROBE_1 (SdmLockUtility_ReleaseLocks_start,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_ReleaseLocks_start",
	       tnf_opaque, locksToRelease, (void *)locksToRelease);

  SdmError updateError, sessionError;

  // Grab the mutex for this object

  //SdmMutexEntry entry(_luMutex);
  mutex_lock(&_luMutex);
  
  err = Sdm_EC_Success;

  assert(locksToRelease);    	// must specify at least one lock to be released

  if (locksToRelease & Sdm_LKT_Update) {
    (void) ReleaseUpdateLock(updateError);
  }

  if (locksToRelease & Sdm_LKT_Session) {
    (void) ReleaseSessionLock(sessionError);
  }

  if (sessionError != Sdm_EC_Success)
    err = sessionError;
  else if (updateError != Sdm_EC_Success)
    err = updateError;

  TNF_PROBE_1 (SdmLockUtility_ReleaseLocks_end,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_ReleaseLocks_end",
	       tnf_ulong, err, (unsigned long)((SdmErrorCode)err));

  mutex_unlock(&_luMutex);

  return (err);
}

// --------------------------------------------------------------------------------
// Private methods
// --------------------------------------------------------------------------------

// Release specified locks previously obtained; differs from ReleaseLocks in that
// its ok to ask for the release of a lock that is not held - this is used internally
// to recover from lock requests failures

SdmErrorCode SdmLockUtility::ForceReleaseLocks
(
 SdmError& err,
 SdmLockTypes locksToRelease	// bit mask detailing locks to be released
 )
{
  TNF_PROBE_1 (SdmLockUtility_ForceReleaseLocks_start,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_ForceReleaseLocks_start",
	       tnf_opaque, locksToRelease, (void *)locksToRelease);

  SdmError updateError, sessionError;

  err = Sdm_EC_Success;

  assert(locksToRelease);    	// must specify at least one lock to be released

  if ( (locksToRelease & Sdm_LKT_Update) && (_luLocksHeld & Sdm_LKT_Update) ) {
    (void) ReleaseUpdateLock(updateError);
  }

  if ( (locksToRelease & Sdm_LKT_Session) && (_luLocksHeld & Sdm_LKT_Session) ) {
    (void) ReleaseSessionLock(sessionError);
  }

  if (sessionError != Sdm_EC_Success)
    err = sessionError;
  else if (updateError != Sdm_EC_Success)
    err = updateError;

  TNF_PROBE_1 (SdmLockUtility_ForceReleaseLocks_end,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_ForceReleaseLocks_end",
	       tnf_ulong, err, (unsigned long)((SdmErrorCode)err));

  return (err);
}

// Generate the "update lock file name" for a given object
// In this case it consists of the object name with ".lock" appended

void SdmLockUtility::GenerateUpdateLockFileName(SdmString& lockFileName)
{
  assert(_luLockObjectName.Length());
  lockFileName = _luLockObjectName;
  lockFileName += ".lock";
}

// Generate the "update temporary lock file name" for a given object
// We generate a properly constructed "temporary file name" that can be
// used to create the new lock file prior linking it to the real lock file name

void SdmLockUtility::GenerateUpdateTempLockFileName(SdmString& lockFileName)
{
  int return_status;
  assert(_luLockObjectName.Length());

  // Create the temporary mail lock file name
  // It has the form <_lockfilename><XXXXXX> or mailbox.lockXXXXXX
  // mktemp then creates a unique temporary file for the template
  //
  char tempLockFileName[MAXPATHLEN];
  memset(tempLockFileName, 0, MAXPATHLEN);
  sprintf(tempLockFileName, "%sXXXXXX", (const char *)_luLockObjectName);
  mktemp(tempLockFileName);
  // If the first character is NULL, then this means the lock file
  // cannot be created - probably a permissions or path problem
  lockFileName = tempLockFileName;
  if (tempLockFileName[0] != '\0') {
    PRIV2(return_status,SdmSystemUtility::SafeRemove(tempLockFileName));
  }
}

// Obtain an session lock for the previously identified object

SdmErrorCode SdmLockUtility::ObtainSessionLock(SdmError& err)
{
  TNF_PROBE_1 (SdmLockUtility_ObtainSessionLock_start,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_ObtainSessionLock_start",
	       tnf_ulong, _luLocksHeld, (unsigned long) _luLocksHeld);

  assert(!(_luLocksHeld & Sdm_LKT_Session));	// asking twice is illegal

  SdmMailRc *mailrc = SdmMailRc::GetMailRc();
  assert(mailrc);

  // Obtain a tooltalk session lock if appropriate
  // First, see if we already have a session lock on this object from another context;
  // if so then just returned an error right now as we cant lock something we already
  // have a lock on

  SdmLockUtility* otherLock = SdmLockUtility::LookupLockObjectByObjectName(_luLockObjectName);
  if (otherLock && otherLock != this && otherLock->_luLocksHeld & Sdm_LKT_Session) {
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_ThisMailerOwnsWriteAccess, (const char *)_luLockObjectName);

    TNF_PROBE_4 (SdmLockUtility_ObtainSessionLock_end,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_ObtainSessionLock_end this mailer owns write access",
		 tnf_opaque, otherLock, (void *) otherLock,
		 tnf_opaque, this, (void *) this,
		 tnf_ulong, _luLocksHeld, (unsigned long) _luLocksHeld,
		 tnf_ulong, err, (unsigned long)((SdmErrorCode)err));

    return(err);
  }

  // We need to be careful here about the phases of obtaining a tooltalk lock.
  // For the first attempt, you initialize a one time major "tooltalk session",
  // which stays with us for the duration of execution
  // Then you register interest in the object to be locked - this establishes a 
  // connection with a tooltalk server that has dominion over the object, and
  // tells the server we are interested in activity on that file (e.g. receive
  // callback messages on activity concerning that file).
  // If this is successful, then issue requests to lock access to the file and
  // based upon the callback results determine whether or not we gain access.
  // We need to be careful to de-register interest in the file if the lock fails.

  _luTtLockEnabled = mailrc->IsValueDefined("cdenotooltalklock") ? Sdm_False : Sdm_True;

  if (_luTtLockEnabled) {

    // Initialize the tooltalk session locking code
    // We defer starting a toootalk session until the first tooltalk request
    // comes in - if we are not using tooltalk locking, then there is no need
    // to start a session at all as no request to release a lock is valid,
    // nor will we make any lock requests. Saves startup time as the tooltalk
    // session will not even be started until the lock request is done

    if (TooltalkSessionInitialize(err) != Sdm_EC_Success) {
      err.AddMajorErrorCode(Sdm_EC_MBL_TooltalkNotResponding);
      return(err);
    }

    // register interest in the file - failure means we cant talk to the db engine
    // or other tooltalk related problem

    if (TooltalkRegisterInterestInLockObject(err) != Sdm_EC_Success) {
      err.AddMajorErrorCode(Sdm_EC_MBL_TooltalkNotResponding);
      return(err);
    }

    // obtain a session lock on the file

    if (TooltalkSessionLockFile(err) != Sdm_EC_Success) {
      // Failed to lock - unregister interest in the file so we dont get any requests
      SdmError localError;
      TooltalkUnregisterInterestInLockObject(localError);
      return(err);
    }

  }

  _luLocksHeld |= Sdm_LKT_Session;

  TNF_PROBE_2 (SdmLockUtility_ObtainSessionLock_end,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_ObtainSessionLock_end",
	       tnf_ulong, _luLocksHeld, (unsigned long) _luLocksHeld,
	       tnf_ulong, err, (unsigned long)((SdmErrorCode)err));

  return(err);
}

// Obtain an update lock for the previously identified object

SdmErrorCode SdmLockUtility::ObtainUpdateLock(SdmError& err)
{
  SdmString tempUpdateLockFileName;
  int return_status = 0;

  assert(!(_luLocksHeld & Sdm_LKT_Update));	// asking twice is illegal

  // Generate the real name of the lock file to be

  GenerateUpdateLockFileName(_luUpdateLockFileName);
  assert(_luUpdateLockFileName.Length());

  // Generate a temporary file name that can be used 

  GenerateUpdateTempLockFileName(tempUpdateLockFileName);
  if (tempUpdateLockFileName.Length() == 0) {
    // This means the temporary file name cannot be created
    // Probably a permissions or path problem
    int lerrno = errno;
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_CannotCreateMailboxLockFile,
				    (const char *)_luUpdateLockFileName,
				    (const char *)SdmUtility::ErrnoMessage(lerrno));
    return (err);
  }
  assert(tempUpdateLockFileName.Length());

  // Generate unique lock i.d.

  GenerateUniqueUpdateLockId(_luUpdateLockUniqueId);
  assert(_luUpdateLockUniqueId.Length());

  // Implement the .lock short term lock protocol
  // This code was "adapted" from Solaris 2.5 (SunOS 5.5) usr/src/cmd/mail/maillock.c.
  //

  // loop through attempting to create the temporary lock file, and if successful attempt
  // to link the temporary lock file to the real lock file. If not successful, retry for
  // up to 5 minutes, and remove the current lock file if it is more than 5 minutes old.
  //
  int statFailed = 0;
  struct stat sbuf;

  SdmLockWaitObject updateLockWait(this, Sdm_LKEC_WaitingForUpdateLock, Sdm_LKEC_NotWaitingForUpdateLock);
  
  for (;;) {
    time_t t;

    // Attempt to create a temporary file and link it to the intended lock file
    // If it is successful AND t==0, we have the lock and can return.
    // If it is successful AND t!=0, it is a recoverable error, in which case
    // we spin and try again according to the retry rules.
    // If it is not successful then it is a non-recoverable error,
    // in which case the mailbox is deemed not-writable any more.
    //

    if (LinkUpdateLockFile(err, tempUpdateLockFileName, t) != Sdm_EC_Success) {
      (void) SdmSystemUtility::SafeRemove(tempUpdateLockFileName);
      return(err);
    }

    if (t == 0) {
      // In theory we have the lock - check to make damn sure
      // If we dont it means it was stolen out from under us

      if (CheckUpdateLockFileOwnership(err) != Sdm_EC_Success) {
	return(err);
      }

      // We really have the lock - indicate as such 

      _luLocksHeld |= Sdm_LKT_Update;
      return (err = Sdm_EC_Success);
    }

    // Could not link the temporary lock file to the intended real lock file
    // See if the lock file exists and if so if we can remove it because it is > 5 mins old.
    // If the stat fails it means the lock file disappeared between our attempt to link to it
    // and now - only allow this to go on so many times before punting
    //

    if (SdmSystemUtility::SafeStat((const char *)_luUpdateLockFileName, &sbuf) == -1) {
      if (statFailed++ > 5) {
	int lerrno = errno;
	err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_CannotStatMailboxLockFile,
					(const char *)_luUpdateLockFileName,
					(const char *)SdmUtility::ErrnoMessage(lerrno));
	return(err);
      }
      updateLockWait.IssueStartEvent();
      sleep(5);
      continue;
    }

    // The lock file already exists - compare the time of the temp
    // file with the time of the lock file, rather than with the
    // current time of day, since the files may reside on another
    // machine whose time of day differs from the one this program
    // is running on. If the lock file is less than 5 minutes old,
    // keep trying, otherwise, remove the lock file and try again.
    //
    statFailed = 0;
    if (t < (sbuf.st_ctime + 300)) {
      updateLockWait.IssueStartEvent();
      sleep(5);
      continue;
    }

    updateLockWait.IssueStopEvent();

    SdmUtility::LogError(Sdm_False,
			 "ObtainUpdateLock(): removing stale update lock file %s ctime %08ld temp lock %s ctime %08ld diff %08ld\n",
			 (const char *)_luUpdateLockFileName, sbuf.st_ctime,
			 (const char *)tempUpdateLockFileName, t, t-sbuf.st_ctime);

    PRIV2(return_status,SdmSystemUtility::SafeRemove(_luUpdateLockFileName));
    if (return_status == -1) {
      int lerrno = errno;
      // We were not able to unlink the file. This means that
      // we do not have write access to the directory. We will
      // have to pass on taking long locks.
      //
      err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_CannotRemoveStaleMailboxLockFile,
				      (const char *)_luUpdateLockFileName,
				      (const char *)SdmUtility::ErrnoMessage(lerrno));
      return(err);
    }
  }
}

SdmErrorCode SdmLockUtility::ReleaseSessionLock(SdmError& err)
{
  TNF_PROBE_2 (SdmLockUtility_ReleaseSessionLock_start,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_ReleaseSessionLock_start",
	       tnf_ulong, TtLockEnabled, (unsigned long)_luTtLockEnabled,
	       tnf_ulong, TtHaveWriteAccess, (unsigned long)_luTtHaveWriteAccess);

  assert(_luLocksHeld & Sdm_LKT_Session);

  // If _luTtUnlockCallbackInProgress is set, this means that a tooltalk callback
  // is in progress that will eventually release the session lock on this
  // object - therefore, we should just return as though this were successful
  // and let the callback do ths unlock when it is done

  if (!_luTtUnlockCallbackInProgress) {

    if (_luTtHaveWriteAccess == Sdm_True && _luTtFilePatterns) {
      _luTtPendingFlag = PENDING_DESTROY;
	
      // ttdt_file_event(NULL, TTDT_SAVED, _file_pats, 1);

      // _session->removeObjectKey(_key);
    }

    // Release any registered patterns or file events

    TooltalkUnregisterInterestInLockObject(err);
    assert(_luTtFilePatterns == NULL);
    assert(_luTtMailtoolPattern == NULL);
    assert(!_luTtMailtoolLock);

    // Indicate we no longer have write access

    _luTtHaveWriteAccess = Sdm_False;
    _luLocksHeld &= ~Sdm_LKT_Session;
  }

  TNF_PROBE_3 (SdmLockUtility_ReleaseSessionLock_end,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_ReleaseSessionLock_end",
	       tnf_ulong, TtUnlockCallbackInProgress, (unsigned long)  _luTtUnlockCallbackInProgress,
	       tnf_ulong, err, (unsigned long)((SdmErrorCode)err),
	       tnf_ulong, TtHaveWriteAccess, (unsigned long)_luTtHaveWriteAccess);

  return(err);
}

SdmErrorCode SdmLockUtility::ReleaseUpdateLock(SdmError& err)
{
  int return_status = 0;

  assert(_luLocksHeld & Sdm_LKT_Update);
  
  if (CheckUpdateLockFileOwnership(err) == Sdm_EC_Success) {
    PRIV2(return_status,SdmSystemUtility::SafeUnlink(_luUpdateLockFileName));
    if (return_status == -1) {
      int lerrno = errno;
      SdmUtility::LogError(Sdm_False,
			   "ReleaseUpdateLock(): cannot remove update lock file %s errno %d\n",
			   (const char *)_luUpdateLockFileName, errno);
      err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_CannotRemoveMailboxLockFile,
				      (const char *)_luUpdateLockFileName,
				      (const char *)SdmUtility::ErrnoMessage(lerrno));
    }
  }

  _luLocksHeld &= ~Sdm_LKT_Update;
  return (err);
}

// Function: generateUniqueUpdateLockId - create unique ID for this mailbox lock files
// Description:
//   generateUniqueUpdateLockId creates a unique ID which is written into .lock files and
//   can then be checked to make sure that the lock file has not been compromised by
//   another process.
// Method:
//   The ID generated consists of three parts:
//     <process id/%08d><current time in seconds/%08d><hardware serial number/%d>
//   Thus, a "typical" id would look like this:
//      000018577971028681915751068
//   Which breaks down as:
//      00001857 797102868 1915751068

void SdmLockUtility::GenerateUniqueUpdateLockId(SdmString& lockId)
{
  char theId[128];
  char hwserialbuf[64];
  
  memset(theId, 0, 128);
  memset(hwserialbuf, 0, 64);
  if (sysinfo(SI_HW_SERIAL, (char *)hwserialbuf, sizeof(hwserialbuf)-1) == -1)
    strcpy(hwserialbuf, "sdtmail");
  (void) sprintf(theId, "%08d%08d%s\0", getpid(), time((time_t *)0), hwserialbuf);
  assert(strlen(theId)<sizeof(theId));
  lockId = theId;
}

SdmErrorCode SdmLockUtility::CheckUpdateLockFileOwnership(SdmError& err)
{
  assert(_luLockObjectName.Length());
  assert(_luUpdateLockFileName.Length());
  assert(_luUpdateLockUniqueId.Length());
  
  struct stat info;
  if (SdmSystemUtility::SafeStat((const char *)_luUpdateLockFileName, &info) < 0) {
    SdmUtility::LogError(Sdm_False,
			 "CheckUpdateLockFileOwnership(): lock cannot be stat()ed: %s, errno = %d\n"
			 "Cannot obtain status information on mailbox lock file.\n"
			 "Suggest you close and reopen the mailbox.\n",
			 (const char *)_luUpdateLockFileName, errno);
    return (err = Sdm_EC_Fail);
  }
  
  int lock_fd;
  PRIV2(lock_fd,SdmSystemUtility::SafeOpen((const char *)_luUpdateLockFileName, O_RDONLY | O_RSYNC | O_SYNC));
  if (lock_fd == -1) {
    SdmUtility::LogError(Sdm_False,
			 "CheckUpdateLockFileOwnership(): lock cannot be open()ed: %s, errno = %d\n"
			 "Cannot open the mailbox lock file.\n"
			 "Suggest you close and reopen the mailbox.\n",
			 (const char *)_luUpdateLockFileName, errno);
    return (err = Sdm_EC_Fail);
  }
  
  char lockBuf[MAXPATHLEN];
  memset(&lockBuf, 0, MAXPATHLEN);
  int status = SdmSystemUtility::SafeRead(lock_fd, lockBuf, sizeof(lockBuf)-1);
  if (status <= 0) {
    SdmUtility::LogError(Sdm_False,
			 "CheckUpdateLockFileOwnership(): lock cannot be read: %s, errno = %d\n"
			 "Cannot read contents of mailbox lock file.\n"
			 "Suggest you close and reopen the mailbox.\n",
			 (const char *)_luUpdateLockFileName, errno);
    (void) SdmSystemUtility::SafeClose(lock_fd);
    return (err = Sdm_EC_Fail);
  }
  
  if ( (status < _luUpdateLockUniqueId.Length()+2)
       || (strncmp(lockBuf+2, (const char *)_luUpdateLockUniqueId, _luUpdateLockUniqueId.Length()) != 0) ) {
    SdmUtility::LogError(Sdm_False,
			 "CheckUpdateLockFileOwnership(): dtmail lock file was stolen by another process.\n"
			 "The contents of your mailbox may have been changed by another process.\n"
			 "Suggest you close and reopen the mailbox.\n");
    (void) SdmSystemUtility::SafeClose(lock_fd);
    return (err = Sdm_EC_Fail);
  }
  (void) SdmSystemUtility::SafeClose(lock_fd);
  return (err = Sdm_EC_Success);
}

// Function: linkUpdateLockFile - create and link temporary lock file to real lock file
// Description:
//  Create a lock file for the current mailbox. Return success of failure.
// Method:
//  . create a temporary lock file with a unique signature id of this process
//  . link the temporary lock file to the real lock file
//  . if the link is not successful, remove the temporary lock file and return the
//    time() in seconds on the remote system of when the temporary lock file was created
//  . if the link is successful, remove the temporary lock file (link) and return 0.
// Arguments:
//   error	-- standard error structure used by caller
// Outputs:
//   If error.isSet() it is a fatal error from which the caller should return to its caller,
//		return value will always be time(0)
//   If !error.isSet() then check results of return value
// Returns:
//   time_t == 0 : indicates that the real lock file has been created and we own it
//          != 0 : could not create real lock file, return value is the time *on the remote system*
//		   that the temporary lock file was created with (from comparison with existing
//		   lock file to see how old it is)
//

SdmErrorCode SdmLockUtility::LinkUpdateLockFile(SdmError& err, SdmString& tempUpdateLockFileName, time_t& r_lockAge)
{
  int lock_fd;
  int return_status;
  struct stat sbuf;

  // Default set the lock age to an impossible value; it will be reset when appropriate

  r_lockAge = -1;

  // Create the temporary lock file. Failure to do so indicates lack of write permission
  // in the directory or some other fatal error
  //
  PRIV2(lock_fd,SdmSystemUtility::SafeOpen((const char *)tempUpdateLockFileName,
					   O_RDWR | O_CREAT | O_EXCL | O_SYNC | O_RSYNC, 0666));
  if (lock_fd < 0) {
    // We are not able to create the temporary lock file.
    // We will have to punt on trying to lock here from now on.
    //
    int lerrno = errno;
    err.SetMajorErrorCodeAndMessage(lerrno == EACCES ? Sdm_EC_MBL_CannotCreateMailboxLockFile_NoPermission :
				    lerrno == EISDIR ? Sdm_EC_MBL_CannotCreateMailboxLockFile_IsDirectory :
				    lerrno == ENOENT ? Sdm_EC_MBL_CannotCreateMailboxLockFile_NoSuchFile :
				    lerrno == ENOLINK ? Sdm_EC_MBL_CannotCreateMailboxLockFile_RemoteAccessLost :
				    Sdm_EC_MBL_CannotCreateMailboxLockFile,
				    (const char *)tempUpdateLockFileName,
				    (const char *)SdmUtility::ErrnoMessage(lerrno));
    assert(err);
    SdmUtility::LogError(Sdm_False,
			 "LinkUpdateLockFile(): cannot create temporary update lock file %s, errno = %d\n",
			 (const char *)tempUpdateLockFileName, lerrno);
    PRIV2(return_status, SdmSystemUtility::SafeRemove((const char *)tempUpdateLockFileName));
    return(err);
  }
  
  // Get creation time of temporary file *on remote system*
  //
  if (SdmSystemUtility::SafeFStat(lock_fd, &sbuf) == -1) {
    int lerrno = errno;
    SdmUtility::LogError(Sdm_False,
			 "LinkUpdateLockFile(): temporary lock file cannot be stat()ed: %s, errno = %d\n",
			 (const char *)tempUpdateLockFileName, lerrno);
    (void) SdmSystemUtility::SafeClose(lock_fd);
    PRIV2(return_status, SdmSystemUtility::SafeRemove((const char *)tempUpdateLockFileName));
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_CannotStatMailboxLockFile,
				    (const char *)tempUpdateLockFileName,
				    (const char *)SdmUtility::ErrnoMessage(lerrno));
    return(err);
  }
  
  // Write proper contents to lock file:
  // Write the string "0" into the lock file to give us some
  // interoperability with SVR4 mailers.  SVR4 mailers expect
  // a process ID to be written into the lock file and then
  // use kill() to see if the process is alive or not.  We write
  // 0 into it so that SVR4 mailers will always think our lock file
  // is valid. In addition we include a unique ID so we can verify
  // if the lock file is stolen out from under us.
  //
  ssize_t writeResults;
  writeResults = SdmSystemUtility::SafeWrite(lock_fd, "0\0", 2);
  if (writeResults == 2)
    writeResults += SdmSystemUtility::SafeWrite(lock_fd, (const char *)_luUpdateLockUniqueId, _luUpdateLockUniqueId.Length());
  if ( (writeResults != _luUpdateLockUniqueId.Length()+2) ){
    int lerrno = errno;
    SdmUtility::LogError(Sdm_False,
			 "LinkUpdateLockFile(): write to temporary lock file failed: %s, errno = %d\n",
			 (const char *)tempUpdateLockFileName, lerrno);
    (void) SdmSystemUtility::SafeClose(lock_fd);
    PRIV2(return_status, SdmSystemUtility::SafeRemove((const char *)tempUpdateLockFileName));
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_CannotStatMailboxLockFile,
				    (const char *)tempUpdateLockFileName,
				    (const char *)SdmUtility::ErrnoMessage(lerrno));
    return(err);
  }
  
  // sync up the lock file with the ultimate storage device
  //
  if (fsync(lock_fd) == -1) {
    int lerrno = errno;
    SdmUtility::LogError(Sdm_False,
			 "LinkUpdateLockFile(): fsync to temporary lock file failed: %s, errno = %d\n",
			 (const char *)tempUpdateLockFileName, lerrno);
    (void) SdmSystemUtility::SafeClose(lock_fd);
    PRIV2(return_status, SdmSystemUtility::SafeRemove((const char *)tempUpdateLockFileName));
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_CannotStatMailboxLockFile,
				    (const char *)tempUpdateLockFileName,
				    (const char *)SdmUtility::ErrnoMessage(lerrno));
    return(err);
  }
  
  // close the file
  //
  if (SdmSystemUtility::SafeClose(lock_fd) == -1) {
    int lerrno = errno;
    SdmUtility::LogError(Sdm_False,
			 "LinkUpdateLockFile(): close of temporary lock file failed: %s, errno = %d\n",
			 (const char *)tempUpdateLockFileName, lerrno);
    PRIV2(return_status, SdmSystemUtility::SafeRemove((const char *)tempUpdateLockFileName));
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_CannotStatMailboxLockFile,
				    (const char *)tempUpdateLockFileName,
				    (const char *)SdmUtility::ErrnoMessage(lerrno));
    return(err);
  }
  
  // The temporary lock file has been created - now try and link it to the real lock file
  // Failure here is not fatal as we will retry and possible try and remove the real lock
  // file later on
  //
  PRIV2(return_status,SdmSystemUtility::SafeLink((const char *)tempUpdateLockFileName, (const char *)_luUpdateLockFileName));
  if (return_status == -1) {
    PRIV2(return_status,SdmSystemUtility::SafeRemove((const char *)tempUpdateLockFileName));
    r_lockAge = sbuf.st_ctime;
    return(err = Sdm_EC_Success);	// Gack: success with time != 0 is a timed wait failure
  }
  
  // We successfully linked the temp lock file to the real lock file name
  // Obtain the stat entry for the real lock file and make sure there are just
  // two links to the file - if not exactly two something went wrong (a race
  // condition of some kind?)

  struct stat info;

  PRIV2(return_status, SdmSystemUtility::SafeStat((const char *)_luUpdateLockFileName, &info));
  if (return_status == -1) {
    int lerrno = errno;
    SdmUtility::LogError(Sdm_False,
			 "LinkUpdateLockFile(): lock cannot be stat()ed: %s, errno = %d\n",
			 (const char *)_luUpdateLockFileName, lerrno);
    PRIV2(return_status,SdmSystemUtility::SafeRemove((const char *)tempUpdateLockFileName));
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_CannotStatMailboxLockFile,
				    (const char *)_luUpdateLockFileName,
				    (const char *)SdmUtility::ErrnoMessage(lerrno));
    return(err);
  }

  if (info.st_nlink != 2) {
    int lerrno = errno;
    SdmUtility::LogError(Sdm_False,
			 "LinkUpdateLockFile(): too many links (%d) on newly aquired lock file: %s, errno = %d\n",
			 info.st_nlink, (const char *)_luUpdateLockFileName, lerrno);
    PRIV2(return_status,SdmSystemUtility::SafeRemove((const char *)tempUpdateLockFileName));
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_CannotStatMailboxLockFile,
				    (const char *) _luUpdateLockFileName,
				    (const char *)SdmUtility::ErrnoMessage(lerrno));
    return(err);
  }

  // This means we have the dot lock for our process - remove the temporary lock
  // file name (link) and return
  //
  PRIV2(return_status,SdmSystemUtility::SafeRemove((const char *)tempUpdateLockFileName));
  r_lockAge = 0;
  return (err = Sdm_EC_Success);
}

SdmErrorCode SdmLockUtility::ConnectionStartup
(
 SdmError& err,
 const SdmString& _appName,
 void* _appContext
 )
{
  assert (_luTooltalkInitializeMutex != NULL);
  // SdmMutexEntry entry(*_luTooltalkInitializeMutex);
  mutex_lock(_luTooltalkInitializeMutex);

  assert(_appName.Length());
  assert(!_luTtAppName.Length());	// Only allow one startup

  _luTtAppName = (SdmString) _appName;
  _luTtAppContext = _appContext;
  
  mutex_unlock(_luTooltalkInitializeMutex);
  return(err = Sdm_EC_Success);
}

SdmErrorCode SdmLockUtility::ConnectionShutdown
(
 SdmError& err
 )
{
  assert (_luTooltalkInitializeMutex != NULL);
  // SdmMutexEntry entry(*_luTooltalkInitializeMutex);
  mutex_lock(_luTooltalkInitializeMutex);

  err = Sdm_EC_Success;

  TooltalkSessionTerminate(err);
  mutex_unlock(_luTooltalkInitializeMutex);
  return(err);
}

// --------------------------------------------------------------------------------
// Tooltalk specific private functions
// --------------------------------------------------------------------------------

SdmErrorCode SdmLockUtility::TooltalkSessionTerminate(SdmError& err)
{
  TNF_PROBE_0 (SdmLockUtility_TooltalkSessionTerminate_start,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkSessionTerminate_start");

  if (_luTtChannel) {
    Tt_status ttErrorStatus;
    if ((ttErrorStatus = ttdt_close(_luTtChannel, NULL, 1)) != TT_OK) {
      SdmUtility::LogError(Sdm_False, "Tooltalk error closing communication endpoint: ttdt_close: ttdt_file_quit: (%lu) %s\n",
			   (unsigned long) ttErrorStatus, tt_status_message(ttErrorStatus));
      TNF_PROBE_2 (SdmLockUtility_TooltalkSessionTerminate_ttdt_close,
		   "lockapi SdmLockUtility",
		   "sunw%debug SdmLockUtility_TooltalkSessionTerminate: ttdt_close failure",
		   tnf_ulong, ttErrorStatus, (unsigned long) ttErrorStatus,
		   tnf_string, tt_status_message, tt_status_message(ttErrorStatus));
    }
    tt_free(_luTtChannel);
    _luTtChannel = 0;		// reset saved context
    _luTtAppName = "";		// reset appname too so another startup will now work
  }

  TNF_PROBE_0 (SdmLockUtility_TooltalkSessionTerminate_end,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkSessionTerminate_end");

  return(err = Sdm_EC_Success);
}

// TooltalkSessionInitialize - initialize a session for tootalk activity

SdmErrorCode SdmLockUtility::TooltalkSessionInitialize(SdmError& err)
{
  TNF_PROBE_0 (SdmLockUtility_TooltalkSessionInitialize_start,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkSessionInitialize_start");

  err = Sdm_EC_Success;

  // Grab the initialization mutex

  assert (_luTooltalkInitializeMutex != NULL);
  // SdmMutexEntry entry(*_luTooltalkInitializeMutex);
  mutex_lock(_luTooltalkInitializeMutex);

  // See if one time class initialization needs to be done

  if (!_luTtInitialized) {
    const char* appName =  _luTtAppName.Length() ? (const char *)_luTtAppName : "libSDtMail";
    const char* vendor = "SunSoft";
    const char* version = "%I";
    int sendStarted = 0;

    assert(!_luTtChannel);

    _luTtChannel = tt_default_procid();
    if (tt_pointer_error(_luTtChannel) != TT_OK) {
      _luTtChannel = ttdt_open(&_luTtFd, appName, vendor, version, sendStarted);
      if (tt_pointer_error(_luTtChannel) != TT_OK) {
	err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkSessionStartupError,
					tt_status_message(tt_pointer_error(_luTtChannel)));
	TNF_PROBE_2 (SdmLockUtility_TooltalkSessionInitialize_ttdt_open_failure,
		     "lockapi SdmLockUtility",
		     "sunw%debug SdmLockUtility_TooltalkSessionInitialize_ttdt_open_failure",
		     tnf_ulong, tt_ptr_error, (unsigned long) tt_ptr_error(_luTtChannel),
		     tnf_string, tt_status_message, tt_status_message(tt_ptr_error(_luTtChannel)));
      }
      else {
	TNF_PROBE_3 (SdmLockUtility_TooltalkSessionInitialize_open,
		     "lockapi SdmLockUtility",
		     "sunw%debug SdmLockUtility_TooltalkSessionInitialize_open",
		     tnf_string, appName, appName,
		     tnf_string, vendor, vendor,
		     tnf_string, version, version);
      }
    }
  }

  // Tooltalk session initialization is successful if no error is indicated

  if (err == Sdm_EC_Success) {
    _luTtFd = tt_fd();
    _luTtInitialized = Sdm_True;
  }
  else {
    _luTtFd = -1;
    _luTtChannel = 0;
    _luTtInitialized = Sdm_False;
  }

  TNF_PROBE_3 (SdmLockUtility_TooltalkSessionInitialize_end,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkSessionInitialize_end",
	       tnf_ulong, err, (unsigned long)((SdmErrorCode)err),
	       tnf_ulong, _luTtInitialized, (unsigned long) _luTtInitialized,
	       tnf_ulong, _luTtFd, (unsigned long) _luTtFd);

  mutex_unlock(_luTooltalkInitializeMutex);

  return(err);
}

// Create a tooltalk pattern for the mailtool locking protocol

SdmErrorCode SdmLockUtility::TooltalkMailtoolPatternCreate
(
 SdmError& err,				// standard error object
 Tt_pattern& r_pattern,			// return -> tooltalk pattern created
 const char* fileOfInterest		// name of file to perform operation on
)
{
  Tt_status ttErrorStatus;
  Tt_pattern pattern;

  r_pattern = NULL;

  // Create a new tooltalk pattern object

  pattern = tt_pattern_create();
  if (tt_pointer_error(pattern) != TT_OK) {
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				    "tt_pattern_create",
				    tt_status_message(tt_pointer_error(pattern)));
    return(err);
  }

  if ((ttErrorStatus = tt_pattern_category_set(pattern, TT_HANDLE)) != TT_OK) {
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				    "tt_pattern_category_set",
				    tt_status_message(ttErrorStatus));
    tt_pattern_destroy(pattern);
    return(err);
  }

  if ((ttErrorStatus = tt_pattern_scope_add(pattern, TT_FILE)) != TT_OK) {
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				    "tt_pattern_scope_add",
				    tt_status_message(ttErrorStatus));
    tt_pattern_destroy(pattern);
    return(err);
  }

  if ((ttErrorStatus = tt_pattern_file_add(pattern, fileOfInterest)) != TT_OK) {
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				    "tt_pattern_file_add",
				    tt_status_message(ttErrorStatus));
    tt_pattern_destroy(pattern);
    return(err);
  }

  if ((ttErrorStatus = tt_pattern_op_add(pattern, "tlock")) != TT_OK) {
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				    "tt_pattern_op_add(tlock)",
				    tt_status_message(ttErrorStatus));
    tt_pattern_destroy(pattern);
    return(err);
  }

  if ((ttErrorStatus = tt_pattern_op_add(pattern, "rulock")) != TT_OK) {
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				    "tt_pattern_op_add(rulock)",
				    tt_status_message(ttErrorStatus));

    tt_pattern_destroy(pattern);
    return(err);
  }

  if ((ttErrorStatus = tt_pattern_callback_add(pattern, TooltalkMailtoolLockCallback)) != TT_OK) {
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				    "tt_pattern_callback_add",
				    tt_status_message(ttErrorStatus));
    tt_pattern_destroy(pattern);
    return(err);
  }

  if ((ttErrorStatus = tt_pattern_user_set(pattern, 1, (void *)_ObjSignature)) != TT_OK) {
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				    "tt_pattern_user_set",
				    tt_status_message(ttErrorStatus));
    tt_pattern_destroy(pattern);
    return(err);
  }

  // Successful - return the newly created tooltalk pattern

  r_pattern = pattern;
  return (err = Sdm_EC_Success);
}

// Create a tooltalk message for the session locking machinery

SdmErrorCode SdmLockUtility::TooltalkMessageCreate
(
 SdmError& err, 			// standard error object
 Tt_message& r_msg, 			// return -> tooltalk message created
 const char *operationName,		// operation to perform (e.g. tlock/rulock)
 const char *fileOfInterest, 		// name of file to perform operation on
 const Tt_class tt_class,		// class of operation (TT_NOTICE/TT_REQUEST)
 Tt_message_callback callbackFunction	// callback function
 )
{
  Tt_status ttErrorStatus;

  r_msg = NULL;

  // Create a new tooltalk message object

  Tt_message msg = tt_message_create();
  if (tt_ptr_error(msg) != TT_OK) {
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				    "tt_message_create",
				    tt_status_message(tt_pointer_error(msg)));
    return(err);
  }
  
  // Set the class attribute for the message
  // Specifies either a notice or a request - this is passed in by the caller 

  if ((ttErrorStatus = tt_message_class_set (msg, tt_class)) != TT_OK) {
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				    "tt_message_class_set",
				    tt_status_message(ttErrorStatus));
    tt_message_destroy(msg);
    return(err);
  }
  
  // Set the address attribute for this message
  // Default to "TT_PROCEDURE" indicating that this message is addressed to any process
  // that can perform this operation with these arguments.

  if ((ttErrorStatus = tt_message_address_set (msg, TT_PROCEDURE)) != TT_OK) {
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				    "tt_message_address_set",
				    tt_status_message(ttErrorStatus));
    tt_message_destroy(msg);
    return (err);
  }
  
  // Set the disposition attribute for this message 
  // Default to "TT_DISCARD" indicating not to start up a receiver for this message if one
  // does not exist; the message is returned to the sender with the status field containing
  // "TT_FAILED" if no receiver can be found.

  if ((ttErrorStatus = tt_message_disposition_set (msg, TT_DISCARD)) != TT_OK) {
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				    "tt_message_disposition_set",
				    tt_status_message(ttErrorStatus));
    tt_message_destroy(msg);
    return (err);
  }
  
  // Set the operation attribute for this message
  // This operation is passed in as part of the message to the receiver - it is passed
  // in by the caller

  if ((ttErrorStatus = tt_message_op_set (msg, operationName)) != TT_OK) {
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				    "tt_message_op_set",
				    tt_status_message(ttErrorStatus));
    tt_message_destroy(msg);
    return (err);
  }
  
  // Set the scope attribute for this message
  // Default to "TT_FILE" indicating that this message is meant for all processes that
  // are joined to the indicated "file" (e.g. file scoped lock hard wired in)

  if ((ttErrorStatus = tt_message_scope_set (msg, TT_FILE)) != TT_OK) {
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				    "tt_message_scope_set",
				    tt_status_message(ttErrorStatus));
    tt_message_destroy(msg);
    return (err);
  }

  // Set the file attribute for the message
  // This is the name of the file involved in this file scoped lock operation and is
  // passed in by the caller

  if ((ttErrorStatus = tt_message_file_set (msg, fileOfInterest)) != TT_OK) {  
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				    "tt_message_file_set",
				    tt_status_message(ttErrorStatus));
    tt_message_destroy(msg);
    return (err);
  }

  // Add a new argument to this message (#0)
  // In this case, we always add the input argument type "DtMail" and value of "lock"
  // to identify this message as a dtmail file scoped locking message
  // This argument is not normally included in a message from mailtool - thus
  // we can test for its existence when received to see if its another dtmail that 
  // is sending this message or a mailtool - if it is present and if it contains "lock" 
  // then its a message from a dtmail; otherwise, its a message from a mailtool

  if ((ttErrorStatus = tt_message_arg_add(msg, TT_IN, "DtMail", "lock")) != TT_OK) {
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				    "tt_message_arg_add",
				    tt_status_message(ttErrorStatus));
    tt_message_destroy(msg);
    return (err);
  }

  // Set a user data cell for this object
  // In this case, we add this object's identity to the message so that we can establish
  // context when the message reply is received - store in the user information area

  if ((ttErrorStatus = tt_message_user_set(msg, 1, (void *)_ObjSignature)) != TT_OK) {
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				    "tt_message_user_set",
				    tt_status_message(ttErrorStatus));
    tt_message_destroy(msg);
    return (err);
  }

  // Register a callback function for this message if the caller has provided one

  if (callbackFunction) {
    if ((ttErrorStatus = tt_message_callback_add(msg, callbackFunction)) != TT_OK) {
      err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				      "tt_message_callback_add",
				      tt_status_message(ttErrorStatus));
      tt_message_destroy(msg);
      return (err);
    }
  }

  // Successful - return the newly created tooltalk message

  r_msg = msg;
  return (err = Sdm_EC_Success);
}

SdmErrorCode SdmLockUtility::TooltalkRegisterInterestInLockObject(SdmError& err)
{
  TNF_PROBE_1 (SdmLockUtility_TooltalkRegisterInterestInLockObject_start,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkRegisterInterestInLockObject_start",
	       tnf_string, LockObjectName, (const char *)_luLockObjectName);

  err = Sdm_EC_Success;
  assert(_luLockObjectName.Length());

  // Initialize all of the tooltalk locking state flags for this lock

  _luTtHaveWriteAccess = Sdm_False;	// assume cannot write to file
  _luTtOtherModified = Sdm_True;
  _luTtMailtoolPattern = NULL;		// reset "mailtool" pattern for mailtool compatibility
  _luTtMailtoolLock = Sdm_False;	// indicate that mailtool protocol NOT used to get lock
  _luTtMailtoolBlockedFlag = MT_BLOCK_TIMEOUT;	// initial tooltalk blocked flag for tttk_blocked_while (wait for timeout to occur)


  // Register the file pattern.
  //
  _luTtFilePatterns = ttdt_file_join((const char *)_luLockObjectName, TT_FILE,
				     0, TooltalkFileLockCallback, (void*)_ObjSignature);
  if (tt_pointer_error(_luTtFilePatterns) != TT_OK) {
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockFileJoinError,
				    (const char *)_luLockObjectName,
				    tt_status_message(tt_pointer_error(_luTtFilePatterns)));
    TNF_PROBE_3 (SdmLockUtility_TooltalkRegisterInterestInLockObject_ttdt_file_join,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkRegisterInterestInLockObject: ttdt_file_join failure",
		 tnf_string, LockObjectName, (const char *)_luLockObjectName,
		 tnf_ulong, tt_ptr_error, (unsigned long) tt_ptr_error(_luTtFilePatterns),
		 tnf_string, tt_status_message, tt_status_message(tt_ptr_error(_luTtFilePatterns)));
    _luTtFilePatterns = NULL;
  }

  TNF_PROBE_1 (SdmLockUtility_TooltalkRegisterInterestInLockObject_end,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkRegisterInterestInLockObject_end",
	       tnf_ulong, err, (unsigned long)((SdmErrorCode)err));

  return(err);
}

// TooltalkUnregisterInterestInLockObject - release any patterns/file events registered

SdmErrorCode SdmLockUtility::TooltalkUnregisterInterestInLockObject(SdmError& err)
{
  err = Sdm_EC_Success;

  TNF_PROBE_4 (SdmLockUtility_TooltalkUnregisterInterestInLockObject_start,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkUnregisterInterestInLockObject_start",
	       tnf_string, LockObjectName, (const char *)_luLockObjectName,
	       tnf_string, TtFilePatterns, _luTtFilePatterns ? "YES" : "no",
	       tnf_string, TtMailtoolPattern, _luTtMailtoolPattern ? "YES" : "no",
	       tnf_ulong, TtMailtoolLock, (unsigned long) _luTtMailtoolLock);

  assert(_luLockObjectName.Length());

  // If ttdt_file_join returned any patterns, cause them to be destroyed by calling ttdt_file_quit

  if (_luTtFilePatterns) {
    Tt_status ttErrorStatus;
    ttErrorStatus = ttdt_file_quit(_luTtFilePatterns, 1);
    if (ttErrorStatus != TT_OK) {
      err.AddMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				      "ttdt_file_quit",
				      tt_status_message(ttErrorStatus));
      TNF_PROBE_2 (SdmLockUtility_TooltalkUnregisterInterestInLockObject_ttdt_file_quit,
		   "lockapi SdmLockUtility",
		   "sunw%debug SdmLockUtility_TooltalkUnregisterInterestInLockObject: ttdt_file_quit failure",
		   tnf_ulong, ttErrorStatus, (unsigned long) ttErrorStatus,
		   tnf_string, tt_status_message, tt_status_message(ttErrorStatus));
    }
    _luTtFilePatterns = NULL;
  }

  // If tt_pattern_create was used to create any mailtool patterns, cause them to be destroyed
  // by calling tt_pattern_destroy

  if (_luTtMailtoolPattern) {
    Tt_status ttErrorStatus;
    if ((ttErrorStatus = tt_pattern_destroy(_luTtMailtoolPattern)) != TT_OK) {
      SdmUtility::LogError(Sdm_False, "Tooltalk error releasing mailtool mailbox lock: tt_pattern_destroy: (%lu) %s\n",
			   (unsigned long) ttErrorStatus, tt_status_message(ttErrorStatus));
      TNF_PROBE_2 (SdmLockUtility_TooltalkunregisterInterestInLockObject_tt_pattern_destroy,
		   "lockapi SdmLockUtility",
		   "sunw%debug SdmLockUtility_TooltalkUnregisterInterestInLockObject: tt_pattern_destroy",
		   tnf_ulong, ttErrorStatus, (unsigned long) ttErrorStatus,
		   tnf_string, tt_status_message, tt_status_message(ttErrorStatus));
    }
    _luTtMailtoolPattern = NULL;
    _luTtMailtoolLock = Sdm_False;
  }

  TNF_PROBE_5 (SdmLockUtility_TooltalkRegisterInterestInLockObject_end,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkRegisterInterestInLockObject_end",
	       tnf_ulong, err, (unsigned long)((SdmErrorCode)err),
	       tnf_string, LockObjectName, (const char *)_luLockObjectName,
	       tnf_string, TtFilePatterns, _luTtFilePatterns ? "YES" : "no",
	       tnf_string, TtMailtoolPattern, _luTtMailtoolPattern ? "YES" : "no",
	       tnf_ulong, TtMailtoolLock, (unsigned long) _luTtMailtoolLock);

  return(err);
}

// TooltalkMailtoolLockCallback - this method is registered with "tt_pattern_op_add"
// to deal with "mailtool pattern" type mailbox locking messages. We can receive
// either a "tlock" (test lock) or "rulock" (release and unlock) message either from
// ourself or from another mailer which may be mailtool or dtmail (we can tell the
// difference based upon message contents). In the case of requests (TT_SENT) we
// process the request and return a result; in the case of responses (TT_HANDLED or
// TT_FAILED) we set state flags in the appropriate object contents, which are then
// interpreted by the initiator of the request (where tttk_block_while() is called:
// either TooltalkIsModified or TooltalkSessionLockFile).
//
Tt_callback_action SdmLockUtility::TooltalkMailtoolLockCallback(Tt_message m, Tt_pattern p)
{
  Tt_status ttErrorStatus;
  Tt_state message_state;
  char *senderid = NULL;
  char *defaultid = NULL;
  char *dtmailFlag = NULL;
  int numargs = 0;
  SdmBoolean itsFromDtmail = Sdm_False;

  TNF_PROBE_0 (SdmLockUtility_TooltalkMailtoolLockCallback_start,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkMailtoolLockCallback_start");

  // Get the message state
  // Failure to retrieve it means a serious problem with the tooltalk system (probably
  // ttsession has died) so its appropriate to fail, destroy and consume the message.

  message_state = tt_message_state(m);
  if (tt_int_error(message_state) != TT_OK) {
    SdmUtility::LogError(Sdm_False, "Tooltalk error in mailtool lock callback: tt_message_state: (%lu) %s\n",
			 (unsigned long) tt_int_error(numargs),
			 tt_status_message(tt_int_error(numargs)));
    TNF_PROBE_2 (SdmLockUtility_TooltalkMailtoolLockCallback_end,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkMailtoolLockCallback_end: tt_message_state_count failure",
		 tnf_ulong, tt_int_error, (unsigned long) tt_int_error(message_state),
		 tnf_string, tt_status_message, tt_status_message(tt_int_error(message_state)));
    (void) tt_message_fail(m);
    (void) tt_message_destroy(m);
    return(TT_CALLBACK_PROCESSED);
  }

  // Get the sender attribute from the message
  // Failure to retrieve it means a serious problem with the tooltalk system (probably
  // ttsession has died) so its appropriate to fail, destroy and consume the message.

  senderid = tt_message_sender(m);
  if (tt_ptr_error(senderid) != TT_OK) {
    SdmUtility::LogError(Sdm_False, "Tooltalk error in mailtool lock callback: tt_message_sender: (%lu) %s\n",
			 (unsigned long) tt_ptr_error(senderid),
			 tt_status_message(tt_ptr_error(senderid)));
    TNF_PROBE_2 (SdmLockUtility_TooltalkMailtoolLockCallback_end,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkMailtoolLockCallback_end: tt_message_sender failure",
		 tnf_ulong, tt_ptr_error, (unsigned long) tt_ptr_error(senderid),
		 tnf_string, tt_status_message, tt_status_message(tt_ptr_error(senderid)));
    (void) tt_message_fail(m);
    (void) tt_message_destroy(m);
    return(TT_CALLBACK_PROCESSED);
  }

  // Get the default proc id 
  // Failure to retrieve it means a serious problem with the tooltalk system (probably
  // ttsession has died) so its appropriate to fail, destroy and consume the message.

  defaultid = tt_default_procid();
  if (tt_ptr_error(defaultid) != TT_OK) {
    SdmUtility::LogError(Sdm_False, "Tooltalk error in mailtool lock callback: tt_default_procid: (%lu) %s\n",
			 (unsigned long) tt_ptr_error(defaultid),
			 tt_status_message(tt_ptr_error(defaultid)));
    TNF_PROBE_2 (SdmLockUtility_TooltalkMailtoolLockCallback_end,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkMailtoolLockCallback_end: tt_default_procid failure",
		 tnf_ulong, tt_ptr_error, (unsigned long) tt_ptr_error(defaultid),
		 tnf_string, tt_status_message, tt_status_message(tt_ptr_error(defaultid)));
    tt_free(senderid);
    (void) tt_message_fail(m);
    (void) tt_message_destroy(m);
    return(TT_CALLBACK_PROCESSED);
  }

  // Set an internal boolean indicating whether or not this message originated from us
  // (the sender tooltalk process i.d. is the same as our tooltalk process i.d.)

  SdmBoolean itsFromMe = (strcmp(senderid,defaultid) == 0 ? Sdm_True : Sdm_False);

  tt_free(defaultid);
  tt_free(senderid);

  // Get the operation attribute for this message
  // Failure to retrieve it means a serious problem with the tooltalk system (probably
  // ttsession has died) so its appropriate to fail, destroy and consume the message.

  char *message_op_tt = tt_message_op(m);
  if (tt_ptr_error(message_op_tt) != TT_OK) {
    SdmUtility::LogError(Sdm_False, "Tooltalk error in mailtool lock callback: tt_message_op: (%lu) %s\n",
			 (unsigned long) tt_ptr_error(message_op_tt),
			 tt_status_message(tt_ptr_error(message_op_tt)));
    TNF_PROBE_2 (SdmLockUtility_TooltalkMailtoolLockCallback_end,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkMailtoolLockCallback_end: tt_message_op failure",
		 tnf_ulong, tt_ptr_error, (unsigned long) tt_ptr_error(message_op_tt),
		 tnf_string, tt_status_message, tt_status_message(tt_ptr_error(message_op_tt)));
    (void) tt_message_fail(m);
    (void) tt_message_destroy(m);
    return(TT_CALLBACK_PROCESSED);
  }

  // If the operation is not specified, ignore this message

  if (!message_op_tt) {
    TNF_PROBE_0 (SdmLockUtility_TooltalkMailtoolLockCallback_end,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkMailtoolLockCallback_end: message operation NIL");
    (void) tt_message_destroy(m);
    return(TT_CALLBACK_CONTINUE);
  }

  SdmString message_op(message_op_tt);	// Place in SdmString so deallocation is automagic
  (void) tt_free(message_op_tt);

  // Get the number of arguments in this message
  // Failure to retrieve it means a serious problem with the tooltalk system (probably
  // ttsession has died) so its appropriate to fail, destroy and consume the message.

  numargs = tt_message_args_count(m);
  if (tt_int_error(numargs) != TT_OK) {
    SdmUtility::LogError(Sdm_False, "Tooltalk error in mailtool lock callback: tt_message_args_count: (%lu) %s\n",
			 (unsigned long) tt_int_error(numargs),
			 tt_status_message(tt_int_error(numargs)));
    TNF_PROBE_2 (SdmLockUtility_TooltalkMailtoolLockCallback_end,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkMailtoolLockCallback_end: tt_message_args_count failure",
		 tnf_ulong, tt_int_error, (unsigned long) tt_int_error(numargs),
		 tnf_string, tt_status_message, tt_status_message(tt_int_error(numargs)));
    (void) tt_message_fail(m);
    (void) tt_message_destroy(m);
    return(TT_CALLBACK_PROCESSED);
  }

  // Get argument #0 from the message
  // This argument is not normally included in a message from mailtool - thus
  // we can test for its existence to see if its another dtmail that is sending
  // this message or a mailtool - if it is present and if it contains "lock" then
  // its a message from a dtmail; otherwise, its a message from a mailtool

  dtmailFlag = tt_message_arg_val(m, 0);
  itsFromDtmail = tt_ptr_error(dtmailFlag) == TT_OK ? Sdm_True : Sdm_False;
  tt_free(dtmailFlag);

  // Handle the message according to whether or not its from us

  // If this message is from us then get the object signature of the lock object that sent it

  int objSig = itsFromMe ? (SdmObjectSignature) tt_message_user(m, 1) : (SdmObjectSignature) tt_pattern_user(p, 1);

  // Lookup the object that this callback is in reference to
  // If the object no longer exists (the object was destroyed),
  // we got here most likely because tooltalk is responding to 
  // a clean up message. In any case, fail the message and return

  SdmLockUtility* myself = SdmLockUtility::LookupLockObjectBySignature((SdmObjectSignature)objSig);

  if ( !myself ) {
    SdmUtility::LogError(Sdm_False, "Mailtool lock callback error: object signature %lu invalid\n",
			 (unsigned long) objSig);
    TNF_PROBE_0 (SdmLockUtility_TooltalkMailtoolLockCallback_end,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkMailtoolLockCallback_end: requestor gone ignored");
    (void) tt_message_destroy(m);
    return(TT_CALLBACK_PROCESSED);
  }

  uid_t sender_uid = tt_message_uid(m);
  gid_t sender_gid = tt_message_gid(m);
  uid_t my_uid = geteuid();
  gid_t my_gid = getegid();

  TNF_PROBE_4 (SdmLockUtility_TooltalkMailtoolLockCallback_message,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkMailtoolLockCallback_message",
	       tnf_ulong, sender_uid, (unsigned long) sender_uid,
	       tnf_ulong, my_uid, (unsigned long) my_uid,
	       tnf_ulong, sender_gid, (unsigned long) sender_gid,
	       tnf_ulong, my_gid, (unsigned long) my_gid);

  TNF_PROBE_4 (SdmLockUtility_TooltalkMailtoolLockCallback_message,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkMailtoolLockCallback_message",
	       tnf_ulong, itsFromMe, (unsigned long) itsFromMe,
	       tnf_ulong, itsFromDtmail, (unsigned long) itsFromDtmail,
	       tnf_ulong, objSig, (unsigned long)objSig,
	       tnf_string, LockObjectName, myself->_luLockObjectName);

  TNF_PROBE_0 (SdmLockUtility_TooltalkMailtoolLockCallback_mutex_lock,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkMailtoolLockCallback_mutex_lock");
  
  mutex_lock(&myself->_luCallbackMutex);

  // At this point we have the message ready to act on: if its from us then
  // handle it differently than if its from another process

  Tt_callback_action ttCallbackActionToPerform = TT_CALLBACK_PROCESSED;	// default action on return

  if (itsFromMe) {
    // Message is from this process (us) - it must use the enhanced mailtool pattern protocol
    //
    assert(itsFromDtmail == Sdm_True);
    if (!strcmp((const char *)message_op, "tlock")) {
      // handle tlock request - we are being asked whether or not we have a
      // lock on the requested object
      switch (message_state) {
      case TT_HANDLED:
	// This case means that we issued a test for lock request and
	// we received an affirmative reply - this means that a mailtool
	// out there has a lock on this object
	TNF_PROBE_0 (SdmLockUtility_TooltalkMailtoolLockCallback_itsfromme_tlock_tt_handled,
		     "lockapi SdmLockUtility",
		     "sunw%debug SdmLockUtility_TooltalkMailtoolLockCallback_itsfromme_tlock: TT_HANDLED - other mailer has pattern lock on object");
	myself->_luTtMailtoolBlockedFlag = MT_BLOCK_ISLOCKED;	// break out of loop, indicate other mailer has a lock
	break;
      case TT_REJECTED:	
	// same process with two patterns all continued yields rejected instead of failed,
	// treat as "failed" since its from us
      case TT_FAILED:
	// This case means that we issued a test for lock request and
	// we did not receive an affirmative reply - this means that no
	// mailtool out there has a lock on this object
	TNF_PROBE_0 (SdmLockUtility_TooltalkMailtoolLockCallback_itsfromme_tlock_tt_failed,
		     "lockapi SdmLockUtility",
		     "sunw%debug SdmLockUtility_TooltalkMailtoolLockCallback_itsfromme_tlock: TT_FAILED - no mailer has pattern lock on object");
	myself->_luTtMailtoolBlockedFlag = MT_BLOCK_NOLOCK;	// break out of loop, indicate no other mailer has the lock
	break;
      case TT_SENT:
	// This case means that we received a lock test request for this object
	// from the same process as ourself - this means we are trying to lock 
	// the object twice - strange - but, ignore it so that a file scoped
	// lock is tried next
	TNF_PROBE_0 (SdmLockUtility_TooltalkMailtoolLockCallback_itsfromme_tlock_tt_sent,
		     "lockapi SdmLockUtility",
		     "sunw%debug SdmLockUtility_TooltalkMailtoolLockCallback_itsfromme_tlock: TT_SENT - this mailer requesting lock state of object");
	tt_message_reject(m);
	break;
      default:
	// Message in state we are unprepared to handle
	// Pass it on to someone else who might be interested in it; if noone processes
	// it will eventually be returned to the sender as TT_FAILED
	TNF_PROBE_1 (SdmLockUtility_TooltalkMailtoolLockCallback_itsfromme_tlock_DEFAULT,
		     "lockapi SdmLockUtility",
		     "sunw%debug SdmLockUtility_TooltalkMailtoolLockCallback_itsfromme_tlock: DEFAULT - unknown message state",
		     tnf_ulong, message_state, (unsigned long) message_state);
	ttCallbackActionToPerform = TT_CALLBACK_CONTINUE;	// message still needs processing	
	break;
      }
    }
    else if (!strcmp((const char *)message_op, "rulock")) {
      // handle rulock request - we are being asked to release our lock on the object
      // from the same process as ourself - this means we are trying to lock an object
      // twice using the mailtool locking protocol - this cannot happen as we never use
      // this protocol between two dtmails...
      switch (message_state) {
      case TT_HANDLED:
      case TT_FAILED:
	// gmg set some flag?
	break;
      case TT_SENT:
	tt_message_fail(m);
	break;
      default:
	ttCallbackActionToPerform = TT_CALLBACK_CONTINUE;	// message still needs processing
	break;
      }
    }
    else {
      // A bad message from us? Just ignore it.
      TNF_PROBE_2 (SdmLockUtility_TooltalkMailtoolLockCallback_itsfromme_badmessageop,
		   "lockapi SdmLockUtility",
		   "sunw%debug SdmLockUtility_TooltalkMailtoolLockCallback_itsfromme_badmessageop",
		   tnf_string, message_op, (const char *)message_op,
		   tnf_ulong, message_state, (unsigned long) message_state);

      switch (message_state) {
      case TT_HANDLED:
      case TT_FAILED:
      default:
	ttCallbackActionToPerform = TT_CALLBACK_CONTINUE;	// message still needs processing
	break;
      }
    }
  }
  else {
    // Message is not from this process - may be from an old mailtool or from dtmail
    //
    if (!strcmp((const char *)message_op, "tlock")) {
      // handle tlock request - we are being asked whether or not we have a
      // lock on the requested object
      switch (message_state) {
      case TT_SENT:
	// This case means that we received a lock test request for this object
	// If its from a mailtool then return an affirmative reply; if its from a
	// dtmail just ignore it, which will cause dtmail to then do a file scoped
	// lock test request which we will handle separately.
	TNF_PROBE_0 (SdmLockUtility_TooltalkMailtoolLockCallback_tlock_tt_sent,
		     "lockapi SdmLockUtility",
		     "sunw%debug SdmLockUtility_TooltalkMailtoolLockCallback_tlock: TT_SENT - other mailer requesting lock state of object");
	if (itsFromDtmail) {
	  // this message is from another dtmail, ignore it, causing a file scoped
	  // lock to be tried next. (sender will receive fail if no positive response)
	  tt_message_reject(m);
	}
	else {
	  // this message is from mailtool - reply indicating that we do have a lock
	  tt_message_reply(m);
	}
	break;
      default:
	// Message in state we are unprepared to handle: its not from us, therefore,
	// it cannot be in a finished state, so it must be an error of some kind -
	// ignore it causing it to be passed on to another interested party or
	// ultimately failed back to the sender
	TNF_PROBE_1 (SdmLockUtility_TooltalkMailtoolLockCallback_tlock_DEFAULT,
		     "lockapi SdmLockUtility",
		     "sunw%debug SdmLockUtility_TooltalkMailtoolLockCallback_tlock: DEFAULT - unknown message state",
		     tnf_ulong, message_state, (unsigned long) message_state);
	ttCallbackActionToPerform = TT_CALLBACK_CONTINUE;
	break;
      }
    } 
    else if (!strcmp((const char *)message_op, "rulock")) {
      // handle rulock request - we are being asked to release our lock on the object
      // from another process. Another dtmail should never do this - it should only
      // come from mailtools that only use the mailtool locking protocol - another dtmail
      // will use the ttdt_Save protocol with us.

      assert(!itsFromDtmail);

      SdmError err;
      switch (message_state) {
      case TT_SENT:
	// A mailtool is requesting that we release our lock on the object

	// We do not accept unlock requests from anyone but ourself - this
	// prevents other users from breaking locks on mailboxes that we
	// have open
	
	if ( (sender_uid != my_uid) || (sender_gid != my_gid)) {
	  (void) tt_message_fail(m);
	  break;
	}

	// Always comply!

	if (myself->_luTtHaveWriteAccess) {
	  TNF_PROBE_0 (SdmLockUtility_TooltalkMailtoolLockCallback_rulock_tt_sent,
		       "lockapi SdmLockUtility",
		       "sunw%debug SdmLockUtility_TooltalkMailtoolLockCallback_rulock: TT_SENT");
	  if (myself->_luLockCallback) {
	    myself->_luTtUnlockCallbackInProgress = Sdm_True;	// Cause unlocks from callback to be ignored
	    (*myself->_luLockCallback)(Sdm_LKEC_SessionLockGoingAway, myself->_luLockCallbackClientData);
	    myself->_luTtUnlockCallbackInProgress = Sdm_False;
	  }
	  assert(myself->_luLocksHeld & Sdm_LKT_Session);	// session lock must still be held after return from callback
	  myself->ReleaseSessionLock(err);	// release lock on object
	}
	tt_message_reply(m);			// return affirmative reply (lock gone)
	break;
      default:
	// Message in state we are unprepared to handle
	// Pass it on to someone else who might be interested in it; if noone processes
	// it will eventually be returned to the sender as TT_FAILED
	TNF_PROBE_1 (SdmLockUtility_TooltalkMailtoolLockCallback_rulock_DEFAULT,
		     "lockapi SdmLockUtility",
		     "sunw%debug SdmLockUtility_TooltalkMailtoolLockCallback_rulock: DEFAULT - unknown message state",
		     tnf_ulong, message_state, (unsigned long) message_state);
	ttCallbackActionToPerform = TT_CALLBACK_CONTINUE;
	break;
      }
    }
  }

  // Made it this far - this message has been handled and 'ttCallbackActionToPerform'
  // contains the return code for this callback - free up temporarily allocated
  // resources and return from this callback

  if (ttCallbackActionToPerform == TT_CALLBACK_PROCESSED)
    tt_message_destroy(m);	// maybe destroy message before returning, may not be immediately done by tooltalk (libtt)

  TNF_PROBE_2 (SdmLockUtility_TooltalkMailtoolLockCallback_end,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkMailtoolLockCallback_end",
	       tnf_string, ttCallbackActionToPerform, ttCallbackActionToPerform == TT_CALLBACK_CONTINUE ? "TT_CALLBACK_CONTINUE" : "TT_CALLBACK_PROCESSED",
	       tnf_ulong, lockDeletePending, (unsigned long) myself->_luLockDeletePending);

  // Last act before exiting: if this objects deletion is pending, now is the time to act

  if (myself->_luLockDeletePending) {
    delete myself;
  } else {
    mutex_unlock(&myself->_luCallbackMutex);
  }
  return(ttCallbackActionToPerform);
}

Tt_message SdmLockUtility::TooltalkFileLockCallback(Tt_message msg,
						Tttk_op message_op,
						char * path,
						void *clientData,
						int,
						int itsFromMe)
{
  TNF_PROBE_3 (SdmLockUtility_TooltalkFileLockCallback_start,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkFileLockCallback_start",
	       tnf_ulong,	itsFromMe,		(unsigned long)itsFromMe,
	       tnf_ulong,	objSig,			(unsigned long)clientData,
	       tnf_string,	path,			path ? path : "(NIL)");

  Tt_state message_state = tt_message_state(msg);
  if ( (tt_int_error(message_state) != TT_OK) || (message_state == TT_CREATED) ) {
    TNF_PROBE_0 (SdmLockUtility_TooltalkFileLockCallback_end,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkFileLockCallback_end: state bad or TT_CREATED ignored");
    return(0);
  }

  // Lookup the object that this callback is in reference to
  // If the object no longer exists (the object was destroyed),
  // we got here most likely because tooltalk is responding to 
  // a clean up message. In any case, fail the message and return

  SdmLockUtility* myself = SdmLockUtility::LookupLockObjectBySignature((SdmObjectSignature)clientData);

  if ( !myself || (myself->_luObjSignature != (SdmObjectSignature)clientData)) {
    SdmUtility::LogError(Sdm_False, "File lock callback error: object signature %lu invalid\n",
			 (unsigned long) clientData);
    TNF_PROBE_0 (SdmLockUtility_TooltalkFileLockCallback_end,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkFileLockCallback_end: requestor gone ignored");
    tttk_message_fail(msg, TT_DESKTOP_ECANCELED, "Object destroyed", 1);
    return(0);
  }
 
  uid_t sender_uid = tt_message_uid(msg);
  gid_t sender_gid = tt_message_gid(msg);
  uid_t my_uid = geteuid();
  gid_t my_gid = getegid();

  TNF_PROBE_4 (SdmLockUtility_TooltalkFileLockCallback_message,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkFileLockCallback_message",
	       tnf_ulong, sender_uid, (unsigned long) sender_uid,
	       tnf_ulong, my_uid, (unsigned long) my_uid,
	       tnf_ulong, sender_gid, (unsigned long) sender_gid,
	       tnf_ulong, my_gid, (unsigned long) my_gid);

  TNF_PROBE_0 (SdmLockUtility_TooltalkFileLockCallback_mutex_lock,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkFileLockCallback_mutex_lock");

  mutex_lock(&myself->_luCallbackMutex);

  SdmBoolean answer;

  // Act on the tooltalk request received

  switch(message_op) {
  case TTDT_MODIFIED:
    // Announcement that some process is going ahead and modifying this data
    // If its us then this means we get the lock on the mailbox; if its not us
    // it means the lock was probably stolen away from us!

    TNF_PROBE_2 (SdmLockUtility_TooltalkFileLockCallback_ttdt_modified,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkFileLockCallback_ttdt_modified",
		 tnf_ulong,	TtOutstanding,	(unsigned long) myself->_luTtOutstanding,
		 tnf_ulong,	TtPendingFlag,	(unsigned long) myself->_luTtPendingFlag);

    if (myself->_luTtOutstanding == TT_BLOCK_NO_REQUEST_OUTSTANDING && !itsFromMe && myself->_luTtHaveWriteAccess) {
      // Very strange: another process has told us it is modifying this file, yet
      // we have a lock on the file! The last saver would win under there conditions;
      // however, thats not playing it safe. Force a close on the mailbox to prevent
      // the inevitable win-lose on this situation.
      SdmError err;
      if (myself->_luLockCallback) {
	myself->_luTtUnlockCallbackInProgress = Sdm_True;	// Cause unlocks from callback to be ignored
	(*myself->_luLockCallback)(Sdm_LKEC_SessionLockTakenAway, myself->_luLockCallbackClientData);
	myself->_luTtUnlockCallbackInProgress = Sdm_False;
      }
      assert(myself->_luLocksHeld & Sdm_LKT_Session);	// session lock must still be held after return from callback
      myself->ReleaseSessionLock(err);	// release lock on object
      myself->_luTtOtherModified = Sdm_True;
      myself->_luTtHaveWriteAccess = Sdm_False;
      break;
    }
	
    if (myself->_luTtOutstanding == TT_BLOCK_REQUEST_OUTSTANDING && myself->_luTtPendingFlag == PENDING_LOCK) {
      // This could be one of 2 conditions. If the message is
      // from us, then we have the lock, and we are done.
      // If not, then someone else is asking for the lock. We
      // reflect this by giving them the lock.

      if (itsFromMe) {
	myself->_luTtOtherModified = Sdm_False;
	myself->_luTtHaveWriteAccess = Sdm_True;
	myself->_luTtOutstanding = TT_BLOCK_NO_REQUEST_OUTSTANDING;
      }
      else {
	myself->_luTtOtherModified = Sdm_True;
	myself->_luTtHaveWriteAccess = Sdm_False;
	// We haven't seen our own request yet. Leave outstanding
	// so we can process it before leaving.
      }
    }
    break;

  case TTDT_GET_MODIFIED:
    // Request to see if anyone has modified this file - essentially a lock test
    // Because we got it it means we have the lock or are going to get the lock so
    // issue a positive acknowledgement that 'we have the lock'

    TNF_PROBE_0 (SdmLockUtility_TooltalkFileLockCallback_ttdt_get_modified,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkFileLockCallback_ttdt_get_modified");

    tt_message_arg_ival_set(msg, 1, 1);	// magic that sez 'return true from ttdt_Get_modified'
    tt_message_reply(msg);
    break;

  case TTDT_SAVED:
  case TTDT_REVERTED:
    // The other process has saved their changes (or tossed them).
    // At this point we should be able to start modifying the file.

    TNF_PROBE_2 (SdmLockUtility_TooltalkFileLockCallback_ttdt_saved_reverted,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkFileLockCallback_ttdt_saved_reverted",
		 tnf_ulong,	TtOutstanding,	(unsigned long) myself->_luTtOutstanding,
		 tnf_ulong,	TtPendingFlag,	(unsigned long) myself->_luTtPendingFlag);

    myself->_luTtOtherModified = Sdm_False;
    if (myself->_luTtOutstanding == TT_BLOCK_REQUEST_OUTSTANDING && myself->_luTtPendingFlag == PENDING_SAVE) {
      myself->_luTtOutstanding = TT_BLOCK_NO_REQUEST_OUTSTANDING;
    }
    break;

  case TTDT_REVERT:
  case TTDT_SAVE:
    // We are being requested to save or revert this mailbox - basically an attempt to steal
    // the lock on the mailbox away from us by causing us to save out changes and close the mailbox.

    TNF_PROBE_2 (SdmLockUtility_TooltalkFileLockCallback_ttdt_revert_save,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkFileLockCallback_ttdt_revert_save",
		 tnf_ulong,	TtOutstanding,	(unsigned long) myself->_luTtOutstanding,
		 tnf_ulong,	TtPendingFlag,	(unsigned long) myself->_luTtPendingFlag);

    // We do not accept unlock requests from anyone but ourself - this
    // prevents other users from breaking locks on mailboxes that we
    // have open

    answer = ( ((sender_uid == my_uid) && (sender_gid == my_gid) ) ? Sdm_True : Sdm_False);

    if (answer == Sdm_True) {
      SdmError err;
      if (myself->_luLockCallback) {
	myself->_luTtUnlockCallbackInProgress = Sdm_True;	// Cause unlocks from callback to be ignored
	(*myself->_luLockCallback)(Sdm_LKEC_SessionLockGoingAway, myself->_luLockCallbackClientData);
	myself->_luTtUnlockCallbackInProgress = Sdm_False;
      }
      assert(myself->_luLocksHeld & Sdm_LKT_Session);	// session lock must still be held after return from callback
      myself->ReleaseSessionLock(err);	// release lock on object
      tt_message_reply(msg);
    } 
    else {
      tttk_message_fail(msg, TT_DESKTOP_EACCES, 0, 0);
    }
    break;

  default:
    // Received a message that we don't know about or care to handle - we simply smile
    // and say thank you but maybe my SO would like it instead :-)

    TNF_PROBE_2 (SdmLockUtility_TooltalkFileLockCallback_DEFAULT_end,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkFileLockCallback_DEFAULT_end",
		 tnf_ulong,	TtOutstanding,	(unsigned long) myself->_luTtOutstanding,
		 tnf_ulong,	TtPendingFlag,	(unsigned long) myself->_luTtPendingFlag);
		 
		 mutex_unlock(&myself->_luCallbackMutex);
    return(msg);
  }

  // Made it this far - this message has been handled - free up temporarily allocated
  // resources (if any)  and return from this callback

  tt_message_destroy(msg);	// always destroy message before returning, may not be immediately done by tooltalk (libtt)
  TNF_PROBE_0 (SdmLockUtility_TooltalkFileLockCallback_end,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkFileLockCallback_end");

  // Last act before exiting: if this objects deletion is pending, now is the time to act

  if (myself->_luLockDeletePending) {
    delete myself;
  } else {
    mutex_unlock(&myself->_luCallbackMutex);
  }

  return(0);
}

SdmErrorCode SdmLockUtility::TooltalkIsModified(SdmError& err, SdmBoolean& r_isModified)
{

  TNF_PROBE_0 (SdmLockUtility_TooltalkIsModified_start,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkIsModified_start");

  Tt_status ttErrorStatus;

  err = Sdm_EC_Success;
  r_isModified = Sdm_False;		// default return value is not modified by other mailer

  if (!_luLockObjectName.Length()) {
    TNF_PROBE_0 (SdmLockUtility_TooltalkIsModified_end,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkIsModified_end: no object name specified");
    return(err = Sdm_EC_Fail);
  }

  Tt_message mt_msg;

  // Send out a mailtool request to see if another mailer has a lock on this object

  if (TooltalkMessageCreate(err, mt_msg, "tlock", (const char *)_luLockObjectName,
			    TT_REQUEST, TooltalkMailtoolLockCallback) != Sdm_EC_Success) {
    TNF_PROBE_1 (SdmLockUtility_TooltalkIsModified_end,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkIsModified_end: TooltalkMessageCreate failure",
		 tnf_ulong, err, (SdmErrorCode) err);
    return(err);
  }

  if ((ttErrorStatus = tt_message_send(mt_msg)) != TT_OK) {
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				    "tt_message_send",
				    tt_status_message(ttErrorStatus));
    TNF_PROBE_2 (SdmLockUtility_TooltalkIsModified_end,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkIsModified_end: tt_message_send failure",
		 tnf_ulong, ttErrorStatus, (unsigned long) ttErrorStatus,
		 tnf_string, tt_status_message, (const char *)tt_status_message(ttErrorStatus));
    return(err);
  }

  // Now wait for the lock check request to generate a response
  // Timeouts or errors cause us to return an error to the caller

  _luTtMailtoolBlockedFlag = MT_BLOCK_TIMEOUT;	// reset the _luTtMailtoolBlockedFlag to wait for a response

  while (_luTtMailtoolBlockedFlag == MT_BLOCK_TIMEOUT) {
    ttErrorStatus = tttk_block_while((XtAppContext)0, (int*)&_luTtMailtoolBlockedFlag, TooltalkMsTimeout);

    switch (ttErrorStatus) {
    case TT_OK:			// The operation completed successfully.
    case TT_DESKTOP_EINTR:	// Interrupted by a signal
    case TT_DESKTOP_EAGAIN:	// poll returned EAGAIN
      // For these if our request has been processed we can break out; otherwise,
      // continue processing messages
      break;
    case TT_DESKTOP_ETIMEDOUT:	// The  timeout  occurred  
    default:
      // For timeout and error conditions, if our request has been processed
      // then we can continue; however, we have a dilema if not. It means for
      // some reason we sent out a tlock request and did not get back anything
      // within the tooltalk timeout limit. Return a tooltalk not responding error.
      if (_luTtMailtoolBlockedFlag != MT_BLOCK_TIMEOUT)
	break;
      err.SetMajorErrorCode(Sdm_EC_MBL_TooltalkNotResponding);
      TNF_PROBE_2 (SdmLockUtility_TooltalkIsModified_end,
		   "lockapi SdmLockUtility",
		   "sunw%debug SdmLockUtility_TooltalkIsModified_end: tttk_block_while return != TT_OK",
		   tnf_ulong, ttErrorStatus, (unsigned long) ttErrorStatus,
		   tnf_string, tt_status_message, (const char *)tt_status_message(ttErrorStatus));
      return(err);
    }
  }

  // TooltalkMailtoolLockCallback sets _luTtMailtoolBlockedFlag to one of the TTBlockedOp flags:
  // MT_BLOCK_ISLOCKED	//  break out of loop: indicate other mailer has a lock
  // MT_BLOCK_NOLOCK 	// break out of loop: indicate no other mailer has the lock
  // MT_BLOCK_TIMEOUT	// wait for tttk_block_while timeout to occur

  if (_luTtMailtoolBlockedFlag == MT_BLOCK_ISLOCKED) {
    // Another mailer has a lock on the object

    r_isModified = Sdm_True;
    _luTtOtherModified = Sdm_True;		// indicate that another mailer has a lock
    _luTtMailtoolLock = Sdm_True;		// indicate mailtool protocol used to test the lock
    TNF_PROBE_0 (SdmLockUtility_TooltalkIsModified_end,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkIsModified_end: MT_BLOCK_ISLOCKED - other mailer has a pattern lock on object");
  } 
  else {
    // Failed to get the lock status using the mailtool protocol

    _luTtMailtoolLock = Sdm_False;		// indicate mailtool protocol NOT used to get lock

    // now let's try the dtmail protocol
    // TRUE means some mailer has changes pending on file (lock on file), FALSE if not
    if (ttdt_Get_Modified(NULL, (const char *)_luLockObjectName,
			  TT_FILE, NULL, TooltalkMsTimeout)) {
      TNF_PROBE_0 (SdmLockUtility_TooltalkIsModified_ttdt_Get_modified_TRUE,
		   "lockapi SdmLockUtility",
		   "sunw%debug SdmLockUtility_TooltalkIsModified_ttdt_Get_modified: TRUE - other mailer has file scoped lock on object");
      r_isModified = Sdm_True;
      _luTtOtherModified = Sdm_True;
    } 
    else {
      TNF_PROBE_0 (SdmLockUtility_TooltalkIsModified_ttdt_Get_modified_FALSE,
		   "lockapi SdmLockUtility",
		   "sunw%debug SdmLockUtility_TooltalkIsModified_ttdt_Get_modified: FALSE - no mailer has file scoped lock on object");
      r_isModified = Sdm_False;
      _luTtOtherModified = Sdm_False;
    }
  }

  TNF_PROBE_3 (SdmLockUtility_TooltalkIsModified_end,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkIsModified_end",
	       tnf_ulong, TtOtherModified, (unsigned long)_luTtOtherModified,
	       tnf_ulong, TtMailtoolLock, (unsigned long)_luTtMailtoolLock,
	       tnf_ulong, isModified, (unsigned long) r_isModified);
  return(err);
}

SdmErrorCode SdmLockUtility::TooltalkGrabLockFromOtherMailer(SdmError& err)
{
  TNF_PROBE_1 (SdmLockUtility_TooltalkGrabLockFromOtherMailer_start,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkGrabLockFromOtherMailer_start",
	       tnf_ulong, GrabLockIfOwnerByOther, (unsigned long) _luGrabLockIfOwnedByOtherMailer);

  Tt_status ttErrorStatus;
  SdmBoolean isModified = Sdm_False;

  if (_luGrabLockIfOwnedByOtherMailer == Sdm_False) {
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_OtherMailerOwnsWriteAccess, (const char *)_luLockObjectName);
    TNF_PROBE_0 (SdmLockUtility_TooltalkGrabLockFromOtherMailer_end,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkGrabLockFromOtherMailer_end: other mailer has lock / not requested to take it away");
    return(err);
  }

  // TooltalkIsModified sets _luTtMailtoolLock to Sdm_True if the mailbox
  // is locked by mailtool protocol

  SdmLockWaitObject updateLockWait(this, Sdm_LKEC_WaitingForSessionLock, Sdm_LKEC_NotWaitingForSessionLock);

  updateLockWait.IssueStartEvent();

  if (_luTtMailtoolLock == Sdm_True) {
    // mailtool style locking
    TNF_PROBE_0 (SdmLockUtility_TooltalkGrabLockFromOtherMailer_message,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkGrabLockFromOtherMailer_message: other mailer has pattern lock / requesting to take it away");

    Tt_message mt_msg;

    if (TooltalkMessageCreate(err, mt_msg, "rulock", (const char *)_luLockObjectName,
			      TT_NOTICE, NULL) != Sdm_EC_Success) {
      return(err);
    }

    if ((ttErrorStatus = tt_message_send(mt_msg)) != TT_OK) {
      err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				      "tt_message_send",
				      tt_status_message(ttErrorStatus));
      return(err);
    }
    tt_message_destroy(mt_msg);	// WORRY //
  }
  else {
    // ttdt style locking
    TNF_PROBE_0 (SdmLockUtility_TooltalkGrabLockFromOtherMailer_message,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkGrabLockFromOtherMailer_message: other mailer has file scoped lock / requesting to take it away");

    // issue the ttdt_Save call to cause the other mailer to exit
    // Only specific errors are really "bail out" errors so handle them carefully here

    ttErrorStatus = ttdt_Save(NULL, (const char *)_luLockObjectName,
			      TT_FILE, (XtAppContext)_luTtAppContext, TooltalkMsTimeout);
    switch (ttErrorStatus) {
    case TT_ERR_DBAVAIL:	// no tooltalk database available
    case TT_ERR_NOMEM:		// ran out of memory
    case TT_ERR_NOMP:		// no ttsession process
    case TT_ERR_OVERFLOW:	// too many active messages
    case TT_ERR_POINTER:	// path component bad
    case TT_DESKTOP_EPROTO:	// invalid error return status
      err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				      "ttdt_Save",
				      tt_status_message(ttErrorStatus));
      return(err);
    }
  }

  TNF_PROBE_0 (SdmLockUtility_TooltalkGrabLockFromOtherMailer_message,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkGrabLockFromOtherMailer_message: request to take lock away issued / waiting for release");

  // Give the other mailer TooltalkMsTimeout seconds to give up the lock

  time_t t_start;

  time(&t_start);
  int sleeptime = 0;
  while (1) {
    if (sleeptime < 5)
      sleeptime++;
    sleep(sleeptime);
    if (TooltalkIsModified(err, isModified) != Sdm_EC_Success) {
      TNF_PROBE_0 (SdmLockUtility_TooltalkGrabLockFromOtherMailer_TooltalkIsModified,
		   "lockapi SdmLockUtility",
		   "sunw%debug SdmLockUtility_TooltalkGrabLockFromOtherMailer_TooltalkIsModified: FAILED");
      return(err);
    }
    if (isModified == Sdm_False) {
      break;
    } 
    if (time((time_t)NULL) - t_start > TooltalkSecondsTimeout) {
      // time out!
      err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_OtherMailerOwnsWriteAccess, (const char *)_luLockObjectName);
      TNF_PROBE_0 (SdmLockUtility_TooltalkGrabLockFromOtherMailer_end,
		   "lockapi SdmLockUtility",
		   "sunw%debug SdmLockUtility_TooltalkGrabLockFromOtherMailer_end: timeout reached - other mailer refused to give up the lock");
      return(err);
    }
    TNF_PROBE_1 (SdmLockUtility_TooltalkGrabLockFromOtherMailer_message,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkGrabLockFromOtherMailer_message: waiting for other mailer to release lock",
		 tnf_ulong, timeoutSecondsRemaining, (unsigned long) (TooltalkSecondsTimeout - (time((time_t)NULL) - t_start)));
  }

  TNF_PROBE_0 (SdmLockUtility_TooltalkGrabLockFromOtherMailer_message,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkGrabLockFromOtherMailer_message: other mailer released lock");
  return(err);
}


SdmErrorCode SdmLockUtility::TooltalkSessionLockFile(SdmError& err)
{
  SdmLockWaitObject updateLockWait(this, Sdm_LKEC_WaitingForSessionLock, Sdm_LKEC_NotWaitingForSessionLock);

  TNF_PROBE_1 (SdmLockUtility_TooltalkSessionLockFile_start,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkSessionLockFile_start",
	       tnf_ulong, TtHaveWriteAccess, (unsigned long) _luTtHaveWriteAccess);

  Tt_status ttErrorStatus;
  SdmBoolean isModified = Sdm_False;

  err = Sdm_EC_Success;

  // If we have the access, then we locked it before. Simply return.
  //
  if (_luTtHaveWriteAccess == Sdm_True) {
    TNF_PROBE_0 (SdmLockUtility_TooltalkSessionLockFile_end,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkSessionLockFile_end: already have a lock on this object");
    return(err);
  }

  // First step in locking is determining if anyone else has the lock.
  // If they do, then we need to ask them to save their changes and
  // exit.
  //

  if (TooltalkIsModified(err, isModified) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmLockUtility_TooltalkSessionLockFile_end,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkSessionLockFile_end: TooltalkIsModified FAILED");
    return(err);
  }

  if (isModified && TooltalkGrabLockFromOtherMailer(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmLockUtility_TooltalkSessionLockFile_end,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkSessionLockFile_end: TooltalkGrabLockFromOtherMailer FAILED");
    return(err);
  }

  // Set this so we don't call our client during this handshake.
  //
  _luTtPendingFlag = PENDING_LOCK;
  _luTtOutstanding = TT_BLOCK_REQUEST_OUTSTANDING;

  // Now we are ready to lock the mailbox

  // register this pattern so we can handle messages from mailtool
  if (TooltalkMailtoolPatternCreate(err, _luTtMailtoolPattern, (const char *)_luLockObjectName) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmLockUtility_TooltalkSessionLockFile_end,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkSessionLockFile_end: TooltalkMailtoolPatternCreate FAILED");
    return(err);
  }

  if ((ttErrorStatus = tt_pattern_register(_luTtMailtoolPattern)) != TT_OK) {
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				    "tt_pattern_register",
				    tt_status_message(ttErrorStatus));
    TNF_PROBE_2 (SdmLockUtility_TooltalkSessionLockFile_end,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkSessionLockFile_end: tt_pattern_register FAILED",
		 tnf_ulong, ttErrorStatus, (unsigned long) ttErrorStatus,
		 tnf_string, tt_status_message, tt_status_message(ttErrorStatus));
    return(err);
  }

  // Send the message saying we want to be the owner.
  if ((ttErrorStatus = ttdt_file_event(NULL, TTDT_MODIFIED, _luTtFilePatterns, 1)) != TT_OK) {
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_TooltalkLockError,
				    "ttdt_file_event",
				    tt_status_message(ttErrorStatus));
    TNF_PROBE_2 (SdmLockUtility_TooltalkSessionLockFile_end,
		 "lockapi SdmLockUtility",
		 "sunw%debug SdmLockUtility_TooltalkSessionLockFile_end: ttdt_file_event FAILED",
		 tnf_ulong, ttErrorStatus, (unsigned long) ttErrorStatus,
		 tnf_string, tt_status_message, tt_status_message(ttErrorStatus));
    return(err);
  }

  // We need to process any messages that have arrived. We will get our own
  // modified message, which is not terribly interesting. What is interesting
  // is a modified message from someone else. That means that we have a race
  // condition where two processes both asked if the file was being modified,
  // and it wasn't. Then both said they were the owner, which is obviously
  // wrong so we need to blow both off and make them try again. Hopefully
  // there is enough randomness in our clients that the race condition will
  // clear itself up and we won't get here very often.

  int tooltalkTimeoutCount = 0;

  while (_luTtOutstanding == TT_BLOCK_REQUEST_OUTSTANDING) {

    ttErrorStatus = tttk_block_while((XtAppContext)0, (int*)&_luTtOutstanding, TooltalkMsCheckInterval);
    switch (ttErrorStatus) {
    case TT_OK:			// The operation completed successfully.
    case TT_DESKTOP_EINTR:	// Interrupted by a signal
    case TT_DESKTOP_EAGAIN:	// poll returned EAGAIN
      // For these if our request has been processed we can break out; otherwise,
      // continue processing messages
      break;
    case TT_DESKTOP_ETIMEDOUT:	// The  timeout  occurred  
      if (!tooltalkTimeoutCount++)
	updateLockWait.IssueStartEvent();
      if (tooltalkTimeoutCount < TooltalkMsCheckInterval)
	continue;
    default:
      // For timeout and error conditions, if our request has been processed
      // then we can continue; however, we have a dilema if not. It means for
      // some reason we sent out a TTDT_MODIFIED and did not get back anything
      // within the tooltalk timeout limit. In this case err on the side of
      // another mailer owning the lock.
      if (_luTtOutstanding == TT_BLOCK_REQUEST_OUTSTANDING)
	_luTtOtherModified = Sdm_True;
      _luTtOutstanding = TT_BLOCK_NO_REQUEST_OUTSTANDING;
      break;
    }
  }

  updateLockWait.IssueStopEvent();

  // Ok, one way or another the locking request has been handled - see if another
  // mailer has the lock flag; if now, we own it now

  if (_luTtOtherModified == Sdm_True) {
    // Well, we have a race. Fail this lock as will the other process,
    // we hope.
    err.SetMajorErrorCodeAndMessage(Sdm_EC_MBL_OtherMailerOwnsWriteAccess, (const char *)_luLockObjectName);
  }
  else {
    // Okay, we now have the lock.
    _luTtHaveWriteAccess = Sdm_True;
  }

  TNF_PROBE_2 (SdmLockUtility_TooltalkSessionLockFile_end,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_TooltalkSessionLockFile_end",
	       tnf_ulong, ttOtherModified, (unsigned long) _luTtOtherModified,
	       tnf_ulong, TtHaveWriteAccess, (unsigned long) _luTtHaveWriteAccess);
  return(err);
}

// --------------------------------------------------------------------------------
// Lock object specific methods
// --------------------------------------------------------------------------------

// Register a new lock object
// This method takes the signature and this pointer of a lock utility object and adds
// it to the registry of lock utility objects in existence

void SdmLockUtility::RegisterLockObject(SdmObjectSignature objSignature, SdmLockUtility* thisPtr)
{
  TNF_PROBE_0 (SdmLockUtility_RegisterLockObject_start,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_RegisterLockObject_start");

  assert (_luObjectRegistryMutex != NULL);
  // SdmMutexEntry entry(*_luObjectRegistryMutex);
  mutex_lock(_luObjectRegistryMutex);

  _luLockObjectRegistry(-1).SetLockObjectValues(objSignature, thisPtr);
  
  mutex_unlock(_luObjectRegistryMutex);

  TNF_PROBE_1 (SdmLockUtility_RegisterLockObject_end,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_RegisterLockObject_end",
	       tnf_ulong, objSignature, (unsigned long) objSignature);
}

// Unregister Lock Object
// This method takes the signature of an existing lock utility object that has previously
// been registered and removes the object from the registry

void SdmLockUtility::UnregisterLockObject(SdmObjectSignature objSignature)
{
  TNF_PROBE_1 (SdmLockUtility_UnregisterLockObject_start,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_UnregisterLockObject_start",
	       tnf_ulong, objSignature, (unsigned long) objSignature);

  assert (_luObjectRegistryMutex != NULL);
  // SdmMutexEntry entry(*_luObjectRegistryMutex);
  mutex_lock(_luObjectRegistryMutex);

  int maxSlot = _luLockObjectRegistry.ElementCount();
  for (int slot = 0; slot < maxSlot; slot++)
    if (_luLockObjectRegistry(slot)._loObjSignature == objSignature)
      break;

  if (slot < maxSlot) {
    assert(_luLockObjectRegistry(slot)._loThis->_luObjSignature == objSignature);
    _luLockObjectRegistry.RemoveElement(slot);
  }

  mutex_unlock(_luObjectRegistryMutex);

  TNF_PROBE_0 (SdmLockUtility_UnregisterLockObject_end,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_UnregisterLockObject_end");
}

// Lookup lock object by signature
// This method takes a lock utility object signature and looks up the object in the
// registry, returning NULL if no such object is registered, and otherwise returning
// the this pointer of the SdmLockUtility object recorded in the registry that matches
// the object signature provided

SdmLockUtility* SdmLockUtility::LookupLockObjectBySignature(SdmObjectSignature objSignature)
{
  TNF_PROBE_1 (SdmLockUtility_LookupLockObjectBySignature_start,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_LookupLockObjectBySignature_start",
	       tnf_ulong, objSignature, (unsigned long) objSignature);

  assert (_luObjectRegistryMutex != NULL);
  // SdmMutexEntry entry(*_luObjectRegistryMutex);
  mutex_lock(_luObjectRegistryMutex);

  int maxSlot = _luLockObjectRegistry.ElementCount();
  for (int slot = 0; slot < maxSlot; slot++)
    if (_luLockObjectRegistry(slot)._loObjSignature == objSignature) {
      assert(_luLockObjectRegistry(slot)._loThis->_luObjSignature == objSignature);
      break;
    }	

  SdmLockUtility* lockObject = slot < maxSlot ? _luLockObjectRegistry(slot)._loThis : 0;

  mutex_unlock(_luObjectRegistryMutex);

  TNF_PROBE_1 (SdmLockUtility_LookupLockObjectBySignature_end,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_LookupLockObjectBySignature_end",
	       tnf_opaque, lockObject, (void *) lockObject);

  return (lockObject);
}

// Lookup lock object by object i.d.

SdmLockUtility* SdmLockUtility::LookupLockObjectByObjectName(const SdmString& lockObjectName)
{
  TNF_PROBE_1 (SdmLockUtility_LookupLockObjectByObjectName_start,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_LookupLockObjectByObjectName_start",
	       tnf_string, lockObjectName, (const char *) lockObjectName);

  assert (_luObjectRegistryMutex != NULL);
  // SdmMutexEntry entry(*_luObjectRegistryMutex);
  mutex_lock(_luObjectRegistryMutex);

  int maxSlot = _luLockObjectRegistry.ElementCount();
  for (int slot = 0; slot < maxSlot; slot++)
    if (_luLockObjectRegistry(slot)._loThis->_luLockObjectName == lockObjectName) {
      break;
    }	

  SdmLockUtility* lockObject = slot < maxSlot ? _luLockObjectRegistry(slot)._loThis : 0;

  mutex_unlock(_luObjectRegistryMutex);

  TNF_PROBE_2 (SdmLockUtility_LookupLockObjectByObjectName_end,
	       "lockapi SdmLockUtility",
	       "sunw%debug SdmLockUtility_LookupLockObjectByObjectName_end",
	       tnf_opaque, lockObject, (void *) lockObject,
	       tnf_ulong, objSignature, (unsigned long) (lockObject ? lockObject->_luObjSignature : 0));

  return (lockObject);
}
