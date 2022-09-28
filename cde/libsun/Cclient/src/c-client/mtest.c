/*
 * @(#)mtest.c	1.18 97/06/06
 *
 * Program:	Mail library test program
 *
 * Author:	Mark Crispin
 *		Networks and Distributed Computing
 *		Computing & Communications
 *		University of Washington
 *		Administration Building, AG-44
 *		Seattle, WA  98195
 *		Internet: MRC@CAC.Washington.EDU
 *
 * Date:	8 July 1988
 *
 * Sponsorship:	The original version of this work was developed in the
 *		Symbolic Systems Resources Group of the Knowledge Systems
 *		Laboratory at Stanford University in 1987-88, and was funded
 *		by the Biomedical Research Technology Program of the National
 *		Institutes of Health under grant number RR-00785.
 *
 * Original version Copyright 1988 by The Leland Stanford Junior University
 * Copyright 1995 by the University of Washington
 *
 *  Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notices appear in all copies and that both the
 * above copyright notices and this permission notice appear in supporting
 * documentation, and that the name of the University of Washington or The
 * Leland Stanford Junior University not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written prior
 * permission.  This software is made available "as is", and
 * THE UNIVERSITY OF WASHINGTON AND THE LELAND STANFORD JUNIOR UNIVERSITY
 * DISCLAIM ALL WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD TO THIS SOFTWARE,
 * INCLUDING WITHOUT LIMITATION ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE UNIVERSITY OF
 * WASHINGTON OR THE LELAND STANFORD JUNIOR UNIVERSITY BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, TORT (INCLUDING NEGLIGENCE) OR STRICT LIABILITY, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
#include "UnixDefs.h"
#include <limits.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include "mail.h"
#include "os_sv5.h"
#include "rfc822.h"
#include "smtp.h"
#include "misc.h"

#include <pwd.h>
#include <unistd.h>
#include "imap4.h"

FILE	*	input = stdin;
FILE	*	output = stdout;
boolean_t	usingAnswerFile = _B_FALSE;
char		remoteMb[PATH_MAX+1];

int		debug;			/* Debug the protocol ? */

gid_t		realGid = -1;
gid_t		effectiveGid = -1;

char 	*	curhst = NULL;		/* currently connected host */
char 	*	curusr = NULL;		/* current login user */
char	*	curpass = NULL;
char 		personalname[MAILTMPLEN];	/* user's personal name */


static const char *hostlist[] = {	/* SMTP server host list */
  "mailhost",
  "localhost",
  NULL
};

static const char *newslist[] = {	/* Netnews server host list */
  "news",
  NULL
};

static void	mm(MAILSTREAM * stream);
static void	sinfo(MAILSTREAM * stream, long msgno, long flags);
static void	headers(MAILSTREAM * stream,  char * arg,  long fuidflags);
static void	envelope(MAILSTREAM * stream, long msgno, long flags);
static void	clearSefFlag(MAILSTREAM * stream,  char * arg,  char * cmdx,  
			     char CM,  long opflags); 
static void	display_body(BODY * body, char * pfx, long i);
static void	status(MAILSTREAM * stream);
static void	prompt(char * msg, char * txt, int theSize);
static void	smtptest(MAILSTREAM * ms);
static void	do_search(MAILSTREAM * stream,  char * criteria,  long flags);
static void	rename_test(MAILSTREAM * stream, char * arg);

static int	set_ttlock(char * mailbox, void * userdata);
static int    	clear_ttlock(char * mailbox, void * userdata);
static int	set_group(void);
static int	clear_group(void);
static void	mm_fputs(const char *msg, FILE *);
static char *	mm_fgets(char * txt, int theSize, FILE *);

static void
usage()
{
  mm_fputs(
   "mtest [-c cannedAnswerFile] [-e createdEmptyFolder] [-f folder}\n\n"
   "\n! <shell-command>"
   "\n0 Search <search criteria>"
   "\n1 set lookahead <n>"
   "\n2 Checksum"
   "\n3 Echo <imapd>"
   "\n4 Send sunversion"
   "\n5 Set 1flag <message-number> <flag>"
   "\n6 Clear 1flag <message-number> <flag>"
   "\n7 Toggle UID CMD"
   "\n8 Ping mailbox (check for changes)"
   "\nA Add CC"
   "\na Append message (will be prompted for destination mailbox"
   "\nB Body <body-part-number>"
   "\nC Check"
   "\nc Create Folder"
   "\nD Delete <message-number>"
   "\ndi disconnect"
   "\nde delete <folder>"
   "\nE Expunge"
   "\nF Find"
   "\nfa Fetch Fast <msgno>"
   "\nfl Fetch Flags <msgno>"
   "\nfe Fetch Subject <msgno>"
   "\nG mail_gcc() ???"
   "\nH Headers <seq>  using short info"
   "\nI Short Info"
   "\nJ Just Copy <seq> to mailbox"
   "\nL Literal command [message-number]"
   "\nM Move Message <???>"
   "\nN New {<hostname>:}<mailbox> i.e. hostname:/var/mail/user"
   "\nn Just like 'N', except it does not keep re-asking for MB name"
   " or /var/mail/user"
   "\no Open a stream in OP_HALFOPEN mode"
   "\nP toggle Peeking for Type command"
   "\nR rename folder <oldname> <newname>"
   "\nS SMTP test"
   "\nsu Subscribe folder <folder>"
   "\nse Search based on criteria <criteria>"
   "\nsy Synchronize <sequence>"
   "\nsc Scan a string in mailfolders"
   "\nT Print headers <message-number>"
   "\nt Print 'X-Unix-From:' header <message-number>"
   "\nU Undelete <message->number>"
   "\nV toggle enVelope fetch with shortinfo"
   "\nW Mail Status"
   "\nX Exit (expunge + close)"
   "\nZ Z <message-number> <section> Z body"
   "\n= Open with lockf sessions ON/OFF"
   "\n+ Debug ON"
   "\n- Debug OFF",
   output);
  
  return;
}

/*
 * Make an IMAP host:/path/to/mb into an IMAP {host}/path/to/mb
 */
static void
makeMailBoxName(const char * name, char * buf)
{
  char	*	lName = strdup(name);
  char	*	colon;

  if ((colon = strchr(lName, ':')) != NULL) {
    *colon = '\0';
    sprintf(buf, "{%s}%s", lName, colon + 1);
  } else {
    strcpy(buf, lName);
  }
  free(lName);
}


/* Main program - initialization */

