/* $XConsortium: SmScreen.c /main/cde1_maint/2 1995/08/30 16:33:22 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:        SmScreen.c
 **
 **  Project:     DT Session Manager (dtsession)
 **
 **  Description:
 **  -----------
 **  This file contains all routines needed to manage external
 **  screen savers.
 **
 *****************************************************************************
 *************************************<+>*************************************/

#include <stdio.h>
#include <signal.h>
#include <X11/Intrinsic.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <Dt/Wsm.h>
#include <Dt/UserMsg.h>
#include <Dt/SaverP.h>
#include "Sm.h"
#include "SmUI.h"                  /* smDD.* */
#include "SmError.h"
#include "SmWindow.h"
#include "SmProtocol.h"
#include "SmGlobals.h"
#include "SmScreen.h"

/*
 * Structures visible to this module only.
 */
typedef struct {
  int count;
  char *saver[1];   
  /* variable length saver[] array */
  /* saver command strings */
} SmSaverParseStruct;

/*
 * Variables global to this module only
 */
static int savernum;             /* current screen saver number */
static void *saverstate = NULL;  /* current running screen saver state */
static int firsttime = 1;        /* first call to StartScreenSaver */

/*
 * Local Function declarations
 */
#ifdef _NO_PROTO
static void ParseSaverList();
#else
static void ParseSaverList(char *, int *, int *, SmSaverParseStruct *);
#endif /* _NO_PROTO */



/*************************************<->*************************************
 *
 *  StartScreenSaver ()
 *
 *
 *  Description:
 *  -----------
 *  Start an external screen saver.
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
StartScreenSaver()
#else
StartScreenSaver( void )
#endif /* _NO_PROTO */
{
  int i;
  SmSaverParseStruct *parse;

  if (!smGD.saverListParse)
  {
   /*
    * Parse the screen saver list.
    */
    smGD.saverListParse = SmSaverParseSaverList(smGD.saverList);

    if (!smGD.saverListParse)
    {
      return;
    }
    savernum = -1;
  }

  parse = (SmSaverParseStruct *)smGD.saverListParse;

  if (parse->count == 0)
  {
    return;
  }

 /*
  * Decide which saver number to use.
  */
  savernum = (savernum + 1) % parse->count;

  if (firsttime)
  {
   /*
    * Load actions database.
    */
    ProcessReloadActionsDatabase();
    firsttime = 0;
  }

 /*
  * Start screen saver. _DtSaverStop() must be called to terminate the
  * screen saver.
  */
  saverstate = _DtSaverStart(smGD.display, smDD.coverDrawing,
                  smGD.numSavedScreens, parse->saver[savernum],
                  smGD.topLevelWid);

}


/*************************************<->*************************************
 *
 *  StopScreenSaver ()
 *
 *
 *  Description:
 *  -----------
 *  Stop an external screen saver.
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
StopScreenSaver()
#else
StopScreenSaver( void )
#endif /* _NO_PROTO */
{
  if (saverstate)
  {
   /*
    * Terminate screen saver.
    */
    _DtSaverStop(smGD.display, saverstate);
    saverstate = NULL;
  }
}

/*************************************<->*************************************
 *
 *  SmSaverParseSaverList()
 *
 *
 *  Description:
 *  -----------
 *  Parse screen saver list into allocated buffer.
 *
 *  SaverLine = {SaverSpec|WhiteSpace}
 *  SaverSpec = WhiteSpace Command WhiteSpace
 *  Command = <valid action name>
 *  WhiteSpace = {<space>|<horizontal tab>|<line feed>}
 *
 *  For example, a saverList resource might be specified as:
 *    *saverList:   \n \
 *       StartDtscreenSwarm      \n\
 *       StartDtscreenQix        \n\
 *       StartDtscreenLife
 *
 *  And be represented in memory as:
 *    "StartDtscreenSwarm   \n StartDtscreenQix\n  StartDtscreenLife"
 *
 *
 *  Inputs:
 *  ------
 *  saverList - pointer to screen saver list. This memory is not changed.
 *
 *  Outputs:
 *  -------
 *  none
 *
 *  Return:
 *  -------
 *  pointer to allocated memory containing parsed saver list
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/

void *
#ifdef _NO_PROTO
SmSaverParseSaverList(saverList)
  char *saverList;
#else
SmSaverParseSaverList(
  char *saverList)
#endif /* _NO_PROTO */
{
   char tokenSep[] = " \n\t";
   char * token;
   int i = 0;
   char * tmpStr;
   int len = strlen(saverList);
   int bytes = sizeof(int);
   char *p;
   SmSaverParseStruct *pstruct;

   tmpStr = (char *)XtMalloc(len + 1);
   memcpy(tmpStr, saverList, len+1);
   token = strtok(tmpStr, tokenSep);
   while(token != NULL)
   {
     i++;
     bytes += sizeof(char *) + strlen(token) + 1;
     token = strtok(NULL, tokenSep);
   }

   pstruct = (SmSaverParseStruct *)XtMalloc(bytes);

   if (pstruct)
   {
     memcpy(tmpStr, saverList, len+1);
     token = strtok(tmpStr, tokenSep);
     pstruct->count = 0;
     p = (char *)(pstruct->saver + i);

     while(token != NULL)
     {
       pstruct->saver[pstruct->count] = p;
       strcpy(pstruct->saver[pstruct->count], token);
       p += strlen(token) + 1;
       token = strtok(NULL, tokenSep);
       pstruct->count++;
     }
   }
   XtFree ((char *) tmpStr);
   return((void *)pstruct);
}
