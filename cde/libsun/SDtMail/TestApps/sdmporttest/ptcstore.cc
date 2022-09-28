#include <SDtMail/Sdtmail.hh>
#include <SDtMail/LinkedList.hh>
#include <PortObjs/DataPort.hh>
#include <SDtMail/MessageStore.hh>
#include <porttest.hh>
#include <ptcmdscan.hh>
#include <ptcport.hh>
#include <ptctoken.hh>
#include <ptcstore.hh>
#include <ptcsession.hh>

/*
** EXTERNALS
*/

extern SdmLinkedList *TOKEN_LIST;

/*
** GLOBALS
*/
char *Hstore =
"Description: manipulate message store objects\n"
"General usage: store command [ store [ arguments ]]\n"
"Specific usage:\n"
"\tstore addtosubscribednamespace store name\n"
"\tstore attach store token\n"
"\tstore cancelpendingoperations [store(s) | *]\n"
"\tstore checkfornewmessages [store(s) | *]\n"
"\tstore clearflags store messagenum flags\n"
"\tstore create input|output name\n"
"\tstore delete [store(s) | *]\n"
"\tstore disconnect [store(s) | *]\n"
"\tstore expungedeleted [+a]\n"
"\tstore getflags store [messagenum]\n"
"\tstore getheaders store [messagenum [header(s)]]\n"
"\tstore getheaders store [messagenum [ -a|-A [abstractheader(s)]]]\n"
"\tstore getstatus [store(s) | *]\n"
"\tstore open [+a] store token\n"
"\tstore reconnect [store(s) | *]\n"
"\tstore removefromsubscribednamespace store name\n"
"\tstore scannamespace store reference pattern\n"
"\tstore scansubscribednamespace store reference pattern\n"
"\tstore setflags store messagenum flags\n"
"\tstore shutdown [store(s) | *]\n"
"\tstore startup [store(s) | *]\n"
"Notes:\n"
"messagenum specifies the message(s) to work on; it takes one of 4 forms:\n"
"1. a single number specifying the message to work on.\n"
"2. a range of the form first:last specifying an inclusive range to work on.\n"
"3. a list of the form a,b,...,x specifying specific message to work on.\n"
"4. the single character '*' which means all messages in the store.\n"
"store create creates a 'store' object, not a message store;\n"
"store storecreate creates a message store on a store.\n"
"store delete deletes a 'store' object, not a message store;\n"
"store storedelete deletes a message store on a store.\n"
"+a means issue an Asynchronous Request for the command; otherwise,\n"
"the command is executed synchronously.\n"
"";

static int subcmd_store_addtosubscribednamespace(int argc, char **argv);
static int subcmd_store_attach(int argc, char **argv);
static int subcmd_store_cancelpendingoperations(int argc, char **argv);
static int subcmd_store_checkfornewmessages(int argc, char **argv);
static int subcmd_store_clearflags(int argc, char **argv);
static int subcmd_store_close(int argc, char **argv);
static int subcmd_store_create(int argc, char **argv);
static int subcmd_store_delete(int argc, char **argv);
static int subcmd_store_disconnect(int argc, char **argv);
static int subcmd_store_expungedeleted(int argc, char **argv);
static int subcmd_store_getflags(int argc, char **argv);
static int subcmd_store_getheaders(int argc, char **argv);
static int subcmd_store_getstatus(int argc, char **argv);
static int subcmd_store_open(int argc, char **argv);
static int subcmd_store_reconnect(int argc, char **argv);
static int subcmd_store_removefromsubscribednamespace(int argc, char **argv);
static int subcmd_store_setflags(int argc, char **argv);
static int subcmd_store_scannamespace(int argc, char **argv);
static int subcmd_store_scansubscribednamespace(int argc, char **argv);
static int subcmd_store_startup(int argc, char **argv);
static int subcmd_store_shutdown(int argc, char **argv);

static SUBCMD storecmdtab[] = {
  // cmdname,		handler,			argsrequired
  {"addtosubscribednamespace",	subcmd_store_addtosubscribednamespace,		0},	// add subscribed message store name
  {"attach",		subcmd_store_attach,		0},	// attach to a store server
  {"cancelpendingoperations",	subcmd_store_cancelpendingoperations, 	0},	// cancel any pending operations on the store
  {"checkfornewmessages",	subcmd_store_checkfornewmessages, 	0},	// check for new messages on the store
  {"clearflags",	subcmd_store_clearflags, 	0},	// clear flags on a message
  {"close",		subcmd_store_close,		0},	// close an open store
  {"create",		subcmd_store_create,		0},	// create a new store
  {"delete",		subcmd_store_delete,		0},	// delete an existing store
  {"disconnect",	subcmd_store_disconnect,	0},	// disconnect a store from its server
  {"expungedeleted",	subcmd_store_expungedeleted,	0},	// expunge deleted messages
  {"getflags",		subcmd_store_getflags,		0},	// get flags from store
  {"getheaders",	subcmd_store_getheaders,	0},	// get headers from store
  {"getstatus",		subcmd_store_getstatus,		0},	// get status from store
  {"open",		subcmd_store_open,		0},	// open message store
  {"reconnect",		subcmd_store_reconnect,		0},	// reconnected a store to its server
  {"removefromsubscribednamespace",	subcmd_store_removefromsubscribednamespace,		0},	// add subscribed message store name
  {"scannamespace",	subcmd_store_scannamespace,	0},	// scan namespace
  {"scansubscribednamespace",	subcmd_store_scansubscribednamespace,	0},	// scan the subscribed namespace
  {"setflags",		subcmd_store_setflags,		0},	// set flags on a message
  {"startup",		subcmd_store_startup,		0},	// startup store
  {"shutdown",		subcmd_store_shutdown,		0},	// shutdown store
  {NULL,		0,				0}	// must be last entry in table
};

SdmLinkedList *STORE_LIST = 0;

/*
** FORWARD DECLARATIONS
*/
static void closeStore(PmstMessageStore *pmst, int printMessage);


/*************************************************************************
 *
 * Function:	Cstore - 
 *
 * Input:	int pargc - number of arguments passed to command
 *		char **pargv - array of pargc arguments passed
 *
 * Output:	TRUE - help given
 *
 ************************************************************************/

int Cstore(int argc, char **argv)
{
  return (argc == 1 ? TRUE : oneSubCommand(argv[0], argv[1], storecmdtab, argc-2, argv+2));
}

static PmstMessageStore *validstoreName(char *storeName)
{
  SdmLinkedListIterator lit(STORE_LIST);
  PmstMessageStore *pmst;

  while (pmst = (PmstMessageStore *)(lit.NextListElement())) {
    assert(pmst->pmst_storename);
    if (strcasecmp((const char *)*pmst->pmst_storename, storeName)==0)
      return(pmst);
  }
  return((PmstMessageStore *)0);
}

static char lastErrorMessage[2048];

