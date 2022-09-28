/*******************************************************************************
**
**  "@(#)pam_svc.h 1.7 97/06/15 
**
**  Copyright 1993, 1994, 1995 Sun Microsystems, Inc.  All rights reserved.
**
**  This file contains header info related to dtlogin use of PAM 
**  (Pluggable Authentication Module) library.
**
*******************************************************************************/
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.                *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef _DTLOGIN_PAM_SVC_H
#define _DTLOGIN_PAM_SVC_H

#include <sys/types.h>

#define PAM_LOGIN_MAXTRIES 5
#define PAM_LOGIN_DISABLETIME 20
#define DTLOGIN_SVC "dtlogin"

 
/*
 *	External procedure declarations
 */

#ifdef _NO_PROTO

extern int PamAuthenticate();
extern int PamAccounting();
extern int PamSetCred();
extern int PamClose();
extern void PamUseGreetPipe();

#ifdef sun
extern int audit_login_save_host();
extern int audit_login_save_ttyn();
extern int audit_login_save_port();
extern int audit_login_success();
extern int audit_login_save_pw();
extern int audit_login_bad_pw();
extern int audit_login_maxtrys();
#endif /*sun*/
 
#else

extern int PamAuthenticate(char*, char*, char*, char*, char*, char*);
extern int PamAccounting(char*, char*, char*, char*, char*, pid_t, int, int);
extern int PamSetCred(char*, char *, struct verify_info*);
extern int PamClose(int);
extern void PamUseGreetPipe(int val); 

#ifdef sun
extern int audit_login_save_host(char *host);
extern int audit_login_save_ttyn(char *ttyn);
extern int audit_login_save_port(void);
extern int audit_login_success(void);
extern int audit_login_save_pw(struct passwd *pwd);
extern int audit_login_bad_pw(void);
extern int audit_login_maxtrys(void);
#endif /*sun*/

#endif /* _NO_PROTO */

#endif /* _DTLOGIN_PAM_SVC_H */
