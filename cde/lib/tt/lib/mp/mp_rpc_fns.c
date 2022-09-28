/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 *
 * mp_rpc_fns.c
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */

#include <tt_options.h>
#include <rpc/rpc.h>

#if defined(OPT_TLI)

#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <tiuser.h>
#include <sys/socket.h>


/* 
 * Sets the TCP_NODELAY option for a tli stream bound using tcp. This
 * option is crucial to good message performance since otherwise there is
 * a 200 ms delay between messages. This code is based on the source code
 * for tli rpc (since documentation was sorely lacking).
 */
int
_tt_tli_set_nodelay(int fd)
{
	struct t_optmgmt *options;
	struct sochdr {
		struct opthdr opthdr;
		long value;
	} sochdr;

	options = (struct t_optmgmt *)t_alloc(fd, T_OPTMGMT, 0);
	if (options == (struct t_optmgmt *)0) {
		return (0);
	}
	sochdr.opthdr.level = IPPROTO_TCP;
	sochdr.opthdr.name = TCP_NODELAY;
	sochdr.opthdr.len = 4;
	sochdr.value = 1;
	options->opt.maxlen = sizeof(sochdr);
	options->opt.len = sizeof(sochdr);
	options->opt.buf =  (char *) &sochdr;
	options->flags = T_NEGOTIATE;
	if (t_optmgmt(fd, options, options) == -1) {
		t_error("t_optmgmt");
		return(0);
	}
	options->opt.buf = 0;
	(void) t_free((char *)options, T_OPTMGMT);
	return(1);
}


int
_tt_bind_endpoint(fd, s, r)
     int fd;
     char *s;
     char *r;
{
	return(t_bind(fd, (struct t_bind *)s, (struct t_bind *)r));
}
#endif				/* OPT_TLI */


/* 
 *   XXX: the following are needed because of bugs in the C++ header files
 *   for these functions. Should use the native definitions when the header
 *   files are fixed.
 */

void
_tt_svc_freeargs(transp, inproc, in)
     SVCXPRT	*transp;
     xdrproc_t	inproc;
     char	*in;
{
	svc_freeargs(transp, inproc, in);
}

int
_tt_svc_getargs(transp, inproc, in)
     SVCXPRT	*transp;
     xdrproc_t	inproc;
     char	*in;
{
	return(svc_getargs(transp, inproc, in));
}
