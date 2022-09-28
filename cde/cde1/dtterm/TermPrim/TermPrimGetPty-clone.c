#ifndef lint
#ifdef  VERBOSE_REV_INFO
static char rcs_id[] = "$XConsortium: TermPrimGetPty-clone.c /main/cde1_maint/1 1995/07/15 01:26:09 drk $";
#endif  /* VERBOSE_REV_INFO */
#endif  /* lint */

/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#include "TermPrimDebug.h"
#include "TermHeader.h"
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <signal.h>
#include <Xm/Xm.h>
#ifdef	HP_ARCHITECTURE
#include <grp.h>
#endif	/* HP_ARCHITECTURE */

#ifdef	__AIX
#define	PTY_CLONE_DEVICE	"/dev/ptc"
#endif	/* __AIX */

static int
GetPty(char **ptySlave, char **ptyMaster)
{
    char *c;
    int ptyFd;
    int ttyFd;

    *ptyMaster = malloc(strlen(PTY_CLONE_DEVICE) + 1);
    (void) strcpy(*ptyMaster, PTY_CLONE_DEVICE);

    if (isDebugFSet('p', 10)) {
#ifdef	BBA
#pragma	BBA_IGNORE
#endif	/*BBA*/
	return(-1);
    }

    if ((ptyFd = open(*ptyMaster, O_RDWR, 0))) {
	if (c = ttyname(ptyFd)) {
	    *ptySlave = malloc(strlen(c) + 1);
	    (void) strcpy(*ptySlave, c);
	    
	    if ((ttyFd = open(*ptySlave, O_RDWR | O_NOCTTY, 0)) < 0) {
		/* failure... */
		(void) perror(*ptySlave);
		(void) close(ptyFd);
	    } else {
		/* success...
		 */
		/* close off the pty slave... */
		(void) close(ttyFd);

		/* fix the owner, mode, and group... */
#ifdef	HP_ARCHITECTURE
		{
		    struct group *grp;
		    gid_t gid;

		    if (grp = getgrnam("tty")) {
			gid = grp->gr_gid;
		    } else {
			gid = 0;
		    }
		    (void) endgrent();
		    (void) chown(*ptySlave, getuid(), gid);
		    (void) chmod(*ptySlave, 0620);
		}
#else	/* HP_ARCHITECTURE */
		(void) chown(*ptySlave, getuid(), getgid());
		(void) chmod(*ptySlave, 0622);
#endif	/* HP_ARCHITECTURE */

		/* pty master and slave names are already set.  Return
		 * the file descriptor...
		 */

		return(ptyFd);
	    }
	} else {
	    /* ttyname on the pty master failed.  This should not happen!... */
	    (void) perror("ttyname");
	    (void) close(ptyFd);
	}
    } else {
	(void) perror(*ptyMaster);
    }
    return(-1);
}

/* this is a public wrapper around the previous function that runs the
 * previous function setuid root...
 */
int
_DtTermPrimGetPty(char **ptySlave, char **ptyMaster)
{
    int retValue;

    /* this function needs to be suid root... */
    (void) _DtTermPrimToggleSuidRoot(True);
    retValue = GetPty(ptySlave, ptyMaster);
    /* we now need to turn off setuid root... */
    (void) _DtTermPrimToggleSuidRoot(False);

    return(retValue);
}

static int
SetupPty(char *ptySlave, int ptyFd)
{
#ifdef	HP_ARCHITECTURE
    {
	struct group *grp;
	gid_t gid;

	if (grp = getgrnam("tty")) {
	    gid = grp->gr_gid;
	} else {
	    gid = 0;
	}
	(void) endgrent();
	(void) chown(ptySlave, getuid(), gid);
	(void) chmod(ptySlave, 0620);
    }
#else	/* HP_ARCHITECTURE */
    (void) chown(ptySlave, getuid(), getgid());
    (void) chmod(ptySlave, 0622);
#endif	/* HP_ARCHITECTURE */
}
    
int
_DtTermPrimSetupPty(char *ptySlave, int ptyFd)
{
    int retValue;

    /* this function needs to be suid root... */
    (void) _DtTermPrimToggleSuidRoot(True);
    retValue = SetupPty(ptySlave, ptyFd);
    /* we now need to turn off setuid root... */
    (void) _DtTermPrimToggleSuidRoot(False);

    return(retValue);
}

static void
ReleasePty(char *ptySlave)
{
    (void) chown(ptySlave, 0, 0);
    (void) chmod(ptySlave, 0666);
}
    
void
_DtTermPrimReleasePty(char *ptySlave)
{
    /* this function needs to be suid root... */
    (void) _DtTermPrimToggleSuidRoot(True);
    (void) ReleasePty(ptySlave);
    /* we now need to turn off setuid root... */
    (void) _DtTermPrimToggleSuidRoot(False);
}

void
_DtTermPrimPtyCleanup()
{
    return;
}
