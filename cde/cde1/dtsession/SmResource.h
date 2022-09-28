/* $XConsortium: SmResource.h /main/cde1_maint/2 1995/08/30 16:31:48 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:        SmResource.h
 **
 **  Project:     HP DT Session Manager (dtsession)
 **
 **  Description:
 **  -----------
 **  Contains all resource names and classes for the session manager
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

/*
 * Global resource names
 */
extern char SmNsessionVersion[];

/*
 * Session Manager Resource Names
 */
/* Resources names for session manager behavior*/
extern char SmNwmStartup[];
extern char SmNquerySettings[];
extern char SmNkeys[];
extern char SmNalarmTime[];
extern char SmNmemThreshold[];
extern char SmNcontManagement[];
extern char SmNwaitClientTimeout[];
extern char SmNwaitWmTimeout[];
extern char SmNuseMessaging[];
extern char SmNcycleTimeout[];
extern char SmNlockTimeout[];
extern char SmNsaverTimeout[];
extern char SmNrandom[];
extern char SmNsaverList[];
extern char SmNsaveYourselfTimeout[];
extern char SmNmergeXdefaults[];


/* Resource names for settings information */

extern char SmNkeyClick[];
extern char SmNbellPercent[];
extern char SmNbellPitch[];
extern char SmNbellDuration[];
extern char SmNledMask[];
extern char SmNglobalRepeats[];
extern char SmNautoRepeats[];
extern char SmNaccelNum[];
extern char SmNaccelDenom[];
extern char SmNthreshold[];
extern char SmNtimeout[];
extern char SmNinterval[];
extern char SmNpreferBlank[];
extern char SmNallowExp[];
extern char SmNfontPath[];
extern char SmNkeySymsPerKey[];
extern char SmNnumKeyCode[];
extern char SmNkeySyms[];
extern char SmNmaxKeyPerMod[];
extern char SmNmodMap[];
extern char SmNbuttonMap[];
extern char SmNhostState[];
extern char SmNhostNames[];
extern char SmNdidQuerySettings[];
extern char SmNshutDownState[];
extern char SmNshutDownMode[];
extern char SmNdisplayResolution[];
extern char SmNsessionLang[];
extern char SmNsaveFontPath[];



/*
 * Global resource classes
 */
extern char SmCsessionVersion[];

/*
 * Session Manager Resource Classes
 */
/* Resources classes for session manager behavior*/
extern char SmCwmStartup[];
extern char SmCquerySettings[];
extern char SmCkeys[];
extern char SmCalarmTime[];
extern char SmCmemThreshold[];
extern char SmCContManagement[];
extern char SmRContManagement[];
extern char SmCWaitClientTimeout[];
extern char SmCWaitWmTimeout[];
extern char SmCUseMessaging[];
extern char SmCcycleTimeout[];
extern char SmClockTimeout[];
extern char SmCsaverTimeout[];
extern char SmCrandom[];
extern char SmCsaverList[];
extern char SmCsaveYourselfTimeout[];
extern char SmCmergeXdefaults[];


/*Class names for session settings information*/
extern char SmCkeyClick[];
extern char SmCbellPercent[];
extern char SmCbellPitch[];
extern char SmCbellDuration[];
extern char SmCledMask[];
extern char SmCglobalRepeats[];
extern char SmCautoRepeats[];
extern char SmCaccelNum[];
extern char SmCaccelDenom[];
extern char SmCthreshold[];
extern char SmCtimeout[];
extern char SmCinterval[];
extern char SmCpreferBlank[];
extern char SmCallowExp[];
extern char SmCfontPath[];
extern char SmCkeySymsPerKey[];
extern char SmCnumKeyCode[];
extern char SmCkeySyms[];
extern char SmCmaxKeyPerMod[];
extern char SmCmodMap[];
extern char SmCbuttonMap[];
extern char SmChostState[];
extern char SmChostNames[];
extern char SmCdidQuerySettings[];
extern char SmCshutDownState[];
extern char SmCshutDownMode[];
extern char SmCdisplayResolution[];
extern char SmCsessionLang[];
extern char SmCsaveFontPath[];



