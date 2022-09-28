/* $XConsortium: SmGlobals.c /main/cde1_maint/6 1995/11/27 10:08:11 barstow $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:        SmGlobals.c
 **
 **  Project:     HP DT Session Manager (dtsession)
 **
 **  Description:
 **  -----------
 **  This file contains all routines in charge of managing all global
 **  variables used by the session manager.  These variables include
 **  mostly state and setting information.
 **
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
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>
#ifdef __apollo
#include <X11/apollosys.h>      /* for pid_t, in hp-ux sys/types.h */
#endif
#include <sys/stat.h>
#include <sys/param.h>
#include <X11/Intrinsic.h>
#include <X11/Xutil.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Dt/DtP.h>
#include <Dt/SessionP.h>
#include <Dt/Wsm.h>
#include <Dt/WsmP.h>
#include <Dt/UserMsg.h>
#include <Dt/HourGlass.h>
#include <Dt/SessionM.h>
#include <Dt/EnvControl.h>
#include <Dt/Qualify.h>
#include "Sm.h"
#include "SmResource.h"
#include "SmError.h"
#include "SmUI.h"
#include "SmGlobals.h"
#include "SmLock.h"
#include "SmRestore.h"
#include "SmProtocol.h"
#ifdef _AIX_PRE_CDE1_0_SUPPORT
#include "SmMigResources.h"
#endif /* _AIX_PRE_CDE1_0_SUPPORT */

/*
 * Internal Function Declaraions
 */
#ifdef _NO_PROTO

static int SetSysDefaults() ;
static int SetResSet() ;
static void RemoveFiles();
static void TrimErrorlog();
static void _SmWaitClientTimeoutDefault ();
static void _SmWaitWmTimeoutDefault ();
void SmCvtStringToContManagement ();
unsigned char *_DtNextToken ();
Boolean _DtWmStringsAreEqual ();

#else

static int SetSysDefaults( void ) ;
static int SetResSet( void ) ;
static void RemoveFiles( char *) ;
static void TrimErrorlog(void);
static void _SmWaitClientTimeoutDefault (Widget widget, 
                                      int offset, XrmValue *value);
static void _SmWaitWmTimeoutDefault (Widget widget, 
                                      int offset, XrmValue *value);
void SmCvtStringToContManagement (XrmValue *args, Cardinal numArgs,
                              XrmValue *fromVal, XrmValue *toVal);
unsigned char *_DtNextToken (unsigned char *pchIn, int *pLen, 
                          unsigned char **ppchNext);
Boolean _DtWmStringsAreEqual (unsigned char *pch1, unsigned char *pch2, 
                         int len);

#endif /*_NO_PROTO*/

/*
 * Global Data
 */

/*
 * These are the global structures used throughout dtsession
 * They are defined in Sm.h
 */
SessionResources     smRes;
SaverResources       smSaverRes;
SettingsSet          smToSet;
SettingsCust         smCust;
SessionSettings      smSettings;
GeneralData          smGD;
char SM_SCREEN_SAVER_LOC [MAXPATHLEN + 1];

/*
 * Internal Global Data
 */
static char     tmpDisplayName[MAXPATHLEN + 1];
int machineType = 0;

static XtResource sessionResources[]=
{
   {SmNwmStartup, SmCwmStartup, XtRString, sizeof(String),
        XtOffset(SessionResourcesPtr, wmStartup),
        XtRImmediate, (XtPointer) NULL},
   {SmNquerySettings, SmCquerySettings, XtRBoolean, sizeof(Boolean),
        XtOffset(SessionResourcesPtr, querySettings),
        XtRImmediate, (XtPointer) False},
   {SmNkeys, SmCkeys, XtRString, sizeof(String),
        XtOffset(SessionResourcesPtr, keyholders), XtRString, NULL},
   {SmNalarmTime, SmCalarmTime, XtRInt, sizeof(int),
        XtOffset(SessionResourcesPtr, alarmTime), 
        XtRImmediate, (XtPointer) 20},
   {SmNmemThreshold, SmCmemThreshold, XtRInt, sizeof(int),
        XtOffset(SessionResourcesPtr, memThreshold), 
        XtRImmediate, (XtPointer) 100},
   {SmNsessionVersion, SmCsessionVersion, XtRString, sizeof(String),
        XtOffset(SessionResourcesPtr, sessionVersion),
        XtRImmediate, (XtPointer) NULL},
   {SmNdisplayResolution, SmCdisplayResolution, XtRInt, sizeof(int),
        XtOffset(SessionResourcesPtr, displayResolution),
        XtRImmediate, (XtPointer) 0},
   {SmNsessionLang, SmCsessionLang, XtRString, sizeof(String),
        XtOffset(SessionResourcesPtr, sessionLang),
        XtRImmediate, (XtPointer) ""},
   {SmNcontManagement, SmCContManagement, SmRContManagement, sizeof(long),
        XtOffset(SessionResourcesPtr, contManagement),
        XtRImmediate, (XtPointer) (SM_CM_DEFAULT)},
   {SmNwaitClientTimeout, SmCWaitClientTimeout, XtRInt, sizeof(int),
        XtOffset(SessionResourcesPtr, waitClientTimeout),
        XtRCallProc, (XtPointer)_SmWaitClientTimeoutDefault },
   {SmNwaitWmTimeout, SmCWaitWmTimeout, XtRInt, sizeof(int),
        XtOffset(SessionResourcesPtr, waitWmTimeout),
        XtRCallProc, (XtPointer)_SmWaitWmTimeoutDefault },
   {SmNuseMessaging, SmCUseMessaging, XtRBoolean, sizeof(Boolean),
        XtOffset(SessionResourcesPtr, useBMS),
        XtRImmediate, (XtPointer) True},
   {SmNsaveFontPath, SmCsaveFontPath, XtRBoolean, sizeof(Boolean),
        XtOffset(SessionResourcesPtr, saveFontPath),
        XtRImmediate, (XtPointer) False},
   {SmNsaveYourselfTimeout, SmCsaveYourselfTimeout, XtRInt, sizeof(int),
        XtOffset(SessionResourcesPtr, saveYourselfTimeout),
        XtRImmediate, (XtPointer) 60},
}
;


static XtResource saverResources[]=
{
   {SmNcycleTimeout, SmCcycleTimeout, XtRInt, sizeof(int),
        XtOffset(SaverResourcesPtr, cycleTimeout),
        XtRImmediate, (XtPointer) 3},
   {SmNlockTimeout, SmClockTimeout, XtRInt, sizeof(int),
        XtOffset(SaverResourcesPtr, lockTimeout),
        XtRImmediate, (XtPointer) 30},
   {SmNsaverTimeout, SmCsaverTimeout, XtRInt, sizeof(int),
        XtOffset(SaverResourcesPtr, saverTimeout),
        XtRImmediate, (XtPointer) 15},
   {SmNrandom, SmCrandom, XtRBoolean, sizeof(Boolean),
        XtOffset(SaverResourcesPtr, random),
        XtRImmediate, (XtPointer) False},
   {SmNsaverList, SmCsaverList, XtRString, sizeof(String),
        XtOffset(SaverResourcesPtr, saverList),
        XtRImmediate, (XtPointer) ""},

}
;



/*
 * Machine specific defaults.
 */
 static struct 
 {
   int machineType;
   int clientTimeout;
   int wmTimeout;
 }
 machineDefault[] =
 {
   {  0,  20,  60},               /* Machine independent default */
#ifdef __hpux
   {300,  10,  60},               /* HP s300                     */
   {400,  10,  60},               /* HP s400                     */
   {600,   5,  60},               /* HP s600                     */
   {700,   5,  60},               /* HP s700                     */
   {800,   5,  60},               /* HP s800                     */
#endif /* __hpux */
 };
 #define MACHINEDEFAULTS (sizeof(machineDefault) / sizeof(machineDefault[0]))

