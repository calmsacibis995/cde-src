/* $XConsortium: SvcStart.c /main/cde1_maint/1 1995/07/17 18:16:22 drk $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **   File:         SvcStart.c
 **
 **   Project:	    DT
 **
 **   Description:  Functions that cause services to be started
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
#include <Dt/Action.h>


/*
 * _DtSvcStartTimeout -- timer callback proc  (waiting for service
 *			to start)
 */
static void
#ifdef _NO_PROTO
_DtSvcStartTimeout (client_data, id)
    XtPointer		client_data;
    XtIntervalId	id;
#else /* _NO_PROTO */
_DtSvcStartTimeout (
    XtPointer		client_data,
    XtIntervalId	id 
    )
#endif /* _NO_PROTO */
{
    DtSvcStartTimeoutData *pTD = (DtSvcStartTimeoutData *) client_data;
    Widget widget;

    /*
     * Is service running?
     */
    if (_DtSvcIsRegistered (T_service(pTD)))
    {
	/*
	 * Call the status proc, it's running!
	 */
	if (T_status_proc(pTD))
	{
	    (*(T_status_proc(pTD))) (T_service(pTD), DT_SVC_START,
				T_client_data(pTD));
	}

	/* release timer client data */
	XtFree ((char *) pTD);
    }
    else
    {
	/* Service is not running yet */

	/* reduce retry count and see if we try again */
	T_retries(pTD) -= 1;

	if (T_retries(pTD) > 0)
	{
	    /*
	     * Service is not running, reset timer.
	     */
	    widget = _DtSvcGetWidget(T_service(pTD));
	    (void) XtAppAddTimeOut(
		    XtWidgetToApplicationContext(widget),
		    SVC_START_INTERVAL, 
		    (XtTimerCallbackProc) _DtSvcStartTimeout, 
		    (XtPointer) pTD);
	}
	else
	{
	    /* no more retries, time to give up */
	    /*
	     * Call the status proc, it's not running!!
	     */
	    if (T_status_proc(pTD))
	    {
		(*(T_status_proc(pTD))) (T_service(pTD), DT_SVC_NO_START,
				    T_client_data(pTD));
	    }

	    /* release timer client data */
	    XtFree ((char *) pTD);
	}
    }
}


/*
 * _DtSvcStartGetActionName -- return an action name for a service
 *				so that the service can be started if
 * 				it is not running.
 */
static String
#ifdef _NO_PROTO
_DtSvcStartGetActionName (service)
    DtSvcHandle	service;
#else /* _NO_PROTO */
_DtSvcStartGetActionName (
    DtSvcHandle	service 
    )
#endif /* _NO_PROTO */
{
    String sName, sActionName;
    int len;

    sName = _DtSvcGetName(service);
    len = strlen(sName) + strlen(DT_SVC_ACTION_PREFIX) + 1;

    sActionName = (String) XtMalloc (len);

    strcpy (sActionName, DT_SVC_ACTION_PREFIX);
    strcat (sActionName, sName);

    return (sActionName);
}


/*
 * _DtSvcStart -- start up a service
 */
void
#ifdef _NO_PROTO
_DtSvcStart (service, status_proc, client_data)
    DtSvcHandle	service;
    DtSvcStatusProc	status_proc;
    Pointer		client_data;
#else /* _NO_PROTO */
_DtSvcStart (
    DtSvcHandle	service,
    DtSvcStatusProc	status_proc,
    Pointer		client_data 
    )
#endif /* _NO_PROTO */
{
    DtSvcStartTimeoutData *pTD;
    Widget widget;
    String sActionName;

    /*
     * Invoke the service
     */
    sActionName = _DtSvcStartGetActionName (service);

    widget = _DtSvcGetWidget(service);
    DtActionInvoke (	widget, 		/* widget */
			sActionName, 		/* action name */
			0, 			/* action arg count */
			NULL, 			/* action arg ptr */
			NULL, 			/* term parms */
			NULL, 			/* exec host */
			NULL,			/* context dir */
			True,			/* use indicator */
                        NULL,			/* status callback proc */
                        NULL);			/* callback client data */
  

    /*
     * Repackage data for timeout processing.
     * We poll the service to detect when it has started.
     */
    pTD = XtNew(DtSvcStartTimeoutData);
    T_service(pTD) =	service;
    T_status_proc(pTD) = status_proc;
    T_client_data(pTD) = client_data;
    T_retries(pTD) = SVC_START_RETRIES;

    (void) XtAppAddTimeOut(XtWidgetToApplicationContext(widget),
		SVC_START_INTERVAL, 
		(XtTimerCallbackProc) _DtSvcStartTimeout, 
		(XtPointer) pTD);

}


