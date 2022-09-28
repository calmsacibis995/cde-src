/* $XConsortium: SmSave.h /main/cde1_maint/2 1995/08/30 16:33:04 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        SmSave.h
 **
 **   Project:     HP DT
 **
 **   Description
 **   -----------
 **   Variables and declarations needed for
 **   Session Save for the session manager
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990.  
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _smsave_h
#define _smsave_h
 
/* 
 *  #include statements 
 */
#include <Dt/SessionP.h>
#include <Dt/SessionM.h>


/* 
 *  #define statements 
 */

/* 
 * typedef statements 
 */

/*
 *  External variables  
 */
extern SmScreenInfo        screenSaverVals;
extern SmAudioInfo         audioVals;
extern SmKeyboardInfo      keyboardVals;

/*  
 *  External Interface  
 */
#ifdef _NO_PROTO

extern int SaveState() ;

#else

extern int SaveState( Boolean, int ) ;

#endif /* _NO_PROTO */

#endif /*_smsave_h*/
/* DON'T ADD ANYTHING AFTER THIS #endif */
