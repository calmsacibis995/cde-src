/* $XConsortium: connection.h /main/cde1_maint/1 1995/07/17 19:51:43 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _CONNECTION_H
#define _CONNECTION_H

#pragma ident "@(#)connection.h	1.12 97/01/31 Sun Microsystems, Inc."

#include <rpc/rpc.h>
#include "ansi_c.h"
#include "csa.h"
#include <pthread.h>

extern int	_DtCM_DEFAULT_TIMEOUT;
extern int	_DtCM_INITIAL_TIMEOUT;
extern int	_DtCM_LONG_TIMEOUT;

typedef struct targetlist {
	char		*cal;
	unsigned long	update_type;
	struct targetlist *next;
} _DtCm_Target_List;

typedef struct cl_info {
	char		*host;
	CLIENT		*tcpcl;
	CLIENT		*udpcl;
	u_long		vers_out;
	long		last_used;
	int		nregistered;
	_DtCm_Target_List	*tlist;
	boolean_t	delete;		/* _B_TRUE => to be deleted */
	pthread_mutex_t	lock_elem;
	pthread_mutex_t	lock_handle;
	struct cl_info *next;
} _DtCm_Client_Info;

typedef struct conn {
	_DtCm_Client_Info	*ci;
	int			retry;
	enum clnt_stat		stat;
} _DtCm_Connection;

#ifdef SunOS
extern enum clnt_stat _DtCm_clnt_call(_DtCm_Connection *conn,
				      u_long proc,
				      xdrproc_t inproc,
				      caddr_t in,
				      xdrproc_t outproc,
				      caddr_t out,
				      struct timeval tout);
#endif

extern CSA_return_code _DtCm_add_registration(_DtCm_Client_Info *ci,
					      char *cal,
					      unsigned long update_type);

extern void _DtCm_remove_registration(_DtCm_Client_Info *ci,
				      char *cal,
				      unsigned long update_type);

/*
 * Obtain client handle for the calendar server running in host.
 * Returns a client info structure for the host with the inuse field
 * incremented (so that the record won't be freed by another thread).
 * When done, the caller should call _DtCm_done_with_client_handle
 * so that the inuse field is decremented properly
 */
extern CSA_return_code _DtCm_get_rpc_handle(char	*host,
					    u_long	version,
					    int	timeout,
					    _DtCm_Client_Info **clnt);

extern CSA_return_code _DtCm_clntstat_to_csastat(enum clnt_stat clntstat);

#endif

