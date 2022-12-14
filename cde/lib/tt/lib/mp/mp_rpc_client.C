//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $XConsortium: mp_rpc_client.C /main/cde1_maint/3 1995/10/04 13:29:35 gtsang $ 			 				
/*
 *
 * mp_rpc_client.cc
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */
#include <sys/time.h>		// ultrix
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <unistd.h>
#include "tt_options.h"
#include "mp/mp_rpc_auth.h"
#include "mp/mp_rpc_client.h"
#include "util/tt_host.h"
#include "util/tt_port.h"

#if defined(ultrix)
extern "C"
{ extern struct XSizeHints;
  extern struct XStandardColormap;
  extern struct XTextProperty;
  extern struct XWMHints;
  extern struct XClassHint;
};
#endif

#if defined(_AIX)
/* AIX's FD_ZERO macro uses bzero() without declaring it. */
#include <strings.h>
/* And arpa/inet.h has a buggy declaration of inet_addr */
extern "C" in_addr_t inet_addr(const char *);
#endif

#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <memory.h>

#if defined(sgi)
/* SGI's FD_ZERO macro uses bzero() without declaring it. */
#include <CC/libc.h>
#endif	

#include <sys/resource.h>
#include <util/tt_global_env.h>
#include <mp/mp_rpc_fns.h>

/* 
 * Constructs an rpc client. 
 */
_Tt_rpc_client::
_Tt_rpc_client(int conn_socket)
{
	_socket = conn_socket;
	_client = (CLIENT *)0;
}


/* 
 * Destroys an rpc client (breaks off connections)
 */
_Tt_rpc_client::
~_Tt_rpc_client()
{
	if (_client != (CLIENT *)0) {
		clnt_destroy(_client);
		_client = (CLIENT *)0;
	}
	_host = (_Tt_host *)0;
}


/* 
 * Returns the socket associated with an rpc client. --> it would be
 * nice to use this socket rather than open up a new one when signalling
 * procids.   
 */
int _Tt_rpc_client::
socket()
{
	return(_socket);
}

/* 
 * create client connection to host,program,version
 */
int _Tt_rpc_client::
init(_Tt_host_ptr &host, int program, int version,
     uid_t servuid, _Tt_rpc_auth_ptr auth)
{
	int		optval;
	static caddr_t	saved_opaque = 0;
	static int	saved_len = 0;

	optval = (_socket == RPC_ANYSOCK);
	_auth = auth;
	_host = host;
	_program = program;
	_version = version;
	_server_uid = servuid;
	if (_client != (CLIENT *)0) {
		if (auth->auth_level() == _TT_AUTH_UNIX) {
			auth_destroy(_client->cl_auth);
		}
		clnt_destroy(_client);
	}
#if defined(OPT_SECURE_RPC)
	if (auth->auth_level() == _TT_AUTH_DES) {
		if (_server_uid == 0) {
			host2netname(_servername, (char *)_host->name(), 0);
		} else {
			user2netname(_servername, _server_uid, 0);
		}
	}
#endif /* OPT_SECURE_RPC */	
#ifndef	OPT_TLI

	memset(&_server_addr, 0, sizeof(_server_addr));
	_server_addr.sin_family = AF_INET;
	_server_addr.sin_port = htons((optval) ? 0 : 4000);
	_server_addr.sin_addr.s_addr = inet_addr((char *)(_host->stringaddr()));
	// Bug 1212990: if we are talking to the local host, use
	// the loopback address.  This will continue to work even
	// if other interfaces are configured down; for example,
	// nomadic machines.
		
	_Tt_host_ptr local_host;

	if ( ( _tt_global->get_local_host(local_host) ) &&
	     ( _host->stringaddr() == local_host->stringaddr())) {
		_server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	}
	
	_client = clnttcp_create(&_server_addr, _program,
			         _version, &_socket, 4000, 4000);
	if (_client == 0) {
		// XXX only when in some kind of debug mode
		//clnt_pcreateerror("_Tt_rpc_client::init(): clnttcp_create()");
		return 0;
	}
	if (auth->auth_level() == _TT_AUTH_UNIX) {
		_client->cl_auth = authunix_create_default();
	}
		
	if (optval) {
		if (setsockopt(_socket, SOL_SOCKET, SO_USELOOPBACK,
			       (char *)&optval, sizeof(int)) == -1) {
			_tt_syslog( 0, LOG_ERR, "_Tt_rpc_client::init(): "
				    "setsockopt(SO_USELOOPBACK): %m");
		}
		if (setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY,
			       (char *)&optval, sizeof(int)) == -1) {
			_tt_syslog( 0, LOG_ERR, "_Tt_rpc_client::init(): "
				    "setsockopt(TCP_NODELAY): %m");
		}
	}
#else

	// Bug 1212990: if we are talking to the local host, use
	// the loopback address.  This will continue to work even
	// if other interfaces are configured down; for example,
	// nomadic machines.
		
	_Tt_host_ptr local_host;
	const char *host_or_localhost;

	if ( ( _tt_global->get_local_host(local_host) ) &&
	     ( _host->stringaddr() == local_host->stringaddr())) {
		host_or_localhost = "127.0.0.1";
	} else {
		host_or_localhost = (char *)_host->name();
	}
	_client = _tt_clnt_create_timed(host_or_localhost,
					_program, _version, "circuit_v");
	host_or_localhost = 0;
	
	if (_client == 0) {
		// Uncomment this message write only when
		// debugging. This clnt_create often fails in normal
		// circumstances, for example when there's a stale
		// session id in the interest list for a file.
		//clnt_pcreateerror( "_Tt_rpc_client::init(): clnt_create()" );
		return 0;
	}

	if (!clnt_control(_client, CLGET_FD, (char *)&_socket)) {
		// I cannot imagine how this could ever fail at this point.
		return 0;
	}
	// We used to call _tt_tli_set_nodelay here on the RPC connection.
	// This stopped working mysteriously, suddenly the endpoint started
	// coming back in state T_DATAXFER instead of T_IDLE from clnt_create.
	// Fortunately, in the meantime the TIRPC library has started
	// setting NODELAY on RPC/TCP connections, so we don't need to
	// do it here anymore anyway.

	if (auth->auth_level() == _TT_AUTH_UNIX) {
		_client->cl_auth = authunix_create_default();
	}

