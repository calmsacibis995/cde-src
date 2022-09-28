#include <porttest.hh>
#include <ptcmdscan.hh>
#include <ptcsession.hh>
#include <ptcconnection.hh>

/*
** EXTERNALS
*/

/*
** GLOBALS
*/
SdmSession *theInputSessionObject = (SdmSession *)0;
SdmSession *theOutputSessionObject = (SdmSession *)0;

char *Hsession =
"Description: manipulation primary midend session\n"
"General usage: session command [arguments]\n"
"Specific usage:\n"
"\tsession create input|output\n"
"\tsession delete input|output\n"
"\tsession shutdown input|output\n"
"\tsession startup input|output\n"
"";

static int subcmd_session_create(int argc, char **argv);
static int subcmd_session_delete(int argc, char **argv);
static int subcmd_session_startup(int argc, char **argv);
static int subcmd_session_shutdown(int argc, char **argv);

static SUBCMD sessioncmdtab[] = {
  // cmdname,		handler,			argsrequired
  {"create",		subcmd_session_create,		0},	// create session
  {"delete",		subcmd_session_delete,		0},	// delete session
  {"shutdown",		subcmd_session_shutdown,	0},	// shutdown session
  {"startup",		subcmd_session_startup,		0},	// startup session
  {NULL,		0,				0}	// must be last entry in table
};

/*
** FORWARD DECLARATIONS
*/

/*************************************************************************
 *
 * Function:	Csession - 
 *
 * Input:	int pargc - number of arguments passed to command
 *		char **pargv - array of pargc arguments passed
 *
 * Output:	TRUE - help given
 *
 ************************************************************************/

int Csession(int argc, char **argv)
{
  return (argc == 1 ? TRUE : oneSubCommand(argv[0], argv[1], sessioncmdtab, argc-2, argv+2));
}

static void sessionAsyncCallback(SdmError& err, void* userdata, SdmServiceFunctionType type, 
  SdmEvent* event)
{
  assert (event != NULL);

  switch ((unsigned long)userdata) {
  case st_input:
    printf("\nsession: input session asynchronous callback invoked\n");
    break;
  case st_output:
    printf("\nsession: output session asynchronous callback invoked\n");
    break;
  default:
    printf("\nsession: unknown session asynchronous callback (%u) invoked\n", (unsigned long) userdata);
    break;
  }

  switch (event->sdmBase.type) {
  case Sdm_Event_RequestError:
    printf("event: Sdm_Event_RequestError\n");
    break;
  case Sdm_Event_GetMessageContents:
    printf("event: Sdm_Event_GetMessageContents\n");
    break;
  case Sdm_Event_GetMessageBodyContents:
    printf("event: Sdm_Event_GetMessageBodyContents\n");
    break;
  case Sdm_Event_GetMessageEnvelopeContents:
    printf("event: Sdm_Event_GetMessageEnvelopeContents\n");
    break;
  case Sdm_Event_SetMessageBodyContents:
    printf("event: Sdm_Event_SetMessageBodyContents\n");
    break;
  case Sdm_Event_OpenMessageStore:
    printf("event: Sdm_Event_OpenMessageStore\n");
    break;
  case Sdm_Event_SubmitMessage:
    printf("event: Sdm_Event_SubmitMessage\n");
    break;
  case Sdm_Event_ExpungeDeletedMessages:
    printf("event: Sdm_Event_ExpungeDeletedMessages\n");
    break;
  case Sdm_Event_CheckNewMail:
    printf("event: Sdm_Event_CheckNewMail: isStarted=%s numMessages=%d\n",
	   event->sdmCheckNewMail.isStart ? "YES" : "NO",
	   event->sdmCheckNewMail.numMessages);
    break;
  case Sdm_Event_AutoSave:
    printf("event: Sdm_Event_AutoSave\n");
    break;
  case Sdm_Event_ErrorLog:
    printf("event: Sdm_Event_ErrorLogL %s\n", (const char *)(*event->sdmErrorLog.errorMessage));
    break;
  case Sdm_Event_DebugLog:
    printf("event: Sdm_Event_DebugLog: %s\n", (const char *)(*event->sdmDebugLog.debugMessage));
    break;
  case Sdm_Event_DataPortBusy:
    printf("event: Sdm_Event_DataPortBusy\n");
    break;
  case Sdm_Event_GetLastInteractiveEventTime:
    printf("event: Sdm_Event_GetLastInteractiveEventTime\n");
    break;
  case Sdm_Event_SessionShutdown:
    printf("event: Sdm_Event_SessionShutdown\n");
    break;
  case Sdm_Event_GetPartialContents:
    printf("event: Sdm_Event_GetPartialContents\n");
    break;
  case Sdm_Event_DataPortLockActivity:
    printf("event: Sdm_Event_DataPortLockActivity: ");
    switch(event->sdmDataPortLockActivity.lockActivityEvent) {
    case SdmDataPortLockActivity::Sdm_DPLA_SessionLockGoingAway:
      printf("Sdm_DPLA_SessionLockGoingAway\n");
      break;
    case SdmDataPortLockActivity::Sdm_DPLA_SessionLockTakenAway:
      printf("Sdm_DPLA_SessionLockTakenAway\n");
      break;
    case SdmDataPortLockActivity::Sdm_DPLA_WaitingForSessionLock:
      printf("Sdm_DPLA_WaitingForSessionLock\n");
      break;
    case SdmDataPortLockActivity::Sdm_DPLA_NotWaitingForSessionLock:
      printf("Sdm_DPLA_NotWaitingForSessionLock\n");
      break;
    case SdmDataPortLockActivity::Sdm_DPLA_WaitingForUpdateLock:
      printf("Sdm_DPLA_WaitingForUpdateLock\n");
      break;
    case SdmDataPortLockActivity::Sdm_DPLA_NotWaitingForUpdateLock:
      printf("Sdm_DPLA_NotWaitingForUpdateLock\n");
      break;
    case SdmDataPortLockActivity::Sdm_DPLA_ServerConnectionBroken:
      printf("Sdm_DPLA_ServerConnectionBroken\n");
      break;
    case SdmDataPortLockActivity::Sdm_DPLA_MailboxConnectionReadOnly:
      printf("Sdm_DPLA_MailboxConnectionReadOnly\n");
      break;
    case SdmDataPortLockActivity::Sdm_DPLA_MailboxChangedByOtherUser:
      printf("Sdm_DPLA_MailboxChangedByOtherUser\n");
      break;
    default:
      printf("UNKNOWN SUBEVENT SPECIFIED %u\n", (unsigned long) event->sdmDataPortLockActivity.lockActivityEvent);
    }
    break;
  default:
    printf("Error: AsyncTestCallback default event received %d\n", event->sdmBase.type);
    break;      
  }
}

