/* $XConsortium: SmSave.c /main/cde1_maint/2 1995/08/30 16:32:45 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:        SmSave.c
 **
 **  Project:     HP DT Session Manager (dtsession)
 **
 **  Description:
 **  -----------
 **  This file contains all routines that save off the session at any point
 **  in time.  The session includes settings, resources, and clients.  How
 **  the session manager behaves during a save depends on how the user has
 **  configured it to behave.
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


#include<stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#ifdef _SUN_OS
#include <arpa/inet.h>
#include <string.h>
#endif /* _SUN_OS */
#include <X11/Intrinsic.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/StringDefs.h>
#include <X11/keysymdef.h>
#include <Dt/DtP.h>
#include <Dt/Connect.h>
#include <Dt/UserMsg.h>
#include <Dt/SessionM.h>
#include "Sm.h"
#include "SmResource.h"
#include "SmError.h"
#include "SmGlobals.h"
#include "SmUI.h"
#include "SmWindow.h"
#include "SmProtocol.h"
#ifdef   MULTIBYTE
#include <stdlib.h>
#include <limits.h>
#endif

/* Busy is also defined in the BMS  -> bms.h. This conflicts with
 * /usr/include/X11/Xasync.h on ibm.
 */
#ifdef _AIX
#ifdef Busy
#undef Busy
#endif
#endif
#include <X11/Xlibint.h>

/*
 * Local Function Declarations
 */
#ifdef _NO_PROTO

static int OutputState() ;
static int PrintStartupInfo() ;
static int QueryServerSettings() ;
static int SaveCustomizeSettings() ;
static int OutputResource() ;
static void PrintPointerControl() ;
static void PrintSessionInfo() ;
static void PrintScreenSaver() ;
static int PrintFontPaths() ;
static int PrintKeyboardControl() ;
static int PrintPointerMapping() ;
static void PrintWorkHintString() ;
static void PrintCmdHintString() ;
static void PrintCmdString() ;
static void PrintRemoteCmdString() ;

#else

static int OutputState( 
                        FILE *stateFP) ;
static int PrintStartupInfo( 
                        FILE *stateFP,
                        int screen,
                        WindowInfo windowInfo,
                        Boolean containedClient) ;
static int QueryServerSettings( void ) ;
static int SaveCustomizeSettings( void ) ;
static int OutputResource( void ) ;
static void PrintPointerControl( 
                        XrmDatabase *smBase) ;
static void PrintSessionInfo( 
                        XrmDatabase *smBase) ;
static void PrintScreenSaver( 
                        XrmDatabase *smBase) ;
static int PrintFontPaths( 
                        XrmDatabase *smBase,
                        char **fontPath,
                        int numPaths) ;
static int PrintKeyboardControl( 
                        XrmDatabase *smBase);
static int PrintPointerMapping( 
                        XrmDatabase *smBase,
                        char *buttonRet,
                        int numButton) ;
static void PrintWorkHintString( 
                        FILE *hintFile,
                        char *hintString) ;
static void PrintCmdHintString( 
                        FILE *hintFile,
                        char *hintString) ;
static void PrintCmdString( 
                        FILE *cmdFile,
                        char *cmdString) ;
static void PrintRemoteCmdString( 
                        FILE *cmdFile,
                        char *cmdString) ;

#endif /* _NO_PROTO */

/*
 * Global variables that are exposed
 */
SmScreenInfo		screenSaverVals;
SmAudioInfo		audioVals;
SmKeyboardInfo		keyboardVals;

/*
 * Variables global to this module only
 */

/*
 * used as a buffer to write resources into
 * before they are written out
 */
static int	resSize = 10000;
static char	*resSpec;


/*************************************<->*************************************
 *
 *  SaveState (saveToHome, mode)
 *
 *
 *  Description:
 *  -----------
 *  In charge of calling all routines that save session information.
 *  Session information includes settings, resources, and client info.
 *
 *
 *  Inputs:
 *  ------
 *  saveToHome:  This session is to be saved to the home session or current
 *  mode = whether the session is to be reset or restarted
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  As a result of this routine files will be saved in the users .dt directory
 *  that will allow the session manager to restore the session at a later
 *  time.
 * 
 *************************************<->***********************************/
int 
#ifdef _NO_PROTO
SaveState( saveToHome, mode )
        Boolean saveToHome ;
        int mode ;
#else
SaveState(
        Boolean saveToHome,
        int mode )
#endif /* _NO_PROTO */
{

  FILE 			*stateFP, *convertFP;
  unsigned char		*propData;
  char				*tmpPath, tmpVersion[20];

  /*
   * First set up the save paths so we know if we have to do anything here
   */
  SetSavePath(saveToHome, mode);
  if(smGD.clientPath[0] == NULL)
  {
      /*
       * This session is to be reset or we are in compatibility mode
       * All we need to do is exit
       */
      return(0);
  }


  resSpec = (char *) SM_MALLOC(resSize * sizeof(char));
  if (resSpec==NULL)
  {
      PrintErrnoError(DtError, smNLS.cantMallocErrorString);
      return(-1);
  }

  /*
   * Don't let anything happen during the save - start a wait state
   */
  ShowWaitState(True);
  
  /*
   * Take care of the save mode property on the root window so that the
   * clients will know how to save this file (if they save files)
   */
  if(saveToHome == False)
  {
      propData = (unsigned char *) SM_CURRENT_DIRECTORY;
  }
  else
  {
      propData = (unsigned char *) SM_HOME_DIRECTORY;
  }

  /*
   * Change the save mode to give the subdirectory saved to
   *
   */
  XChangeProperty(smGD.display, RootWindow(smGD.display, 0),
		  XaSmSaveMode, XA_STRING, 8, PropModeReplace,
		   propData, strlen((char *)propData));
     
  stateFP=fopen(smGD.clientPath,"w");
  if (stateFP==NULL)
  {
      PrintErrnoError(DtError, smNLS.cantOpenFileString);
      ShowWaitState(False);
      SM_FREE(resSpec);
      return(-1);
  }

  /*
   * Do the actual output of each individual client's information.
   */
  if(OutputState(stateFP))
  {
      ShowWaitState(False);
      SM_FREE(resSpec);
      return(-1);
  }
    
    
  /*
   * Close the file 
   */
  (void) fclose(stateFP);

  /*
   * WARNING - THIS VARIABLE MUST BE SET BEFORE THE SETTING INFORMATION IS
   * CALLED - SETTINGS MUST BE SAVED BEFORE RESOURCES BECAUSE SETTINGS
   * GETS INFORMATION SAVED IN RESOURCES (smToSet.dClickBuf)
   */
  smToSet.dClickBuf[0] = NULL;

  /*
   * Do the actual output of the X settings information.  Output will
   * go to the resource database at this point and be written to the
   * resource file with the rest of the resources (below).
   */
  if(smRes.querySettings == True)
  {
      QueryServerSettings();
  }
  else
  {
      SaveCustomizeSettings();
  }


  /*
   * Do the output of resource information.
   */
  if(OutputResource())
  {
      ShowWaitState(False);
      SM_FREE(resSpec);
      return(-1);
  }

  XDeleteProperty(smGD.display, DefaultRootWindow(smGD.display),
		  XaSmSaveMode);
 
/* 
 * Go save the default palette at this time
*/
  if(saveToHome == True)
     SaveDefaultPalette(smGD.display, smGD.savePath, DtSM_HOME_STATE);
  else
     SaveDefaultPalette(smGD.display, smGD.savePath, mode);

 
  /*
   * If we are in a session that has been newly converted to the latest
   * version of HP DT - make that fact visible in the form of a file
   */
  if((smGD.sessionVersion == NULL) || (*smGD.sessionVersion == NULL))
  {
    tmpPath = (char *) SM_MALLOC(strlen(smGD.savePath) + 
								 strlen(SM_CURRENT_DIRECTORY) +
								 strlen(SM_CONVERSION_FILE) + 3);
    if(tmpPath == NULL)
    {
        PrintErrnoError(DtError, smNLS.cantMallocErrorString);
  		ShowWaitState(False);
        return(-1);
    }
	
	strcpy(tmpPath, smGD.savePath);
	strcat(tmpPath, "/");
	if(saveToHome == False)
	{
		strcat(tmpPath, SM_CURRENT_DIRECTORY);
	}
	else
	{
		strcat(tmpPath, SM_HOME_DIRECTORY);
	}
	strcat(tmpPath, "/");
	strcat(tmpPath, SM_CONVERSION_FILE);
		

  	convertFP=fopen(tmpPath,"w");
  	if (stateFP==NULL)
	{
		PrintErrnoError(DtError, smNLS.cantOpenFileString);
		ShowWaitState(False);
		return(-1);
	}

	strcpy(tmpVersion, DtCURRENT_DT_VERSION);
	fprintf(convertFP, DtCURRENT_DT_VERSION);
	free(tmpPath);
  	(void) fclose(convertFP);

  }
	
  ShowWaitState(False);

  return(0);
}


