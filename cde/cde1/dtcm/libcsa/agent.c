/* $XConsortium: agent.c /main/cde1_maint/4 1995/10/10 13:27:26 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)agent.c	1.25 97/01/31 Sun Microsystems, Inc."

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <poll.h>
#if defined(SunOS) || defined(USL) || defined(__uxp__)
#include <netconfig.h>
#include <netdir.h>
#else
#include <sys/signal.h>
#include <sys/socket.h>
#include <errno.h>
#endif /* SunOS || USL || __uxp__ */

#include "agent.h"
#include "cmcb.h"
#include "calendar.h"
#include "debug.h"
#include "iso8601.h"
#include "free.h"
#include "misc.h"

#include <pthread.h>

/*
 * A bug an a version of sun libc does not have the pthread_self()
 */
#ifdef SunOS
#include <thread.h>
#define pthread_self() thr_self()
#endif

/* transient porgram number for getting callbacks from calendar servers */
u_long	_DtCm_transient1 = 0x40000000;
u_long	_DtCm_transient2 = 0x40000000;

/*
 * callback info
 * - contain update information from backend
 */
typedef struct cb_info {
	int	vers;
	char	*cal;
	char	*user;
	int	reason;
	void	*data;
	int	count;
	Calendar **clist;
	struct cb_info *next;
} _CallbackInfo;

static _CallbackInfo *cb_head = NULL, *cb_tail = NULL;

typedef struct {
	boolean_t	cb_registered;
	pthread_mutex_t	lock;	/* synchronize access to cb_registered */
} _CallbackControl;

/* cb_control controls registration and unregistration of the
 * calendar callback handler (and so the initialization of
 * the global variable _DtCm_transient1, _DtCm_transient2)
 */
static _CallbackControl cb_control;

#if defined(SunOS) || defined(USL) || defined(__uxp__)
static struct netconfig *udp_netconf;	/* both are used in init_agent */
static SVCXPRT *udp_transp;		/* and destroy_agent */
#endif /* SunOS || USL || __uxp__ */

typedef struct {
	pthread_mutex_t	lock;
	pthread_cond_t	my_turn;
	pthread_t	owner;
	int		count;
} _GetRpcLock;

static _GetRpcLock get_rpc_lock;


/******************************************************************************
 * forward declaration of static functions used within the file
 ******************************************************************************/
#if defined(SunOS) || defined(USL) || defined(__uxp__)
static int gettransient (u_long *prognum1, u_long *prognum2);
#else
static int gettransient (int proto, int *sockp, u_long *prognum1, u_long *prognum2);
#endif
static void _DtCm_handle_callback();
static CSA_return_code _ConvertCallbackData(cmcb_update_callback_args *args,
					_CallbackInfo **cbi);
static CSA_return_code _CopyAttributeNames(CSA_uint32 fnum, char **fnames,
					CSA_uint32 *tnum, char ***tnames);
static boolean_t _ProcessCallback(_CallbackInfo *ptr);
static void _FreeCallbackInfo(_CallbackInfo *ptr);
static void _InsertCbInfo(_CallbackInfo *ptr);

/*****************************************************************************
 * extern functions used in the library
 *****************************************************************************/

/*
 * svc_getreqset is not MT-safe and it cannot be called recursively
 * _DtCm_lock_get_rpc_request() and _DtCm_unlock_get_rpc_request()
 * are used to synchronize threads calling svc_getreqset().
 * Note: order of locking is important. Always acquire the lock for
 * svc_getreqset() before locking any calendar session.
 */
extern void
_DtCm_lock_get_rpc_request()
{
  	pthread_t	me = pthread_self();

	/* acquire lock */
  	pthread_mutex_lock(&get_rpc_lock.lock);

	while (get_rpc_lock.owner != NULL) {
		if (get_rpc_lock.owner == me) {
			get_rpc_lock.count++;
			goto done;
		} else {
			DP(("%d: trying to lock get rpc\n", me));
			pthread_cond_wait(&get_rpc_lock.my_turn, &get_rpc_lock.lock);
		}
	}

	get_rpc_lock.owner =  me;

done:
	DP(("%d: lock get rpc\n", me));
	pthread_mutex_unlock(&get_rpc_lock.lock);

}