static const char* detailedErrorReport(SdmError& err)
{
  char msgbuf[1024];

  lastErrorMessage[0] = '\0';
  sprintf(msgbuf, "%u: %s\n", (SdmErrorCode) err, err.ErrorMessage());
  strcat(lastErrorMessage, msgbuf);
  int i = err.MinorCount();
  while (i--) {
    sprintf(msgbuf, "  minor code %u: %s\n", (SdmErrorCode) err[i], err.ErrorMessage(i));
    strcat(lastErrorMessage, msgbuf);
  }
  assert(strlen(lastErrorMessage) < sizeof(lastErrorMessage));
  assert(strlen(lastErrorMessage));
  return (lastErrorMessage);
}

const char *expandStoreName(PmstMessageStore *pmst)
{
  assert(pmst->pmst_storename);
  return ((const char *)*pmst->pmst_storename);
}

PmstMessageStore *lookupStore(char *storeName, int printMessage)
{
  PmstMessageStore *ptk;

  assert(storeName);

  // If the store list is empty, nothing to do here
  if (!STORE_LIST || !STORE_LIST->ElementCount()) {
    if (printMessage)
      printf("?No message stores defined\n");
    return((PmstMessageStore *)0);
  }

  // lookup the store by name - if the name is registered, return that store
  ptk = validstoreName(storeName);
  if (ptk)
    return(ptk);

  // store not registered - say so and return nothing
  if (printMessage)
    printf("?message store %s does not exist\n", storeName);
  return((PmstMessageStore *)0);
}

