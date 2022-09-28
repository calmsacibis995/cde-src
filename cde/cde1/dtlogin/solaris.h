/*******************************************************************************
**
**  solaris.h 1.13 97/06/08 
**
**  Copyright 1993, 1994, 1995 Sun Microsystems, Inc.  All rights reserved.
**
**  This file contains header info specific to Sun Solaris login
**
*******************************************************************************/
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*******************************************************************************
**
**  RESTRICTED CONFIDENTIAL INFORMATION:
**
**  The information in this document is subject to special
**  restrictions in a confidential disclosure agreement between
**  HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
**  document outside HP, IBM, Sun, USL, SCO, or Univel without
**  Sun's specific written approval.  This document and all copies
**  and derivative works thereof must be returned or destroyed at
**  Sun's request.
**
**  Copyright 1994 Sun Microsystems, Inc.  All rights reserved.
**
*******************************************************************************/


#ifndef _DTLOGIN_SOLARIS_H
#define _DTLOGIN_SOLARIS_H

#include <sys/types.h>

#define SOLARIS_LOGIN_SLEEPTIME 4

#ifdef PAM
#include    <security/pam_appl.h>
#define	SOLARIS_SUCCESS	PAM_SUCCESS
#else
#ifdef SUNAUTH
#include    <security/ia_appl.h>
#define SOLARIS_SUCCESS IA_SUCCESS
#endif /* SUNAUTH */
#endif /* PAM */

#include <deflt.h>
#define SOLARIS_LOGIN_DEFAULTS "/etc/default/login"


/* Solaris utmp mgt flags */

#define	SOLARIS_UPDATE_ENTRY  	1	/* Update an existing entry */
#define	SOLARIS_NOLOG		2	/* Don't log the new session */
#define	SOLARIS_LOGIN		4	/* login type entry (sigh...) */
 
/* Errors returned by solaris_setutmp_mgmt/solaris_reset_utmp_mgmt() */
#define	SOLARIS_NOENTRY		27 	/* No entry found */
#define	SOLARIS_ENTRYFAIL	28	/* Couldn't edit the entry */

/* user credential UID/GID erros */ 
#define	SOLARIS_BAD_GID		29	/* Invalid Group ID */
#define	SOLARIS_INITGROUP_FAIL	30	/* group IDs init failed */
#define	SOLARIS_BAD_UID		31	/* Invaid User ID */
#define	SOLARIS_SETGROUP_FAIL	32	/* Set of group IDs failed */


/*
 *	External procedure declarations
 */

#ifdef _NO_PROTO

extern int solaris_authentication();
extern int solaris_accounting();
extern int solaris_setcred();
extern int solaris_setdevperm();
extern int solaris_resetdevperm();
extern int solaris_setutmp_mgmt();
extern int solaris_reset_utmp_mgmt();
extern void solaris_use_greet_pipe();
extern char* _sdt_create_devname();
extern char* _sdt_dev_touch();
extern int audit_login_save_host();
extern int audit_login_save_ttyn();
extern int audit_login_save_port();
extern int audit_login_success();
extern int audit_login_save_pw();
extern int audit_login_bad_pw();
extern int audit_login_maxtrys();
 
extern void solaris_xserver_cred();

#else

extern int solaris_authentication(char*, char*, char*, char*, char*, char*);
extern int solaris_accounting(char*, char*, char[], char*, 
			      char*, pid_t, int, int);
extern int solaris_setcred(char*, char *, uid_t, gid_t);
extern int solaris_setdevperm(char *, uid_t, gid_t);
extern int solaris_resetdevperm(char *);
extern int solaris_setutmp_mgmt(char*, char*, char*, int, int, char id[]);
extern int solaris_reset_utmp_mgmt(int, int, char id[]);
extern void solaris_use_greet_pipe(int);
extern char* _sdt_create_devname(char* short_devname);
extern char* _sdt_dev_touch(char* line_dev);

extern int audit_login_save_host(char *host);
extern int audit_login_save_ttyn(char *ttyn);
extern int audit_login_save_port(void);
extern int audit_login_success(void);
extern int audit_login_save_pw(struct passwd *pwd);
extern int audit_login_bad_pw(void);
extern int audit_login_maxtrys(void);

extern void solaris_xserver_cred(struct verify_info*, int, gid_t*);

#endif /* _NO_PROTO */

#endif /* _DTLOGIN_SOLARIS_H */
