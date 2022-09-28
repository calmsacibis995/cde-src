/*
 *+SNOTICE
 *
 *
 *	$Revision: 1.145 $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement between
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel without
 *	Sun's specific written approval.  This document and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993, 1994, 1995, 1996 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)RFCMailBox.C	1.145 06/06/96"
#endif

#ifdef __ppc
#include <DtMail/Buffer.hh>
#endif

#include <EUSCompat.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pwd.h>
#include <ctype.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/uio.h>
#include <sys/systeminfo.h>
#include <assert.h>

#if defined(NEED_MMAP_WRAPPER)
extern "C" {
#endif

#include <sys/mman.h>

#if defined(NEED_MMAP_WRAPPER)
}
#endif

#include <DtMail/DtMail.hh>
#include <DtMail/ImplDriver.hh>
#include <DtMail/Threads.hh>
#include <DtMail/IO.hh>
#include "RFCImpl.hh"

#ifndef MAIL_SPOOL_PATH
#define MAIL_SPOOL_PATH "/var/mail/%s"
#endif

// These macros define a method for executing statements
// with set group id privileges enabled
//
#define PRIV(STATEMENT) { _session->enableGroupPrivileges(); STATEMENT; _session->disableGroupPrivileges(); }
#define PRIV2(STATUSVARIABLE,STATEMENT)   _session->enableGroupPrivileges(); 	\
					  STATUSVARIABLE = STATEMENT; 		\
					  _session->disableGroupPrivileges(); 	\
					  if (STATUSVARIABLE == -1) {		\
					      STATUSVARIABLE = STATEMENT;	\
					  }

static const int RFCSignature = 0x448612e5;

static const int DEFAULT_FOLDER_SIZE = (32 << 10); // 32 KB

struct NewMailData : public DtCPlusPlusAllocator {
    RFCMailBox *	self;
    Condition *		object_valid;
};

static
void HexDump(FILE *pfp, char *pmsg, unsigned char *pbufr, int plen, int plimit)
{
  unsigned char   save[64];
  long int        x, y, z, word, cnt;
  FILE *pfp_r = pfp;

  if (!plen)
    return;
  
  if (pfp_r == (FILE*) NULL) {
    pfp_r = fopen("/tmp/dtmail.dump", "a");
    const time_t clockTime = (const time_t) time((time_t *)0);
    fprintf(pfp_r, "--------------------- pid=%d %s", getpid(), ctime(&clockTime));
  }

  (void) fprintf(pfp_r, "--> %s (%d bytes at 0x%08lx):\n", pmsg, plen, pbufr);
  fflush(pfp_r);
  memset((char *)save, 0, sizeof(save));
  save[0] = ~pbufr[0];
  z = 0;
  cnt = plen;
  for (x = 0; cnt > 0; x++, z += 16)
  {
    (void) fprintf(pfp_r, "0x%08lx(+%6.6ld) ", pbufr + z, z);
    for (y = 0; y < 16; y++)
    {
      save[y] = pbufr[x * 16 + y];
      word = pbufr[x * 16 + y];
      word &= 0377;
      if (y < cnt)
	(void) fprintf(pfp_r, "%2.2lx%c", word, y == 7 ? '-' : ' ');
      else
	(void) fprintf(pfp_r, "   ");
    }
    (void) fprintf(pfp_r, "%s", " *");
    for (y = 0; y < 16; y++)
      if (y >= cnt)
	(void) fprintf(pfp_r, " ");
      else if (pbufr[x * 16 + y] < ' ' || pbufr[x * 16 + y] > '~')
	(void) fprintf(pfp_r, "%s", ".");
      else
	(void) fprintf(pfp_r, "%c", pbufr[x * 16 + y]);
    (void) fprintf(pfp_r, "%s", "*\n");
    cnt -= 16;

    if (plimit && (x >= (plimit-1)) && (cnt > (plimit*16))) {
      while (cnt > (plimit*16)) {
	x++;
	z+=16;
	cnt -= 16;
      }
      fprintf(pfp_r, "...\n");
    }
  }
  fflush(pfp_r);
  if (pfp == (FILE*) NULL) {
    fprintf(pfp_r, "---------------------\n");
    fclose(pfp_r);
  }
}

RFCMailBox::RFCMailBox(DtMailEnv & error,
		       DtMail::Session * session,
		       DtMailObjectSpace space,
		       void * arg,
		       DtMailCallback cb,
		       void * client_data,
		       const char * impl_name)
: DtMail::MailBox(error, session, space, arg, cb, client_data),
_msg_list(128), _mappings(4)
{
    // We are using a condition to block any threads from
    // trying to use this object until it is open. We will
    // not set the object valid until we have a valid thread.
    // Note that this will not be set to true until a stream
    // has been successfully opened.
    //
    DtMailEnv localError;
    localError.clear();
    
    _object_valid = new Condition;

    _object_valid->false();

    _map_lock = MutexInit();

    _impl_name = impl_name;

    _buffer = NULL;
    _parsed = DTM_FALSE;
    _fd = -1;
    _real_path = NULL;

    _mail_box_writable = DTM_FALSE;
    _use_dot_lock = DTM_TRUE;
    _long_lock_active = DTM_FALSE;
    _dot_lock_active = DTM_FALSE;
    _lockf_active = DTM_FALSE;
    _uniqueLockId = generateUniqueLockId();
    assert(_uniqueLockId);
    _uniqueLockIdLength = strlen(_uniqueLockId);
    _lockFileName = (char *)0;
    _dirty = 0;

#ifdef DEAD_WOOD
    // Create a thread for getting new mail and expunging old mail.
    // Of course we don't need threads for buffer objects.
    //
    if (_space != DtMailBufferObject) {
	NewMailData * thr_info = new NewMailData;
	thr_info->self = this;
	thr_info->object_valid = _object_valid;
	_mbox_daemon = ThreadCreate(ThreadNewMailEntry, thr_info);
    }
#endif

    // We will have to add a poll oriented method as well. We'll ignore
    // that for now.
    //
    if (_space != DtMailBufferObject) {
	_session->addEventRoutine(error, PollEntry, this, 15);
	_last_check = 0;
	_last_poll = 0; // Causes first poll to fire right way.
    }

    // We need to figure out what type of locking to use. We use ToolTalk,
    // unless the user has explicitly turned it off via a property.
    //
    DtMail::MailRc * mailrc = _session->mailRc(error);
    const char * value = NULL;
    mailrc->getValue(localError, "cdenotooltalklock", &value);
    if (value && *value != 'f') {
      _tt_lock = DTM_FALSE;
    } else {
      _tt_lock = DTM_TRUE;
    }
    if (value) {
	    free((void *)value);
	    value = NULL;
    }
    
    // Determine if error logging is enabled
    //
    localError.clear();
    mailrc->getValue(localError, "errorlogging", &value);
    _errorLogging = (localError.isSet() ? (localError.clear(), DTM_FALSE) : DTM_TRUE);
    if (value) {
	    free((void *)value);
	    value = NULL;
    }

    _partialList = NULL;
    _partialListCount = 0;

    error.clear();

    _object_signature = RFCSignature;
}

RFCMailBox::~RFCMailBox(void)
{
    if (_object_signature != RFCSignature) {
	return;
    }

    MutexLock lock_scope(_obj_mutex);
    if (_object_signature == RFCSignature) {
	_object_valid->false();

	DtMailEnv error;
	_session->removeEventRoutine(error, PollEntry, this);

	// We need to copy the file if writable and dirty out.
	// NOTE: the caller should really call writeMailBox() or expunge()
	// (as appropriate) first before destroying this mailbox, as there
	// is no way to pass an error indication back up from here to the
	// caller. If an error happens let syslog handle it.
	//
	if (_mail_box_writable == DTM_TRUE) {
	    if (_dirty != 0) {
		error.clear();
		writeMailBox(error);
		if (error.isSet()) {
		    // THE MAILBOX COULD NOT BE WRITTEN!! SHOULD DO SOMETHING
		    error.logError(DTM_TRUE,
			   "RFCMailBox::~RFCMailBox(): Failed to write mailbox upon closure: %s",
			   (const char *)error);
		}
	    }
	}

	// Let's keep the map locked.
	//
	MutexLock lock_map(_map_lock);

	// Next we tear down the message structures.
	//
	while (_msg_list.length()) {
	    MessageCache * mc = _msg_list[0];
	    delete mc->message;
	    delete mc;
	    _msg_list.remove(0); // Won't actually touch the object.
	}

	// Finally we need to get rid of the mapping. There are
	// actually 3 conditions we need to deal with here.
	//
	// 1) We opened a file and mapped it. In that case, we unmap,
	//    and close the file.
	//
	// 2) We created a buffer. In that case the _mapped_region points,
	//    at the region owned by _buffer. We simply destroy _buffer,
	//    but we do not attempt to destroy the mapped region.
	//
	// 3) The caller asked us to open an existing buffer. In that
	//    case, do nothing (we didn't build it so we won't destroy it).
	//
	if (_fd >= 0) {
	    // Loop through the current mappings, and unmap each.
	    // Then clean up the memory used for the MapRegion.
	    //
	    while(_mappings.length()) {
     	    	MapRegion * c_map = _mappings[0];
     	    	munmap(c_map->map_region, c_map->map_size);
     	    	delete c_map;
     	    	_mappings.remove(0);
	    }
	    longUnlock(error);
	    SafeClose(_fd);
	}

	if (_buffer) { // Option 2
	    delete _buffer;
	}

	delete _real_path;
	delete _uniqueLockId;
	delete _lockFileName;
    }

    _object_signature = 0;
    delete _object_valid;
}

static int isPathSystemMailbox(const char * mailboxPath)
{
  static int oneTimeFlag = 0;
  static char *cached_inbox_path = 0;

  assert(mailboxPath);

  if (!oneTimeFlag) {
    char inbox_path[MAXPATHLEN];
    passwd pw;

    GetPasswordEntry(pw);
    sprintf(inbox_path, MAIL_SPOOL_PATH, pw.pw_name);
    cached_inbox_path = strdup(inbox_path);
    oneTimeFlag++;
  }

  assert(cached_inbox_path);
  
  return (!strcmp(mailboxPath, cached_inbox_path));
}

// Function: alterPageMappingAdvice - change OS mapping advice on selected map
// Description:
//  Give the operating system new advice on the referencing activity that
//  should be expected for a region of file mapped pages.
// Method:
//  Spin through all map regions recorded in _mappings looking for the
//  specified map (or all maps if -1 passed in as the map region), and
//  if found issue an madvise call on the specified region of memory.
// Arguments:
//  map		-- either a specific region that is part of _mappings,
//		-- OR (MapRegion *)-1 to change ALL map regions in _mappings
//  advice	-- advice as per madvise(3) Operating System call
// Outputs:
//  <none>
// Returns:
//  <none>
//
void
RFCMailBox::alterPageMappingAdvice(MapRegion *map, int advice)
{
  int me = _mappings.length();
  for (int m = 0; m < me; m++) {
    MapRegion *map_t = _mappings[m];
#if !defined(USL) /* no madvise so we lose this optimization */
    if (map_t == map || map == (MapRegion *)-1)
      madvise(map_t->map_region, map_t->map_size, advice);
#endif
    }
}

// Function: memoryPageSize - return hardware natural memory page size
// Description:
//  Compute and return the natural memory page size of the current hardware
// Method:
//  Use sysconf to query the operating system about the current hardware;
//  cache the value so that repeated calls to this function do not generate
//  repeated calls to the operating system.
// Arguments:
//  <none>
// Outputs:
//  <none>
// Returns:
//  long -- natural page size of the current hardware
// 
static long
memoryPageSize()
{
  static long mach_page_size = -1;		// -1 is "one time value"

  if (mach_page_size == -1) {
    mach_page_size = sysconf(_SC_PAGESIZE);
    assert(mach_page_size != -1);
  }

  return(mach_page_size);
}

// Functions: addressIsMapped - check if address is in file mapped memory
// Description:
//  Check to see if a given memory address is within the bounds of any
//  memory that may have been mapped from a mailbox file into memory,
//  and return an indication of whether it is or not.
// Method:
//  Go through the list of file to memory mappings and check to see if
//  the given address is within the bounds of one of the mappings.
// Arguments:
//  addressToCheck -- address in memory to check against mappings
// Outputs:
//  <none>
// Returns:
//  DTM_TRUE - memory address is in file mapped memory and can be accessed
//  DTM_FALSE - memory address is not in file mapped memory - may not be valid
//
DtMailBoolean
RFCMailBox::addressIsMapped(void *addressToCheck)
{
  assert(addressToCheck);
  int me = _mappings.length();
  for (int m = 0; m < me; m++)
  {
    MapRegion *map = _mappings[m];
    if ( (addressToCheck >= map->map_region)
	 && (addressToCheck < (map->map_region+map->file_size)) )
      return(DTM_TRUE);
  }
  return(DTM_FALSE);
}

void
RFCMailBox::create(DtMailEnv & error, mode_t create_mode)
{
    error.clear();

#if defined(MAILGROUP_REQUIRED)

    char grp_name[100];
    GetGroupName(grp_name);
    if (strcmp(grp_name, DTMAIL_DEFAULT_CREATE_MAILGROUP)) {
	error.setError(DTME_BadRunGroup);
	return;
    }

#endif

    MutexLock lock_scope(_obj_mutex);

    MutexLock lock_map(_map_lock);

    switch (_space) {
      case DtMailBufferObject:
      {
	  DtMailBuffer * buf = (DtMailBuffer *)_arg;

	  _buffer = (char *)malloc(DEFAULT_FOLDER_SIZE);
	  buf->buffer = _buffer;
	  buf->size = DEFAULT_FOLDER_SIZE;

	  MapRegion * map = new MapRegion;
	  map->file_region = map->map_region = _buffer;
	  map->file_size = map->map_size = DEFAULT_FOLDER_SIZE;
	  map->offset = 0;
	  _mappings.append(map);

	  _mail_box_writable = DTM_TRUE;

	  break;
      }

      case DtMailFileObject:
      {
	  openRealFile(error, O_RDWR | O_CREAT, create_mode);
	  if (error.isSet()) {
	      break;
	  }

	  mapFile(error);
	  break;
      }

      default:
	error.setError(DTME_NotSupported);
    }

    _at_eof.true();

    _object_valid->true();

    return;
}

