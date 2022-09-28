/* $XConsortium: SmXdef.h /main/cde1_maint/2 1995/08/30 16:36:05 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        SmXdef.h
 **
 **   Project:     DT
 **
 **   Description
 **   -----------
 **   Variables and declarations needed for
 **   Lock Handling for the session manager
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _smxdef_h
#define _smxdef_h
 
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

extern void SmXdefMerge();
extern XrmDatabase SmXdefSubtract();

#else

extern void SmXdefMerge(Display *display);
extern XrmDatabase SmXdefSubtract(XrmDatabase db);

#endif

#endif /*_smxdef_h*/
/* DON'T ADD ANYTHING AFTER THIS #endif */
