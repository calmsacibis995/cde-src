/*
** INCLUDES
*/

#include <porttest.hh>
#include <ptcmdscan.hh>
#include <ptcquit.hh>
#include <Utils/LockUtility.hh>
#include <Tt/tt_c.h>
#include <Tt/tttk.h>

/*
** EXTERNALS
*/
SdmBoolean __LogOn = Sdm_True; /*Optional Messaging Log On or Off*/

/*
** GLOBALS
*/

int	isatty0 = {0};			// initialized to isatty(0)
int	isatty1 = {0};			// initialized to isatty(1)
int	UserInterrupt = {FALSE}; 	// TRUE when user strikes ^C
int 	vflag = {0};	    		// -v: debug / verbose mode
int 	iflag = {0};	    		// -i: take input from named file
char	*Command = "Sdmporttest";	// Name of this program
char	*Prompt = 0;			// Overriding prompt
char	*CommandFile = 0;		// argument to -i
int	theInputFileDesc = -1;
int	theOutputFileDesc = -1;
int	theTooltalkFileDesc = -1;
void	*theInputQueue = 0;
void	*theOutputQueue = 0;
struct pollfd *pollFileDesc = 0;	
int	numPollFileDescEntries = 0;
PtResult ptLastResult;

static jmp_buf	jmpbuf;			// "jump" buffer used in catchsig
static int	JmpOnInterrupt = {FALSE};
static char	*Cmdline = {(char *)0};	// Cmd line text here


/*
** FORWARDS
*/
static void parseCommands(int pargc, char **pargv);
static void initialization(int pargc, char **pargv);
static void prepareFunction();
static void performFunction();
static int parseArg(char *parg);
static void termination();
static void exitProg(int exvalue);
static void catchsig(int);
static void doInteractiveCommands();
static void donecmd(char *pcmd);
static void usage();


/*************************************************************************
 *
 * Function:    main - program startup procedure
 *
 * Input:       int pargc - Count of arguments passed from command line
 *              char **pargv - Pointer to (argc) individual argument strings
 *
 * Output:      NONE
 *
 * Called by:   UNIX O/S SOFTWARE
 *
 ************************************************************************/

int main(int pargc, char **pargv)
{
  initialization(pargc, pargv);	// Perform necessary initialization 
  parseCommands(pargc, pargv);	// Parse command line 
  prepareFunction();		// Prepare to process based on commands
  performFunction();		// Process based on commands given 
  exitProg(0);			// Perform termination activities 
}

/*************************************************************************
 *
 * Function:    parseCommands - parse UNIX command line parameters
 *
 * Input:       int pargc - Count of arguments passed from command line
 *              char **pargv - Pointer to (argc) individual argument strings
 *
 * Output:      NONE
 *
 ************************************************************************/

static void parseCommands(int pargc, char **pargv)
{
  register int i,j;
  
  /*
  ** Crack command line arguments and set appropriate flags,etc.
  */
  
  if (pargc > 1)
    for(i = 1; i < pargc; i++) 
      {
	if ( pargv[i][0] != '-' )			// Command switch? 
	  {
	    if (parseArg(pargv[i]) == TRUE)		// No, process arg 
	      continue;
	    else
	      {
		fprintf(stderr,"%s: cannot continue\n",Command);
		exitProg(-1);
	      }
	  }
	else		// Process command line switch option 
	  {
	    int i1 = i;
	    
	    for (j = 1; pargv[i1][j] != 0; j++)
	      {
		switch(pargv[i1][1]) 
		  {
		    /*
		     * HANDLE SWITCHES WE RECOGNIZE
		     */
		  case '0':	// -0: standard input is a tty 
		    isatty0 = 1;
		    break;
		  case '1':	// -1: standard output is a tty 
		    isatty1 = 1;
		    break;
		  case 'v':	// -v: debug / verbose mode 
		    vflag++;
		    continue;
		  case 'i':	// -i: take input from named file
		    if (pargc-1 == i) {
		      fprintf(stderr,"%s: -i must be followed by the name of the file to read commands from\n",Command);
		      usage();
		      exitProg(-1);
		    }
		    iflag++;
		    CommandFile = strdup(pargv[++i]);
		    continue;
                  case 'r':     // Display command "results"
                    ptLastResult.OutputResults();
                    continue;
                  case 'g':     // Create "golden" files containing results
                    ptLastResult.SetFileOutput();
                    continue;
		  default:
		    fprintf(stderr, "%s: Unknown option -%c\n", Command, pargv[i1][j]);
		    usage();
		    exitProg(-1);
		  }	// switch
	      }	// for j
	  } // else
      }	// of for
}