/*
 * _DtSvcSendStartStatus -- status proc used when trying to send a 
 *				message to a service that isn't up
 *				up yet.
 */
static void
#ifdef _NO_PROTO
_DtSvcSendStartStatus (service, status, client_data)
    DtSvcHandle	service;
    int			status;
    Pointer		client_data;
#else /* _NO_PROTO */
_DtSvcSendStartStatus (
    DtSvcHandle	service,
    int			status,
    Pointer		client_data 
    )
#endif /* _NO_PROTO */
{
    DtSvcStartData *pSD = (DtSvcStartData *)client_data;
    String message[1];
    int i;
    char ** req_args;

    if (pSD)
    {
	if (status == DT_SVC_START)
	{
	    /* The service has started, send the message */
	    (void) _DtSvcRequestSend (S_service(pSD), 
			    S_request(pSD), S_request_args(pSD), 
			    S_num_args(pSD), S_success_proc(pSD), 
			    S_success_client_data(pSD), 
			    S_fail_proc(pSD), S_fail_client_data(pSD));
	}
	else
	{
	    /* The service failed to start, call the message's fail
	     * callback.
	     * NOTE: This is a MessageProc!
	     */
	    if (S_fail_proc(pSD))
	    {
		message[1] = XtNewString(DT_SVC_MSG_FAIL);

		(*(S_fail_proc(pSD))) (S_service(pSD),
					S_fail_client_data(pSD),
					message, 1);

		XtFree ((char *) message[1]);
	    }
	}

        /* Free up our copy of the request argument strings */
        req_args = S_request_args(pSD);
        for (i = 0; i < S_num_args(pSD); i++)
           XtFree(req_args[i]);
        XtFree((char *)req_args);
        XtFree(S_request(pSD));

	/* free our client data */
	XtFree ((char *) pSD);
    }
}


/* 
 * _DtSvcRequestSendStart -- send a request to a service.
 *			If the service isn't running, start it and 
 *			then send the request
 */
int
#ifdef _NO_PROTO
_DtSvcRequestSendStart (service, request, request_args, num_args, 
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
_DtSvcRequestSendStart (
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
    int status;
    DtSvcStartData	*pSD;
    int i;
    char ** req_args;

    status = _DtSvcRequestSend (service, request, request_args, 
		num_args, success_proc, success_client_data, 
		fail_proc, fail_client_data);

    if (status != DT_SVC_SUCCESS)
    {
	/*
	 * Need to start the service
	 * 
	 * Package up the parameters we need 
	 *
	 */
	pSD = XtNew(DtSvcStartData);

	S_service(pSD) =		service;

	/* We need to save copies of all the string parameters */

	S_request(pSD) =		XtNewString(request);
        if (num_args > 0)
        {
           req_args = (char **)XtMalloc(sizeof(char *) * num_args);
           for (i = 0; i < num_args; i++)
              req_args[i] = XtNewString(request_args[i]);
	   S_request_args(pSD) = req_args;
        }
        else
	   S_request_args(pSD) = NULL;

	S_num_args(pSD) =		num_args;
	S_success_proc(pSD) =		success_proc;
	S_success_client_data(pSD) =	success_client_data;
	S_fail_proc(pSD) =		fail_proc;
	S_fail_client_data(pSD) =	fail_client_data;
	
	_DtSvcStart (service, _DtSvcSendStartStatus, (Pointer) pSD);
	status = DT_SVC_SUCCESS;
    }

    return (status);
}
