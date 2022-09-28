#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stropts.h>
#include <sys/conf.h>
#include <sys/stream.h>
#include <sys/termios.h>
#include <sys/select.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>
#include <Xm/Xm.h>
#include <Dt/Term.h>

/* last ditch fallback.  If the clone device is other than /dev/ptmx,
 * it should have been set previously...
 */
#ifndef PTY_CLONE_DEVICE
#define PTY_CLONE_DEVICE        "/dev/ptmx"
#endif  /* PTY_CLONE_DEVICE */

#define ARGS 128 

#define XTTYMODE_intr    0
#define XTTYMODE_quit    1
#define XTTYMODE_erase   2
#define XTTYMODE_kill    3
#define XTTYMODE_eof     4
#define XTTYMODE_eol     5
#define XTTYMODE_swtch   6
#define XTTYMODE_start   7
#define XTTYMODE_stop    8
#define XTTYMODE_brk     9
#define XTTYMODE_susp   10
#define XTTYMODE_dsusp  11
#define XTTYMODE_rprnt  12
#define XTTYMODE_flush  13
#define XTTYMODE_weras  14
#define XTTYMODE_lnext  15
#define NXTTYMODES      16


typedef struct ttyMode
{
    char    *name;
    int      len;
    int      set;
    char     value;
}
ttyMode;

ttyMode TtyModeList[] =
{
    { "intr" , 4, 0, '\0' }, /* tchars.t_intrc ; VINTR   */
    { "quit" , 4, 0, '\0' }, /* tchars.t_quitc ; VQUIT   */
    { "erase", 5, 0, '\0' }, /* sgttyb.sg_erase ; VERASE */
    { "kill" , 4, 0, '\0' }, /* sgttyb.sg_kill ; VKILL   */
    { "eof"  , 3, 0, '\0' }, /* tchars.t_eofc ; VEOF     */
    { "eol"  , 3, 0, '\0' }, /* VEOL                     */
    { "swtch", 5, 0, '\0' }, /* VSWTCH                   */
    { "start", 5, 0, '\0' }, /* tchars.t_startc          */
    { "stop" , 4, 0, '\0' }, /* tchars.t_stopc           */
    { "brk"  , 3, 0, '\0' }, /* tchars.t_brkc            */
    { "susp" , 4, 0, '\0' }, /* ltchars.t_suspc          */
    { "dsusp", 5, 1, 0x19 }, /* ltchars.t_dsuspc ^y      */
    { "rprnt", 5, 1, 0x12 }, /* ltchars.t_rprntc ^r      */
    { "flush", 5, 1, 0x0F }, /* ltchars.t_flushc ^o      */
    { "weras", 5, 1, 0x17 }, /* ltchars.t_werasc ^w      */
    { "lnext", 5, 1, 0x16 }, /* ltchars.t_lnextc ^v      */
    {    NULL, 0, 0, '\0' }, /* NULL terminate the array */
};

typedef struct modeRec {
        char *ttyModes;         /* ttyModes string      */
} modeRec;

static XtResource resources[] =
{
    {
        DtNttyModes, DtCTtyModes, XmRString, sizeof(char *),
        XtOffsetOf(modeRec, ttyModes),
        XtRImmediate, (XtPointer) NULL
    }
};

parseTtyModes
(
    char    *modeString,
    ttyMode *modeList
)
{
    ttyMode    *pMode;
    int         c, i;
    int         modeCount = 0;

    /*
    ** Search to the end of the.
    */

    while (1) {
        /*
        ** Skip white space,  if this is the end of the list,
        ** return.
        */
        while (*modeString && isascii(*modeString) && isspace(*modeString))
             modeString++;

        if (!*modeString)
            return(modeCount);

        /* Otherwise, see if 'modeString' is in the list of mode names.  */
        for (pMode = modeList; pMode->name; pMode++)
            if (strncmp(modeString, pMode->name, pMode->len) == 0)
		break;
        
        if (!pMode->name)
            return(-1);

        /*
        ** Now look for a value for the setting.
        ** (Skip white space, return an error if no value.)
        */
        modeString += pMode->len;
        while (*modeString && isascii(*modeString) && isspace(*modeString))
            modeString++;
       
        if (!*modeString)
            return(-1);
        
        /*
        ** Make sure we handle control characters correctly.
        */
        if (*modeString == '^')
        {
            modeString++;
            /*
            ** keep control bits
            */
            c = ((*modeString == '?') ? 0177 : *modeString & 31);
        }
        else
            c = *modeString;
        
        /*
        ** Set the values, and get go back for more.
        */
        pMode->value = c;
        pMode->set   = 1;
        modeCount++;
        modeString++;
    }
}

