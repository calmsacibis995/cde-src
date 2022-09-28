/* $XConsortium: SmWindow.h /main/cde1_maint/2 1995/08/30 16:35:00 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        SmWindow.h
 **
 **   Project:     HP DT
 **
 **   Description
 **   -----------
 **   Variables and declarations needed for
 **   Getting Top Level Window Info for the session manager
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990.  
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _smwindow_h
#define _smwindow_h
 
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
 * Structure to hold window and state information to be later written
 * to the output state file
 */

typedef struct
{
    int		termState;
    Window	wid;
} WindowInfo, *WindowInfoPtr;

/*
 * Information returned by application in response to WM_STATE message
 */
typedef struct
{
  int state;
  Window icon;
} WM_STATE;

/*
 *  External variables  
 */


/*  
 *  External Interface  
 */

#ifdef _NO_PROTO

extern int SaveYourself() ;
extern int GetTopLevelWindows() ;

#else

extern int SaveYourself( WindowInfo ) ;
extern int GetTopLevelWindows( int, WindowInfo **, unsigned int *, 
			       unsigned int *) ;

#endif /* _NO_PROTO */


#endif /*_smwindow_h*/
/* DON'T ADD ANYTHING AFTER THIS #endif */