/*************************************<->*************************************
 *
 *  OutputState (stateFP)
 *
 *
 *  Description:
 *  -----------
 *  In charge of saving client information.  Calls routines to find top level
 *  windows, and then prints information found in WM_COMMAND property found
 *  on those windows.
 *
 *
 *  Inputs:
 *  ------
 *  stateFP = a pointer to the client file.
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static int 
#ifdef _NO_PROTO
OutputState( stateFP )
        FILE *stateFP ;
#else
OutputState(
        FILE *stateFP )
#endif /* _NO_PROTO */
{
  unsigned int topListLength, containedListLength;
  WindowInfo *topList;	/* list of top-level windows */
  WindowInfo topLevelWindowInfo;
  Atom actualType;
  int actualFormat;
  unsigned long nitems;
  unsigned long leftover;
  unsigned char *data = NULL;
  Boolean	containedClient;
  int i, j;

  /*
   * Save off the clients currently running.  This will be done
   * one screen at a time
   */
  for(j = 0;j < smGD.numSavedScreens;j++)
  {
      /*
       * Get a list of children of the root window. If there are none
       * return with an error condition.
       */
      if(GetTopLevelWindows(j,&topList,&topListLength, &containedListLength))
	  return(1);
  
      /*
       * Figure out how to restart the client associate with each window.
       */
      for (i=0 ; i<(topListLength + containedListLength); ++i)
      {
	  topLevelWindowInfo=topList[i];

	  if(i >= topListLength)
	  {
	      containedClient = True;
	  }
	  else
	  {
	      containedClient = False;
	  }
      
	  /*
	   * If this window is my window - skip it I will be started
	   * automatically
	   */
	  if (XGetWindowProperty(smGD.display, topLevelWindowInfo.wid,
				 XA_WM_CLASS,0L,
				 (long)BUFSIZ,False,XA_STRING,&actualType,
				 &actualFormat,&nitems,&leftover,
				 &data)==Success)
	  {
	      if (actualType==XA_STRING &&
		  !strcmp((char *) data, SM_RESOURCE_NAME))
	      {
		  SM_FREE((char *)data);
		  continue;
	      }
	      else
	      {
		  SM_FREE((char *)data);
	      }
	  }
      
	  /*
	   * If the client participates in the WM_SAVE_YOURSELF protocol, send
	   * the appropriate messages and wait for the proper responses.
	   */
	  (void) SaveYourself(topLevelWindowInfo);

	  /*
	   * Print the startup information to the proper file.
	   */
	  (void) PrintStartupInfo(stateFP,j, topLevelWindowInfo,
				  containedClient);
      }

      /*
       * Free space malloc'd by GetTopLevelWindowInfo
       */
      if(topListLength > 0)
      {
	  SM_FREE((char *) topList);
      }
  }
  return(0);
}


/*************************************<->*************************************
 *
 *  PrintStartupInfo (stateFP,screen, windowInfo, containedClient)
 *
 *
 *  Description:
 *  -----------
 *  Print startup information for each application out to the client file.
 *  This prints information for only one top level window.
 *
 *
 *  Inputs:
 *  ------
 *  stateFP = a pointer to the client file.
 *  screen = which screen the client is running on
 *  windowInfo = the top level window of the application, and its current state
 *  containedClient = whether its a true top level or contained in the
 *			front panel
 *
 * 
 *  Outputs:
 *  -------
 *  stateFP = the file that is written to.
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static int 
#ifdef _NO_PROTO
PrintStartupInfo( stateFP, screen, windowInfo, containedClient )
        FILE *stateFP ;
        int screen ;
        WindowInfo windowInfo ;
        Boolean containedClient ;
#else
PrintStartupInfo(
        FILE *stateFP,
        int screen,
        WindowInfo windowInfo,
        Boolean containedClient )
#endif /* _NO_PROTO */
{
  int i, j, writeStatus;
  static char myHost[101]="none";
  int x,y,width,height;
  Window child;
  int wargc, final_argc;
  char **wargv, **tmp_argv=NULL, **final_argv;
  char *clientMachine, *tmpName;
  unsigned char *wsData;
  Atom		*workspaces;
  unsigned long numWorkspaces;
  Window iconWindow;
  XWindowAttributes windowAttr;
  XSizeHints hints;
  Boolean	remoteHost;
#ifdef _SUN_OS /* pull out quotes from OpenWindow tools commands */
  char *ptr;
#endif
  Boolean  saveIconPos;
  char bufX[50], bufY[50];
  
  /*
   * Get the attributes and properties on the window - If we get an error
   * return an error condition
   */
  XGetWindowAttributes(smGD.display,windowInfo.wid,&windowAttr);
  if (GetStandardProperties(windowInfo.wid, &wargc,&wargv,&hints,
			    &clientMachine, &wsData, &numWorkspaces,
			    &iconWindow)==0)
    return(1);

  final_argv = wargv;
  final_argc = wargc;
  
  /*
   * If the client doesn't have a WM_COMMAND property, we can't restart it
   */
  if (!wargv||!wargv[0])
    return(0);

#ifdef _SUN_OS /* pull out quotes from OpenWindow tools commands */
  if (wargc == 1) {
     ptr = wargv[0];
     /* check for foreground */
     if ((ptr = strstr (ptr, "fg \"")) != NULL) {
        if (strlen (ptr) > 3)
           ptr[3] = ' ';
        if ((ptr = strchr (ptr, '"')) != NULL)
           ptr[0] = ' ';
     }

     /* check for background */
     if ((ptr = strstr (wargv[0], "bg \"")) != NULL) {
        if (strlen (ptr) > 3)
           ptr[3] = ' ';
        if ((ptr = strchr (ptr, '"')) != NULL)
           ptr[0] = ' ';
     }
  }
#endif
  
  /*
   * Find out what host dtsession is running on
   */
  if (!strcmp(myHost,"none")) 
  {
      DtGetShortHostname(myHost,100);
  }

  /*
   * Find out if the client is running locally
   */
  remoteHost = False;
  if(clientMachine != NULL)
  {
      if(DtIsLocalHostP(clientMachine) == FALSE)
      {
	  remoteHost = True;
      }
  }

  fprintf(stateFP,"dtsmcmd ");

  if(containedClient == False)
  {
      /*
       * Find out what the coordinates of the top-level window are relative
       * to the RootWindow(remember many window managers reparent).
       */
      XTranslateCoordinates(smGD.display,windowInfo.wid,
			    RootWindow(smGD.display,screen),0,0,&x,&y,
			    &child);
      x-=windowAttr.border_width;
	  y-=windowAttr.border_width;
      width=windowAttr.width;
      height=windowAttr.height;


      /*
       * The width in pixels is base_width + (width * width_inc).
       * The height in pixels is base_height + (width * height_inc).
       * Xlib current doesn't know about Base Size.
       */
      if (hints.flags&PBaseSize)
      {
	  width  -= (hints.base_width < 0 ? 0 : hints.base_width);
	  height -= (hints.base_height < 0 ? 0 : hints.base_height);
      }
      else
      {
	  if (hints.flags&PMinSize)
	  {
	      width-=hints.min_width;
	      height-=hints.min_height;
	  }
      }
 
      if (hints.flags&PResizeInc)
      {
	  width=width/hints.width_inc;
	  height=height/hints.height_inc;
      }

      /*
       * Print out the window manager hints for this client
       */
      fprintf(stateFP,"-hints \"");
      fprintf(stateFP,"-geometry %dx%d+%d+%d ",width,height,x,y);
      if (windowInfo.termState == IconicState)
      {
	  fprintf(stateFP,"-state IconicState ");
      }
      else
      {
	  fprintf(stateFP,"-state NormalState ");
      }

      if(remoteHost == True)
      {
	  fprintf(stateFP,"-host ");
	  fprintf(stateFP,"%s ",clientMachine); /* Start on remote host. */
      }

      if(numWorkspaces > 0)
      {
	  workspaces = (Atom *) wsData;
	  fprintf(stateFP, "-workspaces \\\"");
	  for(i=0;i<numWorkspaces;i++)
	  {
	      tmpName = XGetAtomName(smGD.display, workspaces[i]);
	      if(tmpName != NULL)
	      {
		  PrintWorkHintString(stateFP, tmpName);
		  fprintf(stateFP, " ");
		  SM_FREE(tmpName);
	      }
	  }
	  fprintf(stateFP, "\\\" ");
	  SM_FREE((char *)wsData);
      }

      /*
       * Print the command as a hint
       */
      fprintf(stateFP, "-cmd \\\"");
      fprintf(stateFP,"%s ",wargv[0]);

      {
	  /* save icon position info for motif apps */

	  Atom wmMessages, actType;
	  int actFormat;
	  unsigned long nitems, leftover;
	  unsigned char *prop_return;

	  /* First determine if window belongs to a motif app by checking
	     for the existence of the _MOTIF_WM_MESSAGES property. */
	  
	  wmMessages = XInternAtom(smGD.display, "_MOTIF_WM_MESSAGES", False);

	  if ((XGetWindowProperty(smGD.display, windowInfo.wid, wmMessages, 0L,
				 (long) 1000, False, wmMessages, &actType,
				 &actFormat, &nitems, &leftover, &prop_return)
	      == Success) && (actType != None) && (actFormat != 0) &&
	      (iconWindow > 0))
	  {
	      XWindowAttributes attrib;

	      /* This is a motif app. Build the XmNiconX and XmNiconY
		 resource values. */
	      if (XGetWindowAttributes(smGD.display, iconWindow, &attrib))
	      {
		  sprintf(bufX, "*iconX:%d", attrib.x);
		  sprintf(bufY, "*iconY:%d", attrib.y);
		  saveIconPos = True;
	      }
	      else
		  saveIconPos = False;
	  }
	  else
	  {
	      saveIconPos = False;
	  }

	  /* We don't use the value _MOTIF_WM_MESSAGES property */
	  if(prop_return)
	  {
	      XFree(prop_return);
	      prop_return = NULL;
	  }

	  if (saveIconPos)
	  {
	      /* allocate enough memory for all of wargv plus 4 more
		 args for icon X and Y position resource names and values */

	      tmp_argv = (char **)XtMalloc((wargc+4) * sizeof(char *));
	      tmp_argv[0] = wargv[0];

	      /* filter out older icon resource settings */
	      for(i=1, j=1; j<wargc; j++)
	      {
		  char *tok;
		  
		  if(tok = strtok(wargv[j], ":"))
		  {
		      if ((!strcmp(tok, "*iconX") || !strcmp(tok, "*iconY"))
			  && (i>1) && !strcmp(tmp_argv[i-1], "-xrm"))
		      {
			  i--;
			  wargv[j][strlen(tok)] = ':';
			  continue;
		      }
		  }

		  tmp_argv[i] = wargv[j]; i++;
	      }

	      tmp_argv[i] = "-xrm"; i++;
	      tmp_argv[i] = bufX; i++;
	      tmp_argv[i] = "-xrm"; i++;
	      tmp_argv[i] = bufY; i++;

	      final_argv = tmp_argv;
	      final_argc = i;
	  }
      }
      
      for (i=1;i<final_argc;++i)
      {
	  /*
	   * Print out the hint double quoting quotes, quoting spaces and
	   * backslashes
	   */
	  PrintCmdHintString(stateFP, final_argv[i]);
	  fprintf(stateFP, " ");
      }

      fprintf(stateFP,"\\\"\" ");
  }
  else
  {
      /*
       * if this is a contained client free the workspace data if it exists
       */
      if(numWorkspaces > 0)
      {
	  SM_FREE((char *)wsData);
      }
  }
  
  /*
   * See if the client is on the same host.  If not preface the command
   * with the remote host name so that this command can be passed to the
   * execution daemon upon restart
   */
  if(remoteHost == True)
  {
      fprintf(stateFP,"-host ");
      fprintf(stateFP,"%s ",clientMachine); /* Start on remote host. */
  }

  if(clientMachine != NULL)
  {
      SM_FREE(clientMachine);
  }

  fprintf(stateFP, "-screen %d ", screen);
  fprintf(stateFP, "-cmd \"");

  /* Print the command name. */
  fprintf(stateFP,"%s",final_argv[0]);

  for (i=1;i<final_argc;++i)
  {
      fprintf(stateFP, " ");
      if(remoteHost == False)
      {
	  PrintCmdString(stateFP, final_argv[i]);
      }
      else
      {
	  PrintRemoteCmdString(stateFP, final_argv[i]);
      }
  }

  writeStatus = fprintf(stateFP,"\"\n\n");

  /*
   * Do a check on the fprintf to see if there is still no
   * write error
   */
  if(writeStatus < 0)
  {
    /*
     * There should be an error written here - however
     * since this means that there is write errors to
     * the disk - no error can be written
     */
    (void) fclose(stateFP);

    /*
     * On a write error - move the old session back to
     * be used and exit with an error condition
     */
    XFreeStringList(wargv);

    strcat(smGD.savePath, "/");
    if(smGD.sessionType == HOME_SESSION)
    {
        strcat(smGD.savePath, SM_HOME_DIRECTORY);
    }
    else
    {
        strcat(smGD.savePath, SM_CURRENT_DIRECTORY);
    }

    MoveDirectory(smGD.etcPath, smGD.savePath);
    SM_EXIT(-1);
  }

  /*
   * Free up the argv property
   */
  XFreeStringList(wargv);

  if(tmp_argv)
      XtFree((char *)tmp_argv);
  
  /* return a "no error" condition */
  return(0);
}