extern void
_DtCm_unlock_get_rpc_request()
{
	/* release lock */
  	pthread_mutex_lock(&get_rpc_lock.lock);

	if (get_rpc_lock.count > 0)
		get_rpc_lock.count--;
	else {
		get_rpc_lock.owner = NULL;
		pthread_cond_signal(&get_rpc_lock.my_turn);
	}

	DP(("%d: unlock get rpc\n", pthread_self()));
	pthread_mutex_unlock(&get_rpc_lock.lock);
}

/*
 * Register rpc service for server callback
 * When callback is enabled (between calls to _DtCm_init_agent and
 * _DtCm_destroy_agent), the transient program numbers _DtCm_transient1
 * and _DtCm_transient2 are readonly.  Since the numbers are read only
 * after it's initialized and they will not be accessed for read when
 * they are being updated, locking them during read access is not
 * necessary.  So we only do locking in _DtCm_init_agent and
 * _DtCm_destroy_agent
 */
extern void
_DtCm_init_agent()
{
	int 		s = RPC_ANYSOCK;

	pthread_mutex_lock(&cb_control.lock);

	if (cb_control.cb_registered == _B_TRUE)
		goto done;

	DP(("libdtcm: _DtCm_init_agent\n"));

#if defined(SunOS) || defined(USL) || defined(__uxp__)
	if (gettransient(&_DtCm_transient1, &_DtCm_transient2)) {
		_DtCm_print_errmsg("Cannot get transient program number\n");
		_DtCm_print_errmsg("Callback cannot be enabled.\n");
		goto done;
	}
 
#else
        if (gettransient(IPPROTO_UDP, &s, &_DtCm_transient1, &_DtCm_transient2))
	{
		_DtCm_print_errmsg("Cannot get transient program number\n");
		_DtCm_print_errmsg("Callback cannot be enabled.\n");
		goto done;
	}

	if (rpc_reg(_DtCm_transient1, AGENTVERS, update_callback,
	    (char *(*)())_DtCm_update_callback_1, _DtCm_xdr_Table_Res_4,
	    _DtCm_xdr_Update_Status, NULL) == -1) {
		_DtCm_print_errmsg("Cannot register v1 callback handler\n");
		_DtCm_print_errmsg("Callback cannot be enabled.\n");
	}

	if (rpc_reg(_DtCm_transient2, AGENTVERS_2, CMCB_UPDATE_CALLBACK,
	    (char *(*)())cmcb_update_callback_2_svc,
	    xdr_cmcb_update_callback_args, xdr_void, NULL) == -1) {
		_DtCm_print_errmsg("Cannot register v2 callback handler\n");
		_DtCm_print_errmsg("Callback cannot be enabled.\n");
	}

#endif /* SunOS || USL || __uxp__ */
 
	cb_control.cb_registered = _B_TRUE;

done:

	pthread_mutex_unlock(&cb_control.lock);

	return;
}
 
/*
 * Unregister with the rpc service.
 */
extern void
_DtCm_destroy_agent()
{
	DP(("libdtcm: _DtCm_destroy_agent\n"));

	pthread_mutex_lock(&cb_control.lock);

	if (cb_control.cb_registered == _B_TRUE) {

#if defined(SunOS) || defined(USL) || defined(__uxp__)
	(void) rpcb_unset(_DtCm_transient1, AGENTVERS, udp_netconf);
	(void) rpcb_unset(_DtCm_transient2, AGENTVERS_2, udp_netconf);
	svc_destroy(udp_transp);
	freenetconfigent(udp_netconf);

	/* reset the transient program numbers */
	_DtCm_transient1 = 0x40000000;
	_DtCm_transient2 = 0x40000000;
#else
        (void) pmap_unset(_DtCm_transient1, AGENTVERS);
        (void) pmap_unset(_DtCm_transient2, AGENTVERS_2);
#endif /* SunOS || USL || __uxp__ */

		cb_control.cb_registered = _B_FALSE;
	}

	pthread_mutex_unlock(&cb_control.lock);
}

/*
 * _DtCm_lock_get_rpc_request() should have been called by
 * the caller to acquire the lock for synchronizing the
 * call to svc_getreqset()
 */