#ifdef _AIX_PRE_CDE1_0_SUPPORT
static char *                    resources_temp_file = NULL;
static char *                    session_temp_file = NULL;
#define SESSION1 "dtsmcmd -cmd \"/usr/dt/bin/dtfile \
-dir ~ -geometry +700+0\"\n"
#define SESSION2 "dtsmcmd -cmd \"/usr/dt/bin/dthelpview \
-helpVolume Welcome\"\n"
#define MARKER_FILE_FOR_CDE10 ".dtcde1.0"

/*************************************<->*************************************
 *
 *
 *  CheckForPreCDE10(char * resourcepath, char * clientpath)
 *
 *
 *  Description:
 *  -----------
 *  This function determines whether the specified resource directory
 *  was written by a CDE 1.0 desktop
 *  If was not, then the resource path and client path are
 *  set to point to temporary files containing a subset of resources
 *  or clients.
 *
 *  Inputs:
 *  ------
 *  resourcepath  = pointer to buffer containing path to sessions directory
 *                  from which dt.session and dt.resources will be read.
 *                  It is also the buffer that will be updated with
 *                  path to temporary file if the path on input
 *                  points to a directory not written by a CDE 1.0 desktop.
 *
 *  clientpath   =  pointer to buffer that will be updated with
 *                  path to temporary file if the resourcepath on input
 *                  points to a directory not written by a CDE 1.0 desktop.
 *
 *  Outputs:
 *  -------
 *  return code =
 *          -1 (severe occured such as malloc or unable to
 *                    create temporary file)
 *           1  (specified directory was written by a CDE 1.0 desktop)
 *           0  (specified directory was not written a CDE 1.0 desktop)
 *  if return code = 0, then
 *        resourcepath has been set to temporary file.
 *        clientpath has been set to temporary file.
 *
 *************************************<->***********************************/
int CheckForPreCDE10(char * resourcepath, char * clientpath)
{
   int                       rc = 0;
   FILE *                    out_fh;
   struct stat               stat_buf;
   char *                    marker_file;
   char *                    errormsg;
   int chars_written;
   int chars_to_be_written;
   char *                    getmsg;
   /* retrieve the message that will be used if the temporary files
      can not be created.
   */
   getmsg =  GETMESSAGE(4, 11,
             "Unable to create the temporary file: %s. Make sure write permissions exist for this file. Exiting from Session Manager.") ;

   /* allocate space for the error message with insert for name of
      temporary file
   */
   errormsg = malloc(strlen(getmsg) + L_tmpnam + strlen("dt.resources.")
              + 1);

   /* allocate space for the file that is used to indicate whether
      sessions directory was written by the CDE 1.0 desktop
   */
   marker_file = malloc(strlen(clientpath) + strlen(MARKER_FILE_FOR_CDE10) +
                        2);

   session_temp_file    = malloc(L_tmpnam + 1);
   resources_temp_file = malloc(L_tmpnam + 1);

   /* if space was successfully allocated, then ... */
   if ((marker_file != NULL) && (errormsg != NULL) &&
       (session_temp_file != NULL) && (resources_temp_file != NULL))
   {

       /* check if file that is used to indicate whether
          sessions directory was written by the CDE 1.0 desktop
          is found in sessions directory.
       */
       strcpy(marker_file,resourcepath);
       strcat(marker_file,"/");
       strcat(marker_file,MARKER_FILE_FOR_CDE10);
       rc = stat(marker_file, &stat_buf);
       free(marker_file);

       /* if file was found, then exit with return code indicating
          the files in directory were written by CDE 1.0 desktop
       */
       if (rc == 0)
       {
           rc = 1;
       }
       else
       {
           /* create temporary version of dt.session file */
           session_temp_file = tmpnam(session_temp_file);
           out_fh = fopen(session_temp_file,"w");
           if (out_fh != NULL)
           {
               /* write commands to start clients */
               /* if write fails for either command, then
                  put error message in errorlog and exit with
                  unsuccessful return code.
               */
               chars_written = fputs(SESSION1,out_fh);
               if (chars_written == strlen(SESSION1))
               {

                  chars_written = fputs(SESSION2,out_fh);
                  if (chars_written != strlen(SESSION2))
                  {
                      sprintf(errormsg,getmsg,session_temp_file);
                      PrintError(DtError, errormsg);
                      rc = -1;
                  }
                  else
                  {
                      /* update pointer to active session file to
                         point to temporary file
                      */
                      strcpy(clientpath,session_temp_file);
                      rc = 0;
                  }
               }
               else
               {
                  sprintf(errormsg,getmsg,session_temp_file);
                  PrintError(DtError, errormsg);
                  rc = -1;
               }
               fclose(out_fh);
           }
           else
           {
                rc = -1;
                sprintf(errormsg,getmsg,session_temp_file);
                PrintError(DtError, errormsg);
           }
           if (rc == 0)
           {
               /* remove any resources from the session
                  resource file that were not written
                  by desktop on 4.1.1 and 4.1.2
               */
               resources_temp_file = tmpnam(resources_temp_file);
               strcat(resourcepath, "/");
               strcat(resourcepath, SM_RESOURCE_FILE);
               rc = MigrateResources(resourcepath,resources_temp_file);
               if (rc == 0)
               {
                   /* update pointer to active session file to
                      point to temporary file
                   */
                   strcpy(resourcepath,resources_temp_file);
               }
               else
               {
                   sprintf(errormsg,getmsg,resources_temp_file);
                   PrintError(DtError, errormsg);
                   rc = -1;
               }
           }
       }
   }
   else
   {
       PrintError(DtError, smNLS.cantMallocErrorString);
       rc = -1;
   }

   if (errormsg != NULL)
   {
       free(errormsg);
   }
   if (marker_file != NULL)
   {
       free(marker_file);
   }
   return(rc);
}
/*************************************<->*************************************
 *
 *
 *  SetForCDE10(char * savepath)
 *
 *
 *  Description:
 *  -----------
 *  This function writes a marker file to the specified directory to
 *  indicate that the directory was written by a CDE 1.0 desktop

 *  Inputs:
 *  ------
 *  savepath = path to sessions directory being updated
 *
 *  Outputs:
 *  -------
 *  return code =
 *           1  (severe occured such as malloc or unable to write file )
 *           0  (successful)
 *
 *************************************<->***********************************/
int SetForCDE10(char * savepath)
{
   int                       rc = 0;
   FILE *                    out_fh;
   char *                    marker_file;

   marker_file = malloc(strlen(savepath) + strlen(MARKER_FILE_FOR_CDE10) +
                        2);
   if (marker_file != NULL)
   {

       strcpy(marker_file,savepath);
       strcat(marker_file,"/");
       strcat(marker_file,MARKER_FILE_FOR_CDE10);

       out_fh = fopen(marker_file,"w");
       if (out_fh != NULL)
       {
           fclose(out_fh);
           rc = 0;
       }
       else
       {
           PrintErrnoError(DtError, smNLS.cantOpenFileString);
           rc = 1;
       }
       free(marker_file);

   }
   else
   {
       PrintError(DtError, smNLS.cantMallocErrorString);
       rc = 1;
   }
   return(rc);
}
#endif /* _AIX_PRE_CDE1_0_SUPPORT */


/*************************************<->*************************************
 *
 *  _SmWaitClientTimeoutDefault (widget, offset, value)
 *
 *
 *  Description:
 *  -----------
 *  This function generates a default value for the waitClientTimeout resource.
 *  We dynamically default to 10 seconds for s400/s300 and to
 *  5 seconds for s700/s800.
 *
 *  Inputs:
 *  ------
 *  widget = this is not used
 *
 *  offset = this is the resource offset
 *
 *  value = this is a pointer to a XrmValue in which to store the result
 *
 *  Outputs:
 *  -------
 *  value = default resource value and size
 *
 *************************************<->***********************************/
