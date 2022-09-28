/*
** INCLUDES
*/

#include <porttest.hh>
#include <sys/wait.h>
#include <ptcmdscan.hh>
#include <SDtMail/MessageUtility.hh>
#include <SDtMail/SystemUtility.hh>
#include <Utils/LockUtility.hh>

// include the prototypes for the various commands that can be executed
//
#include <ptcbody.hh>		// body command
#include <ptcconnection.hh>		// connection command
#include <ptcenvelope.hh>		// envelope command
#include <ptchelp.hh>		// help and ? commands
#include <ptclock.hh>		// lock command
#include <ptcmessage.hh>		// message command
#include <ptcoverview.hh>		// overview command
#include <ptcport.hh>		// port command
#include <ptcquit.hh>		// quit command
#include <ptcsession.hh>		// Session command
#include <ptcstore.hh>		// Store command
#include <ptctoken.hh>		// token command

/*
** GLOBALS
*/

static char	cmdline[2048+1];	// Command lines are entered here 
static int	margc;			// Number of arguments to a command 
static char	*margv[256];		// Possible arguments to a command 

/*
** STATIC GLOBALS
*/
static char* Herrormessages = "Usage: errormessages\n"
"Print detailed list of all error messages suitable\n"
"for error message catalog inclusion.\n";

static char* HremoveEmptyStore = "Usage: removeemptystore messageStoreName\n"
"Remove the indicated message store if it is empty\n";

static char* Hsleep = "Usage: sleep seconds\n"
"Sleep for the indicated interval allowing messages to be processed\n";

static char* Hsetprompt = "Usage: setprompt prompt\n"
"Set the prompt to be given on command lines\n";

static char* HverifyHost = "Usage: verifyhost hostname [service [port]]\n"
"Verify that the given host name has the specified service at the specified port address.\n"
"Host can be +this to specify the host where sdmporttest is running.\n"
"Service defaults to TCP and port defaults appropriately depending on service.\n"
"Service can be +all to specify all known services, or +none to mean no specific service.\n"
"If service is -, then the port may be specified in which case that port is tested.\n"
"Example: test existence of host:       verifyhost testhost\n"
"         (same as above):              verifyhost testHOST +none\n"
"         test port 25 (smtp) on host:  verifyhost testhost +none 25\n"
"         test port 143 (imap) on host: verifyhost testhost +none 143\n"
"         test smtp service on host:    verifyhost testhost smtp\n"
"         test imap service on host:    verifyhost testhost imap\n"
"         test all services on host:    verifyhost testhost +all\n"
"         test all services on this host: verifyhost +this +all\n"
;

int Cerrormessages(int argc, char **argv);
int CremoveEmptyStore(int argc, char **argv);
int Csleep(int argc, char **argv);
int Csetprompt(int argc, char **argv);
int CverifyHost(int argc, char **argv);

/*
** GLOBALS
*/

CMD maincmdtab[] = {
  /* cmdname,	helpstr,	cmdroutine,	nocmdline, nopts */
  { "body", 	Hbody,		Cbody,		TRUE,	FALSE },
  { "connection", Hconnection,	Cconnection,	TRUE,	FALSE },
  { "envelope", Henvelope,	Cenvelope,	TRUE,	FALSE },
  { "errormessages", Herrormessages, Cerrormessages, FALSE, TRUE },
  { "help",	Hhelp,		Chelp,		FALSE,	FALSE },
  { "?",	Hhelp,		Chelp,		FALSE,	FALSE },
  { "lock",	Hlock,		Clock,		FALSE,  FALSE },
  { "message",	Hmessage,	Cmessage,	TRUE,	FALSE },
  { "overview",	Hoverview,	Coverview,	FALSE,	TRUE },
  { "port",	Hport,		Cport,		TRUE,  	FALSE },
  { "quit",	Hquit,		Cquit,		TRUE,	TRUE },
  { "removeemptystore", HremoveEmptyStore, CremoveEmptyStore, FALSE, FALSE },
  { "session",	Hsession,	Csession,	TRUE,	FALSE },
  { "setprompt", Hsetprompt,	Csetprompt,	TRUE,   FALSE },
  { "sleep",	Hsleep,		Csleep,		TRUE,	FALSE },
  { "store",	Hstore,		Cstore,		TRUE,	FALSE },
  { "token",	Htoken,		Ctoken,		FALSE,	FALSE },
  { "verifyhost", HverifyHost,	CverifyHost,	FALSE,	FALSE },
  { NULL,	NULL,		0,		0,	0 }, // MUST BE LAST 
};

