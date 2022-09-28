/* $XConsortium: CmdInv.h /main/cde1_maint/1 1995/07/17 18:00:10 drk $ */
/***************************************************************************
*
* File:         CmdInv.h
* Description:  Public header for the command invocation system.
* Language:     C
*
** (c) Copyright 1993, 1994 Hewlett-Packard Company
** (c) Copyright 1993, 1994 International Business Machines Corp.
** (c) Copyright 1993, 1994 Sun Microsystems, Inc.
** (c) Copyright 1993, 1994 Novell, Inc.
***************************************************************************/

#ifndef _CmdInv_h
#define _CmdInv_h

#include <X11/Xlib.h>
#include <Dt/Message.h>

/******************************************************************************
 *
 * DtCmdInvExecuteProc - This type is is used to define the parameters
 *   needed in the callback functions for success and failure notification 
 *   of a Command Invoker execution call. 
 *
 *****************************************************************************/

typedef void (*DtCmdInvExecuteProc) (
#ifdef _NO_PROTO
#else
	char *message,		/* NULL if the request is successful.  
				 * Otherwise an error message. */
	void *client_data
#endif /* _NO_PROTO */
);

/******************************************************************************
 *
 * Function:  void _DtInitializeCommandInvoker (
 *	Display *display,
 *	char *toolClass,
 *	char *applicationClass,
 *	DtSvcReceiveProc reloadDBHandler,
 *	XtAppContext appContext)
 *
 * Parameters:		
 *
 *	display		 - The X server display connection.
 *			
 *	toolClass	 - The BMS tool class of the client.
 *
 *	applicationClass - The application class of the client (see 
 *			   XtInitialize).  This is needed to add the 
 *			   Command Invoker's resources to the client's
 *			   resources.
 *			
 *	reloadDBHandler  - Function to be called if a RELOAD-TYPES-DB 
 *			   request is made.  If the client does not read
 *			   DT action and/or filetype databases, NULL must
 *			   be used.
 *
 *      appContext	 - The client's application context.  Must be NULL
 *			   if the client does not use an application 
 *                         context.
 *
 * Purpose:		
 *
 * 	This function allows a client to internalize the functionality
 *	of the DT "Command Invoker".  By using this library and the
 *   	Action Library, requests for the Command Invoker will be done
 *	internally instead of sending a request to a separate Command
 *	Invoker process.
 *
 *	For local execution, the "fork" and "execvp" system calls are
 *	used.  For remote execution, the "SPCD" is used.
 *			
 *****************************************************************************/

extern void
#ifdef _NO_PROTO
_DtInitializeCommandInvoker() ;
#else
_DtInitializeCommandInvoker(
	Display *display,
	char *toolClass,
	char *applicationClass,
	DtSvcReceiveProc reloadDBHandler,
	XtAppContext appContext);
#endif /* _NO_PROTO */

/******************************************************************************
 *
 * Function:  void _DtCommandInvokerExecute (
 *	char *request_name,
 *	char *context_host,
 *	char *context_dir,
 *	char *context_file,
 *	char *exec_parameters,
 *	char *exec_host,
 *	char *exec_string,
 *	DtCmdInvExecProc success_proc,
 *	void *success_data,
 *	DtCmdInvExecProc failure_proc,
 *	void *failure_data)
 *
 * Parameters:		
 *
 *	request_name	- The request name (defined in "CommandM.h").
 *			
 *	context_host	- Name of the host where request is executed from.  If
 *			  NULL, the "exec_host" parameter is used.  Note: this
 *			  should not be confused with execution host, described
 *			  below.
 *
 *	context_dir	- Directory where the request should be executed.  If
 *			  NULL, the HOME directory is used.
 *
 *	context_file	- Not currently used by the Command Invoker.
 *
 *	exec_parameters - Command Invoker execution parameters.  See External
 *			  Specification for more information.
 *			  If set to NULL, "-" is used.
 *
 *	exec_host	- Name of the host where the request is executed.
 *
 *	exec_string	- The command line to execute.
 *
 *	success_proc	- The function to be invoked if the request is 
 *			  successfully executed.
 *
 *	success_data	- Client data for successful execution.
 *
 *	failure_proc	- The function to be invoked if an attempt to exeucte
 *			  the request fails.
 *
 *	failure_data	- Client data for unsuccessful execution.
 *
 * Purpose:		
 *
 * 	This function allows a client to use the DT "Command Invoker"
 *	Library for its' process execution.  This function is intended
 *	for processes which do not use the Action Library.
 *
 *	For local execution, the "fork" and "execvp" system calls are
 *	used.  For remote execution, the "SPCD" is used.
 *
 * Notes:
 *
 *      This function must be preceeded by a call to 
 *	"_DtInitializeCommandInvoker".
 *			
 *****************************************************************************/

extern void
#ifdef _NO_PROTO
_DtCommandInvokerExecute() ;
#else
_DtCommandInvokerExecute(
	char *request_name,
	char *context_host,
	char *context_dir,
	char *context_file,
	char *exec_parameters,
	char *exec_host,
	char *exec_string,
	DtCmdInvExecuteProc success_proc,
	void *success_data,
	DtCmdInvExecuteProc failure_proc,
	void *failure_data);
#endif /* _NO_PROTO */

#endif /* _CmdInv_h */
