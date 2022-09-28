/* $XConsortium: agent_p.h /main/cde1_maint/1 1995/07/17 19:46:25 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _AGENT_P_H
#define _AGENT_P_H

#pragma ident "@(#)agent_p.h	1.8 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"

extern void _DtCm_init_agent();
extern void _DtCm_destroy_agent();
extern void _DtCm_process_updates();

/*
 * svc_getreqset is not MT-safe and it cannot be called recursively
 * _DtCm_lock_get_rpc_request() and _DtCm_unlock_get_rpc_request()
 * are used to synchronize threads calling svc_getreqset().
 * Note: order of locking is important. Always acquire the lock for
 * svc_getreqset() before locking any calendar session.
 */
extern void _DtCm_lock_get_rpc_request();
extern void _DtCm_unlock_get_rpc_request();

#endif /* _AGENT_P_H */
