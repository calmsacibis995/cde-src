/* $XConsortium: connection.c /main/cde1_maint/3 1995/10/10 13:28:53 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)connection.c	1.20 97/01/31 Sun Microsystems, Inc."

/*
 * This file manages server connections.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/resource.h>
#include "connection.h"
#include "rtable2.h"
#include "rtable3.h"
#include "rtable4.h"
#include "cm.h"
#include "debug.h"
#include "agent.h"
#include "convert2-4.h"
#include "convert3-4.h"
#include "rpccalls.h"

#include <pthread.h>

/*
 * A list of client record sorted alphabetically in host name
 */
static _DtCm_Client_Info client_cache_head;

/*****************************************************************************
 * forward declaration of static functions.
 *****************************************************************************/
static _DtCm_Client_Info *get_handle_from_list(char *host, u_long version);
static CSA_return_code create_udp_client(char *host, u_long version,
			int timeout, u_long *vers_out, CLIENT **cl_out);
static CSA_return_code create_tcp_client(char *host, u_long version,
			int timeout, CLIENT	**cl_out);
static void create_auth(CLIENT *cl);
static void destroy_target_list(_DtCm_Target_List *tlist);
static void destroy_client_info(_DtCm_Client_Info *ci);
static void insert_client_info(_DtCm_Client_Info **ci);
static void get_new_client_handle(_DtCm_Client_Info *ci);
static void check_registration(_DtCm_Client_Info *ci);
static boolean_t is_the_right_record(_DtCm_Client_Info *ptr, u_long version);
static void _increase_file_descriptor_limit();

#if !(defined(SunOS) || defined(USL) || defined(__uxp__))
#ifdef __osf__
static CSA_return_code create_client_handle(const char *host,
			const u_int prognum, u_long *vers_outp,
			const u_long vers_low, const u_long vers_high,
			char *nettype, CLIENT **clnt);
#else
static CSA_return_code create_client_handle(const char *host,
			const u_long prognum, u_long *vers_outp,
			const u_long vers_low, const u_long vers_high,
			char *nettype, CLIENT **clnt);
#endif /* __osf__ */
#endif


int		_DtCM_DEFAULT_TIMEOUT = 5;
int		_DtCM_INITIAL_TIMEOUT = 25;
int		_DtCM_LONG_TIMEOUT = 60;

/*****************************************************************************
 * extern functions used in the library
 *****************************************************************************/

/*
 * Obtain rpc handle for the calendar server running in host.
 * The routine first tries to locate the record in the existing list,
 * if no record is found, will create a udp and a tcp client handle
 * to the host and a new record inserted in the list.
 * Before the record is returned, the inuse field is incremented so
 * that the record won't be freed by another thread.
 * The caller should call _DtCm_done_with_client_handle() when
 * it's done with the record.
 */
