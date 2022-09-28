/* $XConsortium: SmConMgmt.h /main/cde1_maint/2 1995/08/30 16:25:51 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        SmConMgmt.h
 **
 **   Project:     HP DT
 **
 **   Description
 **   -----------
 **   Variables and declarations needed for
 **   Contention Management for the session manager
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990.  
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _smconmgmt_h
#define _smconmgmt_h
 
/* 
 *  #include statements 
 */


/* 
 *  #define statements 
 */
#define		MEM_NOT_AVAILABLE	-1
#define		MEM_NOT_FULL		0
#define		MEM_FULL		1


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

extern int GetMemoryUtilization();
extern void WaitForClientMap();

#else

extern int GetMemoryUtilization(void);
extern void WaitForClientMap(void);

#endif /*_NO_PROTO*/

#endif /*_smprotocols_h*/
/* DON'T ADD ANYTHING AFTER THIS #endif */