/*************************************<->*************************************
 *
 *  QueryServerSettings ()
 *
 *
 *  Description:
 *  -----------
 *  Query the server for all it's current settings information, and then
 *  write that information to the session managers settings resource file.
 *
 *  Inputs:
 *  ------
 *  smGD.settingPath = (global) pointer to file where
 *			setting info should be saved.
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  This routine slows down the session manager's shutdown process
 *  considerably, and is therefore only called when the user specifies
 *  that all settings should be queried.  Otherwise, only settings set in
 *  the customizer should be saved.
 * 
 *************************************<->***********************************/
static int 
#ifdef _NO_PROTO
QueryServerSettings()
#else
QueryServerSettings( void )
#endif /* _NO_PROTO */
{
    int			i, numLoops;
    char		**fontPath;
    KeySym		*tmpSyms, tmpMod;
    XModifierKeymap	*modMap;
    char		*buttonRet;
    int			numButton, numPaths;
    int			buttonSize = 128;
    XrmDatabase 	smBase = NULL;
    char		tmpChar[35];


    buttonRet = (char *) SM_MALLOC(buttonSize * sizeof(char));
    if (buttonRet==NULL)
    {
	PrintErrnoError(DtError, smNLS.cantMallocErrorString);
	return(-1);
    }

    /*
     * First set up all pointer control parameters
     * These are simple integer values that get saved
     * here, then read into a structure and set at restore
     * time
     */
    XGetPointerControl(smGD.display, &smSettings.accelNum,
		       &smSettings.accelDenom,
		       &smSettings.threshold);
    PrintPointerControl(&smBase);

    /*
     * Get all the screen saver values.  These are all simple integer
     * values that are read back into a structure and set at restore
     * time.
     */
    XGetScreenSaver(smGD.display, &screenSaverVals.smTimeout,
		    &screenSaverVals.smInterval,
		    &screenSaverVals.smPreferBlank,
		    &screenSaverVals.smAllowExp);
    PrintScreenSaver(&smBase);

    /*
     *  Write out all font path information. 
     *  Write out all the paths one by one concatenated by commas.
     */
    fontPath = XGetFontPath(smGD.display, &numPaths);
    if(fontPath == NULL)
    {
	SM_FREE(buttonRet);
	PrintErrnoError(DtError, smNLS.cantMallocErrorString);
	return(-1);
    }

    if(PrintFontPaths(&smBase, fontPath, numPaths) != 0)
    {
	XFreeFontPath(fontPath);
	SM_FREE(buttonRet);
	return(-1);
    }

    /*
     * Write out all the keyboard control information.
     */
    XGetKeyboardControl(smGD.display, &smSettings.kbdState);
    if(PrintKeyboardControl(&smBase) != 0)
    {
	XFreeFontPath(fontPath);
	return(-1);
    }
    
    /*
     * Put all the button modifier information into the resource database.
     * Find out the number of buttons and then write out the character
     * string for each button.
     */
    numButton = XGetPointerMapping(smGD.display, (unsigned char *)buttonRet, 128);
    if(numButton > 128)
    {
	buttonRet = (char *) SM_REALLOC(buttonRet, numButton * sizeof(char));
	if (buttonRet==NULL)
	{
	    XFreeFontPath(fontPath);
	    PrintErrnoError(DtError, smNLS.cantMallocErrorString);
	    return(-1);
	}
	numButton = XGetPointerMapping(smGD.display, (unsigned char *)buttonRet, numButton);
    }
    if(PrintPointerMapping(&smBase, buttonRet,
			   numButton)  != 0)
    {
	XFreeFontPath(fontPath);
	SM_FREE(buttonRet);
	return(-1);
    }

    /*
     * Put all keyboard mapping information in the resource database
     * Don't bother putting this in a separate print function because
     * the customizer doesn't do keymap savings this way.
     */
    smSettings.numKeyCode = smGD.display->max_keycode -
	smGD.display->min_keycode;
    tmpSyms = XGetKeyboardMapping(smGD.display,
				  (KeyCode) smGD.display->min_keycode,
				  smSettings.numKeyCode,
				  &smSettings.keySymPerCode);
    if(tmpSyms == NULL)
    {
	XFreeFontPath(fontPath);
	SM_FREE(buttonRet);
	PrintErrnoError(DtError, smNLS.cantMallocErrorString);
	return(-1);
    }
    
    strcpy(resSpec, SM_RESOURCE_CLASS);
    strcat(resSpec, "*");
    strcat(resSpec, SmCnumKeyCode);
    strcat(resSpec, ":");
    sprintf(tmpChar,"%d", smSettings.numKeyCode);
    strcat(resSpec, tmpChar);
    XrmPutLineResource( &smBase, resSpec);
    
    strcpy(resSpec, SM_RESOURCE_CLASS);
    strcat(resSpec, "*");
    strcat(resSpec, SmCkeySymsPerKey);
    strcat(resSpec, ":");
    sprintf(tmpChar,"%d", smSettings.keySymPerCode);
    strcat(resSpec, tmpChar);
    XrmPutLineResource( &smBase, resSpec);

    strcpy(resSpec, SM_RESOURCE_CLASS);
    strcat(resSpec, "*");
    strcat(resSpec, SmCkeySyms);
    strcat(resSpec, ":");
    numLoops = smSettings.numKeyCode * smSettings.keySymPerCode;
    sprintf(tmpChar,"%d", tmpSyms[0]);
    strcat(resSpec, tmpChar);
    for(i = 1;i < numLoops;i++)
    {
	if((strlen(resSpec) + 5) >= resSize)
	{
	    resSize += 5000;
	    resSpec = (char *) SM_REALLOC(resSpec, resSize * sizeof(char));
	    if (resSpec==NULL)
	    {
		SM_FREE((char *)tmpSyms);
		SM_FREE((char *)fontPath);
		SM_FREE(buttonRet);
		PrintErrnoError(DtError, smNLS.cantMallocErrorString);
		return(-1);
	    }
	}
	strcat(resSpec, ",");
	sprintf(tmpChar,"%d", tmpSyms[i]);
	strcat(resSpec, tmpChar);
    }
    XrmPutLineResource( &smBase, resSpec);
	

    /*
     * Write out all keyboard modifier info.  This will be a pretty slow
     * process considering that every keycode will have to be turned into
     * a keysym before writing.  Don't bother putting print stuff in a
     * separate function because it will not be handled in the customize
     * version.
     */
    modMap = XGetModifierMapping(smGD.display);
    if(modMap == NULL)
    {
	SM_FREE((char *)tmpSyms);
	XFreeFontPath(fontPath);
	SM_FREE(buttonRet);
	PrintErrnoError(DtError, smNLS.cantMallocErrorString);
	return(-1);
    }

    strcpy(resSpec, SM_RESOURCE_CLASS);
    strcat(resSpec, "*");
    strcat(resSpec, SmCmaxKeyPerMod);
    strcat(resSpec, ":");
    sprintf(tmpChar,"%d", modMap->max_keypermod);
    strcat(resSpec, tmpChar);
    XrmPutLineResource( &smBase, resSpec);

    strcpy(resSpec, SM_RESOURCE_CLASS);
    strcat(resSpec, "*");
    strcat(resSpec, SmCmodMap);
    strcat(resSpec, ":");
    numLoops = (8 * modMap->max_keypermod) - 1;
    for(i = 0;i <= numLoops;i++)
    {
	if((strlen(resSpec) + 9) >= resSize)
	{
	    resSize += 5000;
	    resSpec = (char *) SM_REALLOC(resSpec, resSize * sizeof(char));
	    if (resSpec==NULL)
	    {
		SM_FREE((char *)tmpSyms);
		XFreeModifiermap(modMap);
		XFreeFontPath(fontPath);
		SM_FREE(buttonRet);
		PrintErrnoError(DtError, smNLS.cantMallocErrorString);
		return(-1);
	    }
	}
	tmpMod = XKeycodeToKeysym(smGD.display, modMap->modifiermap[i], 0);
	sprintf(tmpChar,"%ld", tmpMod);
	strcat(resSpec, tmpChar);
	if(i != numLoops)
	{
	    strcat(resSpec, ",");
	}
    }
    XrmPutLineResource( &smBase, resSpec);
    

    /*
     * Print session manager settings
     */
    PrintSessionInfo(&smBase);
    
    /*
     * Write settings resources out to disk
     */
    XrmPutFileDatabase(smBase, smGD.settingPath);

    /*
     * Free all allocated resources
     */
    XFreeFontPath(fontPath);
    SM_FREE((char *)tmpSyms);
    XFreeModifiermap(modMap);
    SM_FREE(buttonRet);
    XrmDestroyDatabase(smBase);
    return(0);
}


