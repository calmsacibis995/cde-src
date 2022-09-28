/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/****************************<+>*************************************
 **
 **   File:     EnvControl.h
 **
 **   RCS:	$XConsortium: GetDispRes.h /main/cde1_maint/1 1995/07/14 20:34:19 drk $
 **   Project:  HP DT Runtime Library
 **
 **   Description: Defines structures, and parameters used
 **                for communication with the environment
 **
 **   (c) Copyright 1992 by Hewlett-Packard Company
 **
 ****************************<+>*************************************/

/********    Function Declarations    ********/
#ifdef _NO_PROTO

int _DtGetDisplayResolution() ;

#else

int _DtGetDisplayResolution(
                        Display *disp,
                        int screen) ;

#endif /* _NO_PROTO */
/********    End Function Declarations    ********/

/**********************************************************************
 * Resolution threshold values (width of screen in pixels) 
 **********************************************************************/

#define   _DT_HIGH_RES_MIN	       1176
#define	  _DT_MED_RES_MIN		851
#define	  _DT_LOW_RES_MIN		512

/**********************************************************************
 * Resolution types of a given screen
 **********************************************************************/
#define   NO_RES_DISPLAY        0
#define   LOW_RES_DISPLAY       1
#define   VGA_RES_DISPLAY       2
#define   MED_RES_DISPLAY       3
#define   HIGH_RES_DISPLAY      4
#define   ALL_RES_DISPLAY       5

/****************************        eof       **********************/
