/* $XConsortium: CmdSpc.c /main/cde1_maint/1 1995/07/17 18:01:20 drk $ */
/***************************************************************************
*
* File:         CmdSpc.c
* Description:  Messaging support for the command execution system.
* Language:     C
*
** (c) Copyright 1993, 1994 Hewlett-Packard Company
** (c) Copyright 1993, 1994 International Business Machines Corp.
** (c) Copyright 1993, 1994 Sun Microsystems, Inc.
** (c) Copyright 1993, 1994 Novell, Inc.
***************************************************************************/

#include "CmdInvP.h"

#include <Dt/UserMsg.h>
#include <Dt/CommandM.h>


/******************************************************************************
 *
 * _DtCmdSPCAddInputHandler - Register the Command Invoker AddInputHandler
 *
 * PARAMETERS: Those need by a SPC Add Input Handler.
 *
 * NOTES:
 *
 *   This function is needed by SPC to get a process termination 
 *   callback to get invoked.
 *
 *****************************************************************************/

SbInputId
#ifdef _NO_PROTO
_DtCmdSPCAddInputHandler (fd, proc, data)
	int fd;
	SbInputCallbackProc proc;
	void *data;
#else
_DtCmdSPCAddInputHandler ( 
	int fd,
	SbInputCallbackProc proc,
	void *data)
#endif /* _NO_PROTO */
{
   if (cmd_Globals.app_context == NULL)
      return(XtAddInput(fd, 
			(XtPointer) XtInputReadMask, 
			proc, 
			data));
   else
      return(XtAppAddInput(cmd_Globals.app_context, 
			   fd, 
			   (XtPointer) XtInputReadMask, 
	                   proc, 
			   data));
}

/******************************************************************************
 *
 * _DtCmdSPCAddExceptionHandler - Register the Command Invoker AddExceptionHandler
 *
 * PARAMETERS: Those need by a SPC Add Exception Input Handler.
 *
 * NOTES:
 *
 *   This function is needed by SPC to get a process termination 
 *   callback to get invoked.
 *
 *****************************************************************************/

SbInputId
#ifdef _NO_PROTO
_DtCmdSPCAddExceptionHandler (fd, proc, data)
	int fd;
	SbInputCallbackProc proc;
	void *data;
#else
_DtCmdSPCAddExceptionHandler ( 
	int fd,
	SbInputCallbackProc proc,
	void *data)
#endif /* _NO_PROTO */
{
   if (cmd_Globals.app_context == NULL)
      return(XtAddInput(fd, 
			(XtPointer) XtInputExceptMask, 
		        proc, 
			data));
   else
      return(XtAppAddInput(cmd_Globals.app_context, 
			  fd, 
			  (XtPointer) XtInputExceptMask, 
	                  proc, 
			  data));
}

/******************************************************************************
 * 
 * _DtCmdLogErrorMessage - write an error message to the default log file.
 *
 * PARAMETERS:
 *
 *   char *message;	- The error message to log.
 *
 *****************************************************************************/

void 
#ifdef _NO_PROTO
_DtCmdLogErrorMessage( message )
        char *message ;
#else
_DtCmdLogErrorMessage(
        char *message )
#endif /* _NO_PROTO */
{
   _DtSimpleError (DtProgName, DtError, NULL, "%.2000s", message);
}