static
void
#ifdef _NO_PROTO
_SmWaitClientTimeoutDefault (widget, offset, value)
        Widget   widget;
        int      offset;
        XrmValue *value;

#else /* _NO_PROTO */
_SmWaitClientTimeoutDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    int i;

    for (i = 0; i < MACHINEDEFAULTS; i++)
    {
      if (machineDefault[i].machineType == machineType)
      {
        break;
      }
    }

    if (i == MACHINEDEFAULTS)
    {
      i = 0;
    }

    value->addr = (char *)&machineDefault[i].clientTimeout;
    value->size = sizeof (int);

} /* END OF FUNCTION _SmWaitClientTimeoutDefault */



/*************************************<->*************************************
 *
 *  _SmWaitWmTimeoutDefault (widget, offset, value)
 *
 *
 *  Description:
 *  -----------
 *  This function generates a default value for the waitWmTimeout resource.
 *  We dynamically default to 60 seconds for s400/s300 and to
 *  60 seconds for s700/s800.  This could change if we get feedback indicating
 *  the need for a new default.
 *
 *  Inputs:
 *  ------
 *  widget = this is not used
 *
 *  offset = this is the resource offset
 *
 *  value = this is a pointer to a XrmValue in which to store the result
 *
 *  Outputs:
 *  -------
 *  value = default resource value and size
 *
 *************************************<->***********************************/
static
void
#ifdef _NO_PROTO
_SmWaitWmTimeoutDefault (widget, offset, value)
        Widget   widget;
        int      offset;
        XrmValue *value;

#else /* _NO_PROTO */
_SmWaitWmTimeoutDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    int i;
    
    smGD.userSetWaitWmTimeout = False; /* if we are here, it is not user set */

    for (i = 0; i < MACHINEDEFAULTS; i++)
    {
      if (machineDefault[i].machineType == machineType)
      {
        break;
      }
    }

    if (i == MACHINEDEFAULTS)
    {
      i = 0;
    }

    value->addr = (char *)&machineDefault[i].wmTimeout;
    value->size = sizeof (int);

} /* END OF FUNCTION _SmWaitWmTimeoutDefault */



/*************************************<->*************************************
 *
 *  InitSMGlobals ()
 *
 *
 *  Description:
 *  -----------
 *  Sets SM global resources and global settings to a starting value.
 *
 *
 *  Inputs:
 *  ------
 *  buttonForm = form widget for button that allows cursor to get colors
 *  smRes(global) = structure that holds session resources.
 *  smToSet(global) = structure that holds "which settings to set and how" info
 *  smGD(global) = structure that holds general data info
 * 
 *  Outputs:
 *  -------
 *  smRes(global) = structure that holds session resources.
 *  smToSet(global) = structure that holds "which settings to set and how" info
 *  smGD(global) = structure that holds general data info
 *
 *  Comments:
 *  --------
 *  Resources are set to an initial value by the resource manager.  The
 *  rest are set in the routine.
 * 
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
InitSMGlobals()
#else
InitSMGlobals( void )
#endif /* _NO_PROTO */
{
    int i;
    PropDtSmWindowInfo property;
    struct utsname nameRec;
    char *firstSlash;
    char *keyNum;

    smCust.screenSavChange = False;
    smCust.audioChange = False;
    smCust.keyboardChange = False;
    smCust.fontChange = False;
    smCust.pointerChange = False;
    smCust.dClickChange = False;
    
    smGD.bmsDead = False;
    smGD.userSetWaitWmTimeout = True; /* assume it is */

#ifdef __hpux
    if (uname(&nameRec) == 0)
    {
        keyNum = nameRec.machine;
        if (firstSlash = strchr(keyNum, '/'))
        {
            keyNum = ++firstSlash;

            if (     keyNum[0] == '3') 
            {
                machineType = 300;
            }
            else if (keyNum[0] == '4') 
            {
                machineType = 400;
            }
            else if (keyNum[0] == '6') 
            {
                machineType = 600;
            }
            else if (keyNum[0] == '7') 
            {
                machineType = 700;
            }
            else if (keyNum[0] == '8') 
            {
                machineType = 800;
            }
        }
    }
#endif /* __hpux */

    /*
     * Get application specific resource values
     */
    XtAppAddConverter (smGD.appCon, XtRString, SmRContManagement,
                       (XtConverter)SmCvtStringToContManagement, NULL, 0);
    XtGetApplicationResources(smGD.topLevelWid, (XtPointer) &smRes,
                              sessionResources,
                              XtNumber(sessionResources), NULL, 0);

    if (smGD.lockOnTimeoutStatus == True)
    {
     /*
      * Pull screen saver resources from Dtsession*extension.<name>.
      */
      smGD.SmNextension = "extension";
      smGD.SmCextension = "Extension";
      smGD.extensionSpec = "extension.";
    }
    else
    {
     /*
      * Pull screen saver resources from Dtsession*<name>.
      */
      smGD.SmNextension = smGD.SmNextension = smGD.extensionSpec = "";
    }

    XtGetSubresources(smGD.topLevelWid, (XtPointer) &smSaverRes,
                      smGD.SmNextension, smGD.SmCextension,
                      saverResources,
                      XtNumber(saverResources), NULL, 0);

    smGD.wmStartup = SmNewString(smRes.wmStartup);
    smGD.keyholders = SmNewString(smRes.keyholders);
    smGD.sessionVersion = SmNewString(smRes.sessionVersion);
    smGD.sessionLang = SmNewString(smRes.sessionLang);
    smGD.saverList = SmNewString(smSaverRes.saverList);

    /*
     * Initialize general data used by apps not initialized by
     * XtInitialize or DtInitialize
     */
    smGD.topLevelWindow = XtWindow(smGD.topLevelWid);
    smGD.numSavedScreens = (ScreenCount(smGD.display) > MAX_SCREENS_SAVED)
         ? MAX_SCREENS_SAVED : ScreenCount(smGD.display);
    smGD.dtwmRunning = False;
    smSettings.confirmMode = DtSM_VERBOSE_MODE;
    smSettings.startState = DtSM_CURRENT_STATE;
    smGD.saverListParse = NULL;

    /*
     * Sanity check on timeouts for negative numbers
     */
    if (smRes.waitClientTimeout < 0)
    {
        smRes.waitClientTimeout = -smRes.waitClientTimeout;
    }
    if (smRes.waitWmTimeout < 0)
    {
        smRes.waitWmTimeout = -smRes.waitWmTimeout;
    }
    if (smRes.saveYourselfTimeout < 0)
    {
        smRes.saveYourselfTimeout = -smRes.saveYourselfTimeout;
    }


    /*
     * Now convert users view of seconds in to XtAppAddTimout's
     * need for milliseconds.
     */
    smRes.waitClientTimeout = 1000 * smRes.waitClientTimeout;
    smRes.waitWmTimeout = 1000 * smRes.waitWmTimeout;
    smRes.saveYourselfTimeout = 1000 * smRes.saveYourselfTimeout;
    smGD.savedWaitWmTimeout = smRes.waitWmTimeout;

    /*
     * Initialize lock data
     */
    smGD.screen = XDefaultScreen(smGD.display);
    smGD.blackPixel = XBlackPixel(smGD.display, smGD.screen);
    smGD.whitePixel = XWhitePixel(smGD.display, smGD.screen);
    smDD.lockDialog = NULL;
    smDD.lockCoverDialog = NULL;
    for(i = 0;i < smGD.numSavedScreens;i++)
    {
        smDD.coverDialog[i] = NULL;
        smDD.coverDrawing[i] = NULL;
    }
    smGD.lockedState = UNLOCKED;

   /*
    * Sanity check screen saver resource values.
    */ 
    if (smRes.alarmTime < 0) smRes.alarmTime = 0;

    #define SMBOUND(A) (A < 0 ? 0 : A)

    smSaverRes.lockTimeout = SMBOUND(smSaverRes.lockTimeout) * 60;
    smSaverRes.saverTimeout = SMBOUND(smSaverRes.saverTimeout) * 60;
    smSaverRes.cycleTimeout = SMBOUND(smSaverRes.cycleTimeout) * 60;


    CreateLockCursor();
    smGD.waitCursor = _DtGetHourGlassCursor(smGD.display);

    /*
     * Initialize other global data related to dialogs
     */
    smDD.confExit = NULL;
    smDD.qExit = NULL;
    smDD.compatExit = NULL;
    smDD.deadWid = NULL;
        smDD.smHelpDialog = NULL;

    smDD.cancelString = XmStringCreateLtoR(_DtCancelString,
                                           XmFONTLIST_DEFAULT_TAG);
    smDD.okString = XmStringCreateLtoR(_DtOkString,
                                       XmFONTLIST_DEFAULT_TAG);
    smDD.helpString = XmStringCreateLtoR(_DtHelpString,
                                           XmFONTLIST_DEFAULT_TAG);
    smDD.okLogoutString = XmStringCreateLtoR(((char *)GETMESSAGE(18, 39,
                                             "OK")),
                                             XmFONTLIST_DEFAULT_TAG);
    smDD.cancelLogoutString = XmStringCreateLtoR(((char *)GETMESSAGE(18,40,
                                                 "Cancel")),
                                                 XmFONTLIST_DEFAULT_TAG);

    /*
     * Intern all the atoms needed for the WSM communication
     */
    XaVsmInfo = XInternAtom (smGD.display, _XA_DT_SM_WINDOW_INFO, False);
    XaSmWmProtocol =
        XInternAtom(smGD.display, _XA_DT_SM_WM_PROTOCOL, False);
    XaSmStartAckWindow =
        XInternAtom(smGD.display, _XA_DT_SM_START_ACK_WINDOWS, False);
    XaSmStopAckWindow =
        XInternAtom(smGD.display, _XA_DT_SM_STOP_ACK_WINDOWS, False);
    XaWmWindowAck =
        XInternAtom(smGD.display, _XA_DT_WM_WINDOW_ACK, False);
    XaWmExitSession =
        XInternAtom(smGD.display, _XA_DT_WM_EXIT_SESSION, False);
    XaWmLockDisplay =
        XInternAtom(smGD.display, _XA_DT_WM_LOCK_DISPLAY, False);
    XaWmReady =
        XInternAtom(smGD.display, _XA_DT_WM_READY, False);

    /*
     * Set the session manager window property on the root window
     */
    property.flags = 0;
    property.smWindow = (unsigned long) smGD.topLevelWindow;
    XChangeProperty (smGD.display, RootWindow(smGD.display, 0),
                     XaVsmInfo, XaVsmInfo,
                     32, PropModeReplace, (unsigned char *)&property,
                     PROP_DT_SM_WINDOW_INFO_ELEMENTS);

    /*
     * Set up the signal handler for forking and execing
     */
    sigaction(SIGCHLD, &smGD.childvec, (struct sigaction *) NULL);
}



