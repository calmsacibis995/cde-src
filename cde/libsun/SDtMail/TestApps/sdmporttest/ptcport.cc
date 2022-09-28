#include <SDtMail/Sdtmail.hh>
#include <SDtMail/LinkedList.hh>
#include <PortObjs/DataPort.hh>
#include <SDtMail/DeliveryResponse.hh>
#include <SDtMail/SystemUtility.hh>
#include <porttest.hh>
#include <ptcmdscan.hh>
#include <ptcport.hh>
#include <ptctoken.hh>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <SDtMail/MessageUtility.hh>

/*
** EXTERNALS
*/

extern SdmLinkedList *TOKEN_LIST;

/*
** GLOBALS
*/
char *Hport =
"Description: manipulate data port objects\n"
"General usage: port command [ port [ arguments ]]\n"
"Specific usage:\n"
"\tport addheader port messagenum header contents\n"
"\tport addmessagebodytomessage port messagenum bodytype bodysubtype [component]\n"
"\tport addtosubscribednamespace port name\n"
"\tport attach port token\n"
"\tport checkfornewmessages [port(s) | *]\n"
"\tport clearflags port messagenum flags\n"
"\tport close [port(s) | *]\n"
"\tport commitpendingmessagechanges port messagenum\n"
"\tport copymessage port messagenum token\n"
"\tport create name\n"
"\tport delete [port(s) | *]\n"
"\tport derivedmessagecreate port messagenum\n"
"\tport discardpendingmessagechanges port messagenum\n"
"\tport disconnect\n"
"\tport expungedeleted\n"
"\tport getattributes port messagenum component [attribute(s)]\n"
"\tport getattributes port messagenum component [-a [abstractattribute(s)]\n"
"\tport getcontents port messagenum  [-p | -f | -h | -b] [-F <outputfile>] [-S] [-H] [component]\n"
"\tport getflags port [messagenum]\n"
"\tport getheaders port [messagenum [header(s)]]\n"
"\tport getheaders port [messagenum [ -a|-A [abstractheader(s)]]]\n"
"\tport getstatus [port(s) | *]\n"
"\tport getstructure port [messagenum]\n"
"\tport messagebodydelete port messagenum component [yes | no]\n"
"\tport movemessage port messagenum token\n"
"\tport newmessagecreate port\n"
"\tport open port token\n"
"\tport removefromsubscribednamespace port name\n"
"\tport removeheader port messagenum header\n"
"\tport replaceheader port messagenum header contents\n"
"\tport resolveandextractaddresses port srcport messagenum [-f]\n"
"\tport scannamespace port reference pattern\n"
"\tport scansubscribednamespace port reference pattern\n"
"\tport setattributes port messagenum component attribute-name [attribute-value]\n"
"\tport setcontents port messagenum component [contents | -F file]\n"
"\tport setflags port messagenum flags\n"
"\tport shutdown [port(s) | *]\n"
"\tport startup [port(s) | *]\n"
"\tport storecreate port token\n"
"\tport storedelete port token\n"
"\tport submit port srcport messagenum [ mime | sunv3 ]\n"
"\tport validate [ string | -F file | -S sum ]\n"
"Notes:\n"
"messagenum specifies the message(s) to work on; it takes one of 4 forms:\n"
"1. a single number specifying the message to work on.\n"
"2. a range of the form first:last specifying an inclusive range to work on.\n"
"3. a list of the form a,b,...,x specifying specific message to work on.\n"
"4. the single character '*' which means all messages in the store.\n"
"port create creates a 'port' object, not a message store;\n"
"port storecreate creates a message store on a port.\n"
"port delete deletes a 'port' object, not a message store;\n"
"port storedelete deletes a message store on a port.\n"
"";

SdmLinkedList *PORT_LIST = 0;

static int subcmd_port_addheader(int argc, char **argv);
static int subcmd_port_addmessagebodytomessage(int argc, char **argv);
static int subcmd_port_addtosubscribednamespace(int argc, char **argv);
static int subcmd_port_attach(int argc, char **argv);
static int subcmd_port_checkfornewmessages(int argc, char **argv);
static int subcmd_port_clearflags(int argc, char **argv);
static int subcmd_port_close(int argc, char **argv);
static int subcmd_port_commitpendingmessagechanges(int argc, char **argv);
static int subcmd_port_copymessage(int argc, char **argv);
static int subcmd_port_create(int argc, char **argv);
static int subcmd_port_delete(int argc, char **argv);
static int subcmd_port_derivedmessagecreate(int argc, char **argv);
static int subcmd_port_discardpendingmessagechanges(int argc, char **argv);
static int subcmd_port_disconnect(int argc, char **argv);
static int subcmd_port_expungedeleted(int argc, char **argv);
static int subcmd_port_getattributes(int argc, char **argv);
static int subcmd_port_getcontents(int argc, char **argv);
static int subcmd_port_getflags(int argc, char **argv);
static int subcmd_port_getheaders(int argc, char **argv);
static int subcmd_port_getstatus(int argc, char **argv);
static int subcmd_port_getstructure(int argc, char **argv);
static int subcmd_port_messagebodydelete(int argc, char **argv);
static int subcmd_port_movemessage(int argc, char **argv);
static int subcmd_port_newmessagecreate(int argc, char **argv);
static int subcmd_port_open(int argc, char **argv);
static int subcmd_port_removefromsubscribednamespace(int argc, char **argv);
static int subcmd_port_removeheader(int argc, char **argv);
static int subcmd_port_replaceheader(int argc, char **argv);
static int subcmd_port_resolveandextractaddresses(int argc, char **argv);
static int subcmd_port_scannamespace(int argc, char **argv);
static int subcmd_port_scansubscribednamespace(int argc, char **argv);
static int subcmd_port_setattributes(int argc, char **argv);
static int subcmd_port_setcontents(int argc, char **argv);
static int subcmd_port_setflags(int argc, char **argv);
static int subcmd_port_shutdown(int argc, char **argv);
static int subcmd_port_startup(int argc, char **argv);
static int subcmd_port_storecreate(int argc, char **argv);
static int subcmd_port_storedelete(int argc, char **argv);
static int subcmd_port_submit(int argc, char **argv);
static int subcmd_port_validate(int argc, char **argv);

static SUBCMD portcmdtab[] = {
  // cmdname,		handler,			argsrequired
  {"addheader",		subcmd_port_addheader,		0},		// add message header
  {"addmessagebodytomessage", subcmd_port_addmessagebodytomessage,	0},		// add a new message body to a message
  {"addtosubscribednamespace",	subcmd_port_addtosubscribednamespace,		0},	// add subscribed message store name
  {"attach",		subcmd_port_attach,		0},	// attach on a port
  {"checkfornewmessages",	subcmd_port_checkfornewmessages,		0},	// check for new messages on port
  {"clearflags",	subcmd_port_clearflags, 	0},	// clear flags on a message
  {"close",		subcmd_port_close,		0},	// close a port
  {"commitpendingmessagechanges",		subcmd_port_commitpendingmessagechanges,		0},	// commid pending changes
  {"copymessage",	subcmd_port_copymessage,	0},	// copy message(s)
  {"create",		subcmd_port_create,		0},	// create a new port
  {"delete",		subcmd_port_delete,		0},	// delete an existing port
  {"derivedmessagecreate", subcmd_port_derivedmessagecreate,	0},	// create derived message
  {"discardpendingmessagechanges",	subcmd_port_discardpendingmessagechanges,		0},	// discard pending changes
  {"disconnect",	subcmd_port_disconnect,		0},	// disconnect port
  {"expungedeleted",	subcmd_port_expungedeleted,	0},	// expunge deleted messages
  {"getattributes",	subcmd_port_getattributes,	0},	// get message attributes
  {"getcontents",	subcmd_port_getcontents,	0},	// get message contents
  {"getflags",		subcmd_port_getflags,		0},	// get message flags
  {"getheaders",	subcmd_port_getheaders,		0},	// get message headers
  {"getstatus",		subcmd_port_getstatus,		0},	// status of port
  {"getstructure",	subcmd_port_getstructure,	0},	// get message structure
  {"messagebodydelete",	subcmd_port_messagebodydelete,	0},	// delete a message body
  {"movemessage",	subcmd_port_movemessage,	0},	// move message(s)
  {"newmessagecreate",	subcmd_port_newmessagecreate,	0},	// create new message
  {"open",		subcmd_port_open,		2},	// open a port
  {"removefromsubscribednamespace",	subcmd_port_removefromsubscribednamespace,		0},	// add subscribed message store name
  {"removeheader",		subcmd_port_removeheader,		0},		// remove message header
  {"replaceheader",		subcmd_port_replaceheader,		0},		// replace message header
  {"resolveandextractaddresses",	subcmd_port_resolveandextractaddresses,		0},	// add subscribed message store name
  {"scannamespace",	subcmd_port_scannamespace,	0},	// scan namespace
  {"scansubscribednamespace",	subcmd_port_scansubscribednamespace,	0},	// scan the subscribed namespace
  {"setattributes",	subcmd_port_setattributes,	0},	// set message attributes
  {"setcontents",	subcmd_port_setcontents,	0},	// set message contents
  {"setflags",		subcmd_port_setflags,		0},	// set flags on a message
  {"shutdown",		subcmd_port_shutdown,		0},	// shutdown a port
  {"startup",		subcmd_port_startup,		0},	// startup a port
  {"storecreate",	subcmd_port_storecreate,	0},	// create a message store
  {"storedelete",	subcmd_port_storedelete,	0},	// delete a message store
  {"submit",		subcmd_port_submit,		0},	// submit message for transmission
  {"validate",		subcmd_port_validate,		0},	// validate last result
  {NULL,		0,				0}	// must be last entry in table
};


/*
** FORWARD DECLARATIONS
*/

/*************************************************************************
 *
 * Function:	Cport - 
 *
 * Input:	int pargc - number of arguments passed to command
 *		char **pargv - array of pargc arguments passed
 *
 * Output:	TRUE - help given
 *
 ************************************************************************/