sessionType parseSessionType(char *type, char *&sessionName, SdmSession **&sesptr)
{
  assert(type);
  int typeLen = strlen(type);
  if (!typeLen)
    return(st_unknown);
  if (strncasecmp("input", type, typeLen)==0)
    {
      sessionName = "input";
      sesptr = &theInputSessionObject;
      return(st_input);
    }
  if (strncasecmp("output", type, typeLen)==0)
    {
      sessionName = "output";
      sesptr = &theOutputSessionObject;
      return(st_output);
    }
  printf("?Session type '%s' not recognized: must be 'input' or 'output'\n", type);
  return(st_unknown);
}

static int subcmd_session_create(int argc, char **argv)
{
  SdmError err;
  sessionType sestype;
  char *sessionName = 0;
  SdmSession **sesptr = 0;

  if (argc != 1) {
    printf("?The create command takes a single argument: input or output\n");
    return(TRUE);
  }

  if (!theConnectionObject) {
    printf("?A connection object has not yet been created\n?A connection object must be created before any session object can be created\n");
    return(TRUE);
  }

  if ((sestype = parseSessionType(*argv, sessionName, sesptr)) == st_unknown)
    return(TRUE);
  assert(sessionName);
  assert(sesptr);

  if (*sesptr) {
    printf("?A %s session already exists - only one allowed at a time\n", sessionName);
    return(TRUE);
  }

  if (theConnectionObject->SdmSessionFactory(err, *sesptr, sestype == st_input ? Sdm_ST_InSession : Sdm_ST_OutSession) != Sdm_EC_Success)
  if (err) {
    printf("session: creation failed with error %u: %s\n", (SdmErrorCode) err, err.ErrorMessage());
    return(TRUE);
  }

  printf("session: created %s session\n", sessionName);
  return(TRUE);
}

