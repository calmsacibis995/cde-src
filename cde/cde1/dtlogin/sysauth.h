/* $XConsortium: sysauth.h /main/cde1_maint/2 1995/10/23 09:21:42 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        sysauth.h
 **
 **   Project:     HP Visual User Environment (DT)
 **
 **   Description: Header file for system authentication routine
 **
 **                Defines, structure definitions, and external declarations
 **		   are specified here.
 **
 **
 **   (c) Copyright 1987, 1988, 1989 by Hewlett-Packard Company
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _SYSAUTH_H
#define _SYSAUTH_H

/****************************************************************************
 *
 *  Defines
 *
 ****************************************************************************/

/*
 *  Authenticate return codes...
 */

#define VF_OK			1
#define VF_INVALID		2
#define VF_HOME			3
#define VF_MAX_USERS		4
#define VF_PASSWD_AGED		5
#define VF_BAD_UID		6
#define VF_BAD_GID		7
#define VF_BAD_AID		8
#define VF_BAD_AFLAG		9
#define VF_NO_LOGIN		10
#define VF_BAD_HOSTNAME		11
#define VF_CHALLENGE            12
#define VF_MESSAGE              13

#ifdef BLS
#define VF_BAD_SEN_LEVEL	14
#endif

#define VF_NOT_CONSOLE		74
#define VF_BAD_SHELL		75
#define VF_ETC_SHELLS		76
 
/****************************************************************************
 *
 *  External procedure declarations
 *
 ****************************************************************************/
#ifdef _NO_PROTO

extern int  Authenticate();	/* entry point to validation policies	   */

#else

extern int  Authenticate(struct display *d, char *name, char *passwd,
                         char **msg) ;

#endif /* _NO_PROTO */

#endif /* _SYSAUTH_H */
