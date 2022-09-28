/* $XConsortium: ManPageI.h /main/cde1_maint/1 1995/07/17 21:02:19 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        ManPageI.h
 **
 **   Project:     helpviewer 3.0
 **
 **   Description: Structures and defines supported in ManPage.c 
 **
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _ManPageI_h
#define _ManPageI_h



/****************************************************************************
 * Function:	    static void PostManDialog();
 *
 * Parameters:      
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Creates and manages a simple prompt dialog that allows a 
 *                  user to type in and display any manpage on the system.
 *
 ****************************************************************************/
#ifdef _NO_PROTO
void PostManDialog();
#else
void PostManDialog(
    Widget parent,
    int     argc,
    char    **argv);
#endif /* _NO_PROTO */



#endif /* _ManPageI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */





