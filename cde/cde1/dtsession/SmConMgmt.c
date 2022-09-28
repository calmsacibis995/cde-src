/* $XConsortium: SmConMgmt.c /main/cde1_maint/4 1995/10/09 10:56:38 pascale $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:        SmConMgmt.c
 **
 **  Project:     HP DT Session Manager (dtsession)
 **
 **  Description:
 **  -----------
 **	 Contains all code which performs contention management functionality
 **
 **
 *******************************************************************
 **  (c) Copyright Hewlett-Packard Company, 1990.  All rights are  
 **  reserved.  Copying or other reproduction of this program      
 **  except for archival purposes is prohibited without prior      
 **  written consent of Hewlett-Packard Company.		     
 ********************************************************************
 **
 **
 **
 *****************************************************************************
 *************************************<+>*************************************/


#include <stdio.h>
#include <fcntl.h>
#include <nlist.h>
#include <X11/Intrinsic.h>
#include "Sm.h"
#include "SmProtocol.h"
#include "SmConMgmt.h"
#include "SmCommun.h"

/*
 * Define statements
 */

#ifdef mips
#include <sys/fixpoint.h>
#endif /* mips */

#ifndef KMEM_FILE
#define KMEM_FILE "/dev/kmem"
#endif

#ifndef KERNEL_FILE
# ifdef mips
#  define KERNEL_FILE "/vmunix"
# else /* not mips */
#  define KERNEL_FILE "/hp-ux"
# endif /* mips */
#endif

#ifdef mips
#define GPGSLIM "gpgslim"
#define FREEMEM "freemem"
#else /* not mips */
# ifdef __hpux
#  ifdef __hp9000s800
#   define GPGSLIM "gpgslim"
#   define FREEMEM "freemem"
#  endif /* __hp9000s800 */
# endif /* __hpux */
#endif /* mips */

#ifndef GPGSLIM
#define GPGSLIM "_gpgslim"
#endif /* not defined GPGSLIM */

#ifndef FREEMEM
#define FREEMEM "_freemem"
#endif /* not defined FREEMEM */


int clientRunning;
/*
 * Variables global to this module only
 */
static struct nlist namelist[3];
static off_t freemem_loc, gpgslim_loc;
static int gpgslim, freemem;
static int clientTimeout;

/*
 * Functions local to this module
 */
#ifdef _NO_PROTO

static void HandleClientMessage();
static void WaitClientTimeout();

#else

static void HandleClientMessage(Widget smWidget, XtPointer dummy, 
				XEvent *event);
static void WaitClientTimeout(XtPointer, XtIntervalId *);

#endif /*_NO_PROTO*/


/*************************************<->*************************************
 *
 *  GetMemoryUtilization ()
 *
 *
 *  Description:
 *  -----------
 *  Returns 1 of 3 values.  Tells the calling program that memory is not
 *  available, that it is full (paging has started), or that it is not
 *  full.
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *  An integer value representing the current memory utilization of the
 *  system.
 *
 *
 *  Comments:
 *  --------
 *  WARNING:  This routine, by its very nature is non-portable.  It depends
 *  on an OS having some kind of access to its memory utilization.
 *
 *  DOUBLE WARNING: The mips architecture code has never been tested.
 *  
 * 
 *************************************<->***********************************/