int
main(int argc,  char ** argv)
{
  MAILSTREAM	*	stream = NULL;
  void 		*	sdb = NULL;
  char 		*	s;
  char			tmp[MAILTMPLEN];
  char 		*	suffix;
  char			argCh;
  char		*	inputFile = NULL;
  char		*	outputFile = NULL;
  char		*	defaultFolder = NULL;
  long			flags;
  unsigned short	stream_flag;
  ENV_GLOBALS	*	eg;
  struct passwd *	pwd;
  boolean_t		gotError = _B_FALSE;

#include "linkage.c"

  while ((argCh = getopt(argc, argv, "c:e:f:")) != EOF) {

    switch (argCh) {

    case 'c':
      inputFile = optarg;
      outputFile = malloc(strlen(inputFile) + 3 + 10);
      sprintf(outputFile, "%s.out", inputFile);

      input = fopen(inputFile, "r");
      if (input == NULL) {
	fprintf(stderr, "ERROR: can not open '%s' for reading.\n", inputFile);
	gotError = _B_TRUE;
      }
      output = fopen(outputFile, "w");
      if (output == NULL) {
	fprintf(stderr, "ERROR: can not open '%s' for writing.\n", outputFile);
	gotError = _B_TRUE;
      }
      if (!gotError) {
	usingAnswerFile = _B_TRUE;
      }
      break;

    case 'e':
      defaultFolder = optarg;
      (void)unlink(defaultFolder);
      if (creat(defaultFolder, 0666) < 0) {
	fprintf(stderr, "ERROR: can not create default empty folder '%s'",
		defaultFolder);
	gotError = _B_TRUE;
      }
      break;

    case 'f':
      defaultFolder = optarg;
      break;

    case '?':
      gotError = _B_TRUE;
      break;
    }
  }
  if (gotError) {
    exit(1);
  }

  realGid = getgid();
  effectiveGid = getegid();

  stream = mail_stream_create(NIL, NULL, NULL, set_group, clear_group);
  eg = (ENV_GLOBALS *)stream->env_globals;
  curusr = strdup(eg->myUserName);
  pwd = getpwnam(curusr);

  /* current user is this name */
  if (pwd != NULL) {
    strcpy(tmp, pwd->pw_gecos);

    /* This looks like a moronic kludge */
    if (suffix = strchr(tmp, ', ')) {
      suffix[0] = '\0';
    }
    strcpy(personalname, tmp);/* make a permanent copy of it */
  } else {
    personalname[0] = '\0';
  }

  curhst = strdup(eg->myLocalHost);

  /* they're on the stream by now! */
  mm_fputs("MTest -- C client test program", output);
  if (!*personalname) {
    prompt("Personal name: ", personalname, sizeof(personalname));
  }

  /* user wants protocol telemetry? */
  prompt("Debug protocol (y/n)?", tmp, sizeof(tmp));
  ucase(tmp);
  debug = (tmp[0] == 'Y') ? OP_DEBUG : 0;

  flags = OP_KEEP_MIME | OP_SECONDARY_MBOX;

  do {
    if (defaultFolder == NULL) {
      prompt("Mailbox ('?' for help): ", tmp, sizeof(tmp));
    } else {
      strcpy(tmp, defaultFolder);
    }
    if (strcmp(tmp, "?") == 0) {
      mm_fputs("Enter INBOX,  mailbox name,  or IMAP mailbox as {host}mailbox",
	       output);
      mm_fputs("Known local mailboxes:", output);

      mail_list(stream,  NIL,  "%");
      if (s = sm_read(&sdb,  stream)) {
	mm_fputs("Local subscribed mailboxes:", output);
	do(mm_lsub(NIL, NIL, s, NIL));
	while (s = sm_read(&sdb,  stream));
      }
      mm_fputs("or just hit return to quit", output);
    } else if (tmp[0]) {
      makeMailBoxName(tmp, remoteMb);
      stream = mail_open(stream, remoteMb, debug+flags);
    }

    /*
     * mail_open and mail_close both can destroy streams.
     * So if it's destroyed,  we'll need to create it again.
     */
    if (!stream) stream = mail_stream_create(NIL, 
					     set_ttlock, clear_ttlock, 
					     set_group, clear_group);
  } while (!( stream->stream_status & S_OPENED ) && tmp[0] && !usingAnswerFile);

	/* Otherwise,  we've succesfully got a stream */
  mm(stream);		/* run user interface if opened */
  return(0);
}

/* To get the Flags
 * to be passed to various
 * operations
 */

long
getflags()
{
  char *ft_flags = getenv ("FETCH_FLAGS");
  if ( ft_flags && strcmp(ft_flags, "FT_UID" ) == 0 )
	return FT_UID;
  else
	return 0;
}

/*
 * MM command loop
 * Accepts: MAIL stream
 */
