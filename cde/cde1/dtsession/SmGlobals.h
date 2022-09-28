/* $XConsortium: SmGlobals.h /main/cde1_maint/3 1995/10/09 10:57:08 pascale $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        SmGlobals.h
 **
 **   Project:     HP DT
 **
 **   Description
 **   -----------
 **   Variables and declarations needed for
 **   the global processing routines for the session manager
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990.  
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _smglobals_h
#define _smglobals_h
 
/* 
 *  #include statements 
 */


/* 
 *  #define statements 
 */

#define SmNewString(str) strdup(str)

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

extern void WaitChildDeath();
extern void InitSMGlobals() ;
extern int SetRestorePath() ;
extern int SetSavePath() ;
extern int SetFontSavePath() ;
extern void InitNlsStrings() ;
extern void MoveDirectory() ;
extern void SetSystemReady();

#else

extern void WaitChildDeath(int i);
extern void InitSMGlobals( void ) ;
extern int SetRestorePath( unsigned int , char **) ;
extern int SetSavePath( Boolean , int ) ;
extern int SetFontSavePath(char *) ;
extern void InitNlsStrings( void ) ;
extern void MoveDirectory( char *, char *) ;
extern void SetSystemReady();

#endif /* _NO_PROTO */

#endif /*_smglobals_h*/
/* DON'T ADD ANYTHING AFTER THIS #endif */