/*************************************************************************
 *
 * Function:    initialization - perform program startup initializations
 *
 *		Set global variables, arm signals, etc.
 *
 * Input:       int pargc - Count of arguments passed from command line
 *              char **pargv - Pointer to (argc) individual argument strings
 *
 * Output:      NONE
 *
 * Called by:   main
 *
 ************************************************************************/

static void initialization(int pargc, char **pargv)
{
  /* Set Command to be name program invoked with
   */
  
  Command = strrchr(pargv[0], '/') ? strrchr(pargv[0], '/') + 1 : pargv[0];
  
  /* Set up isattyx variables
   */
  
  isatty0 = isatty(0);	/* Is stdin a tty? */
  isatty1 = isatty(1);	/* Is stdout a tty? */
  
  /* Arm all signals to catchsig
   */
  
  (void) signal(SIGINT, &catchsig);
  (void) signal(SIGQUIT, &catchsig);
  (void) signal(SIGTERM, &catchsig);

  // Setup the polling file descriptors
  initPollFileDesc();
  addPollFileDesc(fileno(stdin), POLLIN);

  /* Output start-up herald
   */
  
  fprintf(stderr,"data port test utility\n");
  (void) fflush(stdout);
  (void) fflush(stderr);
}

/*************************************************************************
 *
 * Function:    termination - perform program termination processing
 *
 *		disarm all signals, flush buffered file output, 
 *		terminate any processes opened thru pipes, etc.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Called by:   exitProg
 *
 ************************************************************************/

static void termination()
{
  /* disarm all signals
   */
  
  (void) signal(SIGINT, SIG_IGN);
  (void) signal(SIGQUIT, SIG_IGN);
  (void) signal(SIGTERM, SIG_IGN);
  
  /* flush buffered file output
   */
  
  (void) fflush(stderr);
  (void) fflush(stdout);
}       

/*************************************************************************
 *
 * Function:    exitProg - terminate program execution
 *
 *		perform program termination function and exit program.
 *
 * Input:       int status - return status for 'exit' system call
 *
 * Output:      NONE - program exits
 *
 ************************************************************************/

static void exitProg(int exvalue)
{
  termination();		// Perform program termination functions 
  exit(exvalue);		// Exit to calling process 
  /*NOTREACHED*/
}       

/*************************************************************************
 *
 * Function:    catchsig - catch signals armed via "signal"
 *
 *		If "JmpOnInterrupt" is true (set via AllowInterrupts(TRUE))
 *		will abort context back to command scan mode via "longjmp"
 *		ELSE will set UserInterrupt flag to TRUE.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Called by:   UNIX O/S software signal facility
 *
 ************************************************************************/

static void catchsig(int)
{
  /* IF it is ok to interrupt out of the current context,
  ** rearm all signals, reset all flags, flush all pending
  ** output, and longjmp back to command mode
  */
  
  if (JmpOnInterrupt == TRUE)
    {
      (void) signal(SIGINT, SIG_IGN);
      (void) signal(SIGQUIT, SIG_IGN);
      (void) signal(SIGTERM, SIG_IGN);
      //      KillCommand();
      (void) fflush(stdout);
      (void) fflush(stderr);
      (void) write(1,"\n",1);
      JmpOnInterrupt = FALSE;
      UserInterrupt = FALSE;
      (void) signal(SIGINT, &catchsig);
      (void) signal(SIGQUIT, &catchsig);
      (void) signal(SIGTERM, &catchsig);
      longjmp(jmpbuf,1);
    }
  
  /* Set user interrupt flag so various routines can handle
  ** interrupts at safe points within their environment
  */
  
  UserInterrupt = TRUE;
  return;
}

