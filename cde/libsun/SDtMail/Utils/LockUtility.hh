/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */


#ifndef _SDM_LOCKUTILITY_H
#define _SDM_LOCKUTILITY_H

#pragma ident "@(#)LockUtility.hh	1.8 96/09/13 SMI"

#include <iostream.h>
#include <pwd.h>
#include <Tt/tt_c.h>
#include <Tt/tttk.h>

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/ClassIds.hh>
#include <SDtMail/Error.hh>
#include <SDtMail/Prim.hh>
#include <DataStructs/MutexEntry.hh>

// lock types - used to identify the types of locks on objects

typedef SdmBitMask	SdmLockType;		// a single lock type flag
typedef SdmBitMask	SdmLockTypes;		// a lock type flag mask

static const SdmLockType Sdm_LKT_Update = (1L<<0);	// update lock
static const SdmLockType Sdm_LKT_Session = (1L<<1);	// session lock

// lock event - used in lock callback

typedef enum SdmLockEventType_T {
  Sdm_LKEC_SessionLockGoingAway,	// another mailer taking lock - save changes
  Sdm_LKEC_SessionLockTakenAway,	// another mailer stole lock from us - save changes
  Sdm_LKEC_WaitingForSessionLock,	// entered a wait for a session lock
  Sdm_LKEC_NotWaitingForSessionLock,	// no longer waiting for a session lock
  Sdm_LKEC_WaitingForUpdateLock,	// entered a wait for an update lock
  Sdm_LKEC_NotWaitingForUpdateLock	// no longer waiting for an update lock,
} SdmLockEventType;

// SdmLockCallback is a pointer to a function that handles locking object callbacks

typedef void (*SdmLockCallback)(const SdmLockEventType lockEvent, void* clientData);

typedef 
class SdmLockUtility : public SdmPrim {

public:

  SdmLockUtility();
  virtual ~SdmLockUtility();

  static void		DeleteObject(SdmLockUtility& lockObject);
  static SdmErrorCode	ConnectionStartup(SdmError& err, const SdmString& _appName,
					  void* _appContext);
  static SdmErrorCode	ConnectionShutdown(SdmError& err);
  void			DetermineLocks(SdmLockTypes& r_locksInEffect)
  { r_locksInEffect = _luLocksHeld; }
  SdmErrorCode		GrabLockIfOwnedByOtherMailer(SdmError& err, SdmBoolean grabLock);
  SdmErrorCode		IdentifyLockObject(SdmError& err, const SdmString& lockObjectName);
  SdmErrorCode		IdentifyLockCallback(SdmError& err, SdmLockCallback lockCallback, 
					     void* lockCallbackClientData);
  SdmErrorCode		ObtainLocks(SdmError& err, SdmLockTypes locksToGet);
  SdmErrorCode		ReleaseLocks(SdmError& err, SdmLockTypes locksToRelease);
  int			_TooltalkFileDescriptor() { return _luTtFd; }
  void			_IssueCallback(SdmLockEventType event) 
  { if (_luLockCallback) (*_luLockCallback)(event, _luLockCallbackClientData); };

private:

  // We keep a list of each lock object that is in existence - used in tooltalk 
  // callbacks to be able to throw those requests out that reference an object
  // that is already destroyed

  class SdmLockObject {
  private:
    friend class SdmLockUtility;
    SdmLockObject() {};
    virtual ~SdmLockObject() { _loThis = 0; _loObjSignature = ~0; };
    void SetLockObjectValues(SdmObjectSignature& objSignature, SdmLockUtility* thisPtr) 
    { 
      _loObjSignature = objSignature;
      _loThis = thisPtr;
    };
    int operator==(const SdmLockObject& other) const
    {
      return (_loObjSignature == other._loObjSignature && 
             _loThis == other._loThis);
    }
    SdmObjectSignature	_loObjSignature;
    SdmLockUtility*	_loThis;

    friend class SdmVector<SdmLockObject>;
  };

  class SdmLockWaitObject {
  protected:
    friend class SdmLockUtility;
    SdmLockWaitObject(SdmLockUtility* lockUtility, SdmLockEventType startEvent, SdmLockEventType stopEvent) :
      _lwoEventStarted(Sdm_False), _lwoLockUtility(lockUtility), _lwoStartEvent(startEvent), _lwoStopEvent(stopEvent) {};
    virtual ~SdmLockWaitObject() { IssueStopEvent(); }
    void IssueStartEvent() {
      if (!_lwoEventStarted) {
	_lwoEventStarted = Sdm_True;
	_lwoLockUtility->_IssueCallback(_lwoStartEvent);
      }
    };
    void IssueStopEvent() {
      if (_lwoEventStarted) {
	_lwoLockUtility->_IssueCallback(_lwoStopEvent);
	_lwoEventStarted = Sdm_False;
      }
    };
  private:
    SdmLockUtility*	_lwoLockUtility;
    SdmBoolean		_lwoEventStarted;
    SdmLockEventType	_lwoStartEvent;
    SdmLockEventType	_lwoStopEvent;
  };

  // Private methods