void
mm(MAILSTREAM * stream)
{
  void 		*	sdb = NULL;
  char 			cmd[PATH_MAX+1];
  char			cmdx[PATH_MAX+1];
  char 		*	s;
  char      *   retval;
  char		*	arg;
  char 		*	lasts;
  long 			i;
  unsigned long		len;
  long 			last = 0;
  int 			interval;
  BODY 		*	body;
  char 		*	partie;
  char 		*	sec;
  long			ovflags = debug + OP_KEEP_MIME;
  long			cuidflags = 0;	/* search flags */
  long			fuidflags = getflags();	/* fetch  flags */
  long			suidflags = 0;	/* store flags */
  long			muidflags = 0;	/* move uid flags */
  long			fsinfoenv = 0;	/* sinfo+envelope flag */
  char			CM;
  MAILSTREAM	*	stream1;	/* Used in restore cache check */
  int 			errno;
  boolean_t		add_cc = _B_FALSE;/* for an additional line */
  boolean_t		timeToExit = _B_FALSE;
  unsigned short	stream_flag;
  char subject[MAILTMPLEN];
  status(stream);		/* first report message status */

  while (stream != NULL & !timeToExit) {  /* test this bit */

    /* If the mailbox went away, try to re-open it */
    if (stream->dead) {
      cmd[0] = 'N';		/* Load new mailbox */
      cmd[1] = '\0';
      arg = stream->mailbox;	/* With old name */

    } else {
      prompt("MTest>", cmd, sizeof(cmd));	/* prompt user,  get command */

      /* get argument */
      if (arg = strchr(cmd, ' ')) {
	*arg++ = '\0';
      }
    }
    switch (CM = *cmd) {/* dispatch based on command */
	  
    case 'A':
      add_cc ^= _B_TRUE;
      if (add_cc) {
		mm_fputs("Cc: used in Type command", output);
      } else {
		mm_fputs("Cc: line disabled", output);
      }
      break;
	
    case 'a':
      /* Append messages */
      if (!arg) {
        mm_fputs("?Missing Sequence no.", output);
	break;

      } else {

	MAILSTREAM	*	appendStream = NULL;
	STRING			msg;
	STRINGLIST	*	lines = mail_newstringlist();
	STRINGLIST	*	cur = lines;
	unsigned long 		len;
	char		*	headers;
	char		*	text;
	int			msgNum = atoi(arg);

        prompt("Destination Mailbox: ", cmdx, sizeof(cmdx));

	/* X-Unix-From: */
	cur->size = strlen(cur->text = strdup("X-Unix-From"));

	/* date  */
	cur = cur->next = mail_newstringlist();
	cur->size = strlen(cur->text = strdup("Date"));

	/* From:  */
	cur = cur->next = mail_newstringlist(); 
	cur->size = strlen(cur->text = strdup("From"));

	/* To:  */
	cur = cur->next = mail_newstringlist();
	cur->size = strlen(cur->text = strdup("To"));

	/* Subject:  */
	cur = cur->next = mail_newstringlist();
	cur->size = strlen(cur->text = strdup("Subject"));
	
	/* Cc:  */
	if (add_cc) {
	  cur = cur->next = mail_newstringlist();
	  cur->size = strlen(cur->text = strdup("Cc"));
	}

	/* Content-Type:  */
	cur = cur->next = mail_newstringlist();
	cur->size = strlen(cur->text = strdup("Content-Type"));

	/* fetch it ... */
	if ((retval = mail_fetchheader_full(stream,
					    msgNum,
					    lines,
					    &len,
					    fuidflags)) != NULL) {
	  headers = strdup(retval);
	  if ((text = mail_fetchtext_full(stream,
					    msgNum,
					    &len,
					    fuidflags)) != NULL) {

	    char	* cmsg = malloc(strlen(text) + strlen(headers) + 1);

	    sprintf(cmsg, "%s%s", headers, text);

	    INIT(&msg, mail_string, cmsg, strlen(cmsg));

	    text = strdup(retval);
	
	    makeMailBoxName(cmdx, remoteMb);
	    appendStream = mail_stream_create(NIL,  set_ttlock, clear_ttlock, 
					      set_group, clear_group);
	    if (!mail_append_full(appendStream,
				  remoteMb,
				  0,
				  "Mon, 1 Jan 1999 23:59:59 -0700 (PDT)",
		"dougr@pitts.eng.sun.com Mon Jan  1 23:59:59 1999",
				  &msg)) {
	      mm_fputs("?ERROR: mail_append_full() failed!", output);
	    }
	  }
	}
      }

      break;
      
    case 'B':			/* Body command */
      if (arg) {
	last = atoi(arg);
      } else if (!last) {
	mm_fputs("?Missing message number", output);
	break;
      }

      body = NULL;
      mail_fetchstructure_full(stream, last, &body, fuidflags);
      if (body) {
	display_body(body, NIL, (long) 0);
      } else {
	mm_fputs("%No body information available", output);
      }
      break;

    case 'C':			/* Check command */
      mail_check(stream);
      status(stream);
      break;

    case 'c':           /* Create a folder */
      {
	/* Create Mailbox */
	MAILSTREAM	*	cstream = mail_stream_create(NULL,
							     NULL,
							     NULL,
							     set_group,
							     clear_group);

	if (!arg) {
	  mm_fputs("?Missing mailbox name", output);
	  break;
	}
	makeMailBoxName(arg, remoteMb);
	if (mail_create(cstream, remoteMb)) {
	  mm_fputs("Mailbox created.", output);
	}
      }
      break;

    case 'D':			/* Delete command */
      if (arg) {
	last = atoi(arg);
      } else {
	if (last == 0) {
	  mm_fputs("?Missing message number", output);
	  break;
	}
	arg = cmd;
	sprintf(arg, "1:%ld", last);
      }
      mail_setflag_full(stream, arg, "\\DELETED", suidflags);
      break;

    case 'd':  
      if ( cmd[1] == 'i' ) { /* Disconnect mailbox */
	mail_disconnect(stream);
      } else { /* Delete a folder */ 
	if (!arg) {
	  mm_fputs("?Missing mailbox name", output);
	  break;
	}
	makeMailBoxName(arg, remoteMb);
	if (mail_delete(stream, remoteMb)) {
	  mm_fputs("Mailbox deleted.", output);
	}
      }
      break;

    case 'E':			/* Expunge command */
      mail_expunge(stream);
      last = 0;
      break;

    case 'F':			/* Find command */
      if (!arg) {
	arg = "%";
	if (s = sm_read(&sdb, stream)) {
	  mm_fputs("Local network subscribed mailboxes:", output);
	  do {
	    if (*s == '{') {
	      mm_lsub(NIL, NIL, s, NIL);
	    }
	  } while (s = sm_read(&sdb, stream));
	}
      }
      makeMailBoxName(arg, remoteMb);

      mm_fputs("Subscribed mailboxes:", output);
      mail_lsub(stream, NIL, remoteMb);

      mm_fputs("Known mailboxes:", output);
      mail_list(stream, NIL, remoteMb);
      break;

    case 'f':
      if (arg) {
	if (!(last = atol(arg))) {
	  mm_fputs("?illegal message number", output);
	  break;
	}
      } else {
	mm_fputs("?Missing message number", output);
      }

      if (cmd[1] == 'a' ) { /* Fetch Fast */
	mail_fetchfast_full(stream, arg, fuidflags);

      } else if ( cmd[1] == 'l' ) { /* Fetch flags */
	mail_fetchflags_full(stream, arg, fuidflags);

      } else {
	mail_fetchsubject(subject, stream, last,MAILTMPLEN);
	if (subject) {
	  mm_fputs(subject, output);
	}
      }
      break;
	  
    case 'G':
      mail_gc(stream, GC_ENV|GC_TEXTS|GC_ELT);
      break;

    case 'H':
      if (!arg) {
		mm_fputs("An arg is missing", output);
      } else {
		/* display headers using sinfo */
      	headers(stream,  arg,  fuidflags);
      }
      break;

    case 'I':			/* Short info  */
      if (arg) {
	if (!(last = atol(arg))) {
	  mm_fputs("?illegal message number", output);
	  break;
	}
	sinfo(stream, last, fuidflags|fsinfoenv);
      } else {
	mm_fputs("?Missing message number", output);
      }
      break;

    case 'L':			/* Literal command */
      if (arg) {
	if (!(last = atol(arg))) {
	  mm_fputs("?illegal message number", output);
	  break;
	}
	envelope(stream, last, fuidflags);
      } else {
	mm_fputs("?Missing message number", output);
      }
      break;

    case 'M':
      /* Move messages */
      if (!arg) {
        mm_fputs("?Missing Sequence no.", output);
		break;
      } else {
      	muidflags += CP_MOVE;
      }
      /*FALLTHRU*/

    case 'J':
      /* Copy OR Move messages */
      if (!arg) {
        mm_fputs("?Missing Sequence no.", output);
	break;
      } else {
        prompt("Destination: ", cmdx, sizeof(cmdx));
      }
      makeMailBoxName(cmdx, remoteMb);
      if (!mail_copy_full(stream, arg, remoteMb, muidflags)) {
	mm_fputs("?ERROR: mail_copy_full() failed!", output);
      }
      muidflags &=~ CP_MOVE;
      break;

    case 'N':
    case 'n':
      {			/* New mailbox command */
	boolean_t	keepAsking = _B_TRUE;
	boolean_t	firstTime = _B_TRUE;

	if (CM == 'n') {
	  keepAsking = _B_FALSE;
	}

	if (!arg) {
	  mm_fputs("?Missing mailbox", output);
	  break;
	}
	mail_close(stream);
	/* Check for an OPENED stream */
	stream = mail_stream_create(NIL,  set_ttlock, clear_ttlock, 
				    set_group, clear_group);
	strcpy(cmdx, arg);

	if (usingAnswerFile && !stream->stream_status & S_OPENED) {
	  fprintf(stderr, "ERROR in answer script!\n");
	  exit(1);
	}

	while (!(stream->stream_status & S_OPENED )) {
	  if (!firstTime) {
	    prompt("Mailbox: ", cmdx, sizeof(cmdx));
	  }
	  firstTime = _B_FALSE;
	  makeMailBoxName(cmdx, remoteMb);
	  stream = mail_open(stream, remoteMb, ovflags);
	  if (!keepAsking) {
	    break;
	  }
	}
	last = 0;
	status(stream);
      }
      break;

    case 'o':
      {
	MAILSTREAM	*	stream;

	if (!arg) {
	  mm_fputs("?Missing mailbox", output);
	  break;
	}

	stream = mail_stream_create(NIL,  set_ttlock, clear_ttlock, 
				    set_group, clear_group);

	makeMailBoxName(arg, remoteMb);
	stream = mail_open(stream, remoteMb, OP_HALFOPEN);
	if (!stream->stream_status & S_OPENED) {
	  mm_fputs("ERROR: Opening of stream in OP_HALFOPEN mode failed!",
		   output);
	}
      }
      break;

    case 'P':
      fuidflags ^= FT_PEEK;
      if (fuidflags & FT_PEEK) {
	mm_fputs("Peeking ON", output);
      } else {
	mm_fputs("Peeking OFF", output);
      }
      break;

    case 'Q':			/* Quit command */
      if (stream->stream_status & S_OPENED ) {
	mail_close(stream);
      } else {
	mail_stream_flush(stream);
      }
      stream = NULL;
      timeToExit = _B_TRUE;
      break;

    case 'R':			/* Rename mailbox <oldname> <newname> */
      if (arg == NULL) {
	mm_fputs("?Usage error \"R <oldfolder> <newfolder>\"", output);
      } else {
	makeMailBoxName(arg, remoteMb);
	rename_test(stream, remoteMb);
      }
      break;

    case 'S':			/* Send command */
      smtptest(stream);
      break;

    case 's':           /* Scan a folder */
	  
      if (cmd[1] == 'u' ) { /* Subscribe a folder */
	if (!arg) {
	  mm_fputs("?Usage Error \"su <folder name>", output);
	  break;
	}
	makeMailBoxName(arg, remoteMb);
	if (mail_subscribe(stream, remoteMb)) {
	  mm_fputs("Mailbox Subscribed.\n", output);
	}
	
      } else if (cmd[1] == 'e' ) { /* Search a folder */
	if (!arg) {
	  mm_fputs("?Usage Error \"se <Search Criteria>", output);
	  break;
	}
	mail_simple_search(stream, arg, NIL, fuidflags | SE_NOPREFETCH);

      } else if (cmd[1] == 'y' ) { /* Imap Synchronization */
	if (!arg) {
	  mm_fputs("?Usage Error \"sy <UID Sequence>", output);
	  break;
	}
	imap_sync_msgno(stream, arg);

      } else {
	if (!arg) {
	  mm_fputs("?Usage Error \"s <Mailboxes Path> <scan string>", 
		   output);
	  break;
	}
	sec = strtok_r(arg, " ",  &lasts);
	mail_scan(stream, NIL, sec, lasts);
      }
      break;  

    case '\0':			/* null command (type next message) */
      if (suidflags) {
	break;
      }
      if ((last <= 0) || (last++ >= stream->nmsgs)) {
	mm_fputs("%No next message", output);
	break;
      }

    case 't':
      /*FALLTHRU*/
    case 'T':			/* Type command */
      if (!arg) {
	mm_fputs("?Missing message number", output);
	break;
      } else if (!last) {

	STRINGLIST 	*	lines = mail_newstringlist();
	STRINGLIST 	*	cur = lines;
	boolean_t	xUnixFromOnly = CM == 't' ? _B_TRUE : _B_FALSE;
	unsigned long 		len;
	int			msgNum = atoi(arg);

	/* X-Unix-From: */
	cur->size = strlen(cur->text = strdup("X-Unix-From"));

	if (!xUnixFromOnly) {

	  /* date  */
	  cur = cur->next = mail_newstringlist();
	  cur->size = strlen(cur->text = strdup("Date"));

	  /* From:  */
	  cur = cur->next = mail_newstringlist(); 
	  cur->size = strlen(cur->text = strdup("From"));

	  /* To:  */
	  cur = cur->next = mail_newstringlist();
	  cur->size = strlen(cur->text = strdup("To"));

	  /* Subject:  */
	  cur = cur->next = mail_newstringlist();
	  cur->size = strlen(cur->text = strdup("Subject"));
	
	  /* Cc:  */
	  if (add_cc) {
	    cur = cur->next = mail_newstringlist();
	    cur->size = strlen(cur->text = strdup("Cc"));
	  }

	  /* Content-Type:  */
	  cur = cur->next = mail_newstringlist();
	  cur->size = strlen(cur->text = strdup("Content-Type"));
	}

	/* fetch it ... */
	if ((retval = mail_fetchheader_full(stream,
					    msgNum,
					    lines,
					    &len,
					    fuidflags)) != NULL) {
	  fprintf(output, "%s", retval);
	  if (usingAnswerFile) {
	    fflush(output);
	  }
	}

	if (retval = mail_fetchtext_full(stream, msgNum,  &len,  fuidflags)) {
	  mm_fputs(retval, output);
	}
	mail_free_stringlist(&lines);
      }
      break;

    case 'U':			/* Undelete command */
      if (arg) {
	last = atoi(arg);
      }  else {
	if (!last) {
	  mm_fputs("?Missing message number", output);
	  break;
	}
	arg = cmd;
	sprintf(arg, "%ld", last);
      }
      mail_clearflag_full(stream, arg, "\\DELETED", suidflags);
      break;

    case 'u':           /* Unsubscribe a folder */
      if (!arg) {
	mm_fputs("?Usage Error for Unsubscribe: \"u <folder name>", output);
	break;
      }
		
      makeMailBoxName(arg, remoteMb);
      if (mail_unsubscribe(stream, remoteMb)) 
	mm_fputs("Mailbox Unsubscribed.", output);
      break;  
	
    case 'V':
      fsinfoenv ^= FT_SHORTENV;
      if (fsinfoenv) {
	mm_fputs("sinfo/ENVELOPE fetch ON", output);
      } else {
	mm_fputs("sinfo/ENVELOPE fetch off", output);
      }
      break;

    case 'W':
      mail_status(stream,  stream->mailbox,  SA_MESSAGES | SA_CHECKSUM);
      break;

    case 'X':			/* Xit command */
      mail_expunge(stream);
      mail_close(stream);
      stream = NIL;
      break;

    case 'Z':
      /* <message-number> <sequence> for body. EG. 1 2.1 */
      if (arg) {
	last = atoi(arg);
	strtok_r(arg, " ",  &lasts);
      } else {
	mm_fputs("?Missing message number", output);
	break;
      }
      sec = strtok_r(NIL, " ",  &lasts);
      if (sec == NIL) {
	mm_fputs("?Illegal section number", output);
	break;
      } else {
	partie = mail_fetchbody_full(stream, last, sec, &len, fuidflags);
      }
      if (partie && len > 0) {
	mm_fputs(partie, output);
      }
      break;

    case '+':
      mail_debug(stream);
      debug ^= OP_DEBUG;
      break;

    case '-':
      mail_nodebug(stream); 
      debug ^= OP_DEBUG;
      break;

    case '=':
      ovflags ^= OP_SESSIONLOCKF;
      if (ovflags &  OP_SESSIONLOCKF) {
	mm_fputs("  Lockf session ON", output);
      } else {
	mm_fputs("  Lockf session OFF", output);
      }
      break;

    case '!':	/* Shell command */
      if (!arg) {
	mm_fputs("?Missing shell command", output);
      } else {
	system(arg);
      }
      break;

    case '0':
      if (!arg) {
	mm_fputs("?Missing search criteria", output);
      }
      /* just for fun */
      mail_parameters(stream, SET_GETS, (void *) mail_search_gets);
	  
      do_search(stream,  arg,  cuidflags);
      break;

    case '1':
      if (arg) {
	interval = atoi(arg);
      } else {
	mm_fputs("?Missing interval", output);
	break;
      }
      mail_stream_setNIL(stream);
      mail_parameters(stream,  SET_LOOKAHEAD,  (void *)interval);
      mail_stream_unsetNIL(stream); 
      fprintf(output, "Look ahead = %d\n",  interval);
      if (usingAnswerFile) {
	fflush(output);
      }
      break;

    case '2':
	  stream->sunvset = 1;
      mail_status(stream,  stream->mailbox,  STATUSCHECKSUM);
      mail_checksum(stream);
      break;

    case '3':
      if (arg) {
	sprintf(cmdx,  "{%s}",  arg);
        if(mail_echo(cmdx)) {
	  mm_fputs("success (-:", output);
	} else {
	  mm_fputs("failed )-:", output);
	}
      } else {
	mm_fputs("?Missing server name", output);
      }
      break;

    case '4':
      mail_debug(stream);
      mail_sunversion(stream);
      if (debug == OP_DEBUG) {
	mail_nodebug(stream);
      }
      break;

    case '5':
    case '6':
      clearSefFlag(stream,  arg,  cmdx,  CM,  suidflags);
      break;

    case '7':
      cuidflags ^= SE_UID;
      fuidflags ^= FT_UID;
      suidflags ^= ST_UID;
      muidflags ^= CP_UID;

      if (suidflags) {
	mm_fputs("UID flags enabled", output);
      } else {
	mm_fputs("UID flag disabled", output);
      }
      break;

    case '8':
      mail_ping(stream);
      break;

    case '?':			/* ? command */
      usage();
      break;

    default:			/* bogus command */
      fprintf(output, "?Unrecognized command: %s\n", cmd);
      if (usingAnswerFile) {
	fflush(output);
      }
      break;
    }

    /*
     * Free any buffers if mailbox is local:
     * This is a worse case scenario.
     */
    if (stream && stream->mailbox && *stream->mailbox == '/') {
      mail_free_cached_buffers_seq(stream, NIL, NIL);
    }
  }
}