/*************************************************************************
 *
 * Function:    TakeInterrupt - if user interrupt is pending, take it
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 ************************************************************************/

void TakeInterrupt()
{
  
  /* If interrupt is not pending, simply return
   */
  
  if (UserInterrupt == FALSE)
    return;
  
  /* Interrupt is pending - take it
   */
  
  JmpOnInterrupt = TRUE;
  catchsig(0);
  /*NOTREACHED*/
}

/*************************************************************************
 *
 * Function:    AllowInterrupts - allow or disallow interrupt out of
 *		program context.
 *
 * Input:       TRUE - Allow interrupt out of program context
 *		       IF an interrupt is pending, take it now
 *		FALSE - Do NOT allow interrupt out of program context
 *
 * Output:      NONE
 *
 ************************************************************************/

void AllowInterrupts(int pok)
{
  JmpOnInterrupt = pok;	/* Set global flag */
  if (pok != FALSE && UserInterrupt != FALSE)
    catchsig(0);
  return;
}

/*************************************************************************
 *
 * Function:    usage - issue program usage information to stdout
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Called by:   parsecommands
 *
 ************************************************************************/

static void usage()
{
  fprintf(stderr,
"Usage:\t%s\n"
"\t%s 'command [arguments]'\n"
"\t%s -i scriptfile\n"
"\n"
"-i	specify name of file to use in place of standard input\n"
"-v	verbose mode\n"
"-r	display results (used for test validation)\n"
"-g	create golden files for results (used for test validation)\n"
"\n"
"If a command is specified, it is executed with any arguments supplied. Make sure\n"
"you enclose the command and its arguments within single quotes!!\n"
"\n"
"If a command is NOT specified, interactive command mode is entered. You will\n"
"be prompted for commands from the standard input.\n"
"\n"
"The standard input may be redirected causing commands to be read from a file.\n"
"\n"
,Command,Command,Command);
  
}

/******************************************************************************
 *
 * Funciton:	getnum - get number from command line argument
 *
 * Input:
 * string - pointer to command line string to get number from
 * minval - minimum acceptable value
 * maxval - maximum acceptable value
 *
 * Output:
 * int - >= 0 - number parsed from input
 *       < 0  - error code signifying input was not parsable
 *****************************************************************************/

int getnum(char *string,int minval,int maxval)
{
  register char *s;
  register result;
  
  result = 0;
  
  if (*string == '+' || *string == '-') 
    {
      fprintf(stderr,"%s: numeric argument cannot be signed\n", Command);
      return(-4);
    }
  
  if (*string == '\0')
    {
      fprintf(stderr, "%s: option requires numeric argument\n", Command);
      return (-3);
    }
  
  for (s = string; *s != '\0'; s++) 
    {
      if (*s < '0' || *s > '9')
	{
	  fprintf(stderr, "%s: option invalid numeric argument\n", Command);
	  return (-1);
	}
      if (result > maxval)
	{
	  fprintf(stderr, "%s: option maximum value is %d\n", Command, maxval);
	  return(-5);
	}
      result *= 10;
      result += (*s - '0');
    }
  if (result > maxval)
    {
      fprintf(stderr, "%s: option maximum value is %d\n", Command, maxval);
      return(-5);
    }
  if (result < minval)
    {
      fprintf(stderr, "%s: option minimum value is %d\n", Command, minval);
      return (-2);
    }
  return (result);
}

