#ifndef lint
#ifdef  VERBOSE_REV_INFO
static char rcs_id[] = "$XConsortium: TermPrimGetPty-svr4.c /main/cde1_maint/2 1995/09/29 12:08:02 lehors $";
#endif  /* VERBOSE_REV_INFO */
#endif  /* lint */

/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

/**************************************************************************
 *
 *  Note: This code is based on the pty allocation code from xview.  It
 *        was basically taken intact as were the comments...
 */

#include "TermPrimOSDepI.h"
#include "TermPrimDebug.h"
#include "TermHeader.h"
#include <stropts.h>
#include <sys/conf.h>
#include <sys/stream.h>
#include <sys/termios.h>
#ifdef	USE_STREAMS_BUFMOD
#include <sys/bufmod.h>
#endif	/* USE_STREAMS_BUFMOD */
#include <errno.h>

/* last ditch fallback.  If the clone device is other than /dev/ptmx,
 * it should have been set previously...
 */
#ifndef	PTY_CLONE_DEVICE
#define	PTY_CLONE_DEVICE	"/dev/ptmx"
#endif	/* PTY_CLONE_DEVICE */


int _DtTermPrimGetPty(char **ptySlave, char **ptyMaster)
{
    char *c = NULL;
    int ptyFd = -1;
    extern char *ptsname(int fd);

    *ptyMaster = malloc(strlen(PTY_CLONE_DEVICE) + 1);
    (void) strcpy(*ptyMaster, PTY_CLONE_DEVICE);

    if (isDebugFSet('p', 10)) {
#ifdef	BBA
#pragma	BBA_IGNORE
#endif	/*BBA*/
	return(-1);
    }

    if ((ptyFd = open(*ptyMaster, O_RDWR, 0)) >= 0) {

	/* use grantpt to prevent other processes from grabbing the tty that
	 * goes with the pty master we have opened.  It is a mandatory step
	 * in the SVR4 pty-tty initialization.  Note that /dev must be
	 * mounted read/write...
	 */
	Debug('T', timeStamp("_DtTermPrimGetPty() calling grantpt()"));
	if (grantpt(ptyFd) == -1) {
	    (void) perror("grantpt");
	    (void) close(ptyFd);
	    return(-1);
	}

	/* Unlock the pty master/slave pair so the slave can be opened later */
	Debug('T', timeStamp("_DtTermPrimGetPty() calling unlockpt()"));
	if (unlockpt(ptyFd) == -1) {
	    (void) perror("unlockpt");
	    (void) close(ptyFd);
	    return(-1);
	}
	Debug('T', timeStamp("_DtTermPrimGetPty() unlockpt() finished"));

#ifdef	USE_STREAMS_BUFMOD
	if (ioctl(ptyFd, I_PUSH, "bufmod") == -1) {
	    (void) perror("I_PUSH bufmod");

	    /* We can't push bufmod.  This means that we're probably running
	     * on a generic SVR4 system.  We can ignore this error since
	     * bufmod is used for performance reasons only...
	     */
	} else {
	    struct timeval timeval;
	    struct strioctl cmd;
	    unsigned int chunk;

	    /* Note that we're not using SB_SEND_ON_WRITE | SB_DEFER_CHUNK.
	     * Turns out the shell (or someone down the pty) does an ioctl
	     * when sending out each prompt.  Since this flushes any
	     * partially filled chunk automatically, we really don't need
	     * to do this...
	     */

	    chunk = SB_NO_DROPS | SB_NO_PROTO_CVT | SB_NO_HEADER;
	    cmd.ic_timout = 0;
	    cmd.ic_cmd = SBIOCSFLAGS;
	    cmd.ic_len = sizeof(u_long);
	    cmd.ic_dp = (char *) &chunk;
	    if (ioctl(ptyFd, I_STR, &cmd) < 0) {
		(void) perror("SBIOCSFLAGS");
		/* If we pushed bufmod, but this ioctl fails, it means we're
		 * most likely running on a system with old bufmod (i.e., for
		 * released OSs this must be Jupiter).  We treat this error
		 * silently so developers and users of the Mars trees don't
		 * get confused.  Treat it like bufmod wasn't there at all...
		 */
		goto backoff;
	    }

	    timeval.tv_usec = 50000;
	    timeval.tv_sec = 0;
	    cmd.ic_cmd = SBIOCSTIME;
	    cmd.ic_timout = 0;
	    cmd.ic_len = sizeof(timeval);
	    cmd.ic_dp = (char *) &timeval;
	    if (ioctl(ptyFd, I_STR, &cmd) < 0) {
		/* These are legit errors.  If we have new bufmod, this
		 * should have worked...
		 */
		(void) perror("BSIOCSTIME");
		goto backoff;
	    }

	    /* I have made the chunk size the same as the buffer used in the
	     * ttysw.  One could experiment here, but this works...
	     */
	    {
		/* struct cbuf *sizeit; */
		/* chunk = sizeof(sizeit->cb_buf); */
		chunk = 2048; /* taken from xview source def of cbuf... */
	    }

	    cmd.ic_cmd = SBIOCSCHUNK;
	    cmd.ic_len = sizeof(int);
	    cmd.ic_dp = (char *) &chunk;

	    if (ioctl(ptyFd, I_STR, &cmd) < 0) {
		(void) perror("SBIOCSCHUNK");
		goto backoff;
	    }

	    /* We certainly don't want to truncate any packets, so set the
	     * snap length to zero...
	     */
	    chunk = 0;
	    cmd.ic_cmd = SBIOCSSNAP;
	    cmd.ic_len = sizeof(int);
	    cmd.ic_dp = (char *) &chunk;
	    if (ioctl(ptyFd, I_STR, &cmd) < 0) {
		(void) perror("SBIOCSSNAP");
		goto backoff;
	    }

	    goto ok;
	}

backoff:
	/* Something didn't work out, so pull bofmod off the stream and
	 * continue as if it weren't there...
	 */
	if (ioctl(ptyFd, I_POP, 0) == -1) {
	    /* bufmod not working or wrong version... */
	    (void) perror("I_POP bufmod");
	}

ok:
#endif	/* USE_STREAMS_BUFMOD */

	/* get the pty slave name... */
	if (c = ptsname(ptyFd)) {
	    *ptySlave = malloc(strlen(c) + 1);
	    (void) strcpy(*ptySlave, c);
#ifdef	NOTDEF
	    {
		int slaveFd;
		char buffer[BUFSIZ];

		if ((slaveFd = open(*ptySlave, O_RDWR, 0)) < 0) {
		    (void) perror(*ptySlave);
		    (void) fprintf(stderr, "it failed!\n");
		    (void) sprintf(buffer, "ls -l %s", *ptySlave);
		    (void) system(buffer);
		} else {
		    (void) close(slaveFd);
		}
	    }
#endif	/* NOTDEF */
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

int
_DtTermPrimSetupPty(char *ptySlave, int ptyFd)
{
    /*
     * The following "pushes" were done at GetPty time, but
     * they don't seem to stick after the file is closed on
     * SVR4.2.  Not sure where else this applies.
     */
    if (ioctl(ptyFd, I_PUSH, "ptem") == -1) {
	(void) perror("Error pushing ptem");
	/* exit the subprocess */
	return(1);
    }

    if (ioctl(ptyFd, I_PUSH, "ldterm") == -1) {
	(void) perror("Error pushing ldterm");
	/* exit the subprocess */
	return(1);
    }

#ifdef	USE_STREAMS_TTCOMPAT
    if (ioctl(ptyFd, I_PUSH, "ttcompat") == -1) {
	(void) perror("Error pushing ttcompat");
	/* exit the subprocess */
	return(1);
    }
#endif	/* USE_STREAMS_TTCOMPAT */

    /* success... */
    return(0);
}
 
void
_DtTermPrimReleasePty(char *ptySlave)
{
    /* dummy function for STREAMS... */
}

void
_DtTermPrimPtyCleanup()
{
    /* dummy function for STREAMS... */
    return;
}