void
RFCMailBox::open(DtMailEnv & error,
		 DtMailBoolean auto_create,
		 int open_mode, 
		 mode_t create_mode,
		 DtMailBoolean lock_flag,
		 DtMailBoolean auto_parse
)
{
    error.clear();

#if defined(MAILGROUP_REQUIRED)

    char grp_name[100];

    if ( open_mode != O_RDONLY ) {
        GetGroupName(grp_name);
        if (strcmp(grp_name, DTMAIL_DEFAULT_CREATE_MAILGROUP)) {
	    error.setError(DTME_BadRunGroup);
	    return;
        }
    } 

#endif

    if (_tt_lock == DTM_TRUE && lock_flag == DTM_TRUE)
      _lock_flag = DTM_TRUE;
    else
      _lock_flag = DTM_FALSE;

    MutexLock lock_scope(_obj_mutex);

    if (auto_parse) {
	
	MutexLock lock_map(_map_lock);

	switch (_space) {
	  case DtMailBufferObject:
	  {
	      DtMailBuffer * buf = (DtMailBuffer *)_arg;

	      MapRegion * map = new MapRegion;
	      map->file_region = map->map_region = (char *)buf->buffer;
	      map->file_size = map->map_size = buf->size;
	      map->offset = 0;
	      _mappings.append(map);

	      _mail_box_writable = DTM_FALSE;

	      break;
	  }
	    
	  case DtMailFileObject:
          {
	    int mode = O_RDONLY;
	    int return_result;
	    
	    _mail_box_writable = DTM_FALSE;
	    char * path = _session->expandPath(error, (char *)_arg);
	    PRIV2(return_result, SafeAccess(path, W_OK));
	    if (return_result == 0) {
		mode = O_RDWR;
	    }
	    free(path);

	    // We need to use the most restrictive mode that is possible
	    // on the file. If the caller has requested the file be open
	    // read-only, then we should do that, even if read-write is
	    // allowed. We don't want to try to open the file read-write
	    // if we don't have adequate permission however.
	    //
	    mode = open_mode == O_RDONLY ? open_mode : mode;
	    
	    openRealFile(error, mode, create_mode);
	    if (error.isSet()) {
		if (auto_create == DTM_TRUE) {
		    error.clear();
		    lock_scope.unlock();
		    lock_map.unlock();
		    create(error);
		}
		return;
	    }
	    
	    mapFile(error);
	    break;
	}
	  
	default:
	  error.setError(DTME_NotSupported);
      }

      if (error.isSet()) { // Can't parse this.
	  return;
      }

      _at_eof.false();

      lock_map.unlock();

#if defined(POSIX_THREADS)
      ThreadCreate(ThreadParseEntry, this);
#else
      parseFile(error, 0);
#endif

      _object_valid->true(); // New mail watcher starts now.
      _parsed = DTM_TRUE;
    }
    else {
	
	// Open file.
	
	int mode = O_RDONLY;
	int return_status;
	
	_mail_box_writable = DTM_FALSE;
	char * path = _session->expandPath(error, (char *)_arg);
	PRIV2(return_status, SafeAccess(path, W_OK));
	if (return_status == 0) {
	    mode = O_RDWR;
	}
	free(path);
	
	// We need to use the most restrictive mode that is possible
	// on the file. If the caller has requested the file be open
	// read-only, then we should do that, even if read-write is
	// allowed. We don't want to try to open the file read-write
	// if we don't have adequate permission however.
	//
	mode = open_mode == O_RDONLY ? open_mode : mode;
	
	openRealFile(error, mode, create_mode);
	if (error.isSet()) {
	    if (auto_create == DTM_TRUE) {
		error.clear();
		
		// Legacy code
		// Isn't this wrong?  Shouldn't create() be called
		// before unlocking lock_scope?
		
		lock_scope.unlock();
		create(error);
	    }
	    return;
	}

	// Validate this file if it's not empty
	if (_file_size) {
	  // When move/copy to a file, we want to make sure the first
	  // five characters are "From "
	  char inbuf[6];

	  pread(_fd, (void *)inbuf, 5, 0);
	  inbuf[5] = (char)0;

	  if (strcmp(inbuf, "From ") != 0) {
	    error.setError(DTME_NotMailBox);
	    return;
	  }
	}
	
	_at_eof.false();
	_parsed = DTM_FALSE;
    }

    _object_valid->true(); // New mail watcher starts now.
    lock_scope.unlock();
    return;
}

#ifdef DEAD_WOOD
int
RFCMailBox::messageCount(DtMailEnv & error)
{
	error.clear();

    _at_eof.waitTrue();

    if (_object_valid->state() <= 0) {
	error.setError(DTME_ObjectInvalid);
	return(0);
    }

    return(_msg_list.length());
}
#endif /* DEAD_WOOD */

DtMailMessageHandle
RFCMailBox::getFirstMessageSummary(DtMailEnv & error,
				    const DtMailHeaderRequest & request,
				    DtMailHeaderLine & summary)
{
	error.clear();

    waitForMsgs(0);

    if (_object_valid->state() <= 0) {
	error.setError(DTME_ObjectInvalid);
	return(NULL);
    }

    int slot = nextNotDel(0);

    if (slot >= _msg_list.length()) {
	return(NULL);
    }

    makeHeaderLine(error, 0, request, summary);

    return(_msg_list[0]);
}

DtMailMessageHandle
RFCMailBox::getNextMessageSummary(DtMailEnv & error,
				   DtMailMessageHandle last,
				   const DtMailHeaderRequest & request,
				   DtMailHeaderLine & summary)
{
    if (_object_valid->state() <= 0) {
	error.setError(DTME_ObjectInvalid);
	return(NULL);
    }

    // Let's treat a null last as the start of the list.
    //
    if (last == NULL) {
	return(getFirstMessageSummary(error, request, summary));
    }

	error.clear();

    int slot = _msg_list.index((MessageCache *)last);
    if (slot < 0) {
	return(NULL);
    }

    slot += 1;
    waitForMsgs(slot);

    slot = nextNotDel(slot);

    if (slot >= _msg_list.length()) {
	return(NULL);
    }

    makeHeaderLine(error, slot, request, summary);

    return(_msg_list[slot]);
}

void
RFCMailBox::getMessageSummary(DtMailEnv & error,
			       DtMailMessageHandle handle,
			       const DtMailHeaderRequest & request,
			       DtMailHeaderLine & summary)
{
    MutexLock lock_map(_map_lock);

    if (_object_valid->state() <= 0) {
	error.setError(DTME_ObjectInvalid);
	return;
    }

	error.clear();

    int slot = _msg_list.index((MessageCache *)handle);
    if (slot < 0 || slot >= _msg_list.length()) {
	error.setError(DTME_ObjectInvalid);
	return;
    }

    makeHeaderLine(error, slot, request, summary);

    return;
}

DtMail::Message *
RFCMailBox::getMessage(DtMailEnv & error, DtMailMessageHandle hnd)
{
    if (_object_valid->state() <= 0) {
	error.setError(DTME_ObjectInvalid);
	return(NULL);
    }

    error.clear();

    int slot = _msg_list.index((MessageCache *)hnd);
    if (slot < 0) {
	error.setError(DTME_ObjectInvalid);
	return(NULL);
    }

    MessageCache * mc = _msg_list[slot];
    return(mc->message);
}

DtMail::Message *
RFCMailBox::getFirstMessage(DtMailEnv & error)
{
    error.clear();

    waitForMsgs(0);

    if (_object_valid->state() <= 0) {
	error.setError(DTME_ObjectInvalid);
	return(NULL);
    }

    int slot = nextNotDel(0);

    if (slot >= _msg_list.length()) {
	return(NULL);
    }

    MessageCache * mc = _msg_list[slot];
    return(mc->message);
}

DtMail::Message *
RFCMailBox::getNextMessage(DtMailEnv & error,
			   DtMail::Message * last)
{
    error.clear();

    int slot = lookupByMsg((RFCMessage *)last);
    if (slot < 0) {
	return(NULL);
    }

    slot += 1;
    waitForMsgs(slot);

    slot = nextNotDel(slot);

    if (slot >= _msg_list.length()) {
	return(NULL);
    }

    MessageCache * mc = _msg_list[slot];
    return(mc->message);
}

void
RFCMailBox::copyMessage(DtMailEnv & error,
			 DtMail::Message * msg)
{
    if (_object_valid->state() <= 0) {
	error.setError(DTME_ObjectInvalid);
	return;
    }
	
    error.clear();
    
    // The following is a hack for PAR0.5. In the future, we will use
    // this test for an optimization, but right now, we can only copy
    // RFC->RFC.
    //
    const char * msg_impl = msg->impl(error);
    if (strcmp(msg_impl, "Internet MIME") != 0 &&
	strcmp(msg_impl, "Sun Mail Tool") != 0) {
	error.setError(DTME_NotSupported);
	return;
    }
    
    RFCMessage * rfc_msg = (RFCMessage *)msg;
    
    // We need to protect the file from access. Locking this will also
    // block any attempts by the new mail thread.
    //
    error.clear();
      
    lockFile(error);
    if (error.isSet()) {
	return;
    }
    
    
    int status;
    
    off_t end = lseek(_fd, 0, SEEK_END);
    status = SafeWrite(_fd, rfc_msg->_msg_start, 
	rfc_msg->_msg_end - rfc_msg->_msg_start + 1);
    
    // We are going to put this at the real end of the file. We don't
    // really care what the current thought size is because new mail
    // will take care of that problem.
    //

    // Add a new-line at the end.  
    // It serves to distinguish separate messages.

    SafeWrite(_fd, "\n", 1);

    unlockFile(error, _fd);
    
    if (status < 0) {
	error.setError(DTME_ObjectCreationFailed);
    }
    
    return;
}

void
RFCMailBox::copyMailBox(DtMailEnv & error, DtMail::MailBox * mbox)
{
    error.clear();

    for (DtMail::Message * msg = mbox->getFirstMessage(error);
	 msg && error.isNotSet();
	 msg = mbox->getNextMessage(error, msg)) {
	copyMessage(error, msg);
	if (error.isSet()) {
	    return;
	}
    }
}

void
RFCMailBox::checkForMail(
    DtMailEnv & error,
    const DtMailBoolean already_locked
)
{
    error.clear();

    if (_space != DtMailFileObject) {
	error.setError(DTME_NotSupported);
	return;
    }

    NewMailEvent(error, already_locked);
}

void
RFCMailBox::expunge(DtMailEnv & error)
{
    error.clear();

    for (int msg = 0; msg < _msg_list.length(); msg++) {
	MessageCache * mc = _msg_list[msg];
	if (mc->delete_pending == DTM_FALSE) {
	    DtMail::Envelope * env = mc->message->getEnvelope(error);
	    
	    DtMailValueSeq value;
	    env->getHeader(error, RFCDeleteHeader, DTM_FALSE, value);
	    if (!error.isSet()) {
		mc->delete_pending = DTM_TRUE;
	    }
	    else {
		error.clear();
	    }
	}
    }

    error.clear();
    writeMailBox(error);
    if (error.isSet()) {
	// THE MAILBOX COULD NOT BE WRITTEN!! CALLER MUST DO SOMETHING
	error.logError(DTM_TRUE, "RFCMailBox::expunge(): Failed to write mailbox: %s", (const char *)error);
    }
    // If "keep" is not set, and the mailbox is empty, unlink it.
    //
    // gregl - We actually don't want to unlink the mailbox just because
    // the last message was expunged.  This has the nasty side effect of
    // preventing new mail from ever being incorporated into this mailbox.
    // For new users with very little mail, this poses a major problem.
    // The mailbox should only be unlinked if you close an empty mailbox.
//     DtMail::MailRc * mailrc = _session->mailRc(error);
//     DtMailBoolean keep = DTM_FALSE;
//     const char * value = NULL;

//     mailrc->getValue(error, "keep", &value);
//     if (error.isNotSet()) {
// 	keep = DTM_TRUE;
// 	if (value != NULL) {
// 	    free((void *) value);
// 	    value = NULL;
// 	}
//     }
//     error.clear();
//     // XXX - For some reason, C++ makes you put the 0 first instead of last.
//     if (!keep && 0 == _file_size)
// 	SafeUnlink(_real_path);
//     // XXX - What happens if someone keeps using the (empty) mailbox after
//     // the file has been removed?
}

const char *
RFCMailBox::impl(DtMailEnv & error)
{
    error.clear();
    return(_impl_name);
}

void
RFCMailBox::markDirty(const int delta)
{
    _dirty += delta;
    if (!_dirty) {
	// Make sure we really are dirty.
	_dirty += 1;
    }
}

void
RFCMailBox::callCallback(DtMailCallbackOp op, void * arg)
{
    if (_object_signature != RFCSignature || !_object_valid->state()) {
	return;
    }

    const char * path;
    if (_space == DtMailFileObject) {
	path = (char *)_arg;
    }
    else {
	path = NULL;
    }

    _callback(op, path, NULL, _cb_data, arg);

    return;
}

DtMail::Message *
RFCMailBox::newMessage(DtMailEnv & error)
{
    // RFC does not support a straightforward concept of adding
    // a message to a mailbox. We implement move/copy with a sort
    // of kludge, but we can only extend the kludge so far.
    //
    error.setError(DTME_NotSupported);
    return(NULL);
}

void
RFCMailBox::openRealFile(DtMailEnv & error, int open_mode, mode_t create_mode)
{
    int return_status;
    
    // We first want to check the access modes we have on the
    // file. If we can write the file, then we will need to
    // root out the real path just to make sure we don't munge
    // a link someplace.
    //
    struct stat buf;
    char * path = _session->expandPath(error, (char *)_arg);
    if (error.isSet()) {
      return;		// could not expand path
    }

    if ((open_mode & O_RDWR) == O_RDONLY) {
	_real_path = strdup(path);
	SafeStat(_real_path, &buf);
    }
    else {
	_real_path = (char *)malloc(MAXPATHLEN);
	char link_path[MAXPATHLEN];
	strcpy(link_path, path);
	strcpy(_real_path, path);

	if (SafeLStat(link_path, &buf) == 0 && (open_mode & O_CREAT) == 0) {
	    while(S_ISLNK(buf.st_mode)) {
		int size = readlink(link_path, _real_path, sizeof(link_path));
	    	if (size < 0) {
		    error.setError(DTME_NoSuchFile);
		    free(_real_path);
		    _real_path = (char *)0;
		    free(path);
		    path = (char *)0;
		    return;
		}

		_real_path[size] = 0;
		if (_real_path[0] == '/') {
		    strcpy(link_path, _real_path);
		}
		else {
		    char * last_slash = strrchr(link_path, '/');
		    if (last_slash) {
			strcpy(last_slash + 1, _real_path);
		    }
		    else {
			strcpy(link_path, "./");
			strcat(link_path, _real_path);
		    }
		}

		strcpy(_real_path, link_path);

		if (SafeLStat(link_path, &buf)) {
		    error.setError(DTME_NoSuchFile);
		    free(_real_path);
		    _real_path = (char *)0;
		    free(path);
		    path = (char *)0;
		    return;
		}
	    }
	}
	else {
	    if ((open_mode & O_CREAT) == 0) {
		error.setError(DTME_NoSuchFile);
		free(_real_path);
		_real_path = (char *)0;
		free(path);
		path = (char *)0;
		return;
	    }
	}
    }

    free(path);

    // We should now have a path we can open or create.
    // We must now make sure that if the file is being created that
    // it is created with the correct permissions and group owner.
    //
    // If the mailbox to be created is NOT the system mailbox for this
    // user (e.g. the one that official delivery agents use), then the
    // permissions for the file should only let the current user have access.
    //
    // If the mailbox to be created IS the system mailbox for this user,
    // and MAILGROUP_REQUIRED is defined, we must allow group read/write
    // and make sure the mailbox is owned by the correct group
    mode_t oldUmask;
    int isSystemMailbox = 0;
    int saveErrno = 0;
    isSystemMailbox = isPathSystemMailbox(_real_path);

#if defined(MAILGROUP_REQUIRED)
    // delivery agent runs as specific non-root/wheel group
    if ( (open_mode & O_CREAT) && isSystemMailbox) {
      oldUmask = umask(DTMAIL_DEFAULT_CREATE_UMASK_MAILGROUP);
      create_mode = DTMAIL_DEFAULT_CREATE_MODE_MAILGROUP;
    }
    else {
      oldUmask = umask(DTMAIL_DEFAULT_CREATE_UMASK);
    }
#else
    oldUmask = umask(DTMAIL_DEFAULT_CREATE_UMASK);
#endif
    
    // We have 2 choices for locking an RFC file. The first is
    // to use the ToolTalk file scoping paradigm, and the second
    // is the normal lockf protocol. If we are using ToolTalk,
    // we need to initialize the ToolTalk locking object.
    //
    // Unfortunately lockf() on an NFS mounted file system will
    // upset mmap(). A quick hack is to not use lockf() on any
    // remotely mounted file. Since mailx doesn't do tooltalk
    // locking now, it is possible to corrupt the spool file when
    // using mailx while mailtool has a NFS mounted mail file loaded.
    //
    // We should find a solution where mailtool and mailx work
    // well together.
    //
    // open the file before we obtain the lock can cause sync problem
    // if another owns the lock and has modified the mailbox

    DTMBX_LONGLOCK answer = DTMBX_LONGLOCK_SUCCESSFUL;

    if (_lock_flag == DTM_TRUE) {
      answer = longLock(error);
      if (answer == DTMBX_LONGLOCK_FAILED_CANCEL) {
	free(_real_path);
	_real_path = (char *)0;
	return;
      }

      // we have obtained the lock, go ahead and open the mailbox
      // First, attempt to open the mailbox without privileges; if that
      // fails, and if this is the system mailbox, and we are trying to
      // create it, then attempt to do so with privileges enabled.
      //
      _fd = SafeOpen(_real_path, open_mode, create_mode);
      if ( (_fd == -1) && (open_mode & O_CREAT) && isSystemMailbox) {
	PRIV2(_fd, SafeOpen(_real_path, open_mode, create_mode));
      }
      saveErrno = errno;
    }
#if !defined(SENDMAIL_LOCKS)
    // On some systems, sendmail uses lockf while delivering mail.
    // We can not hold a lockf style lock on those systems. Of course
    // if the user has turned off ToolTalk locking, they are playing
    // without a net...
    //
    else {
      
      _fd = SafeOpen(_real_path, open_mode, create_mode);
      if ( (_fd == -1) && (open_mode & O_CREAT) && isSystemMailbox) {
	PRIV2(_fd, SafeOpen(_real_path, open_mode, create_mode));
      }
      saveErrno = errno;
      if ( (_fd >= 0) && ( (open_mode & O_RDWR) == O_RDWR) ) {
	// Only attempt lockf() if file was opened for reading and writing
	//
	switch (DtMail::DetermineFileLocality(_real_path)) {
	case Dtm_FL_UNKNOWN:	// locality unknown -- assume local and lock
	case Dtm_FL_LOCAL:	// locality local - apply lock
	  assert(_lockf_active == DTM_FALSE);
	  lseek(_fd, 0, SEEK_SET);
	  PRIV2(return_status,SafeLockf(_fd, F_TLOCK, 0));
	  if (return_status == -1) {
	    error.setError(DTME_OtherOwnsWrite);
	    SafeClose(_fd);
	    return;
	  } else {
	    // _long_lock_active = DTM_TRUE;
	    _lockf_active = DTM_TRUE;
	  }
	  break;
	default:		// locality otherwise -- assume remote dont lock
	  _long_lock_active = DTM_FALSE;
	  break;      
	}
      }
    }
#endif

    (void) umask(oldUmask);
    
    if (_fd < 0) {
	longUnlock(error);

	switch (saveErrno) {
	  case EACCES:
	    error.setError(DTME_NoPermission);
	    break;

	  case EISDIR:
	    error.setError(DTME_IsDirectory);
	    break;

	  case ENOENT:
	    error.setError(DTME_NoSuchFile);
	    break;

	  default:
	    error.setError(DTME_ObjectAccessFailed);
	}

	free(_real_path);
	_real_path = (char *)0;

	return;
    }

#if defined(MAILGROUP_REQUIRED)
    // Make sure a newly created file has the correct group owner i.d.
    //
    if ( (open_mode & O_CREAT) && isSystemMailbox) {
      gid_t groupId = GetIdForGroupName(DTMAIL_DEFAULT_CREATE_MAILGROUP);
      if (groupId >= 0)
        (void) SafeFchown(_fd, -1, groupId);
    }
#endif

    if ((open_mode & 0x3) == O_RDWR) {
      if (answer == DTMBX_LONGLOCK_SUCCESSFUL || answer == DTMBX_LONGLOCK_FAILED_READWRITE)
	_mail_box_writable = DTM_TRUE;
      else
	_mail_box_writable = DTM_FALSE;
    }
    else {
      _mail_box_writable = DTM_FALSE;
    }
    error.clear();

    _file_size = lseek(_fd, 0, SEEK_END);
    lseek(_fd, 0, SEEK_SET);
    _links = buf.st_nlink;

    _lockFileName = generateLockFileName();
    assert(_lockFileName);
    return;
}