/*************************************<->*************************************
 *
 *  SaveCustomizeSettings ()
 *
 *
 *  Description:
 *  ------------
 *  Saves those settings that have been set in the customizer during the
 *  session to whatever values they were set to during the session.
 *
 *
 *  Inputs:
 *  ------
 *  smGD.resourcePath = (global) a pointer
 *			to where the settings should be stored.
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  This routine is the default saved for saving settings.  If the user
 *  wants all settings saved, they have to set a resource saying that that
 *  will be the case.
 * 
 *************************************<->***********************************/
static int 
#ifdef _NO_PROTO
SaveCustomizeSettings()
#else
SaveCustomizeSettings( void )
#endif /* _NO_PROTO */
{
    int			i,numPaths;
    char		**fontPath;
    XrmDatabase 	smBase = NULL;
    char		*tmpChar[20], *ptrRet = NULL, *tmpKey;
    Status		newStat;
	char		tmpMap[128];


    if(smCust.pointerChange)
    {
	newStat = _DtGetSmPointer(smGD.display, smGD.topLevelWindow,
				 XaDtSmPointerInfo, &ptrRet);
	if(newStat != Success)
	{
	    smToSet.pointerChange = False;
	    smToSet.pointerMapChange = False;
	}
	else
	{
	    sscanf(ptrRet, "%d %s %d %d %s",
		   &smToSet.numButton,
		   &tmpMap,
		   &smSettings.accelNum,
		   &smSettings.threshold,
		   &smToSet.dClickBuf);
	    smSettings.accelDenom = 10;

	    tmpKey = strtok(tmpMap, "_");
	    i = 0;
	    while(tmpKey != NULL)
	    {
		smToSet.pointerMap[i] = (char) atoi(tmpKey);
		i++;
		tmpKey = strtok(NULL, "_");
	    }


	    /*
	     * append a \n to dClickBuf because it was scanned out
	     */
	    strcat(smToSet.dClickBuf, "\n");
	}
        if (ptrRet) XFree(ptrRet);
    }
				     
    if(smToSet.pointerChange == True)
    {
	PrintPointerControl(&smBase);
    }

    if(smToSet.pointerMapChange == True)
    {
	if(PrintPointerMapping(&smBase, smToSet.pointerMap,
			    smToSet.numButton) != 0)
	{
	    return(-1);
	}
    }

    if(smToSet.screenSavChange == True)
    {
	PrintScreenSaver(&smBase);
    }

    if(smCust.audioChange == True)
    {
      newStat = _DtGetSmAudio(smGD.display, smGD.topLevelWindow,
				   XaDtSmAudioInfo, &audioVals);
     
      if(newStat == Success)
      {
        smSettings.kbdState.bell_percent = audioVals.smBellPercent;
        smSettings.kbdState.bell_pitch = audioVals.smBellPitch;
        smSettings.kbdState.bell_duration = audioVals.smBellDuration;
      }  
      else
      {
        smToSet.audioChange = False;
      }
    }
    
    if (smToSet.audioChange)
    {
      strcpy(resSpec, SM_RESOURCE_CLASS);
      strcat(resSpec, "*");
      strcat(resSpec, SmCbellPercent);
      strcat(resSpec, ":");
      sprintf((char *)tmpChar,"%d",smSettings.kbdState.bell_percent);
      strcat(resSpec, (char *)tmpChar);
      XrmPutLineResource(&smBase, resSpec);

      strcpy(resSpec, SM_RESOURCE_CLASS);
      strcat(resSpec, "*");
      strcat(resSpec, SmCbellPitch);
      strcat(resSpec, ":");
      sprintf((char *)tmpChar,"%d",smSettings.kbdState.bell_pitch);
      strcat(resSpec, (char *)tmpChar);
      XrmPutLineResource(&smBase, resSpec);

      strcpy(resSpec, SM_RESOURCE_CLASS);
      strcat(resSpec, "*");
      strcat(resSpec, SmCbellDuration);
      strcat(resSpec, ":");
      sprintf((char *)tmpChar,"%d",smSettings.kbdState.bell_duration);
      strcat(resSpec, (char *)tmpChar);
      XrmPutLineResource(&smBase, resSpec);
    }

    if(smCust.keyboardChange == True)
    {
      newStat = _DtGetSmKeyboard(smGD.display, smGD.topLevelWindow,
                                      XaDtSmKeyboardInfo, &keyboardVals);
      if(newStat == Success)
      {
        smSettings.kbdState.key_click_percent = keyboardVals.smKeyClickPercent;
        smSettings.kbdState.global_auto_repeat = keyboardVals.smGlobalAutoRepeat;
      }
      else
      {
        smToSet.keyboardChange = False;
      }
    }

    if(smToSet.keyboardChange == True)
    {
      strcpy(resSpec, SM_RESOURCE_CLASS);
      strcat(resSpec, "*");
      strcat(resSpec, SmCkeyClick);
      strcat(resSpec, ":");
      sprintf((char *)tmpChar,"%d", smSettings.kbdState.key_click_percent);
      strcat(resSpec, (char *)tmpChar);
      XrmPutLineResource(&smBase, resSpec);

      strcpy(resSpec, SM_RESOURCE_CLASS);
      strcat(resSpec, "*");
      strcat(resSpec, SmCglobalRepeats);
      strcat(resSpec, ":");
      sprintf((char *)tmpChar,"%d", smSettings.kbdState.global_auto_repeat);
      strcat(resSpec, (char *)tmpChar);
      XrmPutLineResource(&smBase, resSpec);
    }


    /*
     *  Write out all font path information. 
     *  Write out all the paths one by one concatenated by commas.
     */

    fontPath = XGetFontPath(smGD.display, &numPaths);

    if(fontPath == NULL) {
	PrintErrnoError(DtError, smNLS.cantMallocErrorString);
	return(-1);
    }
	
    if(PrintFontPaths(&smBase,  fontPath, numPaths) != 0) {
	XFreeFontPath(fontPath);
	return(-1);
    }

    /*
     * Save session manager settings
     */
    PrintSessionInfo(&smBase);
    
    XFreeFontPath(fontPath);

    /*
     * Write settings resources out to disk
     */
    XrmPutFileDatabase(smBase, smGD.settingPath);
    XrmDestroyDatabase(smBase);

    return(0);
}