extern void
_DtCm_process_updates()
{
	int	i, j, nfd;
	fd_set	rpc_bits;
	fd_mask	fmask, *inbits;
	struct	pollfd pfd[FD_SETSIZE];
	struct	pollfd *p;
	int	last;
	int	do_rpc;

	while (_B_TRUE) {
		rpc_bits = svc_fdset;

		/* convert to pollfd structure */
		inbits = rpc_bits.fds_bits;
		p = pfd;
		for (i = 0; i < FD_SETSIZE; i += NFDBITS) {
			fmask = *inbits;
			for (j = 0; fmask != 0 ; j++, fmask >>= 1) {
				if (fmask & 0x1) {
					p->fd = i + j;
					if (p->fd >= FD_SETSIZE)
						break;
					p->events = POLLIN;
					p++;
				}
			}
			inbits++;
		}

		/* poll and return right away */
		i = p - pfd;

		nfd = poll(pfd, i, 0);

		if (nfd <= 0)
			/* done */
			break;

		/* if set, handle rpc calls */

		/* convert back to fd_set structure */
		last = -1;
		do_rpc = 0;
		for (p = pfd, FD_ZERO(&rpc_bits); i-- > 0; p++) {
			j = p->fd / NFDBITS;
			if (j != last) {
				inbits = &rpc_bits.fds_bits[j];
				last = j;
			}
			if (p->revents & POLLIN) {
				*inbits |= (1 << (p->fd % NFDBITS));
				do_rpc = 1;
			}
		}

		if (do_rpc)
			svc_getreqset(&rpc_bits);
	}

	/* process any left over events */
	_DtCm_handle_callback();
}

/*
 * This routine is invoked by the rpc library when
 * a callback is received from a calendar server of
 * version 2 to 4
 */
Update_Status *
_DtCm_update_callback_1(Table_Res_4 *t, void *dummy)
{
	static Update_Status status = update_succeeded;
	_CallbackInfo *cbi;

	DP(("agent.c: _DtCm_update_callback_1()\n"));

	/*
	 * no point to save data for version 4 and before
	 * since the info from old backends does not contain
	 * calendar info
	 * so we just invoke all registered callback with no data
	 */
	if (cbi = (_CallbackInfo *)calloc(1, sizeof(_CallbackInfo))) {
		cbi->vers = AGENTVERS;
		cbi->reason = CSA_CB_ENTRY_ADDED | CSA_CB_ENTRY_DELETED |
				CSA_CB_ENTRY_UPDATED;

		_InsertCbInfo(cbi);

		/* handle callback from backend */
		_DtCm_handle_callback();
	}

	return (&status);
}

/*
 * This routine is invoked by the rpc library when
 * a callback is received from a calendar server of
 * version 5
 */
void *
cmcb_update_callback_2_svc(cmcb_update_callback_args *args, struct svc_req *d)
{
	_CallbackInfo *cbi;

	DP(("agent.c: cmcb_update_callback_2_svc()\n"));

	if (args == NULL)
		return (NULL);

	if (_ConvertCallbackData(args, &cbi) == CSA_SUCCESS) {
		cbi->vers = AGENTVERS_2;

		_InsertCbInfo(cbi);

		/* handle callback from backend */
		_DtCm_handle_callback();
	}

	return (NULL);
}

/******************************************************************************
 * static functions used within in the file
 ******************************************************************************/

#if defined(SunOS) || defined(USL) || defined(__uxp__)

/*
 * This routine gets called when an rpc request is received from
 * a server.  It checks the version and procedure requested and
 * invokes the appropriate callback handling routine.
 */
static void
dispatcher(struct svc_req *rqstp, register SVCXPRT *transp)
{
	Table_Res_4			arg1;
	cmcb_update_callback_args	arg2;
	char				*result;

	if (rqstp->rq_vers == AGENTVERS && rqstp->rq_proc == update_callback) {

		memset((char *)&arg1, 0, sizeof(arg1));
		if (!svc_getargs(transp, (xdrproc_t)_DtCm_xdr_Table_Res_4,
		    (caddr_t)&arg1)) {
			svcerr_decode(transp);
			return;
		}

		if (result = (char *)_DtCm_update_callback_1(&arg1, rqstp))
			svc_sendreply(transp, _DtCm_xdr_Update_Status, result);

	} else if (rqstp->rq_vers == AGENTVERS_2 &&
	    rqstp->rq_proc == CMCB_UPDATE_CALLBACK) {

		memset((char *)&arg2, 0, sizeof(arg2));
		if (!svc_getargs(transp,
		    (xdrproc_t)xdr_cmcb_update_callback_args, (caddr_t)&arg2)) {
			svcerr_decode(transp);
			return;
		}

		if (result = (char *)cmcb_update_callback_2_svc(&arg2, rqstp))
			svc_sendreply(transp, xdr_void, result);

	} else if (rqstp->rq_proc == NULLPROC) {
		/* by convention, NULLPROC is for pinging */
		svc_sendreply(transp, xdr_void, (char *)NULL);
	} else
		svcerr_noproc(transp);
}

