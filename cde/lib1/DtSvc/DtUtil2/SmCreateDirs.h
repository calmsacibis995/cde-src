/* $XConsortium: SmCreateDirs.h /main/cde1_maint/1 1995/07/14 20:38:41 drk $ */
/*                                                                            *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                           *
 * (c) Copyright 1993, 1994 International Business Machines Corp.             *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                            *
 * (c) Copyright 1993, 1994 Novell, Inc.                                      *
 */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:        SmCreateDirs.h
 **
 **  Description:  Header file for SmCreateDirs.c
 **  -----------
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

#ifndef _SmCreateDirs_h
#define _SmCreateDirs_h

/*
 *  include statements
 */

/*
 * define statements
 */

/*
 * typedef statements
 */

/*
 * Function definitions
 */
#ifdef _NO_PROTO

extern char * _DtCreateDtDirs() ;

#else

extern char * _DtCreateDtDirs(Display *display) ;

#endif

#endif /* _SmCreateDirs_h */
/* Do not add anything after this endif. */