int GetPty(char **ptySlave, char **ptyMaster)
{
    char *c;
    int ptyFd;
    int ttyFd;
    extern char *ptsname(int fd);

    *ptyMaster = malloc(strlen(PTY_CLONE_DEVICE) + 1);
    (void) strcpy(*ptyMaster, PTY_CLONE_DEVICE);

    if ((ptyFd = open(*ptyMaster, O_RDWR | O_NONBLOCK, 0)) >= 0) {

        /* use grantpt to prevent other processes from grabbing the tty that
         * goes with the pty master we have opened.  It is a mandatory step
         * in the SVR4 pty-tty initialization.  Note that /dev must be
         * mounted read/write...
         */
        if (grantpt(ptyFd) == -1) {
            (void) perror("grantpt");
            (void) close(ptyFd);
            return(-1);
        }
        /* Unlock the pty master/slave pair so the slave can be opened later */
        if (unlockpt(ptyFd) == -1) {
            (void) perror("unlockpt");
            (void) close(ptyFd);
            return(-1);
        }

        /* get the pty slave name... */
        if (c = ptsname(ptyFd)) {
            *ptySlave = malloc(strlen(c) + 1);
            (void) strcpy(*ptySlave, c);
            return(ptyFd);
        } else {
            /* ptsname on the pty master failed.  This should not happen!... */
            (void) perror("ptsname");
            (void) close(ptyFd);
        }
    } else {
        (void) perror(*ptyMaster);
    }
    return(-1);
}

/* set up the slave side of the pty */
void PtyInit(int pty, char *modeString)
{
    struct termios tio;

    if (ioctl(pty, I_PUSH, "ptem") < 0) {
	(void) perror("I_PUSH ptem");
    }

    if (ioctl(pty, I_PUSH, "ldterm") < 0) {
	(void) perror("I_PUSH ldterm");
    }

    if (ioctl(pty, I_PUSH, "ttcompat") < 0) {
	(void) perror("I_PUSH ttcompat");
    }

    /* let's set a reasonable default... */
    (void) memset(&tio, '\0', sizeof(tio));

    tio.c_iflag       = ICRNL | IXON   | IXOFF;
    tio.c_oflag       = OPOST | ONLCR  | TAB3;
    tio.c_cflag       = ~(PARENB) | B9600 | CS8    | CREAD | HUPCL;
    tio.c_lflag       = ISIG|ECHO|ECHOE|ECHOK|IEXTEN|ECHOCTL|ECHOKE;
    tio.c_cc[VINTR]   = 0x7f;         /* DEL          */
    tio.c_cc[VQUIT]   = '\\' & 0x3f;  /* '^\'         */
    tio.c_cc[VERASE]  = 0x23;         /* '#'          */
    tio.c_cc[VKILL]   = 0x40;          /* '@'          */
    tio.c_cc[VEOF]    = 'D' & 0x3f;   /* '^D'         */
    tio.c_cc[VEOL]    = '@' & 0x3f;   /* '^@'         */

    /* now, let's clean up certain flags... */
    /* input: nl->nl, don't ignore cr, cr->nl
     *        turn on IXOFF pacing so that we can do paste without
     *        overflowing the buffer...
     */
    tio.c_iflag &= ~(INLCR | IGNCR);
    tio.c_iflag |=   ICRNL | IXOFF;

    /* output: cr->cr, nl is not return, no delays, nl->cr/nl
     */
    tio.c_oflag &= ~(OCRNL | ONLRET | NLDLY | CRDLY | TABDLY |
                     BSDLY | VTDLY  | FFDLY);
    tio.c_oflag |=   ONLCR;
    tio.c_cflag      &= ~(PARENB);

    /* baud rate is 9600 (nice default), turn off clocal and turn on
     * hupcl so that the last close will SIGHUP processes running on
     * the tty...
     */
    tio.c_cflag &= ~(PARENB | CBAUD | CLOCAL | PARENB);
    tio.c_cflag |= B9600 | HUPCL;

    /* enable signals, canonical processing (erase, kill, etc), echo...
     */
    tio.c_lflag |= ISIG | ICANON | IEXTEN | ECHOKE;

    /* reset EOL to the default value (ksh mucks this up sometimes)...
     */
    tio.c_cc[VEOL] = '@' & 0x3f;                        /* '^@' */

    /* reset EOF to the default value (ksh and csh muck with this)... */
    (void) tcsetattr(pty, TCSADRAIN, &tio);

    tio.c_cc[VEOF] = 'D' & 0x3f;                        /* '^D' */


    /* Now its time to handle the ttyModes. */

#define TMODE(ind,var) if (TtyModeList[ind].set) var = TtyModeList[ind].value;
    
    if (modeString) {
	if (parseTtyModes(modeString, TtyModeList) < 0)
            fprintf(stderr, "Bad tty modes \"%s\"\n", modeString);
        else {
            TMODE (XTTYMODE_intr,  tio.c_cc[VINTR]);
            TMODE (XTTYMODE_quit,  tio.c_cc[VQUIT]);
            TMODE (XTTYMODE_erase, tio.c_cc[VERASE]);
            TMODE (XTTYMODE_kill,  tio.c_cc[VKILL]);
            TMODE (XTTYMODE_eof,   tio.c_cc[VEOF]);
            TMODE (XTTYMODE_eol,   tio.c_cc[VEOL]);
    	    TMODE (XTTYMODE_swtch, tio.c_cc[VSWTCH]);
            TMODE (XTTYMODE_start, tio.c_cc[VSTART]);
            TMODE (XTTYMODE_stop,  tio.c_cc[VSTOP]);
            TMODE (XTTYMODE_susp,  tio.c_cc[VSUSP]);
            TMODE (XTTYMODE_dsusp, tio.c_cc[VDSUSP]);
            TMODE (XTTYMODE_rprnt, tio.c_cc[VREPRINT]);
            TMODE (XTTYMODE_flush, tio.c_cc[VDISCARD]);
            TMODE (XTTYMODE_weras, tio.c_cc[VWERASE]);
            TMODE (XTTYMODE_lnext, tio.c_cc[VLNEXT]);	
	}
#undef TMODE
    }
    (void) tcsetattr(pty, TCSADRAIN, &tio);
}