/*
 * Just fetch full ...
 */
static void 
envelope(MAILSTREAM * stream,  long messageNumber,  long flags)
{
  BODY 		*	b;
  ENVELOPE 	*	env = mail_fetchstructure_full(stream, 
						       messageNumber,
						       &b,
						       flags);
  if (env) {
    mm_fputs("[Ok]", output);
  } else {
    mm_fputs("[Failed]", output);
  }
  return;
}

/*
 * Display shortinfo header ...
 */
void
sinfo(MAILSTREAM *stream, long messageNumber, long flags)
{
  SHORTINFO 		info;
  MESSAGECACHE	*	cache;
  char 		*	t;
  char			tmp[MAILTMPLEN];
  int 			i;
  int			j;

  mail_fetchshort_full(stream,  messageNumber,  &info,  flags);

  if (info.date == NIL) {
    return;
  }

  /* We want the UID too */
  if (flags & FT_UID) {
    /* messageNumber is uid,  get the uid if it exists */
    i = mail_msgno(stream,  messageNumber);
    if (i == 0) {
      return;
    }
    messageNumber = i;
  }
  cache = mail_elt(stream, messageNumber);
  tmp[0] = cache->recent ? (cache->seen ? 'R': 'N') : ' ';
  tmp[1] = (cache->recent | cache->seen) ? ' ' : 'U';
  tmp[2] = cache->flagged ? 'F' : ' ';
  tmp[3] = cache->answered ? 'A' : ' ';
  tmp[4] = cache->deleted ? 'D' : ' ';
  tmp[5] = (strcasecmp(info.bodytype,  "TEXT") == 0) ? 'T' : 'Z';
  sprintf(tmp+6, "%4ld) ", cache->msgno);
  mail_date(tmp+12, cache);
  tmp[11] = ' ';
  tmp[21] = ' ';
  tmp[22] = '\0';
  strncpy(tmp+22,  info.from,  20);
  i = ((j = strlen(info.from)) < 20 ? j : 20);
  tmp[i+22] = NIL;
  strcat(tmp, " ");
  if (info.subject) {
    strncpy(t = tmp + strlen(tmp),  info.subject,  25);
    i = ((j = strlen(info.subject)) < 25 ? j : 25);
    t[i] = NIL;
  } else {
    strcat(t = tmp + strlen(tmp),  "(No Subject)");
  }
  sprintf(t += strlen(t), " (%s chars)",  info.size);
  sprintf(t += strlen(t), "[UID %ld]",  cache->uid);
  mm_fputs(tmp, output);
}

