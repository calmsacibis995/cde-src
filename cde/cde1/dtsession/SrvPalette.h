/* $XConsortium: SrvPalette.h /main/cde1_maint/2 1995/08/30 16:39:24 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        SrvPalette.h
 **
 **   Project:     HP DT
 **
 **   Description
 **   -----------
 **   Variables and declarations needed for
 **   Session Restoration for the session manager
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990.  
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _srvpalette_h
#define _srvpalette_h
 
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

extern int InitializeDtcolor() ;
extern int CheckMonitor() ;

#else

extern int InitializeDtcolor( Display *, short ) ;
extern int CheckMonitor( Display *) ;

#endif /* _NO_PROTO */

#endif /*_srvpalette_h*/
/* DON'T ADD ANYTHING AFTER THIS #endif */
