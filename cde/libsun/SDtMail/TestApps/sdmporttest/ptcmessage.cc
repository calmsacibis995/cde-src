#include <SDtMail/Sdtmail.hh> 
#include <SDtMail/LinkedList.hh> 
#include <PortObjs/DataPort.hh> 
#include <SDtMail/MessageEnvelope.hh>
#include <SDtMail/MessageStore.hh>
#include <SDtMail/MessageBody.hh>
#include <SDtMail/Message.hh> 
#include <SDtMail/DataTypeUtility.hh>
#include <porttest.hh> 
#include <ptcmdscan.hh> 
#include <ptcmessage.hh> 
#include <ptcstore.hh>
#include <ptcport.hh>

#include <Dt/Dts.h>

#pragma weak DtDtsFindAttribute

/*
** EXTERNALS
*/

extern SdmLinkedList *STOREE_LIST;

/*
** GLOBALS
*/
char *Hmessage =
"Description: manipulate message objects\n"
"General usage: message command [ msgname [ arguments ]]\n"
"Specific usage:\n"
"\tmessage clearflags flags\n"
"\tmessage create store msgnum msgname\n"
"\tmessage delete [msgname(s) | *]\n"
"\tmessage getbodycount [msgname(s) | *]\n"
"\tmessage getcontents msgname [-F <outputfile>] [-S] [-H] [+a]\n"
"\tmessage getflags [msgname(s) | *]\n"
"\tmessage getheaders msgname [header(s)]]\n"
"\tmessage getheaders msgname [ -a|-A [abstractheader(s)]]]\n"
"\tmessage getstructure [msgname(s) | *]"
"\tmessage setflags msgname flags\n"
"\tmessage status [msgname(s) | *]\n"
"";

static int subcmd_message_clearflags(int argc, char **argv);
static int subcmd_message_create(int argc, char **argv);
static int subcmd_message_delete(int argc, char **argv);
static int subcmd_message_getbodycount(int argc, char **argv);
static int subcmd_message_getcontents(int argc, char **argv);
static int subcmd_message_getflags(int argc, char **argv);
static int subcmd_message_getheaders(int argc, char **argv);
static int subcmd_message_getstructure(int argc, char **argv);
static int subcmd_message_setflags(int argc, char **argv);
static int subcmd_message_status(int argc, char **argv);

static SUBCMD messagecmdtab[] = {
  // cmdname,		handler,			argsrequired
  {"clearflags",	subcmd_message_clearflags, 	0},	// clear flags on a message
  {"create",		subcmd_message_create,		0},	// create a new message
  {"delete",		subcmd_message_delete,		0},	// delete an existing message
  {"getbodycount",	subcmd_message_getbodycount,	0},	// get body counts from message
  {"getcontents",	subcmd_message_getcontents,	0},	// get message contents
  {"getflags",		subcmd_message_getflags,	0},	// get flags from message
  {"getheaders",	subcmd_message_getheaders,	0},	// get headers from message
  {"getstructure",	subcmd_message_getstructure,	0},	// get message structure
  {"setflags",		subcmd_message_setflags,	0},	// set flags on a message
  {"status",		subcmd_message_status,		0},	// status of message
  {NULL,		0,				0}	// must be last entry in table
};

SdmLinkedList *MESSAGE_LIST = 0;

/*
** FORWARD DECLARATIONS
*/

/*************************************************************************
 *
 * Function:	Cmessage - 
 *
 * Input:	int pargc - number of arguments passed to command
 *		char **pargv - array of pargc arguments passed
 *
 * Output:	TRUE - help given
 *
 ************************************************************************/

int Cmessage(int argc, char **argv)
{
  return (argc == 1 ? TRUE : oneSubCommand(argv[0], argv[1], messagecmdtab, argc-2, argv+2));
}