/* set/clear 1 flag */
static void 
clearSefFlag(MAILSTREAM * stream,  char * arg,  char * cmdx,  
	     char CM, long opflags) 
{
  long 		i;
  char	*	seq1;
  char 	*	lasts;

  if (arg) {
    char 	*	seq;
    char	*	flag;

    seq = strtok_r(arg,  " ",  &lasts);
    if (opflags) {
      /*
       * validate our UID sequence and generate an
       * equivalent messageNumber sequence for later use
       */
      if (!(seq1 = mail_uidseq_to_sequence(stream,  seq))) {
	fprintf(output, "?Illegal UID sequence %s",  seq);
	if (usingAnswerFile) {
	  fflush(output);
	}
	return;
      }
    } else  if (!mail_sequence(stream, seq)) {
      fprintf(output, "?Illegal sequence %s\n",  seq);
      if (usingAnswerFile) {
	fflush(output);
      }
      return;
    }
    flag = strtok_r(NIL,  "\n",  &lasts);
    if (!flag) {
      prompt("Flag = ",  cmdx, 10);
      flag = cmdx;
      if (!flag) {
	mm_fputs("?Flag required", output);
	return;
      }
    }

    /* set or clear flag */
    if (CM == '5') {
      mail_set1flag_full(stream,  seq,  flag,  opflags);
    } else {
      mail_clear1flag_full(stream,  seq,  flag,  opflags);
    }

    /* Make sure we use a messageNumber sequence */
    if (opflags) {
      seq = seq1;
    }
    mail_sequence(stream, seq);
    for (i = 1; i < stream->nmsgs; ++i) {
      if (mail_elt(stream, i)->sequence) {
	sinfo(stream, i, NIL);
      }
    }

    if (opflags) fs_give((void **)&seq1);
  } else {
     mm_fputs("? <seq> <flag> required", output);
  }
  return;
}

