/* $XConsortium: SmHftRing.h /main/cde1_maint/2 1995/08/30 16:28:03 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        SmHftRing.h
 **
 **   Project:     DT Session Manager (dtsession)
 **
 **   Description
 **   -----------
 **   This file contains the AIX 3.2.x specific declarations required to
 **   enable and disable the HFT ring. This is used for session locking.
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _smhftring_h
#define _smhftring_h
 
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
extern void AixEnableHftRing();
#else
extern void AixEnableHftRing(int);
#endif /* _NO_PROTO */


#endif /*_smhftring_h*/
/* DON'T ADD ANYTHING AFTER THIS #endif */