/******************************************************************************
 *
 * Funciton:	getnum - get number from command line argument
 *
 * Input:
 * string - pointer to command line string to get number from
 * minval - minimum acceptable value
 * maxval - maximum acceptable value
 *
 * Output:
 * int - >= 0 - number parsed from input
 *       < 0  - error code signifying input was not parsable
 *****************************************************************************/

int extractNumber(char *string,int minval,int maxval,int printMessage)
{
  register char *s;
  register result;
  
  result = 0;
  
  if (*string == '+' || *string == '-') 
    {
      if (printMessage)
	printf("?numeric argument cannot be signed: %s\n", string);
      return(-4);
    }
  
  if (*string == '\0')
    {
      if (printMessage)
	printf("?numeric argument required\n");
      return (-3);
    }
  
  for (s = string; *s != '\0'; s++) 
    {
      if (*s < '0' || *s > '9')
	{
	  if (printMessage)
	    printf("?numeric argument must contain all digits: %s\n", string);
	  return (-1);
	}
      if ((maxval != -1) && (result > maxval))
	{
	  if (printMessage)
	    printf("?numeric argument above maximum value of %d: %s\n", maxval, string);
	  return(-5);
	}
      result *= 10;
      result += (*s - '0');
    }
  if ((maxval != -1) && (result > maxval))
    {
      if (printMessage)
	printf("?numeric argument above maximum value of %d: %s\n", maxval, string);
      return(-5);
    }
  if ((minval != -1) && (result < minval))
    {
      if (printMessage)
	printf("?numeric argument below minimum value of %d: %s\n", minval, string);
      return (-2);
    }
  assert(result >= 0);
  return (result);
}

/*************************************************************************
 *
 * Function:	parseArg - parse an non-switch argument from command line
 *
 * Input:	char * to argument to parse
 *
 * Output:	TRUE - argument parsed successfully, can continue
 *		FALSE - argument bogus, abort parse
 *
 ************************************************************************/

static int parseArg(char *parg)
{
  if (Cmdline == (char *)0)
    {
      Cmdline = (char*)malloc((unsigned)(strlen(parg)+2));
      *Cmdline = '\0';
    }
  else
    Cmdline = (char*)realloc(Cmdline,(unsigned)(strlen(Cmdline)+2+strlen(parg)));
  
  if (Cmdline == (char *)0)
    {
      fprintf(stderr,"%s: out of memory\n",Command);
      return(FALSE);
    }
  
  (void) sprintf(Cmdline,"%s %s",Cmdline,parg);
  return(TRUE);
}

/*************************************************************************
 *
 * Function:	prepareFunction - do preprocessing before performing 
 *		primary function
 *
 * Input:	NONE
 *
 * Output:	NONE
 *
 ************************************************************************/

static void prepareFunction()
{
  if (iflag) {
    assert(CommandFile);
    if ( (access(CommandFile,R_OK) != 0) || (freopen(CommandFile,"r",stdin)==NULL)) {
      fprintf(stderr,"%s: cannot open command script file ",Command);
      perror(CommandFile);
      exit(1);
    }
    isatty0 = 0;
  }
  return;
}

/*************************************************************************
 *
 * Function:	performFunction - do what we can given arguments parsed
 *
 * Input:	NONE
 *
 * Output:	NONE
 *
 ************************************************************************/

static void performFunction()
{
  if (Cmdline != (char *)0)	// Command line cmd given? 
    (void) donecmd(Cmdline);	// YES: do cmd line cmd 
  else
    doInteractiveCommands();	// NO: get commands interactively
  
  /* Program function performed - return to main()
   */
  
  return;
}

/*************************************************************************
 *
 * Function:	donecmd - Execute one command as given on UNIX command line
 *
 * Input:	char * to command to execute
 *
 * Output:	NONE
 *
 ************************************************************************/

static void donecmd(char *pcmd)
{
  if (setjmp(jmpbuf) == 0)
    oneCommand(pcmd);
  (void) Cquit(1,(char**)"quit");
}