/*
 * Display headers using sinfo calls
 */
static void
headers(MAILSTREAM * stream,  char * seq,  long fuidflags)
{
  char	*	seq1;
  long 		i;

  if (fuidflags) {
    /* 
     * validate our UID sequence and generate an
     * equivalent messageNumber sequence for later use
     */
    if (!(seq1 = mail_uidseq_to_sequence(stream,  seq))) {
      fprintf(output, "?Illegal UID sequence %s",  seq);
      if (usingAnswerFile) {
	fflush(output);
      }
      return;
    }
    seq = seq1;
  }
  if (!mail_sequence(stream, seq)) {
    fprintf(output, "?Illegal sequence %s\n",  seq);
    if (usingAnswerFile) {
      fflush(output);
    }
    return;
  }
  for (i = 1; i <= stream->nmsgs; ++i) {
    if (mail_elt(stream, i)->sequence) {
      sinfo(stream, i, NIL);
    }
  }
  if (fuidflags) {
    fs_give((void **)&seq1);
  }
  return;
}

/*
 * search for the criteria:
 *  Here we call mail_simple_search() which will default
 *  the search to the local cache if possible.
 */
static nsearched;

static void
do_search(MAILSTREAM * stream,  char * criteria,  long flags)
{
  long 		i;

  nsearched = 1;
  if (flags & SE_UID) {
    mm_fputs("UIDs:", output);
  } else {
    mm_fputs("MessageNumbers:", output);
  }
  mail_simple_search(stream,  criteria,  NIL,  flags);
  mm_fputs("", output);

  /* free cached buffers */
  mail_free_cached_buffers_seq(stream, NIL, NIL);

  /* If not UID search,  then display the headers */
  if (!(flags & SE_UID)) {
    for (i = 1; i <= stream->nmsgs; ++i) {
      if (mail_elt(stream, i)->searched) {
	sinfo(stream,  i,  NIL);
      }
    }
  }
  return;
}

/*
 * MM display body
 * Accepts: BODY structure pointer
 *	    prefix string
 *	    index
 */
static void
display_body(BODY * body, char * pfx, long i)
{
  char		tmp[MAILTMPLEN];
  char	*	s = tmp;
  PARAMETER *	par;
  PART 	*	part;		/* multipart doesn't have a row to itself */

  if (body->type == TYPEMULTIPART) {
				/* if not first time,  extend prefix */
    if (pfx) {
      sprintf(tmp, "%s%ld.", pfx, ++i);
    } else {
      tmp[0] = '\0';
    }
    for (i = 0, part = body->contents.part; part; part = part->next) {
      display_body(&part->body, tmp, i++);
    }
  } else {			/* non-multipart,  output oneline descriptor */
    if (!pfx) {
      pfx = "";		/* dummy prefix if top level */
    }
    sprintf(s, " %s%ld %s", pfx, ++i, body_types[body->type]);

    if (body->subtype) {
      sprintf(s += strlen(s), "/%s", body->subtype);
    }

    if (body->description) {
      sprintf(s += strlen(s), " (%s)", body->description);
    }

    if (par = body->parameter) {
      do {
	sprintf(s += strlen(s), ";%s=%s", par->attribute, par->value);
      }	while (par = par->next);
    }
 
    if (body->id) {
      sprintf(s += strlen(s), ",  id = %s", body->id);
    }

    switch (body->type) {	/* bytes or lines depending upon body type */
    case TYPEMESSAGE:		/* encapsulated message */
    case TYPETEXT:		/* plain text */
      sprintf(s += strlen(s), " (%ld lines)", body->size.lines);
      break;
    default:
      sprintf(s += strlen(s), " (%ld bytes)", body->size.bytes);
      break;
    }

    mm_fputs(tmp, output);			/* output this line */

    /* encapsulated message? */
    if (body->type == TYPEMESSAGE && (body = body->contents.msg.body)) {
      if (body->type == TYPEMULTIPART) {
	display_body(body, pfx, i-1);
      } else {			/* build encapsulation prefix */
	sprintf(tmp, "%s%ld.", pfx, i);
	display_body(body, tmp, (long) 0);
      }
    }
  }
  return;
}

/*
 *  MM status report
 * Accepts: MAIL stream
 */