/*
** FORWARD DECLARATIONS
*/

static void makeargv(char *pcmdline);
static int interactiveCommand();
static int batchCommand();

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

/*************************************************************************
 *
 * Function:	cmdScanner - read commands and process
 *
 * Input:	NONE
 *
 * Output:	TRUE - command processed ok ready for next command
 *		FALSE - finished taking commands, can exit
 *
 ************************************************************************/

int cmdScanner()
{
  if (isatty0)
    return(interactiveCommand());
  return(batchCommand());
}

/*************************************************************************
 *
 * Function:	interactiveCommand - process commands when stdin is tty
 *
 * Input:	NONE
 *
 * Output:	TRUE - command processed ok ready for next command
 *		FALSE - finished taking commands, can exit
 *
 ************************************************************************/

static int interactiveCommand()
{
  register CMD *c;
  char *p;
  
  printf("%s> ",Prompt ? Prompt : Command);		/* Issue prompt */
  (void) fflush(stdout);		/* Force output now */
  
  AllowInterrupts(TRUE);
  p = getCommandString(cmdline);		/* Get input line from stdin */
  AllowInterrupts(FALSE);
  
  if (p == NULL)			/* END OF FILE FROM STDIN? */
    return(FALSE);		/* Yes, exit */
  
  if (cmdline[0] == 0)		/* Null command? */
    return(TRUE);		/* Yes, get next command */

  if (cmdline[0] == '#')	/* Comment line? */
    return(TRUE);		/* Yes, get next command */

  if (cmdline[0] == '!')	/* shell escape line?? */
    {
      if (cmdline[1] == 0)	/* no command following !? */
	return(TRUE);
      int results = system(&cmdline[1]);
      if (results) {
	int saveerrno = errno;
	printf("Results of command execution = %d status = %d", results, WEXITSTATUS(results));
	if (saveerrno)
	  printf(" errno = %d message = %s\n", errno, strerror(errno));
	else
	  printf("\n");
	if (WIFEXITED(results))
	  printf("Extra status info: child process terminated abnormally\n");
	if (WIFSIGNALED(results))
	  printf("Extra status info: child process terminated due to signal\n");
      }
      else
	printf("Command executed successfully\n");
      return(TRUE);
    }

  makeargv(cmdline);
  c = getcmd(margv[0], maincmdtab);
  if (c == (CMD *)-1)
    {
      printf("?Ambiguous command\n");
      return(TRUE);
    }
  if (c == 0)
    {
      printf("?Invalid command\n");
      return(TRUE);
    }
  if (margc > 1 && c->c_nopts)
    {
      printf("?The %s command does not accept any command arguments\n",c->c_name);
      return(TRUE);
    }
  margv[0] = c->c_name;	/* Pass FULL name to command */
  return((int)(*c->c_handler)(margc, margv));
}

/*************************************************************************
 *
 * Function:	batchCommand - process commands when stdin is NOT tty
 *
 * Input:	NONE
 *
 * Output:	TRUE - command processed ok ready for next command
 *		FALSE - finished taking commands, can exit
 *
 ************************************************************************/