extern CSA_return_code
_DtCm_get_rpc_handle(
	char	*host,
	u_long	version,
	int	timeout,
	_DtCm_Client_Info **clnt)
{
	CSA_return_code	stat;
	u_long		vers_out;
	CLIENT		*cl;
	_DtCm_Client_Info *ci;

	_increase_file_descriptor_limit();

	if (host == NULL || clnt == NULL)
		return (CSA_E_INVALID_PARAMETER);

	/* locate the client handle in the list */
	if (((*clnt) = get_handle_from_list(host, version)) != NULL)
		return (CSA_SUCCESS);

	if ((ci = (_DtCm_Client_Info *)calloc(1, sizeof(_DtCm_Client_Info)))
	    == NULL) {
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	if ((ci->host = strdup(host)) == NULL) {
		free(ci);
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	if ((stat = create_udp_client(host, version, timeout, &ci->vers_out,
	    &ci->udpcl)) != CSA_SUCCESS) {
		free(ci->host);
		free(ci);
		return (stat);
	}

	if ((stat = create_tcp_client(host, version, timeout, &ci->tcpcl))
	    != CSA_SUCCESS)
		ci->tcpcl = NULL;

	/* unlock in _DtCm_done_with_client_handle */
	pthread_mutex_lock(&ci->lock_handle);

	insert_client_info(&ci);

	*clnt = ci;
	return (CSA_SUCCESS);
}

/*
 * Used instead of clnt_call by rpc client routines.
 *
 * ***
 * *** the client info record is locked by the caller
 * *** don't call a function which may try to lock the record again
 * ***
 */
extern enum clnt_stat
_DtCm_clnt_call(
	_DtCm_Connection *conn,
	u_long proc,
	xdrproc_t inproc,
	caddr_t in,
	xdrproc_t outproc,
	caddr_t out,
	struct timeval tout)
{
	_DtCm_Client_Info	*ci = conn->ci;
	enum clnt_stat		status = RPC_FAILED;
	int			retry = conn->retry;

	while (_B_TRUE) {
		if (ci->delete == _B_TRUE || ci->udpcl == NULL)
			break;

		ci->last_used = time(0);

		status = clnt_call((ci->tcpcl ? ci->tcpcl : ci->udpcl),
				proc, inproc, in, outproc, out, tout);

		if ((ci->tcpcl == NULL && status == RPC_TIMEDOUT) ||
		     (status == RPC_CANTRECV)) {

			/* don't retry when stat is RPC_TIMEDOUT
			 * and transpart is tcp since if the server
			 * is down, stat would be something else
			 * like RPC_CANTRECV
			 */

			if (retry) {
				retry--;
				get_new_client_handle(ci);
			} else {
				/* get rid of old handles so that
				 * they will be created again
				 */
				clnt_destroy(ci->udpcl);
				ci->udpcl = NULL;

				if (ci->tcpcl) {
					clnt_destroy(ci->tcpcl);
					ci->tcpcl = NULL;
				}
				break;
			}
		} else
			break;
	}

	if (status != RPC_SUCCESS && ci->udpcl != NULL) {
		_DtCm_print_errmsg(clnt_sperror((ci->tcpcl ? ci->tcpcl :
				ci->udpcl), ci->host));
	}
	conn->stat = status;
	return status;
}

/*
 * ***
 * *** the client info record is locked by the caller
 * *** don't call a function which may try to lock the record again
 * ***
 */
extern CSA_return_code
_DtCm_add_registration(
	_DtCm_Client_Info *ci,
	char *cal,
	unsigned long update_type)
{
	_DtCm_Target_List	*listp, *prev;
	_DtCm_Target_List	*listitem;
	int			result;

	if (ci == NULL || cal == NULL)
		return (CSA_E_INVALID_PARAMETER);

	for (listp = prev = ci->tlist; listp != NULL;
	     prev = listp, listp = listp->next) {

		/* add target alphabetically */
		if ((result = strcmp(listp->cal, cal)) == 0) {

			/* registered already, just update the update_type */
			listp->update_type |= update_type;

			return (CSA_SUCCESS);

		} else if (result > 0)
			break;
	}

	/* register the first time, insert in list in ascending order */
	if ((listitem = (_DtCm_Target_List *)calloc(1,
	    sizeof(_DtCm_Target_List))) == NULL) {
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	if ((listitem->cal = strdup(cal)) == NULL) {
		free(listitem);
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	listitem->update_type = update_type;

	if (listp == prev)
		ci->tlist = listitem;
	else
		prev->next = listitem;
	listitem->next = listp;

	ci->nregistered++;

	return (CSA_SUCCESS);
}

/*
 * ***
 * *** the client info record is locked by the caller
 * *** don't call a function which may try to lock the record again
 * ***
 */
extern void
_DtCm_remove_registration(
	_DtCm_Client_Info *ci,
	char *cal,
	unsigned long update_type)
{
	_DtCm_Target_List	*listp, *prev;
	int			result;

	if (ci == NULL || cal == NULL) return;

	for (listp = prev = ci->tlist; listp != NULL;
	     prev = listp, listp = listp->next) {

		if ((result = strcmp(listp->cal, cal)) == 0) {
			if ((listp->update_type =
			    (listp->update_type | update_type) ^ update_type)
			    == 0) {

				/* remove the item */
				if (listp == prev)
					ci->tlist = listp->next;
				else
					prev->next = listp->next;

				/* free target item */
				free(listp->cal);
				free(listp);
			}
			return;
		} else if (result > 0)
			break;
	}
	/* not found; impossible */
}

extern CSA_return_code
_DtCm_clntstat_to_csastat(enum clnt_stat clntstat)
{
	switch (clntstat) {
#if defined(SunOS) || defined(USL) || defined(__uxp__)
	case RPC_N2AXLATEFAILURE:
#endif
	case RPC_UNKNOWNHOST:
		return (CSA_X_DT_E_INVALID_SERVER_LOCATION);
	case RPC_PROGNOTREGISTERED:
		return (CSA_X_DT_E_SERVICE_NOT_REGISTERED);
	case RPC_TIMEDOUT:
		return (CSA_X_DT_E_SERVER_TIMEOUT);
	case RPC_FAILED:
		return (CSA_E_FAILURE);
	default:
		return (CSA_E_SERVICE_UNAVAILABLE);
	}
}

#ifdef CM_DEBUG

extern void
_DtCm_print_host_list()
{
	_DtCm_Client_Info *ptr;

	if (client_cache_head.next == NULL)
		printf("host list is empty\n");
	else
		printf("host list:\n");

	for (ptr = client_cache_head.next; ptr != NULL; ptr = ptr->next) {
		printf("%s (%d)\n", ptr->host, ptr->vers_out);
	}
}

#endif

/*****************************************************************************
 * static functions used within the file
 *****************************************************************************/

/*
 * Given a host name and version, find the _DtCm_Client_Info structure
 * for the calendar server running in the host.
 * This routine also cleans up records with (inuse == 0 and delete == _B_TRUE)
 */
static _DtCm_Client_Info *
get_handle_from_list(char *host, u_long version)
{
	_DtCm_Client_Info	*prev, *ptr, *ci = NULL;
	int			result;

	if (host==NULL) return(NULL);

	prev = &client_cache_head;
	pthread_mutex_lock(&prev->lock_elem);
	while ((ptr = prev->next) != NULL) {
		pthread_mutex_lock(&ptr->lock_elem);

		if (((result = strcmp(ptr->host, host)) == 0) &&
		    ptr->vers_out <= version) {
			if (is_the_right_record(ptr, version) == _B_TRUE) {
				/* the record (lock_handle) is locked already,
				 * it will be unlocked when the caller
				 * calls _DtCm_done_with_client_handle
				 */
				ci = ptr;
				pthread_mutex_unlock(&ptr->lock_elem);
				break;
			}
		} else if (result > 0) {
		  	pthread_mutex_unlock(&ptr->lock_elem);
			break;
		}

		/* continue to traverse the list */
		pthread_mutex_unlock(&prev->lock_elem);
		prev = ptr;
	}

	pthread_mutex_unlock(&prev->lock_elem);

	return (ci);

}

/*
 * if the right record is found, it will be locked (lock_handle)
 * this routine checks to make sure that the record has
 * value rpc handles
 */
static boolean_t
is_the_right_record(_DtCm_Client_Info *ptr, u_long version)
{
	boolean_t	res;

	/* it's ok to lock since rpc calls to the same
	 * host is sequential anyway
	 */
	pthread_mutex_lock(&ptr->lock_handle);

	if (ptr->delete == _B_TRUE) {
		res = _B_FALSE;
	} else if (ptr->vers_out <= version) {
		/* need to check version again since it could have
		 * been changed after we last examined it
		 */
		if (ptr->udpcl) {
			if (ptr->tcpcl == NULL)
				create_tcp_client(ptr->host, ptr->vers_out,
					_DtCM_INITIAL_TIMEOUT, &ptr->tcpcl);
			res = _B_TRUE;
		} else {
			/* get handles again */
			get_new_client_handle(ptr);

			if (ptr->udpcl)
				res = _B_TRUE;
			else {
				res = _B_FALSE; 
				ptr->delete = _B_TRUE;
			}
		}
	} else
		res = _B_FALSE;

	if (res == _B_FALSE) {
	  pthread_mutex_unlock(&ptr->lock_handle);
	}

	return (res);
}

static CSA_return_code
create_udp_client(
	char	*host,
	u_long	version,
	int	timeout,
	u_long	*vers_out,
	CLIENT	**cl_out)
{
	CSA_return_code	stat;
	struct timeval	timeout_tv;
	CLIENT		*cl;

	if (host == NULL)
		return (CSA_E_INVALID_PARAMETER);

#if defined(SunOS) || defined(USL) || defined(__uxp__)
	if ((cl = clnt_create_vers(host, TABLEPROG, vers_out, TABLEVERS_2,
	    version, "udp")) == NULL) {
		_DtCm_print_errmsg(clnt_spcreateerror(host));
		return (_DtCm_clntstat_to_csastat(rpc_createerr.cf_stat));
	}
#else
	if ((stat = create_client_handle(host, TABLEPROG, vers_out, TABLEVERS_2,
	    version, "udp", &cl)) != CSA_SUCCESS)
		return (stat);
#endif

	create_auth(cl);

	/* Adjust Timeout */
	timeout_tv.tv_sec =  (timeout == 0) ? _DtCM_DEFAULT_TIMEOUT : timeout;
	timeout_tv.tv_usec = 0;
	clnt_control(cl, CLSET_TIMEOUT, (char*)&timeout_tv);		

	/*	UDP only!
		time rpc waits for server to reply before retransmission =
		'timeout'. since the retry timeout is set to timeout + 10;
		this guarantees there won't be any automatic retransmisssions
		We will do any retry ourselves.
	*/

	timeout_tv.tv_sec += 10;
	timeout_tv.tv_usec = 0;
	clnt_control(cl, CLSET_RETRY_TIMEOUT, (char*)&timeout_tv);

	*cl_out = cl;
	return (CSA_SUCCESS);
}

static CSA_return_code
create_tcp_client(
	char	*host,
	u_long	version,
	int	timeout,
	CLIENT	**cl_out)
{
	CSA_return_code	stat;
	struct timeval	timeout_tv;
	u_long		vers_out;
	CLIENT		*cl;

	if (host == NULL)
		return (CSA_E_INVALID_PARAMETER);

#if defined(SunOS) || defined(USL) || defined(__uxp__)
	if ((cl = clnt_create_vers(host, TABLEPROG, &vers_out, TABLEVERS_2,
	    version, "tcp")) == NULL) {
		if (rpc_createerr.cf_stat != RPC_PROGNOTREGISTERED)
			_DtCm_print_errmsg(clnt_spcreateerror(host));
		return (_DtCm_clntstat_to_csastat(rpc_createerr.cf_stat));
	}
#else
	if ((stat = create_client_handle(host, TABLEPROG, &vers_out,
	    TABLEVERS_2, version, "tcp", &cl)) != CSA_SUCCESS)
		return (stat);
#endif

	create_auth(cl);

	/* Adjust Timeout */
	timeout_tv.tv_sec =  (timeout == 0) ? _DtCM_DEFAULT_TIMEOUT : timeout;
	timeout_tv.tv_usec = 0;
	clnt_control(cl, CLSET_TIMEOUT, (char*)&timeout_tv);		

	*cl_out = cl;
	return (CSA_SUCCESS);
}

static void
create_auth(CLIENT *cl)
{
	/* unix_credential_lock protects access to unix_credential which is
	 * initialized once and will be read-only thereafter
	 */
	static pthread_mutex_t	unix_credential_lock;

	static AUTH *unix_credential = NULL;

	pthread_mutex_lock(&unix_credential_lock);

	/* Always cache the Unix style credentials. */
	if (unix_credential == NULL)
#if defined(SunOS) || defined(USL) || defined(__uxp__)
		unix_credential = authsys_create_default ();
#else
		unix_credential = authunix_create_default ();
#endif

	cl->cl_auth = unix_credential;

	pthread_mutex_unlock(&unix_credential_lock);
}

static void
destroy_target_list(_DtCm_Target_List *tlist)
{
	_DtCm_Target_List *listp, *listitem;

	for (listp = tlist; listp != NULL; ) {
		listitem = listp;
		listp = listp->next;

		if (listitem->cal)
			free(listitem->cal);
		free(listitem);
	}
}

static void
destroy_client_info(_DtCm_Client_Info *ci)
{
	if (ci==NULL) return;

	if (ci->host != NULL)
		free(ci->host);

	if (ci->tcpcl)
		clnt_destroy(ci->tcpcl);

	if (ci->udpcl)
		clnt_destroy(ci->udpcl);

	destroy_target_list(ci->tlist);
	free(ci);
}

static void
insert_client_info(_DtCm_Client_Info **ci)
{
	_DtCm_Client_Info	*prev, *ptr;

	boolean_t		insert = _B_TRUE;
	int			res;

	/* insert new item alphabetically */
	prev = &client_cache_head;
	pthread_mutex_lock(&prev->lock_elem);
	while ((ptr = prev->next) != NULL) {
	  	pthread_mutex_lock(&ptr->lock_elem);

		/* the record may be in the list already */
		if ((res = strcmp(ptr->host, (*ci)->host)) == 0 &&
		    ptr->vers_out == (*ci)->vers_out) {

		  	pthread_mutex_lock(&ptr->lock_handle);

			/*
			 * good
			 * - get rid of the new one and use the existing one
			 * to-be-regenerated
			 * - use the new handles
			 * to-be-deleted
			 * - continue to traverse the list
			 */
			if (ptr->delete == _B_FALSE) {
				insert = _B_FALSE;

				if (ptr->udpcl == NULL) {
					ptr->udpcl = (*ci)->udpcl;
					(*ci)->udpcl = NULL;
					ptr->tcpcl = (*ci)->tcpcl;
					(*ci)->tcpcl = NULL;
				}

				/* duplicate so replace the new record
				 * with the one already in the list
				 * and increment inuse
				 */
				DP(("found duplicate record in list\n"));
				pthread_mutex_unlock(&((*ci)->lock_handle));
				destroy_client_info(*ci);
				*ci = ptr;
				pthread_mutex_unlock(&ptr->lock_elem);
				break;
			} else
				pthread_mutex_unlock(&ptr->lock_handle);

		} else if (res > 0) {
			break;
		}

		/* advance to next item */
		pthread_mutex_unlock(&prev->lock_elem);
		prev = ptr;
	}

	if (insert == _B_TRUE) {

		/* put the new record in the list */
		prev->next = *ci;
		if (ptr) {
			(*ci)->next = ptr;
			pthread_mutex_unlock(&ptr->lock_elem);
		}
	}

	pthread_mutex_unlock(&prev->lock_elem);
}

/*
 * ***
 * *** the client info record is locked by the caller
 * *** don't call a function which may try to lock the record again
 * ***
 */
static void
get_new_client_handle(_DtCm_Client_Info *ci)
{
	CLIENT		*cl;
	u_long		version;
	CSA_return_code stat;

	if (ci == NULL) return;

	version = ci->vers_out;

	/* always get a udp client handle first */
	if (create_udp_client(ci->host, version, _DtCM_INITIAL_TIMEOUT,
	    &ci->vers_out, &cl) == CSA_SUCCESS) {

		if (ci->udpcl)
			clnt_destroy(ci->udpcl);
		ci->udpcl = cl;
	} else {
		/* if nregistered == 0, mark to delete this record
		 * otherwise, just set the handles to NULL
		 */
		if (ci->nregistered == 0)
			ci->delete = _B_TRUE;
		else {
			if (ci->udpcl) {
				clnt_destroy(ci->udpcl);
				ci->udpcl = NULL;
			}
			if (ci->tcpcl) {
				clnt_destroy(ci->tcpcl);
				ci->tcpcl = NULL;
			}
		}
		return;
	}

	/* now deal with tcp handle */

	/* get rid of old handle first */
	if (ci->tcpcl) {
		clnt_destroy(ci->tcpcl);
		ci->tcpcl = NULL;
	}

	if ((stat = create_tcp_client(ci->host, ci->vers_out,
	    _DtCM_INITIAL_TIMEOUT, &cl)) == CSA_SUCCESS) { 

		ci->tcpcl = cl;

		/* check registration */
		/* if there's anything wrong, nregistered could be zero */
		check_registration(ci);

	} else if (stat != CSA_X_DT_E_SERVICE_NOT_REGISTERED) {

		/* failed to create tcp handle and the reason is
		 * not because server does not support tcp
		 */
		if (ci->nregistered == 0)
			ci->delete = _B_TRUE;
		else {
			clnt_destroy(ci->udpcl);
			ci->udpcl = NULL;
		}
	}
}

/*
 * check registration
 * Deergister the first target:
 * if it succeeded, the old server is still running, just re-register it;
 * else assume that it's a new server so re-register the whole list again.
 *
 * ***
 * *** the client info record is locked by the caller
 * *** don't call a function which may try to lock the record again
 * ***
 */
static void
check_registration(_DtCm_Client_Info *ci)
{
	_DtCm_Connection	conn;
	_DtCm_Target_List	*listp;
	CSA_return_code		stat;

	if (ci->tlist == NULL)
		return;

	conn.retry = 0;
	conn.ci = ci;

	if ((stat = _DtCm_do_unregistration(&conn, ci->tlist->cal,
	    ci->tlist->update_type)) == CSA_SUCCESS) {

		_DtCm_do_registration(&conn, ci->tlist->cal,
			ci->tlist->update_type);

	} else if (stat == CSA_E_CALLBACK_NOT_REGISTERED ||
	    stat == CSA_E_FAILURE) {
		for (listp = ci->tlist; listp != NULL;
		     listp = listp->next) {

			_DtCm_do_registration(&conn, listp->cal,
				listp->update_type);
		}
	}
}

#if !(defined(SunOS) || defined(USL) || defined(__uxp__))

/*
 * Get a client handle to a server that supports the highest
 * version between the given range.
 * This routine is used in a platform where clnt_create_vers()
 * is not available.
 */
static CSA_return_code
create_client_handle(
	const char *host,
#ifdef __osf__
	const u_int prognum,
#else
	const u_long prognum,
#endif	
	u_long *vers_outp,
	const u_long vers_low,
	const u_long vers_high,
	char *nettype,
	CLIENT **clnt)
{
	CLIENT	*cl;
	u_long	vers;
	struct timeval tv;
	enum clnt_stat status;

	tv.tv_sec = 1;
	tv.tv_usec = 0;

	*clnt = NULL;
	for (vers = vers_high; vers >= vers_low; vers--) {
#ifdef __osf__
	        if ((cl = clnt_create((char *)host, prognum, vers, nettype)) != NULL) {
#else

		if ((cl = clnt_create(host, prognum, vers, nettype)) != NULL) {
#endif
			clnt_control(cl, CLSET_TIMEOUT, (char *)&tv);
			status = clnt_call(cl, 0, (xdrproc_t) xdr_void, 
					   (char *)NULL, (xdrproc_t) xdr_void,
					   (char *)NULL, tv);

			if (status == RPC_SUCCESS) {
				*vers_outp = vers;
				*clnt = cl;
				return (CSA_SUCCESS);
			} else if (status != RPC_PROGVERSMISMATCH) {
				return (_DtCm_clntstat_to_csastat(status));
			}
		} else {
			if (strcmp(nettype, "tcp") == 0 &&
			    rpc_createerr.cf_stat != RPC_PROGNOTREGISTERED)
				_DtCm_print_errmsg(clnt_spcreateerror(host));
			return (_DtCm_clntstat_to_csastat(rpc_createerr.cf_stat));
		}
	}

	/* cannot find a server that supports a version in the given range */
	/* Probably will never get here */
	return (CSA_E_SERVICE_UNAVAILABLE);
}

#endif

static void
_increase_file_descriptor_limit()
{
	static pthread_mutex_t bumped_lock;
	static boolean_t bumped = _B_FALSE;
	struct rlimit rl;

	pthread_mutex_lock(&bumped_lock);

	if (bumped == _B_FALSE) {
		bumped = _B_TRUE;

		/* raise the soft limit of number of file descriptor */
		getrlimit(RLIMIT_NOFILE, &rl);
		rl.rlim_cur = rl.rlim_max;
		setrlimit(RLIMIT_NOFILE, &rl);
	}

	pthread_mutex_unlock(&bumped_lock);
}

