/* $XConsortium: Svc.c /main/cde1_maint/1 1995/07/17 18:15:50 drk $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **   File:         Svc.c
 **
 **   Project:	    DT
 **
 **   Description:  Point-to-point interclient messaging routines.
 **		  
 **
 **   (c) Copyright 1993 by Hewlett-Packard Company
 **
 *
 ****************************************************************************
 ************************************<+>*************************************/

/* 
 * Include files
 */
#include <Dt/Msg.h>
#include <Dt/ServiceP.h>
#include <Dt/SvcTT.h>

/*
 * _DtSvcNewHandle -- Get a new service handle
 */
DtSvcHandle 
#ifdef _NO_PROTO
_DtSvcNewHandle (pchName, widget)
	char *			pchName;
	Widget			widget;
#else
_DtSvcNewHandle (
	char *			pchName, 
	Widget 			widget
)
#endif
{
    DtSvcHandle service;

    service = XtNew(struct _DtSvcHandle);
    H_handle(service) = _DtMsgHandleAllocate (pchName, widget);
    H_service_data(service) = NULL;
    H_notify_data(service) = NULL;
    H_use_tt(service) = getenv("USE_DtMsg") == 0;
    if (   (pchName != 0) && (!strcmp (pchName, "DTPAD"))
	&& (getenv("DTPAD_DtMsg")))
    {
	H_use_tt(service) = False;
    }
    H_pat(service) = NULL;

    if (H_use_tt(service))
    {
	/*
	 * Make sure we have a ToolTalk connection.
	 */
	if (_DtSvcInitToolTalk (widget) != TT_OK)
	{
	    _DtSvcDestroyHandle (service);
	    return (0);
	}
    }

    return (service);
}



/* 
 * _DtSvcDestroyServiceData -- Destroy a service handle's service data
 */
static void
#ifdef _NO_PROTO
_DtSvcDestroyServiceData (service)
	DtSvcHandle	service;
#else
_DtSvcDestroyServiceData (
	DtSvcHandle	service
)
#endif
{
    int i;
    DtSvcFilterEntry *pF;

    if (service && H_service_data(service))
    {
	/* take care of data pointers inside the service data */
	if(H_filters(service))
	{
	    pF = H_filters(service);
	    for (i=0; i < H_num_filters(service); i++)
	    {
		if (F_request(&pF[i]))
		{
		    XtFree ((char *) F_request(&pF[i]));
		}
	    }
	    XtFree ((char *) H_filters(service));
	}
	XtFree ((char *) H_service_data (service));
	
	H_service_data(service) = NULL;
    }
}

/* 
 * DtSvcDestroyNotifyData -- Destroy a service handle's notify data
 *
 * (This is in this module to avoid linking in the notify
 *  module unnecessarily)
 */
static void
#ifdef _NO_PROTO
_DtSvcDestroyNotifyData (ngroup)
	DtSvcHandle	ngroup;
#else
_DtSvcDestroyNotifyData (
	DtSvcHandle	ngroup
)
#endif
{
    int i;
    DtSvcNotifyFilterEntry *pF;

    if (ngroup && H_notify_data(ngroup))
    {
	/* take care of data pointers inside the notify data */
	if(H_notify_filters(ngroup))
	{
	    pF = H_notify_filters(ngroup);
	    for (i=0; i < H_num_notify_filters(ngroup); i++)
	    {
		if (N_notify(&pF[i]))
		{
		    XtFree ((char *) N_notify(&pF[i]));
		}
	    }
	    XtFree ((char *) H_notify_filters(ngroup));
	}
	XtFree ((char *) H_notify_data (ngroup));
	
	H_notify_data(ngroup) = NULL;
    }
}


/* 
 * _DtSvcDestroyHandle -- Destroy a previously allocated handle
 */
void
#ifdef _NO_PROTO
_DtSvcDestroyHandle (service)
	DtSvcHandle	service;
#else
_DtSvcDestroyHandle (
	DtSvcHandle	service
)
#endif
{
    if (service)
    {
	if (H_service_data(service))
	{
	    _DtSvcDestroyServiceData (service);
	}

	if (H_notify_data(service))
	{
	    _DtSvcDestroyNotifyData (service);
	}
	_DtMsgHandleDestroy (H_handle(service));
	if (H_pat(service))
	{
	    tt_pattern_destroy (H_pat(service));
	    H_pat(service) = 0;
	}
    }
}

/* 
 * _DtSvcGetName -- Return the string name for a service from the
 * 		    handle
 */
