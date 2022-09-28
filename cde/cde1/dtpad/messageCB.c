/**********************************<+>*************************************
***************************************************************************
**
**  File:        messageCB.c
**
**  Project:     DT dtpad, a memo maker type editor based on the Dt Editor
**               widget.
**
**  Description: Contains the callbacks invoked in response to messages.
**		 Usually, this consists of creating a new "pad" with the
**		 specified file in it.
**  -----------
**
*******************************************************************
**  (c) Copyright Hewlett-Packard Company, 1992, 1993.  All rights are
**  reserved.  Copying or other reproduction of this program
**  except for archival purposes is prohibited without prior
**  written consent of Hewlett-Packard Company.
********************************************************************
**
********************************************************************
**  (c) Copyright 1993, 1994 Hewlett-Packard Company
**  (c) Copyright 1993, 1994 International Business Machines Corp.
**  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
**  (c) Copyright 1993, 1994 Novell, Inc.
********************************************************************
**
**
**************************************************************************
**********************************<+>*************************************/
#include "dtpad.h"
#include "messageParam.h"
#include <dirent.h>

extern XmStringCharSet charset;
/*
 * The following two globals (yeech) are declared in main.c
 */
extern Editor *pPadList;
extern int numActivePads;

static const char nfsdir[] = "/nfs/";


/************************************************************************
 * MakePathSendNotification - dtpad server side routine (called in
 *	HandleTtRequest) that verifies access on the server host to the
 *	directory containing the file to be edited.
 *
 *	Specifically:
 *
 *	o sends notification of either True/False depending whether the
 *	  directory can/can't be opened and,
 *	o if the directory can be opened, returns the path to the file
 *	  relative to the local host.
 *
 ************************************************************************/
static char *
MakePath(
	char *netFile
)
{
    char *localPath;

    /* -----> convert the netfile to a local path name */
    localPath = tt_netfile_file(netFile);

    /* -----> determine local directory and make sure it can be opened */
    if (tt_ptr_error(localPath) != TT_OK) {
	tt_free(localPath);
	localPath = (char *) NULL;
    } else {
	char *pathEnd;
	if (MbStrrchr(localPath, '/') != (char *)NULL) {
	    char *dir; DIR *pDir;
	    dir = (char *) XtMalloc(strlen(localPath) + 1);
	    strcpy(dir, localPath);
	    pathEnd = MbStrrchr(dir, '/');
	    if (pathEnd == dir) /* same as bug 1240376 fix in ttMsgSupport.c */
		strcpy(dir,"/");
	    else 
		*pathEnd = (char)'\0';
	    if ((pDir = opendir(dir)) != (DIR *)NULL) {
		closedir(pDir);
	    } else {
		tt_free(localPath);
		localPath = (char *) NULL;
	    }
	    XtFree(dir);
	}
    }

    /* -----> return the local path to the file */
    return localPath;

}


/************************************************************************
 * RequestCB - message callback for (op) DtPadSession_Run message
 ************************************************************************/
/* ARGSUSED */
Tt_callback_action
RequestCB(Tt_message m, Tt_pattern p)
{
  Tt_state state = tt_message_state(m);
  StartupStruct *pStruct = (StartupStruct *)tt_message_user(m, 0);

  if (state == TT_HANDLED) {
    exit(0);
  }
  else if (state == TT_FAILED) {
    if (pStruct->pPad->xrdb.closeImmediate == True)
        exit(1);
    pStruct->pPad->inUse = True;
    pStruct->pPad->xrdb.standAlone = True;
    InitDtEditSession(pStruct->pPad, pStruct->argc, pStruct->argv);
  }
  return TT_CALLBACK_PROCESSED;
}


/************************************************************************
 * HandleTtRequest - server side callback to handle (op) "DtPadSession_Run"
 *	requests from a requestor dtpad to restore a session.
 ************************************************************************/
/* ARGSUSED */
Tt_callback_action
HandleTtRequest(Tt_message m, Tt_pattern p)
{
  Tt_status status;
  Editor *pPad;
  Boolean foundPad;
  char *netFile, *localPath;


  netFile = (char *)tt_message_file(m);
  localPath = MakePath(netFile);
  if (localPath == 0) {
      tttk_message_fail(m,TT_DESKTOP_ENOENT, 0, 1);
      return TT_CALLBACK_PROCESSED;
  }

  foundPad = FindOrCreatePad(&pPad);
  pPad->xrdb.session = localPath;

  if (foundPad == False)
      RealizeNewPad(pPad);
  else
      ManageOldPad(pPad, False);
  tt_message_reply(m);
  tt_message_destroy(m);
  return TT_CALLBACK_PROCESSED;
}
