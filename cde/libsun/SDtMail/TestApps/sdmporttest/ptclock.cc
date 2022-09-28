#include <porttest.hh>
#include <SDtMail/LinkedList.hh>
#include <ptcmdscan.hh>
#include <Utils/LockUtility.hh>
#include <ptclock.hh>
#include <SDtMail/SystemUtility.hh>
#include <SDtMail/Sdtmail.hh>

/*
** EXTERNALS
*/

/*
** GLOBALS
*/
#if 0
char *Hlock =
"Description: manipulate lock objects\n"
"General usage: lock command [ arguments ]\n"
"Specific usage:\n"
"\tlock check lock name parameter [value]\n"
"\tlock clear [lock-name(s) | *]\n"
"\tlock compare lock1 lock2\n"
"\tlock fetch lock name [parameter]\n"
"\tlock set lock name parameter [value]\n"
"\tlock unset lock name [parameter [value]]\n"
"";
#else
char *Hlock =
"Description: manipulate lock objects\n"
"General usage: lock command [ arguments ]\n"
"Specific usage:\n"
"\tlock create name\n"
"\tlock delete [lock(s) | *]\n"
"\tlock grablock lock [true | false]\n"
"\tlock idobject lock objectname\n"
"\tlock lockupdate lock\n"
"\tlock locksession lock\n"
"\tlock releaseupdate lock\n"
"\tlock releasesession lock\n"
"\tlock status [lock(s) | *]\n"
"\tlock validate [ string | -F file | -S sum ]\n"
"";
#endif

SdmLinkedList		*LOCK_LIST;

static int subcmd_lock_create(int argc, char **argv);
static int subcmd_lock_delete(int argc, char **argv);
static int subcmd_lock_grablock(int argc, char **argv);
static int subcmd_lock_idobject(int argc, char **argv);
static int subcmd_lock_lockupdate(int argc, char **argv);
static int subcmd_lock_locksession(int argc, char **argv);
static int subcmd_lock_releaseupdate(int argc, char **argv);
static int subcmd_lock_releasesession(int argc, char **argv);
static int subcmd_lock_status(int argc, char **argv);
static int subcmd_lock_validate(int argc, char **argv);

static SUBCMD lockcmdtab[] = {
  // cmdname,	handler,	argsrequired
  {"create",	subcmd_lock_create,	0},	// create a new lock
  {"delete",	subcmd_lock_delete,	0},	// delete an existing lock
  {"grablock",	subcmd_lock_grablock,	0},	// set/clear grablock flag
  {"idobject",	subcmd_lock_idobject,	0},	// set the lock object name
  {"lockupdate", subcmd_lock_lockupdate, 0},
  {"locksession", subcmd_lock_locksession, 0},
  {"status",	subcmd_lock_status,	0},	// status of locks
  {"releaseupdate", subcmd_lock_releaseupdate, 0},
  {"releasesession", subcmd_lock_releasesession, 0},
  {"validate",		subcmd_lock_validate,		0},	// validate last result
  {NULL,	0,		0}	// must be last entry in table
};

/*
** FORWARD DECLARATIONS
*/

/*************************************************************************
 *
 * Function:	Clock - lock command handler
 *
 * Input:	int argc - number of arguments passed to command
 *		char **argv - array of pargc arguments passed
 *
 * Output:	TRUE - help given
 *
 ************************************************************************/

int Clock(int argc, char **argv)
{
  //  return (argc == 1 ? TRUE : oneSubCommand(argv[0], argv[1], lockcmdtab, argc-2, argv+2));
  assert(argc != 0);
  if (argc == 1)
    return (TRUE);

  // All cmds except "validate" reset the last result
  if (strcmp(argv[1], "validate") != 0)
    ptLastResult.Reset();

  int rc = (argc == 1 ? TRUE : oneSubCommand(argv[0], argv[1], lockcmdtab, argc-2, argv+2));

  // All cmds except "validate" potentially print the last result
  if (strcmp(argv[1], "validate") != 0)
    ptLastResult.Print();

  return rc;
}

static char lastErrorMessage[2048];