static int batchCommand()
{
  register CMD *c;
  char *pcl;
  
  pcl = getCommandString(cmdline);		/* Get line */
  if (pcl == NULL)		/* End of file? */
    {
      printf("%s> quit\n",Prompt ? Prompt : Command);
      (void) fflush(stdout);
      Cquit(1, 0);
      return(FALSE);
    }
  
  printf("%s> %s\n",Prompt ? Prompt : Command,cmdline);	/* Fake command line */
  (void) fflush(stdout);			/* Force it out now */
  
  if (cmdline[0] == 0)		/* Null command? */
    return(TRUE);		/* Yes, get next command */
  
  if (cmdline[0] == '#')	/* Comment line? */
    return(TRUE);		/* Yes, get next command */

  if (cmdline[0] == '!')	/* shell escape line?? */
    {
      if (cmdline[1] == 0)	/* no command following !? */
	return(TRUE);
      int results = system(&cmdline[1]);
      if (results) {
	int saveerrno = errno;
	printf("Results of command execution = %d status = %d", results, WEXITSTATUS(results));
	if (saveerrno)
	  printf(" errno = %d message = %s\n", errno, strerror(errno));
	else
	  printf("\n");
	if (WIFEXITED(results))
	  printf("Extra status info: child process terminated abnormally\n");
	if (WIFSIGNALED(results))
	  printf("Extra status info: child process terminated due to signal\n");
      }
      else
	printf("Command executed successfully\n");
      return(TRUE);
    }

  makeargv(cmdline);
  c = getcmd(margv[0], maincmdtab);
  if (c == (CMD *)-1)
    {
      printf("?Ambiguous command\n?Remainder of input discarded\n");
      return(FALSE);
    }
  if (c == 0)
    {
      printf("?Invalid command\n?Remainder of input discarded\n");
      return(FALSE);
    }
  if (margc > 1 && c->c_nopts)
    {
      printf("?The %s command does not accept any command arguments\n?Remainder of input discarded\n",c->c_name);
      return(FALSE);
    }
  margv[0] = c->c_name;	/* Pass FULL name to command */
  return((int)(*c->c_handler)(margc, margv));
}

/*************************************************************************
 *
 * Function:	oneCommand - process commands given argv and argc
 *
 * Input:	char * to command to parse and execute
 *
 * Output:	TRUE - command processed ok ready for next command
 *		FALSE - finished taking commands, can exit
 *
 ************************************************************************/

int oneCommand(char *pcmd)
{
  register CMD *c;
  
  makeargv(pcmd);
  c = getcmd(margv[0], maincmdtab);
  if (c == (CMD *)-1)
    {
      printf("?Ambiguous command\n");
      return(FALSE);
    }
  if (c == 0)
    {
      printf("?Invalid command\n");
      return(FALSE);
    }
  if (c->c_nocmdline) {
    printf("?The %s command cannot be used from the system command line\n",c->c_name);
    return(FALSE);
  }
  if (margc > 1 && c->c_nopts)
    {
      printf("?The %s command does not accept any command arguments\n",c->c_name);
      return(FALSE);
    }
  
  margv[0] = c->c_name;	/* Pass FULL name to command */
  return((int)(*c->c_handler)(margc, margv));
}

/*************************************************************************
 *
 * Function:	oneSubCommand - process commands given argv and argc
 *
 * Input:	char * to command to parse and execute
 *
 * Output:	TRUE - command processed ok ready for next command
 *		FALSE - finished taking commands, can exit
 *
 ************************************************************************/

int oneSubCommand(char *cmdname, char *subcmdname, SUBCMD *subcmdtab, int argc, char **argv)
{
  register SUBCMD *c;
  
  c = getsubcmd(subcmdname, subcmdtab);
  if (c == (SUBCMD *)-1)
    {
      printf("?Ambiguous subcommand: %s\n", subcmdname);
      return(TRUE);
    }
  if (c == 0)
    {
      printf("?Invalid subcommand: %s\n", subcmdname);
      return(TRUE);
    }
  if (c->c_argsrequired == -1 && argc != 0) {
    printf("?The '%s' subcommand of the '%s' command does not take any arguments\n", c->c_cmdname, cmdname);
    return(TRUE);
  }

  if (c->c_argsrequired > argc) {
    printf("?The '%s' subcommand of the '%s' command requires at least %d arguments\n", c->c_cmdname, cmdname, c->c_argsrequired);
    return(TRUE);
  }
  return((int)(*c->c_subhandler)(argc, argv));
}

/*************************************************************************
 *
 * Function:	makeargv - slice up given line into margv/margc
 *
 * Input:	char * command line to slice up
 *
 * Output:	NONE
 *
 * Globals:	margc - number of arguments sliced from command line
 *		margv - array points to individual arguments
 *
 ************************************************************************/

