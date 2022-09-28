#ifndef _PMAP_CLNT_INCLUDED
#define _PMAP_CLNT_INCLUDED
#ifdef MODULE_ID
/*
 * @(#)pmap_clnt.h.h: $Revision: 1.1 $
 * $Locker:  $
 */
/* pmap_clnt.h: 1.1	[93/08/28  11:16:22] */
#endif /* MODULE_ID */

/*
 * REVISION: @(#)10.1
 */

/*
 * portmap_clnt.h
 * Supplies C routines to get to portmap services.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#if defined(__cplusplus)
#define __o ...
extern "C" {
#else
#define __o
#endif
/*
 * Usage:
 *	success = pmap_set(program, version, protocol, port);
 *	success = pmap_unset(program, version);
 *	port = pmap_getport(address, program, version, protocol);
 *	head = pmap_getmaps(address);
 *	clnt_stat = pmap_rmtcall(address, program, version, procedure,
 *		xdrargs, argsp, xdrres, resp, tout, port_ptr)
 *		(works for udp only.) 
 * 	clnt_stat = clnt_broadcast(program, version, procedure,
 *		xdrargs, argsp,	xdrres, resp, eachresult)
 *		(like pmap_rmtcall, except the call is broadcasted to all
 *		locally connected nets.  For each valid response received,
 *		the procedure eachresult is called.  Its form is:
 *	done = eachresult(resp, raddr)
 *		bool_t done;
 *		caddr_t resp;
 *		struct sockaddr_in raddr;
 *		where resp points to the results of the call and raddr is the
 *		address if the responder to the broadcast.
 */

extern bool_t		pmap_set(__o);
extern bool_t		pmap_unset(__o);
extern u_short		pmap_getport(__o);
extern struct pmaplist	*pmap_getmaps(__o);
enum clnt_stat		pmap_rmtcall(__o);
enum clnt_stat		clnt_broadcast(__o);
#if defined(__cplusplus)
}
#endif
#endif /* _PMAP_CLNT_INCLUDED */