char *
#ifdef _NO_PROTO
_DtSvcGetName (service)
	DtSvcHandle	service;
#else
_DtSvcGetName (
	DtSvcHandle	service
)
#endif
{
    return ((String) _DtMsgHandleGetName (H_handle(service)));
}


/* 
 * _DtSvcGetWidget -- Return the widget for a service from the handle
 */
Widget
#ifdef _NO_PROTO
_DtSvcGetWidget (service)
	DtSvcHandle	service;
#else
_DtSvcGetWidget (
	DtSvcHandle	service
)
#endif
{
    return (_DtMsgHandleGetWidget (H_handle(service)));
}

/*
 * _DtSvcFilterPass -- Return True if this filter passes this message
 */
static Boolean
#ifdef _NO_PROTO
_DtSvcFilterPass (pFilter, ppchMessage, count)
    DtSvcFilterEntry * pFilterEntry;
    char **		ppchMessage;
    int			count;
#else /* _NO_PROTO */
_DtSvcFilterPass (
    DtSvcFilterEntry * pFilterEntry,
    char **		ppchMessage,
    int			count 
    )
#endif /* _NO_PROTO */
{
   Boolean bPass = False;

   if (F_request(pFilterEntry))
   {
       if (!strcmp(F_request(pFilterEntry), ppchMessage[0]))
       {
	   /* matched request string, pass this message */
	   bPass = True;
       }
   }
   else
   {
       /* null filter, this filter passes every message */
       bPass = True;
   }

   return (bPass);
}


/*
 * _DtSvcFilterRequest -- Filter the request and return
 * 			the correct receive procedure and client data
 */
static void
#ifdef _NO_PROTO
_DtSvcFilterRequest (service, ppchMessage, count, 
			pReceiveProc, pclient_data)
    DtSvcHandle	service;
    char **		ppchMessage;
    int			count;
    DtSvcReceiveProc *	pReceiveProc;
    Pointer *		pclient_data;
#else /* _NO_PROTO */
_DtSvcFilterRequest (
    DtSvcHandle	service,
    char **		ppchMessage,
    int			count,
    DtSvcReceiveProc *	pReceiveProc,
    Pointer *		pclient_data 
    )
#endif /* _NO_PROTO */
{
    DtSvcReceiveProc 	receive_proc = NULL;
    Pointer		receive_client_data = NULL;
    int 		i, num_filters;
    DtSvcFilterList	pFilters;

    /* insure minimal amount to continue */
    if (service && (count > 0) &&
	ppchMessage && *ppchMessage)
    {
	/*
	 * Search backward through the list of filters to find a matching
	 * one. The zeroth element of the list is the default handler.
	 */

	pFilters = H_filters(service);
	num_filters = H_num_filters(service);

	for (i = (num_filters - 1); i >= 0; i--)
	{
	    if (_DtSvcFilterPass (&pFilters[i], ppchMessage, count))
	    {
		receive_proc = F_receive_proc(&pFilters[i]);
		receive_client_data = F_client_data(&pFilters[i]);
		break;
	    }
	}
    }

    /* return whatever we found */
    *pReceiveProc = receive_proc;
    *pclient_data = receive_client_data;
}

/*
 * _DtSvcRequestHandler -- handle requests here, distribute based
 * 			on how filtering is to be done.
 */
void
#ifdef _NO_PROTO
_DtSvcRequestHandler (handle, replyContext, pClientData, 
			ppchMessage, count)
    DtMsgHandle	handle;
    DtMsgContext	replyContext;
    Pointer		pClientData;
    char **		ppchMessage;
    int			count;
#else /* _NO_PROTO */
_DtSvcRequestHandler (
    DtMsgHandle	handle,
    DtMsgContext	replyContext,
    Pointer		pClientData,
    char **		ppchMessage,
    int			count 
    )
#endif /* _NO_PROTO */
{
	int i;
    DtSvcHandle	service = (DtSvcHandle) pClientData;
    DtSvcReceiveProc	receive_proc;
    Pointer		receive_client_data;

    if (service)
    {
	_DtSvcFilterRequest (service, ppchMessage, count, &receive_proc, 
			    &receive_client_data);

	if (receive_proc)
	{
	    _DtSvcLogMsg ("_DtSvcRequestHandler", service, 0, count,
			  ppchMessage);
	    (*(receive_proc)) (service, (DtSvcMsgContext) replyContext, 
				receive_client_data, ppchMessage, count);
	}
    }
}

/*
 * _DtSvcLoseHandler -- handle loss of service
 */