static const char* detailedErrorReport(SdmError& err)
{
  char msgbuf[1024];

  lastErrorMessage[0] = '\0';
  sprintf(msgbuf, "%u: %s", (SdmErrorCode) err, err.ErrorMessage());
  strcat(lastErrorMessage, msgbuf);
  int i = err.MinorCount();
  while (i--) {
    sprintf(msgbuf, "  minor code %u: %s", (SdmErrorCode) err[i], err.ErrorMessage(i));
    strcat(lastErrorMessage, msgbuf);
  }
  assert(strlen(lastErrorMessage) < sizeof(lastErrorMessage));
  assert(strlen(lastErrorMessage));
  return (lastErrorMessage);
}

static PlkLock *validLockName(char *lockName)
{
  SdmLinkedListIterator lit(LOCK_LIST);
  PlkLock *Plk;

  while (Plk = (PlkLock *)(lit.NextListElement())) {
    assert(Plk->plk_name);
    if (strcasecmp((const char *)*Plk->plk_name, lockName)==0)
      return(Plk);
  }
  return((PlkLock *)0);
}

const char *expandLockName(PlkLock *Plk)
{
  assert(Plk->plk_name);
  return ((const char *)*Plk->plk_name);
}

static void lockCallback(const SdmLockEventType lockEvent, void* clientData)
{
  PlkLock *Plk = (PlkLock*)clientData;
  if (!Plk) {
    fprintf(stderr, "\nlockCallback ERROR: client data for event %u is NIL\n", (unsigned long)lockEvent);
    fflush(stderr);
    return;
  }
  SdmLockUtility *lk = Plk->plk_lock;
  if (!lk) {
    fprintf(stderr, "\nlockCallback ERROR: client data %08lx for event %u has NIL lock object\n", 
	    (unsigned long) Plk, (unsigned long)lockEvent);
    fflush(stderr);
    return;
  }

  switch (lockEvent) {
  case Sdm_LKEC_SessionLockGoingAway:		// another mailer taking lock - save changes
    fprintf(stderr, "\nlock %s: callback Session Lock Going Away on object %s\n",
	    expandLockName(Plk), (const char *) Plk->plk_idobject_name);
    fflush(stderr);
    break;
  case Sdm_LKEC_SessionLockTakenAway:		// another mailer stole lock from us - save changes
    fprintf(stderr, "\nlock %s: callback Session Lock Taken Away on object %s\n",
	    expandLockName(Plk), (const char *) Plk->plk_idobject_name);
    fflush(stderr);
    break;
  case Sdm_LKEC_WaitingForSessionLock:		// entered a wait for a session lock
    fprintf(stderr, "\nlock %s: callback Waiting For Session Lock on object %s\n",
	    expandLockName(Plk), (const char *) Plk->plk_idobject_name);
    fflush(stderr);
    break;
  case Sdm_LKEC_NotWaitingForSessionLock:	// no longer waiting for a session lock,
    fprintf(stderr, "\nlock %s: callback Not Waiting For Session Lock on object %s\n",
	    expandLockName(Plk), (const char *) Plk->plk_idobject_name);
    fflush(stderr);
    break;
  case Sdm_LKEC_WaitingForUpdateLock:		// entered a wait for an update lock
    fprintf(stderr, "\nlock %s: callback Waiting For Update Lock on object %s\n",
	    expandLockName(Plk), (const char *) Plk->plk_idobject_name);
    fflush(stderr);
    break;
  case Sdm_LKEC_NotWaitingForUpdateLock:	// no longer waiting for an update lock
    fprintf(stderr, "\nlock %s: callback Not Waiting For Update Lock on object %s\n",
	    expandLockName(Plk), (const char *) Plk->plk_idobject_name);
    fflush(stderr);
    break;
  default:
    fprintf(stderr, "\nlock %s: ERROR: callback specifies unknown lock event code %u on object %s\n",
	    expandLockName(Plk), (unsigned long) lockEvent, (const char *) Plk->plk_idobject_name);
    fflush(stderr);
    break;
  }
  return;
}

static void deleteLock(PlkLock *Plk, int printMessage)
{
  assert(Plk);
  SdmLockUtility *lk = Plk->plk_lock;
  assert(lk);
  printf("lock %s: deleted\n", expandLockName(Plk));
  LOCK_LIST->RemoveElementFromList(Plk);
  delete Plk;
}

