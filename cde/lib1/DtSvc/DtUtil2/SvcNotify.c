/* $XConsortium: SvcNotify.c /main/cde1_maint/1 1995/07/17 18:16:07 drk $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **   File:         SvcNotify.c
 **
 **   Project:	    DT
 **
 **   Description:  Broadcast messaging support
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

/*
 * _DtSvcFilterNotifyPass -- Return True if this filter passes this message
 */
static Boolean
#ifdef _NO_PROTO
_DtSvcFilterNotifyPass (pFilter, ppchMessage, count)
    DtSvcNotifyFilterEntry *	pFilterEntry;
    char **			ppchMessage;
    int				count;
#else /* _NO_PROTO */
_DtSvcFilterNotifyPass (
    DtSvcNotifyFilterEntry * 	pFilterEntry,
    char **			ppchMessage,
    int				count 
    )
#endif /* _NO_PROTO */
{
   Boolean bPass = False;

   if (N_notify(pFilterEntry))
   {
       if (!strcmp(N_notify(pFilterEntry), ppchMessage[0]))
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
 * _DtSvcFilterNotify -- Filter the notify message and return
 * 			the correct receive procedure and client data
 */
static void
#ifdef _NO_PROTO
_DtSvcFilterNotify (ngroup, ppchMessage, count, 
			pMessageProc, pclient_data)
    DtSvcHandle	service;
    char **		ppchMessage;
    int			count;
    DtSvcMessageProc *	pMessageProc;
    Pointer *		pclient_data;
#else /* _NO_PROTO */
_DtSvcFilterNotify (
    DtSvcHandle	ngroup,
    char **		ppchMessage,
    int			count,
    DtSvcMessageProc *	pMessageProc,
    Pointer *		pclient_data 
    )
#endif /* _NO_PROTO */
{
    DtSvcMessageProc 		message_proc = NULL;
    Pointer			client_data = NULL;
    int 			i, num_filters;
    DtSvcNotifyFilterList	pFilters;

    /* insure minimal amount to continue */
    if (ngroup && (count > 0) &&
	ppchMessage && *ppchMessage)
    {
	/*
	 * Search backward through the list of filters to find a matching
	 * one. The zeroth element of the list is the default handler.
	 */

	pFilters = H_notify_filters(ngroup);
	num_filters = H_num_notify_filters(ngroup);

	for (i = (num_filters - 1); i >= 0; i--)
	{
	    if (_DtSvcFilterNotifyPass (&pFilters[i], ppchMessage, count))
	    {
		message_proc = N_message_proc(&pFilters[i]);
		client_data = N_client_data(&pFilters[i]);
		break;
	    }
	}
    }

    /* return whatever we found */
    *pMessageProc = message_proc;
    *pclient_data = client_data;
}

/*
 * _DtSvcNotifyHandler -- handle notify messages here, distribute based
 * 			on how filtering is to be done.
 */
void
#ifdef _NO_PROTO
_DtSvcNotifyHandler (handle, replyContext, pClientData, 
			ppchMessage, count)
    DtMsgHandle	handle;
    DtMsgContext	replyContext;
    Pointer		pClientData;
    char **		ppchMessage;
    int			count;
#else /* _NO_PROTO */
_DtSvcNotifyHandler (
    DtMsgHandle	handle,
    DtMsgContext	replyContext,
    Pointer		pClientData,
    char **		ppchMessage,
    int			count 
    )
#endif /* _NO_PROTO */
{
	int i;
    DtSvcHandle	ngroup = (DtSvcHandle) pClientData;
    DtSvcMessageProc	message_proc;
    Pointer		client_data;

    if (ngroup)
    {
	_DtSvcFilterNotify (ngroup, ppchMessage, count, &message_proc, 
			    &client_data);

	if (message_proc)
	{
	    _DtSvcLogMsg ("_DtSvcNotifyHandler", ngroup, 0, count,
			  ppchMessage);
	    (*(message_proc)) (ngroup, client_data, ppchMessage, count);
	}
    }
}

/*
 * _DtSvcNotifyGroupRegister -- Register to receive a group of 
 *				notify messages
 */
void
#ifdef _NO_PROTO
_DtSvcNotifyGroupRegister (ngroup, message_proc, client_data)
    DtSvcHandle	ngroup;
    DtSvcMessageProc	message_proc;
    Pointer		client_data;
#else /* _NO_PROTO */
_DtSvcNotifyGroupRegister (
    DtSvcHandle	ngroup,
    DtSvcMessageProc	message_proc,
    Pointer		client_data 
    )
#endif /* _NO_PROTO */
{
    DtSvcNotifyFilterEntry *pFilter;

    /* validate service handle */
    if (!ngroup) return;

    if (H_notify_data(ngroup))
    {
	/* 
	 * Already registered something..
	 * Replace default message handler with this one.
	 */
	pFilter = H_notify_filters(ngroup);
    }
    else
    {
	/* allocate new ngroup data and initialize */
	H_notify_data(ngroup) = XtNew(DtSvcNotifyData);

	/* initialize filter list by starting out with the unfiltered
	 * request handler.
	 */
	pFilter = XtNew(DtSvcNotifyFilterEntry);
	H_notify_filters(ngroup) = pFilter;
	H_num_notify_filters(ngroup) = 1;
	H_size_notify_filters(ngroup) = 1;

	/* 
	 * Register interest in this message group
	 *
	 * Use common message proc at this level.
	 * This message proc will implement the message filtering
	 * and properly distribute the callbacks.
	 */
	if (H_use_tt(ngroup))
	{
	    _DtSvcRegisterTT (ngroup, TT_OBSERVE);
	}
	else
	{
	    (void) _DtMsgBroadcastRegister (H_handle(ngroup), 
			    _DtSvcNotifyHandler, (Pointer) ngroup);
	}

    }

    /* set up default "filter" */
    N_notify(pFilter) = NULL; 	/* no filter specified */
    N_message_proc(pFilter) = message_proc;
    N_client_data(pFilter) = client_data;
}

/*
 * _DtSvcNotifyMsgRegister -- Register to receive a specific
 *				notify message
 */
void
#ifdef _NO_PROTO
_DtSvcNotifyMsgRegister (ngroup, notify, message_proc, client_data)
    DtSvcHandle	ngroup;
    String		notify;
    DtSvcMessageProc	message_proc;
    Pointer		client_data;
#else /* _NO_PROTO */
_DtSvcNotifyMsgRegister (
    DtSvcHandle	ngroup,
    String		notify,
    DtSvcMessageProc	message_proc,
    Pointer		client_data 
    )
#endif /* _NO_PROTO */
{
    DtSvcNotifyFilterList 	pFilters;
    DtSvcNotifyFilterEntry * 	pFE;
    int 			i;
    Boolean 			bReplaced;

    if (!H_notify_data(ngroup))
    {
	/*
	 * Base level registration not yet done...
	 * Do minimalist registration to allocate data. Register
	 * no default message handler. 
	 */
	_DtSvcNotifyGroupRegister (ngroup, NULL, NULL);
    }

    if (H_notify_data(ngroup))
    {
	pFilters = H_notify_filters(ngroup);
	bReplaced = False;

	/*
	 * Check for redundant entry.
	 * (Remember, zeroth entry is default handler and doesn't
	 *  need checking).
	 */
	for (i = (H_num_notify_filters(ngroup))-1; i > 0; i--)
	{
	    pFE = &pFilters[i];
	    if (!strcmp(N_notify(pFE), notify))
	    {
		N_message_proc(pFE) = message_proc;
		N_client_data(pFE) = client_data;
		bReplaced = True;
	    }
	}

	if (!bReplaced)
	{
	    /*
	     * Need to add a new filter entry
	     */
	    if (H_num_notify_filters(ngroup) >= H_size_notify_filters(ngroup))
	    {
		/* allocate space for more */
		H_size_notify_filters(ngroup) += F_alloc_amt;
		pFilters = (DtSvcNotifyFilterEntry *) 
			XtRealloc ((char *) H_notify_filters(ngroup), 
				   (H_size_notify_filters(ngroup) *
					sizeof(DtSvcNotifyFilterEntry)));
		H_notify_filters(ngroup) = pFilters;
	    }
	    H_num_notify_filters(ngroup) += 1;
	    i = H_num_notify_filters(ngroup) - 1;
	    pFE = &pFilters[i];

	    N_notify(pFE) = XtNewString(notify);
	    N_message_proc(pFE) = message_proc;
	    N_client_data(pFE) = client_data;
	}
    }
}

/*
 * _DtSvcNotifyMsgUnregister -- Unregister to receive a specific
 *				notify message
 */
void
#ifdef _NO_PROTO
_DtSvcNotifyMsgUnregister (ngroup, notify, message_proc, client_data)
    DtSvcHandle	ngroup;
    String		notify;
    DtSvcMessageProc	message_proc;
    Pointer		client_data;
#else /* _NO_PROTO */
_DtSvcNotifyMsgUnregister (
    DtSvcHandle	ngroup,
    String		notify,
    DtSvcMessageProc	message_proc,
    Pointer		client_data 
    )
#endif /* _NO_PROTO */
{
    DtSvcNotifyFilterList 	pFilters;
    DtSvcNotifyFilterEntry * 	pFE;
    int 			i;

    if (H_notify_data(ngroup))
    {
	pFilters = H_notify_filters(ngroup);

	/*
	 * Find the entry we're deleting
	 * (Remember, zeroth entry is default handler and doesn't
	 *  need checking).
	 */
	for (i = (H_num_notify_filters(ngroup))-1; i > 0; i--)
	{
	    pFE = &pFilters[i];
	    if ((!strcmp(N_notify(pFE), notify)) &&
		(N_message_proc(pFE) == message_proc) &&
		(N_client_data(pFE) == client_data))
	    {
		/* This is it, copy the filters up to fill the gap */
		(void) memmove (&pFilters[i], &pFilters[i+1], 
			((H_num_notify_filters(ngroup) - 1 - i)*
			    sizeof(DtSvcNotifyFilterEntry)));

		/* Adjust the filter count */
		H_num_notify_filters(ngroup) -= 1;
	    }
	}
    }
}


/*
 * _DtSvcNotifySend -- Send a notify message
 */
int
#ifdef _NO_PROTO
_DtSvcNotifySend (ngroup, notify, notify_args, num_args)
    DtSvcHandle	ngroup;
    String		notify;
    String *		notify_args;
    int			num_args;
#else /* _NO_PROTO */
_DtSvcNotifySend (
    DtSvcHandle	ngroup,
    String		notify,
    String *		notify_args,
    int			num_args 
    )
#endif /* _NO_PROTO */
{
    String *		psMessage;
    int 		i;
    int 		status;

    /* validate service handle */
    if (!ngroup) return;

    /*
     * Repackage the message.
     */
    psMessage = (String *) XtMalloc ((1+num_args) * sizeof (String));

    /* copy the string pointers into our vector */
    psMessage[0] = notify;
    for (i=0; i < num_args; i++)
    {
	psMessage[i+1] = notify_args[i];
    }

    /* make the request */
    _DtSvcLogMsg ("_DtSvcNotifySend", ngroup, notify, num_args, notify_args);
    if (H_use_tt(ngroup))
    {
	status = _DtSvcMsgSendTT (ngroup, TT_NOTICE, notify,
				notify_args, num_args, 0);
    }
    else
    {
	status = _DtMsgBroadcastSend (H_handle(ngroup), psMessage, num_args+1);
    }

    /* free our repackaged message (string vector) */
    XtFree ((char *) psMessage);

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