/*************************************<->*************************************
 *
 *  OutputResource ()
 *
 *
 *  Description:
 *  -----------
 *  Save session resources by getting the current RESOURCE_MANAGER property
 *  on the root window.
 *
 *  Inputs:
 *  ------
 *  smGD.resourcePath = (global) file name
 *			where property contents should be saved.
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  This method is not a 100% accurate representation of currently active
 *  resources in the system.  It is however, the best representation (the
 *  values with the highest priority) without becoming overly invasive
 *  and slow.
 * 
 *************************************<->***********************************/
static int 
#ifdef _NO_PROTO
OutputResource()
#else
OutputResource( void )
#endif /* _NO_PROTO */
{
    Atom 		actualType;
    int	 		actualFormat;
    unsigned long	nitems, leftover; 
    char	 	*data = NULL;
    XrmDatabase		db;
    Status		newStat;
    char		*fontBuf = NULL, *langPtr, tmpChar[20], *sessionRes;
    float		fltYRes;
    int			intYRes;

    /*
     * Add anything to the Resource Manager property that needs to be added
     */

    /*
     * Write out the LANG variable and the screen's Y resolution
     */
    *resSpec = NULL;
    langPtr = getenv("LANG");
    if((langPtr != NULL) && (*langPtr != NULL))
    {
	sprintf(resSpec, "%s*%s: %s\n", SM_RESOURCE_NAME, SmNsessionLang,
		langPtr);
    }
    else
    {
	sprintf(resSpec, "%s*%s:   \n", SM_RESOURCE_NAME, SmNsessionLang);
    }

    fltYRes = ((float) DisplayHeight(smGD.display, 0) /
	(float) DisplayHeightMM(smGD.display, 0)) * 1000;
    intYRes = fltYRes;

    if(*resSpec == NULL)
    {
	strcpy(resSpec, SM_RESOURCE_NAME);
    }
    else
    {
	strcat(resSpec, SM_RESOURCE_NAME);
    }
    strcat(resSpec, "*");
    strcat(resSpec, SmNdisplayResolution);
    strcat(resSpec, ":");
    sprintf(tmpChar, "%d",intYRes);
    strcat(resSpec, tmpChar);
    strcat(resSpec, "\n");
    _DtAddToResource(smGD.display, resSpec);
    
    if(smCust.fontChange == True)
    {
	newStat = _DtGetSmFont(smGD.display, smGD.topLevelWindow,
			      XaDtSmFontInfo, &fontBuf);
	if(newStat == Success)
	{
	    /*
	     * Create the auxillary resource file
	     */
	    if(SetFontSavePath(langPtr) != -1)
	    {
		db = XrmGetStringDatabase(fontBuf);
	    
		if(intYRes < MED_RES_Y_RES)
		{
		    sessionRes = SM_LOW_RES_EXT;
		}
		else
		{
		    if(intYRes >= HIGH_RES_Y_RES)
		    {
			sessionRes = SM_HIGH_RES_EXT;
		    }
		    else
		    {
			sessionRes = SM_MED_RES_EXT;
		    }
		}

		strcat(smGD.fontPath, "/");
		strcat(smGD.fontPath, SM_FONT_FILE);
		
		strcat(smGD.fontPath, ".");
		strcat(smGD.fontPath, sessionRes);
		    
		XrmPutFileDatabase(db, smGD.fontPath);    
		XrmDestroyDatabase(db);
	    }
	    
	    /*
	     * Now add this to the resource manager property to be saved
	     */
	    _DtAddToResource(smGD.display, fontBuf);
	}
        if (fontBuf) XFree(fontBuf);
    }

    if((smCust.dClickChange == True) && (smToSet.dClickBuf[0] != NULL))
    {
	_DtAddToResource(smGD.display, (char *)smToSet.dClickBuf);
    }

    if((smGD.sessionVersion == NULL) || (*smGD.sessionVersion == NULL))
    {
	sprintf(resSpec,"*sessionVersion: %s\n", DtCURRENT_DT_VERSION);
	_DtAddToResource(smGD.display, resSpec);
    }

    /*
     * Get the contents of the _DT_SM_PREFERENCES property
     */
    data = _DtGetResString(smGD.display, _DT_ATR_PREFS);

   /* 
    * Get _DT_SM_PREFERENCES database 'db'.
    */
    db  = XrmGetStringDatabase((char *)data);

    XrmPutFileDatabase(db, smGD.resourcePath);

    /*
     * Don't forget to free your data
     */
    SM_FREE((char *)data);
    XrmDestroyDatabase(db);
    
    return(0);
}



/*************************************<->*************************************
 *
 *  PrintPointerControl (smBase)
 *
 *
 *  Description:
 *  -----------
 *  A convenience function that is separated out instead of being included
 *  in both QueryServerSettings, and SaveCustomizeSettings.
 *  Saves pointer control information to the named resource file.
 *
 *  Inputs:
 *  ------
 *  smBase = pointer to newly opened resource db used to store setting info
 *  resSpec = a buffer to hold string resource information until it is
 *            printed.
 *  smSettings = place where settings are stored.
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
PrintPointerControl( smBase)
        XrmDatabase *smBase ;
#else
PrintPointerControl(
        XrmDatabase *smBase)
#endif /* _NO_PROTO */
{
    char tmpChar[20];
    
    strcpy(resSpec, SM_RESOURCE_CLASS);
    strcat(resSpec, "*");
    strcat(resSpec, SmCaccelNum);
    strcat(resSpec, ":");
    sprintf(tmpChar, "%d", smSettings.accelNum);
    strcat(resSpec, tmpChar);
    XrmPutLineResource(smBase, resSpec);
    
    strcpy(resSpec, SM_RESOURCE_CLASS);
    strcat(resSpec, "*");
    strcat(resSpec, SmCaccelDenom);
    strcat(resSpec, ":");
    sprintf(tmpChar, "%d",smSettings.accelDenom);
    strcat(resSpec, tmpChar);
    XrmPutLineResource(smBase, resSpec);
    
    strcpy(resSpec, SM_RESOURCE_CLASS);
    strcat(resSpec, "*");
    strcat(resSpec, SmCthreshold);
    strcat(resSpec, ":");
    sprintf(tmpChar, "%d",smSettings.threshold);
    strcat(resSpec, tmpChar);
    XrmPutLineResource(smBase, resSpec);

    return;
}


/*************************************<->*************************************
 *
 *  PrintSessionInfo (smBase)
 *
 *
 *  Description:
 *  -----------
 *  A convenience function that is separated out instead of being included
 *  in both QueryServerSettings, and SaveCustomizeSettings.
 *  Saves session manager settings to the named resource file.
 *
 *  Inputs:
 *  ------
 *  smBase = pointer to newly opened resource db used to store setting info
 *  resSpec = a buffer to hold string resource information until it is
 *            printed.
 *  smSettings = place where settings are stored.
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
PrintSessionInfo( smBase)
        XrmDatabase *smBase ;
#else
PrintSessionInfo(
        XrmDatabase *smBase)
#endif /* _NO_PROTO */
{
    char tmpChar[20];

    /*
     * Write out the settings for logout confirmation and
     * the correct session to return to
     */
    strcpy(resSpec, SM_RESOURCE_CLASS);
    strcat(resSpec, "*");
    strcat(resSpec, SmCshutDownState);
    strcat(resSpec, ":");
    sprintf(tmpChar, "%d",smSettings.confirmMode);
    strcat(resSpec, tmpChar);
    XrmPutLineResource(smBase, resSpec);
    
    strcpy(resSpec, SM_RESOURCE_CLASS);
    strcat(resSpec, "*");
    strcat(resSpec, SmCshutDownMode);
    strcat(resSpec, ":");
    sprintf(tmpChar, "%d",smSettings.startState);
    strcat(resSpec, tmpChar);
    XrmPutLineResource(smBase, resSpec);

    return;
}