off_t
RFCMailBox::realFileSize(DtMailEnv & error)
{
    // We have to deal with the problem of editors that will unlink
    // the file we are watching, or other processes that may change
    // the file out from under us. We defend against this by checking
    // to see if the inode number of the mailbox has changed. If it has
    // then some process has not honored our lock on the mailbox, and
    // so we return an error.
    //
    struct stat tempStatbuf, statbuf;

    if (SafeFStat(_fd, &tempStatbuf) < 0) {
      if (_errorLogging)
	error.logError(DTM_FALSE,
		     "realFileSize(): (1) fstat(%d/%s) failed errno=%d reason %s\nCannot obtain status information on mailbox.\nSuggest you close and reopen the mailbox.\n", 
		       _fd, _real_path, errno, error.errnoMessage());
      error.setError(DTME_ObjectAccessFailed);
      return(0);
    }

    // Obtain guaranteed current stat buf for mailbox object,
    // regarless of whether it is local or remote
    //

    if (SafeGuaranteedStat(_real_path, &statbuf) == -1) {
      if (_errorLogging)
	error.logError(DTM_FALSE,
		     "realFileSize(): (2) SafeGuaranteedStat(%s) failed errno=%d reason %s\nCannot obtain status information on mailbox.\nSuggest you close and reopen the mailbox.\n",
		       _real_path, errno, error.errnoMessage());
      error.setError(DTME_ObjectAccessFailed);
      return(0);
    }

    // At this point, 
    // statbuf -- contains the guaranteed stat struct for the mailbox
    // tempStatbuf -- contains fstat stat struct for original mailbox
    //

    // See if the inode has changed - if so the file has been
    // modified out from under us
    //
    if (statbuf.st_ino != tempStatbuf.st_ino) {
      if (_errorLogging)
	error.logError(DTM_FALSE,
		    "realFileSize(): (3) inode fstat(%d/%s) != inode stat(%s)\nfstat buffer entries: st_ino = %d, st_dev = %d, st_nlink = %d, st_size = %ld\n stat buffer entries: st_ino = %d, st_dev = %d, st_nlink = %d, st_size = %ld\nThe mailbox file appears to have been overwritten with another file.\nSuggest you close and reopen the mailbox.\n",
		      _fd, _real_path, _real_path,
		      statbuf.st_ino, statbuf.st_dev, statbuf.st_nlink, statbuf.st_size,
		      tempStatbuf.st_ino, tempStatbuf.st_dev, tempStatbuf.st_nlink, tempStatbuf.st_size);
      error.setError(DTME_ObjectInvalid);
      return(0);
    }

    return(statbuf.st_size);
}

int
RFCMailBox::mapFile(DtMailEnv & error,
		    const DtMailBoolean already_locked,
		    mode_t create_mode)
{
  if (already_locked == DTM_FALSE) {
    lockFile(error);
    if (error.isSet()) {
      // We won't map what we can't lock.
      //
      return(-1);
    }
  }
  
  // We must map in whole pages. This is done by rounding the
  // file size up to the next larger size.
  //
  
  // Some notes on the singing and dancing that follows are in order.
  // As of 6/21/95 it has been determined that there is a bug in S494-17
  // and S495-25 whereby a mmap() of the mailbox can return sections that
  // have "nulls" where valid data should be found. To guard against this,
  // we check for nulls at the beginning and the end of a new mmap()ed region
  // and if they are found we retry the operation.
  // 
  // If NFS attribute caching is enabled (which is the default), a
  // stat/fstat of a NFS file may not return the correct true size of the
  // mailbox if it has been appended to since the last time it was
  // mmap()ed. To get around this problem, once it is noticed via
  // stat()/fstat() that the mailbox has changed, we must open the mailbox
  // on a separate file descriptor, read() in a byte, and then do a fstat()
  // to determine the true correct size of the mailbox.
  //

  // fstat() currently open mailbox
  //
  struct stat tempStatbuf;

  if (SafeFStat(_fd, &tempStatbuf) < 0) {
    if (_errorLogging)
      error.logError(DTM_FALSE,
		   "mapFile(): (1) fstat(%d/%s) failed errno=%d reason %s\nCannot obtain status information on mailbox.\nSuggest you close and reopen the mailbox.\n",
		   _fd, _real_path, errno, error.errnoMessage());
    if (already_locked == DTM_FALSE) {
      unlockFile(error, _fd);
    }
    error.setError(DTME_ObjectAccessFailed);
    return(-1);
  }

  // Obtain guaranteed current stat buf for mailbox object,
  // regarless of whether it is local or remote
  //
  struct stat statbuf;

  if (SafeGuaranteedStat(_real_path, &statbuf) == -1) {
    if (_errorLogging)
      error.logError(DTM_FALSE,
		   "mapFile(): (2) SafeGuaranteedStat(%s) failed errno=%d\nCannot obtain status information on mailbox.\nSuggest you close and reopen the mailbox.\n",
		   _real_path, errno, error.errnoMessage());
    if (already_locked == DTM_FALSE) {
      unlockFile(error, _fd);
    }
    error.setError(DTME_ObjectAccessFailed);
    return(-1);
  }

  // At this point, 
  // statbuf -- contains the guaranteed stat struct for the mailbox
  // tempStatbuf -- contains fstat stat struct for original mailbox
  //

  // See if the inode has changed - if so the file has been
  // modified out from under is
  //
  if (statbuf.st_ino != tempStatbuf.st_ino) {
    if (_errorLogging)
      error.logError(DTM_FALSE,
                  "mapFile(): (3) inode fstat(%d/%s) != inode stat(%s)\nfstat buffer entries: st_ino = %d, st_dev = %d, st_nlink = %d, st_size = %ld\n stat buffer entries: st_ino = %d, st_dev = %d, st_nlink = %d, st_size = %ld\nThe mailbox file appears to have been overwritten with another file.\nSuggest you close and reopen the mailbox.\n",
                    _fd, _real_path, _real_path,
                    statbuf.st_ino, statbuf.st_dev, statbuf.st_nlink, statbuf.st_size,
                    tempStatbuf.st_ino, tempStatbuf.st_dev, tempStatbuf.st_nlink, tempStatbuf.st_size);
    if (already_locked == DTM_FALSE) {
      unlockFile(error, _fd);
    }
    error.setError(DTME_ObjectInvalid);
    return(-1);
  }
  
  long pagesize = memoryPageSize();
  
  // We will only map any file space we haven't mapped so far.
  // We always map entire pages to make this easier. We must
  // remap the partial pages so we will get the real bits, not
  // the zero fill bits provided by mmap.
  //
  // The arithmetic for partial mappings is a little odd, and
  // is worthy of explanation. The main issue is that we must
  // always start on a page boundary, and we must map page
  // size chunks. So, for any offset, we need to back up to
  // the start of a page in the file. This gives us the following
  // entries in MapRegion:
  //
  // map_region - Address where the mapping starts.
  // map_size - Size of the mapping (will always be a multiple of pagesize).
  // file_region - Address where requested file offset begins within
  //      map_region.
  // file_size - Size of file within this mapping.
  // offset - Where the mapping begins (which is almost always different
  //      than the offset where file_region begins.
  //
  // So, the new offset begins where the last real file size ended.
  // We get this by adding the previous offset, to the file size mapped,
  // and then add any the difference between the mapped region and the
  // real file offset. This gets us an offset back to the old end of
  // file. Now if you are not confused, we need to adjust this new offset
  // back to the closest page boundary and begin the mapping from there!
  //
  MapRegion * map = new MapRegion;
  long offset_from_map;
  
  if (_mappings.length()) {
    MapRegion * prev_map = _mappings[_mappings.length() - 1];
    map->offset = prev_map->offset + prev_map->file_size + 
      (prev_map->file_region - prev_map->map_region);
    offset_from_map = map->offset % pagesize;
    map->offset -= offset_from_map;
  }
  else {
    map->offset = 0;
    offset_from_map = 0;
  }
  
  map->file_size = statbuf.st_size - map->offset - offset_from_map;
  map->map_size = statbuf.st_size - map->offset;
  map->map_size += (pagesize - (map->map_size % pagesize));
  
  int flags = MAP_PRIVATE;
  
#if defined(MMAP_NORESERVE)
  // We are not supposed to be writing to these pages. If
  // we don't specify MAP_NORESERVE however, the system will
  // reserve swap space equal to the file size to deal with
  // potential writes. This is wasteful to say the least.
  //
  flags |= MAP_NORESERVE;
#endif

  // Need to obtain an mmap()ed region and one way or another
  // have the data from the mail file placed into that
  // region. There are two ways of accomplishing this:
  // 
  //   Method 1: mmap() the data directly from the file
  //   Method 2: mmap() /dev/zero and then read from the file to the region
  // 
  // We want to use method #1 if at all possible as it allows the
  // VM system to page the data in as it is accessed.
  // 
  // There is a potential problem with method #1 in that since
  // the region is a "view" into the real file data, if the file
  // itself is reduced in size behind our back by another
  // process, we have the potential for generating SIGBUS memory
  // reference errors if we try and access a byte that is no
  // longer within the file. This is true even is MAP_PRIVATE is used:
  //
  // ** The behavior of PROT_WRITE can be influenced by setting
  // ** MAP_PRIVATE in the flags parameter. MAP_PRIVATE means
  // ** "Changes are private".
  // ** 
  // ** MAP_SHARED and MAP_PRIVATE describe the disposition of write
  // ** references to the memory object.  If MAP_SHARED is
  // ** specified, write references will change the memory object.
  // ** If MAP_PRIVATE is specified, the initial write reference
  // ** will create a private copy of the memory object page and
  // ** redirect the mapping to the copy. Either MAP_SHARED or
  // ** MAP_PRIVATE must be specified, but not both.  The mapping
  // ** type is retained across a fork(2).
  // ** 
  // ** Note that the private copy is not created until the first
  // ** write; until then, other users who have the object mapped
  // ** MAP_SHARED can change the object.
  // 
  // While this is always the case for a process that does not
  // abide by the advisory only locking protocols used to protect
  // against this, to guard against this method #1 is only used
  // if the mailbox is writable and we can obtain a short term
  // lock on the mailbox. This prevents the mailbox from changing
  // size at the moment we map it. Nonetheless, a SIGBUS
  // interrupt handler must be armed to handle the case where the
  // mailbox file is changed out from under us by devious means.
  // 
  // If this condition is not met, or the mmap() call for some
  // reason fails, then fall back to method #2.
  //
  map->map_region = (char *)-1;

  if ( (_use_dot_lock == DTM_TRUE)
#if 0
       && (_long_lock_active == DTM_TRUE)
#endif
       && (_mail_box_writable == DTM_TRUE) ) for (int retryCount = 10; retryCount >= 0; retryCount--) {
    map->map_region = (char *)-1;
    map->map_region = (char *)mmap(0, map->map_size,
                                   PROT_READ, flags,
                                   _fd, map->offset);

    // ERROR/CONSISTENCY CHECKING: if the region was not mapped,
    // or the first or last byte of the region is a null, then
    // the mmap() is considered to have failed: write an entry
    // to the dump file, undo the damage, and bail to the caller
    //
    if ( (map->map_region == (char *)-1)
	 || (map->file_size && (map->map_region[offset_from_map] == '\0'))
	 || (map->file_size && (map->map_region[offset_from_map + map->file_size -1] == '\0') )) {
      if (_errorLogging)
        writeToDumpFile("mmap(0, %ld, 0x%4x, 0x%04x, %d, 0x%08lx(%08ld)) == 0x%08lx(%ld), errno == %d\nstat buffer entries: st_ino = %d, st_dev = %d, st_nlink = %d, st_size = %ld\n",
		      map->map_size, PROT_READ, flags, _fd, map->offset, map->offset, map->map_region, map->map_region, errno,
		      statbuf.st_ino, statbuf.st_dev, statbuf.st_nlink, statbuf.st_size);
      if (map->map_region != (char *)-1) {	// region mapped but contents invalid
        if (_errorLogging) {
	  HexDump((FILE*) NULL, "Region mapped in", (unsigned char *)map->map_region, map->map_size, _errorLogging ? 0 : 100);
          HexDump((FILE*) NULL, "Offset Region mapped in", (unsigned char *)map->map_region+offset_from_map, map->file_size, _errorLogging ? 0 : 100);
	}
        munmap(map->map_region, map->map_size);	// unmap region just mapped in
	if (retryCount) {	// have we exhausted all possible attempts?
	  struct stat tmpstatbuf;
	  sleep(5);		// NO: sleep for a short time
          (void) SafeGuaranteedStat(_real_path, &tmpstatbuf);	// force attributes to be fetched again
	  (void) SafeFStat(_fd, &tmpstatbuf);
	  continue;		// and try map operation again
	}
	if (already_locked == DTM_FALSE) {
	  unlockFile(error, _fd);
	}
	error.setError(map->map_region != (char *)-1 ? DTME_ObjectAccessFailed : DTME_ObjectCreationFailed);
	delete map;
	return(-1);
      }
    }
  assert(retryCount);
  break;
  }
  
  // If a region was mapped, cause OS to use sequential access paging rules
  //
  if (map->map_region != (char *)-1)
    alterPageMappingAdvice(map, MADV_SEQUENTIAL);
    
  if (map->map_region == (char *)-1) {
    // Either the direct mmap failed, or we decided not to do a direct mmap
    // of the new data in the mailbox file. Now must create a mmap()ed
    // region against /dev/zero and then read the new data into that region
    //
#if defined(DO_ANONYMOUS_MAP)
    int fd = -1;
    flags |= MAP_ANONYMOUS;
#else
    int fd = SafeOpen("/dev/zero", O_RDWR, create_mode);
#endif
    
    map->map_region = (char *)mmap(0, map->map_size,
				   PROT_READ | PROT_WRITE, 
				   flags,
				   fd, 0);
    if (map->map_region == (char *)-1) {
      if (_errorLogging)
        error.logError(DTM_TRUE,
		     "mapFile(): (4) mmap(0, %ld, 0x%4x, 0x%04x, %d(/dev/zero), 0) == -1, errno == %d reason %s\nA system error occurred while reading the mailbox.\nSuggest you close and reopen the mailbox.\n",
		     map->map_size, PROT_READ|PROT_WRITE, flags, fd, errno, error.errnoMessage());
      if (already_locked == DTM_FALSE) {
	unlockFile(error, _fd);
      }
      error.setError(DTME_NoMemory);
      delete map;
      return(-1);
    }
    
    if (fd != -1 && (SafeClose(fd) < 0)) {
      // should deal with the error here (on /dev/zero??)
    }

    // Have created a sufficiently large region mapped to /dev/zero
    // Read the "new bytes" into this region
    // Note that there is a suspicion that a window exists where even
    // though the file size appears to be increased by "n", that all
    // of the data may not be written in the file yet, and a read may
    // fall "short" in this case, especially if running over nfs. We
    // must be prepared to handle any condition whereby a read from
    // the file either yields less data than expected, or even though
    // a good return is received, the data may not be there, and thus
    // the check for "nulls" at the beginning and end of the buffer.
    //
    lseek(_fd, map->offset, SEEK_SET);
    size_t bytesToRead = (size_t)(statbuf.st_size - map->offset);
    ssize_t readResults = SafeRead(_fd, map->map_region, bytesToRead);
    if ( (readResults != bytesToRead)
	 || (readResults && (map->map_region[0] == '\0'))
	 || (readResults && (map->map_region[readResults-1] == '\0') )) {
      if (_errorLogging)
        writeToDumpFile("SafeRead(%d(%s), 0x%08lx, %d) == %d, errno == %d\nstat buffer entries: st_ino = %d, st_dev = %d, st_nlink = %d, st_size = %ld\n",
		      _fd, _real_path, map->map_region, bytesToRead,
		      readResults, errno,
		      statbuf.st_ino, statbuf.st_dev, statbuf.st_nlink, statbuf.st_size);
      if (readResults > 0) {
        if (_errorLogging)
          HexDump((FILE*) NULL, "Buffer read in", (unsigned char *)map->map_region, readResults, _errorLogging ? 0 : 100);
      }
      munmap(map->map_region, map->map_size);
      delete map;
      if (already_locked == DTM_FALSE) {
	unlockFile(error, _fd);
      }
      error.setError(readResults >= 0 ? DTME_ObjectAccessFailed : DTME_ObjectCreationFailed);
      return(-1);
    }
    
    mprotect(map->map_region, map->map_size, PROT_READ);
    alterPageMappingAdvice(map, MADV_SEQUENTIAL);
  }
  
  map->file_region = map->map_region + offset_from_map;

  // Ok, we think we have got all of the new data that has been
  // appended to the mailbox - just to make absolutely sure, stat
  // the file again and make sure that the file size has remained
  // consistent throughout this operation. If it has changed, it
  // means some process ignored our lock on the file and appended
  // data anyway. In this case, throw away our current effort and
  // recursively call this function to try the attempt again.
  //

  if (SafeGuaranteedStat(_real_path, &tempStatbuf) < 0) {
    if (_errorLogging)
      error.logError(DTM_FALSE,
		   "mapFile(): (5) SafeGuaranteedStat(%s) failed errno=%d reason %s\nCannot obtain status information on mailbox.\nSuggest you close and reopen the mailbox.\n",
		   _real_path, errno, error.errnoMessage());
    munmap(map->map_region, map->map_size);
    delete map;
    if (already_locked == DTM_FALSE) {
      unlockFile(error, _fd);
    }
    error.setError(DTME_ObjectAccessFailed);
    return(-1);
  }

  if (tempStatbuf.st_size != statbuf.st_size) {
    if (_errorLogging)
      error.logError(DTM_FALSE,
		   "mapFile(): (6) fstat(%d/%s) size changed %d/%d\nThe mailbox file appears to have been overwritten with another file.\nSuggest you close and reopen the mailbox.\n",
		   _fd, _real_path, statbuf.st_size, tempStatbuf.st_size);
    munmap(map->map_region, map->map_size);
    delete map;
    int mapResults = mapFile(error, DTM_TRUE);
    if (error.isSet()) {
      if (already_locked == DTM_FALSE) {
	unlockFile(error, _fd);
      }
      return(-1);
    }
    return(mapResults);
  }

  if (already_locked == DTM_FALSE) {
    unlockFile(error, _fd);
  }
  
  // We need to set _file_size here, because if we get the file size
  // when the mailbox is not locked, it is possible that we could be
  // stating the file while it is being written to.  Since we have the
  // lock in this routine, we know the file size isn't changing, and it
  // is consistent with the amount being mapped in.
  //
  _file_size = statbuf.st_size;
  return(_mappings.append(map));
}