int 
#ifdef _NO_PROTO
GetMemoryUtilization()
#else
GetMemoryUtilization(void)
#endif /* _NO_PROTO */
{
    static int init = 0;
    static kmem;
#if !defined(SVR4) && !defined(sco) && !defined(hpV4) && !defined(_POWER) && !defined (__osf__)
    extern void nlist();
#endif
    int i;


#ifdef __hpux
    setresgid(-1, smGD.conMgmtGID, -1);
#else
#ifndef	SVR4
    setregid(smGD.runningGID, smGD.conMgmtGID);
#else
    setgid(smGD.runningGID);
    setegid(smGD.conMgmtGID);
#endif
#endif

    if(!init)
    {
	namelist[0].n_name = FREEMEM;
	namelist[1].n_name = GPGSLIM;
	namelist[2].n_name = (char *) NULL;
	nlist( KERNEL_FILE, namelist);
	for(i = 0; i < 2; i++)
	{
	    if (namelist[i].n_type == 0 ||
		namelist[i].n_value == 0)
	    {
#ifdef __hpux
		setresgid(-1, smGD.runningGID, -1);
#else
#ifndef	SVR4
		setregid(smGD.conMgmtGID, smGD.runningGID);
#else
		setgid(smGD.conMgmtGID);
		setegid(smGD.runningGID);
#endif
#endif
		return(MEM_NOT_AVAILABLE);
	    }
	}
	
	freemem_loc =  namelist[0].n_value;
	gpgslim_loc =  namelist[1].n_value;
	
	kmem = open(KMEM_FILE, O_RDONLY);
	if (kmem < 0)
	{
#ifdef __hpux
	    setresgid(-1, smGD.runningGID, -1);
#else
#ifndef	SVR4
	    setregid(smGD.conMgmtGID, smGD.runningGID);
#else
	    setgid(smGD.conMgmtGID);
	    setegid(smGD.runningGID);
#endif
#endif
	    return(MEM_NOT_AVAILABLE);
	}
	(void) lseek(kmem, gpgslim_loc, 0);
#ifdef mips
        {
	    fix temp;
	    (void) read(kmem, (char *)&temp, sizeof(fix));
	    gpgslim = FIX_TO_DBL(temp);
	}
#else /* not mips */
	(void) read(kmem, (char *)&gpgslim, sizeof(int));
#endif /* mips */
	init = 1;
    }
    
    (void) lseek(kmem, freemem_loc, 0);
#ifdef mips
    {
	fix temp;
	(void) read(kmem, (char *)&temp, sizeof(fix));
	freemem = FIX_TO_DBL(temp);
    }
#else /* not mips */
    (void) read(kmem, (char *)&freemem, sizeof(int));
#endif /* mips */

#ifdef __hpux
    setresgid(-1, smGD.runningGID, -1);
#else
#ifndef	SVR4
    setregid(smGD.conMgmtGID, smGD.runningGID);
#else
    setgid(smGD.conMgmtGID);
    setegid(smGD.runningGID);
#endif
#endif

    if(freemem >= gpgslim)
    {
	return(MEM_NOT_FULL);
    }
    else
    {
	return(MEM_FULL);
    }
}



/*************************************<->*************************************
 *
 *  WaitForClientMap ()
 *
 *
 *  Description:
 *  -----------
 *  This routine waits for the workspace manager to send it information
 *  about a client being mapped, before returning to start the next client
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
WaitForClientMap()
#else
WaitForClientMap( void )
#endif /* _NO_PROTO */
{
    XtInputMask 	isThere;
    XEvent 		event;
    XClientMessageEvent	*cEvent = (XClientMessageEvent *) &event;
    XtIntervalId	clientTimerId;
    
    XtAddEventHandler(smGD.topLevelWid,
		      0,
		      True,
		      (XtEventHandler)HandleClientMessage,
		      (XtPointer) NULL);
    
    /*
     * Set a timer which stops the block on waiting for the
     * client to start.  This value is fetched from the 
     * waitClientTimeout resource.
     */
    clientRunning = False;
    clientTimeout = False;
    clientTimerId = XtAppAddTimeOut(smGD.appCon, 
				    smRes.waitClientTimeout,
				    WaitClientTimeout, NULL);
    
    while((clientRunning == False) && (clientTimeout == False))
    {
	XtAppProcessEvent(smGD.appCon, XtIMAll);
    }
    
    XtRemoveTimeOut(clientTimerId);
    XtRemoveEventHandler(smGD.topLevelWid,
		      0,
		      True,
		      (XtEventHandler)HandleClientMessage,
		      (XtPointer) NULL);
    return;
}


/*************************************<->*************************************
 *
 *  WaitClientTimeout
 *
 *
 *  Description:
 *  -----------
 *  Timeout procedure the WaitForClientMap routine.  It stops a loop waiting
 *  for the last started app to get mapped.
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *  clientTimeout = (global) flag that stops the loop
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
WaitClientTimeout( client_data, id )
XtPointer client_data ;
XtIntervalId *id ;
#else
WaitClientTimeout(
		  XtPointer client_data,
		  XtIntervalId *id )
#endif /* _NO_PROTO */
{
    clientTimeout = True;
    return;
}


/*************************************<->*************************************
 *
 *  HandleClientMessage
 *
 *
 *  Description:
 *  -----------
 *  This is the event handler registered to recieve the client message
 *  from dtwm when a client has beem managed
 *
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
HandleClientMessage( smWidget, dummy, event )
    Widget smWidget;
    XtPointer dummy;
    XEvent *event;
#else
HandleClientMessage( Widget smWidget,
		    XtPointer dummy,
		    XEvent *event)
#endif /* _NO_PROTO */
{
    if (event->type == ClientMessage)
    {
	ProcessClientMessage(event);
    }
    return;
} /* END OF FUNCTION HandleClientMessage */