static void
status(MAILSTREAM *stream)
{
  long		i;
  char 		date[50];

  rfc822_date(date);
  /*mm_fputs(date, output);*/
  if (stream != NULL && !stream->dead) {
    if (stream->mailbox) {
      fprintf(output, " %s mailbox: %s,  %ld messages,  %ld recent\n", 
	      stream->dtb ? stream->dtb->name : "<NULL>",
	      stream->mailbox,
	      stream->nmsgs,
	      stream->recent);
      if (usingAnswerFile) {
	fflush(output);
      }
    } else {
      mm_fputs("%No mailbox is open on this stream", output);
    }
    if (stream->user_flags[0]) {
      fprintf(output, "Keywords: %s", stream->user_flags[0]);
      if (usingAnswerFile) {
	fflush(output);
      }
      for (i = 1; i < NUSERFLAGS && stream->user_flags[i]; ++i) {
	fprintf(output, ",  %s", stream->user_flags[i]);
      }
      mm_fputs("", output);
    }
  }
  return;
}

/* 
 * Prompt user for input
 * Accepts: pointer to prompt message
 *          pointer to input buffer
 */
static void
prompt(char * msg, char * txt, int theSize)
{
  fprintf(output, "%s", msg);
  mm_fgets(txt, theSize, input);

  return;
}

/* Interfaces to C-client */
void
mm_searched(MAILSTREAM * stream, unsigned long number)
{
  /* output hit list */
  fprintf(output, "%ld ",  number);
  if (usingAnswerFile) {
    fflush(output);
  }
  if((nsearched++ % 20) == 0) {
    mm_fputs("", output);
  }
  return;
}


void
mm_exists(MAILSTREAM * stream, unsigned long number)
{
}


void
mm_expunged(MAILSTREAM * stream, unsigned long number)
{
}


void
mm_flags(MAILSTREAM * stream, unsigned long number)
{
}


void
mm_notify(MAILSTREAM * stream, char * string, long errflg)
{
  fprintf(output, "MM_NOTIFY:");
  mm_log(string, errflg,  stream);
  return;
}


void
mm_list(MAILSTREAM * stream, char delimiter, char * mailbox, long attributes)
{
  fputc(' ', output);
  if (delimiter) {
    fputc(delimiter, output);
  } else {
    mm_fputs("NIL", output);
  }
  fputc(' ', output);
  mm_fputs(mailbox, output);

  if (attributes & LATT_NOINFERIORS) {
    mm_fputs(",  no inferiors", output);
  }
  if (attributes & LATT_NOSELECT) {
    mm_fputs(",  no select", output);
  }
  if (attributes & LATT_MARKED) {
    mm_fputs(",  marked", output);
  }
  if (attributes & LATT_UNMARKED) {
    mm_fputs(",  unmarked", output);
  }
  fputc('\n', output);

  return;
}


void
mm_lsub(MAILSTREAM * stream, char delimiter, char * mailbox, long attributes)
{
  fputc(' ', output);
  if (delimiter) {
    fputc(delimiter, output);
  } else {
    mm_fputs("NIL", output);
  }
  fputc(' ', output);
  mm_fputs(mailbox, output);

  if (attributes & LATT_NOINFERIORS) {
    mm_fputs(",  no inferiors", output);
  }

  if (attributes & LATT_NOSELECT) {
    mm_fputs(",  no select", output);
  }

  if (attributes & LATT_MARKED) {
    mm_fputs(",  marked", output);
  }

  if (attributes & LATT_UNMARKED) {
    mm_fputs(",  unmarked", output);
  }
  fputc('\n', output);

  return;
}


void
mm_log(char * string, long errflg,  MAILSTREAM * stream)
{
 switch ((short) errflg) {
  case NIL:
    fprintf(output, "[%s]\n", string);
    break;

  case PARSE:
  case WARN:
    fprintf(output, "%%%s\n", string);
    break;

  case ERROR:
    fprintf(output, "?%s\n", string);
    break;
  }
 fflush(output);
 return;
}

void
mm_dlog(MAILSTREAM * stream, char * string)
{
  fprintf(output, "MM_DLOG:");
  mm_fputs(string, output);
  return;
}

void
mm_login(MAILSTREAM 	* stream,
	 NETMBX 	* mbx,
	 char 		* user,
	 char 		* pwd,
	 long		  trial)
{
  static boolean_t	beenHereOnce = _B_FALSE;
  char 			tmp[MAILTMPLEN];
  char			ch = ' ';

  if (!beenHereOnce) {
    if (curusr != NULL) {
      free(curusr);
    }
    if (curpass != NULL) {
      free(curpass);
    }
    curusr = NULL;
    curpass = NULL;
    beenHereOnce = _B_TRUE;
  }

  /* Toss the user name and password if the target host changed */
  if (curhst != NULL) {
    if (strcmp(curhst, mbx->host) != 0) {
      free(curhst);
      if (curusr != NULL) {
	free(curusr);
      }
      if (curpass != NULL) {
	free(curpass);
      }
      curusr = NULL;
      curpass = NULL;
      curhst = strdup(mbx->host);
    }
  } else {
    curusr = NULL;
    curpass = NULL;
    curhst = strdup(mbx->host);
  }

  if (curusr == NULL) {
    sprintf(tmp, "%s:username ", mbx->host);
    prompt(tmp, user, 20);
    /* Change the default to this user name */
    curusr = strdup(user);
  } else {
    strcpy(user, curusr);
  }

  if (curpass == NULL) {
    /* Yes it echo's the password, it is test code */
    sprintf(tmp, "%s:password ", mbx->host);
    prompt(tmp, pwd, 20);
    curpass = strdup(pwd);
  } else {
    strcpy(pwd, curpass);
  }

  return;
}


void
mm_critical(MAILSTREAM * stream)
{
}


void
mm_nocritical(MAILSTREAM * stream)
{
}


long
mm_diskerror(MAILSTREAM * stream, long errcode, long serious)
{
  kill(getpid(), SIGSTOP);
  return NIL;
}


void
mm_fatal(char * string,  MAILSTREAM * stream)
{
  fprintf(output, "?%s\n", string);
  if (usingAnswerFile) {
    fflush(output);
  }
  return;
}

