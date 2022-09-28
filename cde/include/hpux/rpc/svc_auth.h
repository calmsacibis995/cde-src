#ifndef _SVC_AUTH_INCLUDED
#define _SVC_AUTH_INCLUDED
#ifdef MODULE_ID
/*
 * @(#)svc_auth.h: $Revision: 1.1 $ $Date: 93/09/02 07:15:11 $
 * $Locker:  $
 */
#endif /* MODULE_ID */
/*
 * REVISION: @(#)10.1
 */

/*
 * svc_auth.h, Service side of rpc authentication.
 * 
 * (c) Copyright 1987 Hewlett-Packard Company
 * (c) Copyright 1984 Sun Microsystems, Inc.
 */


/*
 * Server side authenticator
 */
extern enum auth_stat _authenticate();
#endif /* _SVC_AUTH_INCLUDED */