int
RFCMailBox::nextNotDel(const int cur)
{
    for (int del = cur; del < _msg_list.length(); del++) {
	MessageCache * mc = _msg_list[del];
	if (mc->delete_pending == DTM_FALSE) {
	    break;
	}
    }

    return(del);
}

int
RFCMailBox::prevNotDel(const int cur)
{
    for (int del = cur; del >= 0; del--) {
	MessageCache * mc = _msg_list[del];
	if (mc->delete_pending == DTM_FALSE) {
	    break;
	}
    }

    return(del);
}

int
RFCMailBox::lookupByMsg(RFCMessage * msg)
{
    for (int slot = 0; slot < _msg_list.length(); slot++) {
	MessageCache * mc = _msg_list[slot];
	if (mc->message == msg) {
	    return(slot);
	}
    }

    return(-1);
}

void *
RFCMailBox::ThreadParseEntry(void * client_data)
{
    RFCMailBox * self = (RFCMailBox *)client_data;

    DtMailEnv error;
    MutexLock lock_map(self->_map_lock);

    self->parseFile(error, 0);

    lock_map.unlock();

    ThreadExit(0);
    return(NULL);
}

void
RFCMailBox::parseFile(DtMailEnv & error, int map_slot)
{

    error.clear();

    // We are not at the eof.
    //
    _at_eof.false();

    const char * begin = _mappings[map_slot]->file_region;
    const char * end = begin + _mappings[map_slot]->file_size - 1;

    if (end <= begin) {
	// Empty file. DO NOTHING OR IT WILL CRASH!
	//
	_at_eof.true();
	return;
    }

    // Parsing will always be a sequential access to the pages.
    // We will give the kernel a clue what we are up to and perhaps
    // help our parsing time in the process.
    //
    unsigned long pagelimit = _mappings[map_slot]->map_size;

#if !defined(USL) /* no madvise so we lose this optimization */
    madvise((char *)_mappings[map_slot]->map_region, pagelimit, MADV_SEQUENTIAL);
#endif
    
    // We should always begin with a "From " if this is an RFC file,
    // with a valid offset. If we have something else, then look
    // forward until we find one.
    //
    const char * parse_loc = begin;
    if (strncmp(parse_loc, "From ", 5)) {
      if (*parse_loc == ' ' || *parse_loc == '\n' || *parse_loc == '\t') {
      	// We allow any number of white spaces before "From"
	// But "From" still has to be the first word in the line
	while ( (*parse_loc == ' ' || *parse_loc == '\n'
		 || *parse_loc == '\t') && parse_loc <= end) {
	  parse_loc++;
	}

	if (parse_loc >= end) {
	    error.setError(DTME_NotMailBox);
	    _at_eof.true(); // We are, but so what.
	    return;
	}

	parse_loc--;

	if (strncmp(parse_loc, "\nFrom ", 6)) {
	  error.setError(DTME_NotMailBox);
	  return;
	}
      } else {
	// This file does not start with either From or white space
	error.setError(DTME_NotMailBox);
	return;
      }
    }

    if (*parse_loc == '\n') {
	parse_loc += 1;
    }

    // We are sitting at the start of a message. We will build message
    // objects for each message in the list.
    //
    do {
	MessageCache * cache = new MessageCache;
	cache->delete_pending = DTM_FALSE;

	cache->message = new RFCMessage(error, this, &parse_loc,
					end);
	if (error.isNotSet()) {

#ifdef MESSAGE_PARTIAL
	// message/partial processing is currently not working, so we are 
	// taking out message/parital processing for now until we figure 
	// out how to get it to work again.  Only the following block of 
	// code needs to be taken out in order to disable message/partial.
	// Also, when it was turned on, it was trying to combine partial 
	// messages that were non-MIME (no MIME-VERSION header).  This 
	// caused even more problems.  We should only check for partial 
	// messages if it is MIME.

	  if (_isPartial(error, cache->message)) {

	    if (error.isNotSet()) {
	      cache->message->setFlag(error, DtMailMessagePartial);

	      if (error.isNotSet()) {
		cache->message = _assemblePartial(error, cache->message);

		if (error.isNotSet()) {
		  _msg_list.append(cache);
		  continue;
		}
	      }
	    }
	  }
#endif // MESSAGE_PARTIAL
	  _msg_list.append(cache);
	}
	else {
	    error.clear();
	}
    } while (parse_loc <= end);

    _at_eof.true();
    error.clear();
}

#ifdef DEAD_WOOD
void *
RFCMailBox::ThreadNewMailEntry(void * client_data)
{
    NewMailData * info = (NewMailData *)client_data;

    // We need to wait for the object to become valid. We have
    // nothing to do until the mail box is open.
    info->object_valid->waitTrue();

    // Get the various configuration parameters from .mailrc. The
    // RFC_PING_INTERVAL controls how often we look for new mail.
    // RFC_CHECK is the number of pings between checks and
    // RFC_EXPUNGE is the number of pings between expunges.
    //
    DtMailEnv error;
    DtMail::MailRc * mailrc = info->self->session()->mailRc(error);

    // Defaults.
    time_t ping = 15;
    time_t check_per_ping = 120;
    time_t expunge_per_ping = 240;

    const char * value = NULL;
    mailrc->getValue(error, "RFC_PING_INTERVAL", &value);
    if (error.isNotSet()) {
	ping = strtol(value, NULL, 10);
    }
    else {
	error.clear();
    }
    if (value) {
	    free((void *)value);
	    value = NULL;
    }

    mailrc->getValue(error, "RFC_CHECK", &value);
    if (error.isNotSet()) {
	check_per_ping = strtol(value, NULL, 10);
    }
    else {
	error.clear();
    }
    if (value) {
	    free((void *)value);
	    value = NULL;
    }

    mailrc->getValue(error, "RFC_EXPUNGE", &value);
    if (error.isNotSet()) {
	expunge_per_ping = strtol(value, NULL, 10);
    }
    else {
	error.clear();
    }
    if (value) {
	    free((void *)value);
	    value = NULL;
    }

    int pinged = 0;

    unsigned int unslept = 0;

    // Wait until the mail file is parsed.
    //
    info->self->_at_eof.waitTrue();

    // We loop until the object tries to close, then we exit.
    //
    while(info->object_valid->state()) {

	// The following sequence is a little wierd, but here is why.
	// We need to sleep for the ping interval. We can be awaken
	// early however if the thread catches a signal. The main
	// thread will send a SIGTERM from the mail box destructor
	// to wake us up. If the object state is no longer valid, then
	// this is the cause, so we need to check it first and exit
	// if this is the case.
	//
	// We can also be awaken by other signals that we don't care
	// about. In that case we need to go back through the loop
	// again and sleep any unslept time. We can't be sure however
	// that this additional sleep won't be awaken before it is
	// done so we have to keep looping and checking the object
	// state.
	//
	if (unslept) {
	    unslept = ThreadSleep(unslept);
	    continue; // We got rudely awaken!
	}

	unslept = ThreadSleep(ping);
	if (!info->object_valid->state()) {
	    break;
	}

	if (unslept) {
	    continue;
	}

	// We made here, we slept and are well rested, and
	// we should have a valid object. Run the events.
	//

	pinged += 1;

	if (check_per_ping && (pinged % check_per_ping) == 0) {
//	    error.clear();
//	    info->self->CheckPointEvent(error);
	    info->self->CheckPointEvent();
// 	    if (error.isSet()) {
// 	      // MAILBOX COULD NOT BE CHECKPOINTED!!! MUST DO SOMETHING!!!
// 	    }
	    continue;
	}

	if (expunge_per_ping && (pinged % expunge_per_ping) == 0) {
	    info->self->ExpungeEvent();
	    continue;
	}

	info->self->NewMailEvent();
    }

    // We are responsible for cleaning up the condition variable.
    //
    delete info->object_valid;
    delete info;

    ThreadExit(0);

    return(NULL);
}
#endif