static void
#ifdef _NO_PROTO
_DtSvcLoseHandler (handle, status, client_data)
    DtMsgHandle	handle;
    int 		status;
    Pointer		client_data;
#else /* _NO_PROTO */
_DtSvcLoseHandler (
    DtMsgHandle	handle,
    int 		status,
    Pointer		client_data 
    )
#endif /* _NO_PROTO */
{
    DtSvcHandle	service = (DtSvcHandle) client_data;
    DtSvcStatusProc	lose_proc;
    Pointer		lose_client_data;

    if (service)
    {
	lose_proc = H_lose_proc(service);
	lose_client_data = H_lose_client_data(service);

	if (lose_proc)
	{
	    (*(lose_proc)) (service, status, lose_client_data);
	}
    }
}

/* 
 * _DtSvcRegister -- register a service and prepare to receive reqeusts
 *
 * 
 */
int
#ifdef _NO_PROTO
_DtSvcRegister (service, assert, receive_proc, receive_client_data,
		lose_proc, lose_client_data)
    DtSvcHandle	service;
    Boolean		assert;
    DtSvcReceiveProc	receive_proc;
    Pointer		receive_client_data;
    DtSvcStatusProc	lose_proc;
    Pointer		lose_client_data;
#else /* _NO_PROTO */
_DtSvcRegister (
    DtSvcHandle	service,
    Boolean		assert,
    DtSvcReceiveProc	receive_proc,
    Pointer		receive_client_data,
    DtSvcStatusProc	lose_proc,
    Pointer		lose_client_data)
#endif /* _NO_PROTO */
{
    int status;
    DtSvcFilterEntry *pFilter;

    /* validate service handle */
    if (!service) return (DT_SVC_FAIL);

    if (H_service_data(service))
    {
	/* 
	 * Already registered something..
	 * Replace default message handler with this one.
	 */
	pFilter = H_filters(service);

	status = dtmsg_SUCCESS;
    }
    else
    {
	/* allocate new service data and initialize */
	H_service_data(service) = XtNew(DtSvcData);

	/* initialize filter list by starting out with the unfiltered
	 * request handler.
	 */
	pFilter = XtNew(DtSvcFilterEntry);
	H_filters(service) = pFilter;
	H_num_filters(service) = 1;
	H_size_filters(service) = 1;

	if (H_use_tt(service))
	{
	    status = _DtSvcRegisterTT (service, TT_HANDLE);
	}
	else
	{
	    /* 
	     * Offer this service to the world.
	     *
	     * Use common receive proc at this level.
	     * This receive proc will implement the message filtering
	     * and properly distribute the callbacks.
	     */
	    status = _DtMsgServiceOffer (H_handle(service), assert,
				_DtSvcRequestHandler, (Pointer) service,
				_DtSvcLoseHandler, (Pointer) service);
	}

    }

    /* set up default "filter" */
    F_request(pFilter) = NULL; 	/* no filter specified */
    F_receive_proc(pFilter) = receive_proc;
    F_client_data(pFilter) = receive_client_data;

    H_lose_proc(service) = lose_proc;
    H_lose_client_data(service) = lose_client_data;

    switch (status)
    {
	case dtmsg_SUCCESS:
	    status = DT_SVC_SUCCESS;
	    break;

	default:
	case dtmsg_FAIL:
	case dtmsg_ANOTHER_PROVIDER:
	    status = DT_SVC_FAIL;
	    _DtSvcDestroyServiceData (service);
	    break;
    }

    return (status);
}

/* 
 * _DtSvcRequestRegister -- add a request filter to a service
 *
 * 
 */
void
#ifdef _NO_PROTO
_DtSvcRequestRegister (service, request, receive_proc, receive_client_data)
    DtSvcHandle	service;
    String		request;
    DtSvcReceiveProc	receive_proc;
    Pointer		receive_client_data;
#else /* _NO_PROTO */
_DtSvcRequestRegister (
    DtSvcHandle	service,
    String		request,
    DtSvcReceiveProc	receive_proc,
    Pointer		receive_client_data 
    )