#endif				// !OPT_TLI

	// Set close-on-exec bit so a libtt client which forks and execs won't
	// be short some fd's in the child.
	if (-1==fcntl(_socket, F_SETFD, 1)) {
		_tt_syslog( 0, LOG_ERR, "_Tt_rpc_client::init(): "
			    "fcntl(F_SETFD): %m");
	}		
	return(1);
}


/* 
 * invoke rpc procedure
 */
clnt_stat _Tt_rpc_client::
call(int procnum, xdrproc_t inproc, char *in,
     xdrproc_t outproc, char *out, int timeout)
{
	timeval		total_timeout;
	struct sigaction curr_action;
	int		need2reset_sigpipe = 0;

	if (_client == (CLIENT *)0) {
		return(RPC_CANTSEND);
	}
	total_timeout.tv_sec = ((timeout < 0) ? 0 : timeout);
	total_timeout.tv_usec = 0;
	clnt_control(_client, CLSET_TIMEOUT, (char *) &total_timeout);

	switch (_auth->auth_level()) {
#if defined(OPT_TT_GSS_API)
	      case _TT_AUTH_GSS:
		// rpc_gss_seccreate() is done  in _Tt_c_procid::init() for
		// reasons documented there...
	      break;
#endif

	      case _TT_AUTH_UNIX:
		break;
#if defined(OPT_SECURE_RPC)
	      case _TT_AUTH_DES:
#ifdef OPT_TLI
		_client->cl_auth =
		authdes_seccreate(_servername, 60, (char *)_host->name(),
				  (des_block *)0);
		if (_client->cl_auth == 0) {
			_tt_syslog( 0, LOG_WARNING, "authdes_seccreate(): 0" );
			// XXX what todo when authdes_seccreate() fails?
			return RPC_AUTHERROR;
		}
#else 
		_client->cl_auth =
		authdes_create(_servername, 60, &_server_addr,
			       (des_block *)0);
		if (_client->cl_auth == 0) {
			_tt_syslog( 0, LOG_WARNING, "authdes_seccreate(): 0" );
			// XXX what todo when authdes_seccreate() fails?
			return RPC_AUTHERROR;
		}
#endif /* OPT_TLI */
		break;
#endif /* OPT_SECURE_RPC */
              case _TT_AUTH_NONE:
		break;
	      default:
		return(RPC_AUTHERROR);
	}
	
	if (timeout == 0) {
		struct pollfd fds[1];

		fds[0].fd = _socket;
		fds[0].events = POLLIN;
		fds[0].revents = 0;
		// Poll must return 0 (normal timeout) else something's
		// wrong.
		if (poll(fds, 1, 0) != 0) {
			return(RPC_CANTSEND);
		} 
	}

#if defined(OPT_BUG_USL) || defined(OPT_BUG_UXP)
	if (timeout <= 0)
	{
		outproc = (xdrproc_t) NULL;
	}
#endif

	//
	// tcp write errors (when the rpc_server on the other end dies)
	// cause a SIGPIPE.  We need to make sure the SIGPIPE is caught,
	// or the process dies.
	//
	if (sigaction(SIGPIPE, 0, &curr_action) != 0) {
		_tt_syslog( 0, LOG_ERR, "sigaction(): %m" );
	}
#if defined(OPT_BUG_SUNOS_5) || defined(OPT_BUG_UW_1)
	if ((SIG_TYP)curr_action.sa_handler == SIG_DFL)
#else
#if defined(OPT_BUG_UW_2) || defined(OPT_BUG_UXP)
	if ((void(*)(int))curr_action.sa_handler == SIG_DFL)
#else
	if (curr_action.sa_handler == SIG_DFL)
#endif
#endif
	{
		need2reset_sigpipe = 1;
		signal(SIGPIPE, SIG_IGN);
	}

	_clnt_stat = clnt_call(_client, procnum,
			       inproc, in,
			       outproc, out,
			       total_timeout);
#if defined(OPT_BUG_USL) || defined(OPT_BUG_UXP)
	if (timeout <= 0)
	{
		total_timeout.tv_sec = 10;
		total_timeout.tv_usec = 0;
		clnt_control(_client, CLSET_TIMEOUT, (char *) &total_timeout);
		clnt_call(_client, NULLPROC, (xdrproc_t) xdr_void, (char *)NULL,
		(xdrproc_t) xdr_void, (char *) NULL, total_timeout);
	}
#endif
	if (need2reset_sigpipe) {
		signal(SIGPIPE, SIG_DFL);
	}
#if !defined(OPT_BUG_HPUX) /* HP-UX doesn\'t seem to have this return */
	if (_clnt_stat == RPC_INTR) {
		return(RPC_CANTSEND);
	}
#endif	
	if (_auth->auth_level() == _TT_AUTH_DES) {
		auth_destroy(_client->cl_auth);
	}

	return(_clnt_stat);
}