DtMailBoolean
RFCMailBox::PollEntry(void * client_data)
{
    RFCMailBox * self = (RFCMailBox *)client_data;

    // Get the various configuration parameters from .mailrc. The
    // RFC_PING_INTERVAL controls how often we look for new mail.
    // RFC_CHECK is the number of pings between checks and
    // RFC_EXPUNGE is the number of pings between expunges.
    //
    DtMailEnv error;
    error.clear();
    
    DtMail::MailRc * mailrc = self->session()->mailRc(error);
    error.clear();		// IGNORING ERRORS FROM MAILRC CALL!!

    // Defaults.
    time_t ping = 60;		// check for new mail every 60 seconds
    time_t save_interval = (10 * 60); // autosave every 10 minutes
    long minimumIdleTime = 60;	// must have 60 seconds idle time to poll

    // Retrieve current setting for how often we are supposed to
    // look for new mail (retrieveinterval)
    //
    const char * value;

    value = NULL;
    mailrc->getValue(error, "retrieveinterval", &value);
    if (error.isNotSet() && value != NULL && *value != '\0') {
	ping = strtol(value, NULL, 10);
	if (ping < 15)
	    ping = 15;	// dont ping any greater than every 15 seconds
    }
    if (value) {
	    free((void *)value);
	    value = NULL;
    }
    error.clear();

    // Retrieve current setting for how often we are supposed to
    // perform an auto save of the mailbox
    //
    mailrc->getValue(error, "dontautosave", &value);
    if (error.isSet()) {
	error.clear();
	if (value) {
		free((void *)value);
		value = NULL;
	}
    	mailrc->getValue(error, "saveinterval", &value);
    	if (error.isNotSet() && value != NULL && *value != '\0') {
	    save_interval = strtol(value, NULL, 10) * 60;
	}
	error.clear();
	if (save_interval < 60)
	  save_interval = 60;	// The minimum time is every minute
    }
    else save_interval = -1;
    if (value) {
	    free((void *)value);
	    value = NULL;
    }

    // Get the current time in seconds, and compute how long it has
    // been since the last interactive input (keystroke, buttonpress)
    // was done by the user
    //
    time_t now = time(NULL);
    time_t lastInteractive = self->session()->lastInteractiveEventTime();
    time_t interactiveIdleTime = (now - lastInteractive);

    // If there has not been at least <<inactivityinterval> seconds of
    // interactive inactivity, skip processing until later so as not to
    // place the user in the horrible position of being "locked out" for
    // the duration of an auto save/and/or/new mail incorporation
    //

    mailrc->getValue(error, "inactivityinterval", &value);
    if (error.isNotSet() && value != NULL && *value != '\0') {
      minimumIdleTime = strtol(value, NULL, 10);
      if (minimumIdleTime < 15)
	  minimumIdleTime = 15;		// at least 15 seconds must go by
      else if (minimumIdleTime > 600)
	  minimumIdleTime = 600;	// but not more than 10 minutes
    }
    error.clear();
    if (value) {
	    free((void *)value);
	    value = NULL;
    }
    
    if (interactiveIdleTime < minimumIdleTime)
      return(DTM_FALSE);

    // See if time's up for doing a new mail incorporate
    //
    if ((now - self->_last_poll) > ping) {
	self->_last_poll = now;

	self->NewMailEvent(error);
    }

    // See if time's up for doing an auto-save.
    // If time's up, check to see if the flag is clear for doing one.
    // Flag is not clear if say, the user is in the middle of composing
    // a message...

    if (save_interval >= 0 &&
	((now - self->_last_check) > save_interval) &&
	(self->session()->getAutoSaveFlag())) {

	self->_last_check = now;

//	error.clear();
//	self->CheckPointEvent(error);
	self->CheckPointEvent();
// 	if (error.isSet()) {
// 	  // MAILBOX COULD NOT BE CHECKPOINTED!!! MUST DO SOMETHING!!!
// 	}
    }
    return(DTM_TRUE);
}

void
RFCMailBox::NewMailEvent(
    DtMailEnv & error,
    const DtMailBoolean already_locked
)
{
    if (!_object_valid->state()) {
	return;
    }

    // We need to compare the current file size to the last size we
    // knew about. If it has changed, we need to do something. There
    // are a few possibilities here that we must deal with:
    //
    // 1. The file size has not changed. Boring. Simply return.
    //
    // 2. The file size is larger, and the first byte begins with
    //    either "From ", "\nFrom ", or "\r\nFrom ". In this case,
    //    we need to parse the rest of the file, and notify the client
    //    that new mail has arrived.
    //
    // 3. The file size is either smaller, or it is larger and the first
    //    few bytes dont match #2. This is the worse possible case. This
    //    means that somebody has modified the object from beneath us.
    //    We will turn on our "lost state" mode and refuse to do any
    //	  further processing. This thread exits immediately.
    //
    // Even though we get the file size here, we can't use it to set the
    // new _file_size, because the file isn't currently locked, so another
    // process could be writing to it at this time.  We rely on mapFile()
    // to get and set _file_size while it has the file locked.

    off_t size = realFileSize(error);
    if (error.isSet()) {
	return;
    }

    MutexLock lock_object(_obj_mutex);

    if (size == _file_size) {
	return;
    }

    if (size > _file_size) {
        _session->setBusyState(error, DtMailBusyState_NewMail);
	incorporate(error, already_locked);
        _session->setBusyState(error, DtMailBusyState_NotBusy);
    }
    else {
	longUnlock(error);
	_mail_box_writable = DTM_FALSE;
	*_object_valid = -1;
	DtMailEventPacket event;
	event.key = _key;
	event.target = DTM_TARGET_MAILBOX;
	event.target_object = this;
	event.operation = (void *)DTMC_BADSTATE;
	event.argument = NULL;
	event.event_time = time(NULL);
	_session->writeEventData(error, &event, sizeof(event));

	// Debug
	error.setError(DTME_ObjectInvalid);
        error.logError(DTM_TRUE, "RFCMailBox::NewMailEvent(): mailbox file size shrunk: %ld < %ld and already_locked is %s\nThe mailbox file appears to have been overwritten with another file.\nSuggest you close and reopen the mailbox.\n",
		(unsigned long)size, (unsigned long)_file_size, already_locked ? "TRUE" : "FALSE");
    }

    // We need to bail right now if we have lost a valid mapping.
    //
    if (_object_valid->state() < 0) {
	lock_object.unlock();
	ThreadExit(1);
    }

    return;
}

void
//RFCMailBox::CheckPointEvent(DtMailEnv & error)
RFCMailBox::CheckPointEvent()
{
  if (!_object_valid->state() || _dirty == 0) {
    return;
  }
  
  // Write the mail box out.
  //
  DtMailEnv error;
  error.clear();

  // This is INCORRECT usage of the DtMailEnv class.  It is against
  // standard policy to not clear the error token before passing
  // it to a function.  We are basically breaking the error handling
  // model in order to percolate an error in the BE up to the FE.

  // setBusyState() does not modify the error token, it simply 
  // uses it to report errors back to the user.
  _session->setBusyState(error, DtMailBusyState_AutoSave);

  writeMailBox(error);
  if (error.isSet()) {
    // MAILBOX COULD NOT BE CHECKPOINTED!!! setBusyState must handle error.
    error.logError(DTM_TRUE, "RFCMailBox::CheckPointEvent(): Failed to write mailbox: %s", (const char *)error);
  }
  _session->setBusyState(error, DtMailBusyState_NotBusy);
}

void
RFCMailBox::ExpungeEvent(DtMailBoolean closing)
{
    if (!_object_valid->state() && closing == DTM_FALSE) {
	return;
    }

    // We need to get the expiration time for a message.
    //
    DtMailEnv error;

    DtMail::MailRc * mailrc = _session->mailRc(error);

    long expire_days;
    const char * expire_time = NULL;
    mailrc->getValue(error, "DTMAIL_EXPIRE_TIME", &expire_time);
    if (error.isNotSet()) {
	expire_days = strtol(expire_time, NULL, 10);
    }
    else {
	error.clear();
	expire_days = 0;
    }
    if (expire_time) {
	    free((void *)expire_time);
	    expire_time = NULL;
    }
    
    if (expire_days == 0 && closing == DTM_FALSE) {
	// Only scan on close. No timed delete in effect.
	//
	return;
    }

    if (expire_days > 0 && closing == DTM_TRUE) {
	// Timed delete is in effect so we don't destroy messages on
	// close.
	return;
    }

    time_t expire_secs = expire_days * (24 * 3600);
    time_t now = time(NULL);

    for (int msg = 0; msg < _msg_list.length(); msg++) {
	MessageCache * mc = _msg_list[msg];
	if (mc->delete_pending == DTM_TRUE) {
	    // Can only delete it so many times!
	    //
	    continue;
	}

	DtMail::Envelope * env = mc->message->getEnvelope(error);

	DtMailValueSeq value;
	env->getHeader(error, RFCDeleteHeader, DTM_FALSE, value);
	if (!error.isSet()) {
	    time_t deleted;
	    deleted = strtol(*(value[0]), NULL, 16);
	    if ((deleted + expire_secs) < now) {
		mc->delete_pending = DTM_TRUE;
		_dirty += 1;

		// We need to tell our client this message is really
		// gone. Of course, if they are closing, why bother
		// them.
		//
		if (closing == DTM_FALSE) {
		    DtMailEventPacket event;
		    event.key = _key;
		    event.target = DTM_TARGET_MAILBOX;
		    event.target_object = this;
		    event.operation = (void *)DTMC_DELETEMSG;
		    event.argument = mc;
		    event.event_time = time(NULL);
		    _session->writeEventData(error, &event, sizeof(event));
		}
	    }
	}
	else {
	    error.clear();
	}
    }
}

// Function: RFCMailBox::createTemporaryMailboxFile
// Description:
//  Given the name for a temporary mailbox file, create a proper
//  temporary mailbox file and return a file descriptor opened on
//  the newly created file
// Method:
//   . obtain information on current mailbox file
//   . create the new temporary file
//   . set the permissions, owner and group of the newly created file
//     to match those of the current mailbox
// Arguments:
//  DtMailEnv & error - standard error structure used by caller
//  tmp_name	-- -> name for temporary file
// Outputs:
//  error.isset() will indicate if there were any errors encountered, in which
//  case the temporary file has not been created.
// Returns:
//  int file descriptor opened on the newly created temporary mailbox file
//  if no error encountered.
// 
int
RFCMailBox::createTemporaryMailboxFile(DtMailEnv & error, char *tmp_name)
{
  int fd;
  int return_status;
  struct stat info;
  if (SafeFStat(_fd, &info) < 0) {
    int errno2 = errno;
    if (_errorLogging)
      error.logError(DTM_FALSE,
		   "createTemporaryMailboxFile(): fstat(%d) failed errno=%d\n",
		   _fd, errno);
    error.vSetError(DTME_CannotObtainInformationOnOpenMailboxFile,
		    DTM_FALSE, NULL, _real_path, error.errnoMessage(errno2));
    return(-1);
  }

  PRIV2(fd,SafeOpen(tmp_name, O_RDWR | O_CREAT | O_TRUNC, info.st_mode));
  if (fd < 0) {
    int errno2 = errno;
    switch (errno2) {
    case EACCES:
      error.setError(DTME_CannotCreateTemporaryMailboxFile_NoPermission);
      break;
      
    case EISDIR:
      error.setError(DTME_CannotCreateTemporaryMailboxFile_IsDirectory);
      break;
      
    case ENOENT:
      error.setError(DTME_CannotCreateTemporaryMailboxFile_NoSuchFile);
      break;

    case ENOLINK:
      error.setError(DTME_CannotCreateTemporaryMailboxFile_RemoteAccessLost);
      break;
      
    default:
      error.vSetError(DTME_CannotCreateTemporaryMailboxFile,
		      DTM_FALSE, NULL, tmp_name, error.errnoMessage(errno2));
    }
    return(-1);
  }

  PRIV2(return_status,SafeFChmod(fd, info.st_mode & 07777));
  if (return_status == -1) {
    int errno2 = errno;
    (void) SafeClose(fd);
    PRIV2(return_status,SafeUnlink(tmp_name));
    error.vSetError(DTME_CannotSetPermissionsOfTemporaryMailboxFile,
		    DTM_FALSE, NULL, tmp_name, info.st_mode & 07777,
		    error.errnoMessage(errno2));
    return(-1);
  }

  PRIV2(return_status,SafeFchown(fd, info.st_uid, -1));
#if 0
  // bug 1216914 - dtmail should be able to auto-save a mailbox which is not opened [Read Only]
  // if the user has write access to the original mailbox, then the user should be able to
  // overwrite the mailbox even if the owner gets changes. A mailbox is only opened read-write
  // if the "access()" system call indicates the user has write permissions.
  //
  if (return_status == -1) {
    int errno2 = errno;
    (void) SafeClose(fd);
    PRIV2(return_status,SafeUnlink(tmp_name));
    error.vSetError(DTME_CannotSetOwnerOfTemporaryMailboxFile,
		    DTM_FALSE, NULL, tmp_name, info.st_uid,
		    error.errnoMessage(errno2));
    return(-1);
  }
#endif

  PRIV2(return_status,SafeFchown(fd, -1, info.st_gid));
  
  return(fd);
}