int Cport(int argc, char **argv)
{
  assert(argc != 0);
  if (argc == 1)
    return (TRUE);

  // All cmds except "validate" reset the last result
  if (strcmp(argv[1], "validate") != 0)
    ptLastResult.Reset();

  int rc = (argc == 1 ? TRUE : oneSubCommand(argv[0], argv[1], portcmdtab, argc-2, argv+2));

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

static const char *expandPortName(PdpDataPort *pdp)
{
  assert(pdp->pdp_name);
  return ((const char *)*pdp->pdp_name);
}

static void handleCallbackEvent(void* userdata, SdmDpEvent* response)
{
  SdmDpErrorLogEvent* errorLogEvent = 0;
  SdmDpDebugLogEvent* debugLogEvent = 0;
  SdmDpBusyIndicationEvent* busyIndicationEvent = 0;

  PdpDataPort *pdp = (PdpDataPort*)userdata;
  if (!pdp)
    printf("Error: user data in callback event is null!\ndata port ?: callback event: ");
  else
    printf("data port %s: callback event: ", expandPortName(pdp));

  switch(response->sdmDpBase.type) {
  case DPEVENT_ErrorLog:
    errorLogEvent = &response->sdmDpErrorLog;
    printf("error log: %s\n", errorLogEvent->errorMessage ? (const char *)*errorLogEvent->errorMessage : "NO ERROR MESSAGE STRING SPECIFIED");
    break;
  case DPEVENT_DebugLog:
    debugLogEvent = &response->sdmDpDebugLog;
    printf("debug log:  %s\n", debugLogEvent->debugMessage ? (const char *)*debugLogEvent->debugMessage : "NO DEBUG MESSAGE STRING SPECIFIED");
    break;
  case DPEVENT_BusyIndication:
    printf("busy indication\n");
    break;
  case DPEVENT_SessionLockGoingAway:		
    printf("session lock going away\n");
    break;
  case DPEVENT_SessionLockTakenAway:
    printf("session lock taken away\n");
    break;
  case DPEVENT_WaitingForSessionLock:
    printf("waiting for session lock\n");
    break;
  case DPEVENT_NotWaitingForSessionLock:
    printf("not waiting for session lock\n");
    break;
  case DPEVENT_WaitingForUpdateLock:
    printf("waiting for update lock\n");
    break;
  case DPEVENT_NotWaitingForUpdateLock:
    printf("not waiting for update lock\n");
    break;
  case DPEVENT_ServerConnectionBroken:
    printf("server connection broken\n");
    break;
  case DPEVENT_MailboxConnectionReadOnly:
    printf("mailbox connection is now read only\n");
    break;
  case DPEVENT_MailboxChangedByOtherUser:
    printf("mailbox has been changed by another user\n");
    break;
  default:
    printf("UNKNOWN EVENT TYPE: %d\n", response->sdmDpBase.type);
    break;
  }

}

void BusyIndicationCallback(void* userdata, SdmDpCallbackType type, SdmDpEvent* response)
{
  switch(type) {
  case DPCBTYPE_ErrorLog:
    printf("\nError: BusyIndicationCallback given error logging callback type\n");
    break;
  case DPCBTYPE_DebugLog:
    printf("\nError: BusyIndicationCallback given debug logging callback type\n");
    break;
  case DPCBTYPE_BusyIndication:
    break;
  case DPCBTYPE_LockActivity:
    printf("\nError: BusyIndicationCallback given lock activity callback type\n");
    break;
  default:
    printf("\nError: BusyIndicationCallback given unknown callback type\n");
    break;
  }

  switch(response->sdmDpBase.type) {
  case DPEVENT_ErrorLog:
    printf("\nError: BusyIndicationCallback given error logging event!\n");
    handleCallbackEvent(userdata, response);
    break;
  case DPEVENT_DebugLog:
    printf("\nError: BusyIndicationCallback given debug logging event!\n");
    handleCallbackEvent(userdata, response);
    break;
  case DPEVENT_BusyIndication:
    handleCallbackEvent(userdata, response);
    break;
  case DPEVENT_SessionLockGoingAway:		
  case DPEVENT_SessionLockTakenAway:
  case DPEVENT_WaitingForSessionLock:
  case DPEVENT_NotWaitingForSessionLock:
  case DPEVENT_WaitingForUpdateLock:
  case DPEVENT_NotWaitingForUpdateLock:
  case DPEVENT_ServerConnectionBroken:
  case DPEVENT_MailboxConnectionReadOnly:
  case DPEVENT_MailboxChangedByOtherUser:
    printf("\nError: BusyIndicationCallback given lock activity event!\n");
    handleCallbackEvent(userdata, response);
    break;
  default:
    printf("\nError: BusyIndicationCallback given unknown event!\n");
    handleCallbackEvent(userdata, response);
    break;
  }
}

void ErrorLogCallback(void* userdata, SdmDpCallbackType type, SdmDpEvent* response)
{
  switch(type) {
  case DPCBTYPE_ErrorLog:
    break;
  case DPCBTYPE_DebugLog:
    printf("\nError: ErrorLogCallback given debug logging callback type\n");
    break;
  case DPCBTYPE_BusyIndication:
    printf("\nError: ErrorLogCallback given busy indication callback type\n");
    break;
  case DPCBTYPE_LockActivity:
    printf("\nError: ErrorLogCallback given lock activity callback type\n");
    break;
  default:
    printf("\nError: ErrorLogCallback given unknown callback type\n");
    break;
  }

  switch(response->sdmDpBase.type) {
  case DPEVENT_ErrorLog:
    handleCallbackEvent(userdata, response);
    break;
  case DPEVENT_DebugLog:
    printf("\nError: ErrorLogCallback given debug logging event!\n");
    handleCallbackEvent(userdata, response);
    break;
  case DPEVENT_BusyIndication:
    printf("\nError: ErrorLogCallback given busy notification event!\n");
    handleCallbackEvent(userdata, response);
    break;
  case DPEVENT_SessionLockGoingAway:		
  case DPEVENT_SessionLockTakenAway:
  case DPEVENT_WaitingForSessionLock:
  case DPEVENT_NotWaitingForSessionLock:
  case DPEVENT_WaitingForUpdateLock:
  case DPEVENT_NotWaitingForUpdateLock:
  case DPEVENT_ServerConnectionBroken:
  case DPEVENT_MailboxConnectionReadOnly:
  case DPEVENT_MailboxChangedByOtherUser:
    printf("\nError: ErrorLogCallback given lock activity event!\n");
    handleCallbackEvent(userdata, response);
    break;
  default:
    printf("\nError: ErrorLogCallback given unknown event!\n");
    handleCallbackEvent(userdata, response);
    break;
  }
}

void DebugLogCallback(void* userdata, SdmDpCallbackType type, SdmDpEvent* response)
{
  switch(type) {
  case DPCBTYPE_ErrorLog:
    printf("\nError: DebugLogCallback given error logging callback type\n");
    break;
  case DPCBTYPE_DebugLog:
    break;
  case DPCBTYPE_BusyIndication:
    printf("\nError: DebugLogCallback given busy indication callback type\n");
    break;
  case DPCBTYPE_LockActivity:
    printf("\nError: DebugLogCallback given lock activity callback type\n");
    break;
  default:
    printf("\nError: DebugLogCallback given unknown callback type\n");
    break;
  }

  switch(response->sdmDpBase.type) {
  case DPEVENT_ErrorLog:
    printf("\nError: DebugLogCallback given error logging event!\n");
    handleCallbackEvent(userdata, response);
    break;
  case DPEVENT_DebugLog:
    handleCallbackEvent(userdata, response);
    break;
  case DPEVENT_BusyIndication:
    printf("\nError: DebugLogCallback given busy notification event!\n");
    handleCallbackEvent(userdata, response);
    break;
  case DPEVENT_SessionLockGoingAway:		
  case DPEVENT_SessionLockTakenAway:
  case DPEVENT_WaitingForSessionLock:
  case DPEVENT_NotWaitingForSessionLock:
  case DPEVENT_WaitingForUpdateLock:
  case DPEVENT_NotWaitingForUpdateLock:
  case DPEVENT_ServerConnectionBroken:
  case DPEVENT_MailboxConnectionReadOnly:
  case DPEVENT_MailboxChangedByOtherUser:
    printf("\nError: DebugLogCallback given lock activity event!\n");
    handleCallbackEvent(userdata, response);
    break;
  default:
    printf("\nError: DebugLogCallback given unknown event!\n");
    handleCallbackEvent(userdata, response);
    break;
  }
}

void LockActivityCallback(void* userdata, SdmDpCallbackType type, SdmDpEvent* response)
{
  switch(type) {
  case DPCBTYPE_ErrorLog:
    printf("\nError: LockActivityCallback given error logging callback type\n");
    break;
  case DPCBTYPE_DebugLog:
    printf("\nError: LockActivityCallback given debug logging callback type\n");
    break;
  case DPCBTYPE_BusyIndication:
    printf("\nError: LockActivityCallback given busy indication callback type\n");
    break;
  case DPCBTYPE_LockActivity:
    break;
  default:
    printf("\nError: LockActivityCallback given unknown callback type\n");
    break;
  }

  switch(response->sdmDpBase.type) {
  case DPEVENT_ErrorLog:
    printf("\nError: LockActivityCallback given error logging event!\n");
    handleCallbackEvent(userdata, response);
    break;
  case DPEVENT_DebugLog:
    printf("\nError: LockActivityCallback given debug logging event!\n");
    handleCallbackEvent(userdata, response);
    break;
  case DPEVENT_BusyIndication:
    printf("\nError: LockActivityCallback given busy notification event!\n");
    handleCallbackEvent(userdata, response);
    break;
  case DPEVENT_SessionLockGoingAway:		
  case DPEVENT_SessionLockTakenAway:
  case DPEVENT_WaitingForSessionLock:
  case DPEVENT_NotWaitingForSessionLock:
  case DPEVENT_WaitingForUpdateLock:
  case DPEVENT_NotWaitingForUpdateLock:
  case DPEVENT_ServerConnectionBroken:
  case DPEVENT_MailboxConnectionReadOnly:
  case DPEVENT_MailboxChangedByOtherUser:
    handleCallbackEvent(userdata, response);
    break;
  default:
    printf("\nError: LockActivityCallback given unknown event!\n");
    handleCallbackEvent(userdata, response);
    break;
  }
}

static void GroupPrivilegeActionCallback(void* userData, SdmBoolean enableFlag)
{
  printf("Data Port ?: group privilege action: %s\n", enableFlag ? "enable" : "disable");
  return;
}

static PdpDataPort *validPortName(char *portName)
{
  SdmLinkedListIterator lit(PORT_LIST);
  PdpDataPort *pdp;

  while (pdp = (PdpDataPort *)(lit.NextListElement())) {
    assert(pdp->pdp_name);
    if (strcasecmp((const char *)*pdp->pdp_name, portName)==0)
      return(pdp);
  }
  return((PdpDataPort *)0);
}

static const char *expandMsDisp(SdmMsgStrDisposition &mdisp)
{
  switch(mdisp) {
  case Sdm_MSDISP_not_specified:	// disposition of component not specified
    return("(unspec)");
  case Sdm_MSDISP_inline:		// component should be handled "in line" if possible
    return("inline");
  case Sdm_MSDISP_attachment:		// component should be handled as "attachment" 
    return("attach");
  default:
    return("unknown!");
  }
}

static const char *expandString(SdmString &str)
{
  if (str) {
    const char *x = (const char *)str;
    if (x && *x)
      return(x);
    return("(none)");
  }
  return("(nil)");
}

static const char *expandMsType(SdmMsgStrType &type)
{
  switch (type) {
  case Sdm_MSTYPE_none:
    return("none");
  case Sdm_MSTYPE_text:
    return("text");
  case Sdm_MSTYPE_multipart:
    return("multipart");
  case Sdm_MSTYPE_message:
    return("message");
  case Sdm_MSTYPE_application:
    return("application");
  case Sdm_MSTYPE_audio:
    return("audio");
  case Sdm_MSTYPE_image:
    return("image");
  case Sdm_MSTYPE_video:
    return("video");
  case Sdm_MSTYPE_other:
    return("other");
  default:
    return("unknown!");
  }
}

static const char *expandMeType(SdmMsgStrEncoding &type)
{
  switch(type) {
  case Sdm_MSENC_none:
    return("none");
  case Sdm_MSENC_7bit:
    return("7bit");
  case Sdm_MSENC_8bit:
    return("8bit");
  case Sdm_MSENC_binary:
    return("binary");
  case Sdm_MSENC_base64:
    return("base64");
  case Sdm_MSENC_quoted_printable:
    return("qprint");
  case Sdm_MSENC_other:
    return("other");
  default:
    return("unknown!");
  }
}

SdmDpMessageComponent *searchComponentStructure(SdmDpMessageStructureL &msgstrL, char *componentString, char *targetComponentString)
{
  char compbuf[256];
  int componentIndex = 0;
  int numEntries = msgstrL.ElementCount();
  for (int i = 0; i < numEntries; i++) {
    SdmDpMessageStructure *mstp = msgstrL[i];
    componentIndex++;
    sprintf(compbuf, "%s.%d", componentString, componentIndex);
    if (::strcmp(compbuf, targetComponentString)==0) {
      return(mstp->ms_component);
    }
    if (mstp->ms_ms) {
      SdmDpMessageComponent *mcmp;
      mcmp = searchComponentStructure(*mstp->ms_ms, compbuf, targetComponentString);
      if (mcmp)
	return(mcmp);
    }
  }
  return(0);
}

SdmDpMessageComponent *lookupMessageComponent(PdpDataPort *pdp, SdmDpMessageStructureL& msgstrL, char *targetComponentString, int printMessage)
{
  SdmError err;
  SdmDpMessageComponent *mcmp;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  mcmp = searchComponentStructure(msgstrL, "0", targetComponentString);
  return(mcmp);
}

char *printAbstractHeaderFlag(SdmMessageHeaderAbstractFlags flags)
{
  static char buf[128];

  if (flags & Sdm_MHA_Bcc)
    return("Bcc");
  if (flags & Sdm_MHA_Cc)
    return("Cc");
  if (flags & Sdm_MHA_Date)
    return("Date");
  if (flags & Sdm_MHA_From)
    return("From");
  if (flags & Sdm_MHA_Subject)
    return("Subject");
  if (flags & Sdm_MHA_To)
    return("To");
  if (flags & Sdm_MHA_P_Bcc)
    return("PBcc");
  if (flags & Sdm_MHA_P_Cc)
    return("PCc");
  if (flags & Sdm_MHA_P_MessageFrom)
    return("PMessageFrom");
  if (flags & Sdm_MHA_P_MessageSize)
    return("PMessageSize");
  if (flags & Sdm_MHA_P_MessageType)
    return("PMessageType");
  if (flags & Sdm_MHA_P_ReceivedDate)
    return("PReceivedDate");
  if (flags & Sdm_MHA_P_Sender)
    return("PSender");
  if (flags & Sdm_MHA_P_SenderPersonalInfo)
    return("PSenderPersonalInfo");
  if (flags & Sdm_MHA_P_SenderUsername)
    return("PSenderUsername");
  if (flags & Sdm_MHA_P_SentDate)
    return("PSentDate");
  if (flags & Sdm_MHA_P_Subject)
    return("PSubject");
  if (flags & Sdm_MHA_P_To)
    return("PTo");

  sprintf(buf, "%08lx", flags);
  return(buf);
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

static void printStructure(SdmDpMessageStructureL &msgstr, char *componentString, int printHeader)
{
  int componentIndex = 0;
  int numEntries = msgstr.ElementCount();
  char compstr[256];

  if (printHeader)
    printf("  %-15s %-11s %-15s %-8s %-5s %-6s %-10s %-8s %-4s %s %s %s\n",
	   "ID","TYPE","SUBTYPE","ENCODING","LINES","BYTES","NAME","DISP","MODE","ISECTION","CACHED?","DESCRIPTION");
  for (int i = 0; i < numEntries; i++) {
    SdmDpMessageStructure *mstp = msgstr[i];
    componentIndex++;
    sprintf(compstr, "%s.%d", componentString, componentIndex);
    printf("  %-15s %-11s %-15s %-8s %-5d %-6d %-10s %-8s %-4o ",
	   compstr, expandMsType(mstp->ms_type), expandString(mstp->ms_subtype), 
	   expandMeType(mstp->ms_encoding), mstp->ms_lines, mstp->ms_bytes,
           expandString(mstp->ms_attachment_name), expandMsDisp(mstp->ms_disposition),
           mstp->ms_unix_mode);
    mstp->ms_component->Print();
    printf(" \t%s", mstp->ms_cached ? "Y" : "N");
    printf(" \t%s\n", expandString(mstp->ms_description));
    if (mstp->ms_ms) {
      char x[126];
      sprintf(x, "%s.%d", componentString, componentIndex);
      assert(strlen(x) < sizeof(x));
      printStructure(*mstp->ms_ms, x, FALSE);
    }
  }
}

void printFlags(SdmMessageFlagAbstractFlags &flags)
{
  printf("0x%08lx ", flags);
  printf("%8s %7s %5s %7s %6s %4s",
	 flags & Sdm_MFA_Answered ? "Answered" : "",
	 flags & Sdm_MFA_Deleted ? "Deleted" : "",
	 flags & Sdm_MFA_Draft ? "Draft" : "",
	 flags & Sdm_MFA_Flagged ? "Flagged" : "",
	 flags & Sdm_MFA_Recent ? "Recent" : "",
	 flags & Sdm_MFA_Seen ? "Seen" : "");
  if (flags & ~Sdm_MFA_ALL)
    printf(" fluff = 0x%08lx", flags & ~Sdm_MFA_ALL);
  printf("\n");
}

int decodeFlagAbstractValue(SdmMessageFlagAbstractFlags &flagAbstractFlags, char *sequence, int printMessage)
{
  assert(*sequence);
  char *p = 0;

  // First, see if its just a simple number - if so accept as abstract value

  flagAbstractFlags = strtoul(sequence, &p, 0);
  if (!*p)
    return(TRUE);

  flagAbstractFlags = 0;

  // Ok, not a simple number, look it up in the abstract message table

  SdmAbstractFlagMap *afmp = _sdmAbstractFlags;

  for (afmp; afmp->flag; afmp++) {
    if (afmp->abstractName && ::strcasecmp(sequence, afmp->abstractName)==0)
      flagAbstractFlags |= afmp->flag;
  }

  if (!flagAbstractFlags) {
    if (printMessage)
      printf("?Abstract flag name '%s' is not recognized\nflag names recognized are: ", sequence);
      for (afmp = _sdmAbstractFlags; afmp->flag; afmp++)
	printf(" %s", afmp->abstractName);
      printf("\n");
    return(FALSE);
  }

  return(TRUE);
}

static int decodeAttributeAbstractValue(SdmMessageAttributeAbstractFlags &attributeAbstractFlags, char *sequence, int printMessage)
{
  assert(*sequence);
  char *p = 0;

  // First, see if its just a simple number - if so accept as abstract value

  attributeAbstractFlags = strtoul(sequence, &p, 0);
  if (!*p)
    return(TRUE);

  attributeAbstractFlags = 0;

  // Ok, not a simple number, look it up in the abstract message table
  SdmAbstractAttributeMap *aamp = _sdmAbstractAttributes;

  for (aamp; aamp->flag; aamp++) {
    if (aamp->abstractName && ::strcasecmp(sequence, aamp->abstractName)==0)
      attributeAbstractFlags |= aamp->flag;
  }

  if (!attributeAbstractFlags) {
    if (printMessage)
      printf("?Abstract attribute name '%s' is not recognized\nattribute names recognized are: ", sequence);
      for (aamp = _sdmAbstractAttributes; aamp->flag; aamp++)
	printf(" %s", aamp->abstractName);
      printf("\n");
    return(FALSE);
  }

  return(TRUE);
}

int decodeHeaderAbstractValue(SdmMessageHeaderAbstractFlags &headerAbstractFlags, char *sequence, int printMessage)
{
  assert(*sequence);
  char *p = 0;

  // First, see if its just a simple number - if so accept as abstract value
  headerAbstractFlags = strtoul(sequence, &p, 0);
  if (!*p)
    return(TRUE);

  headerAbstractFlags = 0;

  // Ok, not a simple number, look it up in the abstract message table
  SdmAbstractHeaderMap *ahmp = _sdmAbstractHeaders;

  for (ahmp; ahmp->flag; ahmp++) {
    if (ahmp->abstractName && ::strcasecmp(sequence, ahmp->abstractName)==0)
      headerAbstractFlags |= ahmp->flag;
  }

  if (!headerAbstractFlags) {
    if (printMessage)
      printf("?Abstract header name '%s' is not recognized\nHeader names recognized are: ", sequence);
      for (ahmp = _sdmAbstractHeaders; ahmp->flag; ahmp++)
	printf(" %s", ahmp->abstractName);
      printf("\n");
    return(FALSE);
  }

  return(TRUE);
}

static int decodeMessageRange(SdmMessageNumberL &msglist, SdmMessageNumber &startnum, SdmMessageNumber &endnum, char *messageSequence, PdpDataPort *pdp, int printMessage)
{
  SdmError err;

  assert(*messageSequence);
  assert(pdp);

  char *p = 0;
  startnum = 0;
  endnum = 0;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  if (::strcmp(messageSequence, "*")==0) {
    SdmMessageStoreStatus statbuf;
    startnum = 1;
    if (dp->GetMessageStoreStatus(err, statbuf, Sdm_MSS_Messages) != Sdm_EC_Success) {
      if (printMessage)
	printf("data port %s: get message store status failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
      return(FALSE);
    }
    endnum = statbuf.messages;
    if (!endnum) {
      if (printMessage)
	printf("data port %s: no messages in store to operate on\n", expandPortName(pdp));
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

  // New messages are given negative numbers - if no range allow this

  if (*p && (startnum < 1)) {
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

static PdpDataPort *lookupPort(char *portName, int printMessage)
{
  PdpDataPort *ptk;

  assert(portName);

  // If the port list is empty, nothing to do here
  if (!PORT_LIST || !PORT_LIST->ElementCount()) {
    if (printMessage)
      printf("?No data ports defined\n");
    return((PdpDataPort *)0);
  }

  // lookup the port by name - if the name is registered, return that port 
  ptk = validPortName(portName);
  if (ptk)
    return(ptk);

  // port not registered - say so and return nothing
  if (printMessage)
    printf("?data port %s does not exist\n", portName);
  return((PdpDataPort *)0);
}

static void closePort(PdpDataPort *pdp, int printMessage)
{
  SdmError err;

  assert(pdp);
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);
  dp->Close(err);
  printf("data port %s: closed", expandPortName(pdp));
  printf(err == Sdm_EC_Success ? "\n" : " with error %s\n", detailedErrorReport(err));
}

static void startupPort(PdpDataPort *pdp, int printMessage)
{
  SdmError err;

  assert(pdp);
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);
  dp->StartUp(err);
  dp->RegisterCallback(err, DPCBTYPE_BusyIndication, BusyIndicationCallback, (void *)pdp);
  dp->RegisterCallback(err, DPCBTYPE_ErrorLog, ErrorLogCallback, (void *)pdp);
  dp->RegisterCallback(err, DPCBTYPE_DebugLog, DebugLogCallback, (void *)pdp);
  dp->RegisterCallback(err, DPCBTYPE_LockActivity, LockActivityCallback, (void *)pdp);
  SdmDataPort::RegisterGroupPrivilegeActionCallback(err, GroupPrivilegeActionCallback, (void*)0);
  printf("data port %s: started", expandPortName(pdp));
  printf(err == Sdm_EC_Success ? "\n" : " with error %s\n", detailedErrorReport(err));
}

static void shutdownPort(PdpDataPort *pdp, int printMessage)
{
  SdmError err;

  assert(pdp);
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);
  dp->ShutDown(err);
  printf("data port %s: shutdown", expandPortName(pdp));
  printf(err == Sdm_EC_Success ? "\n" : " with error %s\n", detailedErrorReport(err));
}

static void statusPort(PdpDataPort *pdp, int printMessage)
{
  SdmError err;
  SdmMessageStoreStatus statbuf;

  assert(pdp);
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);
  if (dp->GetMessageStoreStatus(err, statbuf, Sdm_MSS_ALL) != Sdm_EC_Success) {
    printf("data port %s: get message store status failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return;
  }
  printf("data port %s: status information:\n"
	 " flags = 0x%08lx, uidvalidity = 0x%08lx, checksum = %u, checksumbytes = %u\n"
	 " messages = %d, recent = %d, unseen = %d, uidnext = %d\n",
	 expandPortName(pdp), 
	 statbuf.flags, statbuf.uidvalidity, statbuf.checksum, statbuf.checksum_bytes,
	 statbuf.messages, statbuf.recent, statbuf.unseen, statbuf.uidnext);
  return;
}

static void checkfornewmessagesPort(PdpDataPort *pdp, int printMessage)
{
  SdmError err;
  SdmMessageNumber nummessages = 0;

  assert(pdp);
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);
  if (dp->CheckForNewMessages(err, nummessages) != Sdm_EC_Success) {
    printf("data port %s: check for new messages failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
  }
  else {
    printf("data port %s: check for new messages successful:", expandPortName(pdp));
    if (!nummessages)
      printf(" no new messages\n");
    else
      printf(" %d new messages arrived\n", nummessages);
  }
}

static void deletePort(PdpDataPort *pdp, int printMessage)
{
  SdmError err;

  assert(pdp);
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);
  PORT_LIST->RemoveElementFromList(pdp);
  if (dp->Close(err) != Sdm_EC_Success)
    printf("data port %s: warning: close returned error %u\n", expandPortName(pdp), (SdmErrorCode) err);
  if (dp->ShutDown(err) != Sdm_EC_Success)
    printf("data port %s: warning: shutdown returned error %u\n", expandPortName(pdp), (SdmErrorCode) err);
  printf("data port %s: deleted\n", expandPortName(pdp));
  delete pdp;
}

static int extractBodyType(char* type, SdmMsgStrType& r_bodytype)
{
  if (strcasecmp(type, "none")==0)
    r_bodytype = Sdm_MSTYPE_none;
  else if (strcasecmp(type, "text")==0)
    r_bodytype = Sdm_MSTYPE_text;
  else if (strcasecmp(type, "multipart")==0)
    r_bodytype = Sdm_MSTYPE_multipart;
  else if (strcasecmp(type, "message")==0)
    r_bodytype = Sdm_MSTYPE_message;
  else if (strcasecmp(type, "application")==0)
    r_bodytype = Sdm_MSTYPE_application;
  else if (strcasecmp(type, "audio")==0)
    r_bodytype = Sdm_MSTYPE_audio;
  else if (strcasecmp(type, "image")==0)
    r_bodytype = Sdm_MSTYPE_image;
  else if (strcasecmp(type, "video")==0)
    r_bodytype = Sdm_MSTYPE_video;
  else if (strcasecmp(type, "other")==0)
    r_bodytype = Sdm_MSTYPE_other;
  else {
    printf("Error: body type '%s' is not valid; choices are: none, text, multipart, message, application, audio, image, video, other\n", type);
    return(0);
  }
  return(1);
}

static int copymovemessage(int argc, char **argv, SdmBoolean moveFlag)
{
  SdmError err;
  char *command = moveFlag ? "movemessage" : "copymessage";

  if (argc != 3) {
    printf("The %s command takes the form: %s port messagenum token\n",
	   command, command);
    return(TRUE);
  }

  // Extract the port number
  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // If no message number then its an error

  SdmMessageNumber startnum = 0, endnum = 0;
  SdmMessageNumberL messageList;

  assert(argc);
  char *messageSequence = *argv++;
  argc--;
  if (!decodeMessageRange(messageList, startnum, endnum, messageSequence, pdp, TRUE))
    return(TRUE);

  // extract token

  assert(argc);
  char* token = *argv++;
  argc--;

  PtkToken *ptk = lookupToken(token, TRUE);
  if (!ptk)
    return(TRUE);
  SdmToken *tk = ptk->ptk_token;
  assert(tk);

  // Issue the correct command based on the type of message list specified
  int messageListLength = messageList.ElementCount();
  if (!endnum && !messageListLength) {
    // Single message number given in startnum
    if (moveFlag)
      (void) dp->MoveMessage(err, *tk, startnum);
    else
      (void) dp->CopyMessage(err, *tk, startnum);
  } else if (endnum && !messageListLength) {
    // Range given in startnum-endnum
    assert(startnum && endnum);
    if (moveFlag)
      (void) dp->MoveMessages(err, *tk, startnum, endnum);
    else
      (void) dp->CopyMessages(err, *tk, startnum, endnum);
  } else {
    // message list specified
    assert(!startnum && !endnum);
    if (moveFlag)
      (void) dp->MoveMessages(err, *tk, messageList);
    else
      (void) dp->CopyMessages(err, *tk, messageList);
  }

  // Print report of results

  if (err != Sdm_EC_Success) {
    printf("data port %s: %s failed with error %s\n", expandPortName(pdp),
	   command, detailedErrorReport(err));
    return(TRUE);
  }

  printf("data port %s: %s successful\n", expandPortName(pdp),
	 command);
  return(TRUE);
}

static int portSetFlags(int argc, char **argv, SdmBoolean value)
{
  SdmError err;
  SdmMessageFlagAbstractFlags flags = 0;
  SdmMessageNumberL messageList;
  char *command = value ? "setflags" : "clearflags";

  if (argc < 3) {
    printf("?The %s command takes the form: %s port messagenum flags\n",
	   command, command);
    return(TRUE);
  }

  // Extract the port number
  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // If no message number then for all messages return all flags
  long startnum = 0, endnum = 0;

  char *messageSequence = *argv++;
  argc--;
  if (!decodeMessageRange(messageList, startnum, endnum, messageSequence, pdp, TRUE))
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
  if (!endnum && !messageListLength)
    dp->SetMessageFlagValues(err, value, flags, startnum);
  else if (endnum && !messageListLength)
    dp->SetMessageFlagValues(err, value, flags, startnum, endnum);
  else
    dp->SetMessageFlagValues(err, value, flags, messageList);

  if (err != Sdm_EC_Success) {
    printf("data port %s: %s failed with error %s\n", expandPortName(pdp),
	   command, detailedErrorReport(err));
  
    return(TRUE);
  }

  printf("data port %s: %s successful\n", expandPortName(pdp),
	 command);
  return(TRUE);
}

static int subcmd_port_addheader(int argc, char **argv)
{
  SdmMessageNumberL messageList;
  char *headerName;
  SdmError err;
  SdmMessageNumber startnum = 0, endnum = 0;

  if (argc < 4) {
    printf(
"?The addheader command takes this form:\n"
"\tport addheader port messagenum header contents\n"
);
    return(TRUE);
  }

  // Extract the port number
  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // Extract message number

  assert(argc);
  char *messageSequence = *argv++;
  argc--;
  if (!decodeMessageRange(messageList, startnum, endnum, messageSequence, pdp, TRUE))
    return(TRUE);
  int messageListLength = messageList.ElementCount();
  if (!startnum || endnum || messageListLength) {
    printf("?A single message number must be specified - no range,\nlist, or other multiple specifications allowed.\n");
    return(TRUE);
  }

  // Extract the header name

  assert(argc);
  headerName = *argv++;
  argc--;

  // Extract the contents value

  SdmString contentsValue("");

  while (argc) {
    contentsValue += *argv++;
    if (--argc)
      contentsValue += " ";
  }

  assert(!endnum);
  assert(!messageListLength);

  // Only a single message number was specified
  assert(startnum);
  dp->AddMessageHeader(err, headerName, contentsValue, startnum);

  if (err != Sdm_EC_Success) {
    printf("data port %s: addheader failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  printf("data port %s: addheader successful\n", expandPortName(pdp));

  return(TRUE);
}

static int subcmd_port_addmessagebodytomessage(int argc, char **argv)
{
  SdmDpMessageStructureL msgstrL;
  SdmMessageNumberL messageList;
  char *componentName = 0;
  SdmDpMessageComponent *mcmp = 0;
  SdmError err;
  SdmMessageNumber startnum = 0, endnum = 0;
  SdmMsgStrType bodytype = Sdm_MSTYPE_none;
  SdmDpMessageStructure msgstr;

  msgstrL.SetPointerDeleteFlag();		// Force vector contents to be deleted on object destruction

  if (argc < 4 || argc > 5) {
    printf(
"?The addmessagebodytomessage command takes this form:\n"
"\taddmessagebodytomessage port messagenum bodytype bodysubtype [component]\n"
);
    return(TRUE);
  }

  // Extract the port number
  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // Extract message number

  assert(argc);
  char *messageSequence = *argv++;
  argc--;
  if (!decodeMessageRange(messageList, startnum, endnum, messageSequence, pdp, TRUE))
    return(TRUE);
  int messageListLength = messageList.ElementCount();
  if (!startnum || endnum || messageListLength) {
    printf("?A single message number must be specified - no range,\nlist, or other multiple specifications allowed.\n");
    return(TRUE);
  }

  // Extract the body type

  assert(argc);
  if (!extractBodyType(*argv, bodytype))
    return(TRUE);
  argc--;
  argv++;

  // Extract the body subtype

  assert(argc);
  SdmString bodysubtype = *argv++;
  argc--;

  // Extract the component name if present

  if (argc) {
    componentName = *argv++;
    argc--;

    // Retrieve the message structure for this message

    if (dp->GetMessageStructure(err, msgstrL, startnum) != Sdm_EC_Success) {
      printf("data port %s: getstructure failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
      return(TRUE);
    }

    // lookup the message component
    //
    mcmp = lookupMessageComponent(pdp, msgstrL, componentName, TRUE);
    if (!mcmp) {
      printf("data port %s: message component '%s' does not exist in message %d\n", expandPortName(pdp), componentName, startnum);
      return(TRUE);
    }
  }

  assert(!endnum);
  assert(!messageListLength);

  // execute addmessagebodytomessage
  if (componentName) {
    assert(mcmp);
    dp->AddMessageBodyToMessage(err, msgstr, bodytype, bodysubtype, startnum, *mcmp);
  }
  else {
    dp->AddMessageBodyToMessage(err, msgstr, bodytype, bodysubtype, startnum);
  }

  if (err != Sdm_EC_Success) {
    printf("data port %s: addmessagebodytomessage failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  printf("data port %s: addmessagebodytomessage successful: new structure information:\n", expandPortName(pdp));

  // Well, we have a single message body structure that we need to print
  // out the contents of...yuk.

  SdmDpMessageStructureL msL;

  msL(-1) = &msgstr;

  printStructure(msL, "new", TRUE);

  return(TRUE);
}

static int subcmd_port_addtosubscribednamespace(int argc, char **argv)
{
  SdmError err;
  SdmIntStrL namesL;

  if (argc != 2) {
    printf("?The addtosubscribednamespace command takes the form: addtosubscribednamespace port name\n");
    return(TRUE);
  }

  // Extract the port number
  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // Get the new name
  SdmString newname(*argv++); 

  // Issue the add call
  if (dp->AddToSubscribedNamespace(err, newname) != Sdm_EC_Success) {
    printf("data port %s: addtosubscribednamespace failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  printf("data port %s: addtosubscribednamespace successful\n", expandPortName(pdp));
  return(TRUE);
}

static int subcmd_port_clearflags(int argc, char **argv)
{
  return portSetFlags(argc, argv, Sdm_False);
}

static int subcmd_port_commitpendingmessagechanges(int argc, char **argv)
{
  SdmError err;
  long startnum = 0, endnum = 0;
  SdmMessageNumberL messageList;
  SdmBoolean isRewritten;

  if (argc != 2) {
    printf("?The commitpendingmessagechanges command takes the form: commitpendingmessagechanges port messagenum\n");
    return(TRUE);
  }

  // Extract the port number

  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // Extract message number

  assert(argc);
  char *messageSequence = *argv++;
  argc--;
  if (!decodeMessageRange(messageList, startnum, endnum, messageSequence, pdp, TRUE))
    return(TRUE);
  int messageListLength = messageList.ElementCount();
  if (!startnum || endnum || messageListLength) {
    printf("?A single message number must be specified - no range,\nlist, or other multiple specifications allowed.\n");
    return(TRUE);
  }

  // Create the derived message
  assert(!endnum);
  assert(!messageListLength);

  // execute commitpendingmessagechanges
  dp->CommitPendingMessageChanges(err, isRewritten, startnum, Sdm_False);

  if (err != Sdm_EC_Success) {
    printf("data port %s: commitpendingmessagechanges failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  printf("data port %s: commitpendingmessagechanges successful: message was %srewritten\n", expandPortName(pdp), isRewritten ? "" : "NOT ");
  return(TRUE);
}

static int subcmd_port_create(int argc, char **argv)
{
  if (argc != 1) {
    printf("?The create subcommand requires a name argument: port create name \n");
    return(TRUE);
  }

  // See if the port name already exists
  if (lookupPort(*argv, FALSE) != (PdpDataPort *)0) {
    printf("?data port %s already exists\n", *argv);
    return(TRUE);
  }

  // Name is unique - create port with it
  SdmString *name = new SdmString(*argv);
  assert(name);
  PdpDataPort *pdp = new PdpDataPort();
  assert(pdp);
  SdmDataPort *dp = new SdmDataPort(0);
  assert(dp);
  if (!PORT_LIST)
    PORT_LIST = new SdmLinkedList();
  pdp->pdp_dataport = dp;
  pdp->pdp_name = name;
  PORT_LIST->AddElementToList(pdp);
  printf("data port %s: defined\n", expandPortName(pdp));
  return(TRUE);
}

static int subcmd_port_copymessage(int argc, char **argv)
{
  return(copymovemessage(argc, argv, Sdm_False));
}

static int subcmd_port_close(int argc, char **argv)
{
  if (!argc) {
    printf("?The close subcommand takes one or more arguments which specify\n?the ports to be closed; to close all ports, use a single * character.\n");
    return(TRUE);
  }
  while (argc--) {
    char *currentPort = *argv++;
    if (strcmp(currentPort,"*")==0) {
      SdmLinkedListIterator lit(PORT_LIST);
      PdpDataPort *pdp;

      while (pdp = (PdpDataPort *)(lit.NextListElement())) {
	closePort(pdp, TRUE);
      }
    }
    else {
      PdpDataPort *pdp = lookupPort(currentPort, TRUE);
      if (pdp)
	closePort(pdp, TRUE);
    }
  }
  return(TRUE);
}

static int subcmd_port_delete(int argc, char **argv)
{
  if (!argc) {
    printf("?The delete subcommand takes one or more arguments which specify\n?the ports to be deleted; to delete all ports, use a single * character.\n");
    return(TRUE);
  }
  while (argc--) {
    char *currentPort = *argv++;
    if (strcmp(currentPort,"*")==0) {
      SdmLinkedListIterator lit(PORT_LIST);
      PdpDataPort *pdp;

      while (pdp = (PdpDataPort *)(lit.NextListElement())) {
	deletePort(pdp, TRUE);
      }
    }
    else {
      PdpDataPort *pdp = lookupPort(currentPort, TRUE);
      if (pdp)
	deletePort(pdp, TRUE);
    }
  }
  return(TRUE);
}

static int subcmd_port_getflags(int argc, char **argv)
{
  SdmError err;
  SdmMessageFlagAbstractFlags flags;
  SdmMessageFlagAbstractFlagsL flagsL;
  SdmMessageNumberL messageList;

  if (argc < 1 || argc > 2) {
    printf("?The getflags command takes the form: getflags [messagenum]\n");
    return(TRUE);
  }

  // Extract the port number
  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // If no message number then for all messages return all flags
  long startnum = 0, endnum = 0;

  if (!argc) {
    if (!decodeMessageRange(messageList, startnum, endnum, "*", pdp, TRUE))
      return(TRUE);
  }
  else {
    char *messageSequence = *argv++;
    argc--;
    if (!decodeMessageRange(messageList, startnum, endnum, messageSequence, pdp, TRUE))
      return(TRUE);
  }

  if (argc) {
    printf("?Babble at end of command: %s\n", *argv);
    return(TRUE);
  }

  int messageListLength = messageList.ElementCount();
  if (!endnum && !messageListLength)
    dp->GetMessageFlags(err, flags, startnum);
  else if (endnum && !messageListLength)
    dp->GetMessageFlags(err, flagsL, startnum, endnum);
  else
    dp->GetMessageFlags(err, flagsL, messageList);

  if (err != Sdm_EC_Success) {
    printf("data port %s: getflags failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  printf("data port %s: getflags successful\n", expandPortName(pdp));

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

static int subcmd_port_getattributes(int argc, char **argv)
{
  SdmString attr;		// holds returned attr list
  SdmStringL attrList;	// holds returned attr list of lists
  SdmBoolean attrListSet = Sdm_False;	// indicates if attrList is set
  SdmBoolean attrSet = Sdm_False;	// indicates if attr is set
  SdmDpMessageStructureL msgstrL;
  SdmMessageAttributeAbstractFlags attrAbstractFlags;
  SdmMessageNumberL messageList;
  char *componentName;
  SdmDpMessageComponent *mcmp = 0;
  int abstractFlag = 0;
  SdmError err;

  msgstrL.SetPointerDeleteFlag();		// Force vector contents to be deleted on object destruction

  if (argc < 3) {
    printf(
"?The getattributes command takes several forms:"
"\tgetattributes port messagenum component\n"
"\tgetattributes port messagenum component attribute(s)\n"
"\tgetattributes port messagenum component -a \n"
"\tgetattributes port messagenum component -a abstractattribute(s)\n"
"\tThe -a flag indicates to retrieve attributes s by abstract attribute names\n"
"\tas opposed to attribute name strings themselves.\n"
);
    return(TRUE);
  }

  // Extract the port number
  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // If no message number then for all messages return all headers
  SdmMessageNumber startnum = 0, endnum = 0;

  assert(argc);
  char *messageSequence = *argv++;
  argc--;
  if (!decodeMessageRange(messageList, startnum, endnum, messageSequence, pdp, TRUE))
    return(TRUE);
  int messageListLength = messageList.ElementCount();
  if (!startnum || endnum || messageListLength) {
    printf("?A single message number must be specified - no range,\nlist, or other multiple specifications allowed.\n");
    return(TRUE);
  }

  // Extract the component name
  assert(argc);
  componentName = *argv++;
  argc--;

  // Extract -a
  if (argc && strcmp(*argv, "-a")==0) {
    abstractFlag++; 
    argc--;
    argv++;
  }

  // Ok, anything left is either a message attribute flag value or name sequence
  if (!argc) {
    if (abstractFlag)
      attrAbstractFlags = Sdm_MAA_ALL;
  }
  else if (argc == 1) {
    char *arg = *argv++;
    assert(arg);
    argc--;
    if (abstractFlag) {
      if (!decodeAttributeAbstractValue(attrAbstractFlags, arg, TRUE))
	return(TRUE);
    }
    else {
      attr = arg;
      attrSet = Sdm_True;
    }
  } else if (argc > 1) {
    if (abstractFlag) {
      while (argc--) {
	SdmMessageAttributeAbstractFlag x = 0;
	if (!decodeAttributeAbstractValue(x, *argv++, TRUE))
	  return(TRUE);
	attrAbstractFlags |= x;
      }
    }
    else {
      attrList.ClearAllElements();
      while (argc--) {
	attrList(-1) = *argv++;
      }
      attrListSet = Sdm_True;
    }
  }
    
  // If any attrs are specified, produce list
  // At this point have a message range of some kind, deal with it
  // Issues:
  //	is -a set?
  //	attr list specified?

  // GetMessageAttributes(SdmError &err, SdmStrStrL &r_attribute, SdmMessageNumber &msgnum, SdmDpMessageComponent &mcmp);
  // GetMessageAttribute(SdmError &err, SdmString &r_attribute, SdmString &attribute, SdmMessageNumber &msgnum, SdmDpMessageComponent &mcmp);
  // GetMessageAttributes(SdmError &err, SdmStrStrL &r_attribute, SdmStringL &attributes, SdmMessageNumber &msgnum, SdmDpMessageComponent &mcmp);
  // GetMessageAttributes(SdmError &err, SdmStrStrL &r_attribute, SdmMessageAttributeAbstractFlags &attribute, SdmMessageNumber &msgnum, SdmDpMessageComponent &mcmp);

  // If !endnum then a single message is requested
  SdmString attrValue;
  SdmStrStrL attrValueL;

  assert(!endnum);
  assert(!messageListLength);

  // Retrieve the message structure for this message

  if (dp->GetMessageStructure(err, msgstrL, startnum) != Sdm_EC_Success) {
    printf("data port %s: getstructure failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  // lookup the message component
  //
  mcmp = lookupMessageComponent(pdp, msgstrL, componentName, TRUE);
  if (!mcmp) {
    printf("data port %s: message component '%s' does not exist in message %d\n", expandPortName(pdp), componentName, startnum);
    return(TRUE);
  }

  // Only a single message number was specified
  assert(startnum);
  if (abstractFlag) {
    // getattributes port -a msg abs
    dp->GetMessageAttributes(err, attrValueL, attrAbstractFlags, startnum, *mcmp);
    ptLastResult = attrValueL;
  }
  else if (!attrSet && !attrListSet) {
    // getattributes port msg - get all attrs for a single message
    dp->GetMessageAttributes(err, attrValueL, startnum, *mcmp);
    ptLastResult = attrValueL;
  }
  else if (attrSet && !attrListSet) {
    // getattributes port msg hdr - get a single attr for a single message
    dp->GetMessageAttribute(err, attrValue, attr, startnum, *mcmp);
    ptLastResult = attrValue;
  }
  else if (!attrSet && attrListSet) {
    // getattributes port msg hdr1 hdr2
    dp->GetMessageAttributes(err, attrValueL, attrList, startnum, *mcmp);
    ptLastResult = attrValueL;
  }

  if (err != Sdm_EC_Success) {
    printf("data port %s: getattributes failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  printf("data port %s: getattributes successful\n", expandPortName(pdp));

  if (attrSet && !attrListSet && !abstractFlag) {
    printf(" message %d component %s: 1 attribute returned\n", startnum, componentName);
    printf("  %15s: %s\n", (const char *)attr, (const char *)attrValue);
  } else {
    int i = attrValueL.ElementCount();
    printf(" message %d component %s: %d attributes returned\n", startnum, componentName, i);
    for (int j = 0; j < i; j++)
      printf("  %15s: %s\n",
	     (const char *)attrValueL[j].GetFirstString() ? (const char *)attrValueL[j].GetFirstString() : "(NULL)",
	     (const char *)attrValueL[j].GetSecondString() ? (const char *)attrValueL[j].GetSecondString() : "(NULL)");
  }

  return(TRUE);
}

static int subcmd_port_getcontents(int argc, char **argv)
{
  SdmMessageNumberL messageList;
  char *componentName = 0;
  SdmDpMessageComponent *mcmp = 0;
  SdmContentBuffer theContents;
  SdmDpContentType ctype = DPCTTYPE_processedcomponent;
  SdmDpMessageStructureL msgstrL;
  char *outputFileName = 0;
  SdmBoolean outputRedirected = Sdm_False;
  FILE* outputFd = 0;
  SdmBoolean produceSum = Sdm_False;
  SdmBoolean produceHexdump = Sdm_False;

  int abstractFlag = 0;
  SdmError err;

  msgstrL.SetPointerDeleteFlag();		// Force vector contents to be deleted on object destruction

  if (argc < 2) {
    printf(
	   "?The getcontents command takes one form: getcontents port messagenum [-p | -f | -h | -b] [-F <outputfile>] [-S] [-H] [component]\n"
	   "If the component is unspecified, the request is directed towards 'the entire message'\n"
	   "Normal retrieval is 'fully processed component'; optional flags for get:\n"
	   "-F - place results of getcontents in specified file\n"
	   "     instead of displaying to standard output\n"
	   "-S - produce sum of results\n"
	   "-H - produce a hex dump of results instead of default raw output\n"
	   "-b - get 'raw body'\n"
	   "-f - get 'raw full'\n"
	   "-h - get 'raw header'\n"
	   "-p - get 'processed component' [default]\n"
	   );
    return(TRUE);
  }

  // Extract the port number
  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // If no message number then for all messages return all headers
  SdmMessageNumber startnum = 0, endnum = 0;

  assert(argc);
  char *messageSequence = *argv++;
  argc--;
  if (!decodeMessageRange(messageList, startnum, endnum, messageSequence, pdp, TRUE))
    return(TRUE);
  int messageListLength = messageList.ElementCount();
  if (!startnum || endnum || messageListLength) {
    printf("?A single message number must be specified - no range,\nlist, or other multiple specifications allowed.\n");
    return(TRUE);
  }

  while (argc && *argv[0] == '-') {
    // Extract optional get flags
    if (argc && strcmp(*argv, "-f")==0) {
      ctype = DPCTTYPE_rawfull;
      argc--;
      argv++;
      continue;
    }
    if (argc && strcmp(*argv, "-h")==0) {
      ctype = DPCTTYPE_rawheader;
      argc--;
      argv++;
      continue;
    }
    if (argc && strcmp(*argv, "-b")==0) {
      ctype = DPCTTYPE_rawbody;
      argc--;
      argv++;
      continue;
    }
    if (argc && strcmp(*argv, "-p")==0) {
      ctype = DPCTTYPE_processedcomponent;
      argc--;
      argv++;
      continue;
    }
    if (argc && strcmp(*argv, "-S")==0) {
      produceSum = Sdm_True;
      argc--;
      argv++;
      continue;
    }
    if (argc && strcmp(*argv, "-H")==0) {
      produceHexdump = Sdm_True;
      argc--;
      argv++;
      continue;
    }
    if (argc && strcmp(*argv, "-F")==0) {
      argc--;
      argv++;
      if (!argc) {
	printf("?-F must be followed by the name of the file to place the output in\n");
	return(TRUE);
      }
      outputFileName = *argv++;
      outputRedirected = Sdm_True;
      outputFd = fopen(outputFileName, "w");
      if (outputFd == NULL) {
	printf("?Cant create -F output file '%s': %s\n", outputFileName, strerror(errno));
	return(TRUE);
      }
      argc--;
      continue;
    }
    break;
  }

  // Extract the component name - it is optional
  if (argc) {
    componentName = *argv++;
    argc--;

    // Retrieve the message structure for this message

    if (dp->GetMessageStructure(err, msgstrL, startnum) != Sdm_EC_Success) {
      printf("data port %s: getstructure failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
      if (outputFd)
	fclose(outputFd);
      return(TRUE);
    }

    // lookup the message component
    //
    mcmp = lookupMessageComponent(pdp, msgstrL, componentName, TRUE);
    if (!mcmp) {
      printf("data port %s: message component '%s' does not exist in message %d\n", expandPortName(pdp), componentName, startnum);
      if (outputFd)
	fclose(outputFd);
      return(TRUE);
    }
  }

  if (argc) {
    printf("?Babble at end of command: %s\n", *argv);
    if (outputFd)
      fclose(outputFd);
    return(TRUE);
  }

  // If !endnum then a single message is requested
  assert(!endnum);
  assert(!messageListLength);

  // Only a single message number was specified
  assert(startnum);

  if (!mcmp) {
    // For the whole message
    dp->GetMessageContents(err, theContents, ctype, startnum);
  } 
  else {
    // For a component of the message
    dp->GetMessageContents(err, theContents, ctype, startnum, *mcmp);
  }

  if (err != Sdm_EC_Success) {
    printf("data port %s: getcontents failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    if (outputFd)
      fclose(outputFd);
    return(TRUE);
  }

  printf("data port %s: getcontents successful\n", expandPortName(pdp));

  SdmString str_contents;
  if (theContents.GetContents(err, str_contents)) {
    printf("data port %s: getcontents failed for content buffer with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    if (outputFd)
      fclose(outputFd);
    return(TRUE);
  }

  unsigned first = 0;
  long second;

  if (produceSum) {
    computeSum(str_contents, first, second);
  }

  if (!mcmp) {
    if (outputRedirected) {
      if (produceSum)
	printf("-->message %d: written to %s sum is %u %ld\n", startnum, outputFileName, first, second);
      else
	printf("-->message %d: written to %s\n", startnum, outputFileName);
      if (produceHexdump)
	HexDump(outputFd, "hex dump of message", (const unsigned char *)((const char *)str_contents), str_contents.Length(), 0, TRUE);
      else
	fwrite((const char *)str_contents, str_contents.Length(), 1, outputFd);
      fclose(outputFd);
    }
    else {
      if (produceSum)
	printf("-->message %d: sum is %u %ld\n", startnum, first, second);
      else
	printf("-->message %d: \n", startnum);
      if (produceHexdump)
	HexDump(stdout, "hex dump of message", (const unsigned char *)((const char *)str_contents), str_contents.Length(), 0, TRUE);
      else
	printf("%s\n", (const char *)str_contents);
    }
  }
  else {
    assert(componentName);
    if (outputRedirected) {
      if (produceSum)
	printf("-->message %d component %s: written to %s sum is %u %ld\n", startnum, componentName, outputFileName, first, second);
      else
	printf("-->message %d component %s: written to %s\n", startnum, componentName, outputFileName);
      if (produceHexdump)
	HexDump(outputFd, "hex dump of message component", (const unsigned char *)((const char *)str_contents), str_contents.Length(), 0, TRUE);
      else
	fwrite((const char *)str_contents, str_contents.Length(), 1, outputFd);
      fclose(outputFd);
    }
    else {
      if (produceSum)
	printf("-->message %d component %s: sum is %u %ld\n", startnum, componentName, first, second);
      else
	printf("-->message %d component %s: \n", startnum, componentName);
      if (produceHexdump)
	HexDump(stdout, "hex dump of message component", (const unsigned char *)((const char *)str_contents), str_contents.Length(), 0, TRUE);
      else
	printf("%s\n", (const char *)str_contents);
    }
  }
  return(TRUE);
}

static int subcmd_port_getstructure(int argc, char **argv)
{
  SdmError err;
  SdmDpMessageStructureL msgstrL;
  SdmDpMessageStructureLL msgstrLL;
  SdmMessageNumberL messageList;

  msgstrL.SetPointerDeleteFlag();		// Force vector contents to be deleted on object destruction
  msgstrLL.SetPointerDeleteFlag();		// Force vector contents to be deleted on object destruction

  if (argc < 1 || argc > 2) {
    printf("?The getstructure command takes the form: getstructure [messagenum]\n");
    return(TRUE);
  }

  // Extract the port number
  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // If no message number then for all messages return all structures
  long startnum = 0, endnum = 0;

  if (!argc) {
    if (!decodeMessageRange(messageList, startnum, endnum, "*", pdp, TRUE))
      return(TRUE);
  }
  else {
    char *messageSequence = *argv++;
    argc--;
    if (!decodeMessageRange(messageList, startnum, endnum, messageSequence, pdp, TRUE))
      return(TRUE);
  }

  int messageListLength = messageList.ElementCount();
  if (!endnum && !messageListLength)
    dp->GetMessageStructure(err, msgstrL, startnum);
  else if (endnum && !messageListLength)
    dp->GetMessageStructure(err, msgstrLL, startnum, endnum);
  else
    dp->GetMessageStructure(err, msgstrLL, messageList);

  if (err != Sdm_EC_Success) {
    printf("data port %s: getstructure failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  printf("data port %s: getstructure successful\n", expandPortName(pdp));

  if (!endnum && !messageListLength) {
    printf(" message %d:\n", startnum);
    printStructure(msgstrL, "0", TRUE);
  }
  else if (endnum && !messageListLength) {
    int numMessages = (endnum-startnum)+1;
    for (int k = 0; k < numMessages; k++) {
      printf(" message %d:\n", k+startnum);
      printStructure(*msgstrLL[k], "0", k==0);
      msgstrLL[k]->ClearAndDestroyAllElements();
    }
  } else {
    int numMessages = messageListLength;
    for (int k = 0; k < numMessages; k++) {
      printf(" message %d:\n", messageList[k]);
      printStructure(*msgstrLL[k], "0", k==0);
      msgstrLL[k]->ClearAndDestroyAllElements();
    }
  }

  return(TRUE);
}

static int subcmd_port_getheaders(int argc, char **argv)
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
"\tgetheaders port\n"
"\tgetheaders port messagenum\n"
"\tgetheaders port messagenum -a|-A\n"
"\tgetheaders port messagenum header(s)\n"
"\tgetheaders port messagenum -a|-A abstractheader(s)\n"
"\tThe -a flag indicates to retrieve headers by abstract header names\n"
"\tas opposed to header name strings themselves.\n"
"\tThe -A flag is identical to -a but additionally retrieves the headers by abstract\n"
"\theader name/header value pairs as opposed to header name/header value pairs.\n"
);
    return(TRUE);
  }

  // Extract the port number
  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // If no message number then for all messages return all headers
  SdmMessageNumber startnum = 0, endnum = 0;

  if (!argc) {
    if (!decodeMessageRange(messageList, startnum, endnum, "*", pdp, TRUE))
      return(TRUE);
  }
  else {
    char *messageSequence = *argv++;
    argc--;
    if (!decodeMessageRange(messageList, startnum, endnum, messageSequence, pdp, TRUE))
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

  // *GetMessageHeaders(SdmError &err, SdmStrStrL &r_hdr, SdmMessageNumber &msgnum);
  // *GetMessageHeaders(SdmError &err, SdmStrStrLL &r_hdr, SdmMessageNumber &startmsgnum, SdmMessageNumber &endmsgnum);
  // GetMessageHeaders(SdmError &err, SdmStrStrLL &r_hdr, SdmMessageNumberL &msgnums);
  // *GetMessageHeader(SdmError &err, SdmStrStrL &r_hdr, SdmString &hdr, SdmMessageNumber &msgnum);
  // *GetMessageHeader(SdmError &err, SdmStrStrLL &r_hdr, SdmString &hdr, SdmMessageNumber &startmsgnum, SdmMessageNumber &endmsgnum);
  // GetMessageHeader(SdmError &err, SdmStrStrLL &r_hdr, SdmString &hdr, SdmMessageNumberL &msgnums);
  // *GetMessageHeaders(SdmError &err, SdmStrStrL &r_hdrs, SdmStringL &hdrs, SdmMessageNumber &msgnum);
  // *GetMessageHeaders(SdmError &err, SdmStrStrLL &r_hdrs, SdmStringL &hdrs, SdmMessageNumber &startmsgnum, SdmMessageNumber &endmsgnum);
  // GetMessageHeaders(SdmError &err, SdmStrStrLL &r_hdrs, SdmStringL &hdrs, SdmMessageNumberL &msgnums);
  // *GetMessageHeader(SdmError &err, SdmStrStrL &r_hdr, SdmMessageHeaderAbstractFlags &hdr, SdmMessageNumber &msgnum);
  // *GetMessageHeader(SdmError &err, SdmStrStrLL &r_hdr, SdmMessageHeaderAbstractFlags &hdr, SdmMessageNumber &startmsgnum, SdmMessageNumber &endmsgnum);
  // GetMessageHeader(SdmError &err, SdmStrStrLL &r_hdr, SdmMessageHeaderAbstractFlags &hdr, SdmMessageNumberL &msgnums);

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
	// getheaders port -A msg abs
	dp->GetMessageHeaders(err, headerValueA, headerAbstractFlags, startnum);
        ptLastResult = headerValueA;
      }
      else {
	// getheaders port -a msg abs
	dp->GetMessageHeaders(err, headerValue, headerAbstractFlags, startnum);
        ptLastResult = headerValue;
      }
    }
    else if (!headerSet && !headerListSet) {
      // getheaders port msg - get all headers for a single message
      dp->GetMessageHeaders(err, headerValue, startnum);
      ptLastResult = headerValue;
    }
    else if (headerSet && !headerListSet) {
      // getheaders port msg hdr - get a single header for a single message
      dp->GetMessageHeader(err, headerValue, header, startnum);
      ptLastResult = headerValue;
    }
    else if (!headerSet && headerListSet) {
      // getheaders port msg hdr1 hdr2
      dp->GetMessageHeaders(err, headerValue, headerList, startnum);
      ptLastResult = headerValue;
    }
  }
  else if (endnum && !messageListLength) {
    // A range was specified
    assert(startnum);
    if (abstractFlag) {
      if (abstractRetrieveFlag) {
	// getheaders port -A msg abs - get abstract headers for a range
	dp->GetMessageHeaders(err, headerValueAL, headerAbstractFlags, startnum, endnum);
        ptLastResult = headerValueAL;
      }
      else {
	// getheaders port -a msg abs - get abstract headers for a range
	dp->GetMessageHeaders(err, headerValueL, headerAbstractFlags, startnum, endnum);
        ptLastResult = headerValueL;
      }
    }
    else if (!headerSet && !headerListSet) {
      // getheaders port msg - get all headers for a range
      dp->GetMessageHeaders(err, headerValueL, startnum, endnum);
      ptLastResult = headerValueL;
    }
    else if (headerSet && !headerListSet) {
      // getheaders port msg hdr - get a single header for a range
      dp->GetMessageHeader(err, headerValueL, header, startnum, endnum);
      ptLastResult = headerValueL;
    }
    else if (!headerSet && headerListSet) {
      // getheaders port msg hdr1 hdr2
      dp->GetMessageHeaders(err, headerValueL, headerList, startnum, endnum);
      ptLastResult = headerValueL;
    }
  }
  else {
    // A message list was specified
    assert(!startnum && !endnum);
    if (abstractFlag) {
      if (abstractRetrieveFlag) {
	// getheaders port -A msg abs - get abstract headers for a range
	dp->GetMessageHeaders(err, headerValueAL, headerAbstractFlags, messageList);
        ptLastResult = headerValueAL;
      }
      else {
	// getheaders port -a msg abs - get abstract headers for a range
	dp->GetMessageHeaders(err, headerValueL, headerAbstractFlags, messageList);
        ptLastResult = headerValueL;
      }
    }
    else if (!headerSet && !headerListSet) {
      // getheaders port msg - get all headers for a range
      dp->GetMessageHeaders(err, headerValueL, messageList);
      ptLastResult = headerValueL;
    }
    else if (headerSet && !headerListSet) {
      // getheaders port msg hdr - get a single header for a range
      dp->GetMessageHeader(err, headerValueL, header, messageList);
      ptLastResult = headerValueL;
    }
    else if (!headerSet && headerListSet) {
      // getheaders port msg hdr1 hdr2
      dp->GetMessageHeaders(err, headerValueL, headerList, messageList);
      ptLastResult = headerValueL;
    }
  }

  if (err != Sdm_EC_Success) {
    printf("data port %s: getheaders failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  printf("data port %s: getheaders successful\n", expandPortName(pdp));

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

static int subcmd_port_checkfornewmessages(int argc, char **argv)
{
  if (!argc) {
    printf("?The checkfornewmessages subcommand takes one or more arguments which specify\n?the ports to be checked; to check all ports, use a single * character.\n");
    return(TRUE);
  }
  while (argc--) {
    char *currentPort = *argv++;
    if (strcmp(currentPort,"*")==0) {
      SdmLinkedListIterator lit(PORT_LIST);
      PdpDataPort *pdp;

      while (pdp = (PdpDataPort *)(lit.NextListElement())) {
	checkfornewmessagesPort(pdp, TRUE);
      }
    }
    else {
      PdpDataPort *pdp = lookupPort(currentPort, TRUE);
      if (pdp)
	checkfornewmessagesPort(pdp, TRUE);
    }
  }
  return(TRUE);
}

static int subcmd_port_removefromsubscribednamespace(int argc, char **argv)
{
  SdmError err;
  SdmIntStrL namesL;

  if (argc != 2) {
    printf("?The removefromsubscribednamespace command takes the form: removefromsubscribednamespace port name\n");
    return(TRUE);
  }

  // Extract the port number
  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // Get the new name
  SdmString newname(*argv++); 

  // Issue the add call
  if (dp->RemoveFromSubscribedNamespace(err, newname) != Sdm_EC_Success) {
    printf("data port %s: removefromsubscribednamespace failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  printf("data port %s: removefromsubscribednamespace successful\n", expandPortName(pdp));
  return(TRUE);
}

static int subcmd_port_removeheader(int argc, char **argv)
{
  SdmMessageNumberL messageList;
  char *headerName;
  SdmError err;
  SdmMessageNumber startnum = 0, endnum = 0;

  if (argc != 3) {
    printf(
"?The removeheader command takes this form:\n"
"\tport removeheader port messagenum header\n"
);
    return(TRUE);
  }

  // Extract the port number
  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // Extract message number

  assert(argc);
  char *messageSequence = *argv++;
  argc--;
  if (!decodeMessageRange(messageList, startnum, endnum, messageSequence, pdp, TRUE))
    return(TRUE);
  int messageListLength = messageList.ElementCount();
  if (!startnum || endnum || messageListLength) {
    printf("?A single message number must be specified - no range,\nlist, or other multiple specifications allowed.\n");
    return(TRUE);
  }

  // Extract the header name

  assert(argc);
  headerName = *argv++;
  argc--;

  assert(!endnum);
  assert(!messageListLength);

  // Only a single message number was specified
  assert(startnum);
  dp->RemoveMessageHeader(err, headerName, startnum);

  if (err != Sdm_EC_Success) {
    printf("data port %s: removeheader failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  printf("data port %s: removeheader successful\n", expandPortName(pdp));

  return(TRUE);
}

static int subcmd_port_replaceheader(int argc, char **argv)
{
  SdmMessageNumberL messageList;
  char *headerName;
  SdmError err;
  SdmMessageNumber startnum = 0, endnum = 0;

  if (argc < 4) {
    printf(
"?The replaceheader command takes this form:\n"
"\tport replaceheader port messagenum header contents\n"
);
    return(TRUE);
  }

  // Extract the port number
  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // Extract message number

  assert(argc);
  char *messageSequence = *argv++;
  argc--;
  if (!decodeMessageRange(messageList, startnum, endnum, messageSequence, pdp, TRUE))
    return(TRUE);
  int messageListLength = messageList.ElementCount();
  if (!startnum || endnum || messageListLength) {
    printf("?A single message number must be specified - no range,\nlist, or other multiple specifications allowed.\n");
    return(TRUE);
  }

  // Extract the header name

  assert(argc);
  headerName = *argv++;
  argc--;

  // Extract the contents value

  SdmString contentsValue("");

  while (argc) {
    contentsValue += *argv++;
    if (--argc)
      contentsValue += " ";
  }

  assert(!endnum);
  assert(!messageListLength);

  // Only a single message number was specified
  assert(startnum);
  dp->ReplaceMessageHeader(err, headerName, contentsValue, startnum);

  if (err != Sdm_EC_Success) {
    printf("data port %s: replaceheader failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  printf("data port %s: replaceheader successful\n", expandPortName(pdp));

  return(TRUE);
}

static int subcmd_port_resolveandextractaddresses(int argc, char **argv)
{
  SdmError err;
  long startnum = 0, endnum = 0;
  SdmMessageNumberL messageList;
  SdmBoolean isRewritten;
  SdmStringL addressList;
  int addressListCount = 0;
  SdmStringL filenameList;
  int filenameListCount = 0;
  SdmBoolean writeFiles = Sdm_False;

  if (argc < 3 || argc > 4) {
    printf(
	   "?The resolveandextractaddresses command takes the form: resolveandextractaddresses port srcport messagenum [-f]\n"
	   "The -f flag means write messages that are destined for files if any are indicated\n"
	   );
    return(TRUE);
  }

  // Extract the port number

  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // Extract the source port number

  PdpDataPort *srcpdp = lookupPort(*argv, TRUE);
  if (!srcpdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *srcdp = srcpdp->pdp_dataport;
  assert(srcdp);

  // Extract message number

  assert(argc);
  char *messageSequence = *argv++;
  argc--;
  if (!decodeMessageRange(messageList, startnum, endnum, messageSequence, pdp, TRUE))
    return(TRUE);
  int messageListLength = messageList.ElementCount();
  if (!startnum || endnum || messageListLength) {
    printf("?A single message number must be specified - no range,\nlist, or other multiple specifications allowed.\n");
    return(TRUE);
  }

  // Extract optional get flags
  if (argc && strcmp(*argv, "-f")==0) {
    writeFiles = Sdm_True;
    argc--;
    argv++;
  }

  // Create the derived message
  assert(!endnum);
  assert(!messageListLength);

  // execute resolveandextractaddresses
  dp->ResolveAndExtractAddresses(err, addressList, filenameList, *srcdp, startnum);

  if (err != Sdm_EC_Success) {
    printf("data port %s: resolveandextractaddresses failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  addressListCount = addressList.ElementCount();
  filenameListCount = filenameList.ElementCount();

  printf("data port %s: resolveandextractaddresses successful: %d addresses, %d files\n", 
	 expandPortName(pdp), addressListCount, filenameListCount);
  if (addressListCount) {
    printf("  %d Addresses:\t", addressListCount);
    for (int i = 0; i < addressListCount; i++) {
      printf("%s[%3d] %s\n", i == 0 ? "" : "\t\t", i+1, (const char *)addressList[i]);
    }
  }

  // If any files are specified, list them out

  if (filenameListCount) {
    printf("  %d Filenames:\t", filenameListCount);
    for (int i = 0; i < filenameListCount; i++) {
      printf("%s[%3d] %s\n", i == 0 ? "" : "\t\t", i+1, (const char *)filenameList[i]);
    }
  }

  // If -f flag is present and any files are specified, cause the messages to be files

  if (writeFiles && filenameListCount) {
    SdmString theHeaders, theBody;
    SdmError localError;
    printf("  Writing message to %d file%s:\n", filenameListCount, filenameListCount == 1 ? "" : "s");
    if (SdmMessageUtility::ConstructWholeMessage(localError, theHeaders, theBody, *srcdp, startnum, Sdm_False, Sdm_False, Sdm_True) != Sdm_EC_Success) {
      printf("  ConstructWholeMessage failed with error %s\n", detailedErrorReport(localError));
    } else {
      SdmError localError1;
      for (int i = 0; i < filenameListCount; i++) {
	printf("  %s: ", (const char *)filenameList[i]);
	if (SdmMessageUtility::AppendMessageToFile(localError1, filenameList[i], theHeaders, theBody) != Sdm_EC_Success)
	  printf("failed with error %s\n", detailedErrorReport(localError1));
	else
	  printf("succeeded\n");
      }
    }
  }

  return(TRUE);
}

static int subcmd_port_scannamespace(int argc, char **argv)
{
  SdmError err;
  SdmSortIntStrL namesL;

  if (argc !=3) {
    printf("?The scannamespace command takes the form: scannamespace port reference pattern\n");
    return(TRUE);
  }

  // Extract the port number
  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // Get the reference and pattern arguments
  SdmString reference(*argv++);
  SdmString pattern(*argv++); 

  // Issue the scan call
  if (dp->ScanNamespace(err, namesL, reference, pattern) != Sdm_EC_Success) {
    printf("data port %s: scannamespace failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  int numEntries = namesL.ElementCount();
  printf("data port %s: scannamespace successful, returned %d names\n", expandPortName(pdp), numEntries);

  for (int i = 0; i < numEntries; i ++)
    printf("\t%s\t%s\n", printNamespaceFlag(namesL[i].GetNumber()), (const char *)namesL[i].GetString());
  return(TRUE);
}

static int subcmd_port_scansubscribednamespace(int argc, char **argv)
{
  SdmError err;
  SdmIntStrL namesL;

  if (argc !=3) {
    printf("?The scansubscribednamespace command takes the form: scansubscribednamespace port reference pattern\n");
    return(TRUE);
  }

  // Extract the port number
  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // Get the reference and pattern arguments
  SdmString reference(*argv++);
  SdmString pattern(*argv++); 

  // Issue the scan call
  if (dp->ScanSubscribedNamespace(err, namesL, reference, pattern) != Sdm_EC_Success) {
    printf("data port %s: scansubscribednamespace failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  int numEntries = namesL.ElementCount();
  printf("data port %s: scansubscribednamespace successful, returned %d names\n", expandPortName(pdp), numEntries);

  for (int i = 0; i < numEntries; i ++)
    printf("\t%s\t%s\n", printNamespaceFlag(namesL[i].GetNumber()), (const char *)namesL[i].GetString());
  return(TRUE);
}

static int subcmd_port_setattributes(int argc, char **argv)
{
  SdmDpMessageStructureL msgstrL;
  SdmMessageNumberL messageList;
  char *componentName;
  SdmDpMessageComponent *mcmp = 0;
  SdmError err;
  SdmMessageNumber startnum = 0, endnum = 0;

  msgstrL.SetPointerDeleteFlag();		// Force vector contents to be deleted on object destruction

  if (argc < 4) {
    printf(
"?The setattributes command takes this form:\n"
"\tsetattributes port messagenum component attribute-name [attribute-value]\n"
);
    return(TRUE);
  }

  // Extract the port number
  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // Extract message number

  assert(argc);
  char *messageSequence = *argv++;
  argc--;
  if (!decodeMessageRange(messageList, startnum, endnum, messageSequence, pdp, TRUE))
    return(TRUE);
  int messageListLength = messageList.ElementCount();
  if (!startnum || endnum || messageListLength) {
    printf("?A single message number must be specified - no range,\nlist, or other multiple specifications allowed.\n");
    return(TRUE);
  }

  // Extract the component name

  assert(argc);
  componentName = *argv++;
  argc--;

  // Extract the attribute name

  SdmString attributeName(*argv++);
  argc--;

  // Extract the attribute value

  SdmString attributeValue("");

  while (argc) {
    attributeValue += *argv++;
    if (--argc)
      attributeValue += " ";
  }

  assert(!endnum);
  assert(!messageListLength);

  // Retrieve the message structure for this message

  if (dp->GetMessageStructure(err, msgstrL, startnum) != Sdm_EC_Success) {
    printf("data port %s: getstructure failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  // lookup the message component
  //
  mcmp = lookupMessageComponent(pdp, msgstrL, componentName, TRUE);
  if (!mcmp) {
    printf("data port %s: message component '%s' does not exist in message %d\n", expandPortName(pdp), componentName, startnum);
    return(TRUE);
  }

  assert(!endnum);
  assert(!messageListLength);

  // Only a single message number was specified
  assert(startnum);
  dp->SetMessageAttribute(err, attributeName, attributeValue, startnum, *mcmp);

  if (err != Sdm_EC_Success) {
    printf("data port %s: setattributes failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  printf("data port %s: setattributes successful\n", expandPortName(pdp));

  return(TRUE);
}

static int subcmd_port_setcontents(int argc, char **argv)
{
  SdmDpMessageStructureL msgstrL;
  SdmMessageNumberL messageList;
  char *componentName;
  SdmDpMessageComponent *mcmp = 0;
  SdmError err;
  SdmMessageNumber startnum = 0, endnum = 0;

  msgstrL.SetPointerDeleteFlag();		// Force vector contents to be deleted on object destruction

  if (argc < 4) {
    printf(
	   "?The setcontents command takes this form:\n"
	   "\tsetcontents port messagenum component [contents | -F file]\n"
	   );
    return(TRUE);
  }

  // Extract the port number
  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // Extract message number

  assert(argc);
  char *messageSequence = *argv++;
  argc--;
  if (!decodeMessageRange(messageList, startnum, endnum, messageSequence, pdp, TRUE))
    return(TRUE);
  int messageListLength = messageList.ElementCount();
  if (!startnum || endnum || messageListLength) {
    printf("?A single message number must be specified - no range,\nlist, or other multiple specifications allowed.\n");
    return(TRUE);
  }

  // Extract the component name

  assert(argc);
  componentName = *argv++;
  argc--;

  // Extract the contents value

  SdmString contentsValue("");

  // Extract optional get flags

  if (argc && strcasecmp(*argv, "-f")==0) {
    argc--;
    argv++;
    if (!argc) {
      printf("?A file name must follow the -F option\n");
      return(TRUE);
    }
    char* fileName = *argv++;
    argc--;
    int fd = SdmSystemUtility::SafeOpen(fileName, O_RDONLY);
    if (fd == -1) {
      int saveerrno = errno;
      printf("?Could not open %s errno %d: %s\n", fileName, saveerrno, strerror(saveerrno));
      return(TRUE);
    }
    off_t fileSize = SdmSystemUtility::SafeLseek(fd, 0L, SEEK_END);
    if (fileSize) {
      char *buffer = (char *)malloc((unsigned)fileSize);
      assert(buffer);
      (void) SdmSystemUtility::SafeLseek(fd, 0L, SEEK_SET);
      if (SdmSystemUtility::SafeRead(fd, buffer, fileSize) != fileSize) {
	int saveerrno = errno;
	(void) SdmSystemUtility::SafeClose(fd);
	printf("?Could not read %s errno %d: %s\n", fileName, saveerrno, strerror(saveerrno));
	return(TRUE);
      }
      SdmString hack(buffer,fileSize);
      contentsValue = hack;
      (void) free(buffer);
    }
  }
  else {
    while (argc) {
      contentsValue += *argv++;
      if (--argc)
	contentsValue += " ";
    }
  }

  assert(!endnum);
  assert(!messageListLength);

  // Retrieve the message structure for this message

  if (dp->GetMessageStructure(err, msgstrL, startnum) != Sdm_EC_Success) {
    printf("data port %s: getstructure failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  // lookup the message component
  //
  mcmp = lookupMessageComponent(pdp, msgstrL, componentName, TRUE);
  if (!mcmp) {
    printf("data port %s: message component '%s' does not exist in message %d\n", expandPortName(pdp), componentName, startnum);
    return(TRUE);
  }

  assert(!endnum);
  assert(!messageListLength);

  // Only a single message number was specified
  assert(startnum);
  dp->SetMessageContents(err, contentsValue, startnum, *mcmp);

  if (err != Sdm_EC_Success) {
    printf("data port %s: setcontents failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  printf("data port %s: setcontents successful\n", expandPortName(pdp));

  return(TRUE);
}

static int subcmd_port_setflags(int argc, char **argv)
{
  return portSetFlags(argc, argv, Sdm_True);
}

static int subcmd_port_shutdown(int argc, char **argv)
{
  if (!argc) {
    printf("?The shutdown subcommand takes one or more arguments which specify\n?the ports to be shutdownd; to shutdown all ports, use a single * character.\n");
    return(TRUE);
  }
  while (argc--) {
    char *currentPort = *argv++;
    if (strcmp(currentPort,"*")==0) {
      SdmLinkedListIterator lit(PORT_LIST);
      PdpDataPort *pdp;

      while (pdp = (PdpDataPort *)(lit.NextListElement())) {
	shutdownPort(pdp, TRUE);
      }
    }
    else {
      PdpDataPort *pdp = lookupPort(currentPort, TRUE);
      if (pdp)
	shutdownPort(pdp, TRUE);
    }
  }
  return(TRUE);
}

static int subcmd_port_startup(int argc, char **argv)
{
  if (!argc) {
    printf("?The startup subcommand takes one or more arguments which specify\n?the ports to be startupd; to startup all ports, use a single * character.\n");
    return(TRUE);
  }
  while (argc--) {
    char *currentPort = *argv++;
    if (strcmp(currentPort,"*")==0) {
      SdmLinkedListIterator lit(PORT_LIST);
      PdpDataPort *pdp;

      while (pdp = (PdpDataPort *)(lit.NextListElement())) {
	startupPort(pdp, TRUE);
      }
    }
    else {
      PdpDataPort *pdp = lookupPort(currentPort, TRUE);
      if (pdp)
	startupPort(pdp, TRUE);
    }
  }
  return(TRUE);
}

static int subcmd_port_getstatus(int argc, char **argv)
{
  if (!argc) {
    printf("?The getstatus subcommand takes one or more arguments which specify\n?the ports to be stated; to stat all ports, use a single * character.\n");
    return(TRUE);
  }
  while (argc--) {
    char *currentPort = *argv++;
    if (strcmp(currentPort,"*")==0) {
      SdmLinkedListIterator lit(PORT_LIST);
      PdpDataPort *pdp;

      while (pdp = (PdpDataPort *)(lit.NextListElement())) {
	statusPort(pdp, TRUE);
      }
    }
    else {
      PdpDataPort *pdp = lookupPort(currentPort, TRUE);
      if (pdp)
	statusPort(pdp, TRUE);
    }
  }
  return(TRUE);
}

static int subcmd_port_messagebodydelete(int argc, char **argv)
{
  SdmDpMessageStructureL msgstrL;
  SdmMessageNumberL messageList;
  char *componentName;
  SdmDpMessageComponent *mcmp = 0;
  SdmError err;
  SdmMessageNumber startnum = 0, endnum = 0;
  SdmBoolean previousState;

  msgstrL.SetPointerDeleteFlag();		// Force vector contents to be deleted on object destruction

  if (argc != 4) {
    printf(
"?The messagebodydelete command takes this form:\n"
"\tport messagebodydelete port messagenum component [yes | no]\n"
);
    return(TRUE);
  }

  // Extract the port number
  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // Extract message number

  assert(argc);
  char *messageSequence = *argv++;
  argc--;
  if (!decodeMessageRange(messageList, startnum, endnum, messageSequence, pdp, TRUE))
    return(TRUE);
  int messageListLength = messageList.ElementCount();
  if (!startnum || endnum || messageListLength) {
    printf("?A single message number must be specified - no range,\nlist, or other multiple specifications allowed.\n");
    return(TRUE);
  }

  // Extract the component name

  assert(argc);
  componentName = *argv++;
  argc--;

  // determine message delete flag new state

  SdmBoolean deleteFlag;

  if (**argv == 'y' || **argv == 'Y') 
    deleteFlag = Sdm_True;
  else if (**argv == 'n' || **argv == 'N')
    deleteFlag = Sdm_False;
  else {
    printf("must specify yes or no for delete state flag: %s\n", *argv);
    return(TRUE);
  }

  assert(!endnum);
  assert(!messageListLength);

  // Retrieve the message structure for this message

  if (dp->GetMessageStructure(err, msgstrL, startnum) != Sdm_EC_Success) {
    printf("data port %s: getstructure failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  // lookup the message component
  //
  mcmp = lookupMessageComponent(pdp, msgstrL, componentName, TRUE);
  if (!mcmp) {
    printf("data port %s: message component '%s' does not exist in message %d\n", expandPortName(pdp), componentName, startnum);
    return(TRUE);
  }

  assert(!endnum);
  assert(!messageListLength);

  // Only a single message number was specified
  assert(startnum);
  dp->DeleteMessageBodyFromMessage(err, previousState, deleteFlag, startnum, *mcmp);

  if (err != Sdm_EC_Success) {
    printf("data port %s: messagebodydelete failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  printf("data port %s: messagebodydelete successful; previous state = %s\n", expandPortName(pdp),
	 previousState ? "TRUE" : "FALSE");

  return(TRUE);
}

static int subcmd_port_movemessage(int argc, char **argv)
{
  return(copymovemessage(argc, argv, Sdm_True));
}

static int subcmd_port_newmessagecreate(int argc, char **argv)
{
  SdmError err;
  SdmMessageNumber newmsgnum;

  if (argc != 1) {
    printf("?The newmessagecreate command takes the form: newmessagecreate port\n");
    return(TRUE);
  }

  // Extract the port number

  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // Create the new message
  dp->CreateNewMessage(err, newmsgnum);

  if (err != Sdm_EC_Success) {
    printf("data port %s: createnewmessage failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  printf("data port %s: createnewmessage successful: new message number = %ld\n", expandPortName(pdp), newmsgnum);
  return(TRUE);
}

static int subcmd_port_open(int argc, char **argv)
{
  SdmError err;
  SdmMessageNumber nmsgs;
  SdmBoolean readOnly;

  if (argc != 2) {
    printf("?The open command takes a port object and a\ntoken argument specifying the object\nto open.\n");
    return(TRUE);
  }

  char *port = *argv++;
  char *token = *argv;

  PdpDataPort *pdp = lookupPort(port, TRUE);
  if (!pdp)
    return(TRUE);
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  PtkToken *ptk = lookupToken(token, TRUE);
  if (!ptk)
    return(TRUE);
  SdmToken *tk = ptk->ptk_token;
  assert(tk);

  dp->Open(err, nmsgs, readOnly, *tk);
  if (err != Sdm_EC_Success)
    printf("data port %s: open failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
  else {
    printf("data port %s: opened%s\n", expandPortName(pdp), readOnly ? " IN READ ONLY MODE!" : "");
  }
  return(TRUE);
}

static int subcmd_port_attach(int argc, char **argv)
{
  SdmError err;

  if (argc != 2) {
    printf("?The attach command takes a port object and a\ntoken argument specifying the object\nto attach to.\n");
    return(TRUE);
  }

  char *port = *argv++;
  char *token = *argv;

  PdpDataPort *pdp = lookupPort(port, TRUE);
  if (!pdp)
    return(TRUE);
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  PtkToken *ptk = lookupToken(token, TRUE);
  if (!ptk)
    return(TRUE);
  SdmToken *tk = ptk->ptk_token;
  assert(tk);

  dp->Attach(err, *tk);
  if (err != Sdm_EC_Success)
    printf("data port %s: attach failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
  else
    printf("data port %s: attached\n", expandPortName(pdp));
  return(TRUE);
}

static int subcmd_port_storedelete(int argc, char **argv)
{
  SdmError err;

  if (argc != 2) {
    printf("?The storedelete command takes a port object and the name of\nthe message store to be deleted.\n");
    return(TRUE);
  }

  char *port = *argv++;
  SdmString name(*argv);

  PdpDataPort *pdp = lookupPort(port, TRUE);
  if (!pdp)
    return(TRUE);
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  dp->Delete(err, name);
  if (err != Sdm_EC_Success)
    printf("data port %s: storedelete failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
  else
    printf("data port %s: store deleted\n", expandPortName(pdp));
  return(TRUE);
}

static int subcmd_port_derivedmessagecreate(int argc, char **argv)
{
  SdmError err;
  long startnum = 0, endnum = 0;
  SdmMessageNumberL messageList;
  SdmMessageNumber newmsgnum;

  if (argc != 2) {
    printf("?The derivedmessagecreate command takes the form: derivedmessagecreate port messagenum\n");
    return(TRUE);
  }

  // Extract the port number

  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // Extract message number

  assert(argc);
  char *messageSequence = *argv++;
  argc--;
  if (!decodeMessageRange(messageList, startnum, endnum, messageSequence, pdp, TRUE))
    return(TRUE);
  int messageListLength = messageList.ElementCount();
  if (!startnum || endnum || messageListLength) {
    printf("?A single message number must be specified - no range,\nlist, or other multiple specifications allowed.\n");
    return(TRUE);
  }

  // Create the derived message
  assert(!endnum);
  assert(!messageListLength);

  // execute createderivedmessage
  dp->CreateDerivedMessage(err, newmsgnum, *dp, startnum);

  if (err != Sdm_EC_Success) {
    printf("data port %s: createderivedmessage failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  printf("data port %s: createderivedmessage successful: new message number = %ld\n", expandPortName(pdp), newmsgnum);
  return(TRUE);
}

static int subcmd_port_discardpendingmessagechanges(int argc, char **argv)
{
  SdmError err;
  long startnum = 0, endnum = 0;
  SdmMessageNumberL messageList;

  if (argc != 2) {
    printf("?The discardpendingmessagechanges command takes the form: discardpendingmessagechanges port messagenum\n");
    return(TRUE);
  }

  // Extract the port number

  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // Extract message number

  assert(argc);
  char *messageSequence = *argv++;
  argc--;
  if (!decodeMessageRange(messageList, startnum, endnum, messageSequence, pdp, TRUE))
    return(TRUE);
  int messageListLength = messageList.ElementCount();
  if (!startnum || endnum || messageListLength) {
    printf("?A single message number must be specified - no range,\nlist, or other multiple specifications allowed.\n");
    return(TRUE);
  }

  // Create the derived message
  assert(!endnum);
  assert(!messageListLength);

  // execute discardpendingmessagechanges
  dp->DiscardPendingMessageChanges(err, startnum);

  if (err != Sdm_EC_Success) {
    printf("data port %s: discardpendingmessagechanges failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  printf("data port %s: discardpendingmessagechanges successful\n", expandPortName(pdp));
  return(TRUE);
}

static int subcmd_port_disconnect(int argc, char **argv)
{
  SdmError err;

  if (argc != 1) {
    printf("?The disconnect command takes a port object and disconnects\nfrom the message store open on that object.\n");
    return(TRUE);
  }

  char *port = *argv++;
  char *token = *argv;

  PdpDataPort *pdp = lookupPort(port, TRUE);
  if (!pdp)
    return(TRUE);
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  dp->Disconnect(err);
  if (err != Sdm_EC_Success)
    printf("data port %s: disconnect failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
  else
    printf("data port %s: disconnected\n", expandPortName(pdp));
  return(TRUE);
}

static int subcmd_port_expungedeleted(int argc, char **argv)
{
  SdmError err;
  SdmMessageNumberL msgnums;

  if (argc != 1) {
    printf("?The expungedeleted command takes a port object and expunges any deleted\nmessages from the message store.\n");
    return(TRUE);
  }

  char *port = *argv++;
  char *token = *argv;

  PdpDataPort *pdp = lookupPort(port, TRUE);
  if (!pdp)
    return(TRUE);
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  dp->ExpungeDeletedMessages(err, msgnums);
  if (err != Sdm_EC_Success)
    printf("data port %s: expungedeleted failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
  else {
    int i = msgnums.ElementCount();
    printf("data port %s: expungedeleted successful; %d messages deleted: ", expandPortName(pdp), i);
    for (int j = 0; j < i; j++)
      printf("%d%s", (SdmMessageNumber)msgnums[j], j == i-1 ? "" : ", ");
    printf("\n");
  }
  return(TRUE);
}

static int subcmd_port_storecreate(int argc, char **argv)
{
  SdmError err;

  if (argc != 2) {
    printf("?The storecreate command takes a port object and the\nname of the message store to be created.\n");
    return(TRUE);
  }

  char *port = *argv++;
  SdmString name(*argv);

  PdpDataPort *pdp = lookupPort(port, TRUE);
  if (!pdp)
    return(TRUE);
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  dp->Create(err, name);
  if (err != Sdm_EC_Success)
    printf("data port %s: storecreate failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
  else
    printf("data port %s: store created\n", expandPortName(pdp));
  return(TRUE);
}

static int subcmd_port_submit(int argc, char **argv)
{
  SdmError err;
  long startnum = 0, endnum = 0;
  SdmMessageNumberL messageList;
  SdmDeliveryResponse deliveryResponse;

  if (argc != 4) {
    printf("?The submit command takes the form: submit port srcport messagenum [ mime | sunv3 ]\n");
    return(TRUE);
  }

  // Extract the port number

  PdpDataPort *pdp = lookupPort(*argv, TRUE);
  if (!pdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *dp = pdp->pdp_dataport;
  assert(dp);

  // Extract the source port number

  PdpDataPort *srcpdp = lookupPort(*argv, TRUE);
  if (!srcpdp)
    return(TRUE);
  argc--;
  argv++;

  // Determine data port to operate on
  SdmDataPort *srcdp = srcpdp->pdp_dataport;
  assert(srcdp);

  // Extract message number

  assert(argc);
  char *messageSequence = *argv++;
  argc--;
  if (!decodeMessageRange(messageList, startnum, endnum, messageSequence, pdp, TRUE))
    return(TRUE);
  int messageListLength = messageList.ElementCount();
  if (!startnum || endnum || messageListLength) {
    printf("?A single message number must be specified - no range,\nlist, or other multiple specifications allowed.\n");
    return(TRUE);
  }

  // Extract the format
  SdmMsgFormat msgFormat;
  char *format = *argv++;
  argc--;

  if ( (::strcasecmp(format, "mime")==0)
       || (::strcasecmp(format, "mim")==0)
       || (::strcasecmp(format, "mi")==0)
       || (::strcasecmp(format, "m")==0) ) msgFormat = Sdm_MSFMT_Mime;
  else if ( (::strcasecmp(format, "sunv3")==0)
       || (::strcasecmp(format, "sunv")==0)
       || (::strcasecmp(format, "sun")==0)
       || (::strcasecmp(format, "su")==0)
       || (::strcasecmp(format, "s")==0) ) msgFormat = Sdm_MSFMT_SunV3;
  else {
    printf("?format must be 'mime' or 'sunv3': %s\n", format);
    return(TRUE);
  }

  // Create the derived message
  assert(!endnum);
  assert(!messageListLength);

  // The following custom headers will only be sent if the outgoing message has
  // contains matching headers.  These could be setup with, for example:
  // port addheader port1 -1 X-Cheese Quais, cest le Brie, Camembert et Gorgonzola!
  SdmStringL customHeaders;
  customHeaders(-1) = "X-Mailer";
  customHeaders(-1) = "X-Country";
  customHeaders(-1) = "X-Wine";
  customHeaders(-1) = "X-Cheese";
  customHeaders(-1) = "X-Non-Existent";

  // execute submit on this channel adapter
  dp->Submit(err, deliveryResponse, msgFormat, *srcdp, startnum, customHeaders);

  if (err != Sdm_EC_Success) {
    printf("data port %s: submit failed with error %s\n", expandPortName(pdp), detailedErrorReport(err));
    return(TRUE);
  }

  printf("data port %s: submit successful\n", 
	 expandPortName(pdp));

  return(TRUE);
}

static int subcmd_port_validate(int argc, char **argv)
{
  SdmError err;
  long startnum = 0, endnum = 0;
  SdmString validateString;
  SdmString testName;
  int fd;
  const char *s;

  if (argc < 2) {
    printf
      (
       "?The validate command takes the form: validate [ string | -F file | -S sum ]\n"
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