  SdmErrorCode		CheckUpdateLockFileOwnership(SdmError& err);
  SdmErrorCode  	ForceReleaseLocks(SdmError& err, SdmLockTypes locksToRelease);
  void			GenerateUniqueUpdateLockId(SdmString& lockId);
  void			GenerateUpdateLockFileName(SdmString& lockFileName);
  void			GenerateUpdateTempLockFileName(SdmString& lockFileName);
  SdmErrorCode		LinkUpdateLockFile(SdmError& err, SdmString& tempUpdateLockFileName, time_t& r_lockAge);
  static SdmLockUtility*	LookupLockObjectBySignature(SdmObjectSignature objSignature);
  static SdmLockUtility*	LookupLockObjectByObjectName(const SdmString& lockObjectName);
  SdmErrorCode		ObtainSessionLock(SdmError& err);
  SdmErrorCode		ObtainUpdateLock(SdmError& err);
  static void		RegisterLockObject(SdmObjectSignature objSignature, SdmLockUtility* thisPtr);
  SdmErrorCode		ReleaseSessionLock(SdmError& err);
  SdmErrorCode		ReleaseUpdateLock(SdmError& err);
  static Tt_message	TooltalkFileLockCallback(Tt_message msg, Tttk_op op,
					     char * path, void *clientData, int same_euid,
					     int same_proc);
  SdmErrorCode		TooltalkGrabLockFromOtherMailer(SdmError& err);
  SdmErrorCode		TooltalkIsModified(SdmError& err, SdmBoolean& r_isModified);
  SdmErrorCode		TooltalkSessionLockFile(SdmError& err);
  static Tt_callback_action	TooltalkMailtoolLockCallback(Tt_message m, Tt_pattern p);
  SdmErrorCode		TooltalkMailtoolPatternCreate(SdmError& err, Tt_pattern& r_pattern,
						      const char* fileOfInterest);
  SdmErrorCode		TooltalkMessageCreate(SdmError& err, Tt_message& r_msg, const char *op,
					      const char *file, const Tt_class tt_class, Tt_message_callback f);
  SdmErrorCode		TooltalkRegisterInterestInLockObject(SdmError& err);
  SdmErrorCode		TooltalkSessionInitialize(SdmError& err);
  static SdmErrorCode		TooltalkSessionTerminate(SdmError& err);
  SdmErrorCode		TooltalkUnregisterInterestInLockObject(SdmError& err);
  static void		UnregisterLockObject(SdmObjectSignature objSignature);

  // Settings for the _luTtPending flag

  typedef enum TTPendingOp_T {
    PENDING_LOCK = 1,
    PENDING_SAVE = 2,
    PENDING_DESTROY = 3
  } TTPendingOp;

  // Settings for the _luTtMailtoolBlockedFlag flag
  // This is fed in as an argument to the tttk_block_while call, which defines
  // these values as follows: the loop is run until either the timeout occurs
  // or the value of _luTtMailtoolBlockedFlag is less than 1.

  typedef enum TTMailtoolBlockedOp_T {
    MT_BLOCK_ISLOCKED = -1,	// break out of loop: indicate other mailer has a lock
    MT_BLOCK_NOLOCK = 0, 	// break out of loop: indicate no other mailer has the lock
    MT_BLOCK_TIMEOUT = 1   	// wait for tttk_block_while timeout to occur
  } TTMailtoolBlockedOp;

  // Settings for the _luTtOutstanding flag
  // This is fed in as an argument to the tttk_block_while call, which defines
  // these values as follows: the loop is run until either the timeout occurs
  // or the value of _luTtOutstanding is less than 1.

  typedef enum TTBlockedOp_T {
    TT_BLOCK_NO_REQUEST_OUTSTANDING = 0,
    TT_BLOCK_REQUEST_OUTSTANDING = 1
  } TTBlockedOp;

  // Private data

  SdmLockCallback     	_luLockCallback;		// initiator callback for events on this lock object
  void*			_luLockCallbackClientData;	// lock object callback client data
  SdmBoolean		_luLockDeletePending;		// delete on this object is pending
  SdmLockTypes		_luLocksHeld;			// locks currently held (in effect)
  SdmString     	_luLockObjectName;		// name of object to lock
  SdmString		_luUpdateLockUniqueId;		// update lock unique i.d.
  SdmString		_luUpdateLockFileName;		// file name used for update lock
  SdmBoolean		_luGrabLockIfOwnedByOtherMailer; // flag indicating if lock should be taken if held by other mailer
  Tt_pattern*		_luTtFilePatterns;		// patterns registered with join
  SdmBoolean		_luTtHaveWriteAccess;		// indicates we have write access to the object
  SdmBoolean		_luTtLockEnabled;		// indicates tooltalk locking is enabled
  TTMailtoolBlockedOp	_luTtMailtoolBlockedFlag;	// flag for tttk_block_while
  SdmBoolean		_luTtMailtoolLock;		// indicates mailtool pattern lock held by other mailer
  Tt_pattern		_luTtMailtoolPattern;		// mailtool pattern registered for this object
  TTPendingOp		_luTtPendingFlag;
  SdmBoolean		_luTtOtherModified;		// other mailer has lock on this object
  TTBlockedOp		_luTtOutstanding;		// indicates file scoped lock message outstanding
  SdmBoolean		_luTtUnlockCallbackInProgress;	// an protocol unlock is in progress
  mutex_t		_luMutex;			// mutex for caller lock on lock object
  mutex_t		_luCallbackMutex;		// mutex for callback lock on this object
  SdmObjectSignature	_luObjSignature;		// unique signature for this lock object

  // Static data members

  static SdmVector<SdmLockObject> _luLockObjectRegistry;
  static SdmString	_luTtAppName;			// application name for tooltalk session
  static void*		_luTtAppContext;		// application context for tooltalk requests
  static char*		_luTtChannel;			// tooltalk channel opened via ttdt_open
  static SdmBoolean	_luTtInitialized;		// Indicates if tooltalk session initialized
  static int		_luTtFd;			// tooltalk session event file descriptor

};

#endif