static void makeargv(char *pcmdline)
{
  register char *cp,*cpp;
  register char **argp = margv;
  char lastc;
  
  margc = 0;
  lastc = '\0';
  for (cp = pcmdline; *cp;)
    {
      while (isspace(*cp))
	cp++;
      if (*cp == '\0')
	break;
      *argp++ = cpp = cp;
      margc += 1;
      while (*cp != '\0' && (lastc == '\\' || !isspace(*cp)))
	{
	  if (lastc == '\\')
	    {
	      lastc = '\0';
	      *cpp++ = *cp++;
	    }
	  else if (*cp == '\\')
	    lastc = *cp++;
	  else
	    lastc = *cpp++ = *cp++;
	}
      if (*cp == '\0')
	{
	  *cpp++ = '\0';
	  break;
	}
      *cpp++ = '\0';
      *cp++ = '\0';
    }
  *argp++ = 0;

  // Postprocess the list and do item substitution:
  // --"" gets an empty string substituted
  // --0 gets a null pointer returned

  for (int i = 0; i < margc; i++) {
    if (strcmp(margv[i], "--\"\"")==0)
      margv[i] = "";
    else if (strcmp(margv[i], "--0")==0)
      margv[i] = 0;
  }
}

/*************************************************************************
 *
 * Function:	getcmd - given command string, look up in command table
 *
 * Input:	*name - command name to look up
 *
 * Output:	(CMD *)-1 - Ambiguous command entered
 *		(CMD *)0 - Invalid command entered
 *		else (CMD *) to cmd structure of command entered
 *
 ************************************************************************/

CMD *getcmd(char *name, CMD *cmdtab)
{
  register char *p, *q;
  register CMD *c, *found;
  register int nmatches, longest;
  
  longest = 0;
  nmatches = 0;
  found = 0;
  for (c = cmdtab; c->c_name != NULL; c++) 
    {
      p = c->c_name;
      for (q = name; *q == *p++; q++)
	if (*q == 0)		/* exact match? */
	  return(c);
      if (!*q)			/* the name was a prefix */
	{
	  if (q - name > longest)
	    {
	      longest = q - name;
	      nmatches = 1;
	      found = c;
	    }
	  else
	    {
	      if (q - name == longest)
		nmatches++;
	    }
	}
    }
  if (nmatches > 1)
    return((CMD *)-1);
  return(found);
}

/*************************************************************************
 *
 * Function:	getsubcmd - given command string, look up in command table
 *
 * Input:	*name - command name to look up
 *
 * Output:	(CMD *)-1 - Ambiguous command entered
 *		(CMD *)0 - Invalid command entered
 *		else (CMD *) to cmd structure of command entered
 *
 ************************************************************************/

SUBCMD *getsubcmd(char *cmdname, SUBCMD *subcmdtab)
{
  register char *p, *q;
  register SUBCMD *c, *found;
  register int nmatches, longest;
  
  longest = 0;
  nmatches = 0;
  found = 0;
  for (c = subcmdtab; c->c_cmdname != NULL; c++) 
    {
      p = c->c_cmdname;
      for (q = cmdname; *q == *p++; q++)
	if (*q == 0)		/* exact match? */
	  return(c);
      if (!*q)			/* the cmdname was a prefix */
	{
	  if (q - cmdname > longest)
	    {
	      longest = q - cmdname;
	      nmatches = 1;
	      found = c;
	    }
	  else
	    {
	      if (q - cmdname == longest)
		nmatches++;
	    }
	}
    }
  if (nmatches > 1)
    return((SUBCMD *)-1);
  return(found);
}