static void statusLock(PlkLock *Plk, int printMessage)
{
  SdmLockTypes locksHeld = 0;
  assert(Plk);
  SdmLockUtility *lk = Plk->plk_lock;
  assert(lk);

  (void) lk->DetermineLocks(locksHeld);

  printf("lock %s: exists", expandLockName(Plk));
  if (Plk->plk_idobject_name.Length())
    printf(", object='%s'", (const char *)Plk->plk_idobject_name);
  if (locksHeld & Sdm_LKT_Update)
    printf(", update-locked");
  if (locksHeld & Sdm_LKT_Session)
    printf(", session-locked");
  printf("\n");
}

PlkLock *lookupLock(char *lockName, int printMessage)
{
  PlkLock *Plk;

  assert(lockName);

  // If the lock list is empty, nothing to do here
  if (!LOCK_LIST || !LOCK_LIST->ElementCount()) {
    if (printMessage)
      printf("?No locks defined\n");
    return((PlkLock *)0);
  }

  // lookup the lock by name - if the name is registered, return that lock 
  Plk = validLockName(lockName);
  if (Plk)
    return(Plk);

  // lock not registered - say so and return nothing
  if (printMessage)
    printf("?lock %s does not exist\n", lockName);
  return((PlkLock *)0);
}

static int subcmd_lock_create(int argc, char **argv)
{
  static int locksCreated = 0;
  SdmError err;

  if (argc != 1) {
    printf("?The create subcommand requires a name argument: lock create name \n");
    ptLastResult = Sdm_False;
    return(TRUE);
  }

  // See if the lock name already exists
  if (lookupLock(*argv, FALSE) != (PlkLock *)0) {
    printf("?Lock %s already exists\n", *argv);
    ptLastResult = Sdm_False;
    return(TRUE);
  }

  // If first lock object created, initialize connection to lock object manager

  if (!locksCreated++) {
    SdmLockUtility::ConnectionStartup(err, "sdmporttest", (void *)0);
    if (err != Sdm_EC_Success) {
      printf("lock %s: connection startup failed with error %s\n", *argv, detailedErrorReport(err));
      return(TRUE);
    }
  }

  // Name is unique - create lock with it
  SdmString *name = new SdmString(*argv);
  assert(name);
  PlkLock *Plk = new PlkLock();
  assert(Plk);
  SdmLockUtility *lk = new SdmLockUtility();
  assert(lk);
  (void) lk->IdentifyLockCallback(err, lockCallback, (void *)Plk);
  if (err != Sdm_EC_Success) {
    printf("lock %s: WARNING: IdentifyLockCallback failed with error %s\n", *argv, detailedErrorReport(err));
  }
  if (!LOCK_LIST)
    LOCK_LIST = new SdmLinkedList();
  Plk->plk_lock = lk;
  Plk->plk_name = name;
  LOCK_LIST->AddElementToList(Plk);
  printf("lock %s: defined\n", expandLockName(Plk));
  ptLastResult = Sdm_True;
  return(TRUE);
}

static int subcmd_lock_delete(int argc, char **argv)
{
  if (!argc) {
    printf("?The delete subcommand takes one or more arguments which specify\n?the locks to be deleted; to delete all locks, use a single * character.\n");
    ptLastResult = Sdm_False;
    return(TRUE);
  }
  while (argc--) {
    char *currentLock = *argv++;
    if (strcmp(currentLock,"*")==0) {
      SdmLinkedListIterator lit(LOCK_LIST);
      PlkLock *Plk;

      while (Plk = (PlkLock *)(lit.NextListElement())) {
	deleteLock(Plk, TRUE);
      }
    }
    else {
      PlkLock *Plk = lookupLock(currentLock, TRUE);
      if (Plk)
	deleteLock(Plk, TRUE);
    }
  }
  ptLastResult = Sdm_True;
  return(TRUE);
}