/*************************************<->*************************************
 *
 *  SetRestorePath ()
 *
 *
 *  Description:
 *  -----------
 *  Sets SM global resources and global settings to a starting value.
 *
 *
 *  Inputs:
 *  ------
 *  smGD.display = display structure for session manager.  Used to construct
 *                     a display directory
 * 
 *  Outputs:
 *  -------
 *  smGD.resourcePath(global) = Path where the resources to be restored are
 *                              held
 *  smGD.settingPath(global) = Path where the settings to be restored are
 *                              held
 *  smGD.clientPath(global) = Path where the clients to be restored are
 *                              held
 *  smGD.savePath(global) = Path where all save files are to be saved
 *
 *  Return:
 *  ------
 *  Display connection
 *
 *  Comments:
 *  --------
 *  WARNING:  This routine also determines whether dtsession is being started
 *            in compatibility mode.  If so - no restore paths are set up and
 *            the routine is exited.
 * 
 *************************************<->***********************************/
int
#ifdef _NO_PROTO
SetRestorePath( argc, argv )
        unsigned int argc ;
        char *argv[] ;
#else
SetRestorePath(
        unsigned int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
    Display  *tmpDisplay;
#ifdef _AIX_PRE_CDE1_0_SUPPORT
    int rc = 0;
#endif /* _AIX_PRE_CDE1_0_SUPPORT */
    struct stat buf;
    int status, i;
    char        *displayName = NULL;
    int session_needed = 1;

    smGD.compatMode = False;

    for(i = 0;i < argc;i++)
    {
        if(!strcmp(argv[i], "-display"))
        {
            displayName = argv[i + 1];
            /* 
             * If -display is used but DISPLAY is not set,
             * put DISPLAY into the environment
             */
            if (getenv("DISPLAY") == 0)
            {
                
                sprintf(tmpDisplayName, "DISPLAY=%s", displayName);
                putenv(tmpDisplayName);
            }
        }

        if(!strcmp(argv[i], "-norestore"))
        {
            smGD.compatMode = True;
        }
    }

    /*
     *  If we are in compatibility mode - no restore paths are set
     *  up and we just return
     */
    if(smGD.compatMode == True)
    {
        smGD.clientPath[0] = NULL;
        smGD.resourcePath[0] = NULL;
        smGD.settingPath[0] = NULL;
        smGD.sessionType = DEFAULT_SESSION;
        smGD.restoreSession = NULL;
        return(0);
    }

    tmpDisplay = XOpenDisplay(displayName);
    if(tmpDisplay == NULL)
    {
        PrintError(DtError, GETMESSAGE(4, 1, "Invalid display name - exiting."));
        SM_EXIT(-1);
    }

    /*
     * Create any of the directories that don't already exist
     */
    smGD.savePath = _DtCreateDtDirs(tmpDisplay);

    TrimErrorlog();
    XCloseDisplay(tmpDisplay);

    if (session_needed)
    {
      strcpy(smGD.clientPath, smGD.savePath);
      strcat(smGD.clientPath, "/");
      strcat(smGD.clientPath, SM_CURRENT_DIRECTORY);
      strcat(smGD.clientPath, "/");
      strcat(smGD.clientPath, SM_CLIENT_FILE);

      status = stat(smGD.clientPath, &buf);
      if(status == 0)
      {
       /*
        * sessions/current/dt.settings exist, restore to 'current' session
        */
        strcpy(smGD.resourcePath, smGD.savePath);
        strcat(smGD.resourcePath, "/");
        strcat(smGD.resourcePath, SM_CURRENT_DIRECTORY);
#ifdef _AIX_PRE_CDE1_0_SUPPORT
        /* determine if directory was written by a CDE 1.0 desktop */
        rc = CheckForPreCDE10(smGD.resourcePath,smGD.clientPath);
        /* if a fatal error occurred during the check, then exit */
        if (rc == -1)
        {
            SM_EXIT(-1);
        }
        /* If directory was written by a CDE 1.0 desktop,
           then update resourcePath to point to resource file in
           current directory.
        */
        if (rc == 1)
        {
            strcat(smGD.resourcePath, "/");
            strcat(smGD.resourcePath, SM_RESOURCE_FILE);
        }
#else
	strcat(smGD.resourcePath, "/");
	strcat(smGD.resourcePath, SM_RESOURCE_FILE);
#endif /* _AIX_PRE_CDE1_0_SUPPORT */
        strcpy(smGD.settingPath, smGD.savePath);
        strcat(smGD.settingPath, "/");
        strcat(smGD.settingPath, SM_CURRENT_DIRECTORY);
        strcat(smGD.settingPath, "/");
        strcat(smGD.settingPath, SM_SETTING_FILE);
        SetResSet();
        smGD.sessionType = CURRENT_SESSION;
        smGD.restoreSession = (char *) SM_CURRENT_DIRECTORY;
        session_needed = 0;
      }
    }

    if (session_needed)
    {
      strcpy(smGD.clientPath, smGD.savePath);
      strcat(smGD.clientPath, "/");
      strcat(smGD.clientPath, SM_HOME_DIRECTORY);
      strcat(smGD.clientPath, "/");
      strcat(smGD.clientPath, SM_CLIENT_FILE);

      status = stat(smGD.clientPath, &buf);

      if(status == 0)
      {
       /*
        * sessions/home/dt.settings exist, restore to 'home' session
        */
        strcpy(smGD.resourcePath, smGD.savePath);
        strcat(smGD.resourcePath, "/");
        strcat(smGD.resourcePath, SM_HOME_DIRECTORY);
#ifdef _AIX_PRE_CDE1_0_SUPPORT
        /* determine if directory was written by a CDE 1.0 desktop */
        rc = CheckForPreCDE10(smGD.resourcePath,smGD.clientPath);

        /* if a fatal error occurred during the check, then exit */
        if (rc == -1)
        {
            SM_EXIT(-1);
        }
        /* If directory was written by a CDE 1.0 desktop,
           then update resourcePath to point to resource file in
           current directory.
        */
        if (rc == 1)
        {
            strcat(smGD.resourcePath, "/");
            strcat(smGD.resourcePath, SM_RESOURCE_FILE);
        }
#else
	strcat(smGD.resourcePath, "/");
	strcat(smGD.resourcePath, SM_RESOURCE_FILE);
#endif /* _AIX_PRE_CDE1_0_SUPPORT */
        strcpy(smGD.settingPath, smGD.savePath);
        strcat(smGD.settingPath, "/");
        strcat(smGD.settingPath, SM_HOME_DIRECTORY);
        strcat(smGD.settingPath, "/");
        strcat(smGD.settingPath, SM_SETTING_FILE);
        SetResSet();
        smGD.sessionType = HOME_SESSION;
        smGD.restoreSession = (char *) SM_HOME_DIRECTORY;
        session_needed = 0;
      }
    }

    if (session_needed)
    {
      SetSysDefaults();
    }

    /* 
    ** User's session startup script:
    **   $HOME/.dt/sessions/sessionetc
    */
    strcpy(smGD.etcPath, smGD.savePath);
    strcat(smGD.etcPath, "/");
    strcat(smGD.etcPath, smEtcFile);

    /* 
    ** User's session shutdown script: 
    **   $HOME/.dt/sessions/sessionexit
    */
    strcpy(smGD.exitPath, smGD.savePath);
    strcat(smGD.exitPath, "/");
    strcat(smGD.exitPath, smExitFile);

    return(0);
}