/*************************************<->*************************************
 *
 *  PrintScreenSaver (smBase)
 *
 *
 *  Description:
 *  -----------
 *  A convenience function that is separated out instead of being included
 *  in both QueryServerSettings, and SaveCustomizeSettings.
 *  Saves screen saver information to the named resource file.
 *
 *  Inputs:
 *  ------
 *  smBase = pointer to newly opened resource db used to store setting info
 *  resSpec = a buffer to hold string resource information until it is
 *            printed.
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
PrintScreenSaver( smBase)
        XrmDatabase *smBase ;
#else
PrintScreenSaver(
        XrmDatabase *smBase)
#endif /* _NO_PROTO */
{
    Status		newScreenStat;
    char 		tmpChar[20];

    if(smCust.screenSavChange == True)
    {
	newScreenStat = _DtGetSmScreen(smGD.display, smGD.topLevelWindow,
				      XaDtSmScreenInfo, &screenSaverVals);
	/*
	 *  If the property has been deleted, it means that we return to
	 *  the default
	 */
	if(newScreenStat != Success)
	{
	    return;
	}
    }

    strcpy(resSpec, SM_RESOURCE_CLASS);
    strcat(resSpec, "*");
    strcat(resSpec, SmCtimeout);
    strcat(resSpec, ":");
    sprintf(tmpChar, "%d", screenSaverVals.smTimeout);
    strcat(resSpec, tmpChar);
    XrmPutLineResource(smBase, resSpec);
    
    strcpy(resSpec, SM_RESOURCE_CLASS);
    strcat(resSpec, "*");
    strcat(resSpec, SmCinterval);
    strcat(resSpec, ":");
    sprintf(tmpChar, "%d", screenSaverVals.smInterval);
    strcat(resSpec, tmpChar);
    XrmPutLineResource(smBase, resSpec);
    
    strcpy(resSpec, SM_RESOURCE_CLASS);
    strcat(resSpec, "*");
    strcat(resSpec, SmCpreferBlank);
    strcat(resSpec, ":");
    sprintf(tmpChar, "%d", screenSaverVals.smPreferBlank);
    strcat(resSpec, tmpChar);
    XrmPutLineResource(smBase, resSpec);
    
    strcpy(resSpec, SM_RESOURCE_CLASS);
    strcat(resSpec, "*");
    strcat(resSpec, SmCallowExp);
    strcat(resSpec, ":");
    sprintf(tmpChar, "%d", screenSaverVals.smAllowExp);
    strcat(resSpec, tmpChar);
    XrmPutLineResource(smBase, resSpec);

    return;
}


/*************************************<->*************************************
 *
 *  PrintFontPaths (smBase, fontPaths, numPaths)
 *
 *
 *  Description:
 *  -----------
 *  A convenience function that is separated out instead of being included
 *  in both QueryServerSettings, and SaveCustomizeSettings.
 *  Saves pointer control information to the named resource file.
 *
 *  Inputs:
 *  ------
 *  smBase = pointer to newly opened resource db used to store setting info
 *  resSpec = a buffer to hold string resource information until it is
 *            printed
 *  fontPaths = font paths to be saved
 *  numPaths = number of font paths to be saved
 * 
 *  Outputs:
 *  -------
 *  resSize = made bigger if current buffer is too small to hold all font info
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static int 
#ifdef _NO_PROTO
PrintFontPaths( smBase, fontPath, numPaths)
        XrmDatabase *smBase ;
        char **fontPath ;
        int numPaths ;
#else
PrintFontPaths(
        XrmDatabase *smBase,
        char **fontPath,
        int numPaths)
#endif /* _NO_PROTO */
{
    int	i;
    
    /* If no fp save req'd, just pretend we did it */
    if (smRes.saveFontPath == False) return 0;

    strcpy(resSpec, SM_RESOURCE_CLASS);
    strcat(resSpec, "*");
    strcat(resSpec, SmCfontPath);
    strcat(resSpec, ":");
    strcat(resSpec, *fontPath);
    for(i = 1;i < numPaths;i++)
    {
	if((strlen(resSpec) + strlen(fontPath[i]) + 1) >= resSize)
	{
	    resSize += 5000;
	    resSpec = (char *) SM_REALLOC(resSpec, resSize * sizeof(char));
	    if (resSpec==NULL)
	    {
		PrintErrnoError(DtError, smNLS.cantMallocErrorString);
		return(-1);
	    }
	}
	strcat(resSpec, ",");
	strcat(resSpec, fontPath[i]);
    }
    XrmPutLineResource(smBase, resSpec);

    return(0);
}



/*************************************<->*************************************
 *
 *  PrintKeyboardControl (smBase)
 *
 *
 *  Description:
 *  -----------
 *  A convenience function that is separated out instead of being included
 *  in both QueryServerSettings, and SaveCustomizeSettings.
 *  Saves pointer control information to the named resource file.
 *
 *  Inputs:
 *  ------
 *  smBase = pointer to newly opened resource db used to store setting info
 *  resSpec = a buffer to hold string resource information until it is
 *            printed
 *  resSize = size of current resource buffer
 * 
 *  Outputs:
 *  -------
 *  resSize = buffer is enlarged if more room is need to hold this info
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static int 
#ifdef _NO_PROTO
PrintKeyboardControl( smBase)
        XrmDatabase *smBase ;
#else
PrintKeyboardControl(
        XrmDatabase *smBase)
#endif /* _NO_PROTO */
{
    int		i,j;
    char	bitTest;
    Bool	firstRepeat = True;
    char	tmpChar[20];
    
    strcpy(resSpec, SM_RESOURCE_CLASS);
    strcat(resSpec, "*");
    strcat(resSpec, SmCkeyClick);
    strcat(resSpec, ":");
    sprintf(tmpChar, "%d",smSettings.kbdState.key_click_percent);
    strcat(resSpec, tmpChar);
    XrmPutLineResource(smBase, resSpec);

    strcpy(resSpec, SM_RESOURCE_CLASS);
    strcat(resSpec, "*");
    strcat(resSpec, SmCbellPercent);
    strcat(resSpec, ":");
    sprintf(tmpChar, "%d",smSettings.kbdState.bell_percent);
    strcat(resSpec, tmpChar);
    XrmPutLineResource(smBase, resSpec);

    strcpy(resSpec, SM_RESOURCE_CLASS);
    strcat(resSpec, "*");
    strcat(resSpec, SmCbellPitch);
    strcat(resSpec, ":");
    sprintf(tmpChar, "%d",smSettings.kbdState.bell_pitch);
    strcat(resSpec, tmpChar);
    XrmPutLineResource(smBase, resSpec);

    strcpy(resSpec, SM_RESOURCE_CLASS);
    strcat(resSpec, "*");
    strcat(resSpec, SmCbellDuration);
    strcat(resSpec, ":");
    sprintf(tmpChar, "%d",smSettings.kbdState.bell_duration);
    strcat(resSpec, tmpChar);
    XrmPutLineResource(smBase, resSpec);
    
    strcpy(resSpec, SM_RESOURCE_CLASS);
    strcat(resSpec, "*");
    strcat(resSpec, SmCledMask);
    strcat(resSpec, ":");
    sprintf(tmpChar, "%d",smSettings.kbdState.led_mask);
    strcat(resSpec, tmpChar);
    XrmPutLineResource(smBase, resSpec);

    strcpy(resSpec, SM_RESOURCE_CLASS);
    strcat(resSpec, "*");
    strcat(resSpec, SmCglobalRepeats);
    strcat(resSpec, ":");
    sprintf(tmpChar, "%d",smSettings.kbdState.global_auto_repeat);
    strcat(resSpec, tmpChar);
    XrmPutLineResource(smBase, resSpec);

    strcpy(resSpec, SM_RESOURCE_CLASS);
    strcat(resSpec, "*");
    strcat(resSpec, SmCautoRepeats);
    strcat(resSpec, ":");

    /*
     * Now write out which keys need to be auto repeated one at a time
     */
    if(smSettings.kbdState.global_auto_repeat != AutoRepeatModeOn)
    {
	for(i = 0;i < 32;i++)
	{
	    bitTest = 0x01;
	    for(j = 0;j < 8;j++)
	    {
		if((bitTest & smSettings.kbdState.auto_repeats[i]) != 0)
		{
		    if(firstRepeat == True)
		    {
			firstRepeat = False;
		    }
		    else
		    {
			strcat(resSpec, ",");
		    }
		    
		    if((strlen(resSpec) + 5) >= resSize)
		    {
			resSize += 5000;
			resSpec = (char *) SM_REALLOC(resSpec,
						   resSize * sizeof(char));
			if (resSpec==NULL)
			{
			    PrintErrnoError(DtError,
					    smNLS.cantMallocErrorString);
			    return(-1);
			}
		    }
			
		    sprintf(tmpChar, "%d", ((8 * i) + j));
		    strcat(resSpec, tmpChar);
		}
		bitTest <<= 1;
	    }
	}
    }
    else
    {
	strcat(resSpec, "");
    }
    XrmPutLineResource(smBase, resSpec);

    return(0);
}