static int subcmd_lock_grablock(int argc, char **argv)
{
  if (argc != 2) {
    printf("The grablock subcommand takes the form: lock idobject lockname [true | false]\n");
    ptLastResult = Sdm_False;
    return(TRUE);
  }

  SdmError err;
  PlkLock *Plk;
  SdmLockUtility *lk;
  char *lockName = *argv++;
  Plk = lookupLock(lockName, TRUE);
  if (!Plk)
    return(TRUE);
  lk = Plk->plk_lock;
  assert(lk);

  SdmString tfstr = (const char *)*argv++;
  SdmBoolean grabflag;

  if (strcasecmp(tfstr,"t")==0
      || strcasecmp(tfstr,"tr")==0
      || strcasecmp(tfstr,"tru")==0
      || strcasecmp(tfstr,"true")==0)
    grabflag = Sdm_True;
  else if (strcasecmp(tfstr,"f")==0
	   || strcasecmp(tfstr,"fa")==0
	   || strcasecmp(tfstr,"fal")==0
	   || strcasecmp(tfstr,"fals")==0
	   || strcasecmp(tfstr,"false")==0)
    grabflag = Sdm_False;
  else {
    printf("The grablock flag requires an argument of 'true' or 'false'\n");
    ptLastResult = Sdm_False;
    return(TRUE);
  }    


  if (  lk->GrabLockIfOwnedByOtherMailer(err, grabflag) != Sdm_EC_Success) {
    printf("lock %s: grab lock flag failed with error %s\n", expandLockName(Plk), detailedErrorReport(err));
    ptLastResult = Sdm_False;
    return(TRUE);
  }

  printf("lock %s: grab lock flag successfully set\n", expandLockName(Plk));
  ptLastResult = Sdm_True;
  return(TRUE);
}

static int subcmd_lock_idobject(int argc, char **argv)
{
  if (argc != 2) {
    printf("The idobject subcommand takes the form: lock idobject lockname objectname\n");
    ptLastResult = Sdm_False;
    return(TRUE);
  }

  SdmError err;
  PlkLock *Plk;
  SdmLockUtility *lk;
  char *lockName = *argv++;
  Plk = lookupLock(lockName, TRUE);
  if (!Plk)
    return(TRUE);
  lk = Plk->plk_lock;
  assert(lk);

  SdmString objectname = (const char *)*argv++;

  if (lk->IdentifyLockObject(err, objectname) != Sdm_EC_Success) {
    printf("lock %s: identify lock object failed with error %s\n", expandLockName(Plk), detailedErrorReport(err));
    ptLastResult = Sdm_False;
    return(TRUE);
  }

  printf("lock %s: object identify successfully set\n", expandLockName(Plk));
  Plk->plk_idobject_name = objectname;
  ptLastResult = Sdm_True;
  return(TRUE);
}

static int subcmd_lock_lockupdate(int argc, char **argv)
{
  if (argc != 1) {
    printf("The lockupdate subcommand takes the form: lock lockupdate lockname\n");
    ptLastResult = Sdm_False;
    return(TRUE);
  }

  SdmError err;
  PlkLock *Plk;
  SdmLockUtility *lk;
  SdmLockTypes locksHeld = 0;

  char *lockName = *argv++;
  Plk = lookupLock(lockName, TRUE);
  if (!Plk)
    return(TRUE);
  lk = Plk->plk_lock;
  assert(lk);

  (void) lk->DetermineLocks(locksHeld);

  if (lk->ObtainLocks(err, Sdm_LKT_Update) != Sdm_EC_Success) {
    printf("lock %s: obtain update lock on lock object failed with error %s\n", expandLockName(Plk), detailedErrorReport(err));
    if (locksHeld & Sdm_LKT_Update) {
      printf("lock %s: update lock already obtained\n", expandLockName(Plk));
    }
    if (!Plk->plk_idobject_name.Length()) {
      printf("lock %s: object not identified\n", expandLockName(Plk));
    }
    ptLastResult = Sdm_False;
    return(TRUE);
  }

  printf("lock %s: obtain update lock successful\n", expandLockName(Plk));
  ptLastResult = Sdm_True;
  return(TRUE);
}