/*************************************************************************
 *
 * Function:	doInteractiveCommands - go into interactive command mode
 *
 * Input:	NONE
 *
 * Output:	NONE
 *
 ************************************************************************/

static void doInteractiveCommands()
{
  /* While commands are entered, perform them
   */
  
  (void) setjmp(jmpbuf);
  int prettfd = tt_fd();
  while (cmdScanner() == TRUE)
    {
      AllowInterrupts(FALSE);
      int postttfd = tt_fd();
      if ((tt_int_error(postttfd) == TT_OK && (prettfd != postttfd))) {
	theTooltalkFileDesc = postttfd;
	assert(theTooltalkFileDesc != -1);
	addPollFileDesc(postttfd, POLLIN);
	prettfd = postttfd;
      }
    }
}

#define NUMPOLLFDS (3+2)		// just in case :-)

void initPollFileDesc()
{
  assert(!pollFileDesc);
  pollFileDesc = (struct pollfd *)malloc(sizeof(struct pollfd)*(NUMPOLLFDS));
  assert(pollFileDesc);
  numPollFileDescEntries = NUMPOLLFDS;

  for (int i = 0; i < numPollFileDescEntries; i++) {
    pollFileDesc[i].fd = -1;
    pollFileDesc[i].events = 0;
  }
}

void addPollFileDesc(int fd, short event)
{
  for (int i = 0; i < numPollFileDescEntries; i++) {
    if (pollFileDesc[i].fd != -1)
      continue;
    pollFileDesc[i].fd = fd;
    pollFileDesc[i].events = event;
    return;
  }
  printf("?Poll structure ran out of entries!!\n");
  assert(0);
}

void removePollFileDesc(int fd)
{
  for (int i = 0; i < numPollFileDescEntries; i++) {
    if (pollFileDesc[i].fd != fd)
      continue;
    pollFileDesc[i].fd = -1;
    pollFileDesc[i].events = 0;
    return;
  }
  printf("?Poll structure does not contain fd %d\n", fd);
  assert(0);
}

void HexDump(FILE *pfp, const char *pmsg, const unsigned char *pbufr, const int plen, const int plimit, const int pcontents)
{
  unsigned char   save[64];
  long int        x, y, z, word, cnt;
  FILE *pfp_r = pfp;

  if (!plen)
    return;
  
  if (pfp_r == (FILE*) NULL) {
    pfp_r = stdout;
  }

  if (pmsg != NULL) {
    if (pcontents)
      (void) fprintf(pfp_r, "--> %s (%d bytes):\n", pmsg, plen);
    else
      (void) fprintf(pfp_r, "--> %s (%d bytes at 0x%08lx):\n", pmsg, plen, pbufr);
  }
  fflush(pfp_r);
  memset((char *)save, 0, sizeof(save));
  save[0] = ~pbufr[0];
  z = 0;
  cnt = plen;
  for (x = 0; cnt > 0; x++, z += 16)
    {
      if (pcontents)
	(void) fprintf(pfp_r, "+%6.6ld ", z);
      else
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
}

void computeSum(SdmString data, unsigned& first, long& second)
{
#define	WDMSK 0177777L
#define	BUFSIZE 512

  unsigned int		lsavhi;
  unsigned int		lsavlo;

  struct part {
    short unsigned hi, lo;
  };

  union hilo { /* this only works right in case short is 1/2 of long */
    struct part hl;
    long	lg;
  } tempa, suma;

  suma.lg = 0;

  const unsigned char *datap = (const unsigned char*)(const char *)data;
  int len = data.Length();
  for (int i = 0; i < len; datap++, i++) {
    suma.lg += *datap & WDMSK;
  }

  tempa.lg = (suma.hl.lo & WDMSK) + (suma.hl.hi & WDMSK);
  lsavhi = (unsigned) tempa.hl.hi;
  lsavlo = (unsigned) tempa.hl.lo;

  first = (unsigned)(lsavhi + lsavlo);
  second = (len+BUFSIZE-1)/BUFSIZE;
}