// Function: RFCMailBox::writeMailBox - create new copy of complete mailbox
// Description:
//   writeMailBox causes a complete copy of the current mailbox to be written
//   out to permanent storage. This is done by creating a temporary file, in
//   the same location the permanent file resides, and writing the new mailbox
//   contents to it. If this is successful, the temporary file is then renamed
//   over the old permanent file, taking its place. In this way, if there is
//   an error creating the new file, the old file still remains intact.
// Method:
//   . make sure the current mailbox is writeable
//   . lock current mailbox
//   . check for new mail and incorporate if necessary
//   . cause the OS to begin mapping as much of the current mailbox into memory
//	as it can so it is available as soon as possible to be written
//   . create temporary file to hold new mailbox contents
//   . create message list fixing the location of all non-deleted messages
//	(dirty messages will be allocated temporary storage and created)
//   . build a vectored write array based upon the message list
//   . cause the new mailbox contents to be written via one call to SafeWritev
//   . delete all "deleted" messages from the message list
//   . map the new mailbox file into memory
//   . revise all message pointers based upon new mailbox contents
//   . remove the mappings and unmap all previous regions used
//   . add the one single new mailbox region to the mappings list
//   . transfer any lock on the old mailbox file to the new mailbox file
//   . rename the new mailbox file to the correct permanent path name
//   . set the modification and access times of the mailbox file properly
// Arguments:
//  DtMailEnv & error - standard error structure used by caller
// Outputs:
//  error.isset() will indicate if there were any errors encountered, in which
//  case the current contents of the mailbox have not been written.
//
//  At some point in the call chain, someone must check to see if this is
//  a FATAL ERROR (e.g. error.isFatal) and if so, display the returned
//  message and then immediately exit, as this means the mailbox could not
//  be written and the internal mailbox state is hopelessly munged..
// Returns:
//  <none>
// 
void
RFCMailBox::writeMailBox(DtMailEnv & error)
{
  DtMailEnv error2; 	// When we need to preserve error during error cleanup
  int return_status;	// for handling priv/unpriv operations

  MutexLock lock_map(_map_lock);

  error.clear();
  if (_mail_box_writable == DTM_FALSE) {
    return;
  }

  // Need to lock the file while we do this.
  //
  lockFile(error);
  if (error.isSet()) {
    return;
  }

  // Need to deal with any potentially new mail that
  // has arrived and we don't know about.
  //
  checkForMail(error, DTM_TRUE);
  if (error.isSet()) {
    unlockFile(error2, _fd);
    return;
  }

  // Create the new temporary file to hold the mailbox contents
  //
  char tmp_name[MAXPATHLEN+1];
  memset(tmp_name, 0, sizeof(tmp_name));
  sprintf(tmp_name, "%s.tmp.%08x", _real_path, time(NULL));
  assert(strlen(tmp_name)<sizeof(tmp_name));

  int fd = createTemporaryMailboxFile(error, tmp_name);
  if (error.isSet()) {
    unlockFile(error2, _fd);
    return;
  }
  assert(fd != -1);
  
  // Got the new mailbox file all set up to be our friend.
  // zip through current message structure fixing the location of all
  // non-deleted messages in preparation of writing them all out to
  // the temporary file just opened. The act of "fixing" the location of
  // a message will cause dirty messages to be constructed in temporary
  // allocated areas which must later be deallocated. This deallocation
  // is done either by calling adjustMessageLocation() to make the location
  // of the message permanent, or by calling unfixMessageLocation) to
  // abandon the new message (e.g. if new mailbox cannot be created).
  //
  int deletesPending = 0;	// count # of delete_pending messages scanned
  struct tempMsgList {
    void *tmlMc;		// -> mc of this message
    long tmlRealOffset;		// byte offset from 0 where message is written
    long tmlBodyOffset;		// byte offset from 0 of body parts of message
    char *tmlHeaderStart;	// -> start of headers for this message
    long tmlHeaderLen;		// length of headers
    char *tmlBodyStart;		// -> start of bodies for this message
    long tmlBodyLen;		// length of bodies
    int tmlTemporary;		// ==0:permanent, !=0:temporary(must move)
  };
  
  long tmlTotalSize = _msg_list.length()*4;
  tempMsgList *const tmlFirst = (tempMsgList *)malloc(sizeof(tempMsgList)*tmlTotalSize);
  tempMsgList *tmlLast = tmlFirst;

  for (int msg = 0; msg < _msg_list.length(); msg++) {
    MessageCache *mc = _msg_list[msg];
    if (mc->delete_pending == DTM_TRUE) {	// ignore deleted messages
      deletesPending++;				// but remember if any seen
      continue;
    }
    tmlLast->tmlMc = (void *)mc;
    mc->message->fixMessageLocation(&tmlLast->tmlHeaderStart, tmlLast->tmlHeaderLen,
				    &tmlLast->tmlBodyStart, tmlLast->tmlBodyLen,
				    tmlLast->tmlTemporary, tmlLast->tmlBodyOffset);
#if 0
    fprintf(stdout, "msg %03d @ %08lx %08lx/%06d %08lx/%06d %04d %s\n",
	   msg, mc, tmlLast->tmlHeaderStart, tmlLast->tmlHeaderLen,
	   tmlLast->tmlBodyStart, tmlLast->tmlBodyLen,
	   tmlLast->tmlBodyOffset, tmlLast->tmlTemporary ? "T" : "P");
    HexDump(stdout, "header", (unsigned char *)tmlLast->tmlHeaderStart, tmlLast->tmlHeaderLen, 10);
    HexDump(stdout, "body", (unsigned char *)tmlLast->tmlBodyStart, tmlLast->tmlBodyLen, 10);
    fflush(stdout);
#endif
    assert(tmlLast->tmlHeaderStart);
    assert(tmlLast->tmlHeaderLen);
    tmlLast->tmlRealOffset = -1;
    tmlLast++;
  }
  assert((tmlLast-tmlFirst) < tmlTotalSize);

  // we can now allocate the vectored write array and fill it according
  // to the data stored in the message list we just created
  //
  long iovSize = ((tmlLast-tmlFirst)+2)*3;
  iovec *const iovFirst = (iovec *)malloc(sizeof(iovec)*iovSize);
  iovec *iovLast = iovFirst;
  iovec *iovPrev = (iovec *)0;
  long iovCurrentOffset = 0;

  for (tempMsgList *tmlNdx = tmlFirst; tmlNdx < tmlLast; tmlNdx++) {
    // if this message happens to start on the first byte following the
    // last byte of the previous message, combine into a single vector,
    // else add this as another vector in the vector list
    //
    tmlNdx->tmlRealOffset = iovCurrentOffset;
    if ( (iovPrev != (iovec *)0) && (!tmlNdx->tmlTemporary)
      && ((iovPrev->iov_base+iovPrev->iov_len) == tmlNdx->tmlHeaderStart)) {
      iovPrev->iov_len += (int)(tmlNdx->tmlHeaderLen+tmlNdx->tmlBodyLen);
      iovCurrentOffset += (int)(tmlNdx->tmlHeaderLen+tmlNdx->tmlBodyLen);
    }
    else if (!tmlNdx->tmlTemporary) {
      iovLast->iov_base = (caddr_t)tmlNdx->tmlHeaderStart;
      iovLast->iov_len = (int)(tmlNdx->tmlHeaderLen+tmlNdx->tmlBodyLen);
      iovCurrentOffset += iovLast->iov_len;
      iovPrev = iovLast++;
    }
    else {
      // Message is temporary - headers and bodies are in different areas,
      // and the headers are in a "temporary" area
      //
      iovLast->iov_base = (caddr_t)tmlNdx->tmlHeaderStart;
      iovLast->iov_len = (int)tmlNdx->tmlHeaderLen;
      iovCurrentOffset += iovLast->iov_len;
      iovPrev = iovLast++;
      
      // Write out bodies only if the length is non-zero, otherwise,
      // optimize out the inclusion of a zero length write from the vector
      //
      if (tmlNdx->tmlBodyLen > 0) {
	iovLast->iov_base = (caddr_t)tmlNdx->tmlBodyStart;
	iovLast->iov_len = (int)tmlNdx->tmlBodyLen;
	iovCurrentOffset += iovLast->iov_len;
	iovPrev = iovLast++;
      }
    }
    // The last two bytes of the message must be \n\n
    // If not then this must be forced
    // Obtain pointer tp to the last byte of the current message
    // Given this pointer we now have:
    //   tp[-1] -- next to last byte in current message
    //   tp[ 0] -- last byte in current message
    //   tp[+1] -- first byte of next message (if any)
    // There must always be two \n characters between each message. If not,
    // we must insert sufficient \n characters into the message stream to
    // accomplish this. We want to avoid plopping these in, however, as each
    // one will add an extra 1- or 2-byte vector into the vectored write array,
    // which will affect the throughput of the overall write.
    // Irregardless of whether the current message is temporary, we check to see if the next
    // byte or two (as necessary) is in a mapped region; if so, we can then
    // peek ahead to see if there are the additional \ns we need.
    // If all this fails, we punt and put in a small 1- or 2-byte write
    // into the vector.
    //
    char *tp = iovPrev->iov_base + (iovPrev->iov_len - 1);	// -> last byte

    if (*tp == '\n') {
      if ( ((addressIsMapped(tp+1) == DTM_TRUE) )
	   && (*(tp+1) == '\n') ) {
	iovPrev->iov_len++;
	iovCurrentOffset++;
	continue;
      }
      else {
	iovLast->iov_base = (caddr_t)"\n";		// add \n
	iovLast->iov_len = 1;
	iovCurrentOffset++;
	iovPrev = iovLast++;
      }
    }
    else {
      if ( (( (addressIsMapped(tp+1) == DTM_TRUE)
				      && (addressIsMapped(tp+2) == DTM_TRUE)))
	   && (*(tp+1) == '\n') && (*(tp+2) == '\n')) {
	iovPrev->iov_len+= 2;
	iovCurrentOffset+= 2;
	continue;
      }
      else {
	iovLast->iov_base = (caddr_t)"\n\n";		// add \n\n
	iovLast->iov_len = 2;
	iovCurrentOffset += 2;
	iovPrev = iovLast++;
      }
    }
  }

  assert((iovLast-iovFirst) < iovSize);

  // All of the messages are properly accounted for in the write vector;
  // cause the damage to be done by calling SafeWritev. After it returns,
  // Make absolutely sure that all of the mailbox data has made it to
  // the final destination, especially if the mailbox is not local -
  // this way if we run out of disk space or some other such problem,
  // it is caught here and now.
  //
  unsigned long bytesWritten = SafeWritev(fd, iovFirst, iovLast-iovFirst);
  if (bytesWritten == (unsigned long)-1 || fsync(fd) == -1) {
    int errno2 = errno;
    for (tempMsgList *tml = tmlFirst; tml < tmlLast; tml++) {
      MessageCache *mc = (MessageCache *)tml->tmlMc;
      assert(mc);
      mc->message->unfixMessageLocation(tml->tmlHeaderStart, tml->tmlTemporary);
    }
    (void) SafeClose(fd);
    unlockFile(error2, _fd);
    PRIV2(return_status,SafeUnlink(tmp_name));
    free (tmlFirst);
    free (iovFirst);
    switch (errno2) {
    case EFBIG:
      error.setError(DTME_CannotWriteToTemporaryMailboxFile_ProcessLimitsExceeded);
      break;

    case ENOLINK:
      error.setError(DTME_CannotWriteToTemporaryMailboxFile_RemoteAccessLost);
      break;

    case ENOSPC:
      error.setError(DTME_CannotWriteToTemporaryMailboxFile_NoFreeSpaceLeft);
      break;

    default:
      error.vSetError(DTME_CannotWriteToTemporaryMailboxFile,
		      DTM_FALSE, NULL, tmp_name, error.errnoMessage(errno2));
    }
    return;
  }

  // The current contents of the mailbox have successfully been written
  // to the temporary file. Cause the new mailbox file to be mapped
  // into memory.
  //
  MapRegion * map = mapNewRegion(error, fd, bytesWritten);
  if (error.isSet()) {
    for (tempMsgList *tml = tmlFirst; tml < tmlLast; tml++) {
      MessageCache *mc = (MessageCache *)tml->tmlMc;
      assert(mc);
      mc->message->unfixMessageLocation(tml->tmlHeaderStart, tml->tmlTemporary);
    }
    (void) SafeClose(fd);
    unlockFile(error2, _fd);
    PRIV2(return_status,SafeUnlink(tmp_name));
    free (tmlFirst);
    free (iovFirst);
    return;
  }

  // POINT OF NO RETURN -- NEW MAILBOX MUST BE SUCCESSFULLY LINKED UP WITH
  // BECAUSE THE MACHINATIONS AND POINTER MUNGING DONE BELOW CANNOT BE UNDONE
  //

  // Flush all deleted messages (if any were previously detected)
  //
  if (deletesPending)
    for (msg = 0; msg < _msg_list.length(); msg++) {
      MessageCache * mc = _msg_list[msg];
      if (mc->delete_pending == DTM_TRUE) {
	delete mc->message;		// remove message storage
	delete mc;			// remove message cache storage
	_msg_list.remove(msg);		// remove message from message list
	msg -= 1; 			// next message is where we are at now
      }
    }

  // spin through all "written messages" and fixup their pointers so they
  // point into the new region
  //

  // For this occasion advise the OS that we will be doing sequential access
  //
  alterPageMappingAdvice(map, MADV_SEQUENTIAL);

  for (tempMsgList *tml = tmlFirst; tml < tmlLast; tml++) {
    MessageCache *mc = (MessageCache *)tml->tmlMc;
    assert(mc);
    assert(mc->delete_pending == DTM_FALSE);
    mc->message->adjustMessageLocation(tml->tmlHeaderStart, map->file_region+tml->tmlRealOffset, tml->tmlHeaderLen+tml->tmlBodyLen, tml->tmlTemporary, tml->tmlBodyOffset);
  }

  // Loop through the current mappings, and unmap each.
  // Then make the new single large map the only mapping.
  //
  while(_mappings.length()) {
      MapRegion * c_map = _mappings[0];
      munmap(c_map->map_region, c_map->map_size);
      delete c_map;
      _mappings.remove(0);
  }
  _mappings.append(map);

  // fix for cmvc defect 7912 - Queued mail lost upon closing dtmail
  // If we are using the .lock protocol, we are locking on a file name
  // basis, and therefore can rename the new mailbox over the old mailbox
  // without a worry about locks; however, if we are using another type
  // of locking which locks on a *file* basis, then as soon as the rename
  // is done if there is not a lock on the file a process like sendmail
  // could come in and complicate matters. After being properly locked,
  // rename the new mailbox file over the old mailbox file, and then
  // remove the old lock if applicable.
  //
  lockNewMailboxFile(fd);
  PRIV2(return_status,SafeRename(tmp_name, _real_path));
  if (return_status == -1) {
    // the rename failed -- we are in a world of hurt now.
    // We have successfully written the new mailbox out, unmapped the
    // old file, mapped in the new file, and bashed all of the various
    // pointers to point to the new mailbox; however, we cannot rename
    // the new mailbox over the old mailbox. We cannot continue, so return
    // this as a fatal error so that the caller can exit properly.
    //
    error.vSetError(DTME_CannotRenameNewMailboxFileOverOld,
		    DTM_TRUE, NULL, _real_path, tmp_name,
		    error.errnoMessage());
    (void) SafeClose(fd);
    (void) SafeClose(_fd);
    return;	// no complete cleanup necessary as we should exit real fast...
  }

  assert(map->file_size == bytesWritten);
  _file_size = map->file_size;

  struct stat info;
  if (SafeFStat(fd, &info) < 0) {
    if (_errorLogging)
      error.logError(DTM_TRUE, "writeMailBox(): fstat(%d/%s) failed errno=%d reason %s\nCannot obtain status information on mailbox.\nSuggest you close and reopen the mailbox.\n", _fd, tmp_name, errno, error.errnoMessage());
    error.setError(DTME_ObjectCreationFailed, DTM_TRUE, (Tt_message) 0);
  }
  else {
    if (info.st_size != _file_size) {
      error.logError(DTM_TRUE,
		     "writeMailBox(): new mailbox size not consistent with expected size = %d\nfstat: st_ino = %d, st_dev = %d, st_nlink = %d, st_size = %ld\nThe mailbox file appears to have been overwritten with another file.\nSuggest you close and reopen the mailbox.\n",
		     bytesWritten, info.st_ino, info.st_dev, info.st_nlink, info.st_size);
      error.setError(DTME_ObjectCreationFailed, DTM_TRUE, (Tt_message) 0);
    }
    utimbuf new_time;
    new_time.modtime = info.st_mtime;
    new_time.actime = new_time.modtime + 1;
    SafeUTime(_real_path, &new_time);
  }

  // order of unlocks is important here:
  // unlockOldMailboxFile checks the state of _long_lock_active but does not alter
  // it, whereas unlockFile does.
  //
  unlockOldMailboxFile(_fd);		// unlock old mailbox file first
  unlockFile(error2,fd);		// then unlock new mailbox file 
  if (SafeClose(_fd) < 0) {
      // should do something with the error here.
  }

  _fd = fd;				// new mailbox file now current one
  _dirty = 0;				// mark mailbox as no longer dirty.

  free (tmlFirst);
  free (iovFirst);

  return;
}

void
RFCMailBox::incorporate(DtMailEnv & error, const DtMailBoolean already_locked)
{
    if (already_locked == DTM_FALSE) {
	MutexLock lock_map(_map_lock);
    }

    int slot = mapFile(error, already_locked);
    if (error.isSet()) {
	return;
    }

    error.clear();

    MapRegion * map = _mappings[slot];
    const char * buf = map->file_region;
    
    // Let's accept any white space as okay in front of the
    // "From ".
    //
    for (; buf < (map->map_region + map->map_size) && 
	 isspace(*buf); buf++) {
	continue;
    }
    
    int num_tries = 0;
    DtMailBoolean done = DTM_FALSE;

    while (num_tries < 5 && !done) {
	if (strncmp(buf, "From ", 5) == 0) {
	    DtMailMessageHandle last = NULL;

	    // We can be here via either of two scenarios:
	    // 1- Aside from incorporating new mail, there is no other 
	    // activity on the mail box.  This is the "normal" case.
	    // already_locked is its default value (FALSE).
	    //
	    // 2- We are here because we were doing a Destroy Deleted Messages
	    // when we noticed new mail that had to be first incorporated.
	    // already_locked is TRUE (i.e., the Expunge method has lock...)
	    // We cannot place the handle of the last entity in the array in
	    // the stream because the entity may be marked for delete, and
	    // the Destroy Deleted Messages operation will expunge the entity
	    // shortly after it has been placed on the stream for the FE.

	    // In both cases, we want to get the last UNDELETED message and
	    // place it on the stream for the FE.  Undeleted entities will
	    // be valid even after a DDM while deleted entities will be invalid
	    // after a DMM. 

	    // Place the handle of the last entity in the array in the
	    // callback stream.  The FE will get it and can invoke 
	    // getNextMessageSummary() on the mailbox using the handle to 
	    // retrieve the new messages.

	    if (_msg_list.length() > 0) {
		
		if (already_locked) {
		    // already_locked is TRUE only in one case:
		    // We were trying to destroy deleted messages when we 
		    // noticed new mail has come in and we need to incorporate
		    // it.
		    // This will return the index of the last undeleted 
		    // message. That entity, we are assured, will remain 
		    // valid after a DMM that may appear just before the event
		    // is received by the FE's callback method.
		    
		    int index = prevNotDel(_msg_list.length() - 1);
		    last = _msg_list[index];
		} 
		else {
		    // already_locked is FALSE
		    // Normal case.
		    // No possiblity of entities in _msg_list being expunged.
		    // Give the current last one to the FE and let it retrieve
		    // new messages via the getNext() method on that entity.
		    last = _msg_list[_msg_list.length() - 1];
		}
	    }
	    
	    // Does locking...

	    parseFile(error, slot);
	    
	    DtMailEventPacket event;
	    event.key = _key;
	    event.target = DTM_TARGET_MAILBOX;
	    event.target_object = this;
	    event.operation = (void *)DTMC_NEWMAIL;
	    event.argument = last;
	    event.event_time = time(NULL);
	    _session->writeEventData(error, &event, sizeof(event));
	    done = DTM_TRUE;
	}
	else {
	    longUnlock(error);
	    _mail_box_writable = DTM_FALSE;
	    error.setError(DTME_ObjectInvalid);
	    *_object_valid = -1;
	    DtMailEventPacket event;
	    event.key = _key;
	    event.target = DTM_TARGET_MAILBOX;
	    event.target_object = this;
	    event.operation = (void *)DTMC_BADSTATE;
	    event.argument = NULL;
	    event.event_time = time(NULL);
	    _session->writeEventData(error, &event, sizeof(event));

	    // Debug
	    char strbuf[256];
	    if (already_locked) {
	    
		sprintf(strbuf, 
		    "Attempt #%d: RFCMailBox::incorporate(): No From: buf=<%.10s>, already_locked is TRUE\n", num_tries + 1, buf);
	    }
	    else {
		sprintf(strbuf, 
		    "Attempt #%d: RFCMailBox::incorporate(): No From: buf=<%.10s>, already_locked is FALSE\n", num_tries + 1, buf);
	    }
	    dumpMaps(strbuf);
	    if (num_tries > 0)
		sleep(num_tries);
	    num_tries++;
	}
    }
}