static int subcmd_lock_locksession(int argc, char **argv)
{
  if (argc != 1) {
    printf("The locksession subcommand takes the form: lock locksession lockname\n");
    ptLastResult = Sdm_False;
    return(TRUE);
  }

  SdmError err;
  PlkLock *Plk;
  SdmLockUtility *lk;
  SdmLockTypes locksHeld = 0;
  char *lockName = *argv++;
  Plk = lookupLock(lockName, TRUE);
  if (!Plk)
    return(TRUE);
  lk = Plk->plk_lock;
  assert(lk);

  (void) lk->DetermineLocks(locksHeld);

  lk->ObtainLocks(err, Sdm_LKT_Session);

  if (err != Sdm_EC_Success) {
    printf("lock %s: obtain session lock on lock object failed with error %s\n", expandLockName(Plk), detailedErrorReport(err));
    if (locksHeld & Sdm_LKT_Session) {
      printf("lock %s: session lock already obtained\n", expandLockName(Plk));
    }
    if (!Plk->plk_idobject_name.Length()) {
      printf("lock %s: object not identified\n", expandLockName(Plk));
    }
    ptLastResult = Sdm_False;
    return(TRUE);
  }

  printf("lock %s: obtain session lock successful\n", expandLockName(Plk));
  ptLastResult = Sdm_True;
  return(TRUE);
}

static int subcmd_lock_releaseupdate(int argc, char **argv)
{
  if (argc != 1) {
    printf("The releaseupdate subcommand takes the form: lock releaseupdate lockname\n");
    ptLastResult = Sdm_False;
    return(TRUE);
  }

  SdmError err;
  PlkLock *Plk;
  SdmLockUtility *lk;
  SdmLockTypes locksHeld = 0;

  char *lockName = *argv++;
  Plk = lookupLock(lockName, TRUE);
  if (!Plk)
    return(TRUE);
  lk = Plk->plk_lock;
  assert(lk);

  (void) lk->DetermineLocks(locksHeld);

  if (!(locksHeld & Sdm_LKT_Update)) {
    printf("lock %s: no update lock in effect\n", expandLockName(Plk));
    ptLastResult = Sdm_False;
    return(TRUE);
  }

  if (lk->ReleaseLocks(err, Sdm_LKT_Update) != Sdm_EC_Success) {
    printf("lock %s: release update lock on lock object failed with error %s\n", expandLockName(Plk), detailedErrorReport(err));
    ptLastResult = Sdm_False;
    return(TRUE);
  }

  printf("lock %s: release update lock successful\n", expandLockName(Plk));
  ptLastResult = Sdm_True;
  return(TRUE);
}

static int subcmd_lock_releasesession(int argc, char **argv)
{
  if (argc != 1) {
    printf("The releasesession subcommand takes the form: lock releasesession lockname\n");
    ptLastResult = Sdm_False;
    return(TRUE);
  }

  SdmError err;
  PlkLock *Plk;
  SdmLockUtility *lk;
  SdmLockTypes locksHeld = 0;
  char *lockName = *argv++;
  Plk = lookupLock(lockName, TRUE);
  if (!Plk)
    return(TRUE);
  lk = Plk->plk_lock;
  assert(lk);

  (void) lk->DetermineLocks(locksHeld);

  if (!(locksHeld & Sdm_LKT_Session)) {
    printf("lock %s: no session lock in effect\n", expandLockName(Plk));
    ptLastResult = Sdm_False;
    return(TRUE);
  }

  if (lk->ReleaseLocks(err, Sdm_LKT_Session) != Sdm_EC_Success) {
    printf("lock %s: release session lock on lock object failed with error %s\n", expandLockName(Plk), detailedErrorReport(err));
    ptLastResult = Sdm_False;
    return(TRUE);
  }

  printf("lock %s: release session lock successful\n", expandLockName(Plk));
  ptLastResult = Sdm_True;
  return(TRUE);
}

static int subcmd_lock_status(int argc, char **argv)
{
  if (!argc) {
    printf("?The status subcommand takes one or more arguments which specify\n?the locks to report status on; to report status of all locks, use a single * character.\n");
    ptLastResult = Sdm_False;
    return(TRUE);
  }
  while (argc--) {
    char *currentLock = *argv++;
    if (strcmp(currentLock,"*")==0) {
      SdmLinkedListIterator lit(LOCK_LIST);
      PlkLock *Plk;

      while (Plk = (PlkLock *)(lit.NextListElement())) {
	statusLock(Plk, TRUE);
      }
    }
    else {
      PlkLock *Plk = lookupLock(currentLock, TRUE);
      if (Plk)
	statusLock(Plk, TRUE);
    }
  }
  ptLastResult = Sdm_True;
  return(TRUE);
}

