#include <SDtMail/Sdtmail.hh>
#include <SDtMail/LinkedList.hh>
#include <SDtMail/Connection.hh>
#include <PortObjs/DataPort.hh>
#include <porttest.hh>
#include <ptcmdscan.hh>
#include <ptcconnection.hh>

/*
** EXTERNALS
*/

/*
** GLOBALS
*/
SdmConnection *theConnectionObject = (SdmConnection *)0;
static char* theConnectionData = "hello world2";

char *Hconnection =
"Description: manipulation primary midend connection\n"
"General usage: connection command [arguments]\n"
"Specific usage:\n"
"\tconnection create\n"
"\tconnection delete\n"
"\tconnection shutdown\n"
"\tconnection startup\n"
"";

static int subcmd_connection_create(int argc, char **argv);
static int subcmd_connection_delete(int argc, char **argv);
static int subcmd_connection_startup(int argc, char **argv);
static int subcmd_connection_shutdown(int argc, char **argv);

static SUBCMD connectioncmdtab[] = {
  // cmdname,		handler,			argsrequired
  {"create",		subcmd_connection_create,	0},	// create connection
  {"delete",		subcmd_connection_delete,		0},	// delete connection
  {"shutdown",		subcmd_connection_shutdown,		0},	// shutdown connection
  {"startup",		subcmd_connection_startup,		0},	// startup connection
  {NULL,		0,				0}	// must be last entry in table
};

/*
** FORWARD DECLARATIONS
*/

/*************************************************************************
 *
 * Function:	Cconnection - 
 *
 * Input:	int pargc - number of arguments passed to command
 *		char **pargv - array of pargc arguments passed
 *
 * Output:	TRUE - help given
 *
 ************************************************************************/

int Cconnection(int argc, char **argv)
{
  return (argc == 1 ? TRUE : oneSubCommand(argv[0], argv[1], connectioncmdtab, argc-2, argv+2));
}

static void connectionAsyncCallback(SdmError& err, void* userdata, SdmServiceFunctionType type, 
  SdmEvent* event)
{
  assert (event != NULL);

  printf("connectionAsyncCallback invoked\n");
  if (userdata != theConnectionData) {
    printf("*** Error: userdata given is not correct.\n");
  }

#if 0  
  switch (event->type) {
    case SdmGetPartialContents:  
      {    
        char buffer[2000];
        int stOffS = event->sdmGetContents.startOffset;
        int length = event->sdmGetContents.length;
      
        sprintf(buffer, "AsyncTestCallback called with args=%d,%d, buffer=%s\n",
          stOffS, length, (const char *)(*(event->sdmGetContents.buffer)));
        cout << buffer;
        break;
      }
      
    default:
      cout << "Error: AsyncTestCallback default event received.\n";
      break;      
  }
#endif
}

static int subcmd_connection_create(int argc, char **argv)
{
  SdmError err;

  if (argc) {
    printf("?There are no arguments to the create subcommand\n");
    return(TRUE);
  }

  if (theConnectionObject) {
    printf("?A connection already exists - only one connection allowed at a time\n");
    return(TRUE);
  }

  theConnectionObject = new SdmConnection("sdmporttest");
  if (theConnectionObject == NULL) {
    printf("connection: creation failed.\n");
    delete theConnectionObject;
    theConnectionObject = (SdmConnection *)0;
    return(TRUE);
  }

  printf("connection: created\n");
  return(TRUE);
}

static int subcmd_connection_delete(int argc, char **argv)
{
  SdmError err;

  if (!theConnectionObject) {
    printf("?A connection has not yet been created\n");
    return(TRUE);
  }

  delete theConnectionObject;
  theConnectionObject = (SdmConnection *)0;
  printf("connection: deleted\n");
  return(TRUE);
}

static void GroupPrivilegeActionCallback(void* userData, SdmBoolean enableFlag)
{
  printf("Connection: group privilege action: %s\n", enableFlag ? "enable" : "disable");
  return;
}

static int subcmd_connection_startup(int argc, char **argv)
{
  SdmError err;

  if (!theConnectionObject) {
    printf("?A connection has not yet been created\n");
    return(TRUE);
  }

  theConnectionObject->StartUp(err);
  if (!err) {
    SdmServiceFunction svf(&connectionAsyncCallback, (void *)theConnectionData, Sdm_SVF_Any);
    if (theConnectionObject->RegisterServiceFunction(err, svf) != Sdm_EC_Success) {
      printf("connection: error: could not register SVF_Any service function\n");
      delete theConnectionObject;
    }
  }
  theConnectionObject->RegisterGroupPrivilegeActionCallback(err, GroupPrivilegeActionCallback, (void*) 0);
  printf("connection: startup");
  printf(err == Sdm_EC_Success ? " successful\n" : " failed with error %u\n", (SdmErrorCode) err);
  return(TRUE);
}

static int subcmd_connection_shutdown(int argc, char **argv)
{
  SdmError err;

  if (!theConnectionObject) {
    printf("?A connection has not yet been created\n");
    return(TRUE);
  }

  theConnectionObject->ShutDown(err);
  printf("connection: shutdown");
  printf(err == Sdm_EC_Success ? " successful\n" : " failed with error %u\n", (SdmErrorCode) err);
  return(TRUE);
}