char *getCommandString(char *s)
{
  int result;

  for (;;) {

    // Wait until an error or at least on hooked file descriptor has a processable event

    while ((result = poll(pollFileDesc, numPollFileDescEntries, -1)) == 0);
    if (result == -1 && errno == EINTR)
      continue;	// probably a ^C - try again
    if (result == -1) {
      printf("\n?ERROR: poll returned %d errno %d\n", result, errno);
      assert(result != -1);
      exit(1);
    }

    // spin through file descriptor poll list handling processable events

    for (int i = numPollFileDescEntries-1; i >= 0; i--) {
      int targFd = pollFileDesc[i].fd;
      if (targFd != -1 && (pollFileDesc[i].revents & pollFileDesc[i].events)) {
	if (targFd == fileno(stdin))
	  return(gets(s));
	else if (targFd == theInputFileDesc) {
	  SdmHandleSessionEvent(theInputQueue, &theInputFileDesc, NULL);
	  *s = '\0';
	  return(s);
	}
	else if (targFd == theOutputFileDesc) {
	  SdmHandleSessionEvent(theOutputQueue, &theOutputFileDesc, NULL);
	  *s = '\0';
	  return(s);
	}
	else if (targFd == theTooltalkFileDesc) {
	  // handle tooltalk event
	  int always = 1;
	  printf("\nProcessing tooltalk asynchronous event.\n");
	  tttk_block_while((XtAppContext)0, &always, 0);
	  *s = '\0';
	  return(s);
	}
	else {
	  printf("\n?ERROR: poll returned event %d on fd %d which is not hooked in\n", pollFileDesc[i].revents, targFd);
	  assert(0);
	  exit(1);
	}
      }
    }
  }
}

// Produce list of error messages

int Cerrormessages(int argc, char **argv)
{
  for (SdmErrorCode i = 1; i < 100000; i++) {
    SdmError err;
    err = i;
    const char *msg = err.ErrorMessage();
    if (::strncmp(msg, "Unknown error code", 18) != 0) {
      printf("%u \"", i);
      for (const char *p=msg; *p; p++) {
	if (p[0] == '\n')
	    printf("\\n");
	else if (p[0] == '"')
	  printf("\\\"");
	else if (p[0] == '\\')
	  printf("\\\\");
	else
	  printf("%c", *p);
      }
      printf("\"\n");
    }
  }
  return(TRUE);
}

int CremoveEmptyStore(int argc, char **argv)
{
  assert(argc);
  if (argc != 2) {
    printf("Usage: removeemptystore storename\n");
    return(TRUE);
  }

  SdmString messageStoreName = argv[1];
  SdmError err;
  SdmMessageUtility::RemoveMessageStoreIfEmpty(err, messageStoreName);
  printf("The message store %s removed.\n", 
	 access((const char *)messageStoreName, F_OK) == -1 ? "was" : "was NOT");
    
  printf("Remove message store results: %s", detailedErrorReport(err));
  return(TRUE);
}

int Csleep(int argc, char **argv)
{
  assert(argc);
  if (argc != 2) {
    printf("Usage: sleep seconds\n");
    return(TRUE);
  }

  assert(argv[1]);
  char *stop = argv[1];
  long secondsToSleep = strtol(argv[1], &stop, 10);
  if (argv[1] == stop || *stop != '\0') {
    printf("The input to sleep must be a valid integer number of seconds to sleep: %s\n", argv[1]);
    return(TRUE);
  }

  fprintf(stderr, "Sleeping for %u seconds.\n", (unsigned long) secondsToSleep);
  fflush(stderr);

  // For this purpose, remove the waiting on standard input as we dont want to
  // process any commands until the wait period is over

  removePollFileDesc(fileno(stdin));

  // Wait until an error or at least on hooked file descriptor has a processable event

  int result;

  while ((result = poll(pollFileDesc, numPollFileDescEntries, secondsToSleep*1000)) != 0) {
    if (result == -1 && errno == EINTR) {
      printf("Interrupted!\n");
      break;
    }
    else if (result == -1) {
      printf("\n?ERROR: poll returned %d errno %d\n", result, errno);
      assert(result != -1);
      exit(1);
    }
    else {
      // spin through file descriptor poll list handling processable events

      for (int i = 0; i < numPollFileDescEntries; i++) {
	int targFd = pollFileDesc[i].fd;
	if (targFd != -1 && (pollFileDesc[i].revents & pollFileDesc[i].events)) {
	  assert(targFd != fileno(stdin));
	  if (targFd == theInputFileDesc) {
	    SdmHandleSessionEvent(theInputQueue, &theInputFileDesc, NULL);
	  }
	  else if (targFd == theOutputFileDesc) {
	    SdmHandleSessionEvent(theOutputQueue, &theOutputFileDesc, NULL);
	  }
	  else if (targFd == theTooltalkFileDesc) {
	    // handle tooltalk event
	    int always = 1;
	    printf("\nProcessing tooltalk asynchronous event.\n");
	    tttk_block_while((XtAppContext)0, &always, 0);
	  }
	  else {
	    printf("\n?ERROR: poll returned event %d on fd %d which is not hooked in\n", pollFileDesc[i].revents, targFd);
	    assert(0);
	    exit(1);
	  }
	}
      }
    }
  }

  // Done with sleep - put stdin back in the poll list so commands can be processed

  addPollFileDesc(fileno(stdin), POLLIN);
  fprintf(stderr, "Sleep command execution completed.\n");
  fflush(stderr);
  return(TRUE);
}