char *SetupArgs(int argc, char *argv[], char *args[], int loginShell)
{
    char *shellpath;
    char *shell;
    char *str1, *str2;
    int i;

    /* get $SHELL */
    shellpath = getenv("SHELL");
    str1 = strdup(shellpath);

    /* extract shell name */
    str2 = strtok(str1, "/");
    while ((str2 = strtok((char *)NULL, "/")) != NULL)
	shell = strdup(str2);
    
    /* if loginShell is set, then pre-pend a '-' to shell. */
    args[0] = malloc(sizeof(shell) + 2);
    if (loginShell) {
           (void) strcpy(args[0], "-");
           strcat(args[0], shell);
    } else
           (void) strcpy(args[0], shell);

    /* Load rest of args array */
    for (i = 1; i < argc; i++) 
	args[i] = argv[i];
    args[i] = '\0';

    free(shell);
    free(str2);
    free(str1);
    return (shellpath);
}

pid_t
SubprocExec(int argc, char *argv[], int ptyMaster, char *ptySlaveName, int loginShell, char *ttyModes)
{
    char *path;
    char buf[BUFSIZ];
    char *args[ARGS];
    struct pollfd pollfds[1];
    int i;
    int ctrl = 0;
    char *eot;
    int ptySlave;
    pid_t pid;
    int err;
    int savedStderr;
    struct sigaction sa;
    sigset_t ss;
    int len;

    /* Set up the args list */
    path = SetupArgs(argc, argv, args, loginShell);

    savedStderr = fcntl(2, F_DUPFD, 3);

    /* set close on exec flags on all files... */
    for (i = 0; i < _NFILE; i++) {
        (void) fcntl(i, F_SETFD, 1);
    }

    /* do a setpgrp() so that we can... */
    (void) setpgrp();

    /* open the pty slave as our controlling terminal... */
    ptySlave = open(ptySlaveName, O_RDWR, 0);

    if (ptySlave < 0) {
        (void) perror(ptySlaveName);
        (void) _exit(1);
    }

    /* Initialize pty characteristics */
    (void) PtyInit(ptySlave, ttyModes);

    /* fork.  We can't use vfork() since we need to do lots of stuff
     * below...
     */
    for (i = 0; ((pid = fork()) < 0) && (i < 10); i++) {
        /* if we are out of process slots, then let's sleep a bit and
         * try again...
         */
        if (errno != EAGAIN) {
            break;
        }

        /* give it a chance to clear up... */
        (void) sleep((unsigned long) 2);
    }

    if (pid < 0) {
        (void) perror("fork()");
        return((pid_t) - 1);

    } else if (pid == 0) {
        /* child...
         */

        /* dup pty into fd's 0, 1, and 2... */
        for (i = 0; i < 3; i++)
            if (i != ptySlave) {
                (void) close(i);
                (void) dup(ptySlave);
            }

	/* flush the pty's read & write queues */
	ioctl(ptySlave, I_FLUSH, FLUSHRW);

        if (ptySlave >= 3) 
            (void) close(ptySlave);

        /* reset any alarms... */
        (void) alarm(0);

        /* reset all signal handlers... */
        sa.sa_handler = SIG_DFL;
        (void) sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        for (i = 1; i < NSIG; i++)
            (void) sigaction(i, &sa, (struct sigaction *) 0);

        /* unblock all signals... */
        (void) sigemptyset(&ss);
        (void) sigprocmask(SIG_SETMASK, &ss, (sigset_t *) 0);

	/* fire off the shell... */
        (void) execv(path, args);

        /* if we got to this point we error'ed out.  Let's write out the
         * error...
         */
        err = errno;

        /* restore stderr... */
        (void) close(2);
        (void) dup(savedStderr);

        /* restore errno... */
        errno = err;
        (void) perror(argv[0]);

        /* and we need to exit the subprocess... */
        _exit(1);
    }


    /* Parent...
     */
    (void) close(savedStderr);

    /* set up polling structure */
    pollfds[0].fd = ptyMaster;
    pollfds[0].events = POLLIN;

    /* clear out the read buffers */
    bzero(buf, sizeof(buf));

    /* ^D */
    eot = "\4";

    while (waitpid((pid_t)-1, 0, WNOHANG) == 0) {

	pollfds[0].revents = 0;

	/* Poll for activity on ptyMaster */
	if (poll(pollfds, 1, 200) < 0) {
		(void) perror("poll failed");
		_exit(1);
	}
	switch (pollfds[0].revents) {

	    case POLLIN: 	/* Read incoming data */
		while ((len = read(pollfds[0].fd, buf, sizeof(buf))) > 0) {

		    /* print out the string unless it's a ^D  */
		    if (buf[0] == '^' && buf[1] == 'D')
			ctrl = 1;
		    else 
			fprintf(stderr, "%s", buf);

		    bzero(buf, sizeof(buf));
		}

		/* If we've started a login shell, and if the last character
		   read was not a control character, send a ^D.  This needs 
		   to be done in case the shell looks for input while sourcing 
		   the user's .login or .profile...  */

		if ( loginShell && (ctrl != 1) ) {
			write(ptyMaster, eot, 1);
			ctrl = 0;
		}
		break;

	    case 0: /* No events */
		break;
	    default: /* error */
		(void) perror ("poll error");
		_exit(1);
	} /* switch */
    } /* while */
   
    (void) close(ptyMaster);
}