static void
#ifdef _NO_PROTO
FixPath (the1stPath)
    char * the1stPath;
#else
FixPath
    (
    char * the1stPath
    )
#endif
{
    char * tempPath;
    char * pathList = (char *)XtMalloc(strlen(SM_SYSTEM_PATH) +
			 strlen(":" CDE_INSTALLATION_TOP "/config") + 1);

    strcpy(pathList,SM_SYSTEM_PATH);
    strcat(pathList,":" CDE_INSTALLATION_TOP "/config");

    tempPath = _DtQualifyWithFirst(the1stPath,pathList);
    if (tempPath != NULL) {
        strcpy(the1stPath,tempPath);
        free(tempPath);
    }

    XtFree(pathList);
}


/*************************************<->*************************************
 *
 *  SetSysDefaults ()
 *
 *
 *  Description:
 *  -----------
 *  Sets the path to restore the system default files. A convenience routine
 *
 *
 *  Inputs:
 *  ------
 *  smGD.savePath = path that files are to be saved in (set up in
 *                      SetRestorePaths)
 * 
 *  Outputs:
 *  -------
 *  smGD.resourcePath(global) = Path where the resources to be saved are
 *                              to be saved.
 *  smGD.settingPath(global) = Path where the settings to be saved are
 *                              to be saved.
 *  smGD.clientPath(global) = Path where the clients to be saved are
 *                              to be saved.
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static int 
#ifdef _NO_PROTO
SetSysDefaults()
#else
SetSysDefaults( void )
#endif /* _NO_PROTO */
{
    int status;
    struct stat buf;
    String tmpString;
    char        *langSpec;
    char        *tempPath;

    /*
     * No files exist for restoration - use the
     * system defaults
     */
    strcpy(smGD.resourcePath, "");
    strcpy(smGD.clientPath, "");
    smGD.settingPath[0] = NULL;
    smGD.sessionType = DEFAULT_SESSION;
    smGD.restoreSession = (char *) SM_SYSTEM_DIRECTORY;

    langSpec = getenv("LANG");
    if ((langSpec != NULL) && (*langSpec != 0))
    {
        strcat(smGD.clientPath, "/");
        strcat(smGD.clientPath, langSpec);
    }
    
    strcat(smGD.clientPath, "/");
    strcat(smGD.clientPath, SM_SYSTEM_CLIENT_FILE);
   
    FixPath(smGD.clientPath);

    /*
     * If the system files don't exist - we're in
     * trouble - First try LANG location then default
     */
    status = stat(smGD.clientPath, &buf);
    if(status == -1)
    {
        if((langSpec == NULL) || (*langSpec == 0))
        {
            PrintErrnoError(DtError, GETMESSAGE(4, 2,
                        "No defaults files exist.  "
                        "No applications will be restarted."));
            smGD.clientPath[0] = NULL;
            smGD.resourcePath[0] = NULL;
        }
        else
        {
            strcpy(smGD.clientPath, "/C/");
            strcat(smGD.clientPath, SM_SYSTEM_CLIENT_FILE);

	    FixPath(smGD.clientPath);

            status = stat(smGD.clientPath, &buf);
            if(status == -1)
            {
                PrintErrnoError(DtError, GETMESSAGE(4, 3,
                            "No defaults files exist.  "
                            "No applications will be restarted."));
                smGD.clientPath[0] = NULL;
                smGD.resourcePath[0] = NULL;
            }
        }
    }
    
    return(0);
}



/*************************************<->*************************************
 *
 *  SetResSet ()
 *
 *
 *  Description:
 *  -----------
 *  Sets the path to restore the settings and resource files.
 *  A convenience routine
 *
 *
 *  Inputs:
 *  ------
 *  smGD.savePath = path that files are to be saved in (set up in
 *                      SetRestorePaths)
 * 
 *  Outputs:
 *  -------
 *  smGD.resourcePath(global) = Path where the resources to be saved are
 *                              to be saved.
 *  smGD.settingPath(global) = Path where the settings to be saved are
 *                              to be saved.
 *  smGD.clientPath(global) = Path where the clients to be saved are
 *                              to be saved.
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static int 
#ifdef _NO_PROTO
SetResSet()
#else
SetResSet( void )
#endif /* _NO_PROTO */
{
    int status;
    struct stat buf;
    
    /*
     * If resource or settings file does not exist - just null out
     * the path so these things will not get restored
     */
    status = stat(smGD.resourcePath, &buf);
    if(status == -1)
    {
        smGD.resourcePath[0] = NULL;
    }

    status = stat(smGD.settingPath, &buf);
    if(status == -1)
    {
        smGD.settingPath[0] = NULL;
    }
    return(0);
}



