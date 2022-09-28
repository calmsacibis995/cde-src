/*
 * xdm - display manager daemon
 *
 * $XConsortium: bls.h,v 1.31 90/03/29 11:36:37 keith Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * bls.h
 *
 * public interfaces for B1 greet/verify functionality
 */


#ifndef _BLS_H
#define _BLS_H

#ifndef _NO_PROTO
#if    !defined(__STDC__) && !defined(__cplusplus) && !defined(c_plusplus)
#define _NO_PROTO
#endif /* __STDC__ */
#endif /* _NO_PROTO */

typedef unsigned char BOOL;

#ifndef FALSE
#define	FALSE	0
#endif

#ifndef	TRUE
#define TRUE	1
#endif

#ifdef BLS
#include <sys/security.h>
#include <sys/audit.h>     /* for passwd and pr_passwd */
#endif

#ifdef pegasus
#undef dirty            /* Some bozo put a macro called dirty in sys/param.h */
#endif /* pegasus */

struct greet_info {
	char            *name;          /* user name */
	char            *password;      /* user password */
#ifdef BLS
	char            *b1security;    /* user's b1 security */
#endif
	char            *string;        /* random string */
};
				
struct verify_info {
	int             uid;            /* user id */
#ifdef NGROUPS
	int             groups[NGROUPS];/* group list */
	int             ngroups;        /* number of elements in groups */
#else
	int             gid;            /* group id */
#endif
	char            **argv;         /* arguments to session */
	char            **userEnviron;  /* environment for session */
	char            **systemEnviron;/* environment for startup/reset */
#ifdef BLS
	char *user_name;
	struct mand_ir_t *sec_label_ir;
	struct mand_ir_t *clearance_ir;
	/* save these for logout time */
	struct pr_passwd *prpwd;
	struct passwd *pwd;
	char terminal[16];
#endif
};

/***************************************************************************
 *
 *  Global variables
 *
 ***************************************************************************/
extern struct pr_passwd		*b1_pwd;
extern struct verify_info	*verify;	
extern struct greet_info	*greet;	

#endif	/* _BLS_H */