/*************************************<->*************************************
 *
 *  PrintPointerMapping (smBase, buttonRet,  numButton)
 *
 *
 *  Description:
 *  -----------
 *  A convenience function that is separated out instead of being included
 *  in both QueryServerSettings, and SaveCustomizeSettings.
 *  Saves pointer mapping information to the named resource file.
 *
 *  Inputs:
 *  ------
 *  smBase = pointer to newly opened resource db used to store setting info
 *  resSpec = a buffer to hold string resource information until it is
 *            printed
 *  resSize = size of resource buffer
 *  buttonRet = button mapping to be saved
 *  numButton = number of buttons in button mapping
 * 
 *  Outputs:
 *  -------
 *  resSize = size of buffer is change if it needs to be enlarged during
 *            routine
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static int 
#ifdef _NO_PROTO
PrintPointerMapping( smBase, buttonRet, numButton )
        XrmDatabase *smBase ;
        char *buttonRet ;
        int numButton ;
#else
PrintPointerMapping(
        XrmDatabase *smBase,
        char *buttonRet,
        int numButton )
#endif /* _NO_PROTO */
{
    int		i, numLoops;
    char	tmpChar[20];
    
    strcpy(resSpec, SM_RESOURCE_CLASS);
    strcat(resSpec, "*");
    strcat(resSpec, SmCbuttonMap);
    strcat(resSpec, ":");

    numLoops = numButton - 1;
    for(i = 0;i <= numLoops;i++)
    {
	if((strlen(resSpec) + 5) >= resSize)
	{
	    resSize += 5000;
	    resSpec = (char *) SM_REALLOC(resSpec, resSize * sizeof(char));
	    if (resSpec==NULL)
	    {
		PrintErrnoError(DtError, smNLS.cantMallocErrorString);
		return(-1);
	    }
	}
	    
	sprintf(tmpChar, "%d",buttonRet[i]);
	strcat(resSpec, tmpChar);
	if(i != numLoops)
	{
	    strcat(resSpec, ",");
	}
	XrmPutLineResource(smBase, resSpec);
    }

    return(0);
}



/*************************************<->*************************************
 *
 *  PrintWorkHintString (hintFile, hintString)
 *
 *
 *  Description:
 *  -----------
 *  Print the workspace hint string - backquote all unquoted spaces and
 *  backquotes - leave quoted backspaces and quotes alone - quote all
 *  quotes 2 levels deep
 *
 *
 *  Inputs:
 *  ------
 *  hintFile = pointer to the output file
 *  hintString = character string to be printed
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
PrintWorkHintString( hintFile, hintString )
        FILE *hintFile ;
        char *hintString ;
#else
PrintWorkHintString(
        FILE *hintFile,
        char *hintString )
#endif /* _NO_PROTO */
{
    char 	*curP;
    int		chlen;
    
    curP = hintString;
    chlen = 1;

    while(*curP != NULL && chlen > 0)
    {
#ifdef MULTIBYTE
	chlen = mblen (curP, MB_CUR_MAX);
	if ((chlen == 1) && (*curP == '"'))
	{
	    /*
	     * Quoted string - double quote it
	     */
	    fprintf(hintFile, "\\\\\\\\\\\\\\\"");
	    curP++;
	    
	    /*
	     * Start beyond double quote and find the end of the quoted string.
	     * Print every character along the way - quote backquotes
	     */
	    while ((*curP != NULL) &&
		   ((chlen = mblen (curP, MB_CUR_MAX)) > 0) &&
		   ((chlen > 1) || (*curP != '"')))
	    {

		if ((chlen == 1) && ((*curP == '\\') || (*curP == ' ')) && 
		    ((chlen = mblen (curP, MB_CUR_MAX)) > 0))
		    /*
		     * if we have a blank or blackslash - quote and write
		     */
		{
		    fprintf(hintFile, "\\%c", *curP);
		}
		else
		{
		    fprintf(hintFile, "%c", *curP);
		}
		curP++;

		if(chlen > 1)
		    /* Multibyte (nonwhitespace) character
		     * Finish character byte copy.
		     */
		{
		    while (--chlen)
		    {
			fprintf(hintFile, "%c", *curP);
			*curP++;
		    }
		}
	    }

	    /*
	     *  Found matching quote or NULL.
	     *  Finish printing quoted string
	     */
	    if(*curP != NULL)
	    {
		if(((chlen = mblen (curP, MB_CUR_MAX)) > 0) &&
		    ((chlen > 1) || (*curP == '"')))
		{
		    fprintf(hintFile, "\\\\\\\\\\\\\\\"");
		}
		curP++;
	    }
	}
	else
	{
	    /* 
	     * Find the end of the nonquoted string.
	     * Quote spaces and backslashes
	     */

	    while ((*curP != NULL) &&
		   ((chlen = mblen (curP, MB_CUR_MAX)) > 0) &&
		   ((chlen > 1) || (*curP != '"')))
	    {
		if ((chlen == 1) && ((*curP == '\\') || (*curP == ' ')) && 
		    ((chlen = mblen (curP, MB_CUR_MAX)) > 0))
		    /*
		     * if we have a blank or blackslash - quote and write
		     */
		{
		    fprintf(hintFile, "\\%c", *curP);
		}
		else
		{
		    fprintf(hintFile, "%c", *curP);
		}
		curP++;

		if (chlen > 1)
		    /* Multibyte character:  finish character print. */
		{
		    while (--chlen)
		    {
			fprintf(hintFile, "%c", *curP);
			curP++;
		    }
		}
	    }
	}
#else

	if (*curP == '"')
	    /* Quoted string */
	{
	    /*
	     * Print out the first quote
	     */
	    fprintf(hintFile, "\\\\\\\\\\\\\\\"");
	    curP++;
	    
	    /*
	     * Start beyond double quote and find the end of the quoted string.
	     */
	    while ((*curP != NULL) && (*curP != '"'))
		/* haven't found matching quote yet */
	    {
		/* print char and point curP to next character */
		if (((*curP == '\\') || (*curP == ' ')) && (*curP != NULL))
		    /* shift quoted nonNULL character down and curP ahead */
		{
		    fprintf(hintFile, "\\%c", *curP);
		}
		else
		{	
		    fprintf(hintFile, "%c", *curP);
		}
		curP++;
	    }

	    /*
	     *  Found matching quote or NULL.
	     *  Finish printing quoted string
	     */
	    if(*curP != NULL)
	    {
		if(*curP == '"')
		{
		    fprintf(hintFile, "\\\\\\\\\\\\\\\"");
		}
		curP++;
	    }
	}
	else
	{
	    /* 
	     * Find the end of the nonquoted string.
	     * Quote spaces and backslashes
	     */

	    while ((*curP != NULL) && (*curP != '"'))
	    {
		/* point curP to next character */
		if (((*curP == '\\') || (*curP == ' ')) && (*curP != NULL))
		    /* shift quoted nonNULL character down and curP ahead */
		{
		    fprintf(hintFile, "\\%c", *curP);
		}
		else
		{	
		    fprintf(hintFile, "%c", *curP);
		}
		curP++;
	    }
	}
#endif
    }
    return;
} /* END OF FUNCTION PrintWorkHintString */


/*************************************<->*************************************
 *
 *  PrintCmdHintString (hintFile, hintString)
 *
 *
 *  Description:
 *  -----------
 *  Print the workspace hint string - backquote all unquoted spaces and
 *  backquotes - leave quoted backspaces and quotes alone - quote all
 *  quotes 4 levels deep
 *
 *
 *  Inputs:
 *  ------
 *  hintFile = pointer to the output file
 *  hintString = character string to be printed
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
PrintCmdHintString( hintFile, hintString )
        FILE *hintFile ;
        char *hintString ;
#else
PrintCmdHintString(
        FILE *hintFile,
        char *hintString )
#endif /* _NO_PROTO */
{
    char 	*curP;
    int		chlen;

    curP = hintString;
    chlen = 1;

    while(*curP != NULL && chlen > 0)
    {
#ifdef MULTIBYTE
	chlen = mblen (curP, MB_CUR_MAX);
	if ((chlen == 1) && (*curP == '"'))
	{
	    /*
	     * Quoted string - quadruple quote it
	     */
	    fprintf(hintFile, "\\\\\\\\\\\\\\\"");
	    curP++;
	    
	    /*
	     * Start beyond double quote and find the end of the quoted string.
	     * Print every character along the way - quote backquotes
	     */
	    while ((*curP != NULL) &&
		   ((chlen = mblen (curP, MB_CUR_MAX)) > 0) &&
		   ((chlen > 1) || (*curP != '"')))
	    {

		if ((chlen == 1) && ((*curP == '\\') || (*curP == ' ')) && 
		    ((chlen = mblen (curP, MB_CUR_MAX)) > 0))
		{
		    fprintf(hintFile, "\\%c", *curP);
		}
		else
		{
		    fprintf(hintFile, "%c", *curP);
		}
		curP++;

		if(chlen > 1)
		    /* Multibyte (nonwhitespace) character
		     * Finish character byte copy.
		     */
		{
		    while (--chlen)
		    {
			fprintf(hintFile, "%c", *curP);
			*curP++;
		    }
		}
	    }

	    /*
	     *  Found matching quote or NULL.
	     *  Finish printing quoted string
	     */
	    if(*curP != NULL)
	    {
		if(((chlen = mblen (curP, MB_CUR_MAX)) > 0) &&
		    ((chlen > 1) || (*curP == '"')))
		{
		    fprintf(hintFile, "\\\\\\\\\\\\\\\"");
		}
		curP++;
	    }
	}
	else
	{
	    /* 
	     * Find the end of the nonquoted string.
	     * Quote spaces and backslashes
	     */

	    while ((*curP != NULL) &&
		   ((chlen = mblen (curP, MB_CUR_MAX)) > 0) &&
		   ((chlen > 1) || (*curP != '"')))
	    {
		if ((chlen == 1) && ((*curP == '\\') || (*curP == ' ')) && 
		    ((chlen = mblen (curP, MB_CUR_MAX)) > 0))
		{
		    fprintf(hintFile, "\\%c", *curP);
		}
		else
		{
		    fprintf(hintFile, "%c", *curP);
		}
		curP++;

		if (chlen > 1)
		    /* Multibyte character:  finish character print. */
		{
		    while (--chlen)
		    {
			fprintf(hintFile, "%c", *curP);
			curP++;
		    }
		}
	    }
	}
#else

	if (*curP == '"')
	    /* Quoted string */
	{
	    /*
	     * Print out the first quote
	     */
	    fprintf(hintFile, "\\\\\\\\\\\\\\\"");
	    curP++;
	    
	    /*
	     * Start beyond double quote and find the end of the quoted string.
	     */
	    while ((*curP != NULL) && (*curP != '"'))
		/* haven't found matching quote yet */
	    {
		/* print char and point curP to next character */
		if (((*curP == '\\') || (*curP == ' ')) && (*curP != NULL))
		{
		    fprintf(hintFile, "\\%c", *curP);
		}
		else
		{	
		    fprintf(hintFile, "%c", *curP);
		}
		curP++;
	    }

	    /*
	     *  Found matching quote or NULL.
	     *  Finish printing quoted string
	     */
	    if(*curP != NULL)
	    {
		if(*curP == '"')
		{
		    fprintf(hintFile, "\\\\\\\\\\\\\\\"");
		}
		curP++;
	    }
	}
	else
	{
	    /* 
	     * Find the end of the nonquoted string.
	     * Quote spaces and backslashes
	     */

	    while ((*curP != NULL) && (*curP != '"'))
	    {
		/* point curP to next character */
		if (((*curP == '\\') || (*curP == ' ')) && (*curP != NULL))
		{
		    fprintf(hintFile, "\\%c", *curP);
		}
		else
		{	
		    fprintf(hintFile, "%c", *curP);
		}
		curP++;
	    }
	}
#endif
    }
    return;
} /* END OF FUNCTION PrintCmdHintString */