/*************************************<->*************************************
 *
 *  SetSavePath (saveToHome, mode)
 *
 *
 *  Description:
 *  -----------
 *  Sets up paths for files that need to be saved.  Also removes any files
 *  that shouldn't be there after the save.
 *
 *
 *  Inputs:
 *  ------
 *  smGD.display = display structure for session manager.  Used to construct
 *                     a display directory
 *  saveToHome = whether this is a save to home session or not
 *  mode = whether we are resetting or restarting
 * 
 *  Outputs:
 *  -------
 *  smGD.resourcePath(global) = Path where the resources to be saved are
 *                              to be saved.
 *  smGD.settingPath(global) = Path where the settings to be saved are
 *                              to be saved.
 *  smGD.clientPath(global) = Path where the clients to be saved are
 *                              to be saved.
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
int 
#ifdef _NO_PROTO
SetSavePath( saveToHome, mode )
        Boolean saveToHome ;
        int mode ;
#else
SetSavePath(
        Boolean saveToHome,
        int mode )
#endif /* _NO_PROTO */
{
    struct stat         buf;
    int                 status;
    
    /*
     * Make sure the user hasn't done something like delete the .dt
     * directories during the session.  If so - recreate them
     */
    SM_FREE(smGD.savePath);
    smGD.savePath = _DtCreateDtDirs(smGD.display);
    if(smGD.savePath == NULL)
    {
        PrintErrnoError(DtError, smNLS.cantCreateDirsString);
        smGD.clientPath[0] = NULL;
        smGD.settingPath[0] = NULL;
        smGD.resourcePath[0] = NULL;
        return(-1);
    }
        
    
    /*
     * Path for a save defaults to save the current session.
     * Otherwise just remove the directories
     */
    strcpy(smGD.clientPath, smGD.savePath);
    strcpy(smGD.settingPath, smGD.savePath);
    strcpy(smGD.resourcePath, smGD.savePath);
    strcat(smGD.clientPath, "/");
    strcat(smGD.settingPath, "/");
    strcat(smGD.resourcePath, "/");

    if(saveToHome == False)
    {
        strcat(smGD.clientPath, SM_CURRENT_DIRECTORY);
        strcat(smGD.resourcePath, SM_CURRENT_DIRECTORY);
        strcat(smGD.settingPath, SM_CURRENT_DIRECTORY);
        smGD.sessionType = CURRENT_SESSION;
    }
    else
    {
        strcat(smGD.clientPath, SM_HOME_DIRECTORY);
        strcat(smGD.resourcePath, SM_HOME_DIRECTORY);
        strcat(smGD.settingPath, SM_HOME_DIRECTORY);
        smGD.sessionType = HOME_SESSION;
    }

#ifdef _AIX_PRE_CDE1_0_SUPPORT
    if (resources_temp_file != NULL)
    {
        remove(resources_temp_file);
        free(resources_temp_file);
        resources_temp_file = NULL;

    }
    if (session_temp_file != NULL)
    {
        remove(session_temp_file);
        free(session_temp_file);
        session_temp_file = NULL;
    }
#endif /* _AIX_PRE_CDE1_0_SUPPORT */


    if((mode == DtSM_HOME_STATE) && (saveToHome == False))
    {
        /*
         * If we are going to reset to the home session - remove any
         * current session files and dirs and set the path to NULL
         */
        status = stat(smGD.clientPath, &buf);
        if(status != -1)
        {
            sprintf(smGD.etcPath, "%s.%s", smGD.clientPath, SM_OLD_EXTENSION);
            MoveDirectory(smGD.clientPath, smGD.etcPath);
        }

        smGD.clientPath[0] = NULL;
        smGD.settingPath[0] = NULL;
        smGD.resourcePath[0] = NULL;
    }
    else
    {
        /*
         * Delete all files that currently exist in the directory
         * or create the directory if it does not exist
         */
        status = stat(smGD.clientPath, &buf);
        if(status == -1)
        {
            status = mkdir(smGD.clientPath, 0000);
            if(status == -1)
            {
                PrintErrnoError(DtError, smNLS.cantCreateDirsString);
                smGD.clientPath[0] = NULL;
                smGD.settingPath[0] = NULL;
                smGD.resourcePath[0] = NULL;
                return(-1);
            }
            chmod(smGD.clientPath, 0755);
        }
        else
        {
            sprintf(smGD.etcPath, "%s.%s", smGD.clientPath, SM_OLD_EXTENSION);
            MoveDirectory(smGD.clientPath, smGD.etcPath);

            /*
             * Now re-make the directory
             */
            status = mkdir(smGD.clientPath, 0000);
            if(status == -1)
            {
                PrintErrnoError(DtError, smNLS.cantCreateDirsString);
                smGD.clientPath[0] = NULL;
                smGD.settingPath[0] = NULL;
                smGD.resourcePath[0] = NULL;
                return(-1);
            }
            chmod(smGD.clientPath, 0755);
        }
        
#ifdef _AIX_PRE_CDE1_0_SUPPORT
        /* write file indicating that directory was updated by a CDE 1.0
           desktop
        */
        if (SetForCDE10(smGD.clientPath) != 0)
        {
            return(-1);
        }
#endif /* _AIX_PRE_CDE1_0_SUPPORT */
        strcat(smGD.clientPath, "/");
        strcat(smGD.clientPath, SM_CLIENT_FILE);
        strcat(smGD.settingPath, "/");
        strcat(smGD.settingPath, SM_SETTING_FILE);
        strcat(smGD.resourcePath, "/");
        strcat(smGD.resourcePath, SM_RESOURCE_FILE);
    }

    return(0);
}


/*************************************<->*************************************
 *
 *  SetFontSavePath (saveToHome, mode)
 *
 *
 *  Description:
 *  -----------
 *  Sets up the save path for the auxillary directory.
 *
 *
 *  Inputs:
 *  ------
 *
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
int 
#ifdef _NO_PROTO
SetFontSavePath(langPtr)
char *langPtr;
#else
SetFontSavePath(char *langPtr)
#endif /* _NO_PROTO */
{
    struct stat         buf;
    int                 status;
    char                *sessionSaved;
    

    /*
     * Set up the session font directory and see if it exists
     */
    if(smGD.sessionType == CURRENT_SESSION)
    {
        sessionSaved = SM_CURRENT_FONT_DIRECTORY;
    }
    else
    {
        sessionSaved = SM_HOME_FONT_DIRECTORY;
    }

    sprintf(smGD.fontPath, "%s/%s", smGD.savePath, sessionSaved);
    status = stat(smGD.fontPath, &buf);
    if(status == -1)
    {
        status = mkdir(smGD.fontPath, 0000);
        if(status == -1)
        {
            PrintErrnoError(DtError, smNLS.cantCreateDirsString);
            smGD.fontPath[0] = NULL;
            return(-1);
        }
        chmod(smGD.fontPath, 0755);
    }

    /*
     * Now add the lang subdirectory and see if it exists
     */
    strcat(smGD.fontPath, "/");
    strcat(smGD.fontPath, langPtr);
    status = stat(smGD.fontPath, &buf);
    if(status == -1)
    {
        status = mkdir(smGD.fontPath, 0000);
        if(status == -1)
        {
            PrintErrnoError(DtError, smNLS.cantCreateDirsString);
            smGD.fontPath[0] = NULL;
            return(-1);
        }
        chmod(smGD.fontPath, 0755);
    }

    return(0);
}

                      

/*************************************<->*************************************
 *
 *  RemoveFiles()
 *
 *
 *  Description:
 *  -----------
 *  Remove the files that need to be removed
 *
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
RemoveFiles( path )
        char *path ;
#else
RemoveFiles(
        char *path )
#endif /* _NO_PROTO */
{
    pid_t  clientFork;
    int    execStatus, childStatus, i, statLoc;
    String tmpString;

           
    /*
     * Fork and exec the client process
     */
    sigaction(SIGCHLD, &smGD.defvec, (struct sigaction *) NULL);

    clientFork = vfork();
    
    /*
     * If the fork fails - Send out an error and return
     */
    if(clientFork < 0)
    {
        PrintErrnoError(DtError, smNLS.cantForkClientString);
        return;
    }
    
    /*
     * Fork succeeded - now do the exec
     */
    if(clientFork == 0)
    {
        /*
         * Set the gid of the process back from bin
         */
#ifndef __hpux
#ifndef SVR4
        setregid(smGD.runningGID, smGD.runningGID);
#else
        setgid(smGD.runningGID);
        setegid(smGD.runningGID);
#endif
#endif

        _DtEnvControl(DT_ENV_RESTORE_PRE_DT);
        
#ifdef __osf__
        setsid();
#else
        (void) setpgrp();
#endif /* __osf__ */
         
        execStatus = execlp("rm","rm", "-rf", path, (char *) 0);
        if(execStatus != 0)
        {
            tmpString = ((char *)GETMESSAGE(4, 4, "Unable to remove session directory.  Make sure write permissions exist on $HOME/.dt directory.  Invalid session files will not be removed.")) ;
            PrintErrnoError(DtError, tmpString);
            SM_FREE(tmpString);
            SM_EXIT(-1);
        }
    }

    while(wait(&statLoc) != clientFork);
    
    sigaction(SIGCHLD, &smGD.childvec, (struct sigaction *) NULL);
}
                      

