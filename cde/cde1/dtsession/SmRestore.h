/* $XConsortium: SmRestore.h /main/cde1_maint/2 1995/08/30 16:32:26 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        SmRestore.h
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
#ifndef _smrestore_h
#define _smrestore_h
 
/* 
 *  #include statements 
 */

/* 
 *  #define statements 
 */
#define	MAX_REMOTE_CLIENTS	200
#define CONVERSION_SHELL        CDE_INSTALLATION_TOP "/bin/convertVS.sh"

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

extern void ReloadResources() ;
extern int RestoreState() ;
extern int SetCompatState() ;
extern int StartWM() ;
extern int RestoreResources() ;
extern void RestoreIndependentResources() ;
extern void StartEtc() ;
extern void CreateExecString() ;
extern void ForkClient() ;
extern void KillParent() ;
extern void ScanWhitespace() ;
#else

extern void ReloadResources( void ) ;
extern int RestoreState( void ) ;
extern int SetCompatState( void ) ;
extern int StartWM( void ) ;
extern int RestoreResources(Boolean, ...) ;
extern void RestoreIndependentResources(void) ;
extern void StartEtc( Boolean ) ;
extern void CreateExecString(char *) ;
extern void ForkClient( char **) ;
extern void KillParent( void ) ;
extern void ScanWhitespace( unsigned char **) ;

#endif /* _NO_PROTO */


#endif /*_smrestore_h*/
/* DON'T ADD ANYTHING AFTER THIS #endif */
