/* $XConsortium: SmHelp.h /main/cde1_maint/2 1995/08/29 20:03:09 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        SmHelp.h
 **
 **   Project:     HP DT
 **
 **   Description
 **   -----------
 **   Variables and declarations needed for
 **   Help for the session manager
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990.  
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _smhelp_h
#define _smhelp_h
 
/* 
 *  #include statements 
 */
#include <Xm/Xm.h>


/* 
 *  #define statements 
 */
#define	HELP_LOGOUT_STR			"ConfirmLogoutDE"
#define HELP_LOGOUT_QUERY_STR		"SEConfirmLogoutWithChoiceDE"
#define HELP_LOGOUT_COMPAT_MODE_STR	"logoutCompatModeHelp"
#define HELP_BMS_DEAD_STR		"bmsDeadHelp"

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

extern void TopicHelpRequested() ;

#else

extern void TopicHelpRequested( Widget, XtPointer, XtPointer ) ;

#endif


#endif /*_smhelp_h*/
/* DON'T ADD ANYTHING AFTER THIS #endif */