/*************************************<->*************************************
 *
 *  MoveDirectory()
 *
 *
 *  Description:
 *  -----------
 *  Move the directory specified by pathFrom - to the directory specified
 *  by pathTo.
 *
 *
 *  Inputs:
 *  ------
 *  pathTo = the directory to move to
 *  pathFrom = the directory to move from
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
MoveDirectory(pathFrom, pathTo)
char *pathFrom;
char *pathTo;
#else
MoveDirectory(char *pathFrom,
              char *pathTo)
#endif /* _NO_PROTO */
{
    struct stat buf;
    pid_t  clientFork;
    int    status, execStatus, childStatus, i, statLoc;
    String tmpString;

    /*
     * If the pathTo directory exists - remove it
     */
    status = stat(pathTo, &buf);
    if(status != -1)
    {
        RemoveFiles(pathTo);
    }
    
           
    /*
     * Fork and exec the client process
     */

    sigaction(SIGCHLD, &smGD.defvec, (struct sigaction *) NULL);

    clientFork = vfork();
    
    /*
     * If the fork fails - Send out an error and return
     */
    if(clientFork < 0)
    {
        PrintErrnoError(DtError, smNLS.cantForkClientString);
        return;
    }
    
    /*
     * Fork succeeded - now do the exec
     */
    if(clientFork == 0)
    {
        /*
         * Set the gid of the process back from bin
         */
#ifndef __hpux
#ifndef SVR4
        setregid(smGD.runningGID, smGD.runningGID);
#else
        setgid(smGD.runningGID);
        setegid(smGD.runningGID);
#endif
#endif

        _DtEnvControl(DT_ENV_RESTORE_PRE_DT);
        
#ifdef __osf__
        setsid();
#else
        (void) setpgrp();
#endif /* __osf__ */
        
        execStatus = execlp("mv","mv", pathFrom, pathTo, (char *) 0);
        if(execStatus != 0)
        {
            tmpString = ((char *)GETMESSAGE(4, 4, "Unable to remove session directory.  Make sure write permissions exist on $HOME/.dt directory.  Invalid session files will not be removed.")) ;
            PrintErrnoError(DtError, tmpString);
            SM_FREE(tmpString);
            SM_EXIT(-1);
        }
    }

    while(wait(&statLoc) != clientFork);
    
    sigaction(SIGCHLD, &smGD.childvec, (struct sigaction *) NULL);
}

                                      

/*************************************<->*************************************
 *
 *  InitNlsStrings()
 *
 *
 *  Description:
 *  -----------
 *  Initialize the NLS strings used in dtsession
 *
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
InitNlsStrings()
#else
InitNlsStrings( void )
#endif /* _NO_PROTO */
{
    char        *tmpString;

    /*
     * Malloc failure error message - THIS MESSAGE MUST BE INITIALIZED FIRST
     */
    smNLS.cantMallocErrorString = ((char *)GETMESSAGE(4, 5, "Unable to malloc memory for operation."));

    /*
     * Error message when dtsession cant lock the display
     */
    smNLS.cantLockErrorString = ((char *)GETMESSAGE(4, 6, "Unable to lock display.  Another application may have the pointer or keyboard grabbed."));

    /*
     * Error message when dtsession cant open files for reading or writing
     */
    smNLS.cantOpenFileString = ((char *)GETMESSAGE(4, 7, "Unable to open session file.  No clients will be restarted."));

    /*
     * Error message when dtsession cant fork client process
     */
    smNLS.cantForkClientString = ((char *)GETMESSAGE(4, 8, "Unable to fork client process."));

    /*
     * Error message when dtsession cant create dt directories
     */
    smNLS.cantCreateDirsString = ((char *)GETMESSAGE(4, 9, "Unable to create DT directories.  Check permissions on home directory."));

    /*
     * Error message when trying to lock display on trusted system
     */
    smNLS.trustedSystemErrorString = ((char *)GETMESSAGE(4, 10, "Unable to lock display due to security restrictions"));
}
 
                                      

/*************************************<->*************************************
 *
 *  WaitChildDeath()
 *
 *
 *  Description:
 *  -----------
 *  When a SIGCHLD signal comes in, wait for all child processes to die.
 *
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

void
#ifdef _NO_PROTO
WaitChildDeath(i)
    int i;
#else
WaitChildDeath( int i )
#endif /* _NO_PROTO */
{
  int   stat_loc;
  pid_t pid;

 /*
  * Wait on any children that have died since the last SIGCHLD we
  * received. Any child process death that occurs while we are 
  * in WaitChildDeath() will not result in a SIGCHLD. Any
  * child proceses that die after our last call to waitpid() will
  * remain zombiefied until the next invocation of WaitChildDeath().
  */
  while ((pid = waitpid(-1, &stat_loc, WNOHANG)) > 0)
  ;

  sigaction(SIGCHLD, &smGD.childvec, (struct sigaction *) NULL);
}

                                      

/*************************************<->*************************************
 *
 *  TrimErrorlog()
 *
 *
 *  Description:
 *  -----------
 *  Trim the errorlog file using the following algorithm:
 *      if(errorlog.oldest exists) delete it;
 *      if(errorlog.old exists) move it to errorlog.oldest
 *      if(errorlog exists) move it to errorlog.old
 *
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void
#ifdef _NO_PROTO
TrimErrorlog()
#else
TrimErrorlog( void )
#endif /* _NO_PROTO */
{
    char        *savePath, *checkPath1, *checkPath2, *c;
    struct stat buf;
    int status;
    char *home;

    /*
     * Allocate the strings needed
     */
    savePath = (char *) SM_MALLOC(MAXPATHLEN + 1);
    checkPath1 = (char *) SM_MALLOC(MAXPATHLEN + 1);
    checkPath2 = (char *) SM_MALLOC(MAXPATHLEN + 1);
    if ((home=getenv("HOME")) == NULL)
      home="";
    sprintf(savePath, "%s/%s", home, DtPERSONAL_CONFIG_DIRECTORY);
    
    /*
     * If errorlog.oldest exists - delete it
     */
    sprintf(checkPath1, "%s/%s", savePath, DtOLDER_ERRORLOG_FILE);
    status = stat(checkPath1, &buf);
    if(status != -1)
    {
        (void) unlink(checkPath1);
    }

    /*
     * If errorlog.old exists - move it to errorlog.oldest
     */
    sprintf(checkPath2, "%s/%s", savePath, DtOLD_ERRORLOG_FILE);
    status = stat(checkPath2, &buf);
    if(status != -1)
    {
        (void) link(checkPath2, checkPath1);
        (void) unlink(checkPath2);
    }

    /*
     * If errorlog exists = move it to errorlog.old
     */
    sprintf(checkPath1, "%s/%s", savePath, DtERRORLOG_FILE);
    status = stat(checkPath1, &buf);
    if(status != -1)
    {
        (void) link(checkPath1, checkPath2);
        (void) unlink(checkPath1);
    }

    SM_FREE((char *) savePath);
    SM_FREE((char *) checkPath1);
    SM_FREE((char *) checkPath2);

    return;
}

                                      

/*************************************<->*************************************
 *
 *  SetSystemReady()
 *
 *
 *  Description:
 *  -----------
 *  Do everything that needs to be done to get the system back into a
 *  READY state.  This includes checking to be sure that the BMS did not
 *  die while we were in process
 *
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void
#ifdef _NO_PROTO
SetSystemReady()
#else
SetSystemReady( void )
#endif /* _NO_PROTO */
{
    smGD.smState = READY;
    if(smGD.bmsDead == True)
    {
        WarnMsgFailure();
    }

    return;
}


/*************************************<->*************************************
 *
 *  SmCvtStringToContManagement (args, numArgs, fromVal, toVal)
 *
 *
 *  Description:
 *  -----------
 *  This function converts a string to a value for the 
 *  contention management flag set.
 *
 *
 *  Inputs:
 *  ------
 *  args = additional XrmValue arguments to the converter - NULL here
 *
 *  numArgs = number of XrmValue arguments - 0 here
 *
 *  fromVal = resource value to convert
 *
 * 
 *  Outputs:
 *  -------
 *  toVal = descriptor to use to return converted value
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void SmCvtStringToContManagement (args, numArgs, fromVal, toVal)

        XrmValue * args;
        Cardinal   numArgs;
        XrmValue * fromVal;
        XrmValue * toVal;
#else /* _NO_PROTO */
void SmCvtStringToContManagement (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal)
#endif /* _NO_PROTO */
{

    unsigned char       *pch = (unsigned char *) (fromVal->addr);
    unsigned char       *pchNext;
    int         len;
    static long cval;
    Boolean     fHit = False;
    Boolean    fAddNext = True;

/*
 * Names of contention management options
 */

#define CM_ALL_STR              (unsigned char *)"all"
#define CM_SYSTEM_STR           (unsigned char *)"system"
#define CM_HANDSHAKE_STR        (unsigned char *)"handshake"
#define CM_NONE_STR             (unsigned char *)"none"

    /*
     * Check first token. If '-' we subtract from all options.
     * Otherwise, we start with no contention management and add things in.
     */

    if (*pch &&
        (_DtNextToken (pch, &len, &pchNext)) &&
        (*pch == '-'))
    {
        cval = SM_CM_ALL;
        fHit = True;
    }
    else
    {
        cval = SM_CM_NONE;
    }

    while (*pch && _DtNextToken (pch, &len, &pchNext))
    {
           /*
            * Strip off "sign" if prepended to another token, and process
            * that token the next time through.
            */

        if (*pch == '+')
        {
            if (len != 1)
            {
                pchNext = pch + 1;
            }
            fAddNext = TRUE;
        }

        else if (*pch == '-')
        {
            if (len != 1)
            {
                pchNext = pch + 1;
            }
            fAddNext = FALSE;
        }

        if ((*pch == 'A') || (*pch == 'a'))
        {
            if (_DtWmStringsAreEqual (pch, CM_ALL_STR, len))
            {
                cval = fAddNext ? (cval | SM_CM_ALL) :
                                  (cval & ~SM_CM_ALL);
                fHit = True;
            }
        }

        else if ((*pch == 'S') || (*pch == 's'))
        {
            if (_DtWmStringsAreEqual (pch, CM_SYSTEM_STR, len))
            {
                cval = fAddNext ? (cval | SM_CM_SYSTEM) :
                                  (cval & ~SM_CM_SYSTEM);
                fHit = True;
            }
        }

        else if ((*pch == 'H') || (*pch == 'h'))
        {
            if (_DtWmStringsAreEqual (pch, CM_HANDSHAKE_STR, len))
            {
                cval = fAddNext ? (cval | SM_CM_HANDSHAKE) :
                                  (cval & ~SM_CM_HANDSHAKE);
                fHit = True;
            }
        }

        else if ((*pch == 'N') || (*pch == 'n'))
        {
            if (_DtWmStringsAreEqual (pch, CM_NONE_STR, len))
            {
                /* don't bother adding or subtracting nothing */
                fHit = True;
            }
        }

        pch = pchNext;
    }



    /*
     * If we didn't match anything then set to default.
     */
    if (!fHit)
    {
        cval =  SM_CM_DEFAULT;
    }


    (*toVal).size = sizeof (long);
    (*toVal).addr = (caddr_t) &cval;


} /* END OF FUNCTION SmCvtStringToContManagement */


/*************************************<->*************************************
 *
 *  _DtNextToken (pchIn, pLen, ppchNext)
 *
 *
 *  Description:
 *  -----------
 *  XXDescription ...
 *
 *
 *  Inputs:
 *  ------
 *  pchIn = pointer to start of next token
 *
 * 
 *  Outputs:
 *  -------
 *  pLen  =    pointer to integer containing number of characters in next token
 *  ppchNext = address of pointer to following token
 *
 *  Return =   next token or NULL
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
unsigned char *_DtNextToken (pchIn, pLen, ppchNext)

        unsigned char  *pchIn;
        int            *pLen;
        unsigned char **ppchNext;
#else /* _NO_PROTO */
unsigned char *_DtNextToken (unsigned char *pchIn, int *pLen, 
        unsigned char **ppchNext)
#endif /* _NO_PROTO */
{
    unsigned char *pchR = pchIn;
    register int   i;

#ifdef MULTIBYTE
    register int   chlen;

    for (i = 0; ((chlen = mblen ((char *)pchIn, MB_CUR_MAX)) > 0); i++)
    /* find end of word: requires singlebyte whitespace terminator */
    {
        if ((chlen == 1) && isspace (*pchIn))
        {
            break;
        }
        pchIn += chlen;
    }

#else
    for (i = 0; *pchIn && !isspace (*pchIn); i++, pchIn++)
    /* find end of word */
    {
    }
#endif

    /* skip to next word */
    ScanWhitespace (&pchIn);

    *ppchNext = pchIn;
    *pLen = i;
    if (i)
    {
        return(pchR);
    }
    else
    {
       return(NULL);
    }

} /* END OF FUNCTION _DtNextToken */   



/*************************************<->*************************************
 *
 *  _DtWmStringsAreEqual (pch1, pch2, len)
 *
 *
 *  Description:
 *  -----------
 *  XXDescription ...
 *
 *
 *  Inputs:
 *  ------
 *  pch1 =
 *  pch2 =
 *  len  =
 *
 * 
 *  Outputs:
 *  -------
 *  Return = (Boolean) True iff strings match (case insensitive)
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean _DtWmStringsAreEqual (pch1, pch2, len)

        unsigned char *pch1;
        unsigned char *pch2;
        int            len;             /* character count */
#else /* _NO_PROTO */
Boolean _DtWmStringsAreEqual (unsigned char *pch1, unsigned char *pch2, int len)
#endif /* _NO_PROTO */
{
#ifdef MULTIBYTE
    int       chlen1;
    int       chlen2;
    wchar_t   wch1;
    wchar_t   wch2;

    while (len  && 
           ((chlen1 = mbtowc (&wch1, (char *) pch1, 2)) > 0) &&
           ((chlen2 = mbtowc (&wch2, (char *) pch2, 2)) == chlen1) )
    {
        if (chlen1 == 1)
        /* singlebyte characters -- make case insensitive */
        {
            if ((isupper (*pch1) ? tolower(*pch1) : *pch1) !=
                (isupper (*pch2) ? tolower(*pch2) : *pch2))
            {
                break;
            }
        }
        else
        /* multibyte characters */
        {
            if (wch1 != wch2)
            {
                break;
            }
        }
        pch1 += chlen1;
        pch2 += chlen2;
        len--;
    }

#else
    while (len  && *pch1 && *pch2 &&
           ((isupper (*pch1) ? tolower(*pch1) : *pch1) ==
            (isupper (*pch2) ? tolower(*pch2) : *pch2)))
    {
        *pch1++;
        *pch2++;
        len--;
    }
#endif

    return (len == 0);

} /* END OF _DtWmStringsAreEqual */   
