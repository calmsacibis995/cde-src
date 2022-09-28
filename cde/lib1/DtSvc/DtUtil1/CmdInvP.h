/* $XConsortium: CmdInvP.h /main/cde1_maint/1 1995/07/17 18:00:32 drk $ */
/***************************************************************************
*
* File:         CmdInvP.h
* Description:  Command execution system private externs and includes
* Language:     C
*
** (c) Copyright 1993, 1994 Hewlett-Packard Company
** (c) Copyright 1993, 1994 International Business Machines Corp.
** (c) Copyright 1993, 1994 Sun Microsystems, Inc.
** (c) Copyright 1993, 1994 Novell, Inc.
***************************************************************************/

#ifndef _CmdInvP_h
#define _CmdInvP_h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>   /* MAXPATHNAMELEN */

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#define  TOOLKIT

#include <Dt/DtP.h>
#include <Dt/DtNlUtils.h>
#include <Dt/Message.h>
#include <bms/SbEvent.h>
#include <bms/XeUserMsg.h>
#include <Dt/Spc.h>
#include <Dt/CmdInv.h>
#include <Dt/ActionP.h>
#include <Dt/Connect.h>

#ifndef CDE_INSTALLATION_TOP
#define CDE_INSTALLATION_TOP "/opt/dt"
#endif

/*
 * Resource names and classes for the Command Invoker.
 */
#define DtLOCAL_TERMINAL_NAME           "localTerminal"
#define DtLOCAL_TERMINAL_CLASS          "LocalTerminal"

#define DtREMOTE_TERMINALS_NAME         "remoteTerminals"
#define DtREMOTE_TERMINALS_CLASS        "RemoteTerminals"

#define DtWAIT_TIME_NAME         	"waitTime"
#define DtWAIT_TIME_CLASS        	"WaitTime"

#define DtDTEXEC_PATH_NAME        	"dtexecPath"
#define DtDTEXEC_PATH_CLASS         	"DtexecPath"

#define DtEXECUTION_HOST_LOGGING_NAME	"executionHostLogging"
#define DtEXECUTION_HOST_LOGGING_CLASS	"ExecutionHostLogging"


/*
 * Structure for saving the "state" of a remote request that is
 * queued while waiting for a remote subprocess to terminate.
 */
typedef struct _Cmd_RequestQueue {
   SPC_Channel_Ptr		channel;
   char 			*context;
   char 			*exec_host;
   char 			*exec_string;
   char				**argv;
   int				winType;
   unsigned long		request_num;
   DtSvcMsgContext		replyContext;
   DtCmdInvExecuteProc		success_proc;
   void				*success_data;
   DtCmdInvExecuteProc		failure_proc;
   void				*failure_data;
   struct _Cmd_RequestQueue	*next;
} Cmd_RequestQueue;

/*
 * Command invocation resources.
 */
typedef struct {
   char				*localTerminal;
   char				**remoteHosts;
   char				**remoteTerminals;
   int				waitTime;
   char				*dtexecPath;
   Boolean			executionHostLogging;
} Cmd_Resources;

/*
 * Command invocation global variables.
 */
typedef struct {
   Boolean			terminal_ok;
   Boolean			subprocess_ok;
   XtAppContext			app_context;
   char 			**path_list;
   char				*error_directory_name_map;
   char				*error_subprocess;
   char				*error_terminal;
} Cmd_Globals;

/*
 * Defaults for the Resources.
 */
#define DtWAIT_TIME_DEFAULT		3	/* In seconds. */
#define DtTERMINAL_DEFAULT		"dtterm"

#define DtCMD_INV_SUB_PROCESS		CDE_INSTALLATION_TOP "/bin/dtexec"

/*
 * Temporary buffer size.
 */
#define MAX_BUF_SIZE			1024

/* 
 * Window types:
 */
#define NO_STDIO			0
#define TERMINAL			1
#define PERM_TERMINAL			2

/*
 * Command execution return status:
 */
#define _CMD_EXECUTE_SUCCESS		1
#define _CMD_EXECUTE_FAILURE		2
#define _CMD_EXECUTE_QUEUED		3
#define _CMD_EXECUTE_FATAL		4

/*
 * External declarations for the global Command Invoker variables.
 */
extern Cmd_Resources 	cmd_Resources;
extern Cmd_Globals 	cmd_Globals;

/*
 *******    Public Function Declarations for CmdSpc.c    *******
 */
#ifdef _NO_PROTO

extern SbInputId _DtCmdSPCAddInputHandler() ;
extern SbInputId _DtCmdSPCAddExceptionHandler() ;
extern void _DtCmdLogErrorMessage() ;

#else

extern SbInputId _DtCmdSPCAddInputHandler (
			int fd,
			SbInputCallbackProc proc,
			void *data);
extern SbInputId _DtCmdSPCAddExceptionHandler (
			int fd,
			SbInputCallbackProc proc,
			void *data);
extern void _DtCmdLogErrorMessage( 
                        char *message) ;

#endif /* _NO_PROTO */

/*
 *******    Public Function Declarations for CmdProcess.c    *******
 */
#ifdef _NO_PROTO

extern void _DtCmdCreateTerminalCommand() ;
extern Boolean _DtCmdCheckForExecutable() ;
extern Boolean _DtCmdValidDir() ;
extern void _DtCmdGetResources() ;

#else

extern void _DtCmdCreateTerminalCommand( 
                        char **theCommand,
                        int windowType,
                        char *execString,
                        char *execParms,
			char *execHost,
                        char *procId,
                        char *tmpFiles) ;
extern Boolean _DtCmdCheckForExecutable( 
                        char *fileName) ;
extern Boolean _DtCmdValidDir( 
                        char *clientHost,
                        char *contextDir,
			char *contextHost) ;
extern void _DtCmdGetResources( 
			Display *display);

#endif /* _NO_PROTO */


/*
 *******    Public Function Declarations for CmdUtilityP.c    *******
 */
#ifdef _NO_PROTO

extern void _DtCmdBuildPathList() ;

#else

extern void _DtCmdBuildPathList( void ) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

#endif /* _CmdInvP_h */