#endif /* _NO_PROTO */
{
    DtSvcFilterList pFilters;
    DtSvcFilterEntry * pFE;
    int i;
    Boolean bReplaced;

    if (!H_service_data(service))
    {
	/*
	 * Base level registration not yet done...
	 * Do minimalist registration to allocate data. Register
	 * no default message handler. Also, there is no lose_proc
	 * registered.
	 */
	(void) _DtSvcRegister (service, False, NULL, NULL, NULL, NULL);
    }

    if (H_service_data(service))
    {
	pFilters = H_filters(service);
	bReplaced = False;

	/*
	 * Check for redundant entry.
	 * (Remember, zeroth entry is default handler and doesn't
	 *  need checking).
	 */
	for (i = (H_num_filters(service))-1; i > 0; i--)
	{
	    pFE = &pFilters[i];
	    if (!strcmp(F_request(pFE), request))
	    {
		F_receive_proc(pFE) = receive_proc;
		F_client_data(pFE) = receive_client_data;
		bReplaced = True;
	    }
	}

	if (!bReplaced)
	{
	    /*
	     * Need to add a new filter entry
	     */
	    if (H_num_filters(service) >= H_size_filters(service))
	    {
		/* allocate space for more */
		H_size_filters(service) += F_alloc_amt;
		pFilters = (DtSvcFilterEntry *) XtRealloc (
	 		(char *) H_filters(service), 
			(H_size_filters(service)*sizeof (DtSvcFilterEntry)));
		H_filters(service) = pFilters;
	    }
	    H_num_filters(service) += 1;
	    i = H_num_filters(service) - 1;
	    pFE = &pFilters[i];

	    F_request(pFE) = XtNewString(request);
	    F_receive_proc(pFE) = receive_proc;
	    F_client_data(pFE) = receive_client_data;
	}
    }
}

/* 
 * _DtSvcUnregister -- unregister a service so no more requests will
 *			be received
 */
void
#ifdef _NO_PROTO
_DtSvcUnregister (service)
    DtSvcHandle	service;
#else /* _NO_PROTO */
_DtSvcUnregister (
    DtSvcHandle	service
    )
#endif /* _NO_PROTO */
{
    if (service)
    {
	_DtSvcDestroyServiceData (service);
	if (H_use_tt(service))
	{
	    tt_pattern_destroy (H_pat(service));
	    H_pat(service) = 0;
	}
	else
	{
	    _DtMsgServiceWithdraw (H_handle(service));
	}
    }
}

/* 
 * _DtSvcIsRegistered -- Returns "true" if a service is registered
 */
Boolean
#ifdef _NO_PROTO
_DtSvcIsRegistered (service)
    DtSvcHandle	service;
#else /* _NO_PROTO */
_DtSvcIsRegistered (
    DtSvcHandle	service
    )
#endif /* _NO_PROTO */
{
    if (! service) return False;
    if (H_use_tt(service))
    {
	abort();
    }
    else
    {
        return _DtMsgServiceIsOffered(H_handle(service));
    }
}


/*
 * _DtSvcReplyHandler -- handle replies to requests. 
 *			Interpret the reply and call the 
 *			success or failure proc.
 */
void
#ifdef _NO_PROTO
_DtSvcReplyHandler (handle, msg_context, client_data, ppchMessage, count)
    DtMsgHandle	handle;
    DtMsgContext	msg_context;
    Pointer		client_data;
    char **		ppchMessage;
    int			count;
#else /* _NO_PROTO */
_DtSvcReplyHandler (
    DtMsgHandle	handle,
    DtMsgContext	msg_context,
    Pointer		client_data,
    char **		ppchMessage,
    int			count 
    )
#endif /* _NO_PROTO */
{
    DtSvcReplyData *	pRD;

    pRD = (DtSvcReplyData *) client_data;

    if (pRD)
    {
	/*
	 * Determine if this was a success or failure reply.
	 */
	if ((count > 0) && ppchMessage && *ppchMessage)
	{
	    if (!(strcmp (ppchMessage[0], DT_SVC_MSG_FAIL)))
	    {
                if (R_fail_proc(pRD))
		   (*(R_fail_proc(pRD))) (R_service(pRD),
				R_fail_client_data(pRD),
				ppchMessage, count);
	    }
	    else
	    {
		/*
		 * It's not a failure, so consider it a success.
		 * (!!! Is this OK? !!!)
		 */
                if (R_success_proc(pRD))
		   (*(R_success_proc(pRD))) (R_service(pRD),
				R_success_client_data(pRD),
				ppchMessage, count);
	    }
	}

	/* 
	 * Discard our client data wrapper.
	 * This was allocated when the request was sent.
	 */
	XtFree((char *) pRD);
    }
}

/* 
 * _DtSvcRequestSend -- send a request to a service
 * 
 */
int
#ifdef _NO_PROTO
_DtSvcRequestSend (service, request, request_args, num_args, 
		success_proc, success_client_data, fail_proc,
		fail_client_data)
    DtSvcHandle	service;
    String		request;
    String *		request_args;
    int			num_args;
    DtSvcStatusProc	success_proc;
    Pointer		success_client_data;
    DtSvcStatusProc	fail_proc;
    Pointer		fail_client_data;