char *
RFCMailBox::generateLockFileName(void)
{
  char lock_path[MAXPATHLEN+20];

  assert(_real_path);
  (void) sprintf(lock_path, "%s.lock", _real_path);
  return(strdup(lock_path));
}

// Function: RFCMailBox::generateUniqueLockId - create unique ID for this mailbox lock files
// Description:
//   generateUniqueLockId creates a unique ID which is written into .lock files and
//   can then be checked to make sure that the lock file has not been compromised by
//   another process.
// Method:
//   The ID generated consists of three parts:
//     <process id/%08d><current time in seconds/%08d><hardware serial number/%d>
//   Thus, a "typical" id would look like this:
//      000018577971028681915751068
//   Which breaks down as:
//      00001857 797102868 1915751068
// Arguments:
//  <none>
// Outputs:
//  <none>
// Returns:
//  char * -> allocated memory in which unique id has been created
// 
char *
RFCMailBox::generateUniqueLockId(void)
{
  char theId[128];
  char hwserialbuf[64];
  
  memset(theId, 0, 128);
  memset(hwserialbuf, 0, 64);
  if (sysinfo(SI_HW_SERIAL, (char *)hwserialbuf, sizeof(hwserialbuf)-1) == -1)
    strcpy(hwserialbuf, "dtmail");
  (void) sprintf(theId, "%08d%08d%s\0", getpid(), time((time_t *)0), hwserialbuf);
  assert(strlen(theId)<sizeof(theId));
  return(strdup(theId));
}

void
RFCMailBox::checkLockFileOwnership(DtMailEnv & error)
{

  assert(_lockFileName);
  struct stat info;
  if (SafeStat(_lockFileName, &info) < 0) {
    if (_errorLogging)
      error.logError(DTM_FALSE,
		   "checkLockFileOwnership(): lock cannot be stat()ed: %s, errno = %d reason %s\nCannot obtain status information on mailbox lock file.\nSuggest you close and reopen the mailbox.\n",
		   _lockFileName, errno, error.errnoMessage());
    error.setError(DTME_ObjectInvalid);
    return;
  }

  int lock_fd;
  PRIV2(lock_fd,SafeOpen(_lockFileName, O_RDONLY | O_RSYNC | O_SYNC));
  if (lock_fd == -1) {
    if (_errorLogging)
      error.logError(DTM_FALSE,
		   "checkLockFileOwnership(): lock cannot be open()ed: %s, errno = %d reason %s\nCannot open the mailbox lock file.\nSuggest you close and reopen the mailbox.\n",
		   _lockFileName, errno, error.errnoMessage());
    error.setError(DTME_ObjectInvalid);
    return;
  }

  char lockBuf[MAXPATHLEN];
  int status = SafeRead(lock_fd, lockBuf, sizeof(lockBuf)-1);
  if (status <= 0) {
    if (_errorLogging)
      error.logError(DTM_FALSE,
		   "checkLockFileOwnership(): lock cannot be read: %s, errno = %d reason %s\nCannot read contents of mailbox lock file.\nSuggest you close and reopen the mailbox.\n",
		   _lockFileName, errno, error.errnoMessage());
    (void) SafeClose(lock_fd);
    error.setError(DTME_ObjectInvalid);
    return;
  }

  if ( (status < _uniqueLockIdLength+2)
       || (strncmp(lockBuf+2, _uniqueLockId, _uniqueLockIdLength) != 0) ) {
    error.logError(DTM_FALSE,
		   "checkLockFileOwnership(): dtmail lock file was stolen by another process.\nThe contents of your mailbox may have been changed by another process.\nSuggest you close and reopen the mailbox.\n");
    if (_errorLogging)
      HexDump((FILE*) NULL, "lock file stolen - lock file contents", (unsigned char *)lockBuf, status, 0);
    (void) SafeClose(lock_fd);
    error.setError(DTME_ObjectInvalid);
    return;
  }
  (void) SafeClose(lock_fd);
  return;  
}

