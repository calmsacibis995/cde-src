/* $XConsortium: SmLock.h /main/cde1_maint/2 1995/08/30 16:28:41 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        SmLock.h
 **
 **   Project:     HP DT
 **
 **   Description
 **   -----------
 **   Variables and declarations needed for
 **   Lock Handling for the session manager
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990.  
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _smlock_h
#define _smlock_h
 
/* 
 *  #include statements 
 */


/* 
 *  #define statements 
 */

/*
 * Type of lock states possible during session
 */
#define		UNLOCKED		0
#define 	LOCKED			1


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

extern void LockDisplay() ;
extern void CreateLockCursor() ;

#else

extern void LockDisplay( Boolean locknow ) ;
extern void CreateLockCursor( void ) ;

#endif /* _NO_PROTO */


#endif /*_smlock_h*/
/* DON'T ADD ANYTHING AFTER THIS #endif */