// Called on quit - destroy all lock objects created

void DestroyAllLocks()
{
  SdmError err;
  SdmLinkedListIterator lit(LOCK_LIST);
  PlkLock *Plk;
  int onetime = 0;

  // First off, destroy all locks

  while (Plk = (PlkLock *)(lit.NextListElement())) {
    if (!onetime++)
      printf("Cleaning up existing lock objects...\n");
    deleteLock(Plk, TRUE);
  }

  // Now, force the connection to shutdown
  SdmLockUtility::ConnectionShutdown(err);
}

static int subcmd_lock_validate(int argc, char **argv)
{
  SdmError err;
  long startnum = 0, endnum = 0;
  SdmString validateString;
  SdmString testName;
  int fd;
  const char *s;

  if (argc < 1) {
    printf
      (
       "?The validate command takes the form: validate [ -s string | -F file | -S sum ]\n"
       "If a string is specified, then it is compared to the results of the last command\n"
       "-F - validate the results of the last command against the text in the specified file\n"
       "-S - validate the specified sum\n"
       );
    return(TRUE);
  }

  char *fileName = NULL;

  while (argc) {
    if (strcmp(*argv, "-F") == 0) {
      argc--;
      argv++;
      if (!argc) {
        printf("?-F must be followed by the name of the file to validate against\n");
        return(TRUE);
      }
      s = (const char*) validateString;
      if ((s && *s) || fileName) {
        printf("?Specify either a string or a single -F file, not both\n");
        return(TRUE);
      }
      fileName = *argv++;
      argc--;
      fd = SdmSystemUtility::SafeOpen(fileName, O_RDONLY);
      if (fd == -1) {
        int saveerrno = errno;
        printf("?Cant create file '%s': %s\n", fileName, strerror(errno));
        return(TRUE);
      }
      off_t fileSize = SdmSystemUtility::SafeLseek(fd, 0L, SEEK_END);
      if (fileSize) {
        char *buffer = (char *)malloc((unsigned)fileSize+1);
        assert(buffer);
        (void) SdmSystemUtility::SafeLseek(fd, 0L, SEEK_SET);
        if (SdmSystemUtility::SafeRead(fd, buffer, fileSize) != fileSize) {
          int saveerrno = errno;
          (void) SdmSystemUtility::SafeClose(fd);
          printf("?Could not read %s errno %d: %s\n", fileName, saveerrno, strerror(saveerrno));
          return(TRUE);
        }
        buffer[fileSize] = '\0';
        validateString = buffer;
        (void) free(buffer);
      }
      else {
        printf("?Can't validate against empty -F file: %s\n", fileName);
        return(TRUE);
      }
      continue;
    }
    else if (strcmp(*argv, "-d") == 0) {
      argc--;
      argv++;
      if (!argc) {
        printf("?-F must be followed by the name of the file to validate against\n");
        return(TRUE);
      }
      while (argc && *argv[0] != '-') {
        testName += *argv;
        testName += " ";
        argc--;
        argv++;
      }
    }
    else if (strcmp(*argv, "-s") == 0) {
      argc--;
      argv++;
      if (!argc) {
        printf("?-F must be followed by the name of the file to validate against\n");
        return(TRUE);
      }
      if (fileName) {
        (void) SdmSystemUtility::SafeClose(fd);
        printf("?Specify either a string or a single -F file, not both\n");
        return(TRUE);
      }
      while (argc && *argv[0] != '-') {
        validateString += *argv;
        argc--;
        argv++;
        if (argc && *argv[0] != '-') {
          validateString += " ";
        }
      }
    }
    else {
      printf("?Must specify an argument\n");
      return(FALSE);
    }
  }

  if (!(s = (const char*)testName) || !*s)
    testName = "Unnamed Test";

  ptLastResult.Validate(testName, validateString);

  return(TRUE);
}