// Function: RFCMailBox::linkLockFile - create and link temporary lock file to real lock file
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
time_t
RFCMailBox::linkLockFile(DtMailEnv & error, char *tempLockFileName)
{
  int lock_fd;
  int return_status;
  struct stat sbuf;

  // Create the temporary lock file. Failure to do so indicates lack of write permission
  // in the directory or some other fatal error
  //
  PRIV2(lock_fd,SafeOpen(tempLockFileName, O_RDWR | O_CREAT | O_EXCL | O_SYNC | O_RSYNC, 0666));
  if (lock_fd < 0) {
    // We are not able to create the temporary lock file.
    // We will have to punt on trying to lock here from now on.
    //
    switch (errno) {
    case EACCES:
      error.setError(DTME_CannotCreateMailboxLockFile_NoPermission);
      break;
      
    case EISDIR:
      error.setError(DTME_CannotCreateMailboxLockFile_IsDirectory);
      break;
      
    case ENOENT:
      error.setError(DTME_CannotCreateMailboxLockFile_NoSuchFile);
      break;
      
    case ENOLINK:
      error.setError(DTME_CannotCreateMailboxLockFile_RemoteAccessLost);
      break;
      
    default:
      error.vSetError(DTME_CannotCreateMailboxLockFile,
		      DTM_FALSE, NULL, tempLockFileName, error.errnoMessage());
      break;
    }
    return(time(0));
  }

  // Get creation time of temporary file *on remote system*
  //
  if (SafeFStat(lock_fd, &sbuf) == -1) {
    if (_errorLogging)
      error.logError(DTM_FALSE,
		   "linkLockFile(): temporary lock file cannot be stat()ed: %s, errno = %d reason %s\n",
		   tempLockFileName, errno, error.errnoMessage());
    error.vSetError(DTME_CannotCreateMailboxLockFile,
		    DTM_FALSE, NULL, tempLockFileName, error.errnoMessage());
    (void) SafeClose(lock_fd);
    return(time(0));
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
  writeResults = SafeWrite(lock_fd, "0\0", 2);
  if (writeResults == 2)
    writeResults += SafeWrite(lock_fd, _uniqueLockId, _uniqueLockIdLength);
  if ( (writeResults != _uniqueLockIdLength+2) ){
    if (_errorLogging)
      error.logError(DTM_FALSE,
		   "linkLockFile(): write to temporary lock file failed: %s, errno = %d reason %s\n",
		   tempLockFileName, errno, error.errnoMessage());
    error.vSetError(DTME_CannotCreateMailboxLockFile,
		    DTM_FALSE, NULL, tempLockFileName, error.errnoMessage());
    (void) SafeClose(lock_fd);
    return(time(0));
  }

  // sync up the lock file with the ultimate storage device
  //
  if (fsync(lock_fd) == -1) {
    if (_errorLogging)
      error.logError(DTM_FALSE,
		   "linkLockFile(): fsync to temporary lock file failed: %s, errno = %d reason %s\n",
		   tempLockFileName, errno, error.errnoMessage());
    error.vSetError(DTME_CannotCreateMailboxLockFile,
		    DTM_FALSE, NULL, tempLockFileName, error.errnoMessage());
    (void) SafeClose(lock_fd);
    return(time(0));
  }

  // close the file
  //
  if (SafeClose(lock_fd) == -1) {
    if (_errorLogging)
      error.logError(DTM_FALSE,
		   "linkLockFile(): close of temporary lock file failed: %s, errno = %d reason %s\n",
		   tempLockFileName, errno, error.errnoMessage());
    error.vSetError(DTME_CannotCreateMailboxLockFile,
		    DTM_FALSE, NULL, tempLockFileName, error.errnoMessage());
    return(time(0));
  }
         
  // The temporary lock file has been created - now try and link it to the real lock file
  // Failure here is not fatal as we will retry and possible try and remove the real lock
  // file later on
  //
  PRIV2(return_status,SafeLink(tempLockFileName, _lockFileName));
  if (return_status == -1) {
      PRIV2(return_status,SafeRemove(tempLockFileName));
    return(sbuf.st_ctime);
  }

  // We successfully linked the temp lock file to the real lock file name
  // This means we have the dot lock for our process - remove the temporary lock
  // file name (link) and return
  //
  PRIV2(return_status,SafeRemove(tempLockFileName));
  return(0);
}

void
RFCMailBox::lockFile(DtMailEnv & error)
{
  int return_status = 0;
    
  // We will create a simple lock file to keep the file from
  // changing while we are doing critical work.
  //
  
  // On some platforms, sendmail will place a lockf lock on the
  // file during mail delivery. If this is the case, then we
  // need to make sure we have the lock here.
  //
#if defined(SENDMAIL_LOCKS)
  assert(_lockf_active == DTM_FALSE);
  lseek(_fd, 0, SEEK_SET);
  PRIV2(return_status,SafeLockf(_fd, F_LOCK, 0));
  if (return_status != -1)
    _lockf_active = DTM_TRUE;
#endif
  
  if (_use_dot_lock == DTM_FALSE) {
    return;
  }

  // Implement the .lock short term lock protocol
  // This code was "adapted" from Solaris 2.5 (SunOS 5.5) usr/src/cmd/mail/maillock.c.
  //
  assert(_dot_lock_active == DTM_FALSE);
  
  // Create the temporary mail lock file name
  // It has the form <_lockfilename><XXXXXX> or mailbox.lockXXXXXX
  // mktemp then creates a unique temporary file for the template
  //
  assert(_lockFileName);
  char tempLockFileName[MAXPATHLEN];
  sprintf(tempLockFileName, "%sXXXXXX", _real_path);
  mktemp(tempLockFileName);
  PRIV2(return_status,SafeRemove(tempLockFileName));

  // loop through attempting to create the temporary lock file, and if successful attempt
  // to link the temporary lock file to the real lock file. If not successful, retry for
  // up to 5 minutes, and remove the current lock file if it is more than 5 minutes old.
  //
  int statFailed = 0;
  struct stat sbuf;
  
  for (;;) {
    // Attempt to create a temporary file and link it to the intended lock file
    // If it is successful, we have the lock and can return.
    // If it is not successful and error.isSet then it is a non-recoverable error,
    // in which case the mailbox is deemed not-writable any more.
    // If it is not successful and !error.isSet then it is a recoverable error,
    // in which case we spin and try again according to the retry rules.
    //
    time_t t = linkLockFile(error, tempLockFileName);
    if (error.isSet()) {	// hard error? -- something is wrong, assume read/only
      _use_dot_lock = DTM_FALSE;
      _mail_box_writable = DTM_FALSE;
      (void) SafeRemove(tempLockFileName);
      return;
    }
    if (t == 0) {
      checkLockFileOwnership(error);
      if (!error.isSet()) 
        _dot_lock_active = DTM_TRUE;
      return;
    }

    // Could not link the temporary lock file to the intended real lock file
    // See if the lock file exists and if so if we can remove it because it is > 5 mins old.
    // If the stat fails it means the lock file disappeared between our attempt to link to it
    // and now - only allow this to go on so many times before punting
    //
    if (SafeStat(_lockFileName, &sbuf) == -1) {
      if (statFailed++ > 5) {
	error.vSetError(DTME_CannotCreateMailboxLockFile,
			DTM_FALSE, NULL, _lockFileName, error.errnoMessage());
	return;
      }
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
      sleep(5);
      continue;
    }

    error.logError(DTM_FALSE,
		   "lockFile(): removing stale lock file %s ctime %08ld temp lock %s ctime %08ld diff %08ld\n",
		    _lockFileName, sbuf.st_ctime, tempLockFileName, t, t-sbuf.st_ctime);

    PRIV2(return_status,SafeRemove(_lockFileName));
    if (return_status == -1) {
      // We were not able to unlink the file. This means that
      // we do not have write access to the directory. We will
      // have to pass on taking long locks.
      //
      _use_dot_lock = DTM_FALSE;
      _mail_box_writable = DTM_FALSE;
      error.vSetError(DTME_CannotRemoveStaleMailboxLockFile,
		     DTM_FALSE, NULL, _lockFileName, error.errnoMessage());
      return;
    }
  }
}

// lockNewMailboxFile -- before renaming a new mailbox file over an old
// mailbox file, we need to establish a lock on the new mailbox file is
// a lock was established on the old mailbox file. Since the .lock protocol
// works on a file name basis we dont have to worry here, but if this
// system uses lockf() to lock the files we need to establish that lock
// on the new file first before renaming it over the old mailbox file.
//
void
RFCMailBox::lockNewMailboxFile(int new_fd)
{
  int return_status = 0;
  if (_lockf_active == DTM_TRUE) {
    lseek(new_fd, 0, SEEK_SET);
    PRIV2(return_status,SafeLockf(new_fd, F_LOCK, 0));
  }
}

// unlockOldMailboxFile -- after renaming a new mailbox file over an old
// mailbox file, if a lockf() style lock was established on the old mailbox
// file, it needs to be removed
//
void
RFCMailBox::unlockOldMailboxFile(int old_fd)
{
  int return_status = 0;
  if (_lockf_active == DTM_TRUE) {
    lseek(old_fd, 0, SEEK_SET);
    PRIV2(return_status,SafeLockf(old_fd, F_ULOCK, 0));
  }
}

void
RFCMailBox::transferLock(int old_fd, int new_fd)
{
  int return_status = 0;
  if (_lockf_active == DTM_TRUE) {
    lseek(new_fd, 0, SEEK_SET);
    PRIV2(return_status,SafeLockf(new_fd, F_LOCK, 0));

    lseek(old_fd, F_ULOCK, 0);
    PRIV2(return_status,SafeLockf(old_fd, F_ULOCK, 0));
  }
}

void
RFCMailBox::unlockFile(DtMailEnv & error, int fd)
{
  int return_status;
    
  // We will create a simple lock file to keep the file from
  // changing while we are doing critical work.
  //
  
  if (_use_dot_lock == DTM_TRUE) {
    assert(_dot_lock_active == DTM_TRUE);
    assert(_lockFileName);
    _dot_lock_active = DTM_FALSE;
    checkLockFileOwnership(error);
    if (!error.isSet()) {
        PRIV2(return_status,SafeUnlink(_lockFileName));
	if (return_status == -1) {
          error.vSetError(DTME_CannotRemoveMailboxLockFile,
		     DTM_FALSE, NULL, _lockFileName, error.errnoMessage());
      }
    }
  }
  
#if defined(SENDMAIL_LOCKS)
    if (_lockf_active == DTM_TRUE) {
      lseek(fd, 0, SEEK_SET);
      PRIV2(return_status,SafeLockf(fd, F_ULOCK, 0));
      _lockf_active = DTM_FALSE;
    }
#endif
}

DTMBX_LONGLOCK
RFCMailBox::longLock(DtMailEnv & error)
{
  // Possible return values,
  // DTMBX_LONGLOCK_FAILED_CANCEL	- failed to obtain lock, cancel operation
  // DTMBX_LONGLOCK_FAILED_READONLY	- failed to obtain lock, open read only
  // DTMBX_LONGLOCK_FAILED_READWRITE	- failed to obtain lock, open read write [user locking override]
  // DTMBX_LONGLOCK_SUCCESSFUL	       	- obtained the lock
  //
  // This function will also manipulate "_mail_box_writable" and "_lock_flag"
  //
    _lock_obj = new FileShare(error,
			    _session,
			    _real_path,
			    _callback,
			    _cb_data);
    if (error.isSet()) {
      DtMailBoolean returnCode;
      error.clear();
      returnCode = _lock_obj->readWriteOverride(error);
      if (error.isSet()) {
	// error set - this means "cancel the request"
	//
	delete _lock_obj;
	_lock_obj = NULL;
	return(DTMBX_LONGLOCK_FAILED_CANCEL);
      }
      if (returnCode == DTM_TRUE) {
	// return code is "DTM_TRUE" - this means "readWrite"
	//
	delete _lock_obj;
	_lock_obj = NULL;
	_mail_box_writable = DTM_TRUE;
	_lock_flag = DTM_FALSE;
	return(DTMBX_LONGLOCK_FAILED_READWRITE);
      }
      else {
	// return code is "DTM_FALSE" - this means readOnly
	//
	delete _lock_obj;
	_lock_obj = NULL;
	_mail_box_writable = DTM_FALSE;
	return(DTMBX_LONGLOCK_FAILED_READONLY);
      }
    }

    // Now talking with the tooltalk database server on the proper system
    // Request a lock on the mailbox
    //
    _lock_obj->lockFile(error);
    if (error.isSet()) {
      if (_lock_obj->readOnly(error) == DTM_TRUE) {
	delete _lock_obj;
	_lock_obj = NULL;
	_mail_box_writable = DTM_FALSE;
	return(DTMBX_LONGLOCK_FAILED_READONLY);
      } else {
	delete _lock_obj;
	_lock_obj = NULL;
	return(DTMBX_LONGLOCK_FAILED_CANCEL);
      }
    }

    _long_lock_active = DTM_TRUE;
    return(DTMBX_LONGLOCK_SUCCESSFUL);
}

void
RFCMailBox::longUnlock(DtMailEnv & error)
{
  error.clear();
  
  _long_lock_active = DTM_FALSE;
  
  if (_lock_flag == DTM_TRUE && _lock_obj) {
    delete _lock_obj;
    _lock_obj = NULL;
  }
/* XXX
  if (_mail_box_writable == DTM_FALSE) {
    return;
  }
  */
  
#if !defined(SENDMAIL_LOCKS)
  else if (_lockf_active == DTM_TRUE) {
    int return_status = 0;
    lseek(_fd, 0, SEEK_SET);
    PRIV2(return_status,SafeLockf(_fd, F_ULOCK, 0));
    _lockf_active = DTM_FALSE;
  }
#endif
}

RFCMailBox::MapRegion *
RFCMailBox::mapNewRegion(DtMailEnv & error, int fd, unsigned long size)
{
  assert(fd);
  
  // Create the mapped region.
  //
  MapRegion * map = new MapRegion;
  map->offset = 0;
  map->file_size = size;
  long page_size = memoryPageSize();
  map->map_size = map->file_size + (page_size - (map->file_size % page_size)) + page_size;
  
  int flags = MAP_PRIVATE;
  
#if defined(MMAP_NORESERVE)
  // We are not supposed to be writing to these pages. If
  // we don't specify MAP_NORESERVE however, the system will
  // reserve swap space equal to the file size to deal with
  // potential writes. This is wasteful to say the least.
  //
  flags |= MAP_NORESERVE;
#endif
  
  map->map_region = (char *)mmap(0,
				 map->map_size,
				 PROT_READ,
				 flags,
				 fd,
				 0);
  if (map->map_region == (char *)-1) {
    switch(errno) {
    case ENOMEM:
      error.setError(DTME_CannotReadNewMailboxFile_OutOfMemory);
      break;
      
    default:
      error.vSetError(DTME_CannotReadNewMailboxFile,
		      DTM_FALSE, NULL, error.errnoMessage());
      break;
    }
    
    delete map;
    return((MapRegion *)NULL);
  }
  
  map->file_region = map->map_region;
  
  return(map);
}

void
RFCMailBox::makeHeaderLine(DtMailEnv & error, 
			    int slot,
			    const DtMailHeaderRequest & request,
			    DtMailHeaderLine & headers)
{
    MessageCache * mc = _msg_list[slot];
    DtMail::Envelope * env = mc->message->getEnvelope(error);

    // For each request, we need to retrieve the header values.
    //
    headers.number_of_names = request.number_of_names;
    headers.header_values = new DtMailValueSeq[headers.number_of_names];

    for (int req = 0; req < request.number_of_names; req++) {
	// RFC Message::getHeader will pass abstract names through
	// as transport names if they can not be found in the abstract
	// table. Because of this we say all names are abstract and
	// rely on the specific implementation of RFCMessage. This
	// is potentially dangerous, but we are allowed to define
	// and require these semantics for RFCMessage (and this same
	// information appears in the appropriate place in getHeader.
	//
	env->getHeader(error,
		       request.header_name[req],
		       DTM_TRUE,
		       headers.header_values[req]);
	if (error.isSet()) {
	    headers.header_values[req].clear();
	    error.clear();
	}
    }

    error.clear();
}

void
RFCMailBox::waitForMsgs(int needed)
{
    while(_at_eof == 0 && needed >= _msg_list.length()) {
	ThreadSleep(1);
    }
    return;
}

void
RFCMailBox::writeToDumpFile(const char *format, ...)
{
  DtMailEnv error;
  off_t total_file_size = 0;
  
  FILE * df = fopen("/tmp/dtmail.dump", "a");
  
  const time_t clockTime = (const time_t) time((time_t *)0);
  fprintf(df, "--------------------- pid=%d %s", getpid(), ctime(&clockTime));
  
  va_list	var_args;
  va_start(var_args, format);
  vfprintf(df, format, var_args);
  va_end(var_args);
  
  fprintf(df, "---------------------\n");
  fprintf(df, "\n\n");
  
  fclose(df);
}

void
RFCMailBox::dumpMaps(const char *str)
{
  struct stat buf;
  DtMailEnv error;
  off_t total_file_size = 0;
  
  
  FILE * df = fopen("/tmp/dtmail.dump", "a");
  
  if (SafeFStat(_fd, &buf) < 0) {
    fprintf(df, "dumpMaps(): fstat(%d) failed errno=%d\n", _fd, errno);
  }

  if (str == NULL) {
    str = "\n";
  }
  
  const time_t clockTime = (const time_t) time((time_t *)0);
  fprintf(df, "--------------------- pid=%d %s", getpid(), ctime(&clockTime));
  fprintf(df, str);
  fprintf(df, "---------------------\n");
  fprintf(df, "Mappings = %d\n", _mappings.length());
  fprintf(df, "Map Entries:\n");
  for (int m = 0; m < _mappings.length(); m++) {
    MapRegion *map = _mappings[m];
    fprintf(df, "map[%d]: map_region = 0x%08lx, map_size = 0x%08lx(%08ld)\n", m, map->map_region, map->map_size, map->map_size);
    if (map->map_size % memoryPageSize())
      fprintf(df, "ERROR! map->map_size not mod %d\n", memoryPageSize());
    HexDump(df, "map_region", (unsigned char *)map->map_region, map->file_size, _errorLogging ? 0 : 10);
    fprintf(df, "map[%d]: file_region = 0x%08lx, file_size = 0x%08lx(%08ld), offset = 0x%08lx(%08ld)\n", m, map->file_region, map->file_size, map->file_size, map->offset, map->offset);
    if (strncasecmp(map->file_region, "From", 4))
      fprintf(df, "ERROR! map->file_region does not begin with From\n");
    HexDump(df, "file_region", (unsigned char *)map->file_region, map->file_size, _errorLogging ? 0 : 10);
    total_file_size += map->file_size;
    if ((total_file_size % 4096) == 0) {
      fprintf(df, "Total file size falls on page boundary, totalsize = %d\n", total_file_size);
    }
  }
  
  fprintf(df, "\nstat buffer entries: st_ino = %d, st_dev = %d, st_nlink = %d, st_size = %ld\n", buf.st_ino, buf.st_dev, buf.st_nlink, buf.st_size);
  
  fprintf(df, "\n\n");
  
  fclose(df);
}

// The following routines are required to bind this format specific driver
// into the format neutral layer.
//
// The first entry point is the capability query interface. This is used
// by the client to determine what capabilities we support.
//
void
RFCQueryImpl(DtMail::Session & session,
	      DtMailEnv & error,
	      const char * capability,
	      va_list args)
{
    error.clear();

    if (strcmp(capability, DtMailCapabilityPropsSupported) == 0) {
	DtMailBoolean * resp = va_arg(args, DtMailBoolean *);
	*resp = DTM_FALSE;
	return;
    }

    if (strcmp(capability, DtMailCapabilityImplVersion) == 0) {
	char * version = va_arg(args, char *);
	strcpy(version, "1.0");
	return;
    }

    if (strcmp(capability, DtMailCapabilityInboxName) == 0) {
	DtMailObjectSpace * space = va_arg(args, DtMailObjectSpace *);
	void ** inbox = va_arg(args, void **);

	DtMail::MailRc * mailrc = session.mailRc(error);
	const char * path = NULL;
	char inbox_path[MAXPATHLEN];

	mailrc->getValue(error, "DT_MAIL", &path);
	if (error.isSet()) {
	    mailrc->getValue(error, "MAIL", &path);
	    if (error.isSet()) {

		passwd pw;
		GetPasswordEntry(pw);
		sprintf(inbox_path, MAIL_SPOOL_PATH, pw.pw_name);
		path = strdup(inbox_path);
	    }
	    else {
		error.clear();
	    }
	}
	else {
	    error.clear();
	}

	*space = DtMailFileObject;
	*inbox = strdup(path);
	free((void *)path);
	return;
    }

    if (strcmp(capability, DtMailCapabilityTransport) == 0) {
	DtMailBoolean * resp = va_arg(args, DtMailBoolean *);
	*resp = DTM_TRUE;
	return;
    }

    error.setError(DTME_NotSupported);
    return;
}

// The QueryOpen entry point is used to determin if we can open the specified
// path. If the name is of the form host:/path, or is a simple path, then we
// will say we can. Additional work should be done for content typing here,
// but we'll skip it for now.
//
DtMailBoolean
RFCQueryOpen(DtMail::Session &,
	      DtMailEnv & error,
	      DtMailObjectSpace space,
	      void * arg)
{
    error.clear();

    // We can do buffers, so just say yes.
    //
    if (space == DtMailBufferObject) {
	return(DTM_TRUE);
    }

    // If this isn't in the file name space, then give up now.
    //
    if (space != DtMailFileObject) {
	return(DTM_FALSE);
    }

    char * path = (char *) arg;

    // First, is this of the form "host:/path". If so, it is probably
    // an RFC mail box, so we will say we can open it.
    //
    for (const char * c = path; *c; c++) {
	if (*c == '/') {
	    // We hit a slash first, so colon's are not useful.
	    break;
	}

	if (*c == ':' && *(c+1) == '/') {
	    // Looks like we have a host name. We should do a more
	    // robust check at this point, but for now we will assume
	    // that we have a valid host.
	    return(DTM_TRUE);
	}
    }

    // Okay, there defintely is not a host name. See if we can stat the
    // file. If we can, then assume we can open it, otherwise. If the file
    // doesn't exist, then we can create a new one.
    //
    struct stat sbuf;
    if (stat(path, &sbuf) < 0) {
	if (errno == ENOENT) {
	    return(DTM_TRUE);
	}
	else {
	    return(DTM_FALSE);
	}
    }

    return(DTM_TRUE);
}

// The mail box construct entry point creates an instance of the RFC
// mail box object. This object is then accessed through the virtual
// DtMail::MailBox class API.
//
DtMail::MailBox *
RFCMailBoxConstruct(DtMail::Session & session,
		     DtMailEnv & error,
		     DtMailObjectSpace space,
		     void * arg,
		     DtMailCallback cb,
		     void * client_data)
{
    return(new RFCMailBox(error, &session, space, arg, cb, client_data, "Internet MIME"));
}

DtMailBoolean
RFCMessageQuery(DtMail::Session &,
		DtMailEnv & error,
		DtMailObjectSpace space,
		void *)
{
    error.clear();

    if (space != DtMailBufferObject) {
	return(DTM_FALSE);
    }

    return(DTM_TRUE);
}

DtMail::Message *
RFCMessageConstruct(DtMail::Session & session,
		    DtMailEnv & error,
		    DtMailObjectSpace space,
		    void * arg,
		    DtMailCallback cb,
		    void * client_data)
{
    return(new RFCMessage(error, &session, space, arg, cb, client_data));
}

DtMail::Transport *
RFCMIMETransportConstruct(DtMail::Session & session,
			  DtMailEnv & error,
			  DtMailStatusCallback cb,
			  void * cb_data)
{
    return(new RFCTransport(error, &session, cb, cb_data, "Internet MIME"));
}

// The meta factory is responsible for returning the entry points
// required for locating and creating various mail objects based for
// the RFC implementation. It is essentially a switch table.
//

extern "C" void *
RFCMetaFactory(const char * op)
{
    if (strcmp(op, QueryImplEntryOp) == 0) {
	return(RFCQueryImpl);
    }

    if (strcmp(op, QueryOpenEntryOp) == 0) {
	return(RFCQueryOpen);
    }

    if (strcmp(op, MailBoxConstructEntryOp) == 0) {
	return(RFCMailBoxConstruct);
    }

    if (strcmp(op, QueryMessageEntryOp) == 0) {
	return(RFCMessageQuery);
    }

    if (strcmp(op, MessageConstructEntryOp) == 0) {
	return(RFCMessageConstruct);
    }

    if (strcmp(op, TransportConstructEntryOp) == 0) {
	return(RFCMIMETransportConstruct);
    }

    return(NULL);
}

// The mail box construct entry point creates an instance of the RFC
// mail box object. This object is then accessed through the virtual
// DtMail::MailBox class API.
//
DtMail::MailBox *
V3MailBoxConstruct(DtMail::Session & session,
		     DtMailEnv & error,
		     DtMailObjectSpace space,
		     void * arg,
		     DtMailCallback cb,
		     void * client_data)
{
    return(new RFCMailBox(error, &session, space, arg, cb, client_data, 
				"Sun Mail Tool"));
}

DtMail::Transport *
RFCV3TransportConstruct(DtMail::Session & session,
			DtMailEnv & error,
			DtMailStatusCallback cb,
			void * cb_data)
{
    return(new RFCTransport(error, &session, cb, cb_data, "Sun Mail Tool"));
}

// The meta factory is responsible for returning the entry points
// required for locating and creating various mail objects based for
// the RFC implementation. It is essentially a switch table.
//
extern "C" void *
V3MetaFactory(const char * op)
{
    if (strcmp(op, QueryImplEntryOp) == 0) {
	return(RFCQueryImpl);
    }

    if (strcmp(op, QueryOpenEntryOp) == 0) {
	return(RFCQueryOpen);
    }

    if (strcmp(op, MailBoxConstructEntryOp) == 0) {
	return(V3MailBoxConstruct);
    }

    if (strcmp(op, QueryMessageEntryOp) == 0) {
	return(RFCMessageQuery);
    }

    if (strcmp(op, MessageConstructEntryOp) == 0) {
	return(RFCMessageConstruct);
    }

    if (strcmp(op, TransportConstructEntryOp) == 0) {
	return(RFCV3TransportConstruct);
    }

    return(NULL);
}
