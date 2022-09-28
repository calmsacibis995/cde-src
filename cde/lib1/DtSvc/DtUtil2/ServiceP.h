/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/* -*-C-*-
*******************************************************************************
*
* File:         ServiceP.h
* RCS:          $XConsortium: ServiceP.h /main/cde1_maint/1 1995/07/14 20:38:22 drk $
* Description:  Private header file for sending requests/responses to
*		and from DT services.
* Language:     C
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 1993, Hewlett-Packard, all rights reserved.
*
*******************************************************************************
*/

#ifndef _Dt_ServiceP_h
#define _Dt_ServiceP_h

#include <Dt/Msg.h>
#include <Dt/Service.h>
#include <Tt/tt_c.h>

/*
 * Private strings
 */
#define DT_SVC_ACTION_PREFIX	"Invoke"

/*
 * Private data types
 */

/*
 * Receive message filter entry
 *
 * One per message type filtered. 
 */
typedef struct _DtSvcFilterEntry {
	String			pchRequest;
	DtSvcReceiveProc	receive_proc;
	Pointer			client_data;
} DtSvcFilterEntry, *DtSvcFilterList;

/*
 * Notify message filter entry
 *
 * One per notify message type filtered. 
 */
typedef struct _DtSvcNotifyFilterEntry {
	String			notify;
	DtSvcMessageProc	message_proc;
	Pointer			client_data;
} DtSvcNotifyFilterEntry, *DtSvcNotifyFilterList;

/*
 * Filter access macros
 */
#define F_request(f)		((f)->pchRequest)
#define F_receive_proc(f)	((f)->receive_proc)
#define F_client_data(f)	((f)->client_data)

#define N_notify(f)		((f)->notify)
#define N_message_proc(f)	((f)->message_proc)
#define N_client_data(f)	((f)->client_data)

/* number of additional entries to allocate if list fills up */
#define F_alloc_amt		5

/*
 * Service specific data.
 *
 * This is added to the handle when we register for a service.
 * The zero'th element of the filter list always has the
 * callback to call for unfiltered messages. If a message isn't
 * sent to one of the other callbacks in the list, it will be
 * sent to the unfiltered one.
 */
typedef struct _DtSvcData {
    DtSvcFilterList		request_filters;
    int				num_request_filters;
    int				size_request_filters;
    DtMsgStatusProc		lose_proc;
    Pointer			lose_client_data;
} DtSvcData;

/*
 * Notify specific data.
 *
 * This is added to the handle when we register for notification.
 * The zero'th element of the filter list always has the
 * callback to call for unfiltered messages. If a message isn't
 * sent to one of the other callbacks in the list, it will be
 * sent to the unfiltered one.
 */
typedef struct _DtSvcNotifyData {
    DtSvcNotifyFilterList	notify_filters;
    int				num_notify_filters;
    int				size_notify_filters;
} DtSvcNotifyData;

/*
 * The service handle structure. 
 * This is an opaque data structure to the outside world.
 */
struct _DtSvcHandle {
    DtMsgHandle		handle;
    DtSvcData *		service_data;
    DtSvcNotifyData *		notify_data;
    Boolean		use_tt;
    Tt_pattern		pat;
};

/*
 * Access macros for service handles and data
 */
#define H_handle(h)		((h)->handle)

#define H_service_data(h)	((h)->service_data)
#define H_lose_proc(h)		((h)->service_data->lose_proc)
#define H_lose_client_data(h)	((h)->service_data->lose_client_data)
#define H_filters(h)		((h)->service_data->request_filters)
#define H_num_filters(h)	((h)->service_data->num_request_filters)
#define H_size_filters(h)	((h)->service_data->size_request_filters)

#define H_notify_data(h)	((h)->notify_data)
#define H_notify_filters(h)	((h)->notify_data->notify_filters)
#define H_num_notify_filters(h)	((h)->notify_data->num_notify_filters)
#define H_size_notify_filters(h) ((h)->notify_data->size_notify_filters)

#define H_use_tt(h)		((h)->use_tt)
#define H_pat(h)		((h)->pat)


/*
 * Client Data saved when a request is sent.
 * Returned to the reply callback.
 */
typedef struct _DtSvcReplyData {
    DtSvcHandle	service;
    DtSvcMessageProc 	success_proc;
    Pointer		success_client_data;
    DtSvcStatusProc 	fail_proc;
    Pointer		fail_client_data;
} DtSvcReplyData;

/*
 * Access macros for service reply data
 */
#define R_service(r)		((r)->service)
#define R_success_proc(r)	((r)->success_proc)
#define R_success_client_data(r)	((r)->success_client_data)
#define R_fail_proc(r)		((r)->fail_proc)
#define R_fail_client_data(r)	((r)->fail_client_data)

/*
 * Service Start data
 */
typedef struct _DtSvcStartData {
    DtSvcHandle	service;
    String		request;
    String *		request_args;
    int			num_args;
    DtSvcStatusProc	success_proc;
    Pointer		success_client_data;
    DtSvcStatusProc	fail_proc;
    Pointer		fail_client_data;
} DtSvcStartData;

/*
 * Access macros for service start data
 */
#define S_service(s)		((s)->service)
#define S_request(s)		((s)->request)
#define S_request_args(s)	((s)->request_args)
#define S_num_args(s)		((s)->num_args)
#define S_success_proc(s)	((s)->success_proc)
#define S_success_client_data(s) ((s)->success_client_data)
#define S_fail_proc(s)		((s)->fail_proc)
#define S_fail_client_data(s)	((s)->fail_client_data)

/*
 * Check to see if a service is registered after 
 * SVC_START_INTERVAL milliseconds
 */
#define SVC_START_INTERVAL	500
/*
 * Re-check to see if a service is registered every
 * SVC_START_INTERVAL milliseconds for SVC_START_RETRIES
 * before giving up.
 */
#define SVC_START_RETRIES	100

/*
 * Service start timeout data
 */
typedef struct _DtSvcStartTimeoutData {
    DtSvcHandle	service;
    DtSvcStatusProc	status_proc;
    Pointer		client_data;
    int			retries;
} DtSvcStartTimeoutData;

/*
 * Access macros for service start timeout data
 */
#define T_service(s)		((s)->service)
#define T_status_proc(s)	((s)->status_proc)
#define T_client_data(s)	((s)->client_data)
#define T_retries(s)		((s)->retries)

#endif /*_Dt_ServiceP_h*/
/* Do not add anything after this endif. */
