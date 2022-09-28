/* $XConsortium: SmXrm.h /main/cde1_maint/2 1995/08/30 16:36:48 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        SmXrm.h
 **
 **   Project:     DT
 **
 **   Description
 **   -----------
 **   Variables and declarations needed for
 **   Xrm database management
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _smxrm_h
#define _smxrm_h

/*
 *  #include statements
 */


/*
 *  #define statements
 */

/*
 * typedef statements
 */

/*
 *  External variables
 */

/*
 *  External Interface
 */
#ifdef _NO_PROTO

XrmDatabase SmXrmSubtractDatabase();

#else

XrmDatabase SmXrmSubtractDatabase(
  XrmDatabase source_db,
  XrmDatabase target_db);

#endif

#endif /*_smxrm_h*/
/* DON'T ADD ANYTHING AFTER THIS #endif */
