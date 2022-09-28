/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*
 * xdm - display manager daemon
 *
 * $XConsortium: dpylist.c /main/cde1_maint/2 1995/10/16 13:11:30 mgreess $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * a simple linked list of known displays
 */

# include "dm.h"
# include "vgmsg.h"

struct display	*displays;


int 
#ifdef _NO_PROTO
AnyDisplaysLeft()
#else
AnyDisplaysLeft( void )
#endif /* _NO_PROTO */
{
	return displays != (struct display *) 0;
}

void
#ifdef _NO_PROTO
ForEachDisplay( f )
        void (*f)() ;
#else
ForEachDisplay( void (*f)() )
#endif /* _NO_PROTO */
{
	struct display	*d, *next;

	for (d = displays; d; d = next) {
		next = d->next;
		(*f) (d);
	}
}

struct display * 
#ifdef _NO_PROTO
FindDisplayByName( name )
        char *name ;
#else
FindDisplayByName( char *name )
#endif /* _NO_PROTO */
{
	struct display	*d;

	for (d = displays; d; d = d->next)
		if (!strcmp (name, d->name))
			return d;
	return 0;
}

struct display *
#ifdef _NO_PROTO
FindDisplayByPid (pid)
        int pid ;
#else
FindDisplayByPid( int pid )
#endif /* _NO_PROTO */
{
	struct display	*d;

	for (d = displays; d; d = d->next)
		if (pid == d->pid)
			return d;
	return 0;
}

struct display * 
#ifdef _NO_PROTO
FindDisplayByServerPid( serverPid )
        int serverPid ;
#else
FindDisplayByServerPid( int serverPid )
#endif /* _NO_PROTO */
{
	struct display	*d;

	for (d = displays; d; d = d->next)
		if (serverPid == d->serverPid)
			return d;
	return 0;
}

struct display * 
#ifdef _NO_PROTO
FindDisplayBySessionID( sessionID )
        CARD32 sessionID ;
#else
FindDisplayBySessionID( CARD32 sessionID )
#endif /* _NO_PROTO */
{
    struct display	*d;

    for (d = displays; d; d = d->next)
	if (sessionID == d->sessionID)
	    return d;
    return 0;
}

struct display * 
#ifdef _NO_PROTO
FindDisplayByAddress( addr, addrlen, displayNumber )
        struct sockaddr *addr ;
        int addrlen ;
        CARD16 displayNumber ;
#else
FindDisplayByAddress(struct sockaddr *addr, int addrlen,
#if NeedWidePrototypes
        int displayNumber )
#else
        CARD16 displayNumber )
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{
    struct display  *d;

    for (d = displays; d; d = d->next)
	if (d->displayType.origin == FromXDMCP &&
	    d->displayNumber == displayNumber &&
	    addressEqual ((char *)d->from, d->fromlen, (char *)addr, addrlen))
	{
	    return d;
	}
    return 0;
}

#define IfFree(x)  if (x) free ((char *) x)
    
void
#ifdef _NO_PROTO
RemoveDisplay( old )
        struct display *old ;
#else
RemoveDisplay( struct display *old )
#endif /* _NO_PROTO */
{
    struct display	*d, *p;
    char		**x;
    int                       i;

    extern int		wakeupTime;
    
    Debug("Removing display %s from display list.\n", old->name);
    
    p = 0;
    for (d = displays; d; d = d->next) {
	if (d == old) {
	    if (p) {
		p->next = d->next;
	    }
	    else {
		displays = d->next;
	    }

	    if (d->displayType.location == Local) {
	        char        old_pipe[MAXPATHLEN];
		struct stat statbuf;
		/*
         	 * remove old communication pipe (if present).
         	 */
        	sprintf(old_pipe, "%s/%d", SDTLOGIN_DIR, d->displayNumber);
	        if (stat(old_pipe, &statbuf) != 1) unlink(old_pipe);
	    }

	    IfFree (d->name);
	    IfFree (d->class);
	    for (x = d->argv; x && *x; x++)
		IfFree (*x);
	    IfFree (d->argv);
	    IfFree (d->resources);
	    IfFree (d->xrdb);
	    IfFree (d->cpp);
	    IfFree (d->setup);
	    IfFree (d->startup);
	    IfFree (d->reset);
	    IfFree (d->session);
	    IfFree (d->userPath);
	    IfFree (d->systemPath);
	    IfFree (d->systemShell);
	    IfFree (d->failsafeClient);
	    IfFree (d->chooser);
#ifdef __osf__
	    if (d->authorizations)
	    {
		for (i = 0; i < d->authNum; i++)
		    XauDisposeAuth (d->authorizations[i]);
		free ((char *) d->authorizations);
            }
#else
	    if (d->authorization)
		XauDisposeAuth (d->authorization);
#endif
	    IfFree (d->clientAuthFile);
	    if (d->authFile)
		(void) unlink (d->authFile);
	    IfFree (d->authFile);
	    IfFree (d->userAuthDir);
#ifdef __osf__
	    IfFree (d->authNames);
#else
	    IfFree (d->authName);
#endif
	    IfFree (d->peer);
	    IfFree (d->from);
            XdmcpDisposeARRAY8(&d->clientAddr);
	    IfFree (d->language);
	    IfFree (d->langList);
	    IfFree (d->utmpId);
	    IfFree (d->gettyLine);
	    IfFree (d->gettySpeed);
	    IfFree (d->environStr);
	    IfFree (d->verifyName);

	    /*
	     *  turn off polling if we are removing a suspended display...
	     */
	     
	    if ( d->status == suspended )
	    	wakeupTime = -1;


	    free ((char *) d);
	    break;
	}
	p = d;
    }
}