const char *expandMessageName(PmsgMessage *pmsg)
{
  assert(pmsg->pmsg_messagename);
  return ((const char *)*pmsg->pmsg_messagename);
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

static PmsgMessage *validmessageName(char *messageName)
{
  SdmLinkedListIterator lit(MESSAGE_LIST);
  PmsgMessage *pmsg;

  while (pmsg = (PmsgMessage *)(lit.NextListElement())) {
    assert(pmsg->pmsg_messagename);
    if (strcasecmp((const char *)*pmsg->pmsg_messagename, messageName)==0)
      return(pmsg);
  }
  return((PmsgMessage *)0);
}

static PmsgMessage *findMessage(SdmMessageNumber msgnum, int printMessage)
{
  PmsgMessage *pmsg;
  SdmLinkedListIterator lit(MESSAGE_LIST);

  assert(msgnum > 0);

  // If the message list is empty, nothing to do here
  if (!MESSAGE_LIST || !MESSAGE_LIST->ElementCount()) {
    if (printMessage)
      printf("?No messages defined\n");
    return((PmsgMessage *)0);
  }

  while (pmsg = (PmsgMessage *)(lit.NextListElement())) {
    assert(pmsg->pmsg_messagenum);
    if (pmsg->pmsg_messagenum == msgnum)
      return(pmsg);
  }
  return(0);
}

static PmsgMessage *lookupMessage(char *messageName, int printMessage)
{
  PmsgMessage *pmsg;

  assert(messageName);

  // If the message list is empty, nothing to do here
  if (!MESSAGE_LIST || !MESSAGE_LIST->ElementCount()) {
    if (printMessage)
      printf("?No messages defined\n");
    return((PmsgMessage *)0);
  }

  // lookup the message by name - if the name is registered, return that message
  pmsg = validmessageName(messageName);
  if (pmsg)
    return(pmsg);

  // message not registered - say so and return nothing
  if (printMessage)
    printf("?message %s does not exist\n", messageName);
  return((PmsgMessage *)0);
}

static SdmMessageNumber decodeMsgnum(char *msgnumName, int printMessage)
{
  SdmError err;

  assert(msgnumName);

  char *p = 0;
  SdmMessageNumber msgnum = 0;

  // Not a range - handle single number of a:b range

  msgnum = strtol(msgnumName, &p, 10);
  if (p == msgnumName) {
    if (printMessage)
      printf("?Message number must be a positive integer: %s\n", msgnumName);
    return(0);
  }
  if (msgnum < 1) {
    if (printMessage)
      printf("?Message number must be a positive integer greater than zero: %d\n", msgnum);
    return(0);
  }

  if (*p) {
    if (printMessage)
      printf("?Message number ends with invalid character '%c': %s\n", *p, p);
    return(0);
  }

  return(msgnum);
}

static void statusMessage(PmsgMessage *pmsg)
{
  SdmError err;
  SdmMessageNumber msgnum = 0;

  assert(pmsg);
  SdmMessage *msg = pmsg->pmsg_message;
  assert(msg);
  if (msg->GetMessageNumber(err, msgnum) != Sdm_EC_Success) {
    printf("\tfailed to get message number with error %u: %s\n", (SdmErrorCode) err, err.ErrorMessage());
    return;
  }
  printf("message %s:\t store %s\tmsgnum\t%d(%d)\n", expandMessageName(pmsg), (const char *)*pmsg->pmsg_messagestorename, pmsg->pmsg_messagenum, msgnum);
}

static void deleteMessage(PmsgMessage *pmsg, int printMessage)
{
  SdmError err;

  assert(pmsg);
  SdmMessage *msg = pmsg->pmsg_message;
  assert(msg);
  printf("message %s: deleted (store %s msgnum %d)\n", expandMessageName(pmsg), (const char *)*pmsg->pmsg_messagestorename, pmsg->pmsg_messagenum);
  MESSAGE_LIST->RemoveElementFromList(pmsg);
  delete pmsg;
}

static int messageSetFlags(int argc, char **argv, SdmBoolean value)
{
  SdmError err;
  SdmMessageFlagAbstractFlags flags = 0;
  SdmMessageNumberL messageList;

  if (argc < 2) {
    printf("?The %s command takes the form: cmd msgname flags\n",
	   value ? "setflags" : "clearflags");
    return(TRUE);
  }

  // Extract the message name
  PmsgMessage *pmsg = lookupMessage(*argv, TRUE);
  if (!pmsg)
    return(TRUE);
  argc--;
  argv++;

  SdmMessage *msg = pmsg->pmsg_message;
  assert(msg);

  // Collect the flags
  assert(argc);
  while (argc--) {
    SdmMessageFlagAbstractFlags x = 0;
    if (!decodeFlagAbstractValue(x, *argv++, TRUE))
      return(TRUE);
    flags |= x;
  }

  // issue appropriate object method to set or clear the flags

  if (value)
      msg->SetFlags(err, flags);
    else
      msg->ClearFlags(err, flags);

  if (err != Sdm_EC_Success) {
    printf("message %s: %s failed with error %u: %s\n", expandMessageName(pmsg),
	   value ? "setflags" : "clearflags", (SdmErrorCode) err, err.ErrorMessage());
  
    return(TRUE);
  }

  printf("message %s: %s successful\n", expandMessageName(pmsg),
	 value ? "setflags" : "clearflags");
  return(TRUE);
}

static void decodeNestedBodyMessage(SdmMessage *msg, int level)
{
  SdmError err;
  int bodycount = 0;
  if (msg->GetBodyCount(err, bodycount) != Sdm_EC_Success) {
    printf("\tfailed to get body %d count with error %u: %s\n", level, (SdmErrorCode) err, err.ErrorMessage());
    return;
  }

  if (level)
    printf("/");

  printf("%d-%d", level, bodycount);

  // Go through each body part

  for (int i = 1; i <= bodycount; i ++) {
    SdmMessageBody *bdy = 0;
    SdmMessageBodyStructure bdystr;
    SdmMessage *msg1 = 0;

    if (msg->SdmMessageBodyFactory(err, bdy, i) != Sdm_EC_Success) {
      printf("\tfailed to get body %d/%d with error %u: %s\n", level, i, (SdmErrorCode) err, err.ErrorMessage());
      return;
    }

    if (bdy->GetStructure(err, bdystr) != Sdm_EC_Success) {
      printf("\tfailed to get body %d/%d structure with error %u: %s\n", level, i, (SdmErrorCode) err, err.ErrorMessage());
      return;
    }

    if (!bdystr.mbs_is_nested_message)
      break;

    if (bdy->SdmMessageFactory(err, msg1) != Sdm_EC_Success) {
      printf("\tfailed to get body %d/%d nested message with error %u: %s\n", level, i, (SdmErrorCode) err, err.ErrorMessage());
      return;
    }
    decodeNestedBodyMessage(msg1, level+1);
  }
}

static void getbodycountMessage(PmsgMessage *pmsg)
{
  SdmError err;
  int bodycount = 0;

  assert(pmsg);
  SdmMessage *msg = pmsg->pmsg_message;
  assert(msg);

  printf("message %s:\t", expandMessageName(pmsg));
  decodeNestedBodyMessage(msg, 0);
  printf("\n");
}

static void decodeNestedStructureMessage(SdmMessage *msg, char *componentString, int printHeader)
{
  SdmError err;
  int bodycount = 0;
  int subbodycount = 0;
  int componentIndex = 0;
  char compstr[256];
  SdmMessageNumber msgnum = 0;

  if (msg->GetMessageNumber(err, msgnum) != Sdm_EC_Success) {
    printf("\tfailed to get %s message number with error %u: %s\n", componentString, (SdmErrorCode) err, err.ErrorMessage());
    return;
  }

  if (msg->GetBodyCount(err, bodycount) != Sdm_EC_Success) {
    printf("\tfailed to get body %s count with error %u: %s\n", componentString, (SdmErrorCode) err, err.ErrorMessage());
    return;
  }

  // Go through each body part

  if (printHeader)
    printf("  %-4s %-15s %-11s %-15s %-4s %-5s %-8s %-5s %-6s %-8s %-4s %-16s %-10s %-10s %s\n",
	   "MSG#","ID","TYPE","SUBTYPE","TEXT","PARTS","ENCODING","LINES","BYTES","DISP","MODE","MIME-TYPE","CDE-TYPE","FILE-NAME","DESCRIPTION");

  for (int i = 1; i <= bodycount; i ++) {
    SdmMessageBody *bdy = 0;
    SdmMessageBodyStructure bdystr;
    SdmMessage *msg1 = 0;

    sprintf(compstr, "%s.%d", componentString, i);

    if (msg->SdmMessageBodyFactory(err, bdy, i) != Sdm_EC_Success) {
      printf("\tfailed to get body %s.%d with error %u: %s\n", componentString, i, (SdmErrorCode) err, err.ErrorMessage());
      return;
    }

    if (bdy->GetStructure(err, bdystr) != Sdm_EC_Success) {
      printf("\tfailed to get body %s.%d structure with error %u: %s\n", componentString, i, (SdmErrorCode) err, err.ErrorMessage());
      return;
    }

    if (bdystr.mbs_is_nested_message) {
      if (bdy->SdmMessageFactory(err, msg1) != Sdm_EC_Success) {
	printf("\tfailed to get body %s.%d nested message with error %u: %s\n", componentString, i, (SdmErrorCode) err, err.ErrorMessage());
	return;
      }
      if (msg1->GetBodyCount(err, subbodycount) != Sdm_EC_Success) {
	printf("\tfailed to get body %s count with error %u: %s\n", componentString, (SdmErrorCode) err, err.ErrorMessage());
	return;
      }
    }

    SdmString cdeType;
    SdmString mimeType;

    (void) SdmDataTypeUtility::GetMimeTypeStringFromMessage(err, mimeType, *bdy, &bdystr);
    if (&DtDtsFindAttribute)
      (void) SdmDataTypeUtility::DetermineCdeDataTypeForMessage(err, cdeType, *bdy, &bdystr, Sdm_False);

    SdmBoolean isText = Sdm_False;
    SdmMsgStrType xmimeType;
    SdmString xmimeSubtype;

    if (&DtDtsFindAttribute)
      (void) SdmDataTypeUtility::DetermineMimeDataTypesForMessage(err, isText, xmimeType, xmimeSubtype, *bdy, &bdystr);

    printf("  %-4d %-15s %-11s %-15s %-4s %-5d %-8s %-5d %-6d %-8s %-4o %-16s %-10s %-10s %s\n",
	   msgnum, compstr, expandMsType(bdystr.mbs_type), expandString(bdystr.mbs_subtype), 
	   isText ? "yes" : "no", bdystr.mbs_is_nested_message ? subbodycount : 1,
	   expandMeType(bdystr.mbs_encoding), 
	   bdystr.mbs_lines, bdystr.mbs_bytes, 
	   expandMsDisp(bdystr.mbs_disposition), bdystr.mbs_unix_mode,
	   expandString(mimeType), expandString(cdeType), expandString(bdystr.mbs_attachment_name),
	   expandString(bdystr.mbs_description));

    if (bdystr.mbs_is_nested_message) {
      if (bdy->SdmMessageFactory(err, msg1) != Sdm_EC_Success) {
	printf("\tfailed to get body %s.%d nested message with error %u: %s\n", componentString, i, (SdmErrorCode) err, err.ErrorMessage());
	return;
      }
      decodeNestedStructureMessage(msg1, compstr, FALSE);
    }
  }
}

static void getstructureMessage(PmsgMessage *pmsg)
{
  SdmError err;
  int bodycount = 0;

  assert(pmsg);
  SdmMessage *msg = pmsg->pmsg_message;
  assert(msg);

  printf("message %s:\n", expandMessageName(pmsg));
  decodeNestedStructureMessage(msg, "0", TRUE);
}

static void getflagsMessage(PmsgMessage *pmsg)
{
  SdmError err;
  SdmMessageFlagAbstractFlags flags;

  assert(pmsg);
  SdmMessage *msg = pmsg->pmsg_message;
  assert(msg);
  if (msg->GetFlags(err, flags) != Sdm_EC_Success) {
    printf("message %s:\tfailed to get flags with error %u: %s\n", expandMessageName(pmsg), (const char *)*pmsg->pmsg_messagestorename, pmsg->pmsg_messagenum, (SdmErrorCode) err, err.ErrorMessage());
    return;
  }
  printf("message %s:\t", expandMessageName(pmsg));
  printFlags(flags);
}

static void getContentsAsyncCallback(SdmError& err, void* userdata, SdmServiceFunctionType type, 
  SdmEvent* event)
{
  assert (event != NULL);

  printf("\n--> getContentsAsyncCallback invoked\n");
  if (event->sdmBase.type != Sdm_Event_GetMessageContents) {
    printf("unknown event type: userdata = %08lx event type %d\n", userdata, event->sdmBase.type);
    return;
  }

  SdmContentBuffer theContents = *event->sdmGetContents.buffer;   
  SdmMessageNumber theMessageNumber = event->sdmGetContents.messageNumber;
  int theBodyPartNumber = event->sdmGetContents.bodyPartNumber;

#if 0
  PmsgMessage *pmsg = (PmsgMessage *)userdata;
  assert(pmsg);

  if (err != Sdm_EC_Success)
    printf("message store %s: expungedeleted failed with error %u: %s\n", expandStoreName(pmst), (SdmErrorCode) err, err.ErrorMessage());
  else {
    int i = msgnums->ElementCount();
    printf("message store %s: expungedeleted successful; %d messages deleted: ", expandStoreName(pmst), i);
    for (int j = 0; j < i; j++)
      printf("%d%s", (SdmMessageNumber)(*msgnums)[j], j == i-1 ? "" : ", ");
    printf("\n");
  }
#endif
}

static int subcmd_message_clearflags(int argc, char **argv)
{
  return messageSetFlags(argc, argv, Sdm_False);
}

static int subcmd_message_create(int argc, char **argv)
{
  SdmError err;

  if (argc != 3) {
    printf("?The create subcommand requires 3 arguments: message create store msgnum msgname\n");
    return(TRUE);
  }

  char *storeName = *argv++; argc--;
  char *msgnumName = *argv++; argc--;
  char *msgName = *argv++; argc--;

  // Extract the message store name
  PmstMessageStore *pmst = lookupStore(storeName, TRUE);
  if (!pmst)
    return(TRUE);

  // Determine message store to operate on
  SdmMessageStore *mst = pmst->pmst_messagestore;
  assert(mst);

  // Extract the message number
  SdmMessageNumber msgnum = decodeMsgnum(msgnumName, TRUE);
  if (!msgnum)
    return(TRUE);

  // See if the message name already exists
  if (lookupMessage(msgName, FALSE) != (PmsgMessage *)0) {
    printf("?message %s already exists\n", msgName);
    return(TRUE);
  }

  // See if the message number is already created on another name
  PmsgMessage *pmsgx = findMessage(msgnum, FALSE);
  if (pmsgx) {
    printf("?message %d already created as message '%s'\n", msgnum, expandMessageName(pmsgx));
    return(TRUE);
  }

  // Attempt to create the message on the message store
  SdmMessage *msg = 0;
  if (mst->SdmMessageFactory(err, msg, msgnum) != Sdm_EC_Success) {
    printf("message %s: failed to create with error %u: %s\n", msgName, (SdmErrorCode) err, err.ErrorMessage());
    return(TRUE);
  }
  assert(msg);
  PmsgMessage *pmsg = new PmsgMessage();
  assert(pmsg);
  if (!MESSAGE_LIST)
    MESSAGE_LIST = new SdmLinkedList();
  pmsg->pmsg_message = msg;
  pmsg->pmsg_messagename = new SdmString(msgName);
  pmsg->pmsg_messagenum = msgnum;
  pmsg->pmsg_messagestorename = new SdmString(storeName);
  MESSAGE_LIST->AddElementToList(pmsg);
  printf("message %s: created (store %s msgnum %d)\n", expandMessageName(pmsg), storeName, msgnum);
  return(TRUE);
}

static int subcmd_message_delete(int argc, char **argv)
{
  if (!argc) {
    printf("?The delete subcommand takes one or more arguments which specify\n?the messages to be deleted; to delete all messages, use a single * character.\n");
    return(TRUE);
  }
  while (argc--) {
    char *currentMessage = *argv++;
    if (strcmp(currentMessage,"*")==0) {
      SdmLinkedListIterator lit(MESSAGE_LIST);
      PmsgMessage *pmsg;

      while (pmsg = (PmsgMessage *)(lit.NextListElement())) {
	deleteMessage(pmsg, TRUE);
      }
    }
    else {
      PmsgMessage *pmsg = lookupMessage(currentMessage, TRUE);
      if (pmsg)
	deleteMessage(pmsg, TRUE);
    }
  }
  return(TRUE);
}

static int subcmd_message_getbodycount(int argc, char **argv)
{
  if (!argc) {
    printf("?The getbodycount subcommand takes one or more arguments which specify\n?the messages to retrieve body count on; to retrieve body count for all messages, use a single * character.\n");
    return(TRUE);
  }
  while (argc--) {
    char *currentMessage = *argv++;
    if (strcmp(currentMessage,"*")==0) {
      SdmLinkedListIterator lit(MESSAGE_LIST);
      PmsgMessage *pmsg;

      while (pmsg = (PmsgMessage *)(lit.NextListElement())) {
	getbodycountMessage(pmsg);
      }
    }
    else {
      PmsgMessage *pmsg = lookupMessage(currentMessage, TRUE);
      if (pmsg)
	getbodycountMessage(pmsg);
    }
  }
  return(TRUE);
}

static int subcmd_message_getflags(int argc, char **argv)
{
  if (!argc) {
    printf("?The getflags subcommand takes one or more arguments which specify\n?the messages to retrieve flags on; to retrieve flags for all messages, use a single * character.\n");
    return(TRUE);
  }
  while (argc--) {
    char *currentMessage = *argv++;
    if (strcmp(currentMessage,"*")==0) {
      SdmLinkedListIterator lit(MESSAGE_LIST);
      PmsgMessage *pmsg;

      while (pmsg = (PmsgMessage *)(lit.NextListElement())) {
	getflagsMessage(pmsg);
      }
    }
    else {
      PmsgMessage *pmsg = lookupMessage(currentMessage, TRUE);
      if (pmsg)
	getflagsMessage(pmsg);
    }
  }
  return(TRUE);
}

static int subcmd_message_getcontents(int argc, char **argv)
{
  SdmError err;
  int asyncFlag = 0;
  SdmContentBuffer theContents;
  char *outputFileName = 0;
  SdmBoolean outputRedirected = Sdm_False;
  FILE* outputFd = stdout;
  SdmBoolean produceSum = Sdm_False;
  SdmBoolean produceHexdump = Sdm_False;

  if (argc < 1) {
    printf(
	   "?The getcontents subcommand takes the form: getcontents msgname [-F <outputfile>] [-S] [-H] [+a]\n",
	   "-F - place results of getcontents in specified file\n"
	   "     instead of displaying to standard output\n"
	   "-S - produce sum of results\n"
	   "-H - produce a hex dump of results instead of default raw output\n"
	   "+a means issue an Asynchronous Request for the command; otherwise,\n"
	   "the command is executed synchronously.\n");
    return(TRUE);
  }

  // Extract the message name
  PmsgMessage *pmsg = lookupMessage(*argv, TRUE);
  if (!pmsg)
    return(TRUE);
  argc--;
  argv++;

  SdmMessage *msg = pmsg->pmsg_message;
  assert(msg);

  while (argc && *argv[0] == '-') {
    // Extract optional get flags
    if (argc && strcmp(*argv, "+a")==0) {
      asyncFlag = Sdm_True;
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

  if (asyncFlag) {
    SdmServiceFunction svf(&getContentsAsyncCallback, (void *)2);
    if (msg->GetContents_Async(err, svf, pmsg) != Sdm_EC_Success) {
      printf("message %s: GetContents async for store %s msgnum %d failed with error %u: %s\n", 
	     (const char *)*pmsg->pmsg_messagename, (const char *)*pmsg->pmsg_messagestorename, pmsg->pmsg_messagenum, 
	     (SdmErrorCode) err, err.ErrorMessage());
      if (outputFd)
	fclose(outputFd);
    }
    else {
      printf("message %s: GetContents async for store %s msgnum %d successful: pending\n", 
	     (const char *)*pmsg->pmsg_messagename, (const char *)*pmsg->pmsg_messagestorename, pmsg->pmsg_messagenum, 
	     (SdmErrorCode) err, err.ErrorMessage());
    }
    return(TRUE);
  }

  if (msg->GetContents(err, theContents) != Sdm_EC_Success) {
    printf("message %s: failed to get content buffer for store %s msgnum %d with error %u: %s\n", 
	   (const char *)*pmsg->pmsg_messagename, (const char *)*pmsg->pmsg_messagestorename, pmsg->pmsg_messagenum, 
	   (SdmErrorCode) err, err.ErrorMessage());
    if (outputFd)
      fclose(outputFd);
    return(TRUE);
  }

  printf("message %s: getcontents successful\n", expandMessageName(pmsg));

  // Now need to get "the real contents" of the message

  SdmString str_contents;
  if (theContents.GetContents(err, str_contents)) {
    printf("message %s: failed to get contents for store %s msgnum %d with error %u: %s\n", 
	   (const char *)*pmsg->pmsg_messagename, (const char *)*pmsg->pmsg_messagestorename, pmsg->pmsg_messagenum, 
	   (SdmErrorCode) err, err.ErrorMessage());
    if (outputFd)
      fclose(outputFd);
    return(TRUE);  
  }

  if (produceSum) {
    unsigned first = 0;
    long second;

    computeSum(str_contents, first, second);
    printf("message %s: sum is %u %ld\n", expandMessageName(pmsg), first, second);
  }

  if (outputRedirected) {
    if (produceHexdump)
      HexDump(outputFd, "hex dump of message", (const unsigned char *)((const char *)str_contents), str_contents.Length(), 0, TRUE);
    else
      fprintf(outputFd, "%s", (const char *)str_contents);
    fclose(outputFd);
  }
  else {
    if (produceHexdump)
      HexDump(stdout, "hex dump of message", (const unsigned char *)((const char *)str_contents), str_contents.Length(), 0, TRUE);
    else
      printf("%s\n", (const char *)str_contents);
  }

  return(TRUE);
}

static int subcmd_message_getheaders(int argc, char **argv)
{
  SdmString header;		// holds returned header list
  SdmStringL headerList;	// holds returned header list of lists
  SdmBoolean headerListSet = Sdm_False;	// indicates if headerList is set
  SdmBoolean headerSet = Sdm_False;	// indicates if header is set
  SdmMessageHeaderAbstractFlags headerAbstractFlags = 0;
  int abstractFlag = 0;
  int abstractRetrieveFlag = 0;
  SdmError err;

  if (!argc) {
    printf(
"?The getheaders command takes several forms:"
"\tgetheaders msgname\n"
"\tgetheaders msgname -a|-A\n"
"\tgetheaders msgname header(s)\n"
"\tgetheaders msgname -a|-A abstractheader(s)\n"
"\tThe -a flag indicates to retrieve headers by abstract header names\n"
"\tas opposed to header name strings themselves.\n"
"\tThe -A flag is identical to -a but additionally retrieves the headers by abstract\n"
"\theader name/header value pairs as opposed to header name/header value pairs.\n"
);
    return(TRUE);
  }

  // Extract the message name
  PmsgMessage *pmsg = lookupMessage(*argv, TRUE);
  if (!pmsg)
    return(TRUE);
  argc--;
  argv++;

  SdmMessage *msg = pmsg->pmsg_message;
  assert(msg);

  SdmMessageEnvelope *menv = 0;
  if (msg->SdmMessageEnvelopeFactory(err, menv) != Sdm_EC_Success) {
    printf("message %s: failed to get envelope for store %s msgnum %d with error %u: %s\n", 
	   (const char *)*pmsg->pmsg_messagename, (const char *)*pmsg->pmsg_messagestorename, pmsg->pmsg_messagenum, 
	   (SdmErrorCode) err, err.ErrorMessage());
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

  SdmStrStrL headerValue;
  SdmIntStrL headerValueA;

  if (abstractFlag) {
    if (abstractRetrieveFlag) {
      // getheaders store -A msg abs
      menv->GetHeaders(err, headerValueA, headerAbstractFlags);
    }
    else {
      // getheaders store -a msg abs
      menv->GetHeaders(err, headerValue, headerAbstractFlags);
    }
  }
  else if (!headerSet && !headerListSet) {
    // getheaders store msg - get all headers for a single message
    menv->GetHeaders(err, headerValue);
  }
  else if (headerSet && !headerListSet) {
    // getheaders store msg hdr - get a single header for a single message
    menv->GetHeader(err, headerValue, header);
  }
  else if (!headerSet && headerListSet) {
    // getheaders store msg hdr1 hdr2
    menv->GetHeaders(err, headerValue, headerList);
  }
  if (err != Sdm_EC_Success) {
    printf("message %s: getheaders failed with error %u: %s\n", expandMessageName(pmsg), (SdmErrorCode) err, err.ErrorMessage());
    return(TRUE);
  }

  printf("message %s: getheaders successful\n", expandMessageName(pmsg));

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
  return(TRUE);
}

static int subcmd_message_getstructure(int argc, char **argv)
{
  if (!argc) {
    printf("?The getstructure subcommand takes one or more arguments which specify\n?the messages to retrieve structure on; to retrieve structure for all messages, use a single * character.\n");
    return(TRUE);
  }
  while (argc--) {
    char *currentMessage = *argv++;
    if (strcmp(currentMessage,"*")==0) {
      SdmLinkedListIterator lit(MESSAGE_LIST);
      PmsgMessage *pmsg;

      while (pmsg = (PmsgMessage *)(lit.NextListElement())) {
	getstructureMessage(pmsg);
      }
    }
    else {
      PmsgMessage *pmsg = lookupMessage(currentMessage, TRUE);
      if (pmsg)
	getstructureMessage(pmsg);
    }
  }
  return(TRUE);
}

static int subcmd_message_setflags(int argc, char **argv)
{
  return messageSetFlags(argc, argv, Sdm_True);
}

static int subcmd_message_status(int argc, char **argv)
{
  if (!argc) {
    printf("?The status subcommand takes one or more arguments which specify\n?the messages to return status on; to status all messages, use a single * character.\n");
    return(TRUE);
  }
  while (argc--) {
    char *currentMessage = *argv++;
    if (strcmp(currentMessage,"*")==0) {
      SdmLinkedListIterator lit(MESSAGE_LIST);
      PmsgMessage *pmsg;

      while (pmsg = (PmsgMessage *)(lit.NextListElement())) {
	statusMessage(pmsg);
      }
    }
    else {
      PmsgMessage *pmsg = lookupMessage(currentMessage, TRUE);
      if (pmsg)
	statusMessage(pmsg);
    }
  }
  return(TRUE);
}
