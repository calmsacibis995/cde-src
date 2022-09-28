/* $XConsortium: SmProperty.c /main/cde1_maint/2 1995/08/30 16:30:04 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:        SmProperty.c
 **
 **  Project:     HP DT Session Manager (dtsession)
 **
 **  Description:
 **  -----------
 **  This file contains routines that deal with the properties used by the
 **  session manager to save and restore client information.
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
#include <X11/Intrinsic.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include "Sm.h"
#include "SmProtocol.h"



/*************************************<->*************************************
 *
 *  GetStandardProperties (window, iconHints, numHints, argc, argv
 *			   hints,clientMachine, workspaces, numWorkspaces)
 *
 *
 *  Description:
 *  -----------
 *  Handles all toolkit warnings
 *
 *
 *  Inputs:
 *  ------
 *  window = window for which we are getting properties
 *
 * 
 *  Outputs:
 *  -------
 *  argv = data returned from WM_COMMAND property (to restart client)
 *  argc = number of arguments returned from WM_COMMAND property
 *  hints = geometry of client
 *  clientMachine = which machine is client running on
 *  workspaces = which workspaces the client is running in
 *  numWorkspaces = number of workspaces specified in workspaces field
 *  iconWindow = window id of the client's icon window
 *
 *  Comments:
 *  --------
 *  All X (except GetWMHints) were not available until R4 and therefore have
 *  R4 or greater dependencies.
 *
 *  BEWARE OF THESE ROUTINES:  The XGetWindowProperty routine returns 0 if
 *  it succeeds.  These routines (which were derived from XGetWindowProperty
 *  return 0 if they FAIL.
 * 
 *************************************<->***********************************/
Status 
#ifdef _NO_PROTO
GetStandardProperties( window, argc, argv, hints, clientMachine, 
		       workspaces, numWorkspaces, iconWindow )
        Window window ;
        int *argc ;
        char ***argv ;
        XSizeHints *hints ;
        char **clientMachine ;
        unsigned char **workspaces ;
        unsigned long *numWorkspaces ;
        Window *iconWindow ;
#else
GetStandardProperties(
        Window window,
        int *argc,
        char ***argv,
        XSizeHints *hints,
        char **clientMachine,
        unsigned char **workspaces,
        unsigned long *numWorkspaces,
	Window *iconWindow )
#endif /* _NO_PROTO */
{
  int 		cc;
  long		suppliedRet;
  XTextProperty	sessProp;
  Atom		actType;
  int		actFormat;
  unsigned long	bytesAfter;
  
  /*
   * Get WM_COMMAND property
   */
  if ((cc=XGetCommand(smGD.display,window,argv,argc))==0)
  {
      return(cc);
  }

  /*
   * if there is no argc or argv - don't bother going on.  We're not
   * going to save anything anyway
   */
  if(*argc == 0)
  {
      return(0);
  }

  /*
   * Get WM_CLIENT_MACHINE property
   */
  if ((cc=XGetWMClientMachine(smGD.display,window,&sessProp))==0)
  {
      *clientMachine = NULL;
  }
  else
  {
      *clientMachine =  (char *) sessProp.value;
  }

  /*
   * Get the window geometry
   */
  XGetWMNormalHints(smGD.display, window, hints, &suppliedRet);

  /*
   * If dtwm is running get the workspaces the client is running in
   */
  if(smGD.dtwmRunning == True)
  {
      if(XGetWindowProperty(smGD.display, window, XaWsmPresence, 0L,
			 (long) BUFSIZ, False, XaWsmPresence, &actType,
			 &actFormat, numWorkspaces, &bytesAfter, workspaces)
	 != Success)
      {
	  *workspaces = NULL;
	  *numWorkspaces = 0;
      }
  }
  else
  {
      *workspaces = NULL;
      *numWorkspaces = 0;
  }

  /*
   * Get the icon window
   */
  {
      Atom          wmState, actual_type;
      unsigned long nitems, leftover;
      PropWMState  *property;
      int actual_format;

      *iconWindow = (Window) 0;
      
      wmState = XInternAtom(smGD.display, "WM_STATE", False);

      if (XGetWindowProperty (smGD.display, window, wmState, 0L,
			      PROP_WM_STATE_ELEMENTS, False,
			      wmState, &actual_type, &actual_format, &nitems,
			      &leftover, (unsigned char **)&property)
	  != Success)
      {
	  if (property)
	  {
	      XFree(property);
	      property = NULL;
	  }

	  *iconWindow = (Window) 0;
	  return(1);
      }

      if (actual_type != wmState)
      {
	  if (property)
	  {
	      XFree(property);
	      property = NULL;
	  }

	  *iconWindow = (Window) 0;
	  return(1);
      }

      if (property)
      {
	  *iconWindow = property->icon;
	  XFree(property);
      }
  }
  
  return(1);
}