static int subcmd_session_delete(int argc, char **argv)
{
  SdmError err;
  sessionType sestype;
  char *sessionName = 0;
  SdmSession **sesptr = 0;

  if (argc != 1) {
    printf("?The delete command takes a single argument: input or output\n");
    return(TRUE);
  }

  if ((sestype = parseSessionType(*argv, sessionName, sesptr)) == st_unknown)
    return(TRUE);
  assert(sessionName);
  assert(sesptr);

  if (!*sesptr) {
    printf("?A %s session does not exist - you must create one first\n", sessionName);
    return(TRUE);
  }

  if ((st_input ? theInputFileDesc : theOutputFileDesc) != -1) {
    printf("?The %s session is not shutdown - it must be shutdown first\n", sessionName);
    return(TRUE);
  }

  delete *sesptr;
  *sesptr = (SdmSession *)0;
  printf("session: %s deleted\n", sessionName);

  return(TRUE);
}

static int subcmd_session_startup(int argc, char **argv)
{
  SdmError err;
  sessionType sestype;
  char *sessionName = 0;
  SdmSession **sesptr = 0;

  if (argc != 1) {
    printf("?The startup command takes a single argument: input or output\n");
    return(TRUE);
  }

  if ((sestype = parseSessionType(*argv, sessionName, sesptr)) == st_unknown)
    return(TRUE);
  assert(sessionName);
  assert(sesptr);

  if (!*sesptr) {
    printf("?A %s session does not exist - you must create one first\n", sessionName);
    return(TRUE);
  }

  (*sesptr)->StartUp(err);
  if (!err) {
    switch(sestype) {
    case st_input:
      if (SdmAttachSession(err, theInputFileDesc, theInputQueue, **sesptr) == Sdm_EC_Success) {
	assert(theInputFileDesc != -1);
	assert(theInputQueue);
	addPollFileDesc(theInputFileDesc, POLLIN);
      }
      break;
    case st_output:
      if (SdmAttachSession(err, theOutputFileDesc, theOutputQueue, **sesptr) == Sdm_EC_Success) {
	assert(theOutputFileDesc != -1);
	assert(theOutputQueue);
	addPollFileDesc(theOutputFileDesc, POLLIN);
      }
      break;
    default:
      assert(0);
    }
  }

  if (!err) {
    SdmServiceFunction svf(&sessionAsyncCallback, (void *) sestype, Sdm_SVF_Any);
    if ((*sesptr)->RegisterServiceFunction(err, svf) != Sdm_EC_Success) {
      printf("session: error: could not register SVF_Any service function with error %u: %s\n", (SdmErrorCode) err, err.ErrorMessage());
    }
  }
  printf("session: %s startup", sessionName);
  printf(err == Sdm_EC_Success ? " successful\n" : " failed with error %u: %s\n", (SdmErrorCode) err, err.ErrorMessage());

  return(TRUE);
}

static int subcmd_session_shutdown(int argc, char **argv)
{
  SdmError err;
  sessionType sestype;
  char *sessionName = 0;
  SdmSession **sesptr = 0;

  if (argc != 1) {
    printf("?The startup command takes a single argument: input or output\n");
    return(TRUE);
  }

  if ((sestype = parseSessionType(*argv, sessionName, sesptr)) == st_unknown)
    return(TRUE);
  assert(sessionName);
  assert(sesptr);

  if (!*sesptr) {
    printf("?A %s session does not exist - you must create one first\n", sessionName);
    return(TRUE);
  }

  if ((st_input ? theInputFileDesc : theOutputFileDesc) == -1) {
    printf("?The %s session has not been started - cannot shutdown\n", sessionName);
    return(TRUE);
  }

  SdmServiceFunction svf(&sessionAsyncCallback, (void *) sestype, Sdm_SVF_Any);
  if ((*sesptr)->UnregisterServiceFunction(err, svf) != Sdm_EC_Success) {
    printf("session: error: could not unregister SVF_Any service function with error %u: %s\n", (SdmErrorCode) err, err.ErrorMessage());
    err = Sdm_EC_Success;
  }

  (*sesptr)->ShutDown(err);
  if (!err) {
    switch(sestype) {
    case st_input:
      assert(theInputFileDesc != -1);
      assert(theInputQueue);
      removePollFileDesc(theInputFileDesc);
      close(theInputFileDesc);
      theInputFileDesc = -1;
      theInputQueue = 0;
      break;
    case st_output:
      assert(theOutputFileDesc != -1);
      assert(theOutputQueue);
      removePollFileDesc(theOutputFileDesc);
      close(theOutputFileDesc);
      theOutputFileDesc = -1;
      theOutputQueue = 0;
      break;
    default:
      assert(0);
    }
  }

  printf("session: %s shutdown", sessionName);
  printf(err == Sdm_EC_Success ? " successful\n" : " failed with error %u: %s\n", (SdmErrorCode) err, err.ErrorMessage());

  return(TRUE);
}