/*
 * get transient program number for callbacks.
 */
static int
gettransient (u_long *prognum1, u_long *prognum2)
{
	int stat;
	struct nd_hostserv host = {HOST_SELF, "rpcbind"};
	struct nd_addrlist *addrp;
        struct netbuf    *addr;

	/* udp_netconf will be freed in _DtCm_destroy_agent */
	if ((udp_netconf = getnetconfigent("udp")) == NULL) {
		DP(("getnetconfigent(udp) failed\n"));
		return -1;
	}

	/* udp_transp will be destroyed in _DtCm_destroy_agent */
	if ((udp_transp = svc_tli_create(RPC_ANYFD, udp_netconf,
	    (struct t_bind*)NULL, 0, 0)) == NULL) {
		DP(("svc_tli_create failed\n"));
		freenetconfigent(udp_netconf);
		return -1;
	}

	/* get program number for version 1 */
	while (!svc_reg(udp_transp, (*prognum1)++, AGENTVERS, dispatcher,
	    udp_netconf)) {
		continue;
	}
	(*prognum1)--;

	/* get program number for version 2 */
	while (!svc_reg(udp_transp, (*prognum2)++, AGENTVERS_2, dispatcher,
	    udp_netconf)) {
		continue;
	}
	(*prognum2)--;

	return 0;
}

#else /* SunOS || USL || __uxp__ */

static int
gettransient (int proto, int *sockp, u_long *prognum1, u_long *prognum2)
{
	int s, len, socktype;
	struct sockaddr_in addr;

	switch (proto) {
		case IPPROTO_UDP:
			socktype = SOCK_DGRAM;
			break;
		case IPPROTO_TCP:
			socktype = SOCK_STREAM;
			break;
		default:
			DP(("unknown protocol type\n"));
			return -1;
	}

	if (*sockp == RPC_ANYSOCK) {
		if ((s = socket(AF_INET, socktype, 0)) < 0) {
			perror("socket");
			return -1;
		}
		*sockp = s;
	} else
		s = *sockp;
	
	addr.sin_addr.s_addr = 0;
	addr.sin_family = AF_INET;
	addr.sin_port = 0;
	len = sizeof(addr);

	if (bind(s, (struct sockaddr *)&addr, len) != 0) {
		perror("bind");
		return -1;
	}

	if (getsockname(s, (struct sockaddr *)&addr, &len) < 0) {
		perror("getsockname");
		return -1;
	}

	/* get program number for version 1 */
	while (!pmap_set((*prognum1)++, AGENTVERS, proto, ntohs(addr.sin_port)))
		continue;
	(*prognum1)--;

	/* get program number for version 2 */
	while (!pmap_set((*prognum2)++, AGENTVERS_2, proto, ntohs(addr.sin_port)))
		continue;
	(*prognum2)--;

	return (0);
}

#endif /* not SunOS || USL || __uxp__ */

static void
_DtCm_handle_callback()
{
	_CallbackInfo		*ptr, *old = NULL;

	DP(("agent.c: _DtCm_handle_callback()\n"));

	/* since only one thread can be in the callback code
	 * there's no need to synchronize access to the callbcak list
	 */
	while ((ptr = cb_head) != NULL) {

		/* take the first one out */
		cb_head = ptr->next;

		if (_ProcessCallback(ptr) == _B_TRUE) {
			/* put it back in the list */
			if (old == NULL)
				old = ptr;
			else
				cb_tail->next = ptr;
			cb_tail = ptr;
		} else
			_FreeCallbackInfo(ptr); /* may need to put it back */
	}

	if (old == NULL)
		cb_tail = NULL;
	else
		cb_head = old;
}