/*************************************<->*************************************
 *
 *  PrintCmdString (cmdFile, cmdString)
 *
 *
 *  Description:
 *  -----------
 *  Takes the string passed in and prints it out to the passed in file.
 *  However, it double quotes any quotes (" becomes \"), single
 *  quotes any quoted spaces and backslashes, and double quotes any unquoted
 *  spaces and backslashes
 *
 *
 *  Inputs:
 *  ------
 *  cmdFile = pointer to the output file
 *  cmdString = character string to be printed
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
PrintCmdString( cmdFile, cmdString )
        FILE *cmdFile ;
        char *cmdString ;
#else
PrintCmdString(
        FILE *cmdFile,
        char *cmdString )
#endif /* _NO_PROTO */
{
    char 	*curP;
    int		chlen;

    curP = cmdString;
    chlen = 1;

    while(*curP != NULL && chlen > 0)
    {
#ifdef MULTIBYTE
	chlen = mblen(curP, MB_CUR_MAX);
	if ((chlen == 1) && (*curP == '"'))
	{
	    /*
	     * Quoted string -  quote it
	     */
	    fprintf(cmdFile, "\\\\\\\"");
	    curP++;
	    
	    /*
	     * Start beyond double quote and find the end of the quoted string.
	     * Print every character along the way
	     */
	    while ((*curP != NULL) &&
		   ((chlen = mblen (curP, MB_CUR_MAX)) > 0) &&
		   ((chlen > 1) || (*curP != '"')))
	    {
		if ((chlen == 1) && ((*curP == '\\') || (*curP == ' ')) && 
		    ((chlen = mblen (curP, MB_CUR_MAX)) > 0))
		{
		    /*
		     *if we have a blank or blackslash - quote and write
		     */
		    fprintf(cmdFile, "\\%c", *curP);
		}
		else
		{
		    fprintf(cmdFile, "%c", *curP);
		}
		curP++;

		if(chlen > 1)
		    /* Multibyte (nonwhitespace) character
		     * Finish character byte copy.
		     */
		{
		    while (--chlen)
		    {
			fprintf(cmdFile, "%c", *curP);
			*curP++;
		    }
		}
	    }

	    /*
	     *  Found matching quote or NULL.
	     *  Finish printing quoted string
	     */
	    if(*curP != NULL)
	    {
		if(((chlen = mblen (curP, MB_CUR_MAX)) > 0) &&
		    ((chlen > 1) || (*curP == '"')))
		{
		    fprintf(cmdFile, "\\\\\\\"");
		}
		curP++;
	    }
	}
	else
	{
	    /* 
	     * Find the end of the nonquoted string.
	     * Quote spaces and backslashes
	     */

	    while ((*curP != NULL) &&
		   ((chlen = mblen (curP, MB_CUR_MAX)) > 0) &&
		   ((chlen > 1) || (*curP != '"')))
	    {
		if ((chlen == 1) && ((*curP == '\\') || (*curP == ' ')) && 
		    ((chlen = mblen (curP, MB_CUR_MAX)) > 0))
		{
		    /*
		     *if we have a blank or blackslash - double quote and write
		     */
		    fprintf(cmdFile, "\\%c", *curP);
		}
		else
		{
		    fprintf(cmdFile, "%c", *curP);
		}
		curP++;

		if (chlen > 1)
		    /* Multibyte character:  finish character print. */
		{
		    while (--chlen)
		    {
			fprintf(cmdFile, "%c", *curP);
			curP++;
		    }
		}
	    }
	}
#else

	if (*curP == '"')
	    /* Quoted string */
	{
	    /*
	     * Print out the first quote
	     */
	    fprintf(cmdFile, "\\\\\\\"");
	    curP++;
	    
	    /*
	     * Start beyond double quote and find the end of the quoted string.
	     */
	    while ((*curP != NULL) && (*curP != '"'))
		/* haven't found matching quote yet */
	    {
		/* print char and point curP to next character */
		if (((*curP == '\\') || (*curP == ' ')) && (*curP != NULL))
		{
		    fprintf(cmdFile, "\\%c", *curP);
		}
		else
		{	
		    fprintf(cmdFile, "%c", *curP);
		}
		curP++;
	    }

	    /*
	     *  Found matching quote or NULL.
	     *  Finish printing quoted string
	     */
	    if(*curP != NULL)
	    {
		if(*curP == '"')
		{
		    fprintf(cmdFile, "\\\\\\\"");
		}
		curP++;
	    }
	}
	else
	{
	    /* 
	     * Find the end of the nonquoted string.
	     * Quote spaces and backslashes
	     */
	    while ((*curP != NULL) && (*curP != '"'))
	    {
		/* point curP to next character */
		if (((*curP == '\\') || (*curP == ' ')) && (*curP != NULL))
		{
		    fprintf(cmdFile, "\\%c", *curP);
		}
		else
		{	
		    fprintf(cmdFile, "%c", *curP);
		}
		curP++;
	    }
	}
#endif
    }
    return;
} /* END OF FUNCTION PrintCmdString */


/*************************************<->*************************************
 *
 *  PrintRemoteCmdString (cmdFile, cmdString)
 *
 *
 *  Description:
 *  -----------
 *  Prints command strings that the command invoker can understand.  It
 *  backslashes backslashes, double quotes, spaces, and single quotes
 *
 *
 *  Inputs:
 *  ------
 *  cmdFile = pointer to the output file
 *  cmdString = character string to be printed
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
PrintRemoteCmdString( cmdFile, cmdString )
        FILE *cmdFile ;
        char *cmdString ;
#else
PrintRemoteCmdString(
        FILE *cmdFile,
        char *cmdString )
#endif /* _NO_PROTO */
{
    char 	*curP;
    int		chlen;

    curP = cmdString;

#ifdef MULTIBYTE
    /*
     * Find all occurrences of the "special characters and give them
     * the appropriate number of backslashes
     */
    while ((*curP != NULL) &&
	   ((chlen = mblen (curP, MB_CUR_MAX)) > 0))
    {
	if ((chlen == 1) &&
	    ((*curP == '\\') || (*curP == ' ') || (*curP == '\'')
	     || (*curP == '"')) && 
	    ((chlen = mblen (curP, MB_CUR_MAX)) > 0))
	{
	    /*
	     *if we have a blank or blackslash - double quote and write
	     */
	    if(*curP == '"')
	    {
		fprintf(cmdFile, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\%c", *curP);
	    }
	    else
	    {
		fprintf(cmdFile, "\\\\\\%c", *curP);
	    }
	}
	else
	{
	    fprintf(cmdFile, "%c", *curP);
	}
	curP++;
	
	if (chlen > 1)
	    /* Multibyte character:  finish character print. */
	{
	    while (--chlen)
	    {
		fprintf(cmdFile, "%c", *curP);
		curP++;
	    }
	}
    }
#else
    /* 
     * Find end of string 
     * 
     */
    while(*curP != NULL)
    {
	/* point curP to next character */
	if (((*curP == '\\') || (*curP == ' ')
	     || (*curP == '\'') || (*curP == '"')) && (*curP != NULL))
	{
	    if(*curP == '"')
	    {
		fprintf(cmdFile, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\%c", *curP);
	    }
	    else
	    {
		fprintf(cmdFile, "\\\\\\%c", *curP);
	    }
	}
	else
	{	
	    fprintf(cmdFile, "%c", *curP);
	}
	curP++;
    }
#endif
    return;
} /* END OF FUNCTION PrintCmdString */