#else /* _NO_PROTO */
_DtSvcRequestSend (
    DtSvcHandle	service,
    String		request,
    String *		request_args,
    int			num_args,
    DtSvcStatusProc	success_proc,
    Pointer		success_client_data,
    DtSvcStatusProc	fail_proc,
    Pointer		fail_client_data 
    )
#endif /* _NO_PROTO */
{
    String *		psRequest;
    int 		i;
    DtSvcReplyData *	pReplyData;
    int 		status;

    /* validate service handle */
    if (!service) return (DT_SVC_FAIL);

    /* 
     * Allocate our own client data wrapper, this is freed when
     * the reply comes back.
     */
    pReplyData = XtNew(DtSvcReplyData);

    /* fill in the data we need to deal with the reply */
    R_service(pReplyData) = service;
    R_success_proc(pReplyData) = success_proc;
    R_success_client_data(pReplyData) = success_client_data;
    R_fail_proc(pReplyData) = fail_proc;
    R_fail_client_data(pReplyData) = fail_client_data;

    /* make the request */
    _DtSvcLogMsg("_DtSvcRequestSend", service, request, num_args, request_args);
    if (H_use_tt(service))
    {
	status = _DtSvcMsgSendTT (service, TT_REQUEST, request,
				request_args, num_args, pReplyData);
    }
    else
    {

	/*
	 * Repackage the message.
	 */
	psRequest = (String *) XtMalloc ((1+num_args) * sizeof (String));

	/* copy the string pointers into our vector */
	psRequest[0] = request;
	for (i=0; i < num_args; i++)
	{
	    psRequest[i+1] = request_args[i];
	}
	status = _DtMsgServiceRequest (H_handle(service), psRequest,
				num_args+1, _DtSvcReplyHandler, pReplyData);

	/* free our repackaged message (string vector) */
	XtFree ((char *) psRequest);
    }

    /* translate the return code */
    switch (status)
    {
	case dtmsg_SUCCESS:
	    status = DT_SVC_SUCCESS;
	    break;

	default:
	case dtmsg_FAIL:
	    status = DT_SVC_FAIL;
	    break;
    }

    return (status);
}

/* 
 * _DtSvcRequestReply -- send a reply to a request that has been
 *				received
 * 
 */
int
#ifdef _NO_PROTO
_DtSvcRequestReply (service, reply_context, reply_args, num_args, 
		success)
    DtSvcHandle	service;
    DtSvcMsgContext    reply_context;
    String *		reply_args;
    int			num_args;
    Boolean		success;
#else /* _NO_PROTO */
_DtSvcRequestReply (
    DtSvcHandle	service,
    DtSvcMsgContext    reply_context,
    String *		reply_args,
    int			num_args,
    Boolean		success
    )
#endif /* _NO_PROTO */
{
    int 		status;
    int			i;
    char **		ppchMessage;


    /* validate service handle */
    if (!service) return (DT_SVC_FAIL);

    /*
     * 
     */
    _DtSvcLogMsg ("_DtSvcRequestReply", service,
		  success ? DT_SVC_MSG_SUCCESS : DT_SVC_MSG_FAIL,
		  num_args, reply_args);
    if (H_use_tt(service))
    {
	status = _DtSvcRequestReplyTT (service, reply_context, reply_args,
			num_args, success);
    }
    else
    {
	/*
	 * Construct the message.
	 */
	ppchMessage = (char **) XtMalloc ((1+num_args) * sizeof (char *));

	if (success)
	{
	    ppchMessage[0] = XtNewString (DT_SVC_MSG_SUCCESS);
	}
	else
	{
	    ppchMessage[0] = XtNewString (DT_SVC_MSG_FAIL);
	}

	/* copy the rest of the arguments into our string vector */
	for (i=0; i < num_args; i++)
	{
	    ppchMessage[i+1] = reply_args[i];
	}
	status = _DtMsgServiceReply (H_handle(service),
			(DtMsgContext) reply_context,
			ppchMessage, num_args+1);

	/* free up stuff we've allocated */
	XtFree ((char *) ppchMessage[0]);
	XtFree ((char *) ppchMessage);
    }

    /* translate the return code */
    switch (status)
    {
	case dtmsg_SUCCESS:
	    status = DT_SVC_SUCCESS;
	    break;

	default:
	case dtmsg_FAIL:
	    status = DT_SVC_FAIL;
	    break;
    }
    return (status);
}