main(argc, argv)
int argc;
char *argv[];
{
   char *srclogin;
   char *home;
   int  ptyMaster = 0;
   char *ptyMasterName = (char *) 0;
   char *ptySlaveName = (char *) 0;
   int loginShell = 0;
   Widget appshell;
   XtAppContext appContext;
   modeRec mode;

   /* get $HOME */
   home = getenv("HOME");

   /* Check $DTSOURCEPROFILE, set loginShell if true */
   if ((srclogin = getenv("DTSOURCEPROFILE")) && 
			(strcmp(srclogin, "true") == 0))
		loginShell = 1;
 
   /* Allocate a pty pair */
   ptyMaster = GetPty(&ptySlaveName, &ptyMasterName);
   if (ptyMaster < 0) {
	(void) perror("unable to get pty");
        _exit(1);
   }
   else
	fprintf(stderr, "--- starting desktop on %s\n", ptySlaveName);

   /* Get the ttyModes resource */
   appshell = XtAppInitialize(&appContext, "Sdt_shell", NULL, 0, &argc,
		argv, NULL, NULL, 0);
   XtGetApplicationResources(appshell, (XtPointer)&mode, resources, 
		XtNumber(resources), NULL, 0);

   /* Fire off the desktop */
   SubprocExec(argc, argv, ptyMaster, ptySlaveName, loginShell, mode.ttyModes);

}
