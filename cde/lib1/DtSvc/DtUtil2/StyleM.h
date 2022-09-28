/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/* $XConsortium: StyleM.h /main/cde1_maint/1 1995/07/14 20:39:10 drk $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:        StyleM.h
 **
 **  Project:     HP DT Style Manager (dtstyle)
 **
 **  Description:
 **  -----------
 **  Contains all variables needed for BMS messaging.  All messages serviced,
 **  and the tool class name for the style manager
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

#ifdef _NO_PROTO
extern Window XmuClientWindow();
#else
extern Window XmuClientWindow( 
                        Display *dpy,
                        Window win) ;
#endif

/*
 * Tool class for the session manager
 */
#define DtSTYLE_TOOL_CLASS "STYLEMGR"


/*
 * Request types that the style manager services
 */
/*I don't think there will be any here*/
