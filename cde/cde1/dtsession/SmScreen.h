/* $XConsortium: SmScreen.h /main/cde1_maint/2 1995/08/30 16:33:41 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        SmScreen.h
 **
 **   Project:     HP DT
 **
 **   Description
 **   -----------
 **   Variables and declarations needed for
 **   managing external screen savers
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990.  
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _smscreen_h
#define _smscreen_h
 
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
extern void StartScreenSaver();
extern void StopScreenSaver();
extern void *SmSaverParseSaverList();
#else
extern void StartScreenSaver(void);
extern void StopScreenSaver(void);
extern void *SmSaverParseSaverList(char *);
#endif /* _NO_PROTO */


#endif /*_smscreen_h*/
/* DON'T ADD ANYTHING AFTER THIS #endif */