int Csetprompt(int argc, char **argv)
{
  assert(argc);
  if (argc != 2) {
    printf("Usage: setprompt prompt\n");
    return(TRUE);
  }

  assert(argv[1]);
  Prompt = strdup(argv[1]);
  return(TRUE);
}

static void verifyService(char* hostname, char* service, int port)
{
  SdmError err;

  if ( (service && (strcasecmp(service, "tcp")==0) || !service || !*service) && !port)
    port = 7;	// default to echo service for tcp with no port specified

    if (SdmSystemUtility::TcpServiceVerify(err, hostname, service, port) != Sdm_EC_Success) {
      printf("Service '%s' on host '%s' port '%d' not available: %s", service && *service ? service : "(DEFAULT)", hostname, port, detailedErrorReport(err));
    }
    else {
      printf("Service '%s' on host '%s' port '%d' is available.\n", service && *service ? service : "(DEFAULT)", hostname, port);
    }
}

static void verifySmtpService(char* hostname, int port)
{
  SdmError err;

    if (SdmSystemUtility::SmtpServiceVerify(err, hostname, port) != Sdm_EC_Success) {
      printf("Smtp service on host '%s' port '%d' not available: %s", hostname, port, detailedErrorReport(err));
    }
    else {
      printf("Smtp service on host '%s' port '%d' is available.\n", hostname, port);
    }
}

static void verifyImapService(char* hostname, int port)
{
  SdmError err;

    if (SdmSystemUtility::ImapServiceVerify(err, hostname, port) != Sdm_EC_Success) {
      printf("Imap service on host '%s' port '%d' not available: %s", hostname, port, detailedErrorReport(err));
    }
    else {
      printf("Imap service on host '%s' port '%d' is available.\n", hostname, port);
    }
}

int CverifyHost(int argc, char **argv)
{
  if (argc < 2 || argc > 4) {
    printf("%s", HverifyHost);
    return(TRUE);
  }

  argv++; argc--;	// bypass command argument

  char* hostname = *argv++; argc--;
  if (strcmp(hostname, "+this")==0)
    hostname = SdmSystemUtility::GetHostName();
  char* service = "";
  if (argc) {
    service = *argv++; argc--;
  }

  if (strcmp(service,"+none")==0)
    service = NULL;

  int port = 0;

  if (argc) {
    port = atol(*argv);
  }

  // First off, see if the host even exists - basic test will suffice
  SdmError err;
  if (SdmSystemUtility::TcpServiceVerify(err, hostname, "echo", 7) != Sdm_EC_Success) {
    printf("Host '%s': %s", hostname, detailedErrorReport(err));
    return(TRUE);
  }

  if (service && (strcmp(service,"+all") == 0)) {
    printf("Direct verifications:\n");
    verifyService(hostname, "tcp", port);
    verifyService(hostname, "imap", port);
    verifyService(hostname, "smtp", port);
    verifyService(hostname, "nntp", port);
    printf("Indirect verifications:\n");
    verifyImapService(hostname, port);
    verifySmtpService(hostname, port);
  }
  else {
    verifyService(hostname, service, port);
  }

  return(TRUE);
}