static CSA_return_code
_ConvertCallbackData(cmcb_update_callback_args *args, _CallbackInfo **cbi)
{
	_CallbackInfo					*ncbi;
	CSA_calendar_user				*user;
	CSA_logon_callback_data				*ldata;
	CSA_calendar_deleted_callback_data		*rdata;
	CSA_calendar_attr_update_callback_data	*cdata;
	CSA_add_entry_callback_data			*adata;
	CSA_delete_entry_callback_data			*ddata;
	CSA_update_entry_callback_data			*udata;
	char						timebuf[BUFSIZ];
	
	if ((ncbi = (_CallbackInfo *)calloc(1, sizeof(_CallbackInfo))) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	if (args->calendar && (ncbi->cal = strdup(args->calendar)) == NULL) {
		free(ncbi);
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	if (args->user && (ncbi->user = strdup(args->user)) == NULL) {
		_FreeCallbackInfo(ncbi);
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	if ((user = (CSA_calendar_user *)calloc(1, sizeof(CSA_calendar_user)))
	    == NULL) {
		_FreeCallbackInfo(ncbi);
		return (CSA_E_INSUFFICIENT_MEMORY);
	} else
		user->user_name = ncbi->user;

	ncbi->reason = args->data.reason;
	switch (ncbi->reason) {
	case CSA_CB_CALENDAR_LOGON:
		if ((ldata = (CSA_logon_callback_data *)calloc(1,
		    sizeof(CSA_logon_callback_data))) == NULL) {
			free(user);
			_FreeCallbackInfo(ncbi);
			return (CSA_E_INSUFFICIENT_MEMORY);
		}
		ldata->user = user;
		ncbi->data = (void *)ldata;
		break;

	case CSA_CB_CALENDAR_DELETED:
		if ((rdata = (CSA_calendar_deleted_callback_data *)calloc(1,
		    sizeof(CSA_calendar_deleted_callback_data))) == NULL) {
			free(user);
			_FreeCallbackInfo(ncbi);
			return (CSA_E_INSUFFICIENT_MEMORY);
		}
		rdata->user = user;
		ncbi->data = (void *)rdata;
		break;

	case CSA_CB_CALENDAR_ATTRIBUTE_UPDATED:
		if ((cdata = (CSA_calendar_attr_update_callback_data *)
		    calloc(1, sizeof(
		    CSA_calendar_attr_update_callback_data))) == NULL) {
			free(user);
			_FreeCallbackInfo(ncbi);
			return (CSA_E_INSUFFICIENT_MEMORY);
		}
		cdata->user = user;
		ncbi->data = (void *)cdata;

		if (_CopyAttributeNames(args->data.data.cdata->num_names,
		    args->data.data.cdata->names, &cdata->number_attributes,
		    &cdata->attribute_names)) {
			_FreeCallbackInfo(ncbi);
			return (CSA_E_INSUFFICIENT_MEMORY);
		}

		break;

	case CSA_CB_ENTRY_ADDED:
		if ((adata = (CSA_add_entry_callback_data *)calloc(1,
		    sizeof(CSA_add_entry_callback_data))) == NULL) {
			free(user);
			_FreeCallbackInfo(ncbi);
			return (CSA_E_INSUFFICIENT_MEMORY);
		}
		adata->user = user;
		ncbi->data = (void *)adata;

		if (args->data.data.adata->id && (adata->added_entry_id.data =
		    (unsigned char *)strdup(args->data.data.adata->id))
		    == NULL) {
			_FreeCallbackInfo(ncbi);
			return (CSA_E_INSUFFICIENT_MEMORY);
		} else
			adata->added_entry_id.size =
				strlen((char *)adata->added_entry_id.data);

		break;

	case CSA_CB_ENTRY_DELETED:
		if ((ddata = (CSA_delete_entry_callback_data *)calloc(1,
		    sizeof(CSA_delete_entry_callback_data))) == NULL) {
			free(user);
			_FreeCallbackInfo(ncbi);
			return (CSA_E_INSUFFICIENT_MEMORY);
		}
		ddata->user = user;
		ncbi->data = (void *)ddata;

		if (args->data.data.ddata->id && (ddata->deleted_entry_id.data =
		    (unsigned char *)strdup(args->data.data.ddata->id))
		    == NULL) {
			_FreeCallbackInfo(ncbi);
			return (CSA_E_INSUFFICIENT_MEMORY);
		} else
			ddata->deleted_entry_id.size =
				strlen((char *)ddata->deleted_entry_id.data);

		_csa_tick_to_iso8601(args->data.data.ddata->time, timebuf);
		if ((ddata->date_and_time = strdup(timebuf)) == NULL) {
			_FreeCallbackInfo(ncbi);
			return (CSA_E_INSUFFICIENT_MEMORY);
		}

		ddata->scope = args->data.data.ddata->scope;
		break;

	case CSA_CB_ENTRY_UPDATED:
		if ((udata = (CSA_update_entry_callback_data *)calloc(1,
		    sizeof(CSA_update_entry_callback_data))) == NULL) {
			free(user);
			_FreeCallbackInfo(ncbi);
			return (CSA_E_INSUFFICIENT_MEMORY);
		}
		udata->user = user;
		ncbi->data = (void *)udata;

		if (args->data.data.udata->newid && (udata->new_entry_id.data =
		    (unsigned char *)strdup(args->data.data.udata->newid))
		    == NULL) {
			_FreeCallbackInfo(ncbi);
			return (CSA_E_INSUFFICIENT_MEMORY);
		} else
			udata->new_entry_id.size =
				strlen((char *)udata->new_entry_id.data);

		if (args->data.data.udata->oldid && (udata->old_entry_id.data =
		    (unsigned char *)strdup(args->data.data.udata->oldid))
		    == NULL) {
			_FreeCallbackInfo(ncbi);
			return (CSA_E_INSUFFICIENT_MEMORY);
		} else
			udata->old_entry_id.size =
				strlen((char *)udata->old_entry_id.data);

		_csa_tick_to_iso8601(args->data.data.udata->time, timebuf);
		if ((udata->date_and_time = strdup(timebuf)) == NULL) {
			_FreeCallbackInfo(ncbi);
			return (CSA_E_INSUFFICIENT_MEMORY);
		}

		udata->scope = args->data.data.udata->scope;
		break;
	}

	*cbi = ncbi;
	return (CSA_SUCCESS);
}

static CSA_return_code
_CopyAttributeNames(
	CSA_uint32	fnum,
	char		**fnames,
	CSA_uint32	*tnum,
	char		***tnames)
{
	int	i;
	char	**nnames;

	if (fnum == 0) {
		*tnum = 0;
		*tnames = NULL;
		return (CSA_SUCCESS);
	}

	if ((nnames = calloc(1, sizeof(char *) * fnum)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	for (i = 0; i < fnum; i++) {
		if ((nnames[i] = strdup(fnames[i])) == NULL)
			break;
	}

	if (i == fnum) {
		*tnum = i;
		*tnames = nnames;
		return (CSA_SUCCESS);
	} else {
		 _DtCm_free_character_pointers(i, nnames);
		free(nnames);
		return (CSA_E_INSUFFICIENT_MEMORY);
	}
}

static boolean_t
_ProcessCallback(_CallbackInfo *ptr)
{
	Calendar		*cal;
	_DtCmCallbackEntry	*cb;
	boolean_t		keep = _B_FALSE;
	int			i = 0;

	for (i = 0; i < ptr->count; i++) {

		if (ptr->clist[i] == NULL ||
		    _DtCm_get_calendar((CSA_session_handle)ptr->clist[i],
		    _B_FALSE, &cal) != CSA_SUCCESS)
			continue;

		if ((cal->do_reasons & ptr->reason) == 0) {
			/* keep it, will be processed when the
			 * the application calls csa_call_callbacks()
			 */
			keep = _B_TRUE;
			_DtCm_release_calendar(cal);
			continue;
		}

		for (cb = cal->cb_list; cb != NULL; cb = cb->next) {
			if (ptr->reason & cb->reason) {
				if (ptr->vers == AGENTVERS) {
					cb->handler((CSA_session_handle)cal,
						cal->do_reasons & cb->reason,
						(CSA_buffer) NULL,
						cb->client_data,
						(CSA_extension*) NULL);
				} else {
					cb->handler((CSA_session_handle)cal,
						ptr->reason,
						(CSA_buffer)ptr->data,
						cb->client_data,
						(CSA_extension*) NULL);
				}
			}
		}
		ptr->clist[i] = NULL;

		/* release session lock */
		_DtCm_release_calendar(cal);
	}

	return (keep);
}

static void
_FreeCallbackInfo(_CallbackInfo *ptr)
{
	CSA_logon_callback_data				*ldata;
	CSA_calendar_deleted_callback_data		*rdata;
	CSA_calendar_attr_update_callback_data	*cdata;
	CSA_add_entry_callback_data			*adata;
	CSA_delete_entry_callback_data			*ddata;
	CSA_update_entry_callback_data			*udata;

	if (ptr) {
		if (ptr->cal) free(ptr->cal);
		if (ptr->user) free(ptr->user);

		if (ptr->data) switch (ptr->reason) {
		case CSA_CB_CALENDAR_LOGON:
			ldata = ptr->data;
			if (ldata->user) free(ldata->user);
			free(ldata);
			break;
		case CSA_CB_CALENDAR_DELETED:
			rdata = ptr->data;
			if (rdata->user) free(rdata->user);
			free(rdata);
			break;
		case CSA_CB_CALENDAR_ATTRIBUTE_UPDATED:
			cdata = (CSA_calendar_attr_update_callback_data *)
				ptr->data;
			if (cdata->user) free(cdata->user);
			if (cdata->number_attributes > 0)
				_DtCm_free_character_pointers(
					cdata->number_attributes,
					cdata->attribute_names);
			free(cdata);
			break;
		case CSA_CB_ENTRY_ADDED:
			adata = (CSA_add_entry_callback_data *)ptr->data;
			if (adata->user) free(adata->user);
			if (adata->added_entry_id.data)
				free(adata->added_entry_id.data);
			free(adata);
			break;
		case CSA_CB_ENTRY_DELETED:
			ddata = (CSA_delete_entry_callback_data *)ptr->data;
			if (ddata->date_and_time) free(ddata->date_and_time);
			if (ddata->user) free(ddata->user);
			if (ddata->deleted_entry_id.data)
				free(ddata->deleted_entry_id.data);
			free(ddata);
			break;
		case CSA_CB_ENTRY_UPDATED:
			udata = (CSA_update_entry_callback_data *)ptr->data;
			if (udata->user) free(udata->user);
			if (udata->date_and_time) free(udata->date_and_time);
			if (udata->old_entry_id.data)
				free(udata->old_entry_id.data);
			if (udata->new_entry_id.data)
				free(udata->new_entry_id.data);
			free(udata);
		}

		free(ptr->clist);
		free(ptr);
	}
}

/*
 * add calendar information and
 * insert the record at the end of the list
 */
static void
_InsertCbInfo(_CallbackInfo *cbi)
{
	Calendar	*prev, *ptr;
	int		i;

	/* add calendar handles to the record */
	pthread_mutex_lock(&_DtCm_active_cal_list.lock);
	if (_DtCm_active_cal_list.access == 0) {
		pthread_mutex_unlock(&_DtCm_active_cal_list.lock);
		return;
	}

	cbi->count = _DtCm_active_cal_list.access;
	cbi->clist = (Calendar **)calloc(1, sizeof(Calendar*)*(cbi->count));

	prev = &_DtCm_active_cal_list;
	i = 0;
	while ((ptr = prev->next) != NULL) {
		pthread_mutex_lock(&ptr->lock);

		if (((cbi->cal && (strcmp(cbi->cal, ptr->name) == 0)) ||
		    (cbi->cal == NULL && ptr->rpc_version <
		     _DtCM_FIRST_EXTENSIBLE_SERVER_VERSION)) &&
		    (ptr->all_reasons & cbi->reason)) {
			cbi->clist[i++] = ptr;
		}

		pthread_mutex_unlock(&prev->lock);
		prev = ptr;
	}
	pthread_mutex_unlock(&prev->lock);

	/* since only one thread can be in the callback code
	 * there's no need to synchronize access to the callbcak list
	 */
	if (cb_tail == NULL)
		cb_head = cbi;
	else
		cb_tail->next = cbi;

	cb_tail = cbi;
}