struct display * 
#ifdef _NO_PROTO
NewDisplay( name, class )
        char *name ;
        char *class ;
#else
NewDisplay( char *name, char *class )
#endif /* _NO_PROTO */
{
    struct display	*d;

    d = (struct display *) malloc (sizeof (struct display));
    if (!d) {
	LogOutOfMem (ReadCatalog(MC_LOG_SET,MC_LOG_NEW_DPY,MC_DEF_LOG_NEW_DPY));
	return 0;
    }
    d->next = displays;
    d->name = malloc ((unsigned) (strlen (name) + 1));
    if (!d->name) {
	LogOutOfMem (ReadCatalog(MC_LOG_SET,MC_LOG_NEW_DPY,MC_DEF_LOG_NEW_DPY));
	free ((char *) d);
	return 0;
    }
    strcpy (d->name, name);
    if (class)
    {
	d->class = malloc ((unsigned) (strlen (class) + 1));
	if (!d->class) {
	    LogOutOfMem(
		ReadCatalog(MC_LOG_SET,MC_LOG_NEW_DPY,MC_DEF_LOG_NEW_DPY));
	    free (d->name);
	    free ((char *) d);
	    return 0;
	}
	strcpy (d->class, class);
    }
    else
    {
	d->class = (char *) 0;
    }
    /* initialize every field to avoid possible problems */
    d->argv = 0;
    d->status = notRunning;
    d->pid = -1;
    d->serverPid = -1;
    d->state = NewEntry;
    d->resources = NULL;
    d->xrdb = NULL;
    d->cpp = NULL;
    d->setup = NULL;
    d->startup = NULL;
    d->reset = NULL;
    d->session = NULL;
    d->userPath = NULL;
    d->systemPath = NULL;
    d->systemShell = NULL;
    d->failsafeClient = NULL;
    d->chooser = NULL;
    d->authorize = FALSE;
#ifdef __osf__
    d->authorizations = NULL;
    d->authNum = 0;
    d->authNameNum = 0;
#else
    d->authorization = NULL;
#endif
    d->clientAuthFile = NULL;
    d->authFile = NULL;
    d->userAuthDir = NULL;
#ifdef __osf__
    d->authNames = NULL;
    d->authNameLens = NULL;
#else
    d->authName = NULL;
    d->authNameLen = 0;
#endif
    d->openDelay = 0;
    d->openRepeat = 0;
    d->openTimeout = 0;
    d->startAttempts = 0;
    d->startTries = 0;
    d->unexpectedServerDeath = 0;
    d->terminateServer = 0;
    d->grabTimeout = 0;
    d->sessionID = 0;
    d->peer = 0;
    d->peerlen = 0;
    d->from = 0;
    d->fromlen = 0;
    d->displayNumber = 0;
    d->useChooser = 0;
    d->clientAddr.data = NULL;
    d->clientAddr.length = 0;
    d->language = NULL;
    d->langList = NULL;
    d->utmpId = NULL;
    d->gettyLine = NULL;
    d->gettySpeed = NULL;
    d->environStr = NULL;
    d->dtlite = FALSE;
    d->verifyName = NULL;
    d->pmSearchPath = NULL;
    d->bmSearchPath = NULL;
    displays = d;
    return d;
}