/* SMTP tester */
static void	
smtptest(MAILSTREAM * ms)
{		/* passing a mailstream here */
  int 			i;
  SENDSTREAM	*	stream = NULL;
  char 			line[MAILTMPLEN];
  HEADERLIST 	*	header_list = mail_newheaderlist();
  char 			tmp[MAILTMPLEN];
  STRINGLIST 	*	data_ptr;
  STRINGLIST	*	field_ptr;
  unsigned char *	text = (unsigned char *) fs_get(8*MAILTMPLEN);
  ENVELOPE 	*	msg = mail_newenvelope();
  BODY 		*	body = mail_newbody();

  msg->from = mail_newaddr();
  msg->from->personal = strdup(personalname);
  msg->from->mailbox = strdup(curusr);
  msg->from->host = strdup(curhst);
  msg->return_path = mail_newaddr();
  msg->return_path->mailbox = strdup(curusr);
  msg->return_path->host = strdup(curhst);

  prompt("To: ", line, sizeof(line));
  rfc822_parse_adrlist(&msg->to, line, curhst);
  if (msg->to) {
    prompt("cc: ", line, sizeof(line));
    rfc822_parse_adrlist(&msg->cc, line, curhst);
  } else {
    prompt("Newsgroups: ", line, sizeof(line));
    if (*line) {
      msg->newsgroups = strdup(line);
    } else {
      mail_free_body(&body);
      mail_free_envelope(&msg);
      fs_give((void **) &text);
    }
  }

  prompt("Subject: ", line, sizeof(line));
  msg->subject = strdup(line);
  mm_fputs(" Msg (end with a line with only a '.'):", output);
  body->type = TYPETEXT;
  *text = '\0';
  while (mm_fgets(line, sizeof(line), input)) {
    if (line[0] == '.') {
      if (line[1] == '\0') {
	break;
      } else {
	strcat((char *) text, ".");
      }
    }
    strcat((char *) text, line);
    strcat((char *) text, "\015\012");
  }

  body->contents.text = text;
  rfc822_date(line);
  msg->date = (char *) fs_get(1+strlen(line));
  strcpy(msg->date, line);

  /* new additional headers */
  header_list->n_entries = 5;
  header_list->fields = mail_newstringlist();
  header_list->data = mail_newstringlist();
  data_ptr = header_list->data;
  field_ptr = header_list->fields;

  for (i = 0; i < header_list->n_entries; i++) {
    if (i != 0) {
      field_ptr->next = mail_newstringlist();
      data_ptr->next = mail_newstringlist();
      data_ptr = data_ptr->next;
      field_ptr = field_ptr->next;
    }
    sprintf(tmp,  "X-test-%d",  i);
    data_ptr->text = strdup(tmp);
    field_ptr->text = strdup(tmp);
  }

  if (msg->to) {
    mm_fputs("Sending...", output);
    if (stream = smtp_open((char **)hostlist, debug,  ms)) {
      if (smtp_mail(stream, "MAIL", msg, body,  
		     NIL,  MIMEBF,  NIL,  header_list,  ms)) {
	mm_fputs("[Ok]", output);
      } else {
	fprintf(output, "[Failed - %s]\n", stream->reply);
	if (usingAnswerFile) {
	  fflush(output);
	}
      }
    }
  }
#ifdef WANTNNTP
  else {
    mm_fputs("Posting...", output);
    if (stream = nntp_open(newslist, debug)) {
      if (nntp_mail(stream, msg, body)) {
	mm_fputs("[Ok]", output);
      } else {
	fprintf(output, "[Failed - %s]\n", stream->reply);
	if (usingAnswerFile) {
	  fflush(output);
	}
      }
    }
  }
#endif
  if (stream) {
    smtp_close(stream);
  } else {
    mm_fputs("[Can't open connection to any server]", output);
  }
  mail_free_envelope(&msg);
  mail_free_body(&body);
  mail_free_headerlist(&header_list);
}

/* SUN imap4 additions */
void
mm_io_callback(MAILSTREAM * stream,  long nchars,  int type)
{
  return;
}

void 
mm_tcp_log(MAILSTREAM * stream,  char *err_msg)
{
  fprintf(output, "MM_TCP_LOG:");
  mm_log(err_msg,  NIL,  stream);
  return;
}

void
mm_set_ignore(void)
{
  return;
}

void
mm_checksum(MAILSTREAM * stream,  unsigned short checksum, 
	    long nbytes,  char * driver)
{
  return;
}

void
mm_status(MAILSTREAM * stream, char * mailbox, MAILSTATUS * status)
{
  if (status->checksum) {
    fprintf(output, 
	    "Checksum %ld size(checksum_butes) %lu uid-validity %ld\n",  
	    status->checksum,  
	    status->checksum_bytes,  
	    status->uidvalidity);
    if (usingAnswerFile) {
      fflush(output);
    }
  }
  return;
}

void
mm_log_stream(MAILSTREAM * stream,  char * text)
{
  fprintf(output, "MM_LOG_STREAM:%s\n", text);
  return;
}

void
mm_restore_callback(void *userdata,  unsigned long size, 
		     unsigned long count)
{
  return;
}

/*
 * test functions for dtmail
 */
static int
set_ttlock(char *mailbox, void *userdata)
{
  return _B_TRUE;
}

static int
clear_ttlock(char *mailbox, void *userdata)
{
  return _B_TRUE;
}

static int
set_group(void)
{
  return setegid(effectiveGid) == 0 ? _B_TRUE : _B_FALSE;
}

static int
clear_group(void)
{
  return setegid(realGid) == 0 ? _B_TRUE : _B_FALSE;
}

static void
rename_test(MAILSTREAM * stream, char * arg)
{
  boolean_t		err = _B_FALSE;
  char		*	oldFolder;
  char		*	newFolder;
  char			old[1024];
  char			new[1024];

  if (arg != NULL) {
    oldFolder = strtok(arg, " \t");
    
    if (oldFolder == NULL) {
      err = _B_TRUE;
    } else {

      newFolder = strtok(NULL, " \t\n");

      if (newFolder == NULL) {
	err =  _B_TRUE;
      }
    }
  }
  if (err) {
    mm_fputs("?Usage error \"R <oldfolder> <newfolder>\"", output);
  } else {

    long		rename_error;
    MAILSTREAM	*	newStream;

    makeMailBoxName(oldFolder, old);
    makeMailBoxName(newFolder, new);

    newStream = mail_stream_create(NIL, NULL, NULL, set_group, clear_group);
      
    rename_error = mail_rename(newStream, old, new);

    if (rename_error != _B_TRUE) {
      fprintf(output, "ERROR: mail_rename(stream, %s, %s) - Failed!\n",
	      oldFolder,
	      newFolder);
      if (usingAnswerFile) {
	fflush(output);
      }
    }
  }

  return;
}

static void
mm_fputs(const char *msg, FILE * fp)
{
  fputs(msg, fp);
  fputs("\n", fp);
  if (usingAnswerFile) {
    fflush(fp);
  }
}

static char *
mm_fgets(char * txt, int theSize, FILE * fp)
{
  int	len;

  txt[0] = '\0';
  if (fgets(txt, theSize, fp) == NULL) {
    mm_fputs("EOF reached", fp);
    exit(1);
  }
  len = strlen(txt);
  if (len == 0) {
    mm_fputs("EOF reached", fp);
    exit(1);
  }
  if (txt[len-1] == '\n') {
    txt[len-1] = '\0';
  }
  if (usingAnswerFile) {
    fputs(txt, output);
    fputs("\n", output);
  }
  return(txt);
}