static void storeAsyncCallback(SdmError& err, void* userdata, SdmServiceFunctionType type, 
  SdmEvent* event)
{
  assert (event != NULL);
  PmstMessageStore* pmst = (PmstMessageStore*) userdata;
  assert(pmst);
  SdmMessageStore *mst = pmst->pmst_messagestore;
  assert(mst);
  printf("\nmessage store %s: store asynchronous callback: ", expandStoreName(pmst));

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
  case Sdm_Event_NotifyLog:
    printf("event: Sdm_Event_NotifyLog: %s\n", (const char *)(*event->sdmNotifyLog.notifyMessage));
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
      closeStore(pmst, TRUE);
      break;
    case SdmDataPortLockActivity::Sdm_DPLA_SessionLockTakenAway:
      printf("Sdm_DPLA_SessionLockTakenAway\n");
      closeStore(pmst, TRUE);
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

static void startupStore(PmstMessageStore *pmst, int printMessage)
{
  SdmError err;

  assert(pmst);
  SdmMessageStore *mst = pmst->pmst_messagestore;
  assert(mst);
  mst->StartUp(err);
  if (!err) {
    SdmServiceFunction svf(&storeAsyncCallback, (void *) pmst, Sdm_SVF_Any);
    if (mst->RegisterServiceFunction(err, svf) != Sdm_EC_Success) {
      printf("message store %s: error: could not register SVF_Any service function with error %s\n", expandStoreName(pmst), detailedErrorReport(err));
    }
  }
  printf("message store %s: started", expandStoreName(pmst));
  printf(err == Sdm_EC_Success ? "\n" : " with error %s\n", detailedErrorReport(err));
}

static void shutdownStore(PmstMessageStore *pmst, int printMessage)
{
  SdmError err;

  assert(pmst);
  SdmMessageStore *mst = pmst->pmst_messagestore;
  assert(mst);
  mst->ShutDown(err);
  printf("message store %s: shutdown", expandStoreName(pmst));
  printf(err == Sdm_EC_Success ? "\n" : " with error %s\n", detailedErrorReport(err));
}

static void checkfornewmessagesStore(PmstMessageStore *pmst, int printMessage)
{
  SdmError err;
  SdmMessageNumber nummessages = 0;

  assert(pmst);
  SdmMessageStore *mst = pmst->pmst_messagestore;
  assert(mst);
  if (mst->CheckForNewMessages(err, nummessages) != Sdm_EC_Success) {
    printf("message store %s: check for new messages failed with error %s\n", expandStoreName(pmst), detailedErrorReport(err));
  }
  else {
    printf("message store %s: check for new messages successful:", expandStoreName(pmst));
    if (nummessages)
      printf(" no new messages\n");
    else
      printf(" %d new messages arrived\n", nummessages);
  }
}

static void cancelStore(PmstMessageStore *pmst, int printMessage)
{
  SdmError err;

  assert(pmst);
  SdmMessageStore *mst = pmst->pmst_messagestore;
  assert(mst);
  mst->CancelPendingOperations(err);
  printf("message store %s: pending operations cancelled", expandStoreName(pmst));
  printf(err == Sdm_EC_Success ? "\n" : " with error %s\n", detailedErrorReport(err));
}

static void deleteStore(PmstMessageStore *pmst, int printMessage)
{
  SdmError err;

  assert(pmst);
  SdmMessageStore *mst = pmst->pmst_messagestore;
  assert(mst);
  STORE_LIST->RemoveElementFromList(pmst);
  if (mst->Close(err) != Sdm_EC_Success)
    printf("message store %s: warning: close returned error %s\n", expandStoreName(pmst), detailedErrorReport(err));
  if (mst->ShutDown(err) != Sdm_EC_Success)
    printf("message store %s: warning: shutdown returned error %s\n", expandStoreName(pmst), detailedErrorReport(err));
  printf("message store %s: deleted\n", expandStoreName(pmst));
  delete pmst;
}

static void disconnectStore(PmstMessageStore *pmst, int printMessage)
{
  SdmError err;

  assert(pmst);
  SdmMessageStore *mst = pmst->pmst_messagestore;
  assert(mst);
  mst->Disconnect(err);
  err = 1;
  printf("message store %s: disconnected", expandStoreName(pmst));
  printf(err == Sdm_EC_Success ? "\n" : " with error %s\n", detailedErrorReport(err));
}

static void reconnectStore(PmstMessageStore *pmst, int printMessage)
{
  SdmError err;

  assert(pmst);
  SdmMessageStore *mst = pmst->pmst_messagestore;
  assert(mst);
  mst->Reconnect(err);
  err = 1;
  printf("message store %s: reconnected", expandStoreName(pmst));
  printf(err == Sdm_EC_Success ? "\n" : " with error %s\n", detailedErrorReport(err));
}

static int decodeMessageRange(SdmMessageNumberL &msglist, SdmMessageNumber &startnum, SdmMessageNumber &endnum, char *messageSequence, PmstMessageStore *pmst, int printMessage)
{
  SdmError err;

  assert(*messageSequence);
  assert(pmst);

  char *p = 0;
  startnum = 0;
  endnum = 0;

  // Determine message store to operate on
  SdmMessageStore *mst = pmst->pmst_messagestore;
  assert(mst);

  if (::strcmp(messageSequence, "*")==0) {
    SdmMessageStoreStatus statbuf;
    startnum = 1;
    if (mst->GetStatus(err, statbuf, Sdm_MSS_Messages) != Sdm_EC_Success) {
      if (printMessage)
	printf("message store %s: get message store status failed with error %s\n", expandStoreName(pmst), detailedErrorReport(err));
      return(FALSE);
    }
    endnum = statbuf.messages;
    if (!endnum) {
      if (printMessage)
	printf("message store %s: no messages in store to operate on\n", expandStoreName(pmst));
      return(FALSE);
    }
    return(TRUE);
  }

  if (strchr(messageSequence, (int)',')) {
    // Have a list - deal with that
    char *msp = messageSequence;
    long nextNum = 0;
    msglist.ClearAllElements();
    while (nextNum = strtol(msp, &p, 10)) {
      if (*p && *p != ',') {
	if (printMessage)
	  printf("?Message list has invalid character '%c': %s\n", *p, p);
	return(FALSE);
      }
      msglist.AddElementToList(nextNum);
      if (!*p)
	return(TRUE);
      msp = ++p;
    }
    return(TRUE);
  }

  // Not a range - handle single number of a:b range

  startnum = strtol(messageSequence, &p, 10);
  if (p == messageSequence) {
    if (printMessage)
      printf("?Message sequence does not begin with valid message number: %s\n", messageSequence);
    return(FALSE);
  }
  if (startnum < 1) {
    if (printMessage)
      printf("?First message in sequence must be >= 1: %d\n", startnum);
    return(FALSE);
  }

  if (!*p)
    return(TRUE);

  if (*p != ':') {
    if (printMessage)
      printf("?Message sequence ends with invalid character '%c': %s\n", *p, p);
    return(FALSE);
  }

  endnum = strtol(p+1, &p, 10);
  if (*p) {
    if (printMessage)
      printf("?Message sequence ends with invalid character '%c': %s\n", *p, p);
    return(FALSE);
  }

  if (endnum <= startnum) {
    if (printMessage)
      printf("?End message number does not follow first: %d thru %d makes no sense\n", startnum, endnum);
    return(FALSE);
  }

  return(TRUE);
}

static void statusStore(PmstMessageStore *pmst, int printMessage)
{
  SdmError err;
  SdmMessageStoreStatus statbuf;

  assert(pmst);
  SdmMessageStore *mst = pmst->pmst_messagestore;
  assert(mst);
  if (mst->GetStatus(err, statbuf, Sdm_MSS_ALL) != Sdm_EC_Success) {
    printf("message store %s: get message store status failed with error %s\n", expandStoreName(pmst), detailedErrorReport(err));
    return;
  }
  printf("message store %s: status information:\n"
	 " flags = 0x%08lx, uidvalidity = 0x%08lx, checksum = %u, checksumbytes = %u\n"
	 " messages = %d, recent = %d, unseen = %d, uidnext = %d\n",
	 expandStoreName(pmst), 
	 statbuf.flags, statbuf.uidvalidity, statbuf.checksum, statbuf.checksum_bytes,
	 statbuf.messages, statbuf.recent, statbuf.unseen, statbuf.uidnext);
  return;
}

static void closeStore(PmstMessageStore *pmst, int printMessage)
{
  SdmError err;

  assert(pmst);
  SdmMessageStore *mst = pmst->pmst_messagestore;
  assert(mst);
  mst->Close(err);
  printf("message store %s: closed", expandStoreName(pmst));
  printf(err == Sdm_EC_Success ? "\n" : " with error %s\n", detailedErrorReport(err));
}

static char *printNamespaceFlag(SdmNamespaceFlag nsflag)
{
static char buf[128];

  sprintf(buf, "%-12s %-11s %-8s %-10s",
	  nsflag & Sdm_NSA_noInferiors ? "noinferiors" : "",
	  nsflag & Sdm_NSA_cannotOpen ? "cannotOpen" : "",
	  nsflag & Sdm_NSA_changed ? "changed" : "",
	  nsflag & Sdm_NSA_unChanged ? "unchanged" : "");
  return(buf);
}

static int storeSetFlags(int argc, char **argv, SdmBoolean value)
{
  SdmError err;
  SdmMessageFlagAbstractFlags flags = 0;
  SdmMessageNumberL messageList;

  if (argc < 3) {
    printf("?The %s command takes the form: cmd store messagenum flags\n",
	   value ? "setflags" : "clearflags");
    return(TRUE);
  }

  // Extract the message store name
  PmstMessageStore *pmst = lookupStore(*argv, TRUE);
  if (!pmst)
    return(TRUE);
  argc--;
  argv++;

  // Determine message store to operate on
  SdmMessageStore *mst = pmst->pmst_messagestore;
  assert(mst);

  // If no message number then for all messages return all flags
  long startnum = 0, endnum = 0;

  char *messageSequence = *argv++;
  argc--;
  if (!decodeMessageRange(messageList, startnum, endnum, messageSequence, pmst, TRUE))
    return(TRUE);

  // Collect the flags
  assert(argc);
  while (argc--) {
    SdmMessageFlagAbstractFlags x = 0;
    if (!decodeFlagAbstractValue(x, *argv++, TRUE))
      return(TRUE);
    flags |= x;
  }

  int messageListLength = messageList.ElementCount();
  if (!endnum && !messageListLength) {
    if (value)
      mst->SetFlags(err, flags, startnum);
    else
      mst->ClearFlags(err, flags, startnum);
  }
  else if (endnum && !messageListLength) {
    if (value)
      mst->SetFlags(err, flags, startnum, endnum);
    else
      mst->ClearFlags(err, flags, startnum, endnum);
  }
  else {
    if (value)
      mst->SetFlags(err, flags, messageList);
    else
      mst->ClearFlags(err, flags, messageList);
  }

  if (err != Sdm_EC_Success) {
    printf("message store %s: %s failed with error %s\n", expandStoreName(pmst),
	   value ? "setflags" : "clearflags", detailedErrorReport(err));
  
    return(TRUE);
  }

  printf("message store %s: %s successful\n", expandStoreName(pmst),
	 value ? "setflags" : "clearflags");
  return(TRUE);
}

static void expungeDeletedAsyncCallback(SdmError& err, void* userdata, SdmServiceFunctionType type, 
  SdmEvent* event)
{
  assert (event != NULL);

  printf("\n--> expungeDeletedAsyncCallback invoked\n");
  if (event->sdmBase.type != Sdm_Event_ExpungeDeletedMessages) {
    printf("unknown event type: userdata = %08lx event type %d\n", userdata, event->sdmBase.type);
    return;
  }

  SdmMessageNumberL *msgnums =  event->sdmExpungeDeletedMessages.deletedMessages;
  assert(msgnums);
  SdmError* inerr = event->sdmExpungeDeletedMessages.error;

  PmstMessageStore *pmst = (PmstMessageStore *)userdata;
  assert(pmst);

  if (*inerr != Sdm_EC_Success)
    printf("message store %s: expungedeleted failed with error %s\n", expandStoreName(pmst), (SdmErrorCode) *inerr, inerr->ErrorMessage());
  else {
    int i = msgnums->ElementCount();
    printf("message store %s: expungedeleted successful; %d messages deleted: ", expandStoreName(pmst), i);
    for (int j = 0; j < i; j++)
      printf("%d%s", (SdmMessageNumber)(*msgnums)[j], j == i-1 ? "" : ", ");
    printf("\n");
  }
}

static void openAsyncCallback(SdmError& err, void* userdata, SdmServiceFunctionType type, 
  SdmEvent* event)
{
  assert (event != NULL);

  printf("\n--> openAsyncCallback invoked\n");
  if (event->sdmBase.type != Sdm_Event_OpenMessageStore) {
    printf("unknown event type: userdata = %08lx event type %d\n", userdata, event->sdmBase.type);
    return;
  }

  SdmBoolean readOnly = event->sdmOpenMessageStore.readOnly;
  SdmError* inerr = event->sdmOpenMessageStore.error;

  PmstMessageStore *pmst = (PmstMessageStore *)userdata;
  assert(pmst);

  if (*inerr != Sdm_EC_Success)
    printf("message store %s: async open failed with error %s\n", expandStoreName(pmst), detailedErrorReport(*inerr));
  else {
    printf("message store %s: async open successful%s\n", expandStoreName(pmst), readOnly ? " IN READ ONLY MODE!" : "");
  }
}

static int subcmd_store_addtosubscribednamespace(int argc, char **argv)
{
  SdmError err;
  SdmIntStrL namesL;

  if (argc != 2) {
    printf("?The addtosubscribednamespace command takes the form: addtosubscribednamespace store name\n");
    return(TRUE);
  }

  // Extract the store name
  PmstMessageStore *pmst = lookupStore(*argv, TRUE);
  if (!pmst)
    return(TRUE);
  argc--;
  argv++;

  // Determine message store to operate on
  SdmMessageStore *mst = pmst->pmst_messagestore;
  assert(mst);

  // Get the new name
  SdmString newname(*argv++); 

  // Issue the add call
  if (mst->AddToSubscribedNamespace(err, newname) != Sdm_EC_Success) {
    printf("message store %s: addtosubscribednamespace failed with error %s\n", expandStoreName(pmst), detailedErrorReport(err));
    return(TRUE);
  }

  printf("message store %s: addtosubscribednamespace successful\n", expandStoreName(pmst));
  return(TRUE);
}

static int subcmd_store_attach(int argc, char **argv)
{
  SdmError err;

  if (argc != 2) {
    printf("?The attach command takes a store object and a\ntoken argument specifying the object\nto attach to.\n");
    return(TRUE);
  }

  char *storeName = *argv++;
  char *tokenName = *argv;

  PmstMessageStore *pmst = lookupStore(storeName, TRUE);
  if (!pmst)
    return(TRUE);
  SdmMessageStore *mst = pmst->pmst_messagestore;
  assert(mst);

  PtkToken *ptk = lookupToken(tokenName, TRUE);
  if (!ptk)
    return(TRUE);
  SdmToken *tk = ptk->ptk_token;
  assert(tk);

  mst->Attach(err, *tk);
  if (err != Sdm_EC_Success)
    printf("message store %s: attach failed with error %s\n", expandStoreName(pmst), detailedErrorReport(err));
  else
    printf("message store %s: attached\n", expandStoreName(pmst));
  return(TRUE);
}

static int subcmd_store_cancelpendingoperations(int argc, char **argv)
{
  if (!argc) {
    printf("?The cancelpendingoperations subcommand takes one or more arguments which specify\n?the stores to have operations cancelled on; to cancel operations on all stores, use a single * character.\n");
    return(TRUE);
  }
  while (argc--) {
    char *currentStore = *argv++;
    if (strcmp(currentStore,"*")==0) {
      SdmLinkedListIterator lit(STORE_LIST);
      PmstMessageStore *pmst;

      while (pmst = (PmstMessageStore *)(lit.NextListElement())) {
	cancelStore(pmst, TRUE);
      }
    }
    else {
      PmstMessageStore *pmst = lookupStore(currentStore, TRUE);
      if (pmst)
	cancelStore(pmst, TRUE);
    }
  }
  return(TRUE);
}

static int subcmd_store_checkfornewmessages(int argc, char **argv)
{
  if (!argc) {
    printf("?The checkfornewmessages subcommand takes one or more arguments which specify\n?the stores to be checked; to check all stores, use a single * character.\n");
    return(TRUE);
  }
  while (argc--) {
    char *currentStore = *argv++;
    if (strcmp(currentStore,"*")==0) {
      SdmLinkedListIterator lit(STORE_LIST);
      PmstMessageStore *pmst;

      while (pmst = (PmstMessageStore *)(lit.NextListElement())) {
	checkfornewmessagesStore(pmst, TRUE);
      }
    }
    else {
      PmstMessageStore *pmst = lookupStore(currentStore, TRUE);
      if (pmst)
	checkfornewmessagesStore(pmst, TRUE);
    }
  }
  return(TRUE);
}

static int subcmd_store_clearflags(int argc, char **argv)
{
  return storeSetFlags(argc, argv, Sdm_False);
}

static int subcmd_store_close(int argc, char **argv)
{
  if (!argc) {
    printf("?The close subcommand takes one or more arguments which specify\n?the stores to be closed; to close all stores, use a single * character.\n");
    return(TRUE);
  }
  while (argc--) {
    char *currentStore = *argv++;
    if (strcmp(currentStore,"*")==0) {
      SdmLinkedListIterator lit(STORE_LIST);
      PmstMessageStore *pmst;

      while (pmst = (PmstMessageStore *)(lit.NextListElement())) {
	closeStore(pmst, TRUE);
      }
    }
    else {
      PmstMessageStore *pmst = lookupStore(currentStore, TRUE);
      if (pmst)
	closeStore(pmst, TRUE);
    }
  }
  return(TRUE);
}

static int subcmd_store_create(int argc, char **argv)
{
  SdmError err;
  sessionType sestype;
  char *sessionName = 0;
  SdmSession **sesptr = 0;

  if (argc != 2) {
    printf("?The create subcommand requires 2 arguments: store create input|output name\n");
    return(TRUE);
  }

  char *iospec = *argv++;
  char *storeName = *argv++;

  // Parse input/output spec and retrieve session info

  if ((sestype = parseSessionType(iospec, sessionName, sesptr)) == st_unknown)
    return(TRUE);
  assert(sessionName);
  assert(sesptr);

  if (!*sesptr) {
    printf("?A %s session does not exist - you must create one first before creating a message store\n", sessionName);
    return(TRUE);
  }

  // See if the store name already exists

  if (lookupStore(storeName, FALSE) != (PmstMessageStore *)0) {
    printf("?message store %s already exists\n", *argv);
    return(TRUE);
  }

  // Name is unique - create store with it
  SdmMessageStore *mst;
  if ((*sesptr)->SdmMessageStoreFactory(err, mst) != Sdm_EC_Success) {
    printf("message store %s: failed to create with error %s\n", storeName, detailedErrorReport(err));
    return(TRUE);
  }
  assert(mst);
  SdmString *name = new SdmString(storeName);
  assert(name);
  PmstMessageStore *pmst = new PmstMessageStore();
  assert(pmst);
  if (!STORE_LIST)
    STORE_LIST = new SdmLinkedList();
  pmst->pmst_messagestore = mst;
  pmst->pmst_storename = name;
  STORE_LIST->AddElementToList(pmst);
  printf("message store %s: defined\n", expandStoreName(pmst));
  return(TRUE);
}

static int subcmd_store_delete(int argc, char **argv)
{
  if (!argc) {
    printf("?The delete subcommand takes one or more arguments which specify\n?the stores to be deleted; to delete all stores, use a single * character.\n");
    return(TRUE);
  }
  while (argc--) {
    char *currentStore = *argv++;
    if (strcmp(currentStore,"*")==0) {
      SdmLinkedListIterator lit(STORE_LIST);
      PmstMessageStore *pmst;

      while (pmst = (PmstMessageStore *)(lit.NextListElement())) {
	deleteStore(pmst, TRUE);
      }
    }
    else {
      PmstMessageStore *pmst = lookupStore(currentStore, TRUE);
      if (pmst)
	deleteStore(pmst, TRUE);
    }
  }
  return(TRUE);
}

static int subcmd_store_disconnect(int argc, char **argv)
{
  if (!argc) {
    printf("?The disconnect subcommand takes one or more arguments which specify\n?the stores to be disconnected; to disconnect all stores, use a single * character.\n");
    return(TRUE);
  }
  while (argc--) {
    char *currentStore = *argv++;
    if (strcmp(currentStore,"*")==0) {
      SdmLinkedListIterator lit(STORE_LIST);
      PmstMessageStore *pmst;

      while (pmst = (PmstMessageStore *)(lit.NextListElement())) {
	disconnectStore(pmst, TRUE);
      }
    }
    else {
      PmstMessageStore *pmst = lookupStore(currentStore, TRUE);
      if (pmst)
	disconnectStore(pmst, TRUE);
    }
  }
  return(TRUE);
}

static int subcmd_store_expungedeleted(int argc, char **argv)
{
  SdmError err;
  SdmMessageNumberL msgnums;
  int asyncFlag = 0;

  if (argc < 1 || argc > 2) {
    printf("?The expungedeleted command takes the form: expungedeleted store [+a]\n");
    return(TRUE);
  }

  char *storeName = *argv++; argc--;

  // Extract +A if present
  if (argc && strcmp(*argv, "+a")==0) {
    asyncFlag++;
    argc--;
    argv++;
  }

  PmstMessageStore *pmst = lookupStore(storeName, TRUE);
  if (!pmst)
    return(TRUE);
  SdmMessageStore *mst = pmst->pmst_messagestore;
  assert(mst);

  if (asyncFlag) {
    SdmServiceFunction svf(&expungeDeletedAsyncCallback, (void *)2);
    if (mst->ExpungeDeletedMessages_Async(err, svf, pmst) != Sdm_EC_Success) {
      printf("message store %s: expungedeleted async failed with error %s\n", expandStoreName(pmst), detailedErrorReport(err));
    }
    else {
      printf("message store %s: expungedeleted async successful: pending\n", expandStoreName(pmst));
    }
    return(TRUE);
  }

  mst->ExpungeDeletedMessages(err, msgnums);
  if (err != Sdm_EC_Success)
    printf("message store %s: expungedeleted failed with error %s\n", expandStoreName(pmst), detailedErrorReport(err));
  else {
    int i = msgnums.ElementCount();
    printf("message store %s: expungedeleted successful; %d messages deleted: ", expandStoreName(pmst), i);
    for (int j = 0; j < i; j++)
      printf("%d%s", (SdmMessageNumber)msgnums[j], j == i-1 ? "" : ", ");
    printf("\n");
  }
  return(TRUE);
}

static int subcmd_store_getflags(int argc, char **argv)
{
  SdmError err;
  SdmMessageFlagAbstractFlags flags;
  SdmMessageFlagAbstractFlagsL flagsL;
  SdmMessageNumberL messageList;

  if (argc < 1 || argc > 2) {
    printf("?The getflags command takes the form: getflags [messagenum]\n");
    return(TRUE);
  }

  // Extract the store number
  PmstMessageStore *pmst = lookupStore(*argv, TRUE);
  if (!pmst)
    return(TRUE);
  argc--;
  argv++;

  // Determine message store to operate on
  SdmMessageStore *mst = pmst->pmst_messagestore;
  assert(mst);

  // If no message number then for all messages return all flags
  long startnum = 0, endnum = 0;

  if (!argc) {
    if (!decodeMessageRange(messageList, startnum, endnum, "*", pmst, TRUE))
      return(TRUE);
  }
  else {
    char *messageSequence = *argv++;
    argc--;
    if (!decodeMessageRange(messageList, startnum, endnum, messageSequence, pmst, TRUE))
      return(TRUE);
  }

  if (argc) {
    printf("?Bable at end of command: %s\n", *argv);
    return(TRUE);
  }

  int messageListLength = messageList.ElementCount();
  if (!endnum && !messageListLength)
    mst->GetFlags(err, flags, startnum);
  else if (endnum && !messageListLength)
    mst->GetFlags(err, flagsL, startnum, endnum);
  else
    mst->GetFlags(err, flagsL, messageList);

  if (err != Sdm_EC_Success) {
    printf("message store %s: getflags failed with error %s\n", expandStoreName(pmst), detailedErrorReport(err));
    return(TRUE);
  }

  printf("message store %s: getflags successful\n", expandStoreName(pmst));

  if (!endnum && !messageListLength) {
    printf(" message %d: ", startnum);
    printFlags(flags);
  }
  else if (endnum && !messageListLength) {
    int numMessages = (endnum-startnum)+1;
    for (int k = 0; k < numMessages; k++) {
      printf(" message %d: ", k+startnum);
      printFlags(flagsL[k]);
    }
  } else {
    int numMessages = messageListLength;
    for (int k = 0; k < numMessages; k++) {
      printf(" message %d: ", messageList[k]);
      printFlags(flagsL[k]);
    }
  }

  return(TRUE);
}

static int subcmd_store_getheaders(int argc, char **argv)
{
  SdmString header;		// holds returned header list
  SdmStringL headerList;	// holds returned header list of lists
  SdmBoolean headerListSet = Sdm_False;	// indicates if headerList is set
  SdmBoolean headerSet = Sdm_False;	// indicates if header is set
  SdmMessageHeaderAbstractFlags headerAbstractFlags = 0;
  SdmMessageNumberL messageList;
  int abstractFlag = 0;
  int abstractRetrieveFlag = 0;
  SdmError err;

  if (!argc) {
    printf(
"?The getheaders command takes several forms:"
"\tgetheaders store\n"
"\tgetheaders store messagenum\n"
"\tgetheaders store messagenum -a|-A\n"
"\tgetheaders store messagenum header(s)\n"
"\tgetheaders store messagenum -a|-A abstractheader(s)\n"
"\tThe -a flag indicates to retrieve headers by abstract header names\n"
"\tas opposed to header name strings themselves.\n"
"\tThe -A flag is identical to -a but additionally retrieves the headers by abstract\n"
"\theader name/header value pairs as opposed to header name/header value pairs.\n"
);
    return(TRUE);
  }

  // Extract the store number
  PmstMessageStore *pmst = lookupStore(*argv, TRUE);
  if (!pmst)
    return(TRUE);
  argc--;
  argv++;

  // Determine data store to operate on
  SdmMessageStore *mst = pmst->pmst_messagestore;
  assert(mst);

  // If no message number then for all messages return all headers
  SdmMessageNumber startnum = 0, endnum = 0;

  if (!argc) {
    if (!decodeMessageRange(messageList, startnum, endnum, "*", pmst, TRUE))
      return(TRUE);
  }
  else {
    char *messageSequence = *argv++;
    argc--;
    if (!decodeMessageRange(messageList, startnum, endnum, messageSequence, pmst, TRUE))
      return(TRUE);
  }

  // Extract -a
  if (argc && strcmp(*argv, "-a")==0) {
    abstractFlag++; 
    argc--;
    argv++;
  }
  else if (argc && strcmp(*argv, "-A")==0) {
    abstractFlag++;
    abstractRetrieveFlag++;
    argc--;
    argv++;
  }

  // Ok, anything left is either a message header flag value or name sequence
  if (!argc) {
    if (abstractFlag)
      headerAbstractFlags = Sdm_MHA_ALL;
  }
  else if (argc == 1) {
    char *arg = *argv++;
    assert(arg);
    argc--;
    if (abstractFlag) {
      if (!decodeHeaderAbstractValue(headerAbstractFlags, arg, TRUE))
	return(TRUE);
    }
    else {
      header = arg;
      headerSet = Sdm_True;
    }
  } else if (argc > 1) {
    if (abstractFlag) {
      while (argc--) {
	SdmMessageHeaderAbstractFlags x = 0;
	if (!decodeHeaderAbstractValue(x, *argv++, TRUE))
	  return(TRUE);
	headerAbstractFlags |= x;
      }
    }
    else {
      headerList.ClearAllElements();
      while (argc--)
	headerList(-1) = *argv++;
      headerListSet = Sdm_True;
    }
  }
    
  // If any headers are specified, produce list
  // At this point have a message range of some kind, deal with it
  // Issues:
  //	is -a set?
  //	range or single message?
  //	header list specified?

  // *GetHeaders(SdmError &err, SdmStrStrL &r_hdr, SdmMessageNumber &msgnum);
  // *GetHeaders(SdmError &err, SdmStrStrLL &r_hdr, SdmMessageNumber &startmsgnum, SdmMessageNumber &endmsgnum);
  // GetHeaders(SdmError &err, SdmStrStrLL &r_hdr, SdmMessageNumberL &msgnums);
  // *GetHeader(SdmError &err, SdmStrStrL &r_hdr, SdmString &hdr, SdmMessageNumber &msgnum);
  // *GetHeader(SdmError &err, SdmStrStrLL &r_hdr, SdmString &hdr, SdmMessageNumber &startmsgnum, SdmMessageNumber &endmsgnum);
  // GetHeader(SdmError &err, SdmStrStrLL &r_hdr, SdmString &hdr, SdmMessageNumberL &msgnums);
  // *GetHeaders(SdmError &err, SdmStrStrL &r_hdrs, SdmStringL &hdrs, SdmMessageNumber &msgnum);
  // *GetHeaders(SdmError &err, SdmStrStrLL &r_hdrs, SdmStringL &hdrs, SdmMessageNumber &startmsgnum, SdmMessageNumber &endmsgnum);
  // GetHeaders(SdmError &err, SdmStrStrLL &r_hdrs, SdmStringL &hdrs, SdmMessageNumberL &msgnums);
  // *GetHeader(SdmError &err, SdmStrStrL &r_hdr, SdmMessageHeaderAbstractFlags &hdr, SdmMessageNumber &msgnum);
  // *GetHeader(SdmError &err, SdmStrStrLL &r_hdr, SdmMessageHeaderAbstractFlags &hdr, SdmMessageNumber &startmsgnum, SdmMessageNumber &endmsgnum);
  // GetHeader(SdmError &err, SdmStrStrLL &r_hdr, SdmMessageHeaderAbstractFlags &hdr, SdmMessageNumberL &msgnums);

  // If !endnum then a single message is requested
  SdmStrStrL headerValue;
  SdmStrStrLL headerValueL;
  SdmIntStrL headerValueA;
  SdmIntStrLL headerValueAL;

  headerValueL.SetPointerDeleteFlag();		// Force vector contents to be deleted on object destruction
  headerValueAL.SetPointerDeleteFlag();		// ""

  int messageListLength = messageList.ElementCount();

  if (!endnum && !messageListLength) {
    // Only a single message number was specified
    assert(startnum);
    if (abstractFlag) {
      if (abstractRetrieveFlag) {
	// getheaders store -A msg abs
	mst->GetHeaders(err, headerValueA, headerAbstractFlags, startnum);
      }
      else {
	// getheaders store -a msg abs
	mst->GetHeaders(err, headerValue, headerAbstractFlags, startnum);
      }
    }
    else if (!headerSet && !headerListSet) {
      // getheaders store msg - get all headers for a single message
      mst->GetHeaders(err, headerValue, startnum);
    }
    else if (headerSet && !headerListSet) {
      // getheaders store msg hdr - get a single header for a single message
      mst->GetHeader(err, headerValue, header, startnum);
    }
    else if (!headerSet && headerListSet) {
      // getheaders store msg hdr1 hdr2
      mst->GetHeaders(err, headerValue, headerList, startnum);
    }
  }
  else if (endnum && !messageListLength) {
    // A range was specified
    assert(startnum);
    if (abstractFlag) {
      if (abstractRetrieveFlag) {
	// getheaders store -A msg abs - get abstract headers for a range
	mst->GetHeaders(err, headerValueAL, headerAbstractFlags, startnum, endnum);
      }
      else {
	// getheaders store -a msg abs - get abstract headers for a range
	mst->GetHeaders(err, headerValueL, headerAbstractFlags, startnum, endnum);
      }
    }
    else if (!headerSet && !headerListSet) {
      // getheaders store msg - get all headers for a range
      mst->GetHeaders(err, headerValueL, startnum, endnum);
    }
    else if (headerSet && !headerListSet) {
      // getheaders store msg hdr - get a single header for a range
      mst->GetHeader(err, headerValueL, header, startnum, endnum);
    }
    else if (!headerSet && headerListSet) {
      // getheaders store msg hdr1 hdr2
      mst->GetHeaders(err, headerValueL, headerList, startnum, endnum);
    }
  }
  else {
    // A message list was specified
    assert(!startnum && !endnum);
    if (abstractFlag) {
      if (abstractRetrieveFlag) {
	// getheaders store -A msg abs - get abstract headers for a range
	mst->GetHeaders(err, headerValueAL, headerAbstractFlags, messageList);
      }
      else {
	// getheaders store -a msg abs - get abstract headers for a range
	mst->GetHeaders(err, headerValueL, headerAbstractFlags, messageList);
      }
    }
    else if (!headerSet && !headerListSet) {
      // getheaders store msg - get all headers for a range
      mst->GetHeaders(err, headerValueL, messageList);
    }
    else if (headerSet && !headerListSet) {
      // getheaders store msg hdr - get a single header for a range
      mst->GetHeader(err, headerValueL, header, messageList);
    }
    else if (!headerSet && headerListSet) {
      // getheaders store msg hdr1 hdr2
      mst->GetHeaders(err, headerValueL, headerList, messageList);
    }
  }

  if (err != Sdm_EC_Success) {
    printf("message store %s: getheaders failed with error %s\n", expandStoreName(pmst), detailedErrorReport(err));
    return(TRUE);
  }

  printf("message store %s: getheaders successful\n", expandStoreName(pmst));

  if (!endnum && !messageListLength) {
    printf(" message %d:\n", startnum);
    int i = abstractRetrieveFlag ? headerValueA.ElementCount() : headerValue.ElementCount();
    for (int j = 0; j < i; j++)
      if (abstractRetrieveFlag)
	printf("  %15s: %s\n",
	       printAbstractHeaderFlag(headerValueA[j].GetNumber()),
	       (const char *)headerValueA[j].GetString() ? (const char *)headerValueA[j].GetString() : "(NULL)");
      else
	printf("  %15s: %s\n",
	       (const char *)headerValue[j].GetFirstString() ? (const char *)headerValue[j].GetFirstString() : "(NULL)",
	       (const char *)headerValue[j].GetSecondString() ? (const char *)headerValue[j].GetSecondString() : "(NULL)");
  }
  else if (endnum && !messageListLength && !abstractRetrieveFlag) {
    int numMessages = (endnum-startnum)+1;
    for (int k = 0; k < numMessages; k++) {
      SdmStrStrL *thislist = headerValueL[k];
      printf(" message %d:\n", k+startnum);
      int i = thislist->ElementCount();
      for (int j = 0; j < i; j++) {
	SdmStrStr thishdr = ((*thislist)[j]);
	printf("  %15s: %s\n",
	  (const char *)thishdr.GetFirstString(),
          (const char *)thishdr.GetSecondString());
      }
      thislist->ClearAllElements();
    }
  }
  else if (endnum && !messageListLength && abstractRetrieveFlag) {
    int numMessages = (endnum-startnum)+1;
    for (int k = 0; k < numMessages; k++) {
      SdmIntStrL *thislist = headerValueAL[k];
      printf(" message %d:\n", k+startnum);
      int i = thislist->ElementCount();
      for (int j = 0; j < i; j++) {
	SdmIntStr thishdr = ((*thislist)[j]);
	printf("  %15s: %s\n",
	       printAbstractHeaderFlag(thishdr.GetNumber()),
	       (const char *)thishdr.GetString());
      }
      thislist->ClearAllElements();
    }
  }
  else if (!abstractRetrieveFlag) {
    int numMessages = messageListLength;
    for (int k = 0; k < numMessages; k++) {
      SdmStrStrL *thislist = headerValueL[k];
      printf(" message %d:\n", messageList[k]);
      int i = thislist->ElementCount();
      for (int j = 0; j < i; j++) {
	SdmStrStr thishdr = ((*thislist)[j]);
	printf("  %15s: %s\n",
	  (const char *)thishdr.GetFirstString(),
          (const char *)thishdr.GetSecondString());
      }
      thislist->ClearAllElements();
    }
  }
  else {
    int numMessages = messageListLength;
    for (int k = 0; k < numMessages; k++) {
      SdmIntStrL *thislist = headerValueAL[k];
      printf(" message %d:\n", messageList[k]);
      int i = thislist->ElementCount();
      for (int j = 0; j < i; j++) {
	SdmIntStr thishdr = ((*thislist)[j]);
	printf("  %15s: %s\n",
	       printAbstractHeaderFlag(thishdr.GetNumber()),
	       (const char *)thishdr.GetString());
      }
      thislist->ClearAllElements();
    }
  }

  return(TRUE);
}

static int subcmd_store_getstatus(int argc, char **argv)
{
  if (!argc) {
    printf("?The getstatus subcommand takes one or more arguments which specify\n?the stores to be stated; to stat all stores, use a single * character.\n");
    return(TRUE);
  }
  while (argc--) {
    char *currentStore = *argv++;
    if (strcmp(currentStore,"*")==0) {
      SdmLinkedListIterator lit(STORE_LIST);
      PmstMessageStore *pmst;

      while (pmst = (PmstMessageStore *)(lit.NextListElement())) {
	statusStore(pmst, TRUE);
      }
    }
    else {
      PmstMessageStore *pmst = lookupStore(currentStore, TRUE);
      if (pmst)
	statusStore(pmst, TRUE);
    }
  }
  return(TRUE);
}

static int subcmd_store_open(int argc, char **argv)
{
  SdmError err;
  SdmMessageNumber nmsgs;
  SdmBoolean readOnly;
  int asyncFlag = 0;

  if (argc < 2 || argc > 3) {
    printf("?The open command takes the form: open store token [+a]\n");
    return(TRUE);
  }

  char *storeName = *argv++; argc--;
  char *tokenName = *argv++; argc--;

  // Extract +a if present
  if (argc && strcmp(*argv, "+a")==0) {
    asyncFlag++;
    argc--;
    argv++;
  }

  PmstMessageStore *pmst = lookupStore(storeName, TRUE);
  if (!pmst)
    return(TRUE);
  SdmMessageStore *mst = pmst->pmst_messagestore;
  assert(mst);

  PtkToken *ptk = lookupToken(tokenName, TRUE);
  if (!ptk)
    return(TRUE);
  SdmToken *tk = ptk->ptk_token;
  assert(tk);

  if (asyncFlag) {
    SdmServiceFunction svf(&openAsyncCallback, (void *)2);
    if (mst->Open_Async(err, svf, pmst, *tk) != Sdm_EC_Success) {
      printf("message store %s: open async failed with error %s\n", expandStoreName(pmst), detailedErrorReport(err));
    }
    else {
      printf("message store %s: open async successful: pending\n", expandStoreName(pmst));
    }
    return(TRUE);
  }

  mst->Open(err, nmsgs, readOnly, *tk);
  if (err != Sdm_EC_Success)
    printf("message store %s: open failed with error %s\n", expandStoreName(pmst), detailedErrorReport(err));
  else
    printf("message store %s: opened%s\n", expandStoreName(pmst), readOnly ? " IN READ ONLY MODE!" : "");
  return(TRUE);
}

static int subcmd_store_reconnect(int argc, char **argv)
{
  if (!argc) {
    printf("?The reconnect subcommand takes one or more arguments which specify\n?the stores to be reconnected; to reconnect all stores, use a single * character.\n");
    return(TRUE);
  }
  while (argc--) {
    char *currentStore = *argv++;
    if (strcmp(currentStore,"*")==0) {
      SdmLinkedListIterator lit(STORE_LIST);
      PmstMessageStore *pmst;

      while (pmst = (PmstMessageStore *)(lit.NextListElement())) {
	reconnectStore(pmst, TRUE);
      }
    }
    else {
      PmstMessageStore *pmst = lookupStore(currentStore, TRUE);
      if (pmst)
	reconnectStore(pmst, TRUE);
    }
  }
  return(TRUE);
}

static int subcmd_store_removefromsubscribednamespace(int argc, char **argv)
{
  SdmError err;
  SdmIntStrL namesL;

  if (argc != 2) {
    printf("?The removefromsubscribednamespace command takes the form: removefromsubscribednamespace store name\n");
    return(TRUE);
  }

  // Extract the store name
  PmstMessageStore *pmst = lookupStore(*argv, TRUE);
  if (!pmst)
    return(TRUE);
  argc--;
  argv++;

  // Determine message store to operate on
  SdmMessageStore *mst = pmst->pmst_messagestore;
  assert(mst);

  // Get the new name
  SdmString newname(*argv++); 

  // Issue the add call
  if (mst->RemoveFromSubscribedNamespace(err, newname) != Sdm_EC_Success) {
    printf("message store %s: removefromsubscribednamespace failed with error %s\n", expandStoreName(pmst), detailedErrorReport(err));
    return(TRUE);
  }

  printf("message store %s: removefromsubscribednamespace successful\n", expandStoreName(pmst));
  return(TRUE);
}

static int subcmd_store_scannamespace(int argc, char **argv)
{
  SdmError err;
  SdmIntStrL namesL;

  if (argc !=3) {
    printf("?The scannamespace command takes the form: scannamespace store reference pattern\n");
    return(TRUE);
  }

  // Extract the store name
  PmstMessageStore *pmst = lookupStore(*argv, TRUE);
  if (!pmst)
    return(TRUE);
  argc--;
  argv++;

  // Determine message store to operate on
  SdmMessageStore *mst = pmst->pmst_messagestore;
  assert(mst);

  // Get the reference and pattern arguments
  SdmString reference(*argv++);
  SdmString pattern(*argv++); 

  // Issue the scan call
  if (mst->ScanNamespace(err, namesL, reference, pattern) != Sdm_EC_Success) {
    printf("message store %s: scannamespace failed with error %s\n", expandStoreName(pmst), detailedErrorReport(err));
    return(TRUE);
  }

  int numEntries = namesL.ElementCount();
  printf("message store %s: scannamespace successful, returned %d names\n", expandStoreName(pmst), numEntries);

  for (int i = 0; i < numEntries; i ++)
    printf("\t%s\t%s\n", printNamespaceFlag(namesL[i].GetNumber()), (const char *)namesL[i].GetString());
  return(TRUE);
}

static int subcmd_store_scansubscribednamespace(int argc, char **argv)
{
  SdmError err;
  SdmIntStrL namesL;

  if (argc !=3) {
    printf("?The scansubscribednamespace command takes the form: scansubscribednamespace store reference pattern\n");
    return(TRUE);
  }

  // Extract the store name
  PmstMessageStore *pmst = lookupStore(*argv, TRUE);
  if (!pmst)
    return(TRUE);
  argc--;
  argv++;

  // Determine message store to operate on
  SdmMessageStore *mst = pmst->pmst_messagestore;
  assert(mst);

  // Get the reference and pattern arguments
  SdmString reference(*argv++);
  SdmString pattern(*argv++); 

  // Issue the scan call
  if (mst->ScanSubscribedNamespace(err, namesL, reference, pattern) != Sdm_EC_Success) {
    printf("message store %s: scansubscribednamespace failed with error %s\n", expandStoreName(pmst), detailedErrorReport(err));
    return(TRUE);
  }

  int numEntries = namesL.ElementCount();
  printf("message store %s: scansubscribednamespace successful, returned %d names\n", expandStoreName(pmst), numEntries);

  for (int i = 0; i < numEntries; i ++)
    printf("\t%s\t%s\n", printNamespaceFlag(namesL[i].GetNumber()), (const char *)namesL[i].GetString());
  return(TRUE);
}

static int subcmd_store_setflags(int argc, char **argv)
{
  return storeSetFlags(argc, argv, Sdm_True);
}

static int subcmd_store_startup(int argc, char **argv)
{
  if (!argc) {
    printf("?The startup subcommand takes one or more arguments which specify\n?the stores to be startupd; to startup all stores, use a single * character.\n");
    return(TRUE);
  }
  while (argc--) {
    char *currentStore = *argv++;
    if (strcmp(currentStore,"*")==0) {
      SdmLinkedListIterator lit(STORE_LIST);
      PmstMessageStore *pmst;

      while (pmst = (PmstMessageStore *)(lit.NextListElement())) {
	startupStore(pmst, TRUE);
      }
    }
    else {
      PmstMessageStore *pmst = lookupStore(currentStore, TRUE);
      if (pmst)
	startupStore(pmst, TRUE);
    }
  }
  return(TRUE);
}

static int subcmd_store_shutdown(int argc, char **argv)
{
  if (!argc) {
    printf("?The shutdown subcommand takes one or more arguments which specify\n?the stores to be shutdown; to shutdown all stores, use a single * character.\n");
    return(TRUE);
  }
  while (argc--) {
    char *currentStore = *argv++;
    if (strcmp(currentStore,"*")==0) {
      SdmLinkedListIterator lit(STORE_LIST);
      PmstMessageStore *pmst;

      while (pmst = (PmstMessageStore *)(lit.NextListElement())) {
	shutdownStore(pmst, TRUE);
      }
    }
    else {
      PmstMessageStore *pmst = lookupStore(currentStore, TRUE);
      if (pmst)
	shutdownStore(pmst, TRUE);
    }
  }
  return(TRUE);
}
